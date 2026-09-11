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

#include "CSV/Player/RowSyntax.h"

#include "Core/DSPSimd.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Record framing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether every byte of [@p from, @p to) is ASCII whitespace; the cell-start test the
 *        replay splitter applies before letting a quote open a field. Callers carry the answer
 *        forward across a cell instead of re-asking from its start, so each byte is examined
 *        once: rescanning per quote made a 4 MB record of unquoted JSON quadratic.
 */
[[nodiscard]] static bool onlySpaceBetween(QByteArrayView data, qsizetype from, qsizetype to)
{
  SS_ASSERT(from >= 0, return false);
  SS_ASSERT(to <= data.size(), return false);

  for (qsizetype i = from; i < to; ++i) {
    const char c = data.at(i);
    if (c != ' ' && c != '\t' && c != '\n' && c != '\v' && c != '\f' && c != '\r')
      return false;
  }

  return true;
}

/**
 * @brief Index just past the closing quote of the field whose content starts at @p from, or the
 *        size when the file ends inside it. Doubled quotes escape and keep the field open.
 */
[[nodiscard]] static qsizetype skipQuotedField(QByteArrayView data, qsizetype from)
{
  static constexpr quint8 kQuote[] = {'"'};

  const qsizetype size = data.size();
  SS_ASSERT(from >= 0, return size);
  SS_ASSERT(from <= size, return size);

  qsizetype i = from;
  while (i < size) {
    const qsizetype hit = i + DSP::simdFindAnyByte(data.constData() + i, size - i, kQuote, 1);
    if (hit >= size)
      return size;

    if (hit + 1 < size && data.at(hit + 1) == '"') {
      i = hit + 2;
      continue;
    }

    return hit + 1;
  }

  return size;
}

/**
 * @brief Index of the first plain newline at or after @p from; the line-aligned fallback taken
 *        when quote-aware framing overruns the record cap.
 */
[[nodiscard]] static qsizetype firstPlainNewline(QByteArrayView data, qsizetype from)
{
  static constexpr quint8 kNewline[] = {'\n'};

  const qsizetype size = data.size();
  SS_ASSERT(from >= 0, return size);
  SS_ASSERT(from <= size, return size);

  return from + DSP::simdFindAnyByte(data.constData() + from, size - from, kNewline, 1);
}

/**
 * @brief Index of the newline ending the record at @p from, mirroring the replay splitter's
 *        machine so the index and the cell split can never disagree about where a row ends.
 */
qsizetype CSV::nextRecordEnd(QByteArrayView data,
                             qsizetype from,
                             char separator,
                             qsizetype maxRecordBytes)
{
  static constexpr int kEdgeCount = 3;

  const qsizetype size = data.size();
  SS_ASSERT(from >= 0 && from <= size, return size);
  SS_ASSERT(maxRecordBytes > 0, return firstPlainNewline(data, from));

  const quint8 edges[kEdgeCount] = {
    static_cast<quint8>(separator), static_cast<quint8>('"'), static_cast<quint8>('\n')};

  qsizetype i          = from;
  bool was_quoted      = false;
  bool only_space_seen = true;

  while (i < size && i - from <= maxRecordBytes) {
    const qsizetype hit =
      i + DSP::simdFindAnyByte(data.constData() + i, size - i, edges, kEdgeCount);
    if (hit >= size)
      return size;

    const char c = data.at(hit);
    if (c == '\n')
      return hit;

    if (c == separator) {
      was_quoted      = false;
      only_space_seen = true;
      i               = hit + 1;
      continue;
    }

    only_space_seen  = only_space_seen && onlySpaceBetween(data, i, hit);
    const bool opens = !was_quoted && only_space_seen;
    was_quoted       = was_quoted || opens;
    only_space_seen  = false;
    i                = opens ? skipQuotedField(data, hit + 1) : hit + 1;
  }

  const bool overran = (size - from) > maxRecordBytes;
  return overran ? firstPlainNewline(data, from) : size;
}

//--------------------------------------------------------------------------------------------------
// Quote-aware scanning
//--------------------------------------------------------------------------------------------------

/**
 * @brief Index of the first top-level @p separator of a raw row (quote-aware, mirroring the
 *        replay splitter's machine), or -1 when the row has a single cell.
 */
qsizetype CSV::firstTopLevelSeparator(QByteArrayView row, char separator)
{
  bool in_quotes       = false;
  bool was_quoted      = false;
  bool only_space_seen = true;

  const qsizetype length = row.size();
  for (qsizetype i = 0; i < length; ++i) {
    const char c = row.at(i);

    if (in_quotes) {
      const bool escaped = c == '"' && i + 1 < length && row.at(i + 1) == '"';
      if (escaped) {
        ++i;
        continue;
      }

      if (c == '"')
        in_quotes = false;

      continue;
    }

    if (c == separator)
      return i;

    if (c == '"' && !was_quoted && only_space_seen) {
      in_quotes  = true;
      was_quoted = true;
      continue;
    }

    if (c != ' ' && c != '\t' && c != '\n' && c != '\v' && c != '\f' && c != '\r')
      only_space_seen = false;
  }

  return -1;
}

/**
 * @brief Top-level occurrences of @p separator in @p row using a cell-position-independent
 *        quote scanner (any double quote toggles quoted mode, "" escapes): unlike the RFC
 *        splitter's machine, quoted content never scores for ANY candidate, so a quoted cell
 *        cannot leak separators into the sniff of a differently-separated file.
 */
qsizetype CSV::topLevelSeparatorCount(QByteArrayView row, char separator)
{
  SS_ASSERT(separator != '"', return 0);
  SS_ASSERT_LOG(!row.isEmpty());

  bool in_quotes         = false;
  qsizetype count        = 0;
  const qsizetype length = row.size();
  for (qsizetype i = 0; i < length; ++i) {
    const char c = row.at(i);

    if (c == '"') {
      const bool escaped = in_quotes && i + 1 < length && row.at(i + 1) == '"';
      if (escaped) {
        ++i;
        continue;
      }

      in_quotes = !in_quotes;
      continue;
    }

    if (!in_quotes && c == separator)
      ++count;
  }

  return count;
}

//--------------------------------------------------------------------------------------------------
// Header interpretation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Detects the row separator (spec 0048): candidates scored by summed top-level
 *        occurrence count over header + first data row, comma scored first and winning
 *        ties. A non-comma winner needs a data-row hit AND header count == data count,
 *        so text-cell separators ("1,a;b;c") never re-interpret a comma file.
 */
char CSV::sniffSeparator(QByteArrayView headerRow, QByteArrayView dataRow)
{
  SS_ASSERT(!headerRow.isEmpty(), return ',');
  SS_ASSERT(!dataRow.isEmpty(), return ',');

  constexpr char kCandidates[] = {',', ';', '\t', '|'};

  char best           = ',';
  qsizetype bestScore = -1;
  for (const char candidate : kCandidates) {
    const qsizetype data_count   = topLevelSeparatorCount(dataRow, candidate);
    const qsizetype header_count = topLevelSeparatorCount(headerRow, candidate);
    const qsizetype score        = header_count + data_count;

    const bool eligible = (candidate == ',') || (data_count >= 1 && header_count == data_count);
    if (eligible && score > bestScore) {
      best      = candidate;
      bestScore = score;
    }
  }

  return best;
}

/**
 * @brief Seconds-per-tick for a numeric timestamp header (spec 0048 R7): a bracketed unit
 *        ("time(ms)", "t [us]") or a "_unit" suffix ("time_ms") selects the scale; a
 *        header naming no recognizable unit returns nullopt so the caller can ask.
 */
std::optional<double> CSV::timestampUnitScale(const QString& header)
{
  const QString text = header.trimmed().toLower();
  SS_ASSERT(!text.isEmpty(), return std::nullopt);

  QString unit;
  const qsizetype paren   = text.lastIndexOf(QChar('('));
  const qsizetype bracket = text.lastIndexOf(QChar('['));
  if (paren >= 0 && text.endsWith(QChar(')')))
    unit = text.mid(paren + 1, text.size() - paren - 2).trimmed();
  else if (bracket >= 0 && text.endsWith(QChar(']')))
    unit = text.mid(bracket + 1, text.size() - bracket - 2).trimmed();
  else if (text.lastIndexOf(QChar('_')) >= 0)
    unit = text.mid(text.lastIndexOf(QChar('_')) + 1).trimmed();

  SS_ASSERT_LOG(unit.size() <= text.size());

  if (unit == QStringLiteral("ms") || unit == QStringLiteral("msec")
      || unit == QStringLiteral("millis") || unit == QStringLiteral("milliseconds"))
    return 1e-3;

  if (unit == QStringLiteral("us") || unit == QStringLiteral("\u00b5s")
      || unit == QStringLiteral("usec") || unit == QStringLiteral("microseconds"))
    return 1e-6;

  if (unit == QStringLiteral("ns") || unit == QStringLiteral("nsec")
      || unit == QStringLiteral("nanoseconds"))
    return 1e-9;

  if (unit == QStringLiteral("s") || unit == QStringLiteral("sec") || unit == QStringLiteral("secs")
      || unit == QStringLiteral("seconds"))
    return 1.0;

  return std::nullopt;
}
