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

#pragma once

#include <cstdint>
#include <QString>

namespace IO {
namespace Drivers {

/**
 * @brief Absolute S7 addressing vocabulary (spec 0073), Qt Core-only so the driver, the ctest tier
 *        and the project generator share ONE definition of what `DB5.DBD20` means.
 */
namespace S7Address {

inline constexpr int kMaxDbNumber   = 65535;
inline constexpr int kMaxByteOffset = 65535;
inline constexpr int kMaxStringLen  = 254;

/**
 * @brief Memory area; the numeric value is the S7 area code the protocol carries.
 */
enum class Area : std::uint8_t {
  Input   = 0x81,
  Output  = 0x82,
  Memory  = 0x83,
  DataBlk = 0x84,
  Invalid = 0x00,
};

/**
 * @brief Value type read out of an area; the width in bytes comes from @ref typeWidth.
 */
enum class Type : std::uint8_t {
  Bool,
  Byte,
  Word,
  DWord,
  Int,
  DInt,
  Real,
  Str,
  Invalid,
};

/**
 * @brief One parsed absolute address: where to read, how wide, and how to render the bytes.
 */
struct Address {
  Area area      = Area::Invalid;
  Type type      = Type::Invalid;
  int dbNumber   = 0;
  int byteOffset = 0;
  int bitOffset  = -1;
  int size       = 0;
};

[[nodiscard]] Address parse(const QString& text, QString& error);
[[nodiscard]] bool isValid(const Address& address) noexcept;
[[nodiscard]] int typeWidth(Type type) noexcept;
[[nodiscard]] Type typeFromCode(const QString& code) noexcept;
[[nodiscard]] QString codeFromType(Type type) noexcept;
[[nodiscard]] QString areaCode(Area area) noexcept;
[[nodiscard]] QString normalize(const Address& address);

}  // namespace S7Address
}  // namespace Drivers
}  // namespace IO
