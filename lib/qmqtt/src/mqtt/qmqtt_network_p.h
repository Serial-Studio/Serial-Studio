/*
 * qmqtt_network_p.h - qmqtt network private header
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
#ifndef QMQTT_NETWORK_P_H
#define QMQTT_NETWORK_P_H

#include <qmqtt_networkinterface.h>

#include <QObject>
#include <QHostAddress>
#include <QString>
#include <QByteArray>

#ifdef QT_WEBSOCKETS_LIB
#  include <QWebSocket>
#endif // QT_WEBSOCKETS_LIB

#ifndef QT_NO_SSL
#  include <QSslConfiguration>
QT_FORWARD_DECLARE_CLASS(QSslError)
#endif // QT_NO_SSL

namespace QMQTT
{

class SocketInterface;
class TimerInterface;
class Frame;

class Network : public NetworkInterface
{
  Q_OBJECT

public:
  Network(QObject *parent = nullptr);
#ifndef QT_NO_SSL
  Network(const QSslConfiguration &config, QObject *parent = nullptr);
#endif // QT_NO_SSL
#ifdef QT_WEBSOCKETS_LIB
#  ifndef QT_NO_SSL
  Network(const QString &origin, QWebSocketProtocol::Version version,
          const QSslConfiguration *sslConfig, QObject *parent = nullptr);
#  endif // QT_NO_SSL
  Network(const QString &origin, QWebSocketProtocol::Version version,
          QObject *parent = nullptr);
#endif // QT_WEBSOCKETS_LIB
  Network(SocketInterface *socketInterface, TimerInterface *timerInterface,
          QObject *parent = nullptr);
  ~Network();

  void sendFrame(const Frame &frame);
  bool isConnectedToHost() const;
  bool autoReconnect() const;
  void setAutoReconnect(const bool autoReconnect);
  QAbstractSocket::SocketState state() const;
  int autoReconnectInterval() const;
  void setAutoReconnectInterval(const int autoReconnectInterval);
#ifndef QT_NO_SSL
  void ignoreSslErrors(const QList<QSslError> &errors);
  QSslConfiguration sslConfiguration() const;
  void setSslConfiguration(const QSslConfiguration &config);
#endif // QT_NO_SSL

public slots:
  void connectToHost(const QHostAddress &host, const quint16 port);
  void connectToHost(const QString &hostName, const quint16 port);
  void disconnectFromHost();
#ifndef QT_NO_SSL
  void ignoreSslErrors();
#endif // QT_NO_SSL

protected slots:
  void onSocketError(QAbstractSocket::SocketError socketError);

protected:
  void initialize();

  quint16 _port;
  QHostAddress _host;
  QString _hostName;
  bool _autoReconnect;
  int _autoReconnectInterval;
  SocketInterface *_socket;
  TimerInterface *_autoReconnectTimer;

  enum ReadState
  {
    Header,
    Length,
    PayLoad
  };

  ReadState _readState;
  quint8 _header;
  int _length;
  int _shift;
  QByteArray _data;

protected slots:
  void onSocketReadReady();
  void onDisconnected();
  void connectToHost();

private:
  Q_DISABLE_COPY(Network)
};

} // namespace QMQTT

#endif // QMQTT_NETWORK_P_H
