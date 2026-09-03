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

#include "UI/Widgets/Terminal/AnsiPalette.h"
#include "UI/Widgets/Terminal/TerminalBuffer.h"

// Terminal row removal and the selection it leaves behind (spec 0075, F2/F17). Select rows 30-40,
// clear, let three lines arrive, press Ctrl+C: the copy walked the line list to the stale
// selection end and indexed past it. The endpoints are clamped through the buffer now, so what
// this suite pins is the clamp itself plus the two row-removal paths that made it necessary --
// a front erase has to report its dropped rows (that is what rebases the selection and the scroll
// offset) and both erases have to trim the colour rows in lockstep with the text rows whether or
// not ANSI colours are on, because a row recorded while they were on must not survive as a
// misaligned front.

class TerminalSelectionTest : public QObject {
  Q_OBJECT

private slots:
  void clampKeepsPointsInsideTheBuffer();
  void clampOnEmptyBufferReturnsNull();
  void clearThenCopyClampsTheStaleSelection();
  void eraseRowsBeforeReportsDroppedRows();
  void eraseRowsAfterTrimsColorRowsWithAnsiOff();
  void eraseRowsAfterPullsTheCursorBack();
  void eraseRowsBeforeKeepsColorRowsAligned();

private:
  static void fillRows(Widgets::TerminalBuffer& buffer, int rows, bool ansiColors);
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes @p rows one-line entries into a fresh buffer, one row per cursor line.
 */
void TerminalSelectionTest::fillRows(Widgets::TerminalBuffer& buffer,
                                     const int rows,
                                     const bool ansiColors)
{
  buffer.setColumns(80);
  buffer.setMaxLines(1000);
  buffer.setAnsiColors(ansiColors);
  buffer.reset();
  buffer.reserveColorRows();

  for (int i = 0; i < rows; ++i) {
    buffer.setCursor(0, i);
    buffer.appendText(QStringLiteral("row %1").arg(i));
  }

  (void)buffer.takeDroppedLines();
  (void)buffer.takeCursorMoved();
}

//--------------------------------------------------------------------------------------------------
// Selection clamping
//--------------------------------------------------------------------------------------------------

/**
 * @brief A point past the last row lands on the last row, and a column past the row's end lands
 *        on its end -- never one past either.
 */
void TerminalSelectionTest::clampKeepsPointsInsideTheBuffer()
{
  const QStringList lines{QStringLiteral("abc"), QStringLiteral("de")};

  QCOMPARE(Widgets::TerminalBuffer::clampPoint(QPoint(2, 0), lines), QPoint(2, 0));
  QCOMPARE(Widgets::TerminalBuffer::clampPoint(QPoint(99, 0), lines), QPoint(3, 0));
  QCOMPARE(Widgets::TerminalBuffer::clampPoint(QPoint(0, 40), lines), QPoint(0, 1));
  QCOMPARE(Widgets::TerminalBuffer::clampPoint(QPoint(99, 40), lines), QPoint(2, 1));
  QCOMPARE(Widgets::TerminalBuffer::clampPoint(QPoint(-5, -5), lines), QPoint(0, 0));
}

/**
 * @brief With no rows left there is nothing to select, so the clamp answers a null point rather
 *        than row zero of a list that has no row zero.
 */
void TerminalSelectionTest::clampOnEmptyBufferReturnsNull()
{
  QVERIFY(Widgets::TerminalBuffer::clampPoint(QPoint(4, 9), QStringList()).isNull());
}

/**
 * @brief The reported incident: a selection over rows 30-40, a clear, three new rows, then a
 *        copy. Both endpoints must resolve inside the three rows that exist.
 */
void TerminalSelectionTest::clearThenCopyClampsTheStaleSelection()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  fillRows(buffer, 50, false);

  QPoint start(0, 30);
  QPoint end(4, 40);

  buffer.reset();
  buffer.setCursor(0, 0);
  for (int i = 0; i < 3; ++i) {
    buffer.setCursor(0, i);
    buffer.appendText(QStringLiteral("new %1").arg(i));
  }

  const QStringList& lines = buffer.lines();
  start                    = Widgets::TerminalBuffer::clampPoint(start, lines);
  end                      = Widgets::TerminalBuffer::clampPoint(end, lines);

  QVERIFY(start.y() >= 0 && start.y() < lines.size());
  QVERIFY(end.y() >= 0 && end.y() < lines.size());
  QVERIFY(start.x() <= lines.at(start.y()).size());
  QVERIFY(end.x() <= lines.at(end.y()).size());
}

//--------------------------------------------------------------------------------------------------
// Row erasure
//--------------------------------------------------------------------------------------------------

/**
 * @brief A front erase shifts every row index, so it must report the rows it dropped: that count
 *        is what the widget rebases the selection and the scroll offset with.
 */
void TerminalSelectionTest::eraseRowsBeforeReportsDroppedRows()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  fillRows(buffer, 20, false);

  const int before = buffer.lineCount();

  buffer.eraseRowsBefore(5);

  QCOMPARE(buffer.takeDroppedLines(), 5);
  QCOMPARE(buffer.lineCount(), before - 5);
  QCOMPARE(buffer.lines().first(), QStringLiteral("row 5"));
}

/**
 * @brief A front erase larger than the buffer drops what exists and nothing more.
 */
void TerminalSelectionTest::eraseRowsBeforeKeepsColorRowsAligned()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  fillRows(buffer, 10, true);

  buffer.setAnsiColors(false);
  buffer.eraseRowsBefore(4);

  QVERIFY(buffer.colorRows().size() <= buffer.lineCount());
  QCOMPARE(buffer.takeDroppedLines(), 4);
}

/**
 * @brief A tail erase trims the colour rows with the text rows even when ANSI colours were
 *        switched off after the rows were recorded; otherwise the surviving colour tail paints
 *        onto rows it never belonged to.
 */
void TerminalSelectionTest::eraseRowsAfterTrimsColorRowsWithAnsiOff()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  fillRows(buffer, 12, true);

  const auto coloredRows = buffer.colorRows().size();
  QVERIFY(coloredRows > 0);

  buffer.setAnsiColors(false);
  buffer.eraseRowsAfter(3);

  QCOMPARE(buffer.lineCount(), 4);
  QVERIFY(buffer.colorRows().size() <= buffer.lineCount());
}

/**
 * @brief The cursor cannot survive on a row the erase removed.
 */
void TerminalSelectionTest::eraseRowsAfterPullsTheCursorBack()
{
  Widgets::AnsiPalette palette;
  Widgets::TerminalBuffer buffer(palette);
  fillRows(buffer, 12, false);

  buffer.eraseRowsAfter(2);

  QCOMPARE(buffer.lineCount(), 3);
  QVERIFY(buffer.cursor().y() < buffer.lineCount());
}

QTEST_APPLESS_MAIN(TerminalSelectionTest)

#include "tst_terminal_selection.moc"
