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

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
}
// clang-format on

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <limits>

#if defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) \
  && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 130300
#  define SS_APPLE_NO_FLOAT_TO_CHARS 1
#  include <xlocale.h>
#endif
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <stdexcept>

#include "API/Server.h"
#include "AppState.h"
#include "CSV/Export.h"
#include "CSV/Player.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/FrameParserPipeline.h"
#include "IO/ConnectionManager.h"
#include "MDF4/Export.h"
#include "MDF4/Player.h"
#include "Misc/TimerEvents.h"
#include "SessionContext.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"

#ifdef BUILD_COMMERCIAL
#  include "InfluxDB/Export.h"
#  include "Licensing/CommercialToken.h"
#  include "Licensing/LemonSqueezy.h"
#  include "MQTT/Publisher.h"
#  include "Sessions/Export.h"
#  include "Sessions/Player.h"
#  include "UI/Widgets/AudioExport.h"
#endif

#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

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
  , m_parseBudgetEnabled(true)
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
  , m_projectDecoderMethod(SerialStudio::PlainText)
  , m_parsedFrameCount(0)
  , m_skippedFrameCount(0)
  , m_jsTransformTimedOut(false)
  , m_publishedTableGeneration(-1)
  , m_publishedTableClock(0)
  , m_projectSyncInFlight(false)
  , m_guiTableApiUsers(false)
  , m_tableSnapshotRequested(false)
  , m_tableMirrorRing(kTableMirrorSlots)
  , m_tableSnapshotPoolHint(0)
  , m_quickPlot(m_operationMode)
  , m_tableApi(*this, m_tableStore, m_guiTableSnapshot)
  , m_transforms(m_frame, m_tableStore, [this](lua_State* L) { injectTableApiLua(L); })
  , m_streamValuesDirty(false)
  , m_latestFrameSourceId(-1)
  , m_latestFrameSeq(0)
  , m_publishedLatestFrameSeq(0)
  , m_guiLatestFrameUsers(false)
  , m_latestFrameSnapshotRequested(false)
  , m_latestFrameMirrorRing(kLatestFrameMirrorSlots)
  , m_parseLoadMirrorRing(kParseLoadMirrorSlots)
  , m_engineCacheSourceId(-1)
  , m_luaEngineForSource(nullptr)
  , m_jsEngineForSource(nullptr)
  , m_exprEngineForSource(nullptr)
  , m_compileGuard(0)
  , m_compilePending(false)
  , m_poolPolicy(kFramePoolSize)
  , m_framePoolGeneration(1)
  , m_blockPoolHint(0)
  , m_blockSlotsUsable(kBlockPoolSlots)
  , m_maskSinks(false)
{
  m_framePool.reserve(kFramePoolSize);
  for (int i = 0; i < kFramePoolSize; ++i)
    m_framePool.emplace_back(std::make_shared<PooledFrameSlot>());

  m_tableSnapshotPool.reserve(kTableSnapshotPoolSlots);
  for (size_t i = 0; i < kTableSnapshotPoolSlots; ++i)
    m_tableSnapshotPool.emplace_back(std::make_shared<DataModel::DataTableSnapshot>());

  m_blockPool.reserve(kBlockPoolSlots);
  for (int i = 0; i < kBlockPoolSlots; ++i)
    m_blockPool.emplace_back(std::make_shared<PooledBlockSlot>());

#ifdef BUILD_COMMERCIAL
  static auto& lemonSqueezy = Licensing::LemonSqueezy::instance();
  connect(&lemonSqueezy, &Licensing::LemonSqueezy::activatedChanged, this, [this] {
    syncFromProjectModel();
  });
#endif

  if (auto* app = qApp)
    connect(app, &QCoreApplication::aboutToQuit, this, &DataModel::FrameBuilder::prepareShutdown);
}

/**
 * @brief Latches shutdown and tears the script engines down on the frame builder's own thread.
 *        Runs queued ahead of the pipeline thread's quit() (PipelineHost::shutdown) so the Lua
 *        states and QJSEngines die on the thread that owns them; idempotent for the aboutToQuit
 *        fallback on paths that never start the pipeline.
 */
void DataModel::FrameBuilder::prepareShutdown()
{
  m_shuttingDown = true;
  destroyTransformEngines();
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
DataModel::FrameBuilder::PooledFrameSlot::PooledFrameSlot() : generation(0), matchedSrc(nullptr) {}

/**
 * @brief Default-constructs a block slot with no generation or source binding.
 */
DataModel::FrameBuilder::PooledBlockSlot::PooledBlockSlot()
  : generation(0), flushEpoch(0), sourceId(-1)
{}

/**
 * @brief Bumps the pool generation after a template rebuild so stale slots full-assign on reuse,
 *        and clears slot ownership so the next claims re-partition the pool. Structure is not
 *        republished here: the snapshot goes out lazily just before the first block of the new
 *        generation, so a block never arrives ahead of its layout.
 */
void DataModel::FrameBuilder::invalidateFramePool() noexcept
{
  ++m_framePoolGeneration;
  m_publishedStructureGeneration.clear();
  m_poolPolicy.releaseOwnership();
  refreshBlockPoolBudget(m_frame.groups.empty() && !m_sourceFrames.isEmpty()
                           ? m_sourceFrames.constBegin().value()
                           : m_frame);
  refreshFramePoolBudget(m_frame.groups.empty() && !m_sourceFrames.isEmpty()
                           ? m_sourceFrames.constBegin().value()
                           : m_frame);
}

/**
 * @brief Caps how many block slots may be materialised so a wide project cannot blow past
 *        kBlockPoolBudgetBytes: a slot's storage scales with the dataset count, and 64 of them
 *        is a lot of memory once a project carries hundreds of datasets. Never drops below the
 *        dashboard ring plus headroom, since starving staging is worse than exceeding the budget.
 */
void DataModel::FrameBuilder::refreshBlockPoolBudget(const DataModel::Frame& src) noexcept
{
  std::size_t datasets = 0;
  for (const auto& group : src.groups)
    datasets += group.datasets.size();

  constexpr std::size_t kFloor = IO::PipelineHost::kBlockRingSize + 8;
  if (datasets == 0) {
    m_blockSlotsUsable = kBlockPoolSlots;
    return;
  }

  const std::size_t perSample = 2 * sizeof(double) + 2 * sizeof(QString) + 1 + sizeof(qint64);
  const std::size_t perSlot = datasets * static_cast<std::size_t>(kFrameBlockSampleCap) * perSample;
  const std::size_t affordable = perSlot > 0 ? kBlockPoolBudgetBytes / perSlot : kBlockPoolSlots;

  m_blockSlotsUsable =
    static_cast<int>(std::clamp<std::size_t>(affordable, kFloor, kBlockPoolSlots));
}

/**
 * @brief Probes for a free block slot, preferring one already laid out for @p sourceId at the
 *        current generation so openBlockFor() can skip the rebind and keep the slot's column
 *        storage. use_count()==1 is exact here for the same reason it is on the frame pool: every
 *        alias lives on this thread, and the builder holds its own reference while a block is open.
 */
std::shared_ptr<DataModel::FrameBuilder::PooledBlockSlot> DataModel::FrameBuilder::claimBlockSlot(
  int sourceId) noexcept
{
  static_assert(IO::PipelineHost::kBlockRingSize < kBlockPoolSlots - 8,
                "block pool must outsize the dashboard ring or staging starves");

  SS_ASSERT(!m_blockPool.empty(), return nullptr);

  const std::size_t n = std::min(m_blockPool.size(), static_cast<std::size_t>(m_blockSlotsUsable));
  std::size_t freeIdx = n;

  for (std::size_t k = 0; k < n; ++k) {
    const std::size_t idx = (m_blockPoolHint + k) % n;
    const auto& slot      = m_blockPool[idx];
    if (slot.use_count() != 1)
      continue;

    if (slot->generation == m_framePoolGeneration && slot->sourceId == sourceId) {
      m_blockPoolHint = (idx + 1) % n;
      return m_blockPool[idx];
    }

    if (freeIdx == n)
      freeIdx = idx;
  }

  if (freeIdx == n)
    return nullptr;

  m_blockPoolHint = (freeIdx + 1) % n;
  return m_blockPool[freeIdx];
}

/**
 * @brief Builds the structure consumers reconfigure from: the frame's layout stamped with the
 *        generation its blocks will carry. Runs on structural change only, never per frame, so
 *        the deep copy it makes is the copy the per-frame publish no longer has to.
 */
DataModel::StructureSnapshotPtr DataModel::FrameBuilder::buildStructureSnapshot(
  const DataModel::Frame& src)
{
  auto snapshot        = std::make_shared<DataModel::StructureSnapshot>();
  snapshot->generation = m_framePoolGeneration;
  snapshot->data       = src;
  return snapshot;
}

/**
 * @brief Returns whether @p sourceId already saw a snapshot for the current pool generation.
 */
bool DataModel::FrameBuilder::structureIsCurrent(int sourceId) const noexcept
{
  const auto it = m_publishedStructureGeneration.find(sourceId);
  return it != m_publishedStructureGeneration.end() && it->second == m_framePoolGeneration;
}

/**
 * @brief Records that @p sourceId's structure for the current generation has been published.
 */
void DataModel::FrameBuilder::noteStructurePublished(int sourceId) noexcept
{
  m_publishedStructureGeneration[sourceId] = m_framePoolGeneration;
}

//--------------------------------------------------------------------------------------------------
// Block staging (spec 0055)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishes @p sourceId's structure once per pool generation, before that source's first
 *        block, so the dashboard reconfigures before values for the new layout arrive. The
 *        generation is announced first (spec 0074) on its own signal -- the inner Frame does not
 *        carry it -- and the Sparkplug publisher pairs it with the structurePublished that follows.
 */
void DataModel::FrameBuilder::ensureStructurePublished(int sourceId, const DataModel::Frame& src)
{
  if (structureIsCurrent(sourceId)) [[likely]]
    return;

  static auto& pipeline = IO::PipelineHost::instance();
  pipeline.publishStructureToDashboard(buildStructureSnapshot(src));
  noteStructurePublished(sourceId);

  Q_EMIT structureGenerationChanged(m_framePoolGeneration);
  Q_EMIT structurePublished(sourceId, src);
}

/**
 * @brief Binds a claimed slot's block to @p src's dataset layout and sizes its storage once. Runs
 *        on structural change only; every later frame of that generation is a plain store into the
 *        storage laid out here.
 */
void DataModel::FrameBuilder::bindBlockToFrame(PooledBlockSlot& slot,
                                               const DataModel::Frame& src,
                                               bool uniform)
{
  auto& block               = slot.block;
  block.sourceId            = src.sourceId;
  block.structureGeneration = m_framePoolGeneration;
  block.dt                  = std::chrono::nanoseconds(0);

  block.columns.clear();
  for (const auto& group : src.groups)
    for (const auto& dataset : group.datasets) {
      DataModel::BlockColumn column;
      column.uniqueId = dataset.uniqueId;
      column.hasText  = true;
      column.hasRaw   = true;
      block.columns.push_back(std::move(column));
    }

  DataModel::size_block_storage(block, kFrameBlockSampleCap, !uniform);
  slot.generation = m_framePoolGeneration;
  slot.sourceId   = src.sourceId;
}

/**
 * @brief Returns @p sourceId's open block, opening one when none is held and flushing first when
 *        the held block was staged under an older layout -- a block must never straddle a
 *        structural change. Null when every pool slot is still referenced by a consumer.
 */
DataModel::FrameBuilder::PooledBlockSlot* DataModel::FrameBuilder::openBlockFor(
  int sourceId, const DataModel::Frame& src)
{
  static auto& pipeline = IO::PipelineHost::instance();

  const auto it = m_openBlocks.find(sourceId);
  if (it != m_openBlocks.end()) [[likely]] {
    const bool reusable =
      it->second->generation == m_framePoolGeneration && it->second->block.masked == m_maskSinks;
    if (reusable) [[likely]]
      return it->second.get();

    flushBlock(sourceId);
  }

  auto slot = claimBlockSlot(sourceId);
  if (!slot) [[unlikely]] {
    notePoolExhausted();
    return nullptr;
  }

  if (slot->generation != m_framePoolGeneration || slot->sourceId != src.sourceId)
    bindBlockToFrame(*slot, src, false);

  slot->flushEpoch = pipeline.flushEpoch();
  DataModel::reset_block(slot->block);
  slot->block.structureGeneration = m_framePoolGeneration;
  slot->block.masked              = m_maskSinks;

  const auto inserted = m_openBlocks.emplace(sourceId, std::move(slot));
  return inserted.first->second.get();
}

/**
 * @brief Appends one parsed frame's dataset values to @p sourceId's open block, flushing when the
 *        block is full or the display tick moved the flush epoch on (spec 0055 D1). Every write is
 *        a store into storage sized at bind time, so the steady state allocates nothing.
 */
SS_HOT void DataModel::FrameBuilder::stageFrameValues(
  int sourceId, const DataModel::Frame& src, const DataModel::TimestampedFrame::SteadyTimePoint& ts)
{
  SS_ASSERT_HOTPATH(sourceId >= 0);

  static auto& pipeline = IO::PipelineHost::instance();

  ensureStructurePublished(sourceId, src);

  auto* slot = openBlockFor(sourceId, src);
  if (!slot) [[unlikely]]
    return;

  auto& block           = slot->block;
  const qsizetype index = block.samples;
  if (index == 0)
    block.t0 = ts;

  std::size_t column = 0;
  for (const auto& group : src.groups) {
    SS_NO_UNROLL
    for (const auto& dataset : group.datasets) {
      if (column >= block.columns.size()) [[unlikely]]
        break;

      SS_ASSERT_HOTPATH(block.columns[column].uniqueId == dataset.uniqueId);
      DataModel::write_block_sample(
        block.columns[column], index, dataset.numericValue, dataset.value, dataset.isNumeric);
      DataModel::write_block_raw(
        block.columns[column], index, dataset.rawNumericValue, dataset.rawValue);
      ++column;
    }
  }

  DataModel::write_block_time(
    block, index, std::chrono::duration_cast<std::chrono::nanoseconds>(ts - block.t0).count());

  block.samples = index + 1;

  const quint64 epoch = pipeline.flushEpoch();
  if (block.samples >= kFrameBlockSampleCap || epoch != slot->flushEpoch) [[unlikely]]
    flushBlock(sourceId);
}

/**
 * @brief Publishes @p sourceId's open block and releases the builder's reference to its slot. The
 *        hand-out is an aliasing shared_ptr over the pool slot, so there is no per-block control
 *        block and the slot frees itself once every consumer has drained it.
 */
void DataModel::FrameBuilder::flushBlock(int sourceId)
{
  const auto it = m_openBlocks.find(sourceId);
  if (it == m_openBlocks.end())
    return;

  auto slot = it->second;
  m_openBlocks.erase(it);

  if (slot->block.samples == 0)
    return;

  slot->block.blockNumber = ++m_blockNumbers[sourceId];
  publishBlock(DataModel::DataBlockPtr(slot, &slot->block));
}

/**
 * @brief Flushes every open block regardless of fill (queued from the GUI display tick, spec 0055
 *        D1). A source that has gone quiet would otherwise hold its partial block indefinitely,
 *        because the cap and the epoch check are only reached while frames keep arriving.
 */
void DataModel::FrameBuilder::flushOpenBlocks()
{
  if (m_openBlocks.empty()) [[likely]]
    return;

  std::vector<int> sources;
  sources.reserve(m_openBlocks.size());
  for (const auto& [sourceId, slot] : m_openBlocks)
    sources.push_back(sourceId);

  for (const int sourceId : sources)
    flushBlock(sourceId);
}

/**
 * @brief Probes for a free pool slot (use_count()==1 is exact: all aliases live on this thread)
 *        or returns kInvalidSlotIdx when every slot is pinned. Per-source affinity: a source
 *        reclaims its last slot so interleaved multi-source publishes keep the pointer-identity
 *        fast path and the span lane's retained values; foreign slots are stolen only last.
 */
size_t DataModel::FrameBuilder::claimPoolSlot(int sourceId, bool hintedOnly) noexcept
{
  SS_ASSERT_HOTPATH(sourceId >= 0);

  const auto [idx, pick] = m_poolPolicy.claim(
    sourceId, hintedOnly, [this](std::size_t slot) { return m_framePool[slot].use_count() == 1; });

  Q_UNUSED(pick)
  return idx;
}

/**
 * @brief Logs the one-shot block-pool exhaustion warning. There is no heap fallback on this path:
 *        a failed claim drops the batch from every sink, so the message must say so.
 */
SS_COLD void DataModel::FrameBuilder::notePoolExhausted()
{
  static bool warned = false;
  if (!warned) [[unlikely]] {
    warned = true;
    qWarning() << "[FrameBuilder] Block pool exhausted (" << kBlockPoolSlots
               << " slots), consumers are not draining; dropping blocks.";
    static auto& nc = NotificationCenter::instance();
    QMetaObject::invokeMethod(
      &nc,
      "postWarning",
      Qt::QueuedConnection,
      Q_ARG(QString, QStringLiteral("FrameBuilder")),
      Q_ARG(QString, tr("Block pool exhausted")),
      Q_ARG(QString,
            tr("A downstream consumer (dashboard, CSV/MDF4 export, historian, or API "
               "subscriber) is not draining fast enough, so data is being dropped from the "
               "display and from any active recording. Disable a heavy consumer or reduce "
               "the data rate.")));
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
  SS_ASSERT_HOTPATH(slot != nullptr);

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
 * @brief Re-derives the pool's slot ceiling from the current frame's footprint, so the pool is
 *        bounded by memory rather than by a slot count tuned for the small frames the 256 kHz
 *        frame lane produces. Runs on structural change only, never per frame.
 */
void DataModel::FrameBuilder::refreshFramePoolBudget(const DataModel::Frame& src) noexcept
{
  std::size_t datasets = 0;
  for (const auto& group : src.groups)
    datasets += group.datasets.size();

  const std::size_t bytes = sizeof(DataModel::Frame) + src.groups.size() * sizeof(DataModel::Group)
                          + datasets * sizeof(DataModel::Dataset);

  m_poolPolicy.applyMemoryBudget(bytes, kFramePoolBudgetBytes);
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
  return m_transforms.errorCount();
}

/**
 * @brief Returns the uniqueId of the dataset whose transform error message is retained, or -1.
 */
int DataModel::FrameBuilder::lastTransformDataset() const noexcept
{
  return m_transforms.lastErrorDataset();
}

/**
 * @brief Returns the retained message of the first failure of the current failing dataset.
 */
const QString& DataModel::FrameBuilder::lastTransformError() const noexcept
{
  return m_transforms.lastError();
}

/**
 * @brief Zeroes the parsed/skipped frame counters (used by the throughput benchmark).
 */
void DataModel::FrameBuilder::resetFrameCounters()
{
  if (QThread::currentThread() != thread()) {
    invokeOnBuilderThreadBlocking([this] { resetFrameCounters(); });
    return;
  }

  m_parsedFrameCount  = 0;
  m_skippedFrameCount = 0;
}

/**
 * @brief Enables/disables the parse-load budget guard (disabled by the throughput benchmark).
 */
void DataModel::FrameBuilder::setParseBudgetEnabled(bool enabled)
{
  if (QThread::currentThread() != thread()) {
    invokeOnBuilderThreadBlocking([this, enabled] { setParseBudgetEnabled(enabled); });
    return;
  }

  m_parseBudgetEnabled = enabled;
}

/**
 * @brief Returns the current Quick Plot frame.
 */
const DataModel::Frame& DataModel::FrameBuilder::quickPlotFrame() const noexcept
{
  return m_quickPlot.frame();
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
          &DataModel::ProjectModel::sourceChanged,
          this,
          &DataModel::FrameBuilder::refreshProjectSourceSnapshot);

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::changeDrivenTransformsChanged,
          this,
          [this] { m_captureFlagsDirty = true; });

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::luaFastModeChanged,
          this,
          [this] { compileTransforms(); });

  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::timeout1Hz,
          this,
          &DataModel::FrameBuilder::collectTransformEngineGarbage);

  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz, this, [this] {
    m_parseBudget.maintain(BudgetClock::now());
    publishParseLoads();
  });

  wireDisplayTickHooks(Misc::TimerEvents::instance(), IO::PipelineHost::instance());

  const auto onPlayer = &DataModel::FrameBuilder::onPlayerOpenChanged;
  connect(&CSV::Player::instance(), &CSV::Player::openChanged, this, onPlayer);
  connect(&MDF4::Player::instance(), &MDF4::Player::openChanged, this, onPlayer);
#ifdef BUILD_COMMERCIAL
  connect(&Sessions::Player::instance(), &Sessions::Player::openChanged, this, onPlayer);
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
  connect(&Widgets::AudioExport::instance(),
          &Widgets::AudioExport::activeSessionsChanged,
          this,
          [this] { refreshAnyAsyncSink(); });
  connect(&InfluxDB::Export::instance(), &InfluxDB::Export::enabledChanged, this, [this] {
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
 * @brief Wires the two display-tick hooks. Both connections are queued by construction (the
 *        builder is pipeline-affine): the tick advances the flush epoch so a producing source
 *        closes its block on its next frame, and the queued call closes the block of a source that
 *        has gone quiet and would otherwise never reach the epoch check.
 */
void DataModel::FrameBuilder::wireDisplayTickHooks(Misc::TimerEvents& timers,
                                                   IO::PipelineHost& pipeline)
{
  connect(&timers,
          &Misc::TimerEvents::uiTimeout,
          this,
          &DataModel::FrameBuilder::refreshStreamDrivenFrames);

  connect(
    &timers,
    &Misc::TimerEvents::uiTimeout,
    &pipeline,
    [&pipeline] { pipeline.bumpFlushEpoch(); },
    Qt::DirectConnection);

  connect(&timers, &Misc::TimerEvents::uiTimeout, this, [this] { flushOpenBlocks(); });
}

/**
 * @brief Public entry point for the cached any-async-sink refresh, so a sink whose enable state
 *        moved can re-derive it from its own thread. Every sink must reach this: a missed input
 *        leaves the flag stale and the recording produces a valid-looking, empty file.
 */
void DataModel::FrameBuilder::refreshAsyncSinks()
{
  if (QThread::currentThread() != thread()) {
    invokeOnBuilderThread([this] { refreshAsyncSinks(); });
    return;
  }

  refreshAnyAsyncSink();
}

/**
 * @brief Drops the per-source "structure already published" marks so the next staged block
 *        republishes the layout. A consumer that hard-reset itself (Dashboard::resetData(true) on
 *        a player opening or a disconnect) would otherwise wait for a pool-generation bump that a
 *        replay never causes, and sit empty while blocks arrive for a layout it no longer holds.
 */
void DataModel::FrameBuilder::forgetPublishedStructures()
{
  if (QThread::currentThread() != thread()) {
    invokeOnBuilderThread([this] { forgetPublishedStructures(); });
    return;
  }

  m_publishedStructureGeneration.clear();
}

/**
 * @brief Refreshes the cached player flag and playback transforms on any player open/close, and
 *        hands the replay pools' storage back once the last player is gone.
 */
void DataModel::FrameBuilder::onPlayerOpenChanged()
{
  const bool wasOpen  = m_playerOpen;
  m_playerOpen        = SerialStudio::isAnyPlayerOpen();
  m_captureFlagsDirty = true;
  rebuildTransformsForPlayback();

  if (wasOpen && !m_playerOpen)
    releaseReplayPoolStorage();
}

/**
 * @brief Returns every idle pool slot's storage to the allocator after the last player closes: a
 *        replay binds slots to the project's full column layout and cycles hundreds of frame
 *        slots, and pool storage otherwise persists for the whole session. Busy slots (a consumer
 *        still draining) are skipped; the next claim rebinds them.
 */
void DataModel::FrameBuilder::releaseReplayPoolStorage()
{
  SS_ASSERT_LOG(QThread::currentThread() == thread());

  for (auto& slot : m_blockPool) {
    if (slot.use_count() != 1)
      continue;

    slot->block      = DataModel::DataBlock();
    slot->generation = 0;
    slot->sourceId   = -1;
  }

  for (auto& slot : m_framePool) {
    if (slot.use_count() != 1)
      continue;

    slot->frame      = DataModel::TimestampedFrame();
    slot->generation = 0;
    slot->matchedSrc = nullptr;
    slot->flat       = {};
  }

  m_poolPolicy.releaseOwnership();
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
  static auto& audioExport    = Widgets::AudioExport::instance();
  static auto& influxExport   = InfluxDB::Export::instance();
  any                         = any || sessionsExport.exportEnabled() || mqttPublisher.enabled()
     || audioExport.hasActiveSessions() || influxExport.exportEnabled();
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

  if (wasEnabled && !m_captureLatestFrame)
    clearLatestFrames();
}

/**
 * @brief Drops every retained capture and moves the sequence on, so the GUI mirror republishes
 *        the empty map: publishLatestFrameSnapshot() compares sequences, and a clear that left
 *        the sequence alone kept io.getLatestFrame serving the previous connection's frame.
 */
void DataModel::FrameBuilder::clearLatestFrames()
{
  m_latestFrames.clear();
  m_latestFrameSourceId = -1;
  ++m_latestFrameSeq;
}

//--------------------------------------------------------------------------------------------------
// Project model sync
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds m_frame from ProjectModel's in-memory state (no file I/O). A GUI-thread caller
 *        collects the project data here and carries it across, so the builder never blocks back on
 *        the GUI: that round trip made the GUI's event-loop-backed wait re-enter this function and
 *        nest loops without bound. The in-flight latch stops a re-entrant call regardless.
 */
void DataModel::FrameBuilder::syncFromProjectModel()
{
  if (QThread::currentThread() != thread()) {
    if (m_projectSyncInFlight.exchange(true, std::memory_order_acq_rel))
      return;

    auto snapshot = collectProjectSnapshot();
    invokeOnBuilderThreadBlocking([this, snapshot = std::move(snapshot)]() mutable {
      applyProjectSnapshot(std::move(snapshot));
    });
    m_projectSyncInFlight.store(false, std::memory_order_release);
    return;
  }

  ProjectSnapshot snapshot;
  IO::PipelineHost::runOnGuiThreadBlocking([&] { snapshot = collectProjectSnapshot(); });
  applyProjectSnapshot(std::move(snapshot));
}

/**
 * @brief Reads ProjectModel's live state. Must run on the GUI thread, which owns that model.
 */
DataModel::FrameBuilder::ProjectSnapshot DataModel::FrameBuilder::collectProjectSnapshot()
{
  static auto& pm = DataModel::ProjectModel::instance();
  SS_ASSERT_LOG(!pm.title().isEmpty());

  ProjectSnapshot snapshot;
  snapshot.title   = pm.title();
  snapshot.groups  = buildEnabledGroups(pm.groups());
  snapshot.actions = pm.actions();
  snapshot.sources = pm.sources();
  snapshot.decoder = pm.decoderMethod();
  return snapshot;
}

/**
 * @brief Rebuilds m_frame from an already-collected project snapshot. Builder thread only, and it
 *        deliberately never reaches back to the GUI: the caller brought the data with it.
 */
void DataModel::FrameBuilder::applyProjectSnapshot(ProjectSnapshot snapshot)
{
  SS_ASSERT(QThread::currentThread() == thread(), return);

  QString title                          = std::move(snapshot.title);
  std::vector<DataModel::Group> groups   = std::move(snapshot.groups);
  std::vector<DataModel::Action> actions = std::move(snapshot.actions);
  std::vector<DataModel::Source> sources = std::move(snapshot.sources);
  const auto decoder                     = snapshot.decoder;

  clear_frame(m_frame);
  m_sourceFrames.clear();
  m_sourceFrameCounters.clear();
  m_republishGate.clear();
  m_streamDatasetIds.clear();
  m_streamValuesDirty = false;

  m_externalTableApiUsers = false;
  m_captureFlagsDirty     = true;

  m_frame.title          = std::move(title);
  m_frame.groups         = std::move(groups);
  m_frame.actions        = std::move(actions);
  m_frame.sources        = std::move(sources);
  m_projectDecoderMethod = decoder;

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
  if (QThread::currentThread() != thread()) {
    invokeOnBuilderThreadBlocking([this, headers] { registerQuickPlotHeaders(headers); });
    return;
  }

  m_quickPlot.setHeaders(headers);
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
  SS_ASSERT_HOTPATH(data);
  SS_ASSERT_HOTPATH(!data->data.isEmpty());

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
  SS_ASSERT_HOTPATH(sourceId >= 0);
  SS_ASSERT_HOTPATH(data);
  SS_ASSERT_HOTPATH(!data->data.isEmpty());

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
  static auto& pipeline = IO::PipelineHost::instance();
  const auto mode       = pipeline.operationMode();
  SS_ASSERT(mode >= SerialStudio::ProjectFile && mode <= SerialStudio::QuickPlot, return);

  m_operationMode     = mode;
  m_quickPlotChannels = -1;
  m_sourceFrames.clear();
  m_sourceFrameCounters.clear();
  m_republishGate.clear();
  m_streamDatasetIds.clear();
  m_streamValuesDirty = false;
  clearLatestFrames();
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
  m_republishGate.notePublishedTemplate(src.sourceId);
  ensureStructurePublished(src.sourceId, m_sourceFrames[src.sourceId]);
}

/**
 * @brief Publishes the dashboard template frame for one stream source (spec 0051 T25): widget
 *        models build from this structure; live values then arrive only through the stream
 *        display path. Runs on the builder thread (marshaled by ConnectionManager).
 */
void DataModel::FrameBuilder::publishSourceTemplate(int sourceId)
{
  SS_ASSERT(sourceId >= 0, return);

  if (m_operationMode != SerialStudio::ProjectFile || m_frame.groups.empty())
    return;

  for (const auto& src : m_frame.sources)
    if (src.sourceId == sourceId) {
      publishSourceTemplateFrame(src);
      return;
    }
}

/**
 * @brief Builds and publishes the QuickPlot audio structure for a stream-lane session (spec
 *        0051 T25): the text path no longer runs, so the frame is synthesized here once at
 *        connect from the channel count.
 */
void DataModel::FrameBuilder::publishQuickPlotAudioTemplate(int channels)
{
  SS_ASSERT(channels > 0, return);

  if (m_operationMode != SerialStudio::QuickPlot)
    return;

  QStringList channelValues;
  channelValues.reserve(channels);
  for (int i = 0; i < channels; ++i)
    channelValues.append(QStringLiteral("0"));

  invalidateFramePool();
  m_quickPlot.buildAudio(channelValues);
  m_quickPlotChannels = channels;

  const auto& quickPlotFrame = m_quickPlot.frame();
  if (!quickPlotFrame.groups.empty())
    ensureStructurePublished(quickPlotFrame.sourceId, quickPlotFrame);
}

/**
 * @brief Writes a stream source's per-block latest values into the data-table store at block
 *        rate (spec 0051 R13). Queued from the stream workers, executes on the builder thread,
 *        so the store's single-writer invariant holds and change-driven versioning works
 *        unchanged; per-sample store writes never happen.
 */
void DataModel::FrameBuilder::ingestStreamValues(int sourceId,
                                                 const QList<QPair<int, double>>& values)
{
  SS_ASSERT(sourceId >= 0, return);
  SS_ASSERT_LOG(QThread::currentThread() == thread());

  if (!m_tableStore.isInitialized() || values.isEmpty())
    return;

  for (const auto& [uniqueId, value] : values) {
    m_tableStore.setDatasetRaw(uniqueId, value, QString(), true);
    m_tableStore.setDatasetFinal(uniqueId, value, QString(), true);
    m_streamDatasetIds.insert(uniqueId);
  }

  m_streamValuesDirty = true;
}

/**
 * @brief Publishes one dense-lane block through the same tail the frame lane uses (spec 0055 D8).
 *        Queued from the stream workers so the PIPELINE thread stays the single producer for
 *        every sink: routing dense blocks straight to the sinks from the GUI would give each
 *        sink's SPSC queue a second producer, which is what the two-sink split existed to avoid.
 */
void DataModel::FrameBuilder::ingestStreamBlock(const DataModel::DataBlockPtr& block)
{
  SS_ASSERT(block != nullptr, return);
  SS_ASSERT_LOG(QThread::currentThread() == thread());

  if (block->samples <= 0) [[unlikely]]
    return;

  publishBlock(block);
}

/**
 * @brief Records which sources feed the stream lane, so the frame-lane republish paths skip
 *        their template frames (their live values are the Dashboard's stream-ingest copies).
 */
void DataModel::FrameBuilder::setStreamSourceIds(const QSet<int>& sourceIds)
{
  if (QThread::currentThread() != thread()) {
    invokeOnBuilderThread([this, sourceIds] { setStreamSourceIds(sourceIds); });
    return;
  }

  m_streamSourceIds = sourceIds;
  m_streamDatasetIds.clear();
}

/**
 * @brief Re-runs the frame-lane transforms after stream values land in the table store (spec
 *        0051 R13/AC11): without it a virtual dataset reading a stream slot only recomputes
 *        when a frame-lane source publishes, i.e. at the slowest source's rate. Runs on the UI
 *        refresh tick, gated on the dirty flag, so an all-frame-lane session pays nothing.
 */
void DataModel::FrameBuilder::refreshStreamDrivenFrames()
{
  if (!m_streamValuesDirty)
    return;

  m_streamValuesDirty = false;
  if (m_operationMode != SerialStudio::ProjectFile)
    return;

  (void)republishFrames(false);
}

/**
 * @brief Stages and immediately flushes one republished frame as a single-sample block, so a
 *        synthetic refresh reaches the display within the tick that asked for it. Any block
 *        already open is flushed FIRST and unmasked: it holds real captured samples, and
 *        closing it under the mask would drop them from every recording sink.
 */
bool DataModel::FrameBuilder::emitRepublishedFrame(const DataModel::Frame& frame,
                                                   int key,
                                                   bool feedExports)
{
  const int sourceId = frame.sourceId;
  flushBlock(sourceId);

  const auto published_count = [this, sourceId] {
    const auto it = m_blockNumbers.find(sourceId);
    return it != m_blockNumbers.end() ? it->second : quint64(0);
  };

  const quint64 before    = published_count();
  const bool previousMask = m_maskSinks;
  m_maskSinks             = m_maskSinks || !feedExports;

  stageFrameValues(sourceId, frame, DataModel::TimestampedFrame::SteadyClock::now());
  flushBlock(sourceId);

  m_maskSinks = previousMask;

  if (published_count() == before)
    return false;

  m_republishGate.notePublished(key, feedExports);
  return true;
}

/**
 * @brief Re-runs every dataset transform from the last raw values and republishes the live
 *        frames: dashboard only with @p feedExports false, full sink fan-out with it true. A
 *        frame republishes only on a changed dataset value or its first publish, so a synthetic
 *        tick never touches the plot clock of a source whose data did not change.
 */
bool DataModel::FrameBuilder::republishFrames(bool feedExports)
{
  if (m_operationMode != SerialStudio::ProjectFile)
    return false;

  constexpr int combined_frame_key = -1;

  bool published  = false;
  bool any_source = false;
  for (auto& frame : m_sourceFrames) {
    if (frame.groups.empty() || frame.title.isEmpty())
      continue;

    // code-verify off
    // A stream source's channel values are drawn from the Dashboard's stream ingest, so its
    // frame is recomputed (virtual datasets read the store) but never published: publishing
    // would overwrite those widget copies and double-push its plot rings.
    // code-verify on
    if (m_streamSourceIds.contains(frame.sourceId)) {
      (void)reprocessDatasetValues(frame);
      continue;
    }

    any_source = true;
    published  = republishOneFrame(frame, frame.sourceId, feedExports) || published;
  }

  if (!any_source && !m_frame.groups.empty() && !m_frame.title.isEmpty())
    published = republishOneFrame(m_frame, combined_frame_key, feedExports) || published;

  return published;
}

/**
 * @brief Runs @p frame's transform pass and publishes it when this lane still owes @p key a
 *        publish. An export publish is what clears the sink-dirty mark: until one lands, the
 *        recording sinks are behind the values the dashboard already shows (spec 0064).
 */
bool DataModel::FrameBuilder::republishOneFrame(DataModel::Frame& frame, int key, bool feedExports)
{
  const bool changed = reprocessDatasetValues(frame);
  if (changed)
    m_republishGate.noteChanged(key);

  if (!m_republishGate.needed(key, changed, feedExports))
    return false;

  return emitRepublishedFrame(frame, key, feedExports);
}

/**
 * @brief Re-runs transforms from the last received values and republishes to the dashboard only,
 *        with no export fan-out, so a synthetic refresh never re-records frames that were already
 *        exported on arrival. Returns false when no frame structure is available to publish, and
 *        when a GUI caller queued the pass instead of waiting for it.
 */
bool DataModel::FrameBuilder::reprocessFrames()
{
  if (QThread::currentThread() != thread()) {
    if (qApp && QThread::currentThread() == qApp->thread()) {
      invokeOnBuilderThread([this] { (void)reprocessFrames(); });
      return false;
    }

    bool published = false;
    invokeOnBuilderThreadBlocking([this, &published] { published = reprocessFrames(); });
    return published;
  }

  return republishFrames(false);
}

/**
 * @brief Forces a render from the current table/dataset state even when the device is silent:
 *        seeds each source frame from the template, runs the transform-only pass and publishes
 *        as one block, so table-driven datasets render and feed the exports. A GUI
 *        caller queues the pass and reports false; waiting would park it behind the pipeline.
 */
bool DataModel::FrameBuilder::dashboardTick()
{
  if (QThread::currentThread() != thread()) {
    if (qApp && QThread::currentThread() == qApp->thread()) {
      invokeOnBuilderThread([this] { (void)dashboardTick(); });
      return false;
    }

    bool published = false;
    invokeOnBuilderThreadBlocking([this, &published] { published = dashboardTick(); });
    return published;
  }

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
 * @brief GUI-side half of the data-table mirror (spec 0051 M5): adopts the newest snapshot the
 *        builder thread published, then requests the next. Runs once per display tick before
 *        updated() reaches painter and output-widget scripts, so their tableGet/datasetGet calls
 *        read a GUI-local copy instead of parking the tick behind the pipeline thread.
 */
void DataModel::FrameBuilder::drainTableSnapshot()
{
  SS_ASSERT(qApp != nullptr, return);
  SS_ASSERT(QThread::currentThread() == qApp->thread(), return);

  if (!m_guiTableApiUsers.load(std::memory_order_relaxed)) [[likely]]
    return;

  DataModel::DataTableSnapshotPtr snapshot;
  // code-verify off
  // Ring drain: bounded by the mirror ring capacity (4), provably finite per tick.
  while (m_tableMirrorRing.try_dequeue(snapshot))
    if (snapshot)
      m_guiTableSnapshot = snapshot;
  // code-verify on

  snapshot.reset();

  if (!m_tableSnapshotRequested.exchange(true, std::memory_order_acq_rel))
    invokeOnBuilderThread([this] { publishTableSnapshot(); });
}

/**
 * @brief Arms the mirror when a script engine is wired up on the GUI thread. Engines injected on
 *        the pipeline thread (the parser and every dataset transform) read the live store
 *        directly, so they must not make the builder pay for a snapshot nobody reads.
 */
void DataModel::FrameBuilder::noteGuiTableApiUser()
{
  if (qApp && QThread::currentThread() == qApp->thread())
    m_guiTableApiUsers.store(true, std::memory_order_relaxed);
}

/**
 * @brief Table-API context for callers that reach the store through readTableView/writeTableStore,
 *        arming the GUI mirror on the way so an API handler serving a script gets a snapshot to
 *        read instead of a marshal that would park the GUI behind the pipeline.
 */
const DataModel::TableApiContext& DataModel::FrameBuilder::guiTableApiContext()
{
  noteGuiTableApiUser();
  return m_tableApi.context();
}

/**
 * @brief Claims a free pooled snapshot slot, or null when every slot is in flight (the caller
 *        skips the publish and the next display-tick request retries the same state). The
 *        use_count probe is an atomic read and the acquire fence pairs with the GUI's release
 *        of its previously adopted snapshot, so slot reuse happens-after every consumer read.
 */
std::shared_ptr<DataModel::DataTableSnapshot> DataModel::FrameBuilder::claimTableSnapshotSlot()
{
  SS_ASSERT(!m_tableSnapshotPool.empty(), return nullptr);

  const std::size_t n = m_tableSnapshotPool.size();
  for (std::size_t k = 0; k < n; ++k) {
    const std::size_t idx = (m_tableSnapshotPoolHint + k) % n;
    if (m_tableSnapshotPool[idx].use_count() != 1)
      continue;

    std::atomic_thread_fence(std::memory_order_acquire);
    m_tableSnapshotPoolHint = (idx + 1) % n;
    return m_tableSnapshotPool[idx];
  }

  return nullptr;
}

/**
 * @brief Builder-thread half of the mirror: fills a reused pool slot from the store when its
 *        layout generation or write clock moved since the last publish, so the steady state
 *        allocates nothing. Runs on request at display-tick rate, never per frame. Pool
 *        exhaustion or a full ring leaves the bookkeeping untouched so the next request retries.
 */
void DataModel::FrameBuilder::publishTableSnapshot()
{
  SS_ASSERT(QThread::currentThread() == thread(), return);

  m_tableSnapshotRequested.store(false, std::memory_order_release);

  const int generation = m_tableStore.isInitialized() ? m_tableStore.generation() : -1;
  const quint64 clock  = m_tableStore.writeClock();
  if (generation == m_publishedTableGeneration && clock == m_publishedTableClock)
    return;

  const auto slot = claimTableSnapshotSlot();
  if (!slot) [[unlikely]]
    return;

  m_tableStore.snapshotInto(*slot);
  if (!m_tableMirrorRing.try_enqueue(DataModel::DataTableSnapshotPtr(slot, slot.get())))
    [[unlikely]]
    return;

  m_publishedTableGeneration = generation;
  m_publishedTableClock      = clock;
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

  static auto& pipeline = IO::PipelineHost::instance();
  SS_ASSERT(m_operationMode >= SerialStudio::ProjectFile
              && m_operationMode <= SerialStudio::QuickPlot,
            return);

  const bool nowConnected = pipeline.pipelineConnected();
  if (nowConnected == m_lastConnectedState)
    return;

  m_lastConnectedState = nowConnected;
  m_quickPlotChannels  = -1;

  invalidateFramePool();

  parseBudgetReset();

  if (!nowConnected) {
    m_sourceFrames.clear();
    m_sourceFrameCounters.clear();
    m_republishGate.clear();
    m_streamDatasetIds.clear();
    m_streamValuesDirty = false;
    clearLatestFrames();
    destroyTransformEngines();
    m_tableStore.clear();
    return;
  }

  clearLatestFrames();

  Q_EMIT sessionStructureReady(m_operationMode == SerialStudio::ProjectFile ? m_frame
                                                                            : m_quickPlot.frame());

  if (m_operationMode != SerialStudio::ProjectFile)
    return;

  SS_ASSERT_LOG(!m_frame.title.isEmpty());
  initializeTableStore();
  static auto& parser = DataModel::FrameParser::instance();
  parser.readCode();
  compileTransforms();

  static auto& ioManager = IO::ConnectionManager::instance();
  const auto& actions    = m_frame.actions;
  for (const auto& action : actions)
    if (action.autoExecuteOnConnect) {
      const int actionSource  = action.sourceId;
      const QByteArray txData = get_tx_bytes(action);
      const QString title     = action.title;
      QMetaObject::invokeMethod(
        &ioManager,
        [actionSource, txData, title] {
          const qint64 written = ioManager.writeDataToDevice(actionSource, txData);
          if (written < 0) [[unlikely]]
            qWarning() << "[FrameBuilder] Auto-execute write failed for action:" << title;
        },
        Qt::QueuedConnection);
    }

  const auto& sources = m_frame.sources;
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
    ensureStructurePublished(m_frame.sourceId, m_frame);
}

//--------------------------------------------------------------------------------------------------
// Frame parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses a project frame using the configured decoding method.
 */
void DataModel::FrameBuilder::parseProjectFrame(const IO::CapturedDataPtr& data)
{
  SS_ASSERT_HOTPATH(data);
  SS_ASSERT_HOTPATH(!data->data.isEmpty());

  if (m_frame.groups.empty()) [[unlikely]]
    return;

  if (parseBudgetSkipFrame(0)) [[unlikely]]
    return;

  const auto t0 = m_parseBudgetEnabled ? BudgetClock::now() : BudgetClock::time_point{};

  const int published = trySpanLane(0, false, m_frame, data);
  if (published >= 0) {
    m_parsedFrameCount += static_cast<quint64>(published);
    parseBudgetAccount(0, t0);
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

    if (!m_transforms.empty()) {
      info.frameNumber = ++m_sourceFrameCounters[0];
      info.timestampMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(frameTs.time_since_epoch()).count();
    }

    if (m_captureLatestFrame) [[unlikely]]
      captureLatestChannels(0, channels);

    applyDatasetValues(m_frame, channels, info);
    stageFrameValues(0, m_frame, frameTs);
    ++m_parsedFrameCount;
  }

  parseBudgetAccount(0, t0);
}

/**
 * @brief Source-aware variant of parseProjectFrame.
 */
void DataModel::FrameBuilder::parseProjectFrame(int sourceId, const IO::CapturedDataPtr& data)
{
  SS_ASSERT_HOTPATH(sourceId >= 0);
  SS_ASSERT_HOTPATH(data);
  SS_ASSERT_HOTPATH(!data->data.isEmpty());

  if (m_frame.groups.empty()) [[unlikely]]
    return;

  if (parseBudgetSkipFrame(sourceId)) [[unlikely]]
    return;

  const auto t0 = m_parseBudgetEnabled ? BudgetClock::now() : BudgetClock::time_point{};

  const int published = trySpanLane(sourceId, true, ensureSourceFrame(sourceId), data);
  if (published >= 0) {
    m_parsedFrameCount += static_cast<quint64>(published);
    parseBudgetAccount(sourceId, t0);
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

    if (!m_transforms.empty()) {
      info.frameNumber = ++m_sourceFrameCounters[sourceId];
      info.timestampMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(frameTs.time_since_epoch()).count();
    }

    if (m_captureLatestFrame) [[unlikely]]
      captureLatestChannels(sourceId, channels);

    applyDatasetValues(srcFrame, channels, info);
    stageFrameValues(sourceId, srcFrame, frameTs);
    ++m_parsedFrameCount;
  }

  parseBudgetAccount(sourceId, t0);
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
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(
      this,
      [this, sourceId, &channels, &timestamp] { replayChannels(sourceId, channels, timestamp); },
      Qt::BlockingQueuedConnection);
    return;
  }

  SS_ASSERT_HOTPATH(sourceId >= 0);
  SS_ASSERT_HOTPATH(m_playerOpen);
  SS_ASSERT_HOTPATH(m_operationMode == SerialStudio::ProjectFile);

  if (channels.isEmpty() || m_frame.groups.empty()) [[unlikely]]
    return;

  DataModel::Frame& srcFrame = ensureSourceFrame(sourceId);
  if (srcFrame.groups.empty() || srcFrame.title.isEmpty()) [[unlikely]]
    return;

  TransformFrameInfo info;
  info.sourceId = sourceId;

  applyDatasetValues(srcFrame, channels, info);
  publishReplayValues(sourceId, srcFrame, timestamp);
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
  SS_ASSERT_HOTPATH(len > 0 && static_cast<size_t>(len) < sizeof(buf));
  DataModel::assign_utf8_in_place(dst, QByteArrayView(buf, static_cast<qsizetype>(len)));
#else
  const auto res = std::to_chars(buf, buf + sizeof(buf), value, std::chars_format::general, 10);
  SS_ASSERT_HOTPATH(res.ec == std::errc());
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
  SS_ASSERT_HOTPATH(sourceId >= 0);
  SS_ASSERT_HOTPATH(SerialStudio::isFinalValuePlayerOpen());

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
  SS_ASSERT_HOTPATH(cells != nullptr);
  SS_ASSERT_HOTPATH(count > 0);

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

  if (m_exprEngineForSource) [[unlikely]]
    m_exprEngineForSource->exprSlots->publish(dataset.uniqueId, dataset.numericValue);

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
  SS_ASSERT_HOTPATH(cells != nullptr);
  SS_ASSERT_HOTPATH(count > 0);

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

  if (m_exprEngineForSource) [[unlikely]]
    m_exprEngineForSource->exprSlots->publish(dataset.uniqueId, dataset.numericValue);

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
  if (QThread::currentThread() != thread()) {
    // code-verify off
    // Plain BlockingQueued IS the documented design for the replay lanes (dataflow.md): engines
    // are torn down during replay so no apiCall cycle can exist, the borrowed span pointers must
    // stay alive, and the event-loop marshal wakes on the NEXT UI TICK -- measured 8.1 ms per
    // row against 4 us of publish work, i.e. the entire replay speed ceiling (spec 0064).
    // code-verify on
    QMetaObject::invokeMethod(
      this,
      [this, sourceId, cells, count, &timestamp] {
        replayChannelSpans(sourceId, cells, count, timestamp);
      },
      Qt::BlockingQueuedConnection);
    return;
  }

  SS_ASSERT_HOTPATH(sourceId >= 0);
  SS_ASSERT_HOTPATH(cells != nullptr || count == 0);
  SS_ASSERT_HOTPATH(m_playerOpen);
  SS_ASSERT_HOTPATH(m_operationMode == SerialStudio::ProjectFile);

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

  publishReplayValues(sourceId, srcFrame, timestamp);
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
  if (QThread::currentThread() != thread()) {
    QMetaObject::invokeMethod(
      this,
      [this, sourceId, cells, count, &timestamp] {
        replayChannelsTyped(sourceId, cells, count, timestamp);
      },
      Qt::BlockingQueuedConnection);
    return;
  }

  SS_ASSERT_HOTPATH(sourceId >= 0);
  SS_ASSERT_HOTPATH(cells != nullptr || count == 0);
  SS_ASSERT_HOTPATH(m_playerOpen);
  SS_ASSERT_HOTPATH(m_operationMode == SerialStudio::ProjectFile);

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

  publishReplayValues(sourceId, srcFrame, timestamp);
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
  SS_ASSERT_HOTPATH(sourceId >= 0);
  SS_ASSERT_HOTPATH(data);

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

  if (!m_transforms.empty()) [[unlikely]] {
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
    stageFrameValues(sourceId, heap->data, data->timestamp);
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
  stageFrameValues(sourceId, slotRaw->frame.data, data->timestamp);
  return 1;
}

//--------------------------------------------------------------------------------------------------
// Parser-load budget guard
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the fair-share governor thins this frame; counts the skip. Sources at
 *        N=1 (the steady state) pass with a lookup and a compare.
 */
bool DataModel::FrameBuilder::parseBudgetSkipFrame(int sourceId)
{
  if (!m_parseBudgetEnabled) [[unlikely]]
    return false;

  if (!m_parseBudget.skipFrame(sourceId)) [[likely]]
    return false;

  ++m_skippedFrameCount;
  return true;
}

/**
 * @brief Charges the governor with this frame's parse time; logs once when thinning engages
 *        (the live numbers are pulled by the 1 Hz diagnostics, never pushed from here).
 */
void DataModel::FrameBuilder::parseBudgetAccount(int sourceId, BudgetClock::time_point startedAt)
{
  if (!m_parseBudgetEnabled) [[unlikely]]
    return;

  if (m_parseBudget.account(sourceId, startedAt, BudgetClock::now())) [[unlikely]]
    noteParseBudgetThinning(sourceId);
}

/**
 * @brief One-shot console note when thinning first engages, with the offender's snapshot row.
 */
SS_COLD void DataModel::FrameBuilder::noteParseBudgetThinning(int sourceId)
{
  const auto loads = m_parseBudget.snapshot();
  for (const auto& load : loads)
    if (load.sourceId == sourceId)
      qWarning() << "[FrameBuilder] Parse load over fair share for source" << sourceId << "(duty"
                 << load.duty << ") -- keeping every" << load.decimateN
                 << "th frame until load recovers.";
}

/**
 * @brief Clears the parser-budget state -- called when the active project changes.
 */
void DataModel::FrameBuilder::parseBudgetReset() noexcept
{
  m_parseBudget.reset();
}

/**
 * @brief Returns whether any source is currently decimated (polled by the dashboard indicator).
 */
bool DataModel::FrameBuilder::parseBudgetThinning() const noexcept
{
  return m_parseBudget.thinning();
}

/**
 * @brief Snapshots every tracked source's parse load for the 1 Hz diagnostics pull (cold path).
 *        The budget's hash rehashes on first sight of a source, so the GUI reads the mirror the
 *        builder publishes on its own 1 Hz tick and other threads marshal: a GUI marshal spins a
 *        nested loop, which macOS runs re-entrantly and which swallows window resize steps.
 */
std::vector<DataModel::FrameBuilder::ParseLoad> DataModel::FrameBuilder::parseLoadSnapshot()
{
  if (QThread::currentThread() != thread()) {
    if (qApp && QThread::currentThread() == qApp->thread())
      return guiParseLoads();

    std::vector<ParseLoad> loads;
    invokeOnBuilderThreadBlocking([this, &loads] { loads = m_parseBudget.snapshot(); });
    return loads;
  }

  return m_parseBudget.snapshot();
}

/**
 * @brief Builder-thread half of the parse-load mirror, published on the same 1 Hz tick the
 *        diagnostics sample at, so the GUI reader never marshals.
 */
void DataModel::FrameBuilder::publishParseLoads()
{
  SS_ASSERT(QThread::currentThread() == thread(), return);

  auto sample = std::make_shared<const std::vector<ParseLoad>>(m_parseBudget.snapshot());
  (void)m_parseLoadMirrorRing.try_enqueue(std::move(sample));
}

/**
 * @brief GUI-thread read of the parse loads: adopts the newest published sample and serves it.
 *        One tick of staleness is inherent to a 1 Hz diagnostic.
 */
std::vector<DataModel::FrameBuilder::ParseLoad> DataModel::FrameBuilder::guiParseLoads()
{
  ParseLoadsPtr sample;
  // code-verify off
  // Ring drain: bounded by the mirror ring capacity (4), provably finite per call.
  while (m_parseLoadMirrorRing.try_dequeue(sample))
    if (sample)
      m_guiParseLoads = sample;
  // code-verify on

  sample.reset();
  return m_guiParseLoads ? *m_guiParseLoads : std::vector<ParseLoad>();
}

/**
 * @brief Value copy of the latest captured frame for cross-thread consumers (API handlers): the
 *        internal hash rehashes per frame on the builder thread, so pointers must not escape.
 *        A negative @p sourceId selects the newest source; an empty snapshot has sequence 0.
 */
DataModel::FrameBuilder::LatestFrameInfo DataModel::FrameBuilder::latestFrameSnapshot(int sourceId)
{
  if (QThread::currentThread() == thread()) {
    const auto* info = latestFrame(sourceId);
    return info ? *info : LatestFrameInfo();
  }

  if (qApp && QThread::currentThread() == qApp->thread())
    return guiLatestFrame(sourceId);

  LatestFrameInfo copy;
  invokeOnBuilderThreadBlocking([this, sourceId, &copy] {
    const auto* info = latestFrame(sourceId);
    if (info)
      copy = *info;
  });

  return copy;
}

/**
 * @brief GUI-thread read of the latest capture: serves the mirror the builder publishes on the
 *        display tick and arms it on first use. Marshaling here would spin a nested event loop
 *        inside the API dispatch and park the GUI behind the pipeline (spec 0051 M5 rule), which
 *        is what made a polling control script freeze the window's OS event handling.
 */
DataModel::FrameBuilder::LatestFrameInfo DataModel::FrameBuilder::guiLatestFrame(int sourceId)
{
  m_guiLatestFrameUsers.store(true, std::memory_order_relaxed);

  const auto mirror = m_guiLatestFrameMirror;
  if (!mirror)
    return LatestFrameInfo();

  const int key = (sourceId >= 0) ? sourceId : mirror->newestSourceId;
  if (key < 0)
    return LatestFrameInfo();

  const auto it = mirror->frames.constFind(key);
  return (it != mirror->frames.constEnd()) ? it.value() : LatestFrameInfo();
}

/**
 * @brief Builder-thread half of the latest-frame mirror: copies the capture map for the GUI when
 *        a new frame landed since the last publish. Runs at display-tick rate, never per frame.
 */
void DataModel::FrameBuilder::publishLatestFrameSnapshot()
{
  SS_ASSERT(QThread::currentThread() == thread(), return);

  m_latestFrameSnapshotRequested.store(false, std::memory_order_release);

  if (m_latestFrameSeq == m_publishedLatestFrameSeq)
    return;

  auto mirror            = std::make_shared<LatestFrameMirror>();
  mirror->newestSourceId = m_latestFrameSourceId;
  mirror->frames         = m_latestFrames;

  if (!m_latestFrameMirrorRing.try_enqueue(LatestFrameMirrorPtr(std::move(mirror)))) [[unlikely]]
    return;

  m_publishedLatestFrameSeq = m_latestFrameSeq;
}

/**
 * @brief GUI-side half of the latest-frame mirror: adopts the newest published copy, then
 *        requests the next. Gated on a GUI-thread reader having asked at least once, so a session
 *        without a polling script or API client never makes the builder pay for the copy.
 */
void DataModel::FrameBuilder::drainLatestFrameSnapshot()
{
  SS_ASSERT(qApp != nullptr, return);
  SS_ASSERT(QThread::currentThread() == qApp->thread(), return);

  if (!m_guiLatestFrameUsers.load(std::memory_order_relaxed)) [[likely]]
    return;

  LatestFrameMirrorPtr mirror;
  // code-verify off
  // Ring drain: bounded by the mirror ring capacity (4), provably finite per tick.
  while (m_latestFrameMirrorRing.try_dequeue(mirror))
    if (mirror)
      m_guiLatestFrameMirror = mirror;
  // code-verify on

  mirror.reset();

  if (!m_latestFrameSnapshotRequested.exchange(true, std::memory_order_acq_rel))
    invokeOnBuilderThread([this] { publishLatestFrameSnapshot(); });
}

/**
 * @brief Resolves the decoder method from the builder-local project snapshot (m_frame.sources +
 *        the cached project default), never the live ProjectModel: this runs per frame on the
 *        pipeline thread while the GUI may be editing the project (spec 0051 M3).
 */
SerialStudio::DecoderMethod DataModel::FrameBuilder::resolveDecoderMethod(
  int sourceId, bool applyPerSourceOverride) const
{
  if (!applyPerSourceOverride)
    return m_projectDecoderMethod;

  for (const auto& src : m_frame.sources)
    if (src.sourceId == sourceId)
      return static_cast<SerialStudio::DecoderMethod>(src.decoderMethod);

  return m_projectDecoderMethod;
}

/**
 * @brief Refreshes the builder-local source snapshot after a live per-source edit (decoder,
 *        framing): snapshots on the GUI thread, applies on the builder thread.
 */
void DataModel::FrameBuilder::refreshProjectSourceSnapshot()
{
  if (QThread::currentThread() != thread()) {
    invokeOnBuilderThreadBlocking([this] { refreshProjectSourceSnapshot(); });
    return;
  }

  std::vector<DataModel::Source> sources;
  auto decoder = SerialStudio::PlainText;
  IO::PipelineHost::runOnGuiThreadBlocking([&] {
    static auto& pm = DataModel::ProjectModel::instance();
    sources         = pm.sources();
    decoder         = pm.decoderMethod();
  });

  m_frame.sources        = std::move(sources);
  m_projectDecoderMethod = decoder;
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
      dep->hasRun       = true;
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

  if (m_exprEngineForSource) [[unlikely]]
    m_exprEngineForSource->exprSlots->publish(dataset.uniqueId, dataset.numericValue);

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
  SS_ASSERT_HOTPATH(spans != nullptr);
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
      dep->hasRun       = true;
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

  if (m_exprEngineForSource) [[unlikely]]
    m_exprEngineForSource->exprSlots->publish(dataset.uniqueId, dataset.numericValue);

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
    auto* expr            = m_transforms.engineFor(info.sourceId, SerialStudio::Expression);
    m_luaEngineForSource  = m_transforms.engineFor(info.sourceId, SerialStudio::Lua);
    m_jsEngineForSource   = m_transforms.engineFor(info.sourceId, SerialStudio::JavaScript);
    m_exprEngineForSource = (expr && expr->exprSlots) ? expr : nullptr;
  }

  const bool armJsWatchdog =
    (m_jsEngineForSource != nullptr) && (m_jsEngineForSource->jsWatchdog != nullptr);
  SS_ASSERT_HOTPATH(m_jsEngineForSource == nullptr || m_jsEngineForSource->jsWatchdog);

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
  SS_ASSERT_HOTPATH(m_compileGuard > 0);

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
 *        Expression transforms (spec 0060) read their own SlotTable, never the store.
 */
void DataModel::FrameBuilder::refreshDatasetCaptureFlag()
{
  static auto& parser       = DataModel::FrameParser::instance();
  const bool script_engines = m_transforms.hasScriptEngines();

  m_captureDatasetValues =
    !m_playerOpen && m_tableStore.isInitialized()
    && (script_engines || m_externalTableApiUsers || parser.hasTableApiEngines());
  static auto& projectModel = DataModel::ProjectModel::instance();
  m_changeDriven            = projectModel.changeDrivenTransforms();
  m_datasetDeps.clear();
  m_captureFlagsDirty = false;
}

/**
 * @brief Transform-only dataset pass for reprocessFrames(): re-applies transforms from the
 *        dataset's retained raw value so virtual datasets pick up current store contents
 *        without a device frame, honoring the live lanes' change-driven skips so a stateful
 *        transform is never double-invoked on an unchanged input. Returns true on any change.
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
  if (!m_transforms.empty())
    info.frameNumber = ++m_sourceFrameCounters[frame.sourceId];

  const bool armedWatchdog = beginDatasetPass(info);

  bool changed = false;
  for (auto& group : frame.groups) {
    for (auto& dataset : group.datasets) {
      if (dataset.transformCode.isEmpty())
        continue;

      // code-verify off
      // A channel-bound stream dataset already ran its transform on the stream worker; running
      // it again here would double-apply it to a value that is already post-transform.
      // code-verify on
      if (m_streamDatasetIds.contains(dataset.uniqueId))
        continue;

      DatasetDeps* dep = m_changeDriven ? &m_datasetDeps[dataset.uniqueId] : nullptr;
      if (dep && !dep->readSlots.empty()
          && !m_tableStore.changedSince(dep->readSlots, dep->lastRunClock))
        continue;

      if (dep && !dataset.virtual_ && dep->hasRun && dep->readSlots.empty())
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

      if (dep)
        m_tableStore.setReadCaptureTarget(&dep->readSlots);

      const auto result = applyTransform(dataset.transformLanguage, dataset.uniqueId, input, info);

      if (dep) {
        m_tableStore.setReadCaptureTarget(nullptr);
        dep->lastRunClock = m_tableStore.writeClock();
        dep->hasRun       = true;
      }

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

      if (m_exprEngineForSource) [[unlikely]]
        m_exprEngineForSource->exprSlots->publish(dataset.uniqueId, dataset.numericValue);

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
  SS_ASSERT_HOTPATH(spans != nullptr);
  SS_ASSERT_HOTPATH(count > 0);

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
  SS_ASSERT_HOTPATH(datasets != nullptr);
  SS_ASSERT_HOTPATH(spans != nullptr);

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
  SS_ASSERT_HOTPATH(data);
  SS_ASSERT_HOTPATH(!data->data.isEmpty());
  SS_ASSERT_HOTPATH(m_operationMode == SerialStudio::QuickPlot);

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
      m_quickPlot.setHeaders(channels);
      return;
    }
  }

  if (channelCount != m_quickPlotChannels) [[unlikely]] {
    invalidateFramePool();
    m_quickPlot.build(channels);
    m_quickPlotChannels = channelCount;
  }

  auto& quickPlotFrame    = m_quickPlot.frame();
  const auto* channelData = channels.constData();
  const size_t groupCount = quickPlotFrame.groups.size();
  for (size_t g = 0; g < groupCount; ++g) {
    auto& group               = quickPlotFrame.groups[g];
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

  stageFrameValues(quickPlotFrame.sourceId, quickPlotFrame, data->timestamp);
}

//--------------------------------------------------------------------------------------------------
// Hotpath data publishing functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishes one finished block: the dashboard gets the pooled slot, async sinks get ONE
 *        trimmed values-only copy between them -- a queued sink must never hold a pool slot or a
 *        backlog would starve staging. While the sink mask is set only the read-only observers
 *        see it, so a replay or a synthetic refresh can never re-record itself.
 */
void DataModel::FrameBuilder::publishBlock(const DataModel::DataBlockPtr& block)
{
  SS_ASSERT_HOTPATH(block);
  SS_ASSERT_HOTPATH(block->samples > 0);

  static auto& pipeline = IO::PipelineHost::instance();
  pipeline.publishBlockToDashboard(block);

  static auto& pluginsServer = API::Server::instance();
#ifdef ENABLE_GRPC
  static auto& grpcServer = API::GRPC::GRPCServer::instance();
#endif

  if (block->masked || m_maskSinks) [[unlikely]] {
    const bool observed = (pluginsServer.enabled() && pluginsServer.clientCount() > 0)
#ifdef ENABLE_GRPC
                       || (grpcServer.enabled() && grpcServer.clientCount() > 0)
#endif
      ;
    if (!observed)
      return;

    const DataModel::DataBlockPtr replayed = DataModel::clone_block_trimmed(*block);
    if (pluginsServer.enabled() && pluginsServer.clientCount() > 0)
      pluginsServer.ingestBlock(replayed);
#ifdef ENABLE_GRPC
    if (grpcServer.enabled() && grpcServer.clientCount() > 0)
      grpcServer.ingestBlock(replayed);
#endif
    return;
  }

  if (!m_anyAsyncSink)
    return;

  const DataModel::DataBlockPtr detached = DataModel::clone_block_trimmed(*block);

  static auto& csvExport  = CSV::Export::instance();
  static auto& mdf4Export = MDF4::Export::instance();
#ifdef BUILD_COMMERCIAL
  static auto& sqliteExport  = Sessions::Export::instance();
  static auto& mqttPublisher = MQTT::Publisher::instance();
  static auto& audioExport   = Widgets::AudioExport::instance();
  static auto& influxExport  = InfluxDB::Export::instance();
#endif

  csvExport.ingestBlock(detached);
  mdf4Export.ingestBlock(detached);
  pluginsServer.ingestBlock(detached);
#ifdef BUILD_COMMERCIAL
  sqliteExport.ingestBlock(detached);
  mqttPublisher.ingestBlock(detached);
  audioExport.ingestBlock(detached);
  influxExport.ingestBlock(detached);
#endif
#ifdef ENABLE_GRPC
  grpcServer.ingestBlock(detached);
#endif
}

/**
 * @brief Publishes an already-decoded replay block with the recording sinks masked, announcing
 *        the source's structure first exactly like a live source's first block: a dense-only
 *        replay has no frame-lane traffic to publish the layout, and without a snapshot the
 *        dashboard never builds a widget (R11).
 */
void DataModel::FrameBuilder::replayBlock(const DataModel::DataBlockPtr& block)
{
  if (QThread::currentThread() != thread()) {
    invokeOnBuilderThread([this, block] { replayBlock(block); });
    return;
  }

  SS_ASSERT(block != nullptr, return);

  if (block->samples <= 0)
    return;

  if (!structureIsCurrent(block->sourceId) && m_operationMode == SerialStudio::ProjectFile
      && !m_frame.groups.empty()) {
    const DataModel::Frame& srcFrame = ensureSourceFrame(block->sourceId);
    if (!srcFrame.groups.empty())
      ensureStructurePublished(block->sourceId, srcFrame);
  }

  const bool previousMask = m_maskSinks;
  m_maskSinks             = true;

  publishBlock(block);

  m_maskSinks = previousMask;
}

/**
 * @brief Replay publish: stages one recorded row and flushes it immediately with the recording
 *        sinks masked, so a replayed session reaches the dashboard and the read-only observers but
 *        can never re-record itself into a new file or session.
 */
void DataModel::FrameBuilder::publishReplayValues(
  int sourceId, const DataModel::Frame& src, const DataModel::TimestampedFrame::SteadyTimePoint& ts)
{
  SS_ASSERT_HOTPATH(m_playerOpen);

  const bool previousMask = m_maskSinks;
  m_maskSinks             = true;

  // code-verify off
  // No per-row flush: replay batches to the same cap/epoch rules as the live lane. Per-row
  // single-sample blocks overflowed the 32-slot dashboard ring ~35x at replay rate (measured
  // 67k blocks/s vs ~1.9k/s drained), and the random survivors were almost never the sparse
  // sources' -- CAN gauges froze while audio moved. The mask rides on the block, so the display
  // tick's flushOpenBlocks() publishing this batch later cannot leak it into a recording sink.
  // code-verify on
  stageFrameValues(sourceId, src, ts);

  m_maskSinks = previousMask;
}

//--------------------------------------------------------------------------------------------------
// Per-dataset value transforms
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reconciles transform engines with playback state: compileTransforms() keeps engines
 *        down while a player is open (replay never runs a transform, and a live engine arms
 *        the watchdog + dataset mirroring per frame), and the pass guard defers teardown
 *        when a script opens a player synchronously via apiCall mid-pass.
 */
void DataModel::FrameBuilder::rebuildTransformsForPlayback()
{
  if (m_operationMode != SerialStudio::ProjectFile || m_frame.title.isEmpty())
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
  if (QThread::currentThread() != thread()) {
    invokeOnBuilderThreadBlocking([this, &map] { m_replayColumnMap = std::move(map); });
    return;
  }

  m_replayColumnMap = std::move(map);
}

/**
 * @brief Rebuilds every per-dataset transform engine. Defers while a frame is in flight
 * (m_compileGuard > 0), since mutating the engine map under a dataset pass dangles the hot
 * pointers cached from it, keeps the engines down while a player is open, and no-ops after
 * aboutToQuit because rebuilding a QJSEngine once QCoreApplication is gone is a qFatal at exit.
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
  SS_ASSERT_LOG(m_transforms.empty());

  if (m_playerOpen)
    return;

  m_transforms.compile();
}

/**
 * @brief Runs one GC pass over every per-source transform engine.
 */
void DataModel::FrameBuilder::collectTransformEngineGarbage()
{
  m_transforms.collectGarbage();
}

/**
 * @brief Drops the per-frame engine pointers, then destroys the engines themselves. The cached
 *        pointers are cleared FIRST: they alias entries of the map the compiler is about to
 *        erase, and the capture flag is re-derived because the engine set is an input to it.
 */
void DataModel::FrameBuilder::destroyTransformEngines()
{
  m_engineCacheSourceId = -1;
  m_luaEngineForSource  = nullptr;
  m_jsEngineForSource   = nullptr;
  m_exprEngineForSource = nullptr;
  m_captureFlagsDirty   = true;

  m_tableStore.clearLookupCache();
  m_transforms.destroy();

  SS_ASSERT_LOG(m_transforms.empty());
}

/**
 * @brief Destroys every transform engine on the thread that built them. A lua_State and a
 *        QJSEngine belong to their creating thread: a QJSEngine's collector is driven both
 *        synchronously and by posted events, so one surviving a thread move is swept from two
 *        threads at once and double-frees the identifier table.
 */
void DataModel::FrameBuilder::releaseTransformEngines()
{
  destroyTransformEngines();
}

/**
 * @brief Rebuilds the transform engines on the current thread, restoring what
 *        releaseTransformEngines() dropped ahead of a thread move.
 */
void DataModel::FrameBuilder::rebuildTransformEngines()
{
  compileTransforms();
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
  SS_ASSERT_HOTPATH(info.sourceId >= 0);
  SS_ASSERT_HOTPATH(uniqueId >= 0);
  SS_ASSERT_HOTPATH(info.sourceId == m_engineCacheSourceId);

  DataModel::TransformEngine* engine = nullptr;
  if (language == SerialStudio::Lua)
    engine = m_luaEngineForSource;
  else if (language == SerialStudio::Expression)
    engine = m_exprEngineForSource;
  else
    engine = m_jsEngineForSource;

  if (!engine)
    return rawValue;

  if (engine->luaState)
    return applyTransformLua(*engine, uniqueId, rawValue, info);

  if (engine->jsEngine)
    return applyTransformJs(*engine, uniqueId, rawValue, info);

  if (engine->exprSlots)
    return applyTransformExpr(*engine, uniqueId, rawValue, info);

  return rawValue;
}

/**
 * @brief Calls the cached Lua transform function for @p uniqueId under the per-call deadline.
 */
QVariant DataModel::FrameBuilder::applyTransformLua(DataModel::TransformEngine& engine,
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
      m_transforms.noteTransformError(uniqueId, lua_tostring(L, -1));
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
 * @brief Evaluates the compiled expression for @p uniqueId (spec 0060): a text input enters as
 *        NaN so an arithmetic expression over a non-numeric channel degrades instead of parsing
 *        garbage, and `t` is the source's own frame timestamp, never a re-stamp here.
 */
QVariant DataModel::FrameBuilder::applyTransformExpr(DataModel::TransformEngine& engine,
                                                     int uniqueId,
                                                     const QVariant& rawValue,
                                                     const TransformFrameInfo& info)
{
  auto refIt = engine.exprRefs.find(uniqueId);
  if (refIt == engine.exprRefs.end() || !engine.exprSlots)
    return rawValue;

  bool numeric   = false;
  const double v = SerialStudio::toDouble(rawValue, &numeric);
  const double t = static_cast<double>(info.timestampMs) * kMillisecondsToSeconds;
  return QVariant(refIt->second.run(
    numeric ? v : std::numeric_limits<double>::quiet_NaN(), t, *engine.exprSlots));
}

/**
 * @brief Calls the cached JS transform function for @p uniqueId under the watchdog timer, which is
 *        armed once per frame in beginDatasetPass rather than per call (unlike the Lua deadline).
 */
QVariant DataModel::FrameBuilder::applyTransformJs(DataModel::TransformEngine& engine,
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
    m_transforms.noteTransformError(uniqueId, "transform timed out");
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
    m_transforms.noteTransformError(uniqueId, message);
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
  m_tableApi.initializeStore(m_frame);
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
  if (QThread::currentThread() != thread()) {
    invokeOnBuilderThreadBlocking([this] { refreshTableStoreFromProjectModel(); });
    return;
  }

  static auto& pipeline = IO::PipelineHost::instance();

  const bool session_live = pipeline.pipelineConnected() || m_playerOpen;
  if (m_tableStore.isInitialized() && session_live)
    return;

  m_tableApi.refreshStoreFromProject();
  m_captureFlagsDirty = true;
}

/**
 * @brief Injects the Lua table API into @p L, arming the capture flags first: the injected
 *        engine can read dataset values back out of the store, so the mirror that fills them
 *        must be on before it runs. Compiled transform engines reach this through the compiler's
 *        installer, so their sandboxes take the same path as an external script's.
 */
void DataModel::FrameBuilder::injectTableApiLua(lua_State* L)
{
  SS_ASSERT(L, return);

  invokeOnBuilderThreadBlocking([this] {
    m_externalTableApiUsers = true;
    m_captureFlagsDirty     = true;
  });

  noteGuiTableApiUser();
  m_tableApi.installLua(L);
}

/**
 * @brief Installs the __ss table-API bridge; the SDK prelude exposes the friendly globals.
 */
void DataModel::FrameBuilder::injectTableApiJS(QJSEngine* js)
{
  SS_ASSERT(js, return);

  invokeOnBuilderThreadBlocking([this] {
    m_externalTableApiUsers = true;
    m_captureFlagsDirty     = true;
  });

  noteGuiTableApiUser();
  m_tableApi.installJs(js);
}
