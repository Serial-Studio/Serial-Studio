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

#include <QCoreApplication>
#include <QSerialPort>
#include <QString>

namespace IO {
namespace Drivers {

/**
 * @brief The decisions the UART driver makes, as pure functions: whether an error ends the link,
 *        whether the driver's own auto-reconnect owns the recovery (spec 0075), whether RTS may be
 *        driven by hand and how a failed Connection Sequence reads (spec 0103). They live apart
 *        from the driver so each can be pinned without a real port.
 */
namespace UartPolicy {

/**
 * @brief Whether @p error ends the link. A custom device path (a by-id node, a socat pty) is
 *        exempt from UnsupportedOperationError alone -- such nodes reject ioctls the driver does
 *        not need -- but NOT from ResourceError: that one means the node is gone, and ignoring it
 *        left the port "open" after an unplug, with write() still returning byte counts.
 */
[[nodiscard]] inline bool isFatalPortError(QSerialPort::SerialPortError error,
                                           bool customPath) noexcept
{
  if (error == QSerialPort::NoError)
    return false;

  if (customPath && error == QSerialPort::UnsupportedOperationError)
    return false;

  return true;
}

/**
 * @brief Whether the driver's opt-in auto-reconnect owns the recovery for @p error, which is the
 *        only case where a drop is not reported to the user.
 */
[[nodiscard]] inline bool shouldAutoReconnect(QSerialPort::SerialPortError error,
                                              bool autoReconnectEnabled) noexcept
{
  return autoReconnectEnabled && error == QSerialPort::ResourceError;
}

/**
 * @brief Whether the driver may drive RTS by hand. Under RTS/CTS flow control the port driver owns
 *        the line and Qt refuses setRequestToSend() with UnsupportedOperationError.
 */
[[nodiscard]] inline bool rtsSettable(QSerialPort::FlowControl flowControl) noexcept
{
  return flowControl != QSerialPort::HardwareControl;
}

/**
 * @brief The one-line verdict of a failed Connection Sequence: the step by position (@p index is
 *        zero-based) and kind, the reason, and the bytes the device sent when an expect step is
 *        what failed, so a wrong reply is diagnosable from the console alone.
 */
[[nodiscard]] inline QString sequenceFailureText(const QString& kind,
                                                 int index,
                                                 const QString& reason,
                                                 const QString& received)
{
  const auto step = QString::number(index + 1);
  if (received.isEmpty())
    return QCoreApplication::translate("IO::Drivers::UART",
                                       "Connection Sequence failed at step %1 (%2): %3")
      .arg(step, kind, reason);

  return QCoreApplication::translate("IO::Drivers::UART",
                                     "Connection Sequence failed at step %1 (%2): %3; received: %4")
    .arg(step, kind, reason, received);
}

}  // namespace UartPolicy
}  // namespace Drivers
}  // namespace IO
