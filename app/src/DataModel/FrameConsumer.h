/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
#include <QObject>
#include <QThread>
#include <QTimer>
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
 * @brief Non-template base class for frame consumer workers.
 *
 * Provides Q_OBJECT functionality (signals, slots, meta-object) without
 * template parameters. Template classes cannot use Q_OBJECT directly.
 */
class FrameConsumerWorkerBase : public QObject {
  Q_OBJECT

signals:
  void resourceOpenChanged();

public:
  explicit FrameConsumerWorkerBase(QObject* parent = nullptr);
  virtual ~FrameConsumerWorkerBase();

public slots:
  virtual void processData() = 0;
  virtual void close()       = 0;
};

/**
 * @brief Abstract worker class for processing frames on a dedicated thread.
 *
 * This template class provides the worker thread logic for consuming frames
 * from a lock-free queue. It handles queue draining, batch processing, and
 * periodic flushing via timer.
 *
 * Derived classes must implement:
 * - processItems(): Process a batch of dequeued items
 * - closeResources(): Clean up resources (files, connections, etc.)
 * - isResourceOpen(): Check if resources are currently open
 *
 * @tparam T The type of items to process (e.g.,
 * std::shared_ptr<JSON::TimestampFrame>)
 *
 * @note This class is designed to run on a QThread via moveToThread().
 *       All public slots will be invoked on the worker thread.
 *
 * @example
 * class MyWorker : public FrameConsumerWorker<std::shared_ptr<MyFrame>>
 * {
 * protected:
 *   void processItems(const std::vector<std::shared_ptr<MyFrame>> &items)
 * override
 *   {
 *     for (const auto &frame : items)
 *       writeToFile(frame);
 *   }
 *
 *   void closeResources() override { m_file.close(); }
 *   bool isResourceOpen() const override { return m_file.isOpen(); }
 *
 * private:
 *   QFile m_file;
 * };
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
  {}

  ~FrameConsumerWorker() override = default;

  /**
   * @brief Processes all pending frames from the queue.
   *
   * This slot drains the entire queue into a local buffer, updates the queue
   * size counter, and delegates processing to the derived class via
   * processItems().
   *
   * Called by:
   * - Periodic timer (e.g., every 1000ms)
   * - Threshold trigger when queue reaches flushThreshold
   * - Shutdown sequence to flush remaining data
   */
  void processData() override
  {
    if (!m_enabled->load(std::memory_order_relaxed))
      return;

    m_writeBuffer.clear();

    T item;
    while (m_queue->try_dequeue(item))
      m_writeBuffer.push_back(std::move(item));

    const auto count = m_writeBuffer.size();
    if (count == 0)
      return;

    m_queueSize->fetch_sub(count, std::memory_order_relaxed);

    const bool wasOpen = isResourceOpen();
    processItems(m_writeBuffer);
    const bool isOpen = isResourceOpen();

    if (wasOpen != isOpen)
      Q_EMIT resourceOpenChanged();
  }

  /**
   * @brief Flushes remaining data and closes resources.
   *
   * Called during shutdown to ensure all queued data is processed and
   * resources are properly cleaned up.
   */
  void close() override
  {
    processData();
    closeResources();
    Q_EMIT resourceOpenChanged();
  }

protected:
  /**
   * @brief Processes a batch of dequeued items.
   *
   * Derived classes implement this to perform the actual work (e.g., writing
   * to CSV, serializing to JSON, etc.).
   *
   * @param items Vector of items drained from the queue
   *
   * @note This method runs on the worker thread. It may open resources lazily
   *       on first call (e.g., create CSV file when first frame arrives).
   */
  virtual void processItems(const std::vector<T>& items) = 0;

  /**
   * @brief Closes all resources managed by this worker.
   *
   * Derived classes implement this to clean up files, network connections,
   * or other resources.
   *
   * @note Called during shutdown and when processing is disabled.
   */
  virtual void closeResources() = 0;

  /**
   * @brief Checks if resources are currently open.
   *
   * @return true if resources (file, connection, etc.) are open, false
   * otherwise
   *
   * @note Used to emit resourceOpenChanged() signal when state changes.
   */
  virtual bool isResourceOpen() const = 0;

private:
  std::vector<T> m_writeBuffer;
  moodycamel::ReaderWriterQueue<T>* m_queue;
  std::atomic<bool>* m_enabled;
  std::atomic<size_t>* m_queueSize;
};

/**
 * @brief Abstract base class for threaded frame consumers.
 *
 * This template class provides a complete threaded frame consumer
 * implementation with lock-free queuing, batch processing, and dual-trigger
 * flushing (periodic timer + threshold-based).
 *
 * Derived classes must implement:
 * - createWorker(): Factory method to create the worker object
 *
 * The consumer manages:
 * - Dedicated worker thread via QThread
 * - Lock-free queue for wait-free enqueue from main thread
 * - Atomic state flags for thread-safe enable/disable
 * - Periodic flushing timer on worker thread
 * - Threshold-based early flushing when queue depth reaches limit
 * - Graceful shutdown with blocking wait for data flush
 *
 * @tparam T The type of items to process (e.g.,
 * std::shared_ptr<JSON::TimestampFrame>)
 *
 * @note Enqueuing is lock-free and safe to call from the main thread hotpath.
 *
 * @example
 * class MyExport : public FrameConsumer<std::shared_ptr<MyFrame>>
 * {
 * public:
 *   MyExport() : FrameConsumer(FrameConsumerConfig{8192, 1024, 1000}) {}
 *
 *   void hotpathTxFrame(const std::shared_ptr<MyFrame> &frame)
 *   {
 *     enqueueData(frame);
 *   }
 *
 * protected:
 *   FrameConsumerWorker<std::shared_ptr<MyFrame>> *createWorker() override
 *   {
 *     return new MyWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
 *   }
 * };
 */
template<typename T>
class FrameConsumer : public QObject {
public:
  /**
   * @brief Constructs a frame consumer with the given configuration.
   *
   * Sets up the worker thread, creates the worker object, and starts the
   * periodic flushing timer.
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
   *
   * Must be called after construction, typically from derived class
   * constructor.
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
   *
   * Performs a blocking wait for the worker thread to process all remaining
   * queued data and clean up resources.
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
   * When disabled, frames are still enqueued but not processed. When
   * re-enabled, queued frames will be processed on the next timer tick or
   * threshold trigger.
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
   * Derived classes implement this to instantiate their specific worker type.
   *
   * @return Pointer to newly created worker (ownership transferred to thread)
   *
   * @example
   * FrameConsumerWorkerBase *createWorker() override
   * {
   *   return new MyWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
   * }
   */
  virtual FrameConsumerWorkerBase* createWorker() = 0;

  /**
   * @brief Enqueues a data item for processing on the worker thread.
   *
   * This method is lock-free and safe to call from the main thread hotpath.
   * If the queue depth reaches the flush threshold, it triggers an immediate
   * flush on the worker thread.
   *
   * @param item The item to enqueue
   *
   * @note If the queue is full, the item is silently dropped (wait-free
   * guarantee).
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
