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
#include <QString>
#include <QTest>

#include "CSV/Player/RowSyntax.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Quoting, separator-sniffing and timestamp-unit sweep of CSV::RowSyntax, the pure row
 *        scanners the CSV player's quick pass and QuickPlot payload builder read a file with.
 */
class TstCsvRows : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void firstTopLevelSeparator_data();
  void firstTopLevelSeparator();

  void topLevelSeparatorCount_data();
  void topLevelSeparatorCount();

  void sniffSeparator_data();
  void sniffSeparator();

  void timestampUnitScale_data();
  void timestampUnitScale();

  void hugeCellsScanInFullWithoutTruncation();
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting, so the guarded inputs (an
 *        empty row handed to the sniffer) are observable from a debug build.
 */
void TstCsvRows::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

//--------------------------------------------------------------------------------------------------
// firstTopLevelSeparator
//--------------------------------------------------------------------------------------------------

void TstCsvRows::firstTopLevelSeparator_data()
{
  QTest::addColumn<QByteArray>("row");
  QTest::addColumn<char>("separator");
  QTest::addColumn<int>("expected");

  QTest::newRow("plain row") << QByteArray("a,b,c") << ',' << 1;
  QTest::newRow("single cell has none") << QByteArray("abc") << ',' << -1;
  QTest::newRow("empty row has none") << QByteArray("") << ',' << -1;
  QTest::newRow("leading separator") << QByteArray(",abc") << ',' << 0;
  QTest::newRow("trailing separator") << QByteArray("abc,") << ',' << 3;
  QTest::newRow("quoted first cell hides its separator") << QByteArray("\"a,b\",c") << ',' << 5;
  QTest::newRow("escaped quotes inside the first cell") << QByteArray("\"a\"\"b\",c") << ',' << 6;
  QTest::newRow("leading whitespace still opens the quote")
    << QByteArray("  \"a,b\",c") << ',' << 7;
  QTest::newRow("quote mid-cell is literal") << QByteArray("a\"b,c\",d") << ',' << 3;
  QTest::newRow("semicolon file") << QByteArray("a;b;c") << ';' << 1;
  QTest::newRow("tab file") << QByteArray("a\tb") << '\t' << 1;
  QTest::newRow("unquoted comma is invisible to a semicolon scan")
    << QByteArray("a,b;c") << ';' << 3;
  QTest::newRow("unicode cell bytes are skipped whole")
    << QByteArray("\xC3\xA1\xC3\xA9,b") << ',' << 4;
  QTest::newRow("unterminated quote swallows the row") << QByteArray("\"a,b,c") << ',' << -1;
}

/**
 * @brief The scanner only honours a quote that opens the first cell, which is what keeps a
 *        mid-cell quote character from hiding the separator that splits a QuickPlot payload.
 */
void TstCsvRows::firstTopLevelSeparator()
{
  QFETCH(QByteArray, row);
  QFETCH(char, separator);
  QFETCH(int, expected);

  const auto index = CSV::firstTopLevelSeparator(QByteArrayView(row), separator);
  QCOMPARE(static_cast<int>(index), expected);
}

//--------------------------------------------------------------------------------------------------
// topLevelSeparatorCount
//--------------------------------------------------------------------------------------------------

void TstCsvRows::topLevelSeparatorCount_data()
{
  QTest::addColumn<QByteArray>("row");
  QTest::addColumn<char>("separator");
  QTest::addColumn<int>("expected");

  QTest::newRow("three cells") << QByteArray("a,b,c") << ',' << 2;
  QTest::newRow("one cell") << QByteArray("abc") << ',' << 0;
  QTest::newRow("empty row") << QByteArray("") << ',' << 0;
  QTest::newRow("empty cells still count") << QByteArray(",,") << ',' << 2;
  QTest::newRow("quoted separators do not count") << QByteArray("\"a,b\",c") << ',' << 1;
  QTest::newRow("escaped quotes keep the cell quoted") << QByteArray("\"a\"\"b,c\",d") << ',' << 1;
  QTest::newRow("quote anywhere toggles, unlike the RFC splitter")
    << QByteArray("a\"b,c\",d") << ',' << 1;
  QTest::newRow("quoted semicolons never score") << QByteArray("1,\"x;y\",2") << ';' << 0;
  QTest::newRow("semicolon file") << QByteArray("a;b;c") << ';' << 2;
  QTest::newRow("pipe file") << QByteArray("a|b|c|d") << '|' << 3;
  QTest::newRow("tab file") << QByteArray("a\tb\tc") << '\t' << 2;
  QTest::newRow("unterminated quote hides the rest") << QByteArray("a,\"b,c") << ',' << 1;
  QTest::newRow("unicode payload does not disturb the scan")
    << QByteArray("\xE2\x82\xAC,\xC3\xB1,c") << ',' << 2;
}

/**
 * @brief The sniffer's scanner is cell-position-independent: any quote toggles, so a quoted
 *        cell can never leak its separators into another candidate's score.
 */
void TstCsvRows::topLevelSeparatorCount()
{
  QFETCH(QByteArray, row);
  QFETCH(char, separator);
  QFETCH(int, expected);

  const auto count = CSV::topLevelSeparatorCount(QByteArrayView(row), separator);
  QCOMPARE(static_cast<int>(count), expected);
}

//--------------------------------------------------------------------------------------------------
// sniffSeparator
//--------------------------------------------------------------------------------------------------

void TstCsvRows::sniffSeparator_data()
{
  QTest::addColumn<QByteArray>("header");
  QTest::addColumn<QByteArray>("data");
  QTest::addColumn<char>("expected");

  QTest::newRow("comma file") << QByteArray("t,a,b") << QByteArray("1,2,3") << ',';
  QTest::newRow("semicolon file") << QByteArray("t;a;b") << QByteArray("1;2;3") << ';';
  QTest::newRow("tab file") << QByteArray("t\ta") << QByteArray("1\t2") << '\t';
  QTest::newRow("pipe file") << QByteArray("t|a") << QByteArray("1|2") << '|';
  QTest::newRow("single column falls back to comma")
    << QByteArray("time") << QByteArray("1.0") << ',';
  QTest::newRow("text cell separators never re-read a comma file")
    << QByteArray("t,name") << QByteArray("1,a;b;c") << ',';
  QTest::newRow("header count must match the data count")
    << QByteArray("t;a;b;c") << QByteArray("1;2") << ',';
  QTest::newRow("quoted candidate never scores")
    << QByteArray("t,\"a;b\"") << QByteArray("1,\"c;d\"") << ',';
  QTest::newRow("a tie keeps comma") << QByteArray("a,b;c") << QByteArray("1,2;3") << ',';
  QTest::newRow("semicolon wins on volume")
    << QByteArray("a,b;c;d") << QByteArray("1,2;3;4") << ';';
  QTest::newRow("empty rows fall back to comma") << QByteArray("") << QByteArray("") << ',';
}

/**
 * @brief Spec 0048: comma is scored first and wins ties, and a non-comma winner must show up
 *        in the data row with a matching header count -- so a stray text separator cannot
 *        re-interpret a comma recording.
 */
void TstCsvRows::sniffSeparator()
{
  QFETCH(QByteArray, header);
  QFETCH(QByteArray, data);
  QFETCH(char, expected);

  QCOMPARE(CSV::sniffSeparator(QByteArrayView(header), QByteArrayView(data)), expected);
}

//--------------------------------------------------------------------------------------------------
// timestampUnitScale
//--------------------------------------------------------------------------------------------------

void TstCsvRows::timestampUnitScale_data()
{
  QTest::addColumn<QString>("header");
  QTest::addColumn<bool>("resolved");
  QTest::addColumn<double>("scale");

  const QString micro = QStringLiteral("time(") + QChar(0x00B5) + QStringLiteral("s)");

  QTest::newRow("parenthesized ms") << QStringLiteral("time(ms)") << true << 1e-3;
  QTest::newRow("bracketed us") << QStringLiteral("t [us]") << true << 1e-6;
  QTest::newRow("micro sign") << micro << true << 1e-6;
  QTest::newRow("underscore suffix") << QStringLiteral("time_ms") << true << 1e-3;
  QTest::newRow("last underscore wins") << QStringLiteral("elapsed_time_us") << true << 1e-6;
  QTest::newRow("nanoseconds") << QStringLiteral("time(ns)") << true << 1e-9;
  QTest::newRow("seconds") << QStringLiteral("time(s)") << true << 1.0;
  QTest::newRow("spelled out seconds") << QStringLiteral("t(seconds)") << true << 1.0;
  QTest::newRow("case and padding are normalized")
    << QStringLiteral("  Time (NS)  ") << true << 1e-9;
  QTest::newRow("no unit at all") << QStringLiteral("timestamp") << false << 0.0;
  QTest::newRow("unrecognized unit") << QStringLiteral("time(hours)") << false << 0.0;
  QTest::newRow("unclosed bracket") << QStringLiteral("time[ms") << false << 0.0;
  QTest::newRow("trailing underscore") << QStringLiteral("time_") << false << 0.0;
}

/**
 * @brief Spec 0048 R7: a header that names its unit configures the scale silently; one that
 *        does not comes back empty so the player can ask the user instead of guessing.
 */
void TstCsvRows::timestampUnitScale()
{
  QFETCH(QString, header);
  QFETCH(bool, resolved);
  QFETCH(double, scale);

  const auto result = CSV::timestampUnitScale(header);
  QCOMPARE(result.has_value(), resolved);

  if (resolved)
    QCOMPARE(*result, scale);
}

//--------------------------------------------------------------------------------------------------
// Large rows
//--------------------------------------------------------------------------------------------------

/**
 * @brief A megabyte-class cell is scanned whole: the scanners carry no early-out budget, which
 *        is what lets a long text cell hold a separator without breaking the row's split.
 */
void TstCsvRows::hugeCellsScanInFullWithoutTruncation()
{
  constexpr int kCellBytes = 512 * 1024;

  QByteArray row;
  row.append('"');
  row.append(QByteArray(kCellBytes, ','));
  row.append('"');
  row.append(',');
  row.append(QByteArray(kCellBytes, 'x'));

  QCOMPARE(static_cast<int>(CSV::topLevelSeparatorCount(QByteArrayView(row), ',')), 1);
  QCOMPARE(static_cast<int>(CSV::firstTopLevelSeparator(QByteArrayView(row), ',')), kCellBytes + 2);
}

QTEST_APPLESS_MAIN(TstCsvRows)

#include "tst_csv_rows.moc"
