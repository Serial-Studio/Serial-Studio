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

#include <chrono>
#include <QTest>
#include <vector>

#include "DataModel/DataBlock.h"
#include "DataModel/FrameConsumer.h"

// Source owns time (spec 0075 B1). CSV::ExportWorker::bufferBlock and MDF4::ExportWorker::
// writeBlockSample both used ONE worker-wide monotonic clock for irregular blocks, so with two
// frame-lane sources flushed on the same tick the second source's instants were rewritten into a
// nanosecond staircase pinned to the first source's tail -- the sparse merger and MDF4's per-group
// masters both defeated. Both workers now compute t0 - reference + times[i] and take
// FrameConsumerWorkerBase::monotonicSourceNs only as a per-source same-instant tie-break, which is
// the shared helper this suite pins. Writing the rows themselves needs QFile, the workspace manager
// and mdflib, so the file-level assertions live in tests/integration/test_recording_fidelity.py.

using namespace DataModel;

namespace {

/**
 * @brief Minimal concrete worker: the pure virtuals are unreachable here, so each is a no-op that
 *        exists only to make the base class instantiable.
 */
class ConcreteWorker : public FrameConsumerWorkerBase {
public:
  ConcreteWorker() : FrameConsumerWorkerBase(nullptr) {}

  void processData() override {}

  void close() override {}

  void flush() override {}
};

/**
 * @brief Runs the exporters' per-sample time computation over one block: the sample's own instant
 *        relative to the session reference, with the per-source tie-break on the irregular lane.
 */
[[nodiscard]] std::vector<qint64> exportTimes(ConcreteWorker& worker,
                                              const DataBlock& block,
                                              DataBlock::SteadyTimePoint reference)
{
  std::vector<qint64> times;
  times.reserve(static_cast<std::size_t>(block.samples));
  for (qsizetype i = 0; i < block.samples; ++i) {
    const auto stamp = sample_time(block, i);
    qint64 offset = std::chrono::duration_cast<std::chrono::nanoseconds>(stamp - reference).count();
    if (!uniform_grid(block))
      offset = worker.monotonicSourceNs(block.sourceId, offset);

    times.push_back(std::max<qint64>(0, offset));
  }

  return times;
}

/**
 * @brief Builds an irregular block for @p sourceId whose samples sit at @p offsets from @p t0.
 */
[[nodiscard]] DataBlock makeIrregularBlock(int sourceId,
                                           DataBlock::SteadyTimePoint t0,
                                           const std::vector<qint64>& offsets)
{
  DataBlock block;
  block.sourceId = sourceId;
  block.t0       = t0;
  block.dt       = std::chrono::nanoseconds(0);
  block.times    = offsets;
  block.samples  = static_cast<qsizetype>(offsets.size());

  BlockColumn column;
  column.uniqueId = sourceId * 10;
  column.values.assign(offsets.size(), 1.0);
  block.columns.push_back(std::move(column));
  return block;
}

/**
 * @brief Steady time point @p ns after a fixed baseline.
 */
[[nodiscard]] DataBlock::SteadyTimePoint at(qint64 ns)
{
  return DataBlock::SteadyTimePoint{} + std::chrono::nanoseconds(ns);
}

}  // namespace

/**
 * @brief Contract of the export workers' per-source time column.
 */
class TstCsvExportTimes : public QObject {
  Q_OBJECT

private slots:
  void eachSourceKeepsItsOwnInstants();
  void oneSourcesCollisionsDoNotShiftAnother();
  void aUniformGridNeverTakesTheTieBreak();
  void sameInstantWithinOneSourceStaysDistinct();
  void aSessionResetClearsEverySourcesClock();
  void interleavedSourcesStayMonotonicPerSource();
};

/**
 * @brief Two sources flushed in one batch keep the instants they stamped: source B's samples must
 *        not be pushed past source A's tail. This is the B1 failure reproduced directly.
 */
void TstCsvExportTimes::eachSourceKeepsItsOwnInstants()
{
  ConcreteWorker worker;
  const auto reference = at(1'000'000);

  const auto a = makeIrregularBlock(0, at(1'000'000), {0, 1000, 2000});
  const auto b = makeIrregularBlock(1, at(1'000'500), {0, 1000, 2000});

  const auto timesA = exportTimes(worker, a, reference);
  const auto timesB = exportTimes(worker, b, reference);

  QCOMPARE(timesA, (std::vector<qint64>{0, 1000, 2000}));
  QCOMPARE(timesB, (std::vector<qint64>{500, 1500, 2500}));
}

/**
 * @brief A source that collides with itself is bumped; the other source's identical instants are
 *        untouched, because the tie-break state is keyed per source.
 */
void TstCsvExportTimes::oneSourcesCollisionsDoNotShiftAnother()
{
  ConcreteWorker worker;
  const auto reference = at(0);

  const auto a = makeIrregularBlock(0, at(0), {100, 100, 100});
  const auto b = makeIrregularBlock(1, at(0), {100, 200});

  const auto timesA = exportTimes(worker, a, reference);
  const auto timesB = exportTimes(worker, b, reference);

  QCOMPARE(timesA, (std::vector<qint64>{100, 101, 102}));
  QCOMPARE(timesB, (std::vector<qint64>{100, 200}));
}

/**
 * @brief A uniform-grid block derives its offsets exactly and must never be bumped: its cadence is
 *        the source's own, and a 1 ns nudge would make a 48 kHz grid non-uniform.
 */
void TstCsvExportTimes::aUniformGridNeverTakesTheTieBreak()
{
  ConcreteWorker worker;

  DataBlock block;
  block.sourceId = 4;
  block.t0       = at(2000);
  block.dt       = std::chrono::nanoseconds(500);
  block.samples  = 3;

  BlockColumn column;
  column.uniqueId = 1;
  column.values.assign(3, 0.0);
  block.columns.push_back(std::move(column));

  const auto first  = exportTimes(worker, block, at(2000));
  const auto second = exportTimes(worker, block, at(2000));

  QCOMPARE(first, (std::vector<qint64>{0, 500, 1000}));
  QCOMPARE(second, first);
}

/**
 * @brief Two frames of one source landing on the same coarse-clock nanosecond stay distinct rows;
 *        without the bump the sparse merger would coalesce them and one frame would be lost.
 */
void TstCsvExportTimes::sameInstantWithinOneSourceStaysDistinct()
{
  ConcreteWorker worker;

  QCOMPARE(worker.monotonicSourceNs(0, 5000), qint64(5000));
  QCOMPARE(worker.monotonicSourceNs(0, 5000), qint64(5001));
  QCOMPARE(worker.monotonicSourceNs(0, 4999), qint64(5002));
  QCOMPARE(worker.monotonicSourceNs(0, 9000), qint64(9000));
}

/**
 * @brief The clock is session state: a new recording restarts every source at its own instants
 *        instead of continuing past the previous session's tail.
 */
void TstCsvExportTimes::aSessionResetClearsEverySourcesClock()
{
  ConcreteWorker worker;

  QCOMPARE(worker.monotonicSourceNs(0, 1000), qint64(1000));
  QCOMPARE(worker.monotonicSourceNs(1, 7000), qint64(7000));

  worker.resetMonotonicClock();

  QCOMPARE(worker.monotonicSourceNs(0, 10), qint64(10));
  QCOMPARE(worker.monotonicSourceNs(1, 10), qint64(10));
}

/**
 * @brief Blocks arriving interleaved (the multi-source steady state) keep each source's column
 *        strictly increasing on its own terms.
 */
void TstCsvExportTimes::interleavedSourcesStayMonotonicPerSource()
{
  ConcreteWorker worker;
  const auto reference = at(0);

  std::vector<qint64> sourceZero;
  std::vector<qint64> sourceOne;
  for (int round = 0; round < 4; ++round) {
    const qint64 base = round * 1000;
    const auto a      = makeIrregularBlock(0, at(base), {0, 10});
    const auto b      = makeIrregularBlock(1, at(base), {0, 10});

    for (const qint64 ns : exportTimes(worker, a, reference))
      sourceZero.push_back(ns);

    for (const qint64 ns : exportTimes(worker, b, reference))
      sourceOne.push_back(ns);
  }

  QCOMPARE(sourceZero, sourceOne);
  for (std::size_t i = 1; i < sourceZero.size(); ++i)
    QVERIFY(sourceZero[i] > sourceZero[i - 1]);
}

QTEST_APPLESS_MAIN(TstCsvExportTimes)

#include "tst_csv_export_times.moc"
