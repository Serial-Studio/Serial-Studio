/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
