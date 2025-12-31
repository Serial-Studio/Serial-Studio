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

#include "Player.h"

#include <QtMath>
#include <QTimer>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>

#include <map>
#include <algorithm>

#include <mdf/mdffile.h>
#include <mdf/ichannel.h>
#include <mdf/mdfreader.h>
#include <mdf/idatagroup.h>
#include <mdf/ichannelgroup.h>
#include <mdf/isampleobserver.h>

#include "IO/Manager.h"
#include "UI/Dashboard.h"
#include "Misc/Utilities.h"
#include "JSON/FrameBuilder.h"
#include "Misc/WorkspaceManager.h"

/**
 * @class SampleCacheObserver
 * @brief Observer class that caches channel values during MDF4 data reading
 *
 * This observer is attached to data groups during ReadData() operations.
 * It extracts channel values from incoming sample records and stores them
 * in a cache for efficient random-access playback.
 */
class SampleCacheObserver : public mdf::ISampleObserver
{
public:
  SampleCacheObserver(const mdf::IDataGroup &dataGroup,
                      std::map<uint64_t, std::vector<double>> &cache,
                      const std::vector<mdf::IChannel *> &allChannels,
                      const std::vector<mdf::IChannel *> &groupChannels)
    : mdf::ISampleObserver(dataGroup)
    , m_cache(cache)
    , m_allChannels(allChannels)
    , m_groupChannels(groupChannels)
  {
    for (size_t i = 0; i < m_allChannels.size(); ++i)
    {
      for (auto *ch : m_groupChannels)
      {
        if (m_allChannels[i] == ch)
        {
          m_channelIndexMap[ch] = i;
          break;
        }
      }
    }
  }

  bool OnSample(uint64_t sample, uint64_t record_id,
                const std::vector<uint8_t> &record) override
  {
    if (m_cache.find(sample) != m_cache.end())
      return true;

    std::vector<double> values(m_allChannels.size(), 0.0);

    for (auto *channel : m_groupChannels)
    {
      if (!channel)
        continue;

      auto it = m_channelIndexMap.find(channel);
      if (it == m_channelIndexMap.end())
        continue;

      double value = 0.0;
      const bool success = GetEngValue(*channel, record_id, record, value);

      if (!success)
      {
        const bool channelSuccess
            = GetChannelValue(*channel, record_id, record, value);
        if (!channelSuccess)
          value = 0.0;
      }

      values[it->second] = value;
    }

    m_cache[sample] = std::move(values);
    return true;
  }

private:
  std::map<uint64_t, std::vector<double>> &m_cache;
  const std::vector<mdf::IChannel *> &m_allChannels;
  const std::vector<mdf::IChannel *> &m_groupChannels;
  std::map<mdf::IChannel *, size_t> m_channelIndexMap;
};

/**
 * @class TimestampCacheObserver
 * @brief Observer class that caches master time channel values
 *
 * This observer extracts timestamp values from the master time channel
 * for Serial Studio-generated MDF4 files.
 */
class TimestampCacheObserver : public mdf::ISampleObserver
{
public:
  TimestampCacheObserver(const mdf::IDataGroup &dataGroup,
                         std::map<uint64_t, double> &timestampCache,
                         mdf::IChannel *masterTimeChannel)
    : mdf::ISampleObserver(dataGroup)
    , m_timestampCache(timestampCache)
    , m_masterTimeChannel(masterTimeChannel)
  {
  }

  bool OnSample(uint64_t sample, uint64_t record_id,
                const std::vector<uint8_t> &record) override
  {
    if (!m_masterTimeChannel
        || m_timestampCache.find(sample) != m_timestampCache.end())
      return true;

    double timestamp = 0.0;
    const bool success
        = GetEngValue(*m_masterTimeChannel, record_id, record, timestamp);

    if (!success)
    {
      const bool channelSuccess
          = GetChannelValue(*m_masterTimeChannel, record_id, record, timestamp);
      if (!channelSuccess)
        timestamp = 0.0;
    }

    m_timestampCache[sample] = timestamp;
    return true;
  }

private:
  std::map<uint64_t, double> &m_timestampCache;
  mdf::IChannel *m_masterTimeChannel;
};

/**
 * @brief Constructor - Initializes the MDF4 player
 *
 * Sets up event filtering for keyboard shortcuts and connects internal
 * signals for playback state management.
 */
MDF4::Player::Player()
  : m_reader(nullptr)
  , m_framePos(0)
  , m_playing(false)
  , m_isSerialStudioFile(false)
  , m_masterTimeChannel(nullptr)
  , m_timestamp("")
  , m_fileInfo("")
  , m_startTimestamp(0.0)
{
  qApp->installEventFilter(this);
  connect(this, &MDF4::Player::playerStateChanged, this,
          &MDF4::Player::updateData);
}

/**
 * @brief Destructor - Cleans up resources
 */
MDF4::Player::~Player() {}

/**
 * @brief Returns the singleton instance of the MDF4 player
 * @return Reference to the player instance
 */
MDF4::Player &MDF4::Player::instance()
{
  static Player singleton;
  return singleton;
}

/**
 * @brief Checks if an MDF4 file is currently open
 * @return True if a file is open and valid
 */
bool MDF4::Player::isOpen() const
{
  return m_reader != nullptr && m_reader->IsOk();
}

/**
 * @brief Returns the current playback progress
 * @return Progress as a value between 0.0 and 1.0
 */
double MDF4::Player::progress() const
{
  if (frameCount() == 0)
    return 0.0;

  return static_cast<double>(framePosition()) / frameCount();
}

/**
 * @brief Checks if playback is currently active
 * @return True if playing, false if paused
 */
bool MDF4::Player::isPlaying() const
{
  return m_playing;
}

/**
 * @brief Returns the total number of frames in the file
 * @return Frame count (size of frame index)
 */
int MDF4::Player::frameCount() const
{
  return static_cast<int>(m_frameIndex.size());
}

/**
 * @brief Returns the current frame position
 * @return Current frame index (0-based)
 */
int MDF4::Player::framePosition() const
{
  return m_framePos;
}

/**
 * @brief Returns the filename of the currently open file
 * @return Filename without path, or empty string if no file is open
 */
QString MDF4::Player::filename() const
{
  if (isOpen())
  {
    auto fileInfo = QFileInfo(m_filePath);
    return fileInfo.fileName();
  }

  return "";
}

/**
 * @brief Returns the current playback timestamp
 * @return Formatted timestamp string (HH:MM:SS.mmm)
 */
const QString &MDF4::Player::timestamp() const
{
  return m_timestamp;
}

/**
 * @brief Returns file metadata information
 * @return Multi-line string with author, project, subject, and channel count
 */
const QString &MDF4::Player::fileInfo() const
{
  return m_fileInfo;
}

/**
 * @brief Starts real-time playback from current position
 *
 * Begins playback using timestamps for synchronization. If at the end of
 * the file, playback restarts from the beginning.
 */
void MDF4::Player::play()
{
  if (!isOpen())
    return;

  if (m_framePos >= frameCount() - 1)
    m_framePos = 0;

  m_startTimestamp = m_frameIndex[m_framePos].timestamp;
  m_elapsedTimer.start();

  m_playing = true;
  Q_EMIT playerStateChanged();
}

/**
 * @brief Pauses playback at current position
 */
void MDF4::Player::pause()
{
  m_playing = false;
  Q_EMIT playerStateChanged();
}

/**
 * @brief Toggles between play and pause states
 */
void MDF4::Player::toggle()
{
  if (isPlaying())
    pause();
  else
    play();
}

/**
 * @brief Opens a file selection dialog for MDF4 files
 *
 * Displays a file dialog allowing the user to select an MF4 or DAT file.
 * Uses a non-native dialog for better cross-platform stability.
 * Calls openFile(QString) with the selected path.
 */
void MDF4::Player::openFile()
{
  auto *dialog
      = new QFileDialog(nullptr, tr("Select MDF4 file"),
                        Misc::WorkspaceManager::instance().path("MDF4"),
                        tr("MDF4 files (*.mf4 *.dat)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setOption(QFileDialog::DontUseNativeDialog);

  connect(dialog, &QFileDialog::fileSelected, this,
          [this, dialog](const QString &path) {
            if (!path.isEmpty())
              openFile(path);

            dialog->deleteLater();
          });

  dialog->open();
}

/**
 * @brief Opens an MDF4 file from the specified path
 * @param filePath Absolute path to the MDF4 file
 *
 * Performs the following operations:
 * 1. Disconnects from device if currently connected
 * 2. Opens the file using mdflib's MdfReader
 * 3. Validates file structure
 * 4. Builds frame index and reads measurement data
 * 5. Extracts file metadata (author, project, subject)
 */
void MDF4::Player::openFile(const QString &filePath)
{
  if (IO::Manager::instance().isConnected())
  {
    int response = Misc::Utilities::showMessageBox(
        tr("Disconnect from device?"),
        tr("You must disconnect from the current device before opening a "
           "MDF4 file."),
        QMessageBox::Question, "", QMessageBox::Yes | QMessageBox::No);

    if (response == QMessageBox::Yes)
      IO::Manager::instance().disconnectDevice();
    else
      return;
  }

  closeFile();

  m_reader = std::make_unique<mdf::MdfReader>(filePath.toStdString());

  if (!m_reader->IsOk())
  {
    Misc::Utilities::showMessageBox(
        tr("Cannot open MDF4 file"),
        tr("The file may be corrupted or in an unsupported format."),
        QMessageBox::Critical);
    closeFile();
    return;
  }

  if (!m_reader->ReadEverythingButData())
  {
    Misc::Utilities::showMessageBox(
        tr("Invalid MDF4 file"),
        tr("Failed to read file structure. The file may be corrupted."),
        QMessageBox::Critical);
    closeFile();
    return;
  }

  auto *header = m_reader->GetHeader();
  if (header)
  {
    QString author = QString::fromStdString(header->Author());
    m_isSerialStudioFile = (author == "Serial Studio");
  }

  buildFrameIndex();

  if (m_frameIndex.empty())
  {
    Misc::Utilities::showMessageBox(
        tr("No data in file"),
        tr("The MDF4 file contains no measurement data."),
        QMessageBox::Critical);
    closeFile();
    return;
  }

  m_filePath = filePath;

  if (header)
  {
    QString author = QString::fromStdString(header->Author());
    QString project = QString::fromStdString(header->Project());
    QString subject = QString::fromStdString(header->Subject());

    m_fileInfo = tr("Author: %1\nProject: %2\nSubject: %3\nChannels: %4")
                     .arg(author.isEmpty() ? tr("Unknown") : author)
                     .arg(project.isEmpty() ? tr("Unknown") : project)
                     .arg(subject.isEmpty() ? tr("Unknown") : subject)
                     .arg(m_channels.size());
  }

  m_framePos = 0;

  sendHeaderFrame();

  Q_EMIT openChanged();
  Q_EMIT fileInfoChanged();
  Q_EMIT playerStateChanged();
}

/**
 * @brief Closes the currently open file and releases resources
 *
 * Stops playback if active, closes the file, and clears all cached data
 * including the frame index, channel list, and sample cache.
 */
void MDF4::Player::closeFile()
{
  if (isPlaying())
    pause();

  m_reader.reset();
  m_frameIndex.clear();
  m_channels.clear();
  m_sampleCache.clear();
  m_timestampCache.clear();
  m_framePos = 0;
  m_isSerialStudioFile = false;
  m_masterTimeChannel = nullptr;
  m_filePath.clear();
  m_timestamp.clear();
  m_fileInfo.clear();

  JSON::FrameBuilder::instance().registerQuickPlotHeaders(QStringList());

  Q_EMIT openChanged();
  Q_EMIT fileInfoChanged();
  Q_EMIT playerStateChanged();
}

/**
 * @brief Advances to the next frame
 *
 * Pauses playback if active, advances one frame forward, and updates the
 * dashboard with a batch of frames for plot history.
 */
void MDF4::Player::nextFrame()
{
  if (!isOpen())
    return;

  if (isPlaying())
    pause();

  if (m_framePos < frameCount() - 1)
  {
    ++m_framePos;

    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame = std::max(0, m_framePos - framesToLoad);
    int endFrame = m_framePos - 1;

    if (endFrame >= startFrame)
      processFrameBatch(startFrame, endFrame);

    updateData();
  }
}

/**
 * @brief Steps back to the previous frame
 *
 * Pauses playback if active, moves one frame backward, clears plot data,
 * and reloads a batch of frames for plot history.
 */
void MDF4::Player::previousFrame()
{
  if (!isOpen())
    return;

  if (isPlaying())
    pause();

  if (m_framePos > 0)
  {
    --m_framePos;

    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame = std::max(0, m_framePos - framesToLoad);
    int endFrame = m_framePos - 1;

    UI::Dashboard::instance().clearPlotData();

    if (endFrame >= startFrame)
      processFrameBatch(startFrame, endFrame);

    updateData();
  }
}

/**
 * @brief Seeks to a specific position in the file
 * @param progress Playback position (0.0 to 1.0)
 *
 * Pauses playback, jumps to the specified position, clears plot data,
 * and loads a batch of frames around the new position.
 */
void MDF4::Player::setProgress(const double progress)
{
  if (!isOpen())
    return;

  if (isPlaying())
    pause();

  int newFramePos
      = qBound(0, static_cast<int>(progress * frameCount()), frameCount() - 1);

  if (newFramePos != m_framePos)
  {
    m_framePos = newFramePos;

    UI::Dashboard::instance().clearPlotData();

    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame = std::max(0, m_framePos - framesToLoad);
    int endFrame = std::min(frameCount() - 1, m_framePos);

    processFrameBatch(startFrame, endFrame);

    updateData();
  }
}

/**
 * @brief Updates current frame data and manages playback timing
 *
 * Called during playback to:
 * - Update timestamp display
 * - Send current frame to dashboard
 * - Calculate timing for next frame
 * - Schedule next update using QTimer
 *
 * Implements real-time playback by synchronizing with actual timestamps
 * from the MDF4 file. Processes multiple frames in a batch if falling
 * behind to maintain synchronization.
 */
void MDF4::Player::updateData()
{
  if (!isOpen())
    return;

  if (m_framePos >= 0 && m_framePos < frameCount())
  {
    m_timestamp = formatTimestamp(m_frameIndex[m_framePos].timestamp);
    sendFrame(m_framePos);
    Q_EMIT timestampChanged();
  }

  if (isPlaying())
  {
    if (framePosition() >= frameCount() - 1)
    {
      pause();
      return;
    }

    const qint64 elapsedMs = m_elapsedTimer.elapsed();
    const double targetTime = m_startTimestamp + (elapsedMs / 1000.0);

    const double nextTime = m_frameIndex[framePosition() + 1].timestamp;

    if (nextTime <= targetTime)
    {
      constexpr int kMaxBatchSize = 100;
      int processed = 0;

      while (m_framePos < frameCount() - 1 && processed < kMaxBatchSize)
      {
        ++m_framePos;
        sendFrame(m_framePos);
        ++processed;

        if (m_framePos >= frameCount() - 1)
          break;

        double frameTime = m_frameIndex[m_framePos].timestamp;
        if (frameTime > targetTime)
          break;
      }

      m_timestamp = formatTimestamp(m_frameIndex[m_framePos].timestamp);
      Q_EMIT timestampChanged();

      if (isPlaying())
        QTimer::singleShot(1, Qt::PreciseTimer, this,
                           &MDF4::Player::updateData);
    }
    else
    {
      qint64 delayMs
          = qMax(0LL, static_cast<qint64>((nextTime - targetTime) * 1000.0));

      QTimer::singleShot(delayMs, Qt::PreciseTimer, this, [this]() {
        if (isOpen() && isPlaying())
        {
          ++m_framePos;
          updateData();
        }
      });
    }
  }
}

/**
 * @brief Builds a sparse frame index for efficient streaming playback
 *
 * Iterates through all data groups and channel groups in the MDF4 file,
 * creating a lightweight index by sampling every Nth record. This allows
 * handling large (multi-GB) files without loading all data into memory.
 *
 * The index stores:
 * - Timestamp (derived from sample index)
 * - Record index within the channel group
 * - Pointer to the channel group for data access
 *
 * After building the index, ReadData() is called on each data group to
 * load the actual measurement data that can be accessed during playback.
 *
 * @note Sample interval (kSampleInterval) controls index granularity.
 *       Higher values = less memory, coarser seek resolution.
 */
void MDF4::Player::buildFrameIndex()
{
  m_frameIndex.clear();
  m_channels.clear();
  m_sampleCache.clear();
  m_timestampCache.clear();
  m_masterTimeChannel = nullptr;

  if (!m_reader || !m_reader->IsOk())
    return;

  auto *file = m_reader->GetFile();
  if (!file)
    return;

  mdf::DataGroupList dataGroups;
  file->DataGroups(dataGroups);

  if (dataGroups.empty())
    return;

  std::vector<mdf::IChannel *> allChannels;

  for (auto *dg : dataGroups)
  {
    if (!dg)
      continue;

    auto channelGroups = dg->ChannelGroups();
    if (channelGroups.empty())
      continue;

    for (auto *cg : channelGroups)
    {
      if (!cg)
        continue;

      auto cgChannels = cg->Channels();
      if (cgChannels.empty())
        continue;

      for (auto *ch : cgChannels)
      {
        if (ch
            && std::find(allChannels.begin(), allChannels.end(), ch)
                   == allChannels.end())
        {
          if (m_isSerialStudioFile && ch->Type() == mdf::ChannelType::Master)
          {
            m_masterTimeChannel = ch;
            continue;
          }

          allChannels.push_back(ch);
        }
      }
    }
  }

  m_channels = allChannels;

  for (auto *dg : dataGroups)
  {
    if (!dg)
      continue;

    auto channelGroups = dg->ChannelGroups();
    if (channelGroups.empty())
      continue;

    for (auto *cg : channelGroups)
    {
      if (!cg)
        continue;

      auto cgChannels = cg->Channels();
      uint64_t recordCount = cg->NofSamples();

      if (cgChannels.empty() || recordCount == 0)
        continue;

      SampleCacheObserver observer(*dg, m_sampleCache, m_channels, cgChannels);
      observer.AttachObserver();

      if (m_isSerialStudioFile && m_masterTimeChannel)
      {
        TimestampCacheObserver timeObserver(*dg, m_timestampCache,
                                            m_masterTimeChannel);
        timeObserver.AttachObserver();
        m_reader->ReadData(*dg);
        timeObserver.DetachObserver();
      }
      else
      {
        m_reader->ReadData(*dg);
      }

      observer.DetachObserver();
    }
  }

  if (m_sampleCache.empty())
    return;

  constexpr int kSampleInterval = 1;
  uint64_t sampleIndex = 0;

  for (const auto &cachePair : m_sampleCache)
  {
    if (sampleIndex % kSampleInterval == 0)
    {
      FrameIndex frameIdx;

      if (m_isSerialStudioFile && !m_timestampCache.empty())
      {
        auto timeIt = m_timestampCache.find(cachePair.first);
        if (timeIt != m_timestampCache.end())
          frameIdx.timestamp = timeIt->second;
        else
          frameIdx.timestamp = static_cast<double>(sampleIndex) * 0.001;
      }
      else
      {
        frameIdx.timestamp = static_cast<double>(sampleIndex) * 0.001;
      }

      frameIdx.recordIndex = cachePair.first;
      frameIdx.channelGroup = nullptr;

      m_frameIndex.push_back(frameIdx);
    }
    ++sampleIndex;
  }

  std::sort(m_frameIndex.begin(), m_frameIndex.end(),
            [](const FrameIndex &a, const FrameIndex &b) {
              return a.recordIndex < b.recordIndex;
            });
}

/**
 * @brief Processes a batch of frames for plot history
 * @param startFrame First frame index to process
 * @param endFrame Last frame index to process (inclusive)
 *
 * Sends multiple frames to the dashboard to populate plot history.
 * Used when seeking or stepping through frames to maintain context.
 */
void MDF4::Player::processFrameBatch(int startFrame, int endFrame)
{
  for (int i = startFrame; i <= endFrame; ++i)
  {
    if (i >= 0 && i < frameCount())
      sendFrame(i);
  }
}

/**
 * @brief Sends a single frame to the IO manager for processing
 * @param frameIndex Index of the frame to send
 *
 * Extracts frame data using getFrame() and sends it through the normal
 * IO::Manager pipeline for console display, FrameBuilder processing,
 * and dashboard visualization.
 */
void MDF4::Player::sendFrame(int frameIndex)
{
  if (frameIndex < 0 || frameIndex >= frameCount())
    return;

  IO::Manager::instance().processPayload(getFrame(frameIndex));
}

/**
 * @brief Registers MDF4 channel names with Quick Plot
 *
 * Extracts channel names from MDF4 file metadata and explicitly registers
 * them with the FrameBuilder. This ensures accurate channel naming in
 * Quick Plot mode.
 */
void MDF4::Player::sendHeaderFrame()
{
  if (m_channels.empty())
    return;

  QStringList headers;
  for (size_t i = 0; i < m_channels.size(); ++i)
  {
    auto *channel = m_channels[i];
    if (channel)
    {
      QString name = QString::fromStdString(channel->Name());
      if (name.isEmpty())
        name = QString("Channel_%1").arg(i + 1);

      headers.append(name);
    }
    else
    {
      headers.append(QString("Channel_%1").arg(i + 1));
    }
  }

  JSON::FrameBuilder::instance().registerQuickPlotHeaders(headers);
}

/**
 * @brief Formats a timestamp value as HH:MM:SS.mmm
 * @param timestamp Time in seconds
 * @return Formatted time string
 */
QString MDF4::Player::formatTimestamp(double timestamp) const
{
  int hours = static_cast<int>(timestamp / 3600.0);
  int minutes = static_cast<int>((timestamp - hours * 3600.0) / 60.0);
  double seconds = timestamp - hours * 3600.0 - minutes * 60.0;

  return QString("%1:%2:%3")
      .arg(hours, 2, 10, QChar('0'))
      .arg(minutes, 2, 10, QChar('0'))
      .arg(seconds, 6, 'f', 3, QChar('0'));
}

/**
 * @brief Extracts a frame of data at the specified index
 * @param index The frame index in the frame index vector
 * @return QByteArray containing comma-separated channel values
 *
 * Creates a CSV-format line with values for all channels at the specified
 * frame index. Values are retrieved from the sample cache that was populated
 * during ReadData() using the SampleCacheObserver.
 *
 * The output is sent through IO::Manager::processPayload() for display in
 * the console and processing by the FrameBuilder.
 *
 * @note Values are engineering values (post-conversion) extracted using
 *       mdflib's GetEngValue() method which applies channel conversions.
 */
QByteArray MDF4::Player::getFrame(const int index)
{
  if (index < 0 || index >= frameCount())
    return QByteArray();

  const auto &frameIdx = m_frameIndex[index];
  QByteArray frame;

  auto it = m_sampleCache.find(frameIdx.recordIndex);
  if (it != m_sampleCache.end())
  {
    const auto &values = it->second;
    for (size_t i = 0; i < values.size(); ++i)
    {
      frame.append(QString::number(values[i], 'g', 10).toUtf8());

      if (i < values.size() - 1)
        frame.append(',');
    }
  }
  else
  {
    for (size_t i = 0; i < m_channels.size(); ++i)
    {
      frame.append("0");
      if (i < m_channels.size() - 1)
        frame.append(',');
    }
  }

  frame.append('\n');
  return frame;
}

/**
 * @brief Event filter for capturing keyboard events
 * @param obj Object that received the event
 * @param event The event to filter
 * @return True if event was handled, false to propagate
 *
 * Filters keyboard events to provide playback shortcuts when a file is open.
 */
bool MDF4::Player::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::KeyPress)
  {
    auto *keyEvent = static_cast<QKeyEvent *>(event);
    return handleKeyPress(keyEvent);
  }

  return QObject::eventFilter(obj, event);
}

/**
 * @brief Handles keyboard shortcuts for playback control
 * @param keyEvent The key press event
 * @return True if key was handled, false otherwise
 *
 * Supported shortcuts:
 * - Space: Toggle play/pause
 * - Left Arrow: Previous frame
 * - Right Arrow: Next frame
 */
bool MDF4::Player::handleKeyPress(QKeyEvent *keyEvent)
{
  if (!isOpen())
    return false;

  if (keyEvent->key() == Qt::Key_Space)
  {
    toggle();
    return true;
  }

  else if (keyEvent->key() == Qt::Key_Left)
  {
    previousFrame();
    return true;
  }

  else if (keyEvent->key() == Qt::Key_Right)
  {
    nextFrame();
    return true;
  }

  return false;
}
