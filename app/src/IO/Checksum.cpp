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

#include "IO/Checksum.h"

#include <cstdint>

//------------------------------------------------------------------------------
// Endianess detection
//------------------------------------------------------------------------------

#if defined(_MSC_VER)
#  include <stdlib.h>
#  define IS_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
#  define IS_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#else
#  include <endian.h>
#  define IS_LITTLE_ENDIAN (__BYTE_ORDER == __LITTLE_ENDIAN)
#endif

//------------------------------------------------------------------------------
// Endian-correct memory utilities for checksum formatting
//------------------------------------------------------------------------------

//
// These helpers write multibyte checksum values (e.g., CRCs, Adler, Fletcher)
// into output buffers with a defined byte order, regardless of the host CPU's
// native endianness.
//
// This ensures consistent output for binary formats, protocol compliance,
// and cross-platform validation. For example:
// - CRC-16-MODBUS requires little-endian output
// - Network and file formats typically expect big-endian
//
// Use `big_endian_memcpy()` and `little_endian_memcpy()` instead of raw
// memcpy() to ensure deterministic, spec-compliant results.
//

/**
 * @brief Copies data in big-endian format to a frame buffer.
 *
 * This function ensures that data is stored in big-endian byte order,
 * regardless of the host machine's native endianness. On little-endian
 * systems, it reverses the byte order before copying.
 *
 * @param frame A pointer to the destination buffer.
 * @param data A pointer to the source data to copy.
 * @param size The number of bytes to copy.
 *
 * @pre `frame` and `data` must not be null.
 * @pre `size` must be greater than 0.
 */
static void big_endian_memcpy(uint8_t *frame, const void *data, size_t size)
{
  assert(frame && data && size > 0);
  const uint8_t *raw_data = static_cast<const uint8_t *>(data);

#if IS_LITTLE_ENDIAN
  for (size_t i = 0; i < size; ++i)
    frame[i] = raw_data[size - 1 - i];
#else
  memcpy(frame, raw_data, size);
#endif
}

/**
 * @brief Copies data in little-endian format to a frame buffer.
 *
 * This function ensures that data is stored in little-endian byte order,
 * regardless of the host machine's native endianness. On big-endian
 * systems, it reverses the byte order before copying.
 *
 * @param frame A pointer to the destination buffer.
 * @param data A pointer to the source data to copy.
 * @param size The number of bytes to copy.
 *
 * @pre `frame` and `data` must not be null.
 * @pre `size` must be greater than 0.
 */
static void little_endian_memcpy(uint8_t *frame, const void *data, size_t size)
{
  assert(frame && data && size > 0);
  const uint8_t *raw_data = static_cast<const uint8_t *>(data);

#if IS_LITTLE_ENDIAN
  memcpy(frame, raw_data, size);
#else
  for (size_t i = 0; i < size; ++i)
    frame[i] = raw_data[size - 1 - i];
#endif
}

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
  static QStringList list;
  if (list.isEmpty())
  {
    const auto &map = checksumFunctionMap();
    for (auto it = map.begin(); it != map.end(); ++it)
      list.append(it.key());
  }

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
 *         functions.
 */
const QMap<QString, IO::ChecksumFunc> &IO::checksumFunctionMap()
{
  static const QMap<QString, IO::ChecksumFunc> map = {
      {QLatin1String(""), [](const char *, int) { return QByteArray(); }},

      // 8-bit checksums
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

      // 16-bit checksums
      {QStringLiteral("CRC-16"),
       [](const char *d, int l) {
         uint8_t out[2];
         uint16_t v = crc16(d, l);
         big_endian_memcpy(out, &v, sizeof(v));
         return QByteArray(reinterpret_cast<char *>(out), sizeof(out));
       }},
      {QStringLiteral("CRC-16-MODBUS"),
       [](const char *d, int l) {
         uint8_t out[2];
         uint16_t v = crc16_modbus(d, l);
         little_endian_memcpy(out, &v, sizeof(v));
         return QByteArray(reinterpret_cast<char *>(out), sizeof(out));
       }},
      {QStringLiteral("CRC-16-CCITT"),
       [](const char *d, int l) {
         uint8_t out[2];
         uint16_t v = crc16_ccitt(d, l);
         big_endian_memcpy(out, &v, sizeof(v));
         return QByteArray(reinterpret_cast<char *>(out), sizeof(out));
       }},
      {QStringLiteral("Fletcher-16"),
       [](const char *d, int l) {
         uint8_t out[2];
         uint16_t v = fletcher16(d, l);
         big_endian_memcpy(out, &v, sizeof(v));
         return QByteArray(reinterpret_cast<char *>(out), sizeof(out));
       }},

      // 32-bit checksums
      {QStringLiteral("CRC-32"),
       [](const char *d, int l) {
         uint8_t out[4];
         uint32_t v = crc32(d, l);
         big_endian_memcpy(out, &v, sizeof(v));
         return QByteArray(reinterpret_cast<char *>(out), sizeof(out));
       }},
      {QStringLiteral("Adler-32"),
       [](const char *d, int l) {
         uint8_t out[4];
         uint32_t v = adler32(d, l);
         big_endian_memcpy(out, &v, sizeof(v));
         return QByteArray(reinterpret_cast<char *>(out), sizeof(out));
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
