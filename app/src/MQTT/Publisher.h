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

#ifdef BUILD_COMMERCIAL

// clang-format off
#include <QtMqtt>
#include <QJsonObject>
#include <QObject>
#include <QTimer>
#include <QVariantMap>
// clang-format on

#  include "DataModel/DataBlock.h"
#  include "DataModel/FrameConsumer.h"
#  include "IO/HAL_Driver.h"
#  include "MQTT/BrokerOptions.h"
#  include "MQTT/CredentialVault.h"
#  include "MQTT/PublisherWorker.h"
#  include "MQTT/TlsConfig.h"

namespace MQTT {

/**
 * @brief Per-project MQTT publisher; broadcasts frames, raw bytes and notifications. The facade
 *        owns the main-thread configuration and the lock-free queues the consumer path fills;
 *        every broker interaction belongs to PublisherWorker on its own thread.
 */
class Publisher : public DataModel::FrameConsumer<DataModel::DataBlockPtr> {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool enabled
             READ enabled
             WRITE setEnabled
             NOTIFY configurationChanged)
  Q_PROPERTY(bool sslEnabled
             READ sslEnabled
             WRITE setSslEnabled
             NOTIFY configurationChanged)
  Q_PROPERTY(bool isConnected
             READ isConnected
             NOTIFY connectedChanged)
  Q_PROPERTY(bool cleanSession
             READ cleanSession
             WRITE setCleanSession
             NOTIFY configurationChanged)
  Q_PROPERTY(bool publishNotifications
             READ publishNotifications
             WRITE setPublishNotifications
             NOTIFY configurationChanged)
  Q_PROPERTY(int mode
             READ mode
             WRITE setMode
             NOTIFY configurationChanged)
  Q_PROPERTY(int peerVerifyDepth
             READ peerVerifyDepth
             WRITE setPeerVerifyDepth
             NOTIFY configurationChanged)
  Q_PROPERTY(int publishFrequency
             READ publishFrequency
             WRITE setPublishFrequency
             NOTIFY configurationChanged)
  Q_PROPERTY(quint8 mqttVersion
             READ mqttVersion
             WRITE setMqttVersion
             NOTIFY configurationChanged)
  Q_PROPERTY(quint8 sslProtocol
             READ sslProtocol
             WRITE setSslProtocol
             NOTIFY configurationChanged)
  Q_PROPERTY(quint8 peerVerifyMode
             READ peerVerifyMode
             WRITE setPeerVerifyMode
             NOTIFY configurationChanged)
  Q_PROPERTY(quint16 port
             READ port
             WRITE setPort
             NOTIFY configurationChanged)
  Q_PROPERTY(quint16 keepAlive
             READ keepAlive
             WRITE setKeepAlive
             NOTIFY configurationChanged)
  Q_PROPERTY(QString clientId
             READ clientId
             WRITE setClientId
             NOTIFY configurationChanged)
  Q_PROPERTY(bool customClientId
             READ customClientId
             WRITE setCustomClientId
             NOTIFY configurationChanged)
  Q_PROPERTY(QString hostname
             READ hostname
             WRITE setHostname
             NOTIFY configurationChanged)
  Q_PROPERTY(QString username
             READ username
             WRITE setUsername
             NOTIFY configurationChanged)
  Q_PROPERTY(QString password
             READ password
             WRITE setPassword
             NOTIFY configurationChanged)
  Q_PROPERTY(QString clientCertificatePath
             READ clientCertificatePath
             WRITE setClientCertificatePath
             NOTIFY configurationChanged)
  Q_PROPERTY(QString privateKeyPath
             READ privateKeyPath
             WRITE setPrivateKeyPath
             NOTIFY configurationChanged)
  Q_PROPERTY(QString keyPassphrase
             READ keyPassphrase
             WRITE setKeyPassphrase
             NOTIFY configurationChanged)
  Q_PROPERTY(bool alpnEnabled
             READ alpnEnabled
             WRITE setAlpnEnabled
             NOTIFY configurationChanged)
  Q_PROPERTY(QString alpnProtocol
             READ alpnProtocol
             WRITE setAlpnProtocol
             NOTIFY configurationChanged)
  Q_PROPERTY(QString topicBase
             READ topicBase
             WRITE setTopicBase
             NOTIFY configurationChanged)
  Q_PROPERTY(QString notificationTopic
             READ notificationTopic
             WRITE setNotificationTopic
             NOTIFY configurationChanged)
  Q_PROPERTY(QString scriptCode
             READ scriptCode
             WRITE setScriptCode
             NOTIFY configurationChanged)
  Q_PROPERTY(QString scriptTopic
             READ scriptTopic
             WRITE setScriptTopic
             NOTIFY configurationChanged)
  Q_PROPERTY(int scriptLanguage
             READ scriptLanguage
             WRITE setScriptLanguage
             NOTIFY configurationChanged)
  Q_PROPERTY(bool sparkplugEnabled
             READ sparkplugEnabled
             WRITE setSparkplugEnabled
             NOTIFY configurationChanged)
  Q_PROPERTY(QString sparkplugGroupId
             READ sparkplugGroupId
             WRITE setSparkplugGroupId
             NOTIFY configurationChanged)
  Q_PROPERTY(QString sparkplugEdgeNodeId
             READ sparkplugEdgeNodeId
             WRITE setSparkplugEdgeNodeId
             NOTIFY configurationChanged)
  Q_PROPERTY(QString sparkplugDeviceId
             READ sparkplugDeviceId
             WRITE setSparkplugDeviceId
             NOTIFY configurationChanged)
  Q_PROPERTY(QString modeLabel
             READ modeLabel
             NOTIFY configurationChanged)
  Q_PROPERTY(QString brokerEndpoint
             READ brokerEndpoint
             NOTIFY configurationChanged)
  Q_PROPERTY(quint64 messagesSent
             READ messagesSent
             NOTIFY statsChanged)
  Q_PROPERTY(QStringList modes
             READ modes
             CONSTANT)
  Q_PROPERTY(QStringList mqttVersions
             READ mqttVersions
             CONSTANT)
  Q_PROPERTY(QStringList sslProtocols
             READ sslProtocols
             CONSTANT)
  Q_PROPERTY(QStringList peerVerifyModes
             READ peerVerifyModes
             CONSTANT)
  // clang-format on

public:
  /**
   * @brief Publisher output modes.
   */
  enum Mode {
    RawRxData         = 0,
    ScriptDriven      = 1,
    DashboardDataCsv  = 2,
    DashboardDataJson = 3,
  };
  Q_ENUM(Mode)

  static constexpr int kMinPublishHz     = 1;
  static constexpr int kMaxPublishHz     = 30;
  static constexpr int kDefaultPublishHz = 10;

signals:
  void connectedChanged();
  void configurationChanged();
  void statsChanged();
  void scriptError(const QString& message);

private:
  explicit Publisher();
  ~Publisher() override;
  Publisher(Publisher&&)                 = delete;
  Publisher(const Publisher&)            = delete;
  Publisher& operator=(Publisher&&)      = delete;
  Publisher& operator=(const Publisher&) = delete;

public:
  [[nodiscard]] static Publisher& instance();

  [[nodiscard]] bool enabled() const noexcept;
  [[nodiscard]] bool sslEnabled() const noexcept;
  [[nodiscard]] bool isConnected() const;
  [[nodiscard]] bool cleanSession() const noexcept;
  [[nodiscard]] bool publishNotifications() const noexcept;

  [[nodiscard]] int mode() const noexcept;
  [[nodiscard]] int peerVerifyDepth() const noexcept;
  [[nodiscard]] int publishFrequency() const noexcept;

  [[nodiscard]] quint8 mqttVersion() const noexcept;
  [[nodiscard]] quint8 sslProtocol() const noexcept;
  [[nodiscard]] quint8 peerVerifyMode() const noexcept;
  [[nodiscard]] quint16 port() const noexcept;
  [[nodiscard]] quint16 keepAlive() const noexcept;

  [[nodiscard]] QString clientId() const;
  [[nodiscard]] bool customClientId() const noexcept;
  [[nodiscard]] QString hostname() const;
  [[nodiscard]] QString username() const;
  [[nodiscard]] QString password() const;
  [[nodiscard]] QString clientCertificatePath() const;
  [[nodiscard]] QString privateKeyPath() const;
  [[nodiscard]] QString keyPassphrase() const;
  [[nodiscard]] bool alpnEnabled() const noexcept;
  [[nodiscard]] QString alpnProtocol() const;
  [[nodiscard]] QString topicBase() const;
  [[nodiscard]] QString notificationTopic() const;
  [[nodiscard]] QString scriptCode() const;
  [[nodiscard]] QString scriptTopic() const;
  [[nodiscard]] int scriptLanguage() const noexcept;
  [[nodiscard]] bool sparkplugEnabled() const noexcept;
  [[nodiscard]] QString sparkplugGroupId() const;
  [[nodiscard]] QString sparkplugDeviceId() const;
  [[nodiscard]] QString sparkplugEdgeNodeId() const;
  [[nodiscard]] QString modeLabel() const;
  [[nodiscard]] QString brokerEndpoint() const;
  [[nodiscard]] quint64 messagesSent() const noexcept;

  Q_INVOKABLE [[nodiscard]] static QString defaultScriptTemplate();

  [[nodiscard]] const QStringList& modes() const;
  [[nodiscard]] const QStringList& mqttVersions() const;
  [[nodiscard]] const QStringList& sslProtocols() const;
  [[nodiscard]] const QStringList& peerVerifyModes() const;

  [[nodiscard]] QJsonObject toJson() const;

public slots:
  void setupExternalConnections();
  void applyProjectConfig(const QJsonObject& cfg);
  void resetProjectConfig();
  void testConnection();
  void addCaCertificates();
  void regenerateClientId();

  void setEnabled(const bool enabled);
  void setMode(const int mode);
  void setSslEnabled(const bool enabled);
  void setCleanSession(const bool cleanSession);
  void setPublishNotifications(const bool publish);
  void setPeerVerifyDepth(const int depth);
  void setPublishFrequency(const int hz);
  void setMqttVersion(const quint8 version);
  void setSslProtocol(const quint8 protocol);
  void setPeerVerifyMode(const quint8 verifyMode);
  void setPort(const quint16 port);
  void setKeepAlive(const quint16 keepAlive);
  void setClientId(const QString& id);
  void setCustomClientId(const bool custom);
  void setHostname(const QString& hostname);
  void setUsername(const QString& username);
  void setPassword(const QString& password);
  void setClientCertificatePath(const QString& path);
  void setPrivateKeyPath(const QString& path);
  void setKeyPassphrase(const QString& passphrase);
  void setAlpnEnabled(const bool enabled);
  void setAlpnProtocol(const QString& protocol);
  void selectClientCertificate();
  void selectPrivateKey();
  void setTopicBase(const QString& topic);
  void setNotificationTopic(const QString& topic);
  void setScriptCode(const QString& code);
  void setScriptTopic(const QString& topic);
  void setScriptLanguage(const int language);
  void setSparkplugEnabled(const bool enabled);
  void setSparkplugGroupId(const QString& groupId);
  void setSparkplugDeviceId(const QString& deviceId);
  void setSparkplugEdgeNodeId(const QString& edgeNodeId);
  void hotpathTxRawFrame(int deviceId, const IO::CapturedDataPtr& data);

  void ingestBlock(const DataModel::DataBlockPtr& block);
  void hotpathTxRawBytes(int deviceId, const IO::CapturedDataPtr& data);
  void onNotificationPosted(const QVariantMap& event);

  qint64 mqttPublish(const QString& topic,
                     const QByteArray& payload,
                     int qos     = 0,
                     bool retain = false);

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;

private slots:
  void onWorkerBrokerStateChanged(int state);
  void onWorkerBrokerError(const QString& message);
  void onWorkerScriptError(const QString& message);
  void onWorkerTestConnectionFinished(bool ok, const QString& detail);
  void emitStatsIfChanged();

private:
  [[nodiscard]] bool licenseValid() const;
  void registerBrokerOptions();
  void markConfigChanged();
  void scheduleSyncToWorker();
  void syncToWorker();
  void applyTimerInterval();
  void reloadCredentialsFromVault();
  void persistCredentialsToVault();
  void reloadTlsIdentity(const bool interactive);
  void selectPemFile(const QString& title, void (Publisher::*setter)(const QString&));
  [[nodiscard]] BrokerConfig snapshotConfig() const;

private:
  bool m_enabled;
  bool m_publishNotifications;
  bool m_cleanSession;
  bool m_inApply;
  bool m_skipNextSync;
  bool m_savingToProjectModel;
  bool m_reportConnectionErrors;
  bool m_customClientId;
  bool m_sparkplugEnabled;
  int m_mode;
  int m_scriptLanguage;
  int m_publishFrequencyHz;

  QMqttClient::ProtocolVersion m_protocolVersion;
  quint16 m_port;
  quint16 m_keepAlive;

  QString m_clientId;
  QString m_autoClientId;
  QString m_hostname;
  QString m_username;
  QString m_password;
  QString m_keyPassphrase;
  QString m_topicBase;
  QString m_notificationTopic;
  QString m_scriptCode;
  QString m_scriptTopic;
  QString m_sparkplugGroupId;
  QString m_sparkplugEdgeNodeId;
  QString m_sparkplugDeviceId;

  TlsConfig m_tls;
  BrokerOptions m_options;
  CredentialVault m_credentialVault;

  QTimer m_syncTimer;
  QTimer m_statsTimer;
  moodycamel::ReaderWriterQueue<TimestampedRawBytes> m_rawBytesQueue;
  moodycamel::ReaderWriterQueue<TimestampedRawBytes> m_rawFramesQueue;
  alignas(64) std::atomic<int> m_workerMode;
  alignas(64) std::atomic<int> m_workerScriptLanguage;
  alignas(64) std::atomic<bool> m_isConnected;
  alignas(64) std::atomic<quint64> m_messagesSent;
  alignas(64) std::atomic<quint64> m_bytesSent;
  quint64 m_messagesSentSeen;

  static constexpr int kSyncDebounceMs = 200;
  static constexpr int kStatsTickMs    = 500;

  static constexpr QLatin1StringView kKeyEnabled{"enabled"};
  static constexpr QLatin1StringView kKeyMode{"mode"};
  static constexpr QLatin1StringView kKeyPublishNotifications{"publishNotifications"};
  static constexpr QLatin1StringView kKeyPublishFrequency{"publishFrequency"};
  static constexpr QLatin1StringView kKeyTopicBase{"topicBase"};
  static constexpr QLatin1StringView kKeyNotificationTopic{"notificationTopic"};
  static constexpr QLatin1StringView kKeyScriptCode{"scriptCode"};
  static constexpr QLatin1StringView kKeyScriptTopic{"scriptTopic"};
  static constexpr QLatin1StringView kKeyScriptLanguage{"scriptLanguage"};
  static constexpr QLatin1StringView kKeyHostname{"hostname"};
  static constexpr QLatin1StringView kKeyPort{"port"};
  static constexpr QLatin1StringView kKeyClientId{"clientId"};
  static constexpr QLatin1StringView kKeyCustomClientId{"customClientId"};
  static constexpr QLatin1StringView kKeyCleanSession{"cleanSession"};
  static constexpr QLatin1StringView kKeyKeepAlive{"keepAlive"};
  static constexpr QLatin1StringView kKeyMqttVersion{"mqttVersion"};
  static constexpr QLatin1StringView kKeySslEnabled{"sslEnabled"};
  static constexpr QLatin1StringView kKeySslProtocol{"sslProtocol"};
  static constexpr QLatin1StringView kKeyPeerVerifyMode{"peerVerifyMode"};
  static constexpr QLatin1StringView kKeyPeerVerifyDepth{"peerVerifyDepth"};
  static constexpr QLatin1StringView kKeyClientCertPath{"clientCertPath"};
  static constexpr QLatin1StringView kKeyPrivateKeyPath{"privateKeyPath"};
  static constexpr QLatin1StringView kKeyAlpnEnabled{"alpnEnabled"};
  static constexpr QLatin1StringView kKeyAlpnProtocol{"alpnProtocol"};
  static constexpr QLatin1StringView kKeySparkplugEnabled{"sparkplugEnabled"};
  static constexpr QLatin1StringView kKeySparkplugGroupId{"sparkplugGroupId"};
  static constexpr QLatin1StringView kKeySparkplugDeviceId{"sparkplugDeviceId"};
  static constexpr QLatin1StringView kKeySparkplugEdgeNodeId{"sparkplugEdgeNodeId"};
};

}  // namespace MQTT

#endif  // BUILD_COMMERCIAL
