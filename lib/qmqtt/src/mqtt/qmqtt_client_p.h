/*
 * qmqtt_client_p.h - qmqtt client private header
 *
 * Copyright (c) 2013  Ery Lee <ery.lee at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of mqttc nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef QMQTT_CLIENT_P_H
#define QMQTT_CLIENT_P_H

#include <qmqtt_client.h>

#include <QHostAddress>
#include <QString>
#include <QByteArray>
#include <QHash>
#include <QTimer>
#include <QAbstractSocket>

#ifdef QT_WEBSOCKETS_LIB
#  include <QWebSocket>
#endif // QT_WEBSOCKETS_LIB

#ifndef QT_NO_SSL
#  include <QSslConfiguration>
QT_FORWARD_DECLARE_CLASS(QSslError)
#endif // QT_NO_SSL

namespace QMQTT
{

class NetworkInterface;

class ClientPrivate
{
public:
  ClientPrivate(Client *qq_ptr);
  ~ClientPrivate();

  void init(const QHostAddress &host, const quint16 port,
            NetworkInterface *network = nullptr);
#ifndef QT_NO_SSL
  void init(const QString &hostName, const quint16 port,
            const QSslConfiguration &config,
            const bool ignoreSelfSigned = false);
#endif // QT_NO_SSL
  void init(const QString &hostName, const quint16 port, const bool ssl,
            const bool ignoreSelfSigned);
#ifdef QT_WEBSOCKETS_LIB
#  ifndef QT_NO_SSL
  void init(const QString &url, const QString &origin,
            QWebSocketProtocol::Version version,
            const QSslConfiguration *sslConfig, bool ignoreSelfSigned);
#  endif // QT_NO_SSL
  void init(const QString &url, const QString &origin,
            QWebSocketProtocol::Version version);
#endif // QT_WEBSOCKETS_LIB
  void init(NetworkInterface *network);

  QHostAddress _host;
  QString _hostName;
  quint16 _port;
#ifdef QT_WEBSOCKETS_LIB
  QWebSocketProtocol::Version _webSocketVersion;
#endif // QT_WEBSOCKETS_LIB
#ifndef QT_NO_SSL
  bool _ignoreSelfSigned;
#endif // QT_NO_SSL
  quint16 _gmid;
  MQTTVersion _version;
  QString _clientId;
  QString _username;
  QByteArray _password;
  bool _cleanSession;
  ConnectionState _connectionState;
  QScopedPointer<NetworkInterface> _network;
  QTimer _timer;
  QTimer _pingResponseTimer;
  QString _willTopic;
  quint8 _willQos;
  bool _willRetain;
  QByteArray _willMessage;
  QHash<quint16, QString> _midToTopic;
  QHash<quint16, Message> _midToMessage;

  Client *const q_ptr;

  quint16 nextmid();
  void connectToHost();
  void sendConnect();
  void onTimerPingReq();
  void onPingTimeout();
  quint16 sendUnsubscribe(const QString &topic);
  quint16 sendSubscribe(const QString &topic, const quint8 qos);
  quint16 sendPublish(const Message &message);
  void sendPuback(const quint8 type, const quint16 mid);
  void sendDisconnect();
  void sendFrame(const Frame &frame);
  void disconnectFromHost();
  void stopKeepAlive();
  void onNetworkConnected();
  void onNetworkDisconnected();
  quint16 publish(const Message &message);
  void puback(const quint8 type, const quint16 msgid);
  void subscribe(const QString &topic, const quint8 qos);
  void unsubscribe(const QString &topic);
  void onNetworkReceived(const QMQTT::Frame &frame);
  void handleConnack(const quint8 ack);
  void handlePublish(const Message &message);
  void handlePuback(const quint8 type, const quint16 msgid);
  void handleSuback(const QString &topic, const quint8 qos);
  void handleUnsuback(const QString &topic);
  void handlePingresp();
  bool autoReconnect() const;
  void setAutoReconnect(const bool autoReconnect);
  int autoReconnectInterval() const;
  void setAutoReconnectInterval(const int autoReconnectInterval);
  bool isConnectedToHost() const;
  QMQTT::ConnectionState connectionState() const;
  void setCleanSession(const bool cleanSession);
  bool cleanSession() const;
  void setKeepAlive(const quint16 keepAlive);
  quint16 keepAlive() const;
  void setPassword(const QByteArray &password);
  QByteArray password() const;
  void setUsername(const QString &username);
  QString username() const;
  void setVersion(const MQTTVersion);
  MQTTVersion version() const;
  void setClientId(const QString &clientId);
  QString clientId() const;
  void setPort(const quint16 port);
  quint16 port() const;
  void setHost(const QHostAddress &host);
  QHostAddress host() const;
  void setHostName(const QString &hostName);
  QString hostName() const;
  void setWillTopic(const QString &willTopic);
  void setWillQos(const quint8 willQos);
  void setWillRetain(const bool willRetain);
  void setWillMessage(const QByteArray &willMessage);
  QString willTopic() const;
  quint8 willQos() const;
  bool willRetain() const;
  QByteArray willMessage() const;
  void onNetworkError(QAbstractSocket::SocketError error);
#ifndef QT_NO_SSL
  void ignoreSslErrors();
  void ignoreSslErrors(const QList<QSslError> &errors);
  QSslConfiguration sslConfiguration() const;
  void setSslConfiguration(const QSslConfiguration &config);
  void onSslErrors(const QList<QSslError> &errors);
#endif // QT_NO_SSL

  Q_DECLARE_PUBLIC(Client)
};

} // namespace QMQTT

#endif // QMQTT_CLIENT_P_H
