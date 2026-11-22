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

#include <limits>
#include <QTest>

#include "UI/Widgets/PlotLaneRanges.h"

// Multi-plot lane ranges (spec 0127 R23/R24, AC14): two channels of different magnitude keep
// different ranges, a hidden or empty curve holds the range its lane had, a flat signal still
// gets a band, declared ranges are taken as they are, and a noisy signal does not move its band.

using Widgets::LaneRanges::assignDeclared;
using Widgets::LaneRanges::assignScanned;
using Widgets::LaneRanges::Lane;

class MultiPlotLanesTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void channelsOfDifferentMagnitudeKeepTheirOwnRange();
  void scannedRangeContainsTheData_data();
  void scannedRangeContainsTheData();
  void emptyScanHoldsTheLane();
  void flatSignalGetsABand();
  void declaredRangeIsTakenAsIs();
  void jitterDoesNotMoveTheBand();
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting.
 */
void MultiPlotLanesTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief A 0-1 flag and a 0-4000 count, which share one axis in overlay mode, get ranges three
 *        orders of magnitude apart in lane mode.
 */
void MultiPlotLanesTest::channelsOfDifferentMagnitudeKeepTheirOwnRange()
{
  Lane flag;
  Lane count;
  QVERIFY(assignScanned(flag, 0.0, 1.0));
  QVERIFY(assignScanned(count, 0.0, 4000.0));

  QVERIFY(flag.max - flag.min < 3.0);
  QVERIFY(count.max - count.min >= 4000.0);
  QVERIFY((count.max - count.min) > 1000.0 * (flag.max - flag.min));
}

/**
 * @brief One row per scanned extent.
 */
void MultiPlotLanesTest::scannedRangeContainsTheData_data()
{
  QTest::addColumn<double>("lo");
  QTest::addColumn<double>("hi");

  QTest::newRow("unit") << 0.0 << 1.0;
  QTest::newRow("bipolar") << -2.5 << 7.25;
  QTest::newRow("large") << 1000.0 << 4096.0;
  QTest::newRow("small") << 0.0012 << 0.0019;
  QTest::newRow("negative") << -500.0 << -100.0;
}

/**
 * @brief The lane always covers the extent it was given, and its range is a proper interval.
 */
void MultiPlotLanesTest::scannedRangeContainsTheData()
{
  QFETCH(double, lo);
  QFETCH(double, hi);

  Lane lane;
  (void)assignScanned(lane, lo, hi);
  QVERIFY(lane.min <= lo);
  QVERIFY(lane.max >= hi);
  QVERIFY(lane.max > lane.min);
}

/**
 * @brief A hidden curve is not scanned, so its lane sees the min > max sentinel and keeps what it
 *        had; a non-finite extent is held the same way.
 */
void MultiPlotLanesTest::emptyScanHoldsTheLane()
{
  Lane lane;
  QVERIFY(assignScanned(lane, 10.0, 20.0));
  const double min = lane.min;
  const double max = lane.max;

  QVERIFY(!assignScanned(
    lane, std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest()));
  QVERIFY(!assignScanned(lane, std::numeric_limits<double>::quiet_NaN(), 5.0));
  QVERIFY(!assignScanned(lane, 0.0, std::numeric_limits<double>::infinity()));
  QCOMPARE(lane.min, min);
  QCOMPARE(lane.max, max);
}

/**
 * @brief A constant signal, zero or not, gets a band around its value instead of a zero-height
 *        range.
 */
void MultiPlotLanesTest::flatSignalGetsABand()
{
  Lane zero;
  QVERIFY(assignScanned(zero, 0.0, 0.0));
  QVERIFY(zero.min < 0.0);
  QVERIFY(zero.max > 0.0);

  Lane fifty;
  QVERIFY(assignScanned(fifty, 50.0, 50.0));
  QVERIFY(fifty.min < 50.0);
  QVERIFY(fifty.max > 50.0);
}

/**
 * @brief A range the dataset declares is stored unpadded and in order; a degenerate one is
 *        refused.
 */
void MultiPlotLanesTest::declaredRangeIsTakenAsIs()
{
  Lane lane;
  QVERIFY(assignDeclared(lane, 100.0, -100.0));
  QCOMPARE(lane.min, -100.0);
  QCOMPARE(lane.max, 100.0);
  QVERIFY(!assignDeclared(lane, -100.0, 100.0));
  QVERIFY(!assignDeclared(lane, 5.0, 5.0));
  QCOMPARE(lane.min, -100.0);
  QCOMPARE(lane.max, 100.0);
}

/**
 * @brief A signal whose extent wanders by a few percent reports its range once, so the view is
 *        not asked to move the band on every display tick.
 */
void MultiPlotLanesTest::jitterDoesNotMoveTheBand()
{
  Lane lane;
  QVERIFY(assignScanned(lane, 0.0, 100.0));

  int moves = 0;
  for (int i = 0; i < 200; ++i) {
    const double wobble = static_cast<double>(i % 7) * 0.4;
    if (assignScanned(lane, 0.0 + wobble, 100.0 - wobble))
      ++moves;
  }

  QCOMPARE(moves, 0);
}

QTEST_APPLESS_MAIN(MultiPlotLanesTest)

#include "tst_multiplot_lanes.moc"
