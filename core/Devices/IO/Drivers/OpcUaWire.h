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

#include <cstdint>
#include <cstring>
#include <iterator>
#include <QByteArray>
#include <QByteArrayView>
#include <QDateTime>
#include <QString>
#include <QUuid>
#include <QVariant>

#include "Core/SSAssert.h"
#include "SerialStudio.h"

namespace IO {
namespace Drivers {

/**
 * @brief OPC UA delta-frame wire vocabulary (spec 0066), header-only so the driver encoder,
 *        the `opcua` native template and the ctest tier share ONE definition. A frame is
 *        [version u8] then entries [index u16 LE][type u8][payload]; strings carry a u16 LE
 *        byte length first. Only tags that changed since the previous tick are present and
 *        the decoder latches the rest.
 */
namespace OpcUaWire {

inline constexpr std::uint8_t kWireVersion = 1;
inline constexpr int kMaxTags              = 2048;
inline constexpr int kSoftTagLimit         = 512;
inline constexpr int kMaxStringBytes       = 256;
inline constexpr int kMaxFrameBytes        = 65536;
inline constexpr int kHeaderBytes          = 1;
inline constexpr int kEntryHeaderBytes     = 3;

/**
 * @brief Payload type tags; the numeric value is the wire byte.
 */
enum class Type : std::uint8_t {
  Bool    = 0,
  I8      = 1,
  U8      = 2,
  I16     = 3,
  U16     = 4,
  I32     = 5,
  U32     = 6,
  I64     = 7,
  U64     = 8,
  F32     = 9,
  F64     = 10,
  Str     = 11,
  Invalid = 255,
};

/**
 * @brief One decoded entry: the latch index, the payload type and its text rendering.
 */
struct Entry {
  int index = -1;
  Type type = Type::Invalid;
  QString text;
};

/**
 * @brief Maps the schema letter code ("bool", "i16", "f64", "str", ...) to a wire type.
 */
[[nodiscard]] inline Type typeFromCode(const QString& code) noexcept
{
  static constexpr const char* k_codes[] = {
    "bool",
    "i8",
    "u8",
    "i16",
    "u16",
    "i32",
    "u32",
    "i64",
    "u64",
    "f32",
    "f64",
    "str",
  };

  const auto latin = code.toLatin1();
  for (std::uint8_t i = 0; i < static_cast<std::uint8_t>(std::size(k_codes)); ++i)
    if (latin == k_codes[i])
      return static_cast<Type>(i);

  return Type::Invalid;
}

/**
 * @brief Returns the schema letter code for a wire type, or an empty string for Invalid.
 */
[[nodiscard]] inline QString codeFromType(Type type) noexcept
{
  static constexpr const char* k_codes[] = {
    "bool",
    "i8",
    "u8",
    "i16",
    "u16",
    "i32",
    "u32",
    "i64",
    "u64",
    "f32",
    "f64",
    "str",
  };

  const auto raw = static_cast<std::uint8_t>(type);
  SS_ASSERT_LOG(raw < std::size(k_codes) || type == Type::Invalid);
  if (raw >= std::size(k_codes))
    return {};

  return QString::fromLatin1(k_codes[raw]);
}

/**
 * @brief Fixed payload width for scalar types; -1 for strings (length-prefixed) or Invalid.
 */
[[nodiscard]] inline int payloadWidth(Type type) noexcept
{
  switch (type) {
    case Type::Bool:
    case Type::I8:
    case Type::U8:
      return 1;
    case Type::I16:
    case Type::U16:
      return 2;
    case Type::I32:
    case Type::U32:
    case Type::F32:
      return 4;
    case Type::I64:
    case Type::U64:
    case Type::F64:
      return 8;
    case Type::Str:
    case Type::Invalid:
      break;
  }

  return -1;
}

/**
 * @brief Worst-case encoded size of one entry, used to reserve the frame buffer once.
 */
[[nodiscard]] inline int maxEntryBytes(Type type) noexcept
{
  const int width = payloadWidth(type);
  if (width > 0)
    return kEntryHeaderBytes + width;

  return kEntryHeaderBytes + 2 + kMaxStringBytes;
}

/**
 * @brief Appends a little-endian integer of the given byte width.
 */
inline void appendLe(QByteArray& out, std::uint64_t value, int width)
{
  SS_ASSERT(width >= 1 && width <= 8, return);
  SS_ASSERT_LOG(width <= 8);

  for (int i = 0; i < width; ++i)
    out.append(static_cast<char>((value >> (8 * i)) & 0xFF));
}

/**
 * @brief Starts a frame: clears the buffer (keeping its capacity) and writes the version.
 */
inline void beginFrame(QByteArray& out)
{
  out.resize(0);
  out.append(static_cast<char>(kWireVersion));
  SS_ASSERT_LOG(out.size() == kHeaderBytes);
}

/**
 * @brief Renders any value as text for a string channel: GUIDs and byte strings get their
 *        canonical spellings, everything else its QVariant string. OPC UA types the driver
 *        unwraps at ingestion (LocalizedText) never reach here, so this header stays Qt Core-only
 *        and the ctest tier links no OPC UA module.
 */
[[nodiscard]] inline QString stringPayload(const QVariant& value)
{
  switch (value.typeId()) {
    case QMetaType::QUuid:
      return value.toUuid().toString(QUuid::WithoutBraces);
    case QMetaType::QByteArray:
      return QString::fromLatin1(value.toByteArray().toHex());
    case QMetaType::QDateTime:
      return value.toDateTime().toString(Qt::ISODateWithMs);
    default:
      break;
  }

  return value.toString();
}

/**
 * @brief True when the variant cannot be represented by the declared wire type without loss, so
 *        a mis-declared tag (API or CLI) is reported instead of silently truncating every tick.
 */
[[nodiscard]] inline bool valueFitsType(const QVariant& value, Type type) noexcept
{
  const auto id = value.typeId();
  if (type == Type::Str)
    return true;

  if (type == Type::Bool)
    return id == QMetaType::Bool || value.canConvert<bool>();

  if (type == Type::F32 || type == Type::F64)
    return value.canConvert<double>();

  if (id == QMetaType::Double || id == QMetaType::Float)
    return false;

  return value.canConvert<qlonglong>() || value.canConvert<qulonglong>();
}

/**
 * @brief Appends one entry. Strings are truncated to @ref kMaxStringBytes (UTF-8, no split
 *        repair); the caller reserves the worst-case size so steady-state appends stay in place.
 */
inline void appendEntry(QByteArray& out, int index, Type type, const QVariant& value)
{
  SS_ASSERT(index >= 0 && index < kMaxTags, return);
  SS_ASSERT(type != Type::Invalid, return);

  appendLe(out, static_cast<std::uint64_t>(index), 2);
  out.append(static_cast<char>(type));

  switch (type) {
    case Type::Bool:
      out.append(static_cast<char>(value.toBool() ? 1 : 0));
      break;
    case Type::I8:
    case Type::I16:
    case Type::I32:
    case Type::I64:
      appendLe(out, static_cast<std::uint64_t>(value.toLongLong()), payloadWidth(type));
      break;
    case Type::U8:
    case Type::U16:
    case Type::U32:
    case Type::U64:
      appendLe(out, value.toULongLong(), payloadWidth(type));
      break;
    case Type::F32: {
      const float f = static_cast<float>(SerialStudio::toDouble(value));
      std::uint32_t bits;
      std::memcpy(&bits, &f, sizeof(bits));
      appendLe(out, bits, 4);
      break;
    }
    case Type::F64: {
      const double d = SerialStudio::toDouble(value);
      std::uint64_t bits;
      std::memcpy(&bits, &d, sizeof(bits));
      appendLe(out, bits, 8);
      break;
    }
    case Type::Str: {
      QByteArray utf8 = stringPayload(value).toUtf8();
      if (utf8.size() > kMaxStringBytes) {
        int cut = kMaxStringBytes;
        while (cut > 0 && (static_cast<std::uint8_t>(utf8.at(cut)) & 0xC0) == 0x80)
          --cut;

        utf8.truncate(cut);
      }

      appendLe(out, static_cast<std::uint64_t>(utf8.size()), 2);
      out.append(utf8);
      break;
    }
    case Type::Invalid:
      break;
  }
}

/**
 * @brief Reads a little-endian unsigned integer; the caller guarantees the bytes exist.
 */
[[nodiscard]] inline std::uint64_t readLe(QByteArrayView view, qsizetype pos, int width) noexcept
{
  SS_ASSERT(width >= 1 && width <= 8, return 0);
  SS_ASSERT(pos >= 0 && pos + width <= view.size(), return 0);

  std::uint64_t value = 0;
  for (int i = 0; i < width; ++i)
    value |= static_cast<std::uint64_t>(static_cast<std::uint8_t>(view[pos + i])) << (8 * i);

  return value;
}

/**
 * @brief Validates the frame header; true when the version byte matches this vocabulary.
 */
[[nodiscard]] inline bool checkHeader(QByteArrayView view) noexcept
{
  SS_ASSERT_LOG(view.size() >= 0);
  if (view.size() < kHeaderBytes)
    return false;

  return static_cast<std::uint8_t>(view[0]) == kWireVersion;
}

/**
 * @brief Renders a scalar payload as the text the latch row stores.
 */
[[nodiscard]] inline QString scalarText(Type type, std::uint64_t raw) noexcept
{
  SS_ASSERT_LOG(type != Type::Str);
  SS_ASSERT_LOG(type != Type::Invalid);

  switch (type) {
    case Type::Bool:
      return raw ? QStringLiteral("1") : QStringLiteral("0");
    case Type::I8:
      return QString::number(static_cast<std::int8_t>(raw));
    case Type::I16:
      return QString::number(static_cast<std::int16_t>(raw));
    case Type::I32:
      return QString::number(static_cast<std::int32_t>(raw));
    case Type::I64:
      return QString::number(static_cast<qint64>(raw));
    case Type::U8:
    case Type::U16:
    case Type::U32:
    case Type::U64:
      return QString::number(static_cast<quint64>(raw));
    case Type::F32: {
      const auto bits = static_cast<std::uint32_t>(raw);
      float f;
      std::memcpy(&f, &bits, sizeof(f));
      return QString::number(f, 'g', 9);
    }
    case Type::F64: {
      double d;
      std::memcpy(&d, &raw, sizeof(d));
      return QString::number(d, 'g', 17);
    }
    case Type::Str:
    case Type::Invalid:
      break;
  }

  return {};
}

/**
 * @brief Decodes the entry at @p pos, advancing it past the payload. Returns false (and leaves
 *        @p pos untouched) on a truncated entry or an unknown type, which ends the walk.
 */
[[nodiscard]] inline bool readEntry(QByteArrayView view, qsizetype& pos, Entry& out) noexcept
{
  SS_ASSERT(pos >= 0, return false);
  if (pos + kEntryHeaderBytes > view.size())
    return false;

  const int index = static_cast<int>(readLe(view, pos, 2));
  const auto type = static_cast<Type>(static_cast<std::uint8_t>(view[pos + 2]));
  const int width = payloadWidth(type);
  if (index >= kMaxTags)
    return false;

  qsizetype cursor = pos + kEntryHeaderBytes;
  if (width > 0) {
    if (cursor + width > view.size())
      return false;

    out.text  = scalarText(type, readLe(view, cursor, width));
    cursor   += width;
  } else if (type == Type::Str) {
    if (cursor + 2 > view.size())
      return false;

    const auto length  = static_cast<qsizetype>(readLe(view, cursor, 2));
    cursor            += 2;
    if (length > kMaxStringBytes || cursor + length > view.size())
      return false;

    out.text  = QString::fromUtf8(view.data() + cursor, length);
    cursor   += length;
  } else {
    return false;
  }

  out.index = index;
  out.type  = type;
  pos       = cursor;
  return true;
}

}  // namespace OpcUaWire
}  // namespace Drivers
}  // namespace IO
