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

#include "DataModel/Scripting/ReplayRowCodec.h"

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Row writer
//--------------------------------------------------------------------------------------------------

/**
 * @brief Joins replay cells into one comma-separated row, RFC-4180-quoting any cell that
 *        contains a comma, quote or newline. Counterpart of splitReplayRow.
 */
QByteArray DataModel::joinReplayRow(const QStringList& cells)
{
  QString row;
  for (int i = 0; i < cells.size(); ++i) {
    if (i > 0)
      row.append(QChar(','));

    const QString& cell      = cells.at(i);
    const bool needs_quoting = cell.contains(QChar(',')) || cell.contains(QChar('"'))
                            || cell.contains(QChar('\n')) || cell.contains(QChar('\r'));
    if (!needs_quoting) {
      row.append(cell);
      continue;
    }

    QString quoted = cell;
    quoted.replace(QChar('"'), QStringLiteral("\"\""));
    row.append(QChar('"'));
    row.append(quoted);
    row.append(QChar('"'));
  }

  return row.toUtf8();
}

//--------------------------------------------------------------------------------------------------
// QString row reader
//--------------------------------------------------------------------------------------------------

/**
 * @brief Undoes the CSV export's formula-injection guard: a leading apostrophe before a
 *        dangerous first char is a sanitizer artifact, not data. Restores recordings written
 *        before numeric fields were exempted from the guard ("'-0.5" -> "-0.5").
 */
static void stripCsvInjectionGuard(QString& cell)
{
  if (cell.size() < 2 || cell.at(0) != QChar('\''))
    return;

  const QChar c     = cell.at(1);
  const bool danger = c == QChar('=') || c == QChar('+') || c == QChar('-') || c == QChar('@')
                   || c == QChar('\t') || c == QChar('\r');
  if (danger)
    cell.remove(0, 1);
}

/**
 * @brief Quote-aware comma split of one synthesized replay row (RFC-4180 double-quote escape).
 */
QStringList DataModel::splitReplayRow(QStringView row)
{
  QStringList cells;
  QString cell;
  bool in_quotes  = false;
  bool was_quoted = false;

  const qsizetype length = row.size();
  for (qsizetype i = 0; i < length; ++i) {
    const QChar c = row.at(i);

    if (in_quotes) {
      const bool escaped = c == QChar('"') && i + 1 < length && row.at(i + 1) == QChar('"');
      if (escaped) {
        cell.append(QChar('"'));
        ++i;
        continue;
      }

      if (c == QChar('"')) {
        in_quotes = false;
        continue;
      }

      cell.append(c);
      continue;
    }

    if (c == QChar(',')) {
      QString value = was_quoted ? cell : cell.trimmed();
      stripCsvInjectionGuard(value);
      cells.append(std::move(value));
      cell.clear();
      was_quoted = false;
      continue;
    }

    if (c == QChar('"') && !was_quoted && cell.trimmed().isEmpty()) {
      in_quotes  = true;
      was_quoted = true;
      cell.clear();
      continue;
    }

    cell.append(c);
  }

  QString last = was_quoted ? cell : cell.trimmed();
  stripCsvInjectionGuard(last);
  cells.append(std::move(last));
  return cells;
}

//--------------------------------------------------------------------------------------------------
// Byte-view row reader
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when every byte in [begin, end) of @p row is ASCII whitespace.
 */
[[nodiscard]] static bool allAsciiSpace(QByteArrayView row, qsizetype begin, qsizetype end)
{
  for (qsizetype i = begin; i < end; ++i) {
    const char c = row.at(i);
    if (c != ' ' && c != '\t' && c != '\n' && c != '\v' && c != '\f' && c != '\r')
      return false;
  }

  return true;
}

/**
 * @brief ASCII-whitespace trim of a cell view (byte twin of QString::trimmed for the
 *        character set replay rows actually contain).
 */
[[nodiscard]] static QByteArrayView trimmedCellView(QByteArrayView v)
{
  qsizetype b = 0;
  qsizetype e = v.size();
  while (b < e && allAsciiSpace(v, b, b + 1))
    ++b;
  while (e > b && allAsciiSpace(v, e - 1, e))
    --e;

  return v.sliced(b, e - b);
}

/**
 * @brief View twin of stripCsvInjectionGuard: drops the sanitizer apostrophe by advancing
 *        the view instead of mutating a string.
 */
[[nodiscard]] static QByteArrayView stripGuardView(QByteArrayView v)
{
  if (v.size() < 2 || v.at(0) != '\'')
    return v;

  const char c      = v.at(1);
  const bool danger = c == '=' || c == '+' || c == '-' || c == '@' || c == '\t' || c == '\r';
  return danger ? v.sliced(1) : v;
}

/**
 * @brief Byte replica of splitReplayRow's per-character machine for one cell region that
 *        needs rewriting (escaped quotes or content around a closed quote); appends the
 *        finalized cell bytes to @p scratch.
 */
static void appendRewrittenCell(QByteArrayView row,
                                qsizetype begin,
                                qsizetype end,
                                QByteArray& scratch)
{
  SS_ASSERT(begin <= end, return);
  SS_ASSERT(end <= row.size(), return);

  bool in_quotes            = false;
  bool was_quoted           = false;
  const qsizetype cell_base = scratch.size();
  for (qsizetype i = begin; i < end; ++i) {
    const char c = row.at(i);
    if (in_quotes) {
      const bool escaped = c == '"' && i + 1 < end && row.at(i + 1) == '"';
      if (escaped) {
        scratch.append('"');
        ++i;
        continue;
      }

      if (c == '"') {
        in_quotes = false;
        continue;
      }

      scratch.append(c);
      continue;
    }

    if (c == '"' && !was_quoted
        && allAsciiSpace(QByteArrayView(scratch), cell_base, scratch.size())) {
      in_quotes  = true;
      was_quoted = true;
      scratch.resize(cell_base);
      continue;
    }

    scratch.append(c);
  }
}

namespace detail {

/**
 * @brief A cell whose bytes had to be rewritten into the scratch buffer: the slot it occupies
 *        in the output array plus the scratch byte range it owns.
 */
struct RewrittenCell {
  qsizetype slot = 0;
  qsizetype base = 0;
  qsizetype size = 0;
};

/**
 * @brief Scratch-backed cells awaiting resolution. Stack storage covers the rows that carry
 *        escaped quotes at all; plain numeric rows never touch it.
 */
using RewrittenCells = QVarLengthArray<RewrittenCell, 8>;

}  // namespace detail

using detail::RewrittenCell;
using detail::RewrittenCells;

/**
 * @brief Byte-level twin of splitReplayRow: identical quote/trim/guard semantics, cells
 *        returned as views into @p row or @p scratch. Scratch-backed cells stay offsets until
 *        after the final append, so a reallocation cannot strand an emitted view; the reserve
 *        only keeps the append loop allocation-free.
 */
void DataModel::splitReplayRowSpans(QByteArrayView row,
                                    ReplayCellViews& out,
                                    QByteArray& scratch,
                                    char separator)
{
  out.clear();
  scratch.resize(0);
  if (row.endsWith('\r'))
    row.chop(1);

  if (scratch.capacity() < row.size())
    scratch.reserve(row.size());

  SS_ASSERT_LOG(scratch.capacity() >= row.size());

  const qsizetype length  = row.size();
  qsizetype cell_start    = 0;
  qsizetype interior_from = -1;
  qsizetype interior_to   = -1;
  bool in_quotes          = false;
  bool was_quoted         = false;
  bool needs_rewrite      = false;
  RewrittenCells rewritten;

  const auto finalize = [&](qsizetype end) {
    QByteArrayView value;
    if (!was_quoted)
      value = trimmedCellView(row.sliced(cell_start, end - cell_start));
    else if (!needs_rewrite) {
      const qsizetype to = (interior_to >= 0) ? interior_to : end;
      value              = row.sliced(interior_from, to - interior_from);
    } else {
      const qsizetype base = scratch.size();
      appendRewrittenCell(row, cell_start, end, scratch);
      SS_ASSERT_LOG(scratch.size() <= row.size());
      rewritten.append(RewrittenCell{out.size(), base, scratch.size() - base});
    }

    out.append(stripGuardView(value));
    interior_from = -1;
    interior_to   = -1;
    was_quoted    = false;
    needs_rewrite = false;
  };

  for (qsizetype i = 0; i < length; ++i) {
    const char c = row.at(i);

    if (in_quotes) {
      const bool escaped = c == '"' && i + 1 < length && row.at(i + 1) == '"';
      if (escaped) {
        needs_rewrite = true;
        ++i;
        continue;
      }

      if (c == '"') {
        in_quotes   = false;
        interior_to = i;
      }

      continue;
    }

    if (c == separator) {
      finalize(i);
      cell_start = i + 1;
      continue;
    }

    if (c == '"' && !was_quoted && allAsciiSpace(row, cell_start, i)) {
      in_quotes     = true;
      was_quoted    = true;
      interior_from = i + 1;
      continue;
    }

    if (was_quoted)
      needs_rewrite = true;
  }

  finalize(length);

  for (const auto& cell : rewritten)
    out[cell.slot] = stripGuardView(QByteArrayView(scratch).sliced(cell.base, cell.size));
}

/**
 * @brief Replay twin of splitQuickPlotChannels: one quote-aware row per non-empty line.
 */
void DataModel::splitReplayChannels(const QByteArray& rawFrame, QList<QStringList>& outChannels)
{
  outChannels.clear();
  if (rawFrame.isEmpty())
    return;

  const QString text = QString::fromUtf8(rawFrame);
  const auto lines   = QStringView(text).split(QChar('\n'), Qt::SkipEmptyParts);
  for (const auto& line : lines) {
    const auto trimmed = line.trimmed();
    if (trimmed.isEmpty())
      continue;

    outChannels.append(splitReplayRow(trimmed));
  }
}
