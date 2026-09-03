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

#include "IO/Drivers/MQTT/SparkplugSession.h"

#include "DataModel/FrameKeys.h"
#include "SSAssert.h"

namespace SpLimits  = IO::Drivers::SparkplugLimits;
namespace Sparkplug = IO::Drivers::SparkplugB;
namespace SpWire    = IO::Drivers::OpcUaWire;

//--------------------------------------------------------------------------------------------------
// Topic and key helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the identity key of an edge node. The group scopes it, because the same edge-node
 *        name published under two groups names two physically distinct nodes; the separator is the
 *        ASCII unit separator, which no topic element can carry.
 */
static QString nodeKey(const QString& group, const QString& node)
{
  SS_ASSERT_LOG(group.size() >= 0);
  SS_ASSERT_LOG(node.size() >= 0);

  QString key;
  key.reserve(group.size() + node.size() + 1);
  key.append(group);
  key.append(QChar(u'\x1f'));
  key.append(node);
  return key;
}

/**
 * @brief Builds the identity key of a slot from its group, edge node, device and metric name. The
 *        separator is the unit separator rather than the topic slash because Sparkplug metric names
 *        carry slashes of their own ("Node Control/Rebirth"), and a slash-joined key would let a
 *        device-scoped metric collide with a node-scoped one that spells the same path.
 */
static QString slotKey(const QString& group,
                       const QString& node,
                       const QString& device,
                       const QString& metric)
{
  SS_ASSERT_LOG(node.size() >= 0);
  SS_ASSERT_LOG(metric.size() >= 0);

  QString key;
  key.reserve(group.size() + node.size() + device.size() + metric.size() + 3);
  key.append(nodeKey(group, node));
  key.append(QChar(u'\x1f'));
  key.append(device);
  key.append(QChar(u'\x1f'));
  key.append(metric);
  return key;
}

/**
 * @brief Builds the human-readable label of a slot, which is also the dataset title the project
 *        generator uses: "<node>/<metric>" for node metrics, "<node>/<device>/<metric>" otherwise.
 */
static QString slotLabel(const QString& node, const QString& device, const QString& metric)
{
  SS_ASSERT_LOG(node.size() >= 0);
  SS_ASSERT_LOG(metric.size() >= 0);

  if (device.isEmpty())
    return QStringLiteral("%1/%2").arg(node, metric);

  return QStringLiteral("%1/%2/%3").arg(node, device, metric);
}

/**
 * @brief Splits a topic on slashes into at most @p max views over the original string. Returns
 *        false when the topic carries more elements than the Sparkplug namespace defines, so a
 *        hostile topic is rejected instead of allocating a list per message.
 */
static bool splitTopic(QStringView topic, QStringView* parts, int max, int& count)
{
  SS_ASSERT(parts != nullptr, return false);
  SS_ASSERT(max > 0, return false);

  count               = 0;
  qsizetype start     = 0;
  const qsizetype end = topic.size();
  for (qsizetype i = 0; i <= end; ++i) {
    if (i < end && topic[i] != QLatin1Char('/'))
      continue;

    if (count >= max)
      return false;

    parts[count] = topic.sliced(start, i - start);
    start        = i + 1;
    ++count;
  }

  return count > 0;
}

/**
 * @brief Whether every element of a split topic is short enough to keep. Topic elements become the
 *        node, device and slot-key identity of everything that follows, and MQTT admits 65535-byte
 *        topics, so an over-long element is refused before it is ever materialized as a QString.
 */
static bool identitiesFit(const QStringView* parts, int count)
{
  SS_ASSERT(parts != nullptr, return false);
  SS_ASSERT(count > 0, return false);

  for (int i = 0; i < count; ++i)
    if (parts[i].size() > Sparkplug::kMaxIdentityBytes)
      return false;

  return true;
}

//--------------------------------------------------------------------------------------------------
// Construction and lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an empty session: no nodes, no slots, no buffered traffic.
 */
IO::Drivers::SparkplugSession::SparkplugSession()
  : m_newMetrics(0), m_dirtyCount(0), m_flushing(false)
{
  SS_ASSERT_LOG(m_slots.isEmpty());
  SS_ASSERT_LOG(m_counters.seqGaps == 0);
}

/**
 * @brief Drops the birth state, the buffered traffic and every latched value for a connection drop
 *        or a group change, and KEEPS the slot table: a reconnect re-births every node in whatever
 *        order the broker delivers, and a slot index a dataset is already bound to must survive
 *        that. Counters restart with it; the pulled diagnostics are read as deltas (spec 0033).
 */
void IO::Drivers::SparkplugSession::reset()
{
  m_nodes.clear();
  m_preBirth.clear();
  m_rebirthTopics.clear();

  for (qsizetype i = 0; i < m_slots.size(); ++i) {
    SlotValue& slot  = m_slots[i];
    slot.kind        = Sparkplug::ValueKind::None;
    slot.timestampMs = 0;
    slot.num         = 0.0;
    slot.b           = false;
    slot.dirty       = false;
    slot.str.clear();
  }

  m_counters   = Counters();
  m_newMetrics = 0;
  m_dirtyCount = 0;
  m_flushing   = false;

  SS_ASSERT_LOG(m_slotIndex.size() == m_slots.size());
  SS_ASSERT_LOG(m_nodes.isEmpty());
}

/**
 * @brief The slot table as JSON, one entry per wire index in index order. It rides in the MQTT
 *        driver's connection block, so a generated project pins the indices its datasets bind to
 *        and a later session restores them before the first birth certificate arrives.
 */
QJsonArray IO::Drivers::SparkplugSession::slotsJson() const
{
  SS_ASSERT_LOG(m_slots.size() <= SpLimits::kMaxSlots);
  SS_ASSERT_LOG(m_slotIndex.size() == m_slots.size());

  QJsonArray array;
  for (const auto& slot : m_slots) {
    QJsonObject entry;
    entry.insert(Keys::Index, slot.index);
    entry.insert(Keys::SparkplugGroup, slot.group);
    entry.insert(Keys::SparkplugNode, slot.node);
    entry.insert(Keys::SparkplugDevice, slot.device);
    entry.insert(Keys::SparkplugMetric, slot.name);
    array.append(entry);
  }

  return array;
}

/**
 * @brief Restores a persisted slot table before the first birth certificate. Refused WHOLE, never
 *        in part: a session that already assigned slots keeps them, and one malformed or misplaced
 *        entry abandons the restore and counts, because dropping an entry would shift every later
 *        index by one and repoint the datasets the table exists to hold still.
 */
void IO::Drivers::SparkplugSession::restoreSlots(const QJsonArray& stored)
{
  SS_ASSERT_LOG(m_slots.size() <= SpLimits::kMaxSlots);
  SS_ASSERT_LOG(m_slotIndex.size() == m_slots.size());

  if (stored.isEmpty() || !m_slots.isEmpty())
    return;

  QHash<QString, int> index;
  QVector<SlotValue> table;
  if (!buildRestoredTable(stored, table, index)) {
    ++m_counters.capDrops;
    return;
  }

  m_slots.swap(table);
  m_slotIndex.swap(index);
  m_newMetrics = 0;
}

/**
 * @brief Builds the restored table out of @p stored, returning false the moment an entry is
 *        malformed, duplicated or past the slot cap. Nothing is written into the session here, so
 *        a refusal leaves the caller's own state untouched.
 */
bool IO::Drivers::SparkplugSession::buildRestoredTable(const QJsonArray& stored,
                                                       QVector<SlotValue>& table,
                                                       QHash<QString, int>& index) const
{
  SS_ASSERT(table.isEmpty() && index.isEmpty(), return false);
  SS_ASSERT_LOG(!stored.isEmpty());

  if (stored.size() > SpLimits::kMaxSlots)
    return false;

  for (const auto& item : stored) {
    SlotValue slot;
    if (!readSlotEntry(item.toObject(), static_cast<int>(table.size()), slot))
      return false;

    const QString key = slotKey(slot.group, slot.node, slot.device, slot.name);
    if (index.contains(key))
      return false;

    index.insert(key, slot.index);
    table.append(slot);
  }

  return true;
}

/**
 * @brief Reads one persisted entry into @p out, refusing an entry whose stored index does not name
 *        @p position: the array order IS the wire layout, so an entry that disagrees with it comes
 *        from a truncated or hand-edited table and cannot be trusted to name the same metric.
 */
bool IO::Drivers::SparkplugSession::readSlotEntry(const QJsonObject& entry,
                                                  int position,
                                                  SlotValue& out) const
{
  SS_ASSERT(position >= 0 && position < SpLimits::kMaxSlots, return false);
  SS_ASSERT_LOG(out.index < 0);

  const auto node   = entry.value(Keys::SparkplugNode).toString();
  const auto group  = entry.value(Keys::SparkplugGroup).toString();
  const auto device = entry.value(Keys::SparkplugDevice).toString();
  const auto metric = entry.value(Keys::SparkplugMetric).toString();
  if (node.isEmpty() || metric.isEmpty() || entry.value(Keys::Index).toInt(-1) != position)
    return false;

  const bool oversized =
    node.size() > Sparkplug::kMaxIdentityBytes || device.size() > Sparkplug::kMaxIdentityBytes
    || metric.size() > Sparkplug::kMaxIdentityBytes || group.size() > Sparkplug::kMaxIdentityBytes;
  if (oversized)
    return false;

  out.index       = position;
  out.name        = metric;
  out.node        = node;
  out.group       = group;
  out.device      = device;
  out.displayName = slotLabel(node, device, metric);
  return true;
}

/**
 * @brief Marks the current slot table as the one a generated project was built from, so the
 *        metrics discovered afterwards can be counted and offered as a regeneration prompt.
 */
void IO::Drivers::SparkplugSession::markGenerated()
{
  SS_ASSERT_LOG(m_slots.size() <= SpLimits::kMaxSlots);
  SS_ASSERT_LOG(m_slotIndex.size() == m_slots.size());

  m_newMetrics = 0;
}

/**
 * @brief Restricts the session to one Sparkplug group; an empty filter accepts every group. A real
 *        change resets the session, and the slot table survives it: a slot key carries its own
 *        group, so a slot the previous filter created can never collide with one the new filter
 * does.
 */
void IO::Drivers::SparkplugSession::setGroupFilter(const QString& group)
{
  SS_ASSERT_LOG(group.size() >= 0);
  SS_ASSERT_LOG(m_groupFilter.size() >= 0);

  if (m_groupFilter == group)
    return;

  m_groupFilter = group;
  reset();
}

//--------------------------------------------------------------------------------------------------
// Pulled state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Number of slots assigned so far.
 */
int IO::Drivers::SparkplugSession::slotCount() const noexcept
{
  return static_cast<int>(m_slots.size());
}

/**
 * @brief Whether any slot changed since the encoder last drained the table.
 */
bool IO::Drivers::SparkplugSession::hasDirtySlots() const noexcept
{
  return m_dirtyCount > 0;
}

/**
 * @brief Diagnostic counters, polled by the driver's status surface.
 */
const IO::Drivers::SparkplugSession::Counters& IO::Drivers::SparkplugSession::counters()
  const noexcept
{
  return m_counters;
}

/**
 * @brief Group the session accepts, empty when every group is accepted.
 */
const QString& IO::Drivers::SparkplugSession::groupFilter() const noexcept
{
  return m_groupFilter;
}

/**
 * @brief Slot table, indexed by the wire index each slot was assigned. It is the single schema
 *        definition: the project generator reads slot identity straight from here, so a change to
 *        slot identity is made once and the tested surface is the shipped one.
 */
const QVector<IO::Drivers::SparkplugSession::SlotValue>& IO::Drivers::SparkplugSession::slotValues()
  const noexcept
{
  return m_slots;
}

/**
 * @brief Metrics discovered since the last @ref markGenerated call.
 */
quint64 IO::Drivers::SparkplugSession::newMetricsSinceGeneration() const noexcept
{
  return m_newMetrics;
}

/**
 * @brief Hands over the NCMD topics of the nodes flagged desynced since the last call and clears
 *        the pending list. The per-node flag survives the call and is cleared only by that node's
 *        next birth, so a node that never rebirths is asked once and not once per poll; the
 *        request rate limit belongs to the driver that publishes them.
 */
QStringList IO::Drivers::SparkplugSession::takeRebirthTopics()
{
  SS_ASSERT_LOG(m_rebirthTopics.size() <= SpLimits::kMaxNodes);
  SS_ASSERT_LOG(m_nodes.size() <= SpLimits::kMaxNodes);

  QStringList topics;
  topics.swap(m_rebirthTopics);
  return topics;
}

//--------------------------------------------------------------------------------------------------
// Topic parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps the message-type element to its class. The element count is part of the match: a
 *        node verb carrying a device element, or a device verb without one, is malformed and stays
 *        unclassified rather than being applied to the wrong scope.
 */
IO::Drivers::SparkplugSession::MessageType IO::Drivers::SparkplugSession::messageType(
  QStringView token, int count) noexcept
{
  SS_ASSERT_LOG(count >= 4);
  SS_ASSERT_LOG(count <= SpLimits::kMaxTopicElements);

  if (count == 4 && token == QLatin1StringView("NBIRTH"))
    return MessageType::NodeBirth;

  if (count == 4 && token == QLatin1StringView("NDATA"))
    return MessageType::NodeData;

  if (count == 4 && token == QLatin1StringView("NDEATH"))
    return MessageType::NodeDeath;

  if (count == 5 && token == QLatin1StringView("DBIRTH"))
    return MessageType::DeviceBirth;

  if (count == 5 && token == QLatin1StringView("DDATA"))
    return MessageType::DeviceData;

  if (count == 5 && token == QLatin1StringView("DDEATH"))
    return MessageType::DeviceDeath;

  return MessageType::Ignored;
}

/**
 * @brief Parses "spBv1.0/<group>/<verb>/<edge>[/<device>]", returning false when the topic is not
 *        this session's (foreign namespace, bad element count, an empty or over-long element,
 *        another group) so the driver can publish it raw; "spBv1.0/STATE/<host>" is consumed but
 *        unclassified. A trailing slash spells an empty device that would resolve through the node.
 */
bool IO::Drivers::SparkplugSession::parseTopic(QStringView topic, TopicInfo& out) const
{
  SS_ASSERT_LOG(out.type == MessageType::Ignored);
  SS_ASSERT_LOG(m_groupFilter.size() >= 0);

  QStringView parts[SpLimits::kMaxTopicElements];
  int count = 0;
  if (!splitTopic(topic, parts, SpLimits::kMaxTopicElements, count))
    return false;

  if (!identitiesFit(parts, count))
    return false;

  if (count < 3 || parts[0] != QLatin1StringView(SpLimits::kNamespace))
    return false;

  if (count == 3)
    return parts[1] == QLatin1StringView("STATE");

  if (count < 4 || parts[1].isEmpty() || parts[3].isEmpty())
    return false;

  if (count == SpLimits::kMaxTopicElements && parts[4].isEmpty())
    return false;

  if (!m_groupFilter.isEmpty() && parts[1] != m_groupFilter)
    return false;

  out.type  = messageType(parts[2], count);
  out.group = parts[1].toString();
  out.node  = parts[3].toString();
  if (count == SpLimits::kMaxTopicElements)
    out.device = parts[4].toString();

  return true;
}

//--------------------------------------------------------------------------------------------------
// Ingestion
//--------------------------------------------------------------------------------------------------

/**
 * @brief Consumes one broker message. Returns false only when the topic is not Sparkplug traffic
 *        for this session's group, so the caller can publish it as raw bytes instead; everything
 *        else is consumed, and a payload the codec rejects leaves nothing but a counter behind.
 */
bool IO::Drivers::SparkplugSession::ingest(QStringView topic, QByteArrayView payload)
{
  SS_ASSERT_LOG(m_slots.size() <= SpLimits::kMaxSlots);
  SS_ASSERT_LOG(m_preBirth.size() <= SpLimits::kMaxPreBirthMessages);

  TopicInfo info;
  if (!parseTopic(topic, info))
    return false;

  if (info.type == MessageType::Ignored) {
    ++m_counters.ignoredMessages;
    return true;
  }

  Sparkplug::Payload decoded;
  if (!Sparkplug::decodePayload(payload, decoded, nullptr)) {
    ++m_counters.decodeErrors;
    return true;
  }

  switch (info.type) {
    case MessageType::NodeBirth:
    case MessageType::DeviceBirth:
      handleBirth(info, decoded);
      break;
    case MessageType::NodeData:
    case MessageType::DeviceData:
      handleData(info, decoded, topic, payload);
      break;
    case MessageType::NodeDeath:
    case MessageType::DeviceDeath:
      handleDeath(info, decoded);
      break;
    case MessageType::Ignored:
      break;
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Birth certificates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies a birth certificate: a node birth resets that node's alias table, devices and
 *        sequence expectation (the certificate is the only authority on alias meaning), a device
 *        birth resets that device alone. Both mark their scope alive, clear the rebirth flag and
 *        re-ingest the traffic that arrived before them; a device birth also implies a live node.
 */
void IO::Drivers::SparkplugSession::handleBirth(const TopicInfo& info,
                                                const SparkplugB::Payload& payload)
{
  SS_ASSERT_LOG(info.type == MessageType::NodeBirth || info.type == MessageType::DeviceBirth);
  SS_ASSERT_LOG(payload.metrics.size() <= Sparkplug::kMaxMetrics);

  NodeState* node = nodeState(info, true);
  if (!node)
    return;

  const bool node_birth = (info.type == MessageType::NodeBirth);
  if (node_birth) {
    node->aliasSlots.clear();
    node->devices.clear();
  }

  node->born           = true;
  node->lastSeq        = payload.seq % SpLimits::kSeqModulus;
  node->hasSeq         = payload.hasSeq;
  node->rebirthPending = false;
  if (node->onlineSlot < 0)
    node->onlineSlot =
      ensureSlot(info.group, info.node, QString(), QString::fromLatin1(SpLimits::kOnlineMetric));

  setOnline(*node, true, payload.timestampMs);

  QHash<quint64, int>* aliases = &node->aliasSlots;
  if (!node_birth) {
    DeviceState* device = deviceState(*node, info.device);
    if (!device)
      return;

    device->born = true;
    device->aliasSlots.clear();
    aliases = &device->aliasSlots;
  }

  registerBirthMetrics(info, *aliases, payload);
  flushPreBirth(info);
}

/**
 * @brief Assigns a slot to every named metric of a birth certificate, rebuilds the alias table
 *        from the metrics that carry both a name and an alias, and latches the birth values. A
 *        metric with no name has no identity the session can key on and is counted, never guessed.
 */
void IO::Drivers::SparkplugSession::registerBirthMetrics(const TopicInfo& info,
                                                         QHash<quint64, int>& aliases,
                                                         const SparkplugB::Payload& payload)
{
  SS_ASSERT_LOG(payload.metrics.size() <= Sparkplug::kMaxMetrics);
  SS_ASSERT_LOG(m_slots.size() <= SpLimits::kMaxSlots);

  for (const auto& metric : payload.metrics) {
    if (!metric.hasName) {
      ++m_counters.unsupportedMetrics;
      continue;
    }

    const int index = ensureSlot(info.group, info.node, info.device, metric.name);
    if (index < 0)
      continue;

    if (metric.hasAlias)
      registerAlias(aliases, metric.alias, index);

    applyMetric(index, metric, payload.timestampMs);
  }
}

/**
 * @brief Re-ingests the messages buffered for the scope that just came alive, in arrival order,
 *        and returns the rest to the buffer. The flush flag makes a message that is still
 *        unresolvable drop and count instead of being buffered again, which is what bounds the
 *        recursion at one level.
 */
void IO::Drivers::SparkplugSession::flushPreBirth(const TopicInfo& info)
{
  SS_ASSERT_LOG(m_preBirth.size() <= SpLimits::kMaxPreBirthMessages);
  SS_ASSERT_LOG(!m_flushing);

  if (m_flushing || m_preBirth.isEmpty())
    return;

  QList<PendingMessage> pending;
  pending.swap(m_preBirth);
  m_flushing = true;

  for (qsizetype i = 0; i < pending.size() && i < SpLimits::kMaxPreBirthMessages; ++i) {
    const PendingMessage& message = pending.at(i);
    const bool mine =
      message.node == info.node && message.group == info.group && message.device == info.device;
    if (!mine) {
      m_preBirth.append(message);
      continue;
    }

    const bool consumed = ingest(message.topic, message.payload);
    SS_ASSERT_LOG(consumed);
  }

  m_flushing = false;
}

//--------------------------------------------------------------------------------------------------
// Data messages
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies a data message whole: if its scope has no birth certificate, or any alias in it
 *        does not resolve, nothing is applied, the message is buffered for the birth that explains
 *        it and the node is flagged for a rebirth request. Applying the resolvable half would put
 *        a partially-decoded frame on the dashboard, which is the failure the spec forbids.
 */
void IO::Drivers::SparkplugSession::handleData(const TopicInfo& info,
                                               const SparkplugB::Payload& payload,
                                               QStringView topic,
                                               QByteArrayView raw)
{
  SS_ASSERT_LOG(info.type == MessageType::NodeData || info.type == MessageType::DeviceData);
  SS_ASSERT_LOG(payload.metrics.size() <= Sparkplug::kMaxMetrics);

  NodeState* node = nodeState(info, true);
  if (!node)
    return;

  if (!m_flushing)
    checkSequence(info, *node, payload);

  const QHash<quint64, int>* aliases = aliasTable(*node, info);
  if (!aliases || !resolvable(*aliases, payload)) {
    bufferMessage(info, topic, raw);
    flagRebirth(info, *node);
    return;
  }

  applyMetrics(info, *node, payload);
}

/**
 * @brief Alias table of the scope a message addresses, or null when that scope has no birth
 *        certificate: a dead or unborn node, or a device that never birthed under a live node.
 */
const QHash<quint64, int>* IO::Drivers::SparkplugSession::aliasTable(const NodeState& node,
                                                                     const TopicInfo& info) const
{
  SS_ASSERT_LOG(node.devices.size() <= SpLimits::kMaxDevicesPerNode);
  SS_ASSERT_LOG(info.type != MessageType::Ignored);

  if (!node.born)
    return nullptr;

  if (info.device.isEmpty())
    return &node.aliasSlots;

  const auto it = node.devices.constFind(info.device);
  if (it == node.devices.constEnd() || !it->born)
    return nullptr;

  return &it->aliasSlots;
}

/**
 * @brief Whether every alias-only metric of a payload resolves in @p aliases. Named metrics always
 *        resolve, since a name assigns a slot on its own.
 */
bool IO::Drivers::SparkplugSession::resolvable(const QHash<quint64, int>& aliases,
                                               const SparkplugB::Payload& payload) const
{
  SS_ASSERT_LOG(payload.metrics.size() <= Sparkplug::kMaxMetrics);
  SS_ASSERT_LOG(aliases.size() <= SpLimits::kMaxSlots);

  for (const auto& metric : payload.metrics) {
    if (metric.hasName || !metric.hasAlias)
      continue;

    if (!aliases.contains(metric.alias))
      return false;
  }

  return true;
}

/**
 * @brief Latches every metric of a resolved data message into its slot.
 */
void IO::Drivers::SparkplugSession::applyMetrics(const TopicInfo& info,
                                                 NodeState& node,
                                                 const SparkplugB::Payload& payload)
{
  SS_ASSERT_LOG(node.born);
  SS_ASSERT_LOG(payload.metrics.size() <= Sparkplug::kMaxMetrics);

  QHash<quint64, int>* aliases = &node.aliasSlots;
  if (!info.device.isEmpty()) {
    const auto it = node.devices.find(info.device);
    if (it == node.devices.end())
      return;

    aliases = &it->aliasSlots;
  }

  for (const auto& metric : payload.metrics) {
    const int index = slotForMetric(info, *aliases, metric);
    if (index < 0)
      continue;

    applyMetric(index, metric, payload.timestampMs);
  }
}

/**
 * @brief Resolves one metric to its slot: by name when the metric carries one, learning the alias
 *        it declares along the way, and by alias otherwise. A metric carrying neither has no
 *        identity and is counted rather than applied to whatever slot came last.
 */
int IO::Drivers::SparkplugSession::slotForMetric(const TopicInfo& info,
                                                 QHash<quint64, int>& aliases,
                                                 const SparkplugB::Metric& metric)
{
  SS_ASSERT_LOG(aliases.size() <= SpLimits::kMaxSlots);
  SS_ASSERT_LOG(m_slots.size() <= SpLimits::kMaxSlots);

  if (!metric.hasName) {
    if (!metric.hasAlias) {
      ++m_counters.unsupportedMetrics;
      return -1;
    }

    const auto it = aliases.constFind(metric.alias);
    return (it == aliases.constEnd()) ? -1 : *it;
  }

  const int index = ensureSlot(info.group, info.node, info.device, metric.name);
  if (index >= 0 && metric.hasAlias)
    registerAlias(aliases, metric.alias, index);

  return index;
}

/**
 * @brief Writes one metric into its slot. Only a supported, non-null metric moves a value: an
 *        unsupported datatype is counted and the slot keeps what it had, so an unrenderable
 *        payload can never be mistaken for a reading (R6). A string is latched clamped to what the
 *        delta encoder emits, so a 64 KB value cannot sit in a slot to publish 256 bytes of it.
 */
void IO::Drivers::SparkplugSession::applyMetric(int index,
                                                const SparkplugB::Metric& metric,
                                                quint64 fallbackMs)
{
  SS_ASSERT(index >= 0 && index < static_cast<int>(m_slots.size()), return);
  SS_ASSERT_LOG(m_slots.size() <= SpLimits::kMaxSlots);

  if (!metric.supported) {
    ++m_counters.unsupportedMetrics;
    return;
  }

  if (metric.isNull)
    return;

  SlotValue& slot  = m_slots[index];
  slot.kind        = metric.kind;
  slot.timestampMs = (metric.timestampMs != 0) ? metric.timestampMs : fallbackMs;

  switch (metric.kind) {
    case Sparkplug::ValueKind::Numeric:
      slot.num = metric.numericValue;
      break;
    case Sparkplug::ValueKind::Boolean:
      slot.b = metric.boolValue;
      break;
    case Sparkplug::ValueKind::String:
      slot.str = metric.stringValue.left(SpWire::kMaxStringBytes);
      break;
    case Sparkplug::ValueKind::None:
      break;
  }

  markDirty(slot);
}

/**
 * @brief Checks the per-node sequence number, which advances modulo 256 across every message an
 *        edge node publishes. A mismatch is a lost message: it is counted and arms a rebirth
 *        request, but the message itself is still applied, as its own values are not in doubt.
 */
void IO::Drivers::SparkplugSession::checkSequence(const TopicInfo& info,
                                                  NodeState& node,
                                                  const SparkplugB::Payload& payload)
{
  SS_ASSERT_LOG(!m_flushing);
  SS_ASSERT_LOG(node.lastSeq < SpLimits::kSeqModulus);

  if (!payload.hasSeq)
    return;

  const quint64 seq      = payload.seq % SpLimits::kSeqModulus;
  const quint64 expected = (node.lastSeq + 1) % SpLimits::kSeqModulus;
  const bool first       = !node.hasSeq;

  node.lastSeq = seq;
  node.hasSeq  = true;
  if (first || seq == expected)
    return;

  ++m_counters.seqGaps;
  flagRebirth(info, node);
}

//--------------------------------------------------------------------------------------------------
// Death certificates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies a death certificate. A node death zeroes its synthetic Online metric, marks the
 *        node and its devices unborn and drops the sequence expectation, so later traffic buffers
 *        as pre-birth (R5). A device death marks only that device unborn: the Online slot is
 *        node-scoped, and zeroing it for one device would report the whole node offline.
 */
void IO::Drivers::SparkplugSession::handleDeath(const TopicInfo& info,
                                                const SparkplugB::Payload& payload)
{
  SS_ASSERT_LOG(info.type == MessageType::NodeDeath || info.type == MessageType::DeviceDeath);
  SS_ASSERT_LOG(m_slots.size() <= SpLimits::kMaxSlots);

  NodeState* node = nodeState(info, false);
  if (!node) {
    ++m_counters.ignoredMessages;
    return;
  }

  if (info.type == MessageType::DeviceDeath) {
    const auto it = node->devices.find(info.device);
    if (it != node->devices.end())
      it->born = false;

    return;
  }

  node->born   = false;
  node->hasSeq = false;
  for (auto it = node->devices.begin(); it != node->devices.end(); ++it)
    it->born = false;

  setOnline(*node, false, payload.timestampMs);
}

/**
 * @brief Writes the node's synthetic Online metric and marks it changed, so the death of a node is
 *        a value the dashboard receives rather than the absence of further updates.
 */
void IO::Drivers::SparkplugSession::setOnline(NodeState& node, bool online, quint64 timestampMs)
{
  SS_ASSERT_LOG(node.onlineSlot < static_cast<int>(m_slots.size()));
  SS_ASSERT_LOG(m_slots.size() <= SpLimits::kMaxSlots);

  if (node.onlineSlot < 0)
    return;

  SlotValue& slot  = m_slots[node.onlineSlot];
  slot.kind        = Sparkplug::ValueKind::Boolean;
  slot.b           = online;
  slot.num         = online ? 1.0 : 0.0;
  slot.timestampMs = timestampMs;
  markDirty(slot);
}

//--------------------------------------------------------------------------------------------------
// Buffering and rebirth requests
//--------------------------------------------------------------------------------------------------

/**
 * @brief Buffers one message whole until the birth certificate that explains it arrives. The
 *        buffer is a fixed 256 messages across every node: an overflow drops the oldest message
 *        and counts it, so a broker that never births cannot grow the session (R3, R11). A message
 *        that is still unresolvable during a flush is dropped instead of buffered again.
 */
void IO::Drivers::SparkplugSession::bufferMessage(const TopicInfo& info,
                                                  QStringView topic,
                                                  QByteArrayView payload)
{
  SS_ASSERT_LOG(m_preBirth.size() <= SpLimits::kMaxPreBirthMessages);
  SS_ASSERT_LOG(payload.size() >= 0);

  if (m_flushing) {
    ++m_counters.preBirthDropped;
    return;
  }

  if (m_preBirth.size() >= SpLimits::kMaxPreBirthMessages) {
    m_preBirth.removeFirst();
    ++m_counters.preBirthDropped;
  }

  PendingMessage message;
  message.topic   = topic.toString();
  message.node    = info.node;
  message.group   = info.group;
  message.device  = info.device;
  message.payload = payload.toByteArray();

  m_preBirth.append(message);
  ++m_counters.preBirthBuffered;
}

/**
 * @brief Flags a node as desynced and queues the NCMD topic that asks it to rebirth. A node is
 *        flagged once: the flag is cleared by its next birth, so a node that keeps publishing
 *        unresolvable data is asked once instead of once per message (R4).
 */
void IO::Drivers::SparkplugSession::flagRebirth(const TopicInfo& info, NodeState& node)
{
  SS_ASSERT_LOG(m_rebirthTopics.size() <= SpLimits::kMaxNodes);
  SS_ASSERT_LOG(m_nodes.size() <= SpLimits::kMaxNodes);

  if (node.rebirthPending)
    return;

  node.rebirthPending = true;
  ++m_counters.rebirthRequests;
  if (m_rebirthTopics.size() >= SpLimits::kMaxNodes)
    return;

  const QString space = QString::fromLatin1(SpLimits::kNamespace);
  m_rebirthTopics.append(QStringLiteral("%1/%2/NCMD/%3").arg(space, info.group, info.node));
}

//--------------------------------------------------------------------------------------------------
// Slot table
//--------------------------------------------------------------------------------------------------

/**
 * @brief Node state for the topic's (group, edge node), optionally creating it. Data can arrive
 *        before any certificate, and the node needs somewhere to carry its rebirth flag, so a data
 *        message creates the state too; past the node cap the message is refused and counted (R11).
 */
IO::Drivers::SparkplugSession::NodeState* IO::Drivers::SparkplugSession::nodeState(
  const TopicInfo& info, bool create)
{
  SS_ASSERT_LOG(m_nodes.size() <= SpLimits::kMaxNodes);
  SS_ASSERT(!info.node.isEmpty(), return nullptr);

  const QString name = nodeKey(info.group, info.node);
  const auto it      = m_nodes.find(name);
  if (it != m_nodes.end())
    return &it.value();

  if (!create)
    return nullptr;

  if (m_nodes.size() >= SpLimits::kMaxNodes) {
    ++m_counters.capDrops;
    return nullptr;
  }

  return &m_nodes.insert(name, NodeState()).value();
}

/**
 * @brief Device state for @p name under @p node, created on demand up to the per-node cap.
 */
IO::Drivers::SparkplugSession::DeviceState* IO::Drivers::SparkplugSession::deviceState(
  NodeState& node, const QString& name)
{
  SS_ASSERT_LOG(node.devices.size() <= SpLimits::kMaxDevicesPerNode);
  SS_ASSERT(!name.isEmpty(), return nullptr);

  const auto it = node.devices.find(name);
  if (it != node.devices.end())
    return &it.value();

  if (node.devices.size() >= SpLimits::kMaxDevicesPerNode) {
    ++m_counters.capDrops;
    return nullptr;
  }

  return &node.devices.insert(name, DeviceState()).value();
}

/**
 * @brief Index of the slot owning (group, edge node, device, metric name), assigning a new one the
 *        first time that identity is seen. Indices are handed out in order and never reused: the
 *        wire index a dataset binds to must outlive every birth that renames aliases. Past the slot
 *        cap, or past the identity cap bounding what a slot retains, the metric is refused (R11).
 */
int IO::Drivers::SparkplugSession::ensureSlot(const QString& group,
                                              const QString& node,
                                              const QString& device,
                                              const QString& metric)
{
  SS_ASSERT_LOG(m_slots.size() <= SpLimits::kMaxSlots);
  SS_ASSERT_LOG(m_slotIndex.size() == m_slots.size());

  if (node.isEmpty() || metric.isEmpty()) {
    ++m_counters.unsupportedMetrics;
    return -1;
  }

  const bool oversized = node.size() > Sparkplug::kMaxIdentityBytes
                      || device.size() > Sparkplug::kMaxIdentityBytes
                      || metric.size() > Sparkplug::kMaxIdentityBytes;
  if (oversized) {
    ++m_counters.capDrops;
    return -1;
  }

  const QString key = slotKey(group, node, device, metric);
  const auto it     = m_slotIndex.constFind(key);
  if (it != m_slotIndex.constEnd())
    return *it;

  if (m_slots.size() >= SpLimits::kMaxSlots) {
    ++m_counters.capDrops;
    return -1;
  }

  SlotValue slot;
  slot.index       = static_cast<int>(m_slots.size());
  slot.name        = metric;
  slot.node        = node;
  slot.group       = group;
  slot.device      = device;
  slot.displayName = slotLabel(node, device, metric);

  m_slotIndex.insert(key, slot.index);
  m_slots.append(slot);
  ++m_newMetrics;
  return slot.index;
}

/**
 * @brief Points an alias at a slot. Aliases are attacker-chosen 64-bit numbers, so the table is
 *        capped like the slot table itself and a refused alias is counted; it resolves as a
 *        pre-birth miss afterwards rather than silently mapping to the wrong metric.
 */
void IO::Drivers::SparkplugSession::registerAlias(QHash<quint64, int>& aliases,
                                                  quint64 alias,
                                                  int index)
{
  SS_ASSERT(index >= 0, return);
  SS_ASSERT_LOG(aliases.size() <= SpLimits::kMaxSlots);

  if (aliases.size() >= SpLimits::kMaxSlots && !aliases.contains(alias)) {
    ++m_counters.capDrops;
    return;
  }

  aliases.insert(alias, index);
}

/**
 * @brief Marks a slot changed for the next encoder pass, keeping the dirty count in step so the
 *        driver can skip a tick that would encode nothing.
 */
void IO::Drivers::SparkplugSession::markDirty(SlotValue& slot)
{
  SS_ASSERT_LOG(slot.index >= 0);
  SS_ASSERT_LOG(m_dirtyCount <= m_slots.size());

  if (slot.dirty)
    return;

  slot.dirty = true;
  ++m_dirtyCount;
}
