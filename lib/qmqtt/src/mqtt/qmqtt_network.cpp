/*
 * qmqtt_network.cpp - qmqtt network
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

#include "qmqtt_network_p.h"
#include "qmqtt_socket_p.h"
#include "qmqtt_ssl_socket_p.h"
#include "qmqtt_timer_p.h"
#include "qmqtt_websocket_p.h"
#include "qmqtt_frame.h"

#include <QDataStream>

const quint16 DEFAULT_PORT = 1883;
const quint16 DEFAULT_SSL_PORT = 8883;
const bool DEFAULT_AUTORECONNECT = false;
const int DEFAULT_AUTORECONNECT_INTERVAL_MS = 5000;

QMQTT::Network::Network(QObject *parent)
  : NetworkInterface(parent)
  , _port(DEFAULT_PORT)
  , _autoReconnect(DEFAULT_AUTORECONNECT)
  , _autoReconnectInterval(DEFAULT_AUTORECONNECT_INTERVAL_MS)
  , _socket(new QMQTT::Socket)
  , _autoReconnectTimer(new QMQTT::Timer)
  , _readState(Header)
{
  initialize();
}

#ifndef QT_NO_SSL
QMQTT::Network::Network(const QSslConfiguration &config, QObject *parent)
  : NetworkInterface(parent)
  , _port(DEFAULT_SSL_PORT)
  , _autoReconnect(DEFAULT_AUTORECONNECT)
  , _autoReconnectInterval(DEFAULT_AUTORECONNECT_INTERVAL_MS)
  , _socket(new QMQTT::SslSocket(config))
  , _autoReconnectTimer(new QMQTT::Timer)
  , _readState(Header)
{
  initialize();
  connect(_socket, &QMQTT::SslSocket::sslErrors, this,
          &QMQTT::Network::sslErrors);
}
#endif // QT_NO_SSL

#ifdef QT_WEBSOCKETS_LIB
#  ifndef QT_NO_SSL
QMQTT::Network::Network(const QString &origin,
                        QWebSocketProtocol::Version version,
                        const QSslConfiguration *sslConfig, QObject *parent)
  : NetworkInterface(parent)
  , _port(DEFAULT_SSL_PORT)
  , _autoReconnect(DEFAULT_AUTORECONNECT)
  , _autoReconnectInterval(DEFAULT_AUTORECONNECT_INTERVAL_MS)
  , _socket(new QMQTT::WebSocket(origin, version, sslConfig))
  , _autoReconnectTimer(new QMQTT::Timer)
  , _readState(Header)
{
  initialize();
}
#  endif // QT_NO_SSL

QMQTT::Network::Network(const QString &origin,
                        QWebSocketProtocol::Version version, QObject *parent)
  : NetworkInterface(parent)
  , _port(DEFAULT_PORT)
  , _autoReconnect(DEFAULT_AUTORECONNECT)
  , _autoReconnectInterval(DEFAULT_AUTORECONNECT_INTERVAL_MS)
  , _socket(new QMQTT::WebSocket(origin, version))
  , _autoReconnectTimer(new QMQTT::Timer)
  , _readState(Header)
{
  initialize();
}
#endif // QT_WEBSOCKETS_LIB

QMQTT::Network::Network(SocketInterface *socketInterface,
                        TimerInterface *timerInterface, QObject *parent)
  : NetworkInterface(parent)
  , _port(DEFAULT_PORT)
  , _autoReconnect(DEFAULT_AUTORECONNECT)
  , _autoReconnectInterval(DEFAULT_AUTORECONNECT_INTERVAL_MS)
  , _socket(socketInterface)
  , _autoReconnectTimer(timerInterface)
  , _readState(Header)
{
  initialize();
}

void QMQTT::Network::initialize()
{
  _socket->setParent(this);
  _autoReconnectTimer->setParent(this);
  _autoReconnectTimer->setSingleShot(true);
  _autoReconnectTimer->setInterval(_autoReconnectInterval);

  QObject::connect(_socket, &SocketInterface::connected, this,
                   &Network::connected);
  QObject::connect(_socket, &SocketInterface::disconnected, this,
                   &Network::onDisconnected);
  QObject::connect(_socket->ioDevice(), &QIODevice::readyRead, this,
                   &Network::onSocketReadReady);
  QObject::connect(_autoReconnectTimer, &TimerInterface::timeout, this,
                   static_cast<void (Network::*)()>(&Network::connectToHost));
  QObject::connect(
      _socket,
      static_cast<void (SocketInterface::*)(QAbstractSocket::SocketError)>(
          &SocketInterface::error),
      this, &Network::onSocketError);
}

QMQTT::Network::~Network() {}

bool QMQTT::Network::isConnectedToHost() const
{
  return _socket->state() == QAbstractSocket::ConnectedState;
}

void QMQTT::Network::connectToHost(const QHostAddress &host, const quint16 port)
{
  // Reset the hostname, because if it is not empty connectToHost() will use it
  // instead of _host.
  _hostName.clear();
  _host = host;
  _port = port;
  connectToHost();
}

void QMQTT::Network::connectToHost(const QString &hostName, const quint16 port)
{
  _hostName = hostName;
  _port = port;
  connectToHost();
}

void QMQTT::Network::connectToHost()
{
  _readState = Header;
  if (_hostName.isEmpty())
  {
    _socket->connectToHost(_host, _port);
  }
  else
  {
    _socket->connectToHost(_hostName, _port);
  }
}

void QMQTT::Network::onSocketError(QAbstractSocket::SocketError socketError)
{
  emit error(socketError);
  if (_autoReconnect)
  {
    _autoReconnectTimer->start();
  }
}

void QMQTT::Network::sendFrame(const Frame &frame)
{
  if (_socket->state() == QAbstractSocket::ConnectedState)
  {
    QDataStream out(_socket->ioDevice());
    frame.write(out);
  }
}

void QMQTT::Network::disconnectFromHost()
{
  _socket->disconnectFromHost();
}

QAbstractSocket::SocketState QMQTT::Network::state() const
{
  return _socket->state();
}

bool QMQTT::Network::autoReconnect() const
{
  return _autoReconnect;
}

void QMQTT::Network::setAutoReconnect(const bool autoReconnect)
{
  _autoReconnect = autoReconnect;
}

int QMQTT::Network::autoReconnectInterval() const
{
  return _autoReconnectInterval;
}

void QMQTT::Network::setAutoReconnectInterval(const int autoReconnectInterval)
{
  _autoReconnectInterval = autoReconnectInterval;
  _autoReconnectTimer->setInterval(_autoReconnectInterval);
}

void QMQTT::Network::onSocketReadReady()
{
  QIODevice *ioDevice = _socket->ioDevice();
  // Only read the available (cached) bytes, so the read will never block.
  QByteArray data = ioDevice->read(ioDevice->bytesAvailable());
  foreach (char byte, data)
  {
    switch (_readState)
    {
      case Header:
        _header = static_cast<quint8>(byte);
        _readState = Length;
        _length = 0;
        _shift = 0;
        _data.resize(0); // keep allocated buffer
        break;
      case Length:
        _length |= (byte & 0x7F) << _shift;
        _shift += 7;
        if ((byte & 0x80) != 0)
        {
          if (_shift > 21)
          { // only up to 4 bytes (3*7 shifts)
            _readState = Header;
            emit error(QAbstractSocket::SocketError::UnknownSocketError);
          }
          break;
        }
        if (_length == 0)
        {
          _readState = Header;
          Frame frame(_header, _data);
          emit received(frame);
          break;
        }
        _readState = PayLoad;
        _data.reserve(_length);
        break;
      case PayLoad:
        _data.append(byte);
        --_length;
        if (_length > 0)
          break;
        _readState = Header;
        Frame frame(_header, _data);
        emit received(frame);
        break;
    }
  }
}

void QMQTT::Network::onDisconnected()
{
  emit disconnected();
  if (_autoReconnect)
  {
    _autoReconnectTimer->start();
  }
}

#ifndef QT_NO_SSL
void QMQTT::Network::ignoreSslErrors(const QList<QSslError> &errors)
{
  _socket->ignoreSslErrors(errors);
}

void QMQTT::Network::ignoreSslErrors()
{
  _socket->ignoreSslErrors();
}

QSslConfiguration QMQTT::Network::sslConfiguration() const
{
  return _socket->sslConfiguration();
}

void QMQTT::Network::setSslConfiguration(const QSslConfiguration &config)
{
  _socket->setSslConfiguration(config);
}
#endif // QT_NO_SSL
