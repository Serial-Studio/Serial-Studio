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

#include <vector>
#include <cstring>
#include <QByteArray>

namespace IO
{
/**
 * @brief A generic circular buffer for managing data with fixed capacity.
 *
 * This templated class provides a circular buffer implementation for efficient
 * storage and retrieval of elements. It supports a customizable storage type
 * for internal representation, allowing for optimized memory usage.
 *
 * @tparam T The type of elements exposed to the user (e.g., QByteArray,
 *           QString).
 * @tparam StorageType The type of elements used internally in the buffer
 *                      (default: uint8_t).
 */
template<typename T, typename StorageType = uint8_t>
class CircularBuffer
{
public:
  explicit CircularBuffer(qsizetype capacity = 1024 * 1024 * 10);

  [[nodiscard]] StorageType &operator[](qsizetype index);

  void clear();
  void append(const T &data);
  void setCapacity(const qsizetype capacity);

  [[nodiscard]] qsizetype size() const;
  [[nodiscard]] qsizetype freeSpace() const;

  [[nodiscard]] T read(qsizetype size);
  [[nodiscard]] T peek(qsizetype size) const;

  [[nodiscard]] int findPatternKMP(const T &pattern, const int pos = 0);

private:
  [[nodiscard]] std::vector<int> computeKMPTable(const T &p) const;

private:
  qsizetype m_size;
  qsizetype m_head;
  qsizetype m_tail;
  qsizetype m_capacity;
  std::vector<StorageType> m_buffer;
};
} // namespace IO

/**
 * @brief Constructs a CircularBuffer object with a given capacity.
 *
 * Initializes the circular buffer with the specified capacity, setting up
 * internal variables.
 *
 * @param capacity The maximum capacity of the buffer in bytes.
 */
template<typename T, typename StorageType>
IO::CircularBuffer<T, StorageType>::CircularBuffer(qsizetype capacity)
  : m_size(0)
  , m_head(0)
  , m_tail(0)
  , m_capacity(capacity)
{
  m_buffer.resize(capacity);
}

/**
 * @brief Provides direct access to elements in the circular buffer by index.
 *
 * This subscript operator allows accessing elements stored in the circular
 * buffer at a specific logical index, taking the circular nature of the buffer
 * into account. It performs bounds checking and ensures thread-safe access.
 *
 * @param index The logical index of the element to access (0-based).
 *              Must be in the range [0, size()-1].
 * @return The element at the specified index.
 * @throws std::out_of_range If the index is out of bounds.
 */
template<typename T, typename StorageType>
StorageType &IO::CircularBuffer<T, StorageType>::operator[](qsizetype index)
{
  if (index < 0 || index >= m_size)
    throw std::out_of_range("Index out of range");

  qsizetype effectiveIndex = (m_head + index) % m_capacity;
  return m_buffer[effectiveIndex];
}

/**
 * @brief Clears the circular buffer.
 *
 * Resets the buffer, removing all data and resetting internal variables.
 */
template<typename T, typename StorageType>
void IO::CircularBuffer<T, StorageType>::clear()
{
  m_size = 0;
  m_head = 0;
  m_tail = 0;
}

/**
 * @brief Appends data to the circular buffer.
 *
 * Adds the given data to the buffer. If the data exceeds available space,
 * the oldest data is overwritten. If the data exceeds total capacity, only
 * the last `capacity` bytes are stored.
 *
 * @param data The QByteArray containing data to append.
 */
template<typename T, typename StorageType>
void IO::CircularBuffer<T, StorageType>::append(const T &data)
{
  // Obtain the length of the input data & a pointer to it
  const qsizetype dataSize = data.size();
  const uint8_t *src = reinterpret_cast<const uint8_t *>(data.data());

  // If input is too large, keep only the last bytes that fit
  qsizetype copySize = dataSize;
  if (copySize > m_capacity)
  {
    src += (copySize - m_capacity);
    copySize = m_capacity;
  }

  // If not enough free space, advance head to overwrite old data
  if (copySize > freeSpace())
  {
    const qsizetype overwrite = copySize - freeSpace();
    m_head = (m_head + overwrite) % m_capacity;
    m_size -= overwrite;
  }

  // Copy first chunk (may wrap around)
  const qsizetype firstChunk = std::min(copySize, m_capacity - m_tail);
  std::memcpy(&m_buffer[m_tail], src, firstChunk);

  // Copy second chunk if wrapped
  if (copySize > firstChunk)
    std::memcpy(&m_buffer[0], src + firstChunk, copySize - firstChunk);

  // Advance tail and update size
  m_tail = (m_tail + copySize) % m_capacity;
  m_size = std::min(m_size + copySize, m_capacity);
}

/**
 * Clears the buffer and modifies it's maximum capacity.
 */
template<typename T, typename StorageType>
void IO::CircularBuffer<T, StorageType>::setCapacity(const qsizetype capacity)
{
  clear();
  m_capacity = capacity;
  m_buffer.resize(capacity);
}

/**
 * @brief Returns the current size of the buffer.
 *
 * @return The number of bytes currently stored in the buffer.
 */
template<typename T, typename StorageType>
qsizetype IO::CircularBuffer<T, StorageType>::size() const
{
  return m_size;
}

/**
 * @brief Returns the free space available in the buffer.
 *
 * @return The number of bytes of free space in the buffer.
 */
template<typename T, typename StorageType>
qsizetype IO::CircularBuffer<T, StorageType>::freeSpace() const
{
  return m_capacity - m_size;
}

/**
 * @brief Reads data from the circular buffer.
 *
 * Reads the specified number of bytes from the buffer. The read data is removed
 * from the buffer.
 *
 * @param size The number of bytes to read.
 * @return A QByteArray containing the read data.
 * @throws std::underflow_error if there is not enough data in the buffer.
 */
template<typename T, typename StorageType>
T IO::CircularBuffer<T, StorageType>::read(qsizetype size)
{
  if (size > m_size)
    throw std::underflow_error("Not enough data in buffer");

  T result;
  result.resize(size);

  const qsizetype firstChunk = std::min(size, m_capacity - m_head);
  std::memcpy(result.data(), &m_buffer[m_head], firstChunk);

  if (size > firstChunk)
    std::memcpy(result.data() + firstChunk, &m_buffer[0], size - firstChunk);

  m_head = (m_head + size) % m_capacity;
  m_size -= size;

  return result;
}

/**
 * @brief Retrieves data from the buffer without removing it.
 *
 * Extracts up to the specified number of bytes from the buffer, starting from
 * the current head position, without modifying the buffer's head or tail
 * positions.
 *
 * @param size The number of bytes to peek from the buffer.
 * @return A QByteArray containing the requested data. If the buffer
 *         contains less data than requested, the returned QByteArray
 *         will be smaller.
 */
template<typename T, typename StorageType>
T IO::CircularBuffer<T, StorageType>::peek(qsizetype size) const
{
  size = std::min(size, m_size);

  T result;
  result.resize(size);

  qsizetype firstChunk = std::min(size, m_capacity - m_head);
  std::memcpy(result.data(), &m_buffer[m_head], firstChunk);

  if (size > firstChunk)
  {
    size_t secondChunk = size - firstChunk;
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
 * @tparam T The type of elements stored in the pattern and the buffer.
 * @tparam StorageType The type of storage used for the circular buffer.
 * @param pattern The pattern to search for in the buffer.
 * @param pos The starting position (relative to the logical start of the
 *            buffer) for the search. Defaults to the beginning if set to 0.
 *
 * @return The index (relative to the logical start of the buffer) of the first
 *         occurrence of the pattern, or -1 if the pattern is not found.
 *
 * @warning If the `pattern` is empty or its size exceeds the current size of
 *          the buffer, the function returns -1 immediately.
 *
 * @pre The buffer must be properly initialized, and the pattern should not
 *      exceed the capacity of the buffer.
 */
template<typename T, typename StorageType>
int IO::CircularBuffer<T, StorageType>::findPatternKMP(const T &pattern,
                                                       const int pos)
{
  // Validate search pattern
  if (pattern.isEmpty() || m_size < pattern.size())
    return -1;

  // Start search at `pos`
  std::vector<int> lps = computeKMPTable(pattern);
  qsizetype bufferIdx = (m_head + pos) % m_capacity;
  int i = pos, j = 0;

  while (i < m_size)
  {
    // Compare current buffer character with the pattern
    if (m_buffer[bufferIdx] == pattern[j])
    {
      i++;
      j++;
      bufferIdx = (bufferIdx + 1) % m_capacity;

      // If the whole pattern is matched, return the logical start index
      if (j == pattern.size())
      {
        int matchStart = i - j;
        return matchStart;
      }
    }

    // Mismatch after some matches, fall back in pattern
    else if (j != 0)
      j = lps[j - 1];

    // Mismatch at the start, move forward
    else
    {
      i++;
      bufferIdx = (bufferIdx + 1) % m_capacity;
    }
  }

  // Pattern not found
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
template<typename T, typename StorageType>
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
