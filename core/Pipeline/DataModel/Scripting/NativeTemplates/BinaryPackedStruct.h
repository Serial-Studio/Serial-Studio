/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <array>
#include <QByteArray>
#include <QString>

#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"

namespace DataModel {

/**
 * @brief Most fields one packed struct may carry: the Quick Plot channel cap.
 */
inline constexpr int kMaxPackedFields = 512;

/**
 * @brief One field type of a packed struct, in the order the "fields" parameter names them.
 */
enum class PackedType : quint8 {
  I8,
  U8,
  I16,
  U16,
  I32,
  U32,
  I64,
  U64,
  F32,
  F64,
};

/**
 * @brief A parsed packed-struct layout (spec 0111 R14): the field types in frame order, the frame
 *        length they add up to, and the byte order every multi-byte field is read in.
 */
struct PackedLayout {
  std::array<PackedType, kMaxPackedFields> types{};
  int count      = 0;
  int bytes      = 0;
  bool bigEndian = false;
};

[[nodiscard]] bool parsePackedLayout(const QString& fields,
                                     const QString& endianness,
                                     PackedLayout& layout,
                                     QString& error);
[[nodiscard]] qsizetype decodePackedStruct(const QByteArray& frame,
                                           const PackedLayout& layout,
                                           double* out,
                                           qsizetype capacity) noexcept;
[[nodiscard]] const INativeTemplate& packedStructTemplate();

}  // namespace DataModel
