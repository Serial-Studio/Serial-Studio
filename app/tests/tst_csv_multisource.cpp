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

#include <QTest>
#include <QVector>
#include <vector>

#include "CSV/Player/MultiSourceMap.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Column mapping and sparse-backfill sweep of CSV::MultiSourceMap, the layout a
 *        multi-source CSV replay routes its cells and its per-row source presence through.
 */
class TstCsvMultiSource : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void singleSourceKeepsTheFlatReplayMap();
  void multiSourceNumbersEachSourceLocally();
  void dateTimeColumnMapperSkipsTheTimestampCell();
  void sourceBitsStopAtTheTrackingCeiling();
  void fileColumnsOutsideTheHeaderAreIgnored();

  void staleSourcesFindsEachSourcesLatestRow();
  void staleSourcesSkipsSourcesAlreadyRepublished();
  void staleSourcesClampsThePlayheadToTheBitmap();
  void staleSourcesIgnoresSourcesThatNeverAppear();
  void staleSourcesIsEmptyForASingleSourceRecording();
  void clearDropsTheWholeLayout();

private:
  [[nodiscard]] static CSV::MultiSourceMap::ColumnMapper numericMapper();
  [[nodiscard]] static std::vector<CSV::ReplayColumnRef> twoSourceColumns();
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting, so the guarded inputs (a
 *        negative header width) are observable from a debug build.
 */
void TstCsvMultiSource::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief The data-column-to-file-column mapping of Numeric/DateTime mode: cell 0 is the
 *        timestamp, so data column i lives in file cell i + 1.
 */
CSV::MultiSourceMap::ColumnMapper TstCsvMultiSource::numericMapper()
{
  return [](int i) {
    return i + 1;
  };
}

/**
 * @brief Two sources interleaved over four columns, as a project whose groups alternate
 *        between sources produces them in uniqueId order.
 */
std::vector<CSV::ReplayColumnRef> TstCsvMultiSource::twoSourceColumns()
{
  return {
    {10, 1},
    {11, 2},
    {12, 1},
    {13, 2}
  };
}

//--------------------------------------------------------------------------------------------------
// Layout construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief One source keeps the legacy flat map under source 0, and publishes no per-source
 *        column lists: injectRow() then feeds the whole row through the single replay lane.
 */
void TstCsvMultiSource::singleSourceKeepsTheFlatReplayMap()
{
  CSV::MultiSourceMap map;
  const std::vector<CSV::ReplayColumnRef> columns = {
    {1, 0},
    {2, 0},
    {3, 0}
  };
  const auto replay = map.build(columns, 4, numericMapper());

  QVERIFY(!map.multiSource());
  QVERIFY(map.sourceColumnsByIndex().isEmpty());

  QCOMPARE(replay.size(), static_cast<std::size_t>(1));
  QCOMPARE(replay.at(0).at(1), 0);
  QCOMPARE(replay.at(0).at(2), 1);
  QCOMPARE(replay.at(0).at(3), 2);

  QCOMPARE(map.bitSourceIds(), QVector<int>({0}));
  QCOMPARE(map.fileColumnSourceBit(), QVector<quint8>({0, 1, 1, 1}));
}

/**
 * @brief Several sources get one replay map each, numbered from zero in the order their
 *        columns appear: that local order is exactly the cell order injectRow() assembles.
 */
void TstCsvMultiSource::multiSourceNumbersEachSourceLocally()
{
  CSV::MultiSourceMap map;
  const auto replay = map.build(twoSourceColumns(), 5, numericMapper());

  QVERIFY(map.multiSource());

  QCOMPARE(replay.size(), static_cast<std::size_t>(2));
  QCOMPARE(replay.at(1).at(10), 0);
  QCOMPARE(replay.at(1).at(12), 1);
  QCOMPARE(replay.at(2).at(11), 0);
  QCOMPARE(replay.at(2).at(13), 1);

  QCOMPARE(map.sourceColumnsByIndex().value(1), QVector<int>({0, 2}));
  QCOMPARE(map.sourceColumnsByIndex().value(2), QVector<int>({1, 3}));

  QCOMPARE(map.bitSourceIds(), QVector<int>({1, 2}));
  QCOMPARE(map.fileColumnSourceBit(), QVector<quint8>({0, 1, 2, 1, 2}));
}

/**
 * @brief The mapper is what places a source's bit on the right file cell: a date/time column
 *        in the middle shifts every data column past it, and the timestamp cell stays unowned.
 */
void TstCsvMultiSource::dateTimeColumnMapperSkipsTheTimestampCell()
{
  constexpr int kTimestampColumn = 2;
  const auto mapper              = [](int i) {
    return (i < kTimestampColumn) ? i : i + 1;
  };

  CSV::MultiSourceMap map;
  (void)map.build(twoSourceColumns(), 5, mapper);

  QCOMPARE(map.fileColumnSourceBit(), QVector<quint8>({1, 2, 0, 1, 2}));
}

/**
 * @brief Presence masks are one byte, so only the first eight sources are tracked; the rest
 *        still replay, they just never take part in the backfill.
 */
void TstCsvMultiSource::sourceBitsStopAtTheTrackingCeiling()
{
  constexpr int kSources = 10;

  std::vector<CSV::ReplayColumnRef> columns;
  columns.reserve(kSources);
  for (int i = 0; i < kSources; ++i)
    columns.push_back({100 + i, i});

  CSV::MultiSourceMap map;
  (void)map.build(columns, kSources, [](int i) { return i; });

  QVERIFY(map.multiSource());
  QCOMPARE(static_cast<int>(map.bitSourceIds().size()), CSV::kMaxTrackedSources);
  QCOMPARE(map.fileColumnSourceBit().at(CSV::kMaxTrackedSources - 1), quint8(128));
  QCOMPARE(map.fileColumnSourceBit().at(CSV::kMaxTrackedSources), quint8(0));
  QCOMPARE(map.fileColumnSourceBit().at(kSources - 1), quint8(0));
}

/**
 * @brief A file narrower than the schema (a truncated recording) drops the bits that fall off
 *        its end instead of writing past the header width.
 */
void TstCsvMultiSource::fileColumnsOutsideTheHeaderAreIgnored()
{
  CSV::MultiSourceMap map;
  (void)map.build(twoSourceColumns(), 3, numericMapper());

  QCOMPARE(map.fileColumnSourceBit(), QVector<quint8>({0, 1, 2}));
}

//--------------------------------------------------------------------------------------------------
// Sparse backfill
//--------------------------------------------------------------------------------------------------

/**
 * @brief Each source resolves to its own latest present row at or before the playhead, which
 *        is what keeps a slow source's widgets live across a strided catch-up.
 */
void TstCsvMultiSource::staleSourcesFindsEachSourcesLatestRow()
{
  CSV::MultiSourceMap map;
  (void)map.build(twoSourceColumns(), 5, numericMapper());
  map.appendRowSourceBits(QVector<quint8>({0b01, 0b10, 0b01, 0b00, 0b10}));

  const auto stale = map.staleSources(4);
  QCOMPARE(static_cast<int>(stale.size()), 2);
  QCOMPARE(stale.at(0).sourceId, 1);
  QCOMPARE(stale.at(0).row, 2);
  QCOMPARE(stale.at(1).sourceId, 2);
  QCOMPARE(stale.at(1).row, 4);
}

/**
 * @brief A source already republished from that row is left out, so a stationary playhead
 *        does not re-inject the same cells every pass; resetting the marks re-arms it.
 */
void TstCsvMultiSource::staleSourcesSkipsSourcesAlreadyRepublished()
{
  CSV::MultiSourceMap map;
  (void)map.build(twoSourceColumns(), 5, numericMapper());
  map.appendRowSourceBits(QVector<quint8>({0b01, 0b10, 0b01, 0b00, 0b10}));

  QCOMPARE(static_cast<int>(map.staleSources(4).size()), 2);
  QCOMPARE(static_cast<int>(map.staleSources(4).size()), 0);

  map.resetLastSourceRows();
  QCOMPARE(static_cast<int>(map.staleSources(4).size()), 2);

  const auto rewound = map.staleSources(1);
  QCOMPARE(static_cast<int>(rewound.size()), 2);
  QCOMPARE(rewound.at(0).row, 0);
  QCOMPARE(rewound.at(1).row, 1);
}

/**
 * @brief A playhead past the indexed frontier scans from the last known row: batches land
 *        after the position moves, so the two are routinely out of step.
 */
void TstCsvMultiSource::staleSourcesClampsThePlayheadToTheBitmap()
{
  CSV::MultiSourceMap map;
  (void)map.build(twoSourceColumns(), 5, numericMapper());
  map.appendRowSourceBits(QVector<quint8>({0b01, 0b10}));

  const auto stale = map.staleSources(4096);
  QCOMPARE(static_cast<int>(stale.size()), 2);
  QCOMPARE(stale.at(0).row, 0);
  QCOMPARE(stale.at(1).row, 1);
}

/**
 * @brief A source with no cell anywhere in the scanned range is not republished at all: the
 *        backfill holds the last real sample rather than inventing row 0.
 */
void TstCsvMultiSource::staleSourcesIgnoresSourcesThatNeverAppear()
{
  CSV::MultiSourceMap map;
  const std::vector<CSV::ReplayColumnRef> columns = {
    {10, 1},
    {11, 2},
    {12, 3}
  };
  (void)map.build(columns, 4, numericMapper());
  map.appendRowSourceBits(QVector<quint8>({0b001, 0b010, 0b001}));

  const auto stale = map.staleSources(2);
  QCOMPARE(static_cast<int>(stale.size()), 2);
  QCOMPARE(stale.at(0).sourceId, 1);
  QCOMPARE(stale.at(1).sourceId, 2);
}

/**
 * @brief A single-source recording has nothing to backfill: injectRow() already publishes
 *        every column of every row it visits.
 */
void TstCsvMultiSource::staleSourcesIsEmptyForASingleSourceRecording()
{
  CSV::MultiSourceMap map;
  const std::vector<CSV::ReplayColumnRef> columns = {
    {1, 0},
    {2, 0}
  };
  (void)map.build(columns, 3, numericMapper());
  map.appendRowSourceBits(QVector<quint8>({0b01, 0b01}));

  QCOMPARE(static_cast<int>(map.staleSources(1).size()), 0);
}

/**
 * @brief Closing a file drops the layout and the indexed bitmap together, so the next open
 *        cannot inherit a stale source's rows.
 */
void TstCsvMultiSource::clearDropsTheWholeLayout()
{
  CSV::MultiSourceMap map;
  (void)map.build(twoSourceColumns(), 5, numericMapper());
  map.appendRowSourceBits(QVector<quint8>({0b01, 0b10}));

  map.clear();

  QVERIFY(!map.multiSource());
  QVERIFY(map.bitSourceIds().isEmpty());
  QVERIFY(map.fileColumnSourceBit().isEmpty());
  QVERIFY(map.sourceColumnsByIndex().isEmpty());
  QCOMPARE(static_cast<int>(map.staleSources(1).size()), 0);
}

QTEST_APPLESS_MAIN(TstCsvMultiSource)

#include "tst_csv_multisource.moc"
