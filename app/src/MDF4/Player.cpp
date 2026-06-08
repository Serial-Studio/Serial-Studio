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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "Player.h"

#include <algorithm>
#include <map>
#include <mdf/ichannel.h>
#include <mdf/ichannelgroup.h>
#include <mdf/idatagroup.h>
#include <mdf/isampleobserver.h>
#include <mdf/mdffile.h>
#include <mdf/mdfreader.h>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTimer>
#include <QtMath>
#include <unordered_map>

#include "AppState.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Helper observer classes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Observer class that caches channel values during MDF4 data reading
 */
class SampleCacheObserver : public mdf::ISampleObserver {
public:
  /**
   * @brief Constructs the observer and indexes per-group channels for fast lookup.
   */
  SampleCacheObserver(const mdf::IDataGroup& dataGroup,
                      std::map<uint64_t, std::vector<double>>& cache,
                      std::map<uint64_t, double>& timestampCache,
                      std::map<uint64_t, std::vector<bool>>& activeChannels,
                      const std::vector<mdf::IChannel*>& allChannels,
                      const std::vector<mdf::IChannel*>& groupChannels,
                      mdf::IChannel* groupTimeChannel,
                      uint64_t recordId)
    : mdf::ISampleObserver(dataGroup)
    , m_cache(cache)
    , m_timestampCache(timestampCache)
    , m_activeChannels(activeChannels)
    , m_allChannels(allChannels)
    , m_groupChannels(groupChannels)
    , m_groupTimeChannel(groupTimeChannel)
    , m_recordId(recordId)
  {
    for (size_t i = 0; i < m_allChannels.size(); ++i) {
      for (auto* ch : m_groupChannels) {
        if (m_allChannels[i] == ch) {
          m_channelIndexMap[ch] = i;
          break;
        }
      }
    }
  }

  /**
   * @brief Caches per-channel sample values keyed by timestamp (or sample index as fallback).
   */
  bool OnSample(uint64_t sample, uint64_t record_id, const std::vector<uint8_t>& record) override
  {
    if (record_id != m_recordId)
      return true;

    uint64_t cacheKey = sample;
    if (m_groupTimeChannel) {
      double ts          = 0.0;
      const bool success = GetEngValue(*m_groupTimeChannel, record_id, record, ts);
      if (!success)
        GetChannelValue(*m_groupTimeChannel, record_id, record, ts);

      cacheKey                   = static_cast<uint64_t>(ts * 1'000'000'000.0);
      m_timestampCache[cacheKey] = ts;
    }

    auto cacheIt = m_cache.find(cacheKey);
    if (cacheIt == m_cache.end())
      cacheIt = m_cache.emplace(cacheKey, std::vector<double>(m_allChannels.size(), 0.0)).first;

    auto activeIt = m_activeChannels.find(cacheKey);
    if (activeIt == m_activeChannels.end())
      activeIt =
        m_activeChannels.emplace(cacheKey, std::vector<bool>(m_allChannels.size(), false)).first;

    auto& values = cacheIt->second;
    auto& active = activeIt->second;

    for (auto* channel : m_groupChannels) {
      if (!channel)
        continue;

      auto it = m_channelIndexMap.find(channel);
      if (it == m_channelIndexMap.end())
        continue;

      double value       = 0.0;
      const bool success = GetEngValue(*channel, record_id, record, value);

      if (!success) {
        const bool channelSuccess = GetChannelValue(*channel, record_id, record, value);
        if (!channelSuccess)
          value = 0.0;
      }

      values[it->second] = value;
      active[it->second] = true;
    }

    return true;
  }

private:
  std::map<uint64_t, std::vector<double>>& m_cache;
  std::map<uint64_t, double>& m_timestampCache;
  std::map<uint64_t, std::vector<bool>>& m_activeChannels;
  const std::vector<mdf::IChannel*>& m_allChannels;
  const std::vector<mdf::IChannel*>& m_groupChannels;
  mdf::IChannel* m_groupTimeChannel;
  uint64_t m_recordId;
  std::map<mdf::IChannel*, size_t> m_channelIndexMap;
};

/**
 * @brief Reads timestamp values from a single master time channel
 */
class LegacyTimestampObserver : public mdf::ISampleObserver {
public:
  /**
   * @brief Constructs the observer bound to a master time channel and record ID.
   */
  LegacyTimestampObserver(const mdf::IDataGroup& dataGroup,
                          std::map<uint64_t, double>& timestampCache,
                          mdf::IChannel* masterTimeChannel,
                          uint64_t recordId)
    : mdf::ISampleObserver(dataGroup)
    , m_timestampCache(timestampCache)
    , m_masterTimeChannel(masterTimeChannel)
    , m_recordId(recordId)
  {}

  /**
   * @brief Records the master-channel timestamp for the given sample index.
   */
  bool OnSample(uint64_t sample, uint64_t record_id, const std::vector<uint8_t>& record) override
  {
    if (record_id != m_recordId)
      return true;

    if (!m_masterTimeChannel || m_timestampCache.find(sample) != m_timestampCache.end())
      return true;

    double timestamp   = 0.0;
    const bool success = GetEngValue(*m_masterTimeChannel, record_id, record, timestamp);

    if (!success) {
      const bool channelSuccess =
        GetChannelValue(*m_masterTimeChannel, record_id, record, timestamp);
      if (!channelSuccess)
        timestamp = 0.0;
    }

    m_timestampCache[sample] = timestamp;
    return true;
  }

private:
  std::map<uint64_t, double>& m_timestampCache;
  mdf::IChannel* m_masterTimeChannel;
  uint64_t m_recordId;
};

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructor - Initializes the MDF4 player
 */
MDF4::Player::Player()
  : m_framePos(0)
  , m_playing(false)
  , m_multiSource(false)
  , m_isSerialStudioFile(false)
  , m_timestamp("")
  , m_startTimestamp(0.0)
  , m_masterTimeChannel(nullptr)
  , m_reader(nullptr)
{
  qApp->installEventFilter(this);
  connect(this, &MDF4::Player::playerStateChanged, this, &MDF4::Player::updateData);
}

/**
 * @brief Destructor - Cleans up resources
 */
MDF4::Player::~Player() {}

/**
 * @brief Returns the singleton instance of the MDF4 player
 */
MDF4::Player& MDF4::Player::instance()
{
  static Player singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Playback status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks if an MDF4 file is currently open
 */
bool MDF4::Player::isOpen() const
{
  return m_reader != nullptr && m_reader->IsOk();
}

/**
 * @brief Checks if playback is currently active
 */
bool MDF4::Player::isPlaying() const
{
  return m_playing;
}

/**
 * @brief Returns the total number of frames in the file
 */
int MDF4::Player::frameCount() const
{
  return static_cast<int>(m_frameIndex.size());
}

/**
 * @brief Returns the current playback progress
 */
double MDF4::Player::progress() const
{
  if (frameCount() == 0)
    return 0.0;

  return static_cast<double>(framePosition()) / frameCount();
}

/**
 * @brief Returns the filename of the currently open file
 */
QString MDF4::Player::filename() const
{
  if (isOpen()) {
    auto fileInfo = QFileInfo(m_filePath);
    return fileInfo.fileName();
  }

  return "";
}

/**
 * @brief Returns the current frame position
 */
int MDF4::Player::framePosition() const
{
  return m_framePos;
}

/**
 * @brief Returns the current playback timestamp
 */
const QString& MDF4::Player::timestamp() const
{
  return m_timestamp;
}

//--------------------------------------------------------------------------------------------------
// Playback control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts real-time playback from current position
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

//--------------------------------------------------------------------------------------------------
// File operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a file selection dialog for MDF4 files
 */
void MDF4::Player::openFile()
{
  auto* dialog = new QFileDialog(qApp->activeWindow(),
                                 tr("Select MDF4 file"),
                                 Misc::WorkspaceManager::instance().path("MDF4"),
                                 tr("MDF4 files (*.mf4 *.dat)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(this, [this, path]() { openFile(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Opens an MDF4 file from the specified path
 */
void MDF4::Player::openFile(const QString& filePath)
{
  if (IO::ConnectionManager::instance().isConnected()) {
    int response = Misc::Utilities::showMessageBox(
      tr("Disconnect from device?"),
      tr("You must disconnect from the current device before opening a MDF4 file."),
      QMessageBox::Question,
      "",
      QMessageBox::Yes | QMessageBox::No);

    if (response == QMessageBox::Yes)
      IO::ConnectionManager::instance().disconnectDevice();
    else
      return;
  }

  closeFile();

  m_reader = std::make_unique<mdf::MdfReader>(filePath.toStdString());

  if (!m_reader->IsOk()) {
    Misc::Utilities::showMessageBox(tr("Cannot open MDF4 file"),
                                    tr("The file may be corrupted or in an unsupported format."),
                                    QMessageBox::Critical);
    closeFile();
    return;
  }

  if (!m_reader->ReadEverythingButData()) {
    Misc::Utilities::showMessageBox(tr("Invalid MDF4 file"),
                                    tr("Failed to read file structure. The file may be corrupted."),
                                    QMessageBox::Critical);
    closeFile();
    return;
  }

  auto* header = m_reader->GetHeader();
  if (header) {
    QString author       = QString::fromStdString(header->Author());
    m_isSerialStudioFile = (author == "Serial Studio");
  }

  buildFrameIndex();

  if (m_frameIndex.empty()) {
    Misc::Utilities::showMessageBox(tr("No data in file"),
                                    tr("The MDF4 file contains no measurement data."),
                                    QMessageBox::Critical);
    closeFile();
    return;
  }

  m_filePath = filePath;
  m_framePos = 0;

  sendHeaderFrame();

  Q_EMIT openChanged();
  Q_EMIT playerStateChanged();
}

/**
 * @brief Closes the currently open file and releases resources
 */
void MDF4::Player::closeFile()
{
  if (!isOpen())
    return;

  m_framePos = 0;
  m_reader.reset();
  m_filePath.clear();
  m_channels.clear();
  m_timestamp.clear();
  m_frameIndex.clear();
  m_sampleCache.clear();
  m_timestampCache.clear();
  m_activeChannels.clear();
  m_multiSource        = false;
  m_isSerialStudioFile = false;
  m_masterTimeChannel  = nullptr;
  m_sourceChannelsByIndex.clear();

  DataModel::FrameBuilder::instance().registerQuickPlotHeaders(QStringList());
  DataModel::FrameBuilder::instance().setReplayColumnMap({});

  Q_EMIT openChanged();
  Q_EMIT playerStateChanged();
}

/**
 * @brief Advances to the next frame
 */
void MDF4::Player::nextFrame()
{
  if (!isOpen())
    return;

  if (isPlaying())
    pause();

  if (m_framePos < frameCount() - 1) {
    ++m_framePos;

    UI::Dashboard::instance().clearPlotData();

    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame   = std::max(0, m_framePos - framesToLoad);
    int endFrame     = std::min(frameCount() - 1, m_framePos);

    processFrameBatch(startFrame, endFrame);
    updateData();
  }
}

/**
 * @brief Steps back to the previous frame
 */
void MDF4::Player::previousFrame()
{
  if (!isOpen())
    return;

  if (isPlaying())
    pause();

  if (m_framePos > 0) {
    --m_framePos;

    UI::Dashboard::instance().clearPlotData();

    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame   = std::max(0, m_framePos - framesToLoad);
    int endFrame     = std::min(frameCount() - 1, m_framePos);

    processFrameBatch(startFrame, endFrame);
    updateData();
  }
}

//--------------------------------------------------------------------------------------------------
// Progress & seeking
//--------------------------------------------------------------------------------------------------

/**
 * @brief Seeks to a specific position in the file
 */
void MDF4::Player::setProgress(const double progress)
{
  if (!isOpen())
    return;

  if (isPlaying())
    pause();

  int newFramePos = qBound(0, static_cast<int>(progress * frameCount()), frameCount() - 1);

  if (newFramePos != m_framePos) {
    m_framePos = newFramePos;

    UI::Dashboard::instance().clearPlotData();

    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame   = std::max(0, m_framePos - framesToLoad);
    int endFrame     = std::min(frameCount() - 1, m_framePos);

    processFrameBatch(startFrame, endFrame);

    updateData();
  }
}

//--------------------------------------------------------------------------------------------------
// Data processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Plays back as many frames as needed to catch up with the target timestamp.
 */
void MDF4::Player::catchUpToTarget(double targetTime)
{
  constexpr int kMaxBatchSize = 100;
  int processed               = 0;

  while (m_framePos < frameCount() - 1 && processed < kMaxBatchSize) {
    if (!isOpen() || !isPlaying())
      break;

    ++m_framePos;
    sendFrame(m_framePos);
    ++processed;

    if (m_framePos >= frameCount() - 1)
      break;

    const double nextFrameTime = m_frameIndex[m_framePos + 1].timestamp;
    if (nextFrameTime > targetTime)
      break;
  }

  if (isOpen() && static_cast<size_t>(m_framePos) < m_frameIndex.size()) {
    m_timestamp = formatTimestamp(m_frameIndex[m_framePos].timestamp);
    Q_EMIT timestampChanged();
  }

  if (isPlaying())
    QTimer::singleShot(1, Qt::PreciseTimer, this, &MDF4::Player::updateData);
}

/**
 * @brief Updates current frame data and manages playback timing.
 */
void MDF4::Player::updateData()
{
  if (!isOpen())
    return;

  if (m_framePos >= 0 && m_framePos < frameCount()) {
    m_timestamp = formatTimestamp(m_frameIndex[m_framePos].timestamp);
    Q_EMIT timestampChanged();
  }

  if (!isPlaying())
    return;

  if (framePosition() >= frameCount() - 1) {
    pause();
    return;
  }

  sendFrame(m_framePos);

  constexpr double kInvMs = 1.0 / 1000.0;
  const qint64 elapsedMs  = m_elapsedTimer.elapsed();
  const double targetTime = m_startTimestamp + (elapsedMs * kInvMs);
  const double nextTime   = m_frameIndex[framePosition() + 1].timestamp;

  if (nextTime <= targetTime) {
    catchUpToTarget(targetTime);
    return;
  }

  const qint64 delayMs = qMax(0LL, static_cast<qint64>((nextTime - targetTime) * 1000.0));
  QTimer::singleShot(delayMs, Qt::PreciseTimer, this, [this]() {
    if (isOpen() && isPlaying()) {
      ++m_framePos;
      updateData();
    }
  });
}

//--------------------------------------------------------------------------------------------------
// Frame building
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the channel name ends with the " (raw)" suffix.
 */
static bool hasRawSuffix(const std::string& chName)
{
  static constexpr const char* kRawSuffix = " (raw)";
  return chName.size() >= 6 && chName.compare(chName.size() - 6, 6, kRawSuffix) == 0;
}

/**
 * @brief Per-CG state assembled before observers are attached for a data group.
 */
struct CgInfo {
  mdf::IChannelGroup* cg;
  mdf::IChannel* timeCh;
  std::vector<mdf::IChannel*> dataChs;
};

/**
 * @brief Scans a single channel group, collecting its master and non-raw data channels.
 */
void MDF4::Player::scanChannelGroup(
  mdf::IChannelGroup* cg,
  std::vector<mdf::IChannel*>& allChannels,
  std::map<mdf::IChannelGroup*, mdf::IChannel*>& groupTimeChannels,
  int& masterChannelCount)
{
  mdf::IChannel* groupMaster = nullptr;
  for (auto* ch : cg->Channels()) {
    if (!ch)
      continue;

    if (m_isSerialStudioFile && ch->Type() == mdf::ChannelType::Master) {
      groupMaster = ch;
      ++masterChannelCount;
      continue;
    }

    if (hasRawSuffix(ch->Name())) [[unlikely]]
      continue;

    if (std::find(allChannels.begin(), allChannels.end(), ch) == allChannels.end())
      allChannels.push_back(ch);
  }

  if (groupMaster)
    groupTimeChannels[cg] = groupMaster;
}

/**
 * @brief Walks all data groups and channel groups, collecting all data + master channels.
 */
void MDF4::Player::collectAllChannels(
  const std::vector<mdf::IDataGroup*>& dataGroups,
  std::vector<mdf::IChannel*>& allChannels,
  std::map<mdf::IChannelGroup*, mdf::IChannel*>& groupTimeChannels,
  int& masterChannelCount)
{
  for (auto* dg : dataGroups) {
    if (!dg)
      continue;

    for (auto* cg : dg->ChannelGroups()) {
      if (!cg)
        continue;

      scanChannelGroup(cg, allChannels, groupTimeChannels, masterChannelCount);
    }
  }
}

/**
 * @brief Builds per-CG data-channel + time-channel descriptors for a single data group.
 */
static std::vector<CgInfo> buildCgInfos(
  mdf::IDataGroup* dg,
  bool perGroupTime,
  const std::map<mdf::IChannelGroup*, mdf::IChannel*>& groupTimeChannels)
{
  std::vector<CgInfo> cgInfos;
  for (auto* cg : dg->ChannelGroups()) {
    if (!cg)
      continue;

    auto cgChannels      = cg->Channels();
    uint64_t recordCount = cg->NofSamples();
    if (cgChannels.empty() || recordCount == 0)
      continue;

    CgInfo ci;
    ci.cg     = cg;
    ci.timeCh = nullptr;

    if (perGroupTime) {
      auto tit = groupTimeChannels.find(cg);
      if (tit != groupTimeChannels.end())
        ci.timeCh = tit->second;
    }

    for (auto* ch : cgChannels) {
      if (!ch || ch->Type() == mdf::ChannelType::Master)
        continue;

      if (hasRawSuffix(ch->Name())) [[unlikely]]
        continue;

      ci.dataChs.push_back(ch);
    }

    cgInfos.push_back(std::move(ci));
  }

  return cgInfos;
}

/**
 * @brief Reads sample data from one DG, attaching one SampleCacheObserver per CG.
 */
void MDF4::Player::readDataGroupWithObservers(
  mdf::IDataGroup* dg,
  bool perGroupTime,
  const std::map<mdf::IChannelGroup*, mdf::IChannel*>& groupTimeChannels)
{
  auto channelGroups = dg->ChannelGroups();
  if (channelGroups.empty())
    return;

  auto cgInfos = buildCgInfos(dg, perGroupTime, groupTimeChannels);

  std::vector<std::unique_ptr<SampleCacheObserver>> observers;
  observers.reserve(cgInfos.size());
  for (auto& ci : cgInfos) {
    auto obs = std::make_unique<SampleCacheObserver>(*dg,
                                                     m_sampleCache,
                                                     m_timestampCache,
                                                     m_activeChannels,
                                                     m_channels,
                                                     ci.dataChs,
                                                     ci.timeCh,
                                                     ci.cg->RecordId());
    obs->AttachObserver();
    observers.push_back(std::move(obs));
  }

  m_reader->ReadData(*dg);

  for (auto& obs : observers)
    obs->DetachObserver();
}

/**
 * @brief Reads timestamps from a single legacy master-time channel into the timestamp cache.
 */
void MDF4::Player::readLegacyTimestamps(const std::vector<mdf::IDataGroup*>& dataGroups,
                                        uint64_t legacyTimeRecId)
{
  for (auto* dg : dataGroups) {
    if (!dg)
      continue;

    LegacyTimestampObserver tsObs(*dg, m_timestampCache, m_masterTimeChannel, legacyTimeRecId);
    tsObs.AttachObserver();
    m_reader->ReadData(*dg);
    tsObs.DetachObserver();
    break;
  }
}

/**
 * @brief Builds a sparse frame index for efficient streaming playback.
 */
void MDF4::Player::buildFrameIndex()
{
  m_frameIndex.clear();
  m_channels.clear();
  m_sampleCache.clear();
  m_timestampCache.clear();
  m_activeChannels.clear();
  m_masterTimeChannel = nullptr;

  if (!m_reader || !m_reader->IsOk())
    return;

  auto* file = m_reader->GetFile();
  if (!file)
    return;

  mdf::DataGroupList dataGroups;
  file->DataGroups(dataGroups);

  if (dataGroups.empty())
    return;

  std::vector<mdf::IChannel*> allChannels;
  std::map<mdf::IChannelGroup*, mdf::IChannel*> groupTimeChannels;
  int masterChannelCount = 0;
  collectAllChannels(dataGroups, allChannels, groupTimeChannels, masterChannelCount);

  const bool perGroupTime  = (masterChannelCount > 1);
  uint64_t legacyTimeRecId = 0;
  if (masterChannelCount == 1) {
    auto it             = groupTimeChannels.begin();
    m_masterTimeChannel = it->second;
    legacyTimeRecId     = it->first->RecordId();
    groupTimeChannels.clear();
  }

  m_channels = allChannels;

  for (auto* dg : dataGroups) {
    if (!dg)
      continue;

    readDataGroupWithObservers(dg, perGroupTime, groupTimeChannels);
  }

  if (m_isSerialStudioFile && !perGroupTime && m_masterTimeChannel)
    readLegacyTimestamps(dataGroups, legacyTimeRecId);

  buildFrameIndexFromCache();
}

/**
 * @brief Builds the frame index from the sample cache populated during observer-based data reading.
 */
void MDF4::Player::buildFrameIndexFromCache()
{
  if (m_sampleCache.empty())
    return;

  uint64_t sampleIndex = 0;
  m_frameIndex.reserve(m_sampleCache.size());

  for (const auto& cachePair : m_sampleCache) {
    FrameIndex frameIdx;

    if (m_isSerialStudioFile && !m_timestampCache.empty()) {
      auto timeIt = m_timestampCache.find(cachePair.first);
      if (timeIt != m_timestampCache.end())
        frameIdx.timestamp = timeIt->second;
      else
        frameIdx.timestamp = static_cast<double>(sampleIndex) * 0.001;
    } else {
      frameIdx.timestamp = static_cast<double>(sampleIndex) * 0.001;
    }

    frameIdx.recordIndex  = cachePair.first;
    frameIdx.channelGroup = nullptr;

    m_frameIndex.push_back(frameIdx);
    ++sampleIndex;
  }

  std::sort(m_frameIndex.begin(), m_frameIndex.end(), [](const FrameIndex& a, const FrameIndex& b) {
    return a.recordIndex < b.recordIndex;
  });
}

//--------------------------------------------------------------------------------------------------
// Frame transmission helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Processes a batch of frames for plot history
 */
void MDF4::Player::processFrameBatch(int startFrame, int endFrame)
{
  for (int i = startFrame; i <= endFrame; ++i)
    if (i >= 0 && i < frameCount())
      sendFrame(i);
}

/**
 * @brief Sends a single frame to the IO manager for processing
 */
void MDF4::Player::sendFrame(int frameIndex)
{
  if (!isOpen() || frameIndex < 0 || frameIndex >= frameCount())
    return;

  injectFrame(getFrame(frameIndex), frameIndex);
}

/**
 * @brief Registers MDF4 channel names with Quick Plot
 */
void MDF4::Player::sendHeaderFrame()
{
  if (!isOpen() || m_channels.empty())
    return;

  if (AppState::instance().operationMode() == SerialStudio::ProjectFile) {
    buildReplayLayout();
    if (m_multiSource)
      return;
  }

  QStringList headers;
  for (size_t i = 0; i < m_channels.size(); ++i) {
    auto* channel = m_channels[i];
    if (channel) {
      QString name = QString::fromStdString(channel->Name());
      if (name.isEmpty())
        name = QString("Channel_%1").arg(i + 1);

      headers.append(name);
    }

    else
      headers.append(QString("Channel_%1").arg(i + 1));
  }

  DataModel::FrameBuilder::instance().registerQuickPlotHeaders(headers);
}

//--------------------------------------------------------------------------------------------------
// Date/time operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Formats a timestamp value as HH:MM:SS.mmm
 */
QString MDF4::Player::formatTimestamp(double timestamp) const
{
  constexpr double kInvHour = 1.0 / 3600.0;
  constexpr double kInvMin  = 1.0 / 60.0;
  int hours                 = static_cast<int>(timestamp * kInvHour);
  int minutes               = static_cast<int>((timestamp - hours * 3600.0) * kInvMin);
  double seconds            = timestamp - hours * 3600.0 - minutes * 60.0;

  return QString("%1:%2:%3")
    .arg(qMax(hours, 0), 2, 10, QChar('0'))
    .arg(qMax(minutes, 0), 2, 10, QChar('0'))
    .arg(qMax(seconds, 0.0), 6, 'f', 3, QChar('0'));
}

/**
 * @brief Extracts a frame of data at the specified index
 */
QByteArray MDF4::Player::getFrame(const int index)
{
  if (!isOpen() || index < 0 || index >= frameCount())
    return QByteArray();

  const auto& frameIdx = m_frameIndex[index];
  QByteArray frame;

  auto it = m_sampleCache.find(frameIdx.recordIndex);
  if (it != m_sampleCache.end()) {
    const auto& values = it->second;
    for (size_t i = 0; i < values.size(); ++i) {
      frame.append(QString::number(values[i], 'g', 10).toUtf8());

      if (i < values.size() - 1)
        frame.append(',');
    }
  }

  else {
    for (size_t i = 0; i < m_channels.size(); ++i) {
      frame.append("0");
      if (i < m_channels.size() - 1)
        frame.append(',');
    }
  }

  frame.append('\n');
  return frame;
}

//--------------------------------------------------------------------------------------------------
// Multi-source playback helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a channel-to-source mapping for multi-source MDF4 playback.
 */
void MDF4::Player::buildReplayLayout()
{
  m_sourceChannelsByIndex.clear();

  struct ChMeta {
    int uid;
    int sourceId;
  };

  QVector<ChMeta> chs;
  for (const auto& g : DataModel::ProjectModel::instance().groups()) {
    if (g.widget == QLatin1String("image"))
      continue;

    for (const auto& d : g.datasets)
      chs.append({d.uniqueId, g.sourceId});
  }

  std::unordered_map<int, int> localCounter;
  for (const auto& m : chs)
    localCounter[m.sourceId];

  m_multiSource = localCounter.size() > 1;

  std::unordered_map<int, std::unordered_map<int, int>> replay;

  if (!m_multiSource) {
    for (int ch = 0; ch < chs.size(); ++ch)
      replay[0][chs[ch].uid] = ch;
  }

  else {
    for (auto& kv : localCounter)
      kv.second = 0;

    for (int ch = 0; ch < chs.size(); ++ch) {
      m_sourceChannelsByIndex[chs[ch].sourceId].append(ch);
      replay[chs[ch].sourceId][chs[ch].uid] = localCounter[chs[ch].sourceId]++;
    }
  }

  DataModel::FrameBuilder::instance().setReplayColumnMap(std::move(replay));
}

/**
 * @brief Injects an MDF4 frame through the appropriate pipeline path.
 */
void MDF4::Player::injectFrame(const QByteArray& frame, int frameIndex)
{
  if (frame.isEmpty())
    return;

  if (!m_multiSource) {
    IO::ConnectionManager::instance().processPayload(frame);
    return;
  }

  const std::vector<bool>* active = nullptr;
  if (frameIndex >= 0 && frameIndex < static_cast<int>(m_frameIndex.size())) {
    auto ait = m_activeChannels.find(m_frameIndex[frameIndex].recordIndex);
    if (ait != m_activeChannels.end())
      active = &ait->second;
  }

  const auto fields = QString::fromUtf8(frame).trimmed().split(',');

  QMap<int, QStringList> sourceFields;
  QMap<int, bool> sourceHasActive;
  for (auto it = m_sourceChannelsByIndex.constBegin(); it != m_sourceChannelsByIndex.constEnd();
       ++it) {
    const int srcId        = it.key();
    const auto& orderedChs = it.value();
    QStringList orderedCells;
    orderedCells.reserve(orderedChs.size());
    bool anyActive = !active;
    for (int ch : orderedChs) {
      const QString cell = (ch >= 0 && ch < fields.size()) ? fields[ch] : QString();
      orderedCells.append(cell);
      if (active && ch >= 0 && ch < static_cast<int>(active->size()) && (*active)[ch])
        anyActive = true;
    }
    sourceFields.insert(srcId, std::move(orderedCells));
    sourceHasActive.insert(srcId, anyActive);
  }

  QMap<int, QByteArray> sourcePayloads;
  for (auto it = sourceFields.constBegin(); it != sourceFields.constEnd(); ++it) {
    if (!sourceHasActive.value(it.key(), true))
      continue;

    sourcePayloads[it.key()] = it.value().join(',').toUtf8() + '\n';
  }

  if (!sourcePayloads.isEmpty())
    IO::ConnectionManager::instance().processMultiSourcePayload(frame, sourcePayloads);
}

//--------------------------------------------------------------------------------------------------
// Event handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles keyboard shortcuts for playback control
 */
bool MDF4::Player::handleKeyPress(QKeyEvent* keyEvent)
{
  if (!isOpen())
    return false;

  if (keyEvent->key() == Qt::Key_Space) {
    toggle();
    return true;
  }

  else if (keyEvent->key() == Qt::Key_Left) {
    previousFrame();
    return true;
  }

  else if (keyEvent->key() == Qt::Key_Right) {
    nextFrame();
    return true;
  }

  return false;
}

/**
 * @brief Event filter for capturing keyboard events
 */
bool MDF4::Player::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::KeyPress) {
    auto* keyEvent = static_cast<QKeyEvent*>(event);
    return handleKeyPress(keyEvent);
  }

  return QObject::eventFilter(obj, event);
}
