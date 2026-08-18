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
#include <QRandomGenerator>
#include <QSet>
#include <utility>

#include "API/CommandHandler.h"
#include "API/CommandProtocol.h"
#include "API/MCPHandler.h"
#include "API/MCPProtocol.h"
#include "API/Mirror/MirrorPublisher.h"
#include "DataModel/FrameBuilder.h"
#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"
#include "SSAssert.h"

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
constexpr int kMinAuthTokenChars       = 32;

// Per-subscriber stream backlog before the oldest block is dropped and counted (spec 0051 R24)
constexpr std::size_t kStreamQueueDepth = 8;

// Broadcast-lane cap per socket: over-cap (non-reading) clients are skipped, bounding the buffer
constexpr qint64 kMaxApiPendingWriteBytes = 16 * 1024 * 1024;

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
  SS_ASSERT(!data.isEmpty(), return false);
  SS_ASSERT(maxDepth > 0, return true);

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
 * @brief Constructs the worker over the shared frame-consumer queue plumbing.
 */
API::ServerWorker::ServerWorker(moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>* queue,
                                std::atomic<bool>* enabled,
                                std::atomic<size_t>* queueSize)
  : DataModel::FrameConsumerWorker<DataModel::DataBlockPtr>(queue, enabled, queueSize)
  , m_droppedBroadcasts(0)
{}

/**
 * @brief Destructor
 */
API::ServerWorker::~ServerWorker() = default;

/**
 * @brief True while @p socket has room for another broadcast; an over-cap socket is skipped and
 *        counted, warned once per socket so a wedged client is visible without per-batch log spam
 *        and a second client's stall is not swallowed by the first one's warning. Skipping is
 *        self-healing: a client that resumes reading drains its backlog and rejoins the broadcast.
 */
bool API::ServerWorker::underWriteCap(QTcpSocket* socket)
{
  SS_ASSERT(socket != nullptr, return false);

  if (socket->bytesToWrite() <= kMaxApiPendingWriteBytes) [[likely]]
    return true;

  ++m_droppedBroadcasts;
  if (!m_warnedSockets.contains(socket)) {
    m_warnedSockets.insert(socket);
    qWarning() << "[API] Client" << socket->peerAddress().toString() << ":" << socket->peerPort()
               << "is not reading the broadcast stream; dropping its broadcasts while the socket"
               << "backlog exceeds" << kMaxApiPendingWriteBytes << "bytes";
  }

  return false;
}

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
  SS_ASSERT_LOG(QThread::currentThread() == thread());

  for (auto it = m_sockets.keyBegin(); it != m_sockets.keyEnd(); ++it) {
    auto* socket = *it;
    if (socket) {
      socket->abort();
      socket->deleteLater();
    }
  }

  m_sockets.clear();
  m_mutedSockets.clear();
  m_warnedSockets.clear();

  Q_EMIT clientCountChanged(0);
}

/**
 * @brief Adds a socket to the worker thread, keyed to its immutable session id (socket pointers
 *        get reused across connect cycles, so every queued hop carries it). Bytes that landed
 *        before this queued adoption ran are drained here: the notifier can buffer them and
 *        emit readyRead with zero receivers, and it never re-fires for buffered data.
 */
void API::ServerWorker::addSocket(QTcpSocket* socket, const QString& sessionId)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT_LOG(!sessionId.isEmpty());
  SS_ASSERT_LOG(socket->state() != QAbstractSocket::UnconnectedState);

  m_sockets.insert(socket, sessionId);
  m_mutedSockets.remove(socket);
  m_warnedSockets.remove(socket);
  connect(socket, &QTcpSocket::readyRead, this, &ServerWorker::onSocketReadyRead);
  connect(socket, &QTcpSocket::disconnected, this, &ServerWorker::onSocketDisconnected);

  if (socket->bytesAvailable() > 0)
    Q_EMIT dataReceived(socket, sessionId, socket->readAll());

  Q_EMIT clientCountChanged(m_sockets.count());
}

/**
 * @brief Removes a socket from the worker thread. Reads the session id from the live socket
 *        entry before scheduling deletion so the removal notification identifies the exact
 *        connection generation, never a later pointer reuse.
 */
void API::ServerWorker::removeSocket(QTcpSocket* socket)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT(m_sockets.contains(socket), {
    socket->deleteLater();
    return;
  });

  const QString sessionId = m_sockets.value(socket);
  m_sockets.remove(socket);
  m_mutedSockets.remove(socket);
  m_warnedSockets.remove(socket);

  Q_EMIT socketRemoved(socket, sessionId);
  Q_EMIT clientCountChanged(m_sockets.count());

  if (socket)
    socket->deleteLater();
}

/**
 * @brief Writes raw data to all connected sockets (worker thread); a socket over the outbound
 *        write cap is skipped so a non-reading client cannot grow its buffer without bound.
 */
void API::ServerWorker::writeRawData(const QByteArray& data)
{
  if (data.isEmpty() || m_sockets.isEmpty())
    return;

  QJsonObject object;
  object.insert(QStringLiteral("data"), QString::fromUtf8(data.toBase64()));
  const QJsonDocument document(object);
  const auto json = document.toJson(QJsonDocument::Compact) + "\n";

  for (auto it = m_sockets.keyBegin(); it != m_sockets.keyEnd(); ++it) {
    auto* socket = *it;
    if (socket && socket->isWritable() && underWriteCap(socket))
      socket->write(json);
  }
}

/**
 * @brief Broadcasts a lifecycle event JSON object to all connected API clients.
 */
void API::ServerWorker::broadcastEvent(const QJsonObject& event)
{
  SS_ASSERT_LOG(!event.isEmpty());

  if (m_sockets.isEmpty())
    return;

  const QJsonDocument document(event);
  const auto json = document.toJson(QJsonDocument::Compact) + "\n";

  for (auto it = m_sockets.keyBegin(); it != m_sockets.keyEnd(); ++it) {
    auto* socket = *it;
    if (socket && socket->isWritable())
      socket->write(json);
  }
}

/**
 * @brief Handles incoming data from a socket (worker thread), tagging it with the session id
 *        so the main thread can reject data attributed to a stale pointer generation.
 */
void API::ServerWorker::onSocketReadyRead()
{
  auto* socket = qobject_cast<QTcpSocket*>(sender());
  if (socket && m_sockets.contains(socket))
    Q_EMIT dataReceived(socket, m_sockets.value(socket), socket->readAll());
}

/**
 * @brief Writes data to a specific socket (worker thread); dropped when the session id no
 *        longer matches, which means the target connection is gone.
 */
void API::ServerWorker::writeToSocket(QTcpSocket* socket,
                                      const QString& sessionId,
                                      const QByteArray& data)
{
  SS_ASSERT(!data.isEmpty(), return);

  if (socket && m_sockets.value(socket) == sessionId && socket->isWritable())
    socket->write(data);
}

/**
 * @brief Writes one mirror push (worker thread). Unlike a command response, this is producer-paced
 *        at display-tick rate with nothing on the wire asking for it, so a viewer that stops
 *        reading would grow the socket buffer without bound: the broadcast cap applies here.
 */
void API::ServerWorker::writeMirrorPayload(QTcpSocket* socket,
                                           const QString& sessionId,
                                           const QByteArray& data)
{
  SS_ASSERT(!data.isEmpty(), return);

  if (socket && m_sockets.value(socket) == sessionId && socket->isWritable()
      && underWriteCap(socket))
    socket->write(data);
}

/**
 * @brief Writes one typed stream block and acknowledges it, which is what paces the subscriber:
 *        the server only sends the next block after this ack, so a slow reader accumulates the
 *        bounded per-connection queue (drop-oldest + missed count) instead of the socket buffer.
 *        A session mismatch still acks so the sender's in-flight latch cannot wedge.
 */
void API::ServerWorker::writeStreamBlock(QTcpSocket* socket,
                                         const QString& sessionId,
                                         const QByteArray& data)
{
  SS_ASSERT(!data.isEmpty(), return);

  if (socket && m_sockets.value(socket) == sessionId && socket->isWritable())
    socket->write(data);

  Q_EMIT streamWriteDone(socket, sessionId);
}

/**
 * @brief Disconnects a socket (worker thread); a session id mismatch means the connection
 *        already ended, so the request is ignored instead of hitting an unrelated socket.
 */
void API::ServerWorker::disconnectSocket(QTcpSocket* socket, const QString& sessionId)
{
  if (socket && m_sockets.value(socket) == sessionId) {
    socket->flush();
    socket->disconnectFromHost();
  }
}

/**
 * @brief Turns the per-frame broadcast off (or back on) for one socket, dropped when the session
 *        id no longer matches so a reused pointer can never mute an unrelated client. Mirror
 *        viewers are the only callers: a client that never subscribes stays on the stream.
 */
void API::ServerWorker::setSocketStreamFrames(QTcpSocket* socket,
                                              const QString& sessionId,
                                              const bool enabled)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT_LOG(!sessionId.isEmpty());

  if (m_sockets.value(socket) != sessionId)
    return;

  if (enabled)
    m_mutedSockets.remove(socket);
  else
    m_mutedSockets.insert(socket);
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
 * @brief Processes frames by serializing them to JSON and writing to sockets. Sockets whose
 *        client opted out of the stream (mirror viewers) or whose outbound backlog is over the
 *        write cap are skipped, and the serialization is skipped entirely when no socket is
 *        eligible for this batch.
 */
void API::ServerWorker::processItems(const std::vector<DataModel::DataBlockPtr>& items)
{
  if (items.empty() || m_sockets.isEmpty())
    return;

  QVector<QTcpSocket*> targets;
  targets.reserve(m_sockets.size());
  for (auto it = m_sockets.keyBegin(); it != m_sockets.keyEnd(); ++it) {
    auto* socket = *it;
    if (socket && socket->isWritable() && !m_mutedSockets.contains(socket) && underWriteCap(socket))
      targets.append(socket);
  }

  if (targets.isEmpty())
    return;

  QJsonArray array;
  for (const auto& block : items) {
    if (!block || block->samples <= 0)
      continue;

    const auto tpl = m_templates.find(block->sourceId);
    if (tpl == m_templates.end())
      continue;

    for (qsizetype i = 0; i < block->samples; ++i) {
      if (array.size() >= kMaxBroadcastSamples)
        break;

      DataModel::apply_block_sample(tpl->second, *block, i);

      QJsonObject object;
      object.insert(QStringLiteral("data"), serialize(tpl->second.frame));
      array.append(object);
    }

    if (array.size() >= kMaxBroadcastSamples)
      break;
  }

  if (array.isEmpty())
    return;

  QJsonObject object;
  object.insert(QStringLiteral("frames"), array);
  const QJsonDocument document(object);
  const auto json = document.toJson(QJsonDocument::Compact) + "\n";

  for (auto* socket : std::as_const(targets))
    socket->write(json);
}

/**
 * @brief Stores one source's structure and rebuilds its uniqueId lookup. Queued from the GUI, so
 *        the assignment never races processItems().
 */
void API::ServerWorker::setTemplateFrame(int sourceId, const DataModel::Frame& frame)
{
  DataModel::bind_frame_template(m_templates[sourceId], frame);
}

//--------------------------------------------------------------------------------------------------
// Server implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the API server.
 */
API::Server::Server()
  : DataModel::FrameConsumer<DataModel::DataBlockPtr>(
      {.queueCapacity = 2048, .flushThreshold = 512, .timerIntervalMs = 1000})
  , m_clientCount(0)
  , m_enabled(false)
  , m_mirrorLinked(false)
  , m_externalConnections(false)
  , m_deviceWriteConsent(DeviceWriteConsent::Unset)
  , m_anyStreamSubscriber(false)
{
  m_externalConnections = m_settings.value("API/ExternalConnections", false).toBool();
  m_authToken           = m_settings.value("API/AuthToken").toString();

  if (m_settings.value("API/DeviceWriteConsent", false).toBool())
    m_deviceWriteConsent = DeviceWriteConsent::Granted;

  if (qEnvironmentVariableIntValue("SERIAL_STUDIO_API_AUTO_CONSENT") != 0)
    m_deviceWriteConsent = DeviceWriteConsent::Granted;

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
          &Server::onSocketDisconnected,
          Qt::QueuedConnection);
  connect(
    worker, &ServerWorker::streamWriteDone, this, &Server::onStreamWriteDone, Qt::QueuedConnection);

  connect(&m_server, &QTcpServer::newConnection, this, &Server::acceptConnection);

  static auto& commandHandler = API::CommandHandler::instance();
  (void)commandHandler;
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
 * @brief Enables or disables the TCP API server.
 */
void API::Server::setEnabled(const bool enabled)
{
  SS_ASSERT(m_worker != nullptr, return);

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
    if (m_mirrorLinked)
      mirrorPublisher().clearSubscribers();

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
    ensureAuthToken();
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
    ensureAuthToken();

  if (!m_enabled)
    return;

  m_server.close();
  m_connections.clear();
  if (m_mirrorLinked)
    mirrorPublisher().clearSubscribers();

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
 * @brief Pins a caller-supplied auth token, for provisioning a headless machine from the command
 *        line. Refuses anything shorter than 32 hex characters rather than quietly weakening the
 *        credential that guards every non-loopback connection.
 */
bool API::Server::setAuthToken(const QString& token)
{
  const auto trimmed = token.trimmed().toLower();
  if (trimmed.size() < kMinAuthTokenChars)
    return false;

  for (const QChar character : trimmed)
    if (!character.isDigit() && (character < QLatin1Char('a') || character > QLatin1Char('f')))
      return false;

  if (m_authToken == trimmed)
    return true;

  m_authToken = trimmed;
  m_settings.setValue("API/AuthToken", m_authToken);
  Q_EMIT authTokenChanged();
  return true;
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
 * @brief Gates API-originated device writes behind a one-time user consent prompt. Headless
 *        runs cannot show the prompt, so consent must be pre-granted there via the
 *        SERIAL_STUDIO_API_AUTO_CONSENT env var or the persisted setting (used by CI).
 */
bool API::Server::authorizeDeviceWrite()
{
  if (m_deviceWriteConsent == DeviceWriteConsent::Granted)
    return true;

  if (m_deviceWriteConsent == DeviceWriteConsent::Denied)
    return false;

  if (qApp->platformName() == QLatin1String("offscreen")) {
    m_deviceWriteConsent = DeviceWriteConsent::Denied;
    qWarning() << "[API] Device write denied: no GUI to prompt for consent. Set "
                  "SERIAL_STUDIO_API_AUTO_CONSENT=1 to allow API device writes in headless mode.";
    return false;
  }

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
 * @brief Gates remote-origin device-write commands behind the consent prompt; commands that
 *        never touch the hardware always pass. Keeps the command path consistent with the
 *        raw byte paths, which run the same gate.
 */
bool API::Server::authorizeRemoteCommand(const QString& command)
{
  static const QSet<QString> kControlScriptOnlyCommands = {
    QStringLiteral("system.exec"),
    QStringLiteral("system.kill"),
    QStringLiteral("system.runningProcesses"),
  };

  if (kControlScriptOnlyCommands.contains(command))
    return false;

  static const QSet<QString> kDeviceWriteCommands = {
    QStringLiteral("io.writeData"),
    QStringLiteral("io.ble.writeCharacteristic"),
    QStringLiteral("console.send"),
  };

  if (!kDeviceWriteCommands.contains(command))
    return true;

  return authorizeDeviceWrite();
}

/**
 * @brief Consumes the first line as a {"type":"auth","token":...} handshake before commands.
 */
void API::Server::handleAuthHandshake(QTcpSocket* socket,
                                      ConnectionState& state,
                                      const QByteArray& data)
{
  SS_ASSERT(socket != nullptr, return);

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

  state.authenticated = true;
  qInfo() << "[API] Client authenticated:" << state.peerAddress << ":" << state.peerPort;

  QJsonObject result;
  result[QStringLiteral("authenticated")] = true;
  sendResponseToSocket(socket, CommandResponse::makeSuccess(QString(), result).toJsonBytes());

  if (!state.buffer.isEmpty()) {
    const QByteArray pipelined = state.buffer;
    state.buffer.clear();
    onDataReceived(socket, state.sessionId, pipelined);
  }
}

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
// Server: data reception helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends a response to a specific client socket via the worker thread, tagged with the
 *        connection's session id so a reused socket pointer can never receive it.
 */
void API::Server::sendResponseToSocket(QTcpSocket* socket, const QByteArray& response)
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
  sendResponseToSocket(socket, response);

  auto* worker = static_cast<ServerWorker*>(m_worker);
  QMetaObject::invokeMethod(worker,
                            "disconnectSocket",
                            Qt::QueuedConnection,
                            Q_ARG(QTcpSocket*, socket),
                            Q_ARG(QString, state.sessionId));
  state.buffer.clear();
}

/**
 * @brief Validates rate limits and buffer capacity for incoming data.
 */
bool API::Server::validateRateLimits(QTcpSocket* socket,
                                     ConnectionState& state,
                                     const QByteArray& data)
{
  SS_ASSERT(socket != nullptr, return false);
  SS_ASSERT(!data.isEmpty(), return false);

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

    disconnectClient(
      socket, state, ErrorCode::ExecutionError, QStringLiteral("API rate limit exceeded"));
    return false;
  }

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
  SS_ASSERT(socket != nullptr, return false);
  SS_ASSERT(!jsonBytes.isEmpty(), return false);

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
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT(!jsonBytes.isEmpty(), return);

  if (!validateJsonMessage(socket, state, jsonBytes))
    return;

  if (MCP::isMCPMessage(jsonBytes)) {
    static auto& mcpHandler = API::MCPHandler::instance();
    const auto response     = mcpHandler.processMessage(jsonBytes, state.sessionId);

    if (!response.isEmpty())
      sendResponseToSocket(socket, response);

    return;
  }

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

  if (type == MessageType::Raw) {
    processRawJsonCommand(socket, state, json);
    return;
  }

  if (type == MessageType::Command
      && isMirrorCommand(json.value(QStringLiteral("command")).toString())) {
    handleMirrorCommand(socket, state, json);
    return;
  }

  if (type == MessageType::Command
      && isStreamCommand(json.value(QStringLiteral("command")).toString())) {
    handleStreamCommand(socket, state, json);
    return;
  }

  static auto& cmdHandler = API::CommandHandler::instance();
  sendResponseToSocket(socket, cmdHandler.processMessage(jsonBytes, CommandOrigin::Remote));
}

/**
 * @brief Processes a JSON "raw" command that forwards base64 data to the device.
 */
void API::Server::processRawJsonCommand(QTcpSocket* socket,
                                        ConnectionState& state,
                                        const QJsonObject& json)
{
  SS_ASSERT(socket != nullptr, return);

  const QString id = json.value(QStringLiteral("id")).toString();

  if (!json.contains(QStringLiteral("data"))) {
    sendResponseToSocket(
      socket,
      CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: data"))
        .toJsonBytes());
    return;
  }

  const QString dataStr    = json.value(QStringLiteral("data")).toString();
  const QByteArray rawData = QByteArray::fromBase64(dataStr.toUtf8());
  if (rawData.isEmpty() && !dataStr.isEmpty()) {
    sendResponseToSocket(
      socket,
      CommandResponse::makeError(id, ErrorCode::InvalidParam, QStringLiteral("Invalid base64 data"))
        .toJsonBytes());
    return;
  }

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

  if (!authorizeDeviceWrite()) {
    sendResponseToSocket(socket,
                         CommandResponse::makeError(id,
                                                    ErrorCode::ExecutionError,
                                                    QStringLiteral("Device write denied by user"))
                           .toJsonBytes());
    return;
  }

  static auto& manager = IO::ConnectionManager::instance();
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
  SS_ASSERT(socket != nullptr, return);

  auto& buffer       = state.buffer;
  const auto trimmed = buffer.trimmed();

  const char firstChar = trimmed.isEmpty() ? '\0' : trimmed.at(0);
  if (firstChar == '{' || firstChar == '[') {
    processBufferedJson(socket, state, trimmed);
    return;
  }

  if (buffer.size() > kMaxApiRawBytes) {
    qWarning() << "[API] Raw buffer size limit exceeded:" << state.peerAddress << ":"
               << state.peerPort << "- Buffer size:" << buffer.size()
               << "- Limit:" << kMaxApiRawBytes << "- Disconnecting client";

    disconnectClient(
      socket, state, ErrorCode::ExecutionError, QStringLiteral("Raw payload exceeds size limit"));
    return;
  }

  if (!authorizeDeviceWrite()) {
    buffer.clear();
    return;
  }

  static auto& manager = IO::ConnectionManager::instance();
  const qint64 written = manager.writeData(buffer);
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
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT(!trimmed.isEmpty(), return);

  auto& buffer = state.buffer;

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
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT(!trimmedLine.isEmpty(), return);

  handleJsonMessage(socket, state, trimmedLine);
}

/**
 * @brief Processes a non-JSON raw line from the buffer.
 */
void API::Server::processRawLine(QTcpSocket* socket, ConnectionState& state, const QByteArray& line)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT(!line.isEmpty(), return);

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
    QMetaObject::invokeMethod(worker,
                              "disconnectSocket",
                              Qt::QueuedConnection,
                              Q_ARG(QTcpSocket*, socket),
                              Q_ARG(QString, state.sessionId));
    return;
  }

  if (!authorizeDeviceWrite()) {
    sendResponseToSocket(socket,
                         CommandResponse::makeError(QString(),
                                                    ErrorCode::ExecutionError,
                                                    QStringLiteral("Device write denied by user"))
                           .toJsonBytes());
    return;
  }

  static auto& manager = IO::ConnectionManager::instance();
  const qint64 written = manager.writeData(line);
  if (written < 0) [[unlikely]]
    qWarning() << "[API] writeData() failed for raw line"
               << "-- data not sent to device";
}

//--------------------------------------------------------------------------------------------------
// Server: mirror control (connection-scoped)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a command name is one of the connection-scoped mirror commands. They are handled
 *        here rather than in CommandRegistry because they mutate per-socket state the registry has
 *        no access to, the same reason the MCP branch sits at this level.
 */
bool API::Server::isMirrorCommand(const QString& command)
{
  return command == QLatin1String(Mirror::Command::Subscribe)
      || command == QLatin1String(Mirror::Command::SetRate)
      || command == QLatin1String(Mirror::Command::Unsubscribe);
}

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
 * @brief Dispatches one connection-scoped mirror command and answers on the same socket.
 */
void API::Server::handleMirrorCommand(QTcpSocket* socket,
                                      ConnectionState& state,
                                      const QJsonObject& json)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT_LOG(!state.sessionId.isEmpty());

  const auto request = CommandRequest::fromJson(json);

  if (request.command == QLatin1String(Mirror::Command::Subscribe))
    sendResponseToSocket(socket, mirrorSubscribe(socket, state, request).toJsonBytes());
  else if (request.command == QLatin1String(Mirror::Command::SetRate))
    sendResponseToSocket(socket, mirrorSetRate(state, request).toJsonBytes());
  else
    sendResponseToSocket(socket, mirrorUnsubscribe(state, request).toJsonBytes());
}

/**
 * @brief Subscribes this connection to the mirror and, by default, opts it out of the per-frame
 *        broadcast: at capture rates that stream would disconnect the viewer on the byte cap long
 *        before the network noticed, which is why subscribe is the first request a viewer sends.
 */
API::CommandResponse API::Server::mirrorSubscribe(QTcpSocket* socket,
                                                  ConnectionState& state,
                                                  const CommandRequest& request)
{
  SS_ASSERT(socket != nullptr,
            return CommandResponse::makeError(
              request.id, ErrorCode::ExecutionError, QStringLiteral("No connection")));
  SS_ASSERT_LOG(state.authenticated);

  const int version = request.params.value(QStringLiteral("wireVersion")).toInt(0);
  if (version != Mirror::kWireVersion) {
    return CommandResponse::makeError(
      request.id,
      QLatin1String(Mirror::ErrorCode::VersionMismatch),
      QStringLiteral("This instance speaks mirror wire version %1, the client asked for %2")
        .arg(QString::number(Mirror::kWireVersion), QString::number(version)));
  }

  const auto hzValue = request.params.value(QStringLiteral("hz"));
  const int hz       = hzValue.isUndefined() ? Mirror::kHzDefault : hzValue.toInt(0);
  if (hz < Mirror::kHzMin || hz > Mirror::kHzMax) {
    return CommandResponse::makeError(
      request.id,
      QLatin1String(Mirror::ErrorCode::RateOutOfRange),
      QStringLiteral("Mirror rate %1 Hz is outside the supported range").arg(QString::number(hz)));
  }

  const int precision = request.params.value(QStringLiteral("precision")).toInt(0);
  if (precision < Mirror::kPrecisionMin || precision > Mirror::kPrecisionMax) {
    return CommandResponse::makeError(
      request.id,
      ErrorCode::InvalidParam,
      QStringLiteral("Value precision %1 is outside 0 (full) to 17 significant digits")
        .arg(QString::number(precision)));
  }

  auto& publisher = mirrorPublisher();
  if (!publisher.subscribe(socket, state.sessionId, hz, precision)) {
    return CommandResponse::makeError(
      request.id,
      QLatin1String(Mirror::ErrorCode::ViewerLimit),
      QStringLiteral("This instance is not accepting more mirror viewers"));
  }

  state.mirrorSubscribed = true;
  state.mirrorHz         = hz;
  state.mirrorPrecision  = precision;
  setStreamFrames(socket, state, request.params.value(QStringLiteral("frames")).toBool(false));

  auto result = publisher.info();
  result.insert(QStringLiteral("connectionId"), state.sessionId);
  result.insert(QStringLiteral("hz"), hz);
  result.insert(QStringLiteral("effectiveHz"), publisher.effectiveHz(hz));
  result.insert(QStringLiteral("frames"), state.streamFrames);
  result.insert(QStringLiteral("precision"), precision);
  return CommandResponse::makeSuccess(request.id, result);
}

/**
 * @brief Renegotiates this connection's mirror cadence. An out-of-range rate is refused rather
 *        than clamped: a silent clamp hides a misconfigured viewer.
 */
API::CommandResponse API::Server::mirrorSetRate(ConnectionState& state,
                                                const CommandRequest& request)
{
  if (!state.mirrorSubscribed) {
    return CommandResponse::makeError(request.id,
                                      QLatin1String(Mirror::ErrorCode::NotSubscribed),
                                      QStringLiteral("This connection holds no mirror "
                                                     "subscription"));
  }

  const int hz = request.params.value(QStringLiteral("hz")).toInt(0);
  if (hz < Mirror::kHzMin || hz > Mirror::kHzMax) {
    return CommandResponse::makeError(
      request.id,
      QLatin1String(Mirror::ErrorCode::RateOutOfRange),
      QStringLiteral("Mirror rate %1 Hz is outside the supported range").arg(QString::number(hz)));
  }

  auto& publisher = mirrorPublisher();
  if (!publisher.setRate(state.sessionId, hz)) {
    return CommandResponse::makeError(request.id,
                                      QLatin1String(Mirror::ErrorCode::NotSubscribed),
                                      QStringLiteral("This connection holds no mirror "
                                                     "subscription"));
  }

  state.mirrorHz = hz;

  QJsonObject result;
  result.insert(QStringLiteral("hz"), hz);
  result.insert(QStringLiteral("effectiveHz"), publisher.effectiveHz(hz));
  return CommandResponse::makeSuccess(request.id, result);
}

/**
 * @brief Stops this connection's mirror. The frame stream stays off: only mirror.subscribe ever
 *        changes that flag, so unsubscribing cannot reopen the firehose on a slow reader.
 */
API::CommandResponse API::Server::mirrorUnsubscribe(ConnectionState& state,
                                                    const CommandRequest& request)
{
  if (!state.mirrorSubscribed) {
    return CommandResponse::makeError(request.id,
                                      QLatin1String(Mirror::ErrorCode::NotSubscribed),
                                      QStringLiteral("This connection holds no mirror "
                                                     "subscription"));
  }

  mirrorPublisher().unsubscribe(state.sessionId);
  state.mirrorSubscribed = false;

  QJsonObject result;
  result.insert(QStringLiteral("mirrorSubscribed"), false);
  return CommandResponse::makeSuccess(request.id, result);
}

//--------------------------------------------------------------------------------------------------
// Server: typed stream-block subscription (connection-scoped, spec 0051 M6)
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
    sendResponseToSocket(socket, streamSubscribe(state, request).toJsonBytes());
  else
    sendResponseToSocket(socket, streamUnsubscribe(state, request).toJsonBytes());
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
// Server: data reception & dispatch
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

  const auto it = m_connections.find(socket);
  if (it == m_connections.end() || it->sessionId != sessionId)
    return;

  auto& state = *it;

  if (!validateRateLimits(socket, state, data))
    return;

  if (!state.authenticated) {
    handleAuthHandshake(socket, state, data);
    return;
  }

  state.buffer.append(data);
  auto& buffer = state.buffer;

  constexpr int kMaxBufferIterations = 10000;
  int bufferIterations               = 0;
  while (!buffer.isEmpty() && bufferIterations < kMaxBufferIterations) {
    ++bufferIterations;

    const int newlineIndex = buffer.indexOf('\n');

    if (newlineIndex < 0) {
      processNoNewlineBuffer(socket, state);
      return;
    }

    int bodyLen = newlineIndex;
    if (bodyLen > 0 && buffer.at(bodyLen - 1) == '\r')
      --bodyLen;

    const QByteArray line = buffer.left(bodyLen);
    buffer.remove(0, newlineIndex + 1);

    const auto trimmedLine = line.trimmed();
    if (trimmedLine.isEmpty())
      continue;

    if (trimmedLine.at(0) == '{' || trimmedLine.at(0) == '[')
      processJsonLine(socket, state, trimmedLine);
    else
      processRawLine(socket, state, line);
  }

  if (bufferIterations >= kMaxBufferIterations && !buffer.isEmpty()) [[unlikely]] {
    qWarning() << "[API] Buffer processing iteration limit reached:" << state.peerAddress << ":"
               << state.peerPort << "- Disconnecting client";

    disconnectClient(
      socket, state, ErrorCode::ExecutionError, QStringLiteral("API message flood limit exceeded"));
  }
}

/**
 * @brief Accepts new incoming TCP connections.
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
