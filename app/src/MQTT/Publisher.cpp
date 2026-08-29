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

#  include "MQTT/Publisher.h"

#  include <algorithm>
#  include <QApplication>
#  include <QFileDialog>
#  include <QJsonDocument>
#  include <QLoggingCategory>
#  include <QPointer>
#  include <QRandomGenerator>
#  include <QStandardPaths>

#  include "DataModel/ExportSchema.h"
#  include "DataModel/FrameBuilder.h"
#  include "DataModel/NotificationCenter.h"
#  include "DataModel/ProjectModel.h"
#  include "Licensing/CommercialToken.h"
#  include "Misc/Utilities.h"
#  include "MQTT/PublisherScript.h"
#  include "SSAssert.h"

Q_LOGGING_CATEGORY(lcMqttPub, "serialstudio.mqtt.publisher", QtCriticalMsg)

//--------------------------------------------------------------------------------------------------
// Constants: per-step deadlines. Attempts and backoff belong to the shared Async::RetryPolicy.
//--------------------------------------------------------------------------------------------------

static constexpr int kBrokerConnectTimeoutMs    = 15000;
static constexpr int kBrokerDisconnectTimeoutMs = 5000;

//==================================================================================================
// PublisherWorker
//==================================================================================================

/**
 * @brief Escapes a single field per RFC 4180; returns the original string if no escape is needed.
 */
QString MQTT::PublisherWorker::escapeCsvField(const QString& s)
{
  const bool needs = s.contains(QChar(',')) || s.contains(QChar('"')) || s.contains(QChar('\n'))
                  || s.contains(QChar('\r')) || s.contains(QChar('\t'));
  if (!needs)
    return s;

  QString out = s;
  out.replace(QChar('"'), QStringLiteral("\"\""));
  return QStringLiteral("\"%1\"").arg(out);
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
 * @brief Drains both the frame queue and the raw-bytes queue.
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

  expandBlocks(items);
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

/**
 * @brief Materialises the batch's blocks into one frame per sample, capped at kMaxExpandedSamples:
 *        a dense source can present millions of samples in one batch, and MQTT is a live feed, not
 *        a lossless recorder. A block whose source has published no structure yet is skipped.
 */
void MQTT::PublisherWorker::expandBlocks(const std::vector<DataModel::DataBlockPtr>& blocks)
{
  m_expanded.clear();

  for (const auto& block : blocks) {
    if (!block || block->samples <= 0)
      continue;

    const auto tpl = m_templates.find(block->sourceId);
    if (tpl == m_templates.end())
      continue;

    for (qsizetype i = 0; i < block->samples; ++i) {
      if (m_expanded.size() >= kMaxExpandedSamples)
        return;

      DataModel::apply_block_sample(tpl->second, *block, i);
      m_expanded.push_back(tpl->second.frame);
    }
  }
}

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

    for (size_t i = 0; i < m_csvSchema.columns.size(); ++i) {
      if (i > 0)
        m_csvRowBuffer.append(',');

      const int uid = m_csvSchema.columns[i].uniqueId;
      m_csvRowBuffer.append(escapeCsvField(m_csvLastFinal.value(uid, QString())).toUtf8());
    }

    m_csvRowBuffer.append('\n');
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
  m_csvHeaderPayload.clear();

  if (m_csvSchema.columns.empty())
    return;

  QByteArray header;
  header.reserve(256);

  for (size_t i = 0; i < m_csvSchema.columns.size(); ++i) {
    const auto& col = m_csvSchema.columns[i];
    QString label   = QStringLiteral("%1/%2").arg(col.groupTitle, col.title).simplified();
    if (!col.sourceTitle.isEmpty())
      label = col.sourceTitle + QStringLiteral("/") + label;

    if (i > 0)
      header.append(',');

    header.append(escapeCsvField(label).toUtf8());
  }

  header.append('\n');
  m_csvHeaderPayload = header;
  m_csvHeaderDirty   = true;
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
// Publisher (main thread)
//==================================================================================================

/**
 * @brief Constructs the publisher with safe broker defaults and starts the worker thread.
 */
MQTT::Publisher::Publisher()
  : DataModel::FrameConsumer<DataModel::DataBlockPtr>(
      {.queueCapacity = 8192, .flushThreshold = 1024, .timerIntervalMs = 100})
  , m_enabled(false)
  , m_sslEnabled(false)
  , m_publishNotifications(false)
  , m_cleanSession(true)
  , m_inApply(false)
  , m_skipNextSync(false)
  , m_savingToProjectModel(false)
  , m_reportConnectionErrors(false)
  , m_mode(static_cast<int>(Mode::RawRxData))
  , m_peerVerifyDepth(10)
  , m_publishFrequencyHz(kDefaultPublishHz)
  , m_protocolVersion(QMqttClient::MQTT_5_0)
  , m_sslProtocol(QSsl::SecureProtocols)
  , m_peerVerifyMode(QSslSocket::AutoVerifyPeer)
  , m_port(1883)
  , m_keepAlive(60)
  , m_customClientId(false)
  , m_hostname(QStringLiteral("127.0.0.1"))
  , m_scriptLanguage(0)
  , m_sparkplugEnabled(false)
  , m_alpnEnabled(false)
  , m_alpnProtocol(QStringLiteral("x-amzn-mqtt-ca"))
  , m_rawBytesQueue(8192)
  , m_rawFramesQueue(8192)
  , m_workerMode(static_cast<int>(Mode::RawRxData))
  , m_workerScriptLanguage(0)
  , m_isConnected(false)
  , m_messagesSent(0)
  , m_bytesSent(0)
  , m_messagesSentSeen(0)
{
  qRegisterMetaType<MQTT::BrokerConfig>("MQTT::BrokerConfig");
  qRegisterMetaType<QMqttClient::ClientState>("QMqttClient::ClientState");
  qRegisterMetaType<QMqttClient::ClientError>("QMqttClient::ClientError");

  m_mqttVersions.insert(tr("MQTT 3.1"), QMqttClient::MQTT_3_1);
  m_mqttVersions.insert(tr("MQTT 3.1.1"), QMqttClient::MQTT_3_1_1);
  m_mqttVersions.insert(tr("MQTT 5.0"), QMqttClient::MQTT_5_0);

  m_sslProtocols.insert(tr("TLS 1.2"), QSsl::TlsV1_2);
  m_sslProtocols.insert(tr("TLS 1.3"), QSsl::TlsV1_3);
  m_sslProtocols.insert(tr("TLS 1.3 or Later"), QSsl::TlsV1_3OrLater);
  m_sslProtocols.insert(tr("DTLS 1.2 or Later"), QSsl::DtlsV1_2OrLater);
  m_sslProtocols.insert(tr("Any Protocol"), QSsl::AnyProtocol);
  m_sslProtocols.insert(tr("Secure Protocols Only"), QSsl::SecureProtocols);

  m_peerVerifyModes.insert(tr("None"), QSslSocket::VerifyNone);
  m_peerVerifyModes.insert(tr("Query Peer"), QSslSocket::QueryPeer);
  m_peerVerifyModes.insert(tr("Verify Peer"), QSslSocket::VerifyPeer);
  m_peerVerifyModes.insert(tr("Auto Verify Peer"), QSslSocket::AutoVerifyPeer);

  m_syncTimer.setSingleShot(true);
  m_syncTimer.setInterval(kSyncDebounceMs);
  connect(&m_syncTimer, &QTimer::timeout, this, &Publisher::syncToWorker);

  m_statsTimer.setInterval(kStatsTickMs);
  connect(&m_statsTimer, &QTimer::timeout, this, &Publisher::emitStatsIfChanged);
  m_statsTimer.start();

  regenerateClientId();

  initializeWorker();
  applyTimerInterval();

  auto* w = static_cast<PublisherWorker*>(m_worker);
  connect(w, &PublisherWorker::brokerStateChanged, this, &Publisher::onWorkerBrokerStateChanged);
  connect(w, &PublisherWorker::brokerErrorOccurred, this, &Publisher::onWorkerBrokerError);
  connect(w, &PublisherWorker::scriptErrorOccurred, this, &Publisher::onWorkerScriptError);
  connect(
    w, &PublisherWorker::testConnectionFinished, this, &Publisher::onWorkerTestConnectionFinished);

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  connect(&frameBuilder,
          &DataModel::FrameBuilder::structureGenerationChanged,
          w,
          &PublisherWorker::setStructureGeneration,
          Qt::QueuedConnection);
  connect(&frameBuilder,
          &DataModel::FrameBuilder::structurePublished,
          w,
          &PublisherWorker::setTemplateFrame,
          Qt::QueuedConnection);

  QMetaObject::invokeMethod(w, &PublisherWorker::bootstrap, Qt::QueuedConnection);
}

/**
 * @brief Destructor; the FrameConsumer base waits for the worker thread to finish.
 */
MQTT::Publisher::~Publisher() = default;

/**
 * @brief Returns the singleton instance of the MQTT Publisher.
 */
MQTT::Publisher& MQTT::Publisher::instance()
{
  static Publisher singleton;
  return singleton;
}

/**
 * @brief FrameConsumer factory method; creates the worker for our threading wrapper.
 */
DataModel::FrameConsumerWorkerBase* MQTT::Publisher::createWorker()
{
  return new PublisherWorker(&m_pendingQueue,
                             &m_consumerEnabled,
                             &m_queueSize,
                             &m_rawBytesQueue,
                             &m_rawFramesQueue,
                             &m_workerMode,
                             &m_workerScriptLanguage,
                             &m_messagesSent,
                             &m_bytesSent);
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the publisher is enabled for the current project.
 */
bool MQTT::Publisher::enabled() const noexcept
{
  return m_enabled;
}

/**
 * @brief Returns whether SSL/TLS is enabled.
 */
bool MQTT::Publisher::sslEnabled() const noexcept
{
  return m_sslEnabled;
}

/**
 * @brief Returns true while the broker connection is established.
 */
bool MQTT::Publisher::isConnected() const
{
  return m_isConnected.load(std::memory_order_relaxed);
}

/**
 * @brief Returns the broker clean-session flag.
 */
bool MQTT::Publisher::cleanSession() const noexcept
{
  return m_cleanSession;
}

/**
 * @brief Returns whether notifications are mirrored to MQTT.
 */
bool MQTT::Publisher::publishNotifications() const noexcept
{
  return m_publishNotifications;
}

/**
 * @brief Returns the configured publisher mode (Mode enum index).
 */
int MQTT::Publisher::mode() const noexcept
{
  return m_mode;
}

/**
 * @brief Returns the SSL peer-verification depth.
 */
int MQTT::Publisher::peerVerifyDepth() const noexcept
{
  return m_peerVerifyDepth;
}

/**
 * @brief Returns the configured publish rate in Hz.
 */
int MQTT::Publisher::publishFrequency() const noexcept
{
  return m_publishFrequencyHz;
}

/**
 * @brief Returns the selected MQTT protocol version index.
 */
quint8 MQTT::Publisher::mqttVersion() const noexcept
{
  quint8 index = 0;
  for (auto i = m_mqttVersions.begin(); i != m_mqttVersions.end(); ++i) {
    if (i.value() == m_protocolVersion)
      break;

    ++index;
  }

  return index;
}

/**
 * @brief Returns the selected SSL protocol index.
 */
quint8 MQTT::Publisher::sslProtocol() const noexcept
{
  quint8 index = 0;
  for (auto i = m_sslProtocols.begin(); i != m_sslProtocols.end(); ++i) {
    if (i.value() == m_sslProtocol)
      break;

    ++index;
  }

  return index;
}

/**
 * @brief Returns the selected SSL peer-verification mode index.
 */
quint8 MQTT::Publisher::peerVerifyMode() const noexcept
{
  quint8 index = 0;
  for (auto i = m_peerVerifyModes.begin(); i != m_peerVerifyModes.end(); ++i) {
    if (i.value() == m_peerVerifyMode)
      break;

    ++index;
  }

  return index;
}

/**
 * @brief Returns the broker TCP port.
 */
quint16 MQTT::Publisher::port() const noexcept
{
  return m_port;
}

/**
 * @brief Returns the keep-alive interval in seconds.
 */
quint16 MQTT::Publisher::keepAlive() const noexcept
{
  return m_keepAlive;
}

/**
 * @brief Returns the MQTT client identifier.
 */
QString MQTT::Publisher::clientId() const
{
  return m_clientId;
}

/**
 * @brief Returns true when the user has opted to manage the client id manually.
 */
bool MQTT::Publisher::customClientId() const noexcept
{
  return m_customClientId;
}

/**
 * @brief Returns the broker hostname.
 */
QString MQTT::Publisher::hostname() const
{
  return m_hostname;
}

/**
 * @brief Returns the broker authentication username.
 */
QString MQTT::Publisher::username() const
{
  return m_username;
}

/**
 * @brief Returns the broker authentication password.
 */
QString MQTT::Publisher::password() const
{
  return m_password;
}

/**
 * @brief Returns the client certificate PEM path used for mutual TLS (empty = off).
 */
QString MQTT::Publisher::clientCertificatePath() const
{
  return m_clientCertificatePath;
}

/**
 * @brief Returns the private key PEM path (empty = look in the certificate file).
 */
QString MQTT::Publisher::privateKeyPath() const
{
  return m_privateKeyPath;
}

/**
 * @brief Returns the private-key passphrase (kept in the encrypted vault, never in projects).
 */
QString MQTT::Publisher::keyPassphrase() const
{
  return m_keyPassphrase;
}

/**
 * @brief Returns whether ALPN is requested during the TLS handshake (MQTT over port 443).
 */
bool MQTT::Publisher::alpnEnabled() const noexcept
{
  return m_alpnEnabled;
}

/**
 * @brief Returns the ALPN protocol name announced when ALPN is enabled.
 */
QString MQTT::Publisher::alpnProtocol() const
{
  return m_alpnProtocol;
}

/**
 * @brief Returns the base MQTT topic for dashboard or raw publishing.
 */
QString MQTT::Publisher::topicBase() const
{
  return m_topicBase;
}

/**
 * @brief Returns the dedicated MQTT topic for notifications.
 */
QString MQTT::Publisher::notificationTopic() const
{
  return m_notificationTopic;
}

/**
 * @brief Returns the user script used in ScriptDriven mode.
 */
QString MQTT::Publisher::scriptCode() const
{
  return m_scriptCode;
}

/**
 * @brief Returns the per-script override topic (empty == use topicBase).
 */
QString MQTT::Publisher::scriptTopic() const
{
  return m_scriptTopic;
}

/**
 * @brief Returns the script language enum (0=JavaScript, 1=Lua).
 */
int MQTT::Publisher::scriptLanguage() const noexcept
{
  return m_scriptLanguage;
}

/**
 * @brief Returns the localized label of the active mode.
 */
QString MQTT::Publisher::modeLabel() const
{
  const auto& list = modes();
  if (m_mode >= 0 && m_mode < list.size())
    return list.at(m_mode);

  return QString();
}

/**
 * @brief Returns "host:port" for compact display in the status popup.
 */
QString MQTT::Publisher::brokerEndpoint() const
{
  return QStringLiteral("%1:%2").arg(m_hostname, QString::number(m_port));
}

/**
 * @brief Returns the number of messages successfully published since the singleton booted.
 */
quint64 MQTT::Publisher::messagesSent() const noexcept
{
  return m_messagesSent.load(std::memory_order_relaxed);
}

/**
 * @brief Returns the canonical starter script shown in the editor when no code is saved.
 */
QString MQTT::Publisher::defaultScriptTemplate()
{
  return QStringLiteral(
    "// Called once per parsed frame. Return the payload to publish to the broker.\n"
    "// Return null or undefined to skip this frame.\n"
    "//\n"
    "// The `frame` argument is the JSON shape produced by Frame::serialize().\n"
    "function mqtt(frame) {\n"
    "  return JSON.stringify(frame);\n"
    "}\n");
}

/**
 * @brief Returns the available publisher modes (display names).
 */
const QStringList& MQTT::Publisher::modes() const
{
  static QStringList list;
  if (list.isEmpty()) {
    list.append(tr("Raw RX Data"));
    list.append(tr("Custom Script"));
    list.append(tr("Dashboard Data (CSV)"));
    list.append(tr("Dashboard Data (JSON)"));
  }

  return list;
}

/**
 * @brief Returns the available MQTT protocol versions (display names).
 */
const QStringList& MQTT::Publisher::mqttVersions() const
{
  static QStringList list;
  if (list.isEmpty())
    for (auto i = m_mqttVersions.begin(); i != m_mqttVersions.end(); ++i)
      list.append(i.key());

  return list;
}

/**
 * @brief Returns the available SSL/TLS protocols (display names).
 */
const QStringList& MQTT::Publisher::sslProtocols() const
{
  static QStringList list;
  if (list.isEmpty())
    for (auto i = m_sslProtocols.begin(); i != m_sslProtocols.end(); ++i)
      list.append(i.key());

  return list;
}

/**
 * @brief Returns the available SSL peer-verification modes.
 */
const QStringList& MQTT::Publisher::peerVerifyModes() const
{
  static QStringList list;
  if (list.isEmpty())
    for (auto i = m_peerVerifyModes.begin(); i != m_peerVerifyModes.end(); ++i)
      list.append(i.key());

  return list;
}

//--------------------------------------------------------------------------------------------------
// Project-config serialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes the current configuration as a JSON object for project storage.
 */
QJsonObject MQTT::Publisher::toJson() const
{
  QJsonObject obj;
  obj.insert(kKeyEnabled, m_enabled);
  obj.insert(kKeyMode, m_mode);
  obj.insert(kKeyPublishNotifications, m_publishNotifications);
  obj.insert(kKeyPublishFrequency, m_publishFrequencyHz);
  obj.insert(kKeyTopicBase, m_topicBase);
  obj.insert(kKeyNotificationTopic, m_notificationTopic);
  obj.insert(kKeyScriptCode, m_scriptCode);
  obj.insert(kKeyScriptTopic, m_scriptTopic);
  obj.insert(kKeyScriptLanguage, m_scriptLanguage);
  obj.insert(kKeyHostname, m_hostname);
  obj.insert(kKeyPort, static_cast<int>(m_port));
  obj.insert(kKeyCustomClientId, m_customClientId);

  if (m_customClientId)
    obj.insert(kKeyClientId, m_clientId);

  obj.insert(kKeyCleanSession, m_cleanSession);
  obj.insert(kKeyKeepAlive, static_cast<int>(m_keepAlive));
  obj.insert(kKeyMqttVersion, static_cast<int>(mqttVersion()));
  obj.insert(kKeySslEnabled, m_sslEnabled);
  obj.insert(kKeySslProtocol, static_cast<int>(sslProtocol()));
  obj.insert(kKeyPeerVerifyMode, static_cast<int>(peerVerifyMode()));
  obj.insert(kKeyPeerVerifyDepth, m_peerVerifyDepth);
  obj.insert(kKeyClientCertPath, m_clientCertificatePath);
  obj.insert(kKeyPrivateKeyPath, m_privateKeyPath);
  obj.insert(kKeyAlpnEnabled, m_alpnEnabled);
  obj.insert(kKeyAlpnProtocol, m_alpnProtocol);
  obj.insert(kKeySparkplugEnabled, m_sparkplugEnabled);
  obj.insert(kKeySparkplugGroupId, m_sparkplugGroupId);
  obj.insert(kKeySparkplugEdgeNodeId, m_sparkplugEdgeNodeId);
  obj.insert(kKeySparkplugDeviceId, m_sparkplugDeviceId);
  return obj;
}

/**
 * @brief Loads configuration from a project-supplied JSON object.
 */
void MQTT::Publisher::applyProjectConfig(const QJsonObject& cfg)
{
  m_inApply = true;

  setEnabled(cfg.value(kKeyEnabled).toBool(false));
  setMode(cfg.value(kKeyMode).toInt(static_cast<int>(Mode::RawRxData)));
  setPublishNotifications(cfg.value(kKeyPublishNotifications).toBool(false));
  setPublishFrequency(cfg.value(kKeyPublishFrequency).toInt(kDefaultPublishHz));
  setTopicBase(cfg.value(kKeyTopicBase).toString());
  setNotificationTopic(cfg.value(kKeyNotificationTopic).toString());
  setScriptCode(cfg.value(kKeyScriptCode).toString());
  setScriptTopic(cfg.value(kKeyScriptTopic).toString());
  setScriptLanguage(cfg.value(kKeyScriptLanguage).toInt(0));

  setHostname(cfg.value(kKeyHostname).toString(QStringLiteral("127.0.0.1")));
  setPort(static_cast<quint16>(cfg.value(kKeyPort).toInt(1883)));

  const bool customCid = cfg.value(kKeyCustomClientId).toBool(false);
  setCustomClientId(customCid);

  const auto cid = cfg.value(kKeyClientId).toString();
  if (customCid && !cid.isEmpty())
    setClientId(cid);
  else
    regenerateClientId();

  reloadCredentialsFromVault();

  setCleanSession(cfg.value(kKeyCleanSession).toBool(true));
  setKeepAlive(static_cast<quint16>(cfg.value(kKeyKeepAlive).toInt(60)));
  setMqttVersion(static_cast<quint8>(cfg.value(kKeyMqttVersion).toInt(2)));

  setSslEnabled(cfg.value(kKeySslEnabled).toBool(false));
  setSslProtocol(static_cast<quint8>(cfg.value(kKeySslProtocol).toInt(5)));
  setPeerVerifyMode(static_cast<quint8>(cfg.value(kKeyPeerVerifyMode).toInt(3)));
  setPeerVerifyDepth(cfg.value(kKeyPeerVerifyDepth).toInt(10));

  setAlpnEnabled(cfg.value(kKeyAlpnEnabled).toBool(false));
  setAlpnProtocol(cfg.value(kKeyAlpnProtocol).toString(QStringLiteral("x-amzn-mqtt-ca")));

  setSparkplugEnabled(cfg.value(kKeySparkplugEnabled).toBool(false));
  setSparkplugGroupId(cfg.value(kKeySparkplugGroupId).toString());
  setSparkplugEdgeNodeId(cfg.value(kKeySparkplugEdgeNodeId).toString());
  setSparkplugDeviceId(cfg.value(kKeySparkplugDeviceId).toString());

  m_clientCertificatePath = cfg.value(kKeyClientCertPath).toString();
  m_privateKeyPath        = cfg.value(kKeyPrivateKeyPath).toString();
  reloadTlsIdentity(false);

  m_inApply = false;

  m_savingToProjectModel    = true;
  static auto& projectModel = DataModel::ProjectModel::instance();
  projectModel.setMqttPublisher(toJson());
  m_savingToProjectModel = false;

  m_skipNextSync = true;
  Q_EMIT configurationChanged();
  m_skipNextSync = false;

  m_reportConnectionErrors = m_enabled;
  m_syncTimer.stop();
  syncToWorker();
}

/**
 * @brief Resets the publisher to default state (used on project close / new).
 */
void MQTT::Publisher::resetProjectConfig()
{
  applyProjectConfig(QJsonObject{});
}

//--------------------------------------------------------------------------------------------------
// Connection lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Triggers a one-shot probe connection on the worker.
 */
void MQTT::Publisher::testConnection()
{
  if (!licenseValid()) {
    Misc::Utilities::showMessageBox(
      tr("MQTT publisher unavailable"),
      tr("A valid commercial license is required to use MQTT publishing."),
      QMessageBox::Warning,
      tr("MQTT Test Connection"));
    return;
  }

  if (m_syncTimer.isActive()) {
    m_syncTimer.stop();
    syncToWorker();
  }

  QMetaObject::invokeMethod(m_worker, "runTestConnection", Qt::QueuedConnection);
}

/**
 * @brief Opens a folder picker to load additional CA certificates.
 */
void MQTT::Publisher::addCaCertificates()
{
  auto* dialog =
    new QFileDialog(qApp->activeWindow(),
                    tr("Select PEM Certificates Directory"),
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

  dialog->setFileMode(QFileDialog::Directory);
  dialog->setOption(QFileDialog::ShowDirsOnly, true);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this,
      [this, path]() {
        QDir dir(path);
        if (!dir.exists())
          return;

        const auto entries =
          dir.entryInfoList({"*.pem", "*.crt", "*.cer"}, QDir::Files | QDir::Readable);
        for (const auto& info : entries) {
          QFile f(info.absoluteFilePath());
          if (!f.open(QIODevice::ReadOnly))
            continue;

          const auto data = f.readAll();
          const auto pem  = QSslCertificate::fromData(data, QSsl::Pem);
          const auto der  = QSslCertificate::fromData(data, QSsl::Der);
          for (const auto& cert : pem)
            if (!cert.isNull() && !m_caCertificates.contains(cert))
              m_caCertificates.append(cert);

          for (const auto& cert : der)
            if (!cert.isNull() && !m_caCertificates.contains(cert))
              m_caCertificates.append(cert);
        }

        scheduleSyncToWorker();
      },
      Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Opens a PEM file picker and routes the selection into the given path setter. The work
 *        runs through a queued invoke: on macOS fileSelected fires inside QFileDialog::done()
 *        and re-entering Qt synchronously can delete the dialog under the native panel.
 */
void MQTT::Publisher::selectPemFile(const QString& title, void (Publisher::*setter)(const QString&))
{
  SS_ASSERT(setter != nullptr, return);
  SS_ASSERT(!title.isEmpty(), return);

  auto* dialog =
    new QFileDialog(qApp->activeWindow(),
                    title,
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                    tr("PEM files (*.pem *.crt *.cer *.key);;All files (*)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this, setter](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this,
      [this, setter, path]() {
        (this->*setter)(path);
        reloadTlsIdentity(true);
      },
      Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Opens a file picker for the mutual-TLS client certificate.
 */
void MQTT::Publisher::selectClientCertificate()
{
  selectPemFile(tr("Select Client Certificate"), &Publisher::setClientCertificatePath);
}

/**
 * @brief Opens a file picker for the mutual-TLS private key.
 */
void MQTT::Publisher::selectPrivateKey()
{
  selectPemFile(tr("Select Private Key"), &Publisher::setPrivateKeyPath);
}

/**
 * @brief Generates a fresh random 16-char client ID into the auto slot.
 */
void MQTT::Publisher::regenerateClientId()
{
  QString id;
  constexpr int length  = 16;
  const QString charset = QStringLiteral("abcdefghijklmnopqrstuvwxyz0123456789");
  for (int i = 0; i < length; ++i) {
    const int index = QRandomGenerator::global()->bounded(charset.length());
    id.append(charset.at(index));
  }

  m_autoClientId = id;
  if (!m_customClientId)
    setClientId(id);
}

//--------------------------------------------------------------------------------------------------
// External wiring
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires the publisher to NotificationCenter and ProjectModel so config loads on
 *        project open and edits round-trip back into the project file.
 */
void MQTT::Publisher::setupExternalConnections()
{
  connect(&DataModel::NotificationCenter::instance(),
          &DataModel::NotificationCenter::notificationPosted,
          this,
          &Publisher::onNotificationPosted);

  auto& projectModel = DataModel::ProjectModel::instance();
  connect(&projectModel, &DataModel::ProjectModel::mqttPublisherChanged, this, [this] {
    if (m_savingToProjectModel)
      return;

    applyProjectConfig(DataModel::ProjectModel::instance().mqttPublisher());
  });
  connect(this, &Publisher::configurationChanged, this, [this] {
    if (m_skipNextSync)
      return;

    m_savingToProjectModel = true;
    DataModel::ProjectModel::instance().setMqttPublisher(toJson());
    m_savingToProjectModel = false;
  });

  applyProjectConfig(projectModel.mqttPublisher());
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enables or disables MQTT publishing for the current project.
 */
void MQTT::Publisher::setEnabled(const bool enabled)
{
  if (m_enabled == enabled)
    return;

  m_enabled = enabled;
  setConsumerEnabled(enabled);

  if (enabled) {
    m_messagesSent.store(0, std::memory_order_relaxed);
    m_bytesSent.store(0, std::memory_order_relaxed);
    m_messagesSentSeen = 0;
    Q_EMIT statsChanged();
  }

  markConfigChanged();
}

/**
 * @brief Sets the publisher mode (DashboardData / RawRxData).
 */
void MQTT::Publisher::setMode(const int mode)
{
  if (m_mode == mode)
    return;

  m_mode = mode;
  m_workerMode.store(mode, std::memory_order_relaxed);
  markConfigChanged();
}

/**
 * @brief Enables or disables SSL/TLS for the broker connection.
 */
void MQTT::Publisher::setSslEnabled(const bool enabled)
{
  if (m_sslEnabled == enabled)
    return;

  m_sslEnabled = enabled;
  markConfigChanged();
}

/**
 * @brief Sets the broker clean-session flag.
 */
void MQTT::Publisher::setCleanSession(const bool cleanSession)
{
  if (m_cleanSession == cleanSession)
    return;

  m_cleanSession = cleanSession;
  markConfigChanged();
}

/**
 * @brief Toggles publishing of notifications to MQTT.
 */
void MQTT::Publisher::setPublishNotifications(const bool publish)
{
  if (m_publishNotifications == publish)
    return;

  m_publishNotifications = publish;
  markConfigChanged();
}

/**
 * @brief Sets the SSL peer-verification chain depth.
 */
void MQTT::Publisher::setPeerVerifyDepth(const int depth)
{
  if (m_peerVerifyDepth == depth)
    return;

  m_peerVerifyDepth = depth;
  markConfigChanged();
}

/**
 * @brief Sets the publish frequency in Hz; clamped to [kMinPublishHz, kMaxPublishHz].
 */
void MQTT::Publisher::setPublishFrequency(const int hz)
{
  const int clamped = std::clamp(hz, kMinPublishHz, kMaxPublishHz);
  if (m_publishFrequencyHz == clamped)
    return;

  m_publishFrequencyHz = clamped;
  applyTimerInterval();
  markConfigChanged();
}

/**
 * @brief Sets the MQTT protocol version by index.
 */
void MQTT::Publisher::setMqttVersion(const quint8 version)
{
  quint8 index = 0;
  for (auto i = m_mqttVersions.begin(); i != m_mqttVersions.end(); ++i) {
    if (index == version) {
      if (i.value() == m_protocolVersion)
        return;

      m_protocolVersion = i.value();
      markConfigChanged();
      return;
    }

    ++index;
  }
}

/**
 * @brief Sets the SSL protocol by index.
 */
void MQTT::Publisher::setSslProtocol(const quint8 protocol)
{
  quint8 index = 0;
  for (auto i = m_sslProtocols.begin(); i != m_sslProtocols.end(); ++i) {
    if (index == protocol) {
      if (i.value() == m_sslProtocol)
        return;

      m_sslProtocol = i.value();
      markConfigChanged();
      return;
    }

    ++index;
  }
}

/**
 * @brief Sets the SSL peer-verification mode by index.
 */
void MQTT::Publisher::setPeerVerifyMode(const quint8 verifyMode)
{
  quint8 index = 0;
  for (auto i = m_peerVerifyModes.begin(); i != m_peerVerifyModes.end(); ++i) {
    if (index == verifyMode) {
      if (i.value() == m_peerVerifyMode)
        return;

      if (i.value() == QSslSocket::VerifyNone) [[unlikely]]
        qWarning() << "[MQTT publisher] TLS peer verification disabled -- vulnerable to MITM";

      m_peerVerifyMode = i.value();
      markConfigChanged();
      return;
    }

    ++index;
  }
}

/**
 * @brief Sets the broker TCP port.
 */
void MQTT::Publisher::setPort(const quint16 port)
{
  if (m_port == port)
    return;

  m_port = port;

  if (!m_inApply) {
    reloadCredentialsFromVault();
    reloadTlsIdentity(false);
  }

  markConfigChanged();
}

/**
 * @brief Sets the keep-alive interval in seconds.
 */
void MQTT::Publisher::setKeepAlive(const quint16 keepAlive)
{
  if (m_keepAlive == keepAlive)
    return;

  m_keepAlive = keepAlive;
  markConfigChanged();
}

/**
 * @brief Sets the MQTT client identifier.
 */
void MQTT::Publisher::setClientId(const QString& id)
{
  if (m_clientId == id)
    return;

  m_clientId = id;
  markConfigChanged();
}

/**
 * @brief Toggles user management of the client id.
 */
void MQTT::Publisher::setCustomClientId(const bool custom)
{
  if (m_customClientId == custom)
    return;

  m_customClientId = custom;

  if (!custom) {
    if (m_autoClientId.isEmpty())
      regenerateClientId();
    else
      setClientId(m_autoClientId);
  }

  markConfigChanged();
}

/**
 * @brief Sets the broker hostname.
 */
void MQTT::Publisher::setHostname(const QString& hostname)
{
  if (m_hostname == hostname)
    return;

  m_hostname = hostname;

  if (!m_inApply) {
    reloadCredentialsFromVault();
    reloadTlsIdentity(false);
  }

  markConfigChanged();
}

/**
 * @brief Sets the broker authentication username.
 */
void MQTT::Publisher::setUsername(const QString& username)
{
  if (m_username == username)
    return;

  m_username = username;
  persistCredentialsToVault();
  markConfigChanged();
}

/**
 * @brief Sets the broker authentication password.
 */
void MQTT::Publisher::setPassword(const QString& password)
{
  if (m_password == password)
    return;

  m_password = password;
  persistCredentialsToVault();
  markConfigChanged();
}

/**
 * @brief Sets the client certificate PEM path and reloads the parsed TLS identity.
 */
void MQTT::Publisher::setClientCertificatePath(const QString& path)
{
  if (m_clientCertificatePath == path)
    return;

  m_clientCertificatePath = path;
  reloadTlsIdentity(false);
  markConfigChanged();
}

/**
 * @brief Sets the private key PEM path and reloads the parsed TLS identity.
 */
void MQTT::Publisher::setPrivateKeyPath(const QString& path)
{
  if (m_privateKeyPath == path)
    return;

  m_privateKeyPath = path;
  reloadTlsIdentity(false);
  markConfigChanged();
}

/**
 * @brief Sets the private-key passphrase, persists it to the vault and re-parses the key.
 */
void MQTT::Publisher::setKeyPassphrase(const QString& passphrase)
{
  if (m_keyPassphrase == passphrase)
    return;

  m_keyPassphrase = passphrase;
  persistCredentialsToVault();
  reloadTlsIdentity(false);
  markConfigChanged();
}

/**
 * @brief Enables or disables ALPN announcement during the TLS handshake.
 */
void MQTT::Publisher::setAlpnEnabled(const bool enabled)
{
  if (m_alpnEnabled == enabled)
    return;

  m_alpnEnabled = enabled;
  markConfigChanged();
}

/**
 * @brief Sets the ALPN protocol name (AWS IoT uses "x-amzn-mqtt-ca" on port 443).
 */
void MQTT::Publisher::setAlpnProtocol(const QString& protocol)
{
  if (m_alpnProtocol == protocol)
    return;

  m_alpnProtocol = protocol;
  markConfigChanged();
}

/**
 * @brief Sets the base topic for frame/raw publishing.
 */
void MQTT::Publisher::setTopicBase(const QString& topic)
{
  if (m_topicBase == topic)
    return;

  m_topicBase = topic;
  markConfigChanged();
}

/**
 * @brief Sets the notification publishing topic.
 */
void MQTT::Publisher::setNotificationTopic(const QString& topic)
{
  if (m_notificationTopic == topic)
    return;

  m_notificationTopic = topic;
  markConfigChanged();
}

/**
 * @brief Sets the user script source for ScriptDriven mode.
 */
void MQTT::Publisher::setScriptCode(const QString& code)
{
  if (m_scriptCode == code)
    return;

  m_scriptCode = code;
  markConfigChanged();
}

/**
 * @brief Sets the per-script override topic (empty == fall back to topicBase).
 */
void MQTT::Publisher::setScriptTopic(const QString& topic)
{
  if (m_scriptTopic == topic)
    return;

  m_scriptTopic = topic;
  markConfigChanged();
}

/**
 * @brief Sets the script language (0 = JavaScript, 1 = Lua).
 */
void MQTT::Publisher::setScriptLanguage(const int language)
{
  if (m_scriptLanguage == language)
    return;

  m_scriptLanguage = language;
  m_workerScriptLanguage.store(language, std::memory_order_relaxed);
  markConfigChanged();
}

//--------------------------------------------------------------------------------------------------
// Data publishing hotpaths
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enqueues the frame for the worker; rate-limiting and broker I/O happen off-main. An edge
 *        node addresses its own spBv1.0 topics and declares datasets whatever the payload mode is,
 *        so Sparkplug takes the block on its own terms; with Sparkplug off the gate is unchanged.
 */
void MQTT::Publisher::ingestBlock(const DataModel::DataBlockPtr& block)
{
  if (!m_enabled || !licenseValid()) [[likely]]
    return;

  if (!m_sparkplugEnabled && m_mode != static_cast<int>(Mode::DashboardDataJson)
      && m_mode != static_cast<int>(Mode::DashboardDataCsv))
    return;

  if (!block || (!m_sparkplugEnabled && m_topicBase.isEmpty()))
    return;

  enqueueData(block);
}

/**
 * @brief Enqueues raw driver bytes for the worker (RawRxData mode only). A Sparkplug edge node
 *        publishes its own namespace, so the raw queue is dead under it: gate the enqueue off
 *        rather than filling a queue only discardSuppressedPayloads will drain.
 */
void MQTT::Publisher::hotpathTxRawBytes(int deviceId, const IO::CapturedDataPtr& data)
{
  if (!m_enabled || !licenseValid()) [[likely]]
    return;

  if (m_sparkplugEnabled || m_mode != static_cast<int>(Mode::RawRxData))
    return;

  if (!data || data->data.isEmpty() || m_topicBase.isEmpty())
    return;

  TimestampedRawBytes item{deviceId, data};
  m_rawBytesQueue.try_enqueue(std::move(item));
}

/**
 * @brief Enqueues a delimited frame's raw bytes for the script mode worker. Suppressed while a
 *        Sparkplug edge node owns the session, whose own namespace leaves the script queue dead.
 */
void MQTT::Publisher::hotpathTxRawFrame(int deviceId, const IO::CapturedDataPtr& data)
{
  if (!m_enabled || !licenseValid()) [[likely]]
    return;

  if (m_sparkplugEnabled || m_mode != static_cast<int>(Mode::ScriptDriven))
    return;

  if (!data || data->data.isEmpty() || m_topicBase.isEmpty())
    return;

  TimestampedRawBytes item{deviceId, data};
  m_rawFramesQueue.try_enqueue(std::move(item));
}

/**
 * @brief Forwards a posted notification to the worker.
 */
void MQTT::Publisher::onNotificationPosted(const QVariantMap& event)
{
  if (!m_enabled || !m_publishNotifications || !licenseValid()) [[likely]]
    return;

  const QString topic = m_notificationTopic.isEmpty() ? m_topicBase : m_notificationTopic;
  if (topic.isEmpty())
    return;

  const QJsonDocument doc(QJsonObject::fromVariantMap(event));
  const QByteArray payload = doc.toJson(QJsonDocument::Compact);

  QMetaObject::invokeMethod(m_worker,
                            "publishNotificationOnWorker",
                            Qt::QueuedConnection,
                            Q_ARG(QString, topic),
                            Q_ARG(QByteArray, payload));
}

/**
 * @brief Public publish slot used by scripts and tools.
 */
qint64 MQTT::Publisher::mqttPublish(const QString& topic,
                                    const QByteArray& payload,
                                    int qos,
                                    bool retain)
{
  if (!isConnected() || !licenseValid())
    return -1;

  QMetaObject::invokeMethod(m_worker,
                            "publishCustomOnWorker",
                            Qt::QueuedConnection,
                            Q_ARG(QString, topic),
                            Q_ARG(QByteArray, payload),
                            Q_ARG(int, qos),
                            Q_ARG(bool, retain));

  return 1;
}

//--------------------------------------------------------------------------------------------------
// Worker callbacks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Mirrors the worker's broker state onto the main-thread atomic.
 */
void MQTT::Publisher::onWorkerBrokerStateChanged(int state)
{
  const bool connected = state == static_cast<int>(QMqttClient::Connected);
  m_isConnected.store(connected, std::memory_order_relaxed);
  Q_EMIT connectedChanged();

  if (connected)
    m_reportConnectionErrors = false;
}

/**
 * @brief Shows the test-connection probe result as a messagebox on the main thread.
 */
void MQTT::Publisher::onWorkerTestConnectionFinished(bool ok, const QString& detail)
{
  Misc::Utilities::showMessageBox(ok ? tr("MQTT broker reachable") : tr("MQTT broker unreachable"),
                                  detail,
                                  ok ? QMessageBox::Information : QMessageBox::Critical,
                                  tr("MQTT Test Connection"));
}

/**
 * @brief Displays the first error after a load attempt as a messagebox.
 */
void MQTT::Publisher::onWorkerBrokerError(const QString& message)
{
  if (m_reportConnectionErrors) {
    m_reportConnectionErrors = false;
    Misc::Utilities::showMessageBox(
      tr("MQTT broker connection failed"), message, QMessageBox::Critical, tr("MQTT Publisher"));
  }
}

/**
 * @brief Forwards a script compile/runtime error as a UI signal for the editor to surface.
 */
void MQTT::Publisher::onWorkerScriptError(const QString& message)
{
  Q_EMIT scriptError(message);
}

/**
 * @brief Emits statsChanged when the worker has published at least one new message.
 */
void MQTT::Publisher::emitStatsIfChanged()
{
  const auto current = m_messagesSent.load(std::memory_order_relaxed);
  if (current == m_messagesSentSeen)
    return;

  m_messagesSentSeen = current;
  Q_EMIT statsChanged();
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the current commercial license grants MQTT publishing.
 */
bool MQTT::Publisher::licenseValid() const
{
  const auto& token = Licensing::CommercialToken::current();
  return token.isValid() && SS_LICENSE_GUARD();
}

/**
 * @brief Emits configurationChanged and schedules a debounced worker sync.
 */
void MQTT::Publisher::markConfigChanged()
{
  if (m_inApply)
    return;

  scheduleSyncToWorker();
  Q_EMIT configurationChanged();
}

/**
 * @brief Restarts the debounce timer; final fire flushes one snapshot to the worker.
 */
void MQTT::Publisher::scheduleSyncToWorker()
{
  m_syncTimer.start();
}

/**
 * @brief Builds a BrokerConfig snapshot from the current main-thread state.
 */
MQTT::BrokerConfig MQTT::Publisher::snapshotConfig() const
{
  BrokerConfig cfg;
  cfg.enabled              = m_enabled;
  cfg.sslEnabled           = m_sslEnabled;
  cfg.cleanSession         = m_cleanSession;
  cfg.publishNotifications = m_publishNotifications;
  cfg.mode                 = m_mode;
  cfg.peerVerifyDepth      = m_peerVerifyDepth;
  cfg.port                 = m_port;
  cfg.keepAlive            = m_keepAlive;
  cfg.mqttVersion          = m_protocolVersion;
  cfg.sslProtocol          = m_sslProtocol;
  cfg.peerVerifyMode       = m_peerVerifyMode;
  cfg.clientId             = m_clientId;
  cfg.hostname             = m_hostname;
  cfg.username             = m_username;
  cfg.password             = m_password;
  cfg.topicBase            = m_topicBase;
  cfg.notificationTopic    = m_notificationTopic;
  cfg.scriptCode           = m_scriptCode;
  cfg.scriptTopic          = m_scriptTopic;
  cfg.scriptLanguage       = m_scriptLanguage;
  cfg.sparkplugEnabled     = m_sparkplugEnabled;
  cfg.sparkplugGroupId     = m_sparkplugGroupId;
  cfg.sparkplugEdgeNode    = m_sparkplugEdgeNodeId;
  cfg.sparkplugDeviceId    = m_sparkplugDeviceId;
  cfg.caCertificates       = m_caCertificates;
  cfg.clientCertificate    = m_tlsIdentity.certificate;
  cfg.clientPrivateKey     = m_tlsIdentity.privateKey;
  cfg.alpnProtocol         = m_alpnEnabled ? m_alpnProtocol.toUtf8() : QByteArray();
  return cfg;
}

/**
 * @brief Pushes a fresh BrokerConfig snapshot to the worker.
 */
void MQTT::Publisher::syncToWorker()
{
  if (!m_worker)
    return;

  QMetaObject::invokeMethod(m_worker,
                            "applyBrokerConfig",
                            Qt::QueuedConnection,
                            Q_ARG(MQTT::BrokerConfig, snapshotConfig()));
}

/**
 * @brief Updates the worker's drain timer interval to match the current publish frequency.
 */
void MQTT::Publisher::applyTimerInterval()
{
  const int interval = 1000 / std::clamp(m_publishFrequencyHz, kMinPublishHz, kMaxPublishHz);
  setTimerIntervalMs(interval);
}

/**
 * @brief Loads credentials for the current host:port from the vault into the in-memory fields.
 *        Callers re-parse the TLS identity themselves once their path state is final: parsing
 *        here would run against whatever paths happen to be current mid-restore.
 */
void MQTT::Publisher::reloadCredentialsFromVault()
{
  const auto creds = m_credentialVault.credentials(m_hostname, m_port);
  m_username       = creds.username;
  m_password       = creds.password;
  m_keyPassphrase  = m_credentialVault.keyPassphrase(m_hostname, m_port);
}

/**
 * @brief Writes the current in-memory credentials to the vault keyed by current host:port.
 */
void MQTT::Publisher::persistCredentialsToVault()
{
  if (m_hostname.isEmpty())
    return;

  m_credentialVault.setCredentials(m_hostname, m_port, m_username, m_password);
  m_credentialVault.setKeyPassphrase(m_hostname, m_port, m_keyPassphrase);
}

/**
 * @brief Re-parses the client certificate + key pair from the configured paths. A failed parse
 *        clears the identity (so a stale pair is never sent). Only the queued file-picker path
 *        passes interactive=true: a modal box from a plain setter would open a nested event
 *        loop inside the form model's itemChanged emission and re-enter the editing delegate.
 */
void MQTT::Publisher::reloadTlsIdentity(const bool interactive)
{
  const auto result =
    loadTlsIdentity(m_clientCertificatePath, m_privateKeyPath, m_keyPassphrase, m_tlsIdentity);
  if (result.ok())
    return;

  qCWarning(lcMqttPub) << "TLS identity rejected:" << tlsIdentityErrorString(result);
  if (interactive)
    Misc::Utilities::showMessageBox(tr("MQTT Client Certificate Error"),
                                    tlsIdentityErrorString(result),
                                    QMessageBox::Warning,
                                    tr("MQTT Publisher"));
}

#endif  // BUILD_COMMERCIAL
