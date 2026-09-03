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

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <utility>

namespace API {
namespace GRPC {

/**
 * @brief One gRPC handler's call into the GUI thread, after IO::PipelineHost::MarshalCall: the
 *        functor runs under the mutex, so abandon() cannot return while it runs and a waiter
 *        abandoning before it leaves may capture by reference. Unlike a BlockingQueuedConnection,
 *        stopServer() abandons parked calls before Shutdown() waits (spec 0075 I5).
 */
class PendingCall {
public:
  /**
   * @brief Binds the functor the GUI thread will run.
   */
  explicit PendingCall(std::function<void()> fn)
    : m_fn(std::move(fn)), m_done(false), m_abandoned(false)
  {}

  PendingCall(PendingCall&&)                 = delete;
  PendingCall(const PendingCall&)            = delete;
  PendingCall& operator=(PendingCall&&)      = delete;
  PendingCall& operator=(const PendingCall&) = delete;

  /**
   * @brief Runs the functor on the GUI thread and wakes the waiter, unless it already left.
   */
  void dispatch()
  {
    {
      const std::lock_guard<std::mutex> guard(m_mutex);
      if (m_abandoned)
        return;

      m_fn();
      m_done = true;
    }

    m_cv.notify_all();
  }

  /**
   * @brief Waits for the dispatch; false when it was abandoned or the deadline passed.
   */
  [[nodiscard]] bool wait(std::chrono::milliseconds timeout)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait_for(lock, timeout, [this] { return m_done || m_abandoned; });
    return m_done;
  }

  /**
   * @brief Retires the call once its waiter stops waiting, blocking behind an in-flight dispatch.
   */
  void abandon()
  {
    {
      const std::lock_guard<std::mutex> guard(m_mutex);
      m_abandoned = true;
    }

    m_cv.notify_all();
  }

private:
  std::mutex m_mutex;
  std::condition_variable m_cv;
  std::function<void()> m_fn;
  bool m_done;
  bool m_abandoned;
};

}  // namespace GRPC
}  // namespace API
