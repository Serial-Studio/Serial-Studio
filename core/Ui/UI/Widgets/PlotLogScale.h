/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

namespace Widgets::LogScale {

/**
 * @brief Positive clamp floor for log10-mapped values; mirrored as a literal 1e-12 in
 *        Plot.qml / MultiPlot.qml (trigger-level mapping) and AxisRangeDialog.qml --
 *        keep the four sites in sync when changing it.
 */
inline constexpr double kLogFloor = 1e-12;

/**
 * @brief Log floor for the Samples axis: sample #0 has no log position, so index rings
 *        clamp at sample 1 (log10(1) = 0 becomes the axis origin).
 */
inline constexpr double kSampleFloor = 1.0;

/**
 * @brief Maps a value into log10 axis space, clamping at a positive floor so zero and
 *        negative samples land on the floor instead of producing -inf/NaN.
 */
[[nodiscard]] inline double clampedLog10(const double value, const double floor = kLogFloor)
{
  return std::log10(std::max(value, floor));
}

/**
 * @brief Converts a configured [min, max] range (true units) into log10 axis bounds;
 *        returns false when the range is unusable on a log axis (max non-finite,
 *        <= 0, or collapsed below the floor) so the caller falls back to auto-scale.
 *        A non-positive min collapses to six decades under max.
 */
[[nodiscard]] inline bool resolveLogBounds(double& min, double& max)
{
  if (!std::isfinite(max) || !(max > 0.0))
    return false;

  const double lo = (min > 0.0) ? min : std::max(max * 1e-6, kLogFloor);
  if (!(lo < max))
    return false;

  min = clampedLog10(lo);
  max = std::log10(max);
  return true;
}

}  // namespace Widgets::LogScale
