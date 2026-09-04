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

#include "API/Server/ClientReception.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonParseError>

#include "API/CommandProtocol.h"
#include "API/MCPProtocol.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kApiWindowMs             = 1000;
constexpr int kMaxApiJsonDepth         = 64;
constexpr int kMaxApiRawBytes          = API::Limits::kMaxApiRawBytes;
constexpr int kMaxApiMessagesPerWindow = 200;
constexpr int kMaxApiMessageBytes      = 1024 * 1024;
constexpr int kMaxApiBufferBytes       = 4 * 1024 * 1024;
constexpr int kMaxApiBytesPerWindow    = 128 * 1024 * 1024;
constexpr int kMaxAuthAttempts         = 3;
constexpr int kMaxBufferIterations     = 10000;

//--------------------------------------------------------------------------------------------------
// Static functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the refusal a denied or not-yet-answered device write gets. CONSENT_REQUIRED is a
 *        retryable refusal: the prompt was posted to the GUI, and the client re-sends once the
 *        user answered, which is what keeps the modal off this thread (spec 0075 I1).
 */
static API::CommandResponse refusedWriteResponse(const QString& id,
                                                 const API::DeviceWriteVerdict verdict)
{
  if (verdict == API::DeviceWriteVerdict::ConsentRequired)
    return API::CommandResponse::makeError(id,
                                           API::ErrorCode::ConsentRequired,
                                           QStringLiteral("Device writes need the user's consent; "
                                                          "a prompt was shown, retry after it is "
                                                          "answered"));

  return API::CommandResponse::makeError(
    id, API::ErrorCode::ExecutionError, QStringLiteral("Device write denied by user"));
}

/**
 * @brief Returns true if the JSON byte stream nests deeper than the given limit.
 */
static bool exceedsJsonDepthLimit(const QByteArray& data, int maxDepth)
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
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the machine to the host that owns the sockets and the command dispatchers.
 */
API::ClientReception::ClientReception(ReceptionHost& host) : m_host(host) {}

//--------------------------------------------------------------------------------------------------
// Entry point
//--------------------------------------------------------------------------------------------------

/**
 * @brief Consumes one chunk of bytes for a connection: limits first, the HTTP sniff on the very
 *        first bytes, then the token handshake while unauthenticated, then newline framing over
 *        the accumulated buffer. The connection is addressed by (socket, sessionId) rather than by
 *        reference, because every dispatch below may erase the entry this data belongs to.
 */
void API::ClientReception::consumeBytes(QTcpSocket* socket,
                                        const QString& sessionId,
                                        const QByteArray& data)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT(!data.isEmpty(), return);

  auto* state = m_host.stateFor(socket, sessionId);
  if (!state)
    return;

  if (!validateRateLimits(socket, *state, data))
    return;

  if (rejectHttpPreamble(socket, *state, data))
    return;

  state->buffer.append(data);

  if (!state->authenticated) {
    if (!handleAuthHandshake(socket, *state))
      return;

    if (!m_host.stateFor(socket, sessionId))
      return;
  }

  drainBuffer(socket, sessionId);
}

/**
 * @brief Frames and dispatches whole lines until the buffer runs dry. The state pointer is
 *        re-resolved at the top of every iteration and never survives a dispatch: a handler can
 *        spin an event loop, and the queued disconnect that runs inside it erases the entry
 *        (spec 0075 I1). The iteration cap stops a flood of tiny lines from owning the GUI thread.
 */
void API::ClientReception::drainBuffer(QTcpSocket* socket, const QString& sessionId)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT_LOG(!sessionId.isEmpty());

  for (int iteration = 0; iteration < kMaxBufferIterations; ++iteration) {
    auto* state = m_host.stateFor(socket, sessionId);
    if (!state || state->buffer.isEmpty())
      return;

    const int newlineIndex = state->buffer.indexOf('\n');
    if (newlineIndex < 0) {
      processNoNewlineBuffer(socket, sessionId);
      return;
    }

    int bodyLen = newlineIndex;
    if (bodyLen > 0 && state->buffer.at(bodyLen - 1) == '\r')
      --bodyLen;

    const QByteArray line = state->buffer.left(bodyLen);
    state->buffer.remove(0, newlineIndex + 1);

    const auto trimmedLine = line.trimmed();
    if (trimmedLine.isEmpty())
      continue;

    if (trimmedLine.at(0) == '{' || trimmedLine.at(0) == '[')
      handleJsonMessage(socket, sessionId, trimmedLine);
    else
      processRawLine(socket, *state, line);
  }

  reportBufferFlood(socket, sessionId);
}

/**
 * @brief Drops a client that still has buffered content after the iteration cap.
 */
void API::ClientReception::reportBufferFlood(QTcpSocket* socket, const QString& sessionId)
{
  auto* state = m_host.stateFor(socket, sessionId);
  if (!state || state->buffer.isEmpty()) [[likely]]
    return;

  qWarning() << "[API] Buffer processing iteration limit reached:" << state->peerAddress << ":"
             << state->peerPort << "- Disconnecting client";

  m_host.disconnectClient(
    socket, *state, ErrorCode::ExecutionError, QStringLiteral("API message flood limit exceeded"));
}

/**
 * @brief True when the bytes open with an HTTP request line. A browser can POST JSON to this port
 *        as a simple cross-origin request, and its headers would otherwise arrive as raw device
 *        lines with its body dispatched as an authenticated command (spec 0075 I2).
 */
bool API::ClientReception::looksLikeHttpRequest(const QByteArray& data)
{
  static const char* kMethods[] = {
    "GET ", "PUT ", "HEAD ", "POST ", "PATCH ", "TRACE ", "DELETE ", "CONNECT ", "OPTIONS "};

  for (const auto* method : kMethods)
    if (data.startsWith(method))
      return true;

  return false;
}

/**
 * @brief Closes a connection whose first bytes are an HTTP request. Nothing is written back: a
 *        readable response is exactly what a cross-origin script would want to see.
 */
bool API::ClientReception::rejectHttpPreamble(QTcpSocket* socket,
                                              ConnectionState& state,
                                              const QByteArray& data)
{
  SS_ASSERT(socket != nullptr, return true);

  if (state.firstBytesSeen) [[likely]]
    return false;

  state.firstBytesSeen = true;
  if (!looksLikeHttpRequest(data))
    return false;

  qWarning() << "[API] HTTP request on the API socket:" << state.peerAddress << ":"
             << state.peerPort << "- Disconnecting client";

  state.buffer.clear();
  m_host.closeSocket(socket, state);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Authentication handshake
//--------------------------------------------------------------------------------------------------

/**
 * @brief Consumes the first buffered line as a {"type":"auth","token":...} handshake. True once
 *        the client is authenticated, which is what lets the caller keep framing the same buffer:
 *        re-entering consumeBytes for the pipelined remainder counted those bytes twice (I12).
 */
bool API::ClientReception::handleAuthHandshake(QTcpSocket* socket, ConnectionState& state)
{
  SS_ASSERT(socket != nullptr, return false);
  SS_ASSERT_LOG(!state.authenticated);

  if (state.buffer.size() > kMaxApiMessageBytes) {
    m_host.disconnectClient(
      socket, state, ErrorCode::ExecutionError, QStringLiteral("Authentication required"));
    return false;
  }

  const int newlineIndex = state.buffer.indexOf('\n');
  if (newlineIndex < 0)
    return false;

  const QByteArray line = state.buffer.left(newlineIndex).trimmed();
  state.buffer.remove(0, newlineIndex + 1);

  bool ok = false;
  QJsonParseError parseError;
  const auto doc = QJsonDocument::fromJson(line, &parseError);
  if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
    const auto obj = doc.object();
    if (obj.value(QStringLiteral("type")).toString() == QStringLiteral("auth")) {
      const QByteArray provided = obj.value(QStringLiteral("token")).toString().toUtf8();
      ok                        = m_host.verifyToken(provided);
    }
  }

  if (!ok) {
    if (++state.authAttempts >= kMaxAuthAttempts) {
      qWarning() << "[API] Authentication failed:" << state.peerAddress << ":" << state.peerPort
                 << "- Disconnecting after" << state.authAttempts << "attempts";
      m_host.disconnectClient(
        socket, state, ErrorCode::ExecutionError, QStringLiteral("Authentication failed"));
      return false;
    }

    m_host.sendResponse(socket,
                        CommandResponse::makeError(QString(),
                                                   ErrorCode::ExecutionError,
                                                   QStringLiteral("Authentication required"))
                          .toJsonBytes());
    return false;
  }

  state.authenticated = true;
  qInfo() << "[API] Client authenticated:" << state.peerAddress << ":" << state.peerPort;

  QJsonObject result;
  result[QStringLiteral("authenticated")] = true;
  m_host.sendResponse(socket, CommandResponse::makeSuccess(QString(), result).toJsonBytes());
  return true;
}

//--------------------------------------------------------------------------------------------------
// Validation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates rate limits and buffer capacity for incoming data.
 */
bool API::ClientReception::validateRateLimits(QTcpSocket* socket,
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

    m_host.disconnectClient(
      socket, state, ErrorCode::ExecutionError, QStringLiteral("API rate limit exceeded"));
    return false;
  }

  if (state.buffer.size() + data.size() > kMaxApiBufferBytes) {
    qWarning() << "[API] Buffer size limit exceeded:" << state.peerAddress << ":" << state.peerPort
               << "- Buffer size:" << state.buffer.size() << "- Incoming data:" << data.size()
               << "- Limit:" << kMaxApiBufferBytes << "- Disconnecting client";

    m_host.disconnectClient(
      socket, state, ErrorCode::ExecutionError, QStringLiteral("API buffer limit exceeded"));
    return false;
  }

  return true;
}

/**
 * @brief Validates JSON message size, depth, and rate limits.
 */
bool API::ClientReception::validateJsonMessage(QTcpSocket* socket,
                                               ConnectionState& state,
                                               const QByteArray& jsonBytes)
{
  SS_ASSERT(socket != nullptr, return false);
  SS_ASSERT(!jsonBytes.isEmpty(), return false);

  if (jsonBytes.size() > kMaxApiMessageBytes) {
    qWarning() << "[API] Message size limit exceeded:" << state.peerAddress << ":" << state.peerPort
               << "- Message size:" << jsonBytes.size() << "- Limit:" << kMaxApiMessageBytes;

    m_host.sendResponse(socket,
                        CommandResponse::makeError(QString(),
                                                   ErrorCode::ExecutionError,
                                                   QStringLiteral("API message exceeds size limit"))
                          .toJsonBytes());
    return false;
  }

  if (exceedsJsonDepthLimit(jsonBytes, kMaxApiJsonDepth)) {
    qWarning() << "[API] JSON depth limit exceeded:" << state.peerAddress << ":" << state.peerPort
               << "- Max depth:" << kMaxApiJsonDepth;

    m_host.sendResponse(
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

    m_host.disconnectClient(
      socket, state, ErrorCode::ExecutionError, QStringLiteral("API rate limit exceeded"));
    return false;
  }

  ++state.messageCount;
  return true;
}

//--------------------------------------------------------------------------------------------------
// Message dispatch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Dispatches a validated JSON message to the appropriate handler. Nothing here reads the
 *        connection state after a dispatch: the message is a local copy and the state pointer is
 *        resolved once, before the handler that may spin an event loop runs.
 */
void API::ClientReception::handleJsonMessage(QTcpSocket* socket,
                                             const QString& sessionId,
                                             const QByteArray& jsonBytes)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT(!jsonBytes.isEmpty(), return);

  auto* state = m_host.stateFor(socket, sessionId);
  if (!state)
    return;

  if (!validateJsonMessage(socket, *state, jsonBytes))
    return;

  if (MCP::isMCPMessage(jsonBytes)) {
    state->handshakeSeen = true;
    const auto response  = m_host.dispatchMcp(jsonBytes, sessionId);
    if (!response.isEmpty())
      m_host.sendResponse(socket, response);

    return;
  }

  QString type;
  QJsonObject json;
  try {
    if (!API::parseMessage(jsonBytes, type, json)) {
      m_host.sendResponse(socket,
                          CommandResponse::makeError(QString(),
                                                     ErrorCode::InvalidJson,
                                                     QStringLiteral("Failed to parse JSON message"))
                            .toJsonBytes());
      return;
    }
  } catch (...) {
    qWarning() << "[API] JSON parsing exception:" << state->peerAddress << ":" << state->peerPort
               << "- Message size:" << jsonBytes.size()
               << "- Disconnecting client (malformed or too deep JSON)";

    m_host.disconnectClient(socket,
                            *state,
                            ErrorCode::InvalidJson,
                            QStringLiteral("JSON parsing failed (malformed or too deep)"));
    return;
  }

  state->handshakeSeen = true;
  if (type == MessageType::Raw) {
    processRawJsonCommand(socket, *state, json);
    return;
  }

  if (type == MessageType::Command && m_host.routeConnectionCommand(socket, *state, json))
    return;

  m_host.sendResponse(socket, m_host.dispatchCommand(jsonBytes));
}

/**
 * @brief Processes a JSON "raw" command that forwards base64 data to the device.
 */
void API::ClientReception::processRawJsonCommand(QTcpSocket* socket,
                                                 ConnectionState& state,
                                                 const QJsonObject& json)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT_LOG(!state.sessionId.isEmpty());

  const QString id = json.value(QStringLiteral("id")).toString();

  if (!json.contains(QStringLiteral("data"))) {
    m_host.sendResponse(
      socket,
      CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: data"))
        .toJsonBytes());
    return;
  }

  const QString dataStr    = json.value(QStringLiteral("data")).toString();
  const QByteArray rawData = QByteArray::fromBase64(dataStr.toUtf8());
  if (rawData.isEmpty() && !dataStr.isEmpty()) {
    m_host.sendResponse(
      socket,
      CommandResponse::makeError(id, ErrorCode::InvalidParam, QStringLiteral("Invalid base64 data"))
        .toJsonBytes());
    return;
  }

  if (rawData.size() > kMaxApiRawBytes) {
    qWarning() << "[API] Raw data size limit exceeded:" << state.peerAddress << ":"
               << state.peerPort << "- Raw data size:" << rawData.size()
               << "- Limit:" << kMaxApiRawBytes;

    m_host.sendResponse(socket,
                        CommandResponse::makeError(id,
                                                   ErrorCode::ExecutionError,
                                                   QStringLiteral("Raw payload exceeds size limit"))
                          .toJsonBytes());
    return;
  }

  const auto verdict = m_host.authorizeDeviceWrite();
  if (verdict != DeviceWriteVerdict::Allowed) {
    m_host.sendResponse(socket, refusedWriteResponse(id, verdict).toJsonBytes());
    return;
  }

  if (!m_host.deviceConnected()) {
    m_host.sendResponse(
      socket,
      CommandResponse::makeError(id, ErrorCode::ExecutionError, QStringLiteral("Not connected"))
        .toJsonBytes());
    return;
  }

  const qint64 bytesWritten = m_host.writeToDevice(rawData);
  if (!id.isEmpty()) {
    QJsonObject result;
    result[QStringLiteral("bytesWritten")] = bytesWritten;
    m_host.sendResponse(socket, CommandResponse::makeSuccess(id, result).toJsonBytes());
  }
}

//--------------------------------------------------------------------------------------------------
// Buffer processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles a buffered message when no newline delimiter is present.
 */
void API::ClientReception::processNoNewlineBuffer(QTcpSocket* socket, const QString& sessionId)
{
  SS_ASSERT(socket != nullptr, return);

  auto* state = m_host.stateFor(socket, sessionId);
  if (!state)
    return;

  SS_ASSERT_LOG(state->authenticated);
  const auto trimmed = state->buffer.trimmed();

  const char firstChar = trimmed.isEmpty() ? '\0' : trimmed.at(0);
  if (firstChar == '{' || firstChar == '[') {
    processBufferedJson(socket, sessionId, trimmed);
    return;
  }

  if (state->buffer.size() > kMaxApiRawBytes) {
    qWarning() << "[API] Raw buffer size limit exceeded:" << state->peerAddress << ":"
               << state->peerPort << "- Buffer size:" << state->buffer.size()
               << "- Limit:" << kMaxApiRawBytes << "- Disconnecting client";

    m_host.disconnectClient(
      socket, *state, ErrorCode::ExecutionError, QStringLiteral("Raw payload exceeds size limit"));
    return;
  }

  const QByteArray raw = state->buffer;
  state->buffer.clear();
  if (!authorizeRawWrite(socket, *state))
    return;

  const qint64 written = m_host.writeToDevice(raw);
  if (written < 0) [[unlikely]]
    qWarning() << "[API] writeData() failed for raw buffer"
               << "-- data not sent to device";
}

/**
 * @brief Attempts to parse buffered data as a complete JSON message. Parsing waits for the
 *        closing bracket: a newline-less body arrives in many chunks, and re-parsing the
 *        whole partial buffer per chunk is quadratic (a 1 MB body stalled the GUI thread
 *        for seconds and timed out the next commands).
 */
void API::ClientReception::processBufferedJson(QTcpSocket* socket,
                                               const QString& sessionId,
                                               const QByteArray& trimmed)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT(!trimmed.isEmpty(), return);

  auto* state = m_host.stateFor(socket, sessionId);
  if (!state)
    return;

  if (trimmed.size() > kMaxApiMessageBytes) {
    qWarning() << "[API] Message size limit exceeded:" << state->peerAddress << ":"
               << state->peerPort << "- Message size:" << trimmed.size()
               << "- Limit:" << kMaxApiMessageBytes;

    m_host.sendResponse(socket,
                        CommandResponse::makeError(QString(),
                                                   ErrorCode::ExecutionError,
                                                   QStringLiteral("API message exceeds size limit"))
                          .toJsonBytes());
    state->buffer.clear();
    return;
  }

  const char firstChar = trimmed.at(0);
  const char lastChar  = trimmed.back();
  const bool complete =
    (firstChar == '{' && lastChar == '}') || (firstChar == '[' && lastChar == ']');
  if (!complete)
    return;

  if (exceedsJsonDepthLimit(trimmed, kMaxApiJsonDepth)) {
    qWarning() << "[API] JSON depth limit exceeded (buffered):" << state->peerAddress << ":"
               << state->peerPort << "- Max depth:" << kMaxApiJsonDepth;

    m_host.sendResponse(
      socket,
      CommandResponse::makeError(
        QString(), ErrorCode::ExecutionError, QStringLiteral("JSON nesting depth exceeds limit"))
        .toJsonBytes());
    state->buffer.clear();
    return;
  }

  QString type;
  QJsonObject json;
  try {
    const bool dispatchable = API::parseMessage(trimmed, type, json) || MCP::isMCPMessage(trimmed);
    state->buffer.clear();
    if (dispatchable)
      handleJsonMessage(socket, sessionId, trimmed);
    else
      m_host.sendResponse(socket,
                          CommandResponse::makeError(QString(),
                                                     ErrorCode::InvalidJson,
                                                     QStringLiteral("Failed to parse JSON message"))
                            .toJsonBytes());
  } catch (...) {
    qWarning() << "[API] JSON parsing exception (buffered):" << state->peerAddress << ":"
               << state->peerPort << "- Buffer size:" << trimmed.size()
               << "- Disconnecting client (malformed or too deep JSON)";

    m_host.disconnectClient(socket,
                            *state,
                            ErrorCode::InvalidJson,
                            QStringLiteral("JSON parsing failed (malformed or too deep)"));
  }
}

/**
 * @brief Gates a raw device write from this connection: the handshake must have happened, and the
 *        consent gate must answer Allowed. Answers the client on every refusal so a legitimate
 *        raw-first client learns why instead of watching its bytes vanish.
 */
bool API::ClientReception::authorizeRawWrite(QTcpSocket* socket, ConnectionState& state)
{
  SS_ASSERT(socket != nullptr, return false);

  if (!state.handshakeSeen) {
    m_host.sendResponse(socket,
                        CommandResponse::makeError(QString(),
                                                   ErrorCode::InvalidMessageType,
                                                   QStringLiteral("Raw device forwarding needs one "
                                                                  "valid JSON message first"))
                          .toJsonBytes());
    return false;
  }

  const auto verdict = m_host.authorizeDeviceWrite();
  if (verdict == DeviceWriteVerdict::Allowed) [[likely]]
    return true;

  m_host.sendResponse(socket, refusedWriteResponse(QString(), verdict).toJsonBytes());
  return false;
}

/**
 * @brief Processes a non-JSON raw line from the buffer.
 */
void API::ClientReception::processRawLine(QTcpSocket* socket,
                                          ConnectionState& state,
                                          const QByteArray& line)
{
  SS_ASSERT(socket != nullptr, return);
  SS_ASSERT(!line.isEmpty(), return);

  if (line.size() > kMaxApiRawBytes) {
    qWarning() << "[API] Raw line size limit exceeded:" << state.peerAddress << ":"
               << state.peerPort << "- Line size:" << line.size() << "- Limit:" << kMaxApiRawBytes
               << "- Disconnecting client";

    m_host.sendResponse(socket,
                        CommandResponse::makeError(QString(),
                                                   ErrorCode::ExecutionError,
                                                   QStringLiteral("Raw payload exceeds size limit"))
                          .toJsonBytes());

    m_host.closeSocket(socket, state);
    return;
  }

  if (!authorizeRawWrite(socket, state))
    return;

  const qint64 written = m_host.writeToDevice(line);
  if (written < 0) [[unlikely]]
    qWarning() << "[API] writeData() failed for raw line"
               << "-- data not sent to device";
}
