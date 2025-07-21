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
#include <QObject>
#include <QSslConfiguration>
// clang-format on

namespace MQTT
{
/**
 * @class MQTT::Client
 * @brief Singleton wrapper around QMqttClient for managing MQTT connections.
 *
 * This class encapsulates an MQTT client with support for publisher/subscriber
 * roles, clean session management, SSL configuration, and MQTT 3.1 to 5.0.
 * Provides a Qt-friendly API with slots and signals for integration in GUI
 * apps or service components. It handles connection lifecycle, error reporting,
 * authentication (including MQTT 5.0 extended), and user interaction via
 * dialogs.
 *
 * Modes, protocols, and options are exposed via QStringLists to integrate with
 * Qt model/view components. Single instance, use via Client::instance().
 */
class Client : public QObject
{
  // clang-format off
  Q_OBJECT

  // Connection and identification
  Q_PROPERTY(QString clientId READ clientId WRITE setClientId NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(QString hostname READ hostname WRITE setHostname NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(bool cleanSession READ cleanSession WRITE setCleanSession NOTIFY mqttConfigurationChanged)

  // MQTT mode and state
  Q_PROPERTY(quint8 mode READ mode WRITE setMode NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectedChanged)
  Q_PROPERTY(bool isPublisher READ isPublisher NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(bool isSubscriber READ isSubscriber NOTIFY mqttConfigurationChanged)

  // MQTT protocol config
  Q_PROPERTY(quint8 mqttVersion READ mqttVersion WRITE setMqttVersion NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(QStringList mqttVersions READ mqttVersions CONSTANT)

  // Will message
  Q_PROPERTY(quint8 willQoS READ willQoS WRITE setWillQoS NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(bool willRetain READ willRetain WRITE setWillRetain NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(QString willTopic READ willTopic WRITE setWillTopic NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(QString willMessage READ willMessage WRITE setWillMessage NOTIFY mqttConfigurationChanged)

  // Keep alive
  Q_PROPERTY(quint16 keepAlive READ keepAlive WRITE setKeepAlive NOTIFY mqttConfigurationChanged)
  Q_PROPERTY(bool autoKeepAlive READ autoKeepAlive WRITE setAutoKeepAlive NOTIFY mqttConfigurationChanged)

  // Topic
  Q_PROPERTY(QString topicFilter READ topicFilter WRITE setTopic NOTIFY mqttConfigurationChanged)

  // SSL
  Q_PROPERTY(bool sslEnabled READ sslEnabled WRITE setSslEnabled NOTIFY sslConfigurationChanged)
  Q_PROPERTY(quint8 sslProtocol READ sslProtocol WRITE setSslProtocol NOTIFY sslConfigurationChanged)
  Q_PROPERTY(int peerVerifyDepth READ peerVerifyDepth WRITE setPeerVerifyDepth NOTIFY sslConfigurationChanged)
  Q_PROPERTY(quint8 peerVerifyMode READ peerVerifyMode WRITE setPeerVerifyMode NOTIFY sslConfigurationChanged)
  Q_PROPERTY(QStringList caCertificates READ caCertificates CONSTANT)
  Q_PROPERTY(QStringList sslProtocols READ sslProtocols CONSTANT)
  Q_PROPERTY(QStringList peerVerifyModes READ peerVerifyModes CONSTANT)

  // Modes (Publisher/Subscriber descriptions)
  Q_PROPERTY(QStringList modes READ modes CONSTANT)
  // clang-format on

signals:
  void connectedChanged();
  void sslConfigurationChanged();
  void mqttConfigurationChanged();
  void highlightMqttTopicControl();
  void messageReceived(const QByteArray &data);

private:
  explicit Client();
  Client(Client &&) = delete;
  Client(const Client &) = delete;
  Client &operator=(Client &&) = delete;
  Client &operator=(const Client &) = delete;

public:
  static Client &instance();

  [[nodiscard]] quint8 mode() const;
  [[nodiscard]] bool isConnected() const;
  [[nodiscard]] bool isPublisher() const;
  [[nodiscard]] bool isSubscriber() const;
  [[nodiscard]] bool cleanSession() const;

  [[nodiscard]] QString clientId() const;
  [[nodiscard]] QString hostname() const;
  [[nodiscard]] QString username() const;
  [[nodiscard]] QString password() const;
  [[nodiscard]] QString topicFilter() const;

  [[nodiscard]] quint8 willQoS() const;
  [[nodiscard]] bool willRetain() const;
  [[nodiscard]] QString willTopic() const;
  [[nodiscard]] QString willMessage() const;

  [[nodiscard]] quint16 port() const;
  [[nodiscard]] quint16 keepAlive() const;
  [[nodiscard]] bool autoKeepAlive() const;

  [[nodiscard]] quint8 mqttVersion() const;
  [[nodiscard]] const QStringList &mqttVersions() const;

  [[nodiscard]] bool sslEnabled() const;
  [[nodiscard]] quint8 sslProtocol() const;
  [[nodiscard]] int peerVerifyDepth() const;
  [[nodiscard]] quint8 peerVerifyMode() const;
  [[nodiscard]] const QStringList &caCertificates() const;

  [[nodiscard]] const QStringList &modes() const;
  [[nodiscard]] const QStringList &sslProtocols() const;
  [[nodiscard]] const QStringList &peerVerifyModes() const;

  [[nodiscard]] QMqttClient &client();

public slots:
  void openConnection();
  void closeConnection();
  void toggleConnection();
  void regenerateClientId();
  void setMode(const quint8 mode);
  void setTopic(const QString &topic);
  void setClientId(const QString &id);
  void setHostname(const QString &hostname);
  void setUsername(const QString &username);
  void setPassword(const QString &password);
  void setCleanSession(const bool cleanSession);

  void setWillQoS(const quint8 qos);
  void setWillRetain(const bool retain);
  void setWillTopic(const QString &topic);
  void setWillMessage(const QString &message);

  void setPort(const quint16 port);
  void setKeepAlive(const quint16 keepAlive);
  void setAutoKeepAlive(const bool keepAlive);

  void setMqttVersion(const quint8 version);

  void addCaCertificates();
  void setSslEnabled(const bool enabled);
  void setPeerVerifyDepth(const int depth);
  void setSslProtocol(const quint8 protocol);
  void setPeerVerifyMode(const quint8 verifyMode);

  void hotpathTxFrame(const QByteArray &data);

private slots:
  void onStateChanged(QMqttClient::ClientState state);
  void onErrorChanged(QMqttClient::ClientError error);
  void onAuthenticationFinished(const QMqttAuthenticationProperties &p);
  void onAuthenticationRequested(const QMqttAuthenticationProperties &p);
  void onMessageReceived(const QByteArray &message,
                         const QMqttTopicName &topic = QMqttTopicName());

private:
  quint8 m_mode;
  bool m_publisher;
  bool m_sslEnabled;

  QString m_clientId;
  QString m_topicFilter;

  QMqttClient m_client;
  QMqttTopicName m_topicName;
  QSslConfiguration m_sslConfiguration;

  QMap<QString, QSsl::SslProtocol> m_sslProtocols;
  QMap<QString, QMqttClient::ProtocolVersion> m_mqttVersions;
  QMap<QString, QSslSocket::PeerVerifyMode> m_peerVerifyModes;
};
} // namespace MQTT
