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

#  include "Sessions/Player/ReplayAlignment.h"

#  include <algorithm>
#  include <QPair>

#  include "Core/SSAssert.h"

/**
 * @brief Rebuilds the unique id -> column index lookup from the current column order.
 */
void Sessions::ReplayAlignment::indexColumns(ReplayLayout& layout)
{
  layout.uidToColumn.clear();
  for (int i = 0; i < static_cast<int>(layout.columnUniqueIds.size()); ++i)
    layout.uidToColumn.insert(layout.columnUniqueIds[static_cast<size_t>(i)], i);
}

/**
 * @brief Reorders the recording's columns to match the parsing order of the loaded project:
 *        grouped by source id, then by dataset index inside each source. Columns whose dataset
 *        is not in the project keep their relative order and follow the aligned ones, so a
 *        recording made against an older project still replays every column it stored.
 */
void Sessions::ReplayAlignment::alignColumnsToProject(ReplayLayout& layout,
                                                      const DatasetLocationMap& locations)
{
  if (layout.columnUniqueIds.empty())
    return;

  QMap<int, std::vector<QPair<int, int>>> bySource;
  std::vector<int> orphans;
  for (int uid : layout.columnUniqueIds) {
    const auto it = locations.constFind(uid);
    if (it == locations.constEnd()) {
      orphans.push_back(uid);
      continue;
    }

    bySource[it.value().sourceId].push_back(qMakePair(it.value().index, uid));
  }

  for (auto it = bySource.begin(); it != bySource.end(); ++it)
    std::sort(it.value().begin(), it.value().end(), [](const auto& a, const auto& b) {
      return a.first < b.first;
    });

  std::vector<int> aligned;
  aligned.reserve(layout.columnUniqueIds.size());
  for (auto it = bySource.constBegin(); it != bySource.constEnd(); ++it)
    for (const auto& pair : it.value())
      aligned.push_back(pair.second);

  for (int uid : orphans)
    aligned.push_back(uid);

  SS_ASSERT_LOG(aligned.size() == layout.columnUniqueIds.size());
  layout.columnUniqueIds.swap(aligned);
  indexColumns(layout);
}

/**
 * @brief Builds the per-source column lists and the FrameBuilder replay map
 *        (uid -> payload cell index); runs for any source count. Single-source payloads travel
 *        through processPayload, which routes to source 0, so the map is rekeyed to 0.
 */
Sessions::ReplayColumnMap Sessions::ReplayAlignment::buildMultiSourceMapping(
  ReplayLayout& layout, const DatasetLocationMap& locations)
{
  layout.columnToSource.clear();
  layout.sourceColumns.clear();

  ReplayColumnMap replay;
  for (int col = 0; col < static_cast<int>(layout.columnUniqueIds.size()); ++col) {
    const int uid    = layout.columnUniqueIds[static_cast<size_t>(col)];
    const auto srcIt = locations.constFind(uid);
    if (srcIt == locations.constEnd())
      continue;

    const int srcId    = srcIt.value().sourceId;
    auto& columns      = layout.sourceColumns[srcId];
    replay[srcId][uid] = static_cast<int>(columns.size());
    columns.push_back(uid);

    layout.columnToSource[col] = srcId;
  }

  layout.multiSource = layout.sourceColumns.size() > 1;

  if (!layout.multiSource && !replay.empty() && replay.begin()->first != 0) {
    auto columns = std::move(replay.begin()->second);
    replay.clear();
    replay[0] = std::move(columns);
  }

  return replay;
}

/**
 * @brief Folds each block's start time into the playback timeline, so a session whose data is
 *        entirely stream-lane still advances: the player steps over block starts (block rate),
 *        never over individual samples, which is what keeps the index bounded.
 */
void Sessions::ReplayAlignment::mergeStreamBlockTimes(
  std::vector<qint64>& timeline, const std::vector<PlayerStreamBlockIndex>& blocks)
{
  if (blocks.empty())
    return;

  timeline.reserve(timeline.size() + blocks.size());
  for (const auto& entry : blocks)
    timeline.push_back(entry.t0Ns);

  std::sort(timeline.begin(), timeline.end());
  timeline.erase(std::unique(timeline.begin(), timeline.end()), timeline.end());
}

#endif
