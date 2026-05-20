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

// clang-format off
#include <QtMqtt>
#include <QMap>
#include <QSettings>
#include <QSslConfiguration>
// clang-format on

#include "IO/HAL_Driver.h"
#include "MQTT/CredentialVault.h"

namespace IO {
namespace Drivers {

/**
 * @brief HAL driver that subscribes to a single MQTT topic filter and feeds
 *        received payloads into the frame-reader pipeline.
 */
class MQTT : public HAL_Driver {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY connectedChanged)
  Q_PROPERTY(bool sslEnabled
             READ sslEnabled
             WRITE setSslEnabled
             NOTIFY sslConfigurationChanged)
  Q_PROPERTY(bool cleanSession
             READ cleanSession
             WRITE setCleanSession
             NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(bool autoKeepAlive
             READ autoKeepAlive
             WRITE setAutoKeepAlive
             NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(quint16 port
             READ port
             WRITE setPort
             NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(quint16 keepAlive
             READ keepAlive
             WRITE setKeepAlive
             NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(quint8 mqttVersion
             READ mqttVersion
             WRITE setMqttVersion
             NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(quint8 sslProtocol
             READ sslProtocol
             WRITE setSslProtocol
             NOTIFY sslConfigurationChanged)
  Q_PROPERTY(quint8 peerVerifyMode
             READ peerVerifyMode
             WRITE setPeerVerifyMode
             NOTIFY sslConfigurationChanged)
  Q_PROPERTY(int peerVerifyDepth
             READ peerVerifyDepth
             WRITE setPeerVerifyDepth
             NOTIFY sslConfigurationChanged)
  Q_PROPERTY(QString clientId
             READ clientId
             WRITE setClientId
             NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(QString hostname
             READ hostname
             WRITE setHostname
             NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(QString username
             READ username
             WRITE setUsername
             NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(QString password
             READ password
             WRITE setPassword
             NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(QString topicFilter
             READ topicFilter
             WRITE setTopicFilter
             NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(QStringList mqttVersions
             READ mqttVersions
             CONSTANT)
  Q_PROPERTY(QStringList sslProtocols
             READ sslProtocols
             CONSTANT)
  Q_PROPERTY(QStringList peerVerifyModes
             READ peerVerifyModes
             CONSTANT)
  Q_PROPERTY(QStringList caCertificates
             READ caCertificates
             CONSTANT)
  // clang-format on

signals:
  void connectedChanged();
  void sslConfigurationChanged();
  void mqttConfigurationChanged();

public:
  explicit MQTT();
  ~MQTT() override;

  MQTT(MQTT&&)                 = delete;
  MQTT(const MQTT&)            = delete;
  MQTT& operator=(MQTT&&)      = delete;
  MQTT& operator=(const MQTT&) = delete;

  void close() override;

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;

  [[nodiscard]] bool sslEnabled() const noexcept;
  [[nodiscard]] bool autoKeepAlive() const noexcept;
  [[nodiscard]] bool cleanSession() const noexcept;

  [[nodiscard]] quint16 port() const noexcept;
  [[nodiscard]] quint16 keepAlive() const noexcept;

  [[nodiscard]] quint8 mqttVersion() const noexcept;
  [[nodiscard]] quint8 sslProtocol() const noexcept;
  [[nodiscard]] quint8 peerVerifyMode() const noexcept;
  [[nodiscard]] int peerVerifyDepth() const noexcept;

  [[nodiscard]] QString clientId() const;
  [[nodiscard]] QString hostname() const;
  [[nodiscard]] QString username() const;
  [[nodiscard]] QString password() const;
  [[nodiscard]] QString topicFilter() const;

  [[nodiscard]] const QStringList& mqttVersions() const;
  [[nodiscard]] const QStringList& sslProtocols() const;
  [[nodiscard]] const QStringList& peerVerifyModes() const;
  [[nodiscard]] const QStringList& caCertificates() const;

public slots:
  void setDriverProperty(const QString& key, const QVariant& value) override;
  void regenerateClientId();
  void addCaCertificates();
  void setPort(const quint16 port);
  void setKeepAlive(const quint16 keepAlive);
  void setAutoKeepAlive(const bool autoKeepAlive);
  void setCleanSession(const bool cleanSession);
  void setMqttVersion(const quint8 version);
  void setSslEnabled(const bool enabled);
  void setSslProtocol(const quint8 protocol);
  void setPeerVerifyMode(const quint8 verifyMode);
  void setPeerVerifyDepth(const int depth);
  void setClientId(const QString& id);
  void setHostname(const QString& hostname);
  void setUsername(const QString& username);
  void setPassword(const QString& password);
  void setTopicFilter(const QString& topic);

private slots:
  void onStateChanged(QMqttClient::ClientState state);
  void onErrorChanged(QMqttClient::ClientError error);
  void onMessageReceived(const QByteArray& message, const QMqttTopicName& topic);

private:
  void loadPersistedSettings();
  [[nodiscard]] QString settingsKey(const char* leaf) const;
  void appendMqttSslProperties(QList<IO::DriverProperty>& props) const;
  void scheduleReconnectIfActive();
  void applyPendingToClient();

private:
  bool m_sslEnabled;
  bool m_cleanSession;
  bool m_autoKeepAlive;
  bool m_userWantsOpen;
  bool m_reconnectPending;
  quint16 m_port;
  quint16 m_keepAlive;
  QMqttClient::ProtocolVersion m_protocolVersion;
  QString m_clientId;
  QString m_hostname;
  QString m_username;
  QString m_password;
  QString m_topicFilter;
  QSettings m_settings;
  ::MQTT::CredentialVault m_vault;
  QMqttClient m_client;
  QSslConfiguration m_sslConfiguration;
  QMap<QString, QSsl::SslProtocol> m_sslProtocols;
  QMap<QString, QMqttClient::ProtocolVersion> m_mqttVersions;
  QMap<QString, QSslSocket::PeerVerifyMode> m_peerVerifyModes;
};
}  // namespace Drivers
}  // namespace IO
