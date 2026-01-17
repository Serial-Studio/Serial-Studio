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

#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"

/**
 * Default TCP port to use for incoming connections, I choose 7777 because 7 is
 * one of my favourite numbers :)
 */
#define PLUGINS_TCP_PORT 7777

namespace Plugins
{
class Server;

/**
 * @brief Worker that handles JSON serialization and socket I/O on background
 * thread
 */
class ServerWorker
  : public DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr>
{
  Q_OBJECT

public:
  using DataModel::FrameConsumerWorker<
      DataModel::TimestampedFramePtr>::FrameConsumerWorker;
  ~ServerWorker() override;

  void closeResources() override;
  bool isResourceOpen() const override;

public slots:
  void addSocket(QTcpSocket *socket);
  void removeSocket(QTcpSocket *socket);
  void writeRawData(const IO::ByteArrayPtr &data);
  void writeToSocket(QTcpSocket *socket, const QByteArray &data);

signals:
  void dataReceived(QTcpSocket *socket, const QByteArray &data);

protected:
  void processItems(
      const std::vector<DataModel::TimestampedFramePtr> &items) override;

private slots:
  void onSocketReadyRead();
  void onSocketDisconnected();

private:
  QVector<QTcpSocket *> m_sockets;
};

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
 * **Performance:** JSON serialization and socket I/O are performed on a
 * background worker thread to prevent blocking the main UI thread. This
 * eliminates cross-thread communication overhead for high-frequency frame
 * transmission (writes >> reads in typical plugin usage).
 *
 * Example plugin: https://github.com/Kaan-Sat/CC2021-Control-Panel
 *
 * @note Accessed as a singleton via Plugins::Server::instance().
 * @note Socket management must be on the main Qt thread.
 */
class Server : public DataModel::FrameConsumer<DataModel::TimestampedFramePtr>
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
  void hotpathTxData(const IO::ByteArrayPtr &data);
  void hotpathTxFrame(const DataModel::TimestampedFramePtr &frame);

protected:
  DataModel::FrameConsumerWorkerBase *createWorker() override;

private slots:
  void acceptConnection();
  void onDataReceived(QTcpSocket *socket, const QByteArray &data);
  void onErrorOccurred(const QAbstractSocket::SocketError socketError);

private:
  bool m_enabled;
  QTcpServer m_server;
};
} // namespace Plugins
