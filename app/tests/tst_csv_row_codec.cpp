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

#include <QByteArray>
#include <QByteArrayList>
#include <QTest>

#include "CSV/Player/RowCodec.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Timestamp-exclusion and QuickPlot-payload sweep of CSV::RowCodec, the per-row half of
 *        CSV replay: which cells reach the replay lane, and what bytes QuickPlot mode injects.
 */
class TstCsvRowCodec : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void numericModeDropsTheFirstCell();
  void intervalModeKeepsEveryCell();
  void dateTimeColumnModeDropsThatCell();
  void quotedCellsSurviveTheSplit();
  void dataColumnToFileColumn_data();
  void dataColumnToFileColumn();

  void quickPlotSlicesTheRowVerbatim();
  void quickPlotRebuildsNonCommaRows();
  void quickPlotRebuildsDateTimeColumnRows();
  void quickPlotKeepsTheWholeIntervalRow();
  void quickPlotOnASingleCellRowIsEmpty();

  void resetRestoresTheDefaultReading();

private:
  [[nodiscard]] static QByteArrayList spansOf(const CSV::RowCodec& codec);
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting, so the guarded inputs (a
 *        negative data column) are observable from a debug build.
 */
void TstCsvRowCodec::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief Copies the codec's current data spans out as byte arrays, so an expectation reads as
 *        the cell list it is rather than as pointer arithmetic.
 */
QByteArrayList TstCsvRowCodec::spansOf(const CSV::RowCodec& codec)
{
  QByteArrayList out;
  for (qsizetype i = 0; i < codec.dataSpanCount(); ++i)
    out.append(codec.dataSpans()[i].toByteArray());

  return out;
}

//--------------------------------------------------------------------------------------------------
// Data-cell selection
//--------------------------------------------------------------------------------------------------

/**
 * @brief The default reading treats cell 0 as the elapsed time, so only the rest is data.
 */
void TstCsvRowCodec::numericModeDropsTheFirstCell()
{
  CSV::RowCodec codec;
  const QByteArray row("1.5,10,20,30");

  QCOMPARE(static_cast<int>(codec.splitDataCells(QByteArrayView(row))), 3);
  QCOMPARE(spansOf(codec), QByteArrayList({"10", "20", "30"}));
}

/**
 * @brief Interval mode has no timestamp cell at all: the row's time comes from its index, so
 *        every cell is data and dropping one would shift the whole recording.
 */
void TstCsvRowCodec::intervalModeKeepsEveryCell()
{
  CSV::RowCodec codec;
  codec.setTimestampMode(CSV::PlayerTimestampMode::Interval, 0);
  const QByteArray row("10,20,30");

  QCOMPARE(static_cast<int>(codec.splitDataCells(QByteArrayView(row))), 3);
  QCOMPARE(spansOf(codec), QByteArrayList({"10", "20", "30"}));
}

/**
 * @brief A date/time column in the middle is excluded in place, leaving the cells on both
 *        sides in their original order.
 */
void TstCsvRowCodec::dateTimeColumnModeDropsThatCell()
{
  CSV::RowCodec codec;
  codec.setTimestampMode(CSV::PlayerTimestampMode::DateTimeColumn, 2);
  const QByteArray row("10,20,2026/01/02 03:04:05,40");

  QCOMPARE(static_cast<int>(codec.splitDataCells(QByteArrayView(row))), 3);
  QCOMPARE(spansOf(codec), QByteArrayList({"10", "20", "40"}));
}

/**
 * @brief Cells keep their quoted content as one cell: a text dataset holding the separator is
 *        exactly why the replay path splits with a quote-aware machine.
 */
void TstCsvRowCodec::quotedCellsSurviveTheSplit()
{
  CSV::RowCodec codec;
  const QByteArray row("1.0,\"a,b\",c");

  QCOMPARE(static_cast<int>(codec.splitDataCells(QByteArrayView(row))), 2);
  QCOMPARE(spansOf(codec), QByteArrayList({"a,b", "c"}));
}

void TstCsvRowCodec::dataColumnToFileColumn_data()
{
  QTest::addColumn<int>("mode");
  QTest::addColumn<int>("timestampColumn");
  QTest::addColumn<int>("dataColumn");
  QTest::addColumn<int>("expected");

  const int numeric  = static_cast<int>(CSV::PlayerTimestampMode::Numeric);
  const int dateTime = static_cast<int>(CSV::PlayerTimestampMode::DateTime);
  const int interval = static_cast<int>(CSV::PlayerTimestampMode::Interval);
  const int column   = static_cast<int>(CSV::PlayerTimestampMode::DateTimeColumn);

  QTest::newRow("numeric shifts by one") << numeric << 0 << 0 << 1;
  QTest::newRow("numeric shifts every column") << numeric << 0 << 4 << 5;
  QTest::newRow("date/time shifts by one") << dateTime << 0 << 2 << 3;
  QTest::newRow("interval is identity") << interval << 0 << 0 << 0;
  QTest::newRow("interval is identity everywhere") << interval << 0 << 7 << 7;
  QTest::newRow("before the date/time column") << column << 2 << 0 << 0;
  QTest::newRow("at the date/time column") << column << 2 << 2 << 3;
  QTest::newRow("after the date/time column") << column << 2 << 5 << 6;
  QTest::newRow("date/time column first") << column << 0 << 0 << 1;
}

/**
 * @brief The inverse of the data-cell selection: this is what puts a source's presence bit on
 *        the right file cell, so an off-by-one here silently mislabels a whole recording.
 */
void TstCsvRowCodec::dataColumnToFileColumn()
{
  QFETCH(int, mode);
  QFETCH(int, timestampColumn);
  QFETCH(int, dataColumn);
  QFETCH(int, expected);

  CSV::RowCodec codec;
  codec.setTimestampMode(static_cast<CSV::PlayerTimestampMode>(mode), timestampColumn);
  QCOMPARE(codec.dataColumnToFileColumn(dataColumn), expected);
}

//--------------------------------------------------------------------------------------------------
// QuickPlot payloads
//--------------------------------------------------------------------------------------------------

/**
 * @brief A comma file in the default reading is sliced at the first top-level separator, so
 *        the injected payload is the row's own bytes minus the timestamp cell.
 */
void TstCsvRowCodec::quickPlotSlicesTheRowVerbatim()
{
  CSV::RowCodec codec;
  const QByteArray row("1.5,\"a,b\",30\r");

  QCOMPARE(codec.quickPlotPayload(QByteArrayView(row)), QByteArray("\"a,b\",30\n"));
}

/**
 * @brief A non-comma file is rebuilt through the joiner, because everything downstream of the
 *        injection splits on commas only (spec 0048).
 */
void TstCsvRowCodec::quickPlotRebuildsNonCommaRows()
{
  CSV::RowCodec codec;
  codec.setSeparator(';');
  const QByteArray row("1.5;10;20");

  QCOMPARE(codec.quickPlotPayload(QByteArrayView(row)), QByteArray("10,20\n"));
}

/**
 * @brief Date/time-column mode also rebuilds: the timestamp sits mid-row, so no single slice
 *        of the original bytes can drop it.
 */
void TstCsvRowCodec::quickPlotRebuildsDateTimeColumnRows()
{
  CSV::RowCodec codec;
  codec.setTimestampMode(CSV::PlayerTimestampMode::DateTimeColumn, 1);
  const QByteArray row("10,2026/01/02 03:04:05,30");

  QCOMPARE(codec.quickPlotPayload(QByteArrayView(row)), QByteArray("10,30\n"));
}

/**
 * @brief Interval mode injects the row whole: none of its cells is a timestamp.
 */
void TstCsvRowCodec::quickPlotKeepsTheWholeIntervalRow()
{
  CSV::RowCodec codec;
  codec.setTimestampMode(CSV::PlayerTimestampMode::Interval, 0);
  const QByteArray row("10,20,30\r");

  QCOMPARE(codec.quickPlotPayload(QByteArrayView(row)), QByteArray("10,20,30\n"));
}

/**
 * @brief A row that is nothing but a timestamp has no data to inject, and comes back empty so
 *        the caller skips it rather than publishing a blank frame.
 */
void TstCsvRowCodec::quickPlotOnASingleCellRowIsEmpty()
{
  CSV::RowCodec codec;
  const QByteArray row("1.5");

  QVERIFY(codec.quickPlotPayload(QByteArrayView(row)).isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closing a file restores the legacy default reading, so the next open starts from the
 *        same state a fresh codec would.
 */
void TstCsvRowCodec::resetRestoresTheDefaultReading()
{
  CSV::RowCodec codec;
  codec.setSeparator(';');
  codec.setTimestampMode(CSV::PlayerTimestampMode::DateTimeColumn, 3);

  const QByteArray row("1.5;10;20");
  QCOMPARE(static_cast<int>(codec.splitDataCells(QByteArrayView(row))), 3);

  codec.reset();

  QCOMPARE(codec.separator(), ',');
  QCOMPARE(codec.timestampColumn(), 0);
  QCOMPARE(static_cast<int>(codec.timestampMode()),
           static_cast<int>(CSV::PlayerTimestampMode::Numeric));
  QCOMPARE(static_cast<int>(codec.dataSpanCount()), 0);
}

QTEST_APPLESS_MAIN(TstCsvRowCodec)

#include "tst_csv_row_codec.moc"
