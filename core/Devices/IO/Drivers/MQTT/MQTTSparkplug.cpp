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

#include <chrono>
#include <QDateTime>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QLoggingCategory>
#include <QVector>
#include <utility>

#include "Core/DataModel/Frame.h"
#include "Core/Prompt/UserPrompt.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "IO/Drivers/MQTT.h"

Q_DECLARE_LOGGING_CATEGORY(lcMqttSub)

namespace SpLimits  = IO::Drivers::SparkplugLimits;
namespace Sparkplug = IO::Drivers::SparkplugB;
namespace SpWire    = IO::Drivers::OpcUaWire;

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kSparkplugTickMs       = 33;
static constexpr int kSparkplugDiagTicks    = 30;
static constexpr qint64 kSparkplugNsPerMs   = 1000000LL;
static constexpr qint64 kSparkplugRebirthMs = 5000;
static constexpr qint64 kSparkplugMaxSkewNs = 5000LL * kSparkplugNsPerMs;

// Largest epoch stamp the nanosecond mapping can hold; a broker stamp past it is not mapped
static constexpr quint64 kSparkplugMaxStampMs = 1ULL << 43;

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Current steady-clock reading in nanoseconds, the unit both the mapped broker stamp and
 *        the monotonic clamp work in.
 */
static qint64 sparkplugSteadyNs()
{
  const auto now = IO::CapturedData::SteadyClock::now();
  const auto ns  = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch());

  SS_ASSERT_LOG(ns.count() >= 0);
  return ns.count();
}

/**
 * @brief Reads a driver-property value back as a JSON array. A project round-trip hands the value
 *        through QVariant, which turns a stored array into a QVariantList rather than a QJsonArray.
 */
[[nodiscard]] static QJsonArray sparkplugJsonArray(const QVariant& value)
{
  if (value.canConvert<QJsonArray>())
    return value.toJsonArray();

  if (value.typeId() != QMetaType::QVariantList)
    return {};

  QJsonArray array;
  const auto list = value.toList();
  for (const auto& item : list)
    array.append(QJsonValue::fromVariant(item));

  return array;
}

/**
 * @brief Wire type a latched slot encodes as; a slot with no value channel has none.
 */
static SpWire::Type sparkplugWireType(Sparkplug::ValueKind kind) noexcept
{
  switch (kind) {
    case Sparkplug::ValueKind::Numeric:
      return SpWire::Type::F64;
    case Sparkplug::ValueKind::Boolean:
      return SpWire::Type::Bool;
    case Sparkplug::ValueKind::String:
      return SpWire::Type::Str;
    case Sparkplug::ValueKind::None:
      break;
  }

  return SpWire::Type::Invalid;
}

/**
 * @brief Value channel the slot's kind names, as the encoder consumes it.
 */
static QVariant sparkplugValue(const IO::Drivers::SparkplugSession::SlotValue& slot)
{
  SS_ASSERT_LOG(slot.index >= 0);
  SS_ASSERT_LOG(slot.kind != Sparkplug::ValueKind::None);

  switch (slot.kind) {
    case Sparkplug::ValueKind::Numeric:
      return QVariant(slot.num);
    case Sparkplug::ValueKind::Boolean:
      return QVariant(slot.b);
    case Sparkplug::ValueKind::String:
      return QVariant(slot.str);
    case Sparkplug::ValueKind::None:
      break;
  }

  return {};
}

/**
 * @brief Group one slot belongs to: the edge node alone for a node metric, node and device for a
 *        device metric, so a device never collides with the node that owns it.
 */
static QString sparkplugGroupTitle(const IO::Drivers::SparkplugSession::SlotValue& slot)
{
  SS_ASSERT_LOG(!slot.node.isEmpty());
  SS_ASSERT_LOG(slot.index >= 0);

  if (slot.device.isEmpty())
    return slot.node;

  return QStringLiteral("%1 / %2").arg(slot.node, slot.device);
}

/**
 * @brief One dataset for a latched slot: LED for booleans (the synthetic Online metric included),
 *        plot for numerics, plain display for strings and for a slot no value has typed yet. The
 *        frame index is the wire index plus one, because the latched row is 1-based to the parser.
 */
static DataModel::Dataset sparkplugDataset(const IO::Drivers::SparkplugSession::SlotValue& slot)
{
  SS_ASSERT_LOG(slot.index >= 0);
  SS_ASSERT_LOG(!slot.name.isEmpty());

  DataModel::Dataset dataset;
  dataset.index = slot.index + 1;
  dataset.log   = true;
  dataset.title = slot.name;

  if (slot.kind == Sparkplug::ValueKind::Boolean) {
    dataset.led     = true;
    dataset.ledHigh = 1;
    dataset.wgtMax  = 1;
  } else if (slot.kind == Sparkplug::ValueKind::Numeric)
    dataset.plt = true;

  return dataset;
}

/**
 * @brief The `sparkplug` native template schema: one {index, name} entry per assigned slot, in
 *        wire order. The parser refuses a duplicate or out-of-range index, so the entries are the
 *        session's own dense indices and nothing is renumbered here.
 */
static QJsonArray sparkplugSchema(
  const QVector<IO::Drivers::SparkplugSession::SlotValue>& slotTable)
{
  SS_ASSERT_LOG(slotTable.size() <= SpLimits::kMaxSlots);

  QJsonArray schema;
  for (const auto& slot : slotTable) {
    SS_ASSERT_LOG(slot.index == schema.size());
    if (slot.index < 0 || slot.index >= SpWire::kMaxTags)
      break;

    schema.append(QJsonObject{
      {QStringLiteral("index"),       slot.index},
      { QStringLiteral("name"), slot.displayName}
    });
  }

  return schema;
}

/**
 * @brief One group per distinct (Sparkplug group, edge node, device) triple, in the order the
 *        births assigned the slots, with one dataset per metric of that triple. The Sparkplug group
 *        is part of the key because two of them may publish the same edge-node name.
 */
static QJsonArray sparkplugGroups(
  const QVector<IO::Drivers::SparkplugSession::SlotValue>& slotTable)
{
  SS_ASSERT_LOG(slotTable.size() <= SpLimits::kMaxSlots);

  QStringList order;
  QHash<QString, DataModel::Group> groups;
  for (const auto& slot : slotTable) {
    const QString key = slot.group + QLatin1Char('/') + slot.node + QLatin1Char('/') + slot.device;
    if (!groups.contains(key)) {
      DataModel::Group group;
      group.groupId = order.size();
      group.widget  = QStringLiteral("datagrid");
      group.title   = sparkplugGroupTitle(slot);
      groups.insert(key, group);
      order.append(key);
    }

    auto& group = groups[key];
    group.datasets.push_back(sparkplugDataset(slot));
  }

  QJsonArray array;
  for (const auto& key : order)
    array.append(DataModel::serialize(groups.value(key)));

  SS_ASSERT_LOG(array.size() == order.size());
  return array;
}

//--------------------------------------------------------------------------------------------------
// Session peer
//--------------------------------------------------------------------------------------------------

/**
 * @brief Points this UI-config instance at the per-source instance that owns the live link. The
 *        pane, the project generator and the API server all read the UI instance, but only the live
 *        one ever subscribes, so without this hop every birth certificate is invisible to them.
 */
void IO::Drivers::MQTT::setSparkplugPeer(MQTT* peer)
{
  SS_ASSERT(peer != this, return);
  SS_ASSERT_LOG(m_sparkplug.slotCount() >= 0);

  if (m_sparkplugPeer == peer)
    return;

  m_sparkplugPeer = peer;
}

/**
 * @brief The session a project or status query reads: the live link's when this instance is the
 *        UI-config one, its own otherwise. Only the UI instance ever carries a peer, so the hop is
 *        one deep, and the guarded pointer degrades to the empty local session on teardown.
 */
IO::Drivers::SparkplugSession& IO::Drivers::MQTT::sparkplugSession()
{
  SS_ASSERT_LOG(m_sparkplugPeer.data() != this);
  SS_ASSERT_LOG(m_sparkplug.slotCount() >= 0);

  if (m_sparkplugPeer)
    return m_sparkplugPeer->m_sparkplug;

  return m_sparkplug;
}

/**
 * @brief Const overload of the session accessor; see the non-const one.
 */
const IO::Drivers::SparkplugSession& IO::Drivers::MQTT::sparkplugSession() const
{
  SS_ASSERT_LOG(m_sparkplugPeer.data() != this);
  SS_ASSERT_LOG(m_sparkplug.slotCount() >= 0);

  if (m_sparkplugPeer)
    return m_sparkplugPeer->m_sparkplug;

  return m_sparkplug;
}

/**
 * @brief Number of metrics the births discovered, read through the session peer.
 */
int IO::Drivers::MQTT::sparkplugSlotCount() const
{
  SS_ASSERT_LOG(m_sparkplug.slotCount() >= 0);
  SS_ASSERT_LOG(sparkplugSession().slotCount() <= SpLimits::kMaxSlots);

  return sparkplugSession().slotCount();
}

/**
 * @brief Pulled session counters, read through the session peer (spec 0033).
 */
IO::Drivers::SparkplugSession::Counters IO::Drivers::MQTT::sparkplugCounters() const
{
  SS_ASSERT_LOG(m_sparkplug.slotCount() >= 0);
  SS_ASSERT_LOG(sparkplugSession().slotCount() <= SpLimits::kMaxSlots);

  return sparkplugSession().counters();
}

/**
 * @brief Whether a group id must be refused: it is interpolated straight into a topic filter, so a
 *        wildcard or a level separator would silently subscribe to the wrong namespace, or to none.
 */
bool IO::Drivers::MQTT::rejectSparkplugGroupId(const QString& groupId) const
{
  SS_ASSERT_LOG(groupId.size() >= 0);
  SS_ASSERT_LOG(m_sparkplugGroupId.size() >= 0);

  if (!groupId.contains(u'+') && !groupId.contains(u'#') && !groupId.contains(u'/'))
    return false;

  logDriverError(
    tr("Invalid Sparkplug Group ID"),
    tr("A group ID cannot contain '+', '#' or '/'; keeping \"%1\".").arg(m_sparkplugGroupId));
  return true;
}

//--------------------------------------------------------------------------------------------------
// Topic filter cache
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the effective topic filter and the matcher the per-message check uses, from the
 *        one namespace constant the topic parser also matches against. The cache is anchored to
 *        the setters that can change the filter, never to the connection: the second check exists
 *        to catch a live config edit, and one cached at subscribe time would go stale against it.
 */
void IO::Drivers::MQTT::refreshTopicFilterCache()
{
  SS_ASSERT_LOG(SpLimits::kNamespace != nullptr);
  SS_ASSERT_LOG(qstrlen(SpLimits::kNamespace) > 0);

  const QString ns = QString::fromLatin1(SpLimits::kNamespace);
  if (!m_sparkplugEnabled)
    m_effectiveFilter = m_topicFilter;
  else if (m_sparkplugGroupId.isEmpty())
    m_effectiveFilter = ns + QStringLiteral("/#");
  else
    m_effectiveFilter = ns + QStringLiteral("/") + m_sparkplugGroupId + QStringLiteral("/#");

  m_topicMatcher.setFilter(m_effectiveFilter);
}

//--------------------------------------------------------------------------------------------------
// Message routing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Routes one broker message. Sparkplug off, or a topic the session refuses as foreign,
 *        publishes the raw payload exactly as the subscriber always has; a consumed message
 *        publishes nothing here, because the tick owns the encoded frame.
 */
void IO::Drivers::MQTT::routeReceivedMessage(const QByteArray& message, const QMqttTopicName& topic)
{
  SS_ASSERT(!message.isEmpty(), return);
  SS_ASSERT_LOG(topic.isValid());

  if (!m_sparkplugEnabled) {
    publishReceivedData(message);
    return;
  }

  if (!m_sparkplug.ingest(topic.name(), message)) {
    publishReceivedData(message);
    return;
  }

  publishSparkplugRebirths();
}

/**
 * @brief Asks each desynced edge node to re-publish its birth certificate, one NCMD per node and
 *        at most one every five seconds. A topic the rate limit, the node cap or a failed publish
 *        leaves unsent stays queued and is retried: the session clears a node's flag on its next
 *        birth only, so dropping the topic here would leave that node desynced and never asked.
 */
void IO::Drivers::MQTT::publishSparkplugRebirths()
{
  SS_ASSERT_LOG(m_sparkplugEnabled);
  SS_ASSERT_LOG(m_lastRebirthMs.size() <= SpLimits::kMaxNodes);

  const auto drained = m_sparkplug.takeRebirthTopics();
  for (const auto& topic : drained)
    if (m_pendingRebirths.size() < SpLimits::kMaxNodes && !m_pendingRebirths.contains(topic))
      m_pendingRebirths.append(topic);

  if (m_pendingRebirths.isEmpty())
    return;

  QStringList unsent;
  const qint64 now = QDateTime::currentMSecsSinceEpoch();
  for (const auto& topic : std::as_const(m_pendingRebirths)) {
    const qint64 last   = m_lastRebirthMs.value(topic, 0);
    const bool tracked  = last != 0;
    const bool limited  = tracked && now - last < kSparkplugRebirthMs;
    const bool uncapped = tracked || m_lastRebirthMs.size() < SpLimits::kMaxNodes;
    if (limited || !uncapped) {
      unsent.append(topic);
      continue;
    }

    const auto payload = Sparkplug::encodeRebirthRequest(static_cast<quint64>(now));
    if (m_client.publish(QMqttTopicName(topic), payload, 0, false) < 0) {
      qCWarning(lcMqttSub) << "rebirth NCMD publish rejected for" << topic;
      unsent.append(topic);
      continue;
    }

    m_lastRebirthMs.insert(topic, now);
  }

  m_pendingRebirths.swap(unsent);
}

//--------------------------------------------------------------------------------------------------
// Delta-frame publishing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishing tick: encodes every slot the session latched since the last pass into one
 *        delta frame stamped with the earliest source time it carries. No dirty slot, no frame.
 *        The mode is state a UI toggle can flip under an armed timer, so it is a guard, not an
 *        assertion: an assertion condition runs in every build.
 */
void IO::Drivers::MQTT::onSparkplugTick()
{
  SS_ASSERT_LOG(m_sparkplugState.frame.size() <= SpWire::kMaxFrameBytes);
  SS_ASSERT_LOG(m_sparkplug.slotCount() <= SpLimits::kMaxSlots);

  if (!m_sparkplugEnabled)
    return;

  if (++m_sparkplugState.diagTicks >= kSparkplugDiagTicks)
    reportSparkplugDrops();

  if (!m_sparkplug.hasDirtySlots())
    return;

  SpWire::beginFrame(m_sparkplugState.frame);
  m_sparkplugState.earliestMs = 0;
  m_sparkplug.consumeDirtySlots(
    [this](const SparkplugSession::SlotValue& slot) { appendSparkplugSlot(slot); });

  flushSparkplugFrame();
}

/**
 * @brief Appends one latched slot to the frame under construction, flushing first when the entry
 *        would not fit: the session clears every change mark in one pass, so a slot that is not
 *        encoded now is lost rather than retried.
 */
void IO::Drivers::MQTT::appendSparkplugSlot(const SparkplugSession::SlotValue& slot)
{
  SS_ASSERT(slot.index >= 0 && slot.index < SpWire::kMaxTags, return);
  SS_ASSERT_LOG(m_sparkplugState.frame.size() >= SpWire::kHeaderBytes);

  const auto type = sparkplugWireType(slot.kind);
  if (type == SpWire::Type::Invalid)
    return;

  if (m_sparkplugState.frame.size() + SpWire::maxEntryBytes(type) > SpWire::kMaxFrameBytes)
    flushSparkplugFrame();

  SpWire::appendEntry(m_sparkplugState.frame, slot.index, type, sparkplugValue(slot));

  auto& earliest = m_sparkplugState.earliestMs;
  if (slot.timestampMs > 0 && (earliest == 0 || slot.timestampMs < earliest))
    earliest = slot.timestampMs;
}

/**
 * @brief Hands the staged frame to the pipeline and starts the next one. The stamp is taken here,
 *        before the queued hop to the pipeline thread, so the frame carries the source's time
 *        rather than the time the pipeline got around to it. The replacement buffer is reserved at
 *        the same ceiling the entry loop flushes on, so a busy tick never regrows it.
 */
void IO::Drivers::MQTT::flushSparkplugFrame()
{
  SS_ASSERT_LOG(m_sparkplugState.frame.size() <= SpWire::kMaxFrameBytes);
  SS_ASSERT_LOG(m_sparkplugState.lastStampNs >= 0);

  if (!m_sparkplugEnabled)
    return;

  if (m_sparkplugState.frame.size() <= SpWire::kHeaderBytes)
    return;

  const auto stamp = sparkplugStamp(m_sparkplugState.earliestMs);
  publishReceivedData(std::move(m_sparkplugState.frame), stamp);

  m_sparkplugState.frame      = QByteArray();
  m_sparkplugState.earliestMs = 0;
  m_sparkplugState.frame.reserve(SpWire::kMaxFrameBytes);
  SpWire::beginFrame(m_sparkplugState.frame);
}

/**
 * @brief Maps an edge-node epoch stamp onto the steady clock through the offset sampled at connect,
 *        falling back to now for a missing, overflowing or over-skewed one; the result never goes
 *        backwards. The window is one-sided: a relayed stamp is in the past by construction, and
 *        latching a future one pins every later frame 1 ns apart until wall time catches up.
 */
IO::CapturedData::SteadyTimePoint IO::Drivers::MQTT::sparkplugStamp(const quint64 timestampMs)
{
  SS_ASSERT_LOG(m_sparkplugState.lastStampNs >= 0);
  SS_ASSERT_LOG(m_sparkplugState.frame.size() <= SpWire::kMaxFrameBytes);

  const bool mappable = timestampMs > 0 && timestampMs < kSparkplugMaxStampMs;
  const qint64 nowNs  = sparkplugSteadyNs();
  qint64 stamp        = nowNs;
  if (m_sparkplugState.clockValid && mappable) {
    const qint64 sourceNs = static_cast<qint64>(timestampMs) * kSparkplugNsPerMs;
    const qint64 mapped   = sourceNs + m_sparkplugState.offsetNs;
    if (mapped <= nowNs && nowNs - mapped <= kSparkplugMaxSkewNs)
      stamp = mapped;
  }

  stamp                        = qMax(stamp, m_sparkplugState.lastStampNs + 1);
  m_sparkplugState.lastStampNs = stamp;
  return CapturedData::SteadyTimePoint(std::chrono::nanoseconds(stamp));
}

//--------------------------------------------------------------------------------------------------
// Session lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Follows the broker link: any state but Connected drops the session, the rate-limit map,
 *        the unsent rebirth queue and the clock mapping. The session's per-node rebirth flags and
 *        the driver's send queue are cleared by that same event; a connect re-samples the epoch
 *        offset and arms the publishing tick.
 */
void IO::Drivers::MQTT::sparkplugStateChanged(const bool connected)
{
  SS_ASSERT_LOG(m_lastRebirthMs.size() <= SpLimits::kMaxNodes);
  SS_ASSERT_LOG(m_sparkplugState.frame.size() <= SpWire::kMaxFrameBytes);

  m_sparkplugTimer.stop();
  m_sparkplug.reset();
  m_lastRebirthMs.clear();
  m_pendingRebirths.clear();
  m_sparkplugState    = SparkplugState();
  m_sparkplugCounters = SparkplugSession::Counters();
  if (!connected || !m_sparkplugEnabled)
    return;

  const qint64 wallNs = QDateTime::currentMSecsSinceEpoch() * kSparkplugNsPerMs;
  m_sparkplug.setGroupFilter(m_sparkplugGroupId);
  m_sparkplugState.offsetNs   = sparkplugSteadyNs() - wallNs;
  m_sparkplugState.clockValid = true;
  m_sparkplugState.frame.reserve(SpWire::kMaxFrameBytes);
  SpWire::beginFrame(m_sparkplugState.frame);

  connect(&m_sparkplugTimer, &QTimer::timeout, this, &MQTT::onSparkplugTick, Qt::UniqueConnection);
  m_sparkplugTimer.start(kSparkplugTickMs);
}

/**
 * @brief Reports the session's hardening drops as one line carrying the deltas since the previous
 *        sample, and nothing when they are all zero. The counters are plain integers polled from
 *        the publishing tick once every thirty passes (spec 0033), so the message path never
 *        signals, allocates or logs.
 */
void IO::Drivers::MQTT::reportSparkplugDrops()
{
  SS_ASSERT_LOG(m_pendingRebirths.size() <= SpLimits::kMaxNodes);
  SS_ASSERT_LOG(m_sparkplug.slotCount() <= SpLimits::kMaxSlots);

  m_sparkplugState.diagTicks = 0;

  const auto& now  = m_sparkplug.counters();
  const auto& last = m_sparkplugCounters;
  const auto delta = [](quint64 current, quint64 previous) -> quint64 {
    return (current > previous) ? current - previous : 0;
  };

  const quint64 gaps    = delta(now.seqGaps, last.seqGaps);
  const quint64 caps    = delta(now.capDrops, last.capDrops);
  const quint64 errors  = delta(now.decodeErrors, last.decodeErrors);
  const quint64 dropped = delta(now.preBirthDropped, last.preBirthDropped);
  const quint64 unsup   = delta(now.unsupportedMetrics, last.unsupportedMetrics);

  m_sparkplugCounters = now;
  if ((gaps | caps | errors | dropped | unsup) == 0)
    return;

  qCWarning(lcMqttSub).nospace() << "sparkplug drops since the last sample: decode=" << errors
                                 << " caps=" << caps << " preBirth=" << dropped
                                 << " seqGaps=" << gaps << " unsupported=" << unsup;
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the Sparkplug rows of the driver property model. The slot table rides along only
 *        while the lane is on, so a plain MQTT project carries no table it would never read; a
 *        generated project stores it, which is what pins the wire indices its datasets bind to.
 */
void IO::Drivers::MQTT::appendSparkplugProperties(QList<IO::DriverProperty>& props) const
{
  SS_ASSERT_LOG(sparkplugSession().slotCount() <= SpLimits::kMaxSlots);
  SS_ASSERT_LOG(m_sparkplugGroupId.size() >= 0);

  IO::DriverProperty enabled;
  enabled.key   = QStringLiteral("sparkplugEnabled");
  enabled.label = tr("Sparkplug");
  enabled.type  = IO::DriverProperty::CheckBox;
  enabled.value = m_sparkplugEnabled;
  props.append(enabled);

  IO::DriverProperty group;
  group.key   = QStringLiteral("sparkplugGroupId");
  group.label = tr("Sparkplug Group ID");
  group.type  = IO::DriverProperty::Text;
  group.value = m_sparkplugGroupId;
  group.showWhen(QStringLiteral("sparkplugEnabled"), {true});
  props.append(group);

  if (!m_sparkplugEnabled)
    return;

  IO::DriverProperty table;
  table.key   = Keys::SparkplugSlots;
  table.type  = IO::DriverProperty::Text;
  table.value = QVariant::fromValue(sparkplugSession().slotsJson());
  props.append(table);
}

/**
 * @brief Restores the persisted slot table from a project's connection block, before the session
 *        has seen a birth certificate. The session refuses the restore once it has assigned slots
 *        of its own, so a live link never has its indices moved under it.
 */
void IO::Drivers::MQTT::restoreSparkplugSlots(const QVariant& value)
{
  SS_ASSERT_LOG(sparkplugSession().slotCount() <= SpLimits::kMaxSlots);
  SS_ASSERT_LOG(m_sparkplugState.frame.size() <= SpWire::kMaxFrameBytes);

  sparkplugSession().restoreSlots(sparkplugJsonArray(value));
}

//--------------------------------------------------------------------------------------------------
// Project generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief The project the discovered births describe: one MQTT source decoding the driver's own
 *        delta frames through the `sparkplug` native template, and one group per (edge node,
 *        device) pair. The connection block carries every driver property but the passwords.
 */
QJsonObject IO::Drivers::MQTT::buildSparkplugProject() const
{
  SS_ASSERT_LOG(sparkplugSession().slotCount() > 0);
  SS_ASSERT_LOG(sparkplugSession().slotValues().size() <= SpLimits::kMaxSlots);

  const auto& slotTable = sparkplugSession().slotValues();

  QJsonObject project;
  project[Keys::Title]   = tr("Sparkplug Project");
  project[Keys::Actions] = QJsonArray();

  QJsonObject source;
  source[Keys::SourceId]              = 0;
  source[Keys::Title]                 = tr("MQTT");
  source[Keys::BusType]               = static_cast<int>(SerialStudio::BusType::Mqtt);
  source[Keys::FrameStart]            = QString();
  source[Keys::FrameEnd]              = QString();
  source[Keys::Checksum]              = QString();
  source[Keys::FrameDetection]        = static_cast<int>(SerialStudio::NoDelimiters);
  source[Keys::Decoder]               = static_cast<int>(SerialStudio::Binary);
  source[Keys::HexadecimalDelimiters] = false;
  source[Keys::FrameParserCode]       = QString();
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Native);
  source[Keys::FrameParserTemplate]   = QStringLiteral("sparkplug");
  source[Keys::FrameParserParams]     = QJsonObject{
        {QStringLiteral("schema"), sparkplugSchema(slotTable)}
  };

  QJsonObject conn;
  for (const auto& prop : driverProperties())
    if (prop.type != IO::DriverProperty::Password)
      conn.insert(prop.key, QJsonValue::fromVariant(prop.value));

  source[Keys::SourceConn] = conn;
  project[Keys::Sources]   = QJsonArray{source};
  project[Keys::Groups]    = sparkplugGroups(slotTable);
  return project;
}

/**
 * @brief Generates a project from the metrics the birth certificates declared and opens it in the
 *        editor (over the bus, spec 0077). The slot table is marked as generated once the model
 *        reports the load, so later metrics are the ones a regeneration prompt counts; the model
 *        owns the mode switch and restores the previous mode on a failed load.
 */
void IO::Drivers::MQTT::generateProject()
{
  SS_ASSERT_LOG(sparkplugSession().slotCount() >= 0);
  SS_ASSERT_LOG(sparkplugSession().slotValues().size() <= SpLimits::kMaxSlots);

  if (sparkplugSession().slotCount() <= 0) {
    Core::Prompt::showMessageBox(tr("No Sparkplug metrics discovered"),
                                 tr("Connect to the broker and wait for at least one birth "
                                    "certificate before generating a project."),
                                 Core::Prompt::Warning,
                                 tr("Sparkplug Project Generator"));
    return;
  }

  const auto project   = buildSparkplugProject();
  const int groupCount = project.value(Keys::Groups).toArray().size();
  const int datasets   = sparkplugSession().slotCount();
  m_generatedProject.loadAndSave(
    messageBus(), QJsonDocument(project), [this, groupCount, datasets](bool loaded, bool accepted) {
      if (!loaded) {
        logDriverError(tr("Failed to load generated project"),
                       tr("The generated project JSON could not be loaded."));
        return;
      }

      sparkplugSession().markGenerated();
      if (!accepted)
        return;

      Core::Prompt::showMessageBox(
        tr("Successfully generated project with %1 groups and %2 datasets.")
          .arg(groupCount)
          .arg(datasets),
        tr("The project editor is now open for customization."),
        Core::Prompt::Information,
        tr("Sparkplug Project Generator"));
    });
}
