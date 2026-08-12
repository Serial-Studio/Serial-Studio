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

#include <QTest>

#include "DataModel/ParseBudget.h"

using DataModel::ParseBudget;

static constexpr qint64 kMs = 1'000'000;

/**
 * @brief Returns a fabricated steady-clock time point @p ns nanoseconds from the epoch, so every
 *        fixture drives the governor on a deterministic simulated clock.
 */
static ParseBudget::Clock::time_point tp(qint64 ns)
{
  return ParseBudget::Clock::time_point(std::chrono::nanoseconds(ns));
}

/**
 * @brief Drives one simulated frame through the governor exactly like FrameBuilder does: the
 *        skip gate first, then accounting only for processed frames. Returns true when skipped.
 */
static bool driveFrame(ParseBudget& budget, int sourceId, qint64 startNs, qint64 busyNs)
{
  if (budget.skipFrame(sourceId))
    return true;

  const bool engaged = budget.account(sourceId, tp(startNs), tp(startNs + busyNs));
  Q_UNUSED(engaged)
  return false;
}

/**
 * @brief Returns the snapshot row for @p sourceId, or a defaulted row when untracked.
 */
static ParseBudget::Load loadFor(const ParseBudget& budget, int sourceId)
{
  const auto loads = budget.snapshot();
  for (const auto& load : loads)
    if (load.sourceId == sourceId)
      return load;

  return {sourceId, 1, 0.0, 0};
}

/**
 * @brief Deterministic coverage for the spec-0051 fair-share parse-load governor: no thinning
 *        under the capacity threshold, light sources untouchable beside an offender, proportional
 *        decimation, EWMA recovery inside one second, and reset semantics.
 */
class TestParseBudget : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

private slots:
  void underThresholdNeverThins();
  void lightSourceNeverStarved();
  void proportionalDecimation();
  void recoveryWithinOneSecond();
  void quietOffenderRecovers();
  void resetClearsEverything();
};

/**
 * @brief A single source at ~60% duty for two simulated seconds never skips a frame and never
 *        raises the thinning flag: below the 90% capacity threshold nobody is thinned, no matter
 *        how expensive an individual source is.
 */
void TestParseBudget::underThresholdNeverThins()
{
  ParseBudget budget;

  for (qint64 t = 0; t < 2000 * kMs; t += kMs)
    QVERIFY(!driveFrame(budget, 0, t, kMs * 6 / 10));

  QVERIFY(!budget.thinning());
  QCOMPARE(loadFor(budget, 0).decimateN, 1);
  QVERIFY(loadFor(budget, 0).duty > 0.45 && loadFor(budget, 0).duty < 0.75);
}

/**
 * @brief One saturating source (~95% duty at 1 kHz) beside a light 10 Hz source: the offender is
 *        decimated, while the light source is never skipped -- the bug-report scenario (a 10 Hz
 *        CAN link starved by an audio source) must be impossible by construction.
 */
void TestParseBudget::lightSourceNeverStarved()
{
  ParseBudget budget;

  quint64 heavy_skips = 0;
  for (qint64 t = 0; t < 3000 * kMs; t += kMs) {
    if (driveFrame(budget, 0, t, kMs * 95 / 100))
      ++heavy_skips;

    if ((t / kMs) % 100 == 0)
      QVERIFY(!driveFrame(budget, 1, t, 1000));
  }

  QVERIFY(budget.thinning());
  QVERIFY(heavy_skips > 0);
  QVERIFY(loadFor(budget, 0).decimateN > 1);
  QCOMPARE(loadFor(budget, 1).decimateN, 1);
  QCOMPARE(loadFor(budget, 1).skippedFrames, quint64(0));
}

/**
 * @brief Two equally-heavy sources at ~60% offered duty each oversubscribe the thread (120%
 *        total); both sit above the fair share (45%) and both settle on the stable proportional
 *        factor N = 2 (offered-load scaling keeps the estimate at 60% while decimated).
 */
void TestParseBudget::proportionalDecimation()
{
  ParseBudget budget;

  for (qint64 t = 0; t < 3000 * kMs; t += kMs) {
    (void)driveFrame(budget, 0, t, kMs * 6 / 10);
    (void)driveFrame(budget, 1, t + kMs * 6 / 10, kMs * 6 / 10);
  }

  QVERIFY(budget.thinning());
  QCOMPARE(loadFor(budget, 0).decimateN, 2);
  QCOMPARE(loadFor(budget, 1).decimateN, 2);
}

/**
 * @brief After the overload stops, the offender returns to N=1 and the thinning flag clears in
 *        well under one simulated second (EWMA tau is 250 ms) once cheap frames keep arriving.
 */
void TestParseBudget::recoveryWithinOneSecond()
{
  ParseBudget budget;

  for (qint64 t = 0; t < 3000 * kMs; t += kMs)
    (void)driveFrame(budget, 0, t, kMs * 96 / 100);

  QVERIFY(budget.thinning());
  QVERIFY(loadFor(budget, 0).decimateN > 1);

  qint64 recovered_at = -1;
  for (qint64 t = 3000 * kMs; t < 4000 * kMs; t += kMs) {
    (void)driveFrame(budget, 0, t, 1000);
    if (recovered_at < 0 && loadFor(budget, 0).decimateN == 1)
      recovered_at = t;
  }

  QVERIFY(recovered_at > 0);
  QVERIFY(recovered_at - 3000 * kMs < 1000 * kMs);
  QVERIFY(!budget.thinning());
  QCOMPARE(loadFor(budget, 0).decimateN, 1);
}

/**
 * @brief A source that goes SILENT while decimated must not latch the thinning flag or its
 *        stale factor forever: the 1 Hz maintain() sweep decays it out, so the badge clears
 *        and resumed frames are not wrongly dropped (review finding, 2026-08-11).
 */
void TestParseBudget::quietOffenderRecovers()
{
  ParseBudget budget;

  for (qint64 t = 0; t < 3000 * kMs; t += kMs)
    (void)driveFrame(budget, 0, t, kMs * 96 / 100);

  QVERIFY(budget.thinning());
  QVERIFY(loadFor(budget, 0).decimateN > 1);

  for (qint64 t = 3000 * kMs; t < 3500 * kMs; t += 100 * kMs)
    (void)driveFrame(budget, 1, t, 1000);

  budget.maintain(tp(4000 * kMs));

  QVERIFY(!budget.thinning());
  QCOMPARE(loadFor(budget, 0).decimateN, 1);
  QVERIFY(!budget.skipFrame(0));
}

/**
 * @brief reset() drops every tracked source, the total estimate, and the thinning latch.
 */
void TestParseBudget::resetClearsEverything()
{
  ParseBudget budget;

  for (qint64 t = 0; t < 2000 * kMs; t += kMs)
    (void)driveFrame(budget, 0, t, kMs * 96 / 100);

  QVERIFY(budget.thinning());

  budget.reset();
  QVERIFY(!budget.thinning());
  QVERIFY(budget.snapshot().empty());
  QVERIFY(!budget.skipFrame(0));
}

QTEST_GUILESS_MAIN(TestParseBudget)
#include "tst_parse_budget.moc"
