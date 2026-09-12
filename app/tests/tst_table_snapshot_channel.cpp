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

#include <atomic>
#include <cstdlib>
#include <new>
#include <QObject>
#include <QSet>
#include <QTest>

#include "DataModel/FrameBuilder/TableSnapshotChannel.h"
#include "IO/PipelineHost.h"

// The dataset the fixture store mirrors; the value is addressed by uniqueId, never by index
static constexpr int kDatasetId    = 7;
static constexpr int kSteadyWrites = 1000;
static std::atomic<long> g_allocations{0};

/**
 * @brief Counting replacement for the TU's global operator new, so the spec-0086 slot-write case
 *        can assert that a steady stream of equal-length strings never touches the heap.
 */
void* operator new(std::size_t size)
{
  g_allocations.fetch_add(1, std::memory_order_relaxed);
  if (void* p = std::malloc(size ? size : 1))
    return p;

  throw std::bad_alloc();
}

/**
 * @brief Matching delete for the counting operator new.
 */
void operator delete(void* p) noexcept
{
  std::free(p);
}

/**
 * @brief Matching sized delete for the counting operator new.
 */
void operator delete(void* p, std::size_t) noexcept
{
  std::free(p);
}

/**
 * @brief The two out-of-line statics DataTable.cpp reaches through readTableView/writeTableStore.
 *        Stubbing them here is what keeps the link set at the store TU instead of dragging the
 *        pipeline host and every module it composes; neither routing path runs in this suite,
 *        which drives both halves of the channel from one thread.
 */
bool IO::PipelineHost::pipelineParkedOnGui() noexcept
{
  return false;
}

bool IO::PipelineHost::tearingDown() noexcept
{
  return false;
}

/**
 * @brief Builds a store holding the raw/final mirror registers of one dataset, which is the
 *        smallest state a published snapshot can differ in.
 */
static void primeStore(DataModel::DataTableStore& store)
{
  DataModel::Dataset dataset;
  dataset.uniqueId = kDatasetId;

  DataModel::Group group;
  group.datasets.push_back(dataset);

  DataModel::Frame templateFrame;
  templateFrame.groups.push_back(group);

  store.initialize({}, {}, templateFrame);
}

/**
 * @brief Builder/GUI handshake of DataModel::TableSnapshotChannel. Both halves are driven from the
 *        test thread on purpose: it is simultaneously qApp's thread (what the GUI half asserts on)
 *        and the owner's thread (what the builder half asserts on), so the ordering the two atomics
 *        encode can be exercised without a second thread hiding it behind a race.
 */
class TstTableSnapshotChannel : public QObject {
  Q_OBJECT

private slots:
  void drainStaysIdleUntilArmed();
  void requestIsLatchedUntilPublished();
  void publishedSnapshotReachesTheGui();
  void unchangedStoreDoesNotRepublish();
  void slotsAreRecycled();
  void slotWritesReachTheIdPath();
  void steadySlotWritesDoNotAllocate();
};

/**
 * @brief A session with no GUI-thread table reader must never make the builder copy the store.
 */
void TstTableSnapshotChannel::drainStaysIdleUntilArmed()
{
  QObject owner;
  DataModel::DataTableStore store;
  DataModel::TableSnapshotChannel channel(owner, store);

  primeStore(store);

  QVERIFY(!channel.drainForGui());
  QVERIFY(!channel.guiSnapshot());
}

/**
 * @brief Once armed, the first drain claims the publish request and later drains report false
 *        until the builder half consumed it: the request is a latch, not a per-tick message.
 */
void TstTableSnapshotChannel::requestIsLatchedUntilPublished()
{
  QObject owner;
  DataModel::DataTableStore store;
  DataModel::TableSnapshotChannel channel(owner, store);

  primeStore(store);
  channel.noteGuiUser();

  QVERIFY(channel.drainForGui());
  QVERIFY(!channel.drainForGui());

  channel.publish();
  QVERIFY(channel.drainForGui());
}

/**
 * @brief The round trip: a store write, a builder publish, and the GUI drain that adopts it.
 */
void TstTableSnapshotChannel::publishedSnapshotReachesTheGui()
{
  QObject owner;
  DataModel::DataTableStore store;
  DataModel::TableSnapshotChannel channel(owner, store);

  primeStore(store);
  channel.noteGuiUser();
  store.setDatasetFinal(kDatasetId, 42.0, QStringLiteral("42"), true);

  channel.publish();
  QVERIFY(channel.drainForGui());

  const auto snapshot = channel.guiSnapshot();
  QVERIFY(snapshot);

  const auto* value = snapshot->getDatasetFinal(kDatasetId);
  QVERIFY(value != nullptr);
  QCOMPARE(value->numericValue, 42.0);
}

/**
 * @brief A publish with neither a generation nor a write-clock move must enqueue nothing: the
 *        display tick asks every frame-time, and copying the store each time is the cost this
 *        comparison exists to avoid.
 */
void TstTableSnapshotChannel::unchangedStoreDoesNotRepublish()
{
  QObject owner;
  DataModel::DataTableStore store;
  DataModel::TableSnapshotChannel channel(owner, store);

  primeStore(store);
  channel.noteGuiUser();
  store.setDatasetFinal(kDatasetId, 1.0, QStringLiteral("1"), true);

  channel.publish();
  QVERIFY(channel.drainForGui());

  const auto first = channel.guiSnapshot();
  QVERIFY(first);

  channel.publish();
  QVERIFY(channel.drainForGui());
  QCOMPARE(channel.guiSnapshot().get(), first.get());
}

/**
 * @brief Repeated publishes must cycle a bounded pool instead of allocating a snapshot per tick:
 *        the GUI releases each adopted snapshot when it adopts the next, so the slots come back.
 */
void TstTableSnapshotChannel::slotsAreRecycled()
{
  QObject owner;
  DataModel::DataTableStore store;
  DataModel::TableSnapshotChannel channel(owner, store);

  primeStore(store);
  channel.noteGuiUser();

  QSet<const void*> seen;
  for (int i = 0; i < 64; ++i) {
    store.setDatasetFinal(kDatasetId, static_cast<double>(i), QString::number(i), true);
    channel.publish();
    QVERIFY(channel.drainForGui());
    QVERIFY(channel.guiSnapshot());
    seen.insert(channel.guiSnapshot().get());
  }

  QVERIFY(seen.size() <= 8);

  const auto* newest = channel.guiSnapshot()->getDatasetFinal(kDatasetId);
  QVERIFY(newest != nullptr);
  QCOMPARE(newest->numericValue, 63.0);
}

/**
 * @brief The slot-addressed writers (spec 0086) land in the same registers the id-based writers
 *        and readers use, and an unknown dataset resolves to the {-1, -1} pair that every write
 *        ignores.
 */
void TstTableSnapshotChannel::slotWritesReachTheIdPath()
{
  DataModel::DataTableStore store;
  primeStore(store);

  const auto table_slots = store.datasetSlots(kDatasetId);
  QVERIFY(table_slots.first >= 0);
  QVERIFY(table_slots.second >= 0);
  QVERIFY(table_slots.first != table_slots.second);
  QCOMPARE(store.datasetSlots(kDatasetId + 1), (std::pair<int, int>{-1, -1}));

  const quint64 clock = store.writeClock();
  store.setDatasetRawAt(table_slots.first, 1.5, QStringLiteral("1.5"), true);
  store.setDatasetFinalAt(table_slots.second, 0.0, QStringLiteral("text"), false);
  QCOMPARE(store.writeClock(), clock + 2);

  const auto* raw = store.getDatasetRaw(kDatasetId);
  QVERIFY(raw != nullptr);
  QVERIFY(raw->isNumeric);
  QCOMPARE(raw->numericValue, 1.5);
  const auto* fin = store.getDatasetFinal(kDatasetId);
  QVERIFY(fin != nullptr);
  QVERIFY(!fin->isNumeric);
  QCOMPARE(fin->stringValue, QStringLiteral("text"));

  store.setDatasetRawAt(table_slots.first, 1.5, QStringLiteral("1.5"), true);
  QCOMPARE(store.writeClock(), clock + 2);
  store.setDatasetRawAt(-1, 9.0, QStringLiteral("9"), true);
  store.setDatasetFinalAt(1 << 20, 9.0, QStringLiteral("9"), true);
  QCOMPARE(store.writeClock(), clock + 2);
}

/**
 * @brief R8: the producer mutates one scratch string in place per frame, the way the builder's
 *        dataset value is written. A share-assigning store would link its register to that scratch
 *        and force a detach (one malloc) on every following frame; the in-place copy keeps both
 *        buffers private, so a thousand changing writes allocate nothing after the warm-up.
 */
void TstTableSnapshotChannel::steadySlotWritesDoNotAllocate()
{
  DataModel::DataTableStore store;
  primeStore(store);
  const auto table_slots = store.datasetSlots(kDatasetId);
  QVERIFY(table_slots.second >= 0);

  QString scratch = QStringLiteral("12.5");
  scratch.detach();
  for (int warm = 0; warm < 2; ++warm) {
    scratch[0] = QChar('1' + warm);
    store.setDatasetFinalAt(table_slots.second, 12.5 + warm, scratch, true);
  }

  const quint64 clock                = store.writeClock();
  const long before                  = g_allocations.load(std::memory_order_relaxed);
  const QChar* const register_buffer = store.getDatasetFinal(kDatasetId)->stringValue.constData();
  for (int i = 0; i < kSteadyWrites; ++i) {
    scratch[0] = QChar('1' + (i % 2));
    store.setDatasetFinalAt(table_slots.second, 12.5 + (i % 2), scratch, true);
    QCOMPARE(store.getDatasetFinal(kDatasetId)->stringValue.constData(), register_buffer);
  }

  QCOMPARE(g_allocations.load(std::memory_order_relaxed) - before, 0L);
  QCOMPARE(store.writeClock(), clock + kSteadyWrites);
  QCOMPARE(store.getDatasetFinal(kDatasetId)->stringValue, QStringLiteral("22.5"));
  QVERIFY(store.getDatasetFinal(kDatasetId)->stringValue.constData() != scratch.constData());
}

QTEST_GUILESS_MAIN(TstTableSnapshotChannel)

#include "tst_table_snapshot_channel.moc"
