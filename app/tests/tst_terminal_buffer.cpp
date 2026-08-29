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

#include <QPoint>
#include <QString>
#include <QStringList>
#include <QTest>

#include "UI/Widgets/Terminal/AnsiPalette.h"
#include "UI/Widgets/Terminal/TerminalBuffer.h"

// Every test function here is self-contained: each builds its own buffer, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Known-answer coverage of the terminal's line store: word wrapping at the column count,
 *        the scrollback cap and its front trim, duplicate collapsing with and without a timestamp
 *        prefix, cursor clamping, and the drained bookkeeping the widget republishes from.
 */
class TstTerminalBuffer : public QObject {
  Q_OBJECT

private slots:
  void scanPrintableRun_data();
  void scanPrintableRun();

  void appendWrapsAtColumnCount();
  void appendPastEndPadsWithSpaces();

  void cursorClamp_data();
  void cursorClamp();

  void cursorMoveIsDrained();

  void scrollbackCapTrimsFront();
  void trailingNewlineTrimsOneMore();
  void trimShiftsCursorRow();
  void trimToMaxLinesAfterCapDrop();

  void duplicateCollapseCountsRepeats();
  void duplicateCollapseNeedsAdjacentRows();

  void timestampPrefix_data();
  void timestampPrefix();
  void timestampedDuplicatesCollapse();

  void removeFromCursorBlanksCharacters();
  void eraseRowsKeepsRepeatCountsAligned();

  void visualBottomRowFollowsWrapping();
  void colorRowsTrackCharacters();
  void resetClearsEveryStore();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drives @p data through the buffer the way Terminal::append() does, minus the escape
 *        parser: printable runs accumulate, control characters go to processText().
 */
static void feed(Widgets::TerminalBuffer& buffer, const QString& data)
{
  QString text;
  int pos       = 0;
  const int len = data.size();

  while (pos < len) {
    const int runStart = pos;
    pos                = Widgets::TerminalBuffer::scanPrintableRun(data, pos);

    if (pos > runStart)
      text.append(QStringView(data).mid(runStart, pos - runStart));

    if (pos < len) {
      (void)buffer.processText(data[pos], text, false);
      ++pos;
    }
  }

  buffer.appendText(text);
}

//--------------------------------------------------------------------------------------------------
// Printable-run scanning
//--------------------------------------------------------------------------------------------------

void TstTerminalBuffer::scanPrintableRun_data()
{
  QTest::addColumn<QString>("data");
  QTest::addColumn<int>("expected");

  QTest::newRow("all printable") << QStringLiteral("hello") << 5;
  QTest::newRow("stops at newline") << QStringLiteral("ab\ncd") << 2;
  QTest::newRow("stops at carriage return") << QStringLiteral("ab\rcd") << 2;
  QTest::newRow("stops at escape") << QStringLiteral("ab\x1b[0m") << 2;
  QTest::newRow("stops at backspace") << QStringLiteral("ab\bcd") << 2;
  QTest::newRow("stops at tab") << QStringLiteral("ab\tcd") << 2;
  QTest::newRow("stops at delete") << QStringLiteral("ab\x7f") << 2;
  QTest::newRow("stops at bell") << QStringLiteral("ab\acd") << 2;
  QTest::newRow("leading control byte") << QStringLiteral("\nab") << 0;
  QTest::newRow("empty") << QString() << 0;
}

/**
 * @brief The run scanner is the fast lane of the append path: it must stop on exactly the bytes
 *        processText() knows how to handle, and on every other C0 control character.
 */
void TstTerminalBuffer::scanPrintableRun()
{
  QFETCH(QString, data);
  QFETCH(int, expected);

  QCOMPARE(Widgets::TerminalBuffer::scanPrintableRun(data, 0), expected);
}

//--------------------------------------------------------------------------------------------------
// Append & wrapping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writing past the column count opens a new row; the cursor lands at the start of it and
 *        the overflow is what the next row holds.
 */
void TstTerminalBuffer::appendWrapsAtColumnCount()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(8);

  buffer.appendText(QStringLiteral("abcdefghij"));

  QCOMPARE(buffer.lines(), QStringList({QStringLiteral("abcdefgh"), QStringLiteral("ij")}));
  QCOMPARE(buffer.cursor(), QPoint(2, 1));
  QCOMPARE(buffer.lineCount(), 2);
}

/**
 * @brief A cursor parked past the end of its row pads the gap with spaces rather than writing at
 *        the wrong column, which is what carriage returns and cursor-position sequences rely on.
 */
void TstTerminalBuffer::appendPastEndPadsWithSpaces()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(40);

  buffer.appendText(QStringLiteral("ab"));
  buffer.setCursor(5, 0);
  buffer.appendText(QStringLiteral("z"));

  QCOMPARE(buffer.lines().at(0), QStringLiteral("ab   z"));
}

//--------------------------------------------------------------------------------------------------
// Cursor
//--------------------------------------------------------------------------------------------------

void TstTerminalBuffer::cursorClamp_data()
{
  QTest::addColumn<QPoint>("requested");
  QTest::addColumn<QPoint>("expected");

  QTest::newRow("inside") << QPoint(3, 4) << QPoint(3, 4);
  QTest::newRow("negative column") << QPoint(-5, 2) << QPoint(0, 2);
  QTest::newRow("negative row") << QPoint(2, -7) << QPoint(2, 0);
  QTest::newRow("both negative") << QPoint(-1, -1) << QPoint(0, 0);
  QTest::newRow("column past width") << QPoint(999, 1) << QPoint(10, 1);
  QTest::newRow("row past scrollback") << QPoint(1, 999) << QPoint(1, 50);
  QTest::newRow("column at width") << QPoint(10, 0) << QPoint(10, 0);
}

/**
 * @brief The cursor is clamped to the column count and the scrollback cap, both inclusive: the
 *        column one past the last one is where a wrap is detected, and the row cap is what stops
 *        an absolute-positioning escape from growing the buffer without bound.
 */
void TstTerminalBuffer::cursorClamp()
{
  QFETCH(QPoint, requested);
  QFETCH(QPoint, expected);

  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(10);
  buffer.setMaxLines(50);

  buffer.setCursor(requested);
  QCOMPARE(buffer.cursor(), expected);
}

/**
 * @brief The buffer is not a QObject: a cursor move is recorded once and drained by the facade,
 *        and a write that lands on the same position records nothing.
 */
void TstTerminalBuffer::cursorMoveIsDrained()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(20);

  QVERIFY(!buffer.takeCursorMoved());

  buffer.setCursor(4, 2);
  QVERIFY(buffer.takeCursorMoved());
  QVERIFY(!buffer.takeCursorMoved());

  buffer.setCursor(4, 2);
  QVERIFY(!buffer.takeCursorMoved());
}

//--------------------------------------------------------------------------------------------------
// Scrollback cap
//--------------------------------------------------------------------------------------------------

/**
 * @brief Once the row count reaches the cap, every further row drops one from the front; the
 *        dropped total accumulates for the single drain the widget performs per chunk.
 */
void TstTerminalBuffer::scrollbackCapTrimsFront()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(80);
  buffer.setMaxLines(3);

  feed(buffer, QStringLiteral("1\n2\n3\n4\n5"));

  QCOMPARE(buffer.lines(),
           QStringList({QStringLiteral("3"), QStringLiteral("4"), QStringLiteral("5")}));
  QCOMPARE(buffer.takeDroppedLines(), 2);
  QCOMPARE(buffer.takeDroppedLines(), 0);
}

/**
 * @brief A chunk that ends on a newline leaves the cursor on a row that does not exist yet, and
 *        the closing write of that chunk already makes room for it: the store settles one row
 *        below the cap rather than at it.
 */
void TstTerminalBuffer::trailingNewlineTrimsOneMore()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(80);
  buffer.setMaxLines(3);

  feed(buffer, QStringLiteral("1\n2\n3\n4\n5\n"));

  QCOMPARE(buffer.lines(), QStringList({QStringLiteral("4"), QStringLiteral("5")}));
  QCOMPARE(buffer.takeDroppedLines(), 3);
}

/**
 * @brief A front trim moves the cursor row with the rows it kept, so the next write still lands
 *        on the row the caller was addressing.
 */
void TstTerminalBuffer::trimShiftsCursorRow()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(80);
  buffer.setMaxLines(3);

  feed(buffer, QStringLiteral("1\n2\n3\n"));
  QCOMPARE(buffer.lines(), QStringList({QStringLiteral("2"), QStringLiteral("3")}));
  QCOMPARE(buffer.cursor(), QPoint(0, 2));
  QCOMPARE(buffer.takeDroppedLines(), 1);

  feed(buffer, QStringLiteral("4"));
  QCOMPARE(buffer.lines(),
           QStringList({QStringLiteral("2"), QStringLiteral("3"), QStringLiteral("4")}));
  QCOMPARE(buffer.cursor(), QPoint(1, 2));
  QCOMPARE(buffer.takeDroppedLines(), 0);
}

/**
 * @brief Lowering the scrollback cap on an already-full buffer trims the excess in one step and
 *        reports how many rows went, which is what the widget shifts its view state by.
 */
void TstTerminalBuffer::trimToMaxLinesAfterCapDrop()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(80);
  buffer.setMaxLines(100);

  feed(buffer, QStringLiteral("a\nb\nc\nd\ne\n"));
  QCOMPARE(buffer.lineCount(), 5);

  buffer.setMaxLines(2);
  QCOMPARE(buffer.trimToMaxLines(), 3);
  QCOMPARE(buffer.lines(), QStringList({QStringLiteral("d"), QStringLiteral("e")}));
  QCOMPARE(buffer.takeDroppedLines(), 3);
  QCOMPARE(buffer.trimToMaxLines(), 0);
}

//--------------------------------------------------------------------------------------------------
// Duplicate collapsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Consecutive identical rows merge into the first one, and the repeat count the badge
 *        paints is what carries the number of occurrences.
 */
void TstTerminalBuffer::duplicateCollapseCountsRepeats()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(80);
  buffer.setMaxLines(1000);
  buffer.setCollapseDuplicates(true);

  feed(buffer, QStringLiteral("hello\nhello\nhello\n"));

  QCOMPARE(buffer.lines(), QStringList({QStringLiteral("hello")}));
  QCOMPARE(buffer.repeatCounts(), QList<int>({3}));
}

/**
 * @brief Only the row that just closed collapses, and only into the row directly above it: a
 *        repeat separated by a different row starts its own count.
 */
void TstTerminalBuffer::duplicateCollapseNeedsAdjacentRows()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(80);
  buffer.setMaxLines(1000);
  buffer.setCollapseDuplicates(true);

  feed(buffer, QStringLiteral("a\nb\na\n"));

  QCOMPARE(buffer.lines(),
           QStringList({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("a")}));
  QCOMPARE(buffer.repeatCounts(), QList<int>({1, 1, 1}));
}

//--------------------------------------------------------------------------------------------------
// Timestamp prefixes
//--------------------------------------------------------------------------------------------------

void TstTerminalBuffer::timestampPrefix_data()
{
  QTest::addColumn<QString>("line");
  QTest::addColumn<bool>("expected");

  QTest::newRow("well formed") << QStringLiteral("12:34:56.789 -> x") << true;
  QTest::newRow("exactly the stamp") << QStringLiteral("12:34:56.789 -> ") << true;
  QTest::newRow("one char short") << QStringLiteral("12:34:56.789 ->") << false;
  QTest::newRow("letters for digits") << QStringLiteral("ab:34:56.789 -> x") << false;
  QTest::newRow("wrong separator") << QStringLiteral("12-34:56.789 -> x") << false;
  QTest::newRow("missing arrow") << QStringLiteral("12:34:56.789    x") << false;
  QTest::newRow("plain text") << QStringLiteral("hello world here") << false;
  QTest::newRow("empty") << QString() << false;
}

/**
 * @brief The stamp shape is fixed at "HH:mm:ss.zzz -> "; duplicate collapsing skips exactly that
 *        many characters, so a near-miss must not be treated as a prefix.
 */
void TstTerminalBuffer::timestampPrefix()
{
  QFETCH(QString, line);
  QFETCH(bool, expected);

  QCOMPARE(Widgets::TerminalBuffer::hasTimestampPrefix(line), expected);
}

/**
 * @brief With timestamps on, two rows whose stamps differ but whose payload matches still
 *        collapse; with timestamps off the stamps are content and the rows stay apart.
 */
void TstTerminalBuffer::timestampedDuplicatesCollapse()
{
  const QString stamped = QStringLiteral("12:00:00.000 -> hi\n12:00:01.500 -> hi\n");

  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer stampAware(palette);
  stampAware.setColumns(80);
  stampAware.setMaxLines(1000);
  stampAware.setCollapseDuplicates(true);
  stampAware.setShowTimestamps(true);
  feed(stampAware, stamped);

  QCOMPARE(stampAware.lines(), QStringList({QStringLiteral("12:00:00.000 -> hi")}));
  QCOMPARE(stampAware.repeatCounts(), QList<int>({2}));

  Widgets::TerminalBuffer stampBlind(palette);
  stampBlind.setColumns(80);
  stampBlind.setMaxLines(1000);
  stampBlind.setCollapseDuplicates(true);
  stampBlind.setShowTimestamps(false);
  feed(stampBlind, stamped);

  QCOMPARE(stampBlind.lineCount(), 2);
  QCOMPARE(stampBlind.repeatCounts(), QList<int>({1, 1}));
}

//--------------------------------------------------------------------------------------------------
// Erasure
//--------------------------------------------------------------------------------------------------

/**
 * @brief Erasing blanks characters in place on either side of the cursor and never moves it; the
 *        row keeps its length so the columns after the erase stay where they were.
 */
void TstTerminalBuffer::removeFromCursorBlanksCharacters()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(80);

  buffer.appendText(QStringLiteral("abcdef"));
  buffer.removeFromCursor(Widgets::AnsiEraseDirection::Left, 2);
  QCOMPARE(buffer.lines().at(0), QStringLiteral("abcd  "));
  QCOMPARE(buffer.cursor(), QPoint(6, 0));

  Widgets::TerminalBuffer rightward(palette);
  rightward.setColumns(80);
  rightward.appendText(QStringLiteral("abcdef"));
  rightward.setCursor(2, 0);
  rightward.removeFromCursor(Widgets::AnsiEraseDirection::Right, 3);
  QCOMPARE(rightward.lines().at(0), QStringLiteral("ab   f"));
  QCOMPARE(rightward.cursor(), QPoint(2, 0));
}

/**
 * @brief The repeat counts are indexed by row, so a row erase has to take the same rows out of
 *        them; a misaligned count is what would paint a badge on the wrong line.
 */
void TstTerminalBuffer::eraseRowsKeepsRepeatCountsAligned()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(80);
  buffer.setMaxLines(1000);

  feed(buffer, QStringLiteral("a\nb\nc\nd\n"));
  QCOMPARE(buffer.repeatCounts().size(), qsizetype(4));

  buffer.eraseRowsAfter(1);
  QCOMPARE(buffer.lines(), QStringList({QStringLiteral("a"), QStringLiteral("b")}));
  QCOMPARE(buffer.repeatCounts().size(), qsizetype(2));

  buffer.eraseRowsBefore(1);
  QCOMPARE(buffer.lines(), QStringList({QStringLiteral("b")}));
  QCOMPARE(buffer.repeatCounts().size(), qsizetype(1));
}

//--------------------------------------------------------------------------------------------------
// Derived geometry & stores
//--------------------------------------------------------------------------------------------------

/**
 * @brief Autoscroll pins the viewport to the last visual row of the cursor's line, which counts
 *        the wrapped segments rather than the row index alone.
 */
void TstTerminalBuffer::visualBottomRowFollowsWrapping()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(10);
  buffer.setMaxLines(1000);

  buffer.appendText(QString(25, QLatin1Char('x')));
  buffer.setCursor(0, 0);
  QCOMPARE(buffer.visualBottomRow(), 2);

  buffer.setCursor(0, 7);
  QCOMPARE(buffer.visualBottomRow(), 7);
}

/**
 * @brief With ANSI colors on, every written character gets a color cell, and the color rows stay
 *        indexed the same way the text rows are.
 */
void TstTerminalBuffer::colorRowsTrackCharacters()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(80);
  buffer.setAnsiColors(true);

  buffer.appendText(QStringLiteral("abc"));

  QCOMPARE(buffer.colorRows().size(), qsizetype(1));
  QCOMPARE(buffer.colorRows().at(0).size(), qsizetype(3));
}

/**
 * @brief reset() is what clear() and a new connection run: every row store empties and the
 *        pending drop bookkeeping goes with it.
 */
void TstTerminalBuffer::resetClearsEveryStore()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  buffer.setColumns(80);
  buffer.setMaxLines(2);
  buffer.setAnsiColors(true);

  feed(buffer, QStringLiteral("a\nb\nc\n"));
  QVERIFY(buffer.lineCount() > 0);

  buffer.reset();

  QVERIFY(buffer.lines().isEmpty());
  QVERIFY(buffer.colorRows().isEmpty());
  QVERIFY(buffer.repeatCounts().isEmpty());
  QCOMPARE(buffer.takeDroppedLines(), 0);
}

QTEST_APPLESS_MAIN(TstTerminalBuffer)

#include "tst_terminal_buffer.moc"
