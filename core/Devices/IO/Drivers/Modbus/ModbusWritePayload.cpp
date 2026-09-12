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

#include "IO/Drivers/Modbus/ModbusWritePayload.h"

#include "Core/SSAssert.h"

/**
 * @brief Decodes a write payload; see the header for the two accepted shapes.
 */
std::optional<IO::Drivers::ModbusWriteRequest> IO::Drivers::decodeModbusWritePayload(
  const QByteArray& data, int defaultUnit, int maxRegisters)
{
  SS_ASSERT(maxRegisters > 0, return std::nullopt);

  if (data.length() < 4)
    return std::nullopt;

  const bool has_unit = data.length() > kUnitPrefixBytes && data[0] == kUnitPrefixMarker0
                     && data[1] == kUnitPrefixMarker1;
  const int base      = has_unit ? kUnitPrefixBytes : 0;
  const int unit      = has_unit ? static_cast<quint8>(data[2]) : defaultUnit;
  if ((data.length() - base) % 2 != 0 || data.length() - base < 4)
    return std::nullopt;

  if (unit < kMinWriteUnitId || unit > kMaxWriteUnitId)
    return std::nullopt;

  const int register_count = (data.length() - base - 2) / 2;
  if (register_count > maxRegisters)
    return std::nullopt;

  ModbusWriteRequest request;
  request.unit    = unit;
  request.address = static_cast<quint16>((static_cast<quint8>(data[base]) << 8)
                                         | static_cast<quint8>(data[base + 1]));
  request.values.reserve(register_count);
  for (int i = 0; i < register_count; ++i) {
    const int at = base + 2 + 2 * i;
    request.values.append(static_cast<quint16>((static_cast<quint8>(data[at]) << 8)
                                               | static_cast<quint8>(data[at + 1])));
  }

  SS_ASSERT_LOG(request.values.size() == register_count);
  return request;
}
