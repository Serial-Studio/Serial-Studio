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

#include <deque>
#include <QByteArray>
#include <QElapsedTimer>
#include <QHash>
#include <QHostAddress>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>

#include "API/CommandProtocol.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"
#include "IO/HAL_Driver.h"
#include "IO/StreamWorker.h"

#define API_TCP_PORT 7777

namespace API {
class Server;
class MirrorPublisher;

/**
 * @brief Worker that handles JSON serialization and socket I/O on a background thread.
 */
class ServerWorker : public DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr> {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void clientCountChanged(int count);
  void socketRemoved(QTcpSocket* socket, const QString& sessionId);
  void streamWriteDone(QTcpSocket* socket, const QString& sessionId);
  void dataReceived(QTcpSocket* socket, const QString& sessionId, const QByteArray& data);

public:
  ServerWorker(moodycamel::ReaderWriterQueue<DataModel::TimestampedFramePtr>* queue,
               std::atomic<bool>* enabled,
               std::atomic<size_t>* queueSize);
  ~ServerWorker() override;

  [[nodiscard]] bool isResourceOpen() const override;

public slots:
  void closeResources() override;
  void removeSocket(QTcpSocket* socket);
  void writeRawData(const QByteArray& data);
  void broadcastEvent(const QJsonObject& event);
  void addSocket(QTcpSocket* socket, const QString& sessionId);
  void disconnectSocket(QTcpSocket* socket, const QString& sessionId);
  void writeToSocket(QTcpSocket* socket, const QString& sessionId, const QByteArray& data);
  void writeMirrorPayload(QTcpSocket* socket, const QString& sessionId, const QByteArray& data);
  void writeStreamBlock(QTcpSocket* socket, const QString& sessionId, const QByteArray& data);
  void setSocketStreamFrames(QTcpSocket* socket, const QString& sessionId, const bool enabled);

protected:
  void processItems(const std::vector<DataModel::TimestampedFramePtr>& items) override;

private slots:
  void onSocketReadyRead();
  void onSocketDisconnected();

private:
  [[nodiscard]] bool underWriteCap(QTcpSocket* socket);

private:
  QHash<QTcpSocket*, QString> m_sockets;

  // Sockets whose client opted out of the per-frame broadcast; empty for every ordinary client
  QSet<QTcpSocket*> m_mutedSockets;

  // Sockets already reported as over-cap; per socket so a later client's stall is not swallowed
  QSet<QTcpSocket*> m_warnedSockets;

  quint64 m_droppedBroadcasts;
};

/**
 * @brief TCP server interface for API communication in Serial Studio.
 */
class Server : public DataModel::FrameConsumer<DataModel::TimestampedFramePtr> {
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
  [[nodiscard]] int clientCount() const noexcept;
  [[nodiscard]] QString authToken() const;
  [[nodiscard]] bool authorizeDeviceWrite();
  [[nodiscard]] bool externalConnections() const noexcept;
  [[nodiscard]] bool setAuthToken(const QString& token);
  [[nodiscard]] bool verifyToken(const QByteArray& provided) const;
  [[nodiscard]] bool authorizeRemoteCommand(const QString& command);

public slots:
  void removeConnection();
  void regenerateAuthToken();
  void allowExternalConnections();
  void setEnabled(const bool enabled);
  void setExternalConnections(const bool enabled);
  void hotpathTxData(const QByteArray& data);
  void hotpathTxFrame(const DataModel::TimestampedFramePtr& frame);
  void ingestStreamBlock(const IO::StreamBlockItemPtr& block);
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
  struct ConnectionState {
    QString sessionId;
    QString peerAddress;
    quint16 peerPort = 0;
    QByteArray buffer;
    QElapsedTimer window;
    int messageCount   = 0;
    int byteCount      = 0;
    bool authenticated = false;
    int authAttempts   = 0;

    // Mirror state; streamFrames defaults true so an unmodified client sees no change at all
    bool streamFrames     = true;
    bool mirrorSubscribed = false;
    int mirrorHz          = 20;
    int mirrorPrecision   = 0;

    // Typed stream-block subscription (spec 0051 M6): ack-paced, drop-oldest, counted
    bool streamSubscribed    = false;
    bool streamWriteInFlight = false;
    QSet<int> streamSources;
    quint64 streamSeq    = 0;
    quint64 streamMissed = 0;
    std::deque<IO::StreamBlockItemPtr> streamPending;
  };

  /**
   * @brief Tri-state user consent for API-originated device writes.
   */
  enum class DeviceWriteConsent {
    Unset,
    Granted,
    Denied
  };

  void ensureAuthToken();
  void applyExternalConnections(const bool enabled);
  void handleAuthHandshake(QTcpSocket* socket, ConnectionState& state, const QByteArray& data);
  [[nodiscard]] static bool constantTimeEquals(const QByteArray& a, const QByteArray& b);

  [[nodiscard]] MirrorPublisher& mirrorPublisher();
  [[nodiscard]] static bool isMirrorCommand(const QString& command);
  [[nodiscard]] static bool isStreamCommand(const QString& command);
  void handleStreamCommand(QTcpSocket* socket, ConnectionState& state, const QJsonObject& json);
  [[nodiscard]] CommandResponse streamSubscribe(ConnectionState& state,
                                                const CommandRequest& request);
  [[nodiscard]] CommandResponse streamUnsubscribe(ConnectionState& state,
                                                  const CommandRequest& request);
  void pumpStreamQueue(QTcpSocket* socket, ConnectionState& state);
  void setStreamFrames(QTcpSocket* socket, ConnectionState& state, const bool enabled);
  void handleMirrorCommand(QTcpSocket* socket, ConnectionState& state, const QJsonObject& json);
  [[nodiscard]] CommandResponse mirrorSubscribe(QTcpSocket* socket,
                                                ConnectionState& state,
                                                const CommandRequest& request);
  [[nodiscard]] CommandResponse mirrorSetRate(ConnectionState& state,
                                              const CommandRequest& request);
  [[nodiscard]] CommandResponse mirrorUnsubscribe(ConnectionState& state,
                                                  const CommandRequest& request);

  void sendResponseToSocket(QTcpSocket* socket, const QByteArray& response);
  void disconnectClient(QTcpSocket* socket,
                        ConnectionState& state,
                        const QString& errorCode,
                        const QString& errorMessage);
  [[nodiscard]] bool validateRateLimits(QTcpSocket* socket,
                                        ConnectionState& state,
                                        const QByteArray& data);
  [[nodiscard]] bool validateJsonMessage(QTcpSocket* socket,
                                         ConnectionState& state,
                                         const QByteArray& jsonBytes);
  void handleJsonMessage(QTcpSocket* socket, ConnectionState& state, const QByteArray& jsonBytes);
  void processRawJsonCommand(QTcpSocket* socket, ConnectionState& state, const QJsonObject& json);
  void processNoNewlineBuffer(QTcpSocket* socket, ConnectionState& state);
  void processBufferedJson(QTcpSocket* socket, ConnectionState& state, const QByteArray& trimmed);
  void processJsonLine(QTcpSocket* socket, ConnectionState& state, const QByteArray& trimmedLine);
  void processRawLine(QTcpSocket* socket, ConnectionState& state, const QByteArray& line);

private:
  QSettings m_settings;
  int m_clientCount;
  bool m_enabled;
  bool m_mirrorLinked;
  bool m_externalConnections;
  QString m_authToken;
  DeviceWriteConsent m_deviceWriteConsent;
  QTcpServer m_server;
  QHash<QTcpSocket*, ConnectionState> m_connections;
};
}  // namespace API
