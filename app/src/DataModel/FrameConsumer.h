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

#pragma once

#include <atomic>
#include <chrono>
#include <QDebug>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <stdexcept>
#include <vector>

#include "ThirdParty/readerwriterqueue.h"

namespace DataModel {
/**
 * @brief Configuration parameters for frame consumers.
 *
 * This struct defines the tuning parameters for the threaded frame consumer
 * architecture. These values balance throughput, latency, and resource usage.
 */
struct FrameConsumerConfig {
  size_t queueCapacity  = 8192;
  size_t flushThreshold = 1024;
  int timerIntervalMs   = 1000;
};

/**
 * @class FrameConsumerWorkerBase
 * @brief Non-template Q_OBJECT base for frame consumer workers. Owns the
 *        monotonic-ns clock shared by CSV, MDF4 and Sessions export workers.
 */
class FrameConsumerWorkerBase : public QObject {
  Q_OBJECT

signals:
  void resourceOpenChanged();

public:
  explicit FrameConsumerWorkerBase(QObject* parent = nullptr);
  virtual ~FrameConsumerWorkerBase();

  qint64 monotonicFrameNs(std::chrono::steady_clock::time_point now,
                          std::chrono::steady_clock::time_point baseline);

  void resetMonotonicClock();

public slots:
  virtual void processData() = 0;
  virtual void close()       = 0;

private:
  qint64 m_lastFrameNs;
};

/**
 * @class FrameConsumerWorker
 * @brief Threaded worker that drains a lock-free queue of T into
 *        `processItems()` in batches. Subclasses implement `processItems()`,
 *        `closeResources()`, `isResourceOpen()`.
 *
 * @tparam T The type of items to process.
 */
template<typename T>
class FrameConsumerWorker : public FrameConsumerWorkerBase {
public:
  /**
   * @brief Constructs a frame consumer worker.
   *
   * @param queue Pointer to the lock-free queue shared with main thread
   * @param enabled Pointer to atomic flag controlling processing
   * @param queueSize Pointer to atomic counter tracking queue depth
   */
  FrameConsumerWorker(moodycamel::ReaderWriterQueue<T>* queue,
                      std::atomic<bool>* enabled,
                      std::atomic<size_t>* queueSize)
    : FrameConsumerWorkerBase(nullptr), m_queue(queue), m_enabled(enabled), m_queueSize(queueSize)
  {
    m_writeBuffer.reserve(kMaxItemsPerBatch);
  }

  ~FrameConsumerWorker() override = default;

  /**
   * @brief Processes all pending frames from the queue.
   *
   * This slot drains the queue into a local buffer, updates the queue size
   * counter, and delegates processing to the derived class via processItems().
   */
  void processData() override
  {
    if (!m_enabled->load(std::memory_order_relaxed))
      return;

    m_writeBuffer.clear();

    T item;
    size_t dequeued = 0;
    // code-verify off: m_writeBuffer is pre-reserved to kMaxItemsPerBatch in the constructor
    while (dequeued < kMaxItemsPerBatch && m_queue->try_dequeue(item)) {
      m_writeBuffer.push_back(std::move(item));
      ++dequeued;
    }
    // code-verify on

    if (dequeued >= kMaxItemsPerBatch) [[unlikely]]
      qWarning() << "[FrameConsumer] Batch size limit reached -- remaining items deferred";

    const auto count = m_writeBuffer.size();
    if (count == 0)
      return;

    m_queueSize->fetch_sub(count, std::memory_order_relaxed);

    const bool wasOpen = isResourceOpen();
    try {
      processItems(m_writeBuffer);
    } catch (const std::exception& e) {
      qWarning() << "[FrameConsumer] Exception in processItems:" << e.what();
    } catch (...) {
      qWarning() << "[FrameConsumer] Unknown exception in processItems";
    }

    const bool isOpen = isResourceOpen();

    if (wasOpen != isOpen)
      Q_EMIT resourceOpenChanged();
  }

  /**
   * @brief Flushes remaining data and closes resources.
   */
  void close() override
  {
    // Guard against exceptions - this method runs through Qt's event loop.
    try {
      processData();
      closeResources();
    } catch (const std::exception& e) {
      qWarning() << "[FrameConsumer] Exception during close:" << e.what();
    } catch (...) {
      qWarning() << "[FrameConsumer] Unknown exception during close";
    }

    Q_EMIT resourceOpenChanged();
  }

protected:
  /**
   * @brief Processes a batch of dequeued items.
   *
   * @param items Vector of items drained from the queue
   */
  virtual void processItems(const std::vector<T>& items) = 0;

  /**
   * @brief Closes all resources managed by this worker.
   */
  virtual void closeResources() = 0;

  /**
   * @brief Checks if resources are currently open.
   *
   * @return true if resources are open, false otherwise
   */
  [[nodiscard]] virtual bool isResourceOpen() const = 0;

  /**
   * @brief Returns whether processing is currently enabled.
   */
  [[nodiscard]] bool consumerEnabled() const noexcept
  {
    return m_enabled && m_enabled->load(std::memory_order_relaxed);
  }

private:
  static constexpr size_t kMaxItemsPerBatch = 10000;
  std::vector<T> m_writeBuffer;
  moodycamel::ReaderWriterQueue<T>* m_queue;
  std::atomic<bool>* m_enabled;
  std::atomic<size_t>* m_queueSize;
};

/**
 * @class FrameConsumer
 * @brief Main-thread facade that owns a `FrameConsumerWorker` on a dedicated
 *        QThread. Provides a lock-free `enqueueData()` hotpath with periodic +
 *        threshold-based flushing.
 *
 * @tparam T The type of items to process.
 */
template<typename T>
class FrameConsumer : public QObject {
public:
  /**
   * @brief Constructs a frame consumer with the given configuration.
   *
   * @param config Configuration parameters for queue and timer
   */
  explicit FrameConsumer(const FrameConsumerConfig& config = {})
    : m_config(config)
    , m_pendingQueue(config.queueCapacity)
    , m_consumerEnabled(true)
    , m_queueSize(0)
    , m_worker(nullptr)
  {}

  /**
   * @brief Initializes the worker thread and timer.
   */
  void initializeWorker()
  {
    m_worker = createWorker();
    m_worker->moveToThread(&m_workerThread);

    QObject::connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_workerThread.start();

    auto* timer = new QTimer();
    timer->setInterval(m_config.timerIntervalMs);
    timer->setTimerType(Qt::PreciseTimer);
    timer->moveToThread(&m_workerThread);

    QObject::connect(timer, &QTimer::timeout, m_worker, &FrameConsumerWorkerBase::processData);
    QObject::connect(&m_workerThread, &QThread::finished, timer, &QObject::deleteLater);

    QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);
  }

  /**
   * @brief Destructor ensures all data is flushed before shutdown.
   */
  virtual ~FrameConsumer()
  {
    if (m_worker) {
      QMetaObject::invokeMethod(
        m_worker, &FrameConsumerWorkerBase::close, Qt::BlockingQueuedConnection);

      m_workerThread.quit();
      m_workerThread.wait();
    }
  }

  /**
   * @brief Checks if the consumer is currently enabled.
   *
   * @return true if processing is enabled, false if paused
   */
  [[nodiscard]] bool consumerEnabled() const
  {
    return m_consumerEnabled.load(std::memory_order_relaxed);
  }

  /**
   * @brief Enables or disables frame processing.
   *
   * @param enabled true to enable processing, false to pause
   */
  void setConsumerEnabled(const bool enabled)
  {
    m_consumerEnabled.store(enabled, std::memory_order_relaxed);
  }

protected:
  /**
   * @brief Factory method to create the worker object.
   *
   * @return Pointer to newly created worker
   */
  virtual FrameConsumerWorkerBase* createWorker() = 0;

  /**
   * @brief Enqueues a data item for processing on the worker thread.
   *
   * This method is lock-free and safe to call from the main-thread hotpath.
   *
   * @param item The item to enqueue
   */
  void enqueueData(const T& item)
  {
    if (!m_consumerEnabled.load(std::memory_order_relaxed))
      return;

    if (m_pendingQueue.try_enqueue(item)) {
      const auto size = m_queueSize.fetch_add(1, std::memory_order_relaxed) + 1;
      if (size >= m_config.flushThreshold) {
        QMetaObject::invokeMethod(
          m_worker, &FrameConsumerWorkerBase::processData, Qt::QueuedConnection);
      }
    }
  }

  FrameConsumerConfig m_config;
  moodycamel::ReaderWriterQueue<T> m_pendingQueue;
  std::atomic<bool> m_consumerEnabled;
  std::atomic<size_t> m_queueSize;
  QThread m_workerThread;
  FrameConsumerWorkerBase* m_worker;
};

}  // namespace DataModel
