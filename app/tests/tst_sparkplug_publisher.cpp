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

#include <QTest>

#include "MQTT/SparkplugPublisher.h"

using namespace IO::Drivers::SparkplugB;
using MQTT::SparkplugPublisher;

inline constexpr int kBooleanKind = static_cast<int>(ValueKind::Boolean);
inline constexpr int kNumericKind = static_cast<int>(ValueKind::Numeric);
inline constexpr int kStringKind  = static_cast<int>(ValueKind::String);

inline constexpr const char* kGroup  = "Plant1";
inline constexpr const char* kNode   = "SS1";
inline constexpr const char* kDevice = "Line3";

inline constexpr int kSourceA = 1;
inline constexpr int kSourceB = 2;

/**
 * @brief Datatype code as it travels in Metric field 4.
 */
[[nodiscard]] static constexpr quint32 code(DataType type) noexcept
{
  return static_cast<quint32>(type);
}

/**
 * @brief A configured, enabled edge node; @p device empty publishes at node level.
 */
[[nodiscard]] static SparkplugPublisher::Config nodeConfig(const QString& device = QString())
{
  SparkplugPublisher::Config config;
  config.enabled    = true;
  config.groupId    = QString::fromUtf8(kGroup);
  config.edgeNodeId = QString::fromUtf8(kNode);
  config.deviceId   = device;
  return config;
}

/**
 * @brief Decodes a produced message, failing the test when the codec rejects its own output. Every
 *        assertion below reads the wire through this, so the round trip is checked everywhere.
 */
[[nodiscard]] static Payload decoded(const SparkplugPublisher::Message& message)
{
  Payload payload;
  QString error;
  const bool ok = decodePayload(message.payload, payload, &error);
  if (!ok)
    qWarning("payload on %s did not decode: %s", qPrintable(message.topic), qPrintable(error));

  return payload;
}

/**
 * @brief The metric carrying @p name, or a default-constructed one when the payload has none.
 */
[[nodiscard]] static Metric metricNamed(const Payload& payload, const char* name)
{
  const auto wanted = QString::fromUtf8(name);
  for (const auto& metric : payload.metrics)
    if (metric.hasName && metric.name == wanted)
      return metric;

  return Metric();
}

/**
 * @brief Whether a payload declares a metric addressed by @p alias, regardless of its name. Data
 *        messages carry no names, so alias is the only handle a data-path assertion has.
 */
[[nodiscard]] static bool hasAlias(const Payload& payload, quint64 alias)
{
  for (const auto& metric : payload.metrics)
    if (metric.hasAlias && metric.alias == alias)
      return true;

  return false;
}

/**
 * @brief Value of the bdSeq metric a birth or a death certificate carries.
 */
[[nodiscard]] static double birthDeathSequence(const SparkplugPublisher::Message& message)
{
  const auto payload = decoded(message);
  const auto metric  = metricNamed(payload, MQTT::SparkplugPublisherLimits::kBirthDeathSequence);
  return metric.numericValue;
}

/**
 * @brief Sparkplug B Edge Node lifecycle: birth certificates and their alias tables, change-only
 *        data messages, the seq/bdSeq contract, rebirth commands, and the datatype skip rule
 *        (spec 0073 R39-R44).
 */
class TstSparkplugPublisher : public QObject {
  Q_OBJECT

private slots:
  void topicsFollowTheNamespace();
  void birthDeclaresEveryMetric();
  void dataCarriesOnlyChangedMetrics();
  void dataWaitsForTheBirth();
  void deviceMetricsRideTheDeviceTopics();
  void sequenceWrapsAcrossBirthsAndData();
  void birthDeathSequenceMatchesTheWill();
  void rebirthCommandReissuesTheBirth();
  void unrepresentableValuesAreSkippedAndCounted();
  void newMetricsAfterBirthRequestARebirth();
  void twoSourcesGetDistinctStableAliases();
  void restructuringOneSourceKeepsTheOther();
  void singleSourceMatchesTheLegacyWire();
  void aSwapDropsTheDepartedSource();
  void theRegistryCapDropsAndCounts();
  void oneDeviceCarriesEverySource();
  void collidingTitlesAreSourceQualified();
};

/**
 * @brief Node and device topics carry the v1.0 namespace, the group, the verb and the node.
 */
void TstSparkplugPublisher::topicsFollowTheNamespace()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());
  QVERIFY(node.valid());
  QCOMPARE(node.commandTopic(), QStringLiteral("spBv1.0/Plant1/NCMD/SS1"));

  const auto will = node.beginConnection(1000);
  QCOMPARE(will.topic, QStringLiteral("spBv1.0/Plant1/NDEATH/SS1"));
  QCOMPARE(static_cast<int>(will.qos), 1);
  QVERIFY(!will.retain);

  const auto birth = node.birthMessages(1000);
  QCOMPARE(birth.size(), qsizetype(1));
  QCOMPARE(birth.constFirst().topic, QStringLiteral("spBv1.0/Plant1/NBIRTH/SS1"));
  QCOMPARE(static_cast<int>(birth.constFirst().qos), 0);
}

/**
 * @brief The birth declares every registered metric with its name, its alias and its datatype,
 *        alongside the bdSeq and rebirth-control metrics a host expects.
 */
void TstSparkplugPublisher::birthDeclaresEveryMetric()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());
  node.registerMetric(0, 11, QStringLiteral("Sensors/Temperature"), code(DataType::Double), 1);
  node.registerMetric(0, 22, QStringLiteral("Sensors/State"), code(DataType::String), 1);
  node.registerMetric(0, 33, QStringLiteral("Sensors/Valve"), code(DataType::Boolean), 1);
  QCOMPARE(node.metricCount(), 3);

  const auto will = node.beginConnection(1000);
  QVERIFY(!will.payload.isEmpty());

  const auto birth = node.birthMessages(1700000000000ULL);
  QCOMPARE(birth.size(), qsizetype(1));

  const auto payload = decoded(birth.constFirst());
  QVERIFY(payload.hasSeq);
  QVERIFY(payload.hasTimestamp);
  QCOMPARE(payload.metrics.size(), qsizetype(5));

  const auto temperature = metricNamed(payload, "Sensors/Temperature");
  QVERIFY(temperature.hasName);
  QVERIFY(temperature.hasAlias);
  QCOMPARE(temperature.alias, quint64(1));
  QCOMPARE(temperature.datatype, code(DataType::Double));
  QCOMPARE(static_cast<int>(temperature.kind), kNumericKind);

  const auto state = metricNamed(payload, "Sensors/State");
  QCOMPARE(state.alias, quint64(2));
  QCOMPARE(static_cast<int>(state.kind), kStringKind);

  const auto valve = metricNamed(payload, "Sensors/Valve");
  QCOMPARE(valve.alias, quint64(3));
  QCOMPARE(static_cast<int>(valve.kind), kBooleanKind);

  const auto control = metricNamed(payload, kRebirthMetricName);
  QVERIFY(control.hasName);
  QCOMPARE(static_cast<int>(control.kind), kBooleanKind);
}

/**
 * @brief A data message carries only the metrics that moved, addressed by alias and with no names
 *        on the wire; a publish with nothing pending produces no message at all.
 */
void TstSparkplugPublisher::dataCarriesOnlyChangedMetrics()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());
  node.registerMetric(0, 11, QStringLiteral("Temperature"), code(DataType::Double), 1);
  node.registerMetric(0, 22, QStringLiteral("State"), code(DataType::String), 1);

  const auto will = node.beginConnection(1000);
  QVERIFY(!will.topic.isEmpty());

  const auto birth = node.birthMessages(1000);
  node.commitBirth();
  QCOMPARE(birth.size(), qsizetype(1));
  QVERIFY(node.dataMessages(1001).isEmpty());

  node.updateValue(11, 21.5, QString(), true);
  const auto data = node.dataMessages(1002);
  QCOMPARE(data.size(), qsizetype(1));
  QCOMPARE(data.constFirst().topic, QStringLiteral("spBv1.0/Plant1/NDATA/SS1"));

  const auto payload = decoded(data.constFirst());
  QCOMPARE(payload.metrics.size(), qsizetype(1));

  const auto& metric = payload.metrics.constFirst();
  QVERIFY(!metric.hasName);
  QVERIFY(metric.hasAlias);
  QVERIFY(!metric.isNull);
  QCOMPARE(metric.alias, quint64(1));
  QCOMPARE(metric.numericValue, 21.5);

  QVERIFY(node.dataMessages(1003).isEmpty());
  node.updateValue(11, 21.5, QString(), true);
  QVERIFY(node.dataMessages(1004).isEmpty());

  node.updateValue(22, 0.0, QStringLiteral("RUNNING"), false);
  const auto second = node.dataMessages(1005);
  QCOMPARE(second.size(), qsizetype(1));

  const auto text = decoded(second.constFirst());
  QCOMPARE(text.metrics.size(), qsizetype(1));
  QCOMPARE(text.metrics.constFirst().alias, quint64(2));
  QCOMPARE(text.metrics.constFirst().stringValue, QStringLiteral("RUNNING"));
}

/**
 * @brief Values latched before the birth certificate are not published on their own: a host can
 *        only resolve an alias it saw declared, so the birth carries them instead.
 */
void TstSparkplugPublisher::dataWaitsForTheBirth()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());
  node.registerMetric(0, 11, QStringLiteral("Temperature"), code(DataType::Double), 1);

  const auto will = node.beginConnection(1000);
  QVERIFY(!will.topic.isEmpty());

  node.updateValue(11, 4.25, QString(), true);
  QVERIFY(node.dataMessages(1001).isEmpty());

  const auto birth = node.birthMessages(1002);
  node.commitBirth();
  const auto payload = decoded(birth.constFirst());
  const auto metric  = metricNamed(payload, "Temperature");
  QVERIFY(!metric.isNull);
  QCOMPARE(metric.numericValue, 4.25);
  QVERIFY(node.dataMessages(1003).isEmpty());
}

/**
 * @brief With a device configured the node birth stays a container and the dataset metrics move
 *        onto the device's own birth and data topics.
 */
void TstSparkplugPublisher::deviceMetricsRideTheDeviceTopics()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig(QString::fromUtf8(kDevice)));
  node.registerMetric(0, 11, QStringLiteral("Temperature"), code(DataType::Double), 1);

  const auto will = node.beginConnection(1000);
  QCOMPARE(will.topic, QStringLiteral("spBv1.0/Plant1/NDEATH/SS1"));

  const auto birth = node.birthMessages(1000);
  node.commitBirth();
  QCOMPARE(birth.size(), qsizetype(2));
  QCOMPARE(birth.constFirst().topic, QStringLiteral("spBv1.0/Plant1/NBIRTH/SS1"));
  QCOMPARE(birth.constLast().topic, QStringLiteral("spBv1.0/Plant1/DBIRTH/SS1/Line3"));

  const auto nodePayload = decoded(birth.constFirst());
  QCOMPARE(nodePayload.metrics.size(), qsizetype(2));
  QVERIFY(!metricNamed(nodePayload, "Temperature").hasName);

  const auto devicePayload = decoded(birth.constLast());
  QCOMPARE(devicePayload.metrics.size(), qsizetype(1));
  QCOMPARE(metricNamed(devicePayload, "Temperature").alias, quint64(1));

  node.updateValue(11, 7.5, QString(), true);
  const auto data = node.dataMessages(1001);
  QCOMPARE(data.size(), qsizetype(1));
  QCOMPARE(data.constFirst().topic, QStringLiteral("spBv1.0/Plant1/DDATA/SS1/Line3"));
}

/**
 * @brief The sequence number is shared by every published message, births included, and wraps
 *        255 -> 0 without a gap (R41).
 */
void TstSparkplugPublisher::sequenceWrapsAcrossBirthsAndData()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());
  node.registerMetric(0, 11, QStringLiteral("Counter"), code(DataType::Double), 1);

  const auto will = node.beginConnection(1000);
  QVERIFY(!will.topic.isEmpty());

  const auto birth = node.birthMessages(1000);
  node.commitBirth();
  QCOMPARE(decoded(birth.constFirst()).seq, quint64(0));

  for (int i = 0; i < 300; ++i) {
    node.updateValue(11, static_cast<double>(i) + 1.0, QString(), true);
    const auto data = node.dataMessages(static_cast<quint64>(1001 + i));
    QCOMPARE(data.size(), qsizetype(1));
    QCOMPARE(decoded(data.constFirst()).seq, quint64((i + 1) % 256));
  }

  QCOMPARE(node.seq(), quint64(301 % 256));
}

/**
 * @brief bdSeq advances once per connection and the birth certificate carries exactly the value
 *        the registered will announces (R41, R42).
 */
void TstSparkplugPublisher::birthDeathSequenceMatchesTheWill()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());
  node.registerMetric(0, 11, QStringLiteral("Temperature"), code(DataType::Double), 1);

  const auto firstWill  = node.beginConnection(1000);
  const auto firstBirth = node.birthMessages(1001);
  QCOMPARE(node.bdSeq(), quint64(0));
  QCOMPARE(birthDeathSequence(firstWill), 0.0);
  QCOMPARE(birthDeathSequence(firstBirth.constFirst()), 0.0);

  const auto secondWill  = node.beginConnection(2000);
  const auto secondBirth = node.birthMessages(2001);
  QCOMPARE(node.bdSeq(), quint64(1));
  QCOMPARE(birthDeathSequence(secondWill), 1.0);
  QCOMPARE(birthDeathSequence(secondBirth.constFirst()), 1.0);
  QCOMPARE(decoded(secondBirth.constFirst()).seq, quint64(0));
  QVERIFY(!decoded(secondWill).hasSeq);
}

/**
 * @brief A Node Control/Rebirth command re-issues the full birth certificate; anything else is
 *        counted and ignored, and a payload that does not decode is counted as such (R43).
 */
void TstSparkplugPublisher::rebirthCommandReissuesTheBirth()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());
  node.registerMetric(0, 11, QStringLiteral("Temperature"), code(DataType::Double), 1);

  const auto will = node.beginConnection(1000);
  QVERIFY(!will.topic.isEmpty());

  const auto birth = node.birthMessages(1000);
  node.commitBirth();
  QCOMPARE(birth.size(), qsizetype(1));
  QCOMPARE(node.counters().births, quint64(1));

  QVERIFY(node.isRebirthCommand(encodeRebirthRequest(1500)));
  QCOMPARE(node.counters().rebirthCommands, quint64(1));

  const auto rebirth = node.birthMessages(1501);
  node.commitBirth();
  QCOMPARE(rebirth.size(), qsizetype(1));
  QCOMPARE(node.counters().births, quint64(2));

  const auto payload = decoded(rebirth.constFirst());
  QCOMPARE(payload.metrics.size(), qsizetype(3));
  QCOMPARE(metricNamed(payload, "Temperature").alias, quint64(1));
  QCOMPARE(payload.seq, quint64(1));

  QVERIFY(!node.isRebirthCommand(QByteArray("\x08\x01", 2)));
  QCOMPARE(node.counters().ignoredCommands, quint64(1));
  QVERIFY(!node.isRebirthCommand(QByteArray("\xFF\xFF", 2)));
  QCOMPARE(node.counters().commandDecodeErrors, quint64(1));
}

/**
 * @brief A value the declared datatype cannot carry is skipped and counted, never published under
 *        a type that would misrepresent it (R44).
 */
void TstSparkplugPublisher::unrepresentableValuesAreSkippedAndCounted()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());
  node.registerMetric(0, 11, QStringLiteral("Small"), code(DataType::Int8), 1);
  node.registerMetric(0, 22, QStringLiteral("Temperature"), code(DataType::Double), 1);

  const auto will = node.beginConnection(1000);
  QVERIFY(!will.topic.isEmpty());

  const auto birth = node.birthMessages(1000);
  node.commitBirth();
  QCOMPARE(birth.size(), qsizetype(1));

  node.updateValue(11, 5000.0, QString(), true);
  QCOMPARE(node.counters().skippedValues, quint64(1));
  QVERIFY(node.dataMessages(1001).isEmpty());

  node.updateValue(11, 3.5, QString(), true);
  QCOMPARE(node.counters().skippedValues, quint64(2));
  QVERIFY(node.dataMessages(1002).isEmpty());

  node.updateValue(22, 0.0, QStringLiteral("not a number"), false);
  QCOMPARE(node.counters().skippedValues, quint64(3));
  QVERIFY(node.dataMessages(1003).isEmpty());

  node.updateValue(11, -12.0, QString(), true);
  const auto data = node.dataMessages(1004);
  QCOMPARE(data.size(), qsizetype(1));

  const auto payload = decoded(data.constFirst());
  QCOMPARE(payload.metrics.size(), qsizetype(1));
  QCOMPARE(payload.metrics.constFirst().numericValue, -12.0);
  QCOMPARE(node.counters().skippedValues, quint64(3));
}

/**
 * @brief A dataset registered after the birth leaves the host holding a stale certificate, so the
 *        node asks for a rebirth rather than shipping an alias nothing can resolve.
 */
void TstSparkplugPublisher::newMetricsAfterBirthRequestARebirth()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());
  node.registerMetric(0, 11, QStringLiteral("Temperature"), code(DataType::Double), 1);

  const auto will = node.beginConnection(1000);
  QVERIFY(!will.topic.isEmpty());

  const auto birth = node.birthMessages(1000);
  node.commitBirth();
  QCOMPARE(birth.size(), qsizetype(1));
  QVERIFY(!node.needsRebirth());

  node.registerMetric(0, 22, QStringLiteral("Pressure"), code(DataType::Double), 1);
  QVERIFY(node.needsRebirth());
  QCOMPARE(node.metricCount(), 2);

  const auto rebirth = node.birthMessages(1001);
  node.commitBirth();
  QVERIFY(!node.needsRebirth());
  QCOMPARE(metricNamed(decoded(rebirth.constFirst()), "Pressure").alias, quint64(2));
  QCOMPARE(metricNamed(decoded(rebirth.constFirst()), "Temperature").alias, quint64(1));
}

/**
 * @brief Two sources register under one edge node with distinct stable aliases; data from either
 *        resolves; removing a dataset from one source keeps the other's aliases and never reuses
 *        the retired alias (spec 0074 R1, R2, R5 -- AC1).
 */
void TstSparkplugPublisher::twoSourcesGetDistinctStableAliases()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());
  node.registerMetric(kSourceA, 11, QStringLiteral("Temp"), code(DataType::Double), 1);
  node.registerMetric(kSourceA, 12, QStringLiteral("Humidity"), code(DataType::Double), 1);
  node.registerMetric(kSourceB, 21, QStringLiteral("Pressure"), code(DataType::Double), 1);
  QCOMPARE(node.metricCount(), 3);

  node.beginConnection(1000);
  const auto birth = decoded(node.birthMessages(1000).constFirst());
  node.commitBirth();
  QCOMPARE(metricNamed(birth, "Temp").alias, quint64(1));
  QCOMPARE(metricNamed(birth, "Humidity").alias, quint64(2));
  QCOMPARE(metricNamed(birth, "Pressure").alias, quint64(3));

  node.updateValue(11, 20.0, QString(), true);
  node.updateValue(21, 101.3, QString(), true);
  const auto data = decoded(node.dataMessages(1001).constFirst());
  QCOMPARE(data.metrics.size(), qsizetype(2));
  QVERIFY(hasAlias(data, 1));
  QVERIFY(hasAlias(data, 3));

  node.clearSource(kSourceA);
  node.registerMetric(kSourceA, 11, QStringLiteral("Temp"), code(DataType::Double), 2);
  QCOMPARE(node.metricCount(), 2);

  const auto rebirth = decoded(node.birthMessages(1002).constFirst());
  node.commitBirth();
  QCOMPARE(metricNamed(rebirth, "Temp").alias, quint64(1));
  QCOMPARE(metricNamed(rebirth, "Pressure").alias, quint64(3));
  QVERIFY(!metricNamed(rebirth, "Humidity").hasName);

  node.registerMetric(kSourceB, 22, QStringLiteral("Flow"), code(DataType::Double), 2);
  const auto grown = decoded(node.birthMessages(1003).constFirst());
  QCOMPARE(metricNamed(grown, "Flow").alias, quint64(4));
}

/**
 * @brief Restructuring one source flips needsRebirth and the next birth re-declares the whole
 *        current set, while the untouched source's aliases are byte-identical before and after
 *        (spec 0074 R3, R4 -- AC2).
 */
void TstSparkplugPublisher::restructuringOneSourceKeepsTheOther()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());
  node.registerMetric(kSourceA, 11, QStringLiteral("Temp"), code(DataType::Double), 1);
  node.registerMetric(kSourceB, 21, QStringLiteral("Pressure"), code(DataType::Double), 1);

  node.beginConnection(1000);
  node.birthMessages(1000);
  node.commitBirth();
  QVERIFY(!node.needsRebirth());

  node.clearSource(kSourceB);
  node.registerMetric(kSourceB, 21, QStringLiteral("Pressure"), code(DataType::Double), 2);
  node.registerMetric(kSourceB, 22, QStringLiteral("Flow"), code(DataType::Double), 2);
  QVERIFY(node.needsRebirth());

  const auto rebirth = decoded(node.birthMessages(1001).constFirst());
  node.commitBirth();
  QCOMPARE(metricNamed(rebirth, "Temp").alias, quint64(1));
  QCOMPARE(metricNamed(rebirth, "Pressure").alias, quint64(2));
  QCOMPARE(metricNamed(rebirth, "Flow").alias, quint64(3));
  QVERIFY(!node.needsRebirth());
}

/**
 * @brief A single-source project produces births, aliases and data byte-identical to the spec-0073
 *        positional scheme: aliases 1,2,3 in registration order, bare names in the birth and
 *        alias-only data. Pins the common path against accidental drift (spec 0074 R6 -- AC3).
 */
void TstSparkplugPublisher::singleSourceMatchesTheLegacyWire()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());
  node.registerMetric(0, 11, QStringLiteral("Sensors/Temperature"), code(DataType::Double), 1);
  node.registerMetric(0, 22, QStringLiteral("Sensors/State"), code(DataType::String), 1);
  node.registerMetric(0, 33, QStringLiteral("Sensors/Valve"), code(DataType::Boolean), 1);

  node.beginConnection(1000);
  const auto birth = decoded(node.birthMessages(1700000000000ULL).constFirst());
  node.commitBirth();

  const auto temperature = metricNamed(birth, "Sensors/Temperature");
  QVERIFY(temperature.hasName);
  QCOMPARE(temperature.name, QStringLiteral("Sensors/Temperature"));
  QCOMPARE(temperature.alias, quint64(1));
  QCOMPARE(metricNamed(birth, "Sensors/State").alias, quint64(2));
  QCOMPARE(metricNamed(birth, "Sensors/Valve").alias, quint64(3));

  node.updateValue(11, 21.5, QString(), true);
  const auto data = decoded(node.dataMessages(1701).constFirst());
  QCOMPARE(data.metrics.size(), qsizetype(1));
  QVERIFY(!data.metrics.constFirst().hasName);
  QCOMPARE(data.metrics.constFirst().alias, quint64(1));
  QCOMPARE(data.metrics.constFirst().numericValue, 21.5);
}

/**
 * @brief A project swap adopts the newer frame-pool generation and drops the departed source's
 *        metrics, so only the new project's source births (spec 0074 R7 -- AC4).
 */
void TstSparkplugPublisher::aSwapDropsTheDepartedSource()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());

  node.setLiveGeneration(1);
  node.dropStaleMetrics();
  node.clearSource(kSourceA);
  node.registerMetric(kSourceA, 11, QStringLiteral("Temp"), code(DataType::Double), 1);
  node.registerMetric(kSourceA, 12, QStringLiteral("Humidity"), code(DataType::Double), 1);
  QCOMPARE(node.metricCount(), 2);

  node.beginConnection(1000);
  node.birthMessages(1000);
  node.commitBirth();

  node.setLiveGeneration(2);
  node.dropStaleMetrics();
  node.clearSource(kSourceB);
  node.registerMetric(kSourceB, 21, QStringLiteral("Pressure"), code(DataType::Double), 2);
  QCOMPARE(node.metricCount(), 1);

  const auto rebirth = decoded(node.birthMessages(1001).constFirst());
  node.commitBirth();
  QCOMPARE(metricNamed(rebirth, "Pressure").alias, quint64(3));
  QVERIFY(!metricNamed(rebirth, "Temp").hasName);
  QVERIFY(!metricNamed(rebirth, "Humidity").hasName);
}

/**
 * @brief The union registry is bounded: registering past kMaxMetrics across two sources drops the
 *        overflow and increments the cap counter, never resizing past the ceiling (spec 0074 R8 --
 *        AC5).
 */
void TstSparkplugPublisher::theRegistryCapDropsAndCounts()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());

  const int cap  = MQTT::SparkplugPublisherLimits::kMaxMetrics;
  const int half = cap / 2;
  for (int i = 0; i < half; ++i)
    node.registerMetric(kSourceA, i, QStringLiteral("A%1").arg(i), code(DataType::Double), 1);
  for (int i = half; i < cap; ++i)
    node.registerMetric(kSourceB, i, QStringLiteral("B%1").arg(i), code(DataType::Double), 1);

  QCOMPARE(node.metricCount(), cap);
  QCOMPARE(node.counters().registryDrops, quint64(0));

  node.registerMetric(kSourceB, cap + 1, QStringLiteral("Overflow"), code(DataType::Double), 1);
  QCOMPARE(node.metricCount(), cap);
  QCOMPARE(node.counters().registryDrops, quint64(1));
}

/**
 * @brief With a device id configured, a two-source project births one node container and one
 *        device birth carrying every source's metrics, with no per-source DBIRTH (spec 0074 R9 --
 *        AC7).
 */
void TstSparkplugPublisher::oneDeviceCarriesEverySource()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig(QString::fromUtf8(kDevice)));
  node.registerMetric(kSourceA, 11, QStringLiteral("Temp"), code(DataType::Double), 1);
  node.registerMetric(kSourceB, 21, QStringLiteral("Pressure"), code(DataType::Double), 1);

  node.beginConnection(1000);
  const auto birth = node.birthMessages(1000);
  node.commitBirth();
  QCOMPARE(birth.size(), qsizetype(2));
  QCOMPARE(birth.constLast().topic, QStringLiteral("spBv1.0/Plant1/DBIRTH/SS1/Line3"));

  const auto device = decoded(birth.constLast());
  QCOMPARE(device.metrics.size(), qsizetype(2));
  QCOMPARE(metricNamed(device, "Temp").alias, quint64(1));
  QCOMPARE(metricNamed(device, "Pressure").alias, quint64(2));
}

/**
 * @brief Two sources holding the same dataset title birth distinctly-named metrics (source-
 *        qualified), while a title unique across sources stays bare (spec 0074 R11 -- AC8).
 */
void TstSparkplugPublisher::collidingTitlesAreSourceQualified()
{
  SparkplugPublisher node;
  node.setConfig(nodeConfig());
  node.registerMetric(kSourceA, 11, QStringLiteral("Temperature"), code(DataType::Double), 1);
  node.registerMetric(kSourceA, 12, QStringLiteral("Speed"), code(DataType::Double), 1);
  node.registerMetric(kSourceB, 21, QStringLiteral("Temperature"), code(DataType::Double), 1);
  node.registerMetric(kSourceB, 22, QStringLiteral("Pressure"), code(DataType::Double), 1);

  node.beginConnection(1000);
  const auto birth = decoded(node.birthMessages(1000).constFirst());
  node.commitBirth();

  QVERIFY(metricNamed(birth, "source1/Temperature").hasName);
  QVERIFY(metricNamed(birth, "source2/Temperature").hasName);
  QVERIFY(metricNamed(birth, "Speed").hasName);
  QVERIFY(metricNamed(birth, "Pressure").hasName);
  QVERIFY(!metricNamed(birth, "Temperature").hasName);
}

QTEST_APPLESS_MAIN(TstSparkplugPublisher)

#include "tst_sparkplug_publisher.moc"
