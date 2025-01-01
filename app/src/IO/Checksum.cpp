/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

#include "IO/Checksum.h"

/**
 * @brief Computes an 8-bit CRC (Cyclic Redundancy Check) for the given data.
 *
 * This function calculates the CRC-8 checksum using the polynomial 0x31.
 *
 * @param data Pointer to the input data array.
 * @param length Length of the input data array.
 * @return The computed 8-bit CRC checksum.
 */
uint8_t IO::crc8(const char *data, const int length)
{
  uint8_t crc = 0xff;
  for (int i = 0; i < length; i++)
  {
    crc ^= data[i];
    for (int j = 0; j < 8; j++)
    {
      if ((crc & 0x80) != 0)
        crc = (uint8_t)((crc << 1) ^ 0x31);
      else
        crc <<= 1;
    }
  }

  return crc;
}

/**
 * @brief Computes a 16-bit CRC (Cyclic Redundancy Check) for the given data.
 *
 * This function calculates the CRC-16 checksum using a bitwise algorithm.
 *
 * @param data Pointer to the input data array.
 * @param length Length of the input data array.
 * @return The computed 16-bit CRC checksum.
 */
uint16_t IO::crc16(const char *data, const int length)
{
  uint8_t x;
  uint16_t crc = 0xFFFF;

  for (int i = 0; i < length; ++i)
  {
    x = crc >> 8 ^ data[i];
    x ^= x >> 4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5))
          ^ ((uint16_t)x);
  }

  return crc;
}

/**
 * @brief Computes a 32-bit CRC (Cyclic Redundancy Check) for the given data.
 *
 * This function calculates the CRC-32 checksum using the polynomial 0xEDB88320.
 *
 * @param data Pointer to the input data array.
 * @param length Length of the input data array.
 * @return The computed 32-bit CRC checksum.
 */
uint32_t IO::crc32(const char *data, const int length)
{
  uint32_t mask;
  uint32_t crc = 0xFFFFFFFF;
  for (int i = 0; i < length; ++i)
  {
    crc = crc ^ data[i];
    for (int j = 8; j >= 0; j--)
    {
      mask = -(crc & 1);
      crc = (crc >> 1) ^ (0xEDB88320 & mask);
    }
  }

  return ~crc;
}
