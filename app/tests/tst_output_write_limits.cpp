/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
#include <QTest>
#include <QVariant>

#include "Core/DataModel/Frame.h"

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

using namespace DataModel;

/**
 * @brief A Number text field bounded to 0..100 in steps of 5.
 */
static OutputWidget numberField(const bool quantize = false)
{
  OutputWidget w;
  w.title     = QStringLiteral("Setpoint");
  w.type      = OutputWidgetType::TextField;
  w.inputKind = OutputInputKind::Number;
  w.minValue  = 0;
  w.maxValue  = 100;
  w.stepSize  = 5;
  w.quantize  = quantize;
  return w;
}

/**
 * @brief The admission rule every numeric output control passes before its transmit script (spec
 *        0109): the operator who types 250 into a 0 to 100 setpoint must not reach the device.
 */
class OutputWriteLimitsTests : public QObject {
  Q_OBJECT

private slots:
  void inRangeIsAccepted();
  void outsideTheRangeIsOutOfRange();
  void textThatIsNotANumberIsRefused();
  void quantizeCountsStepsFromMin();
  void quantizeOffLeavesTheValue();
  void quantizeRunsBeforeTheRangeCheck();
  void aNegativeMinIsTheQuantizeOrigin();
  void outputIsNumericPerTypeAndKind();
};

//--------------------------------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------------------------------

/**
 * @brief Values inside Min to Max, bounds included, pass as the double that was typed.
 */
void OutputWriteLimitsTests::inRangeIsAccepted()
{
  const auto w = numberField();
  double out   = -1;

  QCOMPARE(admitOutputValue(w, QVariant(QStringLiteral("43")), out), OutputAdmission::Accepted);
  QCOMPARE(out, 43.0);

  QCOMPARE(admitOutputValue(w, QVariant(0.0), out), OutputAdmission::Accepted);
  QCOMPARE(out, 0.0);

  QCOMPARE(admitOutputValue(w, QVariant(100), out), OutputAdmission::Accepted);
  QCOMPARE(out, 100.0);
}

/**
 * @brief One step past either bound is out of range, as a string or as a number.
 */
void OutputWriteLimitsTests::outsideTheRangeIsOutOfRange()
{
  const auto w = numberField();
  double out   = 0;

  QCOMPARE(admitOutputValue(w, QVariant(QStringLiteral("250")), out), OutputAdmission::OutOfRange);
  QCOMPARE(admitOutputValue(w, QVariant(100.5), out), OutputAdmission::OutOfRange);
  QCOMPARE(admitOutputValue(w, QVariant(-0.5), out), OutputAdmission::OutOfRange);
  QCOMPARE(admitOutputValue(w, QVariant(-1), out), OutputAdmission::OutOfRange);
}

/**
 * @brief Words and the non-finite spellings the project parser accepts are not numbers.
 */
void OutputWriteLimitsTests::textThatIsNotANumberIsRefused()
{
  const auto w = numberField();
  double out   = 0;

  QCOMPARE(admitOutputValue(w, QVariant(QStringLiteral("abc")), out), OutputAdmission::NotANumber);
  QCOMPARE(admitOutputValue(w, QVariant(QStringLiteral("inf")), out), OutputAdmission::NotANumber);
  QCOMPARE(admitOutputValue(w, QVariant(QStringLiteral("nan")), out), OutputAdmission::NotANumber);
  QCOMPARE(admitOutputValue(w, QVariant(QStringLiteral("")), out), OutputAdmission::NotANumber);
}

/**
 * @brief With Min 0 and Step 5, 43 snaps up to 45 and 42 snaps down to 40.
 */
void OutputWriteLimitsTests::quantizeCountsStepsFromMin()
{
  const auto w = numberField(true);
  double out   = 0;

  QCOMPARE(admitOutputValue(w, QVariant(QStringLiteral("43")), out), OutputAdmission::Accepted);
  QCOMPARE(out, 45.0);

  QCOMPARE(admitOutputValue(w, QVariant(QStringLiteral("42")), out), OutputAdmission::Accepted);
  QCOMPARE(out, 40.0);
}

/**
 * @brief Quantize is opt-in: off, 43 stays 43.
 */
void OutputWriteLimitsTests::quantizeOffLeavesTheValue()
{
  const auto w = numberField(false);
  double out   = 0;

  QCOMPARE(admitOutputValue(w, QVariant(QStringLiteral("43")), out), OutputAdmission::Accepted);
  QCOMPARE(out, 43.0);
}

/**
 * @brief 103 snaps to 105 and 105 is past Max, so the quantized value is what gets refused.
 */
void OutputWriteLimitsTests::quantizeRunsBeforeTheRangeCheck()
{
  const auto w = numberField(true);
  double out   = 0;

  QCOMPARE(admitOutputValue(w, QVariant(QStringLiteral("103")), out), OutputAdmission::OutOfRange);
}

/**
 * @brief Steps are counted from Min, not from zero: Min -3 and Step 5 allow -3, 2, 7.
 */
void OutputWriteLimitsTests::aNegativeMinIsTheQuantizeOrigin()
{
  auto w     = numberField(true);
  w.minValue = -3;
  double out = 0;

  QCOMPARE(admitOutputValue(w, QVariant(QStringLiteral("3")), out), OutputAdmission::Accepted);
  QCOMPARE(out, 2.0);

  QCOMPARE(admitOutputValue(w, QVariant(QStringLiteral("5")), out), OutputAdmission::Accepted);
  QCOMPARE(out, 7.0);
}

/**
 * @brief Slider, knob and a Number text field are numeric; a Text field, button and toggle are not.
 */
void OutputWriteLimitsTests::outputIsNumericPerTypeAndKind()
{
  OutputWidget w;

  w.type = OutputWidgetType::Slider;
  QVERIFY(outputIsNumeric(w));

  w.type = OutputWidgetType::Knob;
  QVERIFY(outputIsNumeric(w));

  w.type      = OutputWidgetType::TextField;
  w.inputKind = OutputInputKind::Text;
  QVERIFY(!outputIsNumeric(w));

  w.inputKind = OutputInputKind::Number;
  QVERIFY(outputIsNumeric(w));

  w.type = OutputWidgetType::Button;
  QVERIFY(!outputIsNumeric(w));

  w.type = OutputWidgetType::Toggle;
  QVERIFY(!outputIsNumeric(w));
}

QTEST_APPLESS_MAIN(OutputWriteLimitsTests)

#include "tst_output_write_limits.moc"
