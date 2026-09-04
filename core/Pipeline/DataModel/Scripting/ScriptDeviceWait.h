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
#include <QByteArray>
#include <QJSValue>
#include <QString>
#include <QVariantMap>

namespace DataModel {

class JsWatchdog;
class ControlApiMarshaller;

/**
 * @brief The deviceWriteAndWait() request/reply loop shared by the control script and the macro
 *        runner. Both hosts differ only in what makes the wait give up, so the abort test is a
 *        parameter: the loop itself blocks the calling worker thread and never the GUI, polls the
 *        capture buffer through the GUI-thread marshaller, and re-arms the script watchdog after
 *        the wait so device latency is never billed against the script budget.
 */
namespace ScriptDeviceWait {

/**
 * @brief Called on the worker thread between poll slices; true stops the wait.
 */
using AbortPredicate = std::function<bool()>;

/**
 * @brief Coerces a JS string or byte array into raw bytes; anything else yields empty bytes.
 */
[[nodiscard]] QByteArray payloadFromJsValue(const QJSValue& data);

/**
 * @brief Writes @p data to @p sourceId, then waits for a reply satisfying @p until (terminator
 *        string, byte count, or any non-empty reply) or until @p timeoutMs elapses. Answers
 *        {ok, data, bytesRead, timedOut, error?}; @p abortMessage names the host's own reason
 *        when @p aborted fires before the write.
 */
[[nodiscard]] QVariantMap writeAndWait(ControlApiMarshaller* marshaller,
                                       JsWatchdog* watchdog,
                                       const AbortPredicate& aborted,
                                       const QString& abortMessage,
                                       const QJSValue& data,
                                       int timeoutMs,
                                       const QJSValue& until,
                                       int sourceId);

}  // namespace ScriptDeviceWait

}  // namespace DataModel
