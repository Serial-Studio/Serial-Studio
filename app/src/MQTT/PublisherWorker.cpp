/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
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

#  include "MQTT/PublisherWorker.h"

#  include <algorithm>
#  include <chrono>
#  include <QDateTime>
#  include <QJsonArray>
#  include <QJsonDocument>
#  include <QJsonObject>
#  include <QTimer>

#  include "DataModel/ExportSchema.h"
#  include "MQTT/CsvExpansion.h"
#  include "MQTT/Publisher.h"
#  include "MQTT/PublisherScript.h"
#  include "MQTT/TlsIdentity.h"
#  include "SSAssert.h"

Q_LOGGING_CATEGORY(lcMqttPub, "serialstudio.mqtt.publisher", QtCriticalMsg)

namespace Sparkplug = IO::Drivers::SparkplugB;

//--------------------------------------------------------------------------------------------------
// Constants: per-step deadlines. Attempts and backoff belong to the shared Async::RetryPolicy.
//--------------------------------------------------------------------------------------------------

static constexpr int kBrokerConnectTimeoutMs    = 15000;
static constexpr int kBrokerDisconnectTimeoutMs = 5000;

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
// Construction and thread bootstrap
//==================================================================================================

/**
 * @brief Constructs the worker; QMqttClient is built on the worker thread via bootstrap().
 */
MQTT::PublisherWorker::PublisherWorker(
  moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>* frameQueue,
  std::atomic<bool>* enabled,
  std::atomic<size_t>* queueSize,
  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* rawQueue,
  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* frameQueueBytes,
  std::atomic<int>* mode,
  std::atomic<int>* scriptLanguage,
  std::atomic<quint64>* messagesSent,
  std::atomic<quint64>* bytesSent)
  : DataModel::FrameConsumerWorker<DataModel::DataBlockPtr>(frameQueue, enabled, queueSize)
  , m_client(nullptr)
  , m_rawQueue(rawQueue)
  , m_frameQueueBytes(frameQueueBytes)
  , m_mode(mode)
  , m_scriptLanguage(scriptLanguage)
  , m_messagesSent(messagesSent)
  , m_bytesSent(bytesSent)
  , m_script(nullptr)
  , m_csvHeaderDirty(true)
  , m_pendingStructureGeneration(0)
{
  m_sslConfiguration.setProtocol(QSsl::SecureProtocols);
  m_sslConfiguration.setPeerVerifyMode(QSslSocket::AutoVerifyPeer);
  m_sslConfiguration.setPeerVerifyDepth(10);

  m_rawBatchBuffer.reserve(64 * 1024);
  m_csvRowBuffer.reserve(8 * 1024);
}

/**
 * @brief Destructor closes the broker session and tears down the script engine. Dropping the
 *        runner first cancels whatever the reconnect flow still holds, silently.
 */
MQTT::PublisherWorker::~PublisherWorker()
{
  m_runner.reset();

  if (m_client && m_client->state() != QMqttClient::Disconnected)
    m_client->disconnectFromHost();

  delete m_script;
  m_script = nullptr;
}

/**
 * @brief Worker-thread bootstrap: creates the QMqttClient, the task runner and the script engine
 *        on this thread. The runner is built here and not in the constructor because a task tree
 *        is thread-affine: its timers and connections must belong to the thread driving the client.
 */
void MQTT::PublisherWorker::bootstrap()
{
  if (m_client)
    return;

  m_client = new QMqttClient(this);
  connect(m_client, &QMqttClient::stateChanged, this, &PublisherWorker::onClientStateChanged);
  connect(m_client, &QMqttClient::errorChanged, this, &PublisherWorker::onClientErrorChanged);

  m_runner = std::make_unique<Async::TaskRunner>(this);
  m_script = new PublisherScript();
}

//==================================================================================================
// Resource state
//==================================================================================================

/**
 * @brief Reports the broker connection state.
 */
bool MQTT::PublisherWorker::isResourceOpen() const
{
  return m_client && m_client->state() == QMqttClient::Connected;
}

/**
 * @brief Returns the localized human-readable description for an MQTT error.
 */
QString MQTT::PublisherWorker::errorString(QMqttClient::ClientError error) const
{
  return PublisherWorker::describeMqttError(error);
}

/**
 * @brief Disconnects from the broker, cancelling a reconnect still in flight so it cannot bring
 *        the session back after the caller asked for it to end.
 */
void MQTT::PublisherWorker::closeResources()
{
  if (m_runner)
    m_runner->cancel();

  if (m_client && m_client->state() != QMqttClient::Disconnected)
    m_client->disconnectFromHost();
}

/**
 * @brief Returns a localized human-readable description for an MQTT client error.
 */
QString MQTT::PublisherWorker::describeMqttError(QMqttClient::ClientError error)
{
  switch (error) {
    case QMqttClient::NoError:
      return Publisher::tr("No error");
    case QMqttClient::InvalidProtocolVersion:
      return Publisher::tr("The broker rejected the connection due to an unsupported "
                           "protocol version. Match the broker's MQTT version and try again.");
    case QMqttClient::IdRejected:
      return Publisher::tr("The broker rejected the client ID. It may be malformed, too "
                           "long, or already in use. Regenerate it and try again.");
    case QMqttClient::ServerUnavailable:
      return Publisher::tr("The network reached the broker, but the broker is currently "
                           "unavailable. Verify its status and try again later.");
    case QMqttClient::BadUsernameOrPassword:
      return Publisher::tr("The username or password is incorrect or malformed. "
                           "Double-check the credentials and try again.");
    case QMqttClient::NotAuthorized:
      return Publisher::tr("The broker denied the connection due to insufficient "
                           "permissions. Verify that the account has the required ACLs.");
    case QMqttClient::TransportInvalid:
      return Publisher::tr("A network or transport-layer issue prevented the connection. "
                           "Check connectivity, ports, and TLS configuration.");
    case QMqttClient::ProtocolViolation:
      return Publisher::tr("The client detected an MQTT protocol violation and closed the "
                           "connection. Verify broker and client compatibility.");
    case QMqttClient::UnknownError:
      return Publisher::tr("An unexpected error occurred. Check the broker logs and the "
                           "application console for details.");
    case QMqttClient::Mqtt5SpecificError:
      return Publisher::tr("An MQTT 5 protocol-level error occurred. Inspect the broker's "
                           "reason code for details.");
  }

  return Publisher::tr("Unspecified MQTT error (code %1).").arg(static_cast<int>(error));
}

//==================================================================================================
// Queue draining
//==================================================================================================

/**
 * @brief Drains both the frame queue and the raw-bytes queue. The Sparkplug lane also re-declares
 *        a registry that grew since the birth from HERE and not only from the data path: a project
 *        edit dirties the registry on a structure signal, and a source that is momentarily idle
 *        would otherwise leave the host holding aliases that no longer describe the node.
 */
void MQTT::PublisherWorker::processData()
{
  DataModel::FrameConsumerWorker<DataModel::DataBlockPtr>::processData();

  if (!consumerEnabled())
    return;

  if (!isResourceOpen())
    return;

  if (sparkplugActive()) {
    discardSuppressedPayloads();
    if (m_sparkplug.needsRebirth())
      publishSparkplugBirth();

    return;
  }

  const int mode = m_mode->load(std::memory_order_relaxed);

  if (m_rawQueue && mode != static_cast<int>(Publisher::Mode::RawRxData)) {
    TimestampedRawBytes drain;
    while (m_rawQueue->try_dequeue(drain))
      ;
  }
  if (m_frameQueueBytes && mode != static_cast<int>(Publisher::Mode::ScriptDriven)) {
    TimestampedRawBytes drain;
    while (m_frameQueueBytes->try_dequeue(drain))
      ;
  }

  if (m_cfg.topicBase.isEmpty())
    return;

  if (mode == static_cast<int>(Publisher::Mode::RawRxData) && m_rawQueue) {
    QMqttTopicName topic(m_cfg.topicBase);
    if (!topic.isValid())
      return;

    m_rawBatchBuffer.resize(0);
    TimestampedRawBytes item;
    while (m_rawQueue->try_dequeue(item))
      if (item.data && !item.data->data.isEmpty())
        m_rawBatchBuffer += item.data->data;

    if (!m_rawBatchBuffer.isEmpty())
      publishAndCount(topic, m_rawBatchBuffer);

    return;
  }

  if (mode == static_cast<int>(Publisher::Mode::ScriptDriven) && m_frameQueueBytes) {
    const QString topicStr = m_cfg.scriptTopic.isEmpty() ? m_cfg.topicBase : m_cfg.scriptTopic;
    QMqttTopicName topic(topicStr);
    if (!topic.isValid())
      return;

    recompileScriptIfNeeded();
    if (!m_script || !m_script->isLoaded())
      return;

    QByteArray aggregate;
    aggregate.reserve(4096);

    TimestampedRawBytes item;
    while (m_frameQueueBytes->try_dequeue(item)) {
      if (!item.data || item.data->data.isEmpty())
        continue;

      QByteArray payload;
      QString error;
      if (!m_script->run(item.data->data, payload, error)) {
        Q_EMIT scriptErrorOccurred(error);
        continue;
      }

      if (!payload.isEmpty())
        aggregate += payload;
    }

    if (!aggregate.isEmpty())
      publishAndCount(topic, aggregate);
  }
}

/**
 * @brief Dispatches the latest batched frame to the per-mode publisher.
 */
void MQTT::PublisherWorker::processItems(const std::vector<DataModel::DataBlockPtr>& items)
{
  if (items.empty())
    return;

  if (!isResourceOpen())
    return;

  if (sparkplugActive()) {
    publishSparkplugBlocks(items);
    return;
  }

  if (m_cfg.topicBase.isEmpty())
    return;

  const int mode = m_mode->load(std::memory_order_relaxed);
  if (static_cast<Publisher::Mode>(mode) != Publisher::Mode::DashboardDataJson
      && static_cast<Publisher::Mode>(mode) != Publisher::Mode::DashboardDataCsv)
    return;

  expandBlocks(items, m_templates, kMaxExpandedSamples, m_expanded);
  if (m_expanded.empty())
    return;

  switch (static_cast<Publisher::Mode>(mode)) {
    case Publisher::Mode::DashboardDataJson:
      publishBatchAsJson(m_expanded);
      break;

    case Publisher::Mode::DashboardDataCsv:
      publishBatchAsCsv(m_expanded);
      break;

    case Publisher::Mode::ScriptDriven:
    case Publisher::Mode::RawRxData:
      break;
  }
}

//==================================================================================================
// Structure adoption
//==================================================================================================

/**
 * @brief Caches the frame-pool generation the next structure publish belongs to (spec 0074). The
 *        FrameBuilder emits this immediately before the paired structurePublished, both queued to
 *        this worker in order, so the value is current when setTemplateFrame runs the reconcile;
 *        the inner Frame does not carry the generation, which is why it arrives on its own signal.
 */
void MQTT::PublisherWorker::setStructureGeneration(quint64 generation)
{
  m_pendingStructureGeneration = generation;
}

/**
 * @brief Adopts one source's structure; MQTT publishes frame-shaped payloads (spec 0055 D5).
 */
void MQTT::PublisherWorker::setTemplateFrame(int sourceId, const DataModel::Frame& frame)
{
  DataModel::bind_frame_template(m_templates[sourceId], frame);
  registerSparkplugMetrics(frame);
}

//==================================================================================================
// Payload publishing
//==================================================================================================

/**
 * @brief Publishes the batch as one compact JSON document where each dataset's "value" and
 *        "numericValue" become arrays collecting every frame's sample in arrival order.
 */
void MQTT::PublisherWorker::publishBatchAsJson(const std::vector<DataModel::Frame>& items)
{
  QMqttTopicName topic(m_cfg.topicBase);
  if (!topic.isValid())
    return;

  const auto& latest = items.back();
  QJsonObject root   = DataModel::serialize(latest);

  QMap<int, QJsonArray> valuesByDataset;
  QMap<int, QJsonArray> numericByDataset;
  for (const auto& item : items) {
    for (const auto& g : item.groups) {
      for (const auto& d : g.datasets) {
        valuesByDataset[d.uniqueId].append(d.value.simplified());
        numericByDataset[d.uniqueId].append(d.numericValue);
      }
    }
  }

  QJsonArray groupArray = root.value(Keys::Groups).toArray();
  for (int gi = 0; gi < groupArray.size() && gi < static_cast<int>(latest.groups.size()); ++gi) {
    const auto& liveGroup  = latest.groups[gi];
    QJsonObject groupObj   = groupArray.at(gi).toObject();
    QJsonArray datasetsArr = groupObj.value(Keys::Datasets).toArray();
    for (int di = 0; di < datasetsArr.size() && di < static_cast<int>(liveGroup.datasets.size());
         ++di) {
      const int uid = liveGroup.datasets[di].uniqueId;
      QJsonObject d = datasetsArr.at(di).toObject();
      d.insert(Keys::Value, valuesByDataset.value(uid));
      d.insert(Keys::NumericValue, numericByDataset.value(uid));
      datasetsArr.replace(di, d);
    }

    groupObj.insert(Keys::Datasets, datasetsArr);
    groupArray.replace(gi, groupObj);
  }

  root.insert(Keys::Groups, groupArray);
  root.insert(QStringLiteral("frameCount"), static_cast<int>(items.size()));

  const QJsonDocument doc(root);
  publishAndCount(topic, doc.toJson(QJsonDocument::Compact));
}

/**
 * @brief Publishes every frame in the batch as concatenated CSV rows on topicBase.
 *        (Re)publishes the header retained on schema changes.
 */
void MQTT::PublisherWorker::publishBatchAsCsv(const std::vector<DataModel::Frame>& items)
{
  QMqttTopicName rowsTopic(m_cfg.topicBase);
  if (!rowsTopic.isValid())
    return;

  const auto& latest = items.back();
  if (m_csvHeaderDirty || latest.title != m_csvFrameTitle)
    rebuildCsvSchema(latest);

  if (m_csvHeaderPayload.isEmpty())
    return;

  if (m_csvHeaderDirty) {
    QMqttTopicName headerTopic(m_cfg.topicBase + QStringLiteral("/header"));
    if (headerTopic.isValid())
      m_client->publish(headerTopic, m_csvHeaderPayload, 0, true);

    m_csvHeaderDirty = false;
  }

  m_csvRowBuffer.resize(0);

  for (const auto& item : items) {
    for (const auto& g : item.groups)
      for (const auto& d : g.datasets)
        m_csvLastFinal[d.uniqueId] = d.value.simplified();

    appendCsvRow(m_csvRowBuffer, m_csvSchema, m_csvLastFinal);
  }

  if (!m_csvRowBuffer.isEmpty())
    publishAndCount(rowsTopic, m_csvRowBuffer);
}

/**
 * @brief (Re)builds the cached CSV header payload from the current frame.
 */
void MQTT::PublisherWorker::rebuildCsvSchema(const DataModel::Frame& frame)
{
  m_csvSchema     = DataModel::buildExportSchema(frame);
  m_csvFrameTitle = frame.title;
  m_csvLastFinal.clear();

  m_csvHeaderPayload = buildCsvHeader(m_csvSchema);
  if (m_csvHeaderPayload.isEmpty())
    return;

  m_csvHeaderDirty = true;
}

/**
 * @brief Compiles the script when scriptCode has changed since the last successful compile.
 */
void MQTT::PublisherWorker::recompileScriptIfNeeded()
{
  if (!m_script)
    return;

  const bool sameCode = (m_cfg.scriptCode == m_compiledScriptCode);
  const bool sameLang = (m_cfg.scriptLanguage == m_script->currentLanguage());
  if (sameCode && sameLang && m_script->isLoaded())
    return;

  m_compiledScriptCode = m_cfg.scriptCode;
  QString error;
  if (!m_script->compile(m_cfg.scriptCode, m_cfg.scriptLanguage, error))
    Q_EMIT scriptErrorOccurred(error);
}

/**
 * @brief Publishes a payload, increments the stats counters, and reports whether the broker
 *        accepted it, so a caller that must confirm delivery (the Sparkplug birth) can act on it.
 */
bool MQTT::PublisherWorker::publishAndCount(const QMqttTopicName& topic, const QByteArray& payload)
{
  const auto id = m_client->publish(topic, payload);
  qCDebug(lcMqttPub) << "publish topic=" << topic.name() << "size=" << payload.size() << "id=" << id
                     << "preview=" << payload.left(80);

  if (id < 0) {
    qCWarning(lcMqttPub) << "publish returned -1 for topic" << topic.name();
    return false;
  }

  if (m_messagesSent)
    m_messagesSent->fetch_add(1, std::memory_order_relaxed);

  if (m_bytesSent)
    m_bytesSent->fetch_add(static_cast<quint64>(payload.size()), std::memory_order_relaxed);

  return true;
}

/**
 * @brief Publishes a notification payload to the configured notification topic.
 */
void MQTT::PublisherWorker::publishNotificationOnWorker(const QString& topic,
                                                        const QByteArray& payload)
{
  if (!isResourceOpen() || topic.isEmpty())
    return;

  QMqttTopicName mqttTopic(topic);
  if (!mqttTopic.isValid())
    return;

  publishAndCount(mqttTopic, payload);
}

/**
 * @brief Publishes a user-supplied payload to an arbitrary topic.
 */
void MQTT::PublisherWorker::publishCustomOnWorker(const QString& topic,
                                                  const QByteArray& payload,
                                                  int qos,
                                                  bool retain)
{
  if (!isResourceOpen() || topic.isEmpty())
    return;

  QMqttTopicName mqttTopic(topic);
  if (!mqttTopic.isValid())
    return;

  const auto clampedQos = static_cast<quint8>(std::clamp(qos, 0, 2));
  if (m_client->publish(mqttTopic, payload, clampedQos, retain) >= 0) {
    if (m_messagesSent)
      m_messagesSent->fetch_add(1, std::memory_order_relaxed);

    if (m_bytesSent)
      m_bytesSent->fetch_add(static_cast<quint64>(payload.size()), std::memory_order_relaxed);
  }
}

//==================================================================================================
// Broker session
//==================================================================================================

/**
 * @brief Applies a fresh broker config snapshot. Reconnects when broker-affecting fields change.
 */
void MQTT::PublisherWorker::applyBrokerConfig(const MQTT::BrokerConfig& cfg)
{
  const bool brokerChanged =
    cfg.hostname != m_cfg.hostname || cfg.port != m_cfg.port || cfg.username != m_cfg.username
    || cfg.password != m_cfg.password || cfg.clientId != m_cfg.clientId
    || cfg.mqttVersion != m_cfg.mqttVersion || cfg.keepAlive != m_cfg.keepAlive
    || cfg.cleanSession != m_cfg.cleanSession || cfg.sslEnabled != m_cfg.sslEnabled
    || cfg.sslProtocol != m_cfg.sslProtocol || cfg.peerVerifyMode != m_cfg.peerVerifyMode
    || cfg.peerVerifyDepth != m_cfg.peerVerifyDepth || cfg.caCertificates != m_cfg.caCertificates
    || cfg.clientCertificate != m_cfg.clientCertificate
    || cfg.clientPrivateKey != m_cfg.clientPrivateKey || cfg.alpnProtocol != m_cfg.alpnProtocol
    || cfg.enabled != m_cfg.enabled || cfg.sparkplugEnabled != m_cfg.sparkplugEnabled
    || cfg.sparkplugGroupId != m_cfg.sparkplugGroupId
    || cfg.sparkplugEdgeNode != m_cfg.sparkplugEdgeNode
    || cfg.sparkplugDeviceId != m_cfg.sparkplugDeviceId;

  m_csvHeaderDirty = true;
  m_csvFrameTitle.clear();

  if (cfg.scriptCode != m_cfg.scriptCode && m_script)
    m_script->reset();

  m_cfg = cfg;

  if (!m_client)
    return;

  m_sslConfiguration.setProtocol(m_cfg.sslProtocol);
  m_sslConfiguration.setPeerVerifyMode(m_cfg.peerVerifyMode);
  m_sslConfiguration.setPeerVerifyDepth(m_cfg.peerVerifyDepth);
  applyTlsIdentity(m_sslConfiguration,
                   TlsIdentity{m_cfg.clientCertificate, m_cfg.clientPrivateKey},
                   m_cfg.alpnProtocol);
  if (!m_cfg.caCertificates.isEmpty()) {
    auto existing = m_sslConfiguration.caCertificates();
    for (const auto& cert : m_cfg.caCertificates)
      if (!existing.contains(cert))
        existing.append(cert);

    m_sslConfiguration.setCaCertificates(existing);
  }

  if (m_client->state() == QMqttClient::Disconnected) {
    applyClientPropertiesUnsafe();

    if (brokerChanged && m_cfg.enabled)
      openBroker();

    return;
  }

  if (!brokerChanged)
    return;

  if (!m_runner)
    return;

  m_runner->cancel();
  m_client->disconnectFromHost();
  m_runner->run(buildReconnectFlow());
}

/**
 * @brief Composes the reconnect a broker-setting change needs: wait out the disconnect, push the
 *        staged mirror, then reopen under the shared retry policy. Running it as one tree is what
 *        makes a second settings change supersede the first instead of racing it.
 */
Async::Task* MQTT::PublisherWorker::buildReconnectFlow()
{
  SS_ASSERT_LOG(m_client != nullptr);
  SS_ASSERT_LOG(m_runner != nullptr);

  auto* group = Async::sequential(QStringLiteral("mqtt-publisher-reconnect"));

  if (m_client->state() != QMqttClient::Disconnected) {
    auto* wait = Async::awaitSignal(QStringLiteral("broker-disconnect"));
    wait->onSuccess(m_client, &QMqttClient::disconnected);
    group->addChild(Async::timeout(wait, kBrokerDisconnectTimeoutMs, m_runner->clock()));
  }

  group->addChild(Async::invoke(QStringLiteral("broker-apply"), [this](QString& reason) {
    Q_UNUSED(reason);
    applyClientPropertiesUnsafe();
    return true;
  }));

  if (!m_cfg.enabled)
    return group;

  auto* attempt = Async::sequential(QStringLiteral("broker-open"));
  attempt->addChild(Async::invoke(QStringLiteral("broker-dial"), [this](QString& reason) {
    Q_UNUSED(reason);
    openBroker();
    return true;
  }));

  auto* connected = Async::awaitSignal(QStringLiteral("broker-connect"));
  connected->onSuccess(m_client, &QMqttClient::connected);
  connected->onFailure(
    m_client, &QMqttClient::disconnected, QStringLiteral("the broker closed the connection"));
  connected->setAbortHandler([this]() { m_client->disconnectFromHost(); });
  attempt->addChild(Async::timeout(connected, kBrokerConnectTimeoutMs, m_runner->clock()));

  group->addChild(Async::retry(attempt, Async::RetryPolicy::autoReconnect(), m_runner->clock()));
  return group;
}

/**
 * @brief Writes the staged broker properties into the client. Caller must guarantee
 *        state == Disconnected. The Sparkplug will is registered here for that reason: a will set
 *        after CONNECT never arms, and an unarmed will leaves the node reading online forever
 *        after an ungraceful disconnect (R42).
 */
void MQTT::PublisherWorker::applyClientPropertiesUnsafe()
{
  m_client->setHostname(m_cfg.hostname);
  m_client->setPort(m_cfg.port);
  m_client->setClientId(m_cfg.clientId);
  m_client->setUsername(m_cfg.username);
  m_client->setPassword(m_cfg.password);
  m_client->setKeepAlive(m_cfg.keepAlive);
  m_client->setCleanSession(m_cfg.cleanSession);
  m_client->setProtocolVersion(m_cfg.mqttVersion);
  configureSparkplugWill();
}

/**
 * @brief Opens the broker connection. No-op if disabled or already connecting/connected.
 */
void MQTT::PublisherWorker::openBroker()
{
  if (!m_client || !m_cfg.enabled)
    return;

  if (m_cfg.hostname.isEmpty() || m_cfg.port == 0)
    return;

  if (m_client->state() != QMqttClient::Disconnected)
    return;

  if (m_cfg.sslEnabled)
    m_client->connectToHostEncrypted(m_sslConfiguration);
  else
    m_client->connectToHost();
}

/**
 * @brief Closes the broker connection and cancels any reconnect still in flight.
 */
void MQTT::PublisherWorker::closeBroker()
{
  if (m_runner)
    m_runner->cancel();

  if (m_client && m_client->state() != QMqttClient::Disconnected)
    m_client->disconnectFromHost();
}

/**
 * @brief Runs an out-of-band connection probe using a throwaway QMqttClient.
 */
void MQTT::PublisherWorker::runTestConnection()
{
  if (m_cfg.hostname.isEmpty() || m_cfg.port == 0) {
    Q_EMIT testConnectionFinished(
      false, tr("Configure broker hostname and port before testing the connection."));
    return;
  }

  auto* tester = new QMqttClient(this);
  tester->setHostname(m_cfg.hostname);
  tester->setPort(m_cfg.port);
  tester->setClientId(m_cfg.clientId + QStringLiteral("-probe"));
  tester->setUsername(m_cfg.username);
  tester->setPassword(m_cfg.password);
  tester->setCleanSession(true);
  tester->setKeepAlive(5);
  tester->setProtocolVersion(m_cfg.mqttVersion);

  auto* timeout = new QTimer(tester);
  timeout->setSingleShot(true);
  timeout->setInterval(5000);

  auto done   = std::make_shared<bool>(false);
  auto report = [this, tester, done](bool ok, const QString& detail) {
    if (*done)
      return;

    *done = true;
    if (tester->state() != QMqttClient::Disconnected)
      tester->disconnectFromHost();

    Q_EMIT testConnectionFinished(ok, detail);

    tester->deleteLater();
  };

  connect(
    tester, &QMqttClient::stateChanged, this, [tester, report](QMqttClient::ClientState state) {
      if (state == QMqttClient::Connected)
        report(true,
               tr("Successfully connected to %1:%2.").arg(tester->hostname()).arg(tester->port()));
    });

  connect(tester, &QMqttClient::errorChanged, this, [report](QMqttClient::ClientError error) {
    if (error == QMqttClient::NoError)
      return;

    report(false, describeMqttError(error));
  });

  connect(timeout, &QTimer::timeout, this, [report] {
    report(false, tr("Timed out after 5 seconds without reaching the broker."));
  });

  timeout->start();
  if (m_cfg.sslEnabled)
    tester->connectToHostEncrypted(m_sslConfiguration);
  else
    tester->connectToHost();
}

/**
 * @brief Forwards the client state to the main thread, issuing the Sparkplug birth certificate on
 *        the transition into Connected: a host may only resolve aliases it saw in a birth, so the
 *        birth has to precede the first data message of every session (R40).
 */
void MQTT::PublisherWorker::onClientStateChanged(QMqttClient::ClientState state)
{
  if (state == QMqttClient::Connected && sparkplugActive()) {
    subscribeSparkplugCommands();
    publishSparkplugBirth();
  }

  Q_EMIT brokerStateChanged(static_cast<int>(state));
}

/**
 * @brief Forwards a broker error to the main thread as a human-readable string. A transport
 *        failure with a client identity configured names mutual TLS as the likely cause: a
 *        certificate/key mismatch or a broker-side rejection only surfaces at the handshake.
 */
void MQTT::PublisherWorker::onClientErrorChanged(QMqttClient::ClientError error)
{
  if (error == QMqttClient::NoError)
    return;

  QString message = describeMqttError(error);
  if (error == QMqttClient::TransportInvalid && !m_cfg.clientCertificate.isNull())
    message += QStringLiteral(" ")
             + Publisher::tr("A client certificate is configured: verify that it matches the "
                             "private key and is activated on the broker.");

  Q_EMIT brokerErrorOccurred(message);
}

//==================================================================================================
// Sparkplug: edge-node lifecycle
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
// Sparkplug: dataset publishing
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
                               csvColumnLabel(column),
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

#endif  // BUILD_COMMERCIAL
