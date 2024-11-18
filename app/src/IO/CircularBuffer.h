/*
 * Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <QtCore>

#include <mutex>
#include <vector>
#include <cstring>

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
  explicit CircularBuffer(qsizetype capacity = 1024 * 1024);

  [[nodiscard]] StorageType &operator[](qsizetype index);

  void clear();
  void append(const T &data);

  [[nodiscard]] qsizetype size() const;
  [[nodiscard]] qsizetype freeSpace() const;

  [[nodiscard]] T read(qsizetype size);
  [[nodiscard]] T peek(qsizetype size) const;

  [[nodiscard]] int findPatternKMP(const T &pattern);

private:
  [[nodiscard]] std::vector<int> computeKMPTable(const T &p) const;

private:
  qsizetype m_size;
  qsizetype m_head;
  qsizetype m_tail;
  qsizetype m_capacity;
  std::vector<StorageType> m_buffer;

  mutable std::mutex m_mutex;
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
  std::lock_guard<std::mutex> lock(m_mutex);

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
  std::lock_guard<std::mutex> lock(m_mutex);
  m_size = 0;
  m_head = 0;
  m_tail = 0;
}

/**
 * @brief Appends data to the circular buffer.
 *
 * Adds the given data to the buffer. If the data exceeds the free space, old
 * data is overwritten.
 *
 * @param data The QByteArray containing data to append.
 * @throws std::overflow_error if the data size exceeds the buffer capacity.
 */
template<typename T, typename StorageType>
void IO::CircularBuffer<T, StorageType>::append(const T &data)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  qsizetype dataSize = data.size();
  if (dataSize > m_capacity)
    throw std::overflow_error("Data size exceeds buffer capacity");

  if (freeSpace() < dataSize)
  {
    qsizetype overwrite = dataSize - freeSpace();
    m_head = (m_head + overwrite) % m_capacity;
    m_size -= overwrite;
  }

  for (qsizetype i = 0; i < dataSize; ++i)
  {
    m_buffer[m_tail] = data[i];
    m_tail = (m_tail + 1) % m_capacity;
  }

  m_size += dataSize;
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
  std::lock_guard<std::mutex> lock(m_mutex);

  if (size > m_size)
    throw std::underflow_error("Not enough data in buffer");

  QByteArray result;
  result.resize(size);

  for (qsizetype i = 0; i < size; ++i)
  {
    result[i] = m_buffer[m_head];
    m_head = (m_head + 1) % m_capacity;
  }

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
  std::lock_guard<std::mutex> lock(m_mutex);

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
 * @brief Searches for a pattern in the buffer using the KMP algorithm.
 *
 * Implements the Knuth-Morris-Pratt algorithm to efficiently find a pattern in
 * the buffer.
 *
 * @param pattern The QByteArray representing the pattern to search for.
 * @return The index of the first occurrence of the pattern, or -1 if not found.
 */
template<typename T, typename StorageType>
int IO::CircularBuffer<T, StorageType>::findPatternKMP(const T &pattern)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (pattern.isEmpty() || m_size < pattern.size())
    return -1;

  std::vector<int> lps = computeKMPTable(pattern);
  qsizetype bufferIdx = m_head;

  int i = 0, j = 0;
  while (i < m_size)
  {
    if (m_buffer[bufferIdx] == pattern[j])
    {
      i++;
      j++;
      bufferIdx = (bufferIdx + 1) % m_capacity;

      if (j == pattern.size())
        return i - j;
    }

    else if (j != 0)
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
