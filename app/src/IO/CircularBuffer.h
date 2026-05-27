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
#include <cstddef>
#include <cstring>
#include <new>
#include <QByteArray>
#include <vector>

#include "Concepts.h"

namespace IO {

/**
 * @brief Rounds a positive size up to the next power of two (>= 2). Used so the
 *        SPSC ring can mask wrap-around with `& (cap - 1)` instead of `% cap`.
 */
[[nodiscard]] inline qsizetype roundUpToPowerOfTwo(qsizetype value) noexcept
{
  qsizetype v = value < 2 ? 2 : value;
  --v;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  if constexpr (sizeof(qsizetype) > 4)
    v |= v >> 32;
  return v + 1;
}

/**
 * @brief A lock-free circular buffer for high-throughput data streaming.
 */
template<typename T, Concepts::ByteLike StorageType = uint8_t>
class CircularBuffer {
public:
  explicit CircularBuffer(qsizetype capacity = 1024 * 1024 * 16);

  [[nodiscard]] StorageType& operator[](qsizetype index);

  void clear();
  void append(const T& data);
  void setCapacity(const qsizetype capacity);

  [[nodiscard]] qsizetype size() const noexcept;
  [[nodiscard]] qsizetype freeSpace() const noexcept;

  [[nodiscard]] qsizetype capacity() const noexcept { return m_capacity; }

  [[nodiscard]] T read(qsizetype size);
  [[nodiscard]] T peek(qsizetype size) const;
  [[nodiscard]] T peekRange(qsizetype offset, qsizetype size) const;

  void discard(qsizetype size);

  [[nodiscard]] int findPatternKMP(const T& pattern, const int pos = 0);
  [[nodiscard]] int findPatternKMP(const T& pattern,
                                   const std::vector<int>& lps,
                                   const int pos = 0);

  [[nodiscard]] std::vector<int> buildKMPTable(const T& p) const { return computeKMPTable(p); }

  /**
   * @brief Single-pass multi-pattern scan result.
   */
  struct MultiMatchResult {
    int position     = -1;
    int patternIndex = -1;
  };

  [[nodiscard]] MultiMatchResult findFirstOfPatterns(const QVector<T>& patterns) const;

  [[nodiscard]] qsizetype overflowCount() const noexcept
  {
    return m_overflowCount.load(std::memory_order_relaxed);
  }

  void resetOverflowCount() noexcept { m_overflowCount.store(0, std::memory_order_relaxed); }

private:
  [[nodiscard]] std::vector<int> computeKMPTable(const T& p) const;
  [[nodiscard]] static int kmpScanLinear(const StorageType* base,
                                         qsizetype current_size,
                                         int pos,
                                         const typename T::value_type* pData,
                                         qsizetype pSize,
                                         const std::vector<int>& lps);
  [[nodiscard]] int kmpScanWrap(qsizetype current_size,
                                int pos,
                                qsizetype head,
                                const typename T::value_type* pData,
                                qsizetype pSize,
                                const std::vector<int>& lps) const;

private:
  static constexpr std::size_t kCacheLine = 64;
  alignas(kCacheLine) std::atomic<qsizetype> m_head;
  alignas(kCacheLine) std::atomic<qsizetype> m_tail;
  alignas(kCacheLine) std::atomic<qsizetype> m_overflowCount;
  qsizetype m_capacity;
  qsizetype m_capacityMask;
  std::vector<StorageType> m_buffer;
};
}  // namespace IO

/**
 * @brief Constructs a CircularBuffer object with a given capacity.
 */
template<typename T, Concepts::ByteLike StorageType>
IO::CircularBuffer<T, StorageType>::CircularBuffer(qsizetype capacity)
  : m_head(0)
  , m_tail(0)
  , m_overflowCount(0)
  , m_capacity(roundUpToPowerOfTwo(capacity))
  , m_capacityMask(m_capacity - 1)
{
  m_buffer.resize(m_capacity);
}

/**
 * @brief Provides direct access to elements in the circular buffer by index.
 */
template<typename T, Concepts::ByteLike StorageType>
StorageType& IO::CircularBuffer<T, StorageType>::operator[](qsizetype index)
{
  const qsizetype current_size = size();
  Q_ASSERT(index >= 0 && index < current_size);
  if (index < 0 || index >= current_size)
    return m_buffer[0];

  const qsizetype head           = m_head.load(std::memory_order_acquire);
  const qsizetype effectiveIndex = (head + index) & m_capacityMask;
  return m_buffer[effectiveIndex];
}

/**
 * @brief Clears the circular buffer.
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
 */
template<typename T, Concepts::ByteLike StorageType>
void IO::CircularBuffer<T, StorageType>::append(const T& data)
{
  const qsizetype dataSize = data.size();
  if (dataSize == 0) [[unlikely]]
    return;

  const uint8_t* src = reinterpret_cast<const uint8_t*>(data.data());

  qsizetype copySize = dataSize;
  if (copySize > m_capacity) [[unlikely]] {
    src      += (copySize - m_capacity);
    copySize  = m_capacity;
    m_overflowCount.fetch_add(dataSize - m_capacity, std::memory_order_relaxed);
  }

  const qsizetype head = m_head.load(std::memory_order_acquire);
  const qsizetype tail = m_tail.load(std::memory_order_relaxed);

  qsizetype current_size = (tail >= head) ? (tail - head) : (m_capacity - head + tail);
  qsizetype free_space   = m_capacity - current_size;

  if (copySize > free_space) [[unlikely]] {
    const qsizetype overwrite = copySize - free_space;
    m_overflowCount.fetch_add(overwrite, std::memory_order_relaxed);

    const qsizetype new_head = (head + overwrite) & m_capacityMask;
    m_head.store(new_head, std::memory_order_release);
  }

  const qsizetype firstChunk = std::min(copySize, m_capacity - tail);
  std::memcpy(&m_buffer[tail], src, firstChunk);

  if (copySize > firstChunk) [[unlikely]]
    std::memcpy(&m_buffer[0], src + firstChunk, copySize - firstChunk);

  const qsizetype new_tail = (tail + copySize) & m_capacityMask;
  m_tail.store(new_tail, std::memory_order_release);
}

/**
 * @brief Clears the buffer and modifies its maximum capacity.
 */
template<typename T, Concepts::ByteLike StorageType>
void IO::CircularBuffer<T, StorageType>::setCapacity(const qsizetype capacity)
{
  clear();
  m_capacity     = roundUpToPowerOfTwo(capacity);
  m_capacityMask = m_capacity - 1;
  m_buffer.resize(m_capacity);
}

/**
 * @brief Returns the current size of the buffer.
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
 */
template<typename T, Concepts::ByteLike StorageType>
qsizetype IO::CircularBuffer<T, StorageType>::freeSpace() const noexcept
{
  return m_capacity - size();
}

/**
 * @brief Reads data from the circular buffer (lock-free SPSC consumer).
 */
template<typename T, Concepts::ByteLike StorageType>
T IO::CircularBuffer<T, StorageType>::read(qsizetype size)
{
  const qsizetype current_size = this->size();
  Q_ASSERT(size <= current_size);
  if (size > current_size) [[unlikely]]
    return T{};

  T result;
  result.resize(size);

  const qsizetype head       = m_head.load(std::memory_order_relaxed);
  const qsizetype firstChunk = std::min(size, m_capacity - head);
  std::memcpy(result.data(), &m_buffer[head], firstChunk);

  if (size > firstChunk) [[unlikely]]
    std::memcpy(result.data() + firstChunk, &m_buffer[0], size - firstChunk);

  const qsizetype new_head = (head + size) & m_capacityMask;
  m_head.store(new_head, std::memory_order_release);

  return result;
}

/**
 * @brief Advances the read head by N bytes without copying. Cheaper than
 *        read() when the caller already has the data (e.g. via peek).
 */
template<typename T, Concepts::ByteLike StorageType>
void IO::CircularBuffer<T, StorageType>::discard(qsizetype size)
{
  const qsizetype current_size = this->size();
  if (size <= 0 || current_size <= 0) [[unlikely]]
    return;

  const qsizetype toAdvance = std::min(size, current_size);
  const qsizetype head      = m_head.load(std::memory_order_relaxed);
  const qsizetype new_head  = (head + toAdvance) & m_capacityMask;
  m_head.store(new_head, std::memory_order_release);
}

/**
 * @brief Retrieves data from the buffer without removing it.
 */
template<typename T, Concepts::ByteLike StorageType>
T IO::CircularBuffer<T, StorageType>::peek(qsizetype size) const
{
  return peekRange(0, size);
}

/**
 * @brief Retrieves data from the buffer at the given logical offset without removing it.
 */
template<typename T, Concepts::ByteLike StorageType>
T IO::CircularBuffer<T, StorageType>::peekRange(qsizetype offset, qsizetype size) const
{
  const qsizetype current_size = this->size();
  if (offset >= current_size)
    return T();

  size = std::min(size, current_size - offset);

  T result;
  result.resize(size);

  const qsizetype head       = m_head.load(std::memory_order_acquire);
  const qsizetype start      = (head + offset) & m_capacityMask;
  const qsizetype firstChunk = std::min(size, m_capacity - start);
  std::memcpy(result.data(), &m_buffer[start], firstChunk);

  if (size > firstChunk) [[unlikely]] {
    const qsizetype secondChunk = size - firstChunk;
    std::memcpy(result.data() + firstChunk, &m_buffer[0], secondChunk);
  }

  return result;
}

/**
 * @brief Searches for a pattern in the circular buffer using the KMP algorithm.
 */
template<typename T, Concepts::ByteLike StorageType>
int IO::CircularBuffer<T, StorageType>::findPatternKMP(const T& pattern, const int pos)
{
  return findPatternKMP(pattern, computeKMPTable(pattern), pos);
}

/**
 * @brief Computes the KMP table for a given pattern.
 */
template<typename T, Concepts::ByteLike StorageType>
int IO::CircularBuffer<T, StorageType>::findPatternKMP(const T& pattern,
                                                       const std::vector<int>& lps,
                                                       const int pos)
{
  const qsizetype current_size = size();
  if (pattern.isEmpty() || current_size < pattern.size()) [[unlikely]]
    return -1;

  const qsizetype head = m_head.load(std::memory_order_acquire);
  const auto pSize     = pattern.size();
  const auto* pData    = pattern.constData();

  // Linear region fast path: no wrap-around, scan with plain pointers (no mask).
  if ((head + current_size) <= m_capacity) [[likely]]
    return kmpScanLinear(m_buffer.data() + head, current_size, pos, pData, pSize, lps);

  return kmpScanWrap(current_size, pos, head, pData, pSize, lps);
}

template<typename T, Concepts::ByteLike StorageType>
int IO::CircularBuffer<T, StorageType>::kmpScanLinear(const StorageType* base,
                                                      qsizetype current_size,
                                                      int pos,
                                                      const typename T::value_type* pData,
                                                      qsizetype pSize,
                                                      const std::vector<int>& lps)
{
  int i = pos, j = 0;
  while (i < current_size) {
    if (base[i] == static_cast<StorageType>(pData[j])) {
      ++i;
      ++j;
    } else if (j != 0) [[likely]] {
      j = lps[j - 1];
      continue;
    } else {
      ++i;
      continue;
    }

    if (j == pSize) [[unlikely]]
      return i - j;
  }

  return -1;
}

template<typename T, Concepts::ByteLike StorageType>
int IO::CircularBuffer<T, StorageType>::kmpScanWrap(qsizetype current_size,
                                                    int pos,
                                                    qsizetype head,
                                                    const typename T::value_type* pData,
                                                    qsizetype pSize,
                                                    const std::vector<int>& lps) const
{
  qsizetype bufferIdx = (head + pos) & m_capacityMask;
  int i = pos, j = 0;
  while (i < current_size) {
    if (m_buffer[bufferIdx] == static_cast<StorageType>(pData[j])) {
      ++i;
      ++j;
      bufferIdx = (bufferIdx + 1) & m_capacityMask;
    } else if (j != 0) [[likely]] {
      j = lps[j - 1];
      continue;
    } else {
      ++i;
      bufferIdx = (bufferIdx + 1) & m_capacityMask;
      continue;
    }

    if (j == pSize) [[unlikely]]
      return i - j;
  }

  return -1;
}

/**
 * @brief Computes the KMP table for a given pattern.
 */
template<typename T, Concepts::ByteLike StorageType>
std::vector<int> IO::CircularBuffer<T, StorageType>::computeKMPTable(const T& p) const
{
  qsizetype m = p.size();
  std::vector<int> lps(m, 0);

  qsizetype len = 0;
  qsizetype i   = 1;

  while (i < m)
    if (p[i] == p[len]) {
      len++;
      lps[i++] = len;
    }

    else if (len != 0)
      len = lps[len - 1];

    else
      lps[i++] = 0;

  return lps;
}

/**
 * @brief Single-pass multi-pattern scan over the circular buffer.
 */
template<typename T, Concepts::ByteLike StorageType>
typename IO::CircularBuffer<T, StorageType>::MultiMatchResult IO::CircularBuffer<T, StorageType>::
  findFirstOfPatterns(const QVector<T>& patterns) const
{
  Q_ASSERT(!patterns.isEmpty());

  const qsizetype bufSize = size();
  if (patterns.isEmpty() || bufSize <= 0) [[unlikely]]
    return {};

  static constexpr int kMaxPatterns = 8;
  Q_ASSERT(patterns.size() <= kMaxPatterns);

  struct PatInfo {
    const StorageType* data;
    qsizetype len;
  };

  PatInfo info[kMaxPatterns];
  const int patCount = qMin(patterns.size(), kMaxPatterns);
  qsizetype minLen   = bufSize;
  for (int p = 0; p < patCount; ++p) {
    info[p].data = reinterpret_cast<const StorageType*>(patterns[p].constData());
    info[p].len  = patterns[p].size();
    if (info[p].len < minLen)
      minLen = info[p].len;
  }

  if (minLen <= 0 || bufSize < minLen) [[unlikely]]
    return {};

  const qsizetype head    = m_head.load(std::memory_order_acquire);
  const qsizetype mask    = m_capacityMask;
  const qsizetype scanEnd = bufSize - minLen + 1;

  auto matchAt = [&](qsizetype i, auto byteAt) -> int {
    for (int p = 0; p < patCount; ++p) {
      const auto pLen = info[p].len;
      if (i + pLen > bufSize)
        continue;

      const auto* pData = info[p].data;
      bool match        = true;
      for (qsizetype j = 0; j < pLen; ++j)
        if (byteAt(i + j) != pData[j]) {
          match = false;
          break;
        }

      if (match)
        return p;
    }

    return -1;
  };

  auto scan = [&](auto byteAt) -> MultiMatchResult {
    for (qsizetype i = 0; i < scanEnd; ++i) {
      const int hit = matchAt(i, byteAt);
      if (hit >= 0)
        return {static_cast<int>(i), hit};
    }

    return {};
  };

  // Linear region fast path: pointer scan, no mask per byte.
  if ((head + bufSize) <= m_capacity) [[likely]] {
    const StorageType* base = m_buffer.data() + head;
    return scan([base](qsizetype k) { return base[k]; });
  }

  // Wrap-around path: mask per byte
  return scan([this, head, mask](qsizetype k) { return m_buffer[(head + k) & mask]; });
}
