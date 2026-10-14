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
 * @brief Time-ring sizing policies (spec 0095 M3, spec 0097): a ring is sized from a rate over a
 *        window, never a sample count alone and never for an assumed maximum rate; the 2026-08
 *        audio cases fill their configured window.
 */

class TimeRingSizingTest : public QObject {
  Q_OBJECT

private slots:
  void capacityComesFromRateTimesWindow();
  void capacityClampsToItsBounds();
  void audioStreamsFillTheirWindow();
  void frameLaneStartsSmallAndExact();
  void streamPastTheCeilingStartsDecimating();
  void streamPastTheRailStartsDecimating();
  void historyRingCoversRangePlusHeadroom();
  void sweepCapacityKeepsTheHistoryDensity();
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
    const auto slotCount = UI::TimeRingSizing::initialCapacity(window, rate).value();
    QVERIFY(static_cast<double>(slotCount) / rate >= 10.0);
  }
}

/**
 * @brief The frame lane cannot know its rate at layout time, so it starts at the floor, exact,
 *        and grows on coverage (spec 0097); it never reserves for an assumed rate.
 */
void TimeRingSizingTest::frameLaneStartsSmallAndExact()
{
  const double window = UI::TimeRingSizing::historyWindowSec(10.0);
  QCOMPARE(UI::TimeRingSizing::initialCapacity(window, 0.0).value(), 1024);
  QCOMPARE(UI::TimeRingSizing::initialCapacity(0.0001, 0.0).value(), 1024);
  QVERIFY(UI::TimeRingSizing::initialMode(window, 0.0) == DSP::RingMode::Exact);

  DSP::RingBudget budget;
  const auto ring = UI::TimeRingSizing::historyRing(10.0, 0.0, budget);
  QVERIFY(ring.exact());
  QVERIFY(ring.limit == DSP::RingLimit::None);
  QVERIFY(ring.bytes() < std::size_t(64 * 1024));
  QCOMPARE(budget.used, ring.bytes());
}

/**
 * @brief A stream source whose window exceeds the per-ring ceiling decimates from the start and
 *        records why; one under it starts exact at its real rate plus the growth headroom, so
 *        its first wrap evicts only out-of-window samples and never grows the ring.
 */
void TimeRingSizingTest::streamPastTheCeilingStartsDecimating()
{
  const double longWindow = UI::TimeRingSizing::historyWindowSec(300.0);
  QVERIFY(UI::TimeRingSizing::initialMode(longWindow, 48000.0) == DSP::RingMode::Decimating);

  DSP::RingBudget budget;
  const auto capped = UI::TimeRingSizing::historyRing(300.0, 48000.0, budget);
  QVERIFY(!capped.exact());
  QVERIFY(capped.limit == DSP::RingLimit::Ceiling);
  QCOMPARE(capped.level0.time.capacity(), std::size_t(DSP::RingCapacity::kMaxSamples));

  auto exact = UI::TimeRingSizing::historyRing(10.0, 48000.0, budget);
  QVERIFY(exact.exact());
  QCOMPARE(exact.level0.time.capacity(), std::size_t(660000));

  const std::size_t block = 4096;
  for (std::size_t i = 0; i < exact.level0.time.capacity(); ++i)
    exact.appendDecimated(static_cast<double>(i) / 48000.0, 0.0);

  const double tNewest = static_cast<double>(exact.level0.time.capacity() + block - 1) / 48000.0;
  QVERIFY(!exact.growFor(block, tNewest, budget));
  QVERIFY(exact.exact());
}

/**
 * @brief Spec 0097 R7: a stream ring the rail cannot afford starts decimating at the size the
 *        budget grants, flagged as rail-limited, instead of overrunning the rail.
 */
void TimeRingSizingTest::streamPastTheRailStartsDecimating()
{
  DSP::RingBudget budget(std::size_t(1) << 20);
  const auto ring = UI::TimeRingSizing::historyRing(10.0, 48000.0, budget);
  QVERIFY(!ring.exact());
  QVERIFY(ring.limit == DSP::RingLimit::Rail);
  QVERIFY(ring.level0.time.capacity() < std::size_t(660000));
  QVERIFY(budget.binding);
}

/**
 * @brief A history ring covers the visible range plus headroom, so a saturated min/max source
 *        still spans the full axis.
 */
void TimeRingSizingTest::historyRingCoversRangePlusHeadroom()
{
  QVERIFY(UI::TimeRingSizing::historyWindowSec(10.0) > 10.0);

  DSP::RingBudget budget;
  const auto ring = UI::TimeRingSizing::historyRing(10.0, 48000.0, budget);
  const double spanSec =
    static_cast<double>(ring.level0.time.capacity()) * ring.level0.interval / 2.0;
  QVERIFY(spanSec >= 10.0);
}

/**
 * @brief A sweep captures at the density the history ring keeps: 660000 slots over 12.5 s plus
 *        the growth headroom is 48000 per second, so a 10 s sweep holds 480000 (one slot of
 *        floating-point slack).
 */
void TimeRingSizingTest::sweepCapacityKeepsTheHistoryDensity()
{
  DSP::RingBudget budget;
  const auto history = UI::TimeRingSizing::historyRing(10.0, 48000.0, budget);
  QVERIFY(qAbs(UI::TimeRingSizing::sweepCapacity(history, 10.0).value() - 480000) <= 1);
  QCOMPARE(UI::TimeRingSizing::sweepCapacity(history, 0.001).value(), 1024);
}

QTEST_APPLESS_MAIN(TimeRingSizingTest)

#include "tst_time_ring_sizing.moc"
