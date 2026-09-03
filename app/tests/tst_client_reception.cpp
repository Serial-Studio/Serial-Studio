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

#include <functional>
#include <memory>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QTest>
#include <vector>

#include "API/CommandProtocol.h"
#include "API/Server/ClientReception.h"

/**
 * @brief A host that keeps its connections in the same shape Server does -- a QHash keyed by
 *        socket pointer -- and runs a caller-supplied hook inside the command dispatch. That hook
 *        is what a modal spinning a nested event loop does to the real server: the queued
 *        disconnect erases the entry, or a new connection rehashes the table and moves every
 *        remaining one. A reception machine that held a reference across the dispatch reads freed
 *        memory here, which is exactly what the sanitizer build is meant to catch (spec 0075 I1).
 */
class TableHost : public API::ReceptionHost {
public:
  bool deviceConnected() const override { return true; }

  API::DeviceWriteVerdict authorizeDeviceWrite() override { return verdict; }

  bool verifyToken(const QByteArray& provided) const override { return provided == expectedToken; }

  qint64 writeToDevice(const QByteArray& data) override
  {
    deviceWrites.append(data);
    return data.size();
  }

  QByteArray dispatchCommand(const QByteArray& jsonBytes) override
  {
    dispatchedCommands.append(jsonBytes);
    if (duringDispatch)
      duringDispatch();

    return QByteArrayLiteral("{\"type\":\"response\",\"success\":true}\n");
  }

  QByteArray dispatchMcp(const QByteArray& jsonBytes, const QString& sessionId) override
  {
    Q_UNUSED(jsonBytes);
    Q_UNUSED(sessionId);
    return QByteArray();
  }

  bool routeConnectionCommand(QTcpSocket* socket,
                              API::ConnectionState& state,
                              const QJsonObject& json) override
  {
    Q_UNUSED(socket);
    Q_UNUSED(state);
    Q_UNUSED(json);
    return false;
  }

  API::ConnectionState* stateFor(QTcpSocket* socket, const QString& sessionId) override
  {
    const auto it = connections.find(socket);
    if (it == connections.end() || it->sessionId != sessionId)
      return nullptr;

    return &it.value();
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

  int closes                      = 0;
  API::DeviceWriteVerdict verdict = API::DeviceWriteVerdict::Allowed;
  QByteArray expectedToken;
  QHash<QTcpSocket*, API::ConnectionState> connections;
  QList<QByteArray> responses;
  QList<QByteArray> deviceWrites;
  QList<QByteArray> dispatchedCommands;
  QStringList disconnectCodes;
  std::function<void()> duringDispatch;
};

/**
 * @brief The API reception machine driven against a connection table that mutates underneath it:
 *        entry erased mid-dispatch, table rehashed mid-dispatch, the HTTP sniff, the raw-forward
 *        handshake gate, the non-blocking consent refusal, and single-count byte accounting.
 */
class TstClientReception : public QObject {
  Q_OBJECT

private slots:
  void erasingTheEntryMidDispatchStopsTheLoop();
  void rehashingMidDispatchKeepsFramingTheRest();

  void pipelinedBytesAfterAuthAreCountedOnce();

  void httpRequestLineClosesTheConnection();
  void looksLikeHttpRequest_data();
  void looksLikeHttpRequest();

  void rawForwardingWaitsForOneJsonMessage();
  void consentRequiredRefusesWithoutWriting();

private:
  [[nodiscard]] static API::ConnectionState authenticatedState(const QString& sessionId);
  [[nodiscard]] static QString errorCodeOf(const QByteArray& response);
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief A connection past the token handshake and past the first JSON message.
 */
API::ConnectionState TstClientReception::authenticatedState(const QString& sessionId)
{
  API::ConnectionState state;
  state.sessionId      = sessionId;
  state.peerAddress    = QStringLiteral("127.0.0.1");
  state.peerPort       = 55000;
  state.authenticated  = true;
  state.handshakeSeen  = true;
  state.firstBytesSeen = true;
  return state;
}

/**
 * @brief The error code carried by one serialized CommandResponse, empty for a success.
 */
QString TstClientReception::errorCodeOf(const QByteArray& response)
{
  const auto object = QJsonDocument::fromJson(response).object();
  return object.value(QStringLiteral("error")).toObject().value(QStringLiteral("code")).toString();
}

//--------------------------------------------------------------------------------------------------
// Connection table mutated mid-dispatch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Two pipelined commands where the first dispatch drops the connection: the second line is
 *        never dispatched, and the loop stops instead of resuming on the erased entry.
 */
void TstClientReception::erasingTheEntryMidDispatchStopsTheLoop()
{
  QTcpSocket socket;
  TableHost host;
  host.connections.insert(&socket, authenticatedState(QStringLiteral("11")));

  API::ClientReception reception(host);
  host.duringDispatch = [&host, &socket] {
    host.connections.remove(&socket);
  };

  reception.consumeBytes(&socket,
                         QStringLiteral("11"),
                         QByteArrayLiteral("{\"type\":\"command\",\"command\":\"a\"}\n"
                                           "{\"type\":\"command\",\"command\":\"b\"}\n"));

  QCOMPARE(host.dispatchedCommands.size(), qsizetype(1));
  QVERIFY(host.connections.isEmpty());
  QCOMPARE(host.disconnectCodes.size(), qsizetype(0));
}

/**
 * @brief A connection accepted during the dispatch rehashes the table and moves every entry. The
 *        remaining pipelined line must still be framed, from the re-resolved state.
 */
void TstClientReception::rehashingMidDispatchKeepsFramingTheRest()
{
  QTcpSocket socket;
  TableHost host;
  host.connections.reserve(2);
  host.connections.insert(&socket, authenticatedState(QStringLiteral("11")));

  std::vector<std::unique_ptr<QTcpSocket>> latecomers;
  API::ClientReception reception(host);
  host.duringDispatch = [&host, &latecomers] {
    if (!latecomers.empty())
      return;

    for (int i = 0; i < 64; ++i) {
      latecomers.push_back(std::make_unique<QTcpSocket>());
      host.connections.insert(latecomers.back().get(),
                              authenticatedState(QString::number(100 + i)));
    }
  };

  reception.consumeBytes(&socket,
                         QStringLiteral("11"),
                         QByteArrayLiteral("{\"type\":\"command\",\"command\":\"a\"}\n"
                                           "{\"type\":\"command\",\"command\":\"b\"}\n"));

  QCOMPARE(host.dispatchedCommands.size(), qsizetype(2));
  QVERIFY(host.connections.value(&socket).buffer.isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Byte accounting
//--------------------------------------------------------------------------------------------------

/**
 * @brief The handshake and the bytes pipelined behind it arrive in one chunk, so the window
 *        counter must advance by that chunk exactly once (spec 0075 I12).
 */
void TstClientReception::pipelinedBytesAfterAuthAreCountedOnce()
{
  QTcpSocket socket;
  TableHost host;
  host.expectedToken = QByteArrayLiteral("goodtoken");

  auto state           = authenticatedState(QStringLiteral("12"));
  state.authenticated  = false;
  state.firstBytesSeen = false;
  host.connections.insert(&socket, state);

  const QByteArray chunk = QByteArrayLiteral("{\"type\":\"auth\",\"token\":\"goodtoken\"}\n"
                                             "{\"type\":\"command\",\"command\":\"a\"}\n");

  API::ClientReception reception(host);
  reception.consumeBytes(&socket, QStringLiteral("12"), chunk);

  QVERIFY(host.connections.value(&socket).authenticated);
  QCOMPARE(host.connections.value(&socket).byteCount, static_cast<int>(chunk.size()));
  QCOMPARE(host.dispatchedCommands.size(), qsizetype(1));
}

//--------------------------------------------------------------------------------------------------
// HTTP on the API socket
//--------------------------------------------------------------------------------------------------

/**
 * @brief A browser's simple cross-origin POST is closed on its request line: no header line
 *        reaches the device lane, no body reaches the dispatcher, and nothing readable is
 *        written back to it (spec 0075 I2).
 */
void TstClientReception::httpRequestLineClosesTheConnection()
{
  QTcpSocket socket;
  TableHost host;
  auto state           = authenticatedState(QStringLiteral("13"));
  state.firstBytesSeen = false;
  host.connections.insert(&socket, state);

  API::ClientReception reception(host);
  reception.consumeBytes(&socket,
                         QStringLiteral("13"),
                         QByteArrayLiteral("POST / HTTP/1.1\r\n"
                                           "Host: 127.0.0.1:7777\r\n"
                                           "Content-Type: text/plain\r\n\r\n"
                                           "{\"type\":\"command\",\"command\":\"a\"}\n"));

  QCOMPARE(host.closes, 1);
  QVERIFY(host.dispatchedCommands.isEmpty());
  QVERIFY(host.deviceWrites.isEmpty());
  QVERIFY(host.responses.isEmpty());
}

void TstClientReception::looksLikeHttpRequest_data()
{
  QTest::addColumn<QByteArray>("bytes");
  QTest::addColumn<bool>("expected");

  QTest::newRow("GET") << QByteArrayLiteral("GET / HTTP/1.1\r\n") << true;
  QTest::newRow("POST") << QByteArrayLiteral("POST /x HTTP/1.1\r\n") << true;
  QTest::newRow("OPTIONS") << QByteArrayLiteral("OPTIONS / HTTP/1.1\r\n") << true;
  QTest::newRow("CONNECT") << QByteArrayLiteral("CONNECT host:1 HTTP/1.1\r\n") << true;
  QTest::newRow("json command") << QByteArrayLiteral("{\"type\":\"command\"}\n") << false;
  QTest::newRow("raw device line") << QByteArrayLiteral("GETDATA\n") << false;
  QTest::newRow("posted word") << QByteArrayLiteral("POSTED\n") << false;
  QTest::newRow("empty") << QByteArray() << false;
}

/**
 * @brief The sniff keys on a method token followed by a space, so device traffic that merely
 *        starts with those letters is not mistaken for a browser.
 */
void TstClientReception::looksLikeHttpRequest()
{
  QFETCH(QByteArray, bytes);
  QFETCH(bool, expected);

  QCOMPARE(API::ClientReception::looksLikeHttpRequest(bytes), expected);
}

//--------------------------------------------------------------------------------------------------
// Raw lane gates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Raw device forwarding stays locked until the connection sent one valid JSON message,
 *        which is what stops stray header-shaped lines from reaching the hardware.
 */
void TstClientReception::rawForwardingWaitsForOneJsonMessage()
{
  QTcpSocket socket;
  TableHost host;
  auto state          = authenticatedState(QStringLiteral("14"));
  state.handshakeSeen = false;
  host.connections.insert(&socket, state);

  API::ClientReception reception(host);
  reception.consumeBytes(&socket, QStringLiteral("14"), QByteArrayLiteral("AT+RESET\n"));

  QVERIFY(host.deviceWrites.isEmpty());
  QCOMPARE(host.responses.size(), qsizetype(1));
  QCOMPARE(errorCodeOf(host.responses.first()),
           QString::fromLatin1(API::ErrorCode::InvalidMessageType));

  reception.consumeBytes(&socket,
                         QStringLiteral("14"),
                         QByteArrayLiteral("{\"type\":\"command\",\"command\":\"a\"}\n"
                                           "AT+RESET\n"));

  QCOMPARE(host.deviceWrites, QList<QByteArray>{QByteArrayLiteral("AT+RESET")});
}

/**
 * @brief An unanswered consent refuses the write with a retryable code instead of blocking the
 *        receive path on a modal; the same connection writes once consent is granted.
 */
void TstClientReception::consentRequiredRefusesWithoutWriting()
{
  QTcpSocket socket;
  TableHost host;
  host.connections.insert(&socket, authenticatedState(QStringLiteral("15")));
  host.verdict = API::DeviceWriteVerdict::ConsentRequired;

  API::ClientReception reception(host);
  reception.consumeBytes(&socket, QStringLiteral("15"), QByteArrayLiteral("AT+RESET\n"));

  QVERIFY(host.deviceWrites.isEmpty());
  QCOMPARE(errorCodeOf(host.responses.first()),
           QString::fromLatin1(API::ErrorCode::ConsentRequired));

  host.verdict = API::DeviceWriteVerdict::Allowed;
  reception.consumeBytes(&socket, QStringLiteral("15"), QByteArrayLiteral("AT+RESET\n"));
  QCOMPARE(host.deviceWrites, QList<QByteArray>{QByteArrayLiteral("AT+RESET")});
}

QTEST_GUILESS_MAIN(TstClientReception)

#include "tst_client_reception.moc"
