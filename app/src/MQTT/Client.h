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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

#pragma once

#ifdef COMMERCIAL_BUILD

#  include <QtMqtt>
#  include <QObject>
#  include <QPointer>
#  include <QHostInfo>
#  include <QByteArray>
#  include <QHostAddress>
#  include <QSslConfiguration>

namespace MQTT
{
/**
 * @brief The MQTTClientMode enum
 *
 * Specifies the operation modes of the MQTT client
 */
enum MQTTClientMode
{
  ClientPublisher = 0,
  ClientSubscriber = 1
};

/**
 * @brief The Client class
 *
 * Implements a simple MQTT client, which allows Serial Studio to upload
 * received frames to a MQTT broker so that other devices and/or services can
 * make use of that information. By acting as a MQTT subscriber, Serial Studio
 * can display & process frames from a remote Serial Studio instance that is
 * setup as a MQTT publisher. As you might notice, this has a lot of interesting
 * applications.
 *
 * For example, you can receive frames from a CanSat mission and display them
 * almost in real-time in another location, such as the "ground control" centre
 * or by the media team which streams the GCS display on the internet as the
 * mission is developing.
 */
class Client : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(quint16 port
             READ port
             WRITE setPort
             NOTIFY portChanged)
  Q_PROPERTY(quint8 qos
             READ qos
             WRITE setQos
             NOTIFY qosChanged)
  Q_PROPERTY(QString host
             READ host
             WRITE setHost
             NOTIFY hostChanged)
  Q_PROPERTY(bool retain
             READ retain
             WRITE setRetain
             NOTIFY retainChanged)
  Q_PROPERTY(quint16 keepAlive
             READ keepAlive
             WRITE setKeepAlive
             NOTIFY keepAliveChanged)
  Q_PROPERTY(QString topic
             READ topic
             WRITE setTopic
             NOTIFY topicChanged)
  Q_PROPERTY(int mqttVersion
             READ mqttVersion
             WRITE setMqttVersion
             NOTIFY mqttVersionChanged)
  Q_PROPERTY(int clientMode
             READ clientMode
             WRITE setClientMode
             NOTIFY clientModeChanged)
  Q_PROPERTY(QString username
             READ username
             WRITE setUsername
             NOTIFY usernameChanged)
  Q_PROPERTY(QString password
             READ password
             WRITE setPassword
             NOTIFY passwordChanged)
  Q_PROPERTY(QString clientId
             READ clientId
             WRITE setClientId
             NOTIFY clientIdChanged)
  Q_PROPERTY(bool sslEnabled
             READ sslEnabled
             WRITE setSslEnabled
             NOTIFY sslEnabledChanged)
  Q_PROPERTY(int sslProtocol
             READ sslProtocol
             WRITE setSslProtocol
             NOTIFY sslProtocolChanged)
  Q_PROPERTY(bool isConnectedToHost
             READ isConnectedToHost
             NOTIFY connectedChanged)
  Q_PROPERTY(bool lookupActive
             READ lookupActive
             NOTIFY lookupActiveChanged)
  Q_PROPERTY(QStringList mqttVersions
             READ mqttVersions
             CONSTANT)
  Q_PROPERTY(QStringList clientModes
             READ clientModes
             CONSTANT)
  Q_PROPERTY(QStringList qosLevels
             READ qosLevels
             CONSTANT)
  Q_PROPERTY(QStringList sslProtocols
             READ sslProtocols
             CONSTANT)
  Q_PROPERTY(quint16 defaultPort
             READ defaultPort
             CONSTANT)
  Q_PROPERTY(QString defaultHost
             READ defaultHost
             CONSTANT)
  Q_PROPERTY(QString caFilePath
             READ caFilePath
             NOTIFY caFilePathChanged)
  Q_PROPERTY(bool isSubscribed
             READ isSubscribed
             NOTIFY connectedChanged)
  // clang-format on

signals:
  void qosChanged();
  void portChanged();
  void hostChanged();
  void topicChanged();
  void retainChanged();
  void usernameChanged();
  void passwordChanged();
  void clientIdChanged();
  void keepAliveChanged();
  void connectedChanged();
  void caFilePathChanged();
  void clientModeChanged();
  void sslEnabledChanged();
  void sslProtocolChanged();
  void mqttVersionChanged();
  void lookupActiveChanged();

private:
  explicit Client();
  Client(Client &&) = delete;
  Client(const Client &) = delete;
  Client &operator=(Client &&) = delete;
  Client &operator=(const Client &) = delete;

  ~Client();

public:
  static Client &instance();

  [[nodiscard]] quint8 qos() const;
  [[nodiscard]] bool retain() const;
  [[nodiscard]] quint16 port() const;
  [[nodiscard]] QString host() const;
  [[nodiscard]] QString topic() const;
  [[nodiscard]] int clientMode() const;
  [[nodiscard]] int sslProtocol() const;
  [[nodiscard]] int mqttVersion() const;
  [[nodiscard]] bool sslEnabled() const;
  [[nodiscard]] QString username() const;
  [[nodiscard]] QString password() const;
  [[nodiscard]] QString clientId() const;
  [[nodiscard]] quint16 keepAlive() const;
  [[nodiscard]] bool lookupActive() const;
  [[nodiscard]] bool isSubscribed() const;
  [[nodiscard]] bool isConnectedToHost() const;

  [[nodiscard]] QStringList qosLevels() const;
  [[nodiscard]] QStringList clientModes() const;
  [[nodiscard]] QStringList mqttVersions() const;
  [[nodiscard]] QStringList sslProtocols() const;

  [[nodiscard]] QString caFilePath() const;
  [[nodiscard]] quint16 defaultPort() const { return 1883; }
  [[nodiscard]] QString defaultHost() const { return "127.0.0.1"; }

public slots:
  void loadCaFile();
  void connectToHost();
  void toggleConnection();
  void disconnectFromHost();
  void setQos(const quint8 qos);
  void lookup(const QString &host);
  void setPort(const quint16 port);
  void setHost(const QString &host);
  void setRetain(const bool retain);
  void setClientMode(const int mode);
  void setTopic(const QString &topic);
  void loadCaFile(const QString &path);
  void setSslProtocol(const int index);
  void setSslEnabled(const bool enabled);
  void setUsername(const QString &username);
  void setPassword(const QString &password);
  void setClientId(const QString &clientId);
  void setKeepAlive(const quint16 keepAlive);
  void setMqttVersion(const int versionIndex);

private slots:
  void resetStatistics();
  void onConnectedChanged();
  void sendFrame(const QByteArray &frame);
  void lookupFinished(const QHostInfo &info);
  // void onError(const QMQTT::ClientError error);
  void onSslErrors(const QList<QSslError> &errors);
  // void onMessageReceived(const QMQTT::Message &message);

private:
  void regenerateClient();

private:
  QString m_topic;
  bool m_sslEnabled;
  int m_sslProtocol;
  bool m_lookupActive;
  QString m_caFilePath;
  quint16 m_sentMessages;
  MQTTClientMode m_clientMode;
  QSslConfiguration m_sslConfiguration;
};
} // namespace MQTT

#endif
