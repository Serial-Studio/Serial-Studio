/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Sessions/Player/ReplayFrameValues.h"

#  include <algorithm>
#  include <cmath>

#  include "Core/SSAssert.h"

/**
 * @brief Formats a stored numeric value as a replay cell. 17 significant digits is the shortest
 *        precision that round-trips an IEEE-754 double, so the recording replays the value it
 *        stored rather than a rounded twin of it.
 */
QString Sessions::ReplayFrameValues::formatCell(double value)
{
  return QString::number(value, 'g', 17);
}

/**
 * @brief Adds one decoded row to a replayed instant, ignoring datasets the loaded layout does not
 *        carry, and noting the source the row's column belongs to.
 */
void Sessions::ReplayFrameValues::applyRow(ReplayRowValues& out,
                                           const ReadingRow& row,
                                           const ReplayLayout& layout)
{
  const auto it = layout.uidToColumn.constFind(row.uniqueId);
  if (it == layout.uidToColumn.constEnd())
    return;

  out.values[row.uniqueId] = row.isNumeric ? formatCell(row.finalNumeric) : row.finalString;

  const auto srcIt = layout.columnToSource.constFind(it.value());
  if (srcIt != layout.columnToSource.constEnd())
    out.sources.insert(srcIt.value());
}

/**
 * @brief Selects the rows stored at exactly @p timestampNs. Rows at neighbouring instants are
 *        skipped rather than interpolated: a recording replays the values it captured, and a
 *        synthesised in-between sample would be indistinguishable from a recorded one downstream.
 */
void Sessions::ReplayFrameValues::selectRowsAt(ReplayRowValues& out,
                                               const std::vector<ReadingRow>& rows,
                                               qint64 timestampNs,
                                               const ReplayLayout& layout)
{
  for (const auto& row : rows) {
    if (row.timestampNs != timestampNs)
      continue;

    applyRow(out, row, layout);
  }
}

/**
 * @brief Drops each decoded row onto its own row of a seek window: the exact timestamp match is
 *        the placement rule, so a row whose instant is not a window row (a dense sample between
 *        two frames) is left out instead of shifted onto a neighbour.
 */
void Sessions::ReplayFrameValues::scatterRowsIntoWindow(const std::vector<ReadingRow>& rows,
                                                        std::span<const qint64> rowTimes,
                                                        const QHash<int, qint64>& keyByUid,
                                                        QHash<qint64, QVector<double>>& series)
{
  const auto begin = rowTimes.begin();
  const auto end   = rowTimes.end();

  for (const auto& row : rows) {
    const auto keyIt = keyByUid.constFind(row.uniqueId);
    if (keyIt == keyByUid.constEnd())
      continue;

    const auto pos = std::lower_bound(begin, end, row.timestampNs);
    if (pos == end || *pos != row.timestampNs)
      continue;

    const auto seriesIt = series.find(keyIt.value());
    if (seriesIt == series.end())
      continue;

    const auto index = static_cast<qsizetype>(pos - begin);
    SS_ASSERT_LOG(index < seriesIt.value().size());
    if (index >= seriesIt.value().size())
      continue;

    seriesIt.value()[static_cast<int>(index)] = row.finalNumeric;
  }
}

/**
 * @brief Forward-fills NaN gaps and backfills the leading run from the first stored value, so a
 *        sparse dataset draws as the step function it was recorded as instead of a broken line.
 */
void Sessions::ReplayFrameValues::fillSeekGaps(QVector<double>& values)
{
  int firstSet = -1;
  const int n  = values.size();
  for (int k = 0; k < n; ++k)
    if (std::isnan(values[k]))
      values[k] = (k > 0) ? values[k - 1] : values[k];
    else if (firstSet < 0)
      firstSet = k;

  const double seed = (firstSet >= 0) ? values[firstSet] : 0.0;
  for (int k = 0; k < n && std::isnan(values[k]); ++k)
    values[k] = seed;
}

#endif
