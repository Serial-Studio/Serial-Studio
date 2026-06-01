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
#include <cstddef>
#include <new>
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
 */
struct FrameConsumerConfig {
  size_t queueCapacity  = 8192;
  size_t flushThreshold = 1024;
  int timerIntervalMs   = 1000;
};

/**
 * @brief Non-template Q_OBJECT base for frame consumer workers. Owns the monotonic-ns clock shared
 * by CSV, MDF4 and Sessions export workers.
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
 * @brief Threaded worker that drains a lock-free queue of T into `processItems()` in batches.
 * Subclasses implement `processItems()`, `closeResources()`, `isResourceOpen()`.
 */
template<typename T>
class FrameConsumerWorker : public FrameConsumerWorkerBase {
public:
  /**
   * @brief Constructs a frame consumer worker.
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
  virtual void processItems(const std::vector<T>& items) = 0;
  virtual void closeResources()                          = 0;
  [[nodiscard]] virtual bool isResourceOpen() const      = 0;

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
 * @brief Main-thread facade that owns a `FrameConsumerWorker` on a dedicated QThread. Provides a
 * lock-free `enqueueData()` hotpath with periodic + threshold-based flushing.
 */
template<typename T>
class FrameConsumer : public QObject {
public:
  /**
   * @brief Constructs a frame consumer with the given configuration.
   */
  explicit FrameConsumer(const FrameConsumerConfig& config = {})
    : m_config(config)
    , m_pendingQueue(config.queueCapacity)
    , m_consumerEnabled(true)
    , m_queueSize(0)
    , m_worker(nullptr)
    , m_timer(nullptr)
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

    m_timer = new QTimer();
    m_timer->setInterval(m_config.timerIntervalMs);
    m_timer->setTimerType(Qt::PreciseTimer);
    m_timer->moveToThread(&m_workerThread);

    QObject::connect(m_timer, &QTimer::timeout, m_worker, &FrameConsumerWorkerBase::processData);
    QObject::connect(&m_workerThread, &QThread::finished, m_timer, &QObject::deleteLater);

    QMetaObject::invokeMethod(m_timer, "start", Qt::QueuedConnection);
  }

  /**
   * @brief Updates the worker timer interval at runtime; safe to call from the main thread.
   */
  void setTimerIntervalMs(int ms)
  {
    if (!m_timer || ms <= 0)
      return;

    QTimer* timer = m_timer;
    QMetaObject::invokeMethod(
      m_timer, [timer, ms] { timer->setInterval(ms); }, Qt::QueuedConnection);
  }

  /**
   * @brief Flushes and stops the worker thread. Idempotent; call before QApplication teardown.
   */
  void stopWorker()
  {
    if (!m_worker || !m_workerThread.isRunning())
      return;

    QMetaObject::invokeMethod(
      m_worker, &FrameConsumerWorkerBase::close, Qt::BlockingQueuedConnection);

    m_workerThread.quit();
    m_workerThread.wait();
  }

  /**
   * @brief Destructor ensures all data is flushed before shutdown.
   */
  virtual ~FrameConsumer()
  {
    stopWorker();
  }

  /**
   * @brief Checks if the consumer is currently enabled.
   */
  [[nodiscard]] bool consumerEnabled() const
  {
    return m_consumerEnabled.load(std::memory_order_relaxed);
  }

  /**
   * @brief Enables or disables frame processing.
   */
  void setConsumerEnabled(const bool enabled)
  {
    m_consumerEnabled.store(enabled, std::memory_order_relaxed);
  }

protected:
  /**
   * @brief Factory method to create the worker object.
   */
  virtual FrameConsumerWorkerBase* createWorker() = 0;

  /**
   * @brief Enqueues a data item for processing on the worker thread.
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

  static constexpr std::size_t kCacheLine = 64;

  FrameConsumerConfig m_config;
  moodycamel::ReaderWriterQueue<T> m_pendingQueue;
  alignas(kCacheLine) std::atomic<bool> m_consumerEnabled;
  alignas(kCacheLine) std::atomic<size_t> m_queueSize;
  QThread m_workerThread;
  FrameConsumerWorkerBase* m_worker;
  QTimer* m_timer;
};

}  // namespace DataModel
