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

#include <QSerialPort>

namespace IO {
namespace Drivers {

/**
 * @brief The two decisions the UART error handler makes, as pure functions: whether an error ends
 *        the link, and whether the driver's own auto-reconnect owns the recovery. They live apart
 *        from the driver so both can be pinned without a real port (spec 0075).
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

}  // namespace UartPolicy
}  // namespace Drivers
}  // namespace IO
