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

#include "AppState.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/Scripting/FrameParser.h"
#include "IO/ConnectionManager.h"
#include "IO/FrameReader.h"
#include "SessionContext.h"
#include "SSAssert.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Publisher.h"
#endif

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
IO::PipelineHost::PipelineHost()
  : m_thread(std::make_unique<QThread>())
  , m_abandoned(false)
  , m_frameBuilder(nullptr)
  , m_frameParser(nullptr)
  , m_paused(false)
  , m_connected(false)
  , m_operationMode(static_cast<int>(SerialStudio::ProjectFile))
  , m_dashboardAccepting(false)
  , m_dashboardDrops(0)
  , m_displayDrops(0)
  , m_dashboardRing(kDashboardRingSize)
{
  m_thread->setObjectName(QStringLiteral("FramePipeline"));
  m_thread->start();
}

/**
 * @brief Joins the processing thread if no earlier teardown path already did (early-exit paths
 *        that never reach ModuleManager::onQuit), so the SessionContext release order can free
 *        FrameBuilder afterwards without a live pipeline thread touching it.
 */
IO::PipelineHost::~PipelineHost()
{
  shutdown();
}

/**
 * @brief Returns this session's pipeline host. The object is owned by the SessionContext and built
 *        by the composition root, so a reach before adoption is a named fatal instead of an
 *        out-of-order lazy construction (spec 0039 pattern).
 */
IO::PipelineHost& IO::PipelineHost::instance()
{
  return SessionContext::current().pipelineHost();
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
 * @brief Frames the dashboard never rendered: producer-side ring-full drops plus the GUI drain's
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
  return kDashboardRingSize;
}

/**
 * @brief Wires the GUI-side transition signals into the atomic mirrors and seeds their initial
 *        values. All writes happen on the GUI thread at transition rate; the frame path only
 *        ever reads the atomics.
 */
void IO::PipelineHost::setupExternalConnections()
{
  auto& appState  = AppState::instance();
  auto& ioManager = IO::ConnectionManager::instance();

  m_frameBuilder = &DataModel::FrameBuilder::instance();
  m_frameParser  = &DataModel::FrameParser::instance();

  connect(&appState, &AppState::operationModeChanged, this, [this, &appState] {
    m_operationMode.store(static_cast<int>(appState.operationMode()), std::memory_order_relaxed);
  });
  connect(&ioManager, &IO::ConnectionManager::connectedChanged, this, [this, &ioManager] {
    m_connected.store(ioManager.isConnected(), std::memory_order_relaxed);
  });
  connect(&ioManager, &IO::ConnectionManager::pausedChanged, this, [this, &ioManager] {
    m_paused.store(ioManager.paused(), std::memory_order_relaxed);
  });

  m_paused.store(ioManager.paused(), std::memory_order_relaxed);
  m_connected.store(ioManager.isConnected(), std::memory_order_relaxed);
  m_operationMode.store(static_cast<int>(appState.operationMode()), std::memory_order_relaxed);
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
 * @brief Moves the FrameBuilder and FrameParser onto the processing thread. Called from the
 *        composition root as the LAST wiring step so every startup call into them stays a plain
 *        same-thread call; only steady-state traffic crosses threads afterwards (spec 0051 M3).
 */
void IO::PipelineHost::relocateProcessingObjects()
{
  moveProcessingObjectsTo(m_thread.get());
}

/**
 * @brief Hands the FrameBuilder and FrameParser to @p target's thread, dropping every script
 *        engine first (on their current owner): a lua_State and a QJSEngine belong to the thread
 *        that built them, and parsing through an engine born elsewhere corrupts the QV4 heap.
 *        readCode() rebuilds them on the new owner; a stopped target thread is refused.
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

  runOnObjectThread(m_frameParser, [this, target] {
    m_frameParser->releaseEngines();
    m_frameParser->moveToThread(target);
    m_frameBuilder->moveToThread(target);
  });

  QMetaObject::invokeMethod(m_frameParser, &DataModel::FrameParser::readCode);
}

/**
 * @brief Drains a reader's SPSC queue on the processing thread and routes each frame into
 *        FrameBuilder by the mirrored operation mode (relocated from
 *        ConnectionManager::onFrameReady). Paused sessions still drain so the queue never
 *        backs up into the reader's slot pool.
 */
void IO::PipelineHost::routeFrames(int deviceId, FrameReader* reader)
{
  SS_ASSERT(reader != nullptr, return);
  SS_ASSERT(deviceId >= 0, return);

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
#ifdef BUILD_COMMERCIAL
  static auto& mqttPublisher = MQTT::Publisher::instance();
#endif

  const bool paused = m_paused.load(std::memory_order_relaxed);
  const auto mode   = operationMode();

  auto& queue = reader->queue();
  IO::CapturedDataPtr frame;
  while (queue.try_dequeue(frame)) {
    if (paused) [[unlikely]]
      continue;

    if (mode == SerialStudio::ProjectFile)
      frameBuilder.hotpathRxSourceFrame(deviceId, frame);
    else
      frameBuilder.hotpathRxFrame(frame);

#ifdef BUILD_COMMERCIAL
    mqttPublisher.hotpathTxRawFrame(deviceId, frame);
#endif
  }
}

//--------------------------------------------------------------------------------------------------
// Dashboard hand-off ring
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enqueues a finished pooled frame for the GUI drain (producer: processing thread only).
 *        A full ring means the GUI stalled long enough to pin kDashboardRingSize slots; the frame
 *        is dropped and counted, and the pool-exhaustion warning carries the user-facing signal.
 */
void IO::PipelineHost::publishFrameToDashboard(const DataModel::TimestampedFramePtr& frame)
{
  SS_ASSERT(frame != nullptr, return);

  if (!m_dashboardAccepting.load(std::memory_order_relaxed))
    return;

  if (!m_dashboardRing.try_enqueue(frame)) [[unlikely]]
    ++m_dashboardDrops;
}

/**
 * @brief Pops one pending dashboard frame (consumer: GUI thread only, on the display tick).
 */
bool IO::PipelineHost::dequeueDashboardFrame(DataModel::TimestampedFramePtr& out)
{
  return m_dashboardRing.try_dequeue(out);
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
 * @brief Adds the GUI drain's over-budget discards to the pulled drop total (consumer side, GUI
 *        thread only; its own word so the producer's counter stays free of cross-thread writes).
 */
void IO::PipelineHost::noteDisplayDrops(quint64 count) noexcept
{
  m_displayDrops += count;
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
 * @brief Brackets the pipeline thread's blocking apiCall dispatch (set by ScriptApiCall only).
 */
void IO::PipelineHost::setPipelineParkedOnGui(bool parked) noexcept
{
  s_pipelineParkedOnGui.store(parked, std::memory_order_release);
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
 * @brief Joins the processing thread with a bounded wait: FrameBuilder tears its script engines
 *        down on the thread first (queued ahead of quit), and a hung Fast-mode script trips the
 *        warn-and-abandon path instead of blocking quit forever (spec 0051 R21, 0046 precedent).
 *        Idempotent for the destructor's repeat call.
 */
void IO::PipelineHost::shutdown()
{
  constexpr int kJoinTimeoutMs = 5000;

  beginTeardown();

  if (!m_thread || !m_thread->isRunning())
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
    qWarning() << "[PipelineHost] processing thread did not stop within" << kJoinTimeoutMs
               << "ms (hung script?) -- abandoning it (R21)";
  }
}
