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

#include "DSP.h"
#include "UI/Dashboard/TimeRingSizing.h"

/**
 * @file tst_time_ring_sizing.cpp
 * @brief Time-ring sizing policies (spec 0095 M3): a ring is sized from a rate over a window,
 *        never a sample count alone, and the 2026-08 audio cases fill their configured window.
 */

class TimeRingSizingTest : public QObject {
  Q_OBJECT

private slots:
  void capacityComesFromRateTimesWindow();
  void capacityClampsToItsBounds();
  void audioStreamsFillTheirWindow();
  void frameLaneSizesForTheAssumedMaxRate();
  void saturatedRingGrowsPastHysteresisOnly();
  void growthIgnoresAnUnmeasuredCadence();
  void historyRingCoversRangePlusHeadroom();
};

/**
 * @brief The product of rate and window, truncated, is the slot count.
 */
void TimeRingSizingTest::capacityComesFromRateTimesWindow()
{
  QCOMPARE(DSP::RingCapacity::fromRate(1.0, 64.0).value(), 64);
  QCOMPARE(DSP::RingCapacity::fromRate(2.5, 1000.0).value(), 2500);
}

/**
 * @brief Degenerate inputs give one slot; anything past the shared ceiling folds onto it.
 */
void TimeRingSizingTest::capacityClampsToItsBounds()
{
  QCOMPARE(DSP::RingCapacity::fromRate(0.0, 1000.0).value(), 1);
  QCOMPARE(DSP::RingCapacity::fromRate(1.0, std::nan("")).value(), 1);
  QCOMPARE(DSP::RingCapacity::fromRate(100.0, 1.0e9).value(), DSP::RingCapacity::kMaxSamples);
  QCOMPARE(DSP::RingCapacity::fromRate(1.0, 10.0).atLeast(1024).value(), 1024);
  QCOMPARE(DSP::RingCapacity::fromRate(1.0, 4096.0).atMost(1024).value(), 1024);
}

/**
 * @brief The 2026-08-15 regression: 44.1 kHz on a 10 s axis must hold the whole window (it
 *        filled 5.9 s when bounded in samples), and 48 kHz likewise.
 */
void TimeRingSizingTest::audioStreamsFillTheirWindow()
{
  const double window = UI::TimeRingSizing::historyWindowSec(10.0);
  for (const double rate : {44100.0, 48000.0}) {
    const auto slotCount = UI::TimeRingSizing::streamLaneCapacity(window, rate).value();
    QVERIFY(static_cast<double>(slotCount) / rate >= 10.0);
  }
}

/**
 * @brief The frame lane cannot know its rate at layout time, so it sizes for the fastest device
 *        it supports, capped, and never below the plot-bucket floor.
 */
void TimeRingSizingTest::frameLaneSizesForTheAssumedMaxRate()
{
  QCOMPARE(UI::TimeRingSizing::frameLaneCapacity(10.0).value(), 262144);
  QCOMPARE(UI::TimeRingSizing::frameLaneCapacity(0.0001).value(), 1024);
}

/**
 * @brief A full ring grows once its measured cadence asks for 1.5x its size or more, never less.
 */
void TimeRingSizingTest::saturatedRingGrowsPastHysteresisOnly()
{
  const auto grown = UI::TimeRingSizing::grownCapacity(1024, 10.0, 1.0 / 48000.0);
  QVERIFY(grown.has_value());
  QVERIFY(grown->value() >= 1536);

  QVERIFY(!UI::TimeRingSizing::grownCapacity(400000, 10.0, 1.0 / 48000.0).has_value());
}

/**
 * @brief No measured period yet means no growth decision.
 */
void TimeRingSizingTest::growthIgnoresAnUnmeasuredCadence()
{
  QVERIFY(!UI::TimeRingSizing::grownCapacity(1024, 10.0, 0.0).has_value());
}

/**
 * @brief A history ring covers the visible range plus headroom, so a saturated min/max source
 *        still spans the full axis.
 */
void TimeRingSizingTest::historyRingCoversRangePlusHeadroom()
{
  QVERIFY(UI::TimeRingSizing::historyWindowSec(10.0) > 10.0);

  const auto ring = UI::TimeRingSizing::historyRing(10.0, 48000.0);
  const double spanSec =
    static_cast<double>(ring.level0.time.capacity()) * ring.level0.interval / 2.0;
  QVERIFY(spanSec >= 10.0);
}

QTEST_APPLESS_MAIN(TimeRingSizingTest)

#include "tst_time_ring_sizing.moc"
