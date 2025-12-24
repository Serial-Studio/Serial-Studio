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
#include <vector>
#include <memory>

#include <QObject>
#include <QString>
#include <QKeyEvent>
#include <QByteArray>
#include <QElapsedTimer>

namespace mdf
{
class MdfReader;
class IChannel;
class IChannelGroup;
} // namespace mdf

namespace MDF4
{
/**
 * @class Player
 * @brief MDF4 file player for Serial Studio
 *
 * The MDF4::Player class provides playback functionality for MDF4/MF4 binary
 * measurement files. Unlike the CSV player, this implementation uses a
 * streaming architecture with sparse frame indexing to efficiently handle
 * large files (multi-GB) without loading everything into memory.
 *
 * ## Features
 * - Supports all MDF4 measurement types (CAN, LIN, FlexRay, analog, etc.)
 * - Memory-efficient streaming with sparse sampling
 * - Real-time playback with timestamp synchronization
 * - Manual frame navigation (next/previous)
 * - Seek/scrub support via progress slider
 * - Keyboard shortcuts (Space = play/pause, Arrow keys = next/prev frame)
 *
 * ## Architecture
 * The player builds a sparse frame index on file open, sampling every Nth
 * record to create a lightweight lookup table. During playback, frames are
 * streamed on-demand and sent through the standard IO::Manager pipeline for
 * processing, visualization, and export.
 *
 * ## Data Flow
 * MDF4 File → buildFrameIndex() → getFrame() → IO::Manager::processPayload()
 * → FrameBuilder → Dashboard
 *
 * ## Usage Example
 * @code
 * MDF4::Player::instance().openFile("/path/to/file.mf4");
 * MDF4::Player::instance().play();
 * @endcode
 *
 * @note This is a singleton class. Use MDF4::Player::instance() to access it.
 *
 * @see CSV::Player for similar functionality with CSV files
 * @see IO::Manager for data pipeline integration
 */
class Player : public QObject
{
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
  Q_PROPERTY(const QString& fileInfo
             READ fileInfo
             NOTIFY fileInfoChanged)
  // clang-format on

signals:
  void openChanged();
  void timestampChanged();
  void playerStateChanged();
  void fileInfoChanged();

private:
  explicit Player();
  ~Player();
  Player(Player &&) = delete;
  Player(const Player &) = delete;
  Player &operator=(Player &&) = delete;
  Player &operator=(const Player &) = delete;

public:
  static Player &instance();

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] double progress() const;
  [[nodiscard]] bool isPlaying() const;
  [[nodiscard]] int frameCount() const;
  [[nodiscard]] int framePosition() const;

  [[nodiscard]] QString filename() const;
  [[nodiscard]] const QString &timestamp() const;
  [[nodiscard]] const QString &fileInfo() const;

public slots:
  void play();
  void pause();
  void toggle();
  void openFile();
  void closeFile();
  void nextFrame();
  void previousFrame();
  void openFile(const QString &filePath);
  void setProgress(const double progress);

private slots:
  void updateData();
  void buildFrameIndex();
  void sendFrame(int frameIndex);
  void processFrameBatch(int startFrame, int endFrame);

private:
  void sendHeaderFrame();
  QByteArray getFrame(const int index);
  QString formatTimestamp(double timestamp) const;

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;
  bool handleKeyPress(QKeyEvent *keyEvent);

private:
  struct FrameIndex
  {
    double timestamp;
    uint64_t recordIndex;
    mdf::IChannelGroup *channelGroup;
  };

  std::unique_ptr<mdf::MdfReader> m_reader;
  std::vector<FrameIndex> m_frameIndex;
  std::vector<mdf::IChannel *> m_channels;
  std::map<uint64_t, std::vector<double>> m_sampleCache;
  std::map<uint64_t, double> m_timestampCache;

  int m_framePos;
  bool m_playing;
  bool m_isSerialStudioFile;
  mdf::IChannel *m_masterTimeChannel;
  QString m_filePath;
  QString m_timestamp;
  QString m_fileInfo;

  QElapsedTimer m_elapsedTimer;
  double m_startTimestamp;
};
} // namespace MDF4
