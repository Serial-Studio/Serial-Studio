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

#include "DataModel/Scripting/JsWatchdog.h"

#include <chrono>
#include <QDebug>

#include "DataModel/Scripting/JsWatchdogThread.h"

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the steady-clock reading in nanoseconds, matching the watchdog thread.
 */
static qint64 steadyNowNs()
{
  using namespace std::chrono;
  return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers a watchdog that interrupts the given engine after budgetMs.
 */
DataModel::JsWatchdog::JsWatchdog(QJSEngine* engine, int budgetMs, QString tag)
  : m_engine(engine)
  , m_deadlineNs(0)
  , m_budgetMs(budgetMs)
  , m_tag(std::move(tag))
  , m_lastTimedOut(false)
{
  Q_ASSERT(engine != nullptr);
  Q_ASSERT(budgetMs > 0);
  JsWatchdogThread::instance().registerWatchdog(this);
}

/**
 * @brief Unregisters from the watchdog thread before the engine can be destroyed.
 */
DataModel::JsWatchdog::~JsWatchdog()
{
  JsWatchdogThread::instance().unregisterWatchdog(this);
}

//--------------------------------------------------------------------------------------------------
// Arm / disarm
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clears any prior interrupt and publishes a fresh deadline; lock-free on the hotpath.
 */
void DataModel::JsWatchdog::arm() noexcept
{
  m_engine->setInterrupted(false);
  m_deadlineNs.store(steadyNowNs() + qint64(m_budgetMs) * 1000000, std::memory_order_release);
}

/**
 * @brief Retires the deadline so the watchdog thread stops watching this engine.
 */
void DataModel::JsWatchdog::disarm() noexcept
{
  m_deadlineNs.store(0, std::memory_order_release);
}

//--------------------------------------------------------------------------------------------------
// Watchdog-protected calls
//--------------------------------------------------------------------------------------------------

/**
 * @brief Calls fn(args) under the watchdog with no explicit `this`.
 */
QJSValue DataModel::JsWatchdog::call(QJSValue& fn, const QJSValueList& args)
{
  Q_ASSERT(fn.isCallable());
  Q_ASSERT(m_engine != nullptr);

  m_lastTimedOut = false;
  arm();
  const auto result = fn.call(args);
  disarm();

  if (m_engine->isInterrupted()) [[unlikely]] {
    m_engine->setInterrupted(false);
    m_lastTimedOut = true;
    qWarning().noquote() << "[JsWatchdog]" << (m_tag.isEmpty() ? QStringLiteral("script") : m_tag)
                         << "timed out after" << m_budgetMs << "ms -- interrupted";
  }

  return result;
}

/**
 * @brief Calls fn.callWithInstance(thisObj, args) under the watchdog.
 */
QJSValue DataModel::JsWatchdog::call(QJSValue& fn, QJSValue thisObj, const QJSValueList& args)
{
  Q_ASSERT(fn.isCallable());
  Q_ASSERT(m_engine != nullptr);

  m_lastTimedOut = false;
  arm();
  const auto result = fn.callWithInstance(thisObj, args);
  disarm();

  if (m_engine->isInterrupted()) [[unlikely]] {
    m_engine->setInterrupted(false);
    m_lastTimedOut = true;
    qWarning().noquote() << "[JsWatchdog]" << (m_tag.isEmpty() ? QStringLiteral("script") : m_tag)
                         << "timed out after" << m_budgetMs << "ms -- interrupted";
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// Configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current watchdog budget in milliseconds.
 */
int DataModel::JsWatchdog::budgetMs() const noexcept
{
  return m_budgetMs;
}

/**
 * @brief Updates the watchdog budget; takes effect on the next arm().
 */
void DataModel::JsWatchdog::setBudgetMs(int ms) noexcept
{
  Q_ASSERT(ms > 0);
  m_budgetMs = ms;
}

/**
 * @brief Returns true when the most recent call() was interrupted by timeout.
 */
bool DataModel::JsWatchdog::lastCallTimedOut() const noexcept
{
  return m_lastTimedOut;
}

//--------------------------------------------------------------------------------------------------
// Watchdog-thread accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the guarded engine; read by the watchdog thread under its lock.
 */
QJSEngine* DataModel::JsWatchdog::engine() const noexcept
{
  return m_engine;
}

/**
 * @brief Returns the armed deadline in steady-clock ns, or 0 when disarmed.
 */
qint64 DataModel::JsWatchdog::deadlineNs() const noexcept
{
  return m_deadlineNs.load(std::memory_order_acquire);
}
