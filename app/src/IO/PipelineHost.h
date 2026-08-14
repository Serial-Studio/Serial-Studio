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
#include <QObject>
#include <QThread>
#include <utility>

#include "DataModel/Frame.h"
#include "SerialStudio.h"
#include "ThirdParty/readerwriterqueue.h"

class SessionContext;

namespace DataModel {
class FrameBuilder;
class FrameParser;
}  // namespace DataModel

namespace IO {

class FrameReader;

/**
 * @brief Owner of the frame-processing thread (spec 0051 M3): FrameReaders, FrameParser engines
 *        and FrameBuilder execute here, off the GUI thread. Routes extracted frames into
 *        FrameBuilder (relocated from ConnectionManager::onFrameReady) and carries the finished
 *        pooled frames back to the GUI through an SPSC ring drained on the UI display tick.
 */
class PipelineHost : public QObject {
  Q_OBJECT

private:
  friend class ::SessionContext;
  explicit PipelineHost();
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

  void registerFrameReader(int deviceId, FrameReader* reader);
  void relocateProcessingObjects();
  void moveProcessingObjectsTo(QThread* target);

  void publishFrameToDashboard(const DataModel::TimestampedFramePtr& frame);
  [[nodiscard]] bool dequeueDashboardFrame(DataModel::TimestampedFramePtr& out);
  void setDashboardAccepting(bool accepting) noexcept;
  void noteDisplayDrops(quint64 count) noexcept;

  void shutdown();

  [[nodiscard]] static bool pipelineParkedOnGui() noexcept;
  static void setPipelineParkedOnGui(bool parked) noexcept;

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

    QEventLoop loop;
    QMetaObject::invokeMethod(
      target,
      [&fn, &loop] {
        fn();
        QMetaObject::invokeMethod(&loop, &QEventLoop::quit, Qt::QueuedConnection);
      },
      Qt::QueuedConnection);
    loop.exec();
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

private:
  void routeFrames(int deviceId, FrameReader* reader);

private:
  static constexpr int kDashboardRingSize = 8192;

  std::unique_ptr<QThread> m_thread;
  bool m_abandoned;
  DataModel::FrameBuilder* m_frameBuilder;
  DataModel::FrameParser* m_frameParser;
  alignas(64) std::atomic<bool> m_paused;
  alignas(64) std::atomic<bool> m_connected;
  alignas(64) std::atomic<int> m_operationMode;
  alignas(64) std::atomic<bool> m_dashboardAccepting;
  alignas(64) quint64 m_dashboardDrops;
  alignas(64) quint64 m_displayDrops;
  moodycamel::ReaderWriterQueue<DataModel::TimestampedFramePtr> m_dashboardRing;
};

}  // namespace IO
