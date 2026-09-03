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

#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder/BlockStager.h"

// Block staging is the frame lane's producer: cap and epoch flushes, the mask split, the
// disconnect/pause tail, the structureGeneration stamp and the pool's use_count()==1 free probe
// (spec 0055, spec 0075 A2). The stager reaches the facade only through BlockStagerHost, so this
// suite substitutes a stub host and never links FrameBuilder's dependency web. The builder-side
// half of the boundary contract -- that emitSessionBoundary() flushes BEFORE the signal reaches
// the sinks -- is pinned end to end by tests/integration/test_recording_fidelity.py.

using namespace DataModel;

namespace {

/**
 * @brief Records what the stager asked its facade to do, and hands out a flush epoch the test
 *        controls, so the display tick is a value in this suite rather than a timer.
 */
class StubStagerHost : public BlockStagerHost {
public:
  StubStagerHost() : m_epoch(1), m_exhausted(0) {}

  void noteStagingPoolExhausted() override { ++m_exhausted; }

  void publishStagedBlock(const DataBlockPtr& block) override { m_published.push_back(block); }

  void announceStructure(int sourceId, const Frame&) override { m_announced.push_back(sourceId); }

  [[nodiscard]] quint64 stagingFlushEpoch() const override { return m_epoch; }

  void bumpEpoch() { ++m_epoch; }

  [[nodiscard]] int exhausted() const { return m_exhausted; }

  [[nodiscard]] const std::vector<int>& announced() const { return m_announced; }

  [[nodiscard]] const std::vector<DataBlockPtr>& published() const { return m_published; }

private:
  quint64 m_epoch;
  int m_exhausted;
  std::vector<int> m_announced;
  std::vector<DataBlockPtr> m_published;
};

/**
 * @brief Builds a one-group frame with @p datasets numeric datasets for @p sourceId.
 */
[[nodiscard]] Frame makeFrame(int sourceId, int datasets)
{
  Frame frame;
  frame.sourceId = sourceId;
  frame.title    = QStringLiteral("staging");

  Group group;
  group.groupId  = 0;
  group.sourceId = sourceId;
  group.title    = QStringLiteral("group");
  for (int i = 0; i < datasets; ++i) {
    Dataset dataset;
    dataset.uniqueId     = sourceId * 100 + i;
    dataset.index        = i + 1;
    dataset.isNumeric    = true;
    dataset.numericValue = 0.0;
    group.datasets.push_back(dataset);
  }

  frame.groups.push_back(group);
  return frame;
}

/**
 * @brief Writes @p value into every dataset of @p frame, the way a parse pass would.
 */
void fillFrame(Frame& frame, double value)
{
  for (auto& group : frame.groups)
    for (auto& dataset : group.datasets) {
      dataset.numericValue    = value;
      dataset.rawNumericValue = value;
      dataset.isNumeric       = true;
      dataset.value           = QString::number(value);
      dataset.rawValue        = dataset.value;
    }
}

/**
 * @brief Steady time point @p offsetNs after a fixed baseline; the suite never reads a wall clock.
 */
[[nodiscard]] DataBlock::SteadyTimePoint at(qint64 offsetNs)
{
  return DataBlock::SteadyTimePoint{} + std::chrono::nanoseconds(offsetNs);
}

}  // namespace

/**
 * @brief Contract of DataModel::BlockStager: flush triggers, mask separation, generation stamping
 *        and pool exhaustion.
 */
class TstFrameBuilderStaging : public QObject {
  Q_OBJECT

private slots:
  void structureIsAnnouncedBeforeEveryStagedRow();
  void samplesAccumulateUntilTheSampleCap();
  void anEpochChangeClosesTheOpenBlock();
  void flushAllPublishesThePartialTail();
  void flushAllOnAnEmptyStagerPublishesNothing();
  void publishedBlocksCarryTheCurrentStructureGeneration();
  void aMaskChangeFlushesTheUnmaskedBlockFirst();
  void blockNumbersAreMonotonicPerSource();
  void perSampleTimesAreOffsetsFromTheFirstSample();
  void aFullPoolDropsTheBatchAndNotesExhaustion();
  void releaseIdleStorageKeepsBusySlots();
};

/**
 * @brief A source's structure must reach the consumers before its values do; the stager asks on
 *        every staged row and the facade is what makes that a no-op after the first.
 */
void TstFrameBuilderStaging::structureIsAnnouncedBeforeEveryStagedRow()
{
  StubStagerHost host;
  quint64 generation = 7;
  bool masked        = false;
  BlockStager stager(host, generation, masked);

  Frame frame = makeFrame(3, 2);
  fillFrame(frame, 1.0);
  stager.stage(3, frame, at(0));
  stager.stage(3, frame, at(1000));

  QCOMPARE(host.announced().size(), std::size_t(2));
  QCOMPARE(host.announced().front(), 3);
  QCOMPARE(host.announced().back(), 3);
}

/**
 * @brief The frame lane flushes at kFrameBlockSampleCap; the cap sample is the last of the block,
 *        and the next row opens a fresh one.
 */
void TstFrameBuilderStaging::samplesAccumulateUntilTheSampleCap()
{
  StubStagerHost host;
  quint64 generation = 1;
  bool masked        = false;
  BlockStager stager(host, generation, masked);

  Frame frame = makeFrame(0, 3);
  for (qsizetype i = 0; i < BlockStager::kFrameBlockSampleCap; ++i) {
    fillFrame(frame, static_cast<double>(i));
    stager.stage(0, frame, at(i * 1000));
  }

  QCOMPARE(host.published().size(), std::size_t(1));
  QCOMPARE(host.published().front()->samples, BlockStager::kFrameBlockSampleCap);
  QCOMPARE(host.published().front()->columns.size(), std::size_t(3));
  QCOMPARE(host.published().front()->columns[0].values[0], 0.0);

  fillFrame(frame, 999.0);
  stager.stage(0, frame, at(100000));
  QCOMPARE(host.published().size(), std::size_t(1));

  stager.flushAll();
  QCOMPARE(host.published().size(), std::size_t(2));
  QCOMPARE(host.published().back()->samples, qsizetype(1));
  QCOMPARE(host.published().back()->columns[0].values[0], 999.0);
}

/**
 * @brief The display tick advances the flush epoch, and a producing source closes its block on the
 *        next row it stages -- that is what bounds latency without a timer on the pipeline thread.
 */
void TstFrameBuilderStaging::anEpochChangeClosesTheOpenBlock()
{
  StubStagerHost host;
  quint64 generation = 1;
  bool masked        = false;
  BlockStager stager(host, generation, masked);

  Frame frame = makeFrame(0, 1);
  fillFrame(frame, 4.0);
  stager.stage(0, frame, at(0));
  QCOMPARE(host.published().size(), std::size_t(0));

  host.bumpEpoch();
  fillFrame(frame, 5.0);
  stager.stage(0, frame, at(1000));

  QCOMPARE(host.published().size(), std::size_t(1));
  QCOMPARE(host.published().front()->samples, qsizetype(2));
}

/**
 * @brief The disconnect/pause tail (A2): a partial block must reach the sinks when the session
 *        ends, not sit in the stager until the next session opens.
 */
void TstFrameBuilderStaging::flushAllPublishesThePartialTail()
{
  StubStagerHost host;
  quint64 generation = 1;
  bool masked        = false;
  BlockStager stager(host, generation, masked);

  Frame first  = makeFrame(0, 1);
  Frame second = makeFrame(1, 1);
  fillFrame(first, 1.0);
  fillFrame(second, 2.0);
  stager.stage(0, first, at(0));
  stager.stage(1, second, at(0));
  QCOMPARE(host.published().size(), std::size_t(0));

  stager.flushAll();

  QCOMPARE(host.published().size(), std::size_t(2));
  QCOMPARE(host.published().front()->samples, qsizetype(1));
  QCOMPARE(host.published().back()->samples, qsizetype(1));
}

/**
 * @brief A boundary with nothing staged publishes nothing: an empty block would create a file for
 *        a session that produced no samples.
 */
void TstFrameBuilderStaging::flushAllOnAnEmptyStagerPublishesNothing()
{
  StubStagerHost host;
  quint64 generation = 1;
  bool masked        = false;
  BlockStager stager(host, generation, masked);

  stager.flushAll();
  stager.flush(0);

  QCOMPARE(host.published().size(), std::size_t(0));
}

/**
 * @brief Every publish site stamps the pool generation; a block left at 0 makes the dashboard
 *        either revalidate per block or never reconfigure after a layout change.
 */
void TstFrameBuilderStaging::publishedBlocksCarryTheCurrentStructureGeneration()
{
  StubStagerHost host;
  quint64 generation = 5;
  bool masked        = false;
  BlockStager stager(host, generation, masked);

  Frame frame = makeFrame(0, 1);
  fillFrame(frame, 1.0);
  stager.stage(0, frame, at(0));
  stager.flushAll();

  QCOMPARE(host.published().front()->structureGeneration, quint64(5));

  ++generation;
  fillFrame(frame, 2.0);
  stager.stage(0, frame, at(1000));
  stager.flushAll();

  QCOMPARE(host.published().size(), std::size_t(2));
  QCOMPARE(host.published().back()->structureGeneration, quint64(6));
}

/**
 * @brief A block must never mix masked and unmasked samples: the mask rides on the block, and a
 *        masked (replay or synthetic) row would otherwise leak real captured samples past the
 *        recording sinks or drop them from every recording.
 */
void TstFrameBuilderStaging::aMaskChangeFlushesTheUnmaskedBlockFirst()
{
  StubStagerHost host;
  quint64 generation = 1;
  bool masked        = false;
  BlockStager stager(host, generation, masked);

  Frame frame = makeFrame(0, 1);
  fillFrame(frame, 1.0);
  stager.stage(0, frame, at(0));

  masked = true;
  fillFrame(frame, 2.0);
  stager.stage(0, frame, at(1000));

  QCOMPARE(host.published().size(), std::size_t(1));
  QCOMPARE(host.published().front()->masked, false);
  QCOMPARE(host.published().front()->samples, qsizetype(1));

  stager.flushAll();
  QCOMPARE(host.published().size(), std::size_t(2));
  QCOMPARE(host.published().back()->masked, true);
}

/**
 * @brief Block numbers are per source and strictly increasing; the republish path compares them
 *        before/after to decide whether a synthetic refresh actually published anything.
 */
void TstFrameBuilderStaging::blockNumbersAreMonotonicPerSource()
{
  StubStagerHost host;
  quint64 generation = 1;
  bool masked        = false;
  BlockStager stager(host, generation, masked);

  Frame a = makeFrame(0, 1);
  Frame b = makeFrame(1, 1);
  fillFrame(a, 1.0);
  fillFrame(b, 1.0);

  QCOMPARE(stager.blockNumber(0), quint64(0));

  stager.stage(0, a, at(0));
  stager.flush(0);
  stager.stage(0, a, at(1000));
  stager.flush(0);
  stager.stage(1, b, at(0));
  stager.flush(1);

  QCOMPARE(stager.blockNumber(0), quint64(2));
  QCOMPARE(stager.blockNumber(1), quint64(1));
  QCOMPARE(host.published().size(), std::size_t(3));
}

/**
 * @brief Source owns time: the frame lane is irregular, so each sample keeps its own offset from
 *        the block's t0 instead of a derived grid.
 */
void TstFrameBuilderStaging::perSampleTimesAreOffsetsFromTheFirstSample()
{
  StubStagerHost host;
  quint64 generation = 1;
  bool masked        = false;
  BlockStager stager(host, generation, masked);

  Frame frame = makeFrame(0, 1);
  fillFrame(frame, 1.0);
  stager.stage(0, frame, at(1'000'000));
  stager.stage(0, frame, at(1'000'500));
  stager.stage(0, frame, at(1'003'000));
  stager.flushAll();

  const auto& block = *host.published().front();
  QVERIFY(!DataModel::uniform_grid(block));
  QCOMPARE(block.t0, at(1'000'000));
  QCOMPARE(DataModel::sample_offset_ns(block, 0), qint64(0));
  QCOMPARE(DataModel::sample_offset_ns(block, 1), qint64(500));
  QCOMPARE(DataModel::sample_offset_ns(block, 2), qint64(3000));
}

/**
 * @brief With every slot still referenced by a consumer there is no heap fallback: the batch is
 *        dropped from every sink and the facade is told once, which is the documented behaviour.
 */
void TstFrameBuilderStaging::aFullPoolDropsTheBatchAndNotesExhaustion()
{
  StubStagerHost host;
  quint64 generation = 1;
  bool masked        = false;
  BlockStager stager(host, generation, masked);

  Frame frame = makeFrame(0, 1);
  fillFrame(frame, 1.0);

  for (int i = 0; i < BlockStager::kBlockPoolSlots; ++i) {
    stager.stage(0, frame, at(i * 1000));
    stager.flush(0);
  }

  QCOMPARE(host.published().size(), std::size_t(BlockStager::kBlockPoolSlots));
  QCOMPARE(host.exhausted(), 0);

  stager.stage(0, frame, at(999'000));
  QCOMPARE(host.exhausted(), 1);
  QCOMPARE(host.published().size(), std::size_t(BlockStager::kBlockPoolSlots));
}

/**
 * @brief Storage is handed back only for slots no consumer still holds; a busy slot keeps its
 *        columns, since releasing them under a live reader would free what it is reading.
 */
void TstFrameBuilderStaging::releaseIdleStorageKeepsBusySlots()
{
  StubStagerHost host;
  quint64 generation = 1;
  bool masked        = false;
  BlockStager stager(host, generation, masked);

  Frame frame = makeFrame(0, 2);
  fillFrame(frame, 3.0);
  stager.stage(0, frame, at(0));
  stager.flushAll();

  const auto held = host.published().front();
  QCOMPARE(held->columns.size(), std::size_t(2));

  stager.releaseIdleStorage();

  QCOMPARE(held->columns.size(), std::size_t(2));
  QCOMPARE(held->samples, qsizetype(1));
  QCOMPARE(held->columns[0].values[0], 3.0);
}

QTEST_APPLESS_MAIN(TstFrameBuilderStaging)

#include "tst_frame_builder_staging.moc"
