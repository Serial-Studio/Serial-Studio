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

#include <array>
#include <QByteArrayView>
#include <QtGlobal>

namespace DataModel {

/**
 * @brief Quick Plot's line grammar (spec 0106): one tokenizer shared by the live lane and the
 *        line_values Built-In template, so a saved project parses exactly what the dashboard
 *        showed. Everything here works on byte views into the caller's line: no allocation, and
 *        every loop is bounded by the line length or kMaxLineFields.
 */

/**
 * @brief Fields one line can yield. Wider than the span lane so replay rows of a wide CSV keep
 *        working; the tail of a wider line is dropped, never asserted on.
 */
inline constexpr qsizetype kMaxLineFields = 512;

/**
 * @brief Bytes the JSON scanner may write leaf paths into for one line.
 */
inline constexpr qsizetype kLinePathArenaBytes = 8192;

/**
 * @brief Longest token the decimal-comma rewrite handles on the stack.
 */
inline constexpr qsizetype kMaxNumberTokenBytes = 64;

/**
 * @brief Longest unit suffix accepted after a number.
 */
inline constexpr qsizetype kMaxUnitBytes = 8;

/**
 * @brief Delimited covers every separator shape; Json is a one-line object flattened to leaves.
 */
enum class LineFormatKind : quint8 {
  Delimited,
  Json,
};

/**
 * @brief The shape a stream was detected as. separator is ',', ';', '\t', ' ' (runs of spaces)
 *        or '\0' for a single field; decimalComma reads 23,5 as 23.5; named records that the
 *        establishing line carried name:value fields; wrapper that it came inside a slash-star
 *        comment pair.
 */
struct LineFormat {
  LineFormatKind kind = LineFormatKind::Delimited;
  char separator      = ',';
  bool decimalComma   = false;
  bool named          = false;
  bool wrapper        = false;

  [[nodiscard]] bool operator==(const LineFormat&) const noexcept = default;
};

/**
 * @brief One parsed field. token is the trimmed text the field came from; name is empty for a
 *        positional field; value is the trimmed text after the name delimiter, or the whole
 *        token; unit is the accepted suffix; number holds the parsed value when numeric is true.
 *        A JSON leaf carries its path in name and the number text in token and value.
 */
struct LineField {
  QByteArrayView token;
  QByteArrayView name;
  QByteArrayView value;
  QByteArrayView unit;
  double number = 0.0;
  bool numeric  = false;
};

/**
 * @brief Bounded per-line scratch: the fields plus the arena JSON leaf paths point into.
 */
struct LineFields {
  std::array<LineField, kMaxLineFields> items;
  std::array<char, kLinePathArenaBytes> pathArena;
  qsizetype count     = 0;
  qsizetype arenaUsed = 0;
};

/**
 * @brief Trims ASCII space, tab, CR and LF from both ends.
 */
[[nodiscard]] QByteArrayView trimLineAscii(QByteArrayView text) noexcept;

/**
 * @brief Trims the line and removes a surrounding slash-star comment pair, reporting whether one
 *        was present.
 */
[[nodiscard]] QByteArrayView stripLineWrapper(QByteArrayView line, bool& wrapped) noexcept;

/**
 * @brief Parses a number with an optional unit suffix (23.5, +7, 1.5e3, 23,5 under decimalComma,
 *        1013hPa). Returns false for text, a bad unit or a hexadecimal literal.
 */
[[nodiscard]] bool parseLineNumber(QByteArrayView text,
                                   bool decimalComma,
                                   double& number,
                                   QByteArrayView& unit) noexcept;

/**
 * @brief Parses one token into a field: a name:value or name=value pair when the prefix is a
 *        well-formed name, a positional value otherwise. Returns the field's numeric flag.
 */
[[nodiscard]] bool parseLineField(QByteArrayView token, bool decimalComma, LineField& out) noexcept;

/**
 * @brief Fields of @p fields that parsed as numbers.
 */
[[nodiscard]] qsizetype numericLineFieldCount(const LineFields& fields) noexcept;

/**
 * @brief Splits one line under a fixed format into @p out and returns the field count.
 */
[[nodiscard]] qsizetype tokenizeLine(QByteArrayView line,
                                     const LineFormat& format,
                                     LineFields& out) noexcept;

/**
 * @brief Scores the separator candidates against one line (spec 0106 R9/R10): the first candidate
 *        whose fields are all numeric wins, else the best one with at least half numeric. On
 *        success @p format is set and @p fields holds the winning tokenization.
 */
[[nodiscard]] bool detectLineFormat(QByteArrayView line,
                                    LineFormat& format,
                                    LineFields& fields) noexcept;

}  // namespace DataModel
