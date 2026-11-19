/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "Platform/KioskControl.h"

#include <limits>
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>

#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kRetryIntervalMs = 2000;
static constexpr int kMaxLinesPerRead = 256;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires the socket to the retry timer. A failed dial reports from inside connectToServer()
 *        and Qt refuses a redial from that report, so every redial goes through the timer. The
 *        socket's own read buffer is capped as well, so a peer that floods short lines is held
 *        back by the operating system instead of filling this process.
 */
Platform::KioskControl::KioskControl(QObject* parent)
  : QObject(parent), m_identity(QStringLiteral("dashboard"))
{
  m_retry.setSingleShot(true);
  m_retry.setInterval(kRetryIntervalMs);
  m_drain.setSingleShot(true);
  m_drain.setInterval(0);
  m_socket.setReadBufferSize(kMaxLineBytes + 1);

  connect(&m_drain, &QTimer::timeout, this, &KioskControl::readPending);

  connect(&m_retry, &QTimer::timeout, this, &KioskControl::dial);
  connect(&m_socket, &QLocalSocket::connected, this, &KioskControl::onConnected);
  connect(&m_socket, &QLocalSocket::readyRead, this, &KioskControl::readPending);
  connect(&m_socket, &QLocalSocket::disconnected, this, &KioskControl::scheduleRetry);
  connect(&m_socket, &QLocalSocket::errorOccurred, this, &KioskControl::scheduleRetry);
}

/**
 * @brief Detaches the socket before the members go: a socket closing in its destructor would
 *        otherwise reach scheduleRetry(), and member order would decide whether the timer it
 *        starts still exists.
 */
Platform::KioskControl::~KioskControl()
{
  m_socket.disconnect(this);
  m_drain.stop();
  m_retry.stop();
}

//--------------------------------------------------------------------------------------------------
// Framing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Moves the first complete line of @p buffer into @p line, terminator dropped. Overflow
 *        means a line longer than kMaxLineBytes, terminated or not, so the caller never has to
 *        trust the size a peer chose.
 */
Platform::KioskControl::Frame Platform::KioskControl::takeLine(QByteArray& buffer, QByteArray& line)
{
  const qsizetype end = buffer.indexOf('\n');
  if (end < 0)
    return buffer.size() > kMaxLineBytes ? Frame::Overflow : Frame::None;

  if (end > kMaxLineBytes)
    return Frame::Overflow;

  line = buffer.left(end);
  buffer.remove(0, end + 1);
  return Frame::Line;
}

//--------------------------------------------------------------------------------------------------
// State
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true while the launcher's server holds this client's connection.
 */
bool Platform::KioskControl::isLinked() const
{
  return m_socket.state() == QLocalSocket::ConnectedState;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Overrides the 2 s redial period; the unit tests shorten it.
 */
void Platform::KioskControl::setRetryInterval(int msec)
{
  SS_ASSERT(msec > 0, return);

  m_retry.setInterval(msec);
}

/**
 * @brief Names the window this client stands for in its greeting; the main window keeps the
 *        default, "dashboard". An empty name is refused, since the launcher keys on it.
 */
void Platform::KioskControl::setIdentity(const QString& id)
{
  SS_ASSERT(!id.isEmpty(), return);

  m_identity = id;
}

/**
 * @brief Names the launcher's server and dials it; an empty name leaves the channel closed.
 */
void Platform::KioskControl::start(const QString& server_name)
{
  if (server_name.isEmpty() || !m_serverName.isEmpty())
    return;

  m_serverName = server_name;
  dial();
}

/**
 * @brief Sends the state event: the screen the window is on, whether it is the active one, and
 *        what it shows now when @p title is given, so a launcher can label it.
 */
void Platform::KioskControl::sendState(const QString& screen, bool active, const QString& title)
{
  QJsonObject message;
  message.insert(QStringLiteral("event"), QStringLiteral("state"));
  message.insert(QStringLiteral("screen"), screen);
  message.insert(QStringLiteral("active"), active);
  if (!title.isEmpty())
    message.insert(QStringLiteral("title"), title);

  sendLine(message);
}

/**
 * @brief Sends the greeting that opens every connection, with the commands this build answers.
 */
void Platform::KioskControl::sendHello(const QString& title, const QString& screen)
{
  QJsonObject message;
  message.insert(QStringLiteral("hello"), kProtocolVersion);
  message.insert(QStringLiteral("id"), m_identity);
  message.insert(QStringLiteral("title"), title);
  message.insert(QStringLiteral("pid"), QCoreApplication::applicationPid());
  message.insert(QStringLiteral("screen"), screen);
  message.insert(
    QStringLiteral("can"),
    QJsonArray{QStringLiteral("raise"), QStringLiteral("screen"), QStringLiteral("close")});
  sendLine(message);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts one connection attempt, never blocking; a socket still mid-attempt is reset first.
 */
void Platform::KioskControl::dial()
{
  if (m_serverName.isEmpty() || isLinked())
    return;

  if (m_socket.state() != QLocalSocket::UnconnectedState)
    m_socket.abort();

  m_buffer.clear();
  m_socket.connectToServer(m_serverName);
}

/**
 * @brief Drops bytes left from the previous connection and lets the owner greet the server.
 */
void Platform::KioskControl::onConnected()
{
  m_retry.stop();
  m_buffer.clear();
  Q_EMIT linkEstablished();
}

/**
 * @brief Reads at most one over-long line's worth of bytes at a time, so the buffer stays within
 *        the line cap, and drops the link on an over-long line. A burst above kMaxLinesPerRead
 *        lines continues on the next event-loop pass through one shared timer, and a link that
 *        drops while a command is handled ends the pass.
 */
void Platform::KioskControl::readPending()
{
  for (int i = 0; i < kMaxLinesPerRead; ++i) {
    const qint64 room = kMaxLineBytes + 1 - m_buffer.size();
    if (room > 0)
      m_buffer.append(m_socket.read(room));

    QByteArray line;
    const Frame frame = takeLine(m_buffer, line);
    if (frame == Frame::Overflow) {
      m_socket.abort();
      return;
    }

    if (frame == Frame::None)
      return;

    dispatch(line);
    if (!isLinked())
      return;
  }

  m_drain.start();
}

/**
 * @brief Arms the redial timer after a failed attempt or a dropped connection. Both socket
 *        signals land here, and restarting a single-shot timer makes the pair harmless.
 */
void Platform::KioskControl::scheduleRetry()
{
  if (m_serverName.isEmpty() || isLinked())
    return;

  m_retry.start();
}

//--------------------------------------------------------------------------------------------------
// Protocol
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decodes one received line. Anything that is not a JSON object with a known command is
 *        ignored, which is what lets a newer launcher talk to this build. A numeric
 *        reserve_left is truncated and held within 0..INT_MAX; an absent or non-numeric one
 *        reports -1, which keeps the current width.
 */
void Platform::KioskControl::dispatch(const QByteArray& line)
{
  const QJsonDocument document = QJsonDocument::fromJson(line);
  if (!document.isObject())
    return;

  const QJsonObject message = document.object();
  const QString command     = message.value(QStringLiteral("cmd")).toString();
  if (command == QStringLiteral("raise")) {
    Q_EMIT raiseRequested();
    return;
  }

  if (command == QStringLiteral("close")) {
    Q_EMIT closeRequested();
    return;
  }

  if (command != QStringLiteral("screen"))
    return;

  const QString name = message.value(QStringLiteral("name")).toString();
  if (name.isEmpty())
    return;

  const QJsonValue reserve = message.value(QStringLiteral("reserve_left"));
  if (!reserve.isDouble()) {
    Q_EMIT screenRequested(name, -1);
    return;
  }

  constexpr double kMaxReserve = std::numeric_limits<int>::max();
  const double pixels          = qBound(0.0, SerialStudio::toDouble(reserve, 0.0), kMaxReserve);
  Q_EMIT screenRequested(name, static_cast<int>(pixels));
}

/**
 * @brief Writes one message as a compact JSON line; silently dropped while there is no link. A
 *        peer that stopped reading is treated as gone once a line cap of replies is queued,
 *        since a buffered write never fails on its own.
 */
void Platform::KioskControl::sendLine(const QJsonObject& message)
{
  if (!isLinked())
    return;

  if (m_socket.bytesToWrite() > kMaxLineBytes) {
    m_socket.abort();
    return;
  }

  QByteArray line = QJsonDocument(message).toJson(QJsonDocument::Compact);
  line.append('\n');
  if (m_socket.write(line) != line.size())
    m_socket.abort();
}
