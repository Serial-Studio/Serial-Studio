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

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "IO/Manager.h"
#include "API/Server.h"
#include "Misc/Utilities.h"
#include "API/CommandHandler.h"

//------------------------------------------------------------------------------
// ServerWorker implementation
//------------------------------------------------------------------------------

/**
 * @brief Destructor
 */
API::ServerWorker::~ServerWorker() = default;

/**
 * @brief Returns false (no file resources to manage)
 */
bool API::ServerWorker::isResourceOpen() const
{
  return false;
}

/**
 * @brief Closes all sockets and cleans up resources
 */
void API::ServerWorker::closeResources()
{
  for (auto *socket : std::as_const(m_sockets))
  {
    if (socket)
    {
      socket->abort();
      socket->deleteLater();
    }
  }

  m_sockets.clear();

  Q_EMIT clientCountChanged(0);
}

/**
 * @brief Adds a socket to the worker thread
 *
 * The socket is moved to this thread and managed here for all I/O operations.
 */
void API::ServerWorker::addSocket(QTcpSocket *socket)
{
  if (!socket)
    return;

  m_sockets.append(socket);
  connect(socket, &QTcpSocket::readyRead, this,
          &ServerWorker::onSocketReadyRead);
  connect(socket, &QTcpSocket::disconnected, this,
          &ServerWorker::onSocketDisconnected);

  Q_EMIT clientCountChanged(m_sockets.count());
}

/**
 * @brief Removes a socket from the worker thread
 */
void API::ServerWorker::removeSocket(QTcpSocket *socket)
{
  m_sockets.removeAll(socket);
  if (socket)
    socket->deleteLater();

  Q_EMIT clientCountChanged(m_sockets.count());
}

/**
 * @brief Writes raw data to all connected sockets (worker thread)
 *
 * Handles transmission of raw I/O data to API clients.
 * The data is base64-encoded and wrapped in JSON format.
 */
void API::ServerWorker::writeRawData(const IO::ByteArrayPtr &data)
{
  if (m_sockets.isEmpty())
    return;

  QJsonObject object;
  object.insert(QStringLiteral("data"), QString::fromUtf8(data->toBase64()));
  const QJsonDocument document(object);
  const auto json = document.toJson(QJsonDocument::Compact) + "\n";

  for (auto *socket : std::as_const(m_sockets))
  {
    if (socket && socket->isWritable())
      socket->write(json);
  }
}

/**
 * @brief Handles incoming data from a socket (worker thread)
 */
void API::ServerWorker::onSocketReadyRead()
{
  auto *socket = qobject_cast<QTcpSocket *>(sender());
  if (socket)
    Q_EMIT dataReceived(socket, socket->readAll());
}

/**
 * @brief Writes data to a specific socket (worker thread)
 *
 * Used to send API command responses back to the requesting client.
 *
 * @param socket The socket to write to
 * @param data The data to write
 */
void API::ServerWorker::writeToSocket(QTcpSocket *socket,
                                      const QByteArray &data)
{
  if (socket && socket->isWritable() && m_sockets.contains(socket))
    socket->write(data);
}

/**
 * @brief Handles socket disconnection (worker thread)
 */
void API::ServerWorker::onSocketDisconnected()
{
  auto *socket = qobject_cast<QTcpSocket *>(sender());
  removeSocket(socket);
}

/**
 * @brief Processes frames by serializing them to JSON and writing to sockets
 *
 * Both JSON serialization and socket writes happen on the worker thread,
 * eliminating cross-thread communication overhead for high-frequency writes.
 */
void API::ServerWorker::processItems(
    const std::vector<DataModel::TimestampedFramePtr> &items)
{
  if (items.empty() || m_sockets.isEmpty())
    return;

  QJsonArray array;
  for (const auto &timestampedFrame : items)
  {
    QJsonObject object;
    object.insert(QStringLiteral("data"), serialize(timestampedFrame->data));
    array.append(object);
  }

  QJsonObject object;
  object.insert(QStringLiteral("frames"), array);
  const QJsonDocument document(object);
  const auto json = document.toJson(QJsonDocument::Compact) + "\n";

  for (auto *socket : std::as_const(m_sockets))
  {
    if (socket && socket->isWritable())
      socket->write(json);
  }
}

//------------------------------------------------------------------------------
// Server implementation
//------------------------------------------------------------------------------

/**
 * @brief Constructs the API server.
 *
 * Initializes the worker thread for JSON serialization and socket I/O,
 * and prepares the TCP server for later activation via setEnabled().
 */
API::Server::Server()
  : DataModel::FrameConsumer<DataModel::TimestampedFramePtr>(
        {.queueCapacity = 2048, .flushThreshold = 512, .timerIntervalMs = 1000})
  , m_clientCount(0)
  , m_enabled(false)
{
  initializeWorker();

  auto *worker = static_cast<ServerWorker *>(m_worker);
  connect(worker, &ServerWorker::dataReceived, this, &Server::onDataReceived,
          Qt::QueuedConnection);
  connect(worker, &ServerWorker::clientCountChanged, this,
          &Server::onClientCountChanged, Qt::QueuedConnection);

  connect(&m_server, &QTcpServer::newConnection, this,
          &Server::acceptConnection);

  // Initialize API command handlers
  API::CommandHandler::instance();
}

/**
 * @brief Destroys the API server.
 *
 * Shuts down the TCP server and closes all existing connections.
 */
API::Server::~Server()
{
  m_server.close();
}

/**
 * @brief Creates the server worker instance.
 */
DataModel::FrameConsumerWorkerBase *API::Server::createWorker()
{
  return new ServerWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
}

/**
 * @brief Gets the singleton instance of the API server.
 *
 * @return Reference to the only instance of API::Server.
 */
API::Server &API::Server::instance()
{
  static Server singleton;
  return singleton;
}

/**
 * @brief Checks whether the API server is currently enabled.
 *
 * @return true if the API subsystem is active and serving connections;
 *         false otherwise.
 */
bool API::Server::enabled() const
{
  return m_enabled;
}

/**
 * @brief Gets the number of currently connected API clients.
 *
 * @return The count of active client connections.
 */
int API::Server::clientCount() const
{
  return m_clientCount;
}

/**
 * @brief Disconnects a client socket from the server.
 *
 * Forwards the removal request to the worker thread.
 * Triggered automatically when a client disconnects or encounters an error.
 */
void API::Server::removeConnection()
{
  auto *socket = qobject_cast<QTcpSocket *>(sender());
  if (socket)
  {
    auto *worker = static_cast<ServerWorker *>(m_worker);
    QMetaObject::invokeMethod(worker, "removeSocket", Qt::QueuedConnection,
                              Q_ARG(QTcpSocket *, socket));
  }
}

/**
 * @brief Enables or disables the TCP API server.
 *
 * When enabling, starts listening for incoming TCP connections.
 * When disabling, stops the server, closes all sockets, and clears
 * frame buffers.
 *
 * @param enabled If true, activates the server. If false, deactivates it.
 */
void API::Server::setEnabled(const bool enabled)
{
  m_enabled = enabled;
  Q_EMIT enabledChanged();

  // Enable the TCP API server
  if (enabled)
  {
    if (!m_server.isListening())
    {
      if (!m_server.listen(QHostAddress::Any, API_TCP_PORT))
      {
        Misc::Utilities::showMessageBox(tr("Unable to start API TCP server"),
                                        m_server.errorString(),
                                        QMessageBox::Warning);
        m_server.close();
      }
    }
  }

  // Disable the TCP API server
  else
  {
    m_server.close();

    auto *worker = static_cast<ServerWorker *>(m_worker);
    QMetaObject::invokeMethod(worker, "closeResources", Qt::QueuedConnection);
  }
}

/**
 * @brief Sends raw binary data to all connected clients.
 *
 * Forwards raw I/O data to the worker thread for transmission to API
 * clients. The data will be base64-encoded and wrapped in JSON format.
 *
 * @param data Raw data bytes received from the I/O layer.
 */
void API::Server::hotpathTxData(const IO::ByteArrayPtr &data)
{
  if (!enabled())
    return;

  auto *worker = static_cast<ServerWorker *>(m_worker);
  QMetaObject::invokeMethod(worker, "writeRawData", Qt::QueuedConnection,
                            Q_ARG(IO::ByteArrayPtr, data));
}

/**
 * @brief Registers a new structured data frame.
 *
 * Enqueues the timestamped frame for background JSON serialization.
 * The serialized data will be transmitted to all connected API clients.
 *
 * @param frame Timestamped frame object to register.
 */
void API::Server::hotpathTxFrame(const DataModel::TimestampedFramePtr &frame)
{
  if (enabled())
    enqueueData(frame);
}

/**
 * @brief Handles incoming data from worker thread.
 *
 * Receives data from the worker thread (which read it from a socket).
 * If the data is an API command, processes it and sends response back.
 * Otherwise, forwards raw data to the I/O manager for device transmission.
 *
 * @param socket The socket that sent the data (for sending responses)
 * @param data The received data
 */
void API::Server::onDataReceived(QTcpSocket *socket, const QByteArray &data)
{
  if (!enabled() || data.isEmpty())
    return;

  // Check if this looks like a JSON message attempt (starts with '{')
  const auto trimmed = data.trimmed();
  const bool looksLikeJson = !trimmed.isEmpty() && trimmed.at(0) == '{';

  // If it looks like JSON, always process as API message
  // This ensures proper error responses for malformed API messages
  if (looksLikeJson)
  {
    auto &cmdHandler = API::CommandHandler::instance();
    const QByteArray response = cmdHandler.processMessage(data);
    auto *worker = static_cast<ServerWorker *>(m_worker);
    QMetaObject::invokeMethod(worker, "writeToSocket", Qt::QueuedConnection,
                              Q_ARG(QTcpSocket *, socket),
                              Q_ARG(QByteArray, response));
    return;
  }

  // Not an API command - forward to I/O device (existing behavior)
  IO::Manager::instance().writeData(data);
}

/**
 * @brief Accepts new incoming TCP connections.
 *
 * Creates the socket on the main thread, then moves it to the worker thread
 * for all I/O operations.
 */
void API::Server::acceptConnection()
{
  auto *socket = m_server.nextPendingConnection();
  if (!socket)
  {
    if (enabled())
      Misc::Utilities::showMessageBox(tr("API server"),
                                      tr("Invalid pending connection"),
                                      QMessageBox::Critical);
    return;
  }

  if (!enabled())
  {
    socket->close();
    socket->deleteLater();
    return;
  }

  connect(socket, &QTcpSocket::errorOccurred, this, &Server::onErrorOccurred);

  socket->setParent(nullptr);
  socket->moveToThread(&m_workerThread);

  auto *worker = static_cast<ServerWorker *>(m_worker);
  QMetaObject::invokeMethod(worker, "addSocket", Qt::QueuedConnection,
                            Q_ARG(QTcpSocket *, socket));
}

/**
 * @brief Handles socket-level errors from connected clients.
 *
 * Logs the error to debug output and attempts to cleanly disconnect
 * the faulty socket.
 *
 * @param socketError Error code provided by the QAbstractSocket.
 */
void API::Server::onErrorOccurred(
    const QAbstractSocket::SocketError socketError)
{
  auto socket = static_cast<QTcpSocket *>(QObject::sender());
  if (socket)
    qDebug() << socket->errorString();
  else
    qDebug() << socketError;
}

/**
 * @brief Updates the client count from the worker thread.
 *
 * Called when the worker thread reports a change in the number of connected
 * clients. Updates the local count and emits the clientCountChanged signal.
 *
 * @param count New number of connected clients
 */
void API::Server::onClientCountChanged(int count)
{
  if (m_clientCount != count)
  {
    m_clientCount = count;
    Q_EMIT clientCountChanged();
  }
}
