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

#  include <memory>
#  include <QElapsedTimer>
#  include <QHash>
#  include <QKeyEvent>
#  include <QObject>
#  include <QString>
#  include <QTimer>
#  include <QVector>
#  include <vector>

#  include "Core/SerialStudio.h"
#  include "Core/SSAssert.h"
#  include "DataModel/ReplayPlaybackEngine.h"
#  include "Sessions/Player/PreSessionState.h"
#  include "Sessions/Player/ReplaySynthesis.h"
#  include "Sessions/PlayerLoaderWorker.h"

class QThread;

namespace Core::Bus {
class MessageBus;
}  // namespace Core::Bus

namespace DataModel {
class IReplayPlotSink;
}  // namespace DataModel

namespace IO {
class IPayloadInjector;
}  // namespace IO

namespace Sessions {

/**
 * @brief Replays Serial Studio SQLite export files as if they were live data. The facade owns the
 *        transport of playback and composes three sub-objects for the work itself: SessionDbReader,
 *        ReplaySynthesis and PreSessionState. The singletons those need are resolved here and
 *        handed in, never reached for from inside.
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

  void setPlotSink(DataModel::IReplayPlotSink* sink) noexcept { m_plotSink = sink; }

  void setPayloadInjector(IO::IPayloadInjector* injector) noexcept { m_payloadInjector = injector; }

  void attachMessageBus(Core::Bus::MessageBus& bus);

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool loading() const;
  [[nodiscard]] bool isPlaying() const;
  [[nodiscard]] int frameCount() const;
  [[nodiscard]] int framePosition() const;
  [[nodiscard]] double progress() const;
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
  [[nodiscard]] DataModel::IReplayPlotSink& plotSink() const
  {
    SS_ASSERT(m_plotSink != nullptr, qFatal("Sessions::Player: plot sink reached before binding"));
    return *m_plotSink;
  }

  [[nodiscard]] IO::IPayloadInjector& payloadInjector() const
  {
    SS_ASSERT(m_payloadInjector != nullptr,
              qFatal("Sessions::Player: payload injector reached before binding"));
    return *m_payloadInjector;
  }

  void initWorker();
  void joinWorker();
  void clearLocalState();
  void applyProjectLayout();
  void registerQuickPlotColumns();
  void applyBundledViewState(const QString& viewState, const QString& projectJson);
  [[nodiscard]] ReplaySynthesis& synthesis();

  [[nodiscard]] bool restoreProjectFromJson(const QString& json);

  void capturePreSessionState();
  void restorePreSessionState();
  void schedulePreSessionRestore();
  void performPendingRestore();

  void anchorSteadyBase(int frameIndex);
  void processFrameBatch(int startFrame, int endFrame);
  [[nodiscard]] int seekWindowStartRow(int target) const;
  void buildSeekWindow(int startRow,
                       int endRow,
                       QVector<double>& times,
                       QHash<qint64, QVector<double>>& series);

  void updateTimestampDisplay();

private:
  QThread* m_workerThread;
  PlayerLoaderWorker* m_worker;

  bool m_loading;
  bool m_playing;
  int m_framePos;

  int m_pendingSessionId;
  QString m_filePath;
  QString m_timestamp;
  double m_startTimestampSeconds;

  QElapsedTimer m_elapsedTimer;
  DataModel::ReplayPlaybackEngine m_engine;

  std::vector<qint64> m_timestampsNs;

  ReplayLayout m_layout;
  SessionDbReader m_reader;
  std::unique_ptr<ReplaySynthesis> m_synthesis;

  bool m_restorePending;
  PreSessionState m_preSession;

  Core::Bus::MessageBus* m_bus;
  DataModel::IReplayPlotSink* m_plotSink;
  IO::IPayloadInjector* m_payloadInjector;
};

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL
