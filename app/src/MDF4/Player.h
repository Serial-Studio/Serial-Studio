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
#include <QByteArray>
#include <QElapsedTimer>
#include <QHash>
#include <QKeyEvent>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QVarLengthArray>
#include <QVector>
#include <vector>

#include "DataModel/FrameBuilder.h"
#include "MDF4/PlayerLoaderWorker.h"

namespace MDF4 {
/**
 * @brief MDF4 file player for Serial Studio. Decode runs on a background worker (spec 0022);
 *        playback reads only the columnar payload -- no mdflib object lives on this class.
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
  [[nodiscard]] bool isPlaying() const;
  [[nodiscard]] int frameCount() const;
  [[nodiscard]] double progress() const;
  [[nodiscard]] bool indexing() const;
  [[nodiscard]] double indexProgress() const;
  [[nodiscard]] QString filename() const;
  [[nodiscard]] int framePosition() const;
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
  void sendFrame(int frameIndex);
  void processFrameBatch(int startFrame, int endFrame);
  void performSeekTick();
  void performSeekSettle();
  void onDecodeProgress(double fraction, quint64 generation);
  void onDecodeFinished(const MDF4::PlayerDecodePayloadPtr& payload);

private:
  void startDecoding(const QString& filePath);
  void stopDecoding();
  void catchUpToTarget(double targetTime);
  void sendHeaderFrame();
  void buildReplayLayout();
  void injectFrame(const QByteArray& frame, int frameIndex = -1);
  void injectRow(int frameIndex);
  void injectSourceRow(int frameIndex, int sourceId);
  void backfillSparseSources();
  [[nodiscard]] QVector<quint8> buildChannelSourceBits();
  void anchorSteadyBase(int frameIndex);
  [[nodiscard]] int seekWindowStartRow(int target) const;
  void buildSeekWindow(int startRow,
                       int endRow,
                       QVector<double>& times,
                       QHash<qint64, QVector<double>>& series);
  [[nodiscard]] bool buildRowCells(int index, QStringList& cells) const;
  [[nodiscard]] qsizetype buildRowCellsTyped(
    int index, QVarLengthArray<DataModel::FrameBuilder::ReplayCell, 128>& cells) const;
  [[nodiscard]] QByteArray getFrame(const int index);
  [[nodiscard]] std::chrono::steady_clock::time_point rowSteadyTimestamp(int frameIndex) const;
  [[nodiscard]] QString formatTimestamp(double timestamp) const;
  [[nodiscard]] bool channelActive(int channel, int frameIndex) const;

protected:
  bool handleKeyPress(QKeyEvent* keyEvent);
  bool eventFilter(QObject* obj, QEvent* event) override;

private:
  int m_framePos;
  bool m_injecting;
  bool m_playing;
  bool m_open;
  bool m_decoding;
  bool m_multiSource;
  double m_decodeProgress;
  QString m_filePath;
  QString m_timestamp;
  double m_startTimestamp;
  double m_steadyBaseRowSeconds;
  std::chrono::steady_clock::time_point m_steadyBase;
  QElapsedTimer m_elapsedTimer;
  QElapsedTimer m_catchUpFillTimer;
  QTimer m_seekTimer;
  QTimer m_settleTimer;
  QHash<qint64, int> m_seekColumnByKey;
  QVector<quint8> m_rowSourceBits;
  QVector<int> m_bitSourceIds;
  QVector<int> m_lastSourceRow;

  quint64 m_playbackEpoch;
  quint64 m_decodeGeneration;
  QThread* m_loaderThread;
  PlayerLoaderWorker* m_loader;
  QVarLengthArray<DataModel::FrameBuilder::ReplayCell, 128> m_typedCells;

  QStringList m_channelNames;
  std::vector<bool> m_channelIsString;
  std::vector<double> m_timestamps;
  std::vector<std::vector<double>> m_numeric;
  std::vector<std::vector<QString>> m_text;
  std::vector<std::vector<bool>> m_active;

  QMap<int, QVector<int>> m_sourceChannelsByIndex;
};
}  // namespace MDF4
