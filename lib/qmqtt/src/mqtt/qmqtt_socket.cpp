/*
 * qmqtt_socket.cpp - qmqtt socket
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

#include "qmqtt_socket_p.h"

#include <QTcpSocket>

QMQTT::Socket::Socket(QObject *parent)
  : SocketInterface(parent)
  , _socket(new QTcpSocket(this))
{
  connect(_socket.data(), &QTcpSocket::connected, this,
          &SocketInterface::connected);
  connect(_socket.data(), &QTcpSocket::disconnected, this,
          &SocketInterface::disconnected);
  connect(_socket.data(),
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
          static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(
              &QTcpSocket::errorOccurred),
#else
          static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(
              &QTcpSocket::error),
#endif
          this,
          static_cast<void (SocketInterface::*)(QAbstractSocket::SocketError)>(
              &SocketInterface::error));
}

QMQTT::Socket::~Socket() {}

QIODevice *QMQTT::Socket::ioDevice()
{
  return _socket.data();
}

void QMQTT::Socket::connectToHost(const QHostAddress &address, quint16 port)
{
  _socket->connectToHost(address, port);
}

void QMQTT::Socket::connectToHost(const QString &hostName, quint16 port)
{
  _socket->connectToHost(hostName, port);
}

void QMQTT::Socket::disconnectFromHost()
{
  _socket->disconnectFromHost();
}

QAbstractSocket::SocketState QMQTT::Socket::state() const
{
  return _socket->state();
}

QAbstractSocket::SocketError QMQTT::Socket::error() const
{
  return _socket->error();
}
