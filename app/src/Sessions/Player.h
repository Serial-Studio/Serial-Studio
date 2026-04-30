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

#  include <QElapsedTimer>
#  include <QKeyEvent>
#  include <QMap>
#  include <QObject>
#  include <QSqlDatabase>
#  include <QSqlQuery>
#  include <QString>
#  include <vector>

#  include "SerialStudio.h"
#  include "Sessions/PlayerLoaderWorker.h"

class QThread;

namespace Sessions {

/**
 * @brief Replays Serial Studio SQLite export files as if they were live data.
 */
class Player : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY openChanged)
  Q_PROPERTY(bool loading
             READ loading
             NOTIFY loadingChanged)
  Q_PROPERTY(double progress
             READ progress
             NOTIFY timestampChanged)
  Q_PROPERTY(int frameCount
             READ frameCount
             NOTIFY playerStateChanged)
  Q_PROPERTY(int framePosition
             READ framePosition
             NOTIFY timestampChanged)
  Q_PROPERTY(bool isPlaying
             READ isPlaying
             NOTIFY playerStateChanged)
  Q_PROPERTY(const QString& timestamp
             READ timestamp
             NOTIFY timestampChanged)
  // clang-format on

signals:
  void openChanged();
  void loadingChanged();
  void timestampChanged();
  void playerStateChanged();

private:
  explicit Player();
  Player(Player&&)                 = delete;
  Player(const Player&)            = delete;
  Player& operator=(Player&&)      = delete;
  Player& operator=(const Player&) = delete;
  ~Player();

public:
  [[nodiscard]] static Player& instance();

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool loading() const;
  [[nodiscard]] bool isPlaying() const;
  [[nodiscard]] int frameCount() const;
  [[nodiscard]] int framePosition() const;
  [[nodiscard]] double progress() const;
  [[nodiscard]] QString filename() const;
  [[nodiscard]] const QString& timestamp() const;

  void shutdown();

public slots:
  void play();
  void pause();
  void toggle();
  void openFile();
  void closeFile();
  void nextFrame();
  void previousFrame();
  void openFile(const QString& filePath);
  void openFile(const QString& filePath, int sessionId);
  void setProgress(const double progress);

private slots:
  void updateData();
  void onLoadFinished(const Sessions::PlayerSessionPayloadPtr& payload);

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;
  bool handleKeyPress(QKeyEvent* keyEvent);

private:
  void initWorker();
  void clearLocalState();
  void teardownLocalDb();
  bool openLocalDb(const QString& filePath);

  [[nodiscard]] bool restoreProjectFromJson(const QString& json);

  void capturePreSessionState();
  void restorePreSessionState();

  void alignColumnsToProject();
  void buildMultiSourceMapping();

  [[nodiscard]] QByteArray buildFrameAt(qint64 timestampNs);
  void injectFrame(const QByteArray& frame);
  void processFrameBatch(int startFrame, int endFrame);

  void updateTimestampDisplay();
  [[nodiscard]] QString formatTimestamp(double seconds) const;

private:
  QThread* m_workerThread;
  PlayerLoaderWorker* m_worker;

  QSqlDatabase m_db;
  QSqlQuery m_frameQuery;
  bool m_frameQueryPrepared;
  QString m_filePath;
  QString m_connectionName;
  int m_sessionId;
  int m_pendingSessionId;

  bool m_loading;

  int m_framePos;
  bool m_playing;
  bool m_multiSource;
  QString m_timestamp;
  double m_startTimestampSeconds;

  QElapsedTimer m_elapsedTimer;

  std::vector<int> m_columnUniqueIds;
  QMap<int, int> m_uidToColumn;

  std::vector<qint64> m_timestampsNs;

  QMap<int, int> m_columnToSource;
  QMap<int, int> m_sourceColumnCount;

  bool m_preSessionCaptured;
  SerialStudio::OperationMode m_preSessionOperationMode;
  QString m_preSessionProjectPath;
};

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL
