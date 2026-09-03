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

#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QTest>

#include "API/Server/AuthPrimitives.h"
#include "API/Server/ClientReception.h"

// Mirrors the caps ClientReception.cpp keeps file-private; restating them here is the point of a
// KAT: a silent widening of a limit has to break this suite.
constexpr int kMaxApiJsonDepth         = 64;
constexpr int kMaxApiRawBytes          = 1024 * 1024;
constexpr int kMaxApiMessagesPerWindow = 200;
constexpr int kMaxApiMessageBytes      = 1024 * 1024;
constexpr int kMaxApiBufferBytes       = 4 * 1024 * 1024;
constexpr int kMaxApiBytesPerWindow    = 128 * 1024 * 1024;
constexpr int kMaxAuthAttempts         = 3;

/**
 * @brief Records every effect the reception machine asks its host for, so a test can assert on
 *        what the machine decided without a server, a worker thread, or a live socket.
 */
class StubHost : public API::ReceptionHost {
public:
  bool deviceConnected() const override { return connected; }

  API::DeviceWriteVerdict authorizeDeviceWrite() override
  {
    return consent ? API::DeviceWriteVerdict::Allowed : API::DeviceWriteVerdict::Denied;
  }

  bool verifyToken(const QByteArray& provided) const override { return provided == expectedToken; }

  API::ConnectionState* stateFor(QTcpSocket* socket, const QString& sessionId) override
  {
    Q_UNUSED(socket);
    if (!lastState || lastState->sessionId != sessionId)
      return nullptr;

    return lastState;
  }

  qint64 writeToDevice(const QByteArray& data) override
  {
    deviceWrites.append(data);
    return data.size();
  }

  QByteArray dispatchCommand(const QByteArray& jsonBytes) override
  {
    dispatchedCommands.append(jsonBytes);
    return QByteArrayLiteral("{\"type\":\"response\",\"success\":true}\n");
  }

  QByteArray dispatchMcp(const QByteArray& jsonBytes, const QString& sessionId) override
  {
    Q_UNUSED(sessionId);
    dispatchedMcp.append(jsonBytes);
    return QByteArrayLiteral("{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{}}\n");
  }

  bool routeConnectionCommand(QTcpSocket* socket,
                              API::ConnectionState& state,
                              const QJsonObject& json) override
  {
    Q_UNUSED(socket);
    Q_UNUSED(state);
    routedCommands.append(json.value(QStringLiteral("command")).toString());
    return false;
  }

  void sendResponse(QTcpSocket* socket, const QByteArray& response) override
  {
    Q_UNUSED(socket);
    responses.append(response);
  }

  void closeSocket(QTcpSocket* socket, const API::ConnectionState& state) override
  {
    Q_UNUSED(socket);
    Q_UNUSED(state);
    ++closes;
  }

  void disconnectClient(QTcpSocket* socket,
                        API::ConnectionState& state,
                        const QString& errorCode,
                        const QString& errorMessage) override
  {
    Q_UNUSED(socket);
    Q_UNUSED(errorMessage);
    state.buffer.clear();
    disconnectCodes.append(errorCode);
  }

  bool consent                    = true;
  bool connected                  = true;
  int closes                      = 0;
  API::ConnectionState* lastState = nullptr;
  QByteArray expectedToken;
  QList<QByteArray> responses;
  QList<QByteArray> deviceWrites;
  QList<QByteArray> dispatchedMcp;
  QList<QByteArray> dispatchedCommands;
  QStringList routedCommands;
  QStringList disconnectCodes;
};

/**
 * @brief Auth primitives and the API socket's reception guards, both driven without a server.
 */
class TstServerAuth : public QObject {
  Q_OBJECT

private slots:
  void constantTimeEquals_data();
  void constantTimeEquals();

  void normalizeToken_data();
  void normalizeToken();

  void generatedTokenIsUsable();

  void commandClassification_data();
  void commandClassification();

  void authHandshakeRejectsBadToken();
  void authHandshakeAcceptsAndReplaysPipeline();

  void jsonMessageSizeLimit();
  void jsonMessageDepthLimit();
  void jsonMessageRateLimit();

  void rateLimitsByteWindow();
  void rateLimitsBufferCap();

  void noNewlineRawBufferReachesDevice();
  void rawLineOversizeClosesSocket();

private:
  [[nodiscard]] static API::ConnectionState authenticatedState();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief A connection that already cleared the handshake, as every post-auth path expects.
 */
API::ConnectionState TstServerAuth::authenticatedState()
{
  API::ConnectionState state;
  state.sessionId      = QStringLiteral("7");
  state.peerAddress    = QStringLiteral("127.0.0.1");
  state.peerPort       = 55000;
  state.authenticated  = true;
  state.handshakeSeen  = true;
  state.firstBytesSeen = true;
  return state;
}

//--------------------------------------------------------------------------------------------------
// constantTimeEquals
//--------------------------------------------------------------------------------------------------

void TstServerAuth::constantTimeEquals_data()
{
  QTest::addColumn<QByteArray>("lhs");
  QTest::addColumn<QByteArray>("rhs");
  QTest::addColumn<bool>("expected");

  const QByteArray token(64, 'a');

  QTest::newRow("identical") << token << token << true;
  QTest::newRow("empty pair") << QByteArray() << QByteArray() << true;
  QTest::newRow("first byte differs")
    << QByteArrayLiteral("abcd") << QByteArrayLiteral("zbcd") << false;
  QTest::newRow("last byte differs")
    << QByteArrayLiteral("abcd") << QByteArrayLiteral("abcz") << false;
  QTest::newRow("one bit differs")
    << QByteArrayLiteral("\x01") << QByteArrayLiteral("\x03") << false;
  QTest::newRow("length mismatch prefix")
    << QByteArrayLiteral("abcd") << QByteArrayLiteral("abcde") << false;
  QTest::newRow("empty against value") << QByteArray() << QByteArrayLiteral("a") << false;
  QTest::newRow("high bytes equal")
    << QByteArrayLiteral("\xff\x80") << QByteArrayLiteral("\xff\x80") << true;
  QTest::newRow("high bytes differ")
    << QByteArrayLiteral("\xff\x80") << QByteArrayLiteral("\xff\x81") << false;
}

/**
 * @brief The comparison must agree with byte equality on every row; the constant-time property
 *        itself is structural (no early exit inside the loop) and is guarded by review.
 */
void TstServerAuth::constantTimeEquals()
{
  QFETCH(QByteArray, lhs);
  QFETCH(QByteArray, rhs);
  QFETCH(bool, expected);

  QCOMPARE(API::Auth::constantTimeEquals(lhs, rhs), expected);
  QCOMPARE(API::Auth::constantTimeEquals(rhs, lhs), expected);
}

//--------------------------------------------------------------------------------------------------
// normalizeToken
//--------------------------------------------------------------------------------------------------

void TstServerAuth::normalizeToken_data()
{
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("expected");

  const QString hex32(32, QLatin1Char('a'));
  const QString hex64(64, QLatin1Char('f'));

  QTest::newRow("exactly 32 hex") << hex32 << hex32;
  QTest::newRow("64 hex") << hex64 << hex64;
  QTest::newRow("uppercase folds") << QString(32, QLatin1Char('A')) << hex32;
  QTest::newRow("surrounding whitespace")
    << (QStringLiteral("  ") + hex32 + QStringLiteral("\n")) << hex32;
  QTest::newRow("digits are hex") << QString(32, QLatin1Char('9')) << QString(32, QLatin1Char('9'));
  QTest::newRow("31 chars refused") << QString(31, QLatin1Char('a')) << QString();
  QTest::newRow("non-hex letter refused") << QString(32, QLatin1Char('g')) << QString();
  QTest::newRow("punctuation refused")
    << (QString(31, QLatin1Char('a')) + QStringLiteral("-")) << QString();
  QTest::newRow("empty refused") << QString() << QString();
  QTest::newRow("whitespace only refused") << QString(40, QLatin1Char(' ')) << QString();
}

/**
 * @brief Anything shorter than 32 characters or outside [0-9a-f] is refused rather than accepted
 *        as a weaker credential; a valid token folds to lower case and loses its whitespace.
 */
void TstServerAuth::normalizeToken()
{
  QFETCH(QString, input);
  QFETCH(QString, expected);

  QCOMPARE(API::Auth::normalizeToken(input), expected);
}

/**
 * @brief A generated token is 32 random bytes in hex, and survives its own validator unchanged.
 */
void TstServerAuth::generatedTokenIsUsable()
{
  const auto token = API::Auth::generateToken();
  QCOMPARE(token.size(), qsizetype(64));
  QCOMPARE(API::Auth::normalizeToken(token), token);
  QVERIFY(token != API::Auth::generateToken());
  QVERIFY(API::Auth::constantTimeEquals(token.toUtf8(), token.toUtf8()));
}

//--------------------------------------------------------------------------------------------------
// Command classification
//--------------------------------------------------------------------------------------------------

void TstServerAuth::commandClassification_data()
{
  QTest::addColumn<QString>("command");
  QTest::addColumn<bool>("scriptOnly");
  QTest::addColumn<bool>("writesToDevice");

  QTest::newRow("system.exec") << QStringLiteral("system.exec") << true << false;
  QTest::newRow("system.kill") << QStringLiteral("system.kill") << true << false;
  QTest::newRow("system.runningProcesses")
    << QStringLiteral("system.runningProcesses") << true << false;
  QTest::newRow("io.writeData") << QStringLiteral("io.writeData") << false << true;
  QTest::newRow("io.ble.writeCharacteristic")
    << QStringLiteral("io.ble.writeCharacteristic") << false << true;
  QTest::newRow("console.send") << QStringLiteral("console.send") << false << true;
  QTest::newRow("project.new") << QStringLiteral("project.new") << false << false;
  QTest::newRow("io.connect") << QStringLiteral("io.connect") << false << false;
  QTest::newRow("unknown command") << QStringLiteral("nope.nope") << false << false;
  QTest::newRow("case sensitive") << QStringLiteral("IO.writeData") << false << false;
}

/**
 * @brief The two allowlists behind authorizeRemoteCommand(): a control-script-only command is
 *        refused outright, a device-write command falls through to the consent gate, and
 *        everything else passes untouched.
 */
void TstServerAuth::commandClassification()
{
  QFETCH(QString, command);
  QFETCH(bool, scriptOnly);
  QFETCH(bool, writesToDevice);

  QCOMPARE(API::Auth::commandIsControlScriptOnly(command), scriptOnly);
  QCOMPARE(API::Auth::commandWritesToDevice(command), writesToDevice);
}

//--------------------------------------------------------------------------------------------------
// Authentication handshake
//--------------------------------------------------------------------------------------------------

/**
 * @brief A wrong token is answered with the challenge until the attempt budget runs out, and the
 *        connection is dropped on the third failure without ever reaching the command dispatcher.
 */
void TstServerAuth::authHandshakeRejectsBadToken()
{
  QTcpSocket socket;
  StubHost host;
  host.expectedToken = QByteArrayLiteral("goodtoken");

  API::ClientReception reception(host);
  auto state          = authenticatedState();
  state.authenticated = false;
  host.lastState      = &state;

  const QByteArray attempt = QByteArrayLiteral("{\"type\":\"auth\",\"token\":\"bad\"}\n");
  for (int i = 0; i < kMaxAuthAttempts; ++i)
    reception.consumeBytes(&socket, state.sessionId, attempt);

  QVERIFY(!state.authenticated);
  QCOMPARE(state.authAttempts, kMaxAuthAttempts);
  QCOMPARE(host.responses.size(), qsizetype(kMaxAuthAttempts - 1));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(1));
  QVERIFY(host.dispatchedCommands.isEmpty());
}

/**
 * @brief The correct token authenticates the connection and the bytes pipelined behind the
 *        handshake line are processed in the same pass, not stranded in the buffer.
 */
void TstServerAuth::authHandshakeAcceptsAndReplaysPipeline()
{
  QTcpSocket socket;
  StubHost host;
  host.expectedToken = QByteArrayLiteral("goodtoken");

  API::ClientReception reception(host);
  auto state          = authenticatedState();
  state.authenticated = false;
  host.lastState      = &state;

  reception.consumeBytes(
    &socket,
    state.sessionId,
    QByteArrayLiteral("{\"type\":\"auth\",\"token\":\"goodtoken\"}\n"
                      "{\"type\":\"command\",\"command\":\"api.getCommands\"}\n"));

  QVERIFY(state.authenticated);
  QVERIFY(state.buffer.isEmpty());
  QCOMPARE(host.disconnectCodes.size(), qsizetype(0));
  QCOMPARE(host.dispatchedCommands.size(), qsizetype(1));
  QCOMPARE(host.routedCommands, QStringList{QStringLiteral("api.getCommands")});
  QCOMPARE(host.responses.size(), qsizetype(2));

  const auto handshake = QJsonDocument::fromJson(host.responses.first()).object();
  QCOMPARE(handshake.value(QStringLiteral("success")).toBool(), true);
  QCOMPARE(handshake.value(QStringLiteral("result"))
             .toObject()
             .value(QStringLiteral("authenticated"))
             .toBool(),
           true);
}

//--------------------------------------------------------------------------------------------------
// JSON message validation
//--------------------------------------------------------------------------------------------------

/**
 * @brief A message past the size cap is refused with an error response, not a disconnect: an
 *        oversized single message is a client bug, not an attack on the buffer.
 */
void TstServerAuth::jsonMessageSizeLimit()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  QByteArray oversized(kMaxApiMessageBytes + 1, 'x');
  QVERIFY(!reception.validateJsonMessage(&socket, state, oversized));
  QCOMPARE(host.responses.size(), qsizetype(1));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(0));
  QCOMPARE(state.messageCount, 0);

  const QByteArray atCap =
    QByteArrayLiteral("{") + QByteArray(kMaxApiMessageBytes - 2, 'x') + QByteArrayLiteral("}");
  QCOMPARE(atCap.size(), qsizetype(kMaxApiMessageBytes));
  QVERIFY(reception.validateJsonMessage(&socket, state, atCap));
  QCOMPARE(host.responses.size(), qsizetype(1));
  QCOMPARE(state.messageCount, 1);
}

/**
 * @brief Nesting past the depth cap is refused before any JSON parser sees the payload, which is
 *        what keeps a deeply nested document from recursing inside QJsonDocument.
 */
void TstServerAuth::jsonMessageDepthLimit()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  const QByteArray shallow = QByteArray(kMaxApiJsonDepth, '[') + QByteArray(kMaxApiJsonDepth, ']');
  QVERIFY(reception.validateJsonMessage(&socket, state, shallow));
  QCOMPARE(state.messageCount, 1);
  QCOMPARE(host.responses.size(), qsizetype(0));

  const QByteArray tooDeep =
    QByteArray(kMaxApiJsonDepth + 1, '[') + QByteArray(kMaxApiJsonDepth + 1, ']');
  QVERIFY(!reception.validateJsonMessage(&socket, state, tooDeep));
  QCOMPARE(host.responses.size(), qsizetype(1));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(0));
}

/**
 * @brief The per-window message counter disconnects the client on the first message past the
 *        budget, and every accepted message before it advances the counter by exactly one.
 */
void TstServerAuth::jsonMessageRateLimit()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  const QByteArray message = QByteArrayLiteral("{\"type\":\"command\",\"command\":\"api.ping\"}");
  for (int i = 0; i < kMaxApiMessagesPerWindow; ++i)
    QVERIFY(reception.validateJsonMessage(&socket, state, message));

  QCOMPARE(state.messageCount, kMaxApiMessagesPerWindow);
  QVERIFY(!reception.validateJsonMessage(&socket, state, message));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(1));
}

//--------------------------------------------------------------------------------------------------
// Transport rate limits
//--------------------------------------------------------------------------------------------------

/**
 * @brief The byte budget is per window, so a client that already spent it is dropped on the next
 *        chunk however small that chunk is.
 */
void TstServerAuth::rateLimitsByteWindow()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  state.window.start();
  state.byteCount = kMaxApiBytesPerWindow - 1;
  QVERIFY(reception.validateRateLimits(&socket, state, QByteArrayLiteral("a")));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(0));

  QVERIFY(!reception.validateRateLimits(&socket, state, QByteArrayLiteral("a")));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(1));
}

/**
 * @brief A buffer that cannot absorb the incoming chunk drops the connection instead of growing:
 *        the cap is on the accumulated buffer, not on any single message.
 */
void TstServerAuth::rateLimitsBufferCap()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  state.window.start();
  state.buffer = QByteArray(kMaxApiBufferBytes, 'x');
  QVERIFY(!reception.validateRateLimits(&socket, state, QByteArrayLiteral("a")));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(1));
  QVERIFY(state.buffer.isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Raw byte paths
//--------------------------------------------------------------------------------------------------

/**
 * @brief Bytes with no newline and no JSON opener are the raw device lane: they reach the device
 *        once consent is granted and the buffer is cleared, so nothing is written twice.
 */
void TstServerAuth::noNewlineRawBufferReachesDevice()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  reception.consumeBytes(&socket, state.sessionId, QByteArrayLiteral("AT+RESET"));
  QCOMPARE(host.deviceWrites, QList<QByteArray>{QByteArrayLiteral("AT+RESET")});
  QVERIFY(state.buffer.isEmpty());

  host.consent = false;
  reception.consumeBytes(&socket, state.sessionId, QByteArrayLiteral("AT+AGAIN"));
  QCOMPARE(host.deviceWrites.size(), qsizetype(1));
  QVERIFY(state.buffer.isEmpty());
}

/**
 * @brief A newline-terminated raw line past the payload cap is answered and then closed, and the
 *        oversized bytes never reach the device.
 */
void TstServerAuth::rawLineOversizeClosesSocket()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  reception.consumeBytes(&socket, state.sessionId, QByteArray(kMaxApiRawBytes + 1, 'x') + '\n');
  QCOMPARE(host.closes, 1);
  QCOMPARE(host.responses.size(), qsizetype(1));
  QVERIFY(host.deviceWrites.isEmpty());

  reception.consumeBytes(&socket, state.sessionId, QByteArrayLiteral("short\n"));
  QCOMPARE(host.deviceWrites, QList<QByteArray>{QByteArrayLiteral("short")});
}

QTEST_GUILESS_MAIN(TstServerAuth)

#include "tst_server_auth.moc"
