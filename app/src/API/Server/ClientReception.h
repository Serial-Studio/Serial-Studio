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

#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QString>

#include "API/Server/ConnectionState.h"

class QTcpSocket;

namespace API {

/**
 * @brief Everything the reception machine has to reach outside itself: the worker-thread writes,
 *        the command dispatchers, the device, and the credential check. Implemented by Server,
 *        which owns all four; the machine itself resolves no singleton and touches no socket, so
 *        a suite can drive it against a stub.
 */
class ReceptionHost {
public:
  ReceptionHost()                                = default;
  ReceptionHost(ReceptionHost&&)                 = delete;
  ReceptionHost(const ReceptionHost&)            = delete;
  ReceptionHost& operator=(ReceptionHost&&)      = delete;
  ReceptionHost& operator=(const ReceptionHost&) = delete;
  virtual ~ReceptionHost()                       = default;

  [[nodiscard]] virtual bool deviceConnected() const                            = 0;
  [[nodiscard]] virtual bool authorizeDeviceWrite()                             = 0;
  [[nodiscard]] virtual bool verifyToken(const QByteArray& provided) const      = 0;
  [[nodiscard]] virtual qint64 writeToDevice(const QByteArray& data)            = 0;
  [[nodiscard]] virtual QByteArray dispatchCommand(const QByteArray& jsonBytes) = 0;
  [[nodiscard]] virtual QByteArray dispatchMcp(const QByteArray& jsonBytes,
                                               const QString& sessionId)        = 0;
  [[nodiscard]] virtual bool routeConnectionCommand(QTcpSocket* socket,
                                                    ConnectionState& state,
                                                    const QJsonObject& json)    = 0;

  virtual void sendResponse(QTcpSocket* socket, const QByteArray& response)  = 0;
  virtual void closeSocket(QTcpSocket* socket, const ConnectionState& state) = 0;
  virtual void disconnectClient(QTcpSocket* socket,
                                ConnectionState& state,
                                const QString& errorCode,
                                const QString& errorMessage)                 = 0;
};

/**
 * @brief The API socket's reception state machine: rate and buffer limits, the token handshake,
 *        newline framing, the JSON size/depth guards, and the raw-byte fallback that forwards a
 *        non-JSON line to the device. One instance serves every connection; all per-client state
 *        lives in the ConnectionState it is handed.
 */
class ClientReception {
public:
  explicit ClientReception(ReceptionHost& host);
  ClientReception(ClientReception&&)                 = delete;
  ClientReception(const ClientReception&)            = delete;
  ClientReception& operator=(ClientReception&&)      = delete;
  ClientReception& operator=(const ClientReception&) = delete;

  void consumeBytes(QTcpSocket* socket, ConnectionState& state, const QByteArray& data);

  [[nodiscard]] bool validateRateLimits(QTcpSocket* socket,
                                        ConnectionState& state,
                                        const QByteArray& data);
  [[nodiscard]] bool validateJsonMessage(QTcpSocket* socket,
                                         ConnectionState& state,
                                         const QByteArray& jsonBytes);

private:
  void handleAuthHandshake(QTcpSocket* socket, ConnectionState& state, const QByteArray& data);
  void handleJsonMessage(QTcpSocket* socket, ConnectionState& state, const QByteArray& jsonBytes);
  void processRawJsonCommand(QTcpSocket* socket, ConnectionState& state, const QJsonObject& json);
  void processNoNewlineBuffer(QTcpSocket* socket, ConnectionState& state);
  void processBufferedJson(QTcpSocket* socket, ConnectionState& state, const QByteArray& trimmed);
  void processJsonLine(QTcpSocket* socket, ConnectionState& state, const QByteArray& trimmedLine);
  void processRawLine(QTcpSocket* socket, ConnectionState& state, const QByteArray& line);

  ReceptionHost& m_host;
};

}  // namespace API
