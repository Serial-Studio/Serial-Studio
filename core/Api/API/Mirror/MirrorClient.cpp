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

#include "API/Mirror/MirrorClient.h"

#include <iterator>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kWatchdogTicks   = 3;
constexpr int kWatchdogMinMs   = 500;
constexpr int kWatchdogMaxMs   = 3000;
constexpr int kMaxLineBytes    = 1024 * 1024;
constexpr int kMaxBufferBytes  = 4 * 1024 * 1024;
constexpr int kMaxLinesPerRead = 4096;

constexpr int kBackoffMs[] = {1000, 2000, 4000, 8000, 16000, 30000};
constexpr int kBackoffMax  = static_cast<int>(std::size(kBackoffMs)) - 1;

//--------------------------------------------------------------------------------------------------
// Static functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a failed response is the lazy authentication challenge. The server pre-authorizes
 *        loopback peers, so an unsolicited auth line is itself an error; the token is only ever
 *        sent after the server asks for it.
 */
static bool isAuthChallenge(const QJsonObject& error)
{
  return error.value(QStringLiteral("message")).toString().contains(QStringLiteral("uthenticat"));
}

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an idle client. No socket work happens until open() is called.
 */
API::MirrorClient::MirrorClient(QObject* parent)
  : QObject(parent)
  , m_port(0)
  , m_hz(Mirror::kHzDefault)
  , m_attempt(0)
  , m_live(false)
  , m_stale(true)
  , m_authSent(false)
  , m_structurePending(false)
  , m_userClosed(false)
  , m_epoch(0)
  , m_requestCounter(0)
  , m_valueCount(0)
  , m_stage(Stage::Offline)
  , m_chunkParts(0)
  , m_chunkEpoch(0)
{
  m_watchdog.setSingleShot(false);
  m_reconnect.setSingleShot(true);

  connect(&m_watchdog, &QTimer::timeout, this, &MirrorClient::onWatchdog);
  connect(&m_reconnect, &QTimer::timeout, this, &MirrorClient::onReconnect);
  connect(&m_socket, &QTcpSocket::connected, this, &MirrorClient::onConnected);
  connect(&m_socket, &QTcpSocket::readyRead, this, &MirrorClient::onReadyRead);
  connect(&m_socket, &QTcpSocket::disconnected, this, &MirrorClient::onDisconnected);
  connect(&m_socket, &QTcpSocket::errorOccurred, this, &MirrorClient::onSocketError);
}

//--------------------------------------------------------------------------------------------------
// State access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cadence the remote agreed to serve, which is what sizes the staleness watchdog.
 */
int API::MirrorClient::hz() const noexcept
{
  return m_hz;
}

/**
 * @brief Whether value snapshots are still arriving: the remote capture is producing data.
 */
bool API::MirrorClient::live() const noexcept
{
  return m_live;
}

/**
 * @brief Whether nothing at all arrived within the watchdog, heartbeats included: link suspect.
 */
bool API::MirrorClient::stale() const noexcept
{
  return m_stale;
}

/**
 * @brief Whether the mirror is subscribed and a verified structure is held.
 */
bool API::MirrorClient::linked() const noexcept
{
  return m_stage == Stage::Streaming;
}

/**
 * @brief Structure epoch currently held; every snapshot must carry it.
 */
quint64 API::MirrorClient::epoch() const noexcept
{
  return m_epoch;
}

/**
 * @brief Human-readable host:port of the attached remote.
 */
const QString& API::MirrorClient::endpoint() const noexcept
{
  return m_endpoint;
}

/**
 * @brief Message of the last failure, empty when none has occurred since the last open().
 */
const QString& API::MirrorClient::lastError() const noexcept
{
  return m_lastError;
}

/**
 * @brief Machine-readable code of the last failure; distinguishes refused, unauthorized,
 *        version-mismatch and unreachable for the attach dialog.
 */
const QString& API::MirrorClient::lastErrorCode() const noexcept
{
  return m_lastErrorCode;
}

/**
 * @brief The remote's mirror.subscribe result: wire version, app version, epoch, dataset count.
 */
const QJsonObject& API::MirrorClient::info() const noexcept
{
  return m_info;
}

//--------------------------------------------------------------------------------------------------
// Lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a mirror link to @p host. Any previous link is torn down first, so a re-open with
 *        different credentials can never inherit the old socket's authentication state.
 */
void API::MirrorClient::open(const QString& host,
                             const quint16 port,
                             const QString& token,
                             const int hz)
{
  SS_ASSERT(!host.isEmpty(), return);
  SS_ASSERT(port > 0, return);

  close();

  m_host          = host;
  m_port          = port;
  m_token         = token;
  m_hz            = qBound(Mirror::kHzMin, hz, Mirror::kHzMax);
  m_endpoint      = QStringLiteral("%1:%2").arg(host, QString::number(port));
  m_attempt       = 0;
  m_userClosed    = false;
  m_lastError     = QString();
  m_lastErrorCode = QString();
  m_stage         = Stage::Connecting;

  m_socket.connectToHost(m_host, m_port);
}

/**
 * @brief Closes the link. The remote capture is unaffected: it only sees the socket go away, and
 *        drops this connection's subscription alongside its other per-connection state.
 */
void API::MirrorClient::close()
{
  m_userClosed = true;
  m_reconnect.stop();
  m_watchdog.stop();

  if (m_socket.state() != QAbstractSocket::UnconnectedState)
    m_socket.abort();

  const bool wasLinked = linked();
  m_stage              = Stage::Offline;
  resetStreamState();

  if (wasLinked)
    Q_EMIT linkedChanged();
}

/**
 * @brief Drops every per-connection fact so a reconnect re-derives all of it: the epoch, the
 *        pending request, the chunk accumulator and the liveness flags. Staleness resets to true,
 *        not false: an offline or reconnecting link has no data flowing and must never read healthy
 *        until a push actually arrives.
 */
void API::MirrorClient::resetStreamState()
{
  m_buffer.clear();
  m_chunks.clear();
  m_info = QJsonObject();

  m_epoch            = 0;
  m_valueCount       = 0;
  m_chunkParts       = 0;
  m_chunkEpoch       = 0;
  m_authSent         = false;
  m_structurePending = false;
  m_pendingId        = QString();
  m_pendingCommand   = QString();
  m_pendingParams    = QJsonObject();

  setLive(false);
  setStale(true);
}

//--------------------------------------------------------------------------------------------------
// Socket events
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends mirror.subscribe as the very first request. Anything ahead of it is a window in
 *        which the remote's unconditional per-frame broadcast floods this socket.
 */
void API::MirrorClient::onConnected()
{
  SS_ASSERT_LOG(m_stage == Stage::Connecting);

  m_attempt = 0;
  m_stage   = Stage::Subscribing;

  QJsonObject params;
  params.insert(QStringLiteral("wireVersion"), Mirror::kWireVersion);
  params.insert(QStringLiteral("hz"), m_hz);
  params.insert(QStringLiteral("frames"), false);
  sendCommand(QLatin1String(Mirror::Command::Subscribe), params);
}

/**
 * @brief Reconnects with backoff unless the user detached; a lost link is not a user decision.
 */
void API::MirrorClient::onDisconnected()
{
  const bool wasLinked = linked();
  m_stage              = Stage::Offline;
  resetStreamState();

  if (wasLinked)
    Q_EMIT linkedChanged();

  if (!m_userClosed)
    scheduleReconnect();
}

/**
 * @brief Reports a transport failure with a code the dialog can distinguish, then falls into the
 *        reconnect backoff.
 */
void API::MirrorClient::onSocketError(const QAbstractSocket::SocketError error)
{
  if (m_userClosed)
    return;

  const auto code = (error == QAbstractSocket::ConnectionRefusedError)
                    ? QStringLiteral("MIRROR_REFUSED")
                    : QStringLiteral("MIRROR_UNREACHABLE");
  fail(code, m_socket.errorString());
}

/**
 * @brief Splits the inbound stream on newlines, the framing every other message on this socket
 *        already uses. A partial line is never parsed: it stays buffered until its newline lands.
 */
void API::MirrorClient::onReadyRead()
{
  m_buffer += m_socket.readAll();
  if (m_buffer.size() > kMaxBufferBytes) {
    fail(QStringLiteral("MIRROR_OVERFLOW"), tr("Remote sent more data than this viewer can hold"));
    return;
  }

  int lines = 0;
  while (lines < kMaxLinesPerRead) {
    const int newline = m_buffer.indexOf('\n');
    if (newline < 0)
      return;

    ++lines;
    const auto line = m_buffer.left(newline).trimmed();
    m_buffer.remove(0, newline + 1);

    if (!line.isEmpty() && line.size() <= kMaxLineBytes)
      handleLine(line);
  }
}

//--------------------------------------------------------------------------------------------------
// Message handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Routes one decoded line: a mirror push, or a response to this client's own request.
 *        Anything else on the socket belongs to a message kind this client does not speak and is
 *        dropped, which is the forward-compatibility rule the wire contract requires.
 */
void API::MirrorClient::handleLine(const QByteArray& line)
{
  QJsonParseError parseError;
  const auto document = QJsonDocument::fromJson(line, &parseError);
  if (parseError.error != QJsonParseError::NoError || !document.isObject())
    return;

  const auto object = document.object();
  const auto push   = object.value(QLatin1String(Mirror::kPushKey));
  if (push.isObject()) {
    handlePush(push.toObject());
    return;
  }

  if (object.value(QStringLiteral("type")).toString() == QStringLiteral("response"))
    handleResponse(object);
}

/**
 * @brief Handles a response to one of this client's requests, including the lazy auth challenge:
 *        the challenge carries no request id, so it is recognized by its failure message and the
 *        outstanding request is replayed once the token is accepted.
 */
void API::MirrorClient::handleResponse(const QJsonObject& message)
{
  const bool success = message.value(QStringLiteral("success")).toBool(false);
  const auto result  = message.value(QStringLiteral("result")).toObject();
  const auto error   = message.value(QStringLiteral("error")).toObject();
  const auto id      = message.value(QStringLiteral("id")).toString();

  if (success && result.contains(QStringLiteral("authenticated"))) {
    sendCommand(m_pendingCommand, m_pendingParams);
    return;
  }

  const bool challenge = !success && id.isEmpty() && !m_authSent && isAuthChallenge(error);
  if (challenge) {
    sendAuth();
    return;
  }

  if (id != m_pendingId || m_pendingId.isEmpty())
    return;

  if (!success) {
    fail(error.value(QStringLiteral("code")).toString(),
         error.value(QStringLiteral("message")).toString());
    return;
  }

  if (m_pendingCommand == QLatin1String(Mirror::Command::Subscribe))
    onSubscribed(result);
  else if (m_pendingCommand == QLatin1String(Mirror::Command::GetStructure))
    onStructureFetched(result);
}

/**
 * @brief Adopts the negotiated rate and asks for the structure. effectiveHz is used rather than
 *        the requested rate because the publisher cannot emit faster than its own display tick.
 */
void API::MirrorClient::onSubscribed(const QJsonObject& result)
{
  const int version = result.value(QStringLiteral("wireVersion")).toInt(0);
  if (version != Mirror::kWireVersion) {
    fail(QLatin1String(Mirror::ErrorCode::VersionMismatch),
         tr("The remote speaks mirror version %1; this build speaks %2")
           .arg(QString::number(version), QString::number(Mirror::kWireVersion)));
    return;
  }

  m_info      = result;
  m_pendingId = QString();
  m_hz =
    qBound(Mirror::kHzMin, result.value(QStringLiteral("effectiveHz")).toInt(m_hz), Mirror::kHzMax);
  m_stage = Stage::Fetching;

  armWatchdog();
  requestStructure();
}

/**
 * @brief Consumes the mirror.getStructure result, which is the whole payload on a small project
 *        and one base64 part on a large one; the remaining parts are pulled in order.
 */
void API::MirrorClient::onStructureFetched(const QJsonObject& result)
{
  m_pendingId     = QString();
  const auto kind = result.value(QStringLiteral("kind")).toString();

  if (kind == QLatin1String(Mirror::Kind::Structure)) {
    m_structurePending = false;
    adoptStructure(result);
    return;
  }

  if (kind != QLatin1String(Mirror::Kind::StructureChunk)) {
    fail(QStringLiteral("MIRROR_BAD_STRUCTURE"), tr("Remote returned an unreadable structure"));
    return;
  }

  handleChunk(result);
  if (!m_structurePending)
    return;

  QJsonObject params;
  params.insert(QStringLiteral("part"), m_chunks.size());
  sendCommand(QLatin1String(Mirror::Command::GetStructure), params);
}

/**
 * @brief Dispatches one server push by kind. An unknown kind is dropped in silence: adding a kind
 *        is explicitly not a wire-version bump, so an older viewer must tolerate one.
 */
void API::MirrorClient::handlePush(const QJsonObject& payload)
{
  setStale(false);
  armWatchdog();

  const auto kind = payload.value(QStringLiteral("kind")).toString();
  if (kind == QLatin1String(Mirror::Kind::Snapshot)) {
    handleSnapshot(payload);
    return;
  }

  if (kind == QLatin1String(Mirror::Kind::Structure)) {
    adoptStructure(payload);
    return;
  }

  if (kind == QLatin1String(Mirror::Kind::StructureChunk)) {
    handleChunk(payload);
    return;
  }

  if (kind == QLatin1String(Mirror::Kind::Heartbeat))
    setLive(false);
}

/**
 * @brief Collects base64 structure parts and adopts the payload once the set is complete. A part
 *        carrying a different epoch resets the accumulator rather than mixing two layouts.
 */
void API::MirrorClient::handleChunk(const QJsonObject& payload)
{
  const auto epoch = static_cast<quint64>(payload.value(QStringLiteral("epoch")).toInteger(0));
  const int parts  = payload.value(QStringLiteral("parts")).toInt(0);
  const int part   = payload.value(QStringLiteral("part")).toInt(-1);

  if (parts < 1 || parts > Mirror::kMaxStructureParts || part < 0 || part >= parts) {
    fail(QLatin1String(Mirror::ErrorCode::StructureTooLarge),
         tr("The remote project is too large to mirror"));
    return;
  }

  if (m_chunkEpoch != epoch) {
    m_chunks.clear();
    m_chunkEpoch = epoch;
    m_chunkParts = parts;
  }

  m_chunks.insert(part, payload.value(QStringLiteral("data")).toString());
  if (m_chunks.size() < m_chunkParts)
    return;

  QString blob;
  for (int i = 0; i < m_chunkParts; ++i)
    blob += m_chunks.value(i);

  m_chunks.clear();
  m_chunkEpoch       = 0;
  m_structurePending = false;

  const auto decoded = QJsonDocument::fromJson(QByteArray::fromBase64(blob.toLatin1()));
  if (!decoded.isObject()) {
    fail(QStringLiteral("MIRROR_BAD_STRUCTURE"), tr("Remote returned an unreadable structure"));
    return;
  }

  adoptStructure(decoded.object());
}

/**
 * @brief Adopts a structure only after its announced layout hash is reproduced over the received
 *        identity list. That single check is what makes a positional value format safe; a
 *        structure that fails it is refused, never rendered against.
 */
void API::MirrorClient::adoptStructure(const QJsonObject& payload)
{
  if (!verifyStructure(payload))
    return;

  const bool wasLinked = linked();
  m_epoch              = static_cast<quint64>(payload.value(QStringLiteral("epoch")).toInteger(0));
  m_valueCount         = payload.value(QStringLiteral("datasets")).toArray().size();
  m_stage              = Stage::Streaming;

  Q_EMIT structureReceived(payload);

  if (!wasLinked)
    Q_EMIT linkedChanged();
}

/**
 * @brief Checks the wire version and the layout hash of a structure payload.
 */
bool API::MirrorClient::verifyStructure(const QJsonObject& payload)
{
  const int version = payload.value(QStringLiteral("wireVersion")).toInt(0);
  if (version != Mirror::kWireVersion) {
    fail(QLatin1String(Mirror::ErrorCode::VersionMismatch),
         tr("The remote speaks mirror version %1; this build speaks %2")
           .arg(QString::number(version), QString::number(Mirror::kWireVersion)));
    return false;
  }

  const auto entries = payload.value(QStringLiteral("datasets")).toArray();
  std::vector<Mirror::DatasetId> datasets;
  datasets.reserve(static_cast<std::size_t>(entries.size()));
  for (const auto& entry : entries) {
    const auto pair = entry.toArray();
    datasets.push_back({pair.at(0).toInt(0), pair.at(1).toInt(0)});
  }

  const auto announced = payload.value(QStringLiteral("layoutHash")).toString();
  if (announced != Mirror::layoutHash(datasets)) {
    fail(QStringLiteral("MIRROR_LAYOUT_MISMATCH"),
         tr("The remote's dataset layout does not match its announced hash"));
    return false;
  }

  return true;
}

/**
 * @brief Applies one snapshot. A snapshot whose epoch or value count disagrees with the held
 *        structure is dropped and triggers exactly one structure re-fetch, so there is no path
 *        that hands values to the dashboard against a layout they do not match.
 */
void API::MirrorClient::handleSnapshot(const QJsonObject& payload)
{
  const auto epoch = static_cast<quint64>(payload.value(QStringLiteral("epoch")).toInteger(0));
  const int count  = payload.value(QStringLiteral("values")).toArray().size();

  if (m_stage != Stage::Streaming || epoch != m_epoch || count != m_valueCount) {
    requestStructure();
    return;
  }

  setLive(true);
  Q_EMIT snapshotReceived(payload);
}

//--------------------------------------------------------------------------------------------------
// Requests
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends one API command, remembering it so the lazy auth handshake can replay it.
 */
void API::MirrorClient::sendCommand(const QString& command, const QJsonObject& params)
{
  SS_ASSERT(!command.isEmpty(), return);

  if (m_socket.state() != QAbstractSocket::ConnectedState)
    return;

  m_pendingId      = QStringLiteral("mirror-%1").arg(QString::number(++m_requestCounter));
  m_pendingCommand = command;
  m_pendingParams  = params;

  QJsonObject message;
  message.insert(QStringLiteral("type"), QStringLiteral("command"));
  message.insert(QStringLiteral("id"), m_pendingId);
  message.insert(QStringLiteral("command"), command);
  if (!params.isEmpty())
    message.insert(QStringLiteral("params"), params);

  m_socket.write(QJsonDocument(message).toJson(QJsonDocument::Compact) + '\n');
}

/**
 * @brief Sends the token handshake the server asked for. A viewer with no token surfaces the
 *        refusal instead of retrying, because retrying burns the server's attempt budget.
 */
void API::MirrorClient::sendAuth()
{
  if (m_token.isEmpty()) {
    fail(QStringLiteral("MIRROR_UNAUTHORIZED"),
         tr("The remote requires a token and none was provided"));
    return;
  }

  m_authSent = true;

  QJsonObject message;
  message.insert(QStringLiteral("type"), QStringLiteral("auth"));
  message.insert(QStringLiteral("token"), m_token);
  m_socket.write(QJsonDocument(message).toJson(QJsonDocument::Compact) + '\n');
}

/**
 * @brief Requests the current structure, at most one request in flight at a time.
 */
void API::MirrorClient::requestStructure()
{
  if (m_structurePending)
    return;

  m_structurePending = true;
  sendCommand(QLatin1String(Mirror::Command::GetStructure), QJsonObject());
}

//--------------------------------------------------------------------------------------------------
// Liveness
//--------------------------------------------------------------------------------------------------

/**
 * @brief Arms the staleness watchdog at three mirror ticks, clamped so a 1 Hz mirror does not wait
 *        three seconds to report a dead link and a 60 Hz one does not flap on a single late push.
 */
void API::MirrorClient::armWatchdog()
{
  const int ticks = (kWatchdogTicks * 1000) / qMax(1, m_hz);
  m_watchdog.start(qBound(kWatchdogMinMs, ticks, kWatchdogMaxMs));
}

/**
 * @brief Nothing arrived within the watchdog, heartbeats included: the link is presumed dead and
 *        the dashboard must stop presenting its last values as current.
 */
void API::MirrorClient::onWatchdog()
{
  m_watchdog.stop();
  setLive(false);
  setStale(true);
}

/**
 * @brief Publishes a change of the snapshot-liveness flag.
 */
void API::MirrorClient::setLive(const bool value)
{
  if (m_live == value)
    return;

  m_live = value;
  Q_EMIT liveChanged();
}

/**
 * @brief Publishes a change of the staleness flag.
 */
void API::MirrorClient::setStale(const bool value)
{
  if (m_stale == value)
    return;

  m_stale = value;
  Q_EMIT staleChanged();
}

//--------------------------------------------------------------------------------------------------
// Failure & reconnect
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records a failure, reports it, and falls into the reconnect backoff. A version mismatch
 *        or a refused credential is reported and not retried: retrying cannot fix either.
 */
void API::MirrorClient::fail(const QString& code, const QString& message)
{
  m_lastErrorCode = code;
  m_lastError     = message;

  const bool fatal = code == QLatin1String(Mirror::ErrorCode::VersionMismatch)
                  || code == QStringLiteral("MIRROR_UNAUTHORIZED")
                  || code == QLatin1String(Mirror::ErrorCode::ViewerLimit);

  qWarning() << "[Mirror] " << code << message;
  Q_EMIT failed(code, message, fatal);

  if (fatal) {
    close();
    return;
  }

  if (m_socket.state() != QAbstractSocket::UnconnectedState)
    m_socket.abort();
  else if (!m_userClosed)
    scheduleReconnect();
}

/**
 * @brief Schedules the next reconnect attempt on the 1-30 s backoff ladder.
 */
void API::MirrorClient::scheduleReconnect()
{
  SS_ASSERT_LOG(!m_userClosed);

  if (m_reconnect.isActive())
    return;

  const int index = qMin(m_attempt, kBackoffMax);
  ++m_attempt;

  m_watchdog.stop();
  setStale(true);
  m_reconnect.start(kBackoffMs[index]);
}

/**
 * @brief Reconnects and repeats the whole sequence, structure included: a reconnect never assumes
 *        the epoch it used to hold is still current.
 */
void API::MirrorClient::onReconnect()
{
  if (m_userClosed || m_host.isEmpty())
    return;

  resetStreamState();
  m_stage = Stage::Connecting;
  m_socket.connectToHost(m_host, m_port);
}
