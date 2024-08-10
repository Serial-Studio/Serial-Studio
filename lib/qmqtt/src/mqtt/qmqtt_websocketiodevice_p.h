/*
 * qmqtt_socketinterface.h - qmqtt socket interface header
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

#ifndef QMQTT_WEBSOCKETIODEVICE_H
#define QMQTT_WEBSOCKETIODEVICE_H

#ifdef QT_WEBSOCKETS_LIB

#  include <QByteArray>
#  include <QIODevice>

QT_FORWARD_DECLARE_CLASS(QWebSocket)
QT_FORWARD_DECLARE_CLASS(QNetworkRequest)

namespace QMQTT
{

class WebSocketIODevice : public QIODevice
{
  Q_OBJECT
public:
  explicit WebSocketIODevice(QWebSocket *socket, QObject *parent = nullptr);

  bool connectToHost(const QNetworkRequest &request);

  virtual qint64 bytesAvailable() const;

protected:
  virtual qint64 readData(char *data, qint64 maxSize);

  virtual qint64 writeData(const char *data, qint64 maxSize);

private Q_SLOTS:
  void binaryMessageReceived(const QByteArray &frame);

private:
  QByteArray _buffer;
  QWebSocket *_webSocket;
};

} // namespace QMQTT

#endif // QT_WEBSOCKETS_LIB

#endif // QMQTT_WEBSOCKETIODEVICE_H
