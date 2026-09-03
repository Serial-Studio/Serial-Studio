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

#include <QImage>
#include <QTest>
#include <vector>

#include "UI/Widgets/Waterfall/WaterfallRingTexture.h"
#include "UI/Widgets/Waterfall/WaterfallTiles.h"

// The waterfall's persistent ring texture (spec 0075, R15.1). Profiling measured 2.2 GiB/s of
// texture upload because every tick handed the scene graph a brand new texture; the fix keeps one
// texture alive and stages only the scanlines that changed. What can break silently is the
// bookkeeping around that: a staged row pointing at the wrong destination draws the newest
// spectrum in the middle of the history, an unbounded staging list re-introduces the per-tick
// allocation the finding is about, and an idle gate that answers "changed" on identical data
// leaves the churn exactly where it was. The GPU half needs a device and is not covered here.

class WaterfallRingTextureTest : public QObject {
  Q_OBJECT

private slots:
  void freshTextureHasNothingStaged();
  void stagedRowsKeepTheirDestination();
  void overflowingTheSlotsEscalatesToAFullUpload();
  void fullUploadSupersedesStagedRows();
  void rejectsRowsOutsideTheRing();
  void rejectsAForeignImage();
  void idleGateSkipsAnIdenticalRow();
  void idleGateFollowsASizeChange();
  void ringSpanTilesThePlotAcrossTheSeam();

private:
  [[nodiscard]] static QImage makeImage(int width, int height);
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a spectrogram-shaped image in the exact format the ring texture expects.
 */
QImage WaterfallRingTextureTest::makeImage(const int width, const int height)
{
  QImage image(width, height, QImage::Format_RGB32);
  image.fill(qRgb(0, 0, 0));
  return image;
}

//--------------------------------------------------------------------------------------------------
// Staging
//--------------------------------------------------------------------------------------------------

/**
 * @brief A texture that has never been staged owns no GPU resource and asks for no upload.
 */
void WaterfallRingTextureTest::freshTextureHasNothingStaged()
{
  Widgets::WaterfallRingTexture texture(QSize(64, 32));

  QCOMPARE(texture.textureSize(), QSize(64, 32));
  QCOMPARE(texture.rhiTexture(), nullptr);
  QCOMPARE(texture.stagedRowCount(), 0);
  QVERIFY(!texture.fullUploadPending());
  QVERIFY(!texture.failed());
  QVERIFY(!texture.hasAlphaChannel());
  QVERIFY(!texture.hasMipmaps());
}

/**
 * @brief Each staged scanline remembers the physical row it must land on, in staging order.
 */
void WaterfallRingTextureTest::stagedRowsKeepTheirDestination()
{
  const QImage image = makeImage(64, 32);
  Widgets::WaterfallRingTexture texture(image.size());

  texture.stageRow(image, 31);
  texture.stageRow(image, 0);
  texture.stageRow(image, 17);

  QCOMPARE(texture.stagedRowCount(), 3);
  QCOMPARE(texture.stagedRowAt(0), 31);
  QCOMPARE(texture.stagedRowAt(1), 0);
  QCOMPARE(texture.stagedRowAt(2), 17);
  QCOMPARE(texture.stagedRowAt(3), -1);
  QVERIFY(!texture.fullUploadPending());
}

/**
 * @brief Past the fixed slot count the tick escalates to one full upload instead of growing a
 *        buffer -- the property that keeps the staging cost constant and the tick allocation-free.
 */
void WaterfallRingTextureTest::overflowingTheSlotsEscalatesToAFullUpload()
{
  const QImage image = makeImage(64, 32);
  Widgets::WaterfallRingTexture texture(image.size());

  for (int row = 0; row <= Widgets::WaterfallRingTexture::kStagedRowSlots; ++row)
    texture.stageRow(image, row);

  QVERIFY(texture.fullUploadPending());
  QCOMPARE(texture.stagedRowCount(), 0);
}

/**
 * @brief A whole-image stage drops the rows already staged: the image carries them.
 */
void WaterfallRingTextureTest::fullUploadSupersedesStagedRows()
{
  const QImage image = makeImage(64, 32);
  Widgets::WaterfallRingTexture texture(image.size());

  texture.stageRow(image, 4);
  QCOMPARE(texture.stagedRowCount(), 1);

  texture.stageImage(image);
  QVERIFY(texture.fullUploadPending());
  QCOMPARE(texture.stagedRowCount(), 0);

  texture.stageRow(image, 5);
  QCOMPARE(texture.stagedRowCount(), 0);
}

/**
 * @brief A row index outside the ring is refused rather than clamped: clamping would overwrite a
 *        real history row with the newest spectrum.
 */
void WaterfallRingTextureTest::rejectsRowsOutsideTheRing()
{
  const QImage image = makeImage(64, 32);
  Widgets::WaterfallRingTexture texture(image.size());

  texture.stageRow(image, -1);
  texture.stageRow(image, 32);
  texture.stageRow(image, 4096);

  QCOMPARE(texture.stagedRowCount(), 0);
  QVERIFY(!texture.fullUploadPending());
}

/**
 * @brief An image of another size never reaches the staging buffers, on either path.
 */
void WaterfallRingTextureTest::rejectsAForeignImage()
{
  const QImage other = makeImage(32, 32);
  Widgets::WaterfallRingTexture texture(QSize(64, 32));

  texture.stageRow(other, 4);
  texture.stageImage(other);

  QCOMPARE(texture.stagedRowCount(), 0);
  QVERIFY(!texture.fullUploadPending());
}

//--------------------------------------------------------------------------------------------------
// Idle gate
//--------------------------------------------------------------------------------------------------

/**
 * @brief The idle gate adopts the first row, then answers "unchanged" for a bit-identical one and
 *        "changed" again as soon as a single bin moves.
 */
void WaterfallRingTextureTest::idleGateSkipsAnIdenticalRow()
{
  std::vector<float> cache;
  const std::vector<float> row = {-10.0f, -20.0f, -30.0f, -40.0f};

  QVERIFY(Widgets::WaterfallRingTexture::captureRowIfChanged(row.data(), 4, cache));
  QVERIFY(!Widgets::WaterfallRingTexture::captureRowIfChanged(row.data(), 4, cache));
  QVERIFY(!Widgets::WaterfallRingTexture::captureRowIfChanged(row.data(), 4, cache));

  std::vector<float> moved = row;
  moved[2]                 = -29.5f;
  QVERIFY(Widgets::WaterfallRingTexture::captureRowIfChanged(moved.data(), 4, cache));
  QVERIFY(!Widgets::WaterfallRingTexture::captureRowIfChanged(moved.data(), 4, cache));
}

/**
 * @brief A spectrum of a different width is always a change, so an FFT-size switch cannot leave
 *        the spectrogram frozen against a stale cache.
 */
void WaterfallRingTextureTest::idleGateFollowsASizeChange()
{
  std::vector<float> cache;
  const std::vector<float> narrow = {-10.0f, -20.0f};
  const std::vector<float> wide   = {-10.0f, -20.0f, -30.0f, -40.0f};

  QVERIFY(Widgets::WaterfallRingTexture::captureRowIfChanged(narrow.data(), 2, cache));
  QVERIFY(Widgets::WaterfallRingTexture::captureRowIfChanged(wide.data(), 4, cache));
  QVERIFY(!Widgets::WaterfallRingTexture::captureRowIfChanged(wide.data(), 4, cache));
  QVERIFY(Widgets::WaterfallRingTexture::captureRowIfChanged(narrow.data(), 2, cache));
}

//--------------------------------------------------------------------------------------------------
// Source-rectangle (UV) math for the single-band ring
//--------------------------------------------------------------------------------------------------

/**
 * @brief With the whole image as one band the decomposition yields the ring's two runs in
 *        absolute image rows, and they still tile the plot exactly -- that source rectangle is the
 *        scroll offset the ring path applies instead of moving pixels.
 */
void WaterfallRingTextureTest::ringSpanTilesThePlotAcrossTheSeam()
{
  constexpr int kRows = 256;
  constexpr int kTop  = 200;

  const QRectF source(0, 0, 128, kRows);
  const QRectF plot(10, 20, 300, 400);

  std::vector<Widgets::WaterfallTiles::Piece> pieces;
  Widgets::WaterfallTiles::decompose(source, plot, kRows, kTop, pieces, kRows);

  QCOMPARE(static_cast<int>(pieces.size()), 2);
  QCOMPARE(pieces[0].tile, 0);
  QCOMPARE(pieces[1].tile, 0);

  QCOMPARE(pieces[0].src.top(), static_cast<double>(kTop));
  QCOMPARE(pieces[0].src.height(), static_cast<double>(kRows - kTop));
  QCOMPARE(pieces[1].src.top(), 0.0);
  QCOMPARE(pieces[1].src.height(), static_cast<double>(kTop));

  QCOMPARE(pieces[0].dst.top(), plot.top());
  QVERIFY(qFuzzyCompare(pieces[0].dst.bottom(), pieces[1].dst.top()));
  QVERIFY(qFuzzyCompare(pieces[1].dst.bottom(), plot.bottom()));
}

QTEST_GUILESS_MAIN(WaterfallRingTextureTest)

#include "tst_waterfall_ring_texture.moc"
