/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <chrono>
#  include <optional>
#  include <QElapsedTimer>
#  include <QHash>
#  include <QKeyEvent>
#  include <QMap>
#  include <QObject>
#  include <QSet>
#  include <QSqlDatabase>
#  include <QSqlQuery>
#  include <QString>
#  include <QTimer>
#  include <QVector>
#  include <vector>

#  include "SerialStudio.h"
#  include "Sessions/PlayerLoaderWorker.h"

class QThread;

namespace UI {
class Dashboard;
}  // namespace UI

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
  void performSeekTick();
  void performSeekSettle();
  void onLoadFinished(const Sessions::PlayerSessionPayloadPtr& payload);

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;
  bool handleKeyPress(QKeyEvent* keyEvent);

private:
  void initWorker();
  void joinWorker();
  void clearLocalState();
  void applyBundledViewState(const QString& viewState, const QString& projectJson);
  [[nodiscard]] static UI::Dashboard& viewStateDashboard();
  void teardownLocalDb();
  bool openLocalDb(const QString& filePath);
  void detectFinalValueColumns();
  void fillSeekWindowFromBlocks(int startRow,
                                int endRow,
                                const QHash<int, qint64>& keyByUid,
                                QHash<qint64, QVector<double>>& series);
  [[nodiscard]] QHash<int, QString> frameValuesFromBlocks(qint64 timestampNs);

  [[nodiscard]] bool restoreProjectFromJson(const QString& json);

  void capturePreSessionState();
  void restorePreSessionState();
  void schedulePreSessionRestore();
  void performPendingRestore();

  void alignColumnsToProject();
  void buildMultiSourceMapping();

  [[nodiscard]] QHash<int, QString> buildFrameAt(qint64 timestampNs);
  void injectFrame(const QHash<int, QString>& uidValues, qint64 timestampNs);
  void mergeStreamBlockTimes();
  void injectStreamBlocksAt(qint64 timestampNs);
  void replayStreamGroup(int sourceId, std::size_t first, std::size_t last);
  [[nodiscard]] bool fetchStreamSamples(const PlayerStreamBlockIndex& entry,
                                        std::vector<double>& out);
  void processFrameBatch(int startFrame, int endFrame);
  void anchorSteadyBase(int frameIndex);
  [[nodiscard]] int seekWindowStartRow(int target) const;
  void buildSeekWindow(int startRow,
                       int endRow,
                       QVector<double>& times,
                       QHash<qint64, QVector<double>>& series);
  [[nodiscard]] std::chrono::steady_clock::time_point rowSteadyTimestamp(qint64 timestampNs) const;

  void updateTimestampDisplay();
  [[nodiscard]] QString formatTimestamp(double seconds) const;

private:
  QThread* m_workerThread;
  PlayerLoaderWorker* m_worker;

  std::optional<QSqlDatabase> m_db;
  std::optional<QSqlQuery> m_frameQuery;
  std::optional<QSqlQuery> m_seekQuery;
  bool m_frameQueryPrepared;
  bool m_seekQueryPrepared;
  bool m_hasFinalValues;
  bool m_usesBlocks;
  QString m_filePath;
  QString m_connectionName;
  int m_sessionId;
  int m_pendingSessionId;

  bool m_loading;

  int m_framePos;
  bool m_playing;
  bool m_multiSource;
  bool m_injecting;
  QString m_timestamp;
  double m_startTimestampSeconds;
  double m_steadyBaseRowSeconds;
  std::chrono::steady_clock::time_point m_steadyBase;

  QElapsedTimer m_elapsedTimer;
  QTimer m_seekTimer;
  QTimer m_settleTimer;

  std::vector<int> m_columnUniqueIds;
  QMap<int, int> m_uidToColumn;

  std::vector<qint64> m_timestampsNs;

  std::vector<PlayerStreamBlockIndex> m_streamBlocks;
  std::optional<QSqlQuery> m_streamBlobQuery;
  std::optional<QSqlQuery> m_denseBlobQuery;
  std::vector<std::vector<double>> m_streamChannelBuf;

  QMap<int, int> m_columnToSource;
  QMap<int, std::vector<int>> m_sourceColumns;

  QSet<int> m_sourcesAtCurrentTs;

  bool m_preSessionCaptured;
  bool m_restorePending;
  SerialStudio::OperationMode m_preSessionOperationMode;
  QString m_preSessionProjectPath;
  QString m_preSessionViewState;
};

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL
