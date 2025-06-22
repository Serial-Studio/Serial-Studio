/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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
