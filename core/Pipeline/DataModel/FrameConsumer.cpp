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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "FrameConsumer.h"

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the base export worker with the monotonic clock unset.
 */
DataModel::FrameConsumerWorkerBase::FrameConsumerWorkerBase(QObject* parent)
  : QObject(parent), m_lastFrameNs(-1), m_flushPosted(false)
{}

/**
 * @brief Default destructor.
 */
DataModel::FrameConsumerWorkerBase::~FrameConsumerWorkerBase() = default;

//--------------------------------------------------------------------------------------------------
// Flush-post coalescing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Claims the pending-flush latch; true means a processData post is already in flight, so
 *        the producer must not post another. Single-producer by contract, like the queue it gates.
 */
bool DataModel::FrameConsumerWorkerBase::markFlushPosted() noexcept
{
  return m_flushPosted.exchange(true, std::memory_order_acq_rel);
}

/**
 * @brief Releases the latch at the head of a drain, so work enqueued during it re-arms the trigger.
 */
void DataModel::FrameConsumerWorkerBase::clearFlushPost() noexcept
{
  m_flushPosted.store(false, std::memory_order_release);
}

//--------------------------------------------------------------------------------------------------
// Monotonic clock tracker: shared by every export worker
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a strictly-increasing offset (ns since baseline), bumping by 1 ns on collision.
 */
qint64 DataModel::FrameConsumerWorkerBase::monotonicFrameNs(
  std::chrono::steady_clock::time_point now, std::chrono::steady_clock::time_point baseline)
{
  qint64 ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - baseline).count();
  if (ns <= m_lastFrameNs)
    ns = m_lastFrameNs + 1;

  m_lastFrameNs = ns;
  return ns;
}

/**
 * @brief Strictly-increasing offset for ONE source: @p ns is the instant the source itself stamped
 *        and is returned unchanged unless it collides with that source's previous sample, where it
 *        is bumped by 1 ns. Keyed per source because the worker-wide clock rewrote a second
 *        source's instants into a nanosecond staircase behind the first source's tail (B1).
 */
qint64 DataModel::FrameConsumerWorkerBase::monotonicSourceNs(int sourceId, qint64 ns)
{
  const auto [it, inserted] = m_lastSourceNs.try_emplace(sourceId, ns);
  if (inserted)
    return ns;

  if (ns <= it->second)
    ns = it->second + 1;

  it->second = ns;
  return ns;
}

/**
 * @brief Resets both monotonic clocks back to their initial state.
 */
void DataModel::FrameConsumerWorkerBase::resetMonotonicClock()
{
  m_lastFrameNs = -1;
  m_lastSourceNs.clear();
}
