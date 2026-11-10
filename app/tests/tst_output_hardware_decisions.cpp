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
#include <QTest>

#include "Core/DataModel/OutputWidget.h"

using namespace DataModel;

/**
 * @brief The decisions behind the selector and the emergency stop (spec 0113 AC10 to AC12), as
 *        the Core functions the dashboard controls call. The controls themselves need most of the
 *        Ui library and a valid licence to transmit, so the rules are pinned here instead.
 */
class OutputHardwareDecisionsTests : public QObject {
  Q_OBJECT

private slots:
  void detentsFollowStepFromMinToMax();
  void eachPositionsValueSelectsIt();
  void aLabelSelectsItsPosition();
  void aReportBetweenMarksIsUnknown();
  void springReturnSendsRestOnceFromEitherSide();
  void withoutSpringReturnReleaseSendsNothing();
  void anEmergencyStopStartsReleased();
  void onlyAStopPressSkipsPacing();
};

/**
 * @brief Three positions are Min 0, Max 2, Step 1, and a degenerate range still draws two marks;
 *        the count is the QML's own rule, so Core and the drawing agree on what a position is.
 */
void OutputHardwareDecisionsTests::detentsFollowStepFromMinToMax()
{
  QCOMPARE(selectorPositionCount({0, 2, 1, false}), 3);
  QCOMPARE(selectorPositionCount({-1, 1, 1, false}), 3);
  QCOMPARE(selectorPositionCount({0, 0, 1, false}), 2);
  QCOMPARE(selectorPositionCount({0, 100, 1, false}), kMaxSelectorPositions);
}

/**
 * @brief AC11: a bound selector fed each position's value shows that position, by number or by
 *        the number written as text.
 */
void OutputHardwareDecisionsTests::eachPositionsValueSelectsIt()
{
  const OutputLimits limits{-1, 1, 1, false};
  QCOMPARE(selectorPosition(limits, QString(), -1, QString()), 0);
  QCOMPARE(selectorPosition(limits, QString(), 0, QString()), 1);
  QCOMPARE(selectorPosition(limits, QString(), 1, QStringLiteral("1")), 2);
}

/**
 * @brief R23: a device that reports a mode by name selects the position with that label, in any
 *        letter case, even though the text parses to no number.
 */
void OutputHardwareDecisionsTests::aLabelSelectsItsPosition()
{
  const OutputLimits limits{0, 2, 1, false};
  const auto labels = QStringLiteral("Off, Auto , Hand");
  QCOMPARE(selectorPosition(limits, labels, 0, QStringLiteral("auto")), 1);
  QCOMPARE(selectorPosition(limits, labels, 0, QStringLiteral("HAND")), 2);
}

/**
 * @brief AC11: a value off the marks, outside the range, or a word that names no label is unknown,
 *        never the nearest position and never the zero a failed parse produces.
 */
void OutputHardwareDecisionsTests::aReportBetweenMarksIsUnknown()
{
  const OutputLimits limits{0, 2, 1, false};
  const auto labels = QStringLiteral("Off,Auto,Hand");
  QCOMPARE(selectorPosition(limits, labels, 0.5, QString()), -1);
  QCOMPARE(selectorPosition(limits, labels, 3, QString()), -1);
  QCOMPARE(selectorPosition(limits, labels, -1, QString()), -1);
  QCOMPARE(selectorPosition(limits, labels, 0, QStringLiteral("Remote")), -1);
}

/**
 * @brief AC10 with R22: let go from either side, the selector sends its rest value exactly once;
 *        let go at rest, it sends nothing. Initial Value is the rest position.
 */
void OutputHardwareDecisionsTests::springReturnSendsRestOnceFromEitherSide()
{
  const OutputLimits limits{-1, 1, 1, false};
  double out = 99;
  QVERIFY(selectorReleaseValue(limits, true, 0, -1, out));
  QCOMPARE(out, 0.0);

  out = 99;
  QVERIFY(selectorReleaseValue(limits, true, 0, 1, out));
  QCOMPARE(out, 0.0);

  out = 99;
  QVERIFY(!selectorReleaseValue(limits, true, 0, 0, out));
  QCOMPARE(out, 99.0);

  QVERIFY(selectorReleaseValue(limits, true, 0.4, 1, out));
  QCOMPARE(out, 0.0);
}

/**
 * @brief A maintained selector keeps whatever position it was turned to.
 */
void OutputHardwareDecisionsTests::withoutSpringReturnReleaseSendsNothing()
{
  const OutputLimits limits{-1, 1, 1, false};
  double out = 99;
  QVERIFY(!selectorReleaseValue(limits, false, 0, 1, out));
  QCOMPARE(out, 99.0);
}

/**
 * @brief AC12 and R26: an emergency stop opens released even with Initial Value 1, while an
 *        ordinary latching button still opens at its Initial Value and a momentary one holds none.
 */
void OutputHardwareDecisionsTests::anEmergencyStopStartsReleased()
{
  OutputWidget w;
  w.type         = OutputWidgetType::Button;
  w.checkable    = true;
  w.initialValue = 1;
  QVERIFY(outputStartsLatched(w));

  w.style = OutputWidgetStyle::EmergencyStop;
  QVERIFY(!outputStartsLatched(w));

  w.style     = OutputWidgetStyle::Standard;
  w.checkable = false;
  QVERIFY(!outputStartsLatched(w));
}

/**
 * @brief AC12: a stop press is delivered at once, never held behind the pacing window; its
 *        release and every other control's latch change stay paced.
 */
void OutputHardwareDecisionsTests::onlyAStopPressSkipsPacing()
{
  QVERIFY(outputSendsUnpaced(OutputWidgetStyle::EmergencyStop, true));
  QVERIFY(!outputSendsUnpaced(OutputWidgetStyle::EmergencyStop, false));
  QVERIFY(!outputSendsUnpaced(OutputWidgetStyle::Standard, true));
  QVERIFY(!outputSendsUnpaced(OutputWidgetStyle::Illuminated, true));
}

QTEST_APPLESS_MAIN(OutputHardwareDecisionsTests)

#include "tst_output_hardware_decisions.moc"
