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

#include "API/Server.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "API/CommandHandler.h"
#include "API/CommandProtocol.h"
#include "API/MCPHandler.h"
#include "API/MCPProtocol.h"
#include "IO/Manager.h"
#include "Misc/Utilities.h"

namespace {
constexpr int kMaxApiClients           = 32;
constexpr int kApiWindowMs             = 1000;
constexpr int kMaxApiJsonDepth         = 64;
constexpr int kMaxApiRawBytes          = 1024 * 1024;
constexpr int kMaxApiMessagesPerWindow = 200;
constexpr int kMaxApiMessageBytes      = 1024 * 1024;
constexpr int kMaxApiBufferBytes       = 4 * 1024 * 1024;
constexpr int kMaxApiBytesPerWindow    = 128 * 1024 * 1024;

bool exceedsJsonDepthLimit(const QByteArray& data, int maxDepth)
{
  int depth     = 0;
  bool inString = false;
  bool escaped  = false;

  for (const auto byte : data) {
    const char ch = static_cast<char>(byte);

    if (inString) {
      if (escaped) {
        escaped = false;
        continue;
      }

      if (ch == '\\') {
        escaped = true;
        continue;
      }

      if (ch == '"')
        inString = false;

      continue;
    }

    if (ch == '"') {
      inString = true;
      continue;
    }

    if (ch == '{' || ch == '[') {
      ++depth;
      if (depth > maxDepth)
        return true;
    } else if (ch == '}' || ch == ']') {
      if (depth > 0)
        --depth;
    }
  }

  return false;
}
}  // namespace

//--------------------------------------------------------------------------------------------------
// ServerWorker implementation
//--------------------------------------------------------------------------------------------------

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
  for (auto* socket : std::as_const(m_sockets)) {
    if (socket) {
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
void API::ServerWorker::addSocket(QTcpSocket* socket)
{
  if (!socket)
    return;

  m_sockets.append(socket);
  connect(socket, &QTcpSocket::readyRead, this, &ServerWorker::onSocketReadyRead);
  connect(socket, &QTcpSocket::disconnected, this, &ServerWorker::onSocketDisconnected);

  Q_EMIT clientCountChanged(m_sockets.count());
}

/**
 * @brief Removes a socket from the worker thread
 */
void API::ServerWorker::removeSocket(QTcpSocket* socket)
{
  m_sockets.removeAll(socket);

  Q_EMIT socketRemoved(socket);
  Q_EMIT clientCountChanged(m_sockets.count());

  if (socket)
    socket->deleteLater();
}

/**
 * @brief Writes raw data to all connected sockets (worker thread)
 *
 * Handles transmission of raw I/O data to API clients.
 * The data is base64-encoded and wrapped in JSON format.
 */
void API::ServerWorker::writeRawData(const IO::ByteArrayPtr& data)
{
  if (m_sockets.isEmpty())
    return;

  QJsonObject object;
  object.insert(QStringLiteral("data"), QString::fromUtf8(data->toBase64()));
  const QJsonDocument document(object);
  const auto json = document.toJson(QJsonDocument::Compact) + "\n";

  for (auto* socket : std::as_const(m_sockets))
    if (socket && socket->isWritable())
      socket->write(json);
}

/**
 * @brief Handles incoming data from a socket (worker thread)
 */
void API::ServerWorker::onSocketReadyRead()
{
  auto* socket = qobject_cast<QTcpSocket*>(sender());
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
void API::ServerWorker::writeToSocket(QTcpSocket* socket, const QByteArray& data)
{
  if (socket && m_sockets.contains(socket) && socket->isWritable())
    socket->write(data);
}

/**
 * @brief Disconnects a socket (worker thread)
 *
 * Ensures the disconnect happens after any queued writes are sent.
 *
 * @param socket The socket to disconnect
 */
void API::ServerWorker::disconnectSocket(QTcpSocket* socket)
{
  if (socket && m_sockets.contains(socket)) {
    socket->flush();
    socket->disconnectFromHost();
  }
}

/**
 * @brief Handles socket disconnection (worker thread)
 */
void API::ServerWorker::onSocketDisconnected()
{
  auto* socket = qobject_cast<QTcpSocket*>(sender());
  removeSocket(socket);
}

/**
 * @brief Processes frames by serializing them to JSON and writing to sockets
 *
 * Both JSON serialization and socket writes happen on the worker thread,
 * eliminating cross-thread communication overhead for high-frequency writes.
 */
void API::ServerWorker::processItems(const std::vector<DataModel::TimestampedFramePtr>& items)
{
  if (items.empty() || m_sockets.isEmpty())
    return;

  QJsonArray array;
  for (const auto& timestampedFrame : items) {
    QJsonObject object;
    object.insert(QStringLiteral("data"), serialize(timestampedFrame->data));
    array.append(object);
  }

  QJsonObject object;
  object.insert(QStringLiteral("frames"), array);
  const QJsonDocument document(object);
  const auto json = document.toJson(QJsonDocument::Compact) + "\n";

  for (auto* socket : std::as_const(m_sockets))
    if (socket && socket->isWritable())
      socket->write(json);
}

//--------------------------------------------------------------------------------------------------
// Server implementation
//--------------------------------------------------------------------------------------------------

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
  , m_externalConnections(false)
{
  m_externalConnections = m_settings.value("API/ExternalConnections", false).toBool();

  initializeWorker();

  auto* worker = static_cast<ServerWorker*>(m_worker);
  connect(worker, &ServerWorker::dataReceived, this, &Server::onDataReceived, Qt::QueuedConnection);
  connect(worker,
          &ServerWorker::clientCountChanged,
          this,
          &Server::onClientCountChanged,
          Qt::QueuedConnection);
  connect(worker,
          &ServerWorker::socketRemoved,
          this,
          qOverload<QTcpSocket*>(&Server::onSocketDisconnected),
          Qt::QueuedConnection);

  connect(&m_server, &QTcpServer::newConnection, this, &Server::acceptConnection);

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
DataModel::FrameConsumerWorkerBase* API::Server::createWorker()
{
  return new ServerWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
}

/**
 * @brief Gets the singleton instance of the API server.
 *
 * @return Reference to the only instance of API::Server.
 */
API::Server& API::Server::instance()
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
 * @brief Returns whether the server accepts connections from external hosts.
 * @return true if listening on all interfaces; false if localhost only.
 */
bool API::Server::externalConnections() const
{
  return m_externalConnections;
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
  auto* socket = qobject_cast<QTcpSocket*>(sender());
  if (socket) {
    auto* worker = static_cast<ServerWorker*>(m_worker);
    QMetaObject::invokeMethod(
      worker, "removeSocket", Qt::QueuedConnection, Q_ARG(QTcpSocket*, socket));
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
  bool effectiveEnabled = enabled;
  bool closeResources   = false;

  // Enable the TCP API server
  if (enabled) {
    if (!m_server.isListening()) {
      m_server.setMaxPendingConnections(kMaxApiClients);
      const auto address = m_externalConnections ? QHostAddress::Any : QHostAddress::LocalHost;
      if (!m_server.listen(address, API_TCP_PORT)) {
        Misc::Utilities::showMessageBox(
          tr("Unable to start API TCP server"), m_server.errorString(), QMessageBox::Warning);
        m_server.close();
        effectiveEnabled = false;
        closeResources   = true;
      }
    }
  }

  // Disable the TCP API server
  else {
    m_server.close();
    closeResources = true;

    if (m_externalConnections) {
      m_externalConnections = false;
      m_settings.setValue("API/ExternalConnections", false);
      Q_EMIT externalConnectionsChanged();
    }
  }

  if (closeResources) {
    m_connections.clear();
    auto* worker = static_cast<ServerWorker*>(m_worker);
    QMetaObject::invokeMethod(worker, "closeResources", Qt::QueuedConnection);
  }

  if (m_enabled != effectiveEnabled) {
    m_enabled = effectiveEnabled;
    Q_EMIT enabledChanged();
  } else if (enabled != effectiveEnabled)
    Q_EMIT enabledChanged();
}

/**
 * @brief Sets whether the server accepts connections from external hosts.
 *
 * When enabled, the server listens on all network interfaces (QHostAddress::Any).
 * When disabled, it binds to localhost only. If the server is currently running,
 * it is restarted to apply the change immediately.
 *
 * @param enabled If true, accept external connections; if false, localhost only.
 */
void API::Server::setExternalConnections(const bool enabled)
{
  if (m_externalConnections == enabled)
    return;

  if (enabled) {
    const int result = Misc::Utilities::showMessageBox(
      tr("Allow External API Connections?"),
      tr("Exposing the API server to external hosts allows other devices on your "
         "network to connect to Serial Studio on port 7777.\n\n"
         "Only enable this on trusted networks. "
         "Untrusted clients may read live data or send commands to your device."),
      QMessageBox::Warning,
      QString(),
      QMessageBox::Yes | QMessageBox::No,
      QMessageBox::No);

    if (result == QMessageBox::No) {
      m_externalConnections = false;
      m_settings.setValue("API/ExternalConnections", false);
      Q_EMIT externalConnectionsChanged();
      return;
    }
  }

  m_externalConnections = enabled;
  m_settings.setValue("API/ExternalConnections", m_externalConnections);
  Q_EMIT externalConnectionsChanged();

  if (m_enabled) {
    m_server.close();
    m_connections.clear();
    auto* worker = static_cast<ServerWorker*>(m_worker);
    QMetaObject::invokeMethod(worker, "closeResources", Qt::QueuedConnection);

    const auto address = m_externalConnections ? QHostAddress::Any : QHostAddress::LocalHost;
    m_server.setMaxPendingConnections(kMaxApiClients);
    if (!m_server.listen(address, API_TCP_PORT)) {
      Misc::Utilities::showMessageBox(
        tr("Unable to restart API TCP server"), m_server.errorString(), QMessageBox::Warning);
      m_server.close();
      m_enabled = false;
      Q_EMIT enabledChanged();
    }
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
void API::Server::hotpathTxData(const IO::ByteArrayPtr& data)
{
  if (!enabled())
    return;

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(
    worker, "writeRawData", Qt::QueuedConnection, Q_ARG(IO::ByteArrayPtr, data));
}

/**
 * @brief Registers a new structured data frame.
 *
 * Enqueues the timestamped frame for background JSON serialization.
 * The serialized data will be transmitted to all connected API clients.
 *
 * @param frame Timestamped frame object to register.
 */
void API::Server::hotpathTxFrame(const DataModel::TimestampedFramePtr& frame)
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
void API::Server::onDataReceived(QTcpSocket* socket, const QByteArray& data)
{
  if (!enabled() || data.isEmpty() || !socket)
    return;

  if (!m_connections.contains(socket))
    return;

  auto& state = m_connections[socket];
  if (!state.window.isValid())
    state.window.start();

  if (state.window.elapsed() > kApiWindowMs) {
    state.window.restart();
    state.messageCount = 0;
    state.byteCount    = 0;
  }

  state.byteCount += data.size();
  if (state.byteCount > kMaxApiBytesPerWindow) {
    qWarning() << "[API] Byte rate limit exceeded:" << state.peerAddress << ":" << state.peerPort
               << "- Bytes in window:" << state.byteCount << "- Limit:" << kMaxApiBytesPerWindow
               << "- Disconnecting client";

    auto* worker = static_cast<ServerWorker*>(m_worker);
    const QByteArray response =
      CommandResponse::makeError(
        QString(), ErrorCode::ExecutionError, QStringLiteral("API rate limit exceeded"))
        .toJsonBytes();
    QMetaObject::invokeMethod(worker,
                              "writeToSocket",
                              Qt::QueuedConnection,
                              Q_ARG(QTcpSocket*, socket),
                              Q_ARG(QByteArray, response));
    QMetaObject::invokeMethod(
      worker, "disconnectSocket", Qt::QueuedConnection, Q_ARG(QTcpSocket*, socket));
    state.buffer.clear();
    return;
  }

  // Check buffer size BEFORE appending to prevent allocation crashes
  if (state.buffer.size() + data.size() > kMaxApiBufferBytes) {
    qWarning() << "[API] Buffer size limit exceeded:" << state.peerAddress << ":" << state.peerPort
               << "- Buffer size:" << state.buffer.size() << "- Incoming data:" << data.size()
               << "- Limit:" << kMaxApiBufferBytes << "- Disconnecting client";

    auto* worker = static_cast<ServerWorker*>(m_worker);
    const QByteArray response =
      CommandResponse::makeError(
        QString(), ErrorCode::ExecutionError, QStringLiteral("API buffer limit exceeded"))
        .toJsonBytes();
    QMetaObject::invokeMethod(worker,
                              "writeToSocket",
                              Qt::QueuedConnection,
                              Q_ARG(QTcpSocket*, socket),
                              Q_ARG(QByteArray, response));
    QMetaObject::invokeMethod(
      worker, "disconnectSocket", Qt::QueuedConnection, Q_ARG(QTcpSocket*, socket));
    state.buffer.clear();
    return;
  }

  // Now safe to append - buffer won't exceed limit
  state.buffer.append(data);

  auto* worker      = static_cast<ServerWorker*>(m_worker);
  auto sendResponse = [&](const QByteArray& response) {
    QMetaObject::invokeMethod(worker,
                              "writeToSocket",
                              Qt::QueuedConnection,
                              Q_ARG(QTcpSocket*, socket),
                              Q_ARG(QByteArray, response));
  };

  auto rejectOversize = [&](int messageSize) {
    qWarning() << "[API] Message size limit exceeded:" << state.peerAddress << ":" << state.peerPort
               << "- Message size:" << messageSize << "- Limit:" << kMaxApiMessageBytes;

    sendResponse(CommandResponse::makeError(QString(),
                                            ErrorCode::ExecutionError,
                                            QStringLiteral("API message exceeds size limit"))
                   .toJsonBytes());
  };

  auto handleJson = [&](const QByteArray& jsonBytes) {
    if (jsonBytes.size() > kMaxApiMessageBytes) {
      rejectOversize(jsonBytes.size());
      return;
    }

    if (exceedsJsonDepthLimit(jsonBytes, kMaxApiJsonDepth)) {
      qWarning() << "[API] JSON depth limit exceeded:" << state.peerAddress << ":" << state.peerPort
                 << "- Max depth:" << kMaxApiJsonDepth;

      sendResponse(CommandResponse::makeError(QString(),
                                              ErrorCode::ExecutionError,
                                              QStringLiteral("JSON nesting depth exceeds limit"))
                     .toJsonBytes());
      return;
    }

    if (state.messageCount >= kMaxApiMessagesPerWindow) {
      qWarning() << "[API] Message rate limit exceeded:" << state.peerAddress << ":"
                 << state.peerPort << "- Messages in window:" << state.messageCount
                 << "- Limit:" << kMaxApiMessagesPerWindow << "- Disconnecting client";

      sendResponse(CommandResponse::makeError(QString(),
                                              ErrorCode::ExecutionError,
                                              QStringLiteral("API rate limit exceeded"))
                     .toJsonBytes());
      QMetaObject::invokeMethod(
        worker, "disconnectSocket", Qt::QueuedConnection, Q_ARG(QTcpSocket*, socket));
      state.buffer.clear();
      return;
    }
    ++state.messageCount;

    // Check if this is an MCP (Model Context Protocol) message
    if (MCP::isMCPMessage(jsonBytes)) {
      auto& mcpHandler        = API::MCPHandler::instance();
      const QString sessionId = QString::number(reinterpret_cast<quintptr>(socket));
      const auto response     = mcpHandler.processMessage(jsonBytes, sessionId);

      if (!response.isEmpty())
        sendResponse(response);

      return;
    }

    QString type;
    QJsonObject json;

    // Wrap JSON parsing in try-catch to prevent crashes from malformed data
    try {
      if (!API::parseMessage(jsonBytes, type, json)) {
        sendResponse(CommandResponse::makeError(QString(),
                                                ErrorCode::InvalidJson,
                                                QStringLiteral("Failed to parse JSON message"))
                       .toJsonBytes());
        return;
      }
    } catch (...) {
      qWarning() << "[API] JSON parsing exception:" << state.peerAddress << ":" << state.peerPort
                 << "- Message size:" << jsonBytes.size()
                 << "- Disconnecting client (malformed or too deep JSON)";

      // Catch any exception from Qt's JSON parser (stack overflow, etc.)
      sendResponse(
        CommandResponse::makeError(QString(),
                                   ErrorCode::InvalidJson,
                                   QStringLiteral("JSON parsing failed (malformed or too deep)"))
          .toJsonBytes());
      QMetaObject::invokeMethod(
        worker, "disconnectSocket", Qt::QueuedConnection, Q_ARG(QTcpSocket*, socket));
      state.buffer.clear();
      return;
    }

    if (type == MessageType::Raw) {
      const QString id = json.value(QStringLiteral("id")).toString();
      if (!json.contains(QStringLiteral("data"))) {
        sendResponse(CommandResponse::makeError(id,
                                                ErrorCode::MissingParam,
                                                QStringLiteral("Missing required parameter: data"))
                       .toJsonBytes());
        return;
      }

      const QString dataStr    = json.value(QStringLiteral("data")).toString();
      const QByteArray rawData = QByteArray::fromBase64(dataStr.toUtf8());

      if (rawData.isEmpty() && !dataStr.isEmpty()) {
        sendResponse(CommandResponse::makeError(
                       id, ErrorCode::InvalidParam, QStringLiteral("Invalid base64 data"))
                       .toJsonBytes());
        return;
      }

      if (rawData.size() > kMaxApiRawBytes) {
        qWarning() << "[API] Raw data size limit exceeded:" << state.peerAddress << ":"
                   << state.peerPort << "- Raw data size:" << rawData.size()
                   << "- Limit:" << kMaxApiRawBytes;

        sendResponse(CommandResponse::makeError(id,
                                                ErrorCode::ExecutionError,
                                                QStringLiteral("Raw payload exceeds size limit"))
                       .toJsonBytes());
        return;
      }

      auto& manager = IO::Manager::instance();
      if (!manager.isConnected()) {
        sendResponse(
          CommandResponse::makeError(id, ErrorCode::ExecutionError, QStringLiteral("Not connected"))
            .toJsonBytes());
        return;
      }

      const qint64 bytesWritten = manager.writeData(rawData);
      if (!id.isEmpty()) {
        QJsonObject result;
        result[QStringLiteral("bytesWritten")] = bytesWritten;
        sendResponse(CommandResponse::makeSuccess(id, result).toJsonBytes());
      }

      return;
    }

    auto& cmdHandler = API::CommandHandler::instance();
    sendResponse(cmdHandler.processMessage(jsonBytes));
  };

  auto& buffer = state.buffer;

  // Handles the entire buffer when no newline delimiter is present.
  // Returns true if the caller (while loop) should stop processing.
  auto processNoNewlineBuffer = [&]() {
    const auto trimmed = buffer.trimmed();

    if (!trimmed.isEmpty() && trimmed.at(0) == '{') {
      if (trimmed.size() > kMaxApiMessageBytes) {
        rejectOversize(trimmed.size());
        buffer.clear();
        return;
      }

      if (exceedsJsonDepthLimit(trimmed, kMaxApiJsonDepth)) {
        qWarning() << "[API] JSON depth limit exceeded (buffered):" << state.peerAddress << ":"
                   << state.peerPort << "- Max depth:" << kMaxApiJsonDepth;

        sendResponse(CommandResponse::makeError(QString(),
                                                ErrorCode::ExecutionError,
                                                QStringLiteral("JSON nesting depth exceeds limit"))
                       .toJsonBytes());
        buffer.clear();
        return;
      }

      QString type;
      QJsonObject json;

      // Wrap JSON parsing in try-catch to prevent crashes
      try {
        if (API::parseMessage(trimmed, type, json)) {
          handleJson(trimmed);
          buffer.clear();
        } else if (trimmed.endsWith('}')) {
          sendResponse(CommandResponse::makeError(QString(),
                                                  ErrorCode::InvalidJson,
                                                  QStringLiteral("Failed to parse JSON message"))
                         .toJsonBytes());
          buffer.clear();
        }
      } catch (...) {
        qWarning() << "[API] JSON parsing exception (buffered):" << state.peerAddress << ":"
                   << state.peerPort << "- Buffer size:" << trimmed.size()
                   << "- Disconnecting client (malformed or too deep JSON)";

        sendResponse(
          CommandResponse::makeError(QString(),
                                     ErrorCode::InvalidJson,
                                     QStringLiteral("JSON parsing failed (malformed or too deep)"))
            .toJsonBytes());
        QMetaObject::invokeMethod(
          worker, "disconnectSocket", Qt::QueuedConnection, Q_ARG(QTcpSocket*, socket));
        buffer.clear();
      }
      return;
    }

    if (buffer.size() > kMaxApiRawBytes) {
      qWarning() << "[API] Raw buffer size limit exceeded:" << state.peerAddress << ":"
                 << state.peerPort << "- Buffer size:" << buffer.size()
                 << "- Limit:" << kMaxApiRawBytes << "- Disconnecting client";

      sendResponse(CommandResponse::makeError(QString(),
                                              ErrorCode::ExecutionError,
                                              QStringLiteral("Raw payload exceeds size limit"))
                     .toJsonBytes());
      QMetaObject::invokeMethod(
        worker, "disconnectSocket", Qt::QueuedConnection, Q_ARG(QTcpSocket*, socket));
      buffer.clear();
      return;
    }

    IO::Manager::instance().writeData(buffer);
    buffer.clear();
  };

  while (!buffer.isEmpty()) {
    const int newlineIndex = buffer.indexOf('\n');

    if (newlineIndex < 0) {
      processNoNewlineBuffer();
      return;
    }

    const QByteArray line = buffer.left(newlineIndex + 1);
    buffer.remove(0, newlineIndex + 1);

    const auto trimmedLine = line.trimmed();
    if (trimmedLine.isEmpty())
      continue;

    if (trimmedLine.at(0) != '{') {
      if (line.size() > kMaxApiRawBytes) {
        qWarning() << "[API] Raw line size limit exceeded:" << state.peerAddress << ":"
                   << state.peerPort << "- Line size:" << line.size()
                   << "- Limit:" << kMaxApiRawBytes << "- Disconnecting client";

        sendResponse(CommandResponse::makeError(QString(),
                                                ErrorCode::ExecutionError,
                                                QStringLiteral("Raw payload exceeds size limit"))
                       .toJsonBytes());
        QMetaObject::invokeMethod(
          worker, "disconnectSocket", Qt::QueuedConnection, Q_ARG(QTcpSocket*, socket));
        continue;
      }

      IO::Manager::instance().writeData(line);
      continue;
    }

    if (trimmedLine.size() > kMaxApiMessageBytes) {
      rejectOversize(trimmedLine.size());
      continue;
    }

    if (exceedsJsonDepthLimit(trimmedLine, kMaxApiJsonDepth)) {
      qWarning() << "[API] JSON depth limit exceeded (line):" << state.peerAddress << ":"
                 << state.peerPort << "- Max depth:" << kMaxApiJsonDepth;

      sendResponse(CommandResponse::makeError(QString(),
                                              ErrorCode::ExecutionError,
                                              QStringLiteral("JSON nesting depth exceeds limit"))
                     .toJsonBytes());
      continue;
    }

    handleJson(trimmedLine);
  }
}

/**
 * @brief Accepts new incoming TCP connections.
 *
 * Creates the socket on the main thread, then moves it to the worker thread
 * for all I/O operations.
 */
void API::Server::acceptConnection()
{
  auto* socket = m_server.nextPendingConnection();
  if (!socket) {
    if (enabled())
      Misc::Utilities::showMessageBox(
        tr("API server"), tr("Invalid pending connection"), QMessageBox::Critical);
    return;
  }

  if (!enabled()) {
    socket->close();
    socket->deleteLater();
    return;
  }

  if (m_connections.size() >= kMaxApiClients) {
    qWarning() << "[API] Max clients limit exceeded:" << socket->peerAddress().toString() << ":"
               << socket->peerPort() << "- Current clients:" << m_connections.size()
               << "- Limit:" << kMaxApiClients << "- Rejecting connection";

    socket->abort();
    socket->deleteLater();
    return;
  }

  connect(socket, &QTcpSocket::errorOccurred, this, &Server::onErrorOccurred);
  connect(socket, &QTcpSocket::disconnected, this, [=, this]() { onSocketDisconnected(); });

  ConnectionState state;
  state.peerAddress = socket->peerAddress().toString();
  state.peerPort    = socket->peerPort();
  m_connections.insert(socket, state);

  qInfo() << "[API] New client connected:" << state.peerAddress << ":" << state.peerPort
          << "- Total clients:" << m_connections.size();

  socket->setParent(nullptr);
  socket->moveToThread(&m_workerThread);

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(worker, "addSocket", Qt::QueuedConnection, Q_ARG(QTcpSocket*, socket));
}

/**
 * @brief Handles socket-level errors from connected clients.
 *
 * Logs the error to debug output and attempts to cleanly disconnect
 * the faulty socket.
 *
 * @param socketError Error code provided by the QAbstractSocket.
 */
void API::Server::onErrorOccurred(const QAbstractSocket::SocketError socketError)
{
  qDebug() << socketError;
}

/**
 * @brief Clears socket buffers when a client disconnects.
 *
 * This version is called when the socket itself emits disconnected signal
 * (before being moved to worker thread).
 *
 * NOTE: Cannot safely access peerAddress()/peerPort() after disconnect signal,
 * as the connection information may already be invalid and cause crashes.
 * We use cached peer info from ConnectionState instead.
 */
void API::Server::onSocketDisconnected()
{
  // Validate socket
  auto* socket = qobject_cast<QTcpSocket*>(sender());
  if (!socket)
    return;

  // Clean up MCP session
  const QString sessionId = QString::number(reinterpret_cast<quintptr>(socket));
  MCPHandler::instance().clearSession(sessionId);

  // Use cached peer info for safe logging
  if (m_connections.contains(socket)) {
    const auto& state = m_connections[socket];
    qInfo() << "[API] Client disconnected (via signal):" << state.peerAddress << ":"
            << state.peerPort << "- Remaining clients:" << (m_connections.size() - 1);
  }

  m_connections.remove(socket);
}

/**
 * @brief Clears socket buffers when worker reports socket removal.
 *
 * This version is called from the worker thread when a socket is removed.
 * The socket pointer may already be invalid at this point.
 *
 * @param socket The socket pointer being removed
 */
void API::Server::onSocketDisconnected(QTcpSocket* socket)
{
  if (!socket)
    return;

  const QString sessionId = QString::number(reinterpret_cast<quintptr>(socket));
  MCPHandler::instance().clearSession(sessionId);
  if (m_connections.contains(socket)) {
    qInfo() << "[API] Client disconnected (via worker):"
            << "- Remaining clients:" << (m_connections.size() - 1);

    m_connections.remove(socket);
  }
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
  if (m_clientCount != count) {
    m_clientCount = count;
    Q_EMIT clientCountChanged();
  }
}
