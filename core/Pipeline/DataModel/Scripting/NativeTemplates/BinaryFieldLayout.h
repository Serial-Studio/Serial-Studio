/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QByteArrayView>
#include <QJsonArray>
#include <QString>
#include <vector>

#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"

namespace DataModel {

/**
 * @brief Most fields one layout may declare (spec 0127 R20).
 */
inline constexpr int kMaxLayoutFields = 256;

/**
 * @brief Highest byte offset a field may start at: the per-frame byte cap of every template.
 */
inline constexpr int kMaxLayoutOffset = 65535;

/**
 * @brief How the bits of one field are read as a number.
 */
enum class LayoutFieldType : quint8 {
  Unsigned,
  Signed,
  Float32,
  Float64,
};

/**
 * @brief One declared field: where it starts, how wide it is, how its bits become a number and
 *        the linear scaling applied after. A little-endian field counts its bit offset from the
 *        least significant bit of the start byte and accumulates upwards; a big-endian field
 *        counts from the most significant bit and reads downwards, so byte-aligned fields of
 *        either order are the usual integers.
 */
struct LayoutField {
  qint32 startBit      = 0;
  qint32 bits          = 8;
  double scale         = 1.0;
  double add           = 0.0;
  LayoutFieldType type = LayoutFieldType::Unsigned;
  bool bigEndian       = false;
};

/**
 * @brief A parsed field table in channel order.
 */
struct FieldLayout {
  std::vector<LayoutField> fields;
};

[[nodiscard]] bool parseFieldLayout(const QJsonArray& json, FieldLayout& layout, QString& error);
[[nodiscard]] qsizetype decodeFieldLayout(QByteArrayView frame,
                                          const FieldLayout& layout,
                                          double* out,
                                          qsizetype capacity) noexcept;
[[nodiscard]] QJsonArray defaultFieldLayout();
[[nodiscard]] const INativeTemplate& fieldLayoutTemplate();

}  // namespace DataModel
