/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <chrono>
#include <memory>
#include <QElapsedTimer>
#include <QFile>
#include <QHash>
#include <QKeyEvent>
#include <QMap>
#include <QObject>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QVarLengthArray>
#include <QVector>

#include "CSV/PlayerLoaderWorker.h"

namespace CSV {
/**
 * @brief CSV player that re-plays a recording as if it were live data. The file stays on
 *        disk (memory-mapped); a background worker indexes row offsets + seconds, so opening
 *        is immediate and memory stays bounded regardless of file size (spec 0022).
 */
class Player : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY openChanged)
  Q_PROPERTY(double progress
             READ progress
             NOTIFY timestampChanged)
  Q_PROPERTY(double frameCount
             READ frameCount
             NOTIFY playerStateChanged)
  Q_PROPERTY(double framePosition
             READ framePosition
             NOTIFY timestampChanged)
  Q_PROPERTY(bool isPlaying
             READ isPlaying
             NOTIFY playerStateChanged)
  Q_PROPERTY(const QString& timestamp
             READ timestamp
             NOTIFY timestampChanged)
  Q_PROPERTY(bool indexing
             READ indexing
             NOTIFY indexingChanged)
  Q_PROPERTY(double indexProgress
             READ indexProgress
             NOTIFY indexingChanged)
  // clang-format on

signals:
  void openChanged();
  void timestampChanged();
  void playerStateChanged();
  void indexingChanged();

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
  [[nodiscard]] double progress() const;
  [[nodiscard]] bool isPlaying() const;
  [[nodiscard]] int frameCount() const;
  [[nodiscard]] int framePosition() const;
  [[nodiscard]] bool indexing() const;
  [[nodiscard]] double indexProgress() const;

  [[nodiscard]] QString filename() const;
  [[nodiscard]] const QString& timestamp() const;

public slots:
  void play();
  void pause();
  void toggle();
  void openFile();
  void closeFile();
  void nextFrame();
  void previousFrame();
  void openFile(const QString& filePath);
  void setProgress(const double progress);

private slots:
  void updateData();
  void performSeekTick();
  void performSeekSettle();
  void onIndexBatch(const CSV::PlayerIndexBatchPtr& batch);
  void onIndexFinished(bool ok, quint64 generation);

private:
  bool runQuickPass();
  void startIndexing();
  bool stopIndexing();
  void frontierPause();
  void sendHeaderFrame();
  void updateTimestampDisplay();
  void processFrameBatch(int startFrame, int endFrame);
  bool promptUserForDateTimeOrInterval(QByteArrayView firstDataRow);
  [[nodiscard]] double promptTimestampUnitScale();

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;
  bool handleKeyPress(QKeyEvent* keyEvent);

private:
  void buildReplayLayout();
  void injectFrame(const QByteArray& frame);
  void injectRow(int row);
  void anchorSteadyBase(int row);
  void buildSeekWindow(int startRow,
                       int endRow,
                       QVector<double>& times,
                       QHash<qint64, QVector<double>>& series);
  [[nodiscard]] int seekWindowStartRow(int target);
  [[nodiscard]] double rowSecondsSinceStart(int row) const;
  [[nodiscard]] int catchUpTargetRow(double target) const;
  [[nodiscard]] std::chrono::steady_clock::time_point rowSteadyTimestamp(int row);
  [[nodiscard]] QByteArrayView rawRow(int row) const;
  [[nodiscard]] qsizetype splitDataCells(int row);
  [[nodiscard]] QByteArray quickPlotPayload(int row);
  [[nodiscard]] bool recomputeMsUntilNext(qint64& msUntilNext);

private:
  int m_framePos;
  bool m_playing;
  bool m_multiSource;
  bool m_indexing;
  bool m_pausedAtFrontier;

  // Heap-held so a detached indexer's mapping outlives the player's own close (see closeFile)
  std::unique_ptr<QFile> m_csvFile;

  QString m_timestamp;

  const char* m_mapped;
  qint64 m_mappedSize;
  qint64 m_dataOffset;
  qint64 m_bytesIndexed;
  quint64 m_indexGeneration;
  quint64 m_playbackEpoch;
  QVector<quint64> m_rowOffsets;
  QVector<double> m_rowSeconds;

  PlayerTimestampMode m_tsMode;
  int m_timestampColumn;
  char m_separator;
  double m_timeScale;
  double m_intervalSeconds;
  qint64 m_anchorMs;
  double m_startSeconds;
  QStringList m_headerCells;

  QElapsedTimer m_elapsedTimer;
  QElapsedTimer m_catchUpFillTimer;
  double m_steadyBaseRowSeconds;
  std::chrono::steady_clock::time_point m_steadyBase;

  QThread* m_loaderThread;
  PlayerLoaderWorker* m_loader;

  DataModel::ReplayCellViews m_cells;
  QByteArray m_splitScratch;
  QVarLengthArray<QByteArrayView, 64> m_dataSpans;

  QTimer m_seekTimer;
  QTimer m_settleTimer;
  QHash<qint64, int> m_seekColumnByKey;

  QMap<int, QVector<int>> m_sourceColumnsByIndex;
};
}  // namespace CSV
