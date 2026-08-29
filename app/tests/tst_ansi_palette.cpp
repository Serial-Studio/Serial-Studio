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

#include <QColor>
#include <QList>
#include <QTest>

#include "UI/Widgets/Terminal/AnsiPalette.h"

// Every test function here is self-contained: each builds its own palette, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Known-answer coverage of the terminal's ANSI color unit: the xterm-256 index table,
 *        the theme-selected base palettes, and the SGR codes that pick from them.
 */
class TstAnsiPalette : public QObject {
  Q_OBJECT

private slots:
  void indexedColor_data();
  void indexedColor();

  void themeSelection();

  void sgrBasicCodes_data();
  void sgrBasicCodes();

  void sgrExtendedColors_data();
  void sgrExtendedColors();

  void sgrBoldLightensDefault();
  void resetForegroundKeepsBackground();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a console base/text pair whose lightness ordering selects the dark table.
 */
static QColor darkThemeBase()
{
  return QColor(20, 20, 20);
}

/**
 * @brief Returns the console text color that pairs with darkThemeBase().
 */
static QColor darkThemeText()
{
  return QColor(230, 230, 230);
}

/**
 * @brief Builds a palette already rebuilt against the dark theme table.
 */
static Widgets::AnsiPalette darkPalette()
{
  Widgets::AnsiPalette palette;
  palette.rebuild(darkThemeBase(), darkThemeText());
  return palette;
}

//--------------------------------------------------------------------------------------------------
// indexedColor
//--------------------------------------------------------------------------------------------------

void TstAnsiPalette::indexedColor_data()
{
  QTest::addColumn<int>("index");
  QTest::addColumn<QColor>("expected");

  QTest::newRow("standard 0 black") << 0 << QColor(0, 0, 0);
  QTest::newRow("standard 3 brown") << 3 << QColor(170, 85, 0);
  QTest::newRow("standard 7 silver") << 7 << QColor(170, 170, 170);
  QTest::newRow("bright 8 gray") << 8 << QColor(85, 85, 85);
  QTest::newRow("bright 15 white") << 15 << QColor(255, 255, 255);

  QTest::newRow("cube first corner") << 16 << QColor(0, 0, 0);
  QTest::newRow("cube last corner") << 231 << QColor(255, 255, 255);
  QTest::newRow("cube first blue step") << 17 << QColor(0, 0, 95);
  QTest::newRow("cube pure red") << 196 << QColor(255, 0, 0);
  QTest::newRow("cube pure green") << 46 << QColor(0, 255, 0);
  QTest::newRow("cube pure blue") << 21 << QColor(0, 0, 255);

  QTest::newRow("gray ramp start") << 232 << QColor(8, 8, 8);
  QTest::newRow("gray ramp end") << 255 << QColor(238, 238, 238);

  QTest::newRow("above range clamps to 255") << 300 << QColor(238, 238, 238);
  QTest::newRow("below range clamps to 0") << -5 << QColor(0, 0, 0);
}

/**
 * @brief The 256-color table is a pure function of the index; this pins all four of its
 *        regions plus the clamp that keeps device bytes from walking off either end.
 */
void TstAnsiPalette::indexedColor()
{
  QFETCH(int, index);
  QFETCH(QColor, expected);

  QCOMPARE(Widgets::AnsiPalette::indexedColor(index), expected);
}

//--------------------------------------------------------------------------------------------------
// Theme selection
//--------------------------------------------------------------------------------------------------

/**
 * @brief rebuild() picks the dark table when the console text is lighter than the console
 *        background and the light table otherwise; the two tables must not collide.
 */
void TstAnsiPalette::themeSelection()
{
  Widgets::AnsiPalette dark;
  dark.rebuild(darkThemeBase(), darkThemeText());
  QCOMPARE(dark.standardColor(1), QColor(205, 49, 49));
  QCOMPARE(dark.brightColor(7), QColor(255, 255, 255));

  Widgets::AnsiPalette light;
  light.rebuild(darkThemeText(), darkThemeBase());
  QCOMPARE(light.standardColor(1), QColor(170, 0, 0));
  QCOMPARE(light.brightColor(7), QColor(85, 85, 85));

  QCOMPARE(dark.standardColor(-1), dark.standardColor(0));
  QCOMPARE(dark.brightColor(99), dark.brightColor(7));
}

//--------------------------------------------------------------------------------------------------
// Basic SGR codes
//--------------------------------------------------------------------------------------------------

void TstAnsiPalette::sgrBasicCodes_data()
{
  QTest::addColumn<QList<int>>("codes");
  QTest::addColumn<QColor>("foreground");
  QTest::addColumn<QColor>("background");

  QTest::newRow("30 standard black fg") << QList<int>{30} << QColor(0, 0, 0) << QColor();
  QTest::newRow("31 standard red fg") << QList<int>{31} << QColor(205, 49, 49) << QColor();
  QTest::newRow("37 standard white fg") << QList<int>{37} << QColor(229, 229, 229) << QColor();
  QTest::newRow("40 standard black bg") << QList<int>{40} << QColor() << QColor(0, 0, 0);
  QTest::newRow("44 standard blue bg") << QList<int>{44} << QColor() << QColor(36, 114, 200);
  QTest::newRow("90 bright gray fg") << QList<int>{90} << QColor(102, 102, 102) << QColor();
  QTest::newRow("97 bright white fg") << QList<int>{97} << QColor(255, 255, 255) << QColor();
  QTest::newRow("107 bright white bg") << QList<int>{107} << QColor() << QColor(255, 255, 255);

  QTest::newRow("fg then bg in one run")
    << QList<int>{31, 44} << QColor(205, 49, 49) << QColor(36, 114, 200);
  QTest::newRow("later fg wins") << QList<int>{31, 32} << QColor(13, 188, 121) << QColor();
  QTest::newRow("reset clears both") << QList<int>{31, 44, 0} << QColor() << QColor();
  QTest::newRow("unknown code is ignored") << QList<int>{53} << QColor() << QColor();
  QTest::newRow("empty run changes nothing") << QList<int>{} << QColor() << QColor();
}

/**
 * @brief Sweeps the SGR codes that select from the two base tables, including the ordering
 *        rule that a later code in the same run overrides an earlier one.
 */
void TstAnsiPalette::sgrBasicCodes()
{
  QFETCH(QList<int>, codes);
  QFETCH(QColor, foreground);
  QFETCH(QColor, background);

  Widgets::AnsiPalette palette = darkPalette();
  palette.applySgr(codes, QColor(128, 128, 128));

  QCOMPARE(palette.foreground(), foreground);
  QCOMPARE(palette.background(), background);
}

//--------------------------------------------------------------------------------------------------
// Extended (38/48) color forms
//--------------------------------------------------------------------------------------------------

void TstAnsiPalette::sgrExtendedColors_data()
{
  QTest::addColumn<QList<int>>("codes");
  QTest::addColumn<QColor>("foreground");
  QTest::addColumn<QColor>("background");

  QTest::newRow("38;5 indexed fg") << QList<int>{38, 5, 196} << QColor(255, 0, 0) << QColor();
  QTest::newRow("48;5 indexed bg") << QList<int>{48, 5, 21} << QColor() << QColor(0, 0, 255);
  QTest::newRow("38;2 truecolor fg")
    << QList<int>{38, 2, 10, 20, 30} << QColor(10, 20, 30) << QColor();
  QTest::newRow("48;2 truecolor bg") << QList<int>{48, 2, 1, 2, 3} << QColor() << QColor(1, 2, 3);

  QTest::newRow("indexed run consumes two params")
    << QList<int>{38, 5, 196, 44} << QColor(255, 0, 0) << QColor(36, 114, 200);
  QTest::newRow("truecolor run consumes four params")
    << QList<int>{38, 2, 10, 20, 30, 44} << QColor(10, 20, 30) << QColor(36, 114, 200);

  QTest::newRow("truncated indexed form is inert") << QList<int>{38, 5} << QColor() << QColor();
  QTest::newRow("truncated truecolor form is inert")
    << QList<int>{38, 2, 10, 20} << QColor() << QColor();
  QTest::newRow("unknown selector is inert") << QList<int>{38, 9, 53} << QColor() << QColor();

  QTest::newRow("out of range index clamps")
    << QList<int>{38, 5, 999} << QColor(238, 238, 238) << QColor();
  QTest::newRow("out of range channel clamps")
    << QList<int>{38, 2, 999, -4, 30} << QColor(255, 0, 30) << QColor();
}

/**
 * @brief The 38/48 forms are the only SGR codes that consume following parameters; getting
 *        that count wrong silently repaints the rest of the run, so the trailing 44 in two
 *        of these rows is the real assertion.
 */
void TstAnsiPalette::sgrExtendedColors()
{
  QFETCH(QList<int>, codes);
  QFETCH(QColor, foreground);
  QFETCH(QColor, background);

  Widgets::AnsiPalette palette = darkPalette();
  palette.applySgr(codes, QColor(128, 128, 128));

  QCOMPARE(palette.foreground(), foreground);
  QCOMPARE(palette.background(), background);
}

//--------------------------------------------------------------------------------------------------
// Bold and reset behavior
//--------------------------------------------------------------------------------------------------

/**
 * @brief SGR 1 lightens whatever foreground is in effect, falling back to the caller's
 *        default when no color has been selected yet.
 */
void TstAnsiPalette::sgrBoldLightensDefault()
{
  const QColor defaultForeground(100, 100, 100);

  Widgets::AnsiPalette fromDefault = darkPalette();
  fromDefault.applySgr(QList<int>{1}, defaultForeground);
  QVERIFY(fromDefault.foreground().isValid());
  QVERIFY(fromDefault.foreground().lightness() > defaultForeground.lightness());

  Widgets::AnsiPalette fromSelected = darkPalette();
  fromSelected.applySgr(QList<int>{31, 1}, defaultForeground);
  QVERIFY(fromSelected.foreground().lightness() > QColor(205, 49, 49).lightness());
}

/**
 * @brief A theme change or buffer reset drops only the foreground; an active background must
 *        survive, because neither event is an SGR reset.
 */
void TstAnsiPalette::resetForegroundKeepsBackground()
{
  Widgets::AnsiPalette palette = darkPalette();
  palette.applySgr(QList<int>{31, 44}, QColor(128, 128, 128));

  palette.resetForeground();
  QVERIFY(!palette.foreground().isValid());
  QCOMPARE(palette.background(), QColor(36, 114, 200));

  palette.resetColors();
  QVERIFY(!palette.background().isValid());
}

QTEST_APPLESS_MAIN(TstAnsiPalette)

#include "tst_ansi_palette.moc"
