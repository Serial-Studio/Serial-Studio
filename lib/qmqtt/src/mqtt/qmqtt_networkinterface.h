/*
 * qmqtt_networkinterface.h - qmqtt network interface header
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
#ifndef QMQTT_NETWORK_INTERFACE_H
#define QMQTT_NETWORK_INTERFACE_H

#include <qmqtt_global.h>

#include <QObject>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QString>
#include <QList>

#ifndef QT_NO_SSL
#  include <QSslConfiguration>
QT_FORWARD_DECLARE_CLASS(QSslError)
#endif // QT_NO_SSL

namespace QMQTT
{

class Frame;

class Q_MQTT_EXPORT NetworkInterface : public QObject
{
  Q_OBJECT
public:
  explicit NetworkInterface(QObject *parent = nullptr)
    : QObject(parent)
  {
  }
  virtual ~NetworkInterface() {}

  virtual void sendFrame(const Frame &frame) = 0;
  virtual bool isConnectedToHost() const = 0;
  virtual bool autoReconnect() const = 0;
  virtual void setAutoReconnect(const bool autoReconnect) = 0;
  virtual int autoReconnectInterval() const = 0;
  virtual void setAutoReconnectInterval(const int autoReconnectInterval) = 0;
  virtual QAbstractSocket::SocketState state() const = 0;
#ifndef QT_NO_SSL
  virtual void ignoreSslErrors(const QList<QSslError> &errors) = 0;
  virtual QSslConfiguration sslConfiguration() const = 0;
  virtual void setSslConfiguration(const QSslConfiguration &config) = 0;
#endif // QT_NO_SSL

public slots:
  virtual void connectToHost(const QHostAddress &host, const quint16 port) = 0;
  virtual void connectToHost(const QString &hostName, const quint16 port) = 0;
  virtual void disconnectFromHost() = 0;
#ifndef QT_NO_SSL
  virtual void ignoreSslErrors() = 0;
#endif // QT_NO_SSL

Q_SIGNALS:
  void connected();
  void disconnected();
  void received(const QMQTT::Frame &frame);
  void error(QAbstractSocket::SocketError error);
#ifndef QT_NO_SSL
  void sslErrors(const QList<QSslError> &errors);
#endif // QT_NO_SSL
};

} // namespace QMQTT

#endif // QMQTT_NETWORK_INTERFACE_H
