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

#include "DataModel/FrameBuilder.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdio>

#if defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) \
  && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 130300
#  define SS_APPLE_NO_FLOAT_TO_CHARS 1
#  include <xlocale.h>
#endif
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <stdexcept>

#include "API/Server.h"
#include "AppState.h"
#include "CSV/Export.h"
#include "CSV/Player.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"
#include "DataModel/Scripting/DashboardApi.h"
#include "DataModel/Scripting/DeviceWriteApi.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/FrameParserPipeline.h"
#include "DataModel/Scripting/LuaCompat.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "IO/ConnectionManager.h"
#include "MDF4/Export.h"
#include "MDF4/Player.h"
#include "Misc/TimerEvents.h"
#include "Misc/Utilities.h"
#include "SessionContext.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#  include "Licensing/CommercialToken.h"
#  include "Licensing/LemonSqueezy.h"
#  include "MQTT/Publisher.h"
#  include "Sessions/Export.h"
#  include "Sessions/Player.h"
#endif

#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

/**
 * @brief Returns the per-frame cadence carried by a captured chunk, clamped to >=1 ns.
 */
[[nodiscard]] std::chrono::nanoseconds capturedFrameStep(const IO::CapturedDataPtr& data)
{
  if (!data)
    return std::chrono::nanoseconds(1);

  return std::max(std::chrono::nanoseconds(1), data->frameStep);
}

/**
 * @brief Builds the runtime group list from the project, dropping disabled groups and the
 *        disabled datasets of the survivors so frame building never sees them. The editor keeps
 *        the full set; surviving datasets retain their explicit frame index, so no sibling shifts.
 */
[[nodiscard]] std::vector<DataModel::Group> buildEnabledGroups(
  const std::vector<DataModel::Group>& projectGroups)
{
  std::vector<DataModel::Group> groups;
  groups.reserve(projectGroups.size());

  for (const auto& group : projectGroups) {
    if (!group.enabled)
      continue;

    DataModel::Group runtimeGroup = group;
    std::vector<DataModel::Dataset> datasets;
    datasets.reserve(runtimeGroup.datasets.size());
    for (auto& dataset : runtimeGroup.datasets)
      if (dataset.enabled)
        datasets.push_back(std::move(dataset));

    runtimeGroup.datasets = std::move(datasets);
    groups.push_back(std::move(runtimeGroup));
  }

  return groups;
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the FrameBuilder singleton and wires its watchdog/license hooks.
 */
DataModel::FrameBuilder::FrameBuilder()
  : m_quickPlotChannels(-1)
  , m_quickPlotHasHeader(false)
  , m_parseBudgetSkipping(false)
  , m_parseBudgetWarned(false)
  , m_parseBudgetEnabled(true)
  , m_parseBudgetEpisodeActive(false)
  , m_lastConnectedState(false)
  , m_playerOpen(false)
  , m_anyAsyncSink(false)
  , m_captureDatasetValues(false)
  , m_captureFlagsDirty(true)
  , m_externalTableApiUsers(false)
  , m_captureLatestFrame(false)
  , m_changeDriven(false)
  , m_shuttingDown(false)
  , m_seenEngineEpoch(-1)
  , m_operationMode(SerialStudio::ProjectFile)
  , m_parseBudgetUsedNs(0)
  , m_parseBudgetWindowStart(BudgetClock::time_point{})
  , m_parsedFrameCount(0)
  , m_skippedFrameCount(0)
  , m_transformErrors(0)
  , m_lastTransformDatasetUniqueId(-1)
  , m_lastTransformError()
  , m_jsTransformTimedOut(false)
  , m_latestFrameSourceId(-1)
  , m_latestFrameSeq(0)
  , m_engineCacheSourceId(-1)
  , m_luaEngineForSource(nullptr)
  , m_jsEngineForSource(nullptr)
  , m_compileGuard(0)
  , m_compilePending(false)
  , m_framePoolHint(0)
  , m_framePoolGeneration(1)
{
  m_framePool.reserve(kFramePoolSize);
  for (int i = 0; i < kFramePoolSize; ++i)
    m_framePool.emplace_back(std::make_shared<PooledFrameSlot>());

#ifdef BUILD_COMMERCIAL
  static auto& lemonSqueezy = Licensing::LemonSqueezy::instance();
  connect(&lemonSqueezy, &Licensing::LemonSqueezy::activatedChanged, this, [this] {
    syncFromProjectModel();
  });
#endif

  if (auto* app = qApp)
    connect(app, &QCoreApplication::aboutToQuit, this, [this]() {
      m_shuttingDown = true;
      destroyTransformEngines();
    });
}

/**
 * @brief Returns this session's frame builder. The object is owned by the SessionContext and built
 *        by the composition root, so a reach before adoption is a named fatal instead of an
 *        out-of-order lazy construction. Every hotpath caller binds it once into a static or a
 *        member reference, so the frame path never re-enters this (spec 0039 M2, wave D1).
 */
DataModel::FrameBuilder& DataModel::FrameBuilder::instance()
{
  return SessionContext::current().frameBuilder();
}

//--------------------------------------------------------------------------------------------------
// Frame pool
//--------------------------------------------------------------------------------------------------

/**
 * @brief Default-constructs a pool slot with no template generation or bound source frame.
 */
DataModel::FrameBuilder::PooledFrameSlot::PooledFrameSlot()
  : generation(0), matchedSrc(nullptr), owner(kUnownedSlot)
{}

/**
 * @brief Bumps the pool generation after a template rebuild so stale slots full-assign on reuse
 *        instead of leaking old identity fields through the structure-match fast path, and clears
 *        slot ownership so the next claims re-partition the pool across the new source set.
 */
void DataModel::FrameBuilder::invalidateFramePool() noexcept
{
  ++m_framePoolGeneration;
  m_poolSlotHintBySource.clear();
  for (const auto& slot : m_framePool)
    slot->owner = kUnownedSlot;
}

/**
 * @brief Probes for a free pool slot (use_count()==1 is exact: all aliases live on this thread)
 *        or returns kInvalidSlotIdx when every slot is pinned. Per-source affinity: a source
 *        reclaims its last slot so interleaved multi-source publishes keep the pointer-identity
 *        fast path and the span lane's retained values; foreign slots are stolen only last.
 */
size_t DataModel::FrameBuilder::claimPoolSlot(int sourceId) noexcept
{
  const size_t n = m_framePool.size();

  SS_ASSERT(sourceId >= 0, return kInvalidSlotIdx);
  SS_ASSUME(n == static_cast<size_t>(kFramePoolSize));

  const auto hit = m_poolSlotHintBySource.constFind(sourceId);
  if (hit != m_poolSlotHintBySource.cend()) [[likely]] {
    const size_t idx = hit.value();
    if (m_framePool[idx].use_count() == 1 && m_framePool[idx]->owner == sourceId)
      return idx;
  }

  const size_t hint = m_framePoolHint.load(std::memory_order_relaxed);
  size_t stealable  = kInvalidSlotIdx;

  for (size_t k = 0; k < n; ++k) {
    const size_t idx = (hint + k) % n;
    if (m_framePool[idx].use_count() != 1)
      continue;

    const int owner = m_framePool[idx]->owner;
    if (owner != sourceId && owner != kUnownedSlot) {
      if (stealable == kInvalidSlotIdx)
        stealable = idx;

      continue;
    }

    m_framePool[idx]->owner = sourceId;
    m_poolSlotHintBySource.insert(sourceId, idx);
    m_framePoolHint.store(idx, std::memory_order_relaxed);
    return idx;
  }

  if (stealable != kInvalidSlotIdx) {
    m_framePool[stealable]->owner = sourceId;
    m_poolSlotHintBySource.insert(sourceId, stealable);
    m_framePoolHint.store(stealable, std::memory_order_relaxed);
    return stealable;
  }

  return kInvalidSlotIdx;
}

/**
 * @brief Logs the one-shot pool-exhaustion warning before a heap-allocation fallback.
 */
SS_COLD void DataModel::FrameBuilder::notePoolExhausted()
{
  static bool warned = false;
  if (!warned) [[unlikely]] {
    warned = true;
    qWarning() << "[FrameBuilder] Frame pool exhausted (" << kFramePoolSize
               << " slots), consumers are not draining; falling back to heap allocation.";
    static auto& nc = NotificationCenter::instance();
    QMetaObject::invokeMethod(
      &nc,
      "postWarning",
      Qt::QueuedConnection,
      Q_ARG(QString, QStringLiteral("FrameBuilder")),
      Q_ARG(QString, tr("Frame pool exhausted")),
      Q_ARG(QString,
            tr("A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API "
               "subscriber) is not draining frames fast enough. Serial Studio is falling "
               "back to per-frame allocations until the backlog clears. Disable a heavy "
               "consumer or reduce the data rate.")));
  }
}

/**
 * @brief Binds a structure-synced slot to its source template: remembers the match and rebuilds
 *        the flattened dataset table the span lane walks.
 */
void DataModel::FrameBuilder::bindSlotTemplate(PooledFrameSlot* slot, const DataModel::Frame& src)
{
  SS_ASSERT(slot != nullptr, return);
  // code-verify off
  // Debug-only structural parity check: compare_frames walks every group and dataset, so a
  // release evaluation would add an O(datasets) pass to every slot rebind.
  Q_ASSERT(compare_frames(slot->frame.data, src));
  // code-verify on

  slot->matchedSrc = &src;

  slot->flat.clear();
  for (auto& group : slot->frame.data.groups)
    for (auto& dataset : group.datasets)
      slot->flat.push_back(&dataset);
}

/**
 * @brief Syncs a claimed slot's structure to @p src. Returns true when only values need
 *        refreshing (the steady state); false when the slot was full-assigned from the template.
 */
bool DataModel::FrameBuilder::preparePooledSlot(PooledFrameSlot* slot, const DataModel::Frame& src)
{
  SS_ASSERT(slot != nullptr, return false);

  if (slot->generation == m_framePoolGeneration && slot->matchedSrc == &src) [[likely]] {
    // code-verify off
    // Debug-only parity check on the steady-state fast path; compare_frames is an O(datasets)
    // walk that must never run per frame in release.
    Q_ASSERT(compare_frames(slot->frame.data, src));
    // code-verify on
    return true;
  }

  if (slot->generation == m_framePoolGeneration && slot->frame.data.sourceId == src.sourceId
      && compare_frames(slot->frame.data, src)) {
    bindSlotTemplate(slot, src);
    return true;
  }

  slot->frame.data = src;
  slot->generation = m_framePoolGeneration;
  bindSlotTemplate(slot, src);
  return false;
}

/**
 * @brief Claims a free pool slot, copies @p src + @p ts into it, and returns an aliasing
 *        shared_ptr: no deleter, no per-frame control block.
 */
SS_HOT DataModel::TimestampedFramePtr DataModel::FrameBuilder::acquireFrame(
  const DataModel::Frame& src, const DataModel::TimestampedFrame::SteadyTimePoint& ts)
{
  const size_t idx = claimPoolSlot(src.sourceId);
  if (idx == kInvalidSlotIdx) [[unlikely]] {
    notePoolExhausted();
    auto heap                 = std::make_shared<TimestampedFrame>(src, ts);
    heap->structureGeneration = m_framePoolGeneration;
    return heap;
  }

  const auto& slotOwner = m_framePool[idx];
  auto* slotRaw         = slotOwner.get();

  if (preparePooledSlot(slotRaw, src)) [[likely]]
    copy_frame_values(slotRaw->frame.data, src);

  slotRaw->frame.timestamp           = ts;
  slotRaw->frame.structureGeneration = m_framePoolGeneration;
  SS_ASSERT_LOG(slotRaw->frame.data.groups.size() == src.groups.size());

  return TimestampedFramePtr(slotOwner, &slotRaw->frame);
}

/**
 * @brief Convenience overload that timestamps the slot with SteadyClock::now().
 */
SS_HOT DataModel::TimestampedFramePtr DataModel::FrameBuilder::acquireFrame(
  const DataModel::Frame& src)
{
  return acquireFrame(src, DataModel::TimestampedFrame::SteadyClock::now());
}

//--------------------------------------------------------------------------------------------------
// Public accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current project-mode frame.
 */
const DataModel::Frame& DataModel::FrameBuilder::frame() const noexcept
{
  return m_frame;
}

/**
 * @brief Returns the number of project frames published since the last counter reset.
 */
quint64 DataModel::FrameBuilder::parsedFrameCount() const noexcept
{
  return m_parsedFrameCount;
}

/**
 * @brief Returns the number of frames dropped by the parse-load budget since the last reset.
 */
quint64 DataModel::FrameBuilder::skippedFrameCount() const noexcept
{
  return m_skippedFrameCount;
}

/**
 * @brief Returns how many per-dataset transform calls have failed since the last engine rebuild.
 */
quint64 DataModel::FrameBuilder::transformErrorCount() const noexcept
{
  return m_transformErrors;
}

/**
 * @brief Returns the uniqueId of the dataset whose transform error message is retained, or -1.
 */
int DataModel::FrameBuilder::lastTransformDataset() const noexcept
{
  return m_lastTransformDatasetUniqueId;
}

/**
 * @brief Returns the retained message of the first failure of the current failing dataset.
 */
const QString& DataModel::FrameBuilder::lastTransformError() const noexcept
{
  return m_lastTransformError;
}

/**
 * @brief Zeroes the parsed/skipped frame counters (used by the throughput benchmark).
 */
void DataModel::FrameBuilder::resetFrameCounters() noexcept
{
  m_parsedFrameCount  = 0;
  m_skippedFrameCount = 0;
}

/**
 * @brief Enables/disables the parse-load budget guard (disabled by the throughput benchmark).
 */
void DataModel::FrameBuilder::setParseBudgetEnabled(bool enabled) noexcept
{
  m_parseBudgetEnabled = enabled;
}

/**
 * @brief Returns the current Quick Plot frame.
 */
const DataModel::Frame& DataModel::FrameBuilder::quickPlotFrame() const noexcept
{
  return m_quickPlotFrame;
}

/**
 * @brief Returns the shared DataTableStore for read-only callers (e.g. clearLookupCache).
 */
const DataModel::DataTableStore& DataModel::FrameBuilder::tableStore() const noexcept
{
  return m_tableStore;
}

/**
 * @brief Mutable DataTableStore access for main-thread writers (the API value commands).
 */
DataModel::DataTableStore& DataModel::FrameBuilder::tableStore() noexcept
{
  return m_tableStore;
}

/**
 * @brief Default-constructs an empty latest-frame snapshot (no chunk, sequence 0).
 */
DataModel::FrameBuilder::LatestFrameInfo::LatestFrameInfo()
  : sourceId(-1), sequence(0), timestampMs(0), channelsSequence(0)
{}

/**
 * @brief Returns the latest captured frame for @p sourceId, the newest across all sources when
 *        @p sourceId is negative, or nullptr when capture is off or nothing arrived yet.
 */
const DataModel::FrameBuilder::LatestFrameInfo* DataModel::FrameBuilder::latestFrame(
  int sourceId) const noexcept
{
  const int key = (sourceId >= 0) ? sourceId : m_latestFrameSourceId;
  if (key < 0)
    return nullptr;

  const auto it = m_latestFrames.constFind(key);
  return (it != m_latestFrames.constEnd()) ? &it.value() : nullptr;
}

//--------------------------------------------------------------------------------------------------
// External connection setup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires ConnectionManager and ProjectModel signals to the FrameBuilder.
 */
void DataModel::FrameBuilder::setupExternalConnections()
{
  connect(&IO::ConnectionManager::instance(),
          &IO::ConnectionManager::connectedChanged,
          this,
          &DataModel::FrameBuilder::onConnectedChanged);

  connect(&AppState::instance(),
          &AppState::operationModeChanged,
          this,
          &DataModel::FrameBuilder::onOperationModeChanged);

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::sourceDeleted,
          this,
          &DataModel::FrameBuilder::onSourceRemoved);

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::changeDrivenTransformsChanged,
          this,
          [this] { m_captureFlagsDirty = true; });

  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::timeout1Hz,
          this,
          &DataModel::FrameBuilder::collectTransformEngineGarbage);

  connect(&CSV::Player::instance(), &CSV::Player::openChanged, this, [this] {
    m_playerOpen        = SerialStudio::isAnyPlayerOpen();
    m_captureFlagsDirty = true;
    rebuildTransformsForPlayback();
  });
  connect(&MDF4::Player::instance(), &MDF4::Player::openChanged, this, [this] {
    m_playerOpen        = SerialStudio::isAnyPlayerOpen();
    m_captureFlagsDirty = true;
    rebuildTransformsForPlayback();
  });

#ifdef BUILD_COMMERCIAL
  connect(&Sessions::Player::instance(), &Sessions::Player::openChanged, this, [this] {
    m_playerOpen        = SerialStudio::isAnyPlayerOpen();
    m_captureFlagsDirty = true;
    rebuildTransformsForPlayback();
  });
#endif

  connect(&CSV::Export::instance(), &CSV::Export::enabledChanged, this, [this] {
    refreshAnyAsyncSink();
  });
  connect(&MDF4::Export::instance(), &MDF4::Export::enabledChanged, this, [this] {
    refreshAnyAsyncSink();
  });
  connect(&API::Server::instance(), &API::Server::enabledChanged, this, [this] {
    refreshAnyAsyncSink();
    refreshLatestFrameCapture();
  });
  connect(&API::Server::instance(), &API::Server::clientCountChanged, this, [this] {
    refreshAnyAsyncSink();
  });
  connect(&DataModel::ControlScript::instance(),
          &DataModel::ControlScript::runningChanged,
          this,
          [this] { refreshLatestFrameCapture(); });
#ifdef BUILD_COMMERCIAL
  connect(&Sessions::Export::instance(), &Sessions::Export::enabledChanged, this, [this] {
    refreshAnyAsyncSink();
  });
  connect(&MQTT::Publisher::instance(), &MQTT::Publisher::configurationChanged, this, [this] {
    refreshAnyAsyncSink();
  });
#endif
#ifdef ENABLE_GRPC
  connect(&API::GRPC::GRPCServer::instance(), &API::GRPC::GRPCServer::enabledChanged, this, [this] {
    refreshAnyAsyncSink();
  });
  connect(&API::GRPC::GRPCServer::instance(),
          &API::GRPC::GRPCServer::clientCountChanged,
          this,
          [this] { refreshAnyAsyncSink(); });
#endif

  m_operationMode = AppState::instance().operationMode();
  m_playerOpen    = SerialStudio::isAnyPlayerOpen();
  refreshAnyAsyncSink();
  refreshLatestFrameCapture();
}

/**
 * @brief Recomputes the cached any-async-consumer flag from every export/output module. The
 *        TCP and gRPC servers only count while a client is connected: with zero clients their
 *        workers drop every frame, so the per-frame detached copy would be pure waste.
 */
void DataModel::FrameBuilder::refreshAnyAsyncSink()
{
  static auto& server     = API::Server::instance();
  static auto& csvExport  = CSV::Export::instance();
  static auto& mdf4Export = MDF4::Export::instance();
  bool any                = csvExport.exportEnabled() || mdf4Export.exportEnabled()
          || (server.enabled() && server.clientCount() > 0);
#ifdef BUILD_COMMERCIAL
  static auto& sessionsExport = Sessions::Export::instance();
  static auto& mqttPublisher  = MQTT::Publisher::instance();
  any                         = any || sessionsExport.exportEnabled() || mqttPublisher.enabled();
#endif
#ifdef ENABLE_GRPC
  static auto& grpc = API::GRPC::GRPCServer::instance();
  any               = any || (grpc.enabled() && grpc.clientCount() > 0);
#endif

  m_anyAsyncSink = any;
}

/**
 * @brief Recomputes the cached latest-frame capture flag (control script or API server active);
 *        drops the retained chunks when every consumer is gone so FrameReader slots unpin.
 */
void DataModel::FrameBuilder::refreshLatestFrameCapture()
{
  const bool wasEnabled = m_captureLatestFrame;

  static auto& controlScript = DataModel::ControlScript::instance();
  static auto& server        = API::Server::instance();
  m_captureLatestFrame       = controlScript.running() || server.enabled();

  if (wasEnabled && !m_captureLatestFrame) {
    m_latestFrames.clear();
    m_latestFrameSourceId = -1;
  }
}

//--------------------------------------------------------------------------------------------------
// Project model sync
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds m_frame from ProjectModel's already-parsed in-memory state (no file I/O).
 */
void DataModel::FrameBuilder::syncFromProjectModel()
{
  static auto& pm = DataModel::ProjectModel::instance();
  SS_ASSERT_LOG(!pm.title().isEmpty());

  clear_frame(m_frame);
  m_sourceFrames.clear();
  m_sourceFrameCounters.clear();
  m_republishedSourceIds.clear();

  m_externalTableApiUsers = false;
  m_captureFlagsDirty     = true;

  m_frame.title   = pm.title();
  m_frame.groups  = buildEnabledGroups(pm.groups());
  m_frame.actions = pm.actions();
  m_frame.sources = pm.sources();

  finalize_frame(m_frame);
  invalidateFramePool();
  initializeTableStore();
  compileTransforms();
  parseBudgetReset();

  SS_ASSERT_LOG(!m_frame.title.isEmpty());

  Q_EMIT jsonFileMapChanged();
}

//--------------------------------------------------------------------------------------------------
// Quick Plot header registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers Quick Plot channel headers, or clears them when @p headers is empty.
 */
void DataModel::FrameBuilder::registerQuickPlotHeaders(const QStringList& headers)
{
  if (!headers.isEmpty()) {
    m_quickPlotHasHeader    = true;
    m_quickPlotChannelNames = headers;
  } else {
    m_quickPlotHasHeader = false;
    m_quickPlotChannelNames.clear();
  }
}

//--------------------------------------------------------------------------------------------------
// Hotpath data processing functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records the latest received chunk for @p sourceId: a retained pool reference (the
 *        FrameReader claim probe skips pinned slots), zero copy, gated on m_captureLatestFrame.
 */
void DataModel::FrameBuilder::captureLatestChunk(int sourceId, const IO::CapturedDataPtr& data)
{
  SS_ASSERT(data, return);
  SS_ASSERT(m_captureLatestFrame, return);

  auto& entry    = m_latestFrames[sourceId];
  entry.chunk    = data;
  entry.sourceId = sourceId;
  entry.sequence = ++m_latestFrameSeq;
  entry.timestampMs =
    std::chrono::duration_cast<std::chrono::milliseconds>(data->timestamp.time_since_epoch())
      .count();

  m_latestFrameSourceId = sourceId;
}

/**
 * @brief Records the parser's channel tokens for the latest chunk via implicit sharing.
 */
void DataModel::FrameBuilder::captureLatestChannels(int sourceId, const QStringList& channels)
{
  SS_ASSERT(m_captureLatestFrame, return);
  SS_ASSERT(!channels.isEmpty(), return);

  const auto it = m_latestFrames.find(sourceId);
  if (it == m_latestFrames.end()) [[unlikely]]
    return;

  it->channels         = channels;
  it->channelsSequence = it->sequence;
}

/**
 * @brief Span-lane twin of captureLatestChannels: in-place UTF-8 writes into the reused token
 *        list keep the capture allocation-free in steady state. @p count is span-lane bounded.
 */
void DataModel::FrameBuilder::captureLatestChannelSpans(int sourceId,
                                                        const QByteArrayView* spans,
                                                        qsizetype count)
{
  SS_ASSERT(spans != nullptr, return);
  SS_ASSERT(m_captureLatestFrame, return);
  SS_ASSERT(count > 0 && count <= kMaxSpanFields,
            count = std::clamp<qsizetype>(count, 0, kMaxSpanFields));

  const auto it = m_latestFrames.find(sourceId);
  if (it == m_latestFrames.end()) [[unlikely]]
    return;

  it->channels.resize(count);
  for (qsizetype i = 0; i < count; ++i)
    DataModel::assign_utf8_in_place(it->channels[i], spans[i]);

  it->channelsSequence = it->sequence;
}

/**
 * @brief Dispatches a captured chunk to the parser for the current operation mode.
 */
void DataModel::FrameBuilder::hotpathRxFrame(const IO::CapturedDataPtr& data)
{
  SS_ASSERT(data, return);
  SS_ASSERT(!data->data.isEmpty(), return);
  // code-verify off
  // Debug-only cache-coherence probe: polling AppState::instance() per frame in release is exactly
  // the singleton read the cached m_operationMode exists to avoid (spec 0001).
  Q_ASSERT(m_operationMode == AppState::instance().operationMode());
  // code-verify on

  if (m_captureLatestFrame) [[unlikely]]
    captureLatestChunk(0, data);

  switch (m_operationMode) {
    case SerialStudio::QuickPlot:
      parseQuickPlotFrame(data);
      break;
    case SerialStudio::ProjectFile:
      parseProjectFrame(data);
      break;
    case SerialStudio::ConsoleOnly:
      break;
    default:
      break;
  }
}

/**
 * @brief Per-source variant of hotpathRxFrame -- routes data through the matching source parser.
 */
void DataModel::FrameBuilder::hotpathRxSourceFrame(int sourceId, const IO::CapturedDataPtr& data)
{
  SS_ASSERT(sourceId >= 0, return);
  SS_ASSERT(data, return);
  SS_ASSERT(!data->data.isEmpty(), return);

  if (m_operationMode != SerialStudio::ProjectFile) {
    hotpathRxFrame(data);
    return;
  }

  if (m_captureLatestFrame) [[unlikely]]
    captureLatestChunk(sourceId, data);

  parseProjectFrame(sourceId, data);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wipes transform engines on source deletion -- ProjectModel renumbers IDs and engines
 * recompile lazily.
 */
void DataModel::FrameBuilder::onSourceRemoved()
{
  destroyTransformEngines();
}

/**
 * @brief Drops mode-scoped frame state on operation-mode changes so the next published frame is
 *        rebuilt from the active mode's template instead of a recycled pool slot.
 */
void DataModel::FrameBuilder::onOperationModeChanged()
{
  static auto& appState = AppState::instance();
  SS_ASSERT(appState.operationMode() >= SerialStudio::ProjectFile
              && appState.operationMode() <= SerialStudio::QuickPlot,
            return);

  m_operationMode     = appState.operationMode();
  m_quickPlotChannels = -1;
  m_sourceFrames.clear();
  m_sourceFrameCounters.clear();
  m_republishedSourceIds.clear();
  m_latestFrames.clear();
  m_latestFrameSourceId = -1;
  invalidateFramePool();
  parseBudgetReset();
}

/**
 * @brief Builds and publishes a per-source template frame for dashboard configuration.
 */
void DataModel::FrameBuilder::publishSourceTemplateFrame(const DataModel::Source& src)
{
  DataModel::Frame srcFrame;
  srcFrame.sourceId                   = src.sourceId;
  srcFrame.title                      = m_frame.title;
  srcFrame.actions                    = m_frame.actions;
  srcFrame.containsCommercialFeatures = m_frame.containsCommercialFeatures;
  for (const auto& g : m_frame.groups)
    if (g.sourceId == src.sourceId)
      srcFrame.groups.push_back(g);

  if (srcFrame.groups.empty())
    return;

  m_sourceFrames.insert(src.sourceId, srcFrame);
  m_republishedSourceIds.insert(src.sourceId);
  hotpathTxFrame(acquireFrame(srcFrame));
}

/**
 * @brief Re-runs every dataset transform from the last raw values and republishes the live
 *        frames: dashboard only with @p feedExports false, hotpathTxFrame() fan-out with it
 *        true. A frame republishes only on a changed dataset value or its first publish, so a
 *        synthetic tick never touches the plot clock of a source whose data did not change.
 */
bool DataModel::FrameBuilder::republishFrames(bool feedExports)
{
  if (m_operationMode != SerialStudio::ProjectFile)
    return false;

  static auto& dashboard           = UI::Dashboard::instance();
  constexpr int combined_frame_key = -1;

  bool published  = false;
  bool any_source = false;
  for (auto& frame : m_sourceFrames) {
    if (frame.groups.empty() || frame.title.isEmpty())
      continue;

    any_source         = true;
    const bool changed = reprocessDatasetValues(frame);
    if (!changed && m_republishedSourceIds.contains(frame.sourceId))
      continue;

    m_republishedSourceIds.insert(frame.sourceId);
    if (feedExports)
      hotpathTxFrame(acquireFrame(frame));
    else
      dashboard.hotpathRxFrame(acquireFrame(frame));

    published = true;
  }

  if (!any_source && !m_frame.groups.empty() && !m_frame.title.isEmpty()) {
    const bool changed = reprocessDatasetValues(m_frame);
    if (changed || !m_republishedSourceIds.contains(combined_frame_key)) {
      m_republishedSourceIds.insert(combined_frame_key);
      if (feedExports)
        hotpathTxFrame(acquireFrame(m_frame));
      else
        dashboard.hotpathRxFrame(acquireFrame(m_frame));

      published = true;
    }
  }

  return published;
}

/**
 * @brief Re-runs transforms from the last received values and republishes to the dashboard only,
 *        with no export fan-out, so a synthetic refresh never re-records frames that were already
 *        exported on arrival. Returns false when no frame structure is available to publish.
 */
bool DataModel::FrameBuilder::reprocessFrames()
{
  return republishFrames(false);
}

/**
 * @brief Forces a render from the current table/dataset state even when the device is silent:
 *        seeds each source frame from the template if missing, runs the transform-only pass, and
 *        publishes through hotpathTxFrame so table-driven datasets both render and feed the
 *        CSV/MDF4/session/MQTT/API exports. Works from the first loop().
 */
bool DataModel::FrameBuilder::dashboardTick()
{
  if (m_operationMode != SerialStudio::ProjectFile)
    return false;

  if (m_frame.groups.empty() || m_frame.title.isEmpty())
    return false;

  if (m_sourceFrames.isEmpty())
    for (const auto& g : m_frame.groups)
      (void)ensureSourceFrame(g.sourceId);

  return republishFrames(true);
}

/**
 * @brief Handles connection transitions: recompiles transforms, reloads parser, fires
 *        auto-actions. The latest-frame store clears on both edges so io.getLatestFrame can
 *        never serve a previous connection's frame. No-op after aboutToQuit: a connection
 *        signal fired during static destruction must not reload the parser or rebuild engines.
 */
void DataModel::FrameBuilder::onConnectedChanged()
{
  if (m_shuttingDown) [[unlikely]]
    return;

  static auto& appState  = AppState::instance();
  static auto& ioManager = IO::ConnectionManager::instance();
  SS_ASSERT(appState.operationMode() >= SerialStudio::ProjectFile
              && appState.operationMode() <= SerialStudio::QuickPlot,
            return);

  const bool nowConnected = ioManager.isConnected();
  if (nowConnected == m_lastConnectedState)
    return;

  m_lastConnectedState = nowConnected;
  m_quickPlotChannels  = -1;

  invalidateFramePool();

  parseBudgetReset();

  if (!nowConnected) {
    m_sourceFrames.clear();
    m_sourceFrameCounters.clear();
    m_republishedSourceIds.clear();
    m_latestFrames.clear();
    m_latestFrameSourceId = -1;
    destroyTransformEngines();
    m_tableStore.clear();
    return;
  }

  m_latestFrames.clear();
  m_latestFrameSourceId = -1;

  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  SS_ASSERT_LOG(!m_frame.title.isEmpty());
  initializeTableStore();
  static auto& parser = DataModel::FrameParser::instance();
  parser.readCode();
  compileTransforms();

  const auto& actions = m_frame.actions;
  for (const auto& action : actions)
    if (action.autoExecuteOnConnect) {
      const qint64 written = ioManager.writeDataToDevice(action.sourceId, get_tx_bytes(action));
      if (written < 0) [[unlikely]]
        qWarning() << "[FrameBuilder] Auto-execute write failed for action:" << action.title;
    }

  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto& sources       = projectModel.sources();
  if (sources.size() > 1) {
    for (const auto& src : sources)
      publishSourceTemplateFrame(src);

    return;
  }

  const bool allImageGroups =
    !m_frame.groups.empty()
    && std::all_of(m_frame.groups.begin(), m_frame.groups.end(), [](const DataModel::Group& g) {
         return g.widget == QLatin1String("image");
       });

  if (allImageGroups)
    hotpathTxFrame(acquireFrame(m_frame));
}

//--------------------------------------------------------------------------------------------------
// Frame parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses a project frame using the configured decoding method.
 */
void DataModel::FrameBuilder::parseProjectFrame(const IO::CapturedDataPtr& data)
{
  SS_ASSERT(data, return);
  SS_ASSERT(!data->data.isEmpty(), return);

  if (m_frame.groups.empty()) [[unlikely]]
    return;

  if (parseBudgetSkipFrame()) [[unlikely]]
    return;

  const auto t0 = m_parseBudgetEnabled ? BudgetClock::now() : BudgetClock::time_point{};

  const int published = trySpanLane(0, false, m_frame, data);
  if (published >= 0) {
    m_parsedFrameCount += static_cast<quint64>(published);
    parseBudgetAccount(t0);
    return;
  }

  QList<QStringList> multiChannels;
  decodeProjectChannels(0, false, data, multiChannels);

  const auto step = capturedFrameStep(data);
  for (int i = 0; i < multiChannels.size(); ++i) {
    const auto& channels = multiChannels.at(i);
    if (channels.isEmpty()) [[unlikely]]
      continue;

    const auto frameTs = data->timestamp + step * i;
    TransformFrameInfo info;
    info.sourceId = 0;

    if (!m_transformEngines.empty()) {
      info.frameNumber = ++m_sourceFrameCounters[0];
      info.timestampMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(frameTs.time_since_epoch()).count();
    }

    if (m_captureLatestFrame) [[unlikely]]
      captureLatestChannels(0, channels);

    applyDatasetValues(m_frame, channels, info);
    hotpathTxFrame(acquireFrame(m_frame, frameTs));
    ++m_parsedFrameCount;
  }

  parseBudgetAccount(t0);
}

/**
 * @brief Source-aware variant of parseProjectFrame.
 */
void DataModel::FrameBuilder::parseProjectFrame(int sourceId, const IO::CapturedDataPtr& data)
{
  SS_ASSERT(sourceId >= 0, return);
  SS_ASSERT(data, return);
  SS_ASSERT(!data->data.isEmpty(), return);

  if (m_frame.groups.empty()) [[unlikely]]
    return;

  if (parseBudgetSkipFrame()) [[unlikely]]
    return;

  const auto t0 = m_parseBudgetEnabled ? BudgetClock::now() : BudgetClock::time_point{};

  const int published = trySpanLane(sourceId, true, ensureSourceFrame(sourceId), data);
  if (published >= 0) {
    m_parsedFrameCount += static_cast<quint64>(published);
    parseBudgetAccount(t0);
    return;
  }

  QList<QStringList> multiChannels;
  decodeProjectChannels(sourceId, true, data, multiChannels);

  const auto step = capturedFrameStep(data);
  for (int i = 0; i < multiChannels.size(); ++i) {
    const auto& channels = multiChannels.at(i);
    if (channels.isEmpty()) [[unlikely]]
      continue;

    DataModel::Frame& srcFrame = ensureSourceFrame(sourceId);
    const auto frameTs         = data->timestamp + step * i;
    TransformFrameInfo info;
    info.sourceId = sourceId;

    if (!m_transformEngines.empty()) {
      info.frameNumber = ++m_sourceFrameCounters[sourceId];
      info.timestampMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(frameTs.time_since_epoch()).count();
    }

    if (m_captureLatestFrame) [[unlikely]]
      captureLatestChannels(sourceId, channels);

    applyDatasetValues(srcFrame, channels, info);
    hotpathTxFrame(acquireFrame(srcFrame, frameTs));
    ++m_parsedFrameCount;
  }

  parseBudgetAccount(t0);
}

/**
 * @brief Replay ingestion fast lane (spec 0020): applies an already-split channel row from a
 *        file player -- no byte round-trip -- and publishes it stamped with the recorded
 *        timestamp. Dashboard and read-only API observers only; recording sinks never see
 *        replayed frames, so a replay can never re-record itself.
 */
void DataModel::FrameBuilder::replayChannels(
  int sourceId,
  const QStringList& channels,
  const DataModel::TimestampedFrame::SteadyTimePoint& timestamp)
{
  SS_ASSERT(sourceId >= 0, return);
  SS_ASSERT(m_playerOpen, return);
  SS_ASSERT(m_operationMode == SerialStudio::ProjectFile, return);

  if (channels.isEmpty() || m_frame.groups.empty()) [[unlikely]]
    return;

  DataModel::Frame& srcFrame = ensureSourceFrame(sourceId);
  if (srcFrame.groups.empty() || srcFrame.title.isEmpty()) [[unlikely]]
    return;

  TransformFrameInfo info;
  info.sourceId = sourceId;

  applyDatasetValues(srcFrame, channels, info);
  publishReplayFrame(acquireFrame(srcFrame, timestamp));
  ++m_parsedFrameCount;
}

/**
 * @brief Formats a double exactly like QString::number(v, 'g', 10) but locale-independent and
 *        in place (C-locale %g semantics): the typed replay lane's display string. Apple ships
 *        float std::to_chars only from macOS 13.3, so older targets use snprintf_l with the
 *        NULL (C) locale. The debug parity assert is temporary scaffolding for spec 0022.
 */
static void assignFormattedDouble(QString& dst, double value)
{
  char buf[32];
#ifdef SS_APPLE_NO_FLOAT_TO_CHARS
  const int len = snprintf_l(buf, sizeof(buf), nullptr, "%.10g", value);
  SS_ASSERT(len > 0 && static_cast<size_t>(len) < sizeof(buf), return);
  DataModel::assign_utf8_in_place(dst, QByteArrayView(buf, static_cast<qsizetype>(len)));
#else
  const auto res = std::to_chars(buf, buf + sizeof(buf), value, std::chars_format::general, 10);
  SS_ASSERT(res.ec == std::errc(), return);
  DataModel::assign_utf8_in_place(dst, QByteArrayView(buf, static_cast<qsizetype>(res.ptr - buf)));
#endif
  // code-verify off
  // Debug-only parity scaffolding (spec 0022): QString::number allocates, so this can never
  // become a per-cell runtime check.
  Q_ASSERT(dst == QString::number(value, 'g', 10));
  // code-verify on
}

/**
 * @brief Returns the installed uniqueId->column replay map for @p sourceId, or nullptr when the
 *        player registered none (index-based fallback applies).
 */
const std::unordered_map<int, int>* DataModel::FrameBuilder::replayColumnsFor(int sourceId) const
{
  SS_ASSERT(sourceId >= 0, return nullptr);
  SS_ASSERT(SerialStudio::isFinalValuePlayerOpen(), return nullptr);

  const auto it = m_replayColumnMap.find(sourceId);
  return (it != m_replayColumnMap.end()) ? &it->second : nullptr;
}

/**
 * @brief Replay twin of applyDatasetValue for UTF-8 view cells: identical final-value branch
 *        order (column map, virtual zeros, index fallback), in-place string writes, one parse
 *        per cell, and no transform run -- replay keeps engines torn down.
 */
void DataModel::FrameBuilder::applyReplaySpanValue(Dataset& dataset,
                                                   const QByteArrayView* cells,
                                                   qsizetype count,
                                                   const std::unordered_map<int, int>* columns)
{
  SS_ASSERT(cells != nullptr, return);
  SS_ASSERT(count > 0, return);

  if (columns) [[likely]] {
    const auto it       = columns->find(dataset.uniqueId);
    const qsizetype col = (it != columns->end()) ? it->second : -1;
    if (col >= 0 && col < count) {
      DataModel::assign_utf8_in_place(dataset.value, cells[col]);
      dataset.numericValue = SerialStudio::toDouble(cells[col], &dataset.isNumeric);
    } else {
      dataset.numericValue = 0.0;
      dataset.value.clear();
      dataset.isNumeric = true;
    }
  } else if (dataset.virtual_) {
    dataset.numericValue = 0.0;
    dataset.value.clear();
    dataset.isNumeric = true;
  } else {
    const qsizetype idx = dataset.index;
    if (idx <= 0 || idx > count) [[unlikely]]
      return;

    DataModel::assign_utf8_in_place(dataset.value, cells[idx - 1]);
    dataset.numericValue = SerialStudio::toDouble(cells[idx - 1], &dataset.isNumeric);
  }

  dataset.rawNumericValue = dataset.numericValue;
  DataModel::assign_string_in_place(dataset.rawValue, dataset.value);

  if (m_captureDatasetValues)
    m_tableStore.setDatasetRaw(
      dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);

  if (!dataset.isNumeric)
    dataset.numericValue = (dataset.wgtMax > dataset.wgtMin) ? dataset.wgtMin : 0.0;

  if (m_captureDatasetValues)
    m_tableStore.setDatasetFinal(
      dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);
}

/**
 * @brief Replay twin of applyDatasetValue for typed cells: numeric cells keep the native double
 *        (spec 0022's R7 -- no format/parse round trip) while the display string is written in
 *        place with the same 'g'/10 rendering as before; text cells parse once like today.
 */
void DataModel::FrameBuilder::applyReplayTypedValue(Dataset& dataset,
                                                    const ReplayCell* cells,
                                                    qsizetype count,
                                                    const std::unordered_map<int, int>* columns)
{
  SS_ASSERT(cells != nullptr, return);
  SS_ASSERT(count > 0, return);

  const auto applyCell = [&](const ReplayCell& cell) {
    if (cell.text) {
      DataModel::assign_string_in_place(dataset.value, *cell.text);
      dataset.numericValue = SerialStudio::toDouble(dataset.value, &dataset.isNumeric);
    } else {
      assignFormattedDouble(dataset.value, cell.number);
      dataset.numericValue = cell.number;
      dataset.isNumeric    = true;
    }
  };

  if (columns) [[likely]] {
    const auto it       = columns->find(dataset.uniqueId);
    const qsizetype col = (it != columns->end()) ? it->second : -1;
    if (col >= 0 && col < count) {
      applyCell(cells[col]);
    } else {
      dataset.numericValue = 0.0;
      dataset.value.clear();
      dataset.isNumeric = true;
    }
  } else if (dataset.virtual_) {
    dataset.numericValue = 0.0;
    dataset.value.clear();
    dataset.isNumeric = true;
  } else {
    const qsizetype idx = dataset.index;
    if (idx <= 0 || idx > count) [[unlikely]]
      return;

    applyCell(cells[idx - 1]);
  }

  dataset.rawNumericValue = dataset.numericValue;
  DataModel::assign_string_in_place(dataset.rawValue, dataset.value);

  if (m_captureDatasetValues)
    m_tableStore.setDatasetRaw(
      dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);

  if (!dataset.isNumeric)
    dataset.numericValue = (dataset.wgtMax > dataset.wgtMin) ? dataset.wgtMin : 0.0;

  if (m_captureDatasetValues)
    m_tableStore.setDatasetFinal(
      dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);
}

/**
 * @brief Span-cell replay lane (spec 0022): UTF-8 view cells from the CSV player's mapped rows
 *        go straight into the per-source frame with zero intermediate QString lists, then
 *        publish through the same pooled-slot replay fan-out as replayChannels.
 */
void DataModel::FrameBuilder::replayChannelSpans(
  int sourceId,
  const QByteArrayView* cells,
  qsizetype count,
  const DataModel::TimestampedFrame::SteadyTimePoint& timestamp)
{
  SS_ASSERT(sourceId >= 0, return);
  SS_ASSERT(cells != nullptr || count == 0, return);
  SS_ASSERT(m_playerOpen, return);
  SS_ASSERT(m_operationMode == SerialStudio::ProjectFile, return);

  if (count <= 0 || m_frame.groups.empty()) [[unlikely]]
    return;

  DataModel::Frame& srcFrame = ensureSourceFrame(sourceId);
  if (srcFrame.groups.empty() || srcFrame.title.isEmpty()) [[unlikely]]
    return;

  const auto* columns = replayColumnsFor(sourceId);

  TransformFrameInfo info;
  info.sourceId = sourceId;

  const bool armedWatchdog = beginDatasetPass(info);
  for (auto& group : srcFrame.groups) {
    SS_NO_UNROLL
    for (auto& dataset : group.datasets)
      applyReplaySpanValue(dataset, cells, count, columns);
  }

  endDatasetPass(armedWatchdog);

  publishReplayFrame(acquireFrame(srcFrame, timestamp));
  ++m_parsedFrameCount;
}

/**
 * @brief Typed-cell replay lane (spec 0022): native doubles and borrowed text pointers from the
 *        MDF4 player's columnar caches, published through the same pooled-slot replay fan-out
 *        as replayChannels.
 */
void DataModel::FrameBuilder::replayChannelsTyped(
  int sourceId,
  const ReplayCell* cells,
  qsizetype count,
  const DataModel::TimestampedFrame::SteadyTimePoint& timestamp)
{
  SS_ASSERT(sourceId >= 0, return);
  SS_ASSERT(cells != nullptr || count == 0, return);
  SS_ASSERT(m_playerOpen, return);
  SS_ASSERT(m_operationMode == SerialStudio::ProjectFile, return);

  if (count <= 0 || m_frame.groups.empty()) [[unlikely]]
    return;

  DataModel::Frame& srcFrame = ensureSourceFrame(sourceId);
  if (srcFrame.groups.empty() || srcFrame.title.isEmpty()) [[unlikely]]
    return;

  const auto* columns = replayColumnsFor(sourceId);

  TransformFrameInfo info;
  info.sourceId = sourceId;

  const bool armedWatchdog = beginDatasetPass(info);
  for (auto& group : srcFrame.groups) {
    SS_NO_UNROLL
    for (auto& dataset : group.datasets)
      applyReplayTypedValue(dataset, cells, count, columns);
  }

  endDatasetPass(armedWatchdog);

  publishReplayFrame(acquireFrame(srcFrame, timestamp));
  ++m_parsedFrameCount;
}

/**
 * @brief Native span fast lane: parses byte views directly into the claimed pool slot; @p frame
 *        stays a structural template. Returns frames published, or -1 to use the QList path.
 */
int DataModel::FrameBuilder::trySpanLane(int sourceId,
                                         bool applyPerSourceOverride,
                                         DataModel::Frame& frame,
                                         const IO::CapturedDataPtr& data)
{
  SS_ASSERT(sourceId >= 0, return -1);
  SS_ASSERT(data, return -1);

  if (m_playerOpen) [[unlikely]]
    return -1;

  if (frame.groups.empty()) [[unlikely]]
    return -1;

  if (resolveDecoderMethod(sourceId, applyPerSourceOverride) != SerialStudio::PlainText)
    return -1;

  static auto& parser = DataModel::FrameParser::instance();
  const qsizetype tokens =
    parser.parseSpansUtf8(data->data, sourceId, m_spanScratch.data(), kMaxSpanFields);
  if (tokens < 0)
    return -1;

  if (tokens == 0)
    return 0;

  if (m_captureLatestFrame) [[unlikely]]
    captureLatestChannelSpans(sourceId, m_spanScratch.data(), tokens);

  TransformFrameInfo info;
  info.sourceId = sourceId;

  if (!m_transformEngines.empty()) [[unlikely]] {
    info.frameNumber = ++m_sourceFrameCounters[sourceId];
    info.timestampMs =
      std::chrono::duration_cast<std::chrono::milliseconds>(data->timestamp.time_since_epoch())
        .count();
  }

  const size_t idx = claimPoolSlot(sourceId);
  if (idx == kInvalidSlotIdx) [[unlikely]] {
    notePoolExhausted();
    auto heap                 = std::make_shared<TimestampedFrame>(frame, data->timestamp);
    heap->structureGeneration = m_framePoolGeneration;
    applyDatasetValuesSpans(heap->data, m_spanScratch.data(), tokens, info);
    hotpathTxFrame(heap);
    return 1;
  }

  const auto& slotOwner = m_framePool[idx];
  auto* slotRaw         = slotOwner.get();

  (void)preparePooledSlot(slotRaw, frame);
  applyDatasetValuesSpans(slotRaw->flat.data(),
                          static_cast<qsizetype>(slotRaw->flat.size()),
                          m_spanScratch.data(),
                          tokens,
                          info);

  slotRaw->frame.timestamp           = data->timestamp;
  slotRaw->frame.structureGeneration = m_framePoolGeneration;
  hotpathTxFrame(TimestampedFramePtr(slotOwner, &slotRaw->frame));
  return 1;
}

//--------------------------------------------------------------------------------------------------
// Parser-load budget guard
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true if the parser should skip this frame to keep the GUI responsive.
 */
bool DataModel::FrameBuilder::parseBudgetSkipFrame()
{
  if (!m_parseBudgetEnabled) [[unlikely]]
    return false;

  const auto now = BudgetClock::now();

  if (m_parseBudgetWindowStart == BudgetClock::time_point{}) [[unlikely]] {
    m_parseBudgetWindowStart = now;
    m_parseBudgetUsedNs      = 0;
    m_parseBudgetSkipping    = false;
    return false;
  }

  const auto windowNs =
    std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_parseBudgetWindowStart).count();
  if (windowNs >= static_cast<qint64>(kParseBudgetWindowMs) * 1'000'000LL) {
    if (!m_parseBudgetSkipping)
      m_parseBudgetEpisodeActive = false;

    m_parseBudgetWindowStart = now;
    m_parseBudgetUsedNs      = 0;
    m_parseBudgetSkipping    = false;
  }

  if (m_parseBudgetSkipping)
    ++m_skippedFrameCount;

  return m_parseBudgetSkipping;
}

/**
 * @brief Updates the parse-time accumulator and trips the breaker past the warn limit.
 */
void DataModel::FrameBuilder::parseBudgetAccount(BudgetClock::time_point startedAt)
{
  if (!m_parseBudgetEnabled) [[unlikely]]
    return;

  const auto elapsed =
    std::chrono::duration_cast<std::chrono::nanoseconds>(BudgetClock::now() - startedAt).count();
  m_parseBudgetUsedNs += elapsed;

  if (m_parseBudgetSkipping)
    return;

  const auto limitNs = static_cast<qint64>(kParseBudgetWarnLimitMs) * 1'000'000LL;
  if (m_parseBudgetUsedNs <= limitNs)
    return;

  m_parseBudgetSkipping = true;

  if (m_parseBudgetEpisodeActive)
    return;

  m_parseBudgetEpisodeActive = true;
  qWarning() << "[FrameBuilder] Parser load exceeded budget (" << m_parseBudgetUsedNs / 1'000'000LL
             << "ms /" << kParseBudgetWindowMs << "ms)"
             << "...dropping frames until parse load recovers.";

  if (!m_parseBudgetWarned) {
    m_parseBudgetWarned = true;
    Misc::Utilities::showMessageBox(
      QObject::tr("The frame parser is using more than %1% of CPU time.")
        .arg(100 * kParseBudgetWarnLimitMs / kParseBudgetWindowMs),
      QObject::tr("Serial Studio is dropping frames to keep the application responsive. "
                  "Please simplify or optimize the frame parser script to reduce its workload."),
      QMessageBox::Warning);
  }
}

/**
 * @brief Clears the parser-budget state -- called when the active project changes.
 */
void DataModel::FrameBuilder::parseBudgetReset() noexcept
{
  m_parseBudgetWindowStart   = BudgetClock::time_point{};
  m_parseBudgetUsedNs        = 0;
  m_parseBudgetSkipping      = false;
  m_parseBudgetWarned        = false;
  m_parseBudgetEpisodeActive = false;
}

/**
 * @brief Resolves the decoder method, optionally honoring a per-source override.
 */
SerialStudio::DecoderMethod DataModel::FrameBuilder::resolveDecoderMethod(
  int sourceId, bool applyPerSourceOverride) const
{
  static auto& project = DataModel::ProjectModel::instance();
  if (!applyPerSourceOverride)
    return project.decoderMethod();

  for (const auto& src : project.sources())
    if (src.sourceId == sourceId)
      return static_cast<SerialStudio::DecoderMethod>(src.decoderMethod);

  return project.decoderMethod();
}

/**
 * @brief Decodes raw captured bytes into one or more channel-string frames.
 */
void DataModel::FrameBuilder::decodeProjectChannels(int sourceId,
                                                    bool applyPerSourceOverride,
                                                    const IO::CapturedDataPtr& data,
                                                    QList<QStringList>& outChannels)
{
  if (m_playerOpen) [[unlikely]] {
    DataModel::splitReplayChannels(data->data, outChannels);
    return;
  }

  static auto& parser = DataModel::FrameParser::instance();
  decodeAndParseFrame(data->data,
                      resolveDecoderMethod(sourceId, applyPerSourceOverride),
                      parser,
                      sourceId,
                      outChannels);
}

/**
 * @brief Returns (and lazily creates) the per-source frame seeded from the project template.
 */
DataModel::Frame& DataModel::FrameBuilder::ensureSourceFrame(int sourceId)
{
  auto it = m_sourceFrames.find(sourceId);
  if (it != m_sourceFrames.end()) [[likely]]
    return it.value();

  DataModel::Frame newFrame;
  newFrame.sourceId                   = sourceId;
  newFrame.title                      = m_frame.title;
  newFrame.actions                    = m_frame.actions;
  newFrame.containsCommercialFeatures = m_frame.containsCommercialFeatures;
  for (const auto& g : m_frame.groups)
    if (g.sourceId == sourceId)
      newFrame.groups.push_back(g);

  it = m_sourceFrames.insert(sourceId, std::move(newFrame));
  return it.value();
}

/**
 * @brief Updates a single dataset from its channel and any registered transform.
 */
void DataModel::FrameBuilder::applyDatasetValue(Dataset& dataset,
                                                const QString* channelData,
                                                int channelCount,
                                                const TransformFrameInfo& info,
                                                const std::unordered_map<int, int>* replayColumns,
                                                bool finalValueReplay)
{
  DatasetDeps* dep = nullptr;
  if (m_changeDriven && dataset.virtual_ && !dataset.transformCode.isEmpty() && !finalValueReplay) {
    dep = &m_datasetDeps[dataset.uniqueId];
    if (!dep->readSlots.empty() && !m_tableStore.changedSince(dep->readSlots, dep->lastRunClock))
      return;
  }

  if (replayColumns) [[unlikely]] {
    const auto it = replayColumns->find(dataset.uniqueId);
    const int col = (it != replayColumns->end()) ? it->second : -1;
    if (col >= 0 && col < channelCount) {
      dataset.value        = channelData[col];
      dataset.numericValue = SerialStudio::toDouble(dataset.value, &dataset.isNumeric);
    } else {
      dataset.numericValue = 0.0;
      dataset.value.clear();
      dataset.isNumeric = true;
    }
  } else if (dataset.virtual_) {
    dataset.numericValue = 0.0;
    dataset.value.clear();
    dataset.isNumeric = true;
  } else {
    const int idx = dataset.index;
    if (idx <= 0 || idx > channelCount) [[unlikely]]
      return;

    dataset.value        = channelData[idx - 1];
    dataset.numericValue = SerialStudio::toDouble(dataset.value, &dataset.isNumeric);
  }

  dataset.rawNumericValue = dataset.numericValue;
  dataset.rawValue        = dataset.value;

  if (m_captureDatasetValues)
    m_tableStore.setDatasetRaw(
      dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);

  if (!dataset.transformCode.isEmpty() && !finalValueReplay) [[unlikely]] {
    const auto input = dataset.isNumeric ? QVariant(dataset.numericValue) : QVariant(dataset.value);
    if (dep)
      m_tableStore.setReadCaptureTarget(&dep->readSlots);

    const auto result = applyTransform(dataset.transformLanguage, dataset.uniqueId, input, info);

    if (dep) {
      m_tableStore.setReadCaptureTarget(nullptr);
      dep->lastRunClock = m_tableStore.writeClock();
    }

    if (result.typeId() == QMetaType::Double) {
      dataset.numericValue = SerialStudio::toDouble(result);
      dataset.value        = QString::number(dataset.numericValue, 'g', 15);
      dataset.isNumeric    = true;
    } else {
      dataset.value     = result.toString();
      dataset.isNumeric = false;
    }
  }

  if (!dataset.isNumeric)
    dataset.numericValue = (dataset.wgtMax > dataset.wgtMin) ? dataset.wgtMin : 0.0;

  if (m_captureDatasetValues)
    m_tableStore.setDatasetFinal(
      dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);
}

/**
 * @brief Span twin of applyDatasetValue: in-place writes keep the producer allocation-free. The
 *        span lane never runs during playback, so unlike its twin it needs no final-value player
 *        check before applying the transform.
 */
SS_HOT void DataModel::FrameBuilder::applyDatasetValueSpan(Dataset& dataset,
                                                           const QByteArrayView* spans,
                                                           qsizetype count,
                                                           const TransformFrameInfo& info)
{
  SS_ASSERT(spans != nullptr, return);
  SS_ASSUME(count > 0);

  DatasetDeps* dep = nullptr;
  if (m_changeDriven && dataset.virtual_ && !dataset.transformCode.isEmpty()) {
    dep = &m_datasetDeps[dataset.uniqueId];
    if (!dep->readSlots.empty() && !m_tableStore.changedSince(dep->readSlots, dep->lastRunClock))
      return;
  }

  if (dataset.virtual_) {
    dataset.numericValue = 0.0;
    dataset.value.clear();
    dataset.isNumeric = true;
  } else {
    const int idx = dataset.index;
    if (idx <= 0 || idx > count) [[unlikely]]
      return;

    // code-verify off
    // Restates the guard above; never assume idx range before the bounds check on a parsed frame.
    SS_ASSUME(idx >= 1 && idx <= count);
    // code-verify on

    const QByteArrayView token = spans[idx - 1];
    DataModel::assign_utf8_in_place(dataset.value, token);
    dataset.numericValue = SerialStudio::toDouble(token, &dataset.isNumeric);
  }

  dataset.rawNumericValue = dataset.numericValue;
  DataModel::assign_string_in_place(dataset.rawValue, dataset.value);

  if (m_captureDatasetValues)
    m_tableStore.setDatasetRaw(
      dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);

  if (!dataset.transformCode.isEmpty()) [[unlikely]] {
    const auto input = dataset.isNumeric ? QVariant(dataset.numericValue) : QVariant(dataset.value);
    if (dep)
      m_tableStore.setReadCaptureTarget(&dep->readSlots);

    const auto result = applyTransform(dataset.transformLanguage, dataset.uniqueId, input, info);

    if (dep) {
      m_tableStore.setReadCaptureTarget(nullptr);
      dep->lastRunClock = m_tableStore.writeClock();
    }

    if (result.typeId() == QMetaType::Double) {
      dataset.numericValue = SerialStudio::toDouble(result);
      dataset.value        = QString::number(dataset.numericValue, 'g', 15);
      dataset.isNumeric    = true;
    } else {
      dataset.value     = result.toString();
      dataset.isNumeric = false;
    }
  }

  if (!dataset.isNumeric)
    dataset.numericValue = (dataset.wgtMax > dataset.wgtMin) ? dataset.wgtMin : 0.0;

  if (m_captureDatasetValues)
    m_tableStore.setDatasetFinal(
      dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);
}

/**
 * @brief Shared per-frame scaffolding (capture flag, engine cache, watchdog arm, storage pin);
 *        returns true when the JS watchdog was armed (forwarded to endDatasetPass).
 */
bool DataModel::FrameBuilder::beginDatasetPass(const TransformFrameInfo& info)
{
  static auto& parser = DataModel::FrameParser::instance();
  const int epoch     = parser.engineEpoch();
  if (m_captureFlagsDirty || epoch != m_seenEngineEpoch) [[unlikely]] {
    m_seenEngineEpoch = epoch;
    refreshDatasetCaptureFlag();
  }

  if (info.sourceId != m_engineCacheSourceId) [[unlikely]] {
    m_engineCacheSourceId = info.sourceId;
    auto luaIt            = m_transformEngines.find({info.sourceId, SerialStudio::Lua});
    auto jsIt             = m_transformEngines.find({info.sourceId, SerialStudio::JavaScript});
    m_luaEngineForSource  = (luaIt != m_transformEngines.end()) ? &luaIt->second : nullptr;
    m_jsEngineForSource   = (jsIt != m_transformEngines.end()) ? &jsIt->second : nullptr;
  }

  const bool armJsWatchdog =
    (m_jsEngineForSource != nullptr) && (m_jsEngineForSource->jsWatchdog != nullptr);
  SS_ASSERT_LOG(m_jsEngineForSource == nullptr || m_jsEngineForSource->jsWatchdog);

  if (armJsWatchdog) [[unlikely]] {
    m_jsTransformTimedOut = false;
    m_jsEngineForSource->jsWatchdog->arm();
  }

  ++m_compileGuard;
  return armJsWatchdog;
}

/**
 * @brief Releases the dataset-pass scaffolding: unpins engine storage, disarms the watchdog and
 *        drains any project mutation a transform queued while hot pointers were live.
 */
void DataModel::FrameBuilder::endDatasetPass(bool armedJsWatchdog)
{
  SS_ASSERT(m_compileGuard > 0, m_compileGuard = 1);

  --m_compileGuard;

  if (armedJsWatchdog) [[unlikely]] {
    m_jsEngineForSource->jsWatchdog->disarm();
    if (m_jsTransformTimedOut) {
      static auto& nc = NotificationCenter::instance();
      nc.postWarning(
        QStringLiteral("FrameBuilder"),
        tr("JavaScript transform exceeded budget"),
        tr("A dataset transform took longer than %1 ms; remaining datasets in the frame fell "
           "back to raw values until the next frame. Profile or simplify the transform code.")
          .arg(kTransformWatchdogMs));
    }
  }

  if (m_compileGuard == 0 && m_compilePending) [[unlikely]] {
    m_compilePending = false;
    QMetaObject::invokeMethod(this, [this] { compileTransforms(); }, Qt::QueuedConnection);
  }
}

/**
 * @brief Recomputes whether per-dataset values must be mirrored into the table store: only
 *        scripts (transforms, Lua parsers, externally-injected engines) can read them back,
 *        and none of them run while a final-value player replays -- capture stays off then.
 */
void DataModel::FrameBuilder::refreshDatasetCaptureFlag()
{
  static auto& parser = DataModel::FrameParser::instance();
  m_captureDatasetValues =
    !m_playerOpen && m_tableStore.isInitialized()
    && (!m_transformEngines.empty() || m_externalTableApiUsers || parser.hasTableApiEngines());
  static auto& projectModel = DataModel::ProjectModel::instance();
  m_changeDriven            = projectModel.changeDrivenTransforms();
  m_datasetDeps.clear();
  m_captureFlagsDirty = false;
}

/**
 * @brief Transform-only dataset pass for reprocessFrames(): re-applies every transform from
 *        the dataset's retained raw value instead of fresh channels, so table-driven (virtual)
 *        datasets pick up the current store contents without a device frame. Returns true when
 *        any dataset value changed, so republishFrames() can skip sources with nothing new.
 */
bool DataModel::FrameBuilder::reprocessDatasetValues(DataModel::Frame& frame)
{
  SS_ASSERT(m_operationMode == SerialStudio::ProjectFile, return false);
  SS_ASSERT(!frame.groups.empty(), return false);

  TransformFrameInfo info;
  info.sourceId    = frame.sourceId;
  info.timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now().time_since_epoch())
                       .count();
  if (!m_transformEngines.empty())
    info.frameNumber = ++m_sourceFrameCounters[frame.sourceId];

  const bool armedWatchdog = beginDatasetPass(info);

  bool changed = false;
  for (auto& group : frame.groups) {
    for (auto& dataset : group.datasets) {
      if (dataset.transformCode.isEmpty())
        continue;

      QVariant input(0.0);
      if (!dataset.virtual_) {
        bool numeric     = false;
        const double raw = SerialStudio::toDouble(dataset.rawValue, &numeric);
        input            = numeric ? QVariant(raw) : QVariant(dataset.rawValue);
      }

      const double prev_numeric  = dataset.numericValue;
      const bool prev_is_numeric = dataset.isNumeric;
      const QString prev_value   = dataset.value;

      const auto result = applyTransform(dataset.transformLanguage, dataset.uniqueId, input, info);
      if (result.typeId() == QMetaType::Double) {
        dataset.numericValue = SerialStudio::toDouble(result);
        dataset.value        = QString::number(dataset.numericValue, 'g', 15);
        dataset.isNumeric    = true;
      } else {
        dataset.value     = result.toString();
        dataset.isNumeric = false;
      }

      if (!dataset.isNumeric)
        dataset.numericValue = (dataset.wgtMax > dataset.wgtMin) ? dataset.wgtMin : 0.0;

      changed = changed || dataset.isNumeric != prev_is_numeric
             || dataset.numericValue != prev_numeric || dataset.value != prev_value;

      if (m_captureDatasetValues)
        m_tableStore.setDatasetFinal(
          dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);
    }
  }

  endDatasetPass(armedWatchdog);
  return changed;
}

/**
 * @brief Writes channel values + transforms into every dataset of @p frame.
 */
void DataModel::FrameBuilder::applyDatasetValues(DataModel::Frame& frame,
                                                 const QStringList& channels,
                                                 const TransformFrameInfo& info)
{
  const auto* channelData = channels.data();
  const int channelCount  = channels.size();

  const bool finalValueReplay                       = SerialStudio::isFinalValuePlayerOpen();
  const std::unordered_map<int, int>* replayColumns = nullptr;
  if (finalValueReplay) [[unlikely]] {
    const auto it = m_replayColumnMap.find(info.sourceId);
    if (it != m_replayColumnMap.end())
      replayColumns = &it->second;
  }

  const bool armedWatchdog = beginDatasetPass(info);

  for (auto& group : frame.groups) {
    SS_NO_UNROLL
    for (auto& dataset : group.datasets)
      applyDatasetValue(dataset, channelData, channelCount, info, replayColumns, finalValueReplay);
  }

  endDatasetPass(armedWatchdog);
}

/**
 * @brief Span twin of applyDatasetValues: writes tokenized byte views into every dataset.
 */
void DataModel::FrameBuilder::applyDatasetValuesSpans(DataModel::Frame& frame,
                                                      const QByteArrayView* spans,
                                                      qsizetype count,
                                                      const TransformFrameInfo& info)
{
  SS_ASSERT(spans != nullptr, return);
  SS_ASSERT(count > 0, return);

  const bool armedWatchdog = beginDatasetPass(info);

  for (auto& group : frame.groups) {
    SS_NO_UNROLL
    for (auto& dataset : group.datasets)
      applyDatasetValueSpan(dataset, spans, count, info);
  }

  endDatasetPass(armedWatchdog);
}

/**
 * @brief Flat-table span apply: the slot's pre-resolved dataset pointers make the walk
 *        pointer-only, with no per-frame group/dataset container traversal.
 */
SS_HOT void DataModel::FrameBuilder::applyDatasetValuesSpans(
  DataModel::Dataset* const* SS_RESTRICT datasets,
  qsizetype datasetCount,
  const QByteArrayView* SS_RESTRICT spans,
  qsizetype count,
  const TransformFrameInfo& info)
{
  SS_ASSERT(datasets != nullptr, return);
  SS_ASSERT(spans != nullptr, return);

  const bool armedWatchdog = beginDatasetPass(info);

  SS_NO_UNROLL
  for (qsizetype i = 0; i < datasetCount; ++i)
    applyDatasetValueSpan(*datasets[i], spans, count, info);

  endDatasetPass(armedWatchdog);
}

/**
 * @brief Parses and updates the Quick Plot frame with incoming CSV values.
 */
void DataModel::FrameBuilder::parseQuickPlotFrame(const IO::CapturedDataPtr& data)
{
  SS_ASSERT(data, return);
  SS_ASSERT(!data->data.isEmpty(), return);
  SS_ASSERT(m_operationMode == SerialStudio::QuickPlot, return);

  QList<QStringList> splitRows;
  if (m_playerOpen) [[unlikely]]
    DataModel::splitReplayChannels(data->data, splitRows);
  else
    DataModel::splitQuickPlotChannels(data->data, splitRows);

  auto& channels = m_channelScratch;
  channels.clear();
  if (!splitRows.isEmpty())
    channels = splitRows.first();

  const int channelCount = channels.size();
  if (channelCount <= 0)
    return;

  if (m_captureLatestFrame) [[unlikely]]
    captureLatestChannels(0, channels);

  if (m_quickPlotChannels == -1) {
    bool allNonNumeric = true;
    for (const auto& channel : std::as_const(channels)) {
      bool isNumeric = false;
      (void)SerialStudio::toDouble(channel, &isNumeric);
      if (!isNumeric)
        continue;

      allNonNumeric = false;
      break;
    }

    if (allNonNumeric) {
      m_quickPlotHasHeader    = true;
      m_quickPlotChannelNames = channels;
      return;
    }
  }

  if (channelCount != m_quickPlotChannels) [[unlikely]] {
    buildQuickPlotFrame(channels);
    m_quickPlotChannels = channelCount;
  }

  const auto* channelData = channels.constData();
  const size_t groupCount = m_quickPlotFrame.groups.size();
  for (size_t g = 0; g < groupCount; ++g) {
    auto& group               = m_quickPlotFrame.groups[g];
    const size_t datasetCount = group.datasets.size();
    for (size_t d = 0; d < datasetCount; ++d) {
      auto& dataset = group.datasets[d];
      const int idx = dataset.index;
      if (idx > 0 && idx <= channelCount) [[likely]] {
        dataset.value           = channelData[idx - 1];
        dataset.numericValue    = SerialStudio::toDouble(dataset.value, &dataset.isNumeric);
        dataset.rawValue        = dataset.value;
        dataset.rawNumericValue = dataset.numericValue;
      }
    }
  }

  hotpathTxFrame(acquireFrame(m_quickPlotFrame, data->timestamp));
}

//--------------------------------------------------------------------------------------------------
// Quick-plot project generation functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the synthetic source row that anchors a QuickPlot frame. Always returns a
 *        non-null title so downstream exporters bound to NOT NULL columns don't reject the row.
 */
DataModel::Source DataModel::FrameBuilder::makeQuickPlotSource() const
{
  static auto& ioManager = IO::ConnectionManager::instance();

  DataModel::Source src;
  src.sourceId = 0;
  src.title    = tr("Device A");
  src.busType  = static_cast<int>(ioManager.busType());
  return src;
}

/**
 * @brief Rebuilds the Quick Plot frame structure when the channel count changes.
 */
void DataModel::FrameBuilder::buildQuickPlotFrame(const QStringList& channels)
{
  SS_ASSERT(!channels.isEmpty(), return);
  SS_ASSERT(m_operationMode == SerialStudio::QuickPlot, return);

  invalidateFramePool();

#ifdef BUILD_COMMERCIAL
  static auto& ioManager = IO::ConnectionManager::instance();
  const auto busType     = ioManager.busType();
  if (busType == SerialStudio::BusType::Audio) {
    buildQuickPlotAudioFrame(channels);
    return;
  }
#endif

  int idx = 1;
  std::vector<DataModel::Dataset> datasets;
  datasets.reserve(channels.count());
  for (const auto& channel : std::as_const(channels)) {
    DataModel::Dataset dataset;
    dataset.groupId   = 0;
    dataset.datasetId = idx - 1;
    dataset.uniqueId  = dataset_unique_id(0, 0, idx - 1);
    dataset.index     = idx;
    dataset.plt       = false;
    dataset.value     = channel;

    if (m_quickPlotHasHeader && idx > 0
        && idx - 1 < static_cast<int>(m_quickPlotChannelNames.size()))
      dataset.title = m_quickPlotChannelNames[idx - 1];
    else
      dataset.title = tr("Channel %1").arg(idx);

    dataset.numericValue = SerialStudio::toDouble(dataset.value, &dataset.isNumeric);
    datasets.push_back(dataset);

    ++idx;
  }

  clear_frame(m_quickPlotFrame);
  m_quickPlotFrame.title = tr("Quick Plot");
  m_quickPlotFrame.sources.push_back(makeQuickPlotSource());

  DataModel::Group datagrid;
  datagrid.groupId  = 0;
  datagrid.uniqueId = runtime_group_unique_id(0);
  datagrid.datasets = datasets;
  datagrid.title    = tr("Quick Plot Data");
  datagrid.widget   = QStringLiteral("datagrid");
  for (size_t i = 0; i < datagrid.datasets.size(); ++i)
    datagrid.datasets[i].plt = true;

  m_quickPlotFrame.groups.push_back(datagrid);

  if (datasets.size() > 1) {
    DataModel::Group multiplot;
    multiplot.groupId  = 1;
    multiplot.uniqueId = runtime_group_unique_id(1);
    multiplot.datasets = datasets;
    multiplot.title    = tr("Multiple Plots");
    multiplot.widget   = QStringLiteral("multiplot");
    for (size_t i = 0; i < multiplot.datasets.size(); ++i) {
      multiplot.datasets[i].groupId  = 1;
      multiplot.datasets[i].uniqueId = dataset_unique_id(0, 1, static_cast<int>(i));
    }

    m_quickPlotFrame.groups.push_back(multiplot);
  }

  finalize_frame(m_quickPlotFrame);
}

/**
 * @brief Builds an audio-specific Quick Plot frame with FFT configuration.
 */
void DataModel::FrameBuilder::buildQuickPlotAudioFrame(const QStringList& channels)
{
  SS_ASSERT(!channels.isEmpty(), return);
  SS_ASSERT(m_operationMode == SerialStudio::QuickPlot, return);

#ifdef BUILD_COMMERCIAL
  static auto& ioManager = IO::ConnectionManager::instance();
  const auto* audioPtr   = ioManager.audio();
  if (!audioPtr)
    return;

  const auto& audio     = *audioPtr;
  const auto format     = audio.config().capture.format;
  const auto sampleRate = audio.config().sampleRate;

  double maxValue = 1.0;
  double minValue = 0.0;
  switch (format) {
    case ma_format_u8:
      maxValue = 255;
      minValue = 0;
      break;
    case ma_format_s16:
      maxValue = 32767;
      minValue = -32768;
      break;
    case ma_format_s24:
      maxValue = 8388607;
      minValue = -8388608;
      break;
    case ma_format_s32:
      maxValue = 2147483647;
      minValue = -2147483648;
      break;
    case ma_format_f32:
      maxValue = 1.0;
      minValue = -1.0;
      break;
    default:
      break;
  }

  const int targetSamples = static_cast<int>(sampleRate * 0.05);
  int fftSamples          = 256;
  while (fftSamples < targetSamples && fftSamples < 8192)
    fftSamples *= 2;

  const bool multipleChannels = channels.count() > 1;
  int index                   = 1;
  std::vector<DataModel::Dataset> datasets;
  datasets.reserve(channels.count());
  for (const auto& channel : std::as_const(channels)) {
    DataModel::Dataset dataset;
    dataset.fft                  = true;
    dataset.plt                  = !multipleChannels;
    dataset.groupId              = 0;
    dataset.datasetId            = index - 1;
    dataset.uniqueId             = dataset_unique_id(0, 0, index - 1);
    dataset.index                = index;
    dataset.value                = channel;
    dataset.pltMax               = maxValue;
    dataset.pltMin               = minValue;
    dataset.fftMax               = maxValue;
    dataset.fftMin               = minValue;
    dataset.fftSamples           = fftSamples;
    dataset.fftSamplingRate      = sampleRate;
    dataset.fftLogX              = true;
    dataset.fftBallistics        = true;
    dataset.fftBallisticsRelease = 100;

    if (m_quickPlotHasHeader && index > 0
        && index - 1 < static_cast<int>(m_quickPlotChannelNames.size()))
      dataset.title = m_quickPlotChannelNames[index - 1];
    else
      dataset.title = tr("Channel %1").arg(index);

    dataset.numericValue = SerialStudio::toDouble(dataset.value, &dataset.isNumeric);
    datasets.push_back(dataset);
    ++index;
  }

  DataModel::Group group;
  group.groupId  = 0;
  group.uniqueId = runtime_group_unique_id(0);
  group.datasets = datasets;
  group.title    = tr("Audio Input");
  if (multipleChannels)
    group.widget = QStringLiteral("multiplot");

  clear_frame(m_quickPlotFrame);
  m_quickPlotFrame.title = tr("Quick Plot");
  m_quickPlotFrame.sources.push_back(makeQuickPlotSource());
  m_quickPlotFrame.groups.push_back(group);
  finalize_frame(m_quickPlotFrame);
#else
  Q_UNUSED(channels);
#endif
}

//--------------------------------------------------------------------------------------------------
// Hotpath data publishing functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishes a fully constructed DataModel frame to all registered output modules. The
 *        dashboard draws the pooled slot directly; async sinks (gated on the cached m_anyAsyncSink
 *        flag) get one detached copy so a slow-export backlog can never pin the pool.
 */
void DataModel::FrameBuilder::hotpathTxFrame(const DataModel::TimestampedFramePtr& frame)
{
  SS_ASSERT(frame, return);
  SS_ASSERT(!frame->data.groups.empty(), return);
  SS_ASSERT(!frame->data.title.isEmpty(), return);

  static auto& dashboard = UI::Dashboard::instance();
  dashboard.hotpathRxFrame(frame);

  if (!m_anyAsyncSink)
    return;

  static auto& csvExport     = CSV::Export::instance();
  static auto& mdf4Export    = MDF4::Export::instance();
  static auto& pluginsServer = API::Server::instance();
#ifdef BUILD_COMMERCIAL
  static auto& sqliteExport  = Sessions::Export::instance();
  static auto& mqttPublisher = MQTT::Publisher::instance();
#endif
#ifdef ENABLE_GRPC
  static auto& grpcServer = API::GRPC::GRPCServer::instance();
#endif

  const auto detached =
    std::make_shared<DataModel::TimestampedFrame>(frame->data, frame->timestamp);

  csvExport.hotpathTxFrame(detached);
  mdf4Export.hotpathTxFrame(detached);
  pluginsServer.hotpathTxFrame(detached);
#ifdef BUILD_COMMERCIAL
  sqliteExport.hotpathTxFrame(detached);
  mqttPublisher.hotpathTxFrame(detached);
#endif
#ifdef ENABLE_GRPC
  grpcServer.hotpathTxFrame(detached);
#endif
}

/**
 * @brief Replay twin of hotpathTxFrame: the dashboard draws the pooled slot, and one detached
 *        copy goes to the read-only observers (API/gRPC, only with a client connected). The
 *        recording sinks are deliberately absent -- replay must never feed an exporter.
 */
void DataModel::FrameBuilder::publishReplayFrame(const DataModel::TimestampedFramePtr& frame)
{
  SS_ASSERT(frame, return);
  SS_ASSERT(m_playerOpen, return);
  SS_ASSERT(!frame->data.groups.empty(), return);

  static auto& dashboard = UI::Dashboard::instance();
  dashboard.hotpathRxFrame(frame);

  static auto& pluginsServer = API::Server::instance();
  bool observers             = pluginsServer.enabled() && pluginsServer.clientCount() > 0;
#ifdef ENABLE_GRPC
  static auto& grpcServer = API::GRPC::GRPCServer::instance();
  observers               = observers || (grpcServer.enabled() && grpcServer.clientCount() > 0);
#endif
  if (!observers)
    return;

  const auto detached =
    std::make_shared<DataModel::TimestampedFrame>(frame->data, frame->timestamp);
  if (pluginsServer.enabled() && pluginsServer.clientCount() > 0)
    pluginsServer.hotpathTxFrame(detached);
#ifdef ENABLE_GRPC
  if (grpcServer.enabled() && grpcServer.clientCount() > 0)
    grpcServer.hotpathTxFrame(detached);
#endif
}

//--------------------------------------------------------------------------------------------------
// Per-dataset value transforms
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the safe Lua libraries needed by transforms and strips dangerous globals, including
 *        string.dump whose bytecode serialization paired with a loader is a sandbox-escape vector.
 */
static void openSafeLibsForTransform(lua_State* L)
{
  static const luaL_Reg kSafeLibs[] = {
    {       "_G",      luaopen_base},
    {    "table",     luaopen_table},
    {   "string",    luaopen_string},
    {     "math",      luaopen_math},
    {     "utf8",      luaopen_utf8},
    {"coroutine", luaopen_coroutine},
    {    nullptr,           nullptr}
  };

  for (const luaL_Reg* lib = kSafeLibs; lib->func; ++lib) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);
  }

  for (const char* name : {"dofile", "loadfile", "load"}) {
    lua_pushnil(L);
    lua_setglobal(L, name);
  }

  lua_getglobal(L, "string");
  if (lua_istable(L, -1)) {
    lua_pushnil(L);
    lua_setfield(L, -2, "dump");
  }
  lua_pop(L, 1);
}

/**
 * @brief Lua LUA_MASKCOUNT hook that aborts runaway transforms via luaL_error() when the per-engine
 * deadline expires.
 */
void DataModel::FrameBuilder::transformLuaWatchdogHook(lua_State* L, lua_Debug* ar)
{
  Q_UNUSED(ar)

  lua_getfield(L, LUA_REGISTRYINDEX, "__ss_transform__");
  auto* engine = static_cast<TransformEngine*>(lua_touserdata(L, -1));
  lua_pop(L, 1);

  if (!engine) [[unlikely]]
    return;

  if (engine->luaDeadline.hasExpired()) [[unlikely]]
    luaL_error(L, "transform timed out after %d ms", kTransformWatchdogMs);
}

/**
 * @brief Reconciles transform engines with playback state: compileTransforms() keeps engines
 *        down while a player is open (replay never runs a transform, and a live engine arms
 *        the watchdog + dataset mirroring per frame), and the pass guard defers teardown
 *        when a script opens a player synchronously via apiCall mid-pass.
 */
void DataModel::FrameBuilder::rebuildTransformsForPlayback()
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile || m_frame.title.isEmpty())
    return;

  if (m_compileGuard > 0) [[unlikely]] {
    m_compilePending = true;
    return;
  }

  initializeTableStore();
  compileTransforms();
}

/**
 * @brief Installs the per-source uniqueId->column map a file player uses for final-value replay.
 */
void DataModel::FrameBuilder::setReplayColumnMap(
  std::unordered_map<int, std::unordered_map<int, int>> map)
{
  m_replayColumnMap = std::move(map);
}

/**
 * @brief Compiles per-dataset transforms into one shared Lua/JS engine per source, caching function
 * refs. Defers when a frame is in flight (m_compileGuard > 0): mutating m_transformEngines while a
 * dataset pass holds hot pointers into it would dangle them. No-op after aboutToQuit: rebuilding
 * a QJSEngine once QCoreApplication is gone is a qFatal at exit.
 */
void DataModel::FrameBuilder::compileTransforms()
{
  if (m_shuttingDown) [[unlikely]]
    return;

  if (m_compileGuard > 0) [[unlikely]] {
    m_compilePending = true;
    return;
  }

  destroyTransformEngines();
  SS_ASSERT_LOG(m_transformEngines.empty());

  if (m_playerOpen)
    return;

  std::map<EngineKey, std::vector<TransformEntry>> byKey;
  for (const auto& group : m_frame.groups) {
    for (const auto& ds : group.datasets) {
      if (ds.transformCode.isEmpty())
        continue;

      byKey[{ds.sourceId, ds.transformLanguage}].push_back({ds.uniqueId, ds.transformCode});
    }
  }

  if (byKey.empty())
    return;

  for (auto& [key, entries] : byKey) {
    auto [it, inserted] = m_transformEngines.emplace(key, TransformEngine{});
    SS_ASSERT_LOG(inserted);
    if (!inserted) [[unlikely]]
      continue;

    TransformEngine& engine = it->second;

    if (key.language == SerialStudio::Lua)
      compileTransformsLua(engine, key.sourceId, entries);
    else
      compileTransformsJS(engine, key.sourceId, entries);

    if (!engine.luaState && !engine.jsEngine)
      m_transformEngines.erase(it);
  }
}

/**
 * @brief Compiles per-dataset Lua transforms into a shared lua_State, caching refs for O(1) hotpath
 * lookup.
 */
void DataModel::FrameBuilder::compileTransformsLua(TransformEngine& engine,
                                                   int sourceId,
                                                   const std::vector<TransformEntry>& entries)
{
  lua_State* L = luaL_newstate();
  if (!L) [[unlikely]]
    return;

  lua_atpanic(L, [](lua_State* state) -> int {
    const char* msg = lua_tostring(state, -1);
    qWarning() << "[FrameBuilder] Lua transform panic:" << (msg ? msg : "<unknown>");
    throw std::runtime_error(msg ? msg : "lua transform panic");
  });

  openSafeLibsForTransform(L);

  DataModel::installLuaConsole(L);

  DataModel::installLuaCompat(L);

  injectTableApiLua(L);

  DataModel::DeviceWriteApi::installLua(L, sourceId);

  DataModel::ActionFireApi::installLua(L);

  DataModel::DashboardApi::installLua(L);

  DataModel::ScriptApiCall::installLua(L, sourceId);

  DataModel::NotificationCenter::installScriptApi(L);

  lua_pushlightuserdata(L, &engine);
  lua_setfield(L, LUA_REGISTRYINDEX, "__ss_transform__");

  lua_sethook(L, &FrameBuilder::transformLuaWatchdogHook, LUA_MASKCOUNT, kTransformHookInstrCount);

  engine.luaDeadline.setRemainingTime(kTransformWatchdogMs);

  for (const auto& entry : entries)
    compileTransformsLuaEntry(L, engine, entry);

  engine.luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);
  engine.luaState    = L;
}

/**
 * @brief Compiles a single Lua dataset transform; logs and skips on any error.
 */
void DataModel::FrameBuilder::compileTransformsLuaEntry(lua_State* L,
                                                        TransformEngine& engine,
                                                        const TransformEntry& entry)
{
  const int baseTop = lua_gettop(L);

  try {
    lua_newtable(L);
    lua_createtable(L, 0, 1);
    lua_pushglobaltable(L);
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);

    const QByteArray utf8 = entry.code.toUtf8();
    const QByteArray chunkName =
      QByteArray("=transform[") + QByteArray::number(entry.uniqueId) + "]";
    if (luaL_loadbufferx(L, utf8.constData(), utf8.size(), chunkName.constData(), "t") != LUA_OK) {
      qWarning() << "[FrameBuilder] Transform compile error for dataset" << entry.uniqueId << ":"
                 << lua_tostring(L, -1);
      lua_settop(L, baseTop);
      return;
    }

    lua_pushvalue(L, -2);
    lua_setupvalue(L, -2, 1);

    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
      qWarning() << "[FrameBuilder] Transform runtime error for dataset" << entry.uniqueId << ":"
                 << lua_tostring(L, -1);
      lua_settop(L, baseTop);
      return;
    }

    lua_getfield(L, -1, "transform");
    if (!lua_isfunction(L, -1)) {
      qWarning() << "[FrameBuilder] Dataset" << entry.uniqueId
                 << "transform code does not define transform()";
      lua_settop(L, baseTop);
      return;
    }

    bool acceptsInfo = false;
    lua_pushvalue(L, -1);
    lua_Debug ar;
    if (lua_getinfo(L, ">u", &ar) != 0) [[likely]]
      acceptsInfo = (ar.nparams >= 2);

    auto existingIt = engine.luaRefs.find(entry.uniqueId);
    if (existingIt != engine.luaRefs.end()) [[unlikely]]
      luaL_unref(L, LUA_REGISTRYINDEX, existingIt->second.ref);

    engine.luaRefs[entry.uniqueId] = LuaTransformRef{luaL_ref(L, LUA_REGISTRYINDEX), acceptsInfo};

    lua_pop(L, 1);
    SS_ASSERT(lua_gettop(L) == baseTop, lua_settop(L, baseTop));
  } catch (const std::exception& e) {
    qWarning() << "[FrameBuilder] Transform compile uncaught exception for dataset"
               << entry.uniqueId << ":" << e.what();
    lua_settop(L, baseTop);
  } catch (...) {
    qWarning() << "[FrameBuilder] Transform compile uncaught non-std exception for dataset"
               << entry.uniqueId;
    lua_settop(L, baseTop);
  }
}

/**
 * @brief Compiles per-dataset JavaScript transforms into a shared QJSEngine; code is IIFE-wrapped
 * for isolation.
 */
void DataModel::FrameBuilder::compileTransformsJS(TransformEngine& engine,
                                                  int sourceId,
                                                  const std::vector<TransformEntry>& entries)
{
  auto* js = new QJSEngine();

  DataModel::ScriptApiCall::installAll(js, sourceId);

  for (const auto& entry : entries) {
    const QString wrapped =
      QStringLiteral("(function() {%1\n"
                     ";return (typeof transform === 'function') ? transform : null;\n"
                     "})();")
        .arg(entry.code);

    auto evalResult = js->evaluate(wrapped);
    if (evalResult.isError()) {
      qWarning() << "[FrameBuilder] Transform compile error for"
                 << "dataset" << entry.uniqueId << "at line"
                 << evalResult.property("lineNumber").toInt() << ":"
                 << evalResult.property("message").toString();
      continue;
    }

    if (!evalResult.isCallable()) {
      qWarning() << "[FrameBuilder] Dataset" << entry.uniqueId
                 << "transform code does not define transform()";
      continue;
    }

    const bool acceptsInfo        = (evalResult.property(QStringLiteral("length")).toInt() >= 2);
    engine.jsRefs[entry.uniqueId] = JsTransformRef{evalResult, acceptsInfo};
  }

  engine.jsEngine = js;
  engine.jsWatchdog =
    std::make_unique<JsWatchdog>(js, kTransformWatchdogMs, QStringLiteral("transform"));
}

/**
 * @brief Runs one GC pass over every per-source transform engine.
 */
void DataModel::FrameBuilder::collectTransformEngineGarbage()
{
  if (m_transformEngines.empty())
    return;

  for (auto& [id, engine] : m_transformEngines) {
    if (engine.luaState)
      lua_gc(engine.luaState, LUA_GCCOLLECT);

    if (engine.jsEngine)
      engine.jsEngine->collectGarbage();
  }
}

/**
 * @brief Destroys all per-source transform engines and releases resources. The transform-error
 *        statistics reset with them, so a repaired transform stops being reported once the
 *        engines recompile.
 */
void DataModel::FrameBuilder::destroyTransformEngines()
{
  m_engineCacheSourceId = -1;
  m_luaEngineForSource  = nullptr;
  m_jsEngineForSource   = nullptr;
  m_captureFlagsDirty   = true;

  m_transformErrors              = 0;
  m_lastTransformDatasetUniqueId = -1;
  m_lastTransformError.clear();

  m_tableStore.clearLookupCache();

  for (auto& [id, engine] : m_transformEngines) {
    engine.jsRefs.clear();

    if (engine.luaState)
      for (const auto& [uid, ref] : engine.luaRefs)
        luaL_unref(engine.luaState, LUA_REGISTRYINDEX, ref.ref);

    engine.luaRefs.clear();

    if (engine.luaState) {
      lua_close(engine.luaState);
      engine.luaState = nullptr;
    }

    engine.jsWatchdog.reset();

    delete engine.jsEngine;
    engine.jsEngine = nullptr;
  }

  m_transformEngines.clear();
  SS_ASSERT_LOG(m_transformEngines.empty());
}

/**
 * @brief Counts a transform failure and retains its message only when the failing dataset differs
 *        from the one already recorded, so a dataset that throws on every frame stores the string
 *        once instead of allocating per frame.
 */
SS_COLD void DataModel::FrameBuilder::noteTransformError(int uniqueId, const char* message)
{
  ++m_transformErrors;
  if (m_lastTransformDatasetUniqueId == uniqueId)
    return;

  m_lastTransformDatasetUniqueId = uniqueId;
  m_lastTransformError           = QString::fromUtf8(message ? message : "");
}

/**
 * @brief Overload for the JavaScript branch, whose message string is already materialized.
 */
SS_COLD void DataModel::FrameBuilder::noteTransformError(int uniqueId, const QString& message)
{
  ++m_transformErrors;
  if (m_lastTransformDatasetUniqueId == uniqueId)
    return;

  m_lastTransformDatasetUniqueId = uniqueId;
  m_lastTransformError           = message;
}

/**
 * @brief Applies the pre-compiled transform for a dataset; returns @p rawValue on error or missing
 * transform.
 */
QVariant DataModel::FrameBuilder::applyTransform(int language,
                                                 int uniqueId,
                                                 const QVariant& rawValue,
                                                 const TransformFrameInfo& info)
{
  SS_ASSERT(info.sourceId >= 0, return rawValue);
  SS_ASSERT(uniqueId >= 0, return rawValue);
  SS_ASSERT(info.sourceId == m_engineCacheSourceId, return rawValue);

  TransformEngine* engine =
    (language == SerialStudio::Lua) ? m_luaEngineForSource : m_jsEngineForSource;
  if (!engine)
    return rawValue;

  if (engine->luaState)
    return applyTransformLua(*engine, uniqueId, rawValue, info);

  if (engine->jsEngine)
    return applyTransformJs(*engine, uniqueId, rawValue, info);

  return rawValue;
}

/**
 * @brief Calls the cached Lua transform function for @p uniqueId under the per-call deadline.
 */
QVariant DataModel::FrameBuilder::applyTransformLua(TransformEngine& engine,
                                                    int uniqueId,
                                                    const QVariant& rawValue,
                                                    const TransformFrameInfo& info)
{
  auto refIt = engine.luaRefs.find(uniqueId);
  if (refIt == engine.luaRefs.end())
    return rawValue;

  lua_State* L           = engine.luaState;
  const auto& transform  = refIt->second;
  const bool acceptsInfo = transform.acceptsInfo;
  engine.luaDeadline.setRemainingTime(kTransformWatchdogMs);

  try {
    lua_rawgeti(L, LUA_REGISTRYINDEX, transform.ref);
    if (rawValue.typeId() == QMetaType::Double) {
      lua_pushnumber(L, SerialStudio::toDouble(rawValue));
    } else {
      const auto utf8 = rawValue.toString().toUtf8();
      lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
    }

    int argCount = 1;
    if (acceptsInfo) {
      lua_createtable(L, 0, 3);
      lua_pushinteger(L, static_cast<lua_Integer>(info.frameNumber));
      lua_setfield(L, -2, "frameNumber");
      lua_pushinteger(L, info.sourceId);
      lua_setfield(L, -2, "sourceId");
      lua_pushinteger(L, static_cast<lua_Integer>(info.timestampMs));
      lua_setfield(L, -2, "timestampMs");
      argCount = 2;
    }

    int pcallStatus = LUA_ERRRUN;
    try {
      pcallStatus = lua_pcall(L, argCount, 1, 0);
    } catch (...) {
      qWarning() << "[FrameBuilder] Uncaught exception escaped lua_pcall in transform for"
                 << uniqueId;
      try {
        lua_settop(L, 0);
        lua_pushstring(L, "uncaught Lua exception (escaped lua_pcall)");
      } catch (...) {
      }
      pcallStatus = LUA_ERRRUN;
    }
    engine.luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);

    if (pcallStatus != LUA_OK) [[unlikely]] {
      qWarning() << "[FrameBuilder] Lua transform call failed for dataset" << uniqueId << ":"
                 << lua_tostring(L, -1);
      noteTransformError(uniqueId, lua_tostring(L, -1));
      lua_pop(L, 1);
      return rawValue;
    }

    if (lua_isnumber(L, -1)) {
      const double result = lua_tonumber(L, -1);
      lua_pop(L, 1);
      if (!std::isfinite(result)) [[unlikely]]
        return rawValue;

      return QVariant(result);
    }

    if (lua_isstring(L, -1)) {
      const QString result = QString::fromUtf8(lua_tostring(L, -1));
      lua_pop(L, 1);
      return QVariant(result);
    }

    lua_pop(L, 1);
    return rawValue;
  } catch (const std::exception& e) {
    qWarning() << "[FrameBuilder] applyTransformLua uncaught exception for" << uniqueId << ":"
               << e.what();
  } catch (...) {
    qWarning() << "[FrameBuilder] applyTransformLua uncaught non-std exception for" << uniqueId;
  }

  engine.luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);
  lua_settop(L, 0);
  return rawValue;
}

/**
 * @brief Calls the cached JS transform function for @p uniqueId under the watchdog timer, which is
 *        armed once per frame in beginDatasetPass rather than per call (unlike the Lua deadline).
 */
QVariant DataModel::FrameBuilder::applyTransformJs(TransformEngine& engine,
                                                   int uniqueId,
                                                   const QVariant& rawValue,
                                                   const TransformFrameInfo& info)
{
  auto refIt = engine.jsRefs.find(uniqueId);
  if (refIt == engine.jsRefs.end())
    return rawValue;

  QJSValueList args;
  if (rawValue.typeId() == QMetaType::Double)
    args << QJSValue(SerialStudio::toDouble(rawValue));
  else
    args << QJSValue(rawValue.toString());

  if (refIt->second.acceptsInfo) {
    QJSValue jsInfo = engine.jsEngine->newObject();
    jsInfo.setProperty(QStringLiteral("frameNumber"),
                       QJSValue(static_cast<double>(info.frameNumber)));
    jsInfo.setProperty(QStringLiteral("sourceId"), QJSValue(info.sourceId));
    jsInfo.setProperty(QStringLiteral("timestampMs"),
                       QJSValue(static_cast<double>(info.timestampMs)));
    args << jsInfo;
  }

  auto result = refIt->second.fn.call(args);

  if (engine.jsEngine->isInterrupted()) [[unlikely]] {
    engine.jsEngine->setInterrupted(false);
    m_jsTransformTimedOut = true;
    qWarning() << "[FrameBuilder] JS transform for dataset" << uniqueId << "timed out after"
               << kTransformWatchdogMs << "ms";
    noteTransformError(uniqueId, "transform timed out");
    return rawValue;
  }

  if (result.isNumber()) {
    const double val = result.toNumber();
    if (!std::isfinite(val)) [[unlikely]]
      return rawValue;

    return QVariant(val);
  }

  if (result.isString())
    return QVariant(result.toString());

  if (result.isError()) [[unlikely]] {
    const auto message = result.toString();
    qWarning() << "[FrameBuilder] JS transform call failed for dataset" << uniqueId << ":"
               << message;
    noteTransformError(uniqueId, message);
  }

  return rawValue;
}

//--------------------------------------------------------------------------------------------------
// Data table store initialization and transform API injection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the DataTableStore from the project model and current frame. Must run
 *        BEFORE scripts (re)load: evaluation resolves table handles (top level or the load-time
 *        parse() probe), and a later rebuild would bump the generation and stale them all.
 */
void DataModel::FrameBuilder::initializeTableStore()
{
  static auto& pm = DataModel::ProjectModel::instance();
  m_tableStore.initialize(pm.tables(), pm.editorTableFolders(), m_frame);
  m_captureFlagsDirty = true;
}

/**
 * @brief Re-initializes the DataTableStore from the project model's in-flight edits. Preview and
 *        editor paths call this; while a connection or player session is live it is a no-op, since
 *        a rebuild would stale script handles and reset live values just to serve a preview.
 *        Definition edits reach a live runtime through the epoch-apply and autosave rebuilds.
 */
void DataModel::FrameBuilder::refreshTableStoreFromProjectModel()
{
  static auto& ioManager = IO::ConnectionManager::instance();

  const bool session_live = ioManager.isConnected() || SerialStudio::isAnyPlayerOpen();
  if (m_tableStore.isInitialized() && session_live)
    return;

  static auto& pm = DataModel::ProjectModel::instance();
  DataModel::Frame scratch;
  scratch.title  = pm.title();
  scratch.groups = pm.groups();
  m_tableStore.initialize(pm.tables(), pm.editorTableFolders(), scratch);
  m_captureFlagsDirty = true;
}

/**
 * @brief Lua C closure for tableGet(table, reg).
 */
static int luaTableGet(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  SS_ASSERT(store, {
    lua_pushnil(L);
    return 1;
  });

  const char* table = luaL_checkstring(L, 1);
  const char* reg   = luaL_checkstring(L, 2);

  const auto* val = store->getByInternedKey(table, reg);
  if (!val) {
    lua_pushnil(L);
    return 1;
  }

  if (val->isNumeric) {
    lua_pushnumber(L, val->numericValue);
  } else {
    const auto utf8 = val->stringValue.toUtf8();
    lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
  }

  return 1;
}

/**
 * @brief Lua C closure for tableSet(table, reg, value). Cache-aware like tableGet. A nil value
 *        (e.g. a failed tonumber()) is a safe no-op for parity with JS, which never raises here.
 */
static int luaTableSet(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  SS_ASSERT(store, return 0);

  const char* table = luaL_checkstring(L, 1);
  const char* reg   = luaL_checkstring(L, 2);

  if (lua_isnoneornil(L, 3))
    return 0;

  DataModel::RegisterValue rv;
  if (lua_isnumber(L, 3)) {
    rv.numericValue = lua_tonumber(L, 3);
    rv.isNumeric    = true;
  } else {
    rv.stringValue = QString::fromUtf8(luaL_checkstring(L, 3));
    rv.isNumeric   = false;
  }

  store->setByInternedKey(table, reg, rv);
  return 0;
}

/**
 * @brief Lua C closure for tableHandle(table, reg) -> handle; resolve once, off the hot path.
 */
static int luaTableHandle(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  SS_ASSERT(store, {
    lua_pushnil(L);
    return 1;
  });

  const char* table   = luaL_checkstring(L, 1);
  const char* reg     = luaL_checkstring(L, 2);
  const qint64 handle = store->handleOf(QString::fromUtf8(table), QString::fromUtf8(reg));

  lua_pushnumber(L, static_cast<lua_Number>(handle));
  return 1;
}

/**
 * @brief Lua C closure for tableHandleMany(table, regs) -> handles; one handle per name, -1 if
 *        unknown.
 */
static int luaTableHandleMany(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  SS_ASSERT(store, {
    lua_pushnil(L);
    return 1;
  });

  const QString table = QString::fromUtf8(luaL_checkstring(L, 1));
  luaL_checktype(L, 2, LUA_TTABLE);

  const lua_Integer n = luaL_len(L, 2);
  lua_newtable(L);
  for (lua_Integer i = 1; i <= n; ++i) {
    lua_geti(L, 2, i);
    const qint64 handle = store->handleOf(table, QString::fromUtf8(luaL_checkstring(L, -1)));
    lua_pop(L, 1);
    lua_pushnumber(L, static_cast<lua_Number>(handle));
    lua_seti(L, -2, i);
  }

  return 1;
}

/**
 * @brief Lua C closure for tableGetH(handle); nil for a stale or invalid handle.
 */
static int luaTableGetH(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  SS_ASSERT(store, {
    lua_pushnil(L);
    return 1;
  });

  const qint64 handle = static_cast<qint64>(luaL_checknumber(L, 1));
  const auto* val     = store->getByHandle(handle);
  if (!val) {
    lua_pushnil(L);
    return 1;
  }

  if (val->isNumeric) {
    lua_pushnumber(L, val->numericValue);
  } else {
    const auto utf8 = val->stringValue.toUtf8();
    lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
  }

  return 1;
}

/**
 * @brief Lua C closure for tableSetH(handle, value); ignores non-computed/stale/invalid handles.
 *        A nil value (e.g. a failed tonumber()) is a safe no-op for parity with JS, which never
 *        raises here; a raise would fail the load-time parse() probe and reject the script.
 */
static int luaTableSetH(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  SS_ASSERT(store, return 0);

  const qint64 handle = static_cast<qint64>(luaL_checknumber(L, 1));

  if (lua_isnoneornil(L, 2))
    return 0;

  DataModel::RegisterValue rv;
  if (lua_isnumber(L, 2)) {
    rv.numericValue = lua_tonumber(L, 2);
    rv.isNumeric    = true;
  } else {
    rv.stringValue = QString::fromUtf8(luaL_checkstring(L, 2));
    rv.isNumeric   = false;
  }

  store->setByHandle(handle, rv);
  return 0;
}

/**
 * @brief Lua C closure for datasetGetRaw(uniqueIdOrAlias). A string arg is always an alias, a
 *        number always a uniqueId -- never coerce one to the other (lua_type, not lua_isnumber).
 */
static int luaDatasetGetRaw(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  SS_ASSERT(store, {
    lua_pushnil(L);
    return 1;
  });

  const auto* val = (lua_type(L, 1) == LUA_TSTRING)
                    ? store->getDatasetRawByAliasInterned(lua_tostring(L, 1))
                    : store->getDatasetRaw(static_cast<int>(luaL_checkinteger(L, 1)));

  if (!val) {
    lua_pushnil(L);
    return 1;
  }

  if (val->isNumeric) {
    lua_pushnumber(L, val->numericValue);
  } else {
    const auto utf8 = val->stringValue.toUtf8();
    lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
  }

  return 1;
}

/**
 * @brief Lua C closure for datasetGetFinal(uniqueIdOrAlias). A string arg is always an alias, a
 *        number always a uniqueId -- never coerce one to the other (lua_type, not lua_isnumber).
 */
static int luaDatasetGetFinal(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  SS_ASSERT(store, {
    lua_pushnil(L);
    return 1;
  });

  const auto* val = (lua_type(L, 1) == LUA_TSTRING)
                    ? store->getDatasetFinalByAliasInterned(lua_tostring(L, 1))
                    : store->getDatasetFinal(static_cast<int>(luaL_checkinteger(L, 1)));

  if (!val) {
    lua_pushnil(L);
    return 1;
  }

  if (val->isNumeric) {
    lua_pushnumber(L, val->numericValue);
  } else {
    const auto utf8 = val->stringValue.toUtf8();
    lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
  }

  return 1;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Lua C function for mqttPublish(topic, payload, qos?, retain?).
 */
static int luaMqttPublish(lua_State* L)
{
  const char* topic = luaL_checkstring(L, 1);

  size_t len            = 0;
  const char* payload_d = luaL_checklstring(L, 2, &len);

  int qos = 0;
  if (lua_gettop(L) >= 3 && !lua_isnil(L, 3))
    qos = static_cast<int>(luaL_checkinteger(L, 3));

  bool retain = false;
  if (lua_gettop(L) >= 4 && !lua_isnil(L, 4))
    retain = lua_toboolean(L, 4) != 0;

  static auto& publisher = MQTT::Publisher::instance();

  const auto id = publisher.mqttPublish(
    QString::fromUtf8(topic), QByteArray(payload_d, static_cast<qsizetype>(len)), qos, retain);

  lua_pushinteger(L, static_cast<lua_Integer>(id));
  return 1;
}
#endif

/**
 * @brief Injects tableGet / tableSet / datasetGetRaw / datasetGetFinal into the Lua state as C
 * closures.
 */
void DataModel::FrameBuilder::injectTableApiLua(lua_State* L)
{
  SS_ASSERT(L, return);

  m_externalTableApiUsers = true;
  m_captureFlagsDirty     = true;

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaTableGet, 1);
  lua_setglobal(L, "tableGet");

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaTableSet, 1);
  lua_setglobal(L, "tableSet");

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaTableHandle, 1);
  lua_setglobal(L, "tableHandle");

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaTableHandleMany, 1);
  lua_setglobal(L, "tableHandleMany");

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaTableGetH, 1);
  lua_setglobal(L, "tableGetH");

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaTableSetH, 1);
  lua_setglobal(L, "tableSetH");

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaDatasetGetRaw, 1);
  lua_setglobal(L, "datasetGetRaw");

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaDatasetGetFinal, 1);
  lua_setglobal(L, "datasetGetFinal");

#ifdef BUILD_COMMERCIAL
  lua_pushcfunction(L, luaMqttPublish);
  lua_setglobal(L, "mqttPublish");
#endif
}

/**
 * @brief Installs the __ss table-API bridge; the SDK prelude exposes the friendly globals.
 */
void DataModel::FrameBuilder::injectTableApiJS(QJSEngine* js)
{
  SS_ASSERT(js, return);

  m_externalTableApiUsers = true;
  m_captureFlagsDirty     = true;

  auto* bridge  = new DataModel::TableApiBridge(js);
  bridge->store = &m_tableStore;

  auto global    = js->globalObject();
  auto bridgeVal = js->newQObject(bridge);
  global.setProperty(QStringLiteral("__ss"), bridgeVal);
}
