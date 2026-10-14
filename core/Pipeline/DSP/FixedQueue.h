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

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <memory>
#include <QDebug>
#include <type_traits>
#include <utility>

#include "Core/Concepts.h"
#include "Core/SSAssert.h"

namespace DSP {
//--------------------------------------------------------------------------------------------------
// Fixed queue implementation with a circular buffer
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rounds @a value up to the next power of two (>= 2). Used by FixedQueue
 *        to size its backing storage so wrap-around can mask with `& (cap-1)`.
 */
[[nodiscard]] inline std::size_t roundUpToPowerOfTwo(std::size_t value) noexcept
{
  std::size_t v = value < 2 ? 2 : value;
  --v;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  if constexpr (sizeof(std::size_t) > 4)
    v |= v >> 32;

  return v + 1;
}

/**
 * @brief Tag selecting FixedQueue storage sized exactly to the capacity instead of rounded up
 *        to a power of two. Such a queue has no storageMask(); it is for rings whose readers
 *        walk spans (time rings), where the rounding could double the bytes held.
 */
struct ExactStorage {};

/**
 * @brief A fixed-capacity, auto-overwriting circular buffer (FIFO queue). Storage is a power
 *        of two by default (mask-walking readers rely on it) or exactly the capacity with the
 *        ExactStorage tag; the queue itself wraps by compare, so both shapes share one path.
 */
template<typename T>
  requires std::copy_constructible<T> && std::is_copy_assignable_v<T>
class FixedQueue {
public:
  /**
   * @brief Constructs a FixedQueue with a given logical capacity. Backing storage
   *        is rounded up to the next power of two for mask-based readers.
   */
  explicit FixedQueue(std::size_t capacity = 100)
    : m_capacity(capacity < 1 ? 1 : capacity)
    , m_storageCapacity(roundUpToPowerOfTwo(capacity < 1 ? 1 : capacity))
    , m_storageMask(m_storageCapacity - 1)
    , m_exactStorage(false)
    , m_data(makeStorage(m_storageCapacity))
    , m_start(0)
    , m_size(0)
  {}

  /**
   * @brief Constructs a FixedQueue whose backing storage is exactly @p capacity elements.
   */
  FixedQueue(std::size_t capacity, ExactStorage)
    : m_capacity(capacity < 1 ? 1 : capacity)
    , m_storageCapacity(capacity < 1 ? 1 : capacity)
    , m_storageMask(m_storageCapacity - 1)
    , m_exactStorage(true)
    , m_data(makeStorage(m_storageCapacity))
    , m_start(0)
    , m_size(0)
  {}

  /**
   * @brief Copy constructor.
   */
  FixedQueue(const FixedQueue&) = default;

  /**
   * @brief Copy assignment operator.
   */
  FixedQueue& operator=(const FixedQueue&) = default;

  /**
   * @brief Move constructor.
   */
  FixedQueue(FixedQueue&&) noexcept = default;

  /**
   * @brief Move assignment operator.
   */
  FixedQueue& operator=(FixedQueue&&) noexcept = default;

  /**
   * @brief Provides read-only access to an element at a given index.
   */
  [[nodiscard]] const T& operator[](std::size_t index) const
  {
    SS_ASSERT_LOG(index < m_size);
    return m_data[wrappedIndex(index < m_size ? index : 0)];
  }

  /**
   * @brief Provides mutable access to an element at a given index.
   */
  [[nodiscard]] T& operator[](std::size_t index)
  {
    SS_ASSERT_LOG(index < m_size);
    return m_data[wrappedIndex(index < m_size ? index : 0)];
  }

  /**
   * @brief Returns the current number of elements in the queue.
   */
  [[nodiscard]] std::size_t size() const { return m_size; }

  /**
   * @brief Returns the maximum capacity of the queue.
   */
  [[nodiscard]] std::size_t capacity() const { return m_capacity; }

  /**
   * @brief Checks whether the queue is full.
   */
  [[nodiscard]] bool full() const { return m_size == m_capacity; }

  /**
   * @brief Checks whether the queue is empty.
   */
  [[nodiscard]] bool empty() const { return m_size == 0; }

  /**
   * @brief Returns the storage index of the front (oldest) element. Pair with
   *        `nextIndex()` (or `storageMask()`) to walk the ring without modulo.
   */
  [[nodiscard]] std::size_t frontIndex() const { return m_start; }

  /**
   * @brief Returns the next storage position when walking the ring from `frontIndex()`,
   *        for either storage shape. Use instead of `% capacity()`.
   */
  [[nodiscard]] std::size_t nextIndex(std::size_t idx) const noexcept
  {
    return idx + 1 >= m_storageCapacity ? 0 : idx + 1;
  }

  /**
   * @brief Returns the storage AND-mask. Lets advanced callers inline the wrap
   *        (`idx = (idx + n) & mask`) when walking by a stride > 1. Power-of-two storage
   *        only: an ExactStorage queue has no mask.
   */
  [[nodiscard]] std::size_t storageMask() const noexcept
  {
    SS_ASSERT_LOG(!m_exactStorage);
    return m_storageMask;
  }

  /**
   * @brief Returns the backing storage size (>= capacity()): a power of two by default,
   *        exactly capacity() for an ExactStorage queue. Span/wrap math on raw indices
   *        wraps at this value.
   */
  [[nodiscard]] std::size_t storageCapacity() const noexcept { return m_storageCapacity; }

  /**
   * @brief True when the backing storage is exactly capacity() (ExactStorage).
   */
  [[nodiscard]] bool exactStorage() const noexcept { return m_exactStorage; }

  /**
   * @brief Provides read-only access to an element at a given index.
   */
  [[nodiscard]] const T& at(std::size_t index) const
  {
    SS_ASSERT_LOG(index < m_capacity);
    return m_data[wrappedIndex(index < m_capacity ? index : 0)];
  }

  /**
   * @brief Returns a reference to the first (oldest) element in the queue.
   */
  [[nodiscard]] const T& front() const
  {
    if (m_size == 0) [[unlikely]] {
      qWarning() << "DSP::RingBuffer::front() called on empty buffer";
      return m_data[0];
    }

    return m_data[m_start];
  }

  /**
   * @brief Returns a reference to the most recently added element in the queue.
   */
  [[nodiscard]] const T& back() const
  {
    if (m_size == 0) [[unlikely]] {
      qWarning() << "DSP::RingBuffer::back() called on empty buffer";
      return m_data[0];
    }

    return m_data[wrappedIndex(m_size - 1)];
  }

  /**
   * @brief Returns a raw pointer to the internal buffer (read-only).
   */
  [[nodiscard]] const T* raw() const { return m_data.get(); }

  /**
   * @brief Returns a raw pointer to the internal buffer (mutable).
   */
  [[nodiscard]] T* raw() { return m_data.get(); }

  /**
   * @brief Clears all elements from the queue.
   */
  void clear()
  {
    m_start = 0;
    m_size  = 0;
  }

  /**
   * @brief Fills the queue with a repeated value, overwriting all contents.
   */
  void fill(const T& value)
  {
    clear();
    for (std::size_t i = 0; i < m_capacity; ++i)
      push(value);
  }

  /**
   * @brief Fills the queue with increasing values starting from a given base.
   */
  void fillRange(const T& start, const T& step = 1)
    requires Concepts::Numeric<T>
  {
    clear();
    for (std::size_t i = 0; i < m_capacity; ++i)
      push(start + static_cast<T>(i) * step);
  }

  /**
   * @brief Inserts an element by copy. Overwrites oldest element if full.
   */
  void push(const T& item)
  {
    m_data[endIndex()] = item;
    advance();
  }

  /**
   * @brief Inserts an element by move. Overwrites oldest element if full.
   */
  void push(T&& item)
  {
    m_data[endIndex()] = std::move(item);
    advance();
  }

  /**
   * @brief Resizes the queue to a new capacity, preserving the most recent elements.
   */
  void resize(std::size_t newCapacity)
  {
    if (newCapacity < 1)
      newCapacity = 1;

    if (newCapacity == m_capacity)
      return;

    const std::size_t newStorage = m_exactStorage ? newCapacity : roundUpToPowerOfTwo(newCapacity);
    std::shared_ptr<T[]> newData = makeStorage(newStorage);
    std::size_t elementsToCopy   = std::min(m_size, newCapacity);
    for (std::size_t i = 0; i < elementsToCopy; ++i)
      newData[i] = std::move((*this)[m_size - elementsToCopy + i]);

    m_start           = 0;
    m_size            = elementsToCopy;
    m_capacity        = newCapacity;
    m_storageCapacity = newStorage;
    m_storageMask     = newStorage - 1;
    m_data            = std::move(newData);
  }

private:
  /**
   * @brief Allocates the backing array. Display storage is ordinary pageable memory (spec
   *        0097 R6): untouched capacity stays uncommitted and the OS may compress it. Pinning
   *        belongs to the acquisition pipeline's own buffers only.
   */
  [[nodiscard]] static std::shared_ptr<T[]> makeStorage(std::size_t n)
  {
    SS_ASSERT_LOG(n > 0);
    return std::shared_ptr<T[]>(new T[n]);
  }

  /**
   * @brief Wraps a storage index in [0, 2 * storageCapacity) back into the storage.
   */
  [[nodiscard]] std::size_t wrap(std::size_t index) const noexcept
  {
    return index >= m_storageCapacity ? index - m_storageCapacity : index;
  }

  /**
   * @brief Computes the storage index where the next element will be inserted.
   */
  std::size_t endIndex() const { return wrap(m_start + m_size); }

  /**
   * @brief Computes the storage index for a logical element index.
   */
  std::size_t wrappedIndex(std::size_t index) const { return wrap(m_start + index); }

  /**
   * @brief Advances the internal index after an insertion.
   *        If the queue is full, the oldest element is overwritten.
   */
  void advance()
  {
    if (m_size < m_capacity)
      ++m_size;
    else
      m_start = wrap(m_start + 1);
  }

private:
  std::size_t m_capacity;         ///< Logical (user-requested) capacity.
  std::size_t m_storageCapacity;  ///< Backing storage size (>= m_capacity).
  std::size_t m_storageMask;      ///< m_storageCapacity - 1; AND-mask for pow2 storage only.
  bool m_exactStorage;            ///< Storage is exactly m_capacity (no pow2 rounding).
  std::shared_ptr<T[]> m_data;    ///< Shared pointer to the internal buffer.
  std::size_t m_start;            ///< Storage index of the oldest element.
  std::size_t m_size;             ///< Current number of elements.
};

}  // namespace DSP
