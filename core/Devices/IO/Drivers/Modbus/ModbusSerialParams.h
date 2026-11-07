/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QSerialPort>

namespace IO {
namespace Drivers {
namespace ModbusSerialParams {
[[nodiscard]] QSerialPort::Parity parityFromIndex(quint8 index) noexcept;
[[nodiscard]] QSerialPort::DataBits dataBitsFromIndex(quint8 index) noexcept;
[[nodiscard]] QSerialPort::StopBits stopBitsFromIndex(quint8 index) noexcept;
}  // namespace ModbusSerialParams
}  // namespace Drivers
}  // namespace IO
