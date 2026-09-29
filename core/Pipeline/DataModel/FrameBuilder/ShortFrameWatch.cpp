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

#include "DataModel/FrameBuilder/ShortFrameWatch.h"

#include <algorithm>

#include "Core/DataModel/Frame.h"
#include "Core/SSAssert.h"

/**
 * @brief Builds an empty watch with every direct-map slot vacant.
 */
DataModel::ShortFrameWatch::ShortFrameWatch() : m_entries{}, m_directMap{}, m_count(0)
{
  m_directMap.fill(-1);
}

/**
 * @brief Rebuilds the watch from the enabled project groups: highest non-computed frame index
 *        per source, watermark zeroed, count published LAST with a release store paired with
 *        stats()'s acquire load, so a concurrent 1 Hz pull never reads entries a rebuild has
 *        not filled yet.
 */
void DataModel::ShortFrameWatch::rebuild(const Frame& frame)
{
  m_count.store(0, std::memory_order_release);
  m_directMap.fill(-1);

  int count = 0;
  for (const auto& group : frame.groups) {
    SS_ASSERT_LOG(group.sourceId >= 0);

    Stat* entry = nullptr;
    for (int i = 0; i < count; ++i)
      if (m_entries[static_cast<std::size_t>(i)].sourceId == group.sourceId) {
        entry = &m_entries[static_cast<std::size_t>(i)];
        break;
      }

    if (!entry) {
      if (count >= kCap) [[unlikely]]
        break;

      entry  = &m_entries[static_cast<std::size_t>(count)];
      *entry = Stat{group.sourceId, 0, 0};
      if (static_cast<unsigned>(group.sourceId) < static_cast<unsigned>(kCap))
        m_directMap[static_cast<std::size_t>(group.sourceId)] = static_cast<qint8>(count);

      ++count;
    }

    for (const auto& dataset : group.datasets)
      if (!dataset.virtual_)
        entry->maxIndex = std::max(entry->maxIndex, dataset.index);
  }

  SS_ASSERT(count <= kCap, count = kCap);
  m_count.store(count, std::memory_order_release);
}

/**
 * @brief Zeroes every entry's delivered-token watermark (connect edge, mode change): the
 *        watermark is session-monotonic, so the 1 Hz pull needs no cross-thread window reset.
 */
void DataModel::ShortFrameWatch::resetWatermarks() noexcept
{
  const int count = std::clamp(m_count.load(std::memory_order_relaxed), 0, kCap);
  for (int i = 0; i < count; ++i)
    m_entries[static_cast<std::size_t>(i)].watermark = 0;
}

/**
 * @brief Records the token count a frame delivered for @p sourceId: one direct-map load and a
 *        conditional store for any interleaving of sources (a negative or oversized id takes
 *        the bounded fallback scan) -- the frame path never allocates, locks or signals here,
 *        and the 1 Hz link check pulls the watermark (spec 0033).
 */
void DataModel::ShortFrameWatch::note(int sourceId, qsizetype count) noexcept
{
  SS_ASSERT_HOTPATH(sourceId >= 0);
  SS_ASSERT_HOTPATH(count >= 0);

  Stat* watch = nullptr;
  if (static_cast<unsigned>(sourceId) < static_cast<unsigned>(kCap)) [[likely]] {
    const qint8 slot = m_directMap[static_cast<std::size_t>(sourceId)];
    if (slot < 0)
      return;

    watch = &m_entries[static_cast<std::size_t>(slot)];
  } else {
    const int total = m_count.load(std::memory_order_relaxed);
    for (int i = 0; i < total && i < kCap; ++i)
      if (m_entries[static_cast<std::size_t>(i)].sourceId == sourceId) {
        watch = &m_entries[static_cast<std::size_t>(i)];
        break;
      }

    if (!watch)
      return;
  }

  if (static_cast<int>(count) > watch->watermark)
    watch->watermark = static_cast<int>(count);
}

/**
 * @brief Snapshot for the 1 Hz link check: plain torn-benign int reads of single-writer fixed
 *        storage (the established pulled-counter pattern, spec 0033).
 */
std::vector<DataModel::ShortFrameWatch::Stat> DataModel::ShortFrameWatch::stats() const
{
  const int count = std::clamp(m_count.load(std::memory_order_acquire), 0, kCap);

  std::vector<Stat> stats;
  stats.reserve(static_cast<std::size_t>(count));
  for (int i = 0; i < count; ++i)
    stats.push_back(m_entries[static_cast<std::size_t>(i)]);

  return stats;
}
