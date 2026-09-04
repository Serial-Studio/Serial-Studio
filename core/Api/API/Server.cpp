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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Server.h"

#include <QAtomicInteger>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <utility>

#include "API/CommandHandler.h"
#include "API/MCPHandler.h"
#include "API/Mirror/MirrorPublisher.h"
#include "API/Server/MirrorCommands.h"
#include "Core/SSAssert.h"
#include "DataModel/FrameBuilder.h"
#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"

// Monotonic counter for session IDs, since socket addresses can be reused.
static QAtomicInteger<quintptr> s_nextSessionId{1};

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kMaxApiClients = 32;

// Bounds of the configurable listening port; anything outside falls back to API_TCP_PORT
constexpr int kMinApiPort = 1;
constexpr int kMaxApiPort = 65535;

// Per-subscriber stream backlog before the oldest block is dropped and counted (spec 0051 R24)
constexpr std::size_t kStreamQueueDepth = 8;

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the API server.
 */
API::Server::Server()
  : DataModel::FrameConsumer<DataModel::DataBlockPtr>(
      {.queueCapacity = 2048, .flushThreshold = 512, .timerIntervalMs = 1000})
  , m_auth(m_settings)
  , m_reception(*this)
  , m_port(API_TCP_PORT)
  , m_clientCount(0)
  , m_enabled(false)
  , m_mirrorLinked(false)
  , m_externalConnections(false)
  , m_anyStreamSubscriber(false)
{
  connect(&m_auth, &ServerAuth::authTokenChanged, this, &Server::authTokenChanged);

  m_externalConnections = m_settings.value("API/ExternalConnections", false).toBool();
  if (m_externalConnections)
    m_auth.ensureAuthToken();

  const int stored = m_settings.value("API/Port", API_TCP_PORT).toInt();
  if (stored >= kMinApiPort && stored <= kMaxApiPort)
    m_port = stored;

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
          &Server::onSocketDisconnected,
          Qt::QueuedConnection);
  connect(
    worker, &ServerWorker::streamWriteDone, this, &Server::onStreamWriteDone, Qt::QueuedConnection);

  connect(&m_server, &QTcpServer::newConnection, this, &Server::acceptConnection);
  connect(&m_serverIpv6, &QTcpServer::newConnection, this, &Server::acceptConnection);

  static auto& commandHandler = API::CommandHandler::instance();
  (void)commandHandler;
  setEnabled(m_settings.value("API/Enabled", false).toBool());
}

/**
 * @brief Destroys the API server.
 */
API::Server::~Server()
{
  stopListening();
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

//--------------------------------------------------------------------------------------------------
// Server state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maximum simultaneous API connections; also the ceiling on mirror viewers.
 */
int API::Server::maxClients() noexcept
{
  return kMaxApiClients;
}

/**
 * @brief Checks whether the API server is currently enabled.
 */
bool API::Server::enabled() const noexcept
{
  return m_enabled;
}

/**
 * @brief True while at least one connection holds a stream subscription. What gates the stream
 *        workers' typed export payloads: pushStreamBlock() drops a block for every connection
 *        that never subscribed, so an enabled server with no subscriber would otherwise pay a
 *        full per-block payload build and a queued fan-out to four sinks that all discard it.
 */
bool API::Server::hasStreamSubscribers() const noexcept
{
  for (auto it = m_connections.constBegin(); it != m_connections.constEnd(); ++it)
    if (it.value().streamSubscribed)
      return true;

  return false;
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
 * @brief The port the API server listens on (API/Port, 7777 by default).
 */
int API::Server::port() const noexcept
{
  return m_port;
}

/**
 * @brief Persists a new listening port and rebinds a running server to it.
 */
void API::Server::setPort(const int port)
{
  if (port < kMinApiPort || port > kMaxApiPort || port == m_port)
    return;

  m_port = port;
  m_settings.setValue("API/Port", m_port);
  Q_EMIT portChanged();

  if (!m_enabled)
    return;

  dropConnections();
  if (startListening())
    return;

  m_enabled = false;
  Q_EMIT enabledChanged();
}

/**
 * @brief Binds the listening sockets: both loopback families when local, one dual-stack Any
 *        listener when external. A single QTcpServer binds a single address, so a lone
 *        LocalHost listener refused every ::1 client -- which is what "localhost" resolves to
 *        first on a modern system (spec 0075 I10). A missing IPv6 stack is not fatal.
 */
bool API::Server::startListening()
{
  stopListening();

  m_server.setMaxPendingConnections(kMaxApiClients);
  const auto address = m_externalConnections ? QHostAddress::Any : QHostAddress::LocalHost;
  if (!m_server.listen(address, static_cast<quint16>(m_port))) {
    Misc::Utilities::showMessageBox(
      tr("Unable to start API TCP server"), m_server.errorString(), QMessageBox::Warning);
    m_server.close();
    return false;
  }

  if (m_externalConnections)
    return true;

  m_serverIpv6.setMaxPendingConnections(kMaxApiClients);
  if (!m_serverIpv6.listen(QHostAddress::LocalHostIPv6, static_cast<quint16>(m_port)))
    qWarning() << "[API] IPv6 loopback listener unavailable:" << m_serverIpv6.errorString()
               << "- IPv4 clients are unaffected";

  return true;
}

/**
 * @brief Closes both listening sockets.
 */
void API::Server::stopListening()
{
  m_server.close();
  m_serverIpv6.close();
}

/**
 * @brief Forgets every connection and tells the worker to drop its sockets.
 */
void API::Server::dropConnections()
{
  SS_ASSERT(m_worker != nullptr, return);

  m_connections.clear();
  if (m_mirrorLinked)
    mirrorPublisher().clearSubscribers();

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(worker, "closeResources", Qt::QueuedConnection);
}

/**
 * @brief Enables or disables the TCP API server.
 */
void API::Server::setEnabled(const bool enabled)
{
  SS_ASSERT(m_worker != nullptr, return);

  bool effectiveEnabled = enabled;
  bool closeResources   = false;

  if (enabled) {
    if (!m_server.isListening() && !startListening()) {
      effectiveEnabled = false;
      closeResources   = true;
    }
  }

  else {
    stopListening();
    closeResources = true;

    if (m_externalConnections) {
      m_externalConnections = false;
      m_settings.setValue("API/ExternalConnections", false);
      Q_EMIT externalConnectionsChanged();
    }
  }

  if (closeResources)
    dropConnections();

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

  applyExternalConnections(enabled);
}

/**
 * @brief Opens the server to non-loopback peers without the confirmation dialog, for the
 *        --api-external flag: a windowless process cannot answer a modal, so a headless-only
 *        machine would otherwise have no way to be made attachable (spec 0040, R16). The flag
 *        is the explicit act the dialog stands in for; nothing here runs unless it was passed.
 */
void API::Server::allowExternalConnections()
{
  if (m_externalConnections) {
    m_auth.ensureAuthToken();
    return;
  }

  applyExternalConnections(true);
}

/**
 * @brief Persists the external-connections decision, provisions the token, and rebinds a running
 *        server to the matching address.
 */
void API::Server::applyExternalConnections(const bool enabled)
{
  m_externalConnections = enabled;
  m_settings.setValue("API/ExternalConnections", m_externalConnections);
  Q_EMIT externalConnectionsChanged();

  if (m_externalConnections)
    m_auth.ensureAuthToken();

  if (!m_enabled)
    return;

  dropConnections();
  if (startListening())
    return;

  m_enabled = false;
  Q_EMIT enabledChanged();
}

//--------------------------------------------------------------------------------------------------
// Authentication (forwarded to the credential half)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the token external (non-loopback) clients must present to authenticate.
 */
QString API::Server::authToken() const
{
  return m_auth.authToken();
}

/**
 * @brief Pins a caller-supplied auth token, for provisioning a headless machine from the command
 *        line; false when the token is not a usable credential.
 */
bool API::Server::setAuthToken(const QString& token)
{
  return m_auth.setAuthToken(token);
}

/**
 * @brief Issues a fresh auth token; already-authenticated sessions stay connected.
 */
void API::Server::regenerateAuthToken()
{
  m_auth.regenerateAuthToken();
}

/**
 * @brief Constant-time check of a client-provided token against the configured one.
 */
bool API::Server::verifyToken(const QByteArray& provided) const
{
  return m_auth.verifyToken(provided);
}

/**
 * @brief Gates API-originated device writes behind the one-time user consent prompt, answering
 *        without blocking: an unanswered consent posts the prompt and refuses this write.
 */
API::DeviceWriteVerdict API::Server::authorizeDeviceWrite()
{
  return m_auth.authorizeDeviceWrite();
}

/**
 * @brief Gates remote-origin device-write commands behind the consent prompt; commands that
 *        never touch the hardware always pass.
 */
bool API::Server::authorizeRemoteCommand(const QString& command)
{
  return m_auth.authorizeRemoteCommand(command);
}

//--------------------------------------------------------------------------------------------------
// Outbound broadcasts
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends raw binary data to all connected clients.
 */
void API::Server::hotpathTxData(const QByteArray& data)
{
  SS_ASSERT(m_worker != nullptr, return);

  if (!enabled())
    return;

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(worker, "writeRawData", Qt::QueuedConnection, Q_ARG(QByteArray, data));
}

/**
 * @brief Adopts the frame structure the pipeline publishes on every layout change. The wire keeps
 *        its per-frame shape (spec 0055 D5), and a block carries values only, so the worker needs
 *        this to rebuild the frame it serializes.
 */
void API::Server::setupExternalConnections()
{
  SS_ASSERT(m_worker != nullptr, return);

  auto* worker = static_cast<ServerWorker*>(m_worker);
  connect(&DataModel::FrameBuilder::instance(),
          &DataModel::FrameBuilder::structurePublished,
          worker,
          &ServerWorker::setTemplateFrame,
          Qt::QueuedConnection);
}

/**
 * @brief Registers one published block. Feeds both wire surfaces: the frame-shaped default every
 *        API client already parses, and the block-shaped payload a stream.subscribe opt-in gets.
 */
void API::Server::ingestBlock(const DataModel::DataBlockPtr& block)
{
  if (!block || !enabled())
    return;

  enqueueData(block);

  if (!m_anyStreamSubscriber.load(std::memory_order_relaxed))
    return;

  QMetaObject::invokeMethod(this, [this, block] { pushStreamBlock(block); }, Qt::QueuedConnection);
}

/**
 * @brief Recomputes the lock-free subscriber flag the pipeline thread reads. Must run on the GUI
 *        thread, which owns m_connections; the pipeline may never touch that container.
 */
void API::Server::refreshStreamSubscriberFlag() noexcept
{
  m_anyStreamSubscriber.store(hasStreamSubscribers(), std::memory_order_relaxed);
}

/**
 * @brief Broadcasts a lifecycle event to all connected API clients.
 */
void API::Server::broadcastLifecycleEvent(const QString& eventName)
{
  SS_ASSERT(!eventName.isEmpty(), return);
  SS_ASSERT(m_worker != nullptr, return);

  if (!enabled())
    return;

  QJsonObject event;
  event.insert(QStringLiteral("event"), eventName);

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(
    worker, "broadcastEvent", Qt::QueuedConnection, Q_ARG(QJsonObject, event));
}

//--------------------------------------------------------------------------------------------------
// Reception host: writes, dispatch and the device
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends a response to a specific client socket via the worker thread, tagged with the
 *        connection's session id so a reused socket pointer can never receive it.
 */
void API::Server::sendResponse(QTcpSocket* socket, const QByteArray& response)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT(!response.isEmpty(), return);

  const auto it = m_connections.constFind(socket);
  if (it == m_connections.constEnd())
    return;

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(worker,
                            "writeToSocket",
                            Qt::QueuedConnection,
                            Q_ARG(QTcpSocket*, socket),
                            Q_ARG(QString, it->sessionId),
                            Q_ARG(QByteArray, response));
}

/**
 * @brief Asks the worker thread to drop one connection, leaving its buffer untouched.
 */
void API::Server::closeSocket(QTcpSocket* socket, const ConnectionState& state)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT_LOG(!state.sessionId.isEmpty());

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(worker,
                            "disconnectSocket",
                            Qt::QueuedConnection,
                            Q_ARG(QTcpSocket*, socket),
                            Q_ARG(QString, state.sessionId));
}

/**
 * @brief Sends an error response and disconnects the client.
 */
void API::Server::disconnectClient(QTcpSocket* socket,
                                   ConnectionState& state,
                                   const QString& errorCode,
                                   const QString& errorMessage)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT_LOG(!errorCode.isEmpty());

  const QByteArray response =
    CommandResponse::makeError(QString(), errorCode, errorMessage).toJsonBytes();
  sendResponse(socket, response);

  closeSocket(socket, state);
  state.buffer.clear();
}

/**
 * @brief Whether a device link is open or still dialing, for the raw-write paths: the driver
 *        holds bytes written during an in-flight dial and flushes them on connect.
 */
bool API::Server::deviceConnected() const
{
  static auto& manager = IO::ConnectionManager::instance();
  return manager.isConnected() || manager.isConnecting();
}

/**
 * @brief Forwards raw bytes to the connected device; negative on failure.
 */
qint64 API::Server::writeToDevice(const QByteArray& data)
{
  static auto& manager = IO::ConnectionManager::instance();
  return manager.writeData(data);
}

/**
 * @brief Runs one API command message through the registry, as a remote-origin caller.
 */
QByteArray API::Server::dispatchCommand(const QByteArray& jsonBytes)
{
  static auto& cmdHandler = API::CommandHandler::instance();
  return cmdHandler.processMessage(jsonBytes, CommandOrigin::Remote);
}

/**
 * @brief Runs one MCP message through the MCP handler for the given session.
 */
QByteArray API::Server::dispatchMcp(const QByteArray& jsonBytes, const QString& sessionId)
{
  static auto& mcpHandler = API::MCPHandler::instance();
  return mcpHandler.processMessage(jsonBytes, sessionId);
}

/**
 * @brief Routes the connection-scoped verbs the registry cannot serve, because they mutate
 *        per-socket state it has no access to. Returns false for every other command.
 */
bool API::Server::routeConnectionCommand(QTcpSocket* socket,
                                         ConnectionState& state,
                                         const QJsonObject& json)
{
  SS_ASSERT(socket != nullptr, return false);
  SS_ASSERT_LOG(!state.sessionId.isEmpty());

  const auto command = json.value(QStringLiteral("command")).toString();
  if (MirrorCommands::isMirrorCommand(command)) {
    handleMirrorCommand(socket, state, json);
    return true;
  }

  if (isStreamCommand(command)) {
    handleStreamCommand(socket, state, json);
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Mirror control (connection-scoped)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves the mirror publisher on first use and wires its outgoing pushes into this
 *        server's session-id-tagged write. Lazy on purpose: the publisher is built after the
 *        pinned singleton order, never from this constructor.
 */
API::MirrorPublisher& API::Server::mirrorPublisher()
{
  static auto& publisher = MirrorPublisher::instance();
  if (!m_mirrorLinked) {
    m_mirrorLinked = true;
    connect(&publisher, &MirrorPublisher::payloadReady, this, &Server::sendMirrorPayload);
  }

  return publisher;
}

/**
 * @brief Turns this connection's per-frame broadcast on or off, mirroring the decision into the
 *        worker thread with the session id carried and verified.
 */
void API::Server::setStreamFrames(QTcpSocket* socket, ConnectionState& state, const bool enabled)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT_LOG(!state.sessionId.isEmpty());

  if (state.streamFrames == enabled)
    return;

  state.streamFrames = enabled;
  auto* worker       = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(worker,
                            "setSocketStreamFrames",
                            Qt::QueuedConnection,
                            Q_ARG(QTcpSocket*, socket),
                            Q_ARG(QString, state.sessionId),
                            Q_ARG(bool, enabled));
}

/**
 * @brief Delivers one mirror push to a subscribed connection; a session id mismatch or a dropped
 *        subscription discards it instead of writing to whatever now owns the socket address.
 */
void API::Server::sendMirrorPayload(QTcpSocket* socket,
                                    const QString& sessionId,
                                    const QByteArray& payload)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT(!payload.isEmpty(), return);

  const auto it = m_connections.constFind(socket);
  if (it == m_connections.constEnd() || it->sessionId != sessionId || !it->mirrorSubscribed)
    return;

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(worker,
                            "writeMirrorPayload",
                            Qt::QueuedConnection,
                            Q_ARG(QTcpSocket*, socket),
                            Q_ARG(QString, sessionId),
                            Q_ARG(QByteArray, payload));
}

/**
 * @brief Dispatches one connection-scoped mirror command and answers on the same socket. The
 *        verbs themselves are publisher-only; the socket, the worker hop and the lazy publisher
 *        link stay here.
 */
void API::Server::handleMirrorCommand(QTcpSocket* socket,
                                      ConnectionState& state,
                                      const QJsonObject& json)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT_LOG(!state.sessionId.isEmpty());

  const auto request   = CommandRequest::fromJson(json);
  const auto setFrames = [this, socket, &state](const bool enabled) {
    setStreamFrames(socket, state, enabled);
  };

  auto& publisher = mirrorPublisher();
  if (request.command == QLatin1String(Mirror::Command::Subscribe))
    sendResponse(
      socket,
      MirrorCommands::subscribe(publisher, socket, state, request, setFrames).toJsonBytes());
  else if (request.command == QLatin1String(Mirror::Command::SetRate))
    sendResponse(socket, MirrorCommands::setRate(publisher, state, request).toJsonBytes());
  else
    sendResponse(socket, MirrorCommands::unsubscribe(publisher, state, request).toJsonBytes());
}

//--------------------------------------------------------------------------------------------------
// Typed stream-block subscription (connection-scoped, spec 0051 M6)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a command name is one of the connection-scoped stream commands. Handled here
 *        rather than in CommandRegistry for the same reason as the mirror verbs: they mutate
 *        per-socket state the registry cannot reach.
 */
bool API::Server::isStreamCommand(const QString& command)
{
  return command == QLatin1String("stream.subscribe")
      || command == QLatin1String("stream.unsubscribe");
}

/**
 * @brief Dispatches one connection-scoped stream command and answers on the same socket.
 */
void API::Server::handleStreamCommand(QTcpSocket* socket,
                                      ConnectionState& state,
                                      const QJsonObject& json)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT_LOG(!state.sessionId.isEmpty());

  const auto request = CommandRequest::fromJson(json);
  if (request.command == QLatin1String("stream.subscribe"))
    sendResponse(socket, streamSubscribe(state, request).toJsonBytes());
  else
    sendResponse(socket, streamUnsubscribe(state, request).toJsonBytes());
}

/**
 * @brief Subscribes this connection to post-transform stream blocks, optionally filtered to a
 *        source id list ("sources": [..]; absent = every stream source).
 */
API::CommandResponse API::Server::streamSubscribe(ConnectionState& state,
                                                  const CommandRequest& request)
{
  SS_ASSERT_LOG(state.authenticated);

  state.streamSources.clear();
  const auto sources = request.params.value(QStringLiteral("sources")).toArray();
  for (const auto& value : sources)
    state.streamSources.insert(value.toInt(-1));

  state.streamSubscribed = true;
  refreshStreamSubscriberFlag();
  state.streamSeq    = 0;
  state.streamMissed = 0;
  state.streamPending.clear();
  Q_EMIT streamSubscribersChanged();

  QJsonArray subscribed;
  for (const int sourceId : std::as_const(state.streamSources))
    subscribed.append(sourceId);

  QJsonObject result;
  result.insert(QStringLiteral("connectionId"), state.sessionId);
  result.insert(QStringLiteral("subscribed"), true);
  result.insert(QStringLiteral("sources"), subscribed);
  result.insert(QStringLiteral("queueDepth"), static_cast<int>(kStreamQueueDepth));
  result.insert(QStringLiteral("encoding"), QStringLiteral("base64:float32le"));
  return CommandResponse::makeSuccess(request.id, result);
}

/**
 * @brief Drops this connection's stream subscription and its pending backlog.
 */
API::CommandResponse API::Server::streamUnsubscribe(ConnectionState& state,
                                                    const CommandRequest& request)
{
  if (!state.streamSubscribed) {
    return CommandResponse::makeError(request.id,
                                      ErrorCode::ExecutionError,
                                      QStringLiteral("This connection holds no stream "
                                                     "subscription"));
  }

  state.streamSubscribed = false;
  refreshStreamSubscriberFlag();
  state.streamPending.clear();
  Q_EMIT streamSubscribersChanged();

  QJsonObject result;
  result.insert(QStringLiteral("subscribed"), false);
  result.insert(QStringLiteral("missed"), static_cast<double>(state.streamMissed));
  return CommandResponse::makeSuccess(request.id, result);
}

/**
 * @brief Queues one post-transform block for every subscribed connection (GUI thread; fed by
 *        each stream worker's queued blockReady). The per-connection queue is bounded: an
 *        overflow drops the OLDEST block and counts it, so a slow subscriber falls behind
 *        visibly instead of growing memory or stalling the worker (R24).
 */
void API::Server::pushStreamBlock(const DataModel::DataBlockPtr& block)
{
  for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
    auto& state = it.value();
    if (!state.streamSubscribed)
      continue;

    if (!state.streamSources.isEmpty() && !state.streamSources.contains(block->sourceId))
      continue;

    state.streamPending.push_back(block);
    while (state.streamPending.size() > kStreamQueueDepth) {
      state.streamPending.pop_front();
      ++state.streamMissed;
    }

    pumpStreamQueue(it.key(), state);
  }
}

/**
 * @brief Sends the next queued block for one connection when no write is in flight: one NDJSON
 *        line per dataset channel, payload base64 float32le, carrying (and clearing) the missed
 *        counter accumulated since the last delivery.
 */
void API::Server::pumpStreamQueue(QTcpSocket* socket, ConnectionState& state)
{
  SS_ASSERT(socket != nullptr, return);

  if (state.streamWriteInFlight || state.streamPending.empty())
    return;

  const auto block = state.streamPending.front();
  state.streamPending.pop_front();
  if (!block)
    return;

  const qint64 t0Ms =
    std::chrono::duration_cast<std::chrono::milliseconds>(block->t0.time_since_epoch()).count();

  QByteArray payload;
  const auto missed  = state.streamMissed;
  state.streamMissed = 0;
  ++state.streamSeq;

  const auto used = static_cast<std::size_t>(block->samples);
  for (const auto& column : block->columns) {
    QByteArray raw;
    raw.resize(static_cast<qsizetype>(used * sizeof(float)));
    auto* out = reinterpret_cast<float*>(raw.data());
    for (std::size_t i = 0; i < used; ++i)
      out[i] = static_cast<float>(column.values[i]);

    QJsonObject entry;
    entry.insert(Keys::SourceId, block->sourceId);
    entry.insert(Keys::UniqueId, column.uniqueId);
    entry.insert(QStringLiteral("seq"), static_cast<double>(state.streamSeq));
    entry.insert(QStringLiteral("missed"), static_cast<double>(missed));
    entry.insert(QStringLiteral("t0Ms"), static_cast<double>(t0Ms));
    entry.insert(QStringLiteral("dtNs"), static_cast<double>(block->dt.count()));
    entry.insert(QStringLiteral("count"), static_cast<double>(used));
    entry.insert(QStringLiteral("data"), QString::fromLatin1(raw.toBase64()));

    QJsonObject line;
    line.insert(QStringLiteral("streamBlock"), entry);
    payload += QJsonDocument(line).toJson(QJsonDocument::Compact) + '\n';
  }

  if (payload.isEmpty())
    return;

  state.streamWriteInFlight = true;
  auto* worker              = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(worker,
                            "writeStreamBlock",
                            Qt::QueuedConnection,
                            Q_ARG(QTcpSocket*, socket),
                            Q_ARG(QString, state.sessionId),
                            Q_ARG(QByteArray, payload));
}

/**
 * @brief Clears the in-flight latch when the worker finished a stream write and sends the next
 *        queued block, which is what paces delivery to the subscriber's actual read rate.
 */
void API::Server::onStreamWriteDone(QTcpSocket* socket, const QString& sessionId)
{
  const auto it = m_connections.find(socket);
  if (it == m_connections.end() || it->sessionId != sessionId)
    return;

  it->streamWriteInFlight = false;
  pumpStreamQueue(socket, it.value());
}

//--------------------------------------------------------------------------------------------------
// Socket lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles incoming data from worker thread. The session id must match the entry for
 *        this socket pointer; a mismatch means the data belongs to an earlier connection whose
 *        freed socket address was reused, so it is dropped instead of corrupting the new one.
 */
void API::Server::onDataReceived(QTcpSocket* socket,
                                 const QString& sessionId,
                                 const QByteArray& data)
{
  if (!enabled() || data.isEmpty() || !socket)
    return;

  m_reception.consumeBytes(socket, sessionId, data);
}

/**
 * @brief Resolves one connection's mutable state, or null once the entry is gone. The reception
 *        machine calls this instead of holding a reference across a dispatch: a command handler
 *        can spin an event loop, and the queued disconnect that runs there erases the entry while
 *        the receive loop is still inside it (spec 0075 I1).
 */
API::ConnectionState* API::Server::stateFor(QTcpSocket* socket, const QString& sessionId)
{
  SS_ASSERT(socket != nullptr, return nullptr);

  const auto it = m_connections.find(socket);
  if (it == m_connections.end() || it->sessionId != sessionId)
    return nullptr;

  return &it.value();
}

/**
 * @brief Accepts new incoming TCP connections, from whichever listener signalled: the loopback
 *        pair are two QTcpServers feeding one connection table.
 */
void API::Server::acceptConnection()
{
  auto* listener = qobject_cast<QTcpServer*>(sender());
  SS_ASSERT(listener != nullptr, return);

  auto* socket = listener->nextPendingConnection();
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

  ConnectionState state;
  state.sessionId   = QString::number(s_nextSessionId.fetchAndAddRelaxed(1));
  state.peerAddress = socket->peerAddress().toString();
  state.peerPort    = socket->peerPort();

  const auto stale = m_connections.constFind(socket);
  if (stale != m_connections.constEnd()) {
    static auto& mcpHandler = MCPHandler::instance();
    mcpHandler.clearSession(stale->sessionId);
  }

  state.authenticated = !(m_externalConnections && !socket->peerAddress().isLoopback());
  m_connections.insert(socket, state);

  qInfo() << "[API] New client connected:" << state.peerAddress << ":" << state.peerPort
          << "- Total clients:" << m_connections.size();

  socket->setParent(nullptr);
  socket->moveToThread(&m_workerThread);

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(worker,
                            "addSocket",
                            Qt::QueuedConnection,
                            Q_ARG(QTcpSocket*, socket),
                            Q_ARG(QString, state.sessionId));
}

/**
 * @brief Handles socket-level errors from connected clients.
 */
void API::Server::onErrorOccurred(const QAbstractSocket::SocketError socketError)
{
  qWarning() << socketError;
}

/**
 * @brief Clears connection state when the worker reports socket removal. The removal is only
 *        honored when the session id matches: the queued notification can arrive after a new
 *        connection reused the freed socket's address, and removing that entry would leave the
 *        new client permanently unanswered (macOS CI hang, 2026-07).
 */
void API::Server::onSocketDisconnected(QTcpSocket* socket, const QString& sessionId)
{
  if (!socket)
    return;

  const auto it = m_connections.constFind(socket);
  if (it != m_connections.constEnd() && it->sessionId == sessionId) {
    static auto& mcpHandler = MCPHandler::instance();
    mcpHandler.clearSession(sessionId);
    if (it->mirrorSubscribed)
      mirrorPublisher().unsubscribe(sessionId);

    qInfo() << "[API] Client disconnected (via worker):"
            << "- Remaining clients:" << (m_connections.size() - 1);

    const bool wasStreaming = it->streamSubscribed;
    m_connections.erase(it);
    refreshStreamSubscriberFlag();
    if (wasStreaming)
      Q_EMIT streamSubscribersChanged();
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
