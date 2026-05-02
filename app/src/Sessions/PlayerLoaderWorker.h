/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <atomic>
#  include <memory>
#  include <QObject>
#  include <QString>
#  include <vector>

class QSqlDatabase;

namespace Sessions {

/**
 * @brief Bundle of per-session data the player needs to start playback.
 */
struct PlayerSessionPayload {
  bool ok;
  QString error;
  QString filePath;
  int sessionId;
  QString projectJson;
  std::vector<int> columnUniqueIds;
  std::vector<qint64> timestampsNs;
};

/**
 * @brief Shared pointer alias for PlayerSessionPayload, exchanged across threads.
 */
using PlayerSessionPayloadPtr = std::shared_ptr<PlayerSessionPayload>;

/**
 * @brief Worker that performs the heavy session-loading SQL off the main thread.
 *
 * Owns a worker-thread @c QSqlDatabase connection only for the duration of a
 * single @c openAndLoad call. The connection is opened, queried, and closed in
 * one slot invocation. Per-frame fetches stay on the main thread under the
 * Player's own connection.
 */
class PlayerLoaderWorker : public QObject {
  Q_OBJECT

signals:
  void loaded(const Sessions::PlayerSessionPayloadPtr& payload);

public:
  explicit PlayerLoaderWorker(QObject* parent = nullptr);
  ~PlayerLoaderWorker() override;

  PlayerLoaderWorker(PlayerLoaderWorker&&)                 = delete;
  PlayerLoaderWorker(const PlayerLoaderWorker&)            = delete;
  PlayerLoaderWorker& operator=(PlayerLoaderWorker&&)      = delete;
  PlayerLoaderWorker& operator=(const PlayerLoaderWorker&) = delete;

  void requestCancel();

public slots:
  void openAndLoad(const QString& filePath, int sessionId);

private:
  [[nodiscard]] bool resolveSessionId(QSqlDatabase& db, int& sessionId, QString& errorOut);
  void loadProjectJson(QSqlDatabase& db, int sessionId, PlayerSessionPayload& payload);
  [[nodiscard]] bool loadColumnOrder(QSqlDatabase& db,
                                     int sessionId,
                                     PlayerSessionPayload& payload,
                                     QString& errorOut);
  [[nodiscard]] bool loadTimestampIndex(QSqlDatabase& db,
                                        int sessionId,
                                        PlayerSessionPayload& payload,
                                        QString& errorOut);

private:
  std::atomic<bool> m_cancelRequested;
};

}  // namespace Sessions

Q_DECLARE_METATYPE(Sessions::PlayerSessionPayloadPtr)

#endif  // BUILD_COMMERCIAL
