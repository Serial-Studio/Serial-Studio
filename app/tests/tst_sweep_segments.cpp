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

#include <cstddef>
#include <QTest>

#include "DSP.h"

/**
 * @file tst_sweep_segments.cpp
 * @brief Retained sweep segments of DSP::SweepEngine (spec 0061): retention clamps to the byte
 *        budget, every completed sweep lands as a deep copy (later captures never disturb it),
 *        segments read newest-first and wrap, and a rebuilt engine takes them over only when the
 *        shape matches.
 */

namespace {

/**
 * @brief Drives one full sweep through the engine: a rising edge at @p start, then samples up to
 *        one window past it so completeSweep() fires. Returns the value written at t = start.
 */
void runSweep(DSP::SweepEngine& engine, double start, double amplitude)
{
  (void)engine.advance(start - 0.001, -1.0);
  const double window = engine.activeWindow();
  const int steps     = 20;
  for (int i = 0; i <= steps; ++i) {
    const double t  = start + window * static_cast<double>(i) / steps;
    const double st = engine.advance(t, amplitude);
    if (st >= 0.0)
      engine.back[0].appendDecimated(st, amplitude);
  }

  (void)engine.advance(start + window * 1.05, amplitude);
}

}  // namespace

class TstSweepSegments : public QObject {
  Q_OBJECT

private slots:
  void retentionClampsToBudget();
  void completedSweepsAreRetainedNewestFirst();
  void segmentsAreDeepCopies();
  void wrapAndClear();
  void takeoverRequiresSameShape();
};

/**
 * @brief The requested count is honoured up to kMaxSegments and clamped by kMaxSegmentBytes.
 */
void TstSweepSegments::retentionClampsToBudget()
{
  DSP::SweepEngine engine;
  engine.configure(1, 1024, 1.0);
  QCOMPARE(engine.segmentCapacity(), 0);

  engine.setSegmentRetention(8);
  QCOMPARE(engine.segmentCapacity(), 8);
  QCOMPARE(engine.segmentCount(), 0);

  engine.setSegmentRetention(1000);
  QCOMPARE(engine.segmentCapacity(), DSP::SweepEngine::kMaxSegments);

  DSP::SweepEngine big;
  big.configure(4, 262144, 10.0);
  big.setSegmentRetention(64);
  const std::size_t perSegment = 4u * 262144u * DSP::SweepEngine::kSegmentSlotBytes;
  QVERIFY(static_cast<std::size_t>(big.segmentCapacity()) * perSegment
          <= DSP::SweepEngine::kMaxSegmentBytes);
  QVERIFY(big.segmentCapacity() >= 1);
  QVERIFY(big.segmentCapacity() < 64);

  engine.setSegmentRetention(0);
  QCOMPARE(engine.segmentCapacity(), 0);
}

/**
 * @brief Each completed sweep is retained with its trigger time; index 0 is the newest.
 */
void TstSweepSegments::completedSweepsAreRetainedNewestFirst()
{
  DSP::SweepEngine engine;
  engine.configure(1, 256, 0.01);
  engine.setTrigger(0.0, DSP::SweepEngine::kRising, DSP::SweepEngine::kNormal, 0.0, 0);
  engine.enabled = true;
  engine.setSegmentRetention(4);

  runSweep(engine, 1.0, 1.0);
  runSweep(engine, 2.0, 2.0);
  runSweep(engine, 3.0, 3.0);
  QCOMPARE(engine.segmentCount(), 3);

  const DSP::SweepSegment* newest = engine.segment(0);
  const DSP::SweepSegment* oldest = engine.segment(2);
  QVERIFY(newest != nullptr);
  QVERIFY(oldest != nullptr);
  QVERIFY(newest->triggerSec > oldest->triggerSec);
  QCOMPARE(newest->curves.size(), std::size_t(1));
  QVERIFY(newest->curves.front().value.size() > 0);
  QCOMPARE(newest->curves.front().value[0], 3.0);
  QCOMPARE(oldest->curves.front().value[0], 1.0);
  QVERIFY(engine.segment(3) == nullptr);
  QVERIFY(engine.segment(-1) == nullptr);
}

/**
 * @brief A retained segment owns its samples: the next capture reuses the front/back rings
 *        without touching it.
 */
void TstSweepSegments::segmentsAreDeepCopies()
{
  DSP::SweepEngine engine;
  engine.configure(1, 256, 0.01);
  engine.setTrigger(0.0, DSP::SweepEngine::kRising, DSP::SweepEngine::kNormal, 0.0, 0);
  engine.enabled = true;
  engine.setSegmentRetention(2);

  runSweep(engine, 1.0, 5.0);
  const DSP::SweepSegment* first = engine.segment(0);
  QVERIFY(first != nullptr);
  const std::size_t n = first->curves.front().value.size();
  QVERIFY(n > 0);

  runSweep(engine, 2.0, 7.0);
  runSweep(engine, 3.0, 9.0);
  const DSP::SweepSegment* stillFirst = engine.segment(1);
  QVERIFY(stillFirst != nullptr);
  QCOMPARE(stillFirst->curves.front().value[0], 7.0);
  QCOMPARE(engine.segment(0)->curves.front().value[0], 9.0);
  QVERIFY(engine.front[0].value.size() > 0);
  QVERIFY(engine.front[0].value.raw() != stillFirst->curves.front().value.raw());
}

/**
 * @brief Past the capacity the oldest segment is overwritten; clearSegments() empties the store
 *        without releasing it, and resetState() drops it too.
 */
void TstSweepSegments::wrapAndClear()
{
  DSP::SweepEngine engine;
  engine.configure(1, 256, 0.01);
  engine.setTrigger(0.0, DSP::SweepEngine::kRising, DSP::SweepEngine::kNormal, 0.0, 0);
  engine.enabled = true;
  engine.setSegmentRetention(3);

  for (int i = 1; i <= 5; ++i)
    runSweep(engine, static_cast<double>(i), static_cast<double>(i));

  QCOMPARE(engine.segmentCount(), 3);
  QCOMPARE(engine.segment(0)->curves.front().value[0], 5.0);
  QCOMPARE(engine.segment(2)->curves.front().value[0], 3.0);

  engine.clearSegments();
  QCOMPARE(engine.segmentCount(), 0);
  QCOMPARE(engine.segmentCapacity(), 3);

  runSweep(engine, 10.0, 1.0);
  QCOMPARE(engine.segmentCount(), 1);

  engine.resetState();
  QCOMPARE(engine.segmentCount(), 0);
}

/**
 * @brief A rebuilt engine takes another's segments over only when curve count and ring capacity
 *        match; otherwise it keeps the retention but starts empty.
 */
void TstSweepSegments::takeoverRequiresSameShape()
{
  DSP::SweepEngine source;
  source.configure(1, 256, 0.01);
  source.setTrigger(0.0, DSP::SweepEngine::kRising, DSP::SweepEngine::kNormal, 0.0, 0);
  source.enabled = true;
  source.setSegmentRetention(2);
  runSweep(source, 1.0, 4.0);
  QCOMPARE(source.segmentCount(), 1);

  DSP::SweepEngine same;
  same.configure(1, 256, 0.01);
  same.takeSegmentsFrom(source);
  QCOMPARE(same.segmentCapacity(), 2);
  QCOMPARE(same.segmentCount(), 1);
  QCOMPARE(same.segment(0)->curves.front().value[0], 4.0);

  DSP::SweepEngine wider;
  wider.configure(2, 256, 0.01);
  wider.takeSegmentsFrom(source);
  QCOMPARE(wider.segmentCapacity(), 2);
  QCOMPARE(wider.segmentCount(), 0);

  DSP::SweepEngine bigger;
  bigger.configure(1, 512, 0.01);
  bigger.takeSegmentsFrom(source);
  QCOMPARE(bigger.segmentCapacity(), 2);
  QCOMPARE(bigger.segmentCount(), 0);
}

QTEST_APPLESS_MAIN(TstSweepSegments)

#include "tst_sweep_segments.moc"
