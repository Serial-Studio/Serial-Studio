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
#include <QString>
#include <QTest>

#include "DataModel/DataBlock.h"

// The unified publication payload (spec 0055). Two properties carry the whole design: a block
// must reproduce every sample's time on both timebases, and a recycled block must not allocate --
// the pool exists so the publish tail costs nothing per frame, and a single share-assign into a
// producer's string would silently undo the span lane's zero-allocation steady state.

using namespace std::chrono_literals;

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a block with @p columns dataset columns, @p capacity sample slots, and either a
 *        uniform grid or explicit per-sample times.
 */
static DataModel::DataBlock makeBlock(int columns, qsizetype capacity, bool irregular, bool text)
{
  DataModel::DataBlock block;
  block.sourceId = 1;
  block.dt       = irregular ? 0ns : 100ns;

  for (int i = 0; i < columns; ++i) {
    DataModel::BlockColumn column;
    column.uniqueId = 1000 + i;
    column.hasText  = text;
    block.columns.push_back(column);
  }

  DataModel::size_block_storage(block, capacity, irregular);
  return block;
}

//--------------------------------------------------------------------------------------------------
// Test suite
//--------------------------------------------------------------------------------------------------

class DataBlockTest : public QObject {
  Q_OBJECT

private slots:
  void uniformGridDerivesSampleTimes();
  void irregularTimebaseUsesStoredOffsets();
  void sizingLeavesTheBlockEmpty();
  void numericColumnAllocatesNoTextStorage();
  void uniformGridStoresNoTimeArray();
  void samplesLandAtTheFillCursor();
  void textWriteKeepsTheSourceUniquelyOwned();
  void recycledBlockReusesItsStorage();
  void resetClearsTheGenerationButNotCapacity();
  void footprintTracksMaterialisedStorage();
  void numericFlagIsPerSample();
};

/**
 * @brief On a uniform grid sample i sits at t0 + i * dt, with no per-sample storage read.
 */
void DataBlockTest::uniformGridDerivesSampleTimes()
{
  auto block    = makeBlock(1, 8, false, false);
  block.samples = 8;
  block.dt      = 250ns;

  QVERIFY(DataModel::uniform_grid(block));
  QCOMPARE(DataModel::sample_offset_ns(block, 0), static_cast<qint64>(0));
  QCOMPARE(DataModel::sample_offset_ns(block, 1), static_cast<qint64>(250));
  QCOMPARE(DataModel::sample_offset_ns(block, 7), static_cast<qint64>(1750));

  const auto expected = block.t0 + 1750ns;
  QVERIFY(DataModel::sample_time(block, 7) == expected);
}

/**
 * @brief With dt == 0 the block is irregular and every sample's offset comes from times[], which
 *        is what lets a parsed frame keep the clock its driver stamped.
 */
void DataBlockTest::irregularTimebaseUsesStoredOffsets()
{
  auto block    = makeBlock(1, 4, true, false);
  block.samples = 3;

  DataModel::write_block_time(block, 0, 0);
  DataModel::write_block_time(block, 1, 17);
  DataModel::write_block_time(block, 2, 4096);

  QVERIFY(!DataModel::uniform_grid(block));
  QCOMPARE(DataModel::sample_offset_ns(block, 0), static_cast<qint64>(0));
  QCOMPARE(DataModel::sample_offset_ns(block, 1), static_cast<qint64>(17));
  QCOMPARE(DataModel::sample_offset_ns(block, 2), static_cast<qint64>(4096));
  QVERIFY(DataModel::sample_time(block, 2) == block.t0 + 4096ns);
}

/**
 * @brief Sizing allocates the storage but leaves the block empty; samples is a fill cursor, not
 *        a capacity.
 */
void DataBlockTest::sizingLeavesTheBlockEmpty()
{
  const auto block = makeBlock(3, 64, false, true);

  QCOMPARE(block.samples, static_cast<qsizetype>(0));
  QCOMPARE(block.columns.size(), static_cast<std::size_t>(3));
  for (const auto& column : block.columns) {
    QCOMPARE(column.values.size(), static_cast<std::size_t>(64));
    QCOMPARE(column.text.size(), static_cast<std::size_t>(64));
  }
}

/**
 * @brief A numeric column carries no text storage at all (spec-0055 D2: a dense source never
 *        produces a string, so a dense block must not pay for one).
 */
void DataBlockTest::numericColumnAllocatesNoTextStorage()
{
  auto block = makeBlock(2, 32, false, false);
  QVERIFY(block.columns[0].text.empty());
  QVERIFY(block.columns[1].text.empty());

  DataModel::write_block_sample(block.columns[0], 0, 1.5, QStringLiteral("ignored"), true);
  QVERIFY(block.columns[0].text.empty());
  QCOMPARE(block.columns[0].values[0], 1.5);
}

/**
 * @brief A uniform-grid block stores no time array; the offsets are derived, not recorded.
 */
void DataBlockTest::uniformGridStoresNoTimeArray()
{
  const auto uniform = makeBlock(1, 16, false, false);
  QVERIFY(uniform.times.empty());

  const auto irregular = makeBlock(1, 16, true, false);
  QCOMPARE(irregular.times.size(), static_cast<std::size_t>(16));
}

/**
 * @brief Writes land at the caller's fill cursor and leave later slots untouched.
 */
void DataBlockTest::samplesLandAtTheFillCursor()
{
  auto block = makeBlock(2, 8, false, false);

  for (qsizetype i = 0; i < 5; ++i) {
    DataModel::write_block_sample(block.columns[0], i, static_cast<double>(i), QString(), true);
    DataModel::write_block_sample(
      block.columns[1], i, static_cast<double>(i) * -2.0, QString(), true);
    ++block.samples;
  }

  QCOMPARE(block.samples, static_cast<qsizetype>(5));
  QCOMPARE(block.columns[0].values[4], 4.0);
  QCOMPARE(block.columns[1].values[4], -8.0);
}

/**
 * @brief The block copies text into its own buffer instead of sharing the producer's. A shared
 *        copy would leave the producer's dataset string non-unique, so the pipeline's next
 *        in-place write would detach and allocate -- the span lane's zero-allocation steady
 *        state undone by the consumer.
 */
void DataBlockTest::textWriteKeepsTheSourceUniquelyOwned()
{
  auto block = makeBlock(1, 4, false, true);

  QString producer = QStringLiteral("22.4");
  producer.reserve(64);
  const void* producerBuffer = producer.constData();

  DataModel::write_block_sample(block.columns[0], 0, 22.4, producer, true);

  QCOMPARE(block.columns[0].text[0], QStringLiteral("22.4"));
  QVERIFY(block.columns[0].text[0].constData() != producerBuffer);
  QVERIFY(producer.isDetached());

  producer.resize(3);
  QCOMPARE(producer.constData(), producerBuffer);
  QCOMPARE(block.columns[0].text[0], QStringLiteral("22.4"));
}

/**
 * @brief A recycled block writes into the same storage it already owns: no reallocation across
 *        fill/reset cycles, which is the whole reason blocks come from a pool.
 */
void DataBlockTest::recycledBlockReusesItsStorage()
{
  auto block = makeBlock(2, 16, false, true);

  const auto fill = [&block](double base) {
    for (qsizetype i = 0; i < 16; ++i) {
      const QString text = QStringLiteral("v%1").arg(i);
      DataModel::write_block_sample(block.columns[0], i, base + static_cast<double>(i), text, true);
      DataModel::write_block_sample(block.columns[1], i, base - static_cast<double>(i), text, true);
      ++block.samples;
    }
  };

  fill(0.0);
  const void* values0 = block.columns[0].values.data();
  const void* values1 = block.columns[1].values.data();
  const void* text0   = block.columns[0].text[0].constData();

  DataModel::reset_block(block);
  fill(100.0);

  QCOMPARE(block.columns[0].values.data(), values0);
  QCOMPARE(block.columns[1].values.data(), values1);
  QCOMPARE(block.columns[0].text[0].constData(), text0);
  QCOMPARE(block.columns[0].values[15], 115.0);
}

/**
 * @brief Reset empties the block and drops its generation binding, but never releases storage.
 */
void DataBlockTest::resetClearsTheGenerationButNotCapacity()
{
  auto block                = makeBlock(1, 32, false, false);
  block.samples             = 32;
  block.blockNumber         = 7;
  block.structureGeneration = 12;

  const auto capacity = block.columns[0].values.capacity();
  DataModel::reset_block(block);

  QCOMPARE(block.samples, static_cast<qsizetype>(0));
  QCOMPARE(block.blockNumber, static_cast<quint64>(0));
  QCOMPARE(block.structureGeneration, static_cast<quint64>(0));
  QCOMPARE(block.columns[0].values.capacity(), capacity);
}

/**
 * @brief The footprint accounting sees the storage the pool budget has to bound.
 */
void DataBlockTest::footprintTracksMaterialisedStorage()
{
  const auto small = makeBlock(2, 16, false, false);
  const auto large = makeBlock(2, 4096, false, false);

  QVERIFY(DataModel::block_footprint_bytes(large) > DataModel::block_footprint_bytes(small));
  QVERIFY(DataModel::block_footprint_bytes(large)
          >= static_cast<std::size_t>(2 * 4096 * sizeof(double)));
}

/**
 * @brief The parsed-as-number flag is per sample, not per column: one channel can parse as a
 *        number on one frame and as text on the next, and the widgets branch on it. A column that
 *        stores no flags is a dense source's and reads as numeric throughout (spec 0055 D2).
 */
void DataBlockTest::numericFlagIsPerSample()
{
  auto textual = makeBlock(1, 4, false, true);
  DataModel::write_block_sample(textual.columns[0], 0, 1.0, QStringLiteral("1"), true);
  DataModel::write_block_sample(textual.columns[0], 1, 0.0, QStringLiteral("n/a"), false);
  textual.samples = 2;

  QVERIFY(DataModel::sample_is_numeric(textual.columns[0], 0));
  QVERIFY(!DataModel::sample_is_numeric(textual.columns[0], 1));

  auto dense = makeBlock(1, 4, false, false);
  QVERIFY(dense.columns[0].numeric.empty());
  QVERIFY(DataModel::sample_is_numeric(dense.columns[0], 0));
}

QTEST_APPLESS_MAIN(DataBlockTest)

#include "tst_data_block.moc"
