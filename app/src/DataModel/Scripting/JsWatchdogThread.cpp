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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Scripting/JsWatchdogThread.h"

#include <algorithm>
#include <chrono>
#include <QCoreApplication>
#include <QJSEngine>
#include <QThread>
#include <QTimer>

#include "DataModel/Scripting/JsWatchdog.h"

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

static constexpr int kPollIntervalMs = 20;

/**
 * @brief Returns the steady-clock reading in nanoseconds shared by arm() and the scan.
 */
static qint64 steadyNowNs()
{
  using namespace std::chrono;
  return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

//--------------------------------------------------------------------------------------------------
// Worker
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the worker without a timer; the timer is created on its own thread.
 */
DataModel::JsWatchdogWorker::JsWatchdogWorker(QObject* parent) : QObject(parent), m_timer(nullptr)
{}

/**
 * @brief Creates and starts the poll timer on the worker thread.
 */
void DataModel::JsWatchdogWorker::begin()
{
  Q_ASSERT(QThread::currentThread() == thread());
  if (m_timer)
    return;

  m_timer = new QTimer(this);
  m_timer->setInterval(kPollIntervalMs);
  connect(m_timer, &QTimer::timeout, this, &JsWatchdogWorker::onTick);
  m_timer->start();
}

/**
 * @brief Stops and releases the poll timer on the worker thread.
 */
void DataModel::JsWatchdogWorker::finish()
{
  Q_ASSERT(QThread::currentThread() == thread());
  if (!m_timer)
    return;

  m_timer->stop();
  delete m_timer;
  m_timer = nullptr;
}

/**
 * @brief Asks the owning thread to interrupt every watchdog whose deadline elapsed.
 */
void DataModel::JsWatchdogWorker::onTick()
{
  JsWatchdogThread::instance().interruptExpired();
}

//--------------------------------------------------------------------------------------------------
// Singleton lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide watchdog thread.
 */
DataModel::JsWatchdogThread& DataModel::JsWatchdogThread::instance()
{
  static JsWatchdogThread singleton;
  return singleton;
}

/**
 * @brief Builds the singleton in a stopped state; the thread starts on first registration.
 */
DataModel::JsWatchdogThread::JsWatchdogThread()
  : m_thread(nullptr), m_worker(nullptr), m_started(false), m_stopped(false)
{}

/**
 * @brief Tears the worker thread down if it is still running.
 */
DataModel::JsWatchdogThread::~JsWatchdogThread()
{
  shutdown();
}

//--------------------------------------------------------------------------------------------------
// Registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adds a watchdog to the scan set, starting the worker thread on first use.
 */
void DataModel::JsWatchdogThread::registerWatchdog(JsWatchdog* watchdog)
{
  Q_ASSERT(watchdog != nullptr);

  QMutexLocker locker(&m_mutex);
  ensureStarted();
  m_watchdogs.push_back(watchdog);
}

/**
 * @brief Removes a watchdog from the scan set before its engine is destroyed.
 */
void DataModel::JsWatchdogThread::unregisterWatchdog(JsWatchdog* watchdog)
{
  Q_ASSERT(watchdog != nullptr);

  QMutexLocker locker(&m_mutex);
  const auto it = std::find(m_watchdogs.begin(), m_watchdogs.end(), watchdog);
  if (it != m_watchdogs.end())
    m_watchdogs.erase(it);
}

//--------------------------------------------------------------------------------------------------
// Scan
//--------------------------------------------------------------------------------------------------

/**
 * @brief Interrupts every armed watchdog past its deadline; the registry lock is held
 * across setInterrupted() so a watchdog can't be freed mid-dereference (UAF guard).
 */
void DataModel::JsWatchdogThread::interruptExpired()
{
  const qint64 now = steadyNowNs();

  QMutexLocker locker(&m_mutex);
  for (auto* watchdog : m_watchdogs) {
    const qint64 deadline = watchdog->deadlineNs();
    if (deadline != 0 && now >= deadline) [[unlikely]]
      watchdog->engine()->setInterrupted(true);
  }
}

//--------------------------------------------------------------------------------------------------
// Thread management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Spins up the worker thread once and arranges an orderly stop at app exit.
 */
void DataModel::JsWatchdogThread::ensureStarted()
{
  if (m_started)
    return;

  m_thread = new QThread();
  m_thread->setObjectName(QStringLiteral("JsWatchdog"));

  m_worker = new JsWatchdogWorker();
  m_worker->moveToThread(m_thread);

  QObject::connect(m_thread, &QThread::started, m_worker, &JsWatchdogWorker::begin);
  m_thread->start();

  if (auto* app = QCoreApplication::instance())
    QObject::connect(app, &QCoreApplication::aboutToQuit, app, [this]() { shutdown(); });

  m_started = true;
}

/**
 * @brief Stops the worker thread and releases it; idempotent and main-thread safe. finish()
 * runs on the worker thread via a blocking call so the timer is gone before the join below.
 */
void DataModel::JsWatchdogThread::shutdown()
{
  if (!m_started || m_stopped)
    return;

  m_stopped = true;

  QMetaObject::invokeMethod(m_worker, "finish", Qt::BlockingQueuedConnection);

  m_thread->quit();
  m_thread->wait();

  delete m_worker;
  m_worker = nullptr;

  delete m_thread;
  m_thread = nullptr;
}
