/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "MQTT/SparkplugPublisher.h"

#include <cmath>
#include <utility>

#include "Core/SSAssert.h"

namespace SpPubLimits = MQTT::SparkplugPublisherLimits;
namespace Sparkplug   = IO::Drivers::SparkplugB;

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Verbs the edge node publishes under; the namespace element and modulus come from SparkplugLimits
static constexpr QStringView kVerbNodeBirth(u"NBIRTH");
static constexpr QStringView kVerbNodeDeath(u"NDEATH");
static constexpr QStringView kVerbNodeData(u"NDATA");
static constexpr QStringView kVerbNodeCommand(u"NCMD");
static constexpr QStringView kVerbDeviceBirth(u"DBIRTH");
static constexpr QStringView kVerbDeviceData(u"DDATA");

// Sparkplug pins the delivery flags per message class, so they are not caller policy
static constexpr quint8 kDataQos  = 0;
static constexpr quint8 kDeathQos = 1;

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clamps a metric name to what the codec can ship, cutting back to a UTF-8 boundary so the
 *        truncated name stays valid text. A name past the cap would otherwise fail to encode and
 *        the metric would vanish from the birth certificate without a trace.
 */
static QString clampIdentity(const QString& name)
{
  QByteArray utf8 = name.toUtf8();
  if (utf8.size() <= Sparkplug::kMaxIdentityBytes)
    return name;

  qsizetype cut = Sparkplug::kMaxIdentityBytes;
  while (cut > 0 && (static_cast<quint8>(utf8.at(cut)) & 0xC0u) == 0x80u)
    --cut;

  utf8.truncate(cut);
  return QString::fromUtf8(utf8);
}

/**
 * @brief Builds one metric header carrying a name, a datatype and nothing else, which is the shape
 *        of the node-level controls a birth certificate declares.
 */
static Sparkplug::Metric namedMetric(const char* name, quint32 datatype, quint64 timestampMs)
{
  SS_ASSERT(name != nullptr, return Sparkplug::Metric());
  SS_ASSERT_LOG(Sparkplug::isSupportedDataType(datatype));

  Sparkplug::Metric metric;
  metric.name        = QString::fromLatin1(name);
  metric.hasName     = true;
  metric.datatype    = datatype;
  metric.kind        = Sparkplug::kindForDataType(datatype);
  metric.timestampMs = timestampMs;
  return metric;
}

//==================================================================================================
// Construction and configuration
//==================================================================================================

/**
 * @brief Builds an unconfigured, unborn edge node with an empty registry.
 */
MQTT::SparkplugPublisher::SparkplugPublisher()
  : m_born(false)
  , m_seq(0)
  , m_bdSeq(0)
  , m_bdSeqAssigned(false)
  , m_registryDirty(false)
  , m_nextAlias(1)
  , m_liveGeneration(0)
{}

/**
 * @brief Returns the edge-node identity currently published under.
 */
const MQTT::SparkplugPublisher::Config& MQTT::SparkplugPublisher::config() const noexcept
{
  return m_config;
}

/**
 * @brief Returns the pulled diagnostic counters.
 */
const MQTT::SparkplugPublisher::Counters& MQTT::SparkplugPublisher::counters() const noexcept
{
  return m_counters;
}

/**
 * @brief Whether the configuration names a publishable edge node: Sparkplug on, and both the
 *        group and the edge node identified. The device id stays optional.
 */
bool MQTT::SparkplugPublisher::valid() const noexcept
{
  return m_config.enabled && !m_config.groupId.isEmpty() && !m_config.edgeNodeId.isEmpty();
}

/**
 * @brief Whether a birth certificate has been issued for the current connection.
 */
bool MQTT::SparkplugPublisher::born() const noexcept
{
  return m_born;
}

/**
 * @brief Returns the sequence number the next published message will carry.
 */
quint64 MQTT::SparkplugPublisher::seq() const noexcept
{
  return m_seq;
}

/**
 * @brief Returns the birth/death sequence number of the current connection.
 */
quint64 MQTT::SparkplugPublisher::bdSeq() const noexcept
{
  return m_bdSeq;
}

/**
 * @brief Returns how many datasets are registered as metrics.
 */
int MQTT::SparkplugPublisher::metricCount() const noexcept
{
  return static_cast<int>(m_metrics.size());
}

/**
 * @brief Returns the newest frame-pool generation the registry has adopted. The reconcile compares
 *        an incoming structure's generation against this to decide whether a wholesale change means
 *        stale sources must be dropped (spec 0074 R7).
 */
quint64 MQTT::SparkplugPublisher::liveGeneration() const noexcept
{
  return m_liveGeneration;
}

/**
 * @brief Whether metrics were registered after the last birth, so the host is holding a birth
 *        certificate that no longer describes the node and needs a fresh one.
 */
bool MQTT::SparkplugPublisher::needsRebirth() const noexcept
{
  return m_born && m_registryDirty;
}

/**
 * @brief Drops the whole edge-node state: registry, aliases, counters and both sequence counters.
 *        Used when Sparkplug is switched off or reconfigured onto a different node identity.
 */
void MQTT::SparkplugPublisher::reset()
{
  SS_ASSERT_LOG(m_metrics.size() <= SpPubLimits::kMaxMetrics);

  m_index.clear();
  m_metrics.clear();
  m_aliasByUniqueId.clear();
  m_counters       = Counters();
  m_seq            = 0;
  m_bdSeq          = 0;
  m_born           = false;
  m_bdSeqAssigned  = false;
  m_registryDirty  = false;
  m_nextAlias      = 1;
  m_liveGeneration = 0;
}

/**
 * @brief Drops the whole metric table but keeps the alias map, so a uniqueId re-registered after
 *        the clear recovers its old alias (aliases retire only on reset(), R10). Emptying the table
 *        invalidates a held birth, so it marks the registry dirty; per-source scoping goes through
 *        clearSource, and this whole-table clear stays for reset() and the single-source rebuild.
 */
void MQTT::SparkplugPublisher::clearRegistry()
{
  SS_ASSERT_LOG(m_metrics.size() <= SpPubLimits::kMaxMetrics);

  m_index.clear();
  m_metrics.clear();
  m_registryDirty = m_born;
}

/**
 * @brief Adopts a new edge-node identity. A change of identity is a different node on the wire, so
 *        the whole state machine restarts; an unchanged configuration is left alone precisely so a
 *        routine settings push cannot bump `bdSeq` behind the host's back.
 */
void MQTT::SparkplugPublisher::setConfig(const Config& config)
{
  SS_ASSERT_LOG(m_bdSeq < SpPubLimits::kSeqModulus);

  const bool same = config.enabled == m_config.enabled && config.groupId == m_config.groupId
                 && config.edgeNodeId == m_config.edgeNodeId
                 && config.deviceId == m_config.deviceId;
  if (same)
    return;

  m_config = config;
  reset();
}

//==================================================================================================
// Metric registry
//==================================================================================================

/**
 * @brief Returns the alias a uniqueId already owns, or assigns the next monotonic one (from 1) and
 *        records it. Decoupling the alias from storage position lets a per-source clear remove
 *        middle entries without renumbering survivors, and keeps a uniqueId on the same alias
 *        across a rebuild (R5, R10); the same order yields the 1,2,3... table 0073 had (R6).
 */
quint64 MQTT::SparkplugPublisher::aliasFor(int uniqueId)
{
  SS_ASSERT(uniqueId >= 0, return 0);
  SS_ASSERT_LOG(m_nextAlias >= 1);

  const auto it = m_aliasByUniqueId.constFind(uniqueId);
  if (it != m_aliasByUniqueId.constEnd())
    return it.value();

  const quint64 alias = m_nextAlias++;
  m_aliasByUniqueId.insert(uniqueId, alias);
  return alias;
}

/**
 * @brief Rebuilds the uniqueId->index map from the current metric vector after a removal, so a
 *        later lookup still lands on the right entry once the table has been compacted.
 */
void MQTT::SparkplugPublisher::rebuildIndex()
{
  SS_ASSERT_LOG(m_metrics.size() <= SpPubLimits::kMaxMetrics);

  m_index.clear();
  m_index.reserve(m_metrics.size());
  for (int i = 0; i < m_metrics.size(); ++i)
    m_index.insert(m_metrics.at(i).uniqueId, i);
}

/**
 * @brief Registers a dataset as a metric, or updates one already registered. The alias comes from
 *        aliasFor and is never reassigned; the source and generation are restamped every pass so a
 *        per-source clear and a stale-generation drop can scope themselves. A registration past the
 *        cap is refused and counted, never resized into (R8).
 */
void MQTT::SparkplugPublisher::registerMetric(
  int sourceId, int uniqueId, const QString& name, quint32 datatype, quint64 generation)
{
  SS_ASSERT(uniqueId >= 0, return);
  SS_ASSERT(!name.isEmpty(), return);

  const auto it = m_index.constFind(uniqueId);
  if (it != m_index.constEnd()) {
    auto& known         = m_metrics[it.value()];
    const QString ident = clampIdentity(name);
    const auto kind     = Sparkplug::kindForDataType(datatype);
    const bool moved    = known.name != ident || known.datatype != datatype;

    if (known.kind != kind) {
      known.numericValue = 0.0;
      known.boolValue    = false;
      known.stringValue.clear();
      known.hasValue = false;
    }

    known.name       = ident;
    known.datatype   = datatype;
    known.kind       = kind;
    known.sourceId   = sourceId;
    known.generation = generation;
    if (moved)
      m_registryDirty = true;

    return;
  }

  if (m_metrics.size() >= SpPubLimits::kMaxMetrics) {
    ++m_counters.registryDrops;
    return;
  }

  MetricEntry entry;
  entry.name       = clampIdentity(name);
  entry.datatype   = datatype;
  entry.kind       = Sparkplug::kindForDataType(datatype);
  entry.uniqueId   = uniqueId;
  entry.sourceId   = sourceId;
  entry.generation = generation;
  entry.alias      = aliasFor(uniqueId);

  m_index.insert(uniqueId, static_cast<int>(m_metrics.size()));
  m_metrics.append(std::move(entry));
  m_registryDirty = true;
}

/**
 * @brief Removes every metric owned by one source and rebuilds the index. The survivors keep their
 *        aliases because the alias lives in the entry and in aliasFor's map, not in the vector
 *        position, so re-registering one source never renumbers another's metrics (spec 0074 R3).
 */
void MQTT::SparkplugPublisher::clearSource(int sourceId)
{
  SS_ASSERT_LOG(m_metrics.size() <= SpPubLimits::kMaxMetrics);
  SS_ASSERT_LOG(sourceId >= -1);

  const auto before = m_metrics.size();
  m_metrics.removeIf([sourceId](const MetricEntry& e) { return e.sourceId == sourceId; });
  if (m_metrics.size() == before)
    return;

  rebuildIndex();
  m_registryDirty = m_born;
}

/**
 * @brief Drops every metric older than the newest adopted generation -- exactly the sources that
 *        did not republish, i.e. a project swap's departed sources (spec 0074 R7). Only strictly-
 *        older entries go, so a source republishing at the newest generation is never removed; the
 *        worst case is a redundant rebirth, never a lost live metric.
 */
void MQTT::SparkplugPublisher::dropStaleMetrics()
{
  SS_ASSERT_LOG(m_metrics.size() <= SpPubLimits::kMaxMetrics);
  SS_ASSERT_LOG(m_index.size() == m_metrics.size());

  const auto live   = m_liveGeneration;
  const auto before = m_metrics.size();
  m_metrics.removeIf([live](const MetricEntry& e) { return e.generation < live; });
  if (m_metrics.size() == before)
    return;

  rebuildIndex();
  m_registryDirty = true;
}

/**
 * @brief Adopts a frame-pool generation from the reconcile. It only ever advances: the FrameBuilder
 *        owns the generation and a rewind would drop live metrics, so a value below the current one
 *        is refused rather than applied (spec 0074, generation is adopted, never invented).
 */
void MQTT::SparkplugPublisher::setLiveGeneration(quint64 generation)
{
  SS_ASSERT(generation >= m_liveGeneration, return);
  SS_ASSERT_LOG(m_metrics.size() <= SpPubLimits::kMaxMetrics);

  m_liveGeneration = generation;
}

/**
 * @brief Latches a numeric value, marking the metric changed only when it actually moved.
 */
void MQTT::SparkplugPublisher::applyNumeric(MetricEntry& entry, double value)
{
  SS_ASSERT_LOG(entry.kind == Sparkplug::ValueKind::Numeric);
  SS_ASSERT_LOG(std::isfinite(value));

  if (entry.hasValue && entry.numericValue == value)
    return;

  entry.numericValue = value;
  entry.hasValue     = true;
  entry.changed      = true;
}

/**
 * @brief Latches a boolean value, marking the metric changed only when it actually moved.
 */
void MQTT::SparkplugPublisher::applyBoolean(MetricEntry& entry, bool value)
{
  SS_ASSERT_LOG(entry.kind == Sparkplug::ValueKind::Boolean);
  SS_ASSERT_LOG(entry.datatype == static_cast<quint32>(Sparkplug::DataType::Boolean));

  if (entry.hasValue && entry.boolValue == value)
    return;

  entry.boolValue = value;
  entry.hasValue  = true;
  entry.changed   = true;
}

/**
 * @brief Latches a string value, marking the metric changed only when it actually moved.
 */
void MQTT::SparkplugPublisher::applyString(MetricEntry& entry, const QString& value)
{
  SS_ASSERT_LOG(entry.kind == Sparkplug::ValueKind::String);
  SS_ASSERT_LOG(entry.datatype != 0);

  if (entry.hasValue && entry.stringValue == value)
    return;

  entry.stringValue = value;
  entry.hasValue    = true;
  entry.changed     = true;
}

/**
 * @brief Feeds one dataset sample into its metric, routing it by the declared datatype. A value
 *        the declared type cannot carry (a text sample under a numeric code, a non-finite or
 *        out-of-range number) is skipped and counted rather than published mistyped (R44).
 */
void MQTT::SparkplugPublisher::updateValue(int uniqueId,
                                           double numericValue,
                                           const QString& text,
                                           bool numeric)
{
  SS_ASSERT(uniqueId >= 0, return);
  SS_ASSERT_LOG(m_metrics.size() <= SpPubLimits::kMaxMetrics);

  const auto it = m_index.constFind(uniqueId);
  if (it == m_index.constEnd())
    return;

  auto& entry = m_metrics[it.value()];
  if (entry.kind == Sparkplug::ValueKind::String) {
    applyString(entry, text);
    return;
  }

  const bool usable = numeric && std::isfinite(numericValue);
  if (entry.kind == Sparkplug::ValueKind::Boolean && usable) {
    applyBoolean(entry, numericValue != 0.0);
    return;
  }

  if (!usable || !Sparkplug::numericFitsDataType(entry.datatype, numericValue)) {
    ++m_counters.skippedValues;
    return;
  }

  applyNumeric(entry, numericValue);
}

/**
 * @brief Clears every change mark, which is what a birth or a data publish owes the registry: the
 *        values it just carried are no longer pending.
 */
void MQTT::SparkplugPublisher::clearChanged()
{
  SS_ASSERT_LOG(m_metrics.size() <= SpPubLimits::kMaxMetrics);

  for (auto& entry : m_metrics)
    entry.changed = false;
}

//==================================================================================================
// Payload construction
//==================================================================================================

/**
 * @brief Returns the sequence number for the next message and advances the counter modulo 256,
 *        which every published message shares, births included (R41).
 */
quint64 MQTT::SparkplugPublisher::nextSeq()
{
  SS_ASSERT_LOG(m_seq < SpPubLimits::kSeqModulus);

  const quint64 current = m_seq % SpPubLimits::kSeqModulus;
  m_seq                 = (current + 1) % SpPubLimits::kSeqModulus;
  return current;
}

/**
 * @brief Builds a node-level topic: spBv1.0/<group>/<verb>/<edge node>.
 */
QString MQTT::SparkplugPublisher::topicFor(QStringView verb) const
{
  SS_ASSERT(!verb.isEmpty(), return QString());
  SS_ASSERT(valid(), return QString());

  QString topic;
  topic.reserve(64);
  topic.append(QLatin1StringView(IO::Drivers::SparkplugLimits::kNamespace));
  topic.append(QLatin1Char('/'));
  topic.append(m_config.groupId);
  topic.append(QLatin1Char('/'));
  topic.append(verb);
  topic.append(QLatin1Char('/'));
  topic.append(m_config.edgeNodeId);
  return topic;
}

/**
 * @brief Builds a device-level topic: the node topic with the device id appended.
 */
QString MQTT::SparkplugPublisher::deviceTopicFor(QStringView verb) const
{
  SS_ASSERT(!m_config.deviceId.isEmpty(), return QString());
  SS_ASSERT(valid(), return QString());

  QString topic = topicFor(verb);
  topic.append(QLatin1Char('/'));
  topic.append(m_config.deviceId);
  return topic;
}

/**
 * @brief Returns the NCMD topic a host addresses this node's commands to.
 */
QString MQTT::SparkplugPublisher::commandTopic() const
{
  return topicFor(kVerbNodeCommand);
}

/**
 * @brief Builds the bdSeq metric both the birth certificate and the death will carry; a host pairs
 *        the two by this number, so the same value must appear in both (R41).
 */
IO::Drivers::SparkplugB::Metric MQTT::SparkplugPublisher::bdSeqMetric(quint64 timestampMs) const
{
  SS_ASSERT_LOG(m_bdSeq < SpPubLimits::kSeqModulus);

  auto metric         = namedMetric(SpPubLimits::kBirthDeathSequence,
                            static_cast<quint32>(Sparkplug::DataType::Int64),
                            timestampMs);
  metric.numericValue = static_cast<double>(m_bdSeq);
  return metric;
}

/**
 * @brief Builds the Node Control/Rebirth metric a birth declares so a host knows the node accepts
 *        a rebirth command (R43).
 */
IO::Drivers::SparkplugB::Metric MQTT::SparkplugPublisher::rebirthControlMetric(
  quint64 timestampMs) const
{
  SS_ASSERT_LOG(Sparkplug::kRebirthMetricName != nullptr);

  auto metric = namedMetric(
    Sparkplug::kRebirthMetricName, static_cast<quint32>(Sparkplug::DataType::Boolean), timestampMs);
  metric.boolValue = false;
  return metric;
}

/**
 * @brief Resolves the name a birth declares: the bare title, unless another source holds the same
 *        title, when it is qualified "source<sourceId>/<title>" so the two stay distinct (R11).
 *        Only a cross-source clash qualifies, so single-source names are never touched (R6); the
 *        prefix carries the unique owning source id, so a qualified name never itself clashes.
 */
QString MQTT::SparkplugPublisher::resolveMetricName(const MetricEntry& entry) const
{
  SS_ASSERT_LOG(!entry.name.isEmpty());
  SS_ASSERT_LOG(m_metrics.size() <= SpPubLimits::kMaxMetrics);

  bool collides = false;
  for (const auto& other : m_metrics)
    collides = collides || (other.sourceId != entry.sourceId && other.name == entry.name);

  if (!collides)
    return entry.name;

  const QString qualified =
    QStringLiteral("source%1/%2").arg(QString::number(entry.sourceId), entry.name);
  return clampIdentity(qualified);
}

/**
 * @brief Renders one registered metric onto the wire. A birth carries the name and the alias so a
 *        host can bind them; a data message carries the alias alone, which is the whole point of
 *        the alias table (R40). The birth name is resolved so a cross-source title clash is
 *        qualified, while the data path stays alias-only and never pays for the resolution.
 */
IO::Drivers::SparkplugB::Metric MQTT::SparkplugPublisher::metricFor(const MetricEntry& entry,
                                                                    quint64 timestampMs,
                                                                    bool withName) const
{
  SS_ASSERT_LOG(entry.alias > 0);
  SS_ASSERT_LOG(!entry.name.isEmpty());

  Sparkplug::Metric metric;
  metric.name         = withName ? resolveMetricName(entry) : QString();
  metric.hasName      = withName;
  metric.alias        = entry.alias;
  metric.hasAlias     = true;
  metric.datatype     = entry.datatype;
  metric.kind         = entry.kind;
  metric.timestampMs  = timestampMs;
  metric.isNull       = !entry.hasValue;
  metric.numericValue = entry.numericValue;
  metric.stringValue  = entry.stringValue;
  metric.boolValue    = entry.boolValue;
  return metric;
}

/**
 * @brief Appends every registered metric to a birth payload, names and aliases included.
 */
void MQTT::SparkplugPublisher::appendBirthMetrics(IO::Drivers::SparkplugB::Payload& payload,
                                                  quint64 timestampMs)
{
  SS_ASSERT(payload.metrics.size() <= SpPubLimits::kMaxMetrics, return);
  SS_ASSERT_LOG(m_metrics.size() <= SpPubLimits::kMaxMetrics);

  payload.metrics.reserve(payload.metrics.size() + m_metrics.size());
  for (const auto& entry : m_metrics)
    payload.metrics.append(metricFor(entry, timestampMs, true));
}

/**
 * @brief Stamps a payload with its sequence number and encodes it onto its topic. Every published
 *        message goes through here, births included, which is what makes the modulo-256 sequence
 *        continuous (R41). The death will is the one payload built without a sequence: it is
 *        registered before the connection exists, so it has no place in that connection's run.
 */
MQTT::SparkplugPublisher::Message MQTT::SparkplugPublisher::encode(
  const QString& topic, IO::Drivers::SparkplugB::Payload& payload)
{
  SS_ASSERT(!topic.isEmpty(), return Message());
  SS_ASSERT_LOG(payload.hasTimestamp);

  payload.hasSeq = true;
  payload.seq    = nextSeq();

  Message message;
  message.topic   = topic;
  message.payload = Sparkplug::encodePayload(payload);
  message.qos     = kDataQos;
  message.retain  = false;
  return message;
}

//==================================================================================================
// Lifecycle
//==================================================================================================

/**
 * @brief Builds the NDEATH payload for the current connection: a timestamp and the bdSeq metric,
 *        with no sequence number. The caller registers it as the MQTT will BEFORE connecting, so
 *        an ungraceful disconnect marks the node offline without this process publishing anything.
 */
MQTT::SparkplugPublisher::Message MQTT::SparkplugPublisher::deathCertificate(
  quint64 timestampMs) const
{
  SS_ASSERT(valid(), return Message());
  SS_ASSERT_LOG(m_bdSeq < SpPubLimits::kSeqModulus);

  Sparkplug::Payload payload;
  payload.hasTimestamp = true;
  payload.timestampMs  = timestampMs;
  payload.metrics.append(bdSeqMetric(timestampMs));

  Message message;
  message.topic   = topicFor(kVerbNodeDeath);
  message.payload = Sparkplug::encodePayload(payload);
  message.qos     = kDeathQos;
  message.retain  = false;
  return message;
}

/**
 * @brief Opens a new connection: next bdSeq, sequence reset to zero, adopted generation dropped so
 *        the first structure republish re-anchors it; the birth that follows carries the same
 *        bdSeq (R41, R42). The alias map is deliberately kept, since a reconnect re-births and
 *        re-declares every alias; the full alias reset lives in reset() (R10).
 */
MQTT::SparkplugPublisher::Message MQTT::SparkplugPublisher::beginConnection(quint64 timestampMs)
{
  SS_ASSERT(valid(), return Message());
  SS_ASSERT_LOG(m_bdSeq < SpPubLimits::kSeqModulus);

  if (m_bdSeqAssigned)
    m_bdSeq = (m_bdSeq + 1) % SpPubLimits::kSeqModulus;

  m_bdSeqAssigned  = true;
  m_seq            = 0;
  m_born           = false;
  m_liveGeneration = 0;
  return deathCertificate(timestampMs);
}

/**
 * @brief Builds the birth certificate: an NBIRTH with bdSeq, the rebirth control and every metric's
 *        identity, plus a DBIRTH when a device is configured (R40). Only commitBirth marks the node
 *        born. The sequence restarts at ZERO here, not in beginConnection: a rebirth is an NBIRTH
 *        too, and every NBIRTH carries seq 0 (Sparkplug B 3.0 tck-id-payloads-sequence-num-zero).
 */
QVector<MQTT::SparkplugPublisher::Message> MQTT::SparkplugPublisher::birthMessages(
  quint64 timestampMs)
{
  SS_ASSERT(valid(), return QVector<Message>());
  SS_ASSERT_LOG(m_metrics.size() <= SpPubLimits::kMaxMetrics);

  m_seq             = 0;
  const bool device = !m_config.deviceId.isEmpty();
  QVector<Message> out;
  out.reserve(2);

  Sparkplug::Payload node;
  node.hasTimestamp = true;
  node.timestampMs  = timestampMs;
  node.metrics.append(bdSeqMetric(timestampMs));
  node.metrics.append(rebirthControlMetric(timestampMs));
  if (!device)
    appendBirthMetrics(node, timestampMs);

  out.append(encode(topicFor(kVerbNodeBirth), node));

  if (device) {
    Sparkplug::Payload unit;
    unit.hasTimestamp = true;
    unit.timestampMs  = timestampMs;
    appendBirthMetrics(unit, timestampMs);
    out.append(encode(deviceTopicFor(kVerbDeviceBirth), unit));
  }

  return out;
}

/**
 * @brief Commits a birth the caller has confirmed on the wire: marks the node born, clears the
 *        registry-dirty flag the rebirth trigger reads, and drops the change marks the birth just
 *        carried. Called only after every birth message published, so DDATA never rides aliases a
 *        host never received (R40).
 */
void MQTT::SparkplugPublisher::commitBirth()
{
  SS_ASSERT(valid(), return);
  SS_ASSERT_LOG(m_metrics.size() <= SpPubLimits::kMaxMetrics);

  clearChanged();
  m_born          = true;
  m_registryDirty = false;
  ++m_counters.births;
}

/**
 * @brief Builds the data message for everything that moved since the last publish: metrics
 *        addressed by alias, no names on the wire, and nothing at all when nothing changed. Data
 *        before a birth is not published, because a host cannot resolve aliases it never saw.
 */
QVector<MQTT::SparkplugPublisher::Message> MQTT::SparkplugPublisher::dataMessages(
  quint64 timestampMs)
{
  SS_ASSERT(valid(), return QVector<Message>());
  SS_ASSERT_LOG(m_metrics.size() <= SpPubLimits::kMaxMetrics);

  QVector<Message> out;
  if (!m_born)
    return out;

  Sparkplug::Payload payload;
  payload.hasTimestamp = true;
  payload.timestampMs  = timestampMs;
  for (auto& entry : m_metrics) {
    if (!entry.changed)
      continue;

    payload.metrics.append(metricFor(entry, timestampMs, false));
    entry.changed = false;
  }

  if (payload.metrics.isEmpty())
    return out;

  const bool device   = !m_config.deviceId.isEmpty();
  const QString topic = device ? deviceTopicFor(kVerbDeviceData) : topicFor(kVerbNodeData);

  out.append(encode(topic, payload));
  ++m_counters.dataMessages;
  return out;
}

/**
 * @brief Whether an inbound command payload asks for a rebirth. Every other command is counted and
 *        ignored, and a payload that does not decode is counted as such: the broker is untrusted
 *        input, so neither case may pass silently (R43).
 */
bool MQTT::SparkplugPublisher::isRebirthCommand(QByteArrayView payload)
{
  SS_ASSERT(payload.size() >= 0, return false);
  SS_ASSERT_LOG(Sparkplug::kRebirthMetricName != nullptr);

  Sparkplug::Payload decoded;
  QString error;
  if (!Sparkplug::decodePayload(payload, decoded, &error)) {
    ++m_counters.commandDecodeErrors;
    return false;
  }

  const auto wanted = QLatin1StringView(Sparkplug::kRebirthMetricName);
  for (const auto& metric : decoded.metrics) {
    const bool match = metric.hasName && metric.name == wanted;
    if (match && metric.kind == Sparkplug::ValueKind::Boolean && metric.boolValue) {
      ++m_counters.rebirthCommands;
      return true;
    }
  }

  ++m_counters.ignoredCommands;
  return false;
}
