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

#include "API/Server/ServerWorker.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QThread>
#include <utility>

#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Broadcast-lane cap per socket: over-cap (non-reading) clients are skipped, bounding the buffer
constexpr qint64 kMaxApiPendingWriteBytes = 16 * 1024 * 1024;

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
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

//--------------------------------------------------------------------------------------------------
// Socket write budget
//--------------------------------------------------------------------------------------------------

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
