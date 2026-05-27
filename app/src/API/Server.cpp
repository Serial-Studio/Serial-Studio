/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
#include <QRandomGenerator>

#include "API/CommandHandler.h"
#include "API/CommandProtocol.h"
#include "API/MCPHandler.h"
#include "API/MCPProtocol.h"
#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"

// Monotonic counter for session IDs, since socket addresses can be reused.
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
constexpr int kMaxAuthAttempts         = 3;
constexpr int kAuthTokenBytes          = 32;

//--------------------------------------------------------------------------------------------------
// Static functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Generates a cryptographically random hex token for external API auth.
 */
static QString generateApiToken()
{
  QByteArray raw;
  raw.reserve(kAuthTokenBytes);

  auto* rng = QRandomGenerator::system();
  for (int i = 0; i < kAuthTokenBytes / int(sizeof(quint32)); ++i) {
    const quint32 value = rng->generate();
    raw.append(reinterpret_cast<const char*>(&value), sizeof(value));
  }

  return QString::fromLatin1(raw.toHex());
}

/**
 * @brief Returns true if the JSON byte stream nests deeper than the given limit.
 */
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

      continue;
    }

    if ((ch == '}' || ch == ']') && depth > 0)
      --depth;
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
 */
void API::ServerWorker::writeRawData(const QByteArray& data)
{
  if (data.isEmpty() || m_sockets.isEmpty())
    return;

  QJsonObject object;
  object.insert(QStringLiteral("data"), QString::fromUtf8(data.toBase64()));
  const QJsonDocument document(object);
  const auto json = document.toJson(QJsonDocument::Compact) + "\n";

  for (auto* socket : std::as_const(m_sockets))
    if (socket && socket->isWritable())
      socket->write(json);
}

/**
 * @brief Broadcasts a lifecycle event JSON object to all connected API clients.
 */
void API::ServerWorker::broadcastEvent(const QJsonObject& event)
{
  Q_ASSERT(!event.isEmpty());

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
 */
void API::ServerWorker::processItems(const std::vector<DataModel::TimestampedFramePtr>& items)
{
  Q_ASSERT(!items.empty());

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
 */
API::Server::Server()
  : DataModel::FrameConsumer<DataModel::TimestampedFramePtr>(
      {.queueCapacity = 2048, .flushThreshold = 512, .timerIntervalMs = 1000})
  , m_clientCount(0)
  , m_enabled(false)
  , m_externalConnections(false)
  , m_deviceWriteConsent(DeviceWriteConsent::Unset)
{
  // Restore persisted settings
  m_externalConnections = m_settings.value("API/ExternalConnections", false).toBool();
  m_authToken           = m_settings.value("API/AuthToken").toString();

  // A previously granted device-write consent is remembered; a denial never persists.
  if (m_settings.value("API/DeviceWriteConsent", false).toBool())
    m_deviceWriteConsent = DeviceWriteConsent::Granted;

  // A token must exist whenever external clients are allowed, or none could connect.
  if (m_externalConnections)
    ensureAuthToken();

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
 */
API::Server& API::Server::instance()
{
  static Server singleton;
  return singleton;
}

/**
 * @brief Checks whether the API server is currently enabled.
 */
bool API::Server::enabled() const noexcept
{
  return m_enabled;
}

/**
 * @brief Returns whether the server accepts connections from external hosts.
 */
bool API::Server::externalConnections() const noexcept
{
  return m_externalConnections;
}

/**
 * @brief Gets the number of currently connected API clients.
 */
int API::Server::clientCount() const noexcept
{
  return m_clientCount;
}

/**
 * @brief Disconnects a client socket from the server.
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

  // External clients authenticate with a token; make sure one exists before exposing the port.
  if (m_externalConnections)
    ensureAuthToken();

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

//--------------------------------------------------------------------------------------------------
// Server: authentication (external connections)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the token external (non-loopback) clients must present to authenticate.
 */
QString API::Server::authToken() const
{
  return m_authToken;
}

/**
 * @brief Generates and persists the auth token once; a no-op when one already exists.
 */
void API::Server::ensureAuthToken()
{
  if (!m_authToken.isEmpty())
    return;

  m_authToken = generateApiToken();
  m_settings.setValue("API/AuthToken", m_authToken);
  Q_EMIT authTokenChanged();
}

/**
 * @brief Issues a fresh auth token; already-authenticated sessions stay connected.
 */
void API::Server::regenerateAuthToken()
{
  m_authToken = generateApiToken();
  m_settings.setValue("API/AuthToken", m_authToken);
  Q_EMIT authTokenChanged();
}

/**
 * @brief Compares two byte arrays in constant time to avoid token timing side channels.
 */
bool API::Server::constantTimeEquals(const QByteArray& a, const QByteArray& b)
{
  if (a.size() != b.size())
    return false;

  quint8 diff = 0;
  for (qsizetype i = 0; i < a.size(); ++i)
    diff |= static_cast<quint8>(a[i]) ^ static_cast<quint8>(b[i]);

  return diff == 0;
}

/**
 * @brief Constant-time check of a client-provided token against the configured one.
 */
bool API::Server::verifyToken(const QByteArray& provided) const
{
  return !m_authToken.isEmpty() && constantTimeEquals(provided, m_authToken.toUtf8());
}

/**
 * @brief Gates API-originated device writes behind a one-time user consent prompt.
 */
bool API::Server::authorizeDeviceWrite()
{
  if (m_deviceWriteConsent == DeviceWriteConsent::Granted)
    return true;

  if (m_deviceWriteConsent == DeviceWriteConsent::Denied)
    return false;

  // First request this session: prompt once. A grant persists; a denial lasts only this run.
  const auto answer = Misc::Utilities::showMessageBox(
    tr("Allow API device control?"),
    tr("A program using Serial Studio's local API is requesting to send data to the connected "
       "device. Allow API clients to write to the device?"),
    QMessageBox::Question,
    tr("Serial Studio"),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::No);

  if (answer == QMessageBox::Yes) {
    m_deviceWriteConsent = DeviceWriteConsent::Granted;
    m_settings.setValue("API/DeviceWriteConsent", true);
    return true;
  }

  m_deviceWriteConsent = DeviceWriteConsent::Denied;
  return false;
}

/**
 * @brief Consumes the first line as a {"type":"auth","token":...} handshake before commands.
 */
void API::Server::handleAuthHandshake(QTcpSocket* socket,
                                      ConnectionState& state,
                                      const QByteArray& data)
{
  Q_ASSERT(socket);

  // Buffer until a full line arrives; cap the pre-auth buffer to reject unauthenticated floods.
  state.buffer.append(data);
  if (state.buffer.size() > kMaxApiMessageBytes) {
    disconnectClient(
      socket, state, ErrorCode::ExecutionError, QStringLiteral("Authentication required"));
    return;
  }

  const int newlineIndex = state.buffer.indexOf('\n');
  if (newlineIndex < 0)
    return;

  const QByteArray line = state.buffer.left(newlineIndex).trimmed();
  state.buffer.remove(0, newlineIndex + 1);

  // Validate {"type":"auth","token":"<token>"} with a constant-time token match.
  bool ok = false;
  QJsonParseError parseError;
  const auto doc = QJsonDocument::fromJson(line, &parseError);
  if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
    const auto obj = doc.object();
    if (obj.value(QStringLiteral("type")).toString() == QStringLiteral("auth")) {
      const QByteArray provided = obj.value(QStringLiteral("token")).toString().toUtf8();
      ok                        = verifyToken(provided);
    }
  }

  // Reject; drop the connection after a few failed attempts.
  if (!ok) {
    if (++state.authAttempts >= kMaxAuthAttempts) {
      qWarning() << "[API] Authentication failed:" << state.peerAddress << ":" << state.peerPort
                 << "- Disconnecting after" << state.authAttempts << "attempts";
      disconnectClient(
        socket, state, ErrorCode::ExecutionError, QStringLiteral("Authentication failed"));
      return;
    }

    sendResponseToSocket(socket,
                         CommandResponse::makeError(QString(),
                                                    ErrorCode::ExecutionError,
                                                    QStringLiteral("Authentication required"))
                           .toJsonBytes());
    return;
  }

  // Authenticated: acknowledge, then process any data pipelined after the handshake.
  state.authenticated = true;
  qInfo() << "[API] Client authenticated:" << state.peerAddress << ":" << state.peerPort;

  QJsonObject result;
  result[QStringLiteral("authenticated")] = true;
  sendResponseToSocket(socket, CommandResponse::makeSuccess(QString(), result).toJsonBytes());

  if (!state.buffer.isEmpty()) {
    const QByteArray pipelined = state.buffer;
    state.buffer.clear();
    onDataReceived(socket, pipelined);
  }
}

/**
 * @brief Sends raw binary data to all connected clients.
 */
void API::Server::hotpathTxData(const QByteArray& data)
{
  Q_ASSERT(m_worker);

  // Server must be active to relay data
  if (!enabled())
    return;

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(worker, "writeRawData", Qt::QueuedConnection, Q_ARG(QByteArray, data));
}

/**
 * @brief Registers a new structured data frame.
 */
void API::Server::hotpathTxFrame(const DataModel::TimestampedFramePtr& frame)
{
  if (enabled())
    enqueueData(frame);
}

/**
 * @brief Broadcasts a lifecycle event to all connected API clients.
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
// Server: data reception helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends a response to a specific client socket via the worker thread.
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

  // Require user consent before letting an API client drive the device.
  if (!authorizeDeviceWrite()) {
    sendResponseToSocket(socket,
                         CommandResponse::makeError(id,
                                                    ErrorCode::ExecutionError,
                                                    QStringLiteral("Device write denied by user"))
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
               << "-- data not sent to device";

  buffer.clear();
}

/**
 * @brief Attempts to parse buffered data as a complete JSON message.
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

  // Require user consent before letting an API client drive the device.
  if (!authorizeDeviceWrite()) {
    sendResponseToSocket(socket,
                         CommandResponse::makeError(QString(),
                                                    ErrorCode::ExecutionError,
                                                    QStringLiteral("Device write denied by user"))
                           .toJsonBytes());
    return;
  }

  // Forward raw line to device
  const qint64 written = IO::ConnectionManager::instance().writeData(line);
  if (written < 0) [[unlikely]]
    qWarning() << "[API] writeData() failed for raw line"
               << "-- data not sent to device";
}

//--------------------------------------------------------------------------------------------------
// Server: data reception & dispatch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles incoming data from worker thread.
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

  // Untrusted (non-loopback) clients must authenticate before any command or raw data.
  if (!state.authenticated) {
    handleAuthHandshake(socket, state, data);
    return;
  }

  state.buffer.append(data);
  auto& buffer = state.buffer;

  // Process all newline-delimited messages in the buffer
  constexpr int kMaxBufferIterations = 10000;
  int bufferIterations               = 0;
  while (!buffer.isEmpty() && bufferIterations < kMaxBufferIterations) {
    ++bufferIterations;

    const int newlineIndex = buffer.indexOf('\n');

    // No newline found, attempt to process partial buffer
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

  // Loopback peers are trusted; only non-loopback peers authenticate (external mode only).
  state.authenticated = !(m_externalConnections && !socket->peerAddress().isLoopback());
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
 */
void API::Server::onErrorOccurred(const QAbstractSocket::SocketError socketError)
{
  qWarning() << socketError;
}

/**
 * @brief Clears socket buffers on disconnect, using cached peer info (post-disconnect peer is
 * unsafe).
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
 */
void API::Server::onClientCountChanged(int count)
{
  if (m_clientCount != count) {
    m_clientCount = count;
    Q_EMIT clientCountChanged();
  }
}
