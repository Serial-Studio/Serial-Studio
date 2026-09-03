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
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>
#include <QTest>

#include "DataModel/FrameKeys.h"
#include "IO/Drivers/MQTT/SparkplugSession.h"

using IO::Drivers::SparkplugSession;

namespace Limits    = IO::Drivers::SparkplugLimits;
namespace Sparkplug = IO::Drivers::SparkplugB;
namespace Wire      = IO::Drivers::OpcUaWire;

using Sparkplug::DataType;

/**
 * @brief Datatype code as it travels in Metric field 4.
 */
[[nodiscard]] static constexpr quint32 code(DataType type) noexcept
{
  return static_cast<quint32>(type);
}

/**
 * @brief Appends a base-128 varint, bounded at the ten bytes the reader accepts.
 */
static void appendVarint(QByteArray& out, quint64 value)
{
  quint64 remaining = value;
  for (int i = 0; i < Sparkplug::kMaxVarintBytes; ++i) {
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
  appendTag(out, field, Sparkplug::kWireLengthDelimited);
  appendVarint(out, static_cast<quint64>(block.size()));
  if (!block.isEmpty())
    out.append(block.data(), block.size());
}

/**
 * @brief Appends a varint-encoded field.
 */
static void appendVarintField(QByteArray& out, quint32 field, quint64 value)
{
  appendTag(out, field, Sparkplug::kWireVarint);
  appendVarint(out, value);
}

/**
 * @brief The fixed64 double value field.
 */
[[nodiscard]] static QByteArray realValue(double value)
{
  QByteArray out;
  appendTag(out, Sparkplug::kMetricDoubleValue, Sparkplug::kWireFixed64);
  const auto bits = std::bit_cast<quint64>(value);
  for (int i = 0; i < 8; ++i)
    out.append(static_cast<char>((bits >> (8 * i)) & 0xFFu));

  return out;
}

/**
 * @brief The varint boolean value field.
 */
[[nodiscard]] static QByteArray flagValue(bool value)
{
  QByteArray out;
  appendVarintField(out, Sparkplug::kMetricBooleanValue, value ? 1 : 0);
  return out;
}

/**
 * @brief The varint long value field, used here to give an unsupported datatype a payload.
 */
[[nodiscard]] static QByteArray longValue(quint64 value)
{
  QByteArray out;
  appendVarintField(out, Sparkplug::kMetricLongValue, value);
  return out;
}

/**
 * @brief The length-delimited string value field.
 */
[[nodiscard]] static QByteArray textValue(const QByteArray& text)
{
  QByteArray out;
  appendLenDelim(out, Sparkplug::kMetricStringValue, text);
  return out;
}

/**
 * @brief Builds one metric block. An empty @p name omits the name field and a negative @p alias
 *        omits the alias field, which is how a birth metric, a data metric and an identity-less
 *        metric are all spelled through the same builder.
 */
[[nodiscard]] static QByteArray makeMetric(const QByteArray& name,
                                           qint64 alias,
                                           quint32 datatype,
                                           const QByteArray& value)
{
  QByteArray out;
  if (!name.isEmpty())
    appendLenDelim(out, Sparkplug::kMetricName, name);

  if (alias >= 0)
    appendVarintField(out, Sparkplug::kMetricAlias, static_cast<quint64>(alias));

  appendVarintField(out, Sparkplug::kMetricDatatype, datatype);
  out.append(value);
  return out;
}

/**
 * @brief Wraps metric blocks into one payload; a negative @p seq omits the sequence number, which
 *        is what makes a fixture opt out of the per-node sequence check.
 */
[[nodiscard]] static QByteArray makePayload(qint64 seq,
                                            quint64 timestampMs,
                                            const QList<QByteArray>& metrics)
{
  QByteArray out;
  appendVarintField(out, Sparkplug::kPayloadTimestamp, timestampMs);
  if (seq >= 0)
    appendVarintField(out, Sparkplug::kPayloadSeq, static_cast<quint64>(seq));

  for (const auto& metric : metrics)
    appendLenDelim(out, Sparkplug::kPayloadMetrics, metric);

  return out;
}

/**
 * @brief Builds a node-scoped Sparkplug topic: "spBv1.0/<group>/<verb>/<edge>".
 */
[[nodiscard]] static QString topicOf(const char* group, const char* verb, const char* node)
{
  return QStringLiteral("%1/%2/%3/%4")
    .arg(QString::fromLatin1(Limits::kNamespace),
         QString::fromLatin1(group),
         QString::fromLatin1(verb),
         QString::fromLatin1(node));
}

/**
 * @brief Builds a device-scoped Sparkplug topic: "spBv1.0/<group>/<verb>/<edge>/<device>".
 */
[[nodiscard]] static QString deviceTopic(const char* group,
                                         const char* verb,
                                         const char* node,
                                         const char* device)
{
  return topicOf(group, verb, node) + QLatin1Char('/') + QString::fromLatin1(device);
}

/**
 * @brief Slot carrying @p label, or a default slot when no such identity was ever assigned.
 */
[[nodiscard]] static SparkplugSession::SlotValue slotNamed(const SparkplugSession& session,
                                                           const char* label)
{
  const QString wanted = QString::fromLatin1(label);
  for (const auto& slot : session.slotValues())
    if (slot.displayName == wanted)
      return slot;

  return SparkplugSession::SlotValue();
}

/**
 * @brief Drains the changed slots the way the delta encoder does and reports which ones it saw.
 */
[[nodiscard]] static QStringList drainDirty(SparkplugSession& session)
{
  QStringList labels;
  session.consumeDirtySlots(
    [&labels](const SparkplugSession::SlotValue& slot) { labels.append(slot.displayName); });

  return labels;
}

/**
 * @brief Sparkplug B session and birth-certificate state machine: alias resolution, pre-birth
 *        buffering, sequence gaps, death certificates and every fixed cap (spec 0073 R2-R6, R11).
 */
class TstSparkplugSession : public QObject {
  Q_OBJECT

private slots:
  void birthThenDataByAlias();
  void dataBeforeBirthIsBuffered();
  void preBirthBufferOverflow();
  void sequenceGapFlagsRebirth();
  void deathCertificates();
  void unsupportedDataTypeKeepsValue();
  void aliasCollisionAcrossNodes();
  void groupsScopeNodeIdentity();
  void topicRouting();
  void identityCapsRefuseGrowth();
  void malformedPayloadCounted();
  void slotCapRefusesGrowth();
  void schemaGenerationLifecycle();
  void resetKeepsSlotsAndClearsSession();
  void reconnectWithReversedBirthsKeepsIndices();
  void slotTableRoundTripsThroughJson();
  void aMisplacedRestoreEntryIsRefusedWhole();
};

/**
 * @brief The happy path: a birth certificate assigns the slots and the synthetic Online metric,
 *        later data resolves by alias alone, and one drain clears exactly what it encoded.
 */
void TstSparkplugSession::birthThenDataByAlias()
{
  SparkplugSession session;
  const QByteArray birth =
    makePayload(0,
                1000,
                {makeMetric("temperature", 1, code(DataType::Double), realValue(20.0)),
                 makeMetric("state", 2, code(DataType::String), textValue("IDLE"))});

  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), birth));
  QCOMPARE(session.slotCount(), 3);
  QVERIFY(session.hasDirtySlots());
  QCOMPARE(drainDirty(session).size(), 3);
  QVERIFY(!session.hasDirtySlots());
  QCOMPARE(slotNamed(session, "edge1/Online").num, 1.0);
  QVERIFY(slotNamed(session, "edge1/Online").b);

  const QByteArray data =
    makePayload(1,
                2000,
                {makeMetric(QByteArray(), 1, code(DataType::Double), realValue(21.5)),
                 makeMetric(QByteArray(), 2, code(DataType::String), textValue("RUN"))});

  QVERIFY(session.ingest(topicOf("g1", "NDATA", "edge1"), data));
  QVERIFY(session.hasDirtySlots());
  QCOMPARE(slotNamed(session, "edge1/temperature").num, 21.5);
  QCOMPARE(slotNamed(session, "edge1/temperature").timestampMs, quint64(2000));
  QCOMPARE(slotNamed(session, "edge1/state").str, QStringLiteral("RUN"));
  QCOMPARE(drainDirty(session).size(), 2);
  QVERIFY(!session.hasDirtySlots());
  QCOMPARE(session.slotCount(), 3);
  QCOMPARE(session.counters().seqGaps, quint64(0));
  QCOMPARE(session.counters().unsupportedMetrics, quint64(0));
}

/**
 * @brief Data for a scope with no birth certificate is buffered whole and applies nothing; the
 *        birth that explains it flushes the buffer and the buffered values land with their own
 *        timestamps.
 */
void TstSparkplugSession::dataBeforeBirthIsBuffered()
{
  SparkplugSession session;
  const QByteArray early =
    makePayload(0, 2000, {makeMetric(QByteArray(), 3, code(DataType::Double), realValue(7.5))});

  QVERIFY(session.ingest(deviceTopic("g1", "DDATA", "edge1", "dev1"), early));
  QCOMPARE(session.counters().preBirthBuffered, quint64(1));
  QCOMPARE(session.counters().rebirthRequests, quint64(1));
  QCOMPARE(session.slotCount(), 0);
  QVERIFY(!session.hasDirtySlots());

  const QByteArray birth =
    makePayload(1, 3000, {makeMetric("flow", 3, code(DataType::Double), realValue(0.0))});

  QVERIFY(session.ingest(deviceTopic("g1", "DBIRTH", "edge1", "dev1"), birth));
  QCOMPARE(session.slotCount(), 2);
  QCOMPARE(slotNamed(session, "edge1/dev1/flow").num, 7.5);
  QCOMPARE(slotNamed(session, "edge1/dev1/flow").timestampMs, quint64(2000));
  QCOMPARE(session.counters().preBirthDropped, quint64(0));
  QCOMPARE(session.counters().preBirthBuffered, quint64(1));

  const QByteArray later =
    makePayload(2, 4000, {makeMetric(QByteArray(), 3, code(DataType::Double), realValue(9.0))});

  QVERIFY(session.ingest(deviceTopic("g1", "DDATA", "edge1", "dev1"), later));
  QCOMPARE(slotNamed(session, "edge1/dev1/flow").num, 9.0);
  QCOMPARE(session.counters().preBirthBuffered, quint64(1));
  QCOMPARE(session.counters().seqGaps, quint64(0));
}

/**
 * @brief One message past the pre-birth cap drops the oldest and counts it: after the birth that
 *        resolves every alias, the dropped message's slot keeps its birth value while all the
 *        buffered ones carry theirs.
 */
void TstSparkplugSession::preBirthBufferOverflow()
{
  SparkplugSession session;
  const int total = Limits::kMaxPreBirthMessages + 1;
  for (int i = 0; i < total; ++i) {
    const QByteArray data = makePayload(
      -1,
      static_cast<quint64>(1000 + i),
      {makeMetric(QByteArray(), i, code(DataType::Double), realValue(static_cast<double>(i)))});
    QVERIFY(session.ingest(topicOf("g1", "NDATA", "edge1"), data));
  }

  QCOMPARE(session.counters().preBirthBuffered, static_cast<quint64>(total));
  QCOMPARE(session.counters().preBirthDropped, quint64(1));
  QCOMPARE(session.counters().rebirthRequests, quint64(1));
  QCOMPARE(session.slotCount(), 0);

  QList<QByteArray> block;
  block.reserve(total);
  for (int i = 0; i < total; ++i)
    block.append(makeMetric(
      QByteArray("m") + QByteArray::number(i), i, code(DataType::Double), realValue(-1.0)));

  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), makePayload(0, 5000, block)));
  QCOMPARE(session.slotCount(), total + 1);
  QCOMPARE(slotNamed(session, "edge1/m0").num, -1.0);
  QCOMPARE(slotNamed(session, "edge1/m1").num, 1.0);
  QCOMPARE(slotNamed(session, "edge1/m256").num, 256.0);
  QCOMPARE(session.counters().preBirthDropped, quint64(1));
}

/**
 * @brief A skipped sequence number counts a gap, arms one rebirth request per node until that node
 *        births again, and still applies the message it arrived on.
 */
void TstSparkplugSession::sequenceGapFlagsRebirth()
{
  SparkplugSession session;
  const QByteArray birth =
    makePayload(0, 1000, {makeMetric("rpm", 1, code(DataType::Double), realValue(0.0))});
  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), birth));

  const QByteArray gapped =
    makePayload(2, 2000, {makeMetric(QByteArray(), 1, code(DataType::Double), realValue(120.0))});
  QVERIFY(session.ingest(topicOf("g1", "NDATA", "edge1"), gapped));
  QCOMPARE(session.counters().seqGaps, quint64(1));
  QCOMPARE(session.counters().rebirthRequests, quint64(1));
  QCOMPARE(slotNamed(session, "edge1/rpm").num, 120.0);

  const QVector<QString> queued = session.takeRebirthTopics();
  QCOMPARE(queued.size(), qsizetype(1));
  QCOMPARE(queued.constFirst(), topicOf("g1", "NCMD", "edge1"));
  QVERIFY(session.takeRebirthTopics().isEmpty());

  const QByteArray again =
    makePayload(5, 3000, {makeMetric(QByteArray(), 1, code(DataType::Double), realValue(130.0))});
  QVERIFY(session.ingest(topicOf("g1", "NDATA", "edge1"), again));
  QCOMPARE(session.counters().seqGaps, quint64(2));
  QCOMPARE(session.counters().rebirthRequests, quint64(1));
  QVERIFY(session.takeRebirthTopics().isEmpty());
  QCOMPARE(slotNamed(session, "edge1/rpm").num, 130.0);

  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), makePayload(0, 4000, {})));
  const QByteArray reborn =
    makePayload(4, 5000, {makeMetric("rpm", 1, code(DataType::Double), realValue(140.0))});
  QVERIFY(session.ingest(topicOf("g1", "NDATA", "edge1"), reborn));
  QCOMPARE(session.counters().rebirthRequests, quint64(2));
  QCOMPARE(session.takeRebirthTopics().size(), qsizetype(1));
}

/**
 * @brief A node death zeroes the synthetic Online metric and sends later traffic back to the
 *        pre-birth buffer; a device death marks only that device unborn, because Online is
 *        node-scoped and one dead device does not take the node offline.
 */
void TstSparkplugSession::deathCertificates()
{
  SparkplugSession session;
  const QByteArray birth =
    makePayload(0, 1000, {makeMetric("t", 1, code(DataType::Double), realValue(5.0))});
  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), birth));
  QCOMPARE(drainDirty(session).size(), 2);

  QVERIFY(session.ingest(topicOf("g1", "NDEATH", "edge1"), makePayload(-1, 4000, {})));
  QCOMPARE(slotNamed(session, "edge1/Online").num, 0.0);
  QVERIFY(!slotNamed(session, "edge1/Online").b);
  QCOMPARE(drainDirty(session), QStringList{QStringLiteral("edge1/Online")});

  const QByteArray orphan =
    makePayload(-1, 5000, {makeMetric(QByteArray(), 1, code(DataType::Double), realValue(9.0))});
  QVERIFY(session.ingest(topicOf("g1", "NDATA", "edge1"), orphan));
  QCOMPARE(session.counters().preBirthBuffered, quint64(1));
  QCOMPARE(session.counters().rebirthRequests, quint64(1));
  QCOMPARE(slotNamed(session, "edge1/t").num, 5.0);

  SparkplugSession devices;
  QVERIFY(devices.ingest(topicOf("g1", "NBIRTH", "edge2"), makePayload(0, 1000, {})));
  const QByteArray deviceBirth =
    makePayload(1, 1100, {makeMetric("p", 7, code(DataType::Double), realValue(3.0))});
  QVERIFY(devices.ingest(deviceTopic("g1", "DBIRTH", "edge2", "dev1"), deviceBirth));
  QCOMPARE(drainDirty(devices).size(), 2);

  QVERIFY(devices.ingest(deviceTopic("g1", "DDEATH", "edge2", "dev1"), makePayload(-1, 1200, {})));
  QVERIFY(!devices.hasDirtySlots());
  QCOMPARE(slotNamed(devices, "edge2/Online").num, 1.0);
  QVERIFY(slotNamed(devices, "edge2/Online").b);

  const QByteArray stale =
    makePayload(-1, 1300, {makeMetric(QByteArray(), 7, code(DataType::Double), realValue(8.0))});
  QVERIFY(devices.ingest(deviceTopic("g1", "DDATA", "edge2", "dev1"), stale));
  QCOMPARE(devices.counters().preBirthBuffered, quint64(1));
  QCOMPARE(slotNamed(devices, "edge2/dev1/p").num, 3.0);
}

/**
 * @brief An unsupported datatype and an identity-less metric are both counted and neither moves a
 *        value, so an unrenderable payload can never be mistaken for a reading (R6).
 */
void TstSparkplugSession::unsupportedDataTypeKeepsValue()
{
  SparkplugSession session;
  const QByteArray birth =
    makePayload(0, 1000, {makeMetric("t", 1, code(DataType::Double), realValue(5.0))});
  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), birth));
  QCOMPARE(drainDirty(session).size(), 2);

  const QByteArray odd =
    makePayload(1,
                2000,
                {makeMetric(QByteArray(), 1, code(DataType::DateTime), longValue(1700000000000ULL)),
                 makeMetric(QByteArray(), -1, code(DataType::Double), realValue(3.0))});

  QVERIFY(session.ingest(topicOf("g1", "NDATA", "edge1"), odd));
  QCOMPARE(session.counters().unsupportedMetrics, quint64(2));
  QCOMPARE(slotNamed(session, "edge1/t").num, 5.0);
  QCOMPARE(slotNamed(session, "edge1/t").timestampMs, quint64(1000));
  QVERIFY(!session.hasDirtySlots());
  QCOMPARE(session.slotCount(), 2);
}

/**
 * @brief Aliases are scoped to the edge node that declared them, so two nodes reusing the same
 *        alias number keep independent tables and neither publishes into the other's slot.
 */
void TstSparkplugSession::aliasCollisionAcrossNodes()
{
  SparkplugSession session;
  const QByteArray birthA =
    makePayload(0, 1000, {makeMetric("a", 1, code(DataType::Double), realValue(1.0))});
  const QByteArray birthB =
    makePayload(0, 1000, {makeMetric("b", 1, code(DataType::Double), realValue(2.0))});

  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edgeA"), birthA));
  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edgeB"), birthB));
  QCOMPARE(session.slotCount(), 4);

  const QByteArray dataA =
    makePayload(1, 2000, {makeMetric(QByteArray(), 1, code(DataType::Double), realValue(10.0))});
  const QByteArray dataB =
    makePayload(1, 2000, {makeMetric(QByteArray(), 1, code(DataType::Double), realValue(20.0))});

  QVERIFY(session.ingest(topicOf("g1", "NDATA", "edgeA"), dataA));
  QVERIFY(session.ingest(topicOf("g1", "NDATA", "edgeB"), dataB));
  QCOMPARE(slotNamed(session, "edgeA/a").num, 10.0);
  QCOMPARE(slotNamed(session, "edgeB/b").num, 20.0);
  QCOMPARE(session.slotCount(), 4);
  QCOMPARE(session.counters().preBirthBuffered, quint64(0));
}

/**
 * @brief The Sparkplug group scopes node identity: two groups publishing the same edge-node name
 *        keep independent alias tables, sequence expectations and slots, so neither overwrites the
 *        other's metric and neither is blamed for the other's sequence.
 */
void TstSparkplugSession::groupsScopeNodeIdentity()
{
  SparkplugSession session;
  const QByteArray birthA =
    makePayload(0, 1000, {makeMetric("a", 1, code(DataType::Double), realValue(1.0))});
  const QByteArray birthB =
    makePayload(0, 1000, {makeMetric("b", 1, code(DataType::Double), realValue(2.0))});

  QVERIFY(session.ingest(topicOf("gA", "NBIRTH", "edge1"), birthA));
  QVERIFY(session.ingest(topicOf("gB", "NBIRTH", "edge1"), birthB));
  QCOMPARE(session.slotCount(), 4);

  const QByteArray dataA =
    makePayload(1, 2000, {makeMetric(QByteArray(), 1, code(DataType::Double), realValue(10.0))});
  const QByteArray dataB =
    makePayload(1, 2000, {makeMetric(QByteArray(), 1, code(DataType::Double), realValue(20.0))});

  QVERIFY(session.ingest(topicOf("gA", "NDATA", "edge1"), dataA));
  QVERIFY(session.ingest(topicOf("gB", "NDATA", "edge1"), dataB));
  QCOMPARE(slotNamed(session, "edge1/a").num, 10.0);
  QCOMPARE(slotNamed(session, "edge1/b").num, 20.0);
  QCOMPARE(slotNamed(session, "edge1/a").group, QStringLiteral("gA"));
  QCOMPARE(slotNamed(session, "edge1/b").group, QStringLiteral("gB"));
  QCOMPARE(session.slotCount(), 4);
  QCOMPARE(session.counters().seqGaps, quint64(0));
  QCOMPARE(session.counters().preBirthBuffered, quint64(0));

  QVERIFY(session.ingest(topicOf("gA", "NDEATH", "edge1"), makePayload(-1, 3000, {})));
  QCOMPARE(session.slotValues().at(0).num, 0.0);
  QCOMPARE(session.slotValues().at(2).num, 1.0);
}

/**
 * @brief Only this session's namespace and group are consumed: a foreign namespace, a malformed
 *        element count, a trailing slash and another group all decline the message so the driver
 *        can publish it raw, while NCMD and STATE are consumed and counted rather than parsed.
 */
void TstSparkplugSession::topicRouting()
{
  SparkplugSession session;
  session.setGroupFilter(QStringLiteral("plant1"));
  QCOMPARE(session.groupFilter(), QStringLiteral("plant1"));

  const QByteArray birth =
    makePayload(0, 1000, {makeMetric("t", 1, code(DataType::Double), realValue(1.0))});

  QVERIFY(!session.ingest(topicOf("plant2", "NBIRTH", "edge1"), birth));
  QVERIFY(!session.ingest(QStringLiteral("acme/plant1/NBIRTH/edge1"), birth));
  QVERIFY(!session.ingest(QStringLiteral("spBv1.0/plant1"), birth));
  QVERIFY(!session.ingest(QStringLiteral("spBv1.0/plant1/NBIRTH/edge1/dev1/extra"), birth));
  QVERIFY(!session.ingest(QStringLiteral("spBv1.0/plant1/DBIRTH/edge1/"), birth));
  QVERIFY(!session.ingest(QStringLiteral("spBv1.0/plant1/DDATA/edge1/"), birth));
  QCOMPARE(session.slotCount(), 0);
  QCOMPARE(session.counters().ignoredMessages, quint64(0));

  QVERIFY(session.ingest(topicOf("plant1", "NBIRTH", "edge1"), birth));
  QCOMPARE(session.slotCount(), 2);

  QVERIFY(session.ingest(topicOf("plant1", "NCMD", "edge1"), QByteArray()));
  QVERIFY(session.ingest(QStringLiteral("spBv1.0/STATE/host1"), QByteArray()));
  QCOMPARE(session.counters().ignoredMessages, quint64(2));
  QCOMPARE(session.slotCount(), 2);
}

/**
 * @brief Identity is capped in bytes, not only in item count: an over-long metric name fails the
 *        decode, an over-long topic element declines the topic outright, and a latched string is
 *        clamped to what the delta encoder can emit rather than retained whole.
 */
void TstSparkplugSession::identityCapsRefuseGrowth()
{
  const QByteArray huge(Sparkplug::kMaxIdentityBytes + 1, 'x');

  SparkplugSession session;
  const QByteArray named =
    makePayload(0, 1000, {makeMetric(huge, 1, code(DataType::Double), realValue(1.0))});
  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), named));
  QCOMPARE(session.counters().decodeErrors, quint64(1));
  QCOMPARE(session.slotCount(), 0);

  const QByteArray birth =
    makePayload(0, 1000, {makeMetric("t", 1, code(DataType::String), textValue("ok"))});
  QVERIFY(!session.ingest(topicOf("g1", "NBIRTH", huge.constData()), birth));
  QVERIFY(!session.ingest(deviceTopic("g1", "DBIRTH", "edge1", huge.constData()), birth));
  QVERIFY(!session.ingest(topicOf(huge.constData(), "NBIRTH", "edge1"), birth));
  QCOMPARE(session.slotCount(), 0);

  const QByteArray wide(Wire::kMaxStringBytes * 4, 'y');
  const QByteArray big =
    makePayload(0, 1000, {makeMetric("t", 1, code(DataType::String), textValue(wide))});
  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), big));
  QCOMPARE(session.slotCount(), 2);
  QCOMPARE(slotNamed(session, "edge1/t").str.size(), qsizetype(Wire::kMaxStringBytes));
}

/**
 * @brief A payload the codec rejects is still consumed, leaves nothing but a counter behind, and
 *        never touches a latched value or the change marks.
 */
void TstSparkplugSession::malformedPayloadCounted()
{
  SparkplugSession session;
  const QByteArray birth =
    makePayload(0, 1000, {makeMetric("t", 1, code(DataType::Double), realValue(4.5))});
  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), birth));
  QCOMPARE(drainDirty(session).size(), 2);

  QByteArray truncated;
  truncated.append("\xFF\xFF\xFF", 3);
  QVERIFY(session.ingest(topicOf("g1", "NDATA", "edge1"), truncated));
  QCOMPARE(session.counters().decodeErrors, quint64(1));
  QVERIFY(!session.hasDirtySlots());
  QCOMPARE(session.slotCount(), 2);
  QCOMPARE(slotNamed(session, "edge1/t").num, 4.5);

  const QByteArray runOn(Sparkplug::kMaxVarintBytes + 1, static_cast<char>(0xFF));
  QVERIFY(session.ingest(topicOf("g1", "NDATA", "edge1"), runOn));
  QCOMPARE(session.counters().decodeErrors, quint64(2));
  QCOMPARE(session.counters().preBirthBuffered, quint64(0));
  QCOMPARE(session.slotCount(), 2);
}

/**
 * @brief The slot table never grows past its cap: eight nodes fill it exactly, and the next metric
 *        is refused and counted instead of resizing the table the wire indices are bound to (R11).
 */
void TstSparkplugSession::slotCapRefusesGrowth()
{
  SparkplugSession session;
  const int nodes   = 8;
  const int metrics = (Limits::kMaxSlots / nodes) - 1;

  QList<QByteArray> block;
  block.reserve(metrics);
  for (int i = 0; i < metrics; ++i)
    block.append(makeMetric(QByteArray("m") + QByteArray::number(i),
                            i,
                            code(DataType::Double),
                            realValue(static_cast<double>(i))));

  const QByteArray birth = makePayload(0, 1000, block);
  for (int n = 0; n < nodes; ++n) {
    const QByteArray node = QByteArray("n") + QByteArray::number(n);
    QVERIFY(session.ingest(topicOf("g1", "NBIRTH", node.constData()), birth));
  }

  QCOMPARE(session.slotCount(), Limits::kMaxSlots);
  QCOMPARE(session.counters().capDrops, quint64(0));

  const QByteArray extra =
    makePayload(1, 2000, {makeMetric("extra", -1, code(DataType::Double), realValue(1.0))});
  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "n0"), extra));
  QCOMPARE(session.slotCount(), Limits::kMaxSlots);
  QCOMPARE(session.counters().capDrops, quint64(1));
}

/**
 * @brief The generation lifecycle: the slot table the generator reads its schema from names every
 *        assigned slot in index order, marking it generated zeroes the new-metric count, and only
 *        an identity never seen before raises it.
 */
void TstSparkplugSession::schemaGenerationLifecycle()
{
  SparkplugSession session;
  const QByteArray birth =
    makePayload(0,
                1000,
                {makeMetric("a", 1, code(DataType::Double), realValue(1.0)),
                 makeMetric("b", 2, code(DataType::Boolean), flagValue(true))});

  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), birth));
  QCOMPARE(session.newMetricsSinceGeneration(), quint64(3));

  const QVector<SparkplugSession::SlotValue> schema = session.slotValues();
  QCOMPARE(schema.size(), qsizetype(3));
  QCOMPARE(schema.at(0).index, 0);
  QCOMPARE(schema.at(0).name, QString::fromLatin1(Limits::kOnlineMetric));
  QCOMPARE(schema.at(0).node, QStringLiteral("edge1"));
  QVERIFY(schema.at(0).device.isEmpty());
  QCOMPARE(schema.at(1).name, QStringLiteral("a"));
  QCOMPARE(schema.at(2).name, QStringLiteral("b"));

  session.markGenerated();
  QCOMPARE(session.newMetricsSinceGeneration(), quint64(0));
  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), birth));
  QCOMPARE(session.newMetricsSinceGeneration(), quint64(0));

  const QByteArray deviceBirth =
    makePayload(1, 2000, {makeMetric("p", 7, code(DataType::Double), realValue(3.0))});
  QVERIFY(session.ingest(deviceTopic("g1", "DBIRTH", "edge1", "dev1"), deviceBirth));
  QCOMPARE(session.newMetricsSinceGeneration(), quint64(1));

  const QVector<SparkplugSession::SlotValue> grown = session.slotValues();
  QCOMPARE(grown.size(), qsizetype(4));
  QCOMPARE(grown.at(3).index, 3);
  QCOMPARE(grown.at(3).device, QStringLiteral("dev1"));
  QCOMPARE(grown.at(3).name, QStringLiteral("p"));
}

/**
 * @brief Reset drops birth state, buffered traffic, the rebirth queue, every latched value and the
 *        pulled counters, and KEEPS the slot table: the indices a generated project's datasets bind
 *        to have to survive a broker drop (spec 0075 E2).
 */
void TstSparkplugSession::resetKeepsSlotsAndClearsSession()
{
  SparkplugSession session;
  const QByteArray birth =
    makePayload(0, 1000, {makeMetric("t", 1, code(DataType::Double), realValue(2.0))});
  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), birth));

  const QByteArray orphan =
    makePayload(-1, 2000, {makeMetric(QByteArray(), 5, code(DataType::Double), realValue(99.0))});
  QVERIFY(session.ingest(topicOf("g1", "NDATA", "edge9"), orphan));
  QCOMPARE(session.slotCount(), 2);
  QVERIFY(session.hasDirtySlots());
  QCOMPARE(session.counters().preBirthBuffered, quint64(1));

  session.reset();
  QCOMPARE(session.slotCount(), 2);
  QVERIFY(!session.hasDirtySlots());
  QVERIFY(session.takeRebirthTopics().isEmpty());
  QCOMPARE(session.newMetricsSinceGeneration(), quint64(0));
  QCOMPARE(session.counters().preBirthBuffered, quint64(0));
  QCOMPARE(session.counters().rebirthRequests, quint64(0));
  QCOMPARE(session.counters().seqGaps, quint64(0));
  QCOMPARE(slotNamed(session, "edge1/t").num, 0.0);
  QCOMPARE(slotNamed(session, "edge1/Online").b, false);

  const QByteArray revived =
    makePayload(0, 3000, {makeMetric("t", 5, code(DataType::Double), realValue(1.0))});
  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), revived));
  QCOMPARE(session.slotCount(), 2);
  QCOMPARE(slotNamed(session, "edge1/t").index, 1);
  QCOMPARE(slotNamed(session, "edge1/t").num, 1.0);
}

/**
 * @brief The E2 regression: after a broker drop the nodes re-birth in whatever order the broker
 *        delivers them. Every metric has to land back in the index it was assigned, or one node's
 *        readings render under another node's dataset titles with nothing to say they moved.
 */
void TstSparkplugSession::reconnectWithReversedBirthsKeepsIndices()
{
  SparkplugSession session;
  const QByteArray first =
    makePayload(0, 1000, {makeMetric("a", 1, code(DataType::Double), realValue(1.0))});
  const QByteArray second =
    makePayload(0, 1000, {makeMetric("b", 1, code(DataType::Double), realValue(2.0))});

  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), first));
  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge2"), second));
  QCOMPARE(session.slotCount(), 4);
  QCOMPARE(slotNamed(session, "edge1/a").index, 1);
  QCOMPARE(slotNamed(session, "edge2/b").index, 3);

  session.reset();
  const QByteArray reborn2 =
    makePayload(0, 5000, {makeMetric("b", 9, code(DataType::Double), realValue(20.0))});
  const QByteArray reborn1 =
    makePayload(0, 5000, {makeMetric("a", 9, code(DataType::Double), realValue(10.0))});

  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge2"), reborn2));
  QVERIFY(session.ingest(topicOf("g1", "NBIRTH", "edge1"), reborn1));
  QCOMPARE(session.slotCount(), 4);
  QCOMPARE(slotNamed(session, "edge1/a").index, 1);
  QCOMPARE(slotNamed(session, "edge1/a").num, 10.0);
  QCOMPARE(slotNamed(session, "edge2/b").index, 3);
  QCOMPARE(slotNamed(session, "edge2/b").num, 20.0);
  QCOMPARE(session.newMetricsSinceGeneration(), quint64(0));
}

/**
 * @brief The persisted table is what carries the indices across a restart, so it has to restore the
 *        identity of every slot before the first birth and let a node that births first land in the
 *        index it held last session rather than in index zero.
 */
void TstSparkplugSession::slotTableRoundTripsThroughJson()
{
  SparkplugSession source;
  const QByteArray nodeBirth =
    makePayload(0, 1000, {makeMetric("a", 1, code(DataType::Double), realValue(1.0))});
  const QByteArray deviceBirth =
    makePayload(1, 1000, {makeMetric("p", 2, code(DataType::Double), realValue(3.0))});

  QVERIFY(source.ingest(topicOf("g1", "NBIRTH", "edge1"), nodeBirth));
  QVERIFY(source.ingest(topicOf("g1", "NBIRTH", "edge2"), nodeBirth));
  QVERIFY(source.ingest(deviceTopic("g1", "DBIRTH", "edge1", "dev1"), deviceBirth));

  const QJsonArray stored = source.slotsJson();
  QCOMPARE(stored.size(), qsizetype(source.slotCount()));

  SparkplugSession restored;
  restored.restoreSlots(stored);
  QCOMPARE(restored.slotCount(), source.slotCount());
  QCOMPARE(restored.counters().capDrops, quint64(0));
  for (int i = 0; i < source.slotCount(); ++i)
    QCOMPARE(restored.slotValues().at(i).displayName, source.slotValues().at(i).displayName);

  const QByteArray reborn =
    makePayload(0, 7000, {makeMetric("a", 4, code(DataType::Double), realValue(42.0))});
  QVERIFY(restored.ingest(topicOf("g1", "NBIRTH", "edge2"), reborn));
  QCOMPARE(restored.slotCount(), source.slotCount());
  QCOMPARE(slotNamed(restored, "edge2/a").index, slotNamed(source, "edge2/a").index);
  QCOMPARE(slotNamed(restored, "edge2/a").num, 42.0);
}

/**
 * @brief A stored table whose entry does not sit at the index it declares cannot be trusted to name
 *        the same metrics, so the restore is refused whole and counted; deriving from scratch is
 *        correct, restoring half of it would repoint every dataset after the bad entry.
 */
void TstSparkplugSession::aMisplacedRestoreEntryIsRefusedWhole()
{
  SparkplugSession source;
  const QByteArray birth =
    makePayload(0, 1000, {makeMetric("a", 1, code(DataType::Double), realValue(1.0))});
  QVERIFY(source.ingest(topicOf("g1", "NBIRTH", "edge1"), birth));

  QJsonArray stored = source.slotsJson();
  QVERIFY(stored.size() >= 2);

  QJsonObject broken = stored.at(1).toObject();
  broken.insert(Keys::Index, 7);
  stored.replace(1, broken);

  SparkplugSession restored;
  restored.restoreSlots(stored);
  QCOMPARE(restored.slotCount(), 0);
  QCOMPARE(restored.counters().capDrops, quint64(1));

  restored.restoreSlots(source.slotsJson());
  QCOMPARE(restored.slotCount(), source.slotCount());

  restored.restoreSlots(source.slotsJson());
  QCOMPARE(restored.slotCount(), source.slotCount());
  QCOMPARE(restored.counters().capDrops, quint64(1));
}

QTEST_APPLESS_MAIN(TstSparkplugSession)

#include "tst_sparkplug_session.moc"
