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

#include "DataModel/FrameBuilder.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <algorithm>
#include <cmath>
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
  , m_lastConnectedState(false)
  , m_playerOpen(false)
  , m_anyAsyncSink(false)
  , m_captureDatasetValues(false)
  , m_captureFlagsDirty(true)
  , m_externalTableApiUsers(false)
  , m_captureLatestFrame(false)
  , m_seenEngineEpoch(-1)
  , m_operationMode(SerialStudio::ProjectFile)
  , m_parseBudgetUsedNs(0)
  , m_parseBudgetWindowStart(BudgetClock::time_point{})
  , m_parsedFrameCount(0)
  , m_skippedFrameCount(0)
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
  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged,
          this,
          [this] { syncFromProjectModel(); });
#endif

  if (auto* app = QCoreApplication::instance())
    connect(app, &QCoreApplication::aboutToQuit, this, [this]() { destroyTransformEngines(); });
}

/**
 * @brief Returns the singleton FrameBuilder instance.
 */
DataModel::FrameBuilder& DataModel::FrameBuilder::instance()
{
  static FrameBuilder singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Frame pool
//--------------------------------------------------------------------------------------------------

/**
 * @brief Default-constructs a pool slot with no template generation or bound source frame.
 */
DataModel::FrameBuilder::PooledFrameSlot::PooledFrameSlot() : generation(0), matchedSrc(nullptr) {}

/**
 * @brief Bumps the pool generation after a template rebuild so stale slots full-assign on reuse
 *        instead of leaking old identity fields through the structure-match fast path.
 */
void DataModel::FrameBuilder::invalidateFramePool() noexcept
{
  ++m_framePoolGeneration;
}

/**
 * @brief Probes for a free pool slot (the pool holds its only reference) and returns its index,
 *        or kInvalidSlotIdx when every slot is pinned by a consumer. All aliases are created and
 *        released on this thread, so the use_count()==1 probe is exact without a mutex.
 */
size_t DataModel::FrameBuilder::claimPoolSlot() noexcept
{
  const size_t n    = m_framePool.size();
  const size_t hint = m_framePoolHint.load(std::memory_order_relaxed);

  SS_ASSUME(n == static_cast<size_t>(kFramePoolSize));

  for (size_t k = 0; k < n; ++k) {
    const size_t idx = (hint + k) % n;

    if (m_framePool[idx].use_count() != 1)
      continue;

    m_framePoolHint.store(idx, std::memory_order_relaxed);
    return idx;
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
    QMetaObject::invokeMethod(
      &NotificationCenter::instance(),
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
  Q_ASSERT(slot != nullptr);
  Q_ASSERT(compare_frames(slot->frame.data, src));

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
  Q_ASSERT(slot != nullptr);

  if (slot->generation == m_framePoolGeneration && slot->matchedSrc == &src) [[likely]] {
    Q_ASSERT(compare_frames(slot->frame.data, src));
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
  const size_t idx = claimPoolSlot();
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
  Q_ASSERT(slotRaw->frame.data.groups.size() == src.groups.size());

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

  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::timeout1Hz,
          this,
          &DataModel::FrameBuilder::collectTransformEngineGarbage);

  connect(&CSV::Player::instance(), &CSV::Player::openChanged, this, [this] {
    m_playerOpen = SerialStudio::isAnyPlayerOpen();
    if (CSV::Player::instance().isOpen())
      rebuildTransformsForPlayback();
  });
  connect(&MDF4::Player::instance(), &MDF4::Player::openChanged, this, [this] {
    m_playerOpen = SerialStudio::isAnyPlayerOpen();
    if (MDF4::Player::instance().isOpen())
      rebuildTransformsForPlayback();
  });

#ifdef BUILD_COMMERCIAL
  connect(&Sessions::Player::instance(), &Sessions::Player::openChanged, this, [this] {
    m_playerOpen = SerialStudio::isAnyPlayerOpen();
    if (Sessions::Player::instance().isOpen())
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
  const auto& server = API::Server::instance();
  bool any = CSV::Export::instance().exportEnabled() || MDF4::Export::instance().exportEnabled()
          || (server.enabled() && server.clientCount() > 0);
#ifdef BUILD_COMMERCIAL
  any =
    any || Sessions::Export::instance().exportEnabled() || MQTT::Publisher::instance().enabled();
#endif
#ifdef ENABLE_GRPC
  const auto& grpc = API::GRPC::GRPCServer::instance();
  any              = any || (grpc.enabled() && grpc.clientCount() > 0);
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

  m_captureLatestFrame =
    DataModel::ControlScript::instance().running() || API::Server::instance().enabled();

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
  const auto& pm = DataModel::ProjectModel::instance();
  Q_ASSERT(!pm.title().isEmpty());

  clear_frame(m_frame);
  m_sourceFrames.clear();
  m_sourceFrameCounters.clear();

  m_externalTableApiUsers = false;
  m_captureFlagsDirty     = true;

  m_frame.title   = pm.title();
  m_frame.groups  = pm.groups();
  m_frame.actions = pm.actions();
  m_frame.sources = pm.sources();

  finalize_frame(m_frame);
  invalidateFramePool();
  compileTransforms();
  initializeTableStore();
  parseBudgetReset();

  Q_ASSERT(!m_frame.title.isEmpty());

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
  Q_ASSERT(data);
  Q_ASSERT(m_captureLatestFrame);

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
  Q_ASSERT(m_captureLatestFrame);
  Q_ASSERT(!channels.isEmpty());

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
  Q_ASSERT(spans != nullptr);
  Q_ASSERT(m_captureLatestFrame);
  Q_ASSERT(count > 0 && count <= kMaxSpanFields);

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
  Q_ASSERT(data);
  Q_ASSERT(!data->data.isEmpty());
  Q_ASSERT(m_operationMode == AppState::instance().operationMode());

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
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(data);
  Q_ASSERT(!data->data.isEmpty());

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
  Q_ASSERT(AppState::instance().operationMode() >= SerialStudio::ProjectFile
           && AppState::instance().operationMode() <= SerialStudio::QuickPlot);

  m_operationMode     = AppState::instance().operationMode();
  m_quickPlotChannels = -1;
  m_sourceFrames.clear();
  m_sourceFrameCounters.clear();
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
  hotpathTxFrame(acquireFrame(srcFrame));
}

/**
 * @brief Re-runs every dataset transform from the last raw values (virtual datasets re-read the
 *        data tables) and republishes the live frames. With @p feedExports false the frames go to
 *        the dashboard only; with it true they also fan out through hotpathTxFrame() to the export
 *        sinks. Returns false when no frame structure is available to publish.
 */
bool DataModel::FrameBuilder::republishFrames(bool feedExports)
{
  if (m_operationMode != SerialStudio::ProjectFile)
    return false;

  static auto& dashboard = UI::Dashboard::instance();

  bool published = false;
  for (auto& frame : m_sourceFrames) {
    if (frame.groups.empty() || frame.title.isEmpty())
      continue;

    reprocessDatasetValues(frame);
    if (feedExports)
      hotpathTxFrame(acquireFrame(frame));
    else
      dashboard.hotpathRxFrame(acquireFrame(frame));

    published = true;
  }

  if (!published && !m_frame.groups.empty() && !m_frame.title.isEmpty()) {
    reprocessDatasetValues(m_frame);
    if (feedExports)
      hotpathTxFrame(acquireFrame(m_frame));
    else
      dashboard.hotpathRxFrame(acquireFrame(m_frame));

    published = true;
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
 *        never serve a previous connection's frame (the capture flag can stay on across the
 *        cycle when the API server is enabled).
 */
void DataModel::FrameBuilder::onConnectedChanged()
{
  Q_ASSERT(AppState::instance().operationMode() >= SerialStudio::ProjectFile
           && AppState::instance().operationMode() <= SerialStudio::QuickPlot);

  const bool nowConnected = IO::ConnectionManager::instance().isConnected();
  if (nowConnected == m_lastConnectedState)
    return;

  m_lastConnectedState = nowConnected;
  m_quickPlotChannels  = -1;

  invalidateFramePool();

  parseBudgetReset();

  if (!nowConnected) {
    m_sourceFrames.clear();
    m_sourceFrameCounters.clear();
    m_latestFrames.clear();
    m_latestFrameSourceId = -1;
    destroyTransformEngines();
    m_tableStore.clear();
    return;
  }

  m_latestFrames.clear();
  m_latestFrameSourceId = -1;

  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  Q_ASSERT(!m_frame.title.isEmpty());
  DataModel::FrameParser::instance().readCode();
  compileTransforms();
  initializeTableStore();

  const auto& actions = m_frame.actions;
  for (const auto& action : actions)
    if (action.autoExecuteOnConnect) {
      const qint64 written =
        IO::ConnectionManager::instance().writeDataToDevice(action.sourceId, get_tx_bytes(action));
      if (written < 0) [[unlikely]]
        qWarning() << "[FrameBuilder] Auto-execute write failed for action:" << action.title;
    }

  const auto& sources = DataModel::ProjectModel::instance().sources();
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
  Q_ASSERT(data);
  Q_ASSERT(!data->data.isEmpty());
  Q_ASSERT(!m_frame.groups.empty());

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
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(data);
  Q_ASSERT(!data->data.isEmpty());

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
 * @brief Native span fast lane: parses byte views directly into the claimed pool slot; @p frame
 *        stays a structural template. Returns frames published, or -1 to use the QList path.
 */
int DataModel::FrameBuilder::trySpanLane(int sourceId,
                                         bool applyPerSourceOverride,
                                         DataModel::Frame& frame,
                                         const IO::CapturedDataPtr& data)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(data);

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

  const size_t idx = claimPoolSlot();
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
  qWarning() << "[FrameBuilder] Parser load exceeded budget (" << m_parseBudgetUsedNs / 1'000'000LL
             << "ms /" << kParseBudgetWindowMs << "ms)"
             << "...dropping frames until the next window rolls.";

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
  m_parseBudgetWindowStart = BudgetClock::time_point{};
  m_parseBudgetUsedNs      = 0;
  m_parseBudgetSkipping    = false;
  m_parseBudgetWarned      = false;
}

/**
 * @brief Resolves the decoder method, optionally honoring a per-source override.
 */
SerialStudio::DecoderMethod DataModel::FrameBuilder::resolveDecoderMethod(
  int sourceId, bool applyPerSourceOverride) const
{
  auto& project = DataModel::ProjectModel::instance();
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

  decodeAndParseFrame(data->data,
                      resolveDecoderMethod(sourceId, applyPerSourceOverride),
                      DataModel::FrameParser::instance(),
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
                                                const std::unordered_map<int, int>* replayColumns)
{
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

  if (!dataset.transformCode.isEmpty() && !SerialStudio::isFinalValuePlayerOpen()) [[unlikely]] {
    const auto input = dataset.isNumeric ? QVariant(dataset.numericValue) : QVariant(dataset.value);
    const auto result = applyTransform(dataset.transformLanguage, dataset.uniqueId, input, info);

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
  Q_ASSERT(spans != nullptr);
  SS_ASSUME(count > 0);

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
    const auto result = applyTransform(dataset.transformLanguage, dataset.uniqueId, input, info);

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

  const bool armJsWatchdog = (m_jsEngineForSource != nullptr);
  if (armJsWatchdog) [[unlikely]] {
    Q_ASSERT(m_jsEngineForSource->jsWatchdog);
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
  Q_ASSERT(m_compileGuard > 0);

  --m_compileGuard;

  if (armedJsWatchdog) [[unlikely]] {
    m_jsEngineForSource->jsWatchdog->disarm();
    if (m_jsTransformTimedOut)
      NotificationCenter::instance().postWarning(
        QStringLiteral("FrameBuilder"),
        tr("JavaScript transform exceeded budget"),
        tr("A dataset transform took longer than %1 ms; remaining datasets in the frame fell "
           "back to raw values until the next frame. Profile or simplify the transform code.")
          .arg(kTransformWatchdogMs));
  }

  if (m_compileGuard == 0 && m_compilePending) [[unlikely]] {
    m_compilePending = false;
    QMetaObject::invokeMethod(this, [this] { compileTransforms(); }, Qt::QueuedConnection);
  }
}

/**
 * @brief Recomputes whether per-dataset values must be mirrored into the table store: only
 *        scripts (transforms, Lua parsers, externally-injected engines) can read them back.
 */
void DataModel::FrameBuilder::refreshDatasetCaptureFlag()
{
  static auto& parser = DataModel::FrameParser::instance();
  m_captureDatasetValues =
    m_tableStore.isInitialized()
    && (!m_transformEngines.empty() || m_externalTableApiUsers || parser.hasTableApiEngines());
  m_captureFlagsDirty = false;
}

/**
 * @brief Transform-only dataset pass for reprocessFrames(): re-applies every transform from
 *        the dataset's retained raw value instead of fresh channels, so table-driven (virtual)
 *        datasets pick up the current store contents without a device frame.
 */
void DataModel::FrameBuilder::reprocessDatasetValues(DataModel::Frame& frame)
{
  Q_ASSERT(m_operationMode == SerialStudio::ProjectFile);
  Q_ASSERT(!frame.groups.empty());

  TransformFrameInfo info;
  info.sourceId    = frame.sourceId;
  info.timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now().time_since_epoch())
                       .count();
  if (!m_transformEngines.empty())
    info.frameNumber = ++m_sourceFrameCounters[frame.sourceId];

  const bool armedWatchdog = beginDatasetPass(info);

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

      if (m_captureDatasetValues)
        m_tableStore.setDatasetFinal(
          dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);
    }
  }

  endDatasetPass(armedWatchdog);
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

  const std::unordered_map<int, int>* replayColumns = nullptr;
  if (SerialStudio::isFinalValuePlayerOpen()) [[unlikely]] {
    const auto it = m_replayColumnMap.find(info.sourceId);
    if (it != m_replayColumnMap.end())
      replayColumns = &it->second;
  }

  const bool armedWatchdog = beginDatasetPass(info);

  for (auto& group : frame.groups) {
    SS_NO_UNROLL
    for (auto& dataset : group.datasets)
      applyDatasetValue(dataset, channelData, channelCount, info, replayColumns);
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
  Q_ASSERT(spans != nullptr);
  Q_ASSERT(count > 0);

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
  Q_ASSERT(datasets != nullptr);
  Q_ASSERT(spans != nullptr);

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
  Q_ASSERT(data);
  Q_ASSERT(!data->data.isEmpty());
  Q_ASSERT(AppState::instance().operationMode() == SerialStudio::QuickPlot);

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
  DataModel::Source src;
  src.sourceId = 0;
  src.title    = tr("Device A");
  src.busType  = static_cast<int>(IO::ConnectionManager::instance().busType());
  return src;
}

/**
 * @brief Rebuilds the Quick Plot frame structure when the channel count changes.
 */
void DataModel::FrameBuilder::buildQuickPlotFrame(const QStringList& channels)
{
  Q_ASSERT(!channels.isEmpty());
  Q_ASSERT(AppState::instance().operationMode() == SerialStudio::QuickPlot);

  invalidateFramePool();

#ifdef BUILD_COMMERCIAL
  const auto busType = IO::ConnectionManager::instance().busType();
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
  Q_ASSERT(!channels.isEmpty());
  Q_ASSERT(AppState::instance().operationMode() == SerialStudio::QuickPlot);

#ifdef BUILD_COMMERCIAL
  const auto* audioPtr = IO::ConnectionManager::instance().audio();
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
    dataset.fft             = true;
    dataset.plt             = !multipleChannels;
    dataset.groupId         = 0;
    dataset.datasetId       = index - 1;
    dataset.uniqueId        = dataset_unique_id(0, 0, index - 1);
    dataset.index           = index;
    dataset.value           = channel;
    dataset.pltMax          = maxValue;
    dataset.pltMin          = minValue;
    dataset.fftMax          = maxValue;
    dataset.fftMin          = minValue;
    dataset.fftSamples      = fftSamples;
    dataset.fftSamplingRate = sampleRate;

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
  Q_ASSERT(frame);
  Q_ASSERT(!frame->data.groups.empty());
  Q_ASSERT(!frame->data.title.isEmpty());

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
 * @brief Recompiles transforms + table store for a file player that bypasses ConnectionManager.
 */
void DataModel::FrameBuilder::rebuildTransformsForPlayback()
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile || m_frame.title.isEmpty())
    return;

  compileTransforms();
  initializeTableStore();
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
 * dataset pass holds hot pointers into it would dangle them.
 */
void DataModel::FrameBuilder::compileTransforms()
{
  if (m_compileGuard > 0) [[unlikely]] {
    m_compilePending = true;
    return;
  }

  destroyTransformEngines();
  Q_ASSERT(m_transformEngines.empty());

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
    Q_ASSERT(inserted);
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
  Q_ASSERT(L != nullptr);
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
    Q_ASSERT(lua_gettop(L) == baseTop);
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
 * @brief Destroys all per-source transform engines and releases resources.
 */
void DataModel::FrameBuilder::destroyTransformEngines()
{
  m_engineCacheSourceId = -1;
  m_luaEngineForSource  = nullptr;
  m_jsEngineForSource   = nullptr;
  m_captureFlagsDirty   = true;

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
  Q_ASSERT(m_transformEngines.empty());
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
  Q_ASSERT(info.sourceId >= 0);
  Q_ASSERT(uniqueId >= 0);
  Q_ASSERT(info.sourceId == m_engineCacheSourceId);

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

  return rawValue;
}

//--------------------------------------------------------------------------------------------------
// Data table store initialization and transform API injection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the DataTableStore from the project model and current frame.
 */
void DataModel::FrameBuilder::initializeTableStore()
{
  const auto& pm = DataModel::ProjectModel::instance();
  m_tableStore.initialize(pm.tables(), pm.editorTableFolders(), m_frame);
  m_captureFlagsDirty = true;
}

/**
 * @brief Re-initializes the DataTableStore from the project model's in-flight edits.
 */
void DataModel::FrameBuilder::refreshTableStoreFromProjectModel()
{
  const auto& pm = DataModel::ProjectModel::instance();
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
  Q_ASSERT(store);

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
 * @brief Lua C closure for tableSet(table, reg, value). Cache-aware like tableGet.
 */
static int luaTableSet(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  Q_ASSERT(store);

  const char* table = luaL_checkstring(L, 1);
  const char* reg   = luaL_checkstring(L, 2);

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
 * @brief Lua C closure for datasetGetRaw(uniqueId).
 */
static int luaDatasetGetRaw(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  Q_ASSERT(store);

  const int uid   = static_cast<int>(luaL_checkinteger(L, 1));
  const auto* val = store->getDatasetRaw(uid);

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
 * @brief Lua C closure for datasetGetFinal(uniqueId).
 */
static int luaDatasetGetFinal(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  Q_ASSERT(store);

  const int uid   = static_cast<int>(luaL_checkinteger(L, 1));
  const auto* val = store->getDatasetFinal(uid);

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

  const auto id = MQTT::Publisher::instance().mqttPublish(
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
  Q_ASSERT(L);

  m_externalTableApiUsers = true;
  m_captureFlagsDirty     = true;

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaTableGet, 1);
  lua_setglobal(L, "tableGet");

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaTableSet, 1);
  lua_setglobal(L, "tableSet");

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
  Q_ASSERT(js);

  m_externalTableApiUsers = true;
  m_captureFlagsDirty     = true;

  auto* bridge  = new DataModel::TableApiBridge(js);
  bridge->store = &m_tableStore;

  auto global    = js->globalObject();
  auto bridgeVal = js->newQObject(bridge);
  global.setProperty(QStringLiteral("__ss"), bridgeVal);
}
