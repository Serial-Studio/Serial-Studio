/*
 * qmqtt_ssl_socket_p.h - qmqtt SSL socket private header
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
#ifndef QMQTT_SSL_SOCKET_P_H
#define QMQTT_SSL_SOCKET_P_H

#ifndef QT_NO_SSL

#  include <qmqtt_socketinterface.h>

#  include <QObject>
#  include <QHostAddress>
#  include <QString>
#  include <QList>
#  include <QScopedPointer>

QT_FORWARD_DECLARE_CLASS(QSslSocket)
QT_FORWARD_DECLARE_CLASS(QSslError)
#  include <QSslConfiguration>

namespace QMQTT
{

class SslSocket : public SocketInterface
{
  Q_OBJECT
public:
  explicit SslSocket(const QSslConfiguration &config,
                     QObject *parent = nullptr);
  virtual ~SslSocket();

  virtual QIODevice *ioDevice();
  void connectToHost(const QHostAddress &address, quint16 port);
  void connectToHost(const QString &hostName, quint16 port);
  void disconnectFromHost();
  QAbstractSocket::SocketState state() const;
  QAbstractSocket::SocketError error() const;
  void ignoreSslErrors(const QList<QSslError> &errors);
  void ignoreSslErrors();
  QSslConfiguration sslConfiguration() const;
  void setSslConfiguration(const QSslConfiguration &config);

protected:
  QScopedPointer<QSslSocket> _socket;
};

} // namespace QMQTT

#endif // QT_NO_SSL

#endif // QMQTT_SSL_SOCKET_P_H
