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

#pragma once

#include <atomic>
#include <memory>
#include <QCoreApplication>
#include <QEventLoop>
#include <QMutex>
#include <QObject>
#include <QPointer>
#include <QThread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Core/DataModel/DataBlock.h"
#include "Core/DataModel/Frame.h"
#include "Core/IO/IIngestBinder.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "Core/ThirdParty/readerwriterqueue.h"
#include "IO/FrameReader.h"

namespace Core::Bus {
class MessageBus;
}  // namespace Core::Bus

class SessionContext;

namespace DataModel {
class FrameBuilder;
class FrameParser;
}  // namespace DataModel

namespace IO {

class IRawFrameTap;
class StreamWorker;
class StreamWorkerPool;

/**
 * @brief Refcounted state for one runOnObjectThread dispatch, so the queued lambda owns what it
 *        touches instead of pointing at the waiter's stack. dispatch() runs the functor only
 *        while a loop is published and abandon() blocks behind it, so the waiter can never
 *        unwind out from under a call in progress, and a late dispatch does nothing.
 */
template<typename Fn>
class MarshalCall {
public:
  template<typename F>
  explicit MarshalCall(F&& fn) : m_fn(std::forward<F>(fn)), m_loop(nullptr)
  {}

  MarshalCall(MarshalCall&&)                 = delete;
  MarshalCall(const MarshalCall&)            = delete;
  MarshalCall& operator=(MarshalCall&&)      = delete;
  MarshalCall& operator=(const MarshalCall&) = delete;

  /**
   * @brief Publishes the waiter's loop before the dispatch is posted.
   */
  void publish(QEventLoop* loop)
  {
    SS_ASSERT(loop != nullptr, return);

    const QMutexLocker locker(&m_mutex);
    m_loop = loop;
  }

  /**
   * @brief Runs the functor on the target thread and wakes the waiter, unless it already left.
   */
  void dispatch()
  {
    const QMutexLocker locker(&m_mutex);
    if (!m_loop)
      return;

    m_fn();
    QMetaObject::invokeMethod(m_loop, &QEventLoop::quit, Qt::QueuedConnection);
  }

  /**
   * @brief Retires the loop once the waiter stops waiting, blocking behind an in-flight dispatch.
   */
  void abandon()
  {
    const QMutexLocker locker(&m_mutex);
    m_loop = nullptr;
  }

private:
  Fn m_fn;
  QMutex m_mutex;
  QEventLoop* m_loop;
};

/**
 * @brief Owner of the frame-processing thread (spec 0051 M3): FrameReaders, FrameParser engines
 *        and FrameBuilder execute here, off the GUI thread. Implements the ingest binder the device
 *        layer talks to (spec 0077): attach() creates, configures and adopts a driver's reader, the
 *        dense-lane workers live in the pool it owns, and frames route into FrameBuilder from here.
 */
class PipelineHost
  : public QObject
  , public IIngestBinder {
  Q_OBJECT

signals:
  void parkedOnGuiChanged(bool parked);

private:
  friend class ::SessionContext;

  static void bindInstance(PipelineHost* instance) noexcept { s_instance = instance; }

  static PipelineHost* s_instance;
  explicit PipelineHost(Core::Bus::MessageBus& bus);
  PipelineHost(PipelineHost&&)                 = delete;
  PipelineHost(const PipelineHost&)            = delete;
  PipelineHost& operator=(PipelineHost&&)      = delete;
  PipelineHost& operator=(const PipelineHost&) = delete;

public:
  ~PipelineHost() override;

  [[nodiscard]] static PipelineHost& instance();

  [[nodiscard]] QThread* pipelineThread() const noexcept;
  [[nodiscard]] bool pipelineConnected() const noexcept;
  [[nodiscard]] bool paused() const noexcept;
  [[nodiscard]] SerialStudio::OperationMode operationMode() const noexcept;
  [[nodiscard]] quint64 dashboardDropCount() const noexcept;
  [[nodiscard]] int dashboardRingCapacity() const noexcept;
  [[nodiscard]] const std::vector<std::unique_ptr<StreamWorker>>& streamWorkers() const noexcept;

  void refreshLinkMirror(bool connected, bool paused) noexcept;
  void refreshOperationModeMirror(int mode) noexcept;
  void setRawFrameTap(IRawFrameTap* tap) noexcept;
  void bindFrameBuilder(DataModel::FrameBuilder& frameBuilder);

  void attach(int deviceId, HAL_Driver* driver, const FrameConfig& config) override;
  void reconfigure(int deviceId, const FrameConfig& config) override;
  void detach(int deviceId) override;
  void rebuildStreams(const std::vector<StreamAttachment>& sources,
                      bool paused,
                      bool connected) override;
  void setStreamPaused(bool paused) override;
  void publishStreamTemplates() override;
  void detachStreams() override;
  void injectPayload(int sourceId, const CapturedDataPtr& payload) override;
  void resetQuickPlotHeaders() override;
  [[nodiscard]] LinkStats linkStats() const override;

  void registerFrameReader(int deviceId, FrameReader* reader);
  void registerIngestThread();
  void relocateProcessingObjects();
  void moveProcessingObjectsTo(QThread* target);

  void publishBlockToDashboard(const DataModel::DataBlockPtr& block);
  void publishStructureToDashboard(const DataModel::StructureSnapshotPtr& structure);
  [[nodiscard]] bool dequeueDashboardBlock(DataModel::DataBlockPtr& out);
  [[nodiscard]] bool dequeueStructureSnapshot(DataModel::StructureSnapshotPtr& out);
  void setDashboardAccepting(bool accepting) noexcept;
  [[nodiscard]] bool dashboardAccepting() const noexcept;
  void noteDisplayDrops(quint64 count) noexcept;

  void bumpFlushEpoch() noexcept;
  [[nodiscard]] quint64 flushEpoch() const noexcept;

  void shutdown();

  [[nodiscard]] static bool pipelineParkedOnGui() noexcept;
  static void setPipelineParkedOnGui(bool parked);

  [[nodiscard]] static bool tearingDown() noexcept;
  static void beginTeardown() noexcept;

  [[nodiscard]] bool pipelineAbandoned() const noexcept;

  /**
   * @brief Runs @p fn on @p target's owning thread and waits: direct when already there or when
   *        the pipeline is parked in an apiCall dispatch the GUI is serving (state quiescent,
   *        today's mid-frame semantics); otherwise queued behind a local event loop so a GUI
   *        caller keeps serving the pipeline's blocking dispatches (deadlock-free).
   */
  template<typename Fn>
  static void runOnObjectThread(QObject* target, Fn&& fn)
  {
    if (QThread::currentThread() == target->thread()) {
      fn();
      return;
    }

    // code-verify off
    // Crash class, kept verbatim: a nested loop during teardown dispatches posted events into
    // half-destroyed objects (seen inside a queued export slot) and the target thread may
    // already be gone; skipping is safe, every teardown-time caller touches dying state.
    // code-verify on
    if (tearingDown())
      return;

    if (pipelineParkedOnGui() && QThread::currentThread() == qApp->thread()) {
      fn();
      return;
    }

    // code-verify off
    // Crash class, kept verbatim: the dispatch may outlive this frame. QThread::quit() unwinds
    // NESTED event loops too, so exec() can return with the post still queued; a by-reference
    // capture would then run off a dead stack (SIGSEGV on the pipeline thread, 2026-08-15). The
    // waiter publishes its loop under the mutex and clears it on the way out, so a late dispatch
    // sees no loop and skips fn entirely rather than touching the abandoned frame.
    // code-verify on
    auto call = std::make_shared<MarshalCall<std::decay_t<Fn>>>(std::forward<Fn>(fn));

    QEventLoop loop;
    call->publish(&loop);

    QMetaObject::invokeMethod(target, [call] { call->dispatch(); }, Qt::QueuedConnection);
    loop.exec();
    call->abandon();
  }

  /**
   * @brief Runs @p fn on the GUI thread and waits (plain blocking): the way pipeline-thread code
   *        snapshots GUI-owned state (ProjectModel). Deadlock-free by protocol -- GUI-side waits
   *        always use runOnObjectThread's event loop, which keeps serving these dispatches, so
   *        the GUI is never parked while the pipeline blocks on it.
   */
  template<typename Fn>
  static void runOnGuiThreadBlocking(Fn&& fn)
  {
    if (QThread::currentThread() == qApp->thread()) {
      fn();
      return;
    }

    // code-verify off
    // Crash class, kept verbatim: after quit the GUI stops pumping, so this blocks forever, the
    // pipeline misses its join deadline, the thread is abandoned, and shutdown frees modules it
    // still uses. Callers get stale state, which is what teardown wants.
    // code-verify on
    if (tearingDown())
      return;

    QMetaObject::invokeMethod(qApp, std::forward<Fn>(fn), Qt::BlockingQueuedConnection);
  }

public slots:
  void setupExternalConnections();

public:
  // Each queued block pins a pool slot: must stay well under BlockStager::kBlockPoolSlots
  static constexpr int kBlockRingSize = 32;

private:
  /**
   * @brief One attached device: the driver whose chunks feed the reader, the reader itself (owned
   *        here, living on the processing thread) and the feed connection retired with it.
   */
  struct ReaderSlot {
    HAL_Driver* driver = nullptr;
    QPointer<FrameReader> reader;
    QMetaObject::Connection feed;
  };

  void routeFrames(int deviceId, FrameReader* reader);
  void retireReader(ReaderSlot& slot);

private:
  // Structure snapshots are published on layout change only, never at frame or block rate
  static constexpr int kStructureRingSize = 32;

  Core::Bus::MessageBus& m_bus;
  std::unique_ptr<QThread> m_thread;
  bool m_abandoned;
  DataModel::FrameBuilder* m_frameBuilder;
  DataModel::FrameParser* m_frameParser;
  std::atomic<IRawFrameTap*> m_rawFrameTap;
  alignas(64) std::atomic<bool> m_paused;
  alignas(64) std::atomic<bool> m_connected;
  alignas(64) std::atomic<int> m_operationMode;
  alignas(64) std::atomic<bool> m_dashboardAccepting;
  alignas(64) std::atomic<quint64> m_flushEpoch;
  alignas(64) quint64 m_dashboardDrops;
  alignas(64) quint64 m_displayDrops;
  moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr> m_dashboardRing;
  moodycamel::ReaderWriterQueue<DataModel::StructureSnapshotPtr> m_structureRing;
  std::unordered_map<int, ReaderSlot> m_readers;

  // Binds m_operationMode above; a pointer keeps the workers' script headers out of this header
  std::unique_ptr<StreamWorkerPool> m_streamPool;
};

}  // namespace IO
