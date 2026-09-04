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

#include <QAbstractSocket>
#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QTimer>

#include "API/Mirror/MirrorProtocol.h"

namespace API {

/**
 * @brief Viewing-side transport of the remote dashboard mirror (spec 0040): one main-thread
 *        QTcpSocket, the normative connect sequence, push decoding, and liveness as two facts,
 *        because an idle capture and a dead link are otherwise identical silence -- `stale` means
 *        nothing arrived within the watchdog, `live` means snapshots rather than only heartbeats.
 */
class MirrorClient : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void liveChanged();
  void staleChanged();
  void linkedChanged();
  void snapshotReceived(const QJsonObject& snapshot);
  void structureReceived(const QJsonObject& structure);
  void failed(const QString& code, const QString& message, const bool fatal);

public:
  explicit MirrorClient(QObject* parent = nullptr);
  MirrorClient(MirrorClient&&)                 = delete;
  MirrorClient(const MirrorClient&)            = delete;
  MirrorClient& operator=(MirrorClient&&)      = delete;
  MirrorClient& operator=(const MirrorClient&) = delete;

public:
  [[nodiscard]] int hz() const noexcept;
  [[nodiscard]] bool live() const noexcept;
  [[nodiscard]] bool stale() const noexcept;
  [[nodiscard]] bool linked() const noexcept;
  [[nodiscard]] quint64 epoch() const noexcept;
  [[nodiscard]] const QString& endpoint() const noexcept;
  [[nodiscard]] const QString& lastError() const noexcept;
  [[nodiscard]] const QString& lastErrorCode() const noexcept;
  [[nodiscard]] const QJsonObject& info() const noexcept;

public slots:
  void close();
  void open(const QString& host, const quint16 port, const QString& token, const int hz);

private slots:
  void onWatchdog();
  void onConnected();
  void onReconnect();
  void onReadyRead();
  void onDisconnected();
  void onSocketError(const QAbstractSocket::SocketError error);

private:
  /**
   * @brief Where the connect sequence has got to. Subscribe is sent before anything else, so the
   *        per-frame firehose is off before a capture running at parse rate can flood the socket.
   */
  enum class Stage {
    Offline,
    Connecting,
    Subscribing,
    Fetching,
    Streaming
  };

  void sendAuth();
  void armWatchdog();
  void resetStreamState();
  void requestStructure();
  void scheduleReconnect();
  void setLive(const bool value);
  void setStale(const bool value);
  void handleLine(const QByteArray& line);
  void handleChunk(const QJsonObject& payload);
  void handlePush(const QJsonObject& payload);
  void onSubscribed(const QJsonObject& result);
  void handleSnapshot(const QJsonObject& payload);
  void handleResponse(const QJsonObject& message);
  void adoptStructure(const QJsonObject& payload);
  void onStructureFetched(const QJsonObject& result);
  void fail(const QString& code, const QString& message);
  void sendCommand(const QString& command, const QJsonObject& params);
  [[nodiscard]] bool verifyStructure(const QJsonObject& payload);

  QString m_host;
  QString m_token;
  QString m_endpoint;
  QString m_lastError;
  QString m_lastErrorCode;
  QString m_pendingId;
  QString m_pendingCommand;
  QJsonObject m_pendingParams;
  QJsonObject m_info;

  quint16 m_port;
  int m_hz;
  int m_attempt;
  bool m_live;
  bool m_stale;
  bool m_authSent;
  bool m_structurePending;
  bool m_userClosed;
  quint64 m_epoch;
  quint64 m_requestCounter;
  int m_valueCount;

  Stage m_stage;
  QByteArray m_buffer;
  QTcpSocket m_socket;
  QTimer m_watchdog;
  QTimer m_reconnect;

  int m_chunkParts;
  quint64 m_chunkEpoch;
  QHash<int, QString> m_chunks;
};

}  // namespace API
