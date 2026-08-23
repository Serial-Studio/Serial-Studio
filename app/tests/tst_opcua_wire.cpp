/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include <QJsonArray>
#include <QJsonObject>
#include <QTest>

#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "IO/Drivers/OpcUaWire.h"

using namespace IO::Drivers::OpcUaWire;

/**
 * @brief Returns the first row of a parse result, or an empty list.
 */
[[nodiscard]] static QStringList firstRow(const QList<QStringList>& frames)
{
  return frames.isEmpty() ? QStringList() : frames.constFirst();
}

/**
 * @brief Builds a schema params object from parallel type codes (index = position).
 */
[[nodiscard]] static QJsonObject schemaParams(const QStringList& codes)
{
  QJsonArray schema;
  for (int i = 0; i < codes.size(); ++i)
    schema.append(QJsonObject{
      {QStringLiteral("i"),        i},
      {QStringLiteral("t"), codes[i]}
    });

  return QJsonObject{
    {QStringLiteral("schema"), schema}
  };
}

/**
 * @brief Encodes one frame holding the given entries.
 */
[[nodiscard]] static QByteArray frameOf(const QList<std::tuple<int, Type, QVariant>>& entries)
{
  QByteArray out;
  out.reserve(kHeaderBytes + entries.size() * maxEntryBytes(Type::Str));
  beginFrame(out);
  for (const auto& [index, type, value] : entries)
    appendEntry(out, index, type, value);

  return out;
}

/**
 * @brief OPC UA delta-frame wire vocabulary and the opcua native latch template (spec 0066).
 */
class TstOpcUaWire : public QObject {
  Q_OBJECT

private slots:
  void typeCodesRoundTrip();
  void scalarsRoundTrip_data();
  void scalarsRoundTrip();
  void stringsTruncateAtCap();
  void truncatedEntryStopsWalk();
  void badVersionRejected();
  void templateLatchesDeltas();
  void templateIgnoresUnknownIndexAndTypeMismatch();
  void templateRejectsBadSchema();
};

/**
 * @brief Every code maps to a distinct type and back.
 */
void TstOpcUaWire::typeCodesRoundTrip()
{
  const QStringList codes = {
    "bool", "i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "f32", "f64", "str"};
  for (const auto& code : codes) {
    const Type type = typeFromCode(code);
    QVERIFY(type != Type::Invalid);
    QCOMPARE(codeFromType(type), code);
  }

  QCOMPARE(typeFromCode(QStringLiteral("nope")), Type::Invalid);
  QVERIFY(codeFromType(Type::Invalid).isEmpty());
}

void TstOpcUaWire::scalarsRoundTrip_data()
{
  QTest::addColumn<int>("type");
  QTest::addColumn<QVariant>("value");
  QTest::addColumn<QString>("text");

  QTest::newRow("bool") << int(Type::Bool) << QVariant(true) << QStringLiteral("1");
  QTest::newRow("i8") << int(Type::I8) << QVariant(-100) << QStringLiteral("-100");
  QTest::newRow("u8") << int(Type::U8) << QVariant(200) << QStringLiteral("200");
  QTest::newRow("i16") << int(Type::I16) << QVariant(-32000) << QStringLiteral("-32000");
  QTest::newRow("u16") << int(Type::U16) << QVariant(65535) << QStringLiteral("65535");
  QTest::newRow("i32") << int(Type::I32) << QVariant(-2000000000) << QStringLiteral("-2000000000");
  QTest::newRow("u32") << int(Type::U32) << QVariant(4000000000u) << QStringLiteral("4000000000");
  QTest::newRow("i64") << int(Type::I64) << QVariant(qint64(-9000000000000000000LL))
                       << QStringLiteral("-9000000000000000000");
  QTest::newRow("u64") << int(Type::U64) << QVariant(quint64(18000000000000000000ULL))
                       << QStringLiteral("18000000000000000000");
  QTest::newRow("f32") << int(Type::F32) << QVariant(1.5f) << QStringLiteral("1.5");
  QTest::newRow("f64") << int(Type::F64) << QVariant(3.25) << QStringLiteral("3.25");
}

/**
 * @brief Each scalar type encodes to its fixed width and decodes to the expected text.
 */
void TstOpcUaWire::scalarsRoundTrip()
{
  QFETCH(int, type);
  QFETCH(QVariant, value);
  QFETCH(QString, text);

  const auto wire  = static_cast<Type>(type);
  const auto frame = frameOf({
    {7, wire, value}
  });
  QCOMPARE(frame.size(), kHeaderBytes + kEntryHeaderBytes + payloadWidth(wire));

  qsizetype pos = kHeaderBytes;
  Entry entry;
  QVERIFY(readEntry(frame, pos, entry));
  QCOMPARE(entry.index, 7);
  QCOMPARE(entry.type, wire);
  QCOMPARE(entry.text, text);
  QCOMPARE(pos, frame.size());
}

/**
 * @brief Strings longer than the cap are truncated on the wire and decode to the prefix.
 */
void TstOpcUaWire::stringsTruncateAtCap()
{
  const QString text(kMaxStringBytes + 50, QLatin1Char('x'));
  const auto frame = frameOf({
    {3, Type::Str, text}
  });
  QCOMPARE(frame.size(), kHeaderBytes + kEntryHeaderBytes + 2 + kMaxStringBytes);

  qsizetype pos = kHeaderBytes;
  Entry entry;
  QVERIFY(readEntry(frame, pos, entry));
  QCOMPARE(entry.text.size(), kMaxStringBytes);
  QCOMPARE(entry.type, Type::Str);
}

/**
 * @brief A frame cut inside a payload decodes every complete entry and stops.
 */
void TstOpcUaWire::truncatedEntryStopsWalk()
{
  auto frame = frameOf({
    {0, Type::U16,   1},
    {1, Type::F64, 2.0}
  });
  frame.chop(3);

  qsizetype pos = kHeaderBytes;
  Entry entry;
  QVERIFY(readEntry(frame, pos, entry));
  QCOMPARE(entry.index, 0);

  const qsizetype before = pos;
  QVERIFY(!readEntry(frame, pos, entry));
  QCOMPARE(pos, before);
}

/**
 * @brief Only the current version byte passes the header check.
 */
void TstOpcUaWire::badVersionRejected()
{
  auto frame = frameOf({
    {0, Type::Bool, true}
  });
  QVERIFY(checkHeader(frame));

  frame[0] = static_cast<char>(kWireVersion + 1);
  QVERIFY(!checkHeader(frame));
  QVERIFY(!checkHeader(QByteArray()));
}

/**
 * @brief The template latches: a delta frame updates only the tags it carries.
 */
void TstOpcUaWire::templateLatchesDeltas()
{
  const auto* tmpl = DataModel::nativeTemplateById(QStringLiteral("opcua"));
  QVERIFY(tmpl != nullptr);

  QString error;
  auto parser = tmpl->makeParser(schemaParams({"f64", "bool", "str"}), error);
  QVERIFY2(parser != nullptr, qPrintable(error));

  auto row = firstRow(parser->parseBinary(frameOf({
    {0,  Type::F64,  21.5},
    {1, Type::Bool,  true},
    {2,  Type::Str, "RUN"}
  })));
  QCOMPARE(row, QStringList({"21.5", "1", "RUN"}));

  row = firstRow(parser->parseBinary(frameOf({
    {1, Type::Bool, false}
  })));
  QCOMPARE(row, QStringList({"21.5", "0", "RUN"}));

  auto bad = frameOf({
    {0, Type::F64, 99.0}
  });
  bad[0]   = static_cast<char>(kWireVersion + 1);
  row      = firstRow(parser->parseBinary(bad));
  QCOMPARE(row, QStringList({"21.5", "0", "RUN"}));
}

/**
 * @brief Entries outside the schema, or with a type that disagrees with it, never touch the latch.
 */
void TstOpcUaWire::templateIgnoresUnknownIndexAndTypeMismatch()
{
  const auto* tmpl = DataModel::nativeTemplateById(QStringLiteral("opcua"));
  QVERIFY(tmpl != nullptr);

  QString error;
  auto parser = tmpl->makeParser(schemaParams({"i32", "i32"}), error);
  QVERIFY2(parser != nullptr, qPrintable(error));

  auto row = firstRow(parser->parseBinary(frameOf({
    { 0, Type::I32,    5},
    { 1, Type::F32, 1.0f},
    {40, Type::I32,    9}
  })));
  QCOMPARE(row, QStringList({"5", "0"}));
}

/**
 * @brief makeParser() rejects an empty schema, a bad index and an unknown type code.
 */
void TstOpcUaWire::templateRejectsBadSchema()
{
  const auto* tmpl = DataModel::nativeTemplateById(QStringLiteral("opcua"));
  QVERIFY(tmpl != nullptr);

  QString error;
  QVERIFY(tmpl->makeParser(QJsonObject(), error) == nullptr);
  QVERIFY(!error.isEmpty());

  QVERIFY(tmpl->makeParser(schemaParams({"f64", "nope"}), error) == nullptr);

  QJsonArray schema{
    QJsonObject{{QStringLiteral("i"), 5}, {QStringLiteral("t"), "f64"}}
  };
  QVERIFY(tmpl->makeParser(
            QJsonObject{
              {QStringLiteral("schema"), schema}
  },
            error)
          == nullptr);
}

QTEST_APPLESS_MAIN(TstOpcUaWire)

#include "tst_opcua_wire.moc"
