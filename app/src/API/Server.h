/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <atomic>
#include <QByteArray>
#include <QHash>
#include <QHostAddress>
#include <QJsonObject>
#include <QObject>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>

#include "API/CommandProtocol.h"
#include "API/Server/ClientReception.h"
#include "API/Server/ConnectionState.h"
#include "API/Server/ServerAuth.h"
#include "API/Server/ServerWorker.h"
#include "DataModel/DataBlock.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"
#include "IO/HAL_Driver.h"
#include "IO/StreamWorker.h"

#define API_TCP_PORT 7777

namespace API {
class MirrorPublisher;

/**
 * @brief TCP server interface for API communication in Serial Studio. Owns the listening socket,
 *        the per-connection table and the worker thread; the credential half lives in ServerAuth
 *        and the parse/validate half in ClientReception, both wired here by injection.
 */
class Server
  : public DataModel::FrameConsumer<DataModel::DataBlockPtr>
  , public ReceptionHost {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int clientCount
             READ clientCount
             NOTIFY clientCountChanged)
  Q_PROPERTY(bool enabled
             READ enabled
             WRITE setEnabled
             NOTIFY enabledChanged)
  Q_PROPERTY(bool externalConnections
             READ externalConnections
             WRITE setExternalConnections
             NOTIFY externalConnectionsChanged)
  Q_PROPERTY(QString authToken
             READ authToken
             NOTIFY authTokenChanged)
  // clang-format on

signals:
  void enabledChanged();
  void authTokenChanged();
  void clientCountChanged();
  void streamSubscribersChanged();
  void externalConnectionsChanged();

private:
  explicit Server();
  Server(Server&&)                 = delete;
  Server(const Server&)            = delete;
  Server& operator=(Server&&)      = delete;
  Server& operator=(const Server&) = delete;

  ~Server();

public:
  [[nodiscard]] static Server& instance();
  [[nodiscard]] static int maxClients() noexcept;
  [[nodiscard]] bool enabled() const noexcept;
  [[nodiscard]] bool hasStreamSubscribers() const noexcept;
  [[nodiscard]] int clientCount() const noexcept;
  [[nodiscard]] QString authToken() const;
  [[nodiscard]] bool authorizeDeviceWrite() override;
  [[nodiscard]] bool externalConnections() const noexcept;
  [[nodiscard]] bool setAuthToken(const QString& token);
  [[nodiscard]] bool verifyToken(const QByteArray& provided) const override;
  [[nodiscard]] bool authorizeRemoteCommand(const QString& command);

public slots:
  void removeConnection();
  void regenerateAuthToken();
  void allowExternalConnections();
  void setEnabled(const bool enabled);
  void setExternalConnections(const bool enabled);
  void hotpathTxData(const QByteArray& data);
  void setupExternalConnections();
  void ingestBlock(const DataModel::DataBlockPtr& block);

private:
  void pushStreamBlock(const DataModel::DataBlockPtr& block);
  void refreshStreamSubscriberFlag() noexcept;

public slots:
  void broadcastLifecycleEvent(const QString& eventName);

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;

private slots:
  void acceptConnection();
  void onClientCountChanged(int count);
  void onStreamWriteDone(QTcpSocket* socket, const QString& sessionId);
  void sendMirrorPayload(QTcpSocket* socket, const QString& sessionId, const QByteArray& payload);
  void onErrorOccurred(const QAbstractSocket::SocketError socketError);
  void onSocketDisconnected(QTcpSocket* socket, const QString& sessionId);
  void onDataReceived(QTcpSocket* socket, const QString& sessionId, const QByteArray& data);

private:
  void applyExternalConnections(const bool enabled);

  [[nodiscard]] bool deviceConnected() const override;
  [[nodiscard]] qint64 writeToDevice(const QByteArray& data) override;
  [[nodiscard]] QByteArray dispatchCommand(const QByteArray& jsonBytes) override;
  [[nodiscard]] QByteArray dispatchMcp(const QByteArray& jsonBytes,
                                       const QString& sessionId) override;
  [[nodiscard]] bool routeConnectionCommand(QTcpSocket* socket,
                                            ConnectionState& state,
                                            const QJsonObject& json) override;

  void sendResponse(QTcpSocket* socket, const QByteArray& response) override;
  void closeSocket(QTcpSocket* socket, const ConnectionState& state) override;
  void disconnectClient(QTcpSocket* socket,
                        ConnectionState& state,
                        const QString& errorCode,
                        const QString& errorMessage) override;

  [[nodiscard]] MirrorPublisher& mirrorPublisher();
  void handleMirrorCommand(QTcpSocket* socket, ConnectionState& state, const QJsonObject& json);
  void setStreamFrames(QTcpSocket* socket, ConnectionState& state, const bool enabled);

  [[nodiscard]] static bool isStreamCommand(const QString& command);
  void handleStreamCommand(QTcpSocket* socket, ConnectionState& state, const QJsonObject& json);
  [[nodiscard]] CommandResponse streamSubscribe(ConnectionState& state,
                                                const CommandRequest& request);
  [[nodiscard]] CommandResponse streamUnsubscribe(ConnectionState& state,
                                                  const CommandRequest& request);
  void pumpStreamQueue(QTcpSocket* socket, ConnectionState& state);

private:
  QSettings m_settings;
  ServerAuth m_auth;
  ClientReception m_reception;
  int m_clientCount;
  bool m_enabled;
  bool m_mirrorLinked;
  bool m_externalConnections;
  QTcpServer m_server;
  QHash<QTcpSocket*, ConnectionState> m_connections;
  alignas(64) std::atomic<bool> m_anyStreamSubscriber;
};
}  // namespace API
