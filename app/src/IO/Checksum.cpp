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
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "IO/Checksum.h"

#include <cstdint>

//------------------------------------------------------------------------------
// Checksum implementations
//------------------------------------------------------------------------------

namespace IO
{
/**
 * @brief Computes an 8-bit XOR checksum.
 *
 * XORs all bytes in the input data. Useful for simple integrity checks in
 * lightweight protocols.
 *
 * @param data Pointer to the input data array.
 * @param length Length of the input data array.
 * @return The computed 8-bit XOR checksum.
 */
static uint8_t xor8(const char *data, const int length)
{
  uint8_t checksum = 0;
  for (int i = 0; i < length; ++i)
    checksum ^= static_cast<uint8_t>(data[i]);

  return checksum;
}

/**
 * @brief Computes an 8-bit CRC (CRC-8) using polynomial 0x31.
 *
 * Common in SMBus and lightweight communication protocols.
 *
 * @param data Pointer to the input data array.
 * @param length Length of the input data array.
 * @return The computed 8-bit CRC checksum.
 */
static uint8_t crc8(const char *data, const int length)
{
  uint8_t crc = 0xFF;
  for (int i = 0; i < length; ++i)
  {
    crc ^= static_cast<uint8_t>(data[i]);
    for (int j = 0; j < 8; ++j)
    {
      if (crc & 0x80)
        crc = (crc << 1) ^ 0x31;
      else
        crc <<= 1;
    }
  }
  return crc;
}

/**
 * @brief Computes an additive modulo-256 checksum.
 *
 * Sums all bytes in the input and returns the result modulo 256.
 *
 * @param data Pointer to the input data array.
 * @param length Length of the input data array.
 * @return The computed 8-bit additive checksum.
 */
static uint8_t mod256(const char *data, const int length)
{
  uint16_t sum = 0;
  for (int i = 0; i < length; ++i)
    sum += static_cast<uint8_t>(data[i]);
  return static_cast<uint8_t>(sum & 0xFF);
}

/**
 * @brief Computes a 16-bit CRC using a custom bitwise algorithm.
 *
 * This is not a standard CRC-16 variant. Rename if needed for clarity.
 *
 * @param data Pointer to the input data array.
 * @param length Length of the input data array.
 * @return The computed 16-bit CRC checksum.
 */
static uint16_t crc16(const char *data, const int length)
{
  uint8_t x;
  uint16_t crc = 0xFFFF;

  for (int i = 0; i < length; ++i)
  {
    x = crc >> 8 ^ static_cast<uint8_t>(data[i]);
    x ^= x >> 4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^ x;
  }

  return crc;
}

/**
 * @brief Computes a 32-bit CRC (CRC-32) using polynomial 0xEDB88320.
 *
 * Common in Ethernet, ZIP, PNG, and other file/integrity systems.
 *
 * @param data Pointer to the input data array.
 * @param length Length of the input data array.
 * @return The computed 32-bit CRC checksum.
 */
static uint32_t crc32(const char *data, const int length)
{
  uint32_t crc = 0xFFFFFFFF;

  for (int i = 0; i < length; ++i)
  {
    crc ^= static_cast<uint8_t>(data[i]);
    for (int j = 0; j < 8; ++j)
    {
      if (crc & 1)
        crc = (crc >> 1) ^ 0xEDB88320;
      else
        crc >>= 1;
    }
  }

  return ~crc;
}

/**
 * @brief Computes the Adler-32 checksum of the input data.
 *
 * Fast checksum algorithm defined in RFC 1950 (used in zlib).
 *
 * @param data Pointer to the input data array.
 * @param length Length of the input data array.
 * @return The computed 32-bit Adler-32 checksum.
 */
static uint32_t adler32(const char *data, const int length)
{
  const uint32_t MOD_ADLER = 65521;
  uint32_t a = 1, b = 0;

  for (int i = 0; i < length; ++i)
  {
    a = (a + static_cast<uint8_t>(data[i])) % MOD_ADLER;
    b = (b + a) % MOD_ADLER;
  }

  return (b << 16) | a;
}

/**
 * @brief Computes a Fletcher-16 checksum.
 *
 * Offers stronger error detection than simple checksums, with low overhead.
 *
 * @param data Pointer to the input data array.
 * @param length Length of the input data array.
 * @return The computed Fletcher-16 checksum.
 */
static uint16_t fletcher16(const char *data, const int length)
{
  uint16_t sum1 = 0;
  uint16_t sum2 = 0;

  for (int i = 0; i < length; ++i)
  {
    sum1 = (sum1 + static_cast<uint8_t>(data[i])) % 255;
    sum2 = (sum2 + sum1) % 255;
  }

  return (sum2 << 8) | sum1;
}

/**
 * @brief Computes a 16-bit CRC (CRC-16-MODBUS) using polynomial 0x8005
 * (reflected: 0xA001).
 *
 * Standard in industrial MODBUS RTU communications.
 *
 * @param data Pointer to the input data array.
 * @param length Length of the input data array.
 * @return The computed 16-bit CRC-MODBUS checksum.
 */
static uint16_t crc16_modbus(const char *data, const int length)
{
  uint16_t crc = 0xFFFF;

  for (int i = 0; i < length; ++i)
  {
    crc ^= static_cast<uint8_t>(data[i]);
    for (int j = 0; j < 8; ++j)
    {
      if (crc & 0x0001)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc >>= 1;
    }
  }

  return crc;
}

/**
 * @brief Computes a 16-bit CRC (CRC-CCITT) using polynomial 0x1021 and initial
 * value 0x0000.
 *
 * Used in XModem, HDLC, and general-purpose data integrity.
 *
 * @param data Pointer to the input data array.
 * @param length Length of the input data array.
 * @return The computed CRC-16-CCITT checksum.
 */
static uint16_t crc16_ccitt(const char *data, const int length)
{
  uint16_t crc = 0x0000;

  for (int i = 0; i < length; ++i)
  {
    crc ^= static_cast<uint8_t>(data[i]) << 8;
    for (int j = 0; j < 8; ++j)
    {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }

  return crc;
}
} // namespace IO

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------

/**
 * @brief Returns a list of supported checksum and CRC algorithm names.
 *
 * The list is derived directly from the internal checksum function map.
 * It reflects all available algorithms that can be passed to IO::checksum().
 *
 * @return A reference to a static QStringList containing supported algorithm
 * names.
 */
const QStringList &IO::availableChecksums()
{
  static const QStringList list = [] {
    auto keys = checksumFunctionMap().keys();
    keys.sort(Qt::CaseInsensitive);
    return keys;
  }();

  return list;
}

/**
 * @brief Returns a static map of supported checksum algorithm names to their
 *        implementations.
 *
 * The map keys are canonical algorithm names (e.g., "CRC-16", "Adler-32") and
 * the values are function objects that compute the corresponding checksum over
 * raw input data. Each function returns the checksum as a QByteArray containing
 * the raw bytes (1, 2, or 4 bytes depending on the algorithm).
 *
 * This map is initialized once and reused for all checksum dispatch operations.
 *
 * @return A reference to the static QMap of checksum algorithm names to
 * functions.
 */
const QMap<QString, IO::ChecksumFunc> &IO::checksumFunctionMap()
{
  static const QMap<QString, ChecksumFunc> map = {
      {QLatin1String(""), [](const char *, int) { return QByteArray(); }},
      {QStringLiteral("XOR-8"),
       [](const char *d, int l) {
         uint8_t v = xor8(d, l);
         return QByteArray(reinterpret_cast<const char *>(&v), sizeof(v));
       }},
      {QStringLiteral("MOD-256"),
       [](const char *d, int l) {
         uint8_t v = mod256(d, l);
         return QByteArray(reinterpret_cast<const char *>(&v), sizeof(v));
       }},
      {QStringLiteral("CRC-8"),
       [](const char *d, int l) {
         uint8_t v = crc8(d, l);
         return QByteArray(reinterpret_cast<const char *>(&v), sizeof(v));
       }},
      {QStringLiteral("CRC-16"),
       [](const char *d, int l) {
         uint16_t v = crc16(d, l);
         return QByteArray(reinterpret_cast<const char *>(&v), sizeof(v));
       }},
      {QStringLiteral("CRC-16-MODBUS"),
       [](const char *d, int l) {
         uint16_t v = crc16_modbus(d, l);
         return QByteArray(reinterpret_cast<const char *>(&v), sizeof(v));
       }},
      {QStringLiteral("CRC-16-CCITT"),
       [](const char *d, int l) {
         uint16_t v = crc16_ccitt(d, l);
         return QByteArray(reinterpret_cast<const char *>(&v), sizeof(v));
       }},
      {QStringLiteral("CRC-32"),
       [](const char *d, int l) {
         uint32_t v = crc32(d, l);
         return QByteArray(reinterpret_cast<const char *>(&v), sizeof(v));
       }},
      {QStringLiteral("Adler-32"),
       [](const char *d, int l) {
         uint32_t v = adler32(d, l);
         return QByteArray(reinterpret_cast<const char *>(&v), sizeof(v));
       }},
      {QStringLiteral("Fletcher-16"),
       [](const char *d, int l) {
         uint16_t v = fletcher16(d, l);
         return QByteArray(reinterpret_cast<const char *>(&v), sizeof(v));
       }},
  };

  return map;
}

/**
 * @brief Computes a checksum or CRC value for the given data using the
 * specified algorithm.
 *
 * This function supports multiple standard algorithms including CRC-8,
 * CRC-16 (various variants), CRC-32, XOR-8, MOD-256, Adler-32, and Fletcher-16.
 *
 * The algorithm name must match exactly one of the entries returned by
 * IO::availableChecksums(). The comparison is case-sensitive.
 *
 * @param name Name of the checksum or CRC algorithm.
 * @param data Input data buffer to be checksummed.
 *
 * @return Raw checksum result as a QByteArray, returns an empty QByteArray if
 *         the algorithm is unknown.
 */
QByteArray IO::checksum(const QString &name, const QByteArray &data)
{
  const auto &map = checksumFunctionMap();
  const auto it = map.find(name);
  if (it != map.end())
    return it.value()(data.constData(), data.size());

  return {};
}
