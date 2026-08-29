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

#pragma once

#ifdef BUILD_COMMERCIAL

// clang-format off
#include <map>
#include <memory>
#include <vector>
#include <QtMqtt>
#include <QLoggingCategory>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>
// clang-format on

#  include "Async/TaskTree.h"
#  include "DataModel/DataBlock.h"
#  include "DataModel/ExportSchema.h"
#  include "DataModel/Frame.h"
#  include "DataModel/FrameConsumer.h"
#  include "IO/HAL_Driver.h"
#  include "MQTT/SparkplugPublisher.h"

Q_DECLARE_LOGGING_CATEGORY(lcMqttPub)

namespace MQTT {

/**
 * @brief Snapshot of every field needed by the worker thread to (re)configure the broker.
 */
struct BrokerConfig {
  bool enabled                              = false;
  bool sslEnabled                           = false;
  bool cleanSession                         = true;
  bool publishNotifications                 = false;
  bool sparkplugEnabled                     = false;
  int mode                                  = 0;
  int scriptLanguage                        = 0;
  int peerVerifyDepth                       = 10;
  quint16 port                              = 1883;
  quint16 keepAlive                         = 60;
  QMqttClient::ProtocolVersion mqttVersion  = QMqttClient::MQTT_5_0;
  QSsl::SslProtocol sslProtocol             = QSsl::SecureProtocols;
  QSslSocket::PeerVerifyMode peerVerifyMode = QSslSocket::AutoVerifyPeer;
  QString clientId;
  QString hostname;
  QString username;
  QString password;
  QString topicBase;
  QString notificationTopic;
  QString scriptCode;
  QString scriptTopic;
  QString sparkplugGroupId;
  QString sparkplugEdgeNode;
  QString sparkplugDeviceId;
  QByteArray alpnProtocol;
  QSslKey clientPrivateKey;
  QSslCertificate clientCertificate;
  QList<QSslCertificate> caCertificates;
};

/**
 * @brief Raw RX-bytes payload paired with the device id and capture timestamp.
 */
struct TimestampedRawBytes {
  int deviceId;
  IO::CapturedDataPtr data;
};

class Publisher;
class PublisherScript;

/**
 * @brief Background worker that owns the QMqttClient and performs broker I/O off-main. Every
 *        member here lives on the worker thread: the facade reaches it only through queued slot
 *        invocations and the lock-free queues whose addresses it is constructed with.
 */
class PublisherWorker : public DataModel::FrameConsumerWorker<DataModel::DataBlockPtr> {
  Q_OBJECT

signals:
  void brokerStateChanged(int state);
  void brokerErrorOccurred(const QString& message);
  void scriptErrorOccurred(const QString& message);
  void testConnectionFinished(bool ok, const QString& detail);

public:
  PublisherWorker(moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>* frameQueue,
                  std::atomic<bool>* enabled,
                  std::atomic<size_t>* queueSize,
                  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* rawQueue,
                  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* frameQueueBytes,
                  std::atomic<int>* mode,
                  std::atomic<int>* scriptLanguage,
                  std::atomic<quint64>* messagesSent,
                  std::atomic<quint64>* bytesSent);
  ~PublisherWorker() override;
  PublisherWorker(PublisherWorker&&)                 = delete;
  PublisherWorker(const PublisherWorker&)            = delete;
  PublisherWorker& operator=(PublisherWorker&&)      = delete;
  PublisherWorker& operator=(const PublisherWorker&) = delete;

public:
  void processData() override;
  void closeResources() override;
  [[nodiscard]] bool isResourceOpen() const override;
  [[nodiscard]] QString errorString(QMqttClient::ClientError error) const;

public slots:
  void bootstrap();
  void openBroker();
  void closeBroker();
  void runTestConnection();
  void setStructureGeneration(quint64 generation);
  void applyBrokerConfig(const MQTT::BrokerConfig& cfg);
  void setTemplateFrame(int sourceId, const DataModel::Frame& frame);
  void publishNotificationOnWorker(const QString& topic, const QByteArray& payload);
  void publishCustomOnWorker(const QString& topic, const QByteArray& payload, int qos, bool retain);

private slots:
  void onClientStateChanged(QMqttClient::ClientState state);
  void onClientErrorChanged(QMqttClient::ClientError error);
  void onSparkplugCommand(const QMqttMessage& message);

protected:
  void processItems(const std::vector<DataModel::DataBlockPtr>& items) override;

private:
  [[nodiscard]] Async::Task* buildReconnectFlow();
  [[nodiscard]] bool sparkplugActive() const;
  bool publishAndCount(const QMqttTopicName& topic, const QByteArray& payload);
  [[nodiscard]] static QString describeMqttError(QMqttClient::ClientError error);

  void publishBatchAsJson(const std::vector<DataModel::Frame>& items);
  void publishBatchAsCsv(const std::vector<DataModel::Frame>& items);
  void rebuildCsvSchema(const DataModel::Frame& frame);
  void recompileScriptIfNeeded();
  void applyClientPropertiesUnsafe();
  void configureSparkplugWill();
  void publishSparkplugBirth();
  void subscribeSparkplugCommands();
  void discardSuppressedPayloads();
  void registerSparkplugMetrics(const DataModel::Frame& frame);
  void publishSparkplugBlocks(const std::vector<DataModel::DataBlockPtr>& blocks);

private:
  // One publish carries at most this many samples; a dense block alone can exceed it
  static constexpr std::size_t kMaxExpandedSamples = 4096;

  BrokerConfig m_cfg;
  QMqttClient* m_client;
  QSslConfiguration m_sslConfiguration;
  QByteArray m_rawBatchBuffer;
  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* m_rawQueue;
  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* m_frameQueueBytes;
  std::atomic<int>* m_mode;
  std::atomic<int>* m_scriptLanguage;
  std::atomic<quint64>* m_messagesSent;
  std::atomic<quint64>* m_bytesSent;

  PublisherScript* m_script;
  QString m_compiledScriptCode;
  std::unique_ptr<Async::TaskRunner> m_runner;

  QString m_csvFrameTitle;
  QByteArray m_csvHeaderPayload;
  QByteArray m_csvRowBuffer;
  bool m_csvHeaderDirty;
  QMap<int, QString> m_csvLastFinal;
  DataModel::ExportSchema m_csvSchema;

  std::vector<DataModel::Frame> m_expanded;
  std::map<int, DataModel::FrameTemplate> m_templates;

  quint64 m_pendingStructureGeneration;
  SparkplugPublisher m_sparkplug;
};

}  // namespace MQTT

Q_DECLARE_METATYPE(MQTT::BrokerConfig)

#endif  // BUILD_COMMERCIAL
