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

#include "UI/Dashboard/TimeRingSizing.h"

#include <algorithm>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kMinRingSlots       = 1024;
constexpr double kHistoryHeadroom = 1.25;

//--------------------------------------------------------------------------------------------------
// Sizing policies
//--------------------------------------------------------------------------------------------------

/**
 * @brief Shared empty, exact ring for a lookup that misses, so it never reads as decimated.
 */
const DSP::EnvelopeRing& UI::TimeRingSizing::emptyRing()
{
  static const DSP::EnvelopeRing kEmpty(
    DSP::RingCapacity::fromRate(1.0, 1.0), 1.0, DSP::RingMode::Exact);
  return kEmpty;
}

/**
 * @brief The window a history ring covers: the visible range plus headroom, so a saturated
 *        min/max source (two slots per decimation cell) still spans the full axis.
 */
double UI::TimeRingSizing::historyWindowSec(const double plotTimeRangeSec) noexcept
{
  SS_ASSERT(plotTimeRangeSec > 0.0, return kHistoryHeadroom);
  return plotTimeRangeSec * kHistoryHeadroom;
}

/**
 * @brief Starting capacity (spec 0097): a stream source's real rate over the window plus the
 *        growth headroom, so it is exact from its first sample and never grows at its first wrap
 *        (a ring of exactly rate x window is one slot short of the window); a frame-lane source,
 *        whose rate is unknown at layout time, starts at the floor and grows on coverage.
 */
DSP::RingCapacity UI::TimeRingSizing::initialCapacity(const double windowSec,
                                                      const double rateHz) noexcept
{
  SS_ASSERT(windowSec > 0.0, return DSP::RingCapacity::fromRate(1.0, kMinRingSlots));
  SS_ASSERT_LOG(!(rateHz < 0.0));

  if (!(rateHz > 0.0))
    return DSP::RingCapacity::fromRate(1.0, kMinRingSlots);

  const double window = windowSec * DSP::EnvelopeRing::kGrowthHeadroom;
  return DSP::RingCapacity::fromRate(window, rateHz).atLeast(kMinRingSlots);
}

/**
 * @brief Starting mode: exact, unless a stream source's window already exceeds the per-ring
 *        ceiling, in which case it decimates from the start (and says so).
 */
DSP::RingMode UI::TimeRingSizing::initialMode(const double windowSec, const double rateHz) noexcept
{
  SS_ASSERT(windowSec > 0.0, return DSP::RingMode::Exact);

  const double ceiling = static_cast<double>(DSP::RingCapacity::kMaxSamples);
  if (rateHz > 0.0 && windowSec * rateHz > ceiling)
    return DSP::RingMode::Decimating;

  return DSP::RingMode::Exact;
}

/**
 * @brief Builds a scrolling-history ring for @p plotTimeRangeSec plus headroom: rate-sized and
 *        exact for a stream source (@p rateHz > 0), small and exact for the frame lane, within
 *        what @p budget grants (spec 0097 R7): a short grant starts it decimating at the
 *        affordable size, flagged as rail-limited.
 */
DSP::EnvelopeRing UI::TimeRingSizing::historyRing(const double plotTimeRangeSec,
                                                  const double rateHz,
                                                  DSP::RingBudget& budget)
{
  const double window = historyWindowSec(plotTimeRangeSec);
  auto capacity       = initialCapacity(window, rateHz);
  auto mode           = initialMode(window, rateHz);
  auto limit = mode == DSP::RingMode::Decimating ? DSP::RingLimit::Ceiling : DSP::RingLimit::None;

  const std::size_t slotBytes = DSP::EnvelopeRing::kSlotBytesBound;
  const std::size_t wanted    = static_cast<std::size_t>(capacity.value()) * slotBytes;
  const std::size_t room      = budget.grant(wanted);
  if (room < wanted) {
    const auto affordable = std::max<std::size_t>(room / slotBytes, kMinRingSlots);
    capacity              = capacity.atMost(static_cast<int>(affordable));
    mode                  = DSP::RingMode::Decimating;
    limit                 = DSP::RingLimit::Rail;
  }

  DSP::EnvelopeRing ring(capacity, window, mode);
  ring.limit = limit;
  budget.charge(0, ring.bytes());
  return ring;
}

/**
 * @brief Sweep capacity at the history ring's retained density (its capacity over the window
 *        plus the growth headroom it was sized with) over @p sweepWindowSec, so a sweep captures
 *        at the resolution the plot already keeps.
 */
DSP::RingCapacity UI::TimeRingSizing::sweepCapacity(const DSP::EnvelopeRing& history,
                                                    const double sweepWindowSec) noexcept
{
  SS_ASSERT(sweepWindowSec > 0.0, return DSP::RingCapacity::fromRate(1.0, kMinRingSlots));
  SS_ASSERT(history.windowSec > 0.0, return DSP::RingCapacity::fromRate(1.0, kMinRingSlots));

  const double span    = history.windowSec * DSP::EnvelopeRing::kGrowthHeadroom;
  const double density = static_cast<double>(history.level0.time.capacity()) / span;
  return DSP::RingCapacity::fromRate(sweepWindowSec, density).atLeast(kMinRingSlots);
}

/**
 * @brief Re-shapes and allocates @p engine for @p curves at the density of @p densest over
 *        @p sweepWindowSec, within what @p budget grants (spec 0097 R7: the sweep's front and
 *        back rings count against the plot-memory rail, so a short grant coarsens the capture
 *        instead of overrunning it). The trigger configuration and the enabled flag stay.
 */
void UI::TimeRingSizing::shapeSweep(DSP::SweepEngine& engine,
                                    const int curves,
                                    const DSP::EnvelopeRing& densest,
                                    const double sweepWindowSec,
                                    DSP::RingBudget& budget)
{
  SS_ASSERT(curves > 0, return);
  SS_ASSERT(sweepWindowSec > 0.0, return);

  budget.charge(engine.bytes(), 0);
  engine.release();

  const auto wanted         = sweepCapacity(densest, sweepWindowSec);
  const std::size_t perSlot = static_cast<std::size_t>(curves) * 2 * 2 * sizeof(double);
  const std::size_t room    = budget.grant(static_cast<std::size_t>(wanted.value()) * perSlot);
  const auto affordable     = std::max<std::size_t>(room / perSlot, kMinRingSlots);
  const auto capacity       = wanted.atMost(static_cast<int>(
    std::min<std::size_t>(affordable, static_cast<std::size_t>(DSP::RingCapacity::kMaxSamples))));

  engine.setShape(curves, capacity, sweepWindowSec);
  engine.allocate();
  budget.charge(0, engine.bytes());
}

/**
 * @brief Grows a running sweep in place to the density of @p densest after that history ring
 *        grew, within what @p budget grants (spec 0097 R4: a sweep that is on behaves as before,
 *        so its capture is never reset). A no-op when it already captures at that density.
 */
void UI::TimeRingSizing::growSweep(DSP::SweepEngine& engine,
                                   const DSP::EnvelopeRing& densest,
                                   const double sweepWindowSec,
                                   DSP::RingBudget& budget)
{
  SS_ASSERT(sweepWindowSec > 0.0, return);
  SS_ASSERT_LOG(engine.shapeCurves >= 0);

  const auto wanted = sweepCapacity(densest, sweepWindowSec);
  const int current = engine.shapeCapacity.value();
  if (!engine.allocated() || wanted.value() <= current)
    return;

  const auto curves         = static_cast<std::size_t>(engine.shapeCurves);
  const std::size_t perSlot = curves * 2 * 2 * sizeof(double);
  const auto extra          = static_cast<std::size_t>(wanted.value() - current);
  const std::size_t room    = budget.grant(extra * perSlot);
  const auto capacity       = wanted.atMost(current + static_cast<int>(room / perSlot));
  const std::size_t before  = engine.bytes();

  engine.growCapacity(capacity);
  budget.charge(before, engine.bytes());
}
