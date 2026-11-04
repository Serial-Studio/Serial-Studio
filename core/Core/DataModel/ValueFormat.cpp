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

#include "Core/DataModel/ValueFormat.h"

#include <algorithm>
#include <cmath>

#include "Core/DataModel/Frame.h"
#include "Core/DataModel/ValueDecoder.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Number helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Magnitude past which a double has no qint64 rounding (2^63).
 */
static constexpr double kInt64Limit = 9223372036854775808.0;

/**
 * @brief Decimal places the automatic format gives @p value: fewer as the magnitude grows.
 */
static int autoDecimals(double value)
{
  static constexpr double kFloors[] = {1e4, 1e3, 1e1, 1.0, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6};
  static constexpr int kDecimals[]  = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  static_assert(std::size(kFloors) == std::size(kDecimals));

  const double magnitude = std::abs(value);
  if (magnitude <= 1e-12)
    return 2;

  for (std::size_t i = 0; i < std::size(kFloors); ++i)
    if (magnitude >= kFloors[i])
      return kDecimals[i];

  return 10;
}

/**
 * @brief Renders @p value in exponential notation with @p digits decimals and the exponent's
 *        leading zeros dropped, the shape JavaScript's toExponential() prints (1.50e+3).
 */
static QString exponential(double value, int digits)
{
  auto text      = QString::number(value, 'e', digits);
  const auto exp = text.indexOf(QLatin1Char('e'));
  if (exp < 0 || exp + 3 > text.size())
    return text;

  qsizetype first = exp + 2;
  while (first < text.size() - 1 && text.at(first) == QLatin1Char('0'))
    ++first;

  text.remove(exp + 2, first - (exp + 2));
  return text;
}

/**
 * @brief Returns the precision of a printf-style "%.Nf" / "%.Ne" format ending in @p conversion,
 *        or -1 when @p fmt is not one; N is capped at 12 like the tick formatter always did.
 */
static int printfPrecision(const QString& fmt, QChar conversion)
{
  if (fmt.size() < 4 || fmt.front() != QLatin1Char('%') || fmt.back() != conversion)
    return -1;

  const auto body = QStringView(fmt).mid(1, fmt.size() - 2);
  const auto dot  = body.lastIndexOf(QLatin1Char('.'));
  if (dot < 0 || dot == body.size() - 1)
    return -1;

  const auto isDigitOrDot = [](QChar c) {
    return c.isDigit() || c == QLatin1Char('.');
  };
  if (!std::all_of(body.begin(), body.end(), isDigitOrDot))
    return -1;

  bool ok        = false;
  const int prec = body.mid(dot + 1).toInt(&ok);
  return ok ? std::min(12, prec) : -1;
}

/**
 * @brief Renders @p value in hexadecimal (@p base 16) or binary (@p base 2) with a 0x / 0b
 *        prefix: two's complement zero-padded to formatWidth when the decoder declares a width,
 *        minimal digits with a minus sign otherwise. Empty when the value has no integer form.
 */
static QString notation(double value, const DataModel::Dataset& d, int base)
{
  SS_ASSERT_LOG(base == 16 || base == 2);
  if (!(std::abs(value) < kInt64Limit))
    return QString();

  const qint64 integer = std::llround(value);
  const auto prefix    = base == 16 ? QStringLiteral("0x") : QStringLiteral("0b");
  const bool hasWidth =
    d.format == DataModel::ValueFormatFixedPoint || d.format == DataModel::ValueFormatBitField;
  if (!hasWidth) {
    const auto magnitude =
      integer < 0 ? quint64{0} - static_cast<quint64>(integer) : static_cast<quint64>(integer);
    const auto sign = integer < 0 ? QStringLiteral("-") : QString();
    return sign + prefix + QString::number(magnitude, base).toUpper();
  }

  const int width  = std::clamp(d.formatWidth, 1, 64);
  const auto all   = static_cast<quint64>(integer);
  const auto bits  = width >= 64 ? all : all & ((quint64{1} << width) - 1);
  const int digits = base == 16 ? (width + 3) / 4 : width;
  SS_ASSERT_LOG(digits >= 1 && digits <= 64);
  return prefix + QString::number(bits, base).toUpper().rightJustified(digits, QLatin1Char('0'));
}

/**
 * @brief Returns the notation base @p fmt names (16 or 2), or 0 for a decimal format.
 */
static int notationBase(const QString& fmt)
{
  if (fmt == QLatin1String("hex"))
    return 16;

  if (fmt == QLatin1String("bin"))
    return 2;

  return 0;
}

/**
 * @brief Renders the number of a value display (Data Grid, captions, extension rows): fixed
 *        decimals when set, else the chosen notation, else the automatic format over the plot
 *        range. The decimal Label Format choices keep reaching tick labels only, as before.
 */
static QString displayNumber(double value, const DataModel::Dataset& d)
{
  if (d.decimalPoints >= 0)
    return QString::number(value, 'f', d.decimalPoints);

  if (const int base = notationBase(d.displayFormat); base != 0) {
    auto text = notation(value, d, base);
    if (!text.isEmpty())
      return text;
  }

  return DataModel::ValueFormat::autoNumber(value, d.pltMin, d.pltMax);
}

//--------------------------------------------------------------------------------------------------
// Public formatter
//--------------------------------------------------------------------------------------------------

/**
 * @brief Renders @p value with as many decimals as the magnitude of the range ends (or of the
 *        value, for an unset 0..0 range) calls for; the dashboard's automatic format.
 */
QString DataModel::ValueFormat::autoNumber(double value, double min, double max)
{
  if (std::abs(min) <= 1e-12 && std::abs(max) <= 1e-12)
    return QString::number(value, 'f', autoDecimals(value));

  return QString::number(value, 'f', std::max(autoDecimals(min), autoDecimals(max)));
}

/**
 * @brief Renders a number the way the dataset's tick labels show it: fixed decimals when set,
 *        else the Label Format (Auto over the widget range, 0-3 decimals, scientific, a printf
 *        "%.Nf" / "%.Ne", hexadecimal, binary). No units, no label.
 */
QString DataModel::ValueFormat::formatNumber(double value, const Dataset& d)
{
  if (d.decimalPoints >= 0)
    return QString::number(value, 'f', std::min(12, d.decimalPoints));

  const auto& fmt = d.displayFormat;
  if (const int base = notationBase(fmt); base != 0) {
    auto text = notation(value, d, base);
    if (!text.isEmpty())
      return text;
  }

  if (fmt == QLatin1String("0d"))
    return QString::number(std::floor(value + 0.5), 'f', 0);

  if (fmt.size() == 2 && fmt.back() == QLatin1Char('d') && fmt.front() >= QLatin1Char('1')
      && fmt.front() <= QLatin1Char('3'))
    return QString::number(value, 'f', fmt.front().digitValue());

  if (fmt == QLatin1String("sci"))
    return exponential(value, 2);

  if (const int fixed = printfPrecision(fmt, QLatin1Char('f')); fixed >= 0)
    return QString::number(value, 'f', fixed);

  if (const int exp = printfPrecision(fmt, QLatin1Char('e')); exp >= 0)
    return exponential(value, exp);

  return autoNumber(value, d.wgtMin, d.wgtMax);
}

/**
 * @brief Returns the label text for @p value: the row whose key equals the rounded value, else
 *        the catch-all text, else empty. A value with no integer form never matches.
 */
QString DataModel::ValueFormat::labelFor(const Dataset& d, double value)
{
  if (d.labels.empty() && d.labelsDefault.isEmpty())
    return QString();

  if (!(std::abs(value) < kInt64Limit))
    return QString();

  const qint64 key   = std::llround(value);
  const auto byValue = [](const ValueLabel& row, qint64 k) {
    return row.value < k;
  };
  const auto it = std::lower_bound(d.labels.begin(), d.labels.end(), key, byValue);
  if (it != d.labels.end() && it->value == key)
    return it->text;

  return d.labelsDefault;
}

/**
 * @brief Renders a dataset's current value as text: empty before the first sample, a text value
 *        verbatim, the label when one matches, else the number with the units appended.
 */
QString DataModel::ValueFormat::formatValue(const Dataset& d)
{
  if (d.value.isEmpty())
    return QString();

  if (!d.isNumeric)
    return d.value;

  auto label = labelFor(d, d.numericValue);
  if (!label.isEmpty())
    return label;

  auto text = displayNumber(d.numericValue, d);
  if (!d.units.isEmpty())
    text += QLatin1Char(' ') + d.units;

  return text;
}

/**
 * @brief Returns the text an analog widget's caption shows in place of its padded number: the
 *        value's label, or the Hexadecimal / Binary notation with units. Empty keeps the caption's
 *        own decimal rendering, so a dataset using neither looks exactly as before.
 */
QString DataModel::ValueFormat::captionText(const Dataset& d)
{
  if (d.value.isEmpty() || !d.isNumeric)
    return QString();

  const bool notation = d.decimalPoints < 0 && notationBase(d.displayFormat) != 0;
  if (!notation && labelFor(d, d.numericValue).isEmpty())
    return QString();

  return formatValue(d);
}
