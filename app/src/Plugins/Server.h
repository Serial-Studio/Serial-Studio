/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QByteArray>
#include <QHostAddress>

#include "JSON/Frame.h"

/**
 * Default TCP port to use for incoming connections, I choose 7777 because 7 is
 * one of my favourite numbers :)
 */
#define PLUGINS_TCP_PORT 7777

namespace Plugins
{
/**
 * @brief The Server class
 *
 * Implements a simple TCP server on port 7777, which allows Serial Studio to
 * interact with other applications on the computer or in the LAN. "Plugins"
 * receive incoming frames processed by Serial Studio, and can write data to the
 * device by simply writting data on the TCP socket.
 *
 * An example of such application can be found at:
 *  https://github.com/Kaan-Sat/CC2021-Control-Panel
 *
 * A benefit of implementing plugins in this manner is that you can write your
 * Serial Studio companion application in any language and framework that you
 * desire, you do not have to force yourself to use Qt or C/C++.
 */
class Server : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

signals:
  void enabledChanged();

private:
  explicit Server();
  Server(Server &&) = delete;
  Server(const Server &) = delete;
  Server &operator=(Server &&) = delete;
  Server &operator=(const Server &) = delete;

  ~Server();

public:
  static Server &instance();
  [[nodiscard]] bool enabled() const;

public slots:
  void removeConnection();
  void setEnabled(const bool enabled);

private slots:
  void onDataReceived();
  void acceptConnection();
  void sendProcessedData();
  void sendRawData(const QByteArray &data);
  void registerFrame(const JSON::Frame &frame);
  void onErrorOccurred(const QAbstractSocket::SocketError socketError);

private:
  bool m_enabled;
  QTcpServer m_server;
  QVector<JSON::Frame> m_frames;
  QVector<QTcpSocket *> m_sockets;
};
} // namespace Plugins
