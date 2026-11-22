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

#include <array>
#include <cmath>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>
#include <QTest>

#include "DataModel/Scripting/NativeTemplates/BinaryFieldLayout.h"

// The Field Layout Built-In template (spec 0127 R20/R21, AC12): every type at byte-aligned and
// unaligned offsets in both byte orders, sign extension at 1, 7, 31 and 63 bits, scaling, a field
// past the frame end and the layout refusals. The mixed frame was packed with Python's struct
// module: a status byte, ">h" -123, "<f" 1.2, "<I" 4000000001, "b" -5, ">d" -2.25, "<Q" 2^40.

//--------------------------------------------------------------------------------------------------
// Shared inputs
//--------------------------------------------------------------------------------------------------

static const QByteArray kMixed =
  QByteArray::fromHex("a5ff859a99993f01286beefbc0020000000000000000000000010000");

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief One field object; scale and offset are written only when they differ from the identity,
 *        as the editor does.
 */
[[nodiscard]] static QJsonObject field(const int offset,
                                       const int bit,
                                       const int length,
                                       const char* type,
                                       const char* endian,
                                       const double scale = 1.0,
                                       const double add   = 0.0)
{
  QJsonObject obj;
  obj.insert(QStringLiteral("offset"), offset);
  obj.insert(QStringLiteral("bit"), bit);
  obj.insert(QStringLiteral("length"), length);
  obj.insert(QStringLiteral("type"), QLatin1String(type));
  obj.insert(QStringLiteral("endian"), QLatin1String(endian));
  if (scale != 1.0)
    obj.insert(QStringLiteral("scale"), scale);

  if (add != 0.0)
    obj.insert(QStringLiteral("add"), add);

  return obj;
}

/**
 * @brief Decodes the single field @p spec against @p frame.
 */
[[nodiscard]] static double decodeOne(const QByteArray& frame, const QJsonObject& spec)
{
  QString error;
  DataModel::FieldLayout layout;
  if (!DataModel::parseFieldLayout(QJsonArray{spec}, layout, error)) {
    qWarning() << "layout refused:" << error;
    return std::nan("");
  }

  double value = 0;
  if (DataModel::decodeFieldLayout(frame, layout, &value, 1) != 1)
    return std::nan("");

  return value;
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class FieldLayoutTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void mixedFrame();
  void unalignedFields_data();
  void unalignedFields();
  void signExtension_data();
  void signExtension();
  void scaleAndOffset();
  void fieldPastTheEndIsEmpty();
  void floatWidthComesFromTheType();
  void layoutRefusals_data();
  void layoutRefusals();
  void templateEmitsOneRow();
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting.
 */
void FieldLayoutTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief Seven byte-aligned fields of every type and both byte orders decode to their literals.
 */
void FieldLayoutTest::mixedFrame()
{
  QCOMPARE(kMixed.size(), 28);
  QCOMPARE(decodeOne(kMixed, field(0, 0, 8, "uint", "little")), 165.0);
  QCOMPARE(decodeOne(kMixed, field(1, 0, 16, "int", "big")), -123.0);
  QCOMPARE(decodeOne(kMixed, field(3, 0, 32, "f32", "little")), 1.2);
  QCOMPARE(decodeOne(kMixed, field(7, 0, 32, "uint", "little")), 4000000001.0);
  QCOMPARE(decodeOne(kMixed, field(11, 0, 8, "int", "little")), -5.0);
  QCOMPARE(decodeOne(kMixed, field(12, 0, 64, "f64", "big")), -2.25);
  QCOMPARE(decodeOne(kMixed, field(20, 0, 64, "uint", "little")), 1099511627776.0);
}

/**
 * @brief One row per unaligned read of the bytes A5 3C.
 */
void FieldLayoutTest::unalignedFields_data()
{
  QTest::addColumn<QJsonObject>("spec");
  QTest::addColumn<double>("expected");

  QTest::newRow("LE 12 bits at bit 4") << field(0, 4, 12, "uint", "little") << 970.0;
  QTest::newRow("BE 12 bits at bit 4") << field(0, 4, 12, "uint", "big") << 1340.0;
  QTest::newRow("LE 7 bits at bit 1") << field(0, 1, 7, "uint", "little") << 82.0;
  QTest::newRow("BE 7 bits at bit 1") << field(0, 1, 7, "uint", "big") << 37.0;
  QTest::newRow("LE flag bit 0") << field(0, 0, 1, "uint", "little") << 1.0;
  QTest::newRow("LE flag bit 1") << field(0, 1, 1, "uint", "little") << 0.0;
  QTest::newRow("BE flag bit 0") << field(0, 0, 1, "uint", "big") << 1.0;
  QTest::newRow("BE flag bit 1") << field(0, 1, 1, "uint", "big") << 0.0;
  QTest::newRow("LE 16 aligned") << field(0, 0, 16, "uint", "little") << 15525.0;
  QTest::newRow("BE 16 aligned") << field(0, 0, 16, "uint", "big") << 42300.0;
}

/**
 * @brief A little-endian field counts from the low bit and grows upwards; a big-endian one counts
 *        from the high bit and reads downwards.
 */
void FieldLayoutTest::unalignedFields()
{
  QFETCH(QJsonObject, spec);
  QFETCH(double, expected);

  QCOMPARE(decodeOne(QByteArray::fromHex("a53c"), spec), expected);
}

/**
 * @brief One row per signed width the extension must handle.
 */
void FieldLayoutTest::signExtension_data()
{
  QTest::addColumn<QByteArray>("frame");
  QTest::addColumn<QJsonObject>("spec");
  QTest::addColumn<double>("expected");

  const auto ones = QByteArray::fromHex("ffffffffffffffff");
  QTest::newRow("1 bit set") << ones << field(0, 0, 1, "int", "little") << -1.0;
  QTest::newRow("1 bit clear") << QByteArray::fromHex("00") << field(0, 0, 1, "int", "big") << 0.0;
  QTest::newRow("7 bits LE") << QByteArray::fromHex("a5") << field(0, 1, 7, "int", "little")
                             << -46.0;
  QTest::newRow("7 bits BE") << QByteArray::fromHex("a5") << field(0, 1, 7, "int", "big") << 37.0;
  QTest::newRow("31 bits") << ones << field(0, 0, 31, "int", "little") << -1.0;
  QTest::newRow("63 bits") << ones << field(0, 0, 63, "int", "big") << -1.0;
  QTest::newRow("63 bits positive") << QByteArray::fromHex("feffffffffffff7f")
                                    << field(0, 1, 63, "int", "little") << 4611686018427387903.0;
  QTest::newRow("63 bits negative") << QByteArray::fromHex("0000000000000080")
                                    << field(0, 1, 63, "int", "little") << -4611686018427387904.0;
  QTest::newRow("64 bits") << ones << field(0, 0, 64, "int", "little") << -1.0;
}

/**
 * @brief The top bit of a signed field of any width is its sign.
 */
void FieldLayoutTest::signExtension()
{
  QFETCH(QByteArray, frame);
  QFETCH(QJsonObject, spec);
  QFETCH(double, expected);

  QCOMPARE(decodeOne(frame, spec), expected);
}

/**
 * @brief Scaling multiplies first and adds second.
 */
void FieldLayoutTest::scaleAndOffset()
{
  const double tenths = decodeOne(kMixed, field(1, 0, 16, "int", "big", 0.1));
  QVERIFY(std::abs(tenths + 12.3) < 1e-9);
  QCOMPARE(decodeOne(kMixed, field(0, 0, 8, "uint", "little", 2.0, -40.0)), 290.0);
  QCOMPARE(decodeOne(kMixed, field(12, 0, 64, "f64", "big", 4.0, 1.0)), -8.0);
}

/**
 * @brief A field the frame does not reach is NaN in the decode and an empty cell in the row; the
 *        fields before and after it still decode.
 */
void FieldLayoutTest::fieldPastTheEndIsEmpty()
{
  const QJsonArray table = {field(0, 0, 8, "uint", "little"),
                            field(1, 0, 32, "uint", "little"),
                            field(1, 4, 4, "uint", "little"),
                            field(2, 0, 1, "uint", "little")};

  QString error;
  DataModel::FieldLayout layout;
  QVERIFY2(DataModel::parseFieldLayout(table, layout, error), qPrintable(error));

  std::array<double, 4> values{};
  const auto frame = QByteArray::fromHex("a53c");
  QCOMPARE(DataModel::decodeFieldLayout(frame, layout, values.data(), values.size()), 4);
  QCOMPARE(values[0], 165.0);
  QVERIFY(std::isnan(values[1]));
  QCOMPARE(values[2], 3.0);
  QVERIFY(std::isnan(values[3]));
  QCOMPARE(DataModel::decodeFieldLayout(frame, layout, values.data(), 3), -1);

  const auto parser = DataModel::fieldLayoutTemplate().makeParser(
    QJsonObject{
      {QStringLiteral("fields"), table}
  },
    error);
  QVERIFY2(parser, qPrintable(error));

  const auto rows = parser->parseBinary(frame);
  QCOMPARE(rows.size(), 1);
  QCOMPARE(rows.constFirst(),
           QStringList({QStringLiteral("165"), QString(), QStringLiteral("3"), QString()}));
  QVERIFY(parser->parseBinary(QByteArray()).isEmpty());
}

/**
 * @brief A float field ignores a stale length left behind by a type change.
 */
void FieldLayoutTest::floatWidthComesFromTheType()
{
  QCOMPARE(decodeOne(kMixed, field(3, 0, 12, "f32", "little")), 1.2);
  QCOMPARE(decodeOne(kMixed, field(12, 0, 8, "f64", "big")), -2.25);
}

/**
 * @brief One row per table the template must refuse.
 */
void FieldLayoutTest::layoutRefusals_data()
{
  QTest::addColumn<QJsonArray>("table");

  QJsonArray tooMany;
  for (int i = 0; i <= DataModel::kMaxLayoutFields; ++i)
    tooMany.append(field(0, 0, 8, "uint", "little"));

  QTest::newRow("empty") << QJsonArray();
  QTest::newRow("too many") << tooMany;
  QTest::newRow("unknown type") << QJsonArray{field(0, 0, 8, "u8", "little")};
  QTest::newRow("zero length") << QJsonArray{field(0, 0, 0, "uint", "little")};
  QTest::newRow("too long") << QJsonArray{field(0, 0, 65, "uint", "little")};
  QTest::newRow("bit offset") << QJsonArray{field(0, 8, 8, "uint", "little")};
  QTest::newRow("negative offset") << QJsonArray{field(-1, 0, 8, "uint", "little")};
  QTest::newRow("offset past cap") << QJsonArray{field(65536, 0, 8, "uint", "little")};
  QTest::newRow("not an object") << QJsonArray{QStringLiteral("uint")};
}

/**
 * @brief A refused table reports why and builds no parser.
 */
void FieldLayoutTest::layoutRefusals()
{
  QFETCH(QJsonArray, table);

  QString error;
  DataModel::FieldLayout layout;
  QVERIFY(!DataModel::parseFieldLayout(table, layout, error));
  QVERIFY(!error.isEmpty());
  QVERIFY(layout.fields.empty());

  error.clear();
  const auto parser = DataModel::fieldLayoutTemplate().makeParser(
    QJsonObject{
      {QStringLiteral("fields"), table}
  },
    error);
  QVERIFY(!parser);
  QVERIFY(!error.isEmpty());
}

/**
 * @brief The template's defaults build a parser, and a full table emits one row in table order.
 */
void FieldLayoutTest::templateEmitsOneRow()
{
  QString error;
  QVERIFY2(DataModel::fieldLayoutTemplate().makeParser(QJsonObject(), error), qPrintable(error));

  const QJsonArray table = {field(1, 0, 16, "int", "big"),
                            field(3, 0, 32, "f32", "little"),
                            field(0, 0, 1, "uint", "little")};
  const auto parser      = DataModel::fieldLayoutTemplate().makeParser(
    QJsonObject{
      {QStringLiteral("fields"), table}
  },
    error);
  QVERIFY2(parser, qPrintable(error));

  const auto rows = parser->parseBinary(kMixed);
  QCOMPARE(rows.size(), 1);
  QCOMPARE(rows.constFirst(),
           QStringList({QStringLiteral("-123"), QStringLiteral("1.2"), QStringLiteral("1")}));
}

QTEST_APPLESS_MAIN(FieldLayoutTest)

#include "tst_field_layout.moc"
