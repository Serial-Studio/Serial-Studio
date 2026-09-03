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

#include <QTest>

#include "UI/Widgets/Waterfall/WaterfallColorMap.h"

// The baked waterfall color maps (spec 0075, F5). The spectrogram used to call the map switch per
// pixel -- three multiplies, three casts and three clamps in the double domain, 32768 times per
// tick at FFT 65536. The LUT replaces that with one array read, so what has to hold is that the
// table is the map: every entry equals sample() at the magnitude the entry stands for, the ends
// are the map's extremes, and the quantization the row colorizer applies lands on the entry the
// continuous function would have produced.

class ColorMapLutTest : public QObject {
  Q_OBJECT

private slots:
  void lutHasOneEntryPerQuantizedLevel();
  void lutMatchesTheContinuousMap();
  void lutEndsAreTheMapExtremes();
  void everyMapIsDistinctAndOpaque();
  void outOfRangeMagnitudesClamp();
};

/**
 * @brief The table covers the full byte range the colorizer indexes with.
 */
void ColorMapLutTest::lutHasOneEntryPerQuantizedLevel()
{
  for (int map = 0; map < Widgets::WaterfallColorMap::MapCount; ++map)
    QCOMPARE(Widgets::WaterfallColorMap::buildLut(map).size(),
             std::size_t(Widgets::WaterfallColorMap::kLutSize));
}

/**
 * @brief Every entry is the sampled color at the magnitude the entry represents, for every map:
 *        the LUT must be a bake of the function, not an approximation of it.
 */
void ColorMapLutTest::lutMatchesTheContinuousMap()
{
  const int last = Widgets::WaterfallColorMap::kLutSize - 1;
  for (int map = 0; map < Widgets::WaterfallColorMap::MapCount; ++map) {
    const auto lut = Widgets::WaterfallColorMap::buildLut(map);
    for (int i = 0; i <= last; ++i) {
      const double t = static_cast<double>(i) / last;
      QCOMPARE(lut[static_cast<std::size_t>(i)], Widgets::WaterfallColorMap::sample(map, t));
    }
  }
}

/**
 * @brief The floor entry is what an empty history is filled with, so it must equal the map at 0;
 *        the last entry must equal the map at full scale.
 */
void ColorMapLutTest::lutEndsAreTheMapExtremes()
{
  for (int map = 0; map < Widgets::WaterfallColorMap::MapCount; ++map) {
    const auto lut = Widgets::WaterfallColorMap::buildLut(map);
    QCOMPARE(lut.front(), Widgets::WaterfallColorMap::sample(map, 0.0));
    QCOMPARE(lut.back(), Widgets::WaterfallColorMap::sample(map, 1.0));
  }
}

/**
 * @brief Each map paints an opaque ramp that actually goes somewhere: a table of one repeated
 *        color would satisfy every check above and render a blank spectrogram.
 */
void ColorMapLutTest::everyMapIsDistinctAndOpaque()
{
  for (int map = 0; map < Widgets::WaterfallColorMap::MapCount; ++map) {
    const auto lut = Widgets::WaterfallColorMap::buildLut(map);
    QVERIFY(lut.front() != lut.back());
    for (const QRgb entry : lut)
      QCOMPARE(qAlpha(entry), 255);
  }
}

/**
 * @brief A magnitude outside [0, 1] clamps onto the map ends instead of indexing past the ramp.
 */
void ColorMapLutTest::outOfRangeMagnitudesClamp()
{
  for (int map = 0; map < Widgets::WaterfallColorMap::MapCount; ++map) {
    QCOMPARE(Widgets::WaterfallColorMap::sample(map, -5.0),
             Widgets::WaterfallColorMap::sample(map, 0.0));
    QCOMPARE(Widgets::WaterfallColorMap::sample(map, 12.0),
             Widgets::WaterfallColorMap::sample(map, 1.0));
  }
}

QTEST_APPLESS_MAIN(ColorMapLutTest)

#include "tst_colormap_lut.moc"
