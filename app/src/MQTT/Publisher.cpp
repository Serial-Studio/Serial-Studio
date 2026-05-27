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
#  include "DataModel/NotificationCenter.h"
#  include "DataModel/ProjectModel.h"
#  include "Licensing/CommercialToken.h"
#  include "Misc/Utilities.h"
#  include "MQTT/PublisherScript.h"

Q_LOGGING_CATEGORY(lcMqttPub, "serialstudio.mqtt.publisher", QtCriticalMsg)

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
  moodycamel::ReaderWriterQueue<DataModel::TimestampedFramePtr>* frameQueue,
  std::atomic<bool>* enabled,
  std::atomic<size_t>* queueSize,
  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* rawQueue,
  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* frameQueueBytes,
  std::atomic<int>* mode,
  std::atomic<int>* scriptLanguage,
  std::atomic<quint64>* messagesSent,
  std::atomic<quint64>* bytesSent)
  : DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr>(frameQueue, enabled, queueSize)
  , m_client(nullptr)
  , m_rawQueue(rawQueue)
  , m_frameQueueBytes(frameQueueBytes)
  , m_mode(mode)
  , m_scriptLanguage(scriptLanguage)
  , m_messagesSent(messagesSent)
  , m_bytesSent(bytesSent)
  , m_script(nullptr)
  , m_reconnectPending(false)
  , m_csvHeaderDirty(true)
{
  m_sslConfiguration.setProtocol(QSsl::SecureProtocols);
  m_sslConfiguration.setPeerVerifyMode(QSslSocket::AutoVerifyPeer);
  m_sslConfiguration.setPeerVerifyDepth(10);

  m_rawBatchBuffer.reserve(64 * 1024);
  m_csvRowBuffer.reserve(8 * 1024);
}

/**
 * @brief Destructor closes the broker session and tears down the script engine.
 */
MQTT::PublisherWorker::~PublisherWorker()
{
  // Disarm any pending reconnect lambda so it cannot run during teardown.
  if (m_reconnectConn) {
    QObject::disconnect(m_reconnectConn);
    m_reconnectConn = {};
  }
  m_reconnectPending = false;

  if (m_client && m_client->state() != QMqttClient::Disconnected)
    m_client->disconnectFromHost();

  delete m_script;
  m_script = nullptr;
}

/**
 * @brief Worker-thread bootstrap: creates the QMqttClient and the script engine on this thread.
 */
void MQTT::PublisherWorker::bootstrap()
{
  if (m_client)
    return;

  m_client = new QMqttClient(this);
  connect(m_client, &QMqttClient::stateChanged, this, &PublisherWorker::onClientStateChanged);
  connect(m_client, &QMqttClient::errorChanged, this, &PublisherWorker::onClientErrorChanged);

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
 * @brief Disconnects from the broker.
 */
void MQTT::PublisherWorker::closeResources()
{
  if (m_client && m_client->state() != QMqttClient::Disconnected)
    m_client->disconnectFromHost();
}

/**
 * @brief Drains both the frame queue and the raw-bytes queue.
 */
void MQTT::PublisherWorker::processData()
{
  // Frame queue first via the base class; routes through processItems below
  DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr>::processData();

  if (!consumerEnabled())
    return;

  if (!isResourceOpen())
    return;

  const int mode = m_mode->load(std::memory_order_relaxed);

  // Drain queues that are not in use for the current mode so they don't unbound-grow.
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

    // Concatenate every queued chunk into one publish per tick
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

    // Concatenate every payload returned by mqtt(frame) into one bulk publish per tick
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
void MQTT::PublisherWorker::processItems(const std::vector<DataModel::TimestampedFramePtr>& items)
{
  if (items.empty())
    return;

  if (!isResourceOpen() || m_cfg.topicBase.isEmpty())
    return;

  const int mode = m_mode->load(std::memory_order_relaxed);

  switch (static_cast<Publisher::Mode>(mode)) {
    case Publisher::Mode::DashboardDataJson:
      publishBatchAsJson(items);
      break;

    case Publisher::Mode::DashboardDataCsv:
      publishBatchAsCsv(items);
      break;

    // Script mode receives raw frame bytes, not parsed frames (handled in processData)
    case Publisher::Mode::ScriptDriven:
    case Publisher::Mode::RawRxData:
      break;
  }
}

/**
 * @brief Publishes the batch as one compact JSON document where each dataset's "value" and
 *        "numericValue" become arrays collecting every frame's sample in arrival order.
 */
void MQTT::PublisherWorker::publishBatchAsJson(
  const std::vector<DataModel::TimestampedFramePtr>& items)
{
  QMqttTopicName topic(m_cfg.topicBase);
  if (!topic.isValid())
    return;

  // Pick the latest frame as the structural template (titles, widget metadata, etc.)
  const auto& latest = items.back();
  if (!latest)
    return;

  QJsonObject root = DataModel::serialize(latest->data);

  // Collect per-dataset arrays of string + numeric samples across the batch in arrival order
  QMap<int, QJsonArray> valuesByDataset;
  QMap<int, QJsonArray> numericByDataset;
  for (const auto& item : items) {
    if (!item)
      continue;

    for (const auto& g : item->data.groups) {
      for (const auto& d : g.datasets) {
        valuesByDataset[d.uniqueId].append(d.value.simplified());
        numericByDataset[d.uniqueId].append(d.numericValue);
      }
    }
  }

  // Walk live groups in lockstep with the serialized JSON, swapping per-dataset value/numericValue
  QJsonArray groupArray = root.value(Keys::Groups).toArray();
  for (int gi = 0; gi < groupArray.size() && gi < static_cast<int>(latest->data.groups.size());
       ++gi) {
    const auto& liveGroup  = latest->data.groups[gi];
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
void MQTT::PublisherWorker::publishBatchAsCsv(
  const std::vector<DataModel::TimestampedFramePtr>& items)
{
  QMqttTopicName rowsTopic(m_cfg.topicBase);
  if (!rowsTopic.isValid())
    return;

  const auto& latest = items.back();
  if (!latest)
    return;

  // Detect schema changes via frame-title pivot; ExportSchema is content-addressed by datasets
  if (m_csvHeaderDirty || latest->data.title != m_csvFrameTitle)
    rebuildCsvSchema(latest->data);

  if (m_csvHeaderPayload.isEmpty())
    return;

  // Publish header as a retained message so late subscribers always get the schema
  if (m_csvHeaderDirty) {
    QMqttTopicName headerTopic(m_cfg.topicBase + QStringLiteral("/header"));
    if (headerTopic.isValid())
      m_client->publish(headerTopic, m_csvHeaderPayload, 0, true);

    m_csvHeaderDirty = false;
  }

  m_csvRowBuffer.resize(0);

  for (const auto& item : items) {
    if (!item)
      continue;

    // Refresh forward-fill cache for every dataset in this frame
    for (const auto& g : item->data.groups)
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
 * @brief Publishes a payload and increments the stats counters.
 */
void MQTT::PublisherWorker::publishAndCount(const QMqttTopicName& topic, const QByteArray& payload)
{
  const auto id = m_client->publish(topic, payload);
  qCDebug(lcMqttPub) << "publish topic=" << topic.name() << "size=" << payload.size() << "id=" << id
                     << "preview=" << payload.left(80);

  if (id < 0) {
    qCWarning(lcMqttPub) << "publish returned -1 for topic" << topic.name();
    return;
  }

  if (m_messagesSent)
    m_messagesSent->fetch_add(1, std::memory_order_relaxed);

  if (m_bytesSent)
    m_bytesSent->fetch_add(static_cast<quint64>(payload.size()), std::memory_order_relaxed);
}

/**
 * @brief Applies a fresh broker config snapshot. Reconnects when broker-affecting fields change.
 */
void MQTT::PublisherWorker::applyBrokerConfig(const MQTT::BrokerConfig& cfg)
{
  // Detect whether broker-identifying fields changed and warrant a reconnect cycle
  const bool brokerChanged =
    cfg.hostname != m_cfg.hostname || cfg.port != m_cfg.port || cfg.username != m_cfg.username
    || cfg.password != m_cfg.password || cfg.clientId != m_cfg.clientId
    || cfg.mqttVersion != m_cfg.mqttVersion || cfg.keepAlive != m_cfg.keepAlive
    || cfg.cleanSession != m_cfg.cleanSession || cfg.sslEnabled != m_cfg.sslEnabled
    || cfg.sslProtocol != m_cfg.sslProtocol || cfg.peerVerifyMode != m_cfg.peerVerifyMode
    || cfg.peerVerifyDepth != m_cfg.peerVerifyDepth || cfg.caCertificates != m_cfg.caCertificates
    || cfg.enabled != m_cfg.enabled;

  // Force CSV header rebuild so dataset add/remove takes effect on next publish
  m_csvHeaderDirty = true;
  m_csvFrameTitle.clear();

  // Drop the compiled script when the source changes; recompile is lazy on next publish
  if (cfg.scriptCode != m_cfg.scriptCode && m_script)
    m_script->reset();

  m_cfg = cfg;

  if (!m_client)
    return;

  // SSL bits take effect only on the next CONNECT
  m_sslConfiguration.setProtocol(m_cfg.sslProtocol);
  m_sslConfiguration.setPeerVerifyMode(m_cfg.peerVerifyMode);
  m_sslConfiguration.setPeerVerifyDepth(m_cfg.peerVerifyDepth);
  if (!m_cfg.caCertificates.isEmpty()) {
    auto existing = m_sslConfiguration.caCertificates();
    for (const auto& cert : m_cfg.caCertificates)
      if (!existing.contains(cert))
        existing.append(cert);

    m_sslConfiguration.setCaCertificates(existing);
  }

  // QMqttClient broker setters corrupt the state machine unless applied while Disconnected
  if (m_client->state() == QMqttClient::Disconnected) {
    applyClientPropertiesUnsafe();

    if (brokerChanged && m_cfg.enabled)
      openBroker();

    return;
  }

  // Non-broker edits take effect on the next publish without dropping the live session
  if (!brokerChanged)
    return;

  // Pending teardown will pick up the fresh m_cfg; nudge another disconnect just in case
  if (m_reconnectPending) {
    if (m_client->state() != QMqttClient::Disconnected)
      m_client->disconnectFromHost();

    return;
  }

  m_reconnectPending = true;

  // Defer the property mutation off the stateChanged emission via a queued slot call
  m_reconnectConn =
    connect(m_client, &QMqttClient::stateChanged, this, [this](QMqttClient::ClientState s) {
      if (s != QMqttClient::Disconnected)
        return;

      // Drop this connection first so a later state change can't re-fire this lambda
      if (m_reconnectConn) {
        QObject::disconnect(m_reconnectConn);
        m_reconnectConn = {};
      }

      QMetaObject::invokeMethod(
        this, &PublisherWorker::finishPendingReconnect, Qt::QueuedConnection);
    });

  m_client->disconnectFromHost();
}

/**
 * @brief Pushes the staged mirror to the client and reopens if still enabled.
 */
void MQTT::PublisherWorker::finishPendingReconnect()
{
  m_reconnectPending = false;

  if (!m_client)
    return;

  // Another applyBrokerConfig may have reopened the client between lambda and slot
  if (m_client->state() != QMqttClient::Disconnected)
    return;

  applyClientPropertiesUnsafe();

  if (m_cfg.enabled)
    openBroker();
}

/**
 * @brief Writes the staged broker properties into the client. Caller must guarantee
 *        state == Disconnected.
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
 * @brief Closes the broker connection.
 */
void MQTT::PublisherWorker::closeBroker()
{
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

    // Marshal the user-visible message back to the main thread via signal
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
 * @brief Forwards the client state to the main thread.
 */
void MQTT::PublisherWorker::onClientStateChanged(QMqttClient::ClientState state)
{
  Q_EMIT brokerStateChanged(static_cast<int>(state));
}

/**
 * @brief Forwards a broker error to the main thread as a human-readable string.
 */
void MQTT::PublisherWorker::onClientErrorChanged(QMqttClient::ClientError error)
{
  if (error == QMqttClient::NoError)
    return;

  Q_EMIT brokerErrorOccurred(describeMqttError(error));
}

//==================================================================================================
// Publisher (main thread)
//==================================================================================================

/**
 * @brief Constructs the publisher with safe broker defaults and starts the worker thread.
 */
MQTT::Publisher::Publisher()
  : DataModel::FrameConsumer<DataModel::TimestampedFramePtr>(
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

  // Debounce timer coalesces rapid setter bursts into one snapshot push
  m_syncTimer.setSingleShot(true);
  m_syncTimer.setInterval(kSyncDebounceMs);
  connect(&m_syncTimer, &QTimer::timeout, this, &Publisher::syncToWorker);

  // Stats poll: cheap atomic read, emit only when the counter advanced
  m_statsTimer.setInterval(kStatsTickMs);
  connect(&m_statsTimer, &QTimer::timeout, this, &Publisher::emitStatsIfChanged);
  m_statsTimer.start();

  regenerateClientId();

  initializeWorker();
  applyTimerInterval();

  // Wire the worker's cross-thread signals (AutoConnection becomes QueuedConnection)
  auto* w = static_cast<PublisherWorker*>(m_worker);
  connect(w, &PublisherWorker::brokerStateChanged, this, &Publisher::onWorkerBrokerStateChanged);
  connect(w, &PublisherWorker::brokerErrorOccurred, this, &Publisher::onWorkerBrokerError);
  connect(w, &PublisherWorker::scriptErrorOccurred, this, &Publisher::onWorkerScriptError);
  connect(
    w, &PublisherWorker::testConnectionFinished, this, &Publisher::onWorkerTestConnectionFinished);

  // Construct QMqttClient on the worker thread so its socket/notifier get the right affinity
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

  // Persist the typed id only when the user opted in; auto ids are regenerated per load
  if (m_customClientId)
    obj.insert(kKeyClientId, m_clientId);

  obj.insert(kKeyCleanSession, m_cleanSession);
  obj.insert(kKeyKeepAlive, static_cast<int>(m_keepAlive));
  obj.insert(kKeyMqttVersion, static_cast<int>(mqttVersion()));
  obj.insert(kKeySslEnabled, m_sslEnabled);
  obj.insert(kKeySslProtocol, static_cast<int>(sslProtocol()));
  obj.insert(kKeyPeerVerifyMode, static_cast<int>(peerVerifyMode()));
  obj.insert(kKeyPeerVerifyDepth, m_peerVerifyDepth);
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

  // Stored clientId is consulted only when customClientId is set
  const bool customCid = cfg.value(kKeyCustomClientId).toBool(false);
  setCustomClientId(customCid);

  const auto cid = cfg.value(kKeyClientId).toString();
  if (customCid && !cid.isEmpty())
    setClientId(cid);
  else
    regenerateClientId();

  // Credentials are stored encrypted per host:port in QSettings, not in the project file.
  reloadCredentialsFromVault();

  setCleanSession(cfg.value(kKeyCleanSession).toBool(true));
  setKeepAlive(static_cast<quint16>(cfg.value(kKeyKeepAlive).toInt(60)));
  setMqttVersion(static_cast<quint8>(cfg.value(kKeyMqttVersion).toInt(2)));

  setSslEnabled(cfg.value(kKeySslEnabled).toBool(false));
  setSslProtocol(static_cast<quint8>(cfg.value(kKeySslProtocol).toInt(5)));
  setPeerVerifyMode(static_cast<quint8>(cfg.value(kKeyPeerVerifyMode).toInt(3)));
  setPeerVerifyDepth(cfg.value(kKeyPeerVerifyDepth).toInt(10));

  m_inApply = false;

  // Normalize the project's stored config so missing fields are filled with defaults.
  m_savingToProjectModel = true;
  DataModel::ProjectModel::instance().setMqttPublisher(toJson());
  m_savingToProjectModel = false;

  m_skipNextSync = true;
  Q_EMIT configurationChanged();
  m_skipNextSync = false;

  // Project load: arm one-shot error reporting, then push the snapshot to the worker now.
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

  // Flush any pending debounced sync so the worker sees the freshest config
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

  // Deferred via queued invoke so QFileDialog::done() unwinds before the slot runs.
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
    // Skip the round-trip echo from interactive edits to avoid close+reopen per keystroke
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

  if (!m_inApply)
    reloadCredentialsFromVault();

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

  if (!m_inApply)
    reloadCredentialsFromVault();

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
 * @brief Enqueues the frame for the worker; rate-limiting and broker I/O happen off-main.
 */
void MQTT::Publisher::hotpathTxFrame(const DataModel::TimestampedFramePtr& frame)
{
  if (!m_enabled || !licenseValid()) [[likely]]
    return;

  // Only dashboard modes consume parsed frames; raw and script modes use byte queues
  if (m_mode != static_cast<int>(Mode::DashboardDataJson)
      && m_mode != static_cast<int>(Mode::DashboardDataCsv))
    return;

  if (!frame || m_topicBase.isEmpty())
    return;

  enqueueData(frame);
}

/**
 * @brief Enqueues raw driver bytes for the worker (RawRxData mode only).
 */
void MQTT::Publisher::hotpathTxRawBytes(int deviceId, const IO::CapturedDataPtr& data)
{
  if (!m_enabled || !licenseValid()) [[likely]]
    return;

  if (m_mode != static_cast<int>(Mode::RawRxData))
    return;

  if (!data || data->data.isEmpty() || m_topicBase.isEmpty())
    return;

  TimestampedRawBytes item{deviceId, data};
  m_rawBytesQueue.try_enqueue(std::move(item));
}

/**
 * @brief Enqueues a delimited frame's raw bytes for the script mode worker.
 */
void MQTT::Publisher::hotpathTxRawFrame(int deviceId, const IO::CapturedDataPtr& data)
{
  if (!m_enabled || !licenseValid()) [[likely]]
    return;

  if (m_mode != static_cast<int>(Mode::ScriptDriven))
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

  // Dispatch succeeded; the actual broker message ID is no longer observable from main
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
  return token.isValid() && SS_LICENSE_GUARD()
      && token.featureTier() >= Licensing::FeatureTier::Hobbyist;
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
  cfg.caCertificates       = m_caCertificates;
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
 */
void MQTT::Publisher::reloadCredentialsFromVault()
{
  const auto creds = m_credentialVault.credentials(m_hostname, m_port);
  m_username       = creds.username;
  m_password       = creds.password;
}

/**
 * @brief Writes the current in-memory credentials to the vault keyed by current host:port.
 */
void MQTT::Publisher::persistCredentialsToVault()
{
  if (m_hostname.isEmpty())
    return;

  m_credentialVault.setCredentials(m_hostname, m_port, m_username, m_password);
}

#endif  // BUILD_COMMERCIAL
