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

#include <cstddef>
#include <cstdint>
#include <QByteArray>
#include <QCoreApplication>
#include <QTcpSocket>

#include "API/Server/ClientReception.h"

/**
 * @brief A host that answers every question the reception machine asks and records nothing.
 *        It never dispatches into the application: what is under test is the framing, the
 *        limits, the HTTP sniff and the handshake gates, not any command handler.
 */
class FuzzHost : public API::ReceptionHost {
public:
  bool deviceConnected() const override { return true; }

  API::DeviceWriteVerdict authorizeDeviceWrite() override
  {
    return API::DeviceWriteVerdict::Allowed;
  }

  bool verifyToken(const QByteArray& provided) const override
  {
    return provided == QByteArrayLiteral("fuzz");
  }

  qint64 writeToDevice(const QByteArray& data) override { return data.size(); }

  QByteArray dispatchCommand(const QByteArray& jsonBytes) override
  {
    Q_UNUSED(jsonBytes);
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
    Q_UNUSED(socket);
    if (closed || sessionId != session.sessionId)
      return nullptr;

    return &session;
  }

  void sendResponse(QTcpSocket* socket, const QByteArray& response) override
  {
    Q_UNUSED(socket);
    Q_UNUSED(response);
  }

  void closeSocket(QTcpSocket* socket, const API::ConnectionState& state) override
  {
    Q_UNUSED(socket);
    Q_UNUSED(state);
    closed = true;
  }

  void disconnectClient(QTcpSocket* socket,
                        API::ConnectionState& state,
                        const QString& errorCode,
                        const QString& errorMessage) override
  {
    Q_UNUSED(socket);
    Q_UNUSED(errorCode);
    Q_UNUSED(errorMessage);
    state.buffer.clear();
    closed = true;
  }

  bool closed = false;
  API::ConnectionState session;
};

/**
 * @brief The Qt runtime the machine needs, built once per process.
 */
static QCoreApplication& fuzzApplication()
{
  if (auto* existing = QCoreApplication::instance())
    return *existing;

  static int argc       = 1;
  static char program[] = "fuzz_api_json";
  static char* argv[]   = {program, nullptr};
  static QCoreApplication instance(argc, argv);
  return instance;
}

/**
 * @brief The socket pointer the machine carries around; it is never dereferenced here, since
 *        every socket effect goes through the host.
 */
static QTcpSocket& fuzzSocket()
{
  static QTcpSocket socket;
  return socket;
}

/**
 * @brief Feeds one input to the API reception machine, once as a single chunk on an
 *        already-authenticated connection and once split in two on a connection that still owes
 *        the token handshake, which is what exercises the handshake and HTTP-sniff branches.
 */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if (size == 0 || size > (1u << 20))
    return 0;

  fuzzApplication();
  const QByteArray input(reinterpret_cast<const char*>(data), static_cast<qsizetype>(size));

  {
    FuzzHost host;
    host.session.sessionId      = QStringLiteral("1");
    host.session.peerAddress    = QStringLiteral("127.0.0.1");
    host.session.authenticated  = true;
    host.session.handshakeSeen  = true;
    host.session.firstBytesSeen = true;

    API::ClientReception reception(host);
    reception.consumeBytes(&fuzzSocket(), host.session.sessionId, input);
  }

  {
    FuzzHost host;
    host.session.sessionId   = QStringLiteral("2");
    host.session.peerAddress = QStringLiteral("127.0.0.1");

    API::ClientReception reception(host);
    const qsizetype cut = input.size() / 2;
    if (cut > 0)
      reception.consumeBytes(&fuzzSocket(), host.session.sessionId, input.left(cut));

    if (!host.closed && cut < input.size())
      reception.consumeBytes(&fuzzSocket(), host.session.sessionId, input.mid(cut));
  }

  return 0;
}
