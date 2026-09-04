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

#include <QString>
#include <vector>

namespace Widgets::WaterfallTicks {

/**
 * @brief Result of axis tick generation -- sampled values + step + display max.
 */
struct AxisTicks {
  std::vector<double> values;
  double step;
  double displayMax;
};

[[nodiscard]] AxisTicks computeFreqTicks(double maxFreq, int targetCount);
[[nodiscard]] AxisTicks computeTimeTicks(double maxSeconds, int targetCount);
[[nodiscard]] QString formatFreqTick(double hz);
[[nodiscard]] QString formatTimeTick(double seconds, double step);

}  // namespace Widgets::WaterfallTicks
