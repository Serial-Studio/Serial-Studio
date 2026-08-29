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

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QTest>

#include "DataModel/Scripting/ReplayRowCodec.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief KATs for the replay row codec: the writer and the two readers are one contract, so the
 *        QString reader and its byte-view twin are held to the same expectations row by row.
 */
class TstReplayRowCodec : public QObject {
  Q_OBJECT

private slots:
  void splitRow_data();
  void splitRow();

  void splitRowSpansMatchesSplitRow();

  void joinRow_data();
  void joinRow();

  void roundTrip_data();
  void roundTrip();

  void splitRowSpansSeparator();
  void splitRowSpansChopsTrailingCr();
  void splitChannelsSkipsEmptyLines();
};

//--------------------------------------------------------------------------------------------------
// splitReplayRow
//--------------------------------------------------------------------------------------------------

void TstReplayRowCodec::splitRow_data()
{
  QTest::addColumn<QString>("row");
  QTest::addColumn<QStringList>("expected");

  QTest::newRow("empty row is one empty cell") << QString() << QStringList{QString()};
  QTest::newRow("plain numbers") << QStringLiteral(
    "1,2,3") << QStringList{QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3")};
  QTest::newRow("unquoted cells are trimmed")
    << QStringLiteral(" 1 , 2 ") << QStringList{QStringLiteral("1"), QStringLiteral("2")};
  QTest::newRow("quoted cell keeps its spaces")
    << QStringLiteral("\" a \",b") << QStringList{QStringLiteral(" a "), QStringLiteral("b")};
  QTest::newRow("quoted cell keeps its comma")
    << QStringLiteral("\"a,b\",c") << QStringList{QStringLiteral("a,b"), QStringLiteral("c")};
  QTest::newRow("escaped quote collapses")
    << QStringLiteral("\"a\"\"b\"") << QStringList{QStringLiteral("a\"b")};
  QTest::newRow("trailing empty cell survives")
    << QStringLiteral("a,") << QStringList{QStringLiteral("a"), QString()};
  QTest::newRow("injection guard is stripped from a negative number")
    << QStringLiteral("'-0.5") << QStringList{QStringLiteral("-0.5")};
  QTest::newRow("injection guard is stripped from a formula")
    << QStringLiteral("'=SUM(A1)") << QStringList{QStringLiteral("=SUM(A1)")};
  QTest::newRow("apostrophe before a safe char is data")
    << QStringLiteral("'abc") << QStringList{QStringLiteral("'abc")};
}

void TstReplayRowCodec::splitRow()
{
  QFETCH(QString, row);
  QFETCH(QStringList, expected);

  QCOMPARE(DataModel::splitReplayRow(row), expected);
}

//--------------------------------------------------------------------------------------------------
// splitReplayRowSpans
//--------------------------------------------------------------------------------------------------

/**
 * @brief The byte-view reader is the disk-backed replay path; a divergence from the QString reader
 *        would make a recording read back differently depending on which player opened it.
 */
void TstReplayRowCodec::splitRowSpansMatchesSplitRow()
{
  const QList<QByteArray> rows = {QByteArray("1,2,3"),
                                  QByteArray(" 1 , 2 "),
                                  QByteArray("\" a \",b"),
                                  QByteArray("\"a,b\",c"),
                                  QByteArray("\"a\"\"b\""),
                                  QByteArray("a,"),
                                  QByteArray("'-0.5"),
                                  QByteArray("'=SUM(A1)"),
                                  QByteArray("'abc"),
                                  QByteArray("")};

  QByteArray scratch;
  DataModel::ReplayCellViews cells;

  for (const auto& row : rows) {
    DataModel::splitReplayRowSpans(QByteArrayView(row), cells, scratch);

    QStringList spanCells;
    for (const auto& cell : cells)
      spanCells.append(QString::fromUtf8(cell.data(), cell.size()));

    QCOMPARE(spanCells, DataModel::splitReplayRow(QString::fromUtf8(row)));
  }
}

/**
 * @brief The CSV player sniffs the separator (spec 0048); quoting and trimming are unchanged by it.
 */
void TstReplayRowCodec::splitRowSpansSeparator()
{
  QByteArray scratch;
  DataModel::ReplayCellViews cells;
  const QByteArray row(" 1 ;\"a;b\";3");

  DataModel::splitReplayRowSpans(QByteArrayView(row), cells, scratch, ';');

  QCOMPARE(cells.size(), qsizetype(3));
  QCOMPARE(QString::fromUtf8(cells[0].data(), cells[0].size()), QStringLiteral("1"));
  QCOMPARE(QString::fromUtf8(cells[1].data(), cells[1].size()), QStringLiteral("a;b"));
  QCOMPARE(QString::fromUtf8(cells[2].data(), cells[2].size()), QStringLiteral("3"));
}

/**
 * @brief CRLF recordings must not leak the CR into the last cell.
 */
void TstReplayRowCodec::splitRowSpansChopsTrailingCr()
{
  QByteArray scratch;
  DataModel::ReplayCellViews cells;
  const QByteArray row("1,2\r");

  DataModel::splitReplayRowSpans(QByteArrayView(row), cells, scratch);

  QCOMPARE(cells.size(), qsizetype(2));
  QCOMPARE(QString::fromUtf8(cells[1].data(), cells[1].size()), QStringLiteral("2"));
}

//--------------------------------------------------------------------------------------------------
// joinReplayRow
//--------------------------------------------------------------------------------------------------

void TstReplayRowCodec::joinRow_data()
{
  QTest::addColumn<QStringList>("cells");
  QTest::addColumn<QByteArray>("expected");

  QTest::newRow("plain cells need no quoting")
    << QStringList{QStringLiteral("1"), QStringLiteral("2")} << QByteArray("1,2");
  QTest::newRow("comma forces quoting")
    << QStringList{QStringLiteral("a,b")} << QByteArray("\"a,b\"");
  QTest::newRow("quote is doubled")
    << QStringList{QStringLiteral("a\"b")} << QByteArray("\"a\"\"b\"");
  QTest::newRow("newline forces quoting")
    << QStringList{QStringLiteral("a\nb")} << QByteArray("\"a\nb\"");
}

void TstReplayRowCodec::joinRow()
{
  QFETCH(QStringList, cells);
  QFETCH(QByteArray, expected);

  QCOMPARE(DataModel::joinReplayRow(cells), expected);
}

//--------------------------------------------------------------------------------------------------
// Round trip
//--------------------------------------------------------------------------------------------------

void TstReplayRowCodec::roundTrip_data()
{
  QTest::addColumn<QStringList>("cells");

  QTest::newRow("numbers") << QStringList{QStringLiteral("1"), QStringLiteral("-2.5")};
  QTest::newRow("embedded comma") << QStringList{QStringLiteral("a,b"), QStringLiteral("c")};
  QTest::newRow("embedded quote") << QStringList{QStringLiteral("a\"b")};
  QTest::newRow("embedded newline") << QStringList{QStringLiteral("a\nb")};
  QTest::newRow("quote and comma together") << QStringList{QStringLiteral("a\",b")};
}

/**
 * @brief What the exporter writes is what a player must read back, cell for cell.
 */
void TstReplayRowCodec::roundTrip()
{
  QFETCH(QStringList, cells);

  const QByteArray row = DataModel::joinReplayRow(cells);
  QCOMPARE(DataModel::splitReplayRow(QString::fromUtf8(row)), cells);
}

//--------------------------------------------------------------------------------------------------
// splitReplayChannels
//--------------------------------------------------------------------------------------------------

/**
 * @brief One frame per non-empty line; blank lines never become empty frames.
 */
void TstReplayRowCodec::splitChannelsSkipsEmptyLines()
{
  QList<QStringList> channels;
  DataModel::splitReplayChannels(QByteArray("1,2\n\n3,4\n"), channels);

  QCOMPARE(channels.size(), qsizetype(2));
  QCOMPARE(channels.at(0), (QStringList{QStringLiteral("1"), QStringLiteral("2")}));
  QCOMPARE(channels.at(1), (QStringList{QStringLiteral("3"), QStringLiteral("4")}));
}

QTEST_APPLESS_MAIN(TstReplayRowCodec)

#include "tst_replay_row_codec.moc"
