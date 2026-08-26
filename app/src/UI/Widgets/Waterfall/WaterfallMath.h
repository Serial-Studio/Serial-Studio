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

#include <cmath>

namespace UI::Widgets::WaterfallDetail {
/**
 * @brief Integer-exponent 10^n via table lookup; falls back to std::pow if out of band.
 *        `static inline` so each including TU keeps its own internal-linkage copy, exactly
 *        as it had when this lived at file scope in Waterfall.cpp.
 */
static inline double waterfallFastPow10(double exponent) noexcept
{
  static constexpr double kTable[] = {
    1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5,
    1e-4,  1e-3,  1e-2,  1e-1,  1e0,   1e1,   1e2,  1e3,  1e4,  1e5,  1e6,
    1e7,   1e8,   1e9,   1e10,  1e11,  1e12,  1e13, 1e14, 1e15,
  };
  const int idx = static_cast<int>(exponent) + 15;
  if (idx < 0 || idx >= static_cast<int>(sizeof(kTable) / sizeof(kTable[0]))) [[unlikely]]
    return std::pow(10.0, exponent);

  return kTable[idx];
}
}  // namespace UI::Widgets::WaterfallDetail
