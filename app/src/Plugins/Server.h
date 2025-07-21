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
#include "ThirdParty/readerwriterqueue.h"

/**
 * Default TCP port to use for incoming connections, I choose 7777 because 7 is
 * one of my favourite numbers :)
 */
#define PLUGINS_TCP_PORT 7777

namespace Plugins
{
/**
 * @class Plugins::Server
 * @brief TCP server interface for plugin communication in Serial Studio.
 *
 * Implements a TCP server that listens on port 7777, allowing external
 * applications (referred to as "plugins") to exchange data with Serial Studio
 * over the local network or localhost.
 *
 * Connected plugins can:
 * - Receive real-time JSON data frames processed by Serial Studio.
 * - Transmit raw data directly to the underlying I/O device via the TCP socket.
 *
 * This design enables companion applications to be written in any language or
 * framework, without requiring integration with Qt or C++.
 *
 * The server supports multiple simultaneous plugin connections and handles
 * connection management, data serialization, and dispatch internally. It can
 * be enabled or disabled at runtime using setEnabled(), and will only accept
 * connections when enabled.
 *
 * Example plugin: https://github.com/Kaan-Sat/CC2021-Control-Panel
 *
 * @note Accessed as a singleton via Plugins::Server::instance().
 * @note Not thread-safe; must be used from the main Qt thread.
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
  void hotpathTxData(const QByteArray &data);
  void hotpathTxFrame(const JSON::Frame &frame);

private slots:
  void onDataReceived();
  void acceptConnection();
  void sendProcessedData();
  void onErrorOccurred(const QAbstractSocket::SocketError socketError);

private:
  bool m_enabled;
  QTcpServer m_server;
  QVector<QTcpSocket *> m_sockets;
  moodycamel::ReaderWriterQueue<JSON::Frame> m_pendingFrames{2048};
};
} // namespace Plugins
