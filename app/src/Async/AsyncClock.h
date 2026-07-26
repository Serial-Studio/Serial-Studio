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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <functional>
#include <QHash>
#include <QObject>
#include <QThread>
#include <QTimerEvent>
#include <utility>

#include "SSAssert.h"

namespace Async {
/**
 * @brief Identifies one scheduled callback.
 */
using TimerId = int;

/**
 * @brief Sentinel handle returned when a scheduling request produced no timer.
 */
inline constexpr TimerId kInvalidTimerId = 0;

/**
 * @brief Timer-scheduling seam so a unit test can drive a task tree on a virtual clock
 *        instead of waiting out a multi-second backoff schedule in wall time.
 */
class AsyncClock {
public:
  using Callback = std::function<void()>;

  AsyncClock()                             = default;
  AsyncClock(AsyncClock&&)                 = delete;
  AsyncClock(const AsyncClock&)            = delete;
  AsyncClock& operator=(AsyncClock&&)      = delete;
  AsyncClock& operator=(const AsyncClock&) = delete;
  virtual ~AsyncClock()                    = default;

  virtual void cancel(TimerId id)                               = 0;
  [[nodiscard]] virtual TimerId schedule(int msec, Callback cb) = 0;
};

/**
 * @brief AsyncClock backed by the Qt event loop of the thread that owns it, so a tree's
 *        timers always belong to the thread driving that tree.
 */
class SystemClock final
  : public QObject
  , public AsyncClock {
public:
  explicit SystemClock(QObject* parent = nullptr) : QObject(parent) {}

  /**
   * @brief Cancels a pending callback; a stale or invalid handle is a no-op.
   */
  void cancel(TimerId id) override
  {
    SS_ASSERT(QThread::currentThread() == thread(), return);

    if (id == kInvalidTimerId)
      return;

    killTimer(id);
    m_callbacks.remove(id);
  }

  /**
   * @brief Schedules a one-shot callback and returns its cancellation handle.
   */
  [[nodiscard]] TimerId schedule(int msec, Callback cb) override
  {
    SS_ASSERT(msec >= 0, return kInvalidTimerId);
    SS_ASSERT(static_cast<bool>(cb), return kInvalidTimerId);
    SS_ASSERT(QThread::currentThread() == thread(), return kInvalidTimerId);

    const TimerId id = startTimer(msec, Qt::PreciseTimer);
    if (id != kInvalidTimerId)
      m_callbacks.insert(id, std::move(cb));

    return id;
  }

protected:
  /**
   * @brief Fires and retires the callback bound to the expired timer.
   */
  void timerEvent(QTimerEvent* event) override
  {
    SS_ASSERT(event != nullptr, return);
    SS_ASSERT(QThread::currentThread() == thread(), return);

    const TimerId id = event->timerId();
    const auto it    = m_callbacks.constFind(id);
    if (it == m_callbacks.constEnd())
      return;

    const Callback cb = it.value();
    killTimer(id);
    m_callbacks.remove(id);
    cb();
  }

private:
  QHash<TimerId, Callback> m_callbacks;
};
}  // namespace Async
