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

#include <limits>
#include <QString>
#include <QTest>

#include "UI/Widgets/Waterfall/WaterfallTicks.h"

// Every test function here is self-contained: the tick helpers are pure functions, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Known-answer sweep of the waterfall axis tick ladder and its label formatters.
 */
class TstWaterfallTicks : public QObject {
  Q_OBJECT

private slots:
  void freqTicksLadder_data();
  void freqTicksLadder();

  void freqTicksRejectDegenerateRanges_data();
  void freqTicksRejectDegenerateRanges();

  void timeTicksMatchFreqTicks();

  void formatFreq_data();
  void formatFreq();

  void formatTime_data();
  void formatTime();
};

//--------------------------------------------------------------------------------------------------
// Tick ladder
//--------------------------------------------------------------------------------------------------

void TstWaterfallTicks::freqTicksLadder_data()
{
  QTest::addColumn<double>("maxFreq");
  QTest::addColumn<int>("targetCount");
  QTest::addColumn<double>("step");
  QTest::addColumn<double>("displayMax");
  QTest::addColumn<int>("count");

  QTest::newRow("1 kHz over 6 ticks") << 1000.0 << 6 << 200.0 << 1000.0 << 6;
  QTest::newRow("60 units over 6 ticks") << 60.0 << 6 << 10.0 << 60.0 << 7;
  QTest::newRow("target below two clamps to two") << 1000.0 << 1 << 500.0 << 1000.0 << 3;
  QTest::newRow("range extends to the next step") << 950.0 << 6 << 200.0 << 1000.0 << 6;
  QTest::newRow("sub-unit range") << 0.6 << 6 << 0.1 << 0.6 << 7;
}

/**
 * @brief The step is the smallest {1,2,5}*10^n that fits the requested tick count, and the ladder
 *        always starts at zero and covers the range.
 */
void TstWaterfallTicks::freqTicksLadder()
{
  QFETCH(double, maxFreq);
  QFETCH(int, targetCount);
  QFETCH(double, step);
  QFETCH(double, displayMax);
  QFETCH(int, count);

  const auto ticks = Widgets::WaterfallTicks::computeFreqTicks(maxFreq, targetCount);

  QVERIFY(qFuzzyCompare(ticks.step, step));
  QVERIFY(qFuzzyCompare(ticks.displayMax, displayMax));
  QCOMPARE(static_cast<int>(ticks.values.size()), count);
  QVERIFY(qFuzzyIsNull(ticks.values.front()));
  QVERIFY(qFuzzyCompare(ticks.values.back(), displayMax));
}

void TstWaterfallTicks::freqTicksRejectDegenerateRanges_data()
{
  QTest::addColumn<double>("maxFreq");

  QTest::newRow("zero range") << 0.0;
  QTest::newRow("negative range") << -100.0;
  QTest::newRow("infinite range") << std::numeric_limits<double>::infinity();
  QTest::newRow("not a number") << std::numeric_limits<double>::quiet_NaN();
}

/**
 * @brief A degenerate range yields no ticks and a unit step, so the axis loop draws nothing
 *        instead of iterating forever on a zero or NaN step.
 */
void TstWaterfallTicks::freqTicksRejectDegenerateRanges()
{
  QFETCH(double, maxFreq);

  const auto ticks = Widgets::WaterfallTicks::computeFreqTicks(maxFreq, 6);

  QVERIFY(ticks.values.empty());
  QVERIFY(qFuzzyCompare(ticks.step, 1.0));
}

/**
 * @brief The seconds axis runs the frequency ladder verbatim; this pins them together so a change
 *        to one cannot silently diverge the other.
 */
void TstWaterfallTicks::timeTicksMatchFreqTicks()
{
  const auto freq = Widgets::WaterfallTicks::computeFreqTicks(60.0, 6);
  const auto time = Widgets::WaterfallTicks::computeTimeTicks(60.0, 6);

  QVERIFY(qFuzzyCompare(time.step, freq.step));
  QVERIFY(qFuzzyCompare(time.displayMax, freq.displayMax));
  QCOMPARE(time.values.size(), freq.values.size());
}

//--------------------------------------------------------------------------------------------------
// Label formatting
//--------------------------------------------------------------------------------------------------

void TstWaterfallTicks::formatFreq_data()
{
  QTest::addColumn<double>("hz");
  QTest::addColumn<QString>("expected");

  QTest::newRow("zero") << 0.0 << QStringLiteral("0 Hz");
  QTest::newRow("hertz") << 500.0 << QStringLiteral("500 Hz");
  QTest::newRow("just below a kilohertz") << 999.0 << QStringLiteral("999 Hz");
  QTest::newRow("exactly a kilohertz") << 1000.0 << QStringLiteral("1 kHz");
  QTest::newRow("fractional kilohertz") << 1500.0 << QStringLiteral("1.5 kHz");
  QTest::newRow("megahertz") << 2500000.0 << QStringLiteral("2.5 MHz");
  QTest::newRow("negative uses the magnitude band") << -1500.0 << QStringLiteral("-1.5 kHz");
}

/**
 * @brief Frequency labels switch unit at each thousand and carry three significant digits.
 */
void TstWaterfallTicks::formatFreq()
{
  QFETCH(double, hz);
  QFETCH(QString, expected);

  QCOMPARE(Widgets::WaterfallTicks::formatFreqTick(hz), expected);
}

void TstWaterfallTicks::formatTime_data()
{
  QTest::addColumn<double>("seconds");
  QTest::addColumn<double>("step");
  QTest::addColumn<QString>("expected");

  QTest::newRow("whole seconds round down") << 2.4 << 1.0 << QStringLiteral("2");
  QTest::newRow("whole seconds round up") << 2.6 << 1.0 << QStringLiteral("3");
  QTest::newRow("coarse step stays integral") << 10.0 << 2.0 << QStringLiteral("10");
  QTest::newRow("half-second step keeps one decimal") << 1.5 << 0.5 << QStringLiteral("1.5");
  QTest::newRow("fine step keeps two decimals") << 0.25 << 0.05 << QStringLiteral("0.25");
}

/**
 * @brief Time labels drop the decimals once the step is a second or more, and otherwise carry
 *        exactly as many decimals as the step needs.
 */
void TstWaterfallTicks::formatTime()
{
  QFETCH(double, seconds);
  QFETCH(double, step);
  QFETCH(QString, expected);

  QCOMPARE(Widgets::WaterfallTicks::formatTimeTick(seconds, step), expected);
}

QTEST_APPLESS_MAIN(TstWaterfallTicks)

#include "tst_waterfall_ticks.moc"
