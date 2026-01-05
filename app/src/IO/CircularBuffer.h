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
#include <vector>
#include <cstring>
#include <QByteArray>

#include "Concepts.h"

namespace IO
{
/**
 * @brief A lock-free circular buffer for high-throughput data streaming.
 *
 * This templated class provides a thread-safe circular buffer optimized for
 * Single-Producer-Single-Consumer (SPSC) scenarios using atomic operations.
 * Designed for high-frequency data acquisition (256 KHz+).
 *
 * **Type Requirements:**
 * - T: Must support `.size()`, `.data()` or similar container interface
 * - StorageType: Must be byte-like (integral, size == 1 byte)
 *
 * **Thread Safety:**
 * - SPSC safe: One producer thread (append), one consumer thread (read/peek)
 * - Uses std::atomic with memory_order_acquire/release semantics
 * - Lock-free for maximum throughput
 *
 * **Performance:**
 * - O(1) append/read for small data
 * - O(n) for pattern search (KMP algorithm)
 * - Zero mutex overhead in hotpath
 *
 * @tparam T The type of elements exposed to the user (e.g., QByteArray).
 * @tparam StorageType The type of elements used internally (default: uint8_t).
 */
template<typename T, Concepts::ByteLike StorageType = uint8_t>
class CircularBuffer
{
public:
  explicit CircularBuffer(qsizetype capacity = 1024 * 1024 * 10);

  [[nodiscard]] StorageType &operator[](qsizetype index);

  void clear();
  void append(const T &data);
  void setCapacity(const qsizetype capacity);

  [[nodiscard]] qsizetype size() const noexcept;
  [[nodiscard]] qsizetype freeSpace() const noexcept;
  [[nodiscard]] qsizetype capacity() const noexcept { return m_capacity; }

  [[nodiscard]] T read(qsizetype size);
  [[nodiscard]] T peek(qsizetype size) const;

  [[nodiscard]] int findPatternKMP(const T &pattern, const int pos = 0);

  [[nodiscard]] qsizetype overflowCount() const noexcept
  {
    return m_overflowCount.load(std::memory_order_relaxed);
  }

  void resetOverflowCount() noexcept
  {
    m_overflowCount.store(0, std::memory_order_relaxed);
  }

private:
  [[nodiscard]] std::vector<int> computeKMPTable(const T &p) const;

private:
  std::atomic<qsizetype> m_head;
  std::atomic<qsizetype> m_tail;
  std::atomic<qsizetype> m_overflowCount;
  qsizetype m_capacity;
  std::vector<StorageType> m_buffer;
};
} // namespace IO

/**
 * @brief Constructs a CircularBuffer object with a given capacity.
 *
 * Initializes the circular buffer with the specified capacity, setting up
 * internal variables with atomic initialization for thread safety.
 *
 * @param capacity The maximum capacity of the buffer in bytes.
 */
template<typename T, Concepts::ByteLike StorageType>
IO::CircularBuffer<T, StorageType>::CircularBuffer(qsizetype capacity)
  : m_head(0)
  , m_tail(0)
  , m_overflowCount(0)
  , m_capacity(capacity)
{
  m_buffer.resize(capacity);
}

/**
 * @brief Provides direct access to elements in the circular buffer by index.
 *
 * This subscript operator allows accessing elements stored in the circular
 * buffer at a specific logical index, taking the circular nature of the buffer
 * into account. It performs bounds checking with atomic size calculation.
 *
 * **Thread Safety:** Safe for SPSC if called from consumer thread only.
 *
 * @param index The logical index of the element to access (0-based).
 *              Must be in the range [0, size()-1].
 * @return The element at the specified index.
 * @throws std::out_of_range If the index is out of bounds.
 */
template<typename T, Concepts::ByteLike StorageType>
StorageType &IO::CircularBuffer<T, StorageType>::operator[](qsizetype index)
{
  const qsizetype current_size = size();
  if (index < 0 || index >= current_size)
    throw std::out_of_range("Index out of range");

  const qsizetype head = m_head.load(std::memory_order_acquire);
  const qsizetype effectiveIndex = (head + index) % m_capacity;
  return m_buffer[effectiveIndex];
}

/**
 * @brief Clears the circular buffer.
 *
 * Resets the buffer, removing all data and resetting internal variables.
 * Uses release semantics to ensure visibility across threads.
 *
 * **Thread Safety:** Not safe during concurrent append/read operations.
 * Call only when no other threads are accessing the buffer.
 */
template<typename T, Concepts::ByteLike StorageType>
void IO::CircularBuffer<T, StorageType>::clear()
{
  m_head.store(0, std::memory_order_release);
  m_tail.store(0, std::memory_order_release);
  m_overflowCount.store(0, std::memory_order_relaxed);
}

/**
 * @brief Appends data to the circular buffer (lock-free SPSC producer).
 *
 * Adds the given data to the buffer. If the data exceeds available space,
 * the oldest data is overwritten and overflow counter is incremented.
 * Uses atomic operations for thread-safe SPSC operation.
 *
 * **Thread Safety:** SPSC safe - call only from producer thread.
 * **Performance:** Lock-free, optimized for 256 KHz+ data rates.
 *
 * @param data The data to append to the buffer.
 */
template<typename T, Concepts::ByteLike StorageType>
void IO::CircularBuffer<T, StorageType>::append(const T &data)
{
  const qsizetype dataSize = data.size();
  if (dataSize == 0) [[unlikely]]
    return;

  const uint8_t *src = reinterpret_cast<const uint8_t *>(data.data());

  qsizetype copySize = dataSize;
  if (copySize > m_capacity) [[unlikely]]
  {
    src += (copySize - m_capacity);
    copySize = m_capacity;
    m_overflowCount.fetch_add(dataSize - m_capacity, std::memory_order_relaxed);
  }

  const qsizetype head = m_head.load(std::memory_order_acquire);
  const qsizetype tail = m_tail.load(std::memory_order_relaxed);

  qsizetype current_size
      = (tail >= head) ? (tail - head) : (m_capacity - head + tail);
  qsizetype free_space = m_capacity - current_size;

  if (copySize > free_space) [[unlikely]]
  {
    const qsizetype overwrite = copySize - free_space;
    m_overflowCount.fetch_add(overwrite, std::memory_order_relaxed);

    const qsizetype new_head = (head + overwrite) % m_capacity;
    m_head.store(new_head, std::memory_order_release);
  }

  const qsizetype firstChunk = std::min(copySize, m_capacity - tail);
  std::memcpy(&m_buffer[tail], src, firstChunk);

  if (copySize > firstChunk) [[unlikely]]
    std::memcpy(&m_buffer[0], src + firstChunk, copySize - firstChunk);

  const qsizetype new_tail = (tail + copySize) % m_capacity;
  m_tail.store(new_tail, std::memory_order_release);
}

/**
 * @brief Clears the buffer and modifies its maximum capacity.
 *
 * **Thread Safety:** Not safe during concurrent operations.
 * Call only when no other threads are accessing the buffer.
 */
template<typename T, Concepts::ByteLike StorageType>
void IO::CircularBuffer<T, StorageType>::setCapacity(const qsizetype capacity)
{
  clear();
  m_capacity = capacity;
  m_buffer.resize(capacity);
}

/**
 * @brief Returns the current size of the buffer.
 *
 * Calculates size from head and tail atomics using acquire semantics.
 *
 * **Thread Safety:** Safe for SPSC from any thread.
 * **Performance:** O(1), lock-free.
 *
 * @return The number of bytes currently stored in the buffer.
 */
template<typename T, Concepts::ByteLike StorageType>
qsizetype IO::CircularBuffer<T, StorageType>::size() const noexcept
{
  const qsizetype head = m_head.load(std::memory_order_acquire);
  const qsizetype tail = m_tail.load(std::memory_order_acquire);

  if (tail >= head)
    return tail - head;
  else
    return m_capacity - head + tail;
}

/**
 * @brief Returns the free space available in the buffer.
 *
 * **Thread Safety:** Safe for SPSC from any thread.
 * **Performance:** O(1), lock-free.
 *
 * @return The number of bytes of free space in the buffer.
 */
template<typename T, Concepts::ByteLike StorageType>
qsizetype IO::CircularBuffer<T, StorageType>::freeSpace() const noexcept
{
  return m_capacity - size();
}

/**
 * @brief Reads data from the circular buffer (lock-free SPSC consumer).
 *
 * Reads the specified number of bytes from the buffer. The read data is removed
 * from the buffer. Uses atomic operations for thread-safe SPSC operation.
 *
 * **Thread Safety:** SPSC safe - call only from consumer thread.
 * **Performance:** O(n) where n = size, lock-free.
 *
 * @param size The number of bytes to read.
 * @return Data read from the buffer.
 * @throws std::underflow_error if there is not enough data in the buffer.
 */
template<typename T, Concepts::ByteLike StorageType>
T IO::CircularBuffer<T, StorageType>::read(qsizetype size)
{
  const qsizetype current_size = this->size();
  if (size > current_size) [[unlikely]]
    throw std::underflow_error("Not enough data in buffer");

  T result;
  result.resize(size);

  const qsizetype head = m_head.load(std::memory_order_relaxed);
  const qsizetype firstChunk = std::min(size, m_capacity - head);
  std::memcpy(result.data(), &m_buffer[head], firstChunk);

  if (size > firstChunk) [[unlikely]]
    std::memcpy(result.data() + firstChunk, &m_buffer[0], size - firstChunk);

  const qsizetype new_head = (head + size) % m_capacity;
  m_head.store(new_head, std::memory_order_release);

  return result;
}

/**
 * @brief Retrieves data from the buffer without removing it.
 *
 * Extracts up to the specified number of bytes from the buffer, starting from
 * the current head position, without modifying the buffer's head or tail
 * positions. Uses atomic load for thread-safe access.
 *
 * **Thread Safety:** SPSC safe - can be called from any thread.
 * **Performance:** O(n) where n = size, lock-free.
 *
 * @param size The number of bytes to peek from the buffer.
 * @return Data peeked from the buffer. If the buffer contains less data
 *         than requested, the returned data will be smaller.
 */
template<typename T, Concepts::ByteLike StorageType>
T IO::CircularBuffer<T, StorageType>::peek(qsizetype size) const
{
  const qsizetype current_size = this->size();
  size = std::min(size, current_size);

  T result;
  result.resize(size);

  const qsizetype head = m_head.load(std::memory_order_acquire);
  const qsizetype firstChunk = std::min(size, m_capacity - head);
  std::memcpy(result.data(), &m_buffer[head], firstChunk);

  if (size > firstChunk) [[unlikely]]
  {
    const qsizetype secondChunk = size - firstChunk;
    std::memcpy(result.data() + firstChunk, &m_buffer[0], secondChunk);
  }

  return result;
}

/**
 * @brief Searches for a pattern in the circular buffer using the KMP algorithm.
 *
 * This function uses the Knuth-Morris-Pratt (KMP) string matching algorithm
 * to efficiently find the first occurrence of a given pattern within the
 * circular buffer, starting from a specified position. The circular nature of
 * the buffer is correctly handled to ensure accurate matching, even if the
 * pattern spans the end of the buffer and the beginning.
 *
 * **Thread Safety:** SPSC safe - can be called from any thread.
 * **Performance:** O(n + m) where n = buffer size, m = pattern size.
 *
 * @param pattern The pattern to search for in the buffer.
 * @param pos The starting position (relative to the logical start of the
 *            buffer) for the search. Defaults to the beginning if set to 0.
 *
 * @return The index (relative to the logical start of the buffer) of the first
 *         occurrence of the pattern, or -1 if the pattern is not found.
 *
 * @warning If the `pattern` is empty or its size exceeds the current size of
 *          the buffer, the function returns -1 immediately.
 */
template<typename T, Concepts::ByteLike StorageType>
int IO::CircularBuffer<T, StorageType>::findPatternKMP(const T &pattern,
                                                       const int pos)
{
  const qsizetype current_size = size();
  if (pattern.isEmpty() || current_size < pattern.size()) [[unlikely]]
    return -1;

  const qsizetype head = m_head.load(std::memory_order_acquire);
  std::vector<int> lps = computeKMPTable(pattern);
  qsizetype bufferIdx = (head + pos) % m_capacity;
  int i = pos, j = 0;

  while (i < current_size)
  {
    if (m_buffer[bufferIdx] == pattern[j])
    {
      i++;
      j++;
      bufferIdx = (bufferIdx + 1) % m_capacity;

      if (j == pattern.size()) [[unlikely]]
      {
        int matchStart = i - j;
        return matchStart;
      }
    }

    else if (j != 0) [[likely]]
      j = lps[j - 1];

    else
    {
      i++;
      bufferIdx = (bufferIdx + 1) % m_capacity;
    }
  }

  return -1;
}

/**
 * @brief Computes the KMP table for a given p.
 *
 * Prepares the longest prefix suffix (LPS) table used by the KMP algorithm.
 *
 * @param p The QByteArray representing the p.
 * @return A vector of integers representing the LPS table.
 */
template<typename T, Concepts::ByteLike StorageType>
std::vector<int>
IO::CircularBuffer<T, StorageType>::computeKMPTable(const T &p) const
{
  qsizetype m = p.size();
  std::vector<int> lps(m, 0);

  qsizetype len = 0;
  qsizetype i = 1;

  while (i < m)
  {
    if (p[i] == p[len])
    {
      len++;
      lps[i++] = len;
    }

    else if (len != 0)
      len = lps[len - 1];

    else
      lps[i++] = 0;
  }

  return lps;
}
