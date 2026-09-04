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
#include <limits>

#include "Core/SSAssert.h"

/**
 * @brief Quantized axis auto-scaling (spec 0058): a data-derived range snaps onto a 1-2-5-per-
 *        decade ladder indexed by an integer, sized so the extent fits kDivisions divisions,
 *        with hysteresis so a signal hovering at a ladder boundary cannot flap the axis.
 */
namespace Widgets::AutoScale {

/**
 * @brief Divisions the data extent must fit into; the axis spans that many ladder steps.
 */
inline constexpr int kDivisions = 8;

/**
 * @brief Fraction of the next finer step's full span the extent must fit under before the
 *        ladder is allowed to shrink (grow is immediate).
 */
inline constexpr double kShrinkMargin = 0.8;

/**
 * @brief Sentinel for "no step chosen yet" (first quantization takes the required step).
 */
inline constexpr int kNoStep = std::numeric_limits<int>::min();

/**
 * @brief Ladder index bounds: three steps per decade, +/- 20 decades around 1.0.
 */
inline constexpr int kMinStepIndex = -60;
inline constexpr int kMaxStepIndex = 60;

/**
 * @brief Value-per-division of ladder index @p index: mantissa {1, 2, 5}[index mod 3] times
 *        10^(index div 3), floor division so negative indices descend through 0.5, 0.2, 0.1.
 */
[[nodiscard]] inline double ladderStep(int index) noexcept
{
  constexpr double kMantissa[3] = {1.0, 2.0, 5.0};
  const int clamped             = std::clamp(index, kMinStepIndex, kMaxStepIndex);
  const int decade              = (clamped >= 0) ? clamped / 3 : -((-clamped + 2) / 3);
  const int mantissa            = clamped - decade * 3;

  double scale = 1.0;
  for (int i = 0; i < std::abs(decade); ++i)
    scale *= 10.0;

  return decade < 0 ? kMantissa[mantissa] / scale : kMantissa[mantissa] * scale;
}

/**
 * @brief Smallest ladder index whose step is at least @p perDivision (0 for degenerate input).
 */
[[nodiscard]] inline int ladderIndexFor(double perDivision) noexcept
{
  if (!std::isfinite(perDivision) || !(perDivision > 0.0))
    return 0;

  const double decade = std::floor(std::log10(perDivision));
  int index           = static_cast<int>(decade) * 3;
  for (int i = 0; i < 4; ++i, ++index)
    if (ladderStep(index) >= perDivision * (1.0 - 1e-12))
      return std::clamp(index, kMinStepIndex, kMaxStepIndex);

  return std::clamp(index, kMinStepIndex, kMaxStepIndex);
}

/**
 * @brief Snaps [min, max] outward onto multiples of the ladder step that fits the extent in
 *        kDivisions, updating @p stepIndex with hysteresis (grow at once, shrink only under
 *        kShrinkMargin of the next finer span). Zero is always a step multiple, so a bipolar
 *        range splits on a division boundary. False (range untouched) on non-finite/empty input.
 */
[[nodiscard]] inline bool quantizeRange(double& min, double& max, int& stepIndex) noexcept
{
  if (!std::isfinite(min) || !std::isfinite(max) || !(max > min))
    return false;

  const double extent = max - min;
  const int required  = ladderIndexFor(extent / static_cast<double>(kDivisions));

  int chosen = required;
  if (stepIndex != kNoStep && required < stepIndex) {
    const double finerSpan = ladderStep(stepIndex - 1) * static_cast<double>(kDivisions);
    chosen                 = (extent <= finerSpan * kShrinkMargin) ? required : stepIndex;
  }

  const double step = ladderStep(chosen);
  SS_ASSERT(step > 0.0, return false);

  const double lo = std::floor(min / step) * step;
  const double hi = std::ceil(max / step) * step;
  SS_ASSERT(hi > lo, return false);

  min       = lo;
  max       = hi;
  stepIndex = chosen;
  return true;
}

}  // namespace Widgets::AutoScale
