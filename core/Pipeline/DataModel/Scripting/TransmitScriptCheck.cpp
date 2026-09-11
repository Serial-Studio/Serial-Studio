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

#include "DataModel/Scripting/TransmitScriptCheck.h"

#include <QJSEngine>
#include <QJSValue>

#include "Core/SSAssert.h"
#include "DataModel/Scripting/ScriptDryRun.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

/**
 * @brief Probes for the entry point with typeof rather than the runtime's bare `return transmit`,
 *        which throws a ReferenceError and would report a missing function as a compile error.
 */
static const auto kProbe = QStringLiteral("(function() { %1\n"
                                          "return typeof transmit === 'function' ? transmit "
                                          ": undefined; })()");

/**
 * @brief Wraps user code in the shared probe; see the header for why it is shared.
 */
QString DataModel::wrapTransmitScript(const QString& code)
{
  return kProbe.arg(code);
}

//--------------------------------------------------------------------------------------------------
// Verdict
//--------------------------------------------------------------------------------------------------

/**
 * @brief Compiles @p code in @p session, which the caller has already prepared, and reports the
 *        verdict while handing back the entry point. This is where the wrapper lives, so every
 *        surface that judges or runs a transmit script evaluates the identical source.
 */
DataModel::TransmitScriptVerdict DataModel::compileTransmitScript(const QString& code,
                                                                  ScriptDryRun& session,
                                                                  QJSValue& transmitOut)
{
  TransmitScriptVerdict verdict;
  SS_ASSERT(session.valid(), {
    verdict.status  = TransmitScriptStatus::CompileError;
    verdict.message = QStringLiteral("Failed to create the validation engine");
    return verdict;
  });

  if (code.trimmed().isEmpty()) {
    verdict.status = TransmitScriptStatus::Empty;
    return verdict;
  }

  transmitOut = session.evaluate(wrapTransmitScript(code), QStringLiteral("transmit_check.js"));
  if (session.timedOut()) {
    verdict.status = TransmitScriptStatus::Timeout;
    return verdict;
  }

  if (transmitOut.isError()) {
    verdict.status  = TransmitScriptStatus::CompileError;
    verdict.line    = transmitOut.property(QStringLiteral("lineNumber")).toInt();
    verdict.message = transmitOut.property(QStringLiteral("message")).toString();
    return verdict;
  }

  SS_ASSERT_LOG(!transmitOut.isError());
  if (transmitOut.isCallable())
    verdict.status = TransmitScriptStatus::Ok;

  return verdict;
}

/**
 * @brief Compiles @p code in a throwaway budgeted engine prepared by @p prepare and reports whether
 *        it defines a callable transmit entry point. The engine is prepared before evaluation
 *        because a script may reach a host helper at top level, and an empty engine would call that
 *        a compile error while the live control runs it fine.
 */
DataModel::TransmitScriptVerdict DataModel::checkTransmitScript(
  const QString& code, const TransmitScriptPrepare& prepare)
{
  ScriptDryRun session(
    ScriptDryRun::Language::JavaScript, kScriptDryRunBudgetMs, "transmitScript.check");

  SS_ASSERT_LOG(session.valid());
  if (session.valid() && prepare && !prepare(*session.jsEngine())) {
    TransmitScriptVerdict verdict;
    verdict.status = TransmitScriptStatus::HostUnavailable;
    return verdict;
  }

  QJSValue transmitFn;
  return compileTransmitScript(code, session, transmitFn);
}
