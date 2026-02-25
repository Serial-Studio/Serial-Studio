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

#include <QByteArray>
#include <QElapsedTimer>
#include <QHash>
#include <QHostAddress>
#include <QObject>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>

#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"
#include "IO/HAL_Driver.h"

/**
 * Default TCP port to use for incoming connections, I choose 7777 because 7 is
 * one of my favourite numbers :)
 */
#define API_TCP_PORT 7777

namespace API {
class Server;

/**
 * @brief Worker that handles JSON serialization and socket I/O on background
 * thread
 */
class ServerWorker : public DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr> {
  Q_OBJECT

public:
  using DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr>::FrameConsumerWorker;
  ~ServerWorker() override;

  bool isResourceOpen() const override;

public slots:
  void closeResources() override;
  void addSocket(QTcpSocket* socket);
  void removeSocket(QTcpSocket* socket);
  void writeRawData(const IO::ByteArrayPtr& data);
  void writeToSocket(QTcpSocket* socket, const QByteArray& data);
  void disconnectSocket(QTcpSocket* socket);

signals:
  void dataReceived(QTcpSocket* socket, const QByteArray& data);
  void clientCountChanged(int count);
  void socketRemoved(QTcpSocket* socket);

protected:
  void processItems(const std::vector<DataModel::TimestampedFramePtr>& items) override;

private slots:
  void onSocketReadyRead();
  void onSocketDisconnected();

private:
  QVector<QTcpSocket*> m_sockets;
};

/**
 * @class API::Server
 * @brief TCP server interface for API communication in Serial Studio.
 *
 * Implements a TCP server that listens on port 7777, allowing external
 * applications to control Serial Studio and exchange data with it
 * over the local network or localhost.
 *
 * Connected clients can:
 * - Receive real-time JSON data frames processed by Serial Studio.
 * - Send API commands to control Serial Studio (configure devices, start/stop
 *   connections, etc.)
 * - Transmit raw data directly to the underlying I/O device via the TCP socket.
 *
 * This design enables companion applications to be written in any language or
 * framework, without requiring integration with Qt or C++.
 *
 * The server supports multiple simultaneous client connections and handles
 * connection management, data serialization, and dispatch internally. It can
 * be enabled or disabled at runtime using setEnabled(), and will only accept
 * connections when enabled.
 *
 * **Performance:** JSON serialization and socket I/O are performed on a
 * background worker thread to prevent blocking the main UI thread. This
 * eliminates cross-thread communication overhead for high-frequency frame
 * transmission (writes >> reads in typical usage).
 *
 * Example client: https://github.com/Kaan-Sat/CC2021-Control-Panel
 *
 * @note Accessed as a singleton via API::Server::instance().
 * @note Socket management must be on the main Qt thread.
 */
class Server : public DataModel::FrameConsumer<DataModel::TimestampedFramePtr> {
  Q_OBJECT
  Q_PROPERTY(int clientCount READ clientCount NOTIFY clientCountChanged)
  Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
  Q_PROPERTY(bool externalConnections READ externalConnections WRITE setExternalConnections NOTIFY
               externalConnectionsChanged)

signals:
  void enabledChanged();
  void clientCountChanged();
  void externalConnectionsChanged();

private:
  explicit Server();
  Server(Server&&)                 = delete;
  Server(const Server&)            = delete;
  Server& operator=(Server&&)      = delete;
  Server& operator=(const Server&) = delete;

  ~Server();

public:
  static Server& instance();
  [[nodiscard]] bool enabled() const;
  [[nodiscard]] bool externalConnections() const;
  [[nodiscard]] int clientCount() const;

public slots:
  void removeConnection();
  void setEnabled(const bool enabled);
  void setExternalConnections(const bool enabled);
  void hotpathTxData(const IO::ByteArrayPtr& data);
  void hotpathTxFrame(const DataModel::TimestampedFramePtr& frame);

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;

private slots:
  void acceptConnection();
  void onSocketDisconnected();
  void onClientCountChanged(int count);
  void onSocketDisconnected(QTcpSocket* socket);
  void onDataReceived(QTcpSocket* socket, const QByteArray& data);
  void onErrorOccurred(const QAbstractSocket::SocketError socketError);

private:
  struct ConnectionState {
    QString peerAddress;
    quint16 peerPort = 0;
    QByteArray buffer;
    QElapsedTimer window;
    int messageCount = 0;
    int byteCount    = 0;
  };

  QSettings m_settings;
  int m_clientCount;
  bool m_enabled;
  bool m_externalConnections;
  QTcpServer m_server;
  QHash<QTcpSocket*, ConnectionState> m_connections;
};
}  // namespace API
