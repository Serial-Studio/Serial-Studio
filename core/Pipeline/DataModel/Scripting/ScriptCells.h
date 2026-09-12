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

#include <QByteArray>
#include <QByteArrayView>
#include <QStringView>
#include <QtGlobal>
#include <vector>

namespace DataModel {

/**
 * @brief What a script produced for one cell: text bytes, or a number that also carries the text
 *        the engine's own formatting rule would have shown for it.
 */
enum class CellKind : quint8 {
  Text,
  Number,
};

/**
 * @brief One parsed cell (spec 0086): a byte span into the owning rows' scratch (offset + length
 *        rather than a view, so a scratch that grows on a wider frame never invalidates it) plus
 *        the numeric value when the script produced a number.
 */
struct ScriptCell {
  qsizetype offset;
  qsizetype length;
  double number;
  CellKind kind;
};

/**
 * @brief A parser result as rows of typed cells, owned by the frame builder and reused across
 *        frames: every container keeps its capacity across clear(), so a steady frame shape is
 *        appended without a heap operation. Cell text is valid until the next clear().
 */
class ScriptCellRows {
public:
  static constexpr qsizetype kMaxCellsPerResult  = 10000;
  static constexpr qsizetype kNumberTextCapacity = 32;
  static constexpr qsizetype kBytesPerCellGuess  = 16;

  ScriptCellRows();
  ScriptCellRows(ScriptCellRows&&)                 = delete;
  ScriptCellRows(const ScriptCellRows&)            = delete;
  ScriptCellRows& operator=(ScriptCellRows&&)      = delete;
  ScriptCellRows& operator=(const ScriptCellRows&) = delete;

  [[nodiscard]] qsizetype rowCount() const noexcept;
  [[nodiscard]] qsizetype cellCount() const noexcept;
  [[nodiscard]] const ScriptCell* rowCells(qsizetype row, qsizetype& count) const noexcept;
  [[nodiscard]] QByteArrayView view(const ScriptCell& cell) const noexcept;

  void clear() noexcept;
  void reserve(qsizetype cells, qsizetype bytes);
  void beginRow();
  void appendText(const char* bytes, qsizetype length);
  void appendUtf16(QStringView text);
  void appendNumber(double value, const char* text, qsizetype length);

private:
  std::vector<ScriptCell> m_cells;
  std::vector<qsizetype> m_rowStarts;
  QByteArray m_scratch;
};

/**
 * @brief Formats a Lua number exactly as the list path's luaValueToString() did: integers through
 *        QString::number(qlonglong), everything else through QString::number(v, 'g', 15). Writes
 *        at most @p capacity bytes into @p out and returns the length, 0 on overflow.
 */
[[nodiscard]] qsizetype formatLuaNumber(double value,
                                        bool isInteger,
                                        char* out,
                                        qsizetype capacity) noexcept;

/**
 * @brief Formats a double exactly as ECMAScript Number::toString does (the text
 *        QJSValue::toString() produces for a number): shortest round-trip digits laid out per
 *        ECMA-262 6.1.6.1.20, NaN / Infinity / -Infinity, and -0 as "0". Writes at most
 *        @p capacity bytes into @p out and returns the length, 0 on overflow.
 */
[[nodiscard]] qsizetype formatJsNumber(double value, char* out, qsizetype capacity) noexcept;

}  // namespace DataModel
