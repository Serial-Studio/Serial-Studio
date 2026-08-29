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

#include <QObject>
#include <QSet>
#include <QTest>

#include "DataModel/FrameBuilder/TableSnapshotChannel.h"
#include "IO/PipelineHost.h"

// The dataset the fixture store mirrors; the value is addressed by uniqueId, never by index
static constexpr int kDatasetId = 7;

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

QTEST_GUILESS_MAIN(TstTableSnapshotChannel)

#include "tst_table_snapshot_channel.moc"
