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

#include "CSV/Player/MultiSourceMap.h"

#include <QSet>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction & queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an empty layout: single-source until a build() says otherwise.
 */
CSV::MultiSourceMap::MultiSourceMap() : m_multiSource(false) {}

/**
 * @brief Returns whether the recording spans more than one source.
 */
bool CSV::MultiSourceMap::multiSource() const
{
  return m_multiSource;
}

/**
 * @brief Returns the source id of every tracked presence bit, in bit order.
 */
const QVector<int>& CSV::MultiSourceMap::bitSourceIds() const
{
  return m_bitSourceIds;
}

/**
 * @brief Returns the per file cell presence bit handed to the background indexer.
 */
const QVector<quint8>& CSV::MultiSourceMap::fileColumnSourceBit() const
{
  return m_fileColumnSourceBit;
}

/**
 * @brief Returns each source's data columns in the local order its replay map expects.
 */
const QMap<int, QVector<int>>& CSV::MultiSourceMap::sourceColumnsByIndex() const
{
  return m_sourceColumnsByIndex;
}

//--------------------------------------------------------------------------------------------------
// Layout construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drops the whole layout, including the indexed presence bitmap.
 */
void CSV::MultiSourceMap::clear()
{
  m_multiSource = false;
  m_bitSourceIds.clear();
  m_lastSourceRow.clear();
  m_rowSourceBits.clear();
  m_rowSourceBits.squeeze();
  m_fileColumnSourceBit.clear();
  m_sourceColumnsByIndex.clear();
}

/**
 * @brief Forgets which row each source was last republished from, so the next backfill pass
 *        re-publishes every source even if it lands on the same row (used after a seek).
 */
void CSV::MultiSourceMap::resetLastSourceRows()
{
  m_lastSourceRow.fill(-1);
}

/**
 * @brief Appends one indexed batch's per-row presence masks to the timeline.
 */
void CSV::MultiSourceMap::appendRowSourceBits(const QVector<quint8>& bits)
{
  m_rowSourceBits += bits;
}

/**
 * @brief Assigns one presence bit per source (first eight win) and paints it onto the file
 *        cells that source owns; @p toFileColumn undoes the timestamp-column exclusion.
 */
void CSV::MultiSourceMap::assignSourceBits(const std::vector<ReplayColumnRef>& columns,
                                           int fileColumnCount,
                                           const ColumnMapper& toFileColumn)
{
  SS_ASSERT(fileColumnCount >= 0, return);
  SS_ASSERT(static_cast<bool>(toFileColumn), return);

  m_bitSourceIds.clear();
  m_fileColumnSourceBit = QVector<quint8>(fileColumnCount, 0);

  const auto columnCount = static_cast<int>(columns.size());
  for (int i = 0; i < columnCount; ++i) {
    const int srcId = columns[static_cast<size_t>(i)].sourceId;
    int bit         = static_cast<int>(m_bitSourceIds.indexOf(srcId));
    if (bit < 0 && m_bitSourceIds.size() < kMaxTrackedSources) {
      bit = m_bitSourceIds.size();
      m_bitSourceIds.append(srcId);
    }

    const int fileCol = toFileColumn(i);
    if (bit >= 0 && fileCol >= 0 && fileCol < m_fileColumnSourceBit.size())
      m_fileColumnSourceBit[fileCol] = static_cast<quint8>(1u << bit);
  }

  m_lastSourceRow = QVector<int>(m_bitSourceIds.size(), -1);
}

/**
 * @brief Builds the layout from the export-schema @p columns and returns the per-source
 *        FrameBuilder replay map for the caller to install. A single-source recording keeps
 *        the legacy flat map under source 0; a multi-source one numbers each source's columns
 *        locally, which is the order injectRow() feeds the replay lane in.
 */
CSV::MultiSourceMap::ReplayColumnMap CSV::MultiSourceMap::build(
  const std::vector<ReplayColumnRef>& columns,
  int fileColumnCount,
  const ColumnMapper& toFileColumn)
{
  SS_ASSERT(fileColumnCount >= 0, return {});
  SS_ASSERT(static_cast<bool>(toFileColumn), return {});

  m_sourceColumnsByIndex.clear();

  QSet<int> sources;
  for (const auto& column : columns)
    sources.insert(column.sourceId);

  m_multiSource          = sources.size() > 1;
  const auto columnCount = static_cast<int>(columns.size());

  ReplayColumnMap replay;
  if (!m_multiSource) {
    for (int i = 0; i < columnCount; ++i)
      replay[0][columns[static_cast<size_t>(i)].uniqueId] = i;
  }

  else {
    std::unordered_map<int, int> nextLocal;
    for (int i = 0; i < columnCount; ++i) {
      const auto& column = columns[static_cast<size_t>(i)];
      m_sourceColumnsByIndex[column.sourceId].append(i);
      replay[column.sourceId][column.uniqueId] = nextLocal[column.sourceId]++;
    }
  }

  assignSourceBits(columns, fileColumnCount, toFileColumn);
  return replay;
}

//--------------------------------------------------------------------------------------------------
// Sparse backfill
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latest present row at or before @p playheadRow for every source the strided catch-up
 *        skipped (spec 0064): a sparse recording holds a slow source's cells on a handful of
 *        rows per second, so strided sampling almost never lands on one and its widgets would
 *        freeze. Sources already republished from that row are left out; the rest are marked.
 */
QVarLengthArray<CSV::SourceRowRef, CSV::kMaxTrackedSources> CSV::MultiSourceMap::staleSources(
  int playheadRow)
{
  constexpr int kBackfillScanMax = 262144;

  QVarLengthArray<SourceRowRef, kMaxTrackedSources> stale;
  if (!m_multiSource || m_bitSourceIds.isEmpty())
    return stale;

  const int last = qMin(playheadRow, static_cast<int>(m_rowSourceBits.size()) - 1);
  if (last < 0)
    return stale;

  SS_ASSERT(m_lastSourceRow.size() == m_bitSourceIds.size(), return stale);

  for (int b = 0; b < m_bitSourceIds.size(); ++b) {
    const auto mask = static_cast<quint8>(1u << b);
    const int stop  = qMax(0, last - kBackfillScanMax);
    int found       = -1;
    for (int r = last; r >= stop; --r) {
      if (m_rowSourceBits.at(r) & mask) {
        found = r;
        break;
      }
    }

    if (found < 0 || found == m_lastSourceRow.value(b, -1))
      continue;

    m_lastSourceRow[b] = found;
    stale.append({found, m_bitSourceIds.at(b)});
  }

  return stale;
}
