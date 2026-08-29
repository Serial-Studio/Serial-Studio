/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <chrono>
#include <QHash>
#include <QObject>
#include <QTest>

#include "DataModel/FrameBuilder/LatestFrameTap.h"

/**
 * @brief The builder-owned capture state a tap mirrors, gathered so a test can move it the way the
 *        gated capture path does: write the entry, then bump the sequence.
 */
struct CaptureState {
  int newestSourceId = -1;
  quint64 sequence   = 0;
  QHash<int, DataModel::LatestFrameInfo> frames;

  void capture(int sourceId, qint64 timestampMs)
  {
    auto& entry       = frames[sourceId];
    entry.sourceId    = sourceId;
    entry.timestampMs = timestampMs;
    entry.sequence    = ++sequence;
    newestSourceId    = sourceId;
  }
};

/**
 * @brief Builder/GUI handshake of DataModel::LatestFrameTap. Both halves run on the test thread,
 *        which is at once qApp's thread (what the GUI half asserts on) and the owner's thread
 *        (what the builder half asserts on), so the gating and sequence rules are exercised
 *        directly instead of through a race.
 */
class TstLatestFrameTap : public QObject {
  Q_OBJECT

private slots:
  void drainStaysIdleUntilArmed();
  void requestIsLatchedUntilPublished();
  void publishedCaptureReachesTheGui();
  void newestSourceIsServedByDefault();
  void unchangedSequenceDoesNotRepublish();
  void parseLoadsMirrorTheBudget();
};

/**
 * @brief A session with no polling script or API client must never make the builder copy the
 *        capture map: the mirror stays cold until a GUI-thread reader asks for a frame.
 */
void TstLatestFrameTap::drainStaysIdleUntilArmed()
{
  QObject owner;
  CaptureState state;
  DataModel::ParseBudget budget;
  DataModel::LatestFrameTap tap(owner, state.frames, state.newestSourceId, state.sequence, budget);

  state.capture(0, 100);

  QVERIFY(!tap.drainForGui());

  tap.publish();
  QVERIFY(!tap.drainForGui());
}

/**
 * @brief Once armed, the first drain claims the publish request and later drains report false
 *        until the builder half consumed it: the request is a latch, not a per-tick message.
 */
void TstLatestFrameTap::requestIsLatchedUntilPublished()
{
  QObject owner;
  CaptureState state;
  DataModel::ParseBudget budget;
  DataModel::LatestFrameTap tap(owner, state.frames, state.newestSourceId, state.sequence, budget);

  QCOMPARE(tap.guiLatestFrame(-1).sequence, quint64(0));
  state.capture(0, 100);

  QVERIFY(tap.drainForGui());
  QVERIFY(!tap.drainForGui());

  tap.publish();
  QVERIFY(tap.drainForGui());
}

/**
 * @brief The round trip: a capture, a builder publish, and the GUI drain that adopts it.
 */
void TstLatestFrameTap::publishedCaptureReachesTheGui()
{
  QObject owner;
  CaptureState state;
  DataModel::ParseBudget budget;
  DataModel::LatestFrameTap tap(owner, state.frames, state.newestSourceId, state.sequence, budget);

  QCOMPARE(tap.guiLatestFrame(1).sequence, quint64(0));
  state.capture(1, 4242);
  tap.publish();
  QVERIFY(tap.drainForGui());

  const auto served = tap.guiLatestFrame(1);
  QCOMPARE(served.sourceId, 1);
  QCOMPARE(served.timestampMs, qint64(4242));
  QCOMPARE(served.sequence, quint64(1));

  QCOMPARE(tap.guiLatestFrame(9).sequence, quint64(0));
}

/**
 * @brief A negative source id asks for the newest capture across every source, which is the
 *        selection io.getLatestFrame makes when a script names no source.
 */
void TstLatestFrameTap::newestSourceIsServedByDefault()
{
  QObject owner;
  CaptureState state;
  DataModel::ParseBudget budget;
  DataModel::LatestFrameTap tap(owner, state.frames, state.newestSourceId, state.sequence, budget);

  QCOMPARE(tap.guiLatestFrame(-1).sequence, quint64(0));
  state.capture(0, 10);
  state.capture(3, 20);
  tap.publish();
  QVERIFY(tap.drainForGui());

  QCOMPARE(tap.guiLatestFrame(-1).sourceId, 3);
  QCOMPARE(tap.guiLatestFrame(0).timestampMs, qint64(10));
}

/**
 * @brief A publish with no new capture must enqueue nothing: the request arrives every display
 *        tick, and copying the capture map each time is the cost the sequence compare avoids.
 */
void TstLatestFrameTap::unchangedSequenceDoesNotRepublish()
{
  QObject owner;
  CaptureState state;
  DataModel::ParseBudget budget;
  DataModel::LatestFrameTap tap(owner, state.frames, state.newestSourceId, state.sequence, budget);

  QCOMPARE(tap.guiLatestFrame(-1).sequence, quint64(0));
  state.capture(2, 55);
  tap.publish();
  QVERIFY(tap.drainForGui());
  QCOMPARE(tap.guiLatestFrame(2).timestampMs, qint64(55));

  state.frames[2].timestampMs = 999;
  tap.publish();
  QVERIFY(tap.drainForGui());
  QCOMPARE(tap.guiLatestFrame(2).timestampMs, qint64(55));

  state.capture(2, 999);
  tap.publish();
  QVERIFY(tap.drainForGui());
  QCOMPARE(tap.guiLatestFrame(2).timestampMs, qint64(999));
}

/**
 * @brief The 1 Hz parse-load mirror serves what the governor held at publish time, with no
 *        marshal on the reading side.
 */
void TstLatestFrameTap::parseLoadsMirrorTheBudget()
{
  QObject owner;
  CaptureState state;
  DataModel::ParseBudget budget;
  DataModel::LatestFrameTap tap(owner, state.frames, state.newestSourceId, state.sequence, budget);

  QVERIFY(tap.guiParseLoads().empty());

  const auto started = DataModel::ParseBudget::Clock::now();
  (void)budget.account(5, started, started + std::chrono::milliseconds(1));

  tap.publishParseLoads();

  const auto loads = tap.guiParseLoads();
  QCOMPARE(static_cast<int>(loads.size()), 1);
  QCOMPARE(loads.front().sourceId, 5);
  QVERIFY(loads.front().decimateN >= 1);
}

QTEST_GUILESS_MAIN(TstLatestFrameTap)

#include "tst_latest_frame_tap.moc"
