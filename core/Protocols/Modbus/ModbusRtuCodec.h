/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
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

#include <QByteArray>
#include <QtGlobal>

namespace IO {
namespace Drivers {

/**
 * @brief The two pieces of Modbus RTU framing the driver publishes with: the function code a
 *        register-type index reads with, and the CRC every real RTU frame ends in. Qt-Core-only and
 *        free of the driver, so the framing a consumer validates is testable without a device.
 */
namespace ModbusRtu {

[[nodiscard]] quint8 functionCodeForType(quint8 registerType) noexcept;
void appendCrc(QByteArray& frame);

}  // namespace ModbusRtu
}  // namespace Drivers
}  // namespace IO
