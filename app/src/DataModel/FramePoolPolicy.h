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

#include <cstddef>
#include <QHash>
#include <QtGlobal>
#include <utility>
#include <vector>

namespace DataModel {

/**
 * @brief Which slot the policy handed back, and therefore what the caller is about to pay for.
 *        Virgin is the expensive one: an untouched slot forces a full Frame deep copy that the
 *        pool then holds for the rest of the session.
 */
enum class SlotPick {
  HintReuse,   ///< The source's own slot, still warm: values-only refresh
  WarmWalk,    ///< Another slot this source already owns: structural rebind
  VirginWalk,  ///< Never-written slot: full deep copy, permanently materialised
  Steal,       ///< Slot taken from another source: full deep copy, already materialised
  None         ///< Nothing available; the caller falls back to the heap or skips
};

/**
 * @brief Slot ownership, per-source affinity and the materialisation budget for FrameBuilder's
 *        frame pool, split out so the selection rules are unit testable. Prefers a warm slot
 *        over a virgin one, and stops materialising once the footprint passes the budget.
 */
class FramePoolPolicy {
public:
  /**
   * @brief Pulled counters, in the pattern spec 0033 fixes for diagnostics: plain increments the
   *        caller reads on a timer, never signalled and never allocated on the frame path.
   */
  struct Stats {
    quint64 hintReuse;
    quint64 warmWalk;
    quint64 virginWalk;
    quint64 steal;
    quint64 fail;
  };

  static constexpr std::size_t kInvalidSlot = static_cast<std::size_t>(-1);
  static constexpr int kUnowned             = -1;

  explicit FramePoolPolicy(std::size_t capacity);

  [[nodiscard]] std::size_t capacity() const noexcept;
  [[nodiscard]] std::size_t materialised() const noexcept;
  [[nodiscard]] std::size_t slotBudget() const noexcept;
  [[nodiscard]] Stats stats() const noexcept;

  void releaseOwnership() noexcept;
  void setSlotBudget(std::size_t maxSlots) noexcept;
  void applyMemoryBudget(std::size_t frameBytes, std::size_t budgetBytes) noexcept;

  /**
   * @brief Picks a slot for @p sourceId. @p isFree reports whether a slot is unreferenced (the
   *        caller's use_count()==1 test). With @p hintedOnly the source's own slot is the only
   *        acceptable answer, which is what a synthetic refresh wants: it carries no new data,
   *        so skipping the tick is free where materialising a slot is not.
   */
  template<typename FreeFn>
  [[nodiscard]] std::pair<std::size_t, SlotPick> claim(int sourceId, bool hintedOnly, FreeFn isFree)
  {
    const auto hinted = m_hintBySource.constFind(sourceId);
    if (hinted != m_hintBySource.cend()) {
      const std::size_t idx = hinted.value();
      if (isFree(idx) && m_owner[idx] == sourceId) {
        ++m_stats.hintReuse;
        return {idx, SlotPick::HintReuse};
      }
    }

    if (hintedOnly) {
      ++m_stats.fail;
      return {kInvalidSlot, SlotPick::None};
    }

    const std::size_t n    = m_owner.size();
    std::size_t virginSlot = kInvalidSlot;
    std::size_t stealable  = kInvalidSlot;

    for (std::size_t k = 0; k < n; ++k) {
      const std::size_t idx = (m_walkHint + k) % n;
      if (!isFree(idx))
        continue;

      const int owner = m_owner[idx];
      if (owner == sourceId) {
        ++m_stats.warmWalk;
        return {adopt(sourceId, idx), SlotPick::WarmWalk};
      }

      if (owner == kUnowned) {
        if (virginSlot == kInvalidSlot)
          virginSlot = idx;

        continue;
      }

      if (stealable == kInvalidSlot)
        stealable = idx;
    }

    const bool underBudget = m_materialised < m_slotBudget;
    if (virginSlot != kInvalidSlot && underBudget) {
      ++m_materialised;
      ++m_stats.virginWalk;
      return {adopt(sourceId, virginSlot), SlotPick::VirginWalk};
    }

    if (stealable != kInvalidSlot) {
      ++m_stats.steal;
      return {adopt(sourceId, stealable), SlotPick::Steal};
    }

    if (virginSlot != kInvalidSlot) {
      ++m_materialised;
      ++m_stats.virginWalk;
      return {adopt(sourceId, virginSlot), SlotPick::VirginWalk};
    }

    ++m_stats.fail;
    return {kInvalidSlot, SlotPick::None};
  }

private:
  [[nodiscard]] std::size_t adopt(int sourceId, std::size_t idx) noexcept;

private:
  std::vector<int> m_owner;
  QHash<int, std::size_t> m_hintBySource;
  std::size_t m_walkHint;
  std::size_t m_materialised;
  std::size_t m_slotBudget;
  Stats m_stats;
};

}  // namespace DataModel
