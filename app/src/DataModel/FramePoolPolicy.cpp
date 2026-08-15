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

#include "DataModel/FramePoolPolicy.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the policy over @p capacity slots, all unowned and none materialised. The budget
 *        starts at the full capacity so a caller that never states a frame size behaves exactly
 *        as the pool did before the budget existed.
 */
DataModel::FramePoolPolicy::FramePoolPolicy(std::size_t capacity)
  : m_owner(capacity < 1 ? 1 : capacity, kUnowned)
  , m_walkHint(0)
  , m_materialised(0)
  , m_slotBudget(capacity < 1 ? 1 : capacity)
  , m_stats{0, 0, 0, 0, 0}
{}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the number of slots the policy manages.
 */
std::size_t DataModel::FramePoolPolicy::capacity() const noexcept
{
  return m_owner.size();
}

/**
 * @brief Returns how many slots have ever been written, which is what the pool actually costs:
 *        an untouched slot holds no frame and no memory.
 */
std::size_t DataModel::FramePoolPolicy::materialised() const noexcept
{
  return m_materialised;
}

/**
 * @brief Returns the current ceiling on materialised slots.
 */
std::size_t DataModel::FramePoolPolicy::slotBudget() const noexcept
{
  return m_slotBudget;
}

/**
 * @brief Returns the pulled counters.
 */
DataModel::FramePoolPolicy::Stats DataModel::FramePoolPolicy::stats() const noexcept
{
  return m_stats;
}

//--------------------------------------------------------------------------------------------------
// Mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drops every ownership claim and affinity hint, the structural-change path. The
 *        materialised count deliberately survives: those slots still hold their frames, so the
 *        memory they cost is still spent.
 */
void DataModel::FramePoolPolicy::releaseOwnership() noexcept
{
  std::fill(m_owner.begin(), m_owner.end(), kUnowned);
  m_hintBySource.clear();
}

/**
 * @brief Sets the ceiling on materialised slots, clamped to [1, capacity].
 */
void DataModel::FramePoolPolicy::setSlotBudget(std::size_t maxSlots) noexcept
{
  m_slotBudget = std::clamp<std::size_t>(maxSlots, 1, m_owner.size());
}

/**
 * @brief Derives the slot ceiling from a frame's estimated footprint, so the pool is bounded by
 *        the memory it will actually occupy rather than by a slot count tuned for small frames.
 *        A project with 600+ datasets reaches the byte budget long before the slot count, which
 *        is the case the raw count never covered.
 */
void DataModel::FramePoolPolicy::applyMemoryBudget(std::size_t frameBytes,
                                                   std::size_t budgetBytes) noexcept
{
  if (frameBytes == 0 || budgetBytes == 0) {
    setSlotBudget(m_owner.size());
    return;
  }

  setSlotBudget(budgetBytes / frameBytes);
}

//--------------------------------------------------------------------------------------------------
// Internals
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records @p idx as owned by @p sourceId and makes it both the source's affinity hint and
 *        the next walk's starting point.
 */
std::size_t DataModel::FramePoolPolicy::adopt(int sourceId, std::size_t idx) noexcept
{
  m_owner[idx] = sourceId;
  m_hintBySource.insert(sourceId, idx);
  m_walkHint = idx;
  return idx;
}
