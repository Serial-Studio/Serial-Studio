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

#include "DataModel/Project/PropertyHooks.h"
#include "SerialStudio.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Boundary sweep of the four ProjectModel-free validators in PropertyValidators.cpp.
 */
class TstPropertyValidators : public QObject {
  Q_OBJECT

private slots:
  void isValidColor_data();
  void isValidColor();

  void isValidDatasetIndex_data();
  void isValidDatasetIndex();

  void isValidFftWindow_data();
  void isValidFftWindow();

  void isValidTransformLanguage_data();
  void isValidTransformLanguage();
};

//--------------------------------------------------------------------------------------------------
// isValidColor
//--------------------------------------------------------------------------------------------------

void TstPropertyValidators::isValidColor_data()
{
  QTest::addColumn<QString>("color");
  QTest::addColumn<bool>("expected");

  QTest::newRow("empty is automatic") << QString() << true;
  QTest::newRow("empty literal is automatic") << QStringLiteral("") << true;
  QTest::newRow("hex RRGGBB") << QStringLiteral("#ff0000") << true;
  QTest::newRow("hex RGB shorthand") << QStringLiteral("#f00") << true;
  QTest::newRow("hex AARRGGBB") << QStringLiteral("#ff0000ff") << true;
  QTest::newRow("named color") << QStringLiteral("red") << true;
  QTest::newRow("named color mixed case") << QStringLiteral("Red") << true;
  QTest::newRow("not a color name") << QStringLiteral("not-a-color") << false;
  QTest::newRow("hex non-digit characters") << QStringLiteral("#zzzzzz") << false;
  QTest::newRow("missing hash prefix") << QStringLiteral("ff0000") << false;
  QTest::newRow("whitespace only") << QStringLiteral("   ") << false;
}

/**
 * @brief isValidColor() special-cases the empty string, then defers everything else to
 *        QColor::fromString(); this pins the boundary between the two.
 */
void TstPropertyValidators::isValidColor()
{
  QFETCH(QString, color);
  QFETCH(bool, expected);

  QCOMPARE(DataModel::PropertyHooks::isValidColor(color), expected);
}

//--------------------------------------------------------------------------------------------------
// isValidDatasetIndex
//--------------------------------------------------------------------------------------------------

void TstPropertyValidators::isValidDatasetIndex_data()
{
  QTest::addColumn<int>("index");
  QTest::addColumn<bool>("expected");

  QTest::newRow("zero is unassigned") << 0 << true;
  QTest::newRow("one is the first parser slot") << 1 << true;
  QTest::newRow("large positive slot") << 4096 << true;
  QTest::newRow("int max") << std::numeric_limits<int>::max() << true;
  QTest::newRow("negative one") << -1 << false;
  QTest::newRow("large negative") << -4096 << false;
  QTest::newRow("int min") << std::numeric_limits<int>::min() << false;
}

/**
 * @brief The only rule is index >= 0: 0 means unassigned, everything positive is a parser slot.
 */
void TstPropertyValidators::isValidDatasetIndex()
{
  QFETCH(int, index);
  QFETCH(bool, expected);

  QCOMPARE(DataModel::PropertyHooks::isValidDatasetIndex(index), expected);
}

//--------------------------------------------------------------------------------------------------
// isValidFftWindow
//--------------------------------------------------------------------------------------------------

void TstPropertyValidators::isValidFftWindow_data()
{
  QTest::addColumn<int>("window");
  QTest::addColumn<bool>("expected");

  QTest::newRow("Rectangular") << int(SerialStudio::FFTWindowRectangular) << true;
  QTest::newRow("Bartlett") << int(SerialStudio::FFTWindowBartlett) << true;
  QTest::newRow("Hann") << int(SerialStudio::FFTWindowHann) << true;
  QTest::newRow("Hamming") << int(SerialStudio::FFTWindowHamming) << true;
  QTest::newRow("Blackman") << int(SerialStudio::FFTWindowBlackman) << true;
  QTest::newRow("BlackmanHarris") << int(SerialStudio::FFTWindowBlackmanHarris) << true;
  QTest::newRow("Nuttall") << int(SerialStudio::FFTWindowNuttall) << true;
  QTest::newRow("BlackmanNuttall") << int(SerialStudio::FFTWindowBlackmanNuttall) << true;
  QTest::newRow("FlatTop") << int(SerialStudio::FFTWindowFlatTop) << true;
  QTest::newRow("Welch") << int(SerialStudio::FFTWindowWelch) << true;
  QTest::newRow("BartlettHann") << int(SerialStudio::FFTWindowBartlettHann) << true;
  QTest::newRow("Bohman") << int(SerialStudio::FFTWindowBohman) << true;
  QTest::newRow("Cosine") << int(SerialStudio::FFTWindowCosine) << true;
  QTest::newRow("Lanczos") << int(SerialStudio::FFTWindowLanczos) << true;
  QTest::newRow("Parzen") << int(SerialStudio::FFTWindowParzen) << true;
  QTest::newRow("one below range") << int(SerialStudio::FFTWindowRectangular) - 1 << false;
  QTest::newRow("one above range") << int(SerialStudio::FFTWindowParzen) + 1 << false;
  QTest::newRow("negative one") << -1 << false;
  QTest::newRow("far above range") << 999 << false;
}

/**
 * @brief The enum is persisted as a plain integer and is append-only, so the valid range is exactly
 *        [FFTWindowRectangular, FFTWindowParzen] with no gaps.
 */
void TstPropertyValidators::isValidFftWindow()
{
  QFETCH(int, window);
  QFETCH(bool, expected);

  QCOMPARE(DataModel::PropertyHooks::isValidFftWindow(window), expected);
}

//--------------------------------------------------------------------------------------------------
// isValidTransformLanguage
//--------------------------------------------------------------------------------------------------

void TstPropertyValidators::isValidTransformLanguage_data()
{
  QTest::addColumn<int>("language");
  QTest::addColumn<bool>("expected");

  QTest::newRow("inherit") << -1 << true;
  QTest::newRow("JavaScript") << int(SerialStudio::JavaScript) << true;
  QTest::newRow("Lua") << int(SerialStudio::Lua) << true;
  QTest::newRow("Native is not a transform language") << int(SerialStudio::Native) << false;
  QTest::newRow("two below inherit") << -2 << false;
  QTest::newRow("far above range") << 99 << false;
}

/**
 * @brief Per-dataset transforms run in JavaScript or Lua only; -1 means "inherit the frame parser
 *        language" and Native (the frame-parser-only third language) is deliberately excluded.
 */
void TstPropertyValidators::isValidTransformLanguage()
{
  QFETCH(int, language);
  QFETCH(bool, expected);

  QCOMPARE(DataModel::PropertyHooks::isValidTransformLanguage(language), expected);
}

QTEST_APPLESS_MAIN(TstPropertyValidators)

#include "tst_property_validators.moc"
