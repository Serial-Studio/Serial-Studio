/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

// clang-format off
#include <QtMqtt>
#include <QObject>
#include <QSslConfiguration>
// clang-format on

namespace MQTT
{
class Client : public QObject
{
  Q_OBJECT

signals:
  void connectedChanged();
  void sslConfigurationChanged();
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

private slots:
  void publishMessage(const QByteArray &data);
  void onStateChanged(QMqttClient::ClientState state);
  void onErrorChanged(QMqttClient::ClientError error);
  void onAuthenticationFinished(const QMqttAuthenticationProperties &p);
  void onAuthenticationRequested(const QMqttAuthenticationProperties &p);

private:
  quint8 m_mode;
  bool m_publisher;
  bool m_sslEnabled;
  QString m_topicFilter;

  QMqttClient m_client;
  QMqttTopicName m_topicName;
  QSslConfiguration m_sslConfiguration;

  QMap<QString, QSsl::SslProtocol> m_sslProtocols;
  QMap<QString, QMqttClient::ProtocolVersion> m_mqttVersions;
  QMap<QString, QSslSocket::PeerVerifyMode> m_peerVerifyModes;
};
} // namespace MQTT
