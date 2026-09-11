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

#pragma once

#include <functional>
#include <QJSValue>
#include <QString>

#include "DataModel/Scripting/ScriptDryRun.h"

class QJSEngine;

namespace DataModel {

/**
 * @brief Why an output-widget transmit script was rejected, or that it was accepted.
 */
enum class TransmitScriptStatus {
  Ok,
  Timeout,
  CompileError,
  NoEntryPoint,
  Empty,
  HostUnavailable,
};

/**
 * @brief One verdict for a transmit script. @c message carries the engine's own error text for a
 *        CompileError and is empty otherwise: the human wording for the other outcomes belongs to
 *        the caller, because the editor wants a short translated line and the assistant wants the
 *        long corrective paragraph its tool description promises.
 */
struct TransmitScriptVerdict {
  int line                    = 0;
  TransmitScriptStatus status = TransmitScriptStatus::NoEntryPoint;
  QString message;

  [[nodiscard]] bool ok() const noexcept { return status == TransmitScriptStatus::Ok; }

  /**
   * @brief Whether this verdict may be written back to the project. Deliberately wider than ok():
   *        deleting a transmit function is a valid edit, and rejecting it left the only way to
   *        clear one leading nowhere -- the project kept the old script while the editor showed an
   *        empty buffer.
   */
  [[nodiscard]] bool persistable() const noexcept
  {
    return status == TransmitScriptStatus::Ok || status == TransmitScriptStatus::Empty;
  }
};

/**
 * @brief Wraps @p code in the IIFE every transmit surface compiles. One definition, because the
 *        live control and the verdict must agree byte for byte: a wrapper that appends `; return`
 *        on the same line lands inside a trailing // comment, so a script the editor accepted
 *        failed to compile in the widget.
 */
[[nodiscard]] QString wrapTransmitScript(const QString& code);

/**
 * @brief Installs the host surface the script is compiled against; see prepareTransmitScriptEngine.
 *        Returns false when the surface could not be established, which is reported as
 *        HostUnavailable rather than as a verdict about the user's code.
 */
using TransmitScriptPrepare = std::function<bool(QJSEngine&)>;

/**
 * @brief The single verdict for a transmit script, shared by the editor and the assistant's dry
 *        run so one script can never get two answers. @p prepare is not optional: a verdict only
 *        means the same thing across callers while the environment behind it is the same one, and
 *        an unprepared engine would reject a script that merely touches a host helper.
 */
[[nodiscard]] TransmitScriptVerdict checkTransmitScript(const QString& code,
                                                        const TransmitScriptPrepare& prepare);

/**
 * @brief The same verdict, compiled into an already-prepared @p session and handing back the
 *        entry point in @p transmitOut. Callers that go on to execute the script use this so the
 *        function they run is the one that was judged, evaluated through one wrapper definition.
 */
[[nodiscard]] TransmitScriptVerdict compileTransmitScript(const QString& code,
                                                          ScriptDryRun& session,
                                                          QJSValue& transmitOut);

}  // namespace DataModel
