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

#include <cmath>
#include <limits>
#include <QTest>

#include "UI/Widgets/PlotAutoScale.h"

/**
 * @file tst_plot_autoscale.cpp
 * @brief The 1-2-5 auto-scale ladder and its hysteresis (spec 0058 A2.3): the ladder is
 *        monotonic through negative indices, the chosen step always fits the extent in
 *        kDivisions, growth is immediate, shrink waits for the margin, and a bipolar range puts
 *        zero on a division boundary.
 */
class TstPlotAutoScale : public QObject {
  Q_OBJECT

private slots:
  void ladderIsMonotonic();
  void ladderIndexPicksFirstFit();
  void firstQuantizationFitsExtent();
  void bipolarRangeSplitsAtZero();
  void positiveRangeIsNotForcedToZero();
  void growsImmediatelyShrinksWithMargin();
  void rejectsDegenerateInput();
};

/**
 * @brief Steps ascend through 1, 2, 5 per decade in both directions from index 0 (= 1.0).
 */
void TstPlotAutoScale::ladderIsMonotonic()
{
  using namespace Widgets::AutoScale;
  QCOMPARE(ladderStep(0), 1.0);
  QCOMPARE(ladderStep(1), 2.0);
  QCOMPARE(ladderStep(2), 5.0);
  QCOMPARE(ladderStep(3), 10.0);
  QCOMPARE(ladderStep(-1), 0.5);
  QCOMPARE(ladderStep(-2), 0.2);
  QCOMPARE(ladderStep(-3), 0.1);
  QCOMPARE(ladderStep(-4), 0.05);

  for (int i = kMinStepIndex; i < kMaxStepIndex; ++i)
    QVERIFY(ladderStep(i) < ladderStep(i + 1));
}

/**
 * @brief The index chosen for a per-division value is the smallest whose step covers it.
 */
void TstPlotAutoScale::ladderIndexPicksFirstFit()
{
  using namespace Widgets::AutoScale;
  QCOMPARE(ladderIndexFor(1.0), 0);
  QCOMPARE(ladderIndexFor(1.5), 1);
  QCOMPARE(ladderIndexFor(2.0), 1);
  QCOMPARE(ladderIndexFor(3.0), 2);
  QCOMPARE(ladderIndexFor(5.0), 2);
  QCOMPARE(ladderIndexFor(7.0), 3);
  QCOMPARE(ladderIndexFor(0.3), -1);
  QCOMPARE(ladderIndexFor(0.05), -4);
  QCOMPARE(ladderIndexFor(0.0), 0);
  QCOMPARE(ladderIndexFor(std::numeric_limits<double>::quiet_NaN()), 0);
}

/**
 * @brief A fresh quantization takes the required step and the snapped range covers the data.
 */
void TstPlotAutoScale::firstQuantizationFitsExtent()
{
  using namespace Widgets::AutoScale;
  double min = 0.13;
  double max = 7.9;
  int index  = kNoStep;
  QVERIFY(quantizeRange(min, max, index));
  QCOMPARE(index, ladderIndexFor((7.9 - 0.13) / kDivisions));
  QCOMPARE(ladderStep(index), 1.0);
  QCOMPARE(min, 0.0);
  QCOMPARE(max, 8.0);
  QVERIFY(min <= 0.13);
  QVERIFY(max >= 7.9);
}

/**
 * @brief Data that goes negative snaps so zero is an exact multiple of the step.
 */
void TstPlotAutoScale::bipolarRangeSplitsAtZero()
{
  using namespace Widgets::AutoScale;
  double min = -0.37;
  double max = 1.42;
  int index  = kNoStep;
  QVERIFY(quantizeRange(min, max, index));
  const double step = ladderStep(index);
  QCOMPARE(step, 0.5);
  QVERIFY(std::fmod(min, step) == 0.0);
  QVERIFY(std::fmod(max, step) == 0.0);
  QCOMPARE(min, -0.5);
  QCOMPARE(max, 1.5);
}

/**
 * @brief A strictly positive range keeps a positive floor instead of being pulled to zero.
 */
void TstPlotAutoScale::positiveRangeIsNotForcedToZero()
{
  using namespace Widgets::AutoScale;
  double min = 100.3;
  double max = 104.1;
  int index  = kNoStep;
  QVERIFY(quantizeRange(min, max, index));
  QCOMPARE(ladderStep(index), 0.5);
  QCOMPARE(min, 100.0);
  QCOMPARE(max, 104.5);
}

/**
 * @brief The step grows as soon as the extent needs it, and shrinks only once the extent fits
 *        under kShrinkMargin of the next finer step's span; a value hovering at a boundary
 *        therefore holds the coarser step.
 */
void TstPlotAutoScale::growsImmediatelyShrinksWithMargin()
{
  using namespace Widgets::AutoScale;
  int index  = kNoStep;
  double min = 0.0;
  double max = 8.0;
  QVERIFY(quantizeRange(min, max, index));
  QCOMPARE(ladderStep(index), 1.0);

  min = 0.0;
  max = 8.4;
  QVERIFY(quantizeRange(min, max, index));
  QCOMPARE(ladderStep(index), 2.0);
  QCOMPARE(max, 10.0);

  min = 0.0;
  max = 7.9;
  QVERIFY(quantizeRange(min, max, index));
  QCOMPARE(ladderStep(index), 2.0);

  min = 0.0;
  max = 6.5;
  QVERIFY(quantizeRange(min, max, index));
  QCOMPARE(ladderStep(index), 2.0);

  min = 0.0;
  max = 6.3;
  QVERIFY(quantizeRange(min, max, index));
  QCOMPARE(ladderStep(index), 1.0);
  QCOMPARE(max, 7.0);

  min = 0.0;
  max = 1.0;
  QVERIFY(quantizeRange(min, max, index));
  QCOMPARE(ladderStep(index), 0.2);
}

/**
 * @brief Non-finite or empty input is refused and leaves the range and index untouched.
 */
void TstPlotAutoScale::rejectsDegenerateInput()
{
  using namespace Widgets::AutoScale;
  int index  = 3;
  double min = 2.0;
  double max = 2.0;
  QVERIFY(!quantizeRange(min, max, index));
  QCOMPARE(index, 3);
  QCOMPARE(min, 2.0);

  min = std::numeric_limits<double>::quiet_NaN();
  max = 5.0;
  QVERIFY(!quantizeRange(min, max, index));
  QCOMPARE(index, 3);

  min = 1.0;
  max = std::numeric_limits<double>::infinity();
  QVERIFY(!quantizeRange(min, max, index));
  QCOMPARE(index, 3);
}

QTEST_APPLESS_MAIN(TstPlotAutoScale)

#include "tst_plot_autoscale.moc"
