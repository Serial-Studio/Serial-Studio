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
#include <QDebug>

//--------------------------------------------------------------------------------------------------
// Endianess detection
//--------------------------------------------------------------------------------------------------

#if defined(_MSC_VER)
#  include <stdlib.h>
#  define IS_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
#  define IS_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#else
#  include <endian.h>
#  define IS_LITTLE_ENDIAN (__BYTE_ORDER == __LITTLE_ENDIAN)
#endif

//--------------------------------------------------------------------------------------------------
// Endian-correct memory utilities for checksum formatting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Copies data in big-endian format to a frame buffer.
 */
static void big_endian_memcpy(uint8_t* frame, const void* data, size_t size)
{
  if (!frame || !data || size == 0) {
    qWarning() << "big_endian_memcpy: invalid arguments";
    return;
  }

  const uint8_t* raw_data = static_cast<const uint8_t*>(data);

#if IS_LITTLE_ENDIAN
  for (size_t i = 0; i < size; ++i)
    frame[i] = raw_data[size - 1 - i];
#else
  memcpy(frame, raw_data, size);
#endif
}

/**
 * @brief Copies data in little-endian format to a frame buffer.
 */
static void little_endian_memcpy(uint8_t* frame, const void* data, size_t size)
{
  if (!frame || !data || size == 0) {
    qWarning() << "little_endian_memcpy: invalid arguments";
    return;
  }

  const uint8_t* raw_data = static_cast<const uint8_t*>(data);

#if IS_LITTLE_ENDIAN
  memcpy(frame, raw_data, size);
#else
  for (size_t i = 0; i < size; ++i)
    frame[i] = raw_data[size - 1 - i];
#endif
}

//--------------------------------------------------------------------------------------------------
// Checksum implementations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes an 8-bit XOR checksum.
 */
static constexpr uint8_t xor8(const char* data, const int length) noexcept
{
  uint8_t checksum = 0;
  for (int i = 0; i < length; ++i)
    checksum ^= static_cast<uint8_t>(data[i]);

  return checksum;
}

/**
 * @brief Computes an 8-bit CRC (CRC-8) using polynomial 0x31.
 */
static constexpr uint8_t crc8(const char* data, const int length) noexcept
{
  uint8_t crc = 0xFF;
  for (int i = 0; i < length; ++i) {
    crc ^= static_cast<uint8_t>(data[i]);
    for (int j = 0; j < 8; ++j)
      if (crc & 0x80)
        crc = (crc << 1) ^ 0x31;
      else
        crc <<= 1;
  }
  return crc;
}

/**
 * @brief Computes an additive modulo-256 checksum.
 */
static constexpr uint8_t mod256(const char* data, const int length) noexcept
{
  uint16_t sum = 0;
  for (int i = 0; i < length; ++i)
    sum += static_cast<uint8_t>(data[i]);
  return static_cast<uint8_t>(sum & 0xFF);
}

/**
 * @brief Computes a 16-bit CRC using a custom bitwise algorithm.
 */
static constexpr uint16_t crc16(const char* data, const int length) noexcept
{
  uint8_t x;
  uint16_t crc = 0xFFFF;

  for (int i = 0; i < length; ++i) {
    x = crc >> 8 ^ static_cast<uint8_t>(data[i]);
    x ^= x >> 4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^ x;
  }

  return crc;
}

/**
 * @brief Computes a 32-bit CRC (CRC-32) using polynomial 0xEDB88320.
 */
static constexpr uint32_t crc32(const char* data, const int length) noexcept
{
  uint32_t crc = 0xFFFFFFFF;

  for (int i = 0; i < length; ++i) {
    crc ^= static_cast<uint8_t>(data[i]);
    for (int j = 0; j < 8; ++j)
      if (crc & 1)
        crc = (crc >> 1) ^ 0xEDB88320;
      else
        crc >>= 1;
  }

  return ~crc;
}

/**
 * @brief Computes the Adler-32 checksum of the input data.
 */
static constexpr uint32_t adler32(const char* data, const int length) noexcept
{
  constexpr uint32_t kModAdler = 65521;
  uint32_t a = 1, b = 0;

  for (int i = 0; i < length; ++i) {
    a = (a + static_cast<uint8_t>(data[i])) % kModAdler;
    b = (b + a) % kModAdler;
  }

  return (b << 16) | a;
}

/**
 * @brief Computes a Fletcher-16 checksum.
 */
static constexpr uint16_t fletcher16(const char* data, const int length) noexcept
{
  uint16_t sum1 = 0;
  uint16_t sum2 = 0;

  for (int i = 0; i < length; ++i) {
    sum1 = (sum1 + static_cast<uint8_t>(data[i])) % 255;
    sum2 = (sum2 + sum1) % 255;
  }

  return (sum2 << 8) | sum1;
}

/**
 * @brief Computes a 16-bit CRC (CRC-16-MODBUS) using polynomial 0x8005 (reflected: 0xA001).
 */
static constexpr uint16_t crc16_modbus(const char* data, const int length) noexcept
{
  uint16_t crc = 0xFFFF;

  for (int i = 0; i < length; ++i) {
    crc ^= static_cast<uint8_t>(data[i]);
    for (int j = 0; j < 8; ++j)
      if (crc & 0x0001)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc >>= 1;
  }

  return crc;
}

/**
 * @brief Computes a 16-bit CRC (CRC-CCITT) using polynomial 0x1021 and initial value 0x0000.
 */
static constexpr uint16_t crc16_ccitt(const char* data, const int length) noexcept
{
  uint16_t crc = 0x0000;

  for (int i = 0; i < length; ++i) {
    crc ^= static_cast<uint8_t>(data[i]) << 8;
    for (int j = 0; j < 8; ++j)
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
  }

  return crc;
}

//--------------------------------------------------------------------------------------------------
// Checksum byte-packing helpers
//--------------------------------------------------------------------------------------------------

static QByteArray packByte(uint8_t v)
{
  return QByteArray(reinterpret_cast<const char*>(&v), sizeof(v));
}

static QByteArray packU16BE(uint16_t v)
{
  uint8_t out[2];
  big_endian_memcpy(out, &v, sizeof(v));
  return QByteArray(reinterpret_cast<char*>(out), sizeof(out));
}

static QByteArray packU16LE(uint16_t v)
{
  uint8_t out[2];
  little_endian_memcpy(out, &v, sizeof(v));
  return QByteArray(reinterpret_cast<char*>(out), sizeof(out));
}

static QByteArray packU32BE(uint32_t v)
{
  uint8_t out[4];
  big_endian_memcpy(out, &v, sizeof(v));
  return QByteArray(reinterpret_cast<char*>(out), sizeof(out));
}

//--------------------------------------------------------------------------------------------------
// Utility functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a list of supported checksum and CRC algorithm names.
 */
const QStringList& IO::availableChecksums()
{
  static QStringList list;
  if (list.isEmpty()) {
    const auto& map = checksumFunctionMap();
    for (auto it = map.begin(); it != map.end(); ++it)
      list.append(it.key());
  }

  return list;
}

/**
 * @brief Returns a static map of supported checksum algorithm names to their implementations.
 */
const QMap<QString, IO::ChecksumFunc>& IO::checksumFunctionMap()
{
  // clang-format off
  static const QMap<QString, IO::ChecksumFunc> map = {
    {QLatin1String(""),            [](const char*, int) { return QByteArray(); }},

    {QStringLiteral("XOR-8"),    [](const char* d, int l) { return packByte(xor8(d, l)); }},
    {QStringLiteral("MOD-256"),  [](const char* d, int l) { return packByte(mod256(d, l)); }},
    {QStringLiteral("CRC-8"),    [](const char* d, int l) { return packByte(crc8(d, l)); }},

    {QStringLiteral("CRC-16"),        [](const char* d, int l) { return packU16BE(crc16(d, l)); }},
    {QStringLiteral("CRC-16-MODBUS"), [](const char* d, int l) { return packU16LE(crc16_modbus(d, l)); }},
    {QStringLiteral("CRC-16-CCITT"),  [](const char* d, int l) { return packU16BE(crc16_ccitt(d, l)); }},
    {QStringLiteral("Fletcher-16"),   [](const char* d, int l) { return packU16BE(fletcher16(d, l)); }},

    {QStringLiteral("CRC-32"),   [](const char* d, int l) { return packU32BE(crc32(d, l)); }},
    {QStringLiteral("Adler-32"), [](const char* d, int l) { return packU32BE(adler32(d, l)); }},
  };
  // clang-format on

  return map;
}

/**
 * @brief Computes a checksum or CRC value for the given data using the specified algorithm.
 */
QByteArray IO::checksum(const QString& name, const QByteArray& data)
{
  const auto& map = checksumFunctionMap();
  const auto it   = map.find(name);
  if (it != map.end())
    return it.value()(data.constData(), data.size());

  return {};
}
