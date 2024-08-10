/*
 * qmqtt_ssl_socket.cpp - qmqtt SSL socket
 *
 * Copyright (c) 2013  Ery Lee <ery.lee at gmail dot com>
 * Copyright (c) 2016  Matthias Dieter Wallnöfer
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

#include "qmqtt_ssl_socket_p.h"

#ifndef QT_NO_SSL

#  include <QSslSocket>
#  include <QSslError>

QMQTT::SslSocket::SslSocket(const QSslConfiguration &config, QObject *parent)
  : SocketInterface(parent)
  , _socket(new QSslSocket(this))
{
  _socket->setSslConfiguration(config);
  connect(_socket.data(), &QSslSocket::encrypted, this,
          &SocketInterface::connected);
  connect(_socket.data(), &QSslSocket::disconnected, this,
          &SocketInterface::disconnected);
  connect(_socket.data(),
#  if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
          static_cast<void (QSslSocket::*)(QAbstractSocket::SocketError)>(
              &QSslSocket::errorOccurred),
#  else
          static_cast<void (QSslSocket::*)(QAbstractSocket::SocketError)>(
              &QSslSocket::error),
#  endif
          this,
          static_cast<void (SocketInterface::*)(QAbstractSocket::SocketError)>(
              &SocketInterface::error));
  connect(_socket.data(),
          static_cast<void (QSslSocket::*)(const QList<QSslError> &)>(
              &QSslSocket::sslErrors),
          this, &SslSocket::sslErrors);
}

QMQTT::SslSocket::~SslSocket() {}

QIODevice *QMQTT::SslSocket::ioDevice()
{
  return _socket.data();
}

void QMQTT::SslSocket::connectToHost(const QHostAddress &address, quint16 port)
{
  _socket->connectToHostEncrypted(address.toString(), port);
}

void QMQTT::SslSocket::connectToHost(const QString &hostName, quint16 port)
{
  _socket->connectToHostEncrypted(hostName, port);
}

void QMQTT::SslSocket::disconnectFromHost()
{
  _socket->disconnectFromHost();
}

QAbstractSocket::SocketState QMQTT::SslSocket::state() const
{
  return _socket->state();
}

QAbstractSocket::SocketError QMQTT::SslSocket::error() const
{
  return _socket->error();
}

void QMQTT::SslSocket::ignoreSslErrors(const QList<QSslError> &errors)
{
  _socket->ignoreSslErrors(errors);
}

void QMQTT::SslSocket::ignoreSslErrors()
{
  _socket->ignoreSslErrors();
}

QSslConfiguration QMQTT::SslSocket::sslConfiguration() const
{
  return _socket->sslConfiguration();
}

void QMQTT::SslSocket::setSslConfiguration(const QSslConfiguration &config)
{
  _socket->setSslConfiguration(config);
}

#endif // QT_NO_SSL
