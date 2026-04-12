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

#include <QAtomicInteger>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "API/CommandHandler.h"
#include "API/CommandProtocol.h"
#include "API/MCPHandler.h"
#include "API/MCPProtocol.h"
#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"

// Monotonic counter for session IDs — avoids pointer-to-int casts which are
// unsafe because addresses can be reused after socket deletion
static QAtomicInteger<quintptr> s_nextSessionId{1};

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kMaxApiClients           = 32;
constexpr int kApiWindowMs             = 1000;
constexpr int kMaxApiJsonDepth         = 64;
constexpr int kMaxApiRawBytes          = 1024 * 1024;
constexpr int kMaxApiMessagesPerWindow = 200;
constexpr int kMaxApiMessageBytes      = 1024 * 1024;
constexpr int kMaxApiBufferBytes       = 4 * 1024 * 1024;
constexpr int kMaxApiBytesPerWindow    = 128 * 1024 * 1024;

//--------------------------------------------------------------------------------------------------
// Static functions
//--------------------------------------------------------------------------------------------------

bool exceedsJsonDepthLimit(const QByteArray& data, int maxDepth)
{
  Q_ASSERT(!data.isEmpty());
  Q_ASSERT(maxDepth > 0);

  // Track nesting depth, skipping string contents
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
  Q_ASSERT(QThread::currentThread() == thread());

  // Clean up all managed sockets
  for (auto* socket : std::as_const(m_sockets)) {
    if (socket) {
      socket->abort();
      socket->deleteLater();
    }
  }

  m_sockets.clear();
  Q_ASSERT(m_sockets.isEmpty());

  Q_EMIT clientCountChanged(0);
}

/**
 * @brief Adds a socket to the worker thread
 *
 * The socket is moved to this thread and managed here for all I/O operations.
 */
void API::ServerWorker::addSocket(QTcpSocket* socket)
{
  Q_ASSERT(socket);
  Q_ASSERT(socket->state() != QAbstractSocket::UnconnectedState);

  // Reject null sockets
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
  Q_ASSERT(socket);
  Q_ASSERT(m_sockets.contains(socket));

  // Remove socket and notify listeners
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
  Q_ASSERT(data);
  Q_ASSERT(!m_sockets.isEmpty());

  // Nothing to send if data is empty or no clients connected
  if (!data || data->isEmpty() || m_sockets.isEmpty())
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
 * @brief Broadcasts a lifecycle event JSON object to all connected API clients.
 *
 * Events are sent as {"event": "eventName", ...} messages, allowing plugins
 * to react to connection state changes and save/restore their state.
 */
void API::ServerWorker::broadcastEvent(const QJsonObject& event)
{
  Q_ASSERT(!event.isEmpty());
  Q_ASSERT(!m_sockets.isEmpty());

  // No clients to notify
  if (m_sockets.isEmpty())
    return;

  const QJsonDocument document(event);
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
  Q_ASSERT(socket);
  Q_ASSERT(!data.isEmpty());

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
  Q_ASSERT(socket);

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
  Q_ASSERT(!items.empty());
  Q_ASSERT(!m_sockets.isEmpty());

  // Nothing to process if queue or client list is empty
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
  // Restore persisted settings
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

  (void)API::CommandHandler::instance();
  setEnabled(m_settings.value("API/Enabled", false).toBool());
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
bool API::Server::enabled() const noexcept
{
  return m_enabled;
}

/**
 * @brief Returns whether the server accepts connections from external hosts.
 * @return true if listening on all interfaces; false if localhost only.
 */
bool API::Server::externalConnections() const noexcept
{
  return m_externalConnections;
}

/**
 * @brief Gets the number of currently connected API clients.
 *
 * @return The count of active client connections.
 */
int API::Server::clientCount() const noexcept
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
  Q_ASSERT(m_worker);

  // Determine effective state and start/stop the TCP listener
  bool effectiveEnabled = enabled;
  bool closeResources   = false;

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

  m_settings.setValue("API/Enabled", effectiveEnabled);
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
  // No change needed
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
  Q_ASSERT(data);
  Q_ASSERT(m_worker);

  // Server must be active to relay data
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
 * @brief Broadcasts a lifecycle event to all connected API clients.
 *
 * Sends {"event": "eventName"} to all clients so plugins can react
 * to connection/disconnection/project changes and save/restore state.
 *
 * @param eventName Event name (e.g. "connected", "disconnected").
 */
void API::Server::broadcastLifecycleEvent(const QString& eventName)
{
  Q_ASSERT(!eventName.isEmpty());
  Q_ASSERT(m_worker);

  // Server must be active to broadcast events
  if (!enabled())
    return;

  QJsonObject event;
  event.insert(QStringLiteral("event"), eventName);

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(
    worker, "broadcastEvent", Qt::QueuedConnection, Q_ARG(QJsonObject, event));
}

//--------------------------------------------------------------------------------------------------
// Server — data reception helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends a response to a specific client socket via the worker thread.
 *
 * @param socket The target socket
 * @param response The response bytes to send
 */
void API::Server::sendResponseToSocket(QTcpSocket* socket, const QByteArray& response)
{
  Q_ASSERT(socket);
  Q_ASSERT(!response.isEmpty());

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(worker,
                            "writeToSocket",
                            Qt::QueuedConnection,
                            Q_ARG(QTcpSocket*, socket),
                            Q_ARG(QByteArray, response));
}

/**
 * @brief Sends an error response and disconnects the client.
 *
 * Writes the error message to the socket, then schedules a disconnect
 * on the worker thread and clears the connection buffer.
 *
 * @param socket The target socket
 * @param state Connection state for the socket
 * @param errorCode The API error code string
 * @param errorMessage Human-readable error description
 */
void API::Server::disconnectClient(QTcpSocket* socket,
                                   ConnectionState& state,
                                   const QString& errorCode,
                                   const QString& errorMessage)
{
  Q_ASSERT(socket);
  Q_ASSERT(!errorCode.isEmpty());

  const QByteArray response =
    CommandResponse::makeError(QString(), errorCode, errorMessage).toJsonBytes();
  sendResponseToSocket(socket, response);

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(
    worker, "disconnectSocket", Qt::QueuedConnection, Q_ARG(QTcpSocket*, socket));
  state.buffer.clear();
}

/**
 * @brief Validates rate limits and buffer capacity for incoming data.
 *
 * Checks byte-rate and buffer-size limits. If either is exceeded, an error
 * response is sent and the client is disconnected.
 *
 * @param socket The source socket
 * @param state Connection state for the socket
 * @param data The incoming data chunk
 * @return true if validation passed and processing may continue;
 *         false if the client was disconnected.
 */
bool API::Server::validateRateLimits(QTcpSocket* socket,
                                     ConnectionState& state,
                                     const QByteArray& data)
{
  Q_ASSERT(socket);
  Q_ASSERT(!data.isEmpty());

  // Reset rate-limit window if elapsed
  if (!state.window.isValid())
    state.window.start();

  if (state.window.elapsed() > kApiWindowMs) {
    state.window.restart();
    state.messageCount = 0;
    state.byteCount    = 0;
  }

  // Check byte-rate limit
  state.byteCount += data.size();
  if (state.byteCount > kMaxApiBytesPerWindow) {
    qWarning() << "[API] Byte rate limit exceeded:" << state.peerAddress << ":" << state.peerPort
               << "- Bytes in window:" << state.byteCount << "- Limit:" << kMaxApiBytesPerWindow
               << "- Disconnecting client";

    disconnectClient(
      socket, state, ErrorCode::ExecutionError, QStringLiteral("API rate limit exceeded"));
    return false;
  }

  // Check buffer capacity before appending
  if (state.buffer.size() + data.size() > kMaxApiBufferBytes) {
    qWarning() << "[API] Buffer size limit exceeded:" << state.peerAddress << ":" << state.peerPort
               << "- Buffer size:" << state.buffer.size() << "- Incoming data:" << data.size()
               << "- Limit:" << kMaxApiBufferBytes << "- Disconnecting client";

    disconnectClient(
      socket, state, ErrorCode::ExecutionError, QStringLiteral("API buffer limit exceeded"));
    return false;
  }

  return true;
}

/**
 * @brief Validates JSON message size, depth, and rate limits.
 *
 * Sends an error response if any limit is exceeded. When the rate limit
 * is exceeded, the client is also disconnected.
 *
 * @param socket The source socket (for sending responses)
 * @param state Connection state for the socket
 * @param jsonBytes The JSON message bytes to validate
 * @return true if the message passes all checks; false if rejected.
 */
bool API::Server::validateJsonMessage(QTcpSocket* socket,
                                      ConnectionState& state,
                                      const QByteArray& jsonBytes)
{
  Q_ASSERT(socket);
  Q_ASSERT(!jsonBytes.isEmpty());

  // Reject oversized messages
  if (jsonBytes.size() > kMaxApiMessageBytes) {
    qWarning() << "[API] Message size limit exceeded:" << state.peerAddress << ":" << state.peerPort
               << "- Message size:" << jsonBytes.size() << "- Limit:" << kMaxApiMessageBytes;

    sendResponseToSocket(
      socket,
      CommandResponse::makeError(
        QString(), ErrorCode::ExecutionError, QStringLiteral("API message exceeds size limit"))
        .toJsonBytes());
    return false;
  }

  // Reject deeply nested JSON
  if (exceedsJsonDepthLimit(jsonBytes, kMaxApiJsonDepth)) {
    qWarning() << "[API] JSON depth limit exceeded:" << state.peerAddress << ":" << state.peerPort
               << "- Max depth:" << kMaxApiJsonDepth;

    sendResponseToSocket(
      socket,
      CommandResponse::makeError(
        QString(), ErrorCode::ExecutionError, QStringLiteral("JSON nesting depth exceeds limit"))
        .toJsonBytes());
    return false;
  }

  // Enforce per-window message rate limit
  if (state.messageCount >= kMaxApiMessagesPerWindow) {
    qWarning() << "[API] Message rate limit exceeded:" << state.peerAddress << ":" << state.peerPort
               << "- Messages in window:" << state.messageCount
               << "- Limit:" << kMaxApiMessagesPerWindow << "- Disconnecting client";

    disconnectClient(
      socket, state, ErrorCode::ExecutionError, QStringLiteral("API rate limit exceeded"));
    return false;
  }

  ++state.messageCount;
  return true;
}

/**
 * @brief Dispatches a validated JSON message to the appropriate handler.
 *
 * Validates the message, then routes to MCP, raw data, or command handler.
 *
 * @param socket The source socket (for sending responses)
 * @param state Connection state for the socket
 * @param jsonBytes The trimmed JSON message bytes
 */
void API::Server::handleJsonMessage(QTcpSocket* socket,
                                    ConnectionState& state,
                                    const QByteArray& jsonBytes)
{
  Q_ASSERT(socket);
  Q_ASSERT(!jsonBytes.isEmpty());

  // Validate size, depth, and rate limits
  if (!validateJsonMessage(socket, state, jsonBytes))
    return;

  // Route MCP messages
  if (MCP::isMCPMessage(jsonBytes)) {
    auto& mcpHandler    = API::MCPHandler::instance();
    const auto response = mcpHandler.processMessage(jsonBytes, state.sessionId);

    if (!response.isEmpty())
      sendResponseToSocket(socket, response);

    return;
  }

  // Parse as a standard API message
  QString type;
  QJsonObject json;
  try {
    if (!API::parseMessage(jsonBytes, type, json)) {
      sendResponseToSocket(
        socket,
        CommandResponse::makeError(
          QString(), ErrorCode::InvalidJson, QStringLiteral("Failed to parse JSON message"))
          .toJsonBytes());
      return;
    }
  } catch (...) {
    qWarning() << "[API] JSON parsing exception:" << state.peerAddress << ":" << state.peerPort
               << "- Message size:" << jsonBytes.size()
               << "- Disconnecting client (malformed or too deep JSON)";

    disconnectClient(socket,
                     state,
                     ErrorCode::InvalidJson,
                     QStringLiteral("JSON parsing failed (malformed or too deep)"));
    return;
  }

  // Handle raw data forwarding to the I/O device
  if (type == MessageType::Raw) {
    processRawJsonCommand(socket, state, json);
    return;
  }

  // Dispatch to the command handler
  auto& cmdHandler = API::CommandHandler::instance();
  sendResponseToSocket(socket, cmdHandler.processMessage(jsonBytes));
}

/**
 * @brief Processes a JSON "raw" command that forwards base64 data to the device.
 *
 * Validates required fields, decodes base64 payload, checks size limits,
 * and writes to the I/O connection manager.
 *
 * @param socket The source socket (for sending responses)
 * @param state Connection state for the socket
 * @param json The parsed JSON object containing the raw command
 */
void API::Server::processRawJsonCommand(QTcpSocket* socket,
                                        ConnectionState& state,
                                        const QJsonObject& json)
{
  Q_ASSERT(socket);

  const QString id = json.value(QStringLiteral("id")).toString();

  // Validate presence of data field
  if (!json.contains(QStringLiteral("data"))) {
    sendResponseToSocket(
      socket,
      CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: data"))
        .toJsonBytes());
    return;
  }

  // Decode and validate base64 payload
  const QString dataStr    = json.value(QStringLiteral("data")).toString();
  const QByteArray rawData = QByteArray::fromBase64(dataStr.toUtf8());
  if (rawData.isEmpty() && !dataStr.isEmpty()) {
    sendResponseToSocket(
      socket,
      CommandResponse::makeError(id, ErrorCode::InvalidParam, QStringLiteral("Invalid base64 data"))
        .toJsonBytes());
    return;
  }

  // Enforce raw payload size limit
  if (rawData.size() > kMaxApiRawBytes) {
    qWarning() << "[API] Raw data size limit exceeded:" << state.peerAddress << ":"
               << state.peerPort << "- Raw data size:" << rawData.size()
               << "- Limit:" << kMaxApiRawBytes;

    sendResponseToSocket(
      socket,
      CommandResponse::makeError(
        id, ErrorCode::ExecutionError, QStringLiteral("Raw payload exceeds size limit"))
        .toJsonBytes());
    return;
  }

  // Write to device
  auto& manager = IO::ConnectionManager::instance();
  if (!manager.isConnected()) {
    sendResponseToSocket(
      socket,
      CommandResponse::makeError(id, ErrorCode::ExecutionError, QStringLiteral("Not connected"))
        .toJsonBytes());
    return;
  }

  const qint64 bytesWritten = manager.writeData(rawData);
  if (!id.isEmpty()) {
    QJsonObject result;
    result[QStringLiteral("bytesWritten")] = bytesWritten;
    sendResponseToSocket(socket, CommandResponse::makeSuccess(id, result).toJsonBytes());
  }
}

/**
 * @brief Handles a buffered message when no newline delimiter is present.
 *
 * Attempts to parse the buffer as JSON if it starts with '{' or '['.
 * Otherwise treats it as raw data and forwards to the I/O device.
 *
 * @param socket The source socket
 * @param state Connection state (buffer is consumed and cleared)
 */
void API::Server::processNoNewlineBuffer(QTcpSocket* socket, ConnectionState& state)
{
  Q_ASSERT(socket);

  auto& buffer       = state.buffer;
  const auto trimmed = buffer.trimmed();

  // Try JSON if buffer looks like JSON
  const char firstChar = trimmed.isEmpty() ? '\0' : trimmed.at(0);
  if (firstChar == '{' || firstChar == '[') {
    processBufferedJson(socket, state, trimmed);
    return;
  }

  // Forward as raw data
  if (buffer.size() > kMaxApiRawBytes) {
    qWarning() << "[API] Raw buffer size limit exceeded:" << state.peerAddress << ":"
               << state.peerPort << "- Buffer size:" << buffer.size()
               << "- Limit:" << kMaxApiRawBytes << "- Disconnecting client";

    disconnectClient(
      socket, state, ErrorCode::ExecutionError, QStringLiteral("Raw payload exceeds size limit"));
    return;
  }

  const qint64 written = IO::ConnectionManager::instance().writeData(buffer);
  if (written < 0) [[unlikely]]
    qWarning() << "[API] writeData() failed for raw buffer"
               << "— data not sent to device";

  buffer.clear();
}

/**
 * @brief Attempts to parse buffered data as a complete JSON message.
 *
 * Called when the buffer has no newline delimiter and starts with '{' or '['.
 * Pre-checks size/depth limits to avoid feeding huge data to the parser,
 * then dispatches via handleJsonMessage() if parseable.
 * The buffer is cleared whenever a message is consumed or rejected.
 *
 * @param socket The source socket
 * @param state Connection state (buffer is cleared on success or error)
 * @param trimmed The trimmed buffer contents
 */
void API::Server::processBufferedJson(QTcpSocket* socket,
                                      ConnectionState& state,
                                      const QByteArray& trimmed)
{
  Q_ASSERT(socket);
  Q_ASSERT(!trimmed.isEmpty());

  auto& buffer = state.buffer;

  // Pre-check size before parsing attempt
  if (trimmed.size() > kMaxApiMessageBytes) {
    qWarning() << "[API] Message size limit exceeded:" << state.peerAddress << ":" << state.peerPort
               << "- Message size:" << trimmed.size() << "- Limit:" << kMaxApiMessageBytes;

    sendResponseToSocket(
      socket,
      CommandResponse::makeError(
        QString(), ErrorCode::ExecutionError, QStringLiteral("API message exceeds size limit"))
        .toJsonBytes());
    buffer.clear();
    return;
  }

  // Pre-check depth before parsing attempt
  if (exceedsJsonDepthLimit(trimmed, kMaxApiJsonDepth)) {
    qWarning() << "[API] JSON depth limit exceeded (buffered):" << state.peerAddress << ":"
               << state.peerPort << "- Max depth:" << kMaxApiJsonDepth;

    sendResponseToSocket(
      socket,
      CommandResponse::makeError(
        QString(), ErrorCode::ExecutionError, QStringLiteral("JSON nesting depth exceeds limit"))
        .toJsonBytes());
    buffer.clear();
    return;
  }

  // Attempt to parse and dispatch
  QString type;
  QJsonObject json;
  try {
    const char firstChar = trimmed.at(0);
    const char lastChar  = trimmed.back();
    const bool complete =
      (firstChar == '{' && lastChar == '}') || (firstChar == '[' && lastChar == ']');

    if (API::parseMessage(trimmed, type, json) || MCP::isMCPMessage(trimmed)) {
      handleJsonMessage(socket, state, trimmed);
      buffer.clear();
    } else if (complete) {
      sendResponseToSocket(
        socket,
        CommandResponse::makeError(
          QString(), ErrorCode::InvalidJson, QStringLiteral("Failed to parse JSON message"))
          .toJsonBytes());
      buffer.clear();
    }
  } catch (...) {
    qWarning() << "[API] JSON parsing exception (buffered):" << state.peerAddress << ":"
               << state.peerPort << "- Buffer size:" << trimmed.size()
               << "- Disconnecting client (malformed or too deep JSON)";

    disconnectClient(socket,
                     state,
                     ErrorCode::InvalidJson,
                     QStringLiteral("JSON parsing failed (malformed or too deep)"));
  }
}

/**
 * @brief Processes a newline-delimited JSON line from the buffer.
 *
 * Validates size and depth limits before dispatching to handleJsonMessage().
 *
 * @param socket The source socket
 * @param state Connection state for the socket
 * @param trimmedLine The trimmed line (already confirmed to start with '{' or '[')
 */
void API::Server::processJsonLine(QTcpSocket* socket,
                                  ConnectionState& state,
                                  const QByteArray& trimmedLine)
{
  Q_ASSERT(socket);
  Q_ASSERT(!trimmedLine.isEmpty());

  handleJsonMessage(socket, state, trimmedLine);
}

/**
 * @brief Processes a non-JSON raw line from the buffer.
 *
 * Validates size limits and forwards data to the I/O connection manager.
 * When the line exceeds the raw size limit, an error response is sent and
 * a disconnect is scheduled, but the buffer is not cleared so the caller
 * can finish processing remaining lines.
 *
 * @param socket The source socket
 * @param state Connection state for the socket
 * @param line The raw line including the newline delimiter
 */
void API::Server::processRawLine(QTcpSocket* socket, ConnectionState& state, const QByteArray& line)
{
  Q_ASSERT(socket);
  Q_ASSERT(!line.isEmpty());

  // Reject oversized raw lines
  if (line.size() > kMaxApiRawBytes) {
    qWarning() << "[API] Raw line size limit exceeded:" << state.peerAddress << ":"
               << state.peerPort << "- Line size:" << line.size() << "- Limit:" << kMaxApiRawBytes
               << "- Disconnecting client";

    sendResponseToSocket(
      socket,
      CommandResponse::makeError(
        QString(), ErrorCode::ExecutionError, QStringLiteral("Raw payload exceeds size limit"))
        .toJsonBytes());

    auto* worker = static_cast<ServerWorker*>(m_worker);
    QMetaObject::invokeMethod(
      worker, "disconnectSocket", Qt::QueuedConnection, Q_ARG(QTcpSocket*, socket));
    return;
  }

  // Forward raw line to device
  const qint64 written = IO::ConnectionManager::instance().writeData(line);
  if (written < 0) [[unlikely]]
    qWarning() << "[API] writeData() failed for raw line"
               << "— data not sent to device";
}

//--------------------------------------------------------------------------------------------------
// Server — data reception & dispatch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles incoming data from worker thread.
 *
 * Receives data from the worker thread (which read it from a socket).
 * Validates rate limits, splits buffered data on newline boundaries,
 * and dispatches each message to the appropriate handler.
 *
 * @param socket The socket that sent the data (for sending responses)
 * @param data The received data
 */
void API::Server::onDataReceived(QTcpSocket* socket, const QByteArray& data)
{
  Q_ASSERT(socket);
  Q_ASSERT(!data.isEmpty());

  // Server must be active and data non-empty
  if (!enabled() || data.isEmpty() || !socket)
    return;

  if (!m_connections.contains(socket))
    return;

  auto& state = m_connections[socket];

  // Enforce rate and buffer limits
  if (!validateRateLimits(socket, state, data))
    return;

  state.buffer.append(data);
  auto& buffer = state.buffer;

  // Process all newline-delimited messages in the buffer
  constexpr int kMaxBufferIterations = 10000;
  int bufferIterations               = 0;
  while (!buffer.isEmpty() && bufferIterations < kMaxBufferIterations) {
    ++bufferIterations;

    const int newlineIndex = buffer.indexOf('\n');

    // No newline found — attempt to process partial buffer
    if (newlineIndex < 0) {
      processNoNewlineBuffer(socket, state);
      return;
    }

    // Extract and consume one line
    const QByteArray line = buffer.left(newlineIndex + 1);
    buffer.remove(0, newlineIndex + 1);

    const auto trimmedLine = line.trimmed();
    if (trimmedLine.isEmpty())
      continue;

    // Dispatch based on content type
    if (trimmedLine.at(0) == '{' || trimmedLine.at(0) == '[')
      processJsonLine(socket, state, trimmedLine);
    else
      processRawLine(socket, state, line);
  }

  if (bufferIterations >= kMaxBufferIterations) [[unlikely]]
    qWarning() << "[API] Buffer processing iteration limit reached:" << state.peerAddress << ":"
               << state.peerPort;
}

/**
 * @brief Accepts new incoming TCP connections.
 *
 * Creates the socket on the main thread, then moves it to the worker thread
 * for all I/O operations.
 */
void API::Server::acceptConnection()
{
  Q_ASSERT(m_server.isListening());
  Q_ASSERT(m_enabled);

  // Retrieve the pending connection
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
  state.sessionId   = QString::number(s_nextSessionId.fetchAndAddRelaxed(1));
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
  // Resolve the disconnected socket from the signal sender
  auto* socket = qobject_cast<QTcpSocket*>(sender());
  if (!socket)
    return;

  // Clean up MCP session and remove connection state
  if (m_connections.contains(socket)) {
    const auto& state = m_connections[socket];
    MCPHandler::instance().clearSession(state.sessionId);
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

  if (m_connections.contains(socket)) {
    const auto& state = m_connections[socket];
    MCPHandler::instance().clearSession(state.sessionId);
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
