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

#include <bit>
#include <cmath>
#include <limits>
#include <QString>
#include <QTest>

#include "Core/DataModel/Frame.h"
#include "Core/DataModel/PropertyValidators.h"
#include "Core/DataModel/ValueDecoder.h"

// Spec 0108 AC1: every expected value is a literal worked out by hand, never computed by the code
// under test, so a decoder that drifts by one bit fails here rather than agreeing with itself.

using namespace DataModel;

/**
 * @brief Compares two doubles bit for bit; QCOMPARE on doubles is fuzzy and the decoders promise
 *        bit-identical results.
 */
static bool sameBits(double actual, double expected)
{
  return std::bit_cast<quint64>(actual) == std::bit_cast<quint64>(expected);
}

/**
 * @brief Builds a dataset declaring the integer decoder @p format at @p width bits.
 */
static Dataset integerDecoder(int format, int width, bool isSigned, int fractionBits)
{
  Dataset d;
  d.format             = format;
  d.formatWidth        = width;
  d.formatSigned       = isSigned;
  d.formatFractionBits = fractionBits;
  return d;
}

/**
 * @brief Builds a dataset extracting @p bitWidth bits at @p bitOffset of a @p width-bit word.
 */
static Dataset bitField(int width, int bitOffset, int bitWidth)
{
  Dataset d;
  d.format          = ValueFormatBitField;
  d.formatWidth     = width;
  d.formatBitOffset = bitOffset;
  d.formatBitWidth  = bitWidth;
  return d;
}

/**
 * @brief Bit-exact contract of the dataset value decoders and the declarations they accept.
 */
class TstValueDecoder : public QObject {
  Q_OBJECT

private slots:
  void fixedPoint_data();
  void fixedPoint();
  void bitFieldExtraction_data();
  void bitFieldExtraction();
  void linearScale_data();
  void linearScale();

  void noDecoderLeavesTheValue();
  void nonFiniteInputIsLeftAsIs();
  void decodeInPlaceRewritesNumberAndText();
  void decodeInPlaceLeavesTextAndOtherStages();

  void fractionValidatorRefusesBitsPastTheWidth();
  void bitFieldValidatorRefusesAFieldPastTheWidth();
  void formatValidatorAcceptsTheFourKinds();
};

//--------------------------------------------------------------------------------------------------
// Fixed point
//--------------------------------------------------------------------------------------------------

void TstValueDecoder::fixedPoint_data()
{
  QTest::addColumn<int>("width");
  QTest::addColumn<bool>("isSigned");
  QTest::addColumn<int>("fractionBits");
  QTest::addColumn<double>("raw");
  QTest::addColumn<double>("expected");

  QTest::newRow("u8 q0") << 8 << false << 0 << 200.0 << 200.0;
  QTest::newRow("s8 q0 negative") << 8 << true << 0 << 200.0 << -56.0;
  QTest::newRow("u8 q4") << 8 << false << 4 << 240.0 << 15.0;
  QTest::newRow("s8 q4") << 8 << true << 4 << 240.0 << -1.0;
  QTest::newRow("s8 q7 half") << 8 << true << 7 << 64.0 << 0.5;
  QTest::newRow("s8 q7 top bit") << 8 << true << 7 << 128.0 << -1.0;
  QTest::newRow("u8 masks wider input") << 8 << false << 0 << 300.0 << 44.0;
  QTest::newRow("u16 q15 0x7FFF") << 16 << false << 15 << 32767.0 << 0.999969482421875;
  QTest::newRow("u16 q15 0x8000") << 16 << false << 15 << 32768.0 << 1.0;
  QTest::newRow("u16 q15 0xFFFF") << 16 << false << 15 << 65535.0 << 1.999969482421875;
  QTest::newRow("s16 q15 0x7FFF") << 16 << true << 15 << 32767.0 << 0.999969482421875;
  QTest::newRow("s16 q15 0x8000") << 16 << true << 15 << 32768.0 << -1.0;
  QTest::newRow("s16 q15 0xFFFF") << 16 << true << 15 << 65535.0 << -0.000030517578125;
  QTest::newRow("s16 q4 0xFFF0") << 16 << true << 4 << 65520.0 << -1.0;
  QTest::newRow("s16 q4 negative input") << 16 << true << 4 << -16.0 << -1.0;
  QTest::newRow("s16 rounds input") << 16 << true << 0 << 2.6 << 3.0;
  QTest::newRow("u32 q16") << 32 << false << 16 << 229376.0 << 3.5;
  QTest::newRow("s32 q0 all ones") << 32 << true << 0 << 4294967295.0 << -1.0;
  QTest::newRow("s32 q31 top bit") << 32 << true << 31 << 2147483648.0 << -1.0;
  QTest::newRow("s32 q31 half") << 32 << true << 31 << 1073741824.0 << 0.5;
  QTest::newRow("u64 q0 2^53") << 64 << false << 0 << 9007199254740992.0 << 9007199254740992.0;
  QTest::newRow("s64 q0 minus one") << 64 << true << 0 << -1.0 << -1.0;
  QTest::newRow("s64 q63 top bit") << 64 << true << 63 << -9223372036854775808.0 << -1.0;
  QTest::newRow("u64 q63 top bit") << 64 << false << 63 << 9223372036854775808.0 << 1.0;
}

/**
 * @brief Fixed point reads the rounded integer's low width bits, sign-extends a signed width and
 *        scales by 2^-fractionBits, bit-exactly.
 */
void TstValueDecoder::fixedPoint()
{
  QFETCH(int, width);
  QFETCH(bool, isSigned);
  QFETCH(int, fractionBits);
  QFETCH(double, raw);
  QFETCH(double, expected);

  const auto d = integerDecoder(ValueFormatFixedPoint, width, isSigned, fractionBits);
  QVERIFY(has_decoder(d));

  const double actual = decode_value(d, raw);
  QVERIFY2(sameBits(actual, expected), qPrintable(QString::number(actual, 'g', 17)));
}

//--------------------------------------------------------------------------------------------------
// Bit field
//--------------------------------------------------------------------------------------------------

void TstValueDecoder::bitFieldExtraction_data()
{
  QTest::addColumn<int>("width");
  QTest::addColumn<int>("bitOffset");
  QTest::addColumn<int>("bitWidth");
  QTest::addColumn<double>("raw");
  QTest::addColumn<double>("expected");

  QTest::newRow("full 16-bit field") << 16 << 0 << 16 << 48879.0 << 48879.0;
  QTest::newRow("full field of -1") << 16 << 0 << 16 << -1.0 << 65535.0;
  QTest::newRow("top bit of 16 set") << 16 << 15 << 1 << 32768.0 << 1.0;
  QTest::newRow("top bit of 16 clear") << 16 << 15 << 1 << 32767.0 << 0.0;
  QTest::newRow("3 bits at 5, all set") << 16 << 5 << 3 << 224.0 << 7.0;
  QTest::newRow("3 bits at 5, 0b101") << 16 << 5 << 3 << 160.0 << 5.0;
  QTest::newRow("3 bits at 5, higher bits") << 16 << 5 << 3 << 928.0 << 5.0;
  QTest::newRow("bit 0 of an odd word") << 8 << 0 << 1 << 129.0 << 1.0;
  QTest::newRow("full 64-bit field") << 64 << 0 << 64 << 9007199254740992.0 << 9007199254740992.0;
  QTest::newRow("bit 63 of 2^63") << 64 << 63 << 1 << 9223372036854775808.0 << 1.0;
  QTest::newRow("bit 63 of -1") << 64 << 63 << 1 << -1.0 << 1.0;
}

/**
 * @brief A bit field shifts the masked word right by the offset and keeps bitWidth bits; it is
 *        never sign-extended, so a signed declaration reads the same field.
 */
void TstValueDecoder::bitFieldExtraction()
{
  QFETCH(int, width);
  QFETCH(int, bitOffset);
  QFETCH(int, bitWidth);
  QFETCH(double, raw);
  QFETCH(double, expected);

  auto d                = bitField(width, bitOffset, bitWidth);
  const double plain    = decode_value(d, raw);
  d.formatSigned        = true;
  const double asSigned = decode_value(d, raw);

  QVERIFY2(sameBits(plain, expected), qPrintable(QString::number(plain, 'g', 17)));
  QVERIFY2(sameBits(asSigned, expected), qPrintable(QString::number(asSigned, 'g', 17)));
}

//--------------------------------------------------------------------------------------------------
// Linear scale
//--------------------------------------------------------------------------------------------------

void TstValueDecoder::linearScale_data()
{
  QTest::addColumn<double>("scale");
  QTest::addColumn<double>("offset");
  QTest::addColumn<double>("raw");
  QTest::addColumn<double>("expected");

  QTest::newRow("half, minus 40") << 0.5 << -40.0 << 100.0 << 10.0;
  QTest::newRow("zero raw is the offset") << 0.5 << -40.0 << 0.0 << -40.0;
  QTest::newRow("negative raw") << 2.0 << -0.5 << -20.0 << -40.5;
  QTest::newRow("no reinterpretation") << 1.0 << 0.0 << 70000.25 << 70000.25;
}

/**
 * @brief Scale computes raw * scale + offset on the double, with no integer reinterpretation.
 */
void TstValueDecoder::linearScale()
{
  QFETCH(double, scale);
  QFETCH(double, offset);
  QFETCH(double, raw);
  QFETCH(double, expected);

  Dataset d;
  d.format       = ValueFormatScale;
  d.formatScale  = scale;
  d.formatOffset = offset;

  const double actual = decode_value(d, raw);
  QVERIFY2(sameBits(actual, expected), qPrintable(QString::number(actual, 'g', 17)));
}

//--------------------------------------------------------------------------------------------------
// Pass-through cases
//--------------------------------------------------------------------------------------------------

/**
 * @brief No decoder, or a kind no release knows, hands the raw value back untouched.
 */
void TstValueDecoder::noDecoderLeavesTheValue()
{
  Dataset d;
  QVERIFY(!has_decoder(d));
  QCOMPARE(decode_value(d, 3.7), 3.7);

  d.format = 9;
  QCOMPARE(decode_value(d, 3.7), 3.7);
}

/**
 * @brief A value with no 64-bit pattern (NaN, infinity) is left as is by the integer decoders.
 */
void TstValueDecoder::nonFiniteInputIsLeftAsIs()
{
  const auto fixed = integerDecoder(ValueFormatFixedPoint, 16, true, 15);
  const auto bits  = bitField(16, 5, 3);
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double inf = std::numeric_limits<double>::infinity();

  QVERIFY(std::isnan(decode_value(fixed, nan)));
  QVERIFY(std::isnan(decode_value(bits, nan)));
  QCOMPARE(decode_value(fixed, inf), inf);
  QCOMPARE(decode_value(bits, -inf), -inf);
}

//--------------------------------------------------------------------------------------------------
// In-place decode
//--------------------------------------------------------------------------------------------------

/**
 * @brief The in-place step replaces the number and re-renders the text from it.
 */
void TstValueDecoder::decodeInPlaceRewritesNumberAndText()
{
  auto d         = integerDecoder(ValueFormatFixedPoint, 16, true, 15);
  d.isNumeric    = true;
  d.numericValue = 32768.0;
  d.value        = QStringLiteral("32768");

  decode_dataset_in_place(d, true);
  QCOMPARE(d.numericValue, -1.0);
  QCOMPARE(d.value, QStringLiteral("-1"));

  d.numericValue = 16384.0;
  decode_dataset_in_place(d, true);
  QCOMPARE(d.numericValue, 0.5);
  QCOMPARE(d.value, QStringLiteral("0.5"));
}

/**
 * @brief A text token, or a stage the dataset does not decode at, is left exactly as received.
 */
void TstValueDecoder::decodeInPlaceLeavesTextAndOtherStages()
{
  auto d         = integerDecoder(ValueFormatFixedPoint, 16, true, 15);
  d.isNumeric    = false;
  d.numericValue = 0.0;
  d.value        = QStringLiteral("Reverse");

  decode_dataset_in_place(d, true);
  QCOMPARE(d.value, QStringLiteral("Reverse"));
  QCOMPARE(d.numericValue, 0.0);

  d.isNumeric    = true;
  d.numericValue = 32768.0;
  d.value        = QStringLiteral("32768");
  decode_dataset_in_place(d, false);
  QCOMPARE(d.numericValue, 32768.0);
  QCOMPARE(d.value, QStringLiteral("32768"));
}

//--------------------------------------------------------------------------------------------------
// Declaration validators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fraction bits run 0 to width - 1: one integer or sign bit always remains.
 */
void TstValueDecoder::fractionValidatorRefusesBitsPastTheWidth()
{
  QVERIFY(PropertyHooks::isValidFormatFraction(integerDecoder(1, 16, true, 0)));
  QVERIFY(PropertyHooks::isValidFormatFraction(integerDecoder(1, 16, true, 15)));
  QVERIFY(PropertyHooks::isValidFormatFraction(integerDecoder(1, 64, true, 63)));
  QVERIFY(!PropertyHooks::isValidFormatFraction(integerDecoder(1, 16, true, 16)));
  QVERIFY(!PropertyHooks::isValidFormatFraction(integerDecoder(1, 8, false, 12)));
  QVERIFY(!PropertyHooks::isValidFormatFraction(integerDecoder(1, 16, false, -1)));
}

/**
 * @brief A bit field must end at or below the declared width's top bit.
 */
void TstValueDecoder::bitFieldValidatorRefusesAFieldPastTheWidth()
{
  QVERIFY(PropertyHooks::isValidFormatBitField(bitField(16, 0, 16)));
  QVERIFY(PropertyHooks::isValidFormatBitField(bitField(16, 13, 3)));
  QVERIFY(PropertyHooks::isValidFormatBitField(bitField(16, 15, 1)));
  QVERIFY(PropertyHooks::isValidFormatBitField(bitField(64, 63, 1)));
  QVERIFY(!PropertyHooks::isValidFormatBitField(bitField(16, 14, 3)));
  QVERIFY(!PropertyHooks::isValidFormatBitField(bitField(16, 16, 1)));
  QVERIFY(!PropertyHooks::isValidFormatBitField(bitField(64, 63, 2)));
  QVERIFY(!PropertyHooks::isValidFormatBitField(bitField(16, 0, 0)));
}

/**
 * @brief The format field accepts none, fixed point, bit field and scale, and nothing else.
 */
void TstValueDecoder::formatValidatorAcceptsTheFourKinds()
{
  QVERIFY(PropertyHooks::isValidValueFormat(0));
  QVERIFY(PropertyHooks::isValidValueFormat(1));
  QVERIFY(PropertyHooks::isValidValueFormat(2));
  QVERIFY(PropertyHooks::isValidValueFormat(3));
  QVERIFY(!PropertyHooks::isValidValueFormat(-1));
  QVERIFY(!PropertyHooks::isValidValueFormat(4));
}

QTEST_APPLESS_MAIN(TstValueDecoder)

#include "tst_value_decoder.moc"
