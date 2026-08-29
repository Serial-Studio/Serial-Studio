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

#include <chrono>
#include <cmath>
#include <limits>
#include <QTest>
#include <vector>

#include "Sessions/Player/ReplayClock.h"
#include "Sessions/Player/ReplayFrameValues.h"

// Every test function here is self-contained: each builds its own rows and layout, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief The database-free half of the historian player's frame synthesis: which stored rows become
 *        a replayed frame, where they land in a seek window, how sparse series are filled, and the
 *        clock that keeps replay time owned by the recording.
 */
class TstSessionSynthesis : public QObject {
  Q_OBJECT

private slots:
  void formatCellRoundTrips();

  void applyRowSkipsUnknownDatasets();
  void applyRowKeepsStringsVerbatim();
  void applyRowRecordsTheSource();

  void selectRowsAtTakesTheExactInstant();
  void selectRowsAtMergesBlocks();

  void scatterPlacesRowsOnTheirOwnRow();
  void scatterDropsOffGridRows();
  void scatterDropsUnknownDatasets();

  void fillSeekGapsForwardFills();
  void fillSeekGapsBackfillsTheLeadingRun();
  void fillSeekGapsSeedsAnEmptySeries();

  void clockAdvancesByRecordedDelta();
  void clockReanchorsWithoutRewinding();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Two datasets on source 7, in stored column order.
 */
static Sessions::ReplayLayout twoColumnLayout()
{
  Sessions::ReplayLayout layout{};
  layout.columnUniqueIds = {10, 20};
  layout.uidToColumn.insert(10, 0);
  layout.uidToColumn.insert(20, 1);
  layout.columnToSource.insert(0, 7);
  layout.columnToSource.insert(1, 7);
  layout.sourceColumns.insert(7, {10, 20});
  return layout;
}

/**
 * @brief One decoded numeric reading.
 */
static Sessions::ReadingRow numericRow(int uniqueId, qint64 timestampNs, double value)
{
  Sessions::ReadingRow row;
  row.uniqueId     = uniqueId;
  row.timestampNs  = timestampNs;
  row.finalNumeric = value;
  row.isNumeric    = true;
  return row;
}

//--------------------------------------------------------------------------------------------------
// Cell formatting
//--------------------------------------------------------------------------------------------------

/**
 * @brief A replayed cell must be the value that was recorded, not a rounded twin: 17 significant
 *        digits is the shortest precision that round-trips an IEEE-754 double.
 */
void TstSessionSynthesis::formatCellRoundTrips()
{
  const double value = 0.1 + 0.2;
  const QString text = Sessions::ReplayFrameValues::formatCell(value);

  bool ok = false;
  QCOMPARE(text.toDouble(&ok), value);
  QVERIFY(ok);
}

//--------------------------------------------------------------------------------------------------
// Row selection
//--------------------------------------------------------------------------------------------------

/**
 * @brief A dataset the loaded layout does not carry is not replayed: its cell has nowhere to go,
 *        and inventing a column for it would shift every real cell.
 */
void TstSessionSynthesis::applyRowSkipsUnknownDatasets()
{
  const auto layout = twoColumnLayout();
  Sessions::ReplayRowValues out;

  Sessions::ReplayFrameValues::applyRow(out, numericRow(99, 1000, 1.0), layout);

  QVERIFY(out.values.isEmpty());
  QVERIFY(out.sources.isEmpty());
}

/**
 * @brief A non-numeric reading replays its stored string, untouched by numeric formatting.
 */
void TstSessionSynthesis::applyRowKeepsStringsVerbatim()
{
  const auto layout = twoColumnLayout();
  Sessions::ReplayRowValues out;

  Sessions::ReadingRow row;
  row.uniqueId    = 10;
  row.timestampNs = 1000;
  row.isNumeric   = false;
  row.finalString = QStringLiteral("ARMED");
  Sessions::ReplayFrameValues::applyRow(out, row, layout);

  QCOMPARE(out.values.value(10), QStringLiteral("ARMED"));
}

/**
 * @brief The source set drives the per-source fan-out of the injection, so reading a cell must
 *        record the source its column belongs to.
 */
void TstSessionSynthesis::applyRowRecordsTheSource()
{
  const auto layout = twoColumnLayout();
  Sessions::ReplayRowValues out;

  Sessions::ReplayFrameValues::applyRow(out, numericRow(20, 1000, 4.5), layout);

  QCOMPARE(out.sources.size(), qsizetype(1));
  QVERIFY(out.sources.contains(7));
}

/**
 * @brief Only rows stamped at the cursor instant are replayed: neighbouring samples are skipped,
 *        never interpolated, so a replayed frame carries recorded values alone.
 */
void TstSessionSynthesis::selectRowsAtTakesTheExactInstant()
{
  const auto layout = twoColumnLayout();
  Sessions::ReplayRowValues out;

  const std::vector<Sessions::ReadingRow> rows{
    numericRow(10, 900, 1.0), numericRow(10, 1000, 2.0), numericRow(20, 1100, 3.0)};
  Sessions::ReplayFrameValues::selectRowsAt(out, rows, 1000, layout);

  QCOMPARE(out.values.size(), qsizetype(1));
  QCOMPARE(out.values.value(10).toDouble(), 2.0);
  QVERIFY(!out.values.contains(20));
}

/**
 * @brief One instant can be spread over several decoded blocks, so selection accumulates into the
 *        same frame instead of replacing it.
 */
void TstSessionSynthesis::selectRowsAtMergesBlocks()
{
  const auto layout = twoColumnLayout();
  Sessions::ReplayRowValues out;

  Sessions::ReplayFrameValues::selectRowsAt(out, {numericRow(10, 1000, 1.0)}, 1000, layout);
  Sessions::ReplayFrameValues::selectRowsAt(out, {numericRow(20, 1000, 2.0)}, 1000, layout);

  QCOMPARE(out.values.size(), qsizetype(2));
  QCOMPARE(out.values.value(20).toDouble(), 2.0);
}

//--------------------------------------------------------------------------------------------------
// Seek window scatter
//--------------------------------------------------------------------------------------------------

/**
 * @brief Each stored sample lands on the window row whose timestamp equals its own.
 */
void TstSessionSynthesis::scatterPlacesRowsOnTheirOwnRow()
{
  const std::vector<qint64> rowTimes{100, 200, 300};
  QHash<int, qint64> keyByUid;
  keyByUid.insert(10, 55);

  QHash<qint64, QVector<double>> series;
  series.insert(55, QVector<double>(3, std::numeric_limits<double>::quiet_NaN()));

  const std::vector<Sessions::ReadingRow> rows{numericRow(10, 300, 9.0), numericRow(10, 100, 7.0)};
  Sessions::ReplayFrameValues::scatterRowsIntoWindow(rows, rowTimes, keyByUid, series);

  QCOMPARE(series.value(55).at(0), 7.0);
  QVERIFY(std::isnan(series.value(55).at(1)));
  QCOMPARE(series.value(55).at(2), 9.0);
}

/**
 * @brief A sample between two window rows (a dense sample inside a block) belongs to no row and is
 *        dropped rather than shifted onto a neighbour, which would misdate it.
 */
void TstSessionSynthesis::scatterDropsOffGridRows()
{
  const std::vector<qint64> rowTimes{100, 200};
  QHash<int, qint64> keyByUid;
  keyByUid.insert(10, 55);

  QHash<qint64, QVector<double>> series;
  series.insert(55, QVector<double>(2, std::numeric_limits<double>::quiet_NaN()));

  Sessions::ReplayFrameValues::scatterRowsIntoWindow(
    {numericRow(10, 150, 5.0)}, rowTimes, keyByUid, series);

  QVERIFY(std::isnan(series.value(55).at(0)));
  QVERIFY(std::isnan(series.value(55).at(1)));
}

/**
 * @brief A dataset the dashboard did not ask for is dropped, and no series is created for it.
 */
void TstSessionSynthesis::scatterDropsUnknownDatasets()
{
  const std::vector<qint64> rowTimes{100};
  QHash<int, qint64> keyByUid;
  QHash<qint64, QVector<double>> series;

  Sessions::ReplayFrameValues::scatterRowsIntoWindow(
    {numericRow(10, 100, 5.0)}, rowTimes, keyByUid, series);

  QVERIFY(series.isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Gap filling
//--------------------------------------------------------------------------------------------------

/**
 * @brief A sparse dataset holds its last recorded value until the next one, drawing as the step
 *        function it was captured as instead of a broken line.
 */
void TstSessionSynthesis::fillSeekGapsForwardFills()
{
  constexpr double nan = std::numeric_limits<double>::quiet_NaN();
  QVector<double> values{1.0, nan, nan, 2.0};

  Sessions::ReplayFrameValues::fillSeekGaps(values);

  const QVector<double> expected{1.0, 1.0, 1.0, 2.0};
  QCOMPARE(values, expected);
}

/**
 * @brief The run before the first stored value has nothing to hold, so it backfills from that
 *        value: the window opens on the series as it was, not on a hole.
 */
void TstSessionSynthesis::fillSeekGapsBackfillsTheLeadingRun()
{
  constexpr double nan = std::numeric_limits<double>::quiet_NaN();
  QVector<double> values{nan, nan, 3.0, nan};

  Sessions::ReplayFrameValues::fillSeekGaps(values);

  const QVector<double> expected{3.0, 3.0, 3.0, 3.0};
  QCOMPARE(values, expected);
}

/**
 * @brief A window with no stored value at all seeds to zero rather than leaving NaNs the plot
 *        would refuse to draw.
 */
void TstSessionSynthesis::fillSeekGapsSeedsAnEmptySeries()
{
  constexpr double nan = std::numeric_limits<double>::quiet_NaN();
  QVector<double> values{nan, nan};

  Sessions::ReplayFrameValues::fillSeekGaps(values);

  const QVector<double> expected{0.0, 0.0};
  QCOMPARE(values, expected);
}

//--------------------------------------------------------------------------------------------------
// Replay clock
//--------------------------------------------------------------------------------------------------

/**
 * @brief Replayed rows are stamped by the recording's own deltas from one anchor, not by the wall
 *        clock: the source owns time, and replay is no exception.
 */
void TstSessionSynthesis::clockAdvancesByRecordedDelta()
{
  const auto base = std::chrono::steady_clock::time_point(std::chrono::seconds(1000));
  Sessions::ReplayClock clock;
  clock.anchorAt(base, 10.0);

  QCOMPARE(clock.timestampFor(10000000000LL), base);
  QCOMPARE(clock.timestampFor(11000000000LL), base + std::chrono::seconds(1));
  QCOMPARE(clock.timestampFor(9500000000LL), base - std::chrono::milliseconds(500));
}

/**
 * @brief Re-anchoring on a seek is what keeps a scrubbed session from replaying its rows into the
 *        past: the new anchor pins the row the user landed on to the new instant.
 */
void TstSessionSynthesis::clockReanchorsWithoutRewinding()
{
  const auto first  = std::chrono::steady_clock::time_point(std::chrono::seconds(10));
  const auto second = std::chrono::steady_clock::time_point(std::chrono::seconds(20));

  Sessions::ReplayClock clock;
  clock.anchorAt(first, 0.0);
  QCOMPARE(clock.timestampFor(5000000000LL), first + std::chrono::seconds(5));

  clock.anchorAt(second, 5.0);
  QCOMPARE(clock.baseRowSeconds(), 5.0);
  QCOMPARE(clock.timestampFor(5000000000LL), second);
  QVERIFY(clock.timestampFor(5000000000LL) > first);
}

QTEST_APPLESS_MAIN(TstSessionSynthesis)

#include "tst_session_synthesis.moc"
