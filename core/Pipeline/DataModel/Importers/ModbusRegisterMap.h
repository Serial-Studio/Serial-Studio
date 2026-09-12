/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
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

#include <QString>
#include <QVector>

namespace DataModel::ModbusMap {

inline constexpr int kMinUnitId   = 1;  ///< Lowest addressable Modbus unit (0 is broadcast)
inline constexpr int kMaxUnitId   = 247;
inline constexpr int kMaxBitIndex = 63;  ///< Widest register type is 64 bits

/**
 * @brief Clamps a map cell to a pollable unit id, 0 meaning "the connection's own unit".
 */
[[nodiscard]] inline quint8 clampUnitId(int unit)
{
  return (unit >= kMinUnitId && unit <= kMaxUnitId) ? static_cast<quint8>(unit) : quint8(0);
}

/**
 * @brief Clamps a map cell to a bit index inside a value of @p registers words, -1 meaning "the
 *        whole word"; an index past the value's width is dropped rather than left to read as 0.
 */
[[nodiscard]] inline int clampBitIndex(int bit, int registers)
{
  const int maxBit = qMin(kMaxBitIndex, 16 * qMax(1, registers) - 1);
  return (bit >= 0 && bit <= maxBit) ? bit : -1;
}

/**
 * @brief One Modbus register parsed from a CSV/XML/JSON map. The spec-0083 columns default to
 *        "the connection's unit, the whole word, read-only, big-endian", so a map without them
 *        imports exactly as before.
 */
struct RegisterEntry {
  quint16 address;
  QString name;
  quint8 registerType;
  QString dataType;
  QString units;
  double min;
  double max;
  double scale;
  double offset;
  quint8 unitId = 0;      ///< Unit the block is polled from (0 = the connection's unit)
  int bitIndex  = -1;     ///< Bit inside a holding/input register (-1 = the whole word)
  bool writable = false;  ///< Produces an output control that writes the register
  QString wordOrder;      ///< "" (abcd) | "cdab" | "badc" | "dcba" for multi-register types
};

[[nodiscard]] quint8 parseRegisterType(const QString& str);

[[nodiscard]] QString normalizeWordOrder(const QString& text);

[[nodiscard]] bool parseWritable(const QString& text);

[[nodiscard]] int registersForDataType(const QString& dataType);

[[nodiscard]] bool parseCsv(const QString& path, QVector<RegisterEntry>& out);

[[nodiscard]] bool parseXml(const QString& path, QVector<RegisterEntry>& out);

[[nodiscard]] bool parseJson(const QString& path, QVector<RegisterEntry>& out);

}  // namespace DataModel::ModbusMap
