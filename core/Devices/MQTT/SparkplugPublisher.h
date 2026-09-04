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
#include <QString>
#include <QStringView>
#include <QVector>

#include "IO/Drivers/MQTT/SparkplugSession.h"
#include "Protocols/Sparkplug/SparkplugPayload.h"

namespace MQTT {

/**
 * @brief Fixed caps for the outbound edge-node state. The registry is fed by the project's
 *        dataset table, so it inherits the inbound side's slot ceiling and sequence modulus
 *        rather than declaring numbers of its own: one Sparkplug vocabulary, one set of limits.
 */
namespace SparkplugPublisherLimits {
inline constexpr int kMaxMetrics     = IO::Drivers::SparkplugLimits::kMaxSlots;
inline constexpr quint64 kSeqModulus = IO::Drivers::SparkplugLimits::kSeqModulus;

// Metric an NBIRTH carries so a host can correlate it with the will that will announce its death
inline constexpr const char* kBirthDeathSequence = "bdSeq";
}  // namespace SparkplugPublisherLimits

/**
 * @brief Sparkplug B v1.0 Edge Node lifecycle (spec 0073 R39-R44): the metric registry, the alias
 *        table, the `bdSeq`/`seq` counters and the birth/data/death payloads. No I/O and no
 *        QObject: entry points return ready-to-publish {topic, payload} pairs, so the ctest tier
 *        drives the whole state machine without a broker. Counters are polled (spec 0033).
 */
class SparkplugPublisher {
public:
  /**
   * @brief One ready-to-publish message. QoS and retain are part of the Sparkplug contract, not
   *        caller policy: births and data go out at QoS 0 unretained, the death will at QoS 1.
   */
  struct Message {
    QString topic;
    QByteArray payload;
    quint8 qos;
    bool retain;

    /**
     * @brief Builds an empty message addressed nowhere.
     */
    Message() : qos(0), retain(false) {}
  };

  /**
   * @brief Edge-node identity. An empty device id publishes at node level (NBIRTH/NDATA); a set
   *        one moves the dataset metrics onto the device (DBIRTH/DDATA).
   */
  struct Config {
    QString groupId;
    QString edgeNodeId;
    QString deviceId;
    bool enabled;

    /**
     * @brief Builds a disabled, unidentified edge node.
     */
    Config() : enabled(false) {}
  };

  /**
   * @brief Pulled diagnostic counters (spec 0033): plain integers incremented in place and read
   *        on the caller's own cadence, so no drop or refusal is silent and none of them signals,
   *        allocates or locks per value.
   */
  struct Counters {
    quint64 births;
    quint64 dataMessages;
    quint64 skippedValues;
    quint64 registryDrops;
    quint64 rebirthCommands;
    quint64 ignoredCommands;
    quint64 commandDecodeErrors;

    /**
     * @brief Starts every counter at zero.
     */
    Counters()
      : births(0)
      , dataMessages(0)
      , skippedValues(0)
      , registryDrops(0)
      , rebirthCommands(0)
      , ignoredCommands(0)
      , commandDecodeErrors(0)
    {}
  };

  SparkplugPublisher();

  void reset();
  void commitBirth();
  void clearRegistry();
  void dropStaleMetrics();
  void clearSource(int sourceId);
  void setLiveGeneration(quint64 generation);
  void setConfig(const Config& config);
  void registerMetric(
    int sourceId, int uniqueId, const QString& name, quint32 datatype, quint64 generation);
  void updateValue(int uniqueId, double numericValue, const QString& text, bool numeric);

  [[nodiscard]] bool born() const noexcept;
  [[nodiscard]] bool valid() const noexcept;
  [[nodiscard]] quint64 seq() const noexcept;
  [[nodiscard]] quint64 bdSeq() const noexcept;
  [[nodiscard]] int metricCount() const noexcept;
  [[nodiscard]] quint64 liveGeneration() const noexcept;
  [[nodiscard]] QString commandTopic() const;
  [[nodiscard]] bool needsRebirth() const noexcept;
  [[nodiscard]] const Config& config() const noexcept;
  [[nodiscard]] const Counters& counters() const noexcept;
  [[nodiscard]] bool isRebirthCommand(QByteArrayView payload);
  [[nodiscard]] Message beginConnection(quint64 timestampMs);
  [[nodiscard]] Message deathCertificate(quint64 timestampMs) const;
  [[nodiscard]] QVector<Message> dataMessages(quint64 timestampMs);
  [[nodiscard]] QVector<Message> birthMessages(quint64 timestampMs);

private:
  /**
   * @brief One registered metric: the identity a birth declares and the latest value a data
   *        message carries. @c alias is assigned once by @c aliasFor and never reassigned within a
   *        connection, so removing another source's entry never renumbers it (spec 0074 R5).
   *        @c sourceId scopes per-source clears; @c generation is the frame-pool stamp a project
   *        swap uses to drop the entries of a source that did not republish (R7).
   */
  struct MetricEntry {
    QString name;
    QString stringValue;
    quint64 alias;
    quint64 generation;
    double numericValue;
    quint32 datatype;
    int uniqueId;
    int sourceId;
    IO::Drivers::SparkplugB::ValueKind kind;
    bool boolValue;
    bool hasValue;
    bool changed;

    /**
     * @brief Builds an unaliased, sourceless metric with no value and no channel.
     */
    MetricEntry()
      : alias(0)
      , generation(0)
      , numericValue(0.0)
      , datatype(0)
      , uniqueId(-1)
      , sourceId(-1)
      , kind(IO::Drivers::SparkplugB::ValueKind::None)
      , boolValue(false)
      , hasValue(false)
      , changed(false)
    {}
  };

  [[nodiscard]] quint64 nextSeq();
  [[nodiscard]] quint64 aliasFor(int uniqueId);
  [[nodiscard]] QString resolveMetricName(const MetricEntry& entry) const;
  [[nodiscard]] QString topicFor(QStringView verb) const;
  [[nodiscard]] QString deviceTopicFor(QStringView verb) const;
  [[nodiscard]] IO::Drivers::SparkplugB::Metric bdSeqMetric(quint64 timestampMs) const;
  [[nodiscard]] IO::Drivers::SparkplugB::Metric rebirthControlMetric(quint64 timestampMs) const;
  [[nodiscard]] IO::Drivers::SparkplugB::Metric metricFor(const MetricEntry& entry,
                                                          quint64 timestampMs,
                                                          bool withName) const;
  [[nodiscard]] Message encode(const QString& topic, IO::Drivers::SparkplugB::Payload& payload);

  void rebuildIndex();
  void clearChanged();
  void applyBoolean(MetricEntry& entry, bool value);
  void applyNumeric(MetricEntry& entry, double value);
  void applyString(MetricEntry& entry, const QString& value);
  void appendBirthMetrics(IO::Drivers::SparkplugB::Payload& payload, quint64 timestampMs);

private:
  bool m_born;
  quint64 m_seq;
  quint64 m_bdSeq;
  Config m_config;
  bool m_bdSeqAssigned;
  bool m_registryDirty;
  quint64 m_nextAlias;
  quint64 m_liveGeneration;
  Counters m_counters;
  QHash<int, int> m_index;
  QHash<int, quint64> m_aliasByUniqueId;
  QVector<MetricEntry> m_metrics;
};

}  // namespace MQTT
