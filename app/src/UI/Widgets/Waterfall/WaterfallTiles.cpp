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

#include "UI/Widgets/Waterfall/WaterfallTiles.h"

#include <cmath>
#include <QtGlobal>

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Row slack: a piece thinner than this contributes no pixel under any zoom
static constexpr double kEpsRows = 1e-6;

// The span crosses the ring seam at most once; the third run is the termination proof
static constexpr int kMaxWrapRuns = 3;

// One band is 64 rows, so the tallest image (4096 rows) yields 64 pieces
static constexpr int kMaxPieces = 4096;

//--------------------------------------------------------------------------------------------------
// Decomposition
//--------------------------------------------------------------------------------------------------

/**
 * @brief Number of texture bands an image of @p imageHeight rows is split into.
 */
int Widgets::WaterfallTiles::tileCount(const int imageHeight, const int tileRows)
{
  if (imageHeight <= 0 || tileRows <= 0)
    return 0;

  return (imageHeight + tileRows - 1) / tileRows;
}

/**
 * @brief Maps the visible logical span @p src of a ring-ordered spectrogram onto the quads that
 *        draw it. Logical row 0 is the newest and sits at physical row @p topRow, so the span
 *        crosses the ring seam at most once; each run is then cut at every band boundary, and
 *        each piece takes the share of @p plotRect its row count is worth.
 */
void Widgets::WaterfallTiles::decompose(const QRectF& src,
                                        const QRectF& plotRect,
                                        const int imageHeight,
                                        const int topRow,
                                        std::vector<Piece>& pieces,
                                        const int tileRows)
{
  pieces.clear();
  if (imageHeight <= 0 || tileRows <= 0 || src.height() <= kEpsRows || plotRect.isEmpty())
    return;

  const double height     = imageHeight;
  const double visible    = qMin(src.height(), height);
  const double invVisible = 1.0 / visible;
  const int lastTile      = tileCount(imageHeight, tileRows) - 1;

  double row = std::fmod(src.top() + topRow, height);
  if (row < 0.0)
    row += height;

  double remaining = visible;
  double placed    = 0.0;

  for (int run = 0; run < kMaxWrapRuns && remaining > kEpsRows; ++run) {
    double runRows = qMin(remaining, height - row);
    while (runRows > kEpsRows && static_cast<int>(pieces.size()) < kMaxPieces) {
      const int tile        = qBound(0, static_cast<int>(row / tileRows), lastTile);
      const double tileTop  = static_cast<double>(tile) * tileRows;
      const double tileEnd  = qMin(height, tileTop + tileRows);
      const double pieceLen = qMin(runRows, tileEnd - row);
      if (pieceLen <= kEpsRows)
        break;

      const double y0 = plotRect.top() + placed * invVisible * plotRect.height();
      const double y1 = plotRect.top() + (placed + pieceLen) * invVisible * plotRect.height();

      Piece piece;
      piece.tile = tile;
      piece.dst  = QRectF(plotRect.left(), y0, plotRect.width(), y1 - y0);
      piece.src  = QRectF(src.left(), row - tileTop, src.width(), pieceLen);
      pieces.push_back(piece);

      row       += pieceLen;
      placed    += pieceLen;
      runRows   -= pieceLen;
      remaining -= pieceLen;
    }

    row = 0.0;
  }
}
