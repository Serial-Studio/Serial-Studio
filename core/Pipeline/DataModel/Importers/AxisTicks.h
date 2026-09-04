/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <algorithm>
#include <cmath>

namespace DataModel {

/**
 * @brief Snapped widget bounds plus a friendly major-tick count for analog widgets.
 */
struct AxisTicks {
  double min;
  double max;
  int tickCount;
};

/**
 * @brief Rounds raw bounds outward to a {2, 5, 10, 20}*10^n step and picks a tick count.
 */
[[nodiscard]] inline AxisTicks niceAxisTicks(double rawMin, double rawMax)
{
  AxisTicks out{rawMin, rawMax, 5};
  if (!(rawMax > rawMin))
    return out;

  const double range = rawMax - rawMin;
  // code-verify off
  const double mag   = std::pow(10.0, std::floor(std::log10(range)));
  // code-verify on
  const double norm  = range / mag;

  double step;
  if (norm < 1.5)
    step = 0.2 * mag;
  else if (norm < 3.0)
    step = 0.5 * mag;
  else if (norm < 6.0)
    step = 1.0 * mag;
  else
    step = 2.0 * mag;

  const double epsilon    = step * 1e-9;
  const double snappedMin = std::floor((rawMin + epsilon) / step) * step;
  const double snappedMax = std::ceil((rawMax - epsilon) / step) * step;

  const double finalMin = std::min(snappedMin, rawMin);
  const double finalMax = std::max(snappedMax, rawMax);

  int intervals = static_cast<int>(std::lround((finalMax - finalMin) / step));
  if (intervals < 2)
    intervals = 2;

  if (intervals > 10)
    intervals = 10;

  out.min       = finalMin;
  out.max       = finalMax;
  out.tickCount = intervals + 1;
  return out;
}

}  // namespace DataModel
