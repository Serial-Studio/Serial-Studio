/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <climits>
#include <QPoint>
#include <QString>
#include <QStringList>
#include <QTest>

#include "UI/Widgets/Terminal/AnsiStateMachine.h"

// Every test function here is self-contained: each builds its own sink and machine, so Qt
// Test's declaration-order execution is never load-bearing.

//--------------------------------------------------------------------------------------------------
// Test doubles
//--------------------------------------------------------------------------------------------------

/**
 * @brief An AnsiSink that records the operation stream instead of touching a terminal. It
 *        also tracks the cursor, because several sequences read it back mid-dispatch.
 */
class RecordingSink final : public Widgets::AnsiSink {
public:
  RecordingSink() : cursor(0, 0) {}

  [[nodiscard]] QPoint currentCursor() const override { return cursor; }

  void eraseAllRows() override { ops.append(QStringLiteral("allRows")); }

  void eraseRowsAfter(int row) override { ops.append(QStringLiteral("rowsAfter(%1)").arg(row)); }

  void eraseRowsBefore(int row) override { ops.append(QStringLiteral("rowsBefore(%1)").arg(row)); }

  void setCursorHidden(bool hidden) override
  {
    ops.append(QStringLiteral("hidden(%1)").arg(hidden ? 1 : 0));
  }

  void moveCursor(const QPoint& position) override
  {
    cursor = position;
    ops.append(QStringLiteral("move(%1,%2)").arg(position.x()).arg(position.y()));
  }

  void applySgrCodes(const QList<int>& codes) override
  {
    QStringList text;
    for (const int code : codes)
      text.append(QString::number(code));

    ops.append(QStringLiteral("sgr(%1)").arg(text.join(QLatin1Char(','))));
  }

  void eraseFromCursor(Widgets::AnsiEraseDirection direction, int length) override
  {
    const QString side =
      (direction == Widgets::AnsiEraseDirection::Left) ? QStringLiteral("L") : QStringLiteral("R");
    const QString len = (length == INT_MAX) ? QStringLiteral("max") : QString::number(length);
    ops.append(QStringLiteral("erase%1(%2)").arg(side, len));
  }

  QPoint cursor;
  QStringList ops;
};

/**
 * @brief Byte-level coverage of the terminal's escape-sequence parser, driven through the
 *        same two-call contract the widget uses: beginEscape() on ESC, feed() until the
 *        machine says it is back in the text state.
 */
class TstAnsiStateMachine : public QObject {
  Q_OBJECT

private slots:
  void sequences_data();
  void sequences();

  void savedCursorRoundTrip();
  void oscTerminatedByEscape();
  void truncatedSequenceStaysPending();
  void parameterOverflowIsBounded();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drives @p machine over @p bytes exactly the way Terminal::append() does: an ESC seen
 *        in the text state opens a sequence, everything else in the text state is the
 *        facade's business and never reaches the parser.
 */
static void feedBytes(Widgets::AnsiStateMachine& machine, const QString& bytes)
{
  for (const QChar byte : bytes) {
    if (machine.inTextState()) {
      if (byte == QChar(0x1b))
        machine.beginEscape();

      continue;
    }

    machine.feed(byte);
  }
}

//--------------------------------------------------------------------------------------------------
// Sequence sweep
//--------------------------------------------------------------------------------------------------

void TstAnsiStateMachine::sequences_data()
{
  QTest::addColumn<QString>("bytes");
  QTest::addColumn<QString>("expected");

  QTest::newRow("CUU default") << QStringLiteral("\x1b[A") << QStringLiteral("move(3,4)");
  QTest::newRow("CUU with count") << QStringLiteral("\x1b[2A") << QStringLiteral("move(3,3)");
  QTest::newRow("CUD") << QStringLiteral("\x1b[B") << QStringLiteral("move(3,6)");
  QTest::newRow("CUF") << QStringLiteral("\x1b[3C") << QStringLiteral("move(6,5)");
  QTest::newRow("CUB") << QStringLiteral("\x1b[2D") << QStringLiteral("move(1,5)");
  QTest::newRow("CNL") << QStringLiteral("\x1b[E") << QStringLiteral("move(0,6)");
  QTest::newRow("CPL") << QStringLiteral("\x1b[F") << QStringLiteral("move(0,4)");
  QTest::newRow("CUU clamps at top") << QStringLiteral("\x1b[99A") << QStringLiteral("move(3,0)");

  QTest::newRow("CUP row and column")
    << QStringLiteral("\x1b[10;20H") << QStringLiteral("move(19,9)");
  QTest::newRow("CUP home") << QStringLiteral("\x1b[H") << QStringLiteral("move(0,0)");
  QTest::newRow("CUP row only") << QStringLiteral("\x1b[5H") << QStringLiteral("move(0,4)");
  QTest::newRow("HVP alias") << QStringLiteral("\x1b[2;3f") << QStringLiteral("move(2,1)");
  QTest::newRow("CHA column") << QStringLiteral("\x1b[7G") << QStringLiteral("move(6,5)");
  QTest::newRow("VPA row") << QStringLiteral("\x1b[3d") << QStringLiteral("move(3,2)");

  QTest::newRow("ED below") << QStringLiteral("\x1b[J")
                            << QStringLiteral("eraseR(max);rowsAfter(5)");
  QTest::newRow("ED above") << QStringLiteral("\x1b[1J")
                            << QStringLiteral("eraseL(max);rowsBefore(5);move(3,0)");
  QTest::newRow("ED all") << QStringLiteral("\x1b[2J") << QStringLiteral("allRows");
  QTest::newRow("ED scrollback") << QStringLiteral("\x1b[3J") << QStringLiteral("allRows");

  QTest::newRow("EL right") << QStringLiteral("\x1b[K") << QStringLiteral("eraseR(max)");
  QTest::newRow("EL left") << QStringLiteral("\x1b[1K") << QStringLiteral("eraseL(max)");
  QTest::newRow("EL whole") << QStringLiteral("\x1b[2K")
                            << QStringLiteral("eraseR(max);eraseL(max)");
  QTest::newRow("DCH") << QStringLiteral("\x1b[3P") << QStringLiteral("eraseL(3);eraseR(max)");

  QTest::newRow("SGR single") << QStringLiteral("\x1b[31m") << QStringLiteral("sgr(31)");
  QTest::newRow("SGR run") << QStringLiteral("\x1b[1;31;44m") << QStringLiteral("sgr(1,31,44)");
  QTest::newRow("SGR empty means reset") << QStringLiteral("\x1b[m") << QStringLiteral("sgr(0)");
  QTest::newRow("SGR extended run")
    << QStringLiteral("\x1b[38;5;196m") << QStringLiteral("sgr(38,5,196)");

  QTest::newRow("DECTCEM hide") << QStringLiteral("\x1b[?25l") << QStringLiteral("hidden(1)");
  QTest::newRow("DECTCEM show") << QStringLiteral("\x1b[?25h") << QStringLiteral("hidden(0)");
  QTest::newRow("other private mode is inert") << QStringLiteral("\x1b[?1049h") << QString();
  QTest::newRow("private mode gates erase") << QStringLiteral("\x1b[?1J") << QString();
  QTest::newRow("private mode gates sgr") << QStringLiteral("\x1b[?31m") << QString();
  QTest::newRow("private mode gates cursor move") << QStringLiteral("\x1b[?2A") << QString();

  QTest::newRow("reverse index") << QStringLiteral("\x1bM") << QStringLiteral("move(3,4)");
  QTest::newRow("charset select is inert") << QStringLiteral("\x1b(B") << QString();
  QTest::newRow("unknown escape is inert") << QStringLiteral("\x1bZ") << QString();
  QTest::newRow("unknown CSI letter is inert") << QStringLiteral("\x1b[5X") << QString();
  QTest::newRow("OSC to BEL is inert") << QStringLiteral("\x1b]0;title\x07") << QString();

  QTest::newRow("two sequences in a row")
    << QStringLiteral("\x1b[31m\x1b[K") << QStringLiteral("sgr(31);eraseR(max)");
}

/**
 * @brief Feeds one sequence at cursor (3,5) and pins the exact operation stream it produces.
 *        The cursor start is off-origin so a dropped or swapped coordinate cannot pass.
 */
void TstAnsiStateMachine::sequences()
{
  QFETCH(QString, bytes);
  QFETCH(QString, expected);

  RecordingSink sink;
  sink.cursor = QPoint(3, 5);

  Widgets::AnsiStateMachine machine(sink);
  feedBytes(machine, bytes);

  QCOMPARE(sink.ops.join(QLatin1Char(';')), expected);
  QVERIFY(machine.inTextState());
}

//--------------------------------------------------------------------------------------------------
// Stateful sequences
//--------------------------------------------------------------------------------------------------

/**
 * @brief DECSC/DECRC and their CSI equivalents must round-trip the cursor through the
 *        machine's own save slot, not through the sink.
 */
void TstAnsiStateMachine::savedCursorRoundTrip()
{
  RecordingSink escSink;
  escSink.cursor = QPoint(4, 9);

  Widgets::AnsiStateMachine escMachine(escSink);
  feedBytes(escMachine, QStringLiteral("\x1b\x37\x1b[1;1H\x1b\x38"));
  QCOMPARE(escSink.ops.join(QLatin1Char(';')), QStringLiteral("move(0,0);move(4,9)"));
  QCOMPARE(escSink.cursor, QPoint(4, 9));

  RecordingSink csiSink;
  csiSink.cursor = QPoint(2, 7);

  Widgets::AnsiStateMachine csiMachine(csiSink);
  feedBytes(csiMachine, QStringLiteral("\x1b[s\x1b[1;1H\x1b[u"));
  QCOMPARE(csiSink.ops.join(QLatin1Char(';')), QStringLiteral("move(0,0);move(2,7)"));
  QCOMPARE(csiSink.cursor, QPoint(2, 7));
}

/**
 * @brief An OSC string may be terminated by ESC instead of BEL, which re-enters the escape
 *        state; the sequence that follows has to parse normally.
 */
void TstAnsiStateMachine::oscTerminatedByEscape()
{
  RecordingSink sink;
  sink.cursor = QPoint(3, 5);

  Widgets::AnsiStateMachine machine(sink);
  feedBytes(machine, QStringLiteral("\x1b]0;title\x1b[31m"));

  QCOMPARE(sink.ops.join(QLatin1Char(';')), QStringLiteral("sgr(31)"));
  QVERIFY(machine.inTextState());
}

/**
 * @brief A sequence cut off mid-flight leaves the machine waiting rather than emitting a
 *        half-formed operation; the continuation bytes complete it.
 */
void TstAnsiStateMachine::truncatedSequenceStaysPending()
{
  RecordingSink sink;
  sink.cursor = QPoint(3, 5);

  Widgets::AnsiStateMachine machine(sink);
  feedBytes(machine, QStringLiteral("\x1b["));
  QVERIFY(!machine.inTextState());
  QVERIFY(sink.ops.isEmpty());

  feedBytes(machine, QStringLiteral("31;4"));
  QVERIFY(!machine.inTextState());
  QVERIFY(sink.ops.isEmpty());

  feedBytes(machine, QStringLiteral("4m"));
  QVERIFY(machine.inTextState());
  QCOMPARE(sink.ops.join(QLatin1Char(';')), QStringLiteral("sgr(31,44)"));
}

/**
 * @brief A parameter of unbounded digit length must not overflow the accumulator; the parser
 *        stops growing it once it passes its cap.
 */
void TstAnsiStateMachine::parameterOverflowIsBounded()
{
  RecordingSink sink;
  sink.cursor = QPoint(3, 5);

  Widgets::AnsiStateMachine machine(sink);
  feedBytes(machine, QStringLiteral("\x1b[99999999999999999999m"));

  QCOMPARE(sink.ops.size(), 1);
  QVERIFY(sink.ops.first().startsWith(QStringLiteral("sgr(")));
  QVERIFY(machine.inTextState());
}

QTEST_APPLESS_MAIN(TstAnsiStateMachine)

#include "tst_ansi_state_machine.moc"
