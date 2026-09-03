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

#include <cmath>
#include <QTest>

#include "UI/Widgets/Waterfall/WaterfallTiles.h"

// The spectrogram's band decomposition (spec 0075, F1). The waterfall used to upload the whole
// history image every display tick -- 64 MiB at FFT 8192 over a 70 s range -- because one texture
// carried every row. Splitting the image into fixed-height bands makes the upload proportional to
// the rows that changed, but only if the quads still tile the plot exactly: no gap, no overlap,
// the newest row at the top, and the ring seam crossed at most once. Those are the properties
// here, because a decomposition that drifts by a fraction of a row draws a seam the eye reads as
// a scan-line artifact.

class WaterfallTilesTest : public QObject {
  Q_OBJECT

private slots:
  void tileCountCoversEveryRow();
  void unwrappedSpanTilesThePlotExactly();
  void piecesStayInsideTheirBand();
  void wrappedSpanIsContiguousOnScreen();
  void spanShorterThanOneBandIsOnePiece();
  void degenerateInputsProduceNoPieces();

private:
  [[nodiscard]] static bool isContiguous(const std::vector<Widgets::WaterfallTiles::Piece>& pieces,
                                         const QRectF& plotRect);
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Verifies the pieces tile @p plotRect top to bottom with no gap and no overlap.
 */
bool WaterfallTilesTest::isContiguous(const std::vector<Widgets::WaterfallTiles::Piece>& pieces,
                                      const QRectF& plotRect)
{
  constexpr double tolerance = 1e-6;
  if (pieces.empty())
    return false;

  double cursor = plotRect.top();
  for (const auto& piece : pieces) {
    if (std::abs(piece.dst.top() - cursor) > tolerance)
      return false;

    if (piece.dst.height() <= 0.0)
      return false;

    cursor = piece.dst.bottom();
  }

  return std::abs(cursor - plotRect.bottom()) < tolerance;
}

//--------------------------------------------------------------------------------------------------
// Cases
//--------------------------------------------------------------------------------------------------

/**
 * @brief The band count rounds up, so the last (partial) band is still addressable.
 */
void WaterfallTilesTest::tileCountCoversEveryRow()
{
  QCOMPARE(Widgets::WaterfallTiles::tileCount(0), 0);
  QCOMPARE(Widgets::WaterfallTiles::tileCount(1), 1);
  QCOMPARE(Widgets::WaterfallTiles::tileCount(64), 1);
  QCOMPARE(Widgets::WaterfallTiles::tileCount(65), 2);
  QCOMPARE(Widgets::WaterfallTiles::tileCount(4096), 64);
}

/**
 * @brief A history whose newest row sits at physical row 0 is one unbroken run, cut only at the
 *        band boundaries it crosses.
 */
void WaterfallTilesTest::unwrappedSpanTilesThePlotExactly()
{
  const QRectF plot(10.0, 20.0, 300.0, 200.0);
  const QRectF src(0.0, 0.0, 512.0, 256.0);

  std::vector<Widgets::WaterfallTiles::Piece> pieces;
  Widgets::WaterfallTiles::decompose(src, plot, 256, 0, pieces);

  QCOMPARE(pieces.size(), std::size_t(4));
  QVERIFY(isContiguous(pieces, plot));
  QCOMPARE(pieces.front().tile, 0);
  QCOMPARE(pieces.back().tile, 3);
}

/**
 * @brief Every piece samples rows inside its own band, in band-local coordinates: a source rect
 *        that ran past the band would sample the neighbour's texture and repeat its rows.
 */
void WaterfallTilesTest::piecesStayInsideTheirBand()
{
  const QRectF plot(0.0, 0.0, 100.0, 400.0);
  const QRectF src(0.0, 0.0, 128.0, 256.0);

  std::vector<Widgets::WaterfallTiles::Piece> pieces;
  Widgets::WaterfallTiles::decompose(src, plot, 256, 37, pieces);

  QVERIFY(!pieces.empty());
  for (const auto& piece : pieces) {
    QVERIFY(piece.src.top() >= 0.0);
    QVERIFY(piece.src.bottom() <= Widgets::WaterfallTiles::kTileRows);
    QCOMPARE(piece.src.left(), src.left());
    QCOMPARE(piece.src.width(), src.width());
    QVERIFY(piece.tile >= 0 && piece.tile < Widgets::WaterfallTiles::tileCount(256));
  }
}

/**
 * @brief With the ring seam inside the visible span the run splits in two, and the two runs must
 *        still hand the plot one contiguous column of rows.
 */
void WaterfallTilesTest::wrappedSpanIsContiguousOnScreen()
{
  const QRectF plot(0.0, 0.0, 100.0, 300.0);
  const QRectF src(0.0, 0.0, 128.0, 256.0);

  std::vector<Widgets::WaterfallTiles::Piece> pieces;
  Widgets::WaterfallTiles::decompose(src, plot, 256, 100, pieces);

  QVERIFY(isContiguous(pieces, plot));

  double rows = 0.0;
  for (const auto& piece : pieces)
    rows += piece.src.height();

  QVERIFY(std::abs(rows - src.height()) < 1e-6);
}

/**
 * @brief A zoomed-in view that fits inside one band draws exactly one quad.
 */
void WaterfallTilesTest::spanShorterThanOneBandIsOnePiece()
{
  const QRectF plot(0.0, 0.0, 100.0, 300.0);
  const QRectF src(0.0, 4.0, 128.0, 16.0);

  std::vector<Widgets::WaterfallTiles::Piece> pieces;
  Widgets::WaterfallTiles::decompose(src, plot, 256, 0, pieces);

  QCOMPARE(pieces.size(), std::size_t(1));
  QCOMPARE(pieces.front().tile, 0);
  QCOMPARE(pieces.front().src.top(), 4.0);
  QCOMPARE(pieces.front().dst, plot);
}

/**
 * @brief An empty image, an empty plot or an empty span produces nothing to draw rather than a
 *        degenerate quad.
 */
void WaterfallTilesTest::degenerateInputsProduceNoPieces()
{
  std::vector<Widgets::WaterfallTiles::Piece> pieces;

  Widgets::WaterfallTiles::decompose(QRectF(0, 0, 10, 10), QRectF(0, 0, 10, 10), 0, 0, pieces);
  QVERIFY(pieces.empty());

  Widgets::WaterfallTiles::decompose(QRectF(0, 0, 10, 0), QRectF(0, 0, 10, 10), 64, 0, pieces);
  QVERIFY(pieces.empty());

  Widgets::WaterfallTiles::decompose(QRectF(0, 0, 10, 10), QRectF(), 64, 0, pieces);
  QVERIFY(pieces.empty());
}

QTEST_APPLESS_MAIN(WaterfallTilesTest)

#include "tst_waterfall_tiles.moc"
