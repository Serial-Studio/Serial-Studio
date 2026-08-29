/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <array>
#include <QTest>
#include <vector>

#include "Sessions/Player/ReplayAlignment.h"

// Every test function here is self-contained: each builds its own layout, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Column-alignment arithmetic of the historian player: the order a recording's columns are
 *        replayed in, the per-source maps built from it, and the timeline merge that lets a
 *        stream-only session advance.
 */
class TstSessionAlignment : public QObject {
  Q_OBJECT

private slots:
  void indexColumns();

  void alignSortsBySourceThenIndex();
  void alignKeepsOrphansAtTheEnd();
  void alignIgnoresAnEmptyRecording();

  void mappingSplitsSources();
  void mappingRekeysSingleSourceToZero();
  void mappingSkipsUnmappedColumns();

  void mergeStreamBlockTimes();
  void mergeStreamBlockTimesWithoutBlocks();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a dataset-location map from (uid, source, index) triples.
 */
static Sessions::DatasetLocationMap locations(const std::vector<std::array<int, 3>>& datasets)
{
  Sessions::DatasetLocationMap map;
  for (const auto& d : datasets)
    map.insert(d[0], Sessions::DatasetLocation{d[1], d[2]});

  return map;
}

/**
 * @brief Builds one index entry of a dense block starting at @p t0Ns.
 */
static Sessions::PlayerStreamBlockIndex streamBlock(qint64 t0Ns)
{
  Sessions::PlayerStreamBlockIndex entry{};
  entry.t0Ns   = t0Ns;
  entry.frames = 4;
  return entry;
}

//--------------------------------------------------------------------------------------------------
// Column index
//--------------------------------------------------------------------------------------------------

/**
 * @brief indexColumns() is the inverse of the stored column order, and nothing else: it must map
 *        every column position exactly once, whatever the unique ids are.
 */
void TstSessionAlignment::indexColumns()
{
  Sessions::ReplayLayout layout{};
  layout.columnUniqueIds = {7, 3, 11};
  Sessions::ReplayAlignment::indexColumns(layout);

  QCOMPARE(layout.uidToColumn.size(), qsizetype(3));
  QCOMPARE(layout.uidToColumn.value(7), 0);
  QCOMPARE(layout.uidToColumn.value(3), 1);
  QCOMPARE(layout.uidToColumn.value(11), 2);
}

//--------------------------------------------------------------------------------------------------
// Column alignment
//--------------------------------------------------------------------------------------------------

/**
 * @brief The recording's stored order is irrelevant: replay order is source id ascending, then
 *        dataset index inside each source, because that is the order FrameBuilder parses cells in.
 */
void TstSessionAlignment::alignSortsBySourceThenIndex()
{
  Sessions::ReplayLayout layout{};
  layout.columnUniqueIds = {30, 20, 10};

  const auto map = locations({
    {10, 0, 0},
    {20, 0, 1},
    {30, 1, 0}
  });
  Sessions::ReplayAlignment::alignColumnsToProject(layout, map);

  const std::vector<int> expected{10, 20, 30};
  QCOMPARE(layout.columnUniqueIds, expected);
  QCOMPARE(layout.uidToColumn.value(10), 0);
  QCOMPARE(layout.uidToColumn.value(30), 2);
}

/**
 * @brief A column whose dataset the loaded project no longer has still replays: it keeps its
 *        relative order and follows the aligned ones, so an older recording loses no data.
 */
void TstSessionAlignment::alignKeepsOrphansAtTheEnd()
{
  Sessions::ReplayLayout layout{};
  layout.columnUniqueIds = {98, 20, 99, 10};

  const auto map = locations({
    {10, 0, 0},
    {20, 0, 1}
  });
  Sessions::ReplayAlignment::alignColumnsToProject(layout, map);

  const std::vector<int> expected{10, 20, 98, 99};
  QCOMPARE(layout.columnUniqueIds, expected);
  QCOMPARE(layout.uidToColumn.size(), qsizetype(4));
  QCOMPARE(layout.uidToColumn.value(99), 3);
}

/**
 * @brief An empty recording is left alone rather than indexed into an empty map.
 */
void TstSessionAlignment::alignIgnoresAnEmptyRecording()
{
  Sessions::ReplayLayout layout{};
  layout.uidToColumn.insert(5, 0);

  Sessions::ReplayAlignment::alignColumnsToProject(layout,
                                                   locations({
                                                     {5, 0, 0}
  }));

  QVERIFY(layout.columnUniqueIds.empty());
  QCOMPARE(layout.uidToColumn.size(), qsizetype(1));
}

//--------------------------------------------------------------------------------------------------
// Source mapping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Two sources make the session multi-source, and each source's replay map indexes its own
 *        payload cells from zero -- the map FrameBuilder addresses cells with.
 */
void TstSessionAlignment::mappingSplitsSources()
{
  Sessions::ReplayLayout layout{};
  layout.columnUniqueIds = {10, 20, 30};
  const auto map         = locations({
    {10, 0, 0},
    {20, 0, 1},
    {30, 1, 0}
  });

  const auto replay = Sessions::ReplayAlignment::buildMultiSourceMapping(layout, map);

  QVERIFY(layout.multiSource);
  QCOMPARE(layout.columnToSource.value(0), 0);
  QCOMPARE(layout.columnToSource.value(2), 1);
  QCOMPARE(layout.sourceColumns.value(0).size(), std::size_t(2));
  QCOMPARE(layout.sourceColumns.value(1).size(), std::size_t(1));
  QCOMPARE(replay.at(0).at(10), 0);
  QCOMPARE(replay.at(0).at(20), 1);
  QCOMPARE(replay.at(1).at(30), 0);
}

/**
 * @brief A single-source recording is replayed through processPayload, which routes to source 0,
 *        so its map is rekeyed to 0 no matter which source id the project gave it.
 */
void TstSessionAlignment::mappingRekeysSingleSourceToZero()
{
  Sessions::ReplayLayout layout{};
  layout.columnUniqueIds = {5, 6};
  const auto map         = locations({
    {5, 3, 0},
    {6, 3, 1}
  });

  const auto replay = Sessions::ReplayAlignment::buildMultiSourceMapping(layout, map);

  QVERIFY(!layout.multiSource);
  QCOMPARE(replay.size(), std::size_t(1));
  QCOMPARE(replay.count(0), std::size_t(1));
  QCOMPARE(replay.at(0).at(5), 0);
  QCOMPARE(replay.at(0).at(6), 1);
  QCOMPARE(layout.sourceColumns.value(3).size(), std::size_t(2));
}

/**
 * @brief A column with no dataset in the project belongs to no source: it is left out of both maps
 *        rather than defaulted into source 0, where it would displace a real cell.
 */
void TstSessionAlignment::mappingSkipsUnmappedColumns()
{
  Sessions::ReplayLayout layout{};
  layout.columnUniqueIds = {10, 99, 20};
  const auto map         = locations({
    {10, 0, 0},
    {20, 0, 1}
  });

  const auto replay = Sessions::ReplayAlignment::buildMultiSourceMapping(layout, map);

  QCOMPARE(layout.columnToSource.size(), qsizetype(2));
  QVERIFY(!layout.columnToSource.contains(1));
  QCOMPARE(replay.at(0).count(99), std::size_t(0));
  QCOMPARE(replay.at(0).at(20), 1);
}

//--------------------------------------------------------------------------------------------------
// Timeline merge
//--------------------------------------------------------------------------------------------------

/**
 * @brief Block starts join the timeline so a stream-only session advances, and the result stays a
 *        sorted set: the player binary-searches it, and a duplicate instant would replay twice.
 */
void TstSessionAlignment::mergeStreamBlockTimes()
{
  std::vector<qint64> timeline{100, 300};
  const std::vector<Sessions::PlayerStreamBlockIndex> blocks{
    streamBlock(300), streamBlock(200), streamBlock(300)};

  Sessions::ReplayAlignment::mergeStreamBlockTimes(timeline, blocks);

  const std::vector<qint64> expected{100, 200, 300};
  QCOMPARE(timeline, expected);
}

/**
 * @brief A session with no dense blocks keeps its timeline untouched.
 */
void TstSessionAlignment::mergeStreamBlockTimesWithoutBlocks()
{
  std::vector<qint64> timeline{5, 1, 3};
  Sessions::ReplayAlignment::mergeStreamBlockTimes(timeline, {});

  const std::vector<qint64> expected{5, 1, 3};
  QCOMPARE(timeline, expected);
}

QTEST_APPLESS_MAIN(TstSessionAlignment)

#include "tst_session_alignment.moc"
