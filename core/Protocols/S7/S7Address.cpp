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

#include "Protocols/S7/S7Address.h"

#include <iterator>
#include <QCoreApplication>
#include <QRegularExpression>

#include "Core/SSAssert.h"

using IO::Drivers::S7Address::Address;
using IO::Drivers::S7Address::Area;
using IO::Drivers::S7Address::Type;

//--------------------------------------------------------------------------------------------------
// Grammar
//--------------------------------------------------------------------------------------------------
//
// Two absolute forms are accepted, both case-insensitive and whitespace-tolerant:
//
//   DB<db>.DB{X|B|W|D}<byte>[.<bit>]     data-block access ("DB5.DBD20", "DB1.DBX0.3")
//   {I|E|Q|A|M}{X|B|W|D}<byte>[.<bit>]   process image and flags ("MW10", "IB0", "Q0.1")
//
// An optional ":<TYPE>" suffix overrides how the bytes are rendered ("DB5.DBD20:REAL",
// "DB2.DBB0:STRING[32]"); without it the width letter decides, which is what the S7 world writes.
//
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the translated error string for the S7 addressing context.
 */
[[nodiscard]] static QString trAddress(const char* text)
{
  return QCoreApplication::translate("S7Address", text);
}

/**
 * @brief Maps the leading area letter onto its S7 area code; E/A are the German spellings of I/Q.
 */
[[nodiscard]] static Area areaFromLetter(QChar letter) noexcept
{
  SS_ASSERT_LOG(!letter.isNull());

  const auto upper = letter.toUpper().toLatin1();
  if (upper == 'I' || upper == 'E')
    return Area::Input;

  if (upper == 'Q' || upper == 'A')
    return Area::Output;

  if (upper == 'M')
    return Area::Memory;

  SS_ASSERT_LOG(upper != '\0');
  return Area::Invalid;
}

/**
 * @brief Maps the width letter (X, B, W, D) onto the type it implies when no suffix is given.
 */
[[nodiscard]] static Type typeFromWidthLetter(QChar letter) noexcept
{
  SS_ASSERT_LOG(letter.isNull() || letter.isLetter());

  const auto upper = letter.toUpper().toLatin1();
  switch (upper) {
    case 'X':
      return Type::Bool;
    case 'B':
      return Type::Byte;
    case 'W':
      return Type::Word;
    case 'D':
      return Type::DWord;
    default:
      break;
  }

  SS_ASSERT_LOG(upper == '\0');
  return Type::Invalid;
}

/**
 * @brief True when an explicit type suffix can be read out of the given width letter: a REAL is
 *        four bytes and cannot live behind a W, and mismatches are rejected rather than truncated.
 */
[[nodiscard]] static bool suffixFitsWidth(Type suffix, Type width) noexcept
{
  if (suffix == Type::Bool || width == Type::Bool)
    return suffix == Type::Bool && width == Type::Bool;

  if (suffix == Type::Str)
    return width == Type::Byte;

  return IO::Drivers::S7Address::typeWidth(suffix) == IO::Drivers::S7Address::typeWidth(width);
}

//--------------------------------------------------------------------------------------------------
// Vocabulary helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the byte width of a scalar type; strings are variable and report 0.
 */
int IO::Drivers::S7Address::typeWidth(Type type) noexcept
{
  switch (type) {
    case Type::Bool:
    case Type::Byte:
      return 1;
    case Type::Word:
    case Type::Int:
      return 2;
    case Type::DWord:
    case Type::DInt:
    case Type::Real:
      return 4;
    case Type::Str:
    case Type::Invalid:
      break;
  }

  SS_ASSERT_LOG(type == Type::Str || type == Type::Invalid);
  return 0;
}

/**
 * @brief Maps a type name ("REAL", "dint", "string") onto its enumerator.
 */
Type IO::Drivers::S7Address::typeFromCode(const QString& code) noexcept
{
  static constexpr const char* k_codes[] = {
    "BOOL",
    "BYTE",
    "WORD",
    "DWORD",
    "INT",
    "DINT",
    "REAL",
    "STRING",
  };

  const auto upper = code.trimmed().toUpper().toLatin1();
  for (std::uint8_t i = 0; i < static_cast<std::uint8_t>(std::size(k_codes)); ++i)
    if (upper == k_codes[i])
      return static_cast<Type>(i);

  SS_ASSERT_LOG(code.size() >= 0);
  return Type::Invalid;
}

/**
 * @brief Returns the canonical type name, or an empty string for Invalid.
 */
QString IO::Drivers::S7Address::codeFromType(Type type) noexcept
{
  static constexpr const char* k_codes[] = {
    "BOOL",
    "BYTE",
    "WORD",
    "DWORD",
    "INT",
    "DINT",
    "REAL",
    "STRING",
  };

  const auto raw = static_cast<std::uint8_t>(type);
  SS_ASSERT_LOG(raw < std::size(k_codes) || type == Type::Invalid);
  if (raw >= std::size(k_codes))
    return {};

  return QString::fromLatin1(k_codes[raw]);
}

/**
 * @brief Returns the single-letter area code used by the canonical spelling.
 */
QString IO::Drivers::S7Address::areaCode(Area area) noexcept
{
  switch (area) {
    case Area::Input:
      return QStringLiteral("I");
    case Area::Output:
      return QStringLiteral("Q");
    case Area::Memory:
      return QStringLiteral("M");
    case Area::DataBlk:
      return QStringLiteral("DB");
    case Area::Invalid:
      break;
  }

  SS_ASSERT_LOG(area == Area::Invalid);
  return {};
}

/**
 * @brief True when the address names a readable location of a known type and a non-negative size.
 */
bool IO::Drivers::S7Address::isValid(const Address& address) noexcept
{
  if (address.area == Area::Invalid || address.type == Type::Invalid)
    return false;

  if (address.byteOffset < 0 || address.byteOffset > kMaxByteOffset || address.size <= 0)
    return false;

  SS_ASSERT_LOG(address.dbNumber >= 0);
  return address.area != Area::DataBlk || address.dbNumber > 0;
}

/**
 * @brief Renders an address back into its canonical spelling, for status text and project titles.
 */
QString IO::Drivers::S7Address::normalize(const Address& address)
{
  SS_ASSERT(isValid(address), return {});

  const int width    = typeWidth(address.type);
  const char* letter = address.type == Type::Bool ? "X"
                     : address.type == Type::Str  ? "B"
                     : width == 2                 ? "W"
                     : width == 4                 ? "D"
                                                  : "B";

  const auto offset = QString::number(address.byteOffset);

  QString out;
  if (address.area == Area::DataBlk)
    out = QStringLiteral("DB%1.DB%2%3")
            .arg(QString::number(address.dbNumber), QLatin1String(letter), offset);
  else
    out = QStringLiteral("%1%2%3").arg(areaCode(address.area), QLatin1String(letter), offset);

  if (address.bitOffset >= 0)
    out += QStringLiteral(".%1").arg(address.bitOffset);

  SS_ASSERT_LOG(!out.isEmpty());
  return out;
}

//--------------------------------------------------------------------------------------------------
// Parser
//--------------------------------------------------------------------------------------------------

/**
 * @brief Splits an optional ":TYPE[len]" suffix off the address body, resolving the declared type
 *        and the string length. Returns false with a reason when the suffix names nothing known.
 */
[[nodiscard]] static bool splitSuffix(
  const QString& text, QString& body, Type& suffix, int& stringLength, QString& error)
{
  SS_ASSERT_LOG(stringLength >= 0);

  const int colon = text.indexOf(QLatin1Char(':'));
  body            = colon < 0 ? text : text.left(colon).trimmed();
  if (colon < 0)
    return true;

  static const QRegularExpression k_suffix(
    QStringLiteral("^([A-Za-z]+)(?:\\[\\s*(\\d+)\\s*\\])?$"));
  const auto match = k_suffix.match(text.mid(colon + 1).trimmed());
  if (!match.hasMatch()) {
    error = trAddress("The type suffix must read \":TYPE\" or \":STRING[length]\".");
    return false;
  }

  suffix = IO::Drivers::S7Address::typeFromCode(match.captured(1));
  if (suffix == Type::Invalid) {
    error = trAddress("Unknown type \"%1\".").arg(match.captured(1));
    return false;
  }

  stringLength = match.captured(2).isEmpty() ? 0 : match.captured(2).toInt();
  SS_ASSERT_LOG(stringLength >= 0);
  return true;
}

/**
 * @brief Fills the area, block number, width letter, byte and bit fields from the address body.
 *        Both accepted forms funnel through one expression so a spelling that matches neither is
 *        rejected here rather than half-parsed downstream.
 */
[[nodiscard]] static bool matchBody(const QString& body, Address& out, QChar& width, QString& error)
{
  static const QRegularExpression k_body(
    QStringLiteral("^(?:DB(\\d+)\\.DB([XBWD])|([IEQAM])([XBWD])?)(\\d+)(?:\\.(\\d+))?$"),
    QRegularExpression::CaseInsensitiveOption);

  const auto match = k_body.match(body);
  if (!match.hasMatch()) {
    error =
      trAddress("\"%1\" is not an S7 address (expected DB5.DBD20, MW10, IB0, Q0.1).").arg(body);
    return false;
  }

  const bool isBlock = !match.captured(1).isEmpty();
  out.area           = isBlock ? Area::DataBlk : areaFromLetter(match.captured(3).at(0));
  out.dbNumber       = isBlock ? match.captured(1).toInt() : 0;
  out.byteOffset     = match.captured(5).toInt();
  out.bitOffset      = match.captured(6).isEmpty() ? -1 : match.captured(6).toInt();

  const auto letters = isBlock ? match.captured(2) : match.captured(4);
  width              = letters.isEmpty() ? QChar() : letters.at(0);

  SS_ASSERT_LOG(out.byteOffset >= 0);
  SS_ASSERT(out.area != Area::Invalid, return false);
  return true;
}

/**
 * @brief Rejects the offset/bit combinations the grammar admits but the protocol does not: a bit
 *        index outside 0-7, a bit on a multi-byte width, a missing width, and out-of-range blocks.
 */
[[nodiscard]] static bool checkBounds(const Address& out, Type width, QString& error)
{
  using namespace IO::Drivers::S7Address;

  if (width == Type::Invalid && out.bitOffset < 0) {
    error = trAddress("The address needs a width letter (X, B, W or D) or a bit index.");
    return false;
  }

  if (out.bitOffset >= 0 && width != Type::Invalid && width != Type::Bool) {
    error = trAddress("A bit index is only valid on a bit address (X).");
    return false;
  }

  if (out.bitOffset > 7) {
    error = trAddress("The bit index must be between 0 and 7.");
    return false;
  }

  if (out.byteOffset > kMaxByteOffset || out.dbNumber > kMaxDbNumber) {
    error = trAddress("The address is out of range for this controller.");
    return false;
  }

  SS_ASSERT_LOG(out.byteOffset >= 0);
  if (out.area == Area::DataBlk && out.dbNumber <= 0) {
    error = trAddress("A data-block address needs a block number above zero.");
    return false;
  }

  return true;
}

/**
 * @brief Resolves the final type and read size once the body and the optional suffix agree.
 */
[[nodiscard]] static bool resolveType(
  Address& out, const QString& body, Type width, Type suffix, int stringLength, QString& error)
{
  using namespace IO::Drivers::S7Address;

  const Type implied = width == Type::Invalid ? Type::Bool : width;
  out.type           = suffix == Type::Invalid ? implied : suffix;
  if (suffix != Type::Invalid && !suffixFitsWidth(suffix, implied)) {
    error = trAddress("Type %1 does not fit the width of \"%2\".").arg(codeFromType(suffix), body);
    return false;
  }

  if (out.type != Type::Str) {
    out.size = typeWidth(out.type);
    SS_ASSERT_LOG(out.size > 0);
    return true;
  }

  if (stringLength <= 0 || stringLength > kMaxStringLen) {
    error = trAddress("A STRING address needs a length between 1 and %1.").arg(kMaxStringLen);
    return false;
  }

  out.size = stringLength + 2;
  SS_ASSERT_LOG(out.size > 2);
  return true;
}

/**
 * @brief Parses an absolute S7 address; an invalid address comes back with @p error set and every
 *        field left at its invalid default, so a caller that ignores the return still cannot read.
 */
Address IO::Drivers::S7Address::parse(const QString& text, QString& error)
{
  error.clear();

  Address out;
  const auto trimmed = text.trimmed();
  if (trimmed.isEmpty()) {
    error = trAddress("The address is empty.");
    return {};
  }

  QString body;
  Type suffix      = Type::Invalid;
  int stringLength = 0;
  if (!splitSuffix(trimmed, body, suffix, stringLength, error))
    return {};

  QChar widthLetter;
  if (!matchBody(body, out, widthLetter, error))
    return {};

  const Type width = typeFromWidthLetter(widthLetter);
  if (!checkBounds(out, width, error))
    return {};

  if (!resolveType(out, body, width, suffix, stringLength, error))
    return {};

  if (out.type == Type::Bool && out.bitOffset < 0)
    out.bitOffset = 0;

  SS_ASSERT(isValid(out), return {});
  return out;
}
