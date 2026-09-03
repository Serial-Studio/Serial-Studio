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
#include <deque>
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
 * @brief Returns true when @p channel is this file's time master. Serial Studio wrote masters with
 *        no sync type until 4.1.0, which ASAM MDF 4.1 does not allow, so both the conforming Time
 *        sync and that legacy zero are accepted and every 4.1.0 archive keeps replaying (B10). A
 *        master synced to angle, distance or sample index is not a clock and is left as data.
 */
[[nodiscard]] static bool isTimeMaster(const mdf::IChannel* channel)
{
  if (!channel || channel->Type() != mdf::ChannelType::Master)
    return false;

  const auto sync = channel->Sync();
  return sync == mdf::ChannelSyncType::Time || sync == mdf::ChannelSyncType::None;
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

    if (isSerialStudioFile && isTimeMaster(ch)) {
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
 * @brief One channel group's decode, stored the way it is read: its own instants plus one
 *        contiguous vector per channel it carries. This is what bounds the loader's memory to
 *        samples x channels-IN-THE-GROUP. The previous keyed cache stored a vector over ALL
 *        channels per distinct instant, which for a ten-minute 48 kHz stream recording is ~29 M
 *        instants and several gigabytes before the columnar copy was even allocated (B6).
 */
struct GroupColumns {
  bool hasClock = false;                     ///< Group carried a time master
  std::vector<uint64_t> keys;                ///< Merge key per row (ns instant, or sample index)
  std::vector<double> timestamps;            ///< Seconds per row; empty unless the group is clocked
  std::vector<uint32_t> order;               ///< Row order; empty when keys are already ascending
  std::vector<std::size_t> channelIndex;     ///< Global channel index per local column
  std::vector<bool> isString;                ///< Per local column
  std::vector<std::vector<double>> numeric;  ///< Per local column (empty for string columns)
  std::vector<std::vector<QString>> text;    ///< Per local column (empty for numeric columns)
};

/**
 * @brief Appends one row per sample into a channel group's columns. Row-aligned by construction:
 *        every local column is pushed exactly once per accepted record.
 */
class GroupColumnObserver : public mdf::ISampleObserver {
public:
  /**
   * @brief Constructs the observer bound to one channel group's data channels and record id.
   */
  GroupColumnObserver(const mdf::IDataGroup& dataGroup,
                      MDF4::PlayerLoaderWorker* worker,
                      GroupColumns& out,
                      const std::vector<mdf::IChannel*>& groupChannels,
                      mdf::IChannel* groupTimeChannel,
                      uint64_t recordId)
    : mdf::ISampleObserver(dataGroup)
    , m_worker(worker)
    , m_out(out)
    , m_groupChannels(groupChannels)
    , m_groupTimeChannel(groupTimeChannel)
    , m_recordId(recordId)
  {
    SS_ASSERT_LOG(worker != nullptr);
    SS_ASSERT_LOG(m_out.numeric.size() == m_groupChannels.size());
  }

  /**
   * @brief Appends this record's instant and one value per channel of the group.
   */
  bool OnSample(uint64_t sample, uint64_t record_id, const std::vector<uint8_t>& record) override
  {
    if (record_id != m_recordId)
      return true;

    uint64_t key = sample;
    if (m_groupTimeChannel) {
      double ts = 0.0;
      if (!GetEngValue(*m_groupTimeChannel, record_id, record, ts))
        (void)GetChannelValue(*m_groupTimeChannel, record_id, record, ts);

      key = static_cast<uint64_t>(ts * 1'000'000'000.0);
      m_out.timestamps.push_back(ts);
    }

    m_out.keys.push_back(key);

    for (std::size_t c = 0; c < m_groupChannels.size(); ++c)
      appendChannel(c, record_id, record);

    return m_worker->recordTick();
  }

private:
  /**
   * @brief Appends local column @p c's value for this record, always exactly one entry so the
   *        group's columns stay row-aligned even for a channel the file cannot decode.
   */
  void appendChannel(std::size_t c, uint64_t record_id, const std::vector<uint8_t>& record)
  {
    auto* channel = m_groupChannels[c];

    if (m_out.isString[c]) {
      std::string text;
      if (channel) {
        if (!GetEngValue(*channel, record_id, record, text))
          (void)GetChannelValue(*channel, record_id, record, text);
      }

      m_out.text[c].push_back(QString::fromStdString(text));
      return;
    }

    double value = 0.0;
    if (channel) {
      if (!GetEngValue(*channel, record_id, record, value))
        if (!GetChannelValue(*channel, record_id, record, value))
          value = 0.0;
    }

    m_out.numeric[c].push_back(value);
  }

private:
  MDF4::PlayerLoaderWorker* m_worker;
  GroupColumns& m_out;
  const std::vector<mdf::IChannel*>& m_groupChannels;
  mdf::IChannel* m_groupTimeChannel;
  uint64_t m_recordId;
};

/**
 * @brief Reads timestamp values from a single legacy master time channel into a per-sample vector.
 */
class LegacyTimestampObserver : public mdf::ISampleObserver {
public:
  /**
   * @brief Constructs the observer bound to a master time channel and record ID.
   */
  LegacyTimestampObserver(const mdf::IDataGroup& dataGroup,
                          MDF4::PlayerLoaderWorker* worker,
                          std::vector<double>& timestamps,
                          mdf::IChannel* masterTimeChannel,
                          uint64_t recordId)
    : mdf::ISampleObserver(dataGroup)
    , m_worker(worker)
    , m_timestamps(timestamps)
    , m_masterTimeChannel(masterTimeChannel)
    , m_recordId(recordId)
  {}

  /**
   * @brief Records the master-channel timestamp at the sample index it belongs to.
   */
  bool OnSample(uint64_t sample, uint64_t record_id, const std::vector<uint8_t>& record) override
  {
    if (record_id != m_recordId || !m_masterTimeChannel)
      return true;

    double timestamp = 0.0;
    if (!GetEngValue(*m_masterTimeChannel, record_id, record, timestamp))
      if (!GetChannelValue(*m_masterTimeChannel, record_id, record, timestamp))
        timestamp = 0.0;

    if (sample >= m_timestamps.size())
      m_timestamps.resize(static_cast<std::size_t>(sample) + 1, 0.0);

    m_timestamps[static_cast<std::size_t>(sample)] = timestamp;
    return m_worker->recordTick();
  }

private:
  MDF4::PlayerLoaderWorker* m_worker;
  std::vector<double>& m_timestamps;
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
 * @brief Decode state for one file: one column set per channel group, plus the single-master
 *        timestamp vector the legacy (pre per-group master) layout needs.
 */
struct DecodeState {
  // code-verify off
  // A deque, not a vector: each observer holds a reference to its group's columns for the whole
  // read, and a vector's growth would move them under the observers still appending rows.
  // code-verify on
  std::deque<GroupColumns> groups;
  std::vector<double> legacyTimestamps;
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
 * @brief Builds the ascending row order for a group whose keys are not already non-decreasing.
 *        Returns an empty vector for the ordinary case, so the merge indexes rows directly and a
 *        conforming recording never pays for the permutation.
 */
[[nodiscard]] static std::vector<uint32_t> ascendingOrder(const std::vector<uint64_t>& keys)
{
  if (std::is_sorted(keys.begin(), keys.end()))
    return {};

  std::vector<uint32_t> order(keys.size());
  for (std::size_t i = 0; i < order.size(); ++i)
    order[i] = static_cast<uint32_t>(i);

  std::stable_sort(order.begin(), order.end(), [&keys](uint32_t lhs, uint32_t rhs) {
    return keys[lhs] < keys[rhs];
  });

  return order;
}

/**
 * @brief Row index of @p cursor inside @p group, honouring a permutation when one was needed.
 */
[[nodiscard]] static std::size_t groupRow(const GroupColumns& group, std::size_t cursor)
{
  return group.order.empty() ? cursor : static_cast<std::size_t>(group.order[cursor]);
}

/**
 * @brief Writes one group's row into the merged payload row @p row: its values, its activity bits
 *        and its contribution to the row's source mask. Channels of other groups keep the zero and
 *        inactive state the payload was sized with, exactly as the keyed merge left them.
 */
static void writeGroupRow(MDF4::PlayerDecodePayload& payload,
                          const GroupColumns& group,
                          std::size_t sourceRow,
                          std::size_t row)
{
  const std::size_t channelCount = payload.channelIsString.size();
  for (std::size_t lc = 0; lc < group.channelIndex.size(); ++lc) {
    const std::size_t gc = group.channelIndex[lc];
    if (gc >= channelCount) [[unlikely]]
      continue;

    if (group.isString[lc]) {
      if (sourceRow < group.text[lc].size())
        payload.text[gc][row] = group.text[lc][sourceRow];
    } else if (sourceRow < group.numeric[lc].size()) {
      payload.numeric[gc][row] = group.numeric[lc][sourceRow];
    }

    payload.active[gc][row] = true;
    if (static_cast<qsizetype>(gc) < payload.channelSourceBit.size())
      payload.rowSourceBits[static_cast<qsizetype>(row)] |=
        payload.channelSourceBit.at(static_cast<qsizetype>(gc));
  }
}

/**
 * @brief Sizes the payload's columns for the merge's upper bound (every group's rows distinct).
 */
static void sizePayloadColumns(MDF4::PlayerDecodePayload& payload, std::size_t rows)
{
  const std::size_t channelCount = payload.channelIsString.size();
  payload.timestamps.reserve(rows);
  payload.numeric.resize(channelCount);
  payload.text.resize(channelCount);
  payload.active.resize(channelCount);

  for (std::size_t c = 0; c < channelCount; ++c) {
    if (payload.channelIsString[c])
      payload.text[c].resize(rows);
    else
      payload.numeric[c].resize(rows, 0.0);

    payload.active[c].resize(rows, false);
  }

  payload.rowSourceBits.resize(static_cast<qsizetype>(rows), 0);
}

/**
 * @brief Trims the payload's columns to the rows the merge actually produced.
 */
static void trimPayloadColumns(MDF4::PlayerDecodePayload& payload, std::size_t rows)
{
  for (auto& column : payload.numeric)
    column.resize(rows);

  for (auto& column : payload.text)
    column.resize(rows);

  for (auto& column : payload.active)
    column.resize(rows);

  payload.rowSourceBits.resize(static_cast<qsizetype>(rows));
}

/**
 * @brief One merged output row in flight: which payload row it is, the instant it merges on, and
 *        the seconds the row will carry once a clocked group has contributed.
 */
struct MergeRow {
  std::size_t row    = 0;
  uint64_t key       = 0;
  bool useTimestamps = false;
  bool stamped       = false;
  double seconds     = 0.0;
};

/**
 * @brief Smallest key still pending across the groups; false when every group is drained.
 */
[[nodiscard]] static bool leastPendingKey(const std::deque<GroupColumns>& groups,
                                          const std::vector<std::size_t>& cursors,
                                          uint64_t& least)
{
  bool found = false;
  for (std::size_t g = 0; g < groups.size(); ++g) {
    if (cursors[g] >= groups[g].keys.size())
      continue;

    const uint64_t key = groups[g].keys[groupRow(groups[g], cursors[g])];
    if (!found || key < least) {
      least = key;
      found = true;
    }
  }

  return found;
}

/**
 * @brief Writes every row of @p group sitting on @p out's key into the merged row and returns the
 *        group's new cursor. Samples of ONE group that quantize to the same instant collapse into
 *        that row, last value winning, exactly as the keyed cache did: coalescing is only correct
 *        within a source, never across two.
 */
[[nodiscard]] static std::size_t consumeGroupRows(MDF4::PlayerDecodePayload& payload,
                                                  const GroupColumns& group,
                                                  std::size_t cursor,
                                                  MergeRow& out)
{
  for (std::size_t k = cursor; k < group.keys.size(); ++k) {
    const std::size_t sourceRow = groupRow(group, k);
    if (group.keys[sourceRow] != out.key)
      break;

    if (out.useTimestamps && group.hasClock && sourceRow < group.timestamps.size()) {
      out.seconds = group.timestamps[sourceRow];
      out.stamped = true;
    }

    writeGroupRow(payload, group, sourceRow, out.row);
    cursor = k + 1;
  }

  return cursor;
}

/**
 * @brief Merges every group's columns into the payload by ascending key, one payload row per
 *        distinct instant -- the same rows, in the same order, the keyed cache produced, without
 *        ever materialising a per-instant vector over all channels (B6).
 */
static void mergeGroups(MDF4::PlayerDecodePayload& payload, DecodeState& state)
{
  bool anyClock          = false;
  std::size_t upperBound = 0;
  for (auto& group : state.groups) {
    group.order  = ascendingOrder(group.keys);
    upperBound  += group.keys.size();
    anyClock     = anyClock || group.hasClock;
  }

  MergeRow merge;
  merge.useTimestamps = payload.isSerialStudioFile && (anyClock || !state.legacyTimestamps.empty());

  sizePayloadColumns(payload, upperBound);

  std::vector<std::size_t> cursors(state.groups.size(), 0);
  std::size_t row = 0;
  for (; row < upperBound; ++row) {
    if (!leastPendingKey(state.groups, cursors, merge.key))
      break;

    merge.row     = row;
    merge.stamped = false;
    merge.seconds = static_cast<double>(row) * 0.001;

    for (std::size_t g = 0; g < state.groups.size(); ++g)
      cursors[g] = consumeGroupRows(payload, state.groups[g], cursors[g], merge);

    if (merge.useTimestamps && !merge.stamped && merge.key < state.legacyTimestamps.size())
      merge.seconds = state.legacyTimestamps[static_cast<std::size_t>(merge.key)];

    payload.timestamps.push_back(merge.seconds);
  }

  trimPayloadColumns(payload, row);
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
 * @brief Prepares one channel group's column set: its local channel list resolved to global
 *        indexes once, and one empty vector per local column for the observer to append into.
 */
[[nodiscard]] static GroupColumns prepareGroupColumns(
  const CgInfo& info,
  const std::map<mdf::IChannel*, std::size_t>& channelIndex,
  const std::vector<bool>& channelIsString)
{
  GroupColumns columns;
  columns.hasClock = info.timeCh != nullptr;
  columns.channelIndex.reserve(info.dataChs.size());
  columns.isString.reserve(info.dataChs.size());
  columns.numeric.resize(info.dataChs.size());
  columns.text.resize(info.dataChs.size());

  for (auto* ch : info.dataChs) {
    const auto it            = channelIndex.find(ch);
    const std::size_t global = (it != channelIndex.end()) ? it->second : channelIsString.size();
    columns.channelIndex.push_back(global);
    columns.isString.push_back(global < channelIsString.size() ? channelIsString[global] : false);
  }

  return columns;
}

/**
 * @brief Runs the observer decode over every data group into per-group columns; returns false
 *        when any ReadData call reported failure (truncated/corrupt data section).
 */
static bool readAllGroups(MDF4::PlayerLoaderWorker* worker,
                          mdf::MdfReader& reader,
                          const std::vector<mdf::IDataGroup*>& dataGroups,
                          const std::map<mdf::IChannel*, std::size_t>& channelIndex,
                          const std::vector<bool>& channelIsString,
                          const TimeConfig& timeConfig,
                          DecodeState& state)
{
  bool read_ok = true;
  for (auto* dg : dataGroups) {
    if (!dg || worker->cancelRequested())
      continue;

    auto cgInfos = buildCgInfos(dg, timeConfig.perGroupTime, timeConfig.groupTimeChannels);
    std::vector<std::unique_ptr<GroupColumnObserver>> observers;
    observers.reserve(cgInfos.size());
    for (auto& ci : cgInfos) {
      state.groups.push_back(prepareGroupColumns(ci, channelIndex, channelIsString));
      auto obs = std::make_unique<GroupColumnObserver>(
        *dg, worker, state.groups.back(), ci.dataChs, ci.timeCh, ci.cg->RecordId());
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
                                    state.legacyTimestamps,
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
void MDF4::PlayerLoaderWorker::decodeFile(const QString& filePath,
                                          quint64 generation,
                                          const QVector<quint8>& channelSourceBit)
{
  SS_ASSERT_LOG(!filePath.isEmpty());

  m_recordsSeen      = 0;
  m_recordsTotal     = 0;
  m_activeGeneration = generation;

  auto payload              = std::make_shared<PlayerDecodePayload>();
  payload->channelSourceBit = channelSourceBit;
  payload->filePath         = filePath;
  payload->generation       = generation;

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
  std::map<mdf::IChannel*, std::size_t> channelIndex;
  for (std::size_t i = 0; i < allChannels.size(); ++i) {
    auto* ch = allChannels[i];
    payload->channelNames.append(ch ? QString::fromStdString(ch->Name()) : QString());
    payload->channelIsString.push_back(isStringChannel(ch));
    if (ch)
      channelIndex.emplace(ch, i);
  }

  DecodeState state;
  const bool read_ok = readAllGroups(
    this, *reader, dataGroups, channelIndex, payload->channelIsString, timeConfig, state);

  if (m_cancelRequested.load(std::memory_order_relaxed)) {
    payload->cancelled = true;
    Q_EMIT finished(payload);
    return;
  }

  mergeGroups(*payload, state);
  payload->ok          = true;
  payload->partialData = !read_ok;
  Q_EMIT finished(payload);
}
