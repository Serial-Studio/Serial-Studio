/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
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

#include <QRgb>
#include <vector>

namespace Widgets::WaterfallColorMap {

/**
 * @brief Entries in a baked color map; one byte of normalized magnitude per entry.
 */
inline constexpr int kLutSize = 256;

/**
 * @brief Built-in color map identifiers, mirrored by Widgets::Waterfall::ColorMap.
 */
enum Map {
  Viridis = 0,
  Inferno,
  Magma,
  Plasma,
  Turbo,
  Jet,
  Hot,
  Grayscale,
  MapCount,
};

[[nodiscard]] QRgb sample(int map, double t);
[[nodiscard]] std::vector<QRgb> buildLut(int map);

}  // namespace Widgets::WaterfallColorMap
