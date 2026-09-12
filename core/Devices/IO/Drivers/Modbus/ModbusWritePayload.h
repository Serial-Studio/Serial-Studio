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

#include <optional>
#include <QByteArray>
#include <QVector>

namespace IO {
namespace Drivers {

inline constexpr int kMinWriteUnitId = 1;  ///< Lowest addressable Modbus unit (0 is broadcast)
inline constexpr int kMaxWriteUnitId = 247;

// Unit-prefix marker (0xFF never occurs in UTF-8 text, 0x83 is an exception code): [FF 83 unit ...]
inline constexpr char kUnitPrefixMarker0 = static_cast<char>(0xFF);
inline constexpr char kUnitPrefixMarker1 = static_cast<char>(0x83);
inline constexpr int kUnitPrefixBytes    = 3;

/**
 * @brief One decoded holding-register write: the unit it targets, the start address and the
 *        16-bit words, decoded from the byte payload a script or the API hands the Modbus driver.
 */
struct ModbusWriteRequest {
  int unit;
  quint16 address;
  QVector<quint16> values;
};

/**
 * @brief The write payload contract (spec 0083): [addr_hi, addr_lo, words...] is aimed at
 *        @p defaultUnit; [0xFF, 0x83, unit, addr_hi, addr_lo, words...] names its unit (1..247).
 *        Any other odd-length payload is refused, as it always was. Returns nothing for a
 *        malformed payload, a unit outside the range, or more than @p maxRegisters words.
 */
[[nodiscard]] std::optional<ModbusWriteRequest> decodeModbusWritePayload(const QByteArray& data,
                                                                         int defaultUnit,
                                                                         int maxRegisters);

}  // namespace Drivers
}  // namespace IO
