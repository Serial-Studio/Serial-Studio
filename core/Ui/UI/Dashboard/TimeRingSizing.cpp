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

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kMinRingSlots        = 1024;
constexpr int kMaxFrameLaneSlots   = 262144;
constexpr double kAssumedMaxRateHz = 1024000.0;
constexpr double kHistoryHeadroom  = 1.25;
constexpr double kRingGrowthFactor = 1.5;

//--------------------------------------------------------------------------------------------------
// Sizing policies
//--------------------------------------------------------------------------------------------------

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
 * @brief Frame-lane capacity: the window at the assumed maximum device rate, capped. The frame
 *        lane cannot know its rate at layout time, so it sizes for the fastest and grows later.
 */
DSP::RingCapacity UI::TimeRingSizing::frameLaneCapacity(const double windowSec) noexcept
{
  SS_ASSERT(windowSec > 0.0, return DSP::RingCapacity::fromRate(1.0, kMinRingSlots));
  return DSP::RingCapacity::fromRate(windowSec, kAssumedMaxRateHz)
    .atMost(kMaxFrameLaneSlots)
    .atLeast(kMinRingSlots);
}

/**
 * @brief Stream-lane capacity: the window at the source's real sample rate, bounded by the shared
 *        ceiling, so a dense source keeps one cell per sample until the byte budget binds.
 */
DSP::RingCapacity UI::TimeRingSizing::streamLaneCapacity(const double windowSec,
                                                         const double rateHz) noexcept
{
  SS_ASSERT(windowSec > 0.0, return frameLaneCapacity(1.0));
  SS_ASSERT(rateHz > 0.0, return frameLaneCapacity(windowSec));
  return DSP::RingCapacity::fromRate(windowSec, rateHz).atLeast(kMinRingSlots);
}

/**
 * @brief Builds a scrolling-history ring for @p plotTimeRangeSec plus headroom: rate-sized for a
 *        stream source (@p rateHz > 0), frame-lane sized otherwise.
 */
DSP::EnvelopeRing UI::TimeRingSizing::historyRing(const double plotTimeRangeSec,
                                                  const double rateHz)
{
  const double window = historyWindowSec(plotTimeRangeSec);
  if (rateHz > 0.0)
    return DSP::EnvelopeRing(streamLaneCapacity(window, rateHz), window);

  return DSP::EnvelopeRing(frameLaneCapacity(window), window);
}

/**
 * @brief The capacity a saturated ring should grow to once its source's measured cadence is known,
 *        or nothing when growth would be under the 1.5x hysteresis. Upward only: shrinking would
 *        throw history away over a momentary lull.
 */
std::optional<DSP::RingCapacity> UI::TimeRingSizing::grownCapacity(
  const int currentSlots, const double windowSec, const double samplePeriodSec) noexcept
{
  SS_ASSERT(currentSlots > 0, return std::nullopt);
  if (!(samplePeriodSec > 0.0) || !(windowSec > 0.0))
    return std::nullopt;

  const auto desired =
    DSP::RingCapacity::fromRate(windowSec, 1.0 / samplePeriodSec).atLeast(kMinRingSlots);
  if (desired.value() < static_cast<double>(currentSlots) * kRingGrowthFactor)
    return std::nullopt;

  return desired;
}
