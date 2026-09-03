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

#include <QSignalSpy>
#include <QTest>

#include "DataModel/ReplayPlaybackEngine.h"

using DataModel::ReplayPlaybackEngine;

/**
 * @brief The playback mechanics the CSV, MDF4 and Sessions players compose (spec 0075 B11): the
 *        trailing seek-window walk, the epoch that retires a superseded timer chain, the
 *        steady-clock anchor that makes the recording own replay time, the catch-up fill gate and
 *        the two-timer scrub chain.
 */
class TstReplayPlaybackEngine : public QObject {
  Q_OBJECT

private slots:
  void theWindowCoversThePlotTimeRange();
  void theWindowNeverHoldsFewerRowsThanPoints();
  void anUntimedRowEndsTheWalk();
  void theEpochRetiresASupersededChain();
  void replayTimeIsTheRecordingsTime();
  void theCatchUpFillIsGated();
  void aScrubArmsTheTickThenTheSettle();
  void theTimestampLabelNeverGoesNegative();
};

/**
 * @brief The walk stops as soon as one more row would exceed the plot's time range: a 10 s range
 *        over rows one second apart reaches back ten rows and no further.
 */
void TstReplayPlaybackEngine::theWindowCoversThePlotTimeRange()
{
  const auto secondsAt = [](int row) {
    return static_cast<double>(row);
  };

  QCOMPARE(ReplayPlaybackEngine::seekWindowStartRow(50, 1, 10.0, secondsAt), 40);
  QCOMPARE(ReplayPlaybackEngine::seekWindowStartRow(50, 1, 0.0, secondsAt), 50);
  QCOMPARE(ReplayPlaybackEngine::seekWindowStartRow(5, 1, 1000.0, secondsAt), 0);
  QCOMPARE(ReplayPlaybackEngine::seekWindowStartRow(0, 1, 10.0, secondsAt), 0);
}

/**
 * @brief points() is a FLOOR, not a ceiling: a plot configured for 300 samples must not be handed
 *        a two-sample window just because the recording is dense.
 */
void TstReplayPlaybackEngine::theWindowNeverHoldsFewerRowsThanPoints()
{
  const auto secondsAt = [](int row) {
    return row * 0.001;
  };

  QCOMPARE(ReplayPlaybackEngine::seekWindowStartRow(1000, 300, 0.0, secondsAt), 701);
  QCOMPARE(ReplayPlaybackEngine::seekWindowStartRow(100, 300, 0.0, secondsAt), 0);
}

/**
 * @brief A row the index has not timed yet reports a negative time; the walk stops there instead
 *        of treating the sentinel as an enormous backwards jump.
 */
void TstReplayPlaybackEngine::anUntimedRowEndsTheWalk()
{
  const auto secondsAt = [](int row) {
    return (row < 20) ? -1.0 : static_cast<double>(row);
  };

  QCOMPARE(ReplayPlaybackEngine::seekWindowStartRow(50, 1, 1000.0, secondsAt), 20);

  const auto untimed = [](int) {
    return -1.0;
  };
  QCOMPARE(ReplayPlaybackEngine::seekWindowStartRow(50, 1, 1000.0, untimed), 50);
}

/**
 * @brief A pause/play cycle opens a new epoch, so the timer chain the previous play() armed sees
 *        a stale epoch and retires instead of injecting rows alongside the new one.
 */
void TstReplayPlaybackEngine::theEpochRetiresASupersededChain()
{
  ReplayPlaybackEngine engine;

  const quint64 first = engine.nextEpoch();
  QVERIFY(engine.isCurrentEpoch(first));
  QCOMPARE(engine.epoch(), first);

  const quint64 second = engine.nextEpoch();
  QVERIFY(second != first);
  QVERIFY(engine.isCurrentEpoch(second));
  QVERIFY(!engine.isCurrentEpoch(first));
}

/**
 * @brief The gap between two replayed rows is the gap the recording captured, not the gap the
 *        wall clock happened to see: the anchored base advances by the recorded delta exactly.
 */
void TstReplayPlaybackEngine::replayTimeIsTheRecordingsTime()
{
  ReplayPlaybackEngine engine;
  engine.anchorSteadyBase(10.0);

  const auto base  = engine.steadyBase();
  const auto later = engine.steadyTimestampFor(12.5);
  const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(later - base).count();
  QCOMPARE(delta, 2500);

  const auto earlier = engine.steadyTimestampFor(9.0);
  QVERIFY(earlier < base);

  engine.anchorSteadyBase(-1.0);
  const auto zeroDelta = std::chrono::duration_cast<std::chrono::milliseconds>(
                           engine.steadyTimestampFor(0.0) - engine.steadyBase())
                           .count();
  QCOMPARE(zeroDelta, 0);
}

/**
 * @brief The catch-up plot fill is what makes a long fast-forward look continuous, but running it
 *        per injected row costs more than the injection; the gate opens once, then holds.
 */
void TstReplayPlaybackEngine::theCatchUpFillIsGated()
{
  ReplayPlaybackEngine engine;

  QVERIFY(engine.catchUpFillDue());
  QVERIFY(!engine.catchUpFillDue());

  engine.resetCatchUpFill();
  QVERIFY(engine.catchUpFillDue());
}

/**
 * @brief A drag coalesces into ~30 Hz ticks and one settle pass: re-arming while the tick is
 *        pending must not queue a second tick, and the settle only fires once the slider rests.
 */
void TstReplayPlaybackEngine::aScrubArmsTheTickThenTheSettle()
{
  ReplayPlaybackEngine engine;

  QSignalSpy ticks(&engine, &ReplayPlaybackEngine::seekTick);
  QSignalSpy settles(&engine, &ReplayPlaybackEngine::seekSettle);

  engine.armSeek();
  engine.armSeek();
  QVERIFY(ticks.wait(ReplayPlaybackEngine::kSeekTickMs * 20));
  QCOMPARE(ticks.count(), 1);
  QCOMPARE(settles.count(), 0);

  QVERIFY(settles.wait(ReplayPlaybackEngine::kSeekSettleMs * 20));
  QCOMPARE(settles.count(), 1);

  engine.armSeek();
  engine.stopSeek();
  QVERIFY(!settles.wait(ReplayPlaybackEngine::kSeekSettleMs * 2));
  QCOMPARE(settles.count(), 1);
}

/**
 * @brief The transport label reads HH:MM:SS.mmm; a negative offset (a recording whose first row
 *        precedes the anchor) clamps to zero rather than printing a time the tape cannot reach.
 */
void TstReplayPlaybackEngine::theTimestampLabelNeverGoesNegative()
{
  QCOMPARE(ReplayPlaybackEngine::formatTimestamp(0.0), QStringLiteral("00:00:00.000"));
  QCOMPARE(ReplayPlaybackEngine::formatTimestamp(3661.5), QStringLiteral("01:01:01.500"));
  QCOMPARE(ReplayPlaybackEngine::formatTimestamp(-5.0), QStringLiteral("00:00:00.000"));
}

QTEST_GUILESS_MAIN(TstReplayPlaybackEngine)

#include "tst_replay_playback_engine.moc"
