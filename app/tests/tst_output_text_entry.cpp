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

#include <QString>
#include <QStringList>
#include <QTest>
#include <QVariant>

#include "Core/DataModel/OutputTextEntry.h"
#include "Core/DataModel/OutputWidget.h"

using namespace DataModel;

namespace {

OutputWidget numberField(int decimals, double minValue, double maxValue)
{
  OutputWidget w;
  w.title     = QStringLiteral("Setpoint");
  w.type      = OutputWidgetType::TextField;
  w.inputKind = OutputInputKind::Number;
  w.decimals  = decimals;
  w.minValue  = minValue;
  w.maxValue  = maxValue;
  return w;
}

QString press(const QString& keys, QString entry = QString())
{
  for (const QChar key : keys)
    entry = keypadApply(entry, key);

  return entry;
}

}  // namespace

/**
 * @brief The keypad and command-history rules of the text-field styles (spec 0118 AC3, AC4), as
 *        the Core functions the QML fields call.
 */
class OutputTextEntryTests : public QObject {
  Q_OBJECT

private slots:
  void decimalIsOfferedOnlyForFractionalNumberFields();
  void signIsOfferedOnlyWhenMinIsNegative();
  void digitsReplaceALoneZero();
  void pointOnEmptyReadsAsZeroPointAndIsAddedOnce();
  void signTogglesAndStaysLeading();
  void backspaceAndClearEditTheEntry();
  void entryNeverGrowsPastTheCap();
  void unknownKeysLeaveTheEntry();
  void outOfRangeEntryIsRefusedNotClamped();
  void historyIsNewestFirstAndCapped();
  void historySkipsOnlyARepeatOfTheNewest();
  void historyIgnoresBlankLines();
};

/**
 * @brief A point on a whole-number or Text field would only produce an entry the field rejects.
 */
void OutputTextEntryTests::decimalIsOfferedOnlyForFractionalNumberFields()
{
  QVERIFY(keypadOffersDecimal(numberField(-1, 0, 10)));
  QVERIFY(keypadOffersDecimal(numberField(2, 0, 10)));
  QVERIFY(!keypadOffersDecimal(numberField(0, 0, 10)));

  OutputWidget text = numberField(2, 0, 10);
  text.inputKind    = OutputInputKind::Text;
  QVERIFY(!keypadOffersDecimal(text));
}

/**
 * @brief A field that can never go below zero must not show a key that builds a refused entry.
 */
void OutputTextEntryTests::signIsOfferedOnlyWhenMinIsNegative()
{
  QVERIFY(keypadOffersSign(numberField(-1, -5, 5)));
  QVERIFY(!keypadOffersSign(numberField(-1, 0, 5)));
  QVERIFY(!keypadOffersSign(numberField(-1, 3, 5)));

  OutputWidget text = numberField(-1, -5, 5);
  text.inputKind    = OutputInputKind::Text;
  QVERIFY(!keypadOffersSign(text));
}

/**
 * @brief Typing 0 then 5 must read 5, not 05, or the sent value carries a stray leading zero.
 */
void OutputTextEntryTests::digitsReplaceALoneZero()
{
  QCOMPARE(press(QStringLiteral("05")), QStringLiteral("5"));
  QCOMPARE(press(QStringLiteral("105")), QStringLiteral("105"));
  QCOMPARE(press(QStringLiteral("-07")), QStringLiteral("-7"));
}

/**
 * @brief ".5" is not how the field reads a fraction, so a point on empty gives "0.", and a second
 *        point would make the entry unparseable.
 */
void OutputTextEntryTests::pointOnEmptyReadsAsZeroPointAndIsAddedOnce()
{
  QCOMPARE(press(QStringLiteral(".")), QStringLiteral("0."));
  QCOMPARE(press(QStringLiteral("1.2.3")), QStringLiteral("1.23"));
  QCOMPARE(press(QStringLiteral("0..")), QStringLiteral("0."));
}

/**
 * @brief The sign key is a toggle on the leading minus whatever was typed first, never a
 *        character inserted mid-entry.
 */
void OutputTextEntryTests::signTogglesAndStaysLeading()
{
  QCOMPARE(press(QStringLiteral("-")), QStringLiteral("-"));
  QCOMPARE(press(QStringLiteral("--")), QString());
  QCOMPARE(press(QStringLiteral("12-")), QStringLiteral("-12"));
  QCOMPARE(press(QStringLiteral("12--")), QStringLiteral("12"));
  QCOMPARE(press(QStringLiteral("-12")), QStringLiteral("-12"));
}

/**
 * @brief Backspace on an empty entry must not fail, and clear must always reach empty.
 */
void OutputTextEntryTests::backspaceAndClearEditTheEntry()
{
  QCOMPARE(press(QStringLiteral("123<")), QStringLiteral("12"));
  QCOMPARE(press(QStringLiteral("<")), QString());
  QCOMPARE(press(QStringLiteral("1<<<")), QString());
  QCOMPARE(press(QStringLiteral("-1.5C")), QString());
}

/**
 * @brief A digit past the cap is ignored rather than truncated, so a held key cannot grow the
 *        entry without bound.
 */
void OutputTextEntryTests::entryNeverGrowsPastTheCap()
{
  const QString full(kMaxKeypadEntry, QLatin1Char('1'));
  QCOMPARE(keypadApply(full, QLatin1Char('7')), full);
  QCOMPARE(keypadApply(full, kKeypadPoint), full);
  QCOMPARE(keypadApply(full.chopped(1), QLatin1Char('7')).size(), kMaxKeypadEntry);
}

/**
 * @brief A key the keypad does not define must be a no-op, not a character appended.
 */
void OutputTextEntryTests::unknownKeysLeaveTheEntry()
{
  QCOMPARE(keypadApply(QStringLiteral("12"), QLatin1Char('x')), QStringLiteral("12"));
  QCOMPARE(keypadApply(QString(), QLatin1Char('x')), QString());
}

/**
 * @brief A keypad entry outside Min..Max is refused whole: a clamped value would send something
 *        the operator never typed to the device.
 */
void OutputTextEntryTests::outOfRangeEntryIsRefusedNotClamped()
{
  const OutputLimits limits{0, 10, 1, false};
  double out = -12345.0;
  QCOMPARE(admitOutputValue(limits, QVariant(QStringLiteral("11")), out),
           OutputAdmission::OutOfRange);
  QCOMPARE(out, -12345.0);

  QCOMPARE(admitOutputValue(limits, QVariant(QStringLiteral("-1")), out),
           OutputAdmission::OutOfRange);
  QCOMPARE(out, -12345.0);

  QCOMPARE(admitOutputValue(limits, QVariant(QStringLiteral("10")), out),
           OutputAdmission::Accepted);
  QCOMPARE(out, 10.0);
}

/**
 * @brief Up must walk back in time from index 0, and the list must stay bounded.
 */
void OutputTextEntryTests::historyIsNewestFirstAndCapped()
{
  QStringList history;
  for (int i = 0; i < 25; ++i)
    QVERIFY(pushCommandHistory(history, QStringLiteral("cmd%1").arg(i)));

  QCOMPARE(history.size(), kMaxCommandHistory);
  QCOMPARE(history.first(), QStringLiteral("cmd24"));
  QCOMPARE(history.last(), QStringLiteral("cmd5"));
  QVERIFY(!history.contains(QStringLiteral("cmd4")));
  QVERIFY(!history.contains(QStringLiteral("cmd0")));
}

/**
 * @brief Pressing Enter twice on one line should not double the entry, but re-sending an older
 *        command is real history and must be recorded.
 */
void OutputTextEntryTests::historySkipsOnlyARepeatOfTheNewest()
{
  QStringList history;
  QVERIFY(pushCommandHistory(history, QStringLiteral("a")));
  QVERIFY(pushCommandHistory(history, QStringLiteral("b")));
  QVERIFY(!pushCommandHistory(history, QStringLiteral("b")));
  QCOMPARE(history, (QStringList{QStringLiteral("b"), QStringLiteral("a")}));

  QVERIFY(pushCommandHistory(history, QStringLiteral("a")));
  QCOMPARE(history, (QStringList{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("a")}));
}

/**
 * @brief An empty or whitespace-only line sends nothing and so has nothing to recall.
 */
void OutputTextEntryTests::historyIgnoresBlankLines()
{
  QStringList history;
  QVERIFY(!pushCommandHistory(history, QString()));
  QVERIFY(!pushCommandHistory(history, QStringLiteral("   \t")));
  QVERIFY(history.isEmpty());
}

QTEST_APPLESS_MAIN(OutputTextEntryTests)

#include "tst_output_text_entry.moc"
