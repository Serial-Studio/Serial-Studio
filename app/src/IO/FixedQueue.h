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

#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

#include <memory>
#include <cstddef>
#include <stdexcept>

/**
 * @brief A fixed-capacity, auto-overwriting circular buffer (FIFO queue).
 *
 * This container provides constant-time insertion and access operations.
 * Once full, new elements overwrite the oldest ones. Internally backed by
 * a shared pointer to a heap-allocated array.
 *
 * @tparam T Type of elements stored in the queue.
 */
namespace IO
{
template<typename T>
class FixedQueue
{
public:
  /**
   * @brief Constructs a FixedQueue with a given capacity.
   * @param capacity Maximum number of elements the queue can hold.
   */
  explicit FixedQueue(std::size_t capacity = 100)
    : m_capacity(capacity)
    , m_data(std::shared_ptr<T[]>(new T[capacity]))
    , m_start(0)
    , m_size(0)
  {
  }

  /**
   * @brief Copy constructor.
   *
   * Creates a new FixedQueue as a copy of another.
   * Shared ownership of the internal buffer is maintained via shared_ptr.
   *
   * @param other The FixedQueue to copy from.
   */
  FixedQueue(const FixedQueue &) = default;

  /**
   * @brief Copy assignment operator.
   *
   * Replaces the contents of this FixedQueue with a copy of another.
   * Shared ownership of the internal buffer is maintained via shared_ptr.
   *
   * @param other The FixedQueue to assign from.
   * @return Reference to this FixedQueue.
   */
  FixedQueue &operator=(const FixedQueue &) = default;

  /**
   * @brief Move constructor.
   *
   * Transfers ownership of the internal state from another FixedQueue.
   *
   * @param other The FixedQueue to move from.
   */
  FixedQueue(FixedQueue &&) noexcept = default;

  /**
   * @brief Move assignment operator.
   *
   * Transfers ownership of the internal state from another FixedQueue.
   *
   * @param other The FixedQueue to move from.
   * @return Reference to this FixedQueue.
   */
  FixedQueue &operator=(FixedQueue &&) noexcept = default;

  /**
   * @brief Provides read-only access to an element at a given index.
   *
   * @param index Index relative to the logical front of the queue.
   * @return Reference to the element at the specified index.
   * @throws std::out_of_range if index is invalid.
   */
  [[nodiscard]] const T &operator[](std::size_t index) const
  {
    if (index >= m_capacity)
      throw std::out_of_range("FixedQueue index out of bounds");

    return m_data[wrappedIndex(index)];
  }

  /**
   * @brief Provides mutable access to an element at a given index.
   *
   * @param index Index relative to the logical front of the queue.
   * @return Reference to the element at the specified index.
   * @throws std::out_of_range if index is invalid.
   */
  [[nodiscard]] T &operator[](std::size_t index)
  {
    if (index >= m_capacity)
      throw std::out_of_range("FixedQueue index out of bounds");

    return m_data[wrappedIndex(index)];
  }

  /**
   * @brief Returns the current number of elements in the queue.
   * @return Number of elements.
   */
  [[nodiscard]] std::size_t size() const { return m_size; }

  /**
   * @brief Returns the maximum capacity of the queue.
   * @return Capacity of the queue.
   */
  [[nodiscard]] std::size_t capacity() const { return m_capacity; }

  /**
   * @brief Checks whether the queue is full.
   * @return True if the queue is full.
   */
  [[nodiscard]] bool full() const { return m_size == m_capacity; }

  /**
   * @brief Checks whether the queue is empty.
   * @return True if the queue is empty.
   */
  [[nodiscard]] bool empty() const { return m_size == 0; }

  /**
   * @brief Returns the internal index of the front (oldest) element.
   * This is the raw index into the underlying buffer where the first valid
   * element resides. Useful for direct buffer access or optimized iteration.
   *
   * @return Index of the front element in the internal array.
   */
  [[nodiscard]] std::size_t frontIndex() const { return m_start; }

  /**
   * @brief Provides read-only access to an element at a given index.
   *
   * @param index Index relative to the logical front of the queue.
   * @return Reference to the element at the specified index.
   * @throws std::out_of_range if index is invalid.
   */
  [[nodiscard]] const T &at(std::size_t index) const
  {
    if (index >= m_capacity)
      throw std::out_of_range("FixedQueue index out of bounds");

    return m_data[wrappedIndex(index)];
  }

  /**
   * @brief Returns a reference to the first (oldest) element in the queue.
   *
   * Equivalent to `std::queue::front()`.
   *
   * @return Reference to the oldest element in the buffer.
   * @throws std::out_of_range if the queue is empty.
   */
  [[nodiscard]] const T &front() const
  {
    if (m_size == 0)
      throw std::out_of_range("FixedQueue::front() called on empty queue");

    return m_data[m_start];
  }

  /**
   * @brief Returns a reference to the most recently added element in the queue.
   *
   * Equivalent to `std::queue::back()`.
   *
   * @return Reference to the most recently inserted element in the buffer.
   * @throws std::out_of_range if the queue is empty.
   */
  [[nodiscard]] const T &back() const
  {
    if (m_size == 0)
      throw std::out_of_range("FixedQueue::back() called on empty queue");

    return m_data[wrappedIndex(m_size - 1)];
  }

  /**
   * @brief Returns a raw pointer to the internal buffer (read-only).
   * @return Pointer to the beginning of the internal array.
   */
  [[nodiscard]] const T *raw() const { return m_data.get(); }

  /**
   * @brief Returns a raw pointer to the internal buffer (mutable).
   * @return Pointer to the beginning of the internal array.
   */
  [[nodiscard]] T *raw() { return m_data.get(); }

  /**
   * @brief Clears all elements from the queue.
   */
  void clear()
  {
    m_start = 0;
    m_size = 0;
  }

  /**
   * @brief Fills the queue with a repeated value, overwriting all contents.
   * @param value The value to fill.
   */
  void fill(const T &value)
  {
    clear();
    for (std::size_t i = 0; i < m_capacity; ++i)
      push(value);
  }

  /**
   * @brief Fills the queue with increasing values starting from a given base.
   * @param start Starting value.
   * @param step  Increment step (default: 1).
   */
  void fillRange(const T &start, const T &step = 1)
  {
    clear();
    for (std::size_t i = 0; i < m_capacity; ++i)
      push(start + static_cast<T>(i) * step);
  }

  /**
   * @brief Inserts an element by copy. Overwrites oldest element if full.
   * @param item The element to insert.
   */
  void push(const T &item)
  {
    m_data[endIndex()] = item;
    advance();
  }

  /**
   * @brief Inserts an element by move. Overwrites oldest element if full.
   * @param item The element to insert.
   */
  void push(T &&item)
  {
    m_data[endIndex()] = std::move(item);
    advance();
  }

  /**
   * @brief Resizes the queue to a new capacity, preserving the most recent
   * elements.
   *
   * If the new capacity is smaller than the current size, the oldest elements
   * are discarded.
   *
   * @param newCapacity The desired new capacity of the queue.
   */
  void resize(std::size_t newCapacity)
  {
    if (newCapacity == m_capacity)
      return;

    std::shared_ptr<T[]> newData(new T[newCapacity]);
    std::size_t elementsToCopy = std::min(m_size, newCapacity);
    for (std::size_t i = 0; i < elementsToCopy; ++i)
      newData[i] = std::move((*this)[m_size - elementsToCopy + i]);

    m_start = 0;
    m_size = elementsToCopy;
    m_capacity = newCapacity;
    m_data = std::move(newData);
  }

private:
  /**
   * @brief Computes the index where the next element will be inserted.
   * @return Index in the underlying array.
   */
  std::size_t endIndex() const { return (m_start + m_size) % m_capacity; }

  /**
   * @brief Computes the actual buffer index for a logical element index.
   * @param index Logical index (0 = front of queue).
   * @return Physical index in the underlying array.
   */
  std::size_t wrappedIndex(std::size_t index) const
  {
    return (m_start + index) % m_capacity;
  }

  /**
   * @brief Advances the internal index after an insertion.
   *        If the queue is full, the oldest element is overwritten.
   */
  void advance()
  {
    if (m_size < m_capacity)
      ++m_size;
    else
      m_start = (m_start + 1) % m_capacity;
  }

private:
  std::size_t m_capacity;      ///< Maximum number of elements.
  std::shared_ptr<T[]> m_data; ///< Shared pointer to the internal buffer.
  std::size_t m_start;         ///< Index of the oldest element.
  std::size_t m_size;          ///< Current number of elements.
};
} // namespace IO
