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
#include <cmath>

#include "UI/Widgets/PlotAutoScale.h"

/**
 * @brief Per-curve Y ranges for the multi-plot's lane mode (spec 0127 R23): each visible curve
 *        gets its own band, so each needs its own range. The extents come out of the scan the
 *        shared axis already runs; nothing here walks curve data.
 */
namespace Widgets::LaneRanges {

/**
 * @brief Fraction of the half extent added on each side of a scanned range.
 */
inline constexpr double kPadding = 0.1;

/**
 * @brief One lane's Y range and the auto-scale ladder step it was last quantized on.
 */
struct Lane {
  double min    = 0.0;
  double max    = 1.0;
  int stepIndex = AutoScale::kNoStep;
};

/**
 * @brief Stores a range the dataset declares, in either order. Returns whether the lane moved.
 */
[[nodiscard]] inline bool assignDeclared(Lane& lane, const double a, const double b) noexcept
{
  const double lo = std::min(a, b);
  const double hi = std::max(a, b);
  if (!std::isfinite(lo) || !std::isfinite(hi) || !(hi > lo))
    return false;

  const bool changed = lo != lane.min || hi != lane.max;
  lane.min           = lo;
  lane.max           = hi;
  lane.stepIndex     = AutoScale::kNoStep;
  return changed;
}

/**
 * @brief Folds one curve's scanned extent into its lane: padded, then snapped onto the auto-scale
 *        ladder with its hysteresis, so a noisy signal does not move its band every tick. An
 *        empty scan (the lo > hi sentinel of a hidden or not yet filled curve) holds the range
 *        the lane had. Returns whether the lane moved.
 */
[[nodiscard]] inline bool assignScanned(Lane& lane, double lo, double hi) noexcept
{
  if (!std::isfinite(lo) || !std::isfinite(hi) || lo > hi)
    return false;

  if (hi == lo) {
    const double margin  = (lo == 0.0) ? 1.0 : std::abs(lo) * kPadding;
    lo                  -= margin;
    hi                  += margin;
  } else {
    const double margin  = (hi - lo) * 0.5 * kPadding;
    lo                  -= margin;
    hi                  += margin;
  }

  (void)AutoScale::quantizeRange(lo, hi, lane.stepIndex);
  if (!(hi > lo))
    return false;

  const bool changed = lo != lane.min || hi != lane.max;
  lane.min           = lo;
  lane.max           = hi;
  return changed;
}

}  // namespace Widgets::LaneRanges
