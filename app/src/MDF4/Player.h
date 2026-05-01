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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <map>
#include <memory>
#include <QByteArray>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QMap>
#include <QObject>
#include <QString>
#include <vector>

namespace mdf {
class MdfReader;
class IChannel;
class IChannelGroup;
}  // namespace mdf

namespace MDF4 {
/**
 * @brief MDF4 file player for Serial Studio.
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
  // clang-format on

signals:
  void openChanged();
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
  [[nodiscard]] bool isPlaying() const;
  [[nodiscard]] int frameCount() const;
  [[nodiscard]] double progress() const;
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
  void buildFrameIndex();
  void sendFrame(int frameIndex);
  void processFrameBatch(int startFrame, int endFrame);

private:
  void buildFrameIndexFromCache();

  void sendHeaderFrame();
  void buildMultiSourceMapping();
  void injectFrame(const QByteArray& frame, int frameIndex = -1);
  QByteArray getFrame(const int index);
  [[nodiscard]] QString formatTimestamp(double timestamp) const;

protected:
  bool handleKeyPress(QKeyEvent* keyEvent);
  bool eventFilter(QObject* obj, QEvent* event) override;

private:
  struct FrameIndex {
    double timestamp;
    uint64_t recordIndex;
    mdf::IChannelGroup* channelGroup;
  };

  int m_framePos;
  bool m_playing;
  bool m_multiSource;
  bool m_isSerialStudioFile;
  QString m_filePath;
  QString m_timestamp;
  double m_startTimestamp;
  QElapsedTimer m_elapsedTimer;
  mdf::IChannel* m_masterTimeChannel;
  std::vector<FrameIndex> m_frameIndex;
  std::vector<mdf::IChannel*> m_channels;
  std::unique_ptr<mdf::MdfReader> m_reader;
  std::map<uint64_t, double> m_timestampCache;
  std::map<uint64_t, std::vector<double>> m_sampleCache;
  std::map<uint64_t, std::vector<bool>> m_activeChannels;

  QMap<int, int> m_channelToSource;
  QMap<int, int> m_sourceChannelCount;

  QMap<int, QVector<int>> m_sourceChannelsByIndex;
};
}  // namespace MDF4
