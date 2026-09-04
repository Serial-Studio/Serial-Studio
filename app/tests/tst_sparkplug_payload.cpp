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

#include <bit>
#include <QTest>

#include "Protocols/Sparkplug/SparkplugPayload.h"

using namespace IO::Drivers::SparkplugB;

inline constexpr int kNoneKind    = static_cast<int>(ValueKind::None);
inline constexpr int kNumericKind = static_cast<int>(ValueKind::Numeric);
inline constexpr int kBooleanKind = static_cast<int>(ValueKind::Boolean);
inline constexpr int kStringKind  = static_cast<int>(ValueKind::String);

/**
 * @brief Datatype code as it travels in Metric field 4.
 */
[[nodiscard]] static constexpr quint32 code(DataType type) noexcept
{
  return static_cast<quint32>(type);
}

/**
 * @brief The unsigned bit pattern protobuf carries for a negative signed value, which is the
 *        sign-extended 64-bit two's complement and therefore always a ten-byte varint.
 */
[[nodiscard]] static constexpr quint64 signedRaw(qint64 value) noexcept
{
  return static_cast<quint64>(value);
}

/**
 * @brief Appends a base-128 varint, bounded at the same ten bytes the reader accepts.
 */
static void appendVarint(QByteArray& out, quint64 value)
{
  quint64 remaining = value;
  for (int i = 0; i < kMaxVarintBytes; ++i) {
    const auto byte   = static_cast<quint8>(remaining & 0x7Fu);
    remaining       >>= 7;
    if (remaining == 0) {
      out.append(static_cast<char>(byte));
      return;
    }

    out.append(static_cast<char>(byte | 0x80u));
  }
}

/**
 * @brief Appends a protobuf tag: the field number shifted left by three, wire type in the low bits.
 */
static void appendTag(QByteArray& out, quint32 field, quint32 wire)
{
  appendVarint(out, (static_cast<quint64>(field) << 3) | wire);
}

/**
 * @brief Appends a length-delimited field: tag, declared byte count, then the block.
 */
static void appendLenDelim(QByteArray& out, quint32 field, QByteArrayView block)
{
  appendTag(out, field, kWireLengthDelimited);
  appendVarint(out, static_cast<quint64>(block.size()));
  if (!block.isEmpty())
    out.append(block.data(), block.size());
}

/**
 * @brief Appends a varint-encoded field.
 */
static void appendVarintField(QByteArray& out, quint32 field, quint64 value)
{
  appendTag(out, field, kWireVarint);
  appendVarint(out, value);
}

/**
 * @brief Appends a little-endian fixed-width field of 4 or 8 bytes.
 */
static void appendFixed(QByteArray& out, quint32 field, quint64 bits, int width)
{
  appendTag(out, field, width == 4 ? kWireFixed32 : kWireFixed64);
  for (int i = 0; i < width; ++i)
    out.append(static_cast<char>((bits >> (8 * i)) & 0xFFu));
}

/**
 * @brief One varint-encoded value field on its own, for the datatype table below.
 */
[[nodiscard]] static QByteArray varintValue(quint32 field, quint64 value)
{
  QByteArray out;
  appendVarintField(out, field, value);
  return out;
}

/**
 * @brief One fixed-width value field on its own.
 */
[[nodiscard]] static QByteArray fixedValue(quint32 field, quint64 bits, int width)
{
  QByteArray out;
  appendFixed(out, field, bits, width);
  return out;
}

/**
 * @brief The length-delimited string value field.
 */
[[nodiscard]] static QByteArray textValue(const QByteArray& text)
{
  QByteArray out;
  appendLenDelim(out, kMetricStringValue, text);
  return out;
}

/**
 * @brief Builds a metric block: optional name, the datatype, then an encoded value field.
 */
[[nodiscard]] static QByteArray metricOf(const QByteArray& name,
                                         quint32 datatype,
                                         const QByteArray& value)
{
  QByteArray out;
  if (!name.isEmpty())
    appendLenDelim(out, kMetricName, name);

  appendVarintField(out, kMetricDatatype, datatype);
  out.append(value);
  return out;
}

/**
 * @brief Wraps metric blocks into a payload, each as one repeated metrics entry.
 */
[[nodiscard]] static QByteArray payloadOf(const QList<QByteArray>& metrics)
{
  QByteArray out;
  for (const auto& metric : metrics)
    appendLenDelim(out, kPayloadMetrics, metric);

  return out;
}

/**
 * @brief A payload exercising every reader branch: both header scalars, a named numeric metric
 *        with a fixed64 value, an alias-only string metric, and a skipped uuid field.
 */
[[nodiscard]] static QByteArray richPayload()
{
  QByteArray numeric;
  appendLenDelim(numeric, kMetricName, QByteArray("temperature"));
  appendVarintField(numeric, kMetricTimestamp, 1700000000001ULL);
  appendVarintField(numeric, kMetricDatatype, code(DataType::Double));
  appendFixed(numeric, kMetricDoubleValue, std::bit_cast<quint64>(21.5), 8);

  QByteArray text;
  appendVarintField(text, kMetricAlias, 9);
  appendVarintField(text, kMetricDatatype, code(DataType::String));
  appendLenDelim(text, kMetricStringValue, QByteArray("RUNNING"));

  QByteArray out;
  appendVarintField(out, kPayloadTimestamp, 1700000000000ULL);
  appendVarintField(out, kPayloadSeq, 3);
  appendLenDelim(out, kPayloadMetrics, numeric);
  appendLenDelim(out, kPayloadMetrics, text);
  appendLenDelim(out, kPayloadUuid, QByteArray("2f1c"));
  return out;
}

/**
 * @brief Sparkplug B payload codec: golden protobuf fixtures for the datatype vocabulary, the
 *        deferred value oneof, the fixed caps, and the hostile-input contract (spec 0073).
 */
class TstSparkplugPayload : public QObject {
  Q_OBJECT

private slots:
  void scalarDataTypes_data();
  void scalarDataTypes();
  void aliasOnlyMetric();
  void datatypeAfterValue();
  void nullMetric();
  void unsupportedDataTypes_data();
  void unsupportedDataTypes();
  void unknownFieldsSkipped();
  void payloadHeaderFlags();
  void hostileInput_data();
  void hostileInput();
  void everyPrefixDecodesCleanly();
  void capsRejectOversizedInput();
  void rebirthRequestRoundTrip();
};

void TstSparkplugPayload::scalarDataTypes_data()
{
  QTest::addColumn<quint32>("datatype");
  QTest::addColumn<QByteArray>("value");
  QTest::addColumn<int>("kind");
  QTest::addColumn<double>("numeric");
  QTest::addColumn<bool>("boolean");
  QTest::addColumn<QString>("text");

  QTest::newRow("int8") << code(DataType::Int8) << varintValue(kMetricIntValue, signedRaw(-5))
                        << kNumericKind << -5.0 << false << QString();
  QTest::newRow("int16") << code(DataType::Int16) << varintValue(kMetricIntValue, signedRaw(-300))
                         << kNumericKind << -300.0 << false << QString();
  QTest::newRow("int32") << code(DataType::Int32) << varintValue(kMetricIntValue, signedRaw(-70000))
                         << kNumericKind << -70000.0 << false << QString();
  QTest::newRow("int64") << code(DataType::Int64)
                         << varintValue(kMetricLongValue, signedRaw(-4000000000LL)) << kNumericKind
                         << -4000000000.0 << false << QString();
  QTest::newRow("uint8") << code(DataType::UInt8) << varintValue(kMetricIntValue, 200)
                         << kNumericKind << 200.0 << false << QString();
  QTest::newRow("uint16") << code(DataType::UInt16) << varintValue(kMetricIntValue, 65535)
                          << kNumericKind << 65535.0 << false << QString();
  QTest::newRow("uint32") << code(DataType::UInt32) << varintValue(kMetricIntValue, 4000000000ULL)
                          << kNumericKind << 4000000000.0 << false << QString();
  QTest::newRow("uint64") << code(DataType::UInt64) << varintValue(kMetricLongValue, 5000000000ULL)
                          << kNumericKind << 5000000000.0 << false << QString();
  QTest::newRow("float") << code(DataType::Float)
                         << fixedValue(kMetricFloatValue, std::bit_cast<quint32>(1.5f), 4)
                         << kNumericKind << 1.5 << false << QString();
  QTest::newRow("double") << code(DataType::Double)
                          << fixedValue(kMetricDoubleValue, std::bit_cast<quint64>(3.25), 8)
                          << kNumericKind << 3.25 << false << QString();
  QTest::newRow("boolean") << code(DataType::Boolean) << varintValue(kMetricBooleanValue, 1)
                           << kBooleanKind << 0.0 << true << QString();
  QTest::newRow("string") << code(DataType::String) << textValue(QByteArray("hello")) << kStringKind
                          << 0.0 << false << QStringLiteral("hello");
  QTest::newRow("text") << code(DataType::Text) << textValue(QByteArray("free form")) << kStringKind
                        << 0.0 << false << QStringLiteral("free form");
  QTest::newRow("uuid") << code(DataType::Uuid) << textValue(QByteArray("6f9619ff-8b86-d011"))
                        << kStringKind << 0.0 << false << QStringLiteral("6f9619ff-8b86-d011");
}

/**
 * @brief Every rendered datatype decodes to its channel and value; the signed codes arrive as
 *        ten-byte two's-complement varints and come back negative.
 */
void TstSparkplugPayload::scalarDataTypes()
{
  QFETCH(quint32, datatype);
  QFETCH(QByteArray, value);
  QFETCH(int, kind);
  QFETCH(double, numeric);
  QFETCH(bool, boolean);
  QFETCH(QString, text);

  Payload payload;
  QString error;
  const auto wire = payloadOf({metricOf(QByteArray("m"), datatype, value)});
  QVERIFY2(decodePayload(wire, payload, &error), qPrintable(error));
  QVERIFY(error.isEmpty());
  QCOMPARE(payload.metrics.size(), qsizetype(1));

  const auto& metric = payload.metrics.constFirst();
  QVERIFY(metric.supported);
  QVERIFY(metric.hasName);
  QVERIFY(!metric.hasAlias);
  QVERIFY(!metric.isNull);
  QCOMPARE(metric.name, QStringLiteral("m"));
  QCOMPARE(metric.datatype, datatype);
  QCOMPARE(static_cast<int>(metric.kind), kind);
  QCOMPARE(metric.numericValue, numeric);
  QCOMPARE(metric.boolValue, boolean);
  QCOMPARE(metric.stringValue, text);
}

/**
 * @brief A data message carries the alias alone, so the name presence flag stays down.
 */
void TstSparkplugPayload::aliasOnlyMetric()
{
  QByteArray metric;
  appendVarintField(metric, kMetricAlias, 42);
  appendVarintField(metric, kMetricDatatype, code(DataType::Int32));
  appendVarintField(metric, kMetricIntValue, 7);

  Payload payload;
  QString error;
  QVERIFY2(decodePayload(payloadOf({metric}), payload, &error), qPrintable(error));
  QCOMPARE(payload.metrics.size(), qsizetype(1));

  const auto& decoded = payload.metrics.constFirst();
  QVERIFY(decoded.hasAlias);
  QVERIFY(!decoded.hasName);
  QVERIFY(decoded.name.isEmpty());
  QCOMPARE(decoded.alias, quint64(42));
  QCOMPARE(decoded.numericValue, 7.0);
}

/**
 * @brief The datatype field is free to follow the value field on the wire, so the oneof is only
 *        resolved once the metric is fully walked.
 */
void TstSparkplugPayload::datatypeAfterValue()
{
  QByteArray narrow;
  appendLenDelim(narrow, kMetricName, QByteArray("late"));
  appendVarintField(narrow, kMetricIntValue, signedRaw(-300));
  appendVarintField(narrow, kMetricDatatype, code(DataType::Int16));

  QByteArray real;
  appendFixed(real, kMetricDoubleValue, std::bit_cast<quint64>(-2.75), 8);
  appendVarintField(real, kMetricDatatype, code(DataType::Double));

  Payload payload;
  QString error;
  QVERIFY2(decodePayload(payloadOf({narrow, real}), payload, &error), qPrintable(error));
  QCOMPARE(payload.metrics.size(), qsizetype(2));
  QCOMPARE(static_cast<int>(payload.metrics.at(0).kind), kNumericKind);
  QCOMPARE(payload.metrics.at(0).numericValue, -300.0);
  QCOMPARE(payload.metrics.at(1).numericValue, -2.75);
}

/**
 * @brief A null metric keeps its datatype and support flag but never renders the value it carries.
 */
void TstSparkplugPayload::nullMetric()
{
  QByteArray metric;
  appendLenDelim(metric, kMetricName, QByteArray("offline"));
  appendVarintField(metric, kMetricDatatype, code(DataType::Double));
  appendVarintField(metric, kMetricIsNull, 1);
  appendFixed(metric, kMetricDoubleValue, std::bit_cast<quint64>(9.5), 8);

  Payload payload;
  QString error;
  QVERIFY2(decodePayload(payloadOf({metric}), payload, &error), qPrintable(error));
  QCOMPARE(payload.metrics.size(), qsizetype(1));

  const auto& decoded = payload.metrics.constFirst();
  QVERIFY(decoded.isNull);
  QVERIFY(decoded.supported);
  QCOMPARE(static_cast<int>(decoded.kind), kNumericKind);
  QCOMPARE(decoded.numericValue, 0.0);
}

void TstSparkplugPayload::unsupportedDataTypes_data()
{
  QTest::addColumn<quint32>("datatype");
  QTest::addColumn<QByteArray>("value");

  QTest::newRow("datetime") << code(DataType::DateTime)
                            << varintValue(kMetricLongValue, 1700000000000ULL);
  QTest::newRow("code-16") << quint32(16) << varintValue(kMetricLongValue, 1);
  QTest::newRow("code-17") << quint32(17) << varintValue(kMetricIntValue, 1);
  QTest::newRow("unknown") << code(DataType::Unknown) << varintValue(kMetricIntValue, 1);
}

/**
 * @brief A datatype this codec does not render leaves the metric present but unsupported, which is
 *        what lets the session layer count it instead of dropping the whole payload.
 */
void TstSparkplugPayload::unsupportedDataTypes()
{
  QFETCH(quint32, datatype);
  QFETCH(QByteArray, value);

  Payload payload;
  QString error;
  const auto wire = payloadOf({metricOf(QByteArray("odd"), datatype, value)});
  QVERIFY2(decodePayload(wire, payload, &error), qPrintable(error));
  QCOMPARE(payload.metrics.size(), qsizetype(1));

  const auto& metric = payload.metrics.constFirst();
  QVERIFY(!metric.supported);
  QVERIFY(metric.hasName);
  QCOMPARE(metric.datatype, datatype);
  QCOMPARE(static_cast<int>(metric.kind), kNoneKind);
  QCOMPARE(metric.numericValue, 0.0);
  QCOMPARE(metric.boolValue, false);
}

/**
 * @brief Field numbers neither level models are skipped by wire type, in both the payload and the
 *        metric, so a later schema revision still reads.
 */
void TstSparkplugPayload::unknownFieldsSkipped()
{
  QByteArray metric;
  appendLenDelim(metric, 5, QByteArray("metadata"));
  appendVarintField(metric, 8, 12345);
  appendFixed(metric, 16, 0x1122334455667788ULL, 8);
  appendLenDelim(metric, kMetricName, QByteArray("kept"));
  appendVarintField(metric, kMetricDatatype, code(DataType::UInt16));
  appendVarintField(metric, kMetricIntValue, 4321);
  appendFixed(metric, 20, 0x11223344ULL, 4);

  QByteArray wire;
  appendVarintField(wire, 42, 99);
  appendLenDelim(wire, kPayloadBody, QByteArray("body"));
  appendLenDelim(wire, kPayloadMetrics, metric);
  appendFixed(wire, 43, 0x55667788ULL, 4);
  appendFixed(wire, 44, 0x1122334455667788ULL, 8);
  appendLenDelim(wire, kPayloadUuid, QByteArray("2f1c"));

  Payload payload;
  QString error;
  QVERIFY2(decodePayload(wire, payload, &error), qPrintable(error));
  QCOMPARE(payload.metrics.size(), qsizetype(1));
  QCOMPARE(payload.metrics.constFirst().name, QStringLiteral("kept"));
  QCOMPARE(payload.metrics.constFirst().numericValue, 4321.0);
}

/**
 * @brief The payload timestamp and sequence number each report their own presence.
 */
void TstSparkplugPayload::payloadHeaderFlags()
{
  Payload payload;
  QString error;
  QVERIFY2(decodePayload(richPayload(), payload, &error), qPrintable(error));
  QVERIFY(payload.hasTimestamp);
  QVERIFY(payload.hasSeq);
  QCOMPARE(payload.timestampMs, quint64(1700000000000ULL));
  QCOMPARE(payload.seq, quint64(3));
  QCOMPARE(payload.metrics.size(), qsizetype(2));
  QCOMPARE(payload.metrics.at(0).timestampMs, quint64(1700000000001ULL));
  QCOMPARE(payload.metrics.at(0).numericValue, 21.5);
  QCOMPARE(payload.metrics.at(1).stringValue, QStringLiteral("RUNNING"));

  QByteArray bare;
  appendLenDelim(
    bare,
    kPayloadMetrics,
    metricOf(QByteArray("n"), code(DataType::Boolean), varintValue(kMetricBooleanValue, 0)));
  QVERIFY2(decodePayload(bare, payload, &error), qPrintable(error));
  QVERIFY(!payload.hasTimestamp);
  QVERIFY(!payload.hasSeq);
  QCOMPARE(payload.timestampMs, quint64(0));
  QCOMPARE(payload.seq, quint64(0));
}

void TstSparkplugPayload::hostileInput_data()
{
  QTest::addColumn<QByteArray>("data");

  QByteArray truncatedVarint;
  appendTag(truncatedVarint, kPayloadTimestamp, kWireVarint);
  truncatedVarint.append("\xFF\xFF", 2);
  QTest::newRow("truncated varint") << truncatedVarint;

  QByteArray truncatedBlock;
  appendTag(truncatedBlock, kPayloadMetrics, kWireLengthDelimited);
  appendVarint(truncatedBlock, 10);
  truncatedBlock.append("abc", 3);
  QTest::newRow("truncated block") << truncatedBlock;

  QByteArray lyingLength;
  appendTag(lyingLength, kPayloadMetrics, kWireLengthDelimited);
  appendVarint(lyingLength, static_cast<quint64>(kMaxPayloadBytes) - 1);
  QTest::newRow("length past buffer") << lyingLength;

  QByteArray hugeLength;
  appendTag(hugeLength, kPayloadMetrics, kWireLengthDelimited);
  appendVarint(hugeLength, static_cast<quint64>(kMaxPayloadBytes) + 1);
  QTest::newRow("length past cap") << hugeLength;

  QTest::newRow("continuation run") << QByteArray(kMaxVarintBytes + 1, static_cast<char>(0xFF));
  QTest::newRow("field number zero") << QByteArray(1, static_cast<char>(0x00));

  QByteArray wideTag;
  appendVarint(wideTag, (kMaxFieldNumber + 1) << 3);
  QTest::newRow("field number too wide") << wideTag;

  QByteArray groupStart;
  appendTag(groupStart, kPayloadUuid, 3);
  QTest::newRow("payload group start") << groupStart;

  QByteArray groupEnd;
  appendTag(groupEnd, kPayloadUuid, 4);
  QTest::newRow("payload group end") << groupEnd;

  QByteArray metricGroup;
  appendTag(metricGroup, 18, 3);
  QTest::newRow("metric group start") << payloadOf({metricGroup});

  QByteArray metricString;
  appendTag(metricString, kMetricName, kWireLengthDelimited);
  appendVarint(metricString, 20);
  metricString.append("xy", 2);
  QTest::newRow("metric string past block") << payloadOf({metricString});

  QByteArray wrongWire;
  appendFixed(wrongWire, kPayloadTimestamp, 1, 4);
  QTest::newRow("timestamp wrong wire type") << wrongWire;

  QByteArray metricsAsVarint;
  appendVarintField(metricsAsVarint, kPayloadMetrics, 5);
  QTest::newRow("metrics wrong wire type") << metricsAsVarint;

  QByteArray shortFixed;
  appendTag(shortFixed, kPayloadUuid, kWireFixed64);
  shortFixed.append("abc", 3);
  QTest::newRow("truncated fixed64") << shortFixed;
}

/**
 * @brief Broker traffic is hostile input: every malformed shape fails with a reason instead of
 *        over-reading, and none of them may reach the caller as a partial success.
 */
void TstSparkplugPayload::hostileInput()
{
  QFETCH(QByteArray, data);

  Payload payload;
  QString error;
  QVERIFY(!decodePayload(data, payload, &error));
  QVERIFY(!error.isEmpty());
  QVERIFY(!decodePayload(data, payload, nullptr));
}

/**
 * @brief Every prefix of a valid payload is a truncation the reader must survive: each one either
 *        decodes cleanly or fails with a reason, and the error string tracks the verdict.
 */
void TstSparkplugPayload::everyPrefixDecodesCleanly()
{
  const QByteArray full = richPayload();
  QVERIFY(full.size() > 32);

  for (qsizetype i = 0; i <= full.size(); ++i) {
    Payload payload;
    QString error = QStringLiteral("stale");
    const bool ok = decodePayload(QByteArrayView(full.constData(), i), payload, &error);
    QVERIFY2(ok == error.isEmpty(), qPrintable(QString::number(i)));
    QVERIFY2(payload.metrics.size() <= qsizetype(2), qPrintable(QString::number(i)));
  }

  Payload payload;
  QVERIFY(decodePayload(QByteArrayView(full.constData(), full.size()), payload, nullptr));
  QCOMPARE(payload.metrics.size(), qsizetype(2));
}

/**
 * @brief The three fixed caps fail the decode rather than allocating on demand, and the metric cap
 *        still admits a payload sitting exactly on it.
 */
void TstSparkplugPayload::capsRejectOversizedInput()
{
  Payload payload;
  QString error;
  QVERIFY(!decodePayload(QByteArray(kMaxPayloadBytes + 1, '\0'), payload, &error));
  QVERIFY(error.contains(QString::number(kMaxPayloadBytes)));

  QByteArray tooMany;
  for (int i = 0; i <= kMaxMetrics; ++i)
    appendLenDelim(tooMany, kPayloadMetrics, QByteArray());

  QVERIFY(!decodePayload(tooMany, payload, &error));
  QVERIFY(error.contains(QString::number(kMaxMetrics)));

  QByteArray longString;
  appendVarintField(longString, kMetricDatatype, code(DataType::String));
  appendLenDelim(longString, kMetricStringValue, QByteArray(kMaxStringBytes + 1, 'x'));
  QVERIFY(!decodePayload(payloadOf({longString}), payload, &error));
  QVERIFY(!error.isEmpty());

  QByteArray atCap;
  for (int i = 0; i < kMaxMetrics; ++i)
    appendLenDelim(atCap, kPayloadMetrics, QByteArray());

  QVERIFY2(decodePayload(atCap, payload, &error), qPrintable(error));
  QCOMPARE(payload.metrics.size(), qsizetype(kMaxMetrics));
}

/**
 * @brief The NCMD rebirth request decodes back through the reader: one boolean metric named
 *        "Node Control/Rebirth" set true, a payload timestamp, and deliberately no sequence number.
 */
void TstSparkplugPayload::rebirthRequestRoundTrip()
{
  const QByteArray encoded = encodeRebirthRequest(1700000000000ULL);
  QVERIFY(!encoded.isEmpty());

  Payload payload;
  QString error;
  QVERIFY2(decodePayload(encoded, payload, &error), qPrintable(error));
  QVERIFY(payload.hasTimestamp);
  QVERIFY(!payload.hasSeq);
  QCOMPARE(payload.timestampMs, quint64(1700000000000ULL));
  QCOMPARE(payload.metrics.size(), qsizetype(1));

  const auto& metric = payload.metrics.constFirst();
  QVERIFY(metric.hasName);
  QVERIFY(metric.supported);
  QVERIFY(!metric.isNull);
  QVERIFY(metric.boolValue);
  QCOMPARE(metric.name, QString::fromUtf8(kRebirthMetricName));
  QCOMPARE(metric.datatype, code(DataType::Boolean));
  QCOMPARE(static_cast<int>(metric.kind), kBooleanKind);
}

QTEST_APPLESS_MAIN(TstSparkplugPayload)

#include "tst_sparkplug_payload.moc"
