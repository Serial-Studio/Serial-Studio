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

#pragma once

#include <atomic>
#include <cstring>
#include <QtGlobal>
#include <vector>

#include "Core/SSAssert.h"

namespace IO {
namespace Drivers {

/**
 * @brief Fixed-capacity SPSC byte ring between the GUI thread that writes playback samples and
 *        the real-time callback that drains them. The callback never allocates, locks or blocks:
 *        a short read zero-fills the rest and counts an underrun, and a write that does not fit
 *        is refused whole, because half a sample frame played out is noise.
 */
class PlaybackRing {
public:
  /**
   * @brief Builds a ring holding @p capacityBytes bytes.
   */
  explicit PlaybackRing(qsizetype capacityBytes)
    : m_buffer(static_cast<std::size_t>(capacityBytes > 0 ? capacityBytes : 1))
    , m_head(0)
    , m_tail(0)
    , m_underruns(0)
    , m_overflows(0)
  {}

  PlaybackRing(PlaybackRing&&)                 = delete;
  PlaybackRing(const PlaybackRing&)            = delete;
  PlaybackRing& operator=(PlaybackRing&&)      = delete;
  PlaybackRing& operator=(const PlaybackRing&) = delete;

  /**
   * @brief Total size of the ring in bytes.
   */
  [[nodiscard]] qsizetype capacity() const noexcept
  {
    return static_cast<qsizetype>(m_buffer.size());
  }

  /**
   * @brief Bytes the consumer can read right now.
   */
  [[nodiscard]] qsizetype available() const noexcept
  {
    const auto head = m_head.load(std::memory_order_acquire);
    const auto tail = m_tail.load(std::memory_order_acquire);
    return static_cast<qsizetype>(head - tail);
  }

  /**
   * @brief Bytes the producer can write right now.
   */
  [[nodiscard]] qsizetype freeSpace() const noexcept { return capacity() - available(); }

  /**
   * @brief How many callbacks ran out of samples; polled, never pushed (spec 0033).
   */
  [[nodiscard]] quint64 underruns() const noexcept
  {
    return m_underruns.load(std::memory_order_relaxed);
  }

  /**
   * @brief How many writes were refused for lack of room.
   */
  [[nodiscard]] quint64 overflows() const noexcept
  {
    return m_overflows.load(std::memory_order_relaxed);
  }

  /**
   * @brief Drops every buffered sample; called at open and close, never while both ends run.
   */
  void reset() noexcept
  {
    m_head.store(0, std::memory_order_relaxed);
    m_tail.store(0, std::memory_order_relaxed);
  }

  /**
   * @brief Producer side: appends @p size bytes, or refuses the write whole.
   */
  [[nodiscard]] bool write(const char* data, qsizetype size) noexcept
  {
    SS_ASSERT(data != nullptr, return false);
    SS_ASSERT(size > 0, return false);

    const auto head      = m_head.load(std::memory_order_relaxed);
    const auto tail      = m_tail.load(std::memory_order_acquire);
    const qsizetype used = static_cast<qsizetype>(head - tail);
    if (size > capacity() - used) {
      m_overflows.fetch_add(1, std::memory_order_relaxed);
      return false;
    }

    const qsizetype offset = static_cast<qsizetype>(head % static_cast<quint64>(capacity()));
    const qsizetype first  = qMin(size, capacity() - offset);
    std::memcpy(m_buffer.data() + offset, data, static_cast<std::size_t>(first));
    if (size > first)
      std::memcpy(m_buffer.data(), data + first, static_cast<std::size_t>(size - first));

    m_head.store(head + static_cast<quint64>(size), std::memory_order_release);
    return true;
  }

  /**
   * @brief Consumer side: fills @p size bytes of @p out, zero-filling and counting an underrun
   *        when fewer were buffered. Returns how many real bytes were delivered.
   */
  [[nodiscard]] qsizetype read(char* out, qsizetype size) noexcept
  {
    SS_ASSERT(out != nullptr, return 0);
    SS_ASSERT(size > 0, return 0);

    const auto tail        = m_tail.load(std::memory_order_relaxed);
    const auto head        = m_head.load(std::memory_order_acquire);
    const qsizetype used   = static_cast<qsizetype>(head - tail);
    const qsizetype toRead = qMin(size, used);

    if (toRead > 0) {
      const qsizetype offset = static_cast<qsizetype>(tail % static_cast<quint64>(capacity()));
      const qsizetype first  = qMin(toRead, capacity() - offset);
      std::memcpy(out, m_buffer.data() + offset, static_cast<std::size_t>(first));
      if (toRead > first)
        std::memcpy(out + first, m_buffer.data(), static_cast<std::size_t>(toRead - first));

      m_tail.store(tail + static_cast<quint64>(toRead), std::memory_order_release);
    }

    if (toRead < size) {
      std::memset(out + toRead, 0, static_cast<std::size_t>(size - toRead));
      m_underruns.fetch_add(1, std::memory_order_relaxed);
    }

    return toRead;
  }

private:
  std::vector<char> m_buffer;

  alignas(64) std::atomic<quint64> m_head;
  alignas(64) std::atomic<quint64> m_tail;
  alignas(64) std::atomic<quint64> m_underruns;
  alignas(64) std::atomic<quint64> m_overflows;
};
}  // namespace Drivers
}  // namespace IO
