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

#include <iterator>
#include <limits>
#include <QString>
#include <QTest>

#include "Core/DataModel/Frame.h"
#include "Core/DataModel/ValueDecoder.h"
#include "Core/DataModel/ValueFormat.h"

// Spec 0108: the one presentation formatter. Every expected string is a literal, so a formatter
// that drifts from the tick labels or the Data Grid of today fails here.

using namespace DataModel;

/**
 * @brief Builds a dataset with @p displayFormat and no decoder, range or units.
 */
static Dataset withFormat(const QString& displayFormat)
{
  Dataset d;
  d.displayFormat = displayFormat;
  return d;
}

/**
 * @brief Builds a numeric dataset currently holding @p value, as the dashboard receives it.
 */
static Dataset holding(double value, const QString& text)
{
  Dataset d;
  d.isNumeric     = true;
  d.numericValue  = value;
  d.value         = text;
  d.displayFormat = QStringLiteral("0d");
  return d;
}

/**
 * @brief Builds a dataset labelling the four gear positions 0 to 3.
 */
static Dataset gearbox()
{
  Dataset d;
  d.labels = {
    {0,    QStringLiteral("Park")},
    {1, QStringLiteral("Reverse")},
    {2, QStringLiteral("Neutral")},
    {3,   QStringLiteral("Drive")},
  };
  return d;
}

/**
 * @brief Contract of DataModel::ValueFormat: notation, decimals, labels and units.
 */
class TstValueFormat : public QObject {
  Q_OBJECT

private slots:
  void formatNumberFollowsTheLabelFormat_data();
  void formatNumberFollowsTheLabelFormat();
  void notationFollowsTheDeclaredWidth_data();
  void notationFollowsTheDeclaredWidth();
  void decimalPointsWinOverTheLabelFormat();
  void printfFormatsKeepWorking();
  void autoNumberFollowsTheRange();

  void labelForMatchesTheRoundedKey();
  void labelForFallsBackToTheCatchAll();
  void labelForIgnoresANonFiniteValue();

  void formatValueShowsTheLabelWithoutUnits();
  void formatValueAppendsUnitsToNumbers();
  void formatValueKeepsTextAndEmptyValues();
  void formatValueIgnoresDecimalChoicesLikeTheGridDid();
  void captionTextOverridesOnlyForLabelsAndNotation();
};

//--------------------------------------------------------------------------------------------------
// formatNumber
//--------------------------------------------------------------------------------------------------

void TstValueFormat::formatNumberFollowsTheLabelFormat_data()
{
  QTest::addColumn<QString>("format");
  QTest::addColumn<double>("value");
  QTest::addColumn<QString>("expected");

  const char* fmts[]        = {"", "0d", "1d", "2d", "3d", "sci", "hex", "bin"};
  const double values[]     = {0, -1, 255, 65535, 2.5};
  const char* expected[][5] = {
    {   "0.00",   "-1.000",     "255.00",              "65535",   "2.500"},
    {      "0",       "-1",        "255",              "65535",       "3"},
    {    "0.0",     "-1.0",      "255.0",            "65535.0",     "2.5"},
    {   "0.00",    "-1.00",     "255.00",           "65535.00",    "2.50"},
    {  "0.000",   "-1.000",    "255.000",          "65535.000",   "2.500"},
    {"0.00e+0", "-1.00e+0",    "2.55e+2",            "6.55e+4", "2.50e+0"},
    {    "0x0",     "-0x1",       "0xFF",             "0xFFFF",     "0x3"},
    {    "0b0",     "-0b1", "0b11111111", "0b1111111111111111",    "0b11"},
  };

  for (std::size_t f = 0; f < std::size(fmts); ++f)
    for (std::size_t v = 0; v < std::size(values); ++v)
      QTest::addRow("%s %g", fmts[f], values[v])
        << QString::fromLatin1(fmts[f]) << values[v] << QString::fromLatin1(expected[f][v]);
}

/**
 * @brief Each Label Format renders the number the way the tick labels always did, plus the two
 *        notations: minimal digits and a sign when no decoder declares a width.
 */
void TstValueFormat::formatNumberFollowsTheLabelFormat()
{
  QFETCH(QString, format);
  QFETCH(double, value);
  QFETCH(QString, expected);

  QCOMPARE(ValueFormat::formatNumber(value, withFormat(format)), expected);
}

void TstValueFormat::notationFollowsTheDeclaredWidth_data()
{
  QTest::addColumn<QString>("format");
  QTest::addColumn<int>("width");
  QTest::addColumn<double>("value");
  QTest::addColumn<QString>("expected");

  const auto hex = QStringLiteral("hex");
  const auto bin = QStringLiteral("bin");
  QTest::newRow("hex -1 at 8") << hex << 8 << -1.0 << QStringLiteral("0xFF");
  QTest::newRow("bin -1 at 8") << bin << 8 << -1.0 << QStringLiteral("0b11111111");
  QTest::newRow("hex -128 at 8") << hex << 8 << -128.0 << QStringLiteral("0x80");
  QTest::newRow("hex 0 at 8") << hex << 8 << 0.0 << QStringLiteral("0x00");
  QTest::newRow("bin 5 at 8") << bin << 8 << 5.0 << QStringLiteral("0b00000101");
  QTest::newRow("hex 255 at 16") << hex << 16 << 255.0 << QStringLiteral("0x00FF");
  QTest::newRow("hex 255 at 12") << hex << 12 << 255.0 << QStringLiteral("0x0FF");
  QTest::newRow("hex -1 at 64") << hex << 64 << -1.0 << QStringLiteral("0xFFFFFFFFFFFFFFFF");
  QTest::newRow("bin 2.6 at 4") << bin << 4 << 2.6 << QStringLiteral("0b0011");
}

/**
 * @brief With a fixed-point or bit-field decoder, the notation shows two's complement zero-padded
 *        to the declared integer width.
 */
void TstValueFormat::notationFollowsTheDeclaredWidth()
{
  QFETCH(QString, format);
  QFETCH(int, width);
  QFETCH(double, value);
  QFETCH(QString, expected);

  auto d        = withFormat(format);
  d.format      = ValueFormatFixedPoint;
  d.formatWidth = width;
  QCOMPARE(ValueFormat::formatNumber(value, d), expected);

  d.format = ValueFormatBitField;
  QCOMPARE(ValueFormat::formatNumber(value, d), expected);
}

/**
 * @brief Fixed decimal points override every Label Format, the notations included.
 */
void TstValueFormat::decimalPointsWinOverTheLabelFormat()
{
  auto d          = withFormat(QStringLiteral("hex"));
  d.decimalPoints = 1;
  QCOMPARE(ValueFormat::formatNumber(255, d), QStringLiteral("255.0"));

  d.displayFormat = QStringLiteral("sci");
  d.decimalPoints = 0;
  QCOMPARE(ValueFormat::formatNumber(2.4, d), QStringLiteral("2"));

  auto shown          = holding(255, QStringLiteral("255"));
  shown.displayFormat = QStringLiteral("hex");
  shown.decimalPoints = 2;
  QCOMPARE(ValueFormat::formatValue(shown), QStringLiteral("255.00"));
}

/**
 * @brief Legacy printf-style formats still reach the tick labels, capped at 12 places.
 */
void TstValueFormat::printfFormatsKeepWorking()
{
  QCOMPARE(ValueFormat::formatNumber(3.14159, withFormat(QStringLiteral("%.2f"))),
           QStringLiteral("3.14"));
  QCOMPARE(ValueFormat::formatNumber(1234.5, withFormat(QStringLiteral("%8.1e"))),
           QStringLiteral("1.2e+3"));
  QCOMPARE(ValueFormat::formatNumber(1.0, withFormat(QStringLiteral("%.20f"))),
           QStringLiteral("1.000000000000"));
  QCOMPARE(ValueFormat::formatNumber(2.5, withFormat(QStringLiteral("%f"))),
           QStringLiteral("2.500"));
}

/**
 * @brief The automatic format takes its decimals from the range ends, or from the value when the
 *        range is unset; the tick formatter reads the widget range.
 */
void TstValueFormat::autoNumberFollowsTheRange()
{
  QCOMPARE(ValueFormat::autoNumber(1234.5678, 1000, 5000), QStringLiteral("1234.6"));
  QCOMPARE(ValueFormat::autoNumber(1234.5678, 0, 0), QStringLiteral("1234.6"));
  QCOMPARE(ValueFormat::autoNumber(0.5, 0, 0.001), QStringLiteral("0.500000"));

  auto d   = withFormat(QString());
  d.wgtMin = 0;
  d.wgtMax = 50000;
  QCOMPARE(ValueFormat::formatNumber(2.5, d), QStringLiteral("2.50"));
}

//--------------------------------------------------------------------------------------------------
// labelFor
//--------------------------------------------------------------------------------------------------

/**
 * @brief The value is rounded to the nearest integer before the lookup.
 */
void TstValueFormat::labelForMatchesTheRoundedKey()
{
  const auto d = gearbox();
  QCOMPARE(ValueFormat::labelFor(d, 1), QStringLiteral("Reverse"));
  QCOMPARE(ValueFormat::labelFor(d, 2.4), QStringLiteral("Neutral"));
  QCOMPARE(ValueFormat::labelFor(d, 2.6), QStringLiteral("Drive"));
  QCOMPARE(ValueFormat::labelFor(d, -0.4), QStringLiteral("Park"));
}

/**
 * @brief An unlisted value shows the catch-all text when one is set, else nothing.
 */
void TstValueFormat::labelForFallsBackToTheCatchAll()
{
  auto d = gearbox();
  QVERIFY(ValueFormat::labelFor(d, 7).isEmpty());

  d.labelsDefault = QStringLiteral("Fault");
  QCOMPARE(ValueFormat::labelFor(d, 7), QStringLiteral("Fault"));
  QCOMPARE(ValueFormat::labelFor(d, 3), QStringLiteral("Drive"));

  QVERIFY(ValueFormat::labelFor(Dataset(), 1).isEmpty());
}

/**
 * @brief A value with no integer form matches no row and never shows the catch-all.
 */
void TstValueFormat::labelForIgnoresANonFiniteValue()
{
  auto d          = gearbox();
  d.labelsDefault = QStringLiteral("Fault");
  QVERIFY(ValueFormat::labelFor(d, std::numeric_limits<double>::quiet_NaN()).isEmpty());
  QVERIFY(ValueFormat::labelFor(d, std::numeric_limits<double>::infinity()).isEmpty());
}

//--------------------------------------------------------------------------------------------------
// formatValue
//--------------------------------------------------------------------------------------------------

/**
 * @brief A matched label replaces the number and carries no units.
 */
void TstValueFormat::formatValueShowsTheLabelWithoutUnits()
{
  auto d         = gearbox();
  d.isNumeric    = true;
  d.numericValue = 1;
  d.value        = QStringLiteral("1");
  d.units        = QStringLiteral("gear");
  QCOMPARE(ValueFormat::formatValue(d), QStringLiteral("Reverse"));

  d.numericValue = 9;
  d.value        = QStringLiteral("9");
  QCOMPARE(ValueFormat::formatValue(d), QStringLiteral("9.000 gear"));
}

/**
 * @brief A number keeps today's grid rendering with the units appended, notation included.
 */
void TstValueFormat::formatValueAppendsUnitsToNumbers()
{
  auto d  = holding(3.3, QStringLiteral("3.3"));
  d.units = QStringLiteral("V");
  QCOMPARE(ValueFormat::formatValue(d), QStringLiteral("3.300 V"));

  d.pltMin = 0;
  d.pltMax = 1000;
  QCOMPARE(ValueFormat::formatValue(d), QStringLiteral("3.30 V"));

  auto word          = holding(255, QStringLiteral("255"));
  word.displayFormat = QStringLiteral("hex");
  QCOMPARE(ValueFormat::formatValue(word), QStringLiteral("0xFF"));
}

/**
 * @brief No sample yet stays empty; a text value is shown verbatim, labels or not.
 */
void TstValueFormat::formatValueKeepsTextAndEmptyValues()
{
  auto d = gearbox();
  QVERIFY(ValueFormat::formatValue(d).isEmpty());

  d.value     = QStringLiteral("ERR");
  d.isNumeric = false;
  d.units     = QStringLiteral("gear");
  QCOMPARE(ValueFormat::formatValue(d), QStringLiteral("ERR"));
}

/**
 * @brief Every dataset stores "0d" by default, so the decimal Label Format choices never reach
 *        the value display: the grid keeps its automatic precision, as it always had.
 */
void TstValueFormat::formatValueIgnoresDecimalChoicesLikeTheGridDid()
{
  auto d = holding(2.5, QStringLiteral("2.5"));
  QCOMPARE(ValueFormat::formatValue(d), QStringLiteral("2.500"));

  d.displayFormat = QStringLiteral("sci");
  QCOMPARE(ValueFormat::formatValue(d), QStringLiteral("2.500"));
}

/**
 * @brief An analog caption keeps its own padded number unless a label matches or a notation is
 *        chosen; fixed decimal points still win over the notation.
 */
void TstValueFormat::captionTextOverridesOnlyForLabelsAndNotation()
{
  auto d = holding(2, QStringLiteral("2"));
  QVERIFY(ValueFormat::captionText(d).isEmpty());

  d.labels = gearbox().labels;
  QCOMPARE(ValueFormat::captionText(d), QStringLiteral("Neutral"));

  d.labels.clear();
  d.displayFormat = QStringLiteral("bin");
  d.units         = QStringLiteral("flags");
  QCOMPARE(ValueFormat::captionText(d), QStringLiteral("0b10 flags"));

  d.decimalPoints = 1;
  QVERIFY(ValueFormat::captionText(d).isEmpty());

  d.isNumeric = false;
  QVERIFY(ValueFormat::captionText(d).isEmpty());
}

QTEST_APPLESS_MAIN(TstValueFormat)

#include "tst_value_format.moc"
