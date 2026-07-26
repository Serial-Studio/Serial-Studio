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

#include "MDF4/PlayerLoaderWorker.h"

#include <algorithm>
#include <map>
#include <mdf/ichannel.h>
#include <mdf/ichannelgroup.h>
#include <mdf/idatagroup.h>
#include <mdf/isampleobserver.h>
#include <mdf/mdffile.h>
#include <mdf/mdfreader.h>

#include "SSAssert.h"

static constexpr quint64 kProgressTickRecords = 262144;

//--------------------------------------------------------------------------------------------------
// mdflib helpers (worker-confined; no mdf::* type leaves this file)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the channel stores text samples (string-typed MDF4 channel).
 */
static bool isStringChannel(const mdf::IChannel* channel)
{
  if (!channel)
    return false;

  const auto type = channel->DataType();
  return type == mdf::ChannelDataType::StringAscii || type == mdf::ChannelDataType::StringUTF8
      || type == mdf::ChannelDataType::StringUTF16Le || type == mdf::ChannelDataType::StringUTF16Be;
}

/**
 * @brief Returns true when the channel name ends with the " (raw)" suffix.
 */
static bool hasRawSuffix(const std::string& chName)
{
  static constexpr const char* kRawSuffix = " (raw)";
  return chName.size() >= 6 && chName.compare(chName.size() - 6, 6, kRawSuffix) == 0;
}

/**
 * @brief Scans a single channel group, collecting its master and non-raw data channels.
 */
static void scanChannelGroup(mdf::IChannelGroup* cg,
                             bool isSerialStudioFile,
                             std::vector<mdf::IChannel*>& allChannels,
                             std::map<mdf::IChannelGroup*, mdf::IChannel*>& groupTimeChannels,
                             int& masterChannelCount)
{
  mdf::IChannel* groupMaster = nullptr;
  for (auto* ch : cg->Channels()) {
    if (!ch)
      continue;

    if (isSerialStudioFile && ch->Type() == mdf::ChannelType::Master) {
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
static void collectAllChannels(const std::vector<mdf::IDataGroup*>& dataGroups,
                               bool isSerialStudioFile,
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

      scanChannelGroup(cg, isSerialStudioFile, allChannels, groupTimeChannels, masterChannelCount);
    }
  }
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
 * @brief Observer that caches channel values during MDF4 data reading; the ns-quantized
 *        cache key is the multi-channel-group frame-merge contract (bit-identical to the
 *        legacy in-player observer). Returns false from OnSample on cancel to abort ReadData.
 */
class SampleCacheObserver : public mdf::ISampleObserver {
public:
  /**
   * @brief Constructs the observer and indexes per-group channels for fast lookup.
   */
  SampleCacheObserver(const mdf::IDataGroup& dataGroup,
                      MDF4::PlayerLoaderWorker* worker,
                      std::map<uint64_t, std::vector<double>>& cache,
                      std::map<uint64_t, std::vector<QString>>& stringCache,
                      std::map<uint64_t, double>& timestampCache,
                      std::map<uint64_t, std::vector<bool>>& activeChannels,
                      const std::vector<mdf::IChannel*>& allChannels,
                      const std::vector<mdf::IChannel*>& groupChannels,
                      mdf::IChannel* groupTimeChannel,
                      uint64_t recordId)
    : mdf::ISampleObserver(dataGroup)
    , m_worker(worker)
    , m_cache(cache)
    , m_stringCache(stringCache)
    , m_timestampCache(timestampCache)
    , m_activeChannels(activeChannels)
    , m_allChannels(allChannels)
    , m_groupChannels(groupChannels)
    , m_groupTimeChannel(groupTimeChannel)
    , m_recordId(recordId)
    , m_hasStringChannels(false)
  {
    for (size_t i = 0; i < m_allChannels.size(); ++i) {
      for (auto* ch : m_groupChannels) {
        if (m_allChannels[i] == ch) {
          m_channelIndexMap[ch] = i;
          break;
        }
      }
    }

    for (auto* ch : m_groupChannels)
      m_hasStringChannels = m_hasStringChannels || isStringChannel(ch);
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

    std::vector<QString>* strings = nullptr;
    if (m_hasStringChannels) {
      auto strIt = m_stringCache.find(cacheKey);
      if (strIt == m_stringCache.end())
        strIt = m_stringCache.emplace(cacheKey, std::vector<QString>(m_allChannels.size())).first;

      strings = &strIt->second;
    }

    for (auto* channel : m_groupChannels) {
      if (!channel)
        continue;

      auto it = m_channelIndexMap.find(channel);
      if (it == m_channelIndexMap.end())
        continue;

      if (strings && isStringChannel(channel)) {
        std::string text;
        const bool success = GetEngValue(*channel, record_id, record, text);
        if (!success)
          (void)GetChannelValue(*channel, record_id, record, text);

        (*strings)[it->second] = QString::fromStdString(text);
        active[it->second]     = true;
        continue;
      }

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

    return m_worker->recordTick();
  }

private:
  MDF4::PlayerLoaderWorker* m_worker;
  std::map<uint64_t, std::vector<double>>& m_cache;
  std::map<uint64_t, std::vector<QString>>& m_stringCache;
  std::map<uint64_t, double>& m_timestampCache;
  std::map<uint64_t, std::vector<bool>>& m_activeChannels;
  const std::vector<mdf::IChannel*>& m_allChannels;
  const std::vector<mdf::IChannel*>& m_groupChannels;
  mdf::IChannel* m_groupTimeChannel;
  uint64_t m_recordId;
  std::map<mdf::IChannel*, size_t> m_channelIndexMap;
  bool m_hasStringChannels;
};

/**
 * @brief Reads timestamp values from a single legacy master time channel.
 */
class LegacyTimestampObserver : public mdf::ISampleObserver {
public:
  /**
   * @brief Constructs the observer bound to a master time channel and record ID.
   */
  LegacyTimestampObserver(const mdf::IDataGroup& dataGroup,
                          MDF4::PlayerLoaderWorker* worker,
                          std::map<uint64_t, double>& timestampCache,
                          mdf::IChannel* masterTimeChannel,
                          uint64_t recordId)
    : mdf::ISampleObserver(dataGroup)
    , m_worker(worker)
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
    return m_worker->recordTick();
  }

private:
  MDF4::PlayerLoaderWorker* m_worker;
  std::map<uint64_t, double>& m_timestampCache;
  mdf::IChannel* m_masterTimeChannel;
  uint64_t m_recordId;
};

//--------------------------------------------------------------------------------------------------
// Worker
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the loader worker (thread affinity is assigned by the player).
 */
MDF4::PlayerLoaderWorker::PlayerLoaderWorker(QObject* parent)
  : QObject(parent)
  , m_cancelRequested(false)
  , m_recordsSeen(0)
  , m_recordsTotal(0)
  , m_activeGeneration(0)
{}

/**
 * @brief Requests cooperative cancellation; the observers abort ReadData on the next record.
 */
void MDF4::PlayerLoaderWorker::requestCancel()
{
  m_cancelRequested.store(true, std::memory_order_relaxed);
}

/**
 * @brief Returns whether cancellation was requested (worker-thread helpers poll this).
 */
bool MDF4::PlayerLoaderWorker::cancelRequested() const
{
  return m_cancelRequested.load(std::memory_order_relaxed);
}

/**
 * @brief Per-record decode tick: counts progress, emits a throttled fraction update, and
 *        returns false once cancellation was requested (aborting the mdflib read).
 */
bool MDF4::PlayerLoaderWorker::recordTick()
{
  ++m_recordsSeen;
  if (m_recordsSeen % kProgressTickRecords == 0 && m_recordsTotal > 0) {
    const double fraction =
      std::min(1.0, static_cast<double>(m_recordsSeen) / static_cast<double>(m_recordsTotal));
    Q_EMIT progressUpdate(fraction, m_activeGeneration);
  }

  return !m_cancelRequested.load(std::memory_order_relaxed);
}

/**
 * @brief Keyed accumulation maps shared by the observers (the ns-key merge state).
 */
struct DecodeCaches {
  std::map<uint64_t, std::vector<double>> samples;
  std::map<uint64_t, std::vector<QString>> strings;
  std::map<uint64_t, double> timestamps;
  std::map<uint64_t, std::vector<bool>> active;
};

/**
 * @brief Per-DG time-resolution state derived from the master-channel census.
 */
struct TimeConfig {
  bool perGroupTime                      = false;
  mdf::IChannel* legacyMasterTimeChannel = nullptr;
  uint64_t legacyTimeRecId               = 0;
  std::map<mdf::IChannelGroup*, mdf::IChannel*> groupTimeChannels;
};

/**
 * @brief Converts one keyed frame's values into the columnar payload at @p sampleIndex.
 */
static void appendFrameColumns(MDF4::PlayerDecodePayload& payload,
                               const DecodeCaches& caches,
                               uint64_t key,
                               const std::vector<double>& values,
                               uint64_t sampleIndex)
{
  const auto strIt = caches.strings.find(key);
  const auto actIt = caches.active.find(key);

  const size_t channelCount = payload.channelIsString.size();
  for (size_t c = 0; c < channelCount && c < values.size(); ++c) {
    if (!payload.channelIsString[c])
      payload.numeric[c][sampleIndex] = values[c];
    else if (strIt != caches.strings.end() && c < strIt->second.size())
      payload.text[c][sampleIndex] = strIt->second[c];

    if (actIt != caches.active.end() && c < actIt->second.size())
      payload.active[c][sampleIndex] = actIt->second[c];
  }
}

/**
 * @brief Finalizes the keyed caches into the payload's columnar vectors, consuming the maps
 *        frame by frame so the keyed and columnar copies never coexist in full (bounds the
 *        load-time memory peak). Iteration order is key-ascending, matching the legacy
 *        recordIndex sort exactly.
 */
static void convertToColumnar(MDF4::PlayerDecodePayload& payload, DecodeCaches& caches)
{
  const size_t channelCount = payload.channelIsString.size();
  const size_t frameCount   = caches.samples.size();
  const bool useTimestamps  = payload.isSerialStudioFile && !caches.timestamps.empty();

  payload.timestamps.reserve(frameCount);
  payload.numeric.resize(channelCount);
  payload.text.resize(channelCount);
  payload.active.resize(channelCount);
  for (size_t c = 0; c < channelCount; ++c) {
    if (payload.channelIsString[c])
      payload.text[c].resize(frameCount);
    else
      payload.numeric[c].resize(frameCount, 0.0);

    payload.active[c].resize(frameCount, false);
  }

  for (uint64_t sampleIndex = 0; sampleIndex < frameCount && !caches.samples.empty();
       ++sampleIndex) {
    const auto it      = caches.samples.begin();
    const uint64_t key = it->first;

    double ts = static_cast<double>(sampleIndex) * 0.001;
    if (useTimestamps) {
      const auto timeIt = caches.timestamps.find(key);
      if (timeIt != caches.timestamps.end())
        ts = timeIt->second;
    }

    payload.timestamps.push_back(ts);
    appendFrameColumns(payload, caches, key, it->second, sampleIndex);

    caches.samples.erase(it);
    caches.strings.erase(key);
    caches.active.erase(key);
    caches.timestamps.erase(key);
  }
}

/**
 * @brief Opens the reader and reads the file structure; fills the payload's error strings and
 *        returns nullptr on failure.
 */
static std::unique_ptr<mdf::MdfReader> openStructure(const QString& filePath,
                                                     MDF4::PlayerDecodePayload& payload)
{
  auto reader = std::make_unique<mdf::MdfReader>(filePath.toStdString());
  if (!reader->IsOk()) {
    payload.errorTitle = QObject::tr("Cannot open MDF4 file");
    payload.errorBody  = QObject::tr("The file may be corrupted or in an unsupported format.");
    return nullptr;
  }

  if (!reader->ReadEverythingButData()) {
    payload.errorTitle = QObject::tr("Invalid MDF4 file");
    payload.errorBody  = QObject::tr("Failed to read file structure. The file may be corrupted.");
    return nullptr;
  }

  if (auto* header = reader->GetHeader())
    payload.isSerialStudioFile = QString::fromStdString(header->Author()) == "Serial Studio";

  return reader;
}

/**
 * @brief Runs the observer decode over every data group into the keyed caches; returns false
 *        when any ReadData call reported failure (truncated/corrupt data section).
 */
static bool readAllGroups(MDF4::PlayerLoaderWorker* worker,
                          mdf::MdfReader& reader,
                          const std::vector<mdf::IDataGroup*>& dataGroups,
                          const std::vector<mdf::IChannel*>& allChannels,
                          const TimeConfig& timeConfig,
                          DecodeCaches& caches)
{
  bool read_ok = true;
  for (auto* dg : dataGroups) {
    if (!dg || worker->cancelRequested())
      continue;

    auto cgInfos = buildCgInfos(dg, timeConfig.perGroupTime, timeConfig.groupTimeChannels);
    std::vector<std::unique_ptr<SampleCacheObserver>> observers;
    observers.reserve(cgInfos.size());
    for (auto& ci : cgInfos) {
      auto obs = std::make_unique<SampleCacheObserver>(*dg,
                                                       worker,
                                                       caches.samples,
                                                       caches.strings,
                                                       caches.timestamps,
                                                       caches.active,
                                                       allChannels,
                                                       ci.dataChs,
                                                       ci.timeCh,
                                                       ci.cg->RecordId());
      obs->AttachObserver();
      observers.push_back(std::move(obs));
    }

    read_ok = reader.ReadData(*dg) && read_ok;

    for (auto& obs : observers)
      obs->DetachObserver();
  }

  const bool wantLegacyTs =
    !timeConfig.perGroupTime && timeConfig.legacyMasterTimeChannel && !worker->cancelRequested();
  if (wantLegacyTs) {
    for (auto* dg : dataGroups) {
      if (!dg)
        continue;

      LegacyTimestampObserver tsObs(*dg,
                                    worker,
                                    caches.timestamps,
                                    timeConfig.legacyMasterTimeChannel,
                                    timeConfig.legacyTimeRecId);
      tsObs.AttachObserver();
      read_ok = reader.ReadData(*dg) && read_ok;
      tsObs.DetachObserver();
      break;
    }
  }

  return read_ok;
}

/**
 * @brief Decodes the whole file into the columnar payload: structure read, observer decode
 *        with the legacy ns-key merge, then a map-to-columnar conversion. The reader and all
 *        mdf::* pointers live and die inside this slot.
 */
void MDF4::PlayerLoaderWorker::decodeFile(const QString& filePath, quint64 generation)
{
  SS_ASSERT_LOG(!filePath.isEmpty());

  m_recordsSeen      = 0;
  m_recordsTotal     = 0;
  m_activeGeneration = generation;

  auto payload        = std::make_shared<PlayerDecodePayload>();
  payload->filePath   = filePath;
  payload->generation = generation;

  auto reader = openStructure(filePath, *payload);
  if (!reader) {
    Q_EMIT finished(payload);
    return;
  }

  auto* file = reader->GetFile();
  mdf::DataGroupList dataGroups;
  if (file)
    file->DataGroups(dataGroups);

  std::vector<mdf::IChannel*> allChannels;
  TimeConfig timeConfig;
  int masterChannelCount = 0;
  collectAllChannels(dataGroups,
                     payload->isSerialStudioFile,
                     allChannels,
                     timeConfig.groupTimeChannels,
                     masterChannelCount);

  timeConfig.perGroupTime = (masterChannelCount > 1);
  if (masterChannelCount == 1) {
    auto it                            = timeConfig.groupTimeChannels.begin();
    timeConfig.legacyMasterTimeChannel = payload->isSerialStudioFile ? it->second : nullptr;
    timeConfig.legacyTimeRecId         = it->first->RecordId();
    timeConfig.groupTimeChannels.clear();
  }

  for (auto* dg : dataGroups) {
    if (!dg)
      continue;

    for (auto* cg : dg->ChannelGroups())
      if (cg)
        m_recordsTotal += cg->NofSamples();
  }

  payload->channelNames.reserve(static_cast<qsizetype>(allChannels.size()));
  payload->channelIsString.reserve(allChannels.size());
  for (const auto* ch : allChannels) {
    payload->channelNames.append(ch ? QString::fromStdString(ch->Name()) : QString());
    payload->channelIsString.push_back(isStringChannel(ch));
  }

  DecodeCaches caches;
  const bool read_ok = readAllGroups(this, *reader, dataGroups, allChannels, timeConfig, caches);

  if (m_cancelRequested.load(std::memory_order_relaxed)) {
    payload->cancelled = true;
    Q_EMIT finished(payload);
    return;
  }

  convertToColumnar(*payload, caches);
  payload->ok          = true;
  payload->partialData = !read_ok;
  Q_EMIT finished(payload);
}
