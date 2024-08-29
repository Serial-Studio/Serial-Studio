/*
 * qmqtt_client.h - qmqtt client header
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
#ifndef QMQTT_CLIENT_H
#define QMQTT_CLIENT_H

#include <qmqtt_global.h>

#include <QObject>
#include <QString>
#include <QHostAddress>
#include <QByteArray>
#include <QAbstractSocket>
#include <QScopedPointer>
#include <QList>

#ifdef QT_WEBSOCKETS_LIB
#  include <QWebSocket>
#endif // QT_WEBSOCKETS_LIB

#ifndef QT_NO_SSL
#  include <QSslConfiguration>
QT_FORWARD_DECLARE_CLASS(QSslError)
#endif // QT_NO_SSL

#ifndef Q_ENUM_NS
#  define Q_ENUM_NS(x)
#endif // Q_ENUM_NS

namespace QMQTT
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
Q_MQTT_EXPORT Q_NAMESPACE
#endif

    static const quint8 LIBRARY_VERSION_MAJOR
    = 0;
static const quint8 LIBRARY_VERSION_MINOR = 3;
static const quint8 LIBRARY_VERSION_REVISION = 1;
// static const char* LIBRARY_VERSION = "0.3.1";

enum MQTTVersion
{
  V3_1_0 = 3,
  V3_1_1 = 4
};
Q_ENUM_NS(MQTTVersion)

enum ConnectionState
{
  STATE_INIT = 0,
  STATE_CONNECTING,
  STATE_CONNECTED,
  STATE_DISCONNECTED
};
Q_ENUM_NS(ConnectionState)

enum ClientError
{
  UnknownError = 0,
  SocketConnectionRefusedError,
  SocketRemoteHostClosedError,
  SocketHostNotFoundError,
  SocketAccessError,
  SocketResourceError,
  SocketTimeoutError,
  SocketDatagramTooLargeError,
  SocketNetworkError,
  SocketAddressInUseError,
  SocketAddressNotAvailableError,
  SocketUnsupportedSocketOperationError,
  SocketUnfinishedSocketOperationError,
  SocketProxyAuthenticationRequiredError,
  SocketSslHandshakeFailedError,
  SocketProxyConnectionRefusedError,
  SocketProxyConnectionClosedError,
  SocketProxyConnectionTimeoutError,
  SocketProxyNotFoundError,
  SocketProxyProtocolError,
  SocketOperationError,
  SocketSslInternalError,
  SocketSslInvalidUserDataError,
  SocketTemporaryError,
  MqttUnacceptableProtocolVersionError = 1 << 16,
  MqttIdentifierRejectedError,
  MqttServerUnavailableError,
  MqttBadUserNameOrPasswordError,
  MqttNotAuthorizedError,
  MqttNoPingResponse
};
Q_ENUM_NS(ClientError)

class ClientPrivate;
class Message;
class Frame;
class NetworkInterface;

class Q_MQTT_EXPORT Client : public QObject
{
  Q_OBJECT
  Q_PROPERTY(quint16 _port READ port WRITE setPort)
  Q_PROPERTY(QHostAddress _host READ host WRITE setHost)
  Q_PROPERTY(QString _hostName READ hostName WRITE setHostName)
  Q_PROPERTY(QString _clientId READ clientId WRITE setClientId)
  Q_PROPERTY(QString _username READ username WRITE setUsername)
  Q_PROPERTY(QByteArray _password READ password WRITE setPassword)
  Q_PROPERTY(quint16 _keepAlive READ keepAlive WRITE setKeepAlive)
  Q_PROPERTY(MQTTVersion _version READ version WRITE setVersion)
  Q_PROPERTY(bool _autoReconnect READ autoReconnect WRITE setAutoReconnect)
  Q_PROPERTY(int _autoReconnectInterval READ autoReconnectInterval WRITE
                 setAutoReconnectInterval)
  Q_PROPERTY(bool _cleanSession READ cleanSession WRITE setCleanSession)
  Q_PROPERTY(QString _willTopic READ willTopic WRITE setWillTopic)
  Q_PROPERTY(quint8 _willQos READ willQos WRITE setWillQos)
  Q_PROPERTY(bool _willRetain READ willRetain WRITE setWillRetain)
  Q_PROPERTY(QByteArray _willMessage READ willMessage WRITE setWillMessage)
  Q_PROPERTY(ConnectionState _connectionState READ connectionState)
#ifndef QT_NO_SSL
  Q_PROPERTY(QSslConfiguration _sslConfiguration READ sslConfiguration WRITE
                 setSslConfiguration)
#endif // QT_NO_SSL

public:
  Client(const QHostAddress &host = QHostAddress::LocalHost,
         const quint16 port = 1883, QObject *parent = nullptr);

#ifndef QT_NO_SSL
  Client(const QString &hostName, const quint16 port,
         const QSslConfiguration &config, const bool ignoreSelfSigned = false,
         QObject *parent = nullptr);
#endif // QT_NO_SSL

  // This function is provided for backward compatibility with older versions of
  // QMQTT. If the ssl parameter is true, this function will load a private key
  // ('cert.key') and a local certificate ('cert.crt') from the current working
  // directory. It will also set PeerVerifyMode to None. This may not be the
  // safest way to set up an SSL connection.
  Client(const QString &hostName, const quint16 port, const bool ssl,
         const bool ignoreSelfSigned, QObject *parent = nullptr);

#ifdef QT_WEBSOCKETS_LIB
  // Create a connection over websockets
  Client(const QString &url, const QString &origin,
         QWebSocketProtocol::Version version, bool ignoreSelfSigned = false,
         QObject *parent = nullptr);

#  ifndef QT_NO_SSL
  Client(const QString &url, const QString &origin,
         QWebSocketProtocol::Version version, const QSslConfiguration &config,
         const bool ignoreSelfSigned = false, QObject *parent = nullptr);
#  endif // QT_NO_SSL
#endif   // QT_WEBSOCKETS_LIB

  // for testing purposes only
  Client(NetworkInterface *network,
         const QHostAddress &host = QHostAddress::LocalHost,
         const quint16 port = 1883, QObject *parent = nullptr);

  virtual ~Client();

  QHostAddress host() const;
  QString hostName() const;
  quint16 port() const;
  QString clientId() const;
  QString username() const;
  QByteArray password() const;
  QMQTT::MQTTVersion version() const;
  quint16 keepAlive() const;
  bool cleanSession() const;
  bool autoReconnect() const;
  int autoReconnectInterval() const;
  ConnectionState connectionState() const;
  QString willTopic() const;
  quint8 willQos() const;
  bool willRetain() const;
  QByteArray willMessage() const;

  bool isConnectedToHost() const;
#ifndef QT_NO_SSL
  QSslConfiguration sslConfiguration() const;
  void setSslConfiguration(const QSslConfiguration &config);
#endif // QT_NO_SSL

public slots:
  void setHost(const QHostAddress &host);
  void setHostName(const QString &hostName);
  void setPort(const quint16 port);
  void setClientId(const QString &clientId);
  void setUsername(const QString &username);
  void setPassword(const QByteArray &password);
  void setVersion(const MQTTVersion version);
  void setKeepAlive(const quint16 keepAlive);
  void setCleanSession(const bool cleanSession);
  void setAutoReconnect(const bool value);
  void setAutoReconnectInterval(const int autoReconnectInterval);
  void setWillTopic(const QString &willTopic);
  void setWillQos(const quint8 willQos);
  void setWillRetain(const bool willRetain);
  void setWillMessage(const QByteArray &willMessage);

  void connectToHost();
  void disconnectFromHost();

  void subscribe(const QString &topic, const quint8 qos = 0);
  void unsubscribe(const QString &topic);

  quint16 publish(const QMQTT::Message &message);

#ifndef QT_NO_SSL
  void ignoreSslErrors();
  void ignoreSslErrors(const QList<QSslError> &errors);
#endif // QT_NO_SSL

Q_SIGNALS:
  void connected();
  void disconnected();
  void error(const QMQTT::ClientError error);

  void subscribed(const QString &topic, const quint8 qos = 0);
  void unsubscribed(const QString &topic);
  void published(const QMQTT::Message &message, quint16 msgid = 0);
  void received(const QMQTT::Message &message);
  void pingresp();
#ifndef QT_NO_SSL
  void sslErrors(const QList<QSslError> &errors);
#endif // QT_NO_SSL

protected slots:
  void onNetworkConnected();
  void onNetworkDisconnected();
  void onNetworkReceived(const QMQTT::Frame &frame);
  void onTimerPingReq();
  void onPingTimeout();
  void onNetworkError(QAbstractSocket::SocketError error);
#ifndef QT_NO_SSL
  void onSslErrors(const QList<QSslError> &errors);
#endif // QT_NO_SSL

protected:
  QScopedPointer<ClientPrivate> d_ptr;

private:
  Q_DISABLE_COPY(Client)
  Q_DECLARE_PRIVATE(Client)
};

} // namespace QMQTT

Q_DECLARE_METATYPE(QMQTT::ClientError)

#endif // QMQTT_CLIENT_H
