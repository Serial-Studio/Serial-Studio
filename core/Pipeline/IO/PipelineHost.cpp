/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "IO/PipelineHost.h"

#include "Core/IO/IRawFrameTap.h"
#include "Core/ModuleConstruction.h"
#include "Core/SSAssert.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/Scripting/FrameParser.h"
#include "IO/FrameReader.h"
#include "IO/StreamWorkerPool.h"
#include "Platform/AppPlatform.h"

IO::PipelineHost* IO::PipelineHost::s_instance = nullptr;

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Process-wide parked flag for runOnObjectThread's quiescent fast path (one pipeline per process)
static std::atomic<bool> s_pipelineParkedOnGui{false};

// Latched at the first teardown step; from then on no marshal may block or spin an event loop
static std::atomic<bool> s_tearingDown{false};

//--------------------------------------------------------------------------------------------------
// Constructor, destructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the pipeline host and starts the processing thread. The constructor reaches
 *        no other session module (ctor-edge rule, spec 0001): mirrors are seeded later by
 *        setupExternalConnections(), and the thread starts with an empty event loop.
 */
IO::PipelineHost::PipelineHost(Core::Bus::MessageBus& bus)
  : m_bus(bus)
  , m_thread(std::make_unique<QThread>())
  , m_abandoned(false)
  , m_frameBuilder(nullptr)
  , m_frameParser(nullptr)
  , m_rawFrameTap(nullptr)
  , m_paused(false)
  , m_connected(false)
  , m_operationMode(static_cast<int>(SerialStudio::ProjectFile))
  , m_dashboardAccepting(false)
  , m_flushEpoch(0)
  , m_dashboardDrops(0)
  , m_displayDrops(0)
  , m_dashboardRing(kBlockRingSize)
  , m_structureRing(kStructureRingSize)
  , m_streamPool(std::make_unique<StreamWorkerPool>(m_operationMode))
{
  m_thread->setObjectName(QStringLiteral("FramePipeline"));
  m_thread->start();
}

/**
 * @brief Joins the processing thread if no earlier teardown path already did (early-exit paths
 *        that never reach ModuleManager::onQuit), so the SessionContext release order can free
 *        FrameBuilder afterwards without a live pipeline thread touching it. Readers a device
 *        never detached die here too, after the thread that owned them.
 */
IO::PipelineHost::~PipelineHost()
{
  shutdown();

  for (auto& [deviceId, slot] : m_readers)
    retireReader(slot);

  m_readers.clear();
}

/**
 * @brief Returns this session's pipeline host, bound by the session context right after adoption;
 *        a reach before that is a named fatal (spec 0039 M2, spec 0077 T66).
 */
IO::PipelineHost& IO::PipelineHost::instance()
{
  SS_ASSERT(s_instance != nullptr, Core::ModuleConstruction::reportUnavailable(staticMetaObject));
  return *s_instance;
}

//--------------------------------------------------------------------------------------------------
// State mirrors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the processing thread FrameReaders, FrameParser and FrameBuilder live on.
 */
QThread* IO::PipelineHost::pipelineThread() const noexcept
{
  return m_thread.get();
}

/**
 * @brief Lock-free connected mirror for worker threads (replaces the MDF4 worker's cross-thread
 *        ConnectionManager::isConnected() read, spec 0051 T17).
 */
bool IO::PipelineHost::pipelineConnected() const noexcept
{
  return m_connected.load(std::memory_order_relaxed);
}

/**
 * @brief Lock-free pause mirror read by the frame router on the processing thread.
 */
bool IO::PipelineHost::paused() const noexcept
{
  return m_paused.load(std::memory_order_relaxed);
}

/**
 * @brief Lock-free operation-mode mirror read by the frame router on the processing thread.
 */
SerialStudio::OperationMode IO::PipelineHost::operationMode() const noexcept
{
  return static_cast<SerialStudio::OperationMode>(m_operationMode.load(std::memory_order_relaxed));
}

/**
 * @brief Blocks the dashboard never rendered: producer-side ring-full drops plus the GUI drain's
 *        over-budget discards, each accumulated in its own word by its own thread. Plain counters
 *        pulled by diagnostics, never pushed (spec 0033).
 */
quint64 IO::PipelineHost::dashboardDropCount() const noexcept
{
  return m_dashboardDrops + m_displayDrops;
}

/**
 * @brief Capacity of the dashboard hand-off ring. The GUI drain takes this as its hard per-tick
 *        dequeue bound, so a producer that outruns the display can never hold the GUI thread
 *        inside one display tick.
 */
int IO::PipelineHost::dashboardRingCapacity() const noexcept
{
  return kBlockRingSize;
}

/**
 * @brief Returns the live dense-lane workers (GUI thread only; Dashboard drains their display
 *        rings on the display tick, the API lists them).
 */
const std::vector<std::unique_ptr<IO::StreamWorker>>& IO::PipelineHost::streamWorkers()
  const noexcept
{
  return m_streamPool->workers();
}

/**
 * @brief Adopts the frame builder the binder feeds and hands it to the stream pool. Bound by the
 *        composition root before any wiring pass: the connection manager's own wiring already
 *        rebuilds the stream workers, which is earlier than this host's setupExternalConnections().
 */
void IO::PipelineHost::bindFrameBuilder(DataModel::FrameBuilder& frameBuilder)
{
  SS_ASSERT_LOG(m_frameBuilder == nullptr || m_frameBuilder == &frameBuilder);
  m_frameBuilder = &frameBuilder;
  m_streamPool->bind(frameBuilder);
}

/**
 * @brief Captures the frame parser (the builder was bound by the root) so the thread move and the
 *        shutdown reach both pipeline-thread peers. The atomic mirrors the frame path reads are
 *        written through refreshLinkMirror() and refreshOperationModeMirror(), which the frame
 *        builder's external wiring drives from the bus directly, at transition rate.
 */
void IO::PipelineHost::setupExternalConnections()
{
  bindFrameBuilder(DataModel::FrameBuilder::instance());
  m_frameParser = &DataModel::FrameParser::instance();
}

/**
 * @brief Stores the link state the frame path consults; GUI thread, transition rate.
 */
void IO::PipelineHost::refreshLinkMirror(const bool connected, const bool paused) noexcept
{
  m_paused.store(paused, std::memory_order_relaxed);
  m_connected.store(connected, std::memory_order_relaxed);
}

/**
 * @brief Stores the operation mode the frame path consults; GUI thread, transition rate.
 */
void IO::PipelineHost::refreshOperationModeMirror(const int mode) noexcept
{
  SS_ASSERT_LOG(mode >= SerialStudio::ProjectFile && mode <= SerialStudio::QuickPlot);
  m_operationMode.store(mode, std::memory_order_relaxed);
}

/**
 * @brief Binds the one observer of every extracted frame (spec 0077 T52), bound once by the
 *        composition root before the first device opens; null in a root without one.
 */
void IO::PipelineHost::setRawFrameTap(IRawFrameTap* tap) noexcept
{
  SS_ASSERT_LOG(m_rawFrameTap.load(std::memory_order_relaxed) == nullptr || tap == nullptr);
  m_rawFrameTap.store(tap, std::memory_order_relaxed);
}

//--------------------------------------------------------------------------------------------------
// Ingest binder: readers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a FrameReader for @p deviceId, configures it on this thread while it has no live
 *        connection, feeds it the driver's chunks (Auto: queued once the reader has moved) and
 *        adopts it onto the processing thread. An earlier reader for the id is retired first, so
 *        this is also how a device is reconfigured: recreate, never lock (FrameReader is SPSC).
 */
void IO::PipelineHost::attach(int deviceId, HAL_Driver* driver, const FrameConfig& config)
{
  SS_ASSERT(deviceId >= 0, return);
  SS_ASSERT(driver != nullptr, return);

  detach(deviceId);

  auto* reader = new FrameReader();
  reader->setChecksum(config.checksumAlgorithm);
  reader->setStartSequences(config.startSequences);
  reader->setFinishSequences(config.finishSequences);
  reader->setOperationMode(config.operationMode);
  reader->setFrameDetectionMode(config.frameDetection);

  ReaderSlot slot;
  slot.driver = driver;
  slot.reader = reader;
  slot.feed   = connect(driver, &HAL_Driver::dataReceived, reader, &FrameReader::processData);

  registerFrameReader(deviceId, reader);
  m_readers.insert_or_assign(deviceId, std::move(slot));
}

/**
 * @brief Recreates @p deviceId's reader with @p config, fed by the driver attach() bound. A device
 *        that was never attached is an ordinary miss: the connection layer attaches on open.
 */
void IO::PipelineHost::reconfigure(int deviceId, const FrameConfig& config)
{
  SS_ASSERT(deviceId >= 0, return);

  const auto it = m_readers.find(deviceId);
  if (it == m_readers.end())
    return;

  HAL_Driver* driver = it->second.driver;
  attach(deviceId, driver, config);
}

/**
 * @brief Drops @p deviceId's driver feed and retires its reader; idempotent.
 */
void IO::PipelineHost::detach(int deviceId)
{
  SS_ASSERT(deviceId >= 0, return);

  const auto it = m_readers.find(deviceId);
  if (it == m_readers.end())
    return;

  retireReader(it->second);
  m_readers.erase(it);
}

/**
 * @brief Disconnects the feed by handle rather than by a wildcard slot and destroys the reader:
 *        deleteLater() on the processing loop while it runs, outright once that thread has
 *        stopped, because a post into a dead loop would never free the reader at all. An abandoned
 *        thread (R21) may still be draining the reader, so that path leaks it like the modules.
 */
void IO::PipelineHost::retireReader(ReaderSlot& slot)
{
  QObject::disconnect(slot.feed);
  slot.feed = QMetaObject::Connection();

  if (slot.reader.isNull())
    return;

  if (m_abandoned) {
    slot.reader.clear();
    return;
  }

  if (m_thread && m_thread->isRunning()) {
    slot.reader->deleteLater();
    slot.reader.clear();
    return;
  }

  delete slot.reader.data();
  slot.reader.clear();
}

/**
 * @brief Sums the per-device frame-reader counters for the 1 Hz diagnostics sample. No caching and
 *        no signal: this is pulled once per second and must never be called on the frame path.
 */
IO::LinkStats IO::PipelineHost::linkStats() const
{
  LinkStats stats{};
  for (const auto& [deviceId, slot] : m_readers) {
    const auto* reader = slot.reader.data();
    if (!reader)
      continue;

    stats.bytesIn         += reader->bytesReceived();
    stats.droppedFrames   += reader->droppedFrameCount();
    stats.overflowBytes   += reader->overflowBytes();
    stats.checksumErrors  += reader->checksumErrorCount();
    stats.framesExtracted += reader->framesExtracted();
  }

  return stats;
}

//--------------------------------------------------------------------------------------------------
// Ingest binder: stream lane & injected payloads
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the dense-lane workers from the sources whose lane is on (spec 0051); the pool
 *        keeps every worker's queued hop into the FrameBuilder, so the pipeline thread stays the
 *        single producer for every sink.
 */
void IO::PipelineHost::rebuildStreams(const std::vector<StreamAttachment>& sources,
                                      bool paused,
                                      bool connected)
{
  m_streamPool->rebuild(sources, paused, connected);
}

/**
 * @brief Mirrors the session pause onto every worker.
 */
void IO::PipelineHost::setStreamPaused(bool paused)
{
  m_streamPool->setPaused(paused);
}

/**
 * @brief Publishes the dashboard structure for every stream source on the connect edge.
 */
void IO::PipelineHost::publishStreamTemplates()
{
  m_streamPool->publishTemplates();
}

/**
 * @brief Stops and destroys every stream worker; idempotent, and the first teardown step.
 */
void IO::PipelineHost::detachStreams()
{
  m_streamPool->stop();
}

/**
 * @brief Feeds a pre-built payload into the frame pipeline (file players, the API, a control
 *        script) through the builder bound in setupExternalConnections(): queued to the processing
 *        thread at command rate, never per frame; the mode is read here so a project-mode payload
 *        lands in @p sourceId's parser.
 */
void IO::PipelineHost::injectPayload(int sourceId, const CapturedDataPtr& payload)
{
  SS_ASSERT(sourceId >= 0, return);
  SS_ASSERT(payload != nullptr, return);

  if (payload->data.isEmpty())
    return;

  SS_ASSERT(m_frameBuilder != nullptr, return);

  auto* builder          = m_frameBuilder;
  const bool projectMode = (operationMode() == SerialStudio::ProjectFile);
  builder->invokeOnBuilderThread([builder, sourceId, payload, projectMode] {
    if (projectMode)
      builder->hotpathRxSourceFrame(sourceId, payload);
    else
      builder->hotpathRxFrame(payload);
  });
}

/**
 * @brief Clears the Quick Plot channel headers on disconnect, so the next session names them.
 */
void IO::PipelineHost::resetQuickPlotHeaders()
{
  SS_ASSERT(m_frameBuilder != nullptr, return);

  m_frameBuilder->registerQuickPlotHeaders(QStringList());
}

//--------------------------------------------------------------------------------------------------
// FrameReader adoption & frame routing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Moves a freshly configured, parentless FrameReader onto the processing thread and wires
 *        its readyRead to the router. The connection is direct: emitter and router state both
 *        live on the processing thread, so the hop stays a plain call (65536-queue rule). The
 *        reader is the connection context, so the wiring dies with the reader on reconfigure.
 */
void IO::PipelineHost::registerFrameReader(int deviceId, FrameReader* reader)
{
  SS_ASSERT(reader != nullptr, return);
  SS_ASSERT(deviceId >= 0, return);
  SS_ASSERT(reader->parent() == nullptr, return);

  reader->moveToThread(m_thread.get());
  connect(
    reader,
    &IO::FrameReader::readyRead,
    reader,
    [this, deviceId, reader] { routeFrames(deviceId, reader); },
    Qt::DirectConnection);
}

/**
 * @brief Asks the processing thread to claim the real-time scheduling band (spec 0075, N2). The
 *        band is per thread and never inherited, so one queued startup post runs the registration
 *        inside the thread, through an object that already lives there. Nothing is posted while
 *        the pipeline still runs on the GUI thread -- boosting it is the defect being fixed.
 */
void IO::PipelineHost::registerIngestThread()
{
  SS_ASSERT(m_thread != nullptr, return);

  if (!m_frameBuilder || m_frameBuilder->thread() != m_thread.get())
    return;

  QMetaObject::invokeMethod(
    m_frameBuilder,
    [] { Platform::AppPlatform::registerIngestThreadWithMmcss(); },
    Qt::QueuedConnection);
}

/**
 * @brief Moves the FrameBuilder and FrameParser onto the processing thread. Called from the
 *        composition root as the LAST wiring step so every startup call into them stays a plain
 *        same-thread call; only steady-state traffic crosses threads afterwards (spec 0051 M3).
 */
void IO::PipelineHost::relocateProcessingObjects()
{
  moveProcessingObjectsTo(m_thread.get());
}

/**
 * @brief Hands the FrameBuilder and FrameParser to @p target's thread. BOTH drop their script
 *        engines first, on their current owner: a QJSEngine surviving the move is swept from its
 *        old thread by posted events and its new one synchronously, double-freeing the identifier
 *        table. readCode() and rebuildTransformEngines() rebuild on the new owner.
 */
void IO::PipelineHost::moveProcessingObjectsTo(QThread* target)
{
  SS_ASSERT(target != nullptr, return);
  SS_ASSERT(m_frameBuilder != nullptr, return);
  SS_ASSERT(m_frameParser != nullptr, return);

  if (m_frameParser->thread() == target)
    return;

  if (target != qApp->thread() && !target->isRunning())
    return;

  SS_ASSERT(m_frameBuilder->thread() == m_frameParser->thread(), return);

  runOnObjectThread(m_frameParser, [this, target] {
    m_frameParser->releaseEngines();
    m_frameBuilder->releaseTransformEngines();
    m_frameParser->moveToThread(target);
    m_frameBuilder->moveToThread(target);
  });

  QMetaObject::invokeMethod(m_frameParser, &DataModel::FrameParser::readCode);
  QMetaObject::invokeMethod(m_frameBuilder, &DataModel::FrameBuilder::rebuildTransformEngines);
}

/**
 * @brief Drains a reader's SPSC queue on the processing thread and routes each frame into
 *        FrameBuilder by the mirrored operation mode, then to the one raw-frame observer when a
 *        root bound one (pointer hoisted out of the loop: one test and one indirect call per
 *        frame). Paused sessions still drain so the queue never backs up into the reader's pool.
 */
void IO::PipelineHost::routeFrames(int deviceId, FrameReader* reader)
{
  SS_ASSERT(reader != nullptr, return);
  SS_ASSERT(deviceId >= 0, return);

  static auto& frameBuilder = DataModel::FrameBuilder::instance();

  const bool paused       = m_paused.load(std::memory_order_relaxed);
  const auto mode         = operationMode();
  IRawFrameTap* const tap = m_rawFrameTap.load(std::memory_order_relaxed);

  auto& queue = reader->queue();
  IO::CapturedDataPtr frame;
  while (queue.try_dequeue(frame)) {
    if (paused) [[unlikely]]
      continue;

    if (mode == SerialStudio::ProjectFile)
      frameBuilder.hotpathRxSourceFrame(deviceId, frame);
    else
      frameBuilder.hotpathRxFrame(frame);

    if (tap)
      tap->onRawFrame(deviceId, frame);
  }
}

//--------------------------------------------------------------------------------------------------
// Dashboard hand-off ring
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enqueues a finished pooled block for the GUI drain (producer: processing thread only).
 *        A full ring means the GUI stalled long enough to pin every block slot; the WHOLE block is
 *        dropped and counted -- never a partial hand-off, because a consumer that saw half a
 *        block's samples would interleave them with the next one's.
 */
void IO::PipelineHost::publishBlockToDashboard(const DataModel::DataBlockPtr& block)
{
  SS_ASSERT(block != nullptr, return);

  if (!m_dashboardAccepting.load(std::memory_order_relaxed))
    return;

  if (!m_dashboardRing.try_enqueue(block)) [[unlikely]]
    ++m_dashboardDrops;
}

/**
 * @brief Enqueues a structure snapshot ahead of the blocks that carry its generation (producer:
 *        processing thread only). Published on layout change only, so a full ring here means the
 *        GUI has not ticked across several project edits; the drop is counted like a block's.
 */
void IO::PipelineHost::publishStructureToDashboard(const DataModel::StructureSnapshotPtr& structure)
{
  SS_ASSERT(structure != nullptr, return);

  if (!m_dashboardAccepting.load(std::memory_order_relaxed))
    return;

  if (!m_structureRing.try_enqueue(structure)) [[unlikely]]
    ++m_dashboardDrops;
}

/**
 * @brief Pops one pending block (consumer: GUI thread only, on the display tick).
 */
bool IO::PipelineHost::dequeueDashboardBlock(DataModel::DataBlockPtr& out)
{
  return m_dashboardRing.try_dequeue(out);
}

/**
 * @brief Pops one pending structure snapshot (consumer: GUI thread only). Drained BEFORE the block
 *        ring on each tick, so a block never reaches the dashboard ahead of its layout.
 */
bool IO::PipelineHost::dequeueStructureSnapshot(DataModel::StructureSnapshotPtr& out)
{
  return m_structureRing.try_dequeue(out);
}

/**
 * @brief Mirrors Dashboard::streamAvailable() so a session with no dashboard consumer never
 *        pins pool slots in the ring (benchmark exporter tiers, ConsoleOnly, headless runs).
 *        Written by the Dashboard's cache refresh on the GUI thread.
 */
void IO::PipelineHost::setDashboardAccepting(bool accepting) noexcept
{
  m_dashboardAccepting.store(accepting, std::memory_order_relaxed);
}

/**
 * @brief The mirrored dashboard-accepting flag, read back by the Dashboard's cached-flag audit.
 */
bool IO::PipelineHost::dashboardAccepting() const noexcept
{
  return m_dashboardAccepting.load(std::memory_order_relaxed);
}

/**
 * @brief Adds the GUI drain's over-budget discards to the pulled drop total (consumer side, GUI
 *        thread only; its own word so the producer's counter stays free of cross-thread writes).
 */
void IO::PipelineHost::noteDisplayDrops(quint64 count) noexcept
{
  m_displayDrops += count;
}

/**
 * @brief Advances the block-flush epoch (spec 0055 D1). Written on the GUI thread from the display
 *        tick at transition rate, exactly like the mode/paused/connected mirrors. It is an epoch
 *        rather than a flag because every source compares independently: a consume-once flag would
 *        let the first source to notice it starve every other source's open block.
 */
void IO::PipelineHost::bumpFlushEpoch() noexcept
{
  m_flushEpoch.fetch_add(1, std::memory_order_relaxed);
}

/**
 * @brief Current flush epoch; a staging source pays one relaxed load per frame to compare against
 *        the epoch its open block was started in, and never a timer on the processing thread.
 */
quint64 IO::PipelineHost::flushEpoch() const noexcept
{
  return m_flushEpoch.load(std::memory_order_relaxed);
}

//--------------------------------------------------------------------------------------------------
// Cross-thread marshal support
//--------------------------------------------------------------------------------------------------

/**
 * @brief True while the pipeline thread is blocked waiting for a GUI-side apiCall dispatch.
 */
bool IO::PipelineHost::pipelineParkedOnGui() noexcept
{
  return s_pipelineParkedOnGui.load(std::memory_order_acquire);
}

/**
 * @brief Brackets the pipeline thread's blocking apiCall dispatch (set by ScriptApiCall only) and
 *        announces the edge on the pipeline thread. The bracket is the only moment a GUI-side
 *        marshal that ran inline against the parked pipeline can be handed back to it (A3), so a
 *        listener that deferred work while parked learns here that the pipeline is its own again.
 */
void IO::PipelineHost::setPipelineParkedOnGui(bool parked)
{
  s_pipelineParkedOnGui.store(parked, std::memory_order_release);

  if (tearingDown()) [[unlikely]]
    return;

  Q_EMIT instance().parkedOnGuiChanged(parked);
}

/**
 * @brief True once application teardown started: from that point the GUI thread has left its
 *        event loop, so every cross-thread marshal degrades to a no-op instead of blocking.
 */
bool IO::PipelineHost::tearingDown() noexcept
{
  return s_tearingDown.load(std::memory_order_acquire);
}

/**
 * @brief Latches teardown. Called as the FIRST statement of the quit path, before any module is
 *        stopped, so a marshal already being decided cannot slip past the latch.
 */
void IO::PipelineHost::beginTeardown() noexcept
{
  s_tearingDown.store(true, std::memory_order_release);
}

/**
 * @brief True when the join deadline expired and the processing thread was left running: the
 *        modules it may still touch must then be leaked rather than freed.
 */
bool IO::PipelineHost::pipelineAbandoned() const noexcept
{
  return m_abandoned;
}

//--------------------------------------------------------------------------------------------------
// Teardown
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stops the stream workers, then joins the processing thread with a bounded wait: engines
 *        tear down on the thread first, and a hung Fast-mode script trips warn-and-abandon instead
 *        of blocking quit (R21). The abandon latch carries idempotency, and releases the thread:
 *        its OS thread still runs.
 */
void IO::PipelineHost::shutdown()
{
  constexpr int kJoinTimeoutMs = 5000;

  beginTeardown();
  m_streamPool->stop();

  if (m_abandoned || !m_thread || !m_thread->isRunning())
    return;

  if (m_frameBuilder)
    QMetaObject::invokeMethod(
      m_frameBuilder, &DataModel::FrameBuilder::prepareShutdown, Qt::QueuedConnection);

  if (m_frameParser)
    QMetaObject::invokeMethod(
      m_frameParser, &DataModel::FrameParser::prepareShutdown, Qt::QueuedConnection);

  m_thread->quit();
  if (!m_thread->wait(kJoinTimeoutMs)) [[unlikely]] {
    m_abandoned = true;
    (void)m_thread.release();
    qWarning() << "[PipelineHost] processing thread did not stop within" << kJoinTimeoutMs
               << "ms (hung script?) -- abandoning it (R21)";
  }
}
