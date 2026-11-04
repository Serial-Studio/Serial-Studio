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

#include <array>
#include <QByteArray>
#include <QJsonObject>
#include <QStringList>
#include <QTest>

#include "DataModel/Scripting/NativeTemplates/BinaryPackedStruct.h"

// The packed_struct Built-In template (spec 0111 R14, AC10): every field type in both byte
// orders against literal values, the single-precision decimal rule, a wrong frame length and the
// layout refusals. The frames were packed with Python's struct module ("<bBhHiIqQfd").

//--------------------------------------------------------------------------------------------------
// Shared inputs
//--------------------------------------------------------------------------------------------------

static const QString kAllFields = QStringLiteral("i8,u8,i16,u16,i32,u32,i64,u64,f32,f64");

static const QByteArray kLittle = QByteArray::fromHex(
  "ffc8fefffffffdffffff01286beefcffffffffffffff00000000000100000000c03f00000000000002c0");

static const QByteArray kBig = QByteArray::fromHex(
  "ffc8fffefffffffffffdee6b2801fffffffffffffffc00000100000000003fc00000c002000000000000");

static const std::array<double, 10> kExpected = {
  -1, 200, -2, 65535, -3, 4000000001.0, -4, 1099511627776.0, 1.5, -2.25};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The layout for @p fields in @p endianness; fails the test when it does not parse.
 */
[[nodiscard]] static DataModel::PackedLayout layoutOf(const QString& fields,
                                                      const QString& endianness)
{
  QString error;
  DataModel::PackedLayout layout;
  const bool ok = DataModel::parsePackedLayout(fields, endianness, layout, error);
  if (!ok)
    qWarning() << "layout refused:" << error;

  return layout;
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class PackedStructTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void everyType_data();
  void everyType();
  void floatReadsAsItsShortestDecimal();
  void wrongLengthIsRejected();
  void layoutRefusals_data();
  void layoutRefusals();
  void templateEmitsOneRow();
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting.
 */
void PackedStructTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief One row per byte order.
 */
void PackedStructTest::everyType_data()
{
  QTest::addColumn<QString>("endianness");
  QTest::addColumn<QByteArray>("frame");

  QTest::newRow("little") << QStringLiteral("little") << kLittle;
  QTest::newRow("big") << QStringLiteral("big") << kBig;
}

/**
 * @brief Each of the ten types decodes to its literal value, and the layout adds up to 42 bytes.
 */
void PackedStructTest::everyType()
{
  QFETCH(QString, endianness);
  QFETCH(QByteArray, frame);

  const auto layout = layoutOf(kAllFields, endianness);
  QCOMPARE(layout.count, 10);
  QCOMPARE(layout.bytes, 42);
  QCOMPARE(frame.size(), 42);

  std::array<double, 10> values{};
  QCOMPARE(DataModel::decodePackedStruct(frame, layout, values.data(), values.size()), 10);
  for (size_t i = 0; i < values.size(); ++i)
    QCOMPARE(values[i], kExpected[i]);
}

/**
 * @brief 1.2f reads as 1.2, not as the widened 1.2000000476837158, in either byte order.
 */
void PackedStructTest::floatReadsAsItsShortestDecimal()
{
  double value      = 0;
  const auto little = layoutOf(QStringLiteral("f32"), QStringLiteral("little"));
  QCOMPARE(DataModel::decodePackedStruct(QByteArray::fromHex("9a99993f"), little, &value, 1), 1);
  QCOMPARE(value, 1.2);

  const auto big = layoutOf(QStringLiteral("f32"), QStringLiteral("big"));
  QCOMPARE(DataModel::decodePackedStruct(QByteArray::fromHex("3f99999a"), big, &value, 1), 1);
  QCOMPARE(value, 1.2);
}

/**
 * @brief A frame one byte short or long, an empty frame and a short output buffer are refused.
 */
void PackedStructTest::wrongLengthIsRejected()
{
  const auto layout = layoutOf(kAllFields, QStringLiteral("little"));
  std::array<double, 10> values{};
  QCOMPARE(DataModel::decodePackedStruct(kLittle.chopped(1), layout, values.data(), 10), -1);
  QCOMPARE(DataModel::decodePackedStruct(kLittle + '\0', layout, values.data(), 10), -1);
  QCOMPARE(DataModel::decodePackedStruct(QByteArray(), layout, values.data(), 10), -1);
  QCOMPARE(DataModel::decodePackedStruct(kLittle, layout, values.data(), 9), -1);

  QString error;
  const auto parser = DataModel::packedStructTemplate().makeParser(
    QJsonObject{
      {    QStringLiteral("fields"),               kAllFields},
      {QStringLiteral("endianness"), QStringLiteral("little")}
  },
    error);
  QVERIFY2(parser, qPrintable(error));
  QVERIFY(parser->parseBinary(kLittle.chopped(1)).isEmpty());
}

/**
 * @brief One row per layout the template must refuse.
 */
void PackedStructTest::layoutRefusals_data()
{
  QTest::addColumn<QString>("fields");
  QTest::addColumn<QString>("endianness");

  QStringList tooMany;
  for (int i = 0; i <= DataModel::kMaxPackedFields; ++i)
    tooMany.append(QStringLiteral("u8"));

  QTest::newRow("empty") << QString() << QStringLiteral("little");
  QTest::newRow("unknown type") << QStringLiteral("f32,f16") << QStringLiteral("little");
  QTest::newRow("too many") << tooMany.join(',') << QStringLiteral("little");
  QTest::newRow("byte order") << QStringLiteral("f32") << QStringLiteral("middle");
}

/**
 * @brief A refused layout reports why and builds no parser.
 */
void PackedStructTest::layoutRefusals()
{
  QFETCH(QString, fields);
  QFETCH(QString, endianness);

  QString error;
  DataModel::PackedLayout layout;
  QVERIFY(!DataModel::parsePackedLayout(fields, endianness, layout, error));
  QVERIFY(!error.isEmpty());

  error.clear();
  const auto parser = DataModel::packedStructTemplate().makeParser(
    QJsonObject{
      {    QStringLiteral("fields"),     fields},
      {QStringLiteral("endianness"), endianness}
  },
    error);
  QVERIFY(!parser);
  QVERIFY(!error.isEmpty());
}

/**
 * @brief The template's parser emits one row with one shortest-decimal cell per field.
 */
void PackedStructTest::templateEmitsOneRow()
{
  QString error;
  const auto parser = DataModel::packedStructTemplate().makeParser(
    QJsonObject{
      {    QStringLiteral("fields"),            kAllFields},
      {QStringLiteral("endianness"), QStringLiteral("big")}
  },
    error);
  QVERIFY2(parser, qPrintable(error));
  QCOMPARE(DataModel::packedStructTemplate().id(), QStringLiteral("packed_struct"));

  const auto rows = parser->parseBinary(kBig);
  QCOMPARE(rows.size(), 1);
  QCOMPARE(rows.first(),
           QStringList({QStringLiteral("-1"),
                        QStringLiteral("200"),
                        QStringLiteral("-2"),
                        QStringLiteral("65535"),
                        QStringLiteral("-3"),
                        QStringLiteral("4000000001"),
                        QStringLiteral("-4"),
                        QStringLiteral("1099511627776"),
                        QStringLiteral("1.5"),
                        QStringLiteral("-2.25")}));
}

QTEST_APPLESS_MAIN(PackedStructTest)

#include "tst_packed_struct.moc"
