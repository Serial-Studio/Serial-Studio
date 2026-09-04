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

#include <QRectF>
#include <vector>

namespace Widgets::WaterfallTiles {

/**
 * @brief Rows of the spectrogram image carried by one texture band.
 */
inline constexpr int kTileRows = 64;

/**
 * @brief One quad of the spectrogram: a band index, where it lands on screen, and the band-local
 *        source rows it samples.
 */
struct Piece {
  int tile;
  QRectF dst;
  QRectF src;
};

[[nodiscard]] int tileCount(int imageHeight, int tileRows = kTileRows);

void decompose(const QRectF& src,
               const QRectF& plotRect,
               int imageHeight,
               int topRow,
               std::vector<Piece>& pieces,
               int tileRows = kTileRows);

}  // namespace Widgets::WaterfallTiles
