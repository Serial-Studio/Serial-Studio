/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Scripting/TransmitScriptEnvironment.h"

#include <QDebug>
#include <QJSEngine>

#include "Core/SSAssert.h"
#include "DataModel/Scripting/ScriptApiCall.h"

/**
 * @brief Replaces the effectful bridges with recorders. The SDK prelude resolves __ss_dw / __ss_af
 *        / __ss_bridge / __ss by name inside its wrapper closures at call time, so reassigning
 *        them here neuters deviceWrite(), actionFire(), apiCall(), tableSet(), tableSetH() and
 *        mqttPublish() while every wrapper the script can name still exists. The __ss replacement
 *        forwards every read to the real bridge: a script that reads a table or a dataset to build
 *        its payload has to preview as the same script that ships. Each replacement carries a
 *        judged marker so the install can be verified rather than assumed.
 */
static const auto kInertBridges =
  QStringLiteral("(function() {"
                 "  var judged = { writes: 0, bytes: 0, fires: 0, calls: 0,"
                 "                 tableWrites: 0, publishes: 0 };"
                 "  __ss_judged = judged;"
                 "  if (typeof __ss_dw !== 'undefined')"
                 "    __ss_dw = { judged: true, write: function(d, s) {"
                 "      judged.writes += 1;"
                 "      judged.bytes += (d && d.length) ? d.length : 0;"
                 "      return { ok: true, judged: true, bytesWritten: 0 }; } };"
                 "  if (typeof __ss_af !== 'undefined')"
                 "    __ss_af = { judged: true, fire: function(a) {"
                 "      judged.fires += 1;"
                 "      return { ok: true, judged: true }; } };"
                 "  if (typeof __ss_bridge !== 'undefined')"
                 "    __ss_bridge = { judged: true, call: function(m, p) {"
                 "      judged.calls += 1;"
                 "      return { ok: false, judged: true, error: 'inert while validating' }; } };"
                 "  if (typeof __ss !== 'undefined') {"
                 "    var live = __ss;"
                 "    var inert = { judged: true,"
                 "      tableGet: function(t, r) { return live.tableGet(t, r); },"
                 "      tableGetH: function(h) { return live.tableGetH(h); },"
                 "      tableHandle: function(t, r) { return live.tableHandle(t, r); },"
                 "      tableHandleMany: function(t, rs) { return live.tableHandleMany(t, rs); },"
                 "      datasetGetRaw: function(s) { return live.datasetGetRaw(s); },"
                 "      datasetGetFinal: function(s) { return live.datasetGetFinal(s); },"
                 "      tableSet: function(t, r, v) { judged.tableWrites += 1; },"
                 "      tableSetH: function(h, v) { judged.tableWrites += 1; } };"
                 "    if (live.mqttPublish)"
                 "      inert.mqttPublish = function(t, p, q, r) {"
                 "        judged.publishes += 1;"
                 "        return 0; };"
                 "    __ss = inert;"
                 "  }"
                 "})();");

/**
 * @brief Confirms every effectful bridge present in the engine is the recorder rather than the
 *        live object. The masking is an assignment, and an assignment that silently did not take
 *        would leave a preview able to actuate hardware, so it is checked rather than assumed.
 */
static const auto kInertProbe = QStringLiteral(
  "(function() {"
  "  if (typeof __ss_judged !== 'object' || __ss_judged === null) return false;"
  "  if (typeof __ss_dw !== 'undefined' && __ss_dw.judged !== true) return false;"
  "  if (typeof __ss_af !== 'undefined' && __ss_af.judged !== true) return false;"
  "  if (typeof __ss_bridge !== 'undefined' && __ss_bridge.judged !== true) return false;"
  "  if (typeof __ss !== 'undefined' && __ss.judged !== true) return false;"
  "  return true;"
  "})();");

/**
 * @brief The transmit environment is what installAll() provides, which already carries the table
 *        API through installHelperBridgesJS. Callers that only validate pass any source id: it
 *        selects the device the helpers target at call time, never which names exist, so a
 *        script's validity stays independent of where it would transmit.
 */
bool DataModel::prepareTransmitScriptEngine(QJSEngine& engine,
                                            int sourceId,
                                            TransmitScriptSurface surface)
{
  SS_ASSERT(sourceId >= 0, return false);

  const auto tableApi = (surface == TransmitScriptSurface::Live)
                        ? ScriptApiCall::TableApi::ArmCapture
                        : ScriptApiCall::TableApi::NamesOnly;
  DataModel::ScriptApiCall::installAll(&engine, sourceId, tableApi);

  if (surface == TransmitScriptSurface::Live)
    return true;

  const auto neutered = engine.evaluate(kInertBridges);
  if (neutered.isError()) {
    qWarning() << "[Transmit] could not install the judging surface:" << neutered.toString();
    return false;
  }

  const auto verified = engine.evaluate(kInertProbe);
  if (!verified.toBool()) {
    qWarning() << "[Transmit] the judging surface did not take; refusing to compile";
    return false;
  }

  return true;
}
