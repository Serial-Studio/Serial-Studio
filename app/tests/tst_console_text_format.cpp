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
#include <QTest>

#include "Console/TextFormat.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Pins the console's pure text shaping: line-ending normalization with timestamps, the
 *        non-printable filter used when VT-100 emulation is off, and the hex-dump layout.
 */
class TstConsoleTextFormat : public QObject {
  Q_OBJECT

private slots:
  void normalizesLineEndings();
  void stampsOnlyLineStarts();
  void stampsNothingWithoutTimestamp();
  void splitCrLfAcrossChunks();
  void whitespaceOnlyLineIsNotStamped();

  void filterControlChars_data();
  void filterControlChars();

  void hexDumpEmpty();
  void hexDumpPartialRow();
  void hexDumpFullRowAndOverflow();
};

//--------------------------------------------------------------------------------------------------
// formatIncoming
//--------------------------------------------------------------------------------------------------

/**
 * @brief CRLF and lone CR both collapse to LF, so the view never renders a blank phantom line.
 */
void TstConsoleTextFormat::normalizesLineEndings()
{
  Console::TextFormat::LineState state;
  const auto out =
    Console::TextFormat::formatIncoming(QStringLiteral("a\r\nb\rc\n"), state, QString());

  QCOMPARE(out, QStringLiteral("a\nb\nc\n"));
}

/**
 * @brief The stamp is written once per line that begins in this chunk, never mid-line.
 */
void TstConsoleTextFormat::stampsOnlyLineStarts()
{
  Console::TextFormat::LineState state;
  const auto first =
    Console::TextFormat::formatIncoming(QStringLiteral("one\ntw"), state, QStringLiteral("T "));
  const auto second =
    Console::TextFormat::formatIncoming(QStringLiteral("o\n"), state, QStringLiteral("T "));

  QCOMPARE(first, QStringLiteral("T one\nT tw"));
  QCOMPARE(second, QStringLiteral("o\n"));
  QVERIFY(state.isStartingLine);
}

/**
 * @brief With no stamp the text is only normalized.
 */
void TstConsoleTextFormat::stampsNothingWithoutTimestamp()
{
  Console::TextFormat::LineState state;
  const auto out = Console::TextFormat::formatIncoming(QStringLiteral("x\ny"), state, QString());

  QCOMPARE(out, QStringLiteral("x\ny"));
  QVERIFY(!state.isStartingLine);
}

/**
 * @brief A CRLF split across two chunks stays one line break: the LF that opens the next chunk
 *        is dropped because the previous one ended on the CR.
 */
void TstConsoleTextFormat::splitCrLfAcrossChunks()
{
  Console::TextFormat::LineState state;
  const auto first =
    Console::TextFormat::formatIncoming(QStringLiteral("line\r"), state, QString());

  QVERIFY(state.lastCharWasCR);

  const auto second =
    Console::TextFormat::formatIncoming(QStringLiteral("\nnext"), state, QString());

  QCOMPARE(first + second, QStringLiteral("line\nnext"));
}

/**
 * @brief A line carrying only whitespace gets no stamp: a keep-alive newline must not print a
 *        timestamp of its own.
 */
void TstConsoleTextFormat::whitespaceOnlyLineIsNotStamped()
{
  Console::TextFormat::LineState state;
  const auto out =
    Console::TextFormat::formatIncoming(QStringLiteral("   \ndata\n"), state, QStringLiteral("T "));

  QCOMPARE(out, QStringLiteral("   \nT data\n"));
}

//--------------------------------------------------------------------------------------------------
// filterControlChars
//--------------------------------------------------------------------------------------------------

void TstConsoleTextFormat::filterControlChars_data()
{
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("expected");

  QTest::newRow("printable ascii") << QStringLiteral("abc") << QStringLiteral("abc");
  QTest::newRow("keeps whitespace") << QStringLiteral("a\r\n\tb") << QStringLiteral("a\r\n\tb");
  QTest::newRow("keeps escape") << QStringLiteral("a\x1B[0m") << QStringLiteral("a\x1B[0m");
  QTest::newRow("dots a bell") << QStringLiteral("a\x07") << QStringLiteral("a.");
  QTest::newRow("dots a nul") << QString(QChar(u'\0')) << QStringLiteral(".");
  QTest::newRow("keeps non-ascii") << QStringLiteral("añ") << QStringLiteral("añ");
  QTest::newRow("empty") << QString() << QString();
}

/**
 * @brief Only CR, LF, TAB, ESC and printable code points survive; everything else becomes '.'.
 */
void TstConsoleTextFormat::filterControlChars()
{
  QFETCH(QString, input);
  QFETCH(QString, expected);
  QCOMPARE(Console::TextFormat::filterControlChars(input), expected);
}

//--------------------------------------------------------------------------------------------------
// hexDump
//--------------------------------------------------------------------------------------------------

/**
 * @brief An empty payload dumps to the trailing newline alone: no header, no empty row.
 */
void TstConsoleTextFormat::hexDumpEmpty()
{
  QCOMPARE(Console::TextFormat::hexDump(QByteArray()), QStringLiteral("\n"));
}

/**
 * @brief A short payload pads both the hex and the ASCII column so the row keeps its width.
 */
void TstConsoleTextFormat::hexDumpPartialRow()
{
  const auto dump  = Console::TextFormat::hexDump(QByteArrayLiteral("AB"));
  const auto lines = dump.split(QLatin1Char('\n'));

  QCOMPARE(lines.size(), 3);
  QCOMPARE(lines.at(0).size(), 78);
  QVERIFY(lines.at(0).startsWith(QStringLiteral("000000 | 41 42 ")));

  const auto ascii = QStringLiteral("| ") + QStringLiteral("AB") + QString(14, QLatin1Char(' '))
                   + QStringLiteral(" |");
  QVERIFY(lines.at(0).endsWith(ascii));
}

/**
 * @brief A full row takes the SIMD ASCII lane and a 17th byte opens a second row at offset 16.
 */
void TstConsoleTextFormat::hexDumpFullRowAndOverflow()
{
  QByteArray payload(16, 'x');
  payload[0]  = char(0x00);
  payload[15] = char(0x7F);
  payload.append('y');

  const auto dump  = Console::TextFormat::hexDump(payload);
  const auto lines = dump.split(QLatin1Char('\n'));

  QCOMPARE(lines.size(), 4);
  QCOMPARE(lines.at(0).size(), 78);
  QCOMPARE(lines.at(1).size(), 78);
  QVERIFY(lines.at(0).startsWith(QStringLiteral("000000 | 00 78 ")));
  QVERIFY(lines.at(1).startsWith(QStringLiteral("000010 | 79 ")));

  const auto ascii = QStringLiteral("| .") + QString(14, QLatin1Char('x')) + QStringLiteral(". |");
  QVERIFY(lines.at(0).endsWith(ascii));
}

QTEST_APPLESS_MAIN(TstConsoleTextFormat)

#include "tst_console_text_format.moc"
