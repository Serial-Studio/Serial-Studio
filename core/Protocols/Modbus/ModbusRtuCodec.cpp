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

#include "Protocols/Modbus/ModbusRtuCodec.h"

#include "Core/SSAssert.h"

static constexpr quint16 kCrcSeed       = 0xFFFF;
static constexpr quint16 kCrcPolynomial = 0xA001;

/**
 * @brief Maps a register-type index to the Modbus function code that reads it. An index this build
 *        does not know reads holding registers, which is the fallback the poll loop takes too. The
 *        index arrives from a project file or the API, so an unknown one is mapped, never asserted.
 */
quint8 IO::Drivers::ModbusRtu::functionCodeForType(const quint8 registerType) noexcept
{
  if (registerType == 1)
    return 0x04;

  if (registerType == 2)
    return 0x01;

  if (registerType == 3)
    return 0x02;

  return 0x03;
}

/**
 * @brief Appends the CRC-16/Modbus checksum of @p frame, low octet first as the wire carries it.
 *        Without it the published bytes are a header-shaped fragment rather than an RTU frame, and
 *        a consumer that validates the checksum rejects every poll.
 */
void IO::Drivers::ModbusRtu::appendCrc(QByteArray& frame)
{
  SS_ASSERT(!frame.isEmpty(), return);

  quint16 crc = kCrcSeed;
  for (const char octet : frame) {
    crc ^= static_cast<quint8>(octet);
    for (int bit = 0; bit < 8; ++bit)
      crc = (crc & 1U) ? static_cast<quint16>((crc >> 1) ^ kCrcPolynomial)
                       : static_cast<quint16>(crc >> 1);
  }

  frame.append(static_cast<char>(crc & 0xFF));
  frame.append(static_cast<char>((crc >> 8) & 0xFF));
}
