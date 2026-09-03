/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/PolledPlcWorkerBase.h"

#include <QTimer>

#include "SSAssert.h"

static constexpr int kPolledPlcMinIntervalMs = 1;

/**
 * @brief Builds the worker with no timer and no channels: both are created on this object's own
 *        thread, never in the constructor, which still runs on the GUI thread.
 */
IO::Drivers::PolledPlcWorkerBase::PolledPlcWorkerBase()
  : m_open(false)
  , m_reported(false)
  , m_interval(kPolledPlcMinIntervalMs)
  , m_frameSlot(0)
  , m_timer(nullptr)
  , m_abort(false)
  , m_readsOk(0)
  , m_readsFailed(0)
  , m_framesPublished(0)
{}

/**
 * @brief Drops the poll timer. The protocol resources are released by the derived destructor's
 *        shutdown() call: releaseResources() is pure virtual and must never be reached from here.
 */
IO::Drivers::PolledPlcWorkerBase::~PolledPlcWorkerBase()
{
  stopPolling();
  SS_ASSERT_LOG(m_timer == nullptr);
  SS_ASSERT_LOG(!m_open);
}

/**
 * @brief Latches a teardown request from the GUI thread so an in-flight poll unwinds promptly. The
 *        flag is atomic and set BEFORE the blocking shutdown invoke, which is what lets the poll
 *        loop return between exchanges instead of freezing the GUI for a full reply deadline.
 */
void IO::Drivers::PolledPlcWorkerBase::requestAbort() noexcept
{
  m_abort.store(true, std::memory_order_relaxed);
}

/**
 * @brief Successful reads since the session opened.
 */
quint64 IO::Drivers::PolledPlcWorkerBase::readsOk() const noexcept
{
  return m_readsOk.load(std::memory_order_relaxed);
}

/**
 * @brief Refused or timed-out reads since the session opened.
 */
quint64 IO::Drivers::PolledPlcWorkerBase::readsFailed() const noexcept
{
  return m_readsFailed.load(std::memory_order_relaxed);
}

/**
 * @brief Delta frames handed to the driver since the session opened.
 */
quint64 IO::Drivers::PolledPlcWorkerBase::framesPublished() const noexcept
{
  return m_framesPublished.load(std::memory_order_relaxed);
}

/**
 * @brief Dials on the worker thread and hands the verdict back through dialFinished(), exactly
 *        once per attempt (spec 0050 latch). The blocking protocol calls stay in the derived
 *        connectToPlc(); the GUI thread no longer waits out a controller that does not answer.
 */
void IO::Drivers::PolledPlcWorkerBase::beginDial()
{
  SS_ASSERT_LOG(!m_open);
  const bool ok = connectToPlc();
  Q_EMIT dialFinished(ok, m_dialError);
}

/**
 * @brief Stops polling and releases the derived protocol resources on the thread that owns them.
 *        Idempotent, because both the driver's teardown and the derived destructor reach it.
 */
void IO::Drivers::PolledPlcWorkerBase::shutdown()
{
  m_open = false;
  stopPolling();
  releaseResources();

  SS_ASSERT_LOG(m_timer == nullptr);
  SS_ASSERT_LOG(!m_open);
}

/**
 * @brief Tears down the poll timer; safe to call with no session open.
 */
void IO::Drivers::PolledPlcWorkerBase::stopPolling()
{
  if (!m_timer)
    return;

  m_timer->stop();
  delete m_timer;
  m_timer = nullptr;
}

/**
 * @brief Arms the poll timer and marks the session live. Called by the derived connectToPlc() as
 *        its last step, so a dial that failed never leaves a timer running.
 */
void IO::Drivers::PolledPlcWorkerBase::startPolling()
{
  SS_ASSERT(m_timer == nullptr, return);
  SS_ASSERT_LOG(m_interval >= kPolledPlcMinIntervalMs);

  m_open     = true;
  m_reported = false;
  m_timer    = new QTimer(this);
  m_timer->setInterval(m_interval);
  connect(m_timer, &QTimer::timeout, this, &PolledPlcWorkerBase::onPollTimer);
  m_timer->start();
}

/**
 * @brief Clears the abort latch at the start of a dial attempt so a worker reused after a cancelled
 *        attempt polls again instead of returning at its first abort check.
 */
void IO::Drivers::PolledPlcWorkerBase::clearAbort() noexcept
{
  m_abort.store(false, std::memory_order_relaxed);
}

/**
 * @brief Reports a lost link exactly once and stops polling; the driver turns it into a queued
 *        disconnect so nothing tears the device down from inside this handler.
 */
void IO::Drivers::PolledPlcWorkerBase::reportFailure(const QString& reason)
{
  SS_ASSERT_LOG(!reason.isEmpty());
  if (m_reported)
    return;

  m_reported = true;
  m_open     = false;
  if (m_timer)
    m_timer->stop();

  Q_EMIT linkLost(reason);
}

/**
 * @brief Encodes every dirty channel into one OpcUaWire delta frame and hands it to the driver with
 *        the poll's own timestamp. The stamp is captured by the caller, before the queued hop,
 *        because the source owns time and a receipt-time stamp on the GUI thread would carry the
 *        queue's latency into every recording.
 */
void IO::Drivers::PolledPlcWorkerBase::publishDirtySlots(qint64 stampNs)
{
  SS_ASSERT_LOG(m_dirty.size() == m_types.size());
  SS_ASSERT_LOG(stampNs > 0);

  QByteArray& frame = m_frames[m_frameSlot];
  OpcUaWire::beginFrame(frame);
  for (int i = 0; i < m_types.size(); ++i) {
    if (!m_dirty.at(i))
      continue;

    if (frame.size() + OpcUaWire::maxEntryBytes(m_types.at(i)) > OpcUaWire::kMaxFrameBytes)
      break;

    OpcUaWire::appendEntry(frame, i, m_types.at(i), m_values.at(i));
    m_dirty[i] = false;
  }

  if (frame.size() <= OpcUaWire::kHeaderBytes)
    return;

  m_framesPublished.fetch_add(1, std::memory_order_relaxed);
  Q_EMIT frameReady(frame, stampNs);
  m_frameSlot ^= 1;
}

/**
 * @brief Records why the dial failed; beginDial() hands this text to the driver with the verdict.
 */
void IO::Drivers::PolledPlcWorkerBase::noteDialError(const QString& reason)
{
  m_dialError = reason;
}

/**
 * @brief Counts successful reads.
 */
void IO::Drivers::PolledPlcWorkerBase::countReadsOk(quint64 count) noexcept
{
  m_readsOk.fetch_add(count, std::memory_order_relaxed);
}

/**
 * @brief Counts refused or timed-out reads.
 */
void IO::Drivers::PolledPlcWorkerBase::countReadsFailed(quint64 count) noexcept
{
  m_readsFailed.fetch_add(count, std::memory_order_relaxed);
}

/**
 * @brief Sizes the latch table from the wire types the derived worker resolved and reserves both
 *        frame buffers against the largest frame those types can produce. Called before the thread
 *        starts, so the worker's state is complete by the time its event loop delivers anything.
 */
void IO::Drivers::PolledPlcWorkerBase::configureChannels(int interval,
                                                         QVector<OpcUaWire::Type> types)
{
  SS_ASSERT(!types.isEmpty(), return);
  SS_ASSERT_LOG(interval >= kPolledPlcMinIntervalMs);

  m_interval = interval;
  m_types    = std::move(types);
  m_values   = QList<QVariant>(m_types.size());
  m_dirty    = QList<bool>(m_types.size(), false);
  m_dialError.clear();

  qsizetype bytes = OpcUaWire::kHeaderBytes;
  for (const auto type : m_types)
    bytes += OpcUaWire::maxEntryBytes(type);

  const auto reserve = qMin<qsizetype>(bytes, OpcUaWire::kMaxFrameBytes);
  m_frames[0].reserve(reserve);
  m_frames[1].reserve(reserve);
}

/**
 * @brief Returns true once the GUI thread has asked the in-flight poll to unwind.
 */
bool IO::Drivers::PolledPlcWorkerBase::aborted() const noexcept
{
  return m_abort.load(std::memory_order_relaxed);
}

/**
 * @brief Returns true while the polled session is established.
 */
bool IO::Drivers::PolledPlcWorkerBase::sessionOpen() const noexcept
{
  return m_open;
}

/**
 * @brief The number of configured channels, which is also the wire index range.
 */
int IO::Drivers::PolledPlcWorkerBase::channelCount() const noexcept
{
  return static_cast<int>(m_types.size());
}

/**
 * @brief The poll interval the session opened with, in milliseconds.
 */
int IO::Drivers::PolledPlcWorkerBase::pollIntervalMs() const noexcept
{
  return m_interval;
}

/**
 * @brief Latches @p value at @p index and returns whether it moved. An unchanged value is dropped
 *        here rather than by the encoder: a delta frame carries only what changed, and a channel
 *        the controller keeps answering with the same number must not cost a wire entry per tick.
 */
bool IO::Drivers::PolledPlcWorkerBase::latchChannel(int index, const QVariant& value)
{
  SS_ASSERT(index >= 0 && index < m_values.size(), return false);
  SS_ASSERT_LOG(m_values.size() == m_dirty.size());

  if (m_values.at(index) == value)
    return false;

  m_values[index] = value;
  m_dirty[index]  = true;
  return true;
}

/**
 * @brief Drives one poll of the derived worker; a session that closed or a teardown already latched
 *        skips the tick rather than starting an exchange nothing is waiting for.
 */
void IO::Drivers::PolledPlcWorkerBase::onPollTimer()
{
  if (!m_open || m_abort.load(std::memory_order_relaxed))
    return;

  pollTick();
}
