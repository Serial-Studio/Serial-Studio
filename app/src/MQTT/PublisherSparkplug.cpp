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

#ifdef BUILD_COMMERCIAL

#  include <chrono>
#  include <QDateTime>

#  include "DataModel/ExportSchema.h"
#  include "MQTT/Publisher.h"
#  include "SSAssert.h"

namespace Sparkplug = IO::Drivers::SparkplugB;

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps a block sample's steady-clock capture time onto the epoch milliseconds the Sparkplug
 *        wire carries. The source still owns the instant: the wall clock is read once and the
 *        sample's own age is subtracted from it, so a batch published late keeps its capture time.
 */
static quint64 sparkplugEpochMs(DataModel::DataBlock::SteadyTimePoint stamp)
{
  const auto now = DataModel::DataBlock::SteadyClock::now();
  const auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - stamp).count();

  const qint64 epoch = QDateTime::currentMSecsSinceEpoch() - age;
  SS_ASSERT(epoch > 0, return 0);
  return static_cast<quint64>(epoch);
}

/**
 * @brief Metric name for one exported column, matching the label the CSV payload mode publishes so
 *        the same dataset carries the same identity whichever way the project pushes it out.
 */
static QString sparkplugMetricName(const DataModel::ExportColumn& column)
{
  SS_ASSERT_LOG(column.uniqueId >= 0);

  QString label = QStringLiteral("%1/%2").arg(column.groupTitle, column.title).simplified();
  if (column.sourceTitle.isEmpty())
    return label;

  return column.sourceTitle + QStringLiteral("/") + label;
}

/**
 * @brief Feeds one block column's last sample into the edge node. A batch is a live feed, not a
 *        recording, so the newest sample is the metric's value and the ones before it are the
 *        frames the dashboard already drew.
 */
static void feedSparkplugColumn(MQTT::SparkplugPublisher& node,
                                const DataModel::BlockColumn& column,
                                qsizetype index)
{
  SS_ASSERT(index >= 0, return);
  SS_ASSERT(static_cast<std::size_t>(index) < column.values.size(), return);

  const auto slot    = static_cast<std::size_t>(index);
  const bool numeric = DataModel::sample_is_numeric(column, index);
  if (!column.hasText) {
    node.updateValue(column.uniqueId, column.values[slot], QString(), numeric);
    return;
  }

  node.updateValue(column.uniqueId, column.values[slot], column.text[slot], numeric);
}

//==================================================================================================
// PublisherWorker: edge-node lifecycle
//==================================================================================================

/**
 * @brief Whether the applied broker config publishes as a Sparkplug edge node. The verdict comes
 *        from the state machine's own copy of the config, which changes only while the client is
 *        disconnected, so a settings edit cannot switch payload formats mid-connection.
 */
bool MQTT::PublisherWorker::sparkplugActive() const
{
  return m_sparkplug.valid();
}

/**
 * @brief Registers (or clears) the NDEATH will and opens the connection's bdSeq. Called from
 *        applyClientPropertiesUnsafe, whose contract is state == Disconnected: a will set after
 *        CONNECT never arms, and an unarmed will leaves the node reading online forever after an
 *        ungraceful exit (R42). Clearing it keeps a stale will from burying a live node.
 */
void MQTT::PublisherWorker::configureSparkplugWill()
{
  SS_ASSERT(m_client != nullptr, return);
  SS_ASSERT_LOG(m_client->state() == QMqttClient::Disconnected);

  SparkplugPublisher::Config config;
  config.enabled    = m_cfg.sparkplugEnabled;
  config.groupId    = m_cfg.sparkplugGroupId;
  config.edgeNodeId = m_cfg.sparkplugEdgeNode;
  config.deviceId   = m_cfg.sparkplugDeviceId;
  m_sparkplug.setConfig(config);

  if (!m_sparkplug.valid()) {
    m_client->setWillTopic(QString());
    m_client->setWillMessage(QByteArray());
    m_client->setWillQoS(0);
    m_client->setWillRetain(false);
    return;
  }

  const auto stamp = static_cast<quint64>(QDateTime::currentMSecsSinceEpoch());
  const auto will  = m_sparkplug.beginConnection(stamp);
  m_client->setWillTopic(will.topic);
  m_client->setWillMessage(will.payload);
  m_client->setWillQoS(will.qos);
  m_client->setWillRetain(will.retain);
}

/**
 * @brief Subscribes to the node's NCMD topic so a host can ask for a rebirth (R43).
 */
void MQTT::PublisherWorker::subscribeSparkplugCommands()
{
  SS_ASSERT(m_client != nullptr, return);
  SS_ASSERT_LOG(sparkplugActive());

  const QMqttTopicFilter filter(m_sparkplug.commandTopic());
  if (!filter.isValid())
    return;

  auto* subscription = m_client->subscribe(filter, 1);
  if (!subscription)
    return;

  connect(subscription,
          &QMqttSubscription::messageReceived,
          this,
          &PublisherWorker::onSparkplugCommand,
          Qt::UniqueConnection);
}

/**
 * @brief Publishes the birth certificate: the NBIRTH, and the DBIRTH when a device is configured.
 *        The node is marked born only once every message reaches the broker: a birth that fails to
 *        publish leaves it unborn, so it never ships DDATA under aliases the host never received.
 */
void MQTT::PublisherWorker::publishSparkplugBirth()
{
  SS_ASSERT(m_client != nullptr, return);
  SS_ASSERT_LOG(sparkplugActive());

  const auto stamp    = static_cast<quint64>(QDateTime::currentMSecsSinceEpoch());
  const auto messages = m_sparkplug.birthMessages(stamp);

  bool published = !messages.isEmpty();
  for (const auto& message : messages) {
    SS_ASSERT_LOG(message.qos == 0);
    QMqttTopicName topic(message.topic);
    if (!topic.isValid() || !publishAndCount(topic, message.payload))
      published = false;
  }

  if (published)
    m_sparkplug.commitBirth();
}

/**
 * @brief Answers a node command: a rebirth request re-publishes the birth certificate, everything
 *        else is counted and ignored by the state machine itself.
 */
void MQTT::PublisherWorker::onSparkplugCommand(const QMqttMessage& message)
{
  SS_ASSERT(m_client != nullptr, return);
  SS_ASSERT_LOG(!message.topic().name().isEmpty());

  if (!sparkplugActive())
    return;

  if (!m_sparkplug.isRebirthCommand(message.payload()))
    return;

  publishSparkplugBirth();
}

//==================================================================================================
// PublisherWorker: dataset publishing
//==================================================================================================

/**
 * @brief Reconciles the registry with one source's structure (spec 0074 R1-R7, R11): a bump in the
 *        cached frame-pool generation retires entries left at an older generation (a swap's dropped
 *        sources), then this source's entries are cleared and re-added at the new generation with
 *        stable aliases. The generation is adopted from the FrameBuilder, never invented.
 */
void MQTT::PublisherWorker::registerSparkplugMetrics(const DataModel::Frame& frame)
{
  SS_ASSERT_LOG(m_sparkplug.metricCount() >= 0);
  SS_ASSERT_LOG(frame.sourceId >= 0);

  if (!m_cfg.sparkplugEnabled)
    return;

  const quint64 generation = m_pendingStructureGeneration;
  if (generation > m_sparkplug.liveGeneration()) {
    m_sparkplug.setLiveGeneration(generation);
    m_sparkplug.dropStaleMetrics();
  }

  m_sparkplug.clearSource(frame.sourceId);
  const auto schema = DataModel::buildExportSchema(frame);
  for (const auto& column : schema.columns) {
    const auto type = column.isNumeric ? Sparkplug::DataType::Double : Sparkplug::DataType::String;
    m_sparkplug.registerMetric(column.sourceId,
                               column.uniqueId,
                               sparkplugMetricName(column),
                               static_cast<quint32>(type),
                               generation);
  }
}

/**
 * @brief Publishes the batch as Sparkplug data: every block's newest sample is latched into its
 *        metric, and one NDATA (or DDATA) carries whatever actually changed, addressed by alias.
 *        A registry that grew since the birth is re-declared first, so no host ever receives an
 *        alias it cannot resolve.
 */
void MQTT::PublisherWorker::publishSparkplugBlocks(
  const std::vector<DataModel::DataBlockPtr>& blocks)
{
  SS_ASSERT(m_client != nullptr, return);
  SS_ASSERT_LOG(sparkplugActive());

  quint64 stamp = 0;
  for (const auto& block : blocks) {
    if (!block || block->samples <= 0)
      continue;

    const qsizetype last = block->samples - 1;
    stamp                = sparkplugEpochMs(DataModel::sample_time(*block, last));
    for (const auto& column : block->columns)
      feedSparkplugColumn(m_sparkplug, column, last);
  }

  if (stamp == 0)
    return;

  if (m_sparkplug.needsRebirth())
    publishSparkplugBirth();

  const auto messages = m_sparkplug.dataMessages(stamp);
  for (const auto& message : messages) {
    QMqttTopicName topic(message.topic);
    if (topic.isValid())
      publishAndCount(topic, message.payload);
  }
}

/**
 * @brief Drains the raw-byte and script queues while Sparkplug owns the session. An edge node
 *        publishes its own namespace and nothing else, so the queues the other payload modes fill
 *        are emptied rather than left to grow behind a mode that is not running.
 */
void MQTT::PublisherWorker::discardSuppressedPayloads()
{
  SS_ASSERT_LOG(sparkplugActive());
  SS_ASSERT_LOG(m_client != nullptr);

  if (m_rawQueue) {
    TimestampedRawBytes drain;
    while (m_rawQueue->try_dequeue(drain))
      ;
  }

  if (m_frameQueueBytes) {
    TimestampedRawBytes drain;
    while (m_frameQueueBytes->try_dequeue(drain))
      ;
  }
}

//==================================================================================================
// Publisher: Sparkplug configuration surface
//==================================================================================================

/**
 * @brief Returns whether the publisher acts as a Sparkplug B edge node.
 */
bool MQTT::Publisher::sparkplugEnabled() const noexcept
{
  return m_sparkplugEnabled;
}

/**
 * @brief Returns the Sparkplug group id this node publishes under.
 */
QString MQTT::Publisher::sparkplugGroupId() const
{
  return m_sparkplugGroupId;
}

/**
 * @brief Returns the optional Sparkplug device id (empty publishes at node level).
 */
QString MQTT::Publisher::sparkplugDeviceId() const
{
  return m_sparkplugDeviceId;
}

/**
 * @brief Returns the Sparkplug edge node id.
 */
QString MQTT::Publisher::sparkplugEdgeNodeId() const
{
  return m_sparkplugEdgeNodeId;
}

/**
 * @brief Enables or disables Sparkplug B edge-node publishing for the current project.
 */
void MQTT::Publisher::setSparkplugEnabled(const bool enabled)
{
  if (m_sparkplugEnabled == enabled)
    return;

  m_sparkplugEnabled = enabled;
  markConfigChanged();
}

/**
 * @brief Sets the Sparkplug group id.
 */
void MQTT::Publisher::setSparkplugGroupId(const QString& groupId)
{
  if (m_sparkplugGroupId == groupId)
    return;

  m_sparkplugGroupId = groupId;
  markConfigChanged();
}

/**
 * @brief Sets the optional Sparkplug device id.
 */
void MQTT::Publisher::setSparkplugDeviceId(const QString& deviceId)
{
  if (m_sparkplugDeviceId == deviceId)
    return;

  m_sparkplugDeviceId = deviceId;
  markConfigChanged();
}

/**
 * @brief Sets the Sparkplug edge node id.
 */
void MQTT::Publisher::setSparkplugEdgeNodeId(const QString& edgeNodeId)
{
  if (m_sparkplugEdgeNodeId == edgeNodeId)
    return;

  m_sparkplugEdgeNodeId = edgeNodeId;
  markConfigChanged();
}

#endif  // BUILD_COMMERCIAL
