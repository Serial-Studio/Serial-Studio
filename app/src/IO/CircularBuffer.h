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
#include <cstring>
#include <QByteArray>
#include <vector>

#include "Concepts.h"

namespace IO {
/**
 * @brief Lock-free SPSC circular buffer for byte streaming.
 */
template<typename T, Concepts::ByteLike StorageType = uint8_t>
class CircularBuffer {
public:
  explicit CircularBuffer(qsizetype capacity = 1024 * 1024 * 10);

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

  [[nodiscard]] int findPatternKMP(const T& pattern, const int pos = 0);
  [[nodiscard]] int findPatternKMP(const T& pattern,
                                   const std::vector<int>& lps,
                                   const int pos = 0);

  [[nodiscard]] std::vector<int> buildKMPTable(const T& p) const { return computeKMPTable(p); }

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

private:
  std::atomic<qsizetype> m_head;
  std::atomic<qsizetype> m_tail;
  std::atomic<qsizetype> m_overflowCount;
  qsizetype m_capacity;
  std::vector<StorageType> m_buffer;
};
}  // namespace IO

template<typename T, Concepts::ByteLike StorageType>
IO::CircularBuffer<T, StorageType>::CircularBuffer(qsizetype capacity)
  : m_head(0), m_tail(0), m_overflowCount(0), m_capacity(capacity)
{
  m_buffer.resize(capacity);
}

template<typename T, Concepts::ByteLike StorageType>
StorageType& IO::CircularBuffer<T, StorageType>::operator[](qsizetype index)
{
  const qsizetype current_size = size();
  Q_ASSERT(index >= 0 && index < current_size);
  if (index < 0 || index >= current_size)
    return m_buffer[0];

  const qsizetype head           = m_head.load(std::memory_order_acquire);
  const qsizetype effectiveIndex = (head + index) % m_capacity;
  return m_buffer[effectiveIndex];
}

template<typename T, Concepts::ByteLike StorageType>
void IO::CircularBuffer<T, StorageType>::clear()
{
  m_head.store(0, std::memory_order_release);
  m_tail.store(0, std::memory_order_release);
  m_overflowCount.store(0, std::memory_order_relaxed);
}

template<typename T, Concepts::ByteLike StorageType>
void IO::CircularBuffer<T, StorageType>::append(const T& data)
{
  const qsizetype dataSize = data.size();
  if (dataSize == 0) [[unlikely]]
    return;

  const uint8_t* src = reinterpret_cast<const uint8_t*>(data.data());

  qsizetype copySize = dataSize;
  if (copySize > m_capacity) [[unlikely]] {
    src += (copySize - m_capacity);
    copySize = m_capacity;
    m_overflowCount.fetch_add(dataSize - m_capacity, std::memory_order_relaxed);
  }

  const qsizetype head = m_head.load(std::memory_order_acquire);
  const qsizetype tail = m_tail.load(std::memory_order_relaxed);

  qsizetype current_size = (tail >= head) ? (tail - head) : (m_capacity - head + tail);
  qsizetype free_space   = m_capacity - current_size;

  if (copySize > free_space) [[unlikely]] {
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

template<typename T, Concepts::ByteLike StorageType>
void IO::CircularBuffer<T, StorageType>::setCapacity(const qsizetype capacity)
{
  clear();
  m_capacity = capacity;
  m_buffer.resize(capacity);
}

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

template<typename T, Concepts::ByteLike StorageType>
qsizetype IO::CircularBuffer<T, StorageType>::freeSpace() const noexcept
{
  return m_capacity - size();
}

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

  const qsizetype new_head = (head + size) % m_capacity;
  m_head.store(new_head, std::memory_order_release);

  return result;
}

template<typename T, Concepts::ByteLike StorageType>
T IO::CircularBuffer<T, StorageType>::peek(qsizetype size) const
{
  return peekRange(0, size);
}

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
  const qsizetype start      = (head + offset) % m_capacity;
  const qsizetype firstChunk = std::min(size, m_capacity - start);
  std::memcpy(result.data(), &m_buffer[start], firstChunk);

  if (size > firstChunk) [[unlikely]] {
    const qsizetype secondChunk = size - firstChunk;
    std::memcpy(result.data() + firstChunk, &m_buffer[0], secondChunk);
  }

  return result;
}

template<typename T, Concepts::ByteLike StorageType>
int IO::CircularBuffer<T, StorageType>::findPatternKMP(const T& pattern, const int pos)
{
  return findPatternKMP(pattern, computeKMPTable(pattern), pos);
}

template<typename T, Concepts::ByteLike StorageType>
int IO::CircularBuffer<T, StorageType>::findPatternKMP(const T& pattern,
                                                       const std::vector<int>& lps,
                                                       const int pos)
{
  const qsizetype current_size = size();
  if (pattern.isEmpty() || current_size < pattern.size()) [[unlikely]]
    return -1;

  const qsizetype head = m_head.load(std::memory_order_acquire);
  qsizetype bufferIdx  = (head + pos) % m_capacity;
  int i = pos, j = 0;

  while (i < current_size) {
    if (m_buffer[bufferIdx] == pattern[j]) {
      i++;
      j++;
      bufferIdx = (bufferIdx + 1) % m_capacity;

      if (j == pattern.size()) [[unlikely]] {
        int matchStart = i - j;
        return matchStart;
      }
    }

    else if (j != 0) [[likely]]
      j = lps[j - 1];

    else {
      i++;
      bufferIdx = (bufferIdx + 1) % m_capacity;
    }
  }

  return -1;
}

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
  const qsizetype cap     = m_capacity;
  const qsizetype scanEnd = bufSize - minLen + 1;

  for (qsizetype i = 0; i < scanEnd; ++i) {
    for (int p = 0; p < patCount; ++p) {
      const auto pLen = info[p].len;
      if (i + pLen > bufSize)
        continue;

      const auto* pData = info[p].data;
      bool match        = true;
      for (qsizetype j = 0; j < pLen; ++j) {
        if (m_buffer[(head + i + j) % cap] != pData[j]) {
          match = false;
          break;
        }
      }

      if (match)
        return {static_cast<int>(i), p};
    }
  }

  return {};
}
