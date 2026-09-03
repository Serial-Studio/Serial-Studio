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

#pragma once

#include <QByteArray>
#include <QByteArrayView>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringView>
#include <QVector>

#include "IO/Drivers/MQTT/SparkplugPayload.h"
#include "IO/Drivers/OpcUaWire.h"
#include "SSAssert.h"

namespace IO {
namespace Drivers {

/**
 * @brief Fixed caps for the Sparkplug B session state. Broker traffic decides how many nodes,
 *        devices, metrics and out-of-order messages arrive, so every container the session grows
 *        has a ceiling here and refuses past it (counted, never resized on demand). The slot
 *        ceiling is the OPC UA wire ceiling because the delta encoder that consumes the slot
 *        table is the same one.
 */
namespace SparkplugLimits {
inline constexpr int kMaxSlots            = OpcUaWire::kMaxTags;
inline constexpr int kMaxPreBirthMessages = 256;
inline constexpr int kMaxNodes            = 256;
inline constexpr int kMaxDevicesPerNode   = 64;
inline constexpr int kMaxTopicElements    = 5;
inline constexpr quint64 kSeqModulus      = 256;

// Namespace element every Sparkplug B v1.0 topic starts with
inline constexpr const char* kNamespace = "spBv1.0";

// Synthetic per-edge-node metric carrying the birth/death state (R5)
inline constexpr const char* kOnlineMetric = "Online";

// The identity cap is the encoder's own truncation point, so nothing is retained that cannot ship
static_assert(SparkplugB::kMaxIdentityBytes == OpcUaWire::kMaxStringBytes);
}  // namespace SparkplugLimits

/**
 * @brief Sparkplug B session and birth-certificate state machine (spec 0073 R2-R6, R11): turns
 *        the `spBv1.0` topic namespace into a flat table of latched slots the OPC UA delta encoder
 *        walks, keyed by (edge node, device, metric name) with indices that never move under a
 *        rebirth. Qt Core only, QObject-free: counters are polled (spec 0033), no drop is silent.
 */
class SparkplugSession {
public:
  /**
   * @brief One latched metric. Only the channel named by @c kind carries a value; @c dirty marks
   *        the slot as changed since the encoder last drained it.
   */
  struct SlotValue {
    QString name;
    QString node;
    QString group;
    QString device;
    QString displayName;
    QString str;
    quint64 timestampMs;
    double num;
    int index;
    SparkplugB::ValueKind kind;
    bool dirty;
    bool b;

    /**
     * @brief Builds an unassigned, clean slot with no value channel.
     */
    SlotValue()
      : timestampMs(0)
      , num(0.0)
      , index(-1)
      , kind(SparkplugB::ValueKind::None)
      , dirty(false)
      , b(false)
    {}
  };

  /**
   * @brief Pulled diagnostic counters (spec 0033): plain integers incremented in place and read on
   *        the caller's own cadence, so nothing here signals, allocates or locks per message.
   */
  struct Counters {
    quint64 seqGaps;
    quint64 capDrops;
    quint64 decodeErrors;
    quint64 ignoredMessages;
    quint64 preBirthBuffered;
    quint64 preBirthDropped;
    quint64 rebirthRequests;
    quint64 unsupportedMetrics;

    /**
     * @brief Starts every counter at zero.
     */
    Counters()
      : seqGaps(0)
      , capDrops(0)
      , decodeErrors(0)
      , ignoredMessages(0)
      , preBirthBuffered(0)
      , preBirthDropped(0)
      , rebirthRequests(0)
      , unsupportedMetrics(0)
    {}
  };

  SparkplugSession();

  void reset();
  void markGenerated();
  void setGroupFilter(const QString& group);
  void restoreSlots(const QJsonArray& stored);

  [[nodiscard]] int slotCount() const noexcept;
  [[nodiscard]] QJsonArray slotsJson() const;
  [[nodiscard]] bool hasDirtySlots() const noexcept;
  [[nodiscard]] const Counters& counters() const noexcept;
  [[nodiscard]] const QString& groupFilter() const noexcept;
  [[nodiscard]] const QVector<SlotValue>& slotValues() const noexcept;
  [[nodiscard]] quint64 newMetricsSinceGeneration() const noexcept;
  [[nodiscard]] QStringList takeRebirthTopics();
  [[nodiscard]] bool ingest(QStringView topic, QByteArrayView payload);

  /**
   * @brief Hands every changed slot to @p callback and clears the change marks, so one encoder
   *        pass and the clearing of what it encoded cannot come apart. The callback sees a const
   *        reference; the loop bound is the slot table, which is capped at construction.
   */
  template<typename Callback>
  void consumeDirtySlots(Callback&& callback)
  {
    SS_ASSERT_LOG(m_dirtyCount >= 0);
    SS_ASSERT_LOG(m_slots.size() <= SparkplugLimits::kMaxSlots);

    for (qsizetype i = 0; i < m_slots.size(); ++i) {
      if (!m_slots.at(i).dirty)
        continue;

      callback(m_slots.at(i));
      m_slots[i].dirty = false;
    }

    m_dirtyCount = 0;
  }

private:
  /**
   * @brief Sparkplug message classes this session acts on; everything else (NCMD, DCMD, STATE and
   *        any future verb) resolves to @c Ignored and is counted rather than parsed.
   */
  enum class MessageType : quint8 {
    Ignored     = 0,
    NodeBirth   = 1,
    NodeData    = 2,
    NodeDeath   = 3,
    DeviceBirth = 4,
    DeviceData  = 5,
    DeviceDeath = 6,
  };

  /**
   * @brief One parsed topic: the namespace elements plus the message class they name.
   */
  struct TopicInfo {
    QString group;
    QString node;
    QString device;
    MessageType type;

    /**
     * @brief Builds an unresolved topic.
     */
    TopicInfo() : type(MessageType::Ignored) {}
  };

  /**
   * @brief One buffered pre-birth message, kept whole so it can be re-ingested after a birth.
   */
  struct PendingMessage {
    QString topic;
    QString node;
    QString group;
    QString device;
    QByteArray payload;
  };

  /**
   * @brief Per-device alias table and birth state, scoped to its edge node.
   */
  struct DeviceState {
    QHash<quint64, int> aliasSlots;
    bool born;

    /**
     * @brief Builds an unborn device with an empty alias table.
     */
    DeviceState() : born(false) {}
  };

  /**
   * @brief Per-(group, edge node) state: node aliases, its devices, sequence and birth state.
   */
  struct NodeState {
    QHash<QString, DeviceState> devices;
    QHash<quint64, int> aliasSlots;
    quint64 lastSeq;
    int onlineSlot;
    bool born;
    bool hasSeq;
    bool rebirthPending;

    /**
     * @brief Builds an unborn node with no sequence expectation and no online slot.
     */
    NodeState() : lastSeq(0), onlineSlot(-1), born(false), hasSeq(false), rebirthPending(false) {}
  };

  [[nodiscard]] static MessageType messageType(QStringView token, int count) noexcept;

  [[nodiscard]] bool parseTopic(QStringView topic, TopicInfo& out) const;
  [[nodiscard]] bool resolvable(const QHash<quint64, int>& aliases,
                                const SparkplugB::Payload& payload) const;
  [[nodiscard]] const QHash<quint64, int>* aliasTable(const NodeState& node,
                                                      const TopicInfo& info) const;

  [[nodiscard]] NodeState* nodeState(const TopicInfo& info, bool create);
  [[nodiscard]] DeviceState* deviceState(NodeState& node, const QString& name);
  [[nodiscard]] int ensureSlot(const QString& group,
                               const QString& node,
                               const QString& device,
                               const QString& metric);
  [[nodiscard]] int slotForMetric(const TopicInfo& info,
                                  QHash<quint64, int>& aliases,
                                  const SparkplugB::Metric& metric);
  [[nodiscard]] bool readSlotEntry(const QJsonObject& entry, int position, SlotValue& out) const;
  [[nodiscard]] bool buildRestoredTable(const QJsonArray& stored,
                                        QVector<SlotValue>& table,
                                        QHash<QString, int>& index) const;

  void markDirty(SlotValue& slot);
  void flushPreBirth(const TopicInfo& info);
  void registerAlias(QHash<quint64, int>& aliases, quint64 alias, int index);
  void handleDeath(const TopicInfo& info, const SparkplugB::Payload& payload);
  void handleBirth(const TopicInfo& info, const SparkplugB::Payload& payload);
  void flagRebirth(const TopicInfo& info, NodeState& node);
  void bufferMessage(const TopicInfo& info, QStringView topic, QByteArrayView payload);
  void setOnline(NodeState& node, bool online, quint64 timestampMs);
  void applyMetric(int index, const SparkplugB::Metric& metric, quint64 fallbackMs);
  void applyMetrics(const TopicInfo& info, NodeState& node, const SparkplugB::Payload& payload);
  void checkSequence(const TopicInfo& info, NodeState& node, const SparkplugB::Payload& payload);
  void registerBirthMetrics(const TopicInfo& info,
                            QHash<quint64, int>& aliases,
                            const SparkplugB::Payload& payload);
  void handleData(const TopicInfo& info,
                  const SparkplugB::Payload& payload,
                  QStringView topic,
                  QByteArrayView raw);

private:
  // Keyed by (group, edge node); one shared state would merge two groups' aliases and sequences
  QHash<QString, NodeState> m_nodes;
  QHash<QString, int> m_slotIndex;
  QList<PendingMessage> m_preBirth;
  QStringList m_rebirthTopics;
  QVector<SlotValue> m_slots;
  QString m_groupFilter;
  Counters m_counters;
  quint64 m_newMetrics;
  qsizetype m_dirtyCount;
  bool m_flushing;
};

}  // namespace Drivers
}  // namespace IO
