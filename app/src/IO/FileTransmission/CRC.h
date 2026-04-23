/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <array>
#include <cstdint>

namespace IO {
namespace Protocols {
namespace CRC {

static constexpr std::array<quint16, 256> kCrc16Table = [] {
  std::array<quint16, 256> table{};
  for (int i = 0; i < 256; ++i) {
    quint16 crc = static_cast<quint16>(i << 8);
    for (int j = 0; j < 8; ++j)
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;

    table[i] = crc;
  }

  return table;
}();

static constexpr std::array<quint32, 256> kCrc32Table = [] {
  std::array<quint32, 256> table{};
  for (quint32 i = 0; i < 256; ++i) {
    quint32 crc = i;
    for (int j = 0; j < 8; ++j)
      if (crc & 1)
        crc = (crc >> 1) ^ 0xEDB88320;
      else
        crc >>= 1;

    table[i] = crc;
  }

  return table;
}();

[[nodiscard]] inline quint16 crc16(const quint8* data, int length)
{
  quint16 crc = 0x0000;
  for (int i = 0; i < length; ++i)
    crc = (crc << 8) ^ kCrc16Table[((crc >> 8) ^ data[i]) & 0xFF];

  return crc;
}

[[nodiscard]] inline quint32 crc32(const quint8* data, int length)
{
  quint32 crc = 0xFFFFFFFF;
  for (int i = 0; i < length; ++i)
    crc = (crc >> 8) ^ kCrc32Table[(crc ^ data[i]) & 0xFF];

  return ~crc;
}

}  // namespace CRC
}  // namespace Protocols
}  // namespace IO
