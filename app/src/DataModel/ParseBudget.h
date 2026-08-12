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

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <map>
#include <vector>

#include "SSAssert.h"

namespace DataModel {

/**
 * @brief Fair-share parse-load governor (spec 0051): parse time is charged per source into EWMA
 *        duty estimates (leaky integrator, tau 250 ms); only sources above their fair share
 *        decimate while the total exceeds the capacity threshold. Header-only, Qt-free,
 *        caller-supplied time points, single owner thread like every hotpath structure.
 */
class ParseBudget {
public:
  using Clock = std::chrono::steady_clock;

  /**
   * @brief One source's reading for the 1 Hz diagnostics pull: EWMA duty as a fraction of one
   *        core, the active keep-every-Nth factor, and total frames thinned away.
   */
  struct Load {
    int sourceId;
    int decimateN;
    double duty;
    std::uint64_t skippedFrames;
  };

  static constexpr std::int64_t kTauNs    = 250'000'000;
  static constexpr double kTotalDutyMax   = 0.90;
  static constexpr double kTotalDutyClear = 0.70;
  static constexpr double kActiveDutyMin  = 0.01;
  static constexpr int kMaxDecimation     = 1024;

  ParseBudget();

  void reset() noexcept;
  void maintain(Clock::time_point now);
  [[nodiscard]] bool thinning() const noexcept;
  [[nodiscard]] bool skipFrame(int sourceId) noexcept;
  [[nodiscard]] std::vector<Load> snapshot() const;
  [[nodiscard]] bool account(int sourceId, Clock::time_point startedAt, Clock::time_point now);

private:
  /**
   * @brief Per-source state: EWMA duty, decimation factor, thinning counters, and the decay
   *        anchor. Allocates once on a source's first accounted frame; steady state is lookup.
   */
  struct Entry {
    double duty                 = 0.0;
    int decimateN               = 1;
    std::uint64_t frameCounter  = 0;
    std::uint64_t skippedFrames = 0;
    Clock::time_point decayAt   = Clock::time_point{};
  };

  static void decay(double& duty, Clock::time_point& decayAt, Clock::time_point now) noexcept;
  void rebalance(Entry& entry) noexcept;

  bool m_thinning;
  double m_totalDuty;
  Clock::time_point m_totalDecayAt;
  std::map<int, Entry> m_entries;
};

/**
 * @brief Constructs an idle governor: no tracked sources, zero total duty, thinning off.
 */
inline ParseBudget::ParseBudget()
  : m_thinning(false), m_totalDuty(0.0), m_totalDecayAt(Clock::time_point{})
{}

/**
 * @brief Clears every tracked source and the total estimate -- called on project/connection edges.
 */
inline void ParseBudget::reset() noexcept
{
  m_entries.clear();
  m_totalDuty    = 0.0;
  m_totalDecayAt = Clock::time_point{};
  m_thinning     = false;
}

/**
 * @brief Returns whether any source is currently decimated (polled, never signalled).
 */
inline bool ParseBudget::thinning() const noexcept
{
  return m_thinning;
}

/**
 * @brief Off-frame-path maintenance sweep (called from the 1 Hz tick): decays every estimate
 *        against @p now and rebalances each entry, so a source that goes silent while decimated
 *        recovers within about a second instead of latching its stale factor and the thinning
 *        badge forever. Bounds the wrongly-thinned window when a decimated source resumes.
 */
inline void ParseBudget::maintain(Clock::time_point now)
{
  if (m_entries.empty())
    return;

  decay(m_totalDuty, m_totalDecayAt, now);
  for (auto& [sourceId, entry] : m_entries)
    decay(entry.duty, entry.decayAt, now);

  bool any = false;
  for (auto& [sourceId, entry] : m_entries) {
    rebalance(entry);
    any = any || entry.decimateN > 1;
  }

  m_thinning = any;
}

/**
 * @brief Applies the leaky-integrator decay to a duty estimate. The linear branch covers the
 *        at-rate case (dt << tau) so no transcendental runs per frame; exp() only fires after
 *        idle gaps, which are off the hot loop by definition.
 */
inline void ParseBudget::decay(double& duty,
                               Clock::time_point& decayAt,
                               Clock::time_point now) noexcept
{
  if (decayAt == Clock::time_point{}) [[unlikely]] {
    decayAt = now;
    return;
  }

  const auto dt_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - decayAt).count();
  if (dt_ns <= 0)
    return;

  const double x  = static_cast<double>(dt_ns) / static_cast<double>(kTauNs);
  duty           *= (x < 0.125) ? (1.0 - x) : std::exp(-x);
  decayAt         = now;
}

/**
 * @brief Returns true when @p sourceId's decimation factor says this frame is thinned. Sources at
 *        N=1 (the steady state, and every source while total load is under the threshold) pass
 *        unconditionally; only the offender pays, and its skipped work is never accounted.
 */
inline bool ParseBudget::skipFrame(int sourceId) noexcept
{
  SS_ASSERT(sourceId >= 0, return false);

  const auto it = m_entries.find(sourceId);
  if (it == m_entries.end())
    return false;

  Entry& entry = it->second;
  if (entry.decimateN <= 1) [[likely]]
    return false;

  ++entry.frameCounter;
  if ((entry.frameCounter % static_cast<std::uint64_t>(entry.decimateN)) == 0)
    return false;

  ++entry.skippedFrames;
  return true;
}

/**
 * @brief Charges @p sourceId with the elapsed parse time (scaled by the current decimation
 *        factor so the estimate tracks the OFFERED load; an unscaled estimate halves on
 *        engagement and turns the governor into a relaxation oscillator) and rebalances the
 *        source. Returns true exactly when this call flipped the governor into thinning.
 */
inline bool ParseBudget::account(int sourceId, Clock::time_point startedAt, Clock::time_point now)
{
  SS_ASSERT(sourceId >= 0, return false);
  SS_ASSERT(now >= startedAt, now = startedAt);

  const auto elapsed_ns =
    std::chrono::duration_cast<std::chrono::nanoseconds>(now - startedAt).count();
  const double charge = static_cast<double>(elapsed_ns) / static_cast<double>(kTauNs);

  Entry& entry        = m_entries[sourceId];
  const double weight = static_cast<double>(entry.decimateN);
  decay(entry.duty, entry.decayAt, now);
  entry.duty += charge * weight;

  decay(m_totalDuty, m_totalDecayAt, now);
  m_totalDuty += charge * weight;

  rebalance(entry);

  const bool engaged = entry.decimateN > 1 && !m_thinning;
  if (engaged) [[unlikely]]
    m_thinning = true;

  return engaged;
}

/**
 * @brief Recomputes one source's decimation factor from the fair-share rule: nobody is thinned
 *        under the engage threshold (hysteresis: a latched governor holds until the lower clear
 *        threshold); past it, only sources above their fair share decimate, proportionally to
 *        the overrun. Clears the thinning latch once the last offender recovers.
 */
inline void ParseBudget::rebalance(Entry& entry) noexcept
{
  const double limit = m_thinning ? kTotalDutyClear : kTotalDutyMax;
  if (m_totalDuty <= limit) {
    entry.decimateN = 1;
    if (m_thinning) [[unlikely]] {
      bool any = false;
      for (const auto& [id, other] : m_entries)
        any = any || other.decimateN > 1;

      m_thinning = any;
    }

    return;
  }

  int active = 0;
  for (const auto& [id, other] : m_entries)
    if (other.duty > kActiveDutyMin)
      ++active;

  const double fair_share = kTotalDutyMax / static_cast<double>(std::max(1, active));
  if (entry.duty <= fair_share) {
    entry.decimateN = 1;
    return;
  }

  const int n     = static_cast<int>(std::ceil(entry.duty / fair_share));
  entry.decimateN = std::clamp(n, 1, kMaxDecimation);
}

/**
 * @brief Snapshots every tracked source for the 1 Hz diagnostics pull. Cold path: allocation here
 *        is off the frame path by design.
 */
inline std::vector<ParseBudget::Load> ParseBudget::snapshot() const
{
  std::vector<Load> loads;
  loads.reserve(m_entries.size());
  for (const auto& [sourceId, entry] : m_entries)
    loads.push_back({sourceId, entry.decimateN, entry.duty, entry.skippedFrames});

  return loads;
}

}  // namespace DataModel
