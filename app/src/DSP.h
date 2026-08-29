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

#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <QDebug>
#include <QList>
#include <QObject>
#include <QPointF>
#include <QVector3D>
#include <QVector>
#include <stdexcept>
#include <vector>

#include "Concepts.h"
#include "DSPSimd.h"
#include "Platform/AppPlatform.h"
#include "SSAssert.h"

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
 * @brief A fixed-capacity, auto-overwriting circular buffer (FIFO queue) with pow2 storage.
 */
template<typename T>
  requires std::copy_constructible<T> && std::is_copy_assignable_v<T>
class FixedQueue {
public:
  /**
   * @brief Constructs a FixedQueue with a given logical capacity. Backing storage
   *        is rounded up to the next power of two for fast wrap-around.
   */
  explicit FixedQueue(std::size_t capacity = 100)
    : m_capacity(capacity < 1 ? 1 : capacity)
    , m_storageCapacity(roundUpToPowerOfTwo(capacity < 1 ? 1 : capacity))
    , m_storageMask(m_storageCapacity - 1)
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
   * @brief Returns `(idx + 1) & storageMask` -- the next storage position when
   *        walking the ring from `frontIndex()`. Use instead of `% capacity()`.
   */
  [[nodiscard]] std::size_t nextIndex(std::size_t idx) const noexcept
  {
    return (idx + 1) & m_storageMask;
  }

  /**
   * @brief Returns the storage AND-mask. Lets advanced callers inline the
   *        wrap (`idx = (idx + n) & mask`) when walking by a stride > 1.
   */
  [[nodiscard]] std::size_t storageMask() const noexcept { return m_storageMask; }

  /**
   * @brief Returns the backing storage size (>= capacity()), always a power of two.
   *        Use for span/wrap math that operates on the raw buffer indices.
   */
  [[nodiscard]] std::size_t storageCapacity() const noexcept { return m_storageCapacity; }

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

    const std::size_t newStorage = roundUpToPowerOfTwo(newCapacity);
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
  // Plot-scale buffers are written per frame; smaller ones would waste page-locked quota
  static constexpr std::size_t kPinThresholdBytes = 64 * 1024;

  /**
   * @brief Allocates the backing array, pinning plot-scale buffers into physical memory. The
   *        deleter owns the unpin, so shared copies / snapshots / resize stay leak-free.
   */
  [[nodiscard]] static std::shared_ptr<T[]> makeStorage(std::size_t n)
  {
    SS_ASSERT_LOG(n > 0);

    T* data                 = new T[n];
    const std::size_t bytes = n * sizeof(T);
    if (bytes >= kPinThresholdBytes && Platform::AppPlatform::lockMemoryResident(data, bytes)) {
      return std::shared_ptr<T[]>(data, [bytes](T* p) {
        Platform::AppPlatform::unlockMemoryResident(p, bytes);
        delete[] p;
      });
    }

    return std::shared_ptr<T[]>(data);
  }

  /**
   * @brief Computes the storage index where the next element will be inserted.
   */
  std::size_t endIndex() const { return (m_start + m_size) & m_storageMask; }

  /**
   * @brief Computes the storage index for a logical element index.
   */
  std::size_t wrappedIndex(std::size_t index) const { return (m_start + index) & m_storageMask; }

  /**
   * @brief Advances the internal index after an insertion.
   *        If the queue is full, the oldest element is overwritten.
   */
  void advance()
  {
    if (m_size < m_capacity)
      ++m_size;
    else
      m_start = (m_start + 1) & m_storageMask;
  }

private:
  std::size_t m_capacity;         ///< Logical (user-requested) capacity.
  std::size_t m_storageCapacity;  ///< Backing storage size (pow2, >= m_capacity).
  std::size_t m_storageMask;      ///< m_storageCapacity - 1; AND-mask for wrap.
  std::shared_ptr<T[]> m_data;    ///< Shared pointer to the internal buffer.
  std::size_t m_start;            ///< Storage index of the oldest element.
  std::size_t m_size;             ///< Current number of elements.
};

//--------------------------------------------------------------------------------------------------
// Type Aliases
//--------------------------------------------------------------------------------------------------

/**
 * @brief Data type to use for all Serial Studio mathematical structures.
 */
typedef double ssfp_t;

/**
 * @typedef PlotDataX
 * @brief Represents the unique X-axis data points for a plot.
 */
typedef FixedQueue<ssfp_t> AxisData;

/**
 * @brief Represents Y-axis data for multiple curves in a multiplot.
 */
typedef std::vector<AxisData> MultiPlotDataY;

//--------------------------------------------------------------------------------------------------
// Composite Data Structures
//--------------------------------------------------------------------------------------------------

/**
 * @brief Represents a paired series of X-axis and Y-axis data for a plot.
 */
typedef struct {
  AxisData* x;  ///< X-axis data (e.g., time or samples)
  AxisData* y;  ///< Y-axis data (e.g., sensor readings)
} LineSeries;

/**
 * @brief Represents a set of line plots sharing a common X-axis.
 */
typedef struct {
  AxisData* x;              ///< Shared X-axis data (e.g., time or index)
  std::vector<AxisData> y;  ///< Y-axis data for each individual curve
} MultiLineSeries;

/**
 * @brief Bounded (time, value) ring that decimates on ingest to span a fixed window.
 */
struct TimeRing {
  AxisData time;
  AxisData value;
  double interval;
  double nextEmit;
  double accMin;
  double accMax;
  double accMinTime;
  double accMaxTime;
  int cellSlots;

  /**
   * @brief Constructs the ring with `capacity` slots covering `windowSec` seconds. Each
   *        grid cell may retain two slots (min + max), so the interval reserves both to
   *        keep a saturated source spanning the full window.
   */
  explicit TimeRing(int capacity = 1, double windowSec = 1.0)
    : time(static_cast<std::size_t>(capacity < 1 ? 1 : capacity))
    , value(static_cast<std::size_t>(capacity < 1 ? 1 : capacity))
    , interval(2.0 * windowSec / std::max(1, capacity))
    , nextEmit(0.0)
    , accMin(0.0)
    , accMax(0.0)
    , accMinTime(0.0)
    , accMaxTime(0.0)
    , cellSlots(0)
  {}

  /**
   * @brief Re-grids the decimation cell in place, keeping retained samples: older data stays at
   *        the resolution it was captured with and the next append opens a fresh cell. Retained
   *        span is capacity/2 cells, so a finer grid trades history for detail, but only while a
   *        source saturates every cell.
   */
  void retune(double cellSeconds)
  {
    if (!(cellSeconds > 0.0) || interval == cellSeconds)
      return;

    interval  = cellSeconds;
    nextEmit  = 0.0;
    cellSlots = 0;
  }

  /**
   * @brief Re-sizes the ring to @p capacity slots and re-grids it to @p windowSec, keeping the
   *        newest samples. This is what lets a ring whose source turned out faster than its
   *        initial sizing assumed still span its axis: a ring bounded below the source's rate
   *        runs out of history in seconds and the trace stops short of the left edge.
   */
  void resizeCapacity(int capacity, double windowSec)
  {
    if (capacity < 1 || !(windowSec > 0.0))
      return;

    if (static_cast<std::size_t>(capacity) == time.capacity())
      return;

    time.resize(static_cast<std::size_t>(capacity));
    value.resize(static_cast<std::size_t>(capacity));

    interval  = 2.0 * windowSec / static_cast<double>(capacity);
    nextEmit  = 0.0;
    cellSlots = 0;
  }

  /**
   * @brief Clears retained samples and the decimation cell state.
   */
  void clear()
  {
    time.clear();
    value.clear();
    cellSlots = 0;
    nextEmit  = 0.0;
  }

  /**
   * @brief True when a (sanitised, monotonic) time @p t opens a fresh decimation cell: the ring
   *        is empty or the open cell's grid boundary has been crossed. The one predicate both
   *        appendDecimated() and the envelope pyramid built on top of it evaluate.
   */
  [[nodiscard]] bool opensCell(double t) const noexcept
  {
    return time.size() == 0 || t >= nextEmit;
  }

  /**
   * @brief Appends one (time, value) as a per-cell min/max pair on an absolute time grid; the
   *        open cell updates in place. Sub-cell backward jitter clamps forward, but a jump back
   *        over a whole cell is a restarted clock and drops the retained span: clamping it
   *        wedges the ring shut until wall time catches up.
   */
  void appendDecimated(double t, double v)
  {
    SS_ASSERT(interval > 0.0, return);
    SS_ASSERT(time.capacity() == value.capacity(), return);

    if (time.raw() == nullptr || value.raw() == nullptr) [[unlikely]]
      return;

    if (!std::isfinite(t) || !std::isfinite(v)) [[unlikely]]
      return;

    const std::size_t held = time.size();
    if (held > 0 && t < time[held - 1]) [[unlikely]] {
      if (t < time[held - 1] - interval)
        clear();
      else
        t = time[held - 1];
    }

    if (cellSlots > static_cast<int>(time.size())) [[unlikely]]
      cellSlots = static_cast<int>(time.size());

    if (opensCell(t)) {
      nextEmit   = (std::floor(t / interval) + 1.0) * interval;
      accMin     = v;
      accMax     = v;
      accMinTime = t;
      accMaxTime = t;
      cellSlots  = 1;
      time.push(t);
      value.push(v);
      return;
    }

    bool changed = false;
    if (v < accMin) {
      accMin     = v;
      accMinTime = t;
      changed    = true;
    }

    if (v > accMax) {
      accMax     = v;
      accMaxTime = t;
      changed    = true;
    }

    if (!changed)
      return;

    const bool minFirst = accMinTime <= accMaxTime;
    const double t0     = minFirst ? accMinTime : accMaxTime;
    const double v0     = minFirst ? accMin : accMax;
    const double t1     = minFirst ? accMaxTime : accMinTime;
    const double v1     = minFirst ? accMax : accMin;

    if (cellSlots == 1 && (t1 > t0 || v1 != v0) && time.capacity() > 1) {
      time.push(t1);
      value.push(v1);
      cellSlots = 2;
    }

    const std::size_t n = time.size();
    if (n == 0) [[unlikely]]
      return;

    if (cellSlots == 2 && n >= 2) {
      time[n - 2]  = t0;
      value[n - 2] = v0;
    }

    time[n - 1]  = t1;
    value[n - 1] = v1;
  }
};

/**
 * @brief One coarse envelope cell: the two extremes of the level-0 span it covers, stored in
 *        time order so a level reads as a monotonic (time, value) sequence of 2 * cells points.
 */
struct EnvelopeCell {
  double t0;
  double v0;
  double t1;
  double v1;
};

/**
 * @brief One coarse level of an EnvelopeRing: a bounded ring of cells each spanning
 *        16^level level-0 grid cells. The back cell is the open one; openIndex is its level-0
 *        cell index shifted right by @c shift, so nesting is exact integer arithmetic.
 */
struct EnvelopeLevel {
  FixedQueue<EnvelopeCell> cells;
  std::int64_t openIndex;
  int shift;
  bool openValid;

  /**
   * @brief Sizes the level to @p capacity cells covering 2^shift level-0 cells each.
   */
  explicit EnvelopeLevel(std::size_t capacity = 1, int shiftBits = 4)
    : cells(capacity < 1 ? 1 : capacity), openIndex(0), shift(shiftBits), openValid(false)
  {}
};

/**
 * @brief Bounded envelope pyramid over a TimeRing (spec 0057): level 0 is the full-detail ring,
 *        level k holds time-ordered min/max pairs per 16^k level-0 cells in its own bounded ring
 *        (16/15 of level 0's bytes in total). A completed level-0 cell folds into every coarser
 *        open cell, never a rescan; no allocation after construction. GUI-thread single writer.
 */
struct EnvelopeRing {
  static constexpr int kLevelShift            = 4;
  static constexpr int kMaxCoarseLevels       = 9;
  static constexpr std::size_t kMinLevelCells = 3;

  TimeRing level0;
  std::vector<EnvelopeLevel> levels;
  std::int64_t openCell;
  bool openCellValid;

  /**
   * @brief Constructs level 0 with `capacity` slots over `windowSec` seconds and derives the
   *        coarse levels from that capacity.
   */
  explicit EnvelopeRing(int capacity = 1, double windowSec = 1.0)
    : level0(capacity, windowSec), levels(), openCell(0), openCellValid(false)
  {
    buildLevels();
  }

  /**
   * @brief Level-0 grid index of time @p t; the same floor(t / interval) appendDecimated uses
   *        to place nextEmit, so both agree on cell boundaries by construction.
   */
  [[nodiscard]] std::int64_t cellIndex(double t) const noexcept
  {
    return static_cast<std::int64_t>(std::floor(t / level0.interval));
  }

  /**
   * @brief Wall-time span of one cell at @p level (0 = the level-0 interval).
   */
  [[nodiscard]] double levelSpanSec(int level) const noexcept
  {
    return std::ldexp(level0.interval, kLevelShift * (level < 0 ? 0 : level));
  }

  /**
   * @brief Number of coarse levels currently built (level 0 excluded).
   */
  [[nodiscard]] int coarseLevelCount() const noexcept { return static_cast<int>(levels.size()); }

  /**
   * @brief Coarsest level whose cell span fits under one pixel of @p spanSec / @p pixels and
   *        whose oldest cell still reaches back to @p oldestSec (absolute time). Level 0 is the
   *        ground truth and the answer for narrow windows or empty coarse levels.
   */
  [[nodiscard]] int selectLevel(double spanSec, int pixels, double oldestSec) const
  {
    if (!(spanSec > 0.0) || pixels <= 0 || !(level0.interval > 0.0) || !std::isfinite(oldestSec))
      return 0;

    const double pixelSec       = spanSec / static_cast<double>(pixels);
    const std::int64_t oldestId = cellIndex(oldestSec);

    int chosen = 0;
    for (std::size_t k = 0; k < levels.size(); ++k) {
      const auto& level = levels[k];
      const int number  = static_cast<int>(k) + 1;
      if (levelSpanSec(number) > pixelSec || level.cells.size() < 2)
        break;

      if ((cellIndex(level.cells.front().t0) >> level.shift) > (oldestId >> level.shift))
        break;

      chosen = number;
    }

    return chosen;
  }

  /**
   * @brief Drops every retained sample and every coarse cell.
   */
  void clear()
  {
    level0.clear();
    for (auto& level : levels) {
      level.cells.clear();
      level.openValid = false;
    }

    openCellValid = false;
  }

  /**
   * @brief Re-sizes level 0 (keeping its newest samples, see TimeRing::resizeCapacity) and
   *        rebuilds the coarse levels from what it retained. Rare by construction: only the
   *        display-tick growth path and a time-range change reach it.
   */
  void resizeCapacity(int capacity, double windowSec)
  {
    if (capacity < 1 || !(windowSec > 0.0))
      return;

    if (static_cast<std::size_t>(capacity) == level0.time.capacity())
      return;

    level0.resizeCapacity(capacity, windowSec);
    buildLevels();
    rebuildLevelsFromLevel0();
  }

  /**
   * @brief Appends one (time, value): rejects non-finite input, folds the level-0 cell this
   *        sample closes into the coarse levels, then decimates into level 0. Costs one branch
   *        over TimeRing::appendDecimated on the common (same-cell) path. A restarted producer
   *        clock drops the pyramid with level 0: those cells describe the abandoned timeline.
   */
  void appendDecimated(double t, double v)
  {
    SS_ASSERT(level0.time.capacity() == level0.value.capacity(), return);

    if (!std::isfinite(t) || !std::isfinite(v)) [[unlikely]]
      return;

    if (!(level0.interval > 0.0) || level0.time.raw() == nullptr) [[unlikely]]
      return;

    const std::size_t n = level0.time.size();
    if (n > 0 && t < level0.time[n - 1]) [[unlikely]] {
      if (t < level0.time[n - 1] - level0.interval)
        clear();
      else
        t = level0.time[n - 1];
    }

    if (level0.opensCell(t)) [[unlikely]] {
      if (openCellValid)
        foldOpenCell();

      openCell      = cellIndex(t);
      openCellValid = true;
    }

    level0.appendDecimated(t, v);
  }

private:
  /**
   * @brief Sizes the coarse levels from level 0's capacity: cells0 = capacity / 2 (a saturated
   *        grid keeps two slots per cell), level k holds ceil(cells0 / 16^k) + 1 cells while that
   *        is at least kMinLevelCells, at most kMaxCoarseLevels levels.
   */
  void buildLevels()
  {
    levels.clear();
    const std::size_t cells0 = level0.time.capacity() / 2;
    for (int k = 1; k <= kMaxCoarseLevels; ++k) {
      const std::size_t denom = static_cast<std::size_t>(1) << (kLevelShift * k);
      const std::size_t count = (cells0 + denom - 1) / denom + 1;
      if (count < kMinLevelCells)
        break;

      levels.emplace_back(count, kLevelShift * k);
    }
  }

  /**
   * @brief Folds level 0's closed cell (its acc* extremes) into every coarse level's open cell.
   */
  void foldOpenCell()
  {
    SS_ASSERT(openCellValid, return);

    const bool minFirst = level0.accMinTime <= level0.accMaxTime;
    EnvelopeCell cell;
    cell.t0 = minFirst ? level0.accMinTime : level0.accMaxTime;
    cell.v0 = minFirst ? level0.accMin : level0.accMax;
    cell.t1 = minFirst ? level0.accMaxTime : level0.accMinTime;
    cell.v1 = minFirst ? level0.accMax : level0.accMin;
    foldCell(openCell, cell);
  }

  /**
   * @brief Merges one level-0 cell (index @p index0, extremes @p cell) into each coarse level:
   *        the open cell when the shifted index matches, a fresh pushed cell otherwise.
   */
  void foldCell(std::int64_t index0, const EnvelopeCell& cell)
  {
    for (auto& level : levels) {
      const std::int64_t index = index0 >> level.shift;
      if (level.openValid && index == level.openIndex && level.cells.size() > 0) {
        mergeCell(level.cells[level.cells.size() - 1], cell);
        continue;
      }

      level.cells.push(cell);
      level.openIndex = index;
      level.openValid = true;
    }
  }

  /**
   * @brief Widens @p target's extremes with @p cell's, keeping the pair time-ordered.
   */
  static void mergeCell(EnvelopeCell& target, const EnvelopeCell& cell)
  {
    const bool targetMinFirst = target.v0 <= target.v1;
    double minT               = targetMinFirst ? target.t0 : target.t1;
    double minV               = targetMinFirst ? target.v0 : target.v1;
    double maxT               = targetMinFirst ? target.t1 : target.t0;
    double maxV               = targetMinFirst ? target.v1 : target.v0;

    const bool cellMinFirst = cell.v0 <= cell.v1;
    const double cMinT      = cellMinFirst ? cell.t0 : cell.t1;
    const double cMinV      = cellMinFirst ? cell.v0 : cell.v1;
    const double cMaxT      = cellMinFirst ? cell.t1 : cell.t0;
    const double cMaxV      = cellMinFirst ? cell.v1 : cell.v0;

    if (cMinV < minV) {
      minV = cMinV;
      minT = cMinT;
    }

    if (cMaxV > maxV) {
      maxV = cMaxV;
      maxT = cMaxT;
    }

    const bool minFirst = minT <= maxT;
    target.t0           = minFirst ? minT : maxT;
    target.v0           = minFirst ? minV : maxV;
    target.t1           = minFirst ? maxT : minT;
    target.v1           = minFirst ? maxV : minV;
  }

  /**
   * @brief Refills the coarse levels from level 0's retained slots, each folded as a one-sample
   *        cell. Level 0's own open-cell state was reset by the resize, so the next append opens
   *        a fresh cell instead of re-folding stale accumulators.
   */
  void rebuildLevelsFromLevel0()
  {
    openCellValid       = false;
    const std::size_t n = std::min(level0.time.size(), level0.value.size());
    for (std::size_t k = 0; k < n; ++k) {
      const double t = level0.time[k];
      const double v = level0.value[k];
      if (!std::isfinite(t) || !std::isfinite(v))
        continue;

      const EnvelopeCell cell{t, v, t, v};
      foldCell(cellIndex(t), cell);
    }
  }
};

/**
 * @brief One retained, completed sweep (spec 0061): the trigger instant on the plot clock, the
 *        window it spans and a deep copy of every curve's ring at completion time.
 */
struct SweepSegment {
  double triggerSec;
  double windowSec;
  std::vector<TimeRing> curves;

  SweepSegment() : triggerSec(0.0), windowSec(0.0), curves() {}
};

/**
 * @brief Oscilloscope-style sweep/trigger engine with a front/back ring per curve, plus a
 *        bounded store of the last N completed sweeps (spec 0061).
 */
struct SweepEngine {
  static constexpr int kAuto    = 0;
  static constexpr int kNormal  = 1;
  static constexpr int kSingle  = 2;
  static constexpr int kRising  = 0;
  static constexpr int kFalling = 1;

  // Windows wider than this draw the live partial trace instead of freezing
  static constexpr double kLiveWindowSec = 0.1;

  // Segment retention bounds: requested N is clamped so a plot never exceeds kMaxSegmentBytes
  static constexpr int kMaxSegments              = 64;
  static constexpr std::size_t kMaxSegmentBytes  = 32u * 1024u * 1024u;
  static constexpr std::size_t kSegmentSlotBytes = 2 * sizeof(double);

  std::vector<TimeRing> front;
  std::vector<TimeRing> back;
  std::vector<SweepSegment> segments;
  std::size_t segmentHead;
  std::size_t segmentFill;
  int segmentRetention;

  double windowSec;
  double timebaseSec;
  double level;
  double holdoffSec;
  int edge;
  int mode;
  int triggerCurve;
  bool enabled;

  double t0;
  double prevValue;
  double prevTime;
  double lastTriggerSec;
  double lastSweepSec;
  bool prevValid;
  bool sweeping;
  bool armed;
  bool hasFront;

  /**
   * @brief Constructs an idle, disabled sweep engine with no curves.
   */
  SweepEngine()
    : segmentHead(0)
    , segmentFill(0)
    , segmentRetention(0)
    , windowSec(1.0)
    , timebaseSec(0.0)
    , level(0.0)
    , holdoffSec(0.0)
    , edge(kRising)
    , mode(kAuto)
    , triggerCurve(0)
    , enabled(false)
    , t0(0.0)
    , prevValue(0.0)
    , prevTime(0.0)
    , lastTriggerSec(-1.0)
    , lastSweepSec(0.0)
    , prevValid(false)
    , sweeping(false)
    , armed(true)
    , hasFront(false)
  {}

  /**
   * @brief Allocates `curveCount` front/back rings, each constructed individually: FixedQueue
   *        copies share their backing array, so a copy-fill would alias every curve.
   */
  void configure(int curveCount, int capacity, double window)
  {
    windowSec                = window > 0 ? window : 1.0;
    const std::size_t curves = static_cast<std::size_t>(curveCount < 0 ? 0 : curveCount);

    front.clear();
    back.clear();
    front.reserve(curves);
    back.reserve(curves);
    for (std::size_t i = 0; i < curves; ++i) {
      front.emplace_back(capacity, windowSec);
      back.emplace_back(capacity, windowSec);
    }

    retuneRings();
    resetState();
    setSegmentRetention(segmentRetention);
  }

  /**
   * @brief Requests retention of the last @p count completed sweeps (0 = off). The effective
   *        count is clamped so `count * curves * capacity * 16 B` stays under kMaxSegmentBytes;
   *        segments are pre-sized here so completeSweep() never allocates.
   */
  void setSegmentRetention(int count)
  {
    segmentRetention = std::clamp(count, 0, kMaxSegments);

    const std::size_t curves   = front.size();
    const std::size_t capacity = front.empty() ? 0 : front.front().time.capacity();
    const std::size_t perSeg   = std::max<std::size_t>(1, curves * capacity * kSegmentSlotBytes);
    const std::size_t maxByMem = std::max<std::size_t>(1, kMaxSegmentBytes / perSeg);
    const std::size_t target =
      std::min<std::size_t>(static_cast<std::size_t>(segmentRetention), maxByMem);

    const bool sameShape =
      !segments.empty() && segments.front().curves.size() == curves
      && (curves == 0 || segments.front().curves.front().time.capacity() == capacity);
    if (segments.size() == target && (segments.empty() || sameShape))
      return;

    segments.clear();
    segments.reserve(target);
    for (std::size_t i = 0; i < target; ++i) {
      SweepSegment segment;
      segment.curves.reserve(curves);
      for (std::size_t c = 0; c < curves; ++c)
        segment.curves.emplace_back(static_cast<int>(capacity), windowSec);

      segments.push_back(std::move(segment));
    }

    segmentHead = 0;
    segmentFill = 0;
  }

  /**
   * @brief Effective retained-segment capacity after the byte-budget clamp.
   */
  [[nodiscard]] int segmentCapacity() const noexcept { return static_cast<int>(segments.size()); }

  /**
   * @brief Number of completed sweeps currently retained.
   */
  [[nodiscard]] int segmentCount() const noexcept { return static_cast<int>(segmentFill); }

  /**
   * @brief Retained segment @p index, 0 = newest; nullptr when out of range.
   */
  [[nodiscard]] const SweepSegment* segment(int index) const noexcept
  {
    if (index < 0 || static_cast<std::size_t>(index) >= segmentFill || segments.empty())
      return nullptr;

    const std::size_t cap  = segments.size();
    const std::size_t slot = (segmentHead + cap - 1 - static_cast<std::size_t>(index)) % cap;
    return &segments[slot];
  }

  /**
   * @brief Forgets every retained sweep but keeps the storage.
   */
  void clearSegments() noexcept
  {
    segmentHead = 0;
    segmentFill = 0;
  }

  /**
   * @brief Takes over @p other's retained segments when the shapes match (curve count and ring
   *        capacity), so a Time-Range or layout rebuild keeps them like the history rings.
   */
  void takeSegmentsFrom(const SweepEngine& other)
  {
    setSegmentRetention(other.segmentRetention);
    if (other.segments.empty() || other.segments.size() != segments.size())
      return;

    const auto& mine   = segments.front().curves;
    const auto& theirs = other.segments.front().curves;
    if (mine.size() != theirs.size())
      return;

    if (!mine.empty() && mine.front().time.capacity() != theirs.front().time.capacity())
      return;

    segments    = other.segments;
    segmentHead = other.segmentHead;
    segmentFill = other.segmentFill;
  }

  /**
   * @brief Re-grids every ring to the window a sweep actually spans, so a short timebase
   *        decimates at the timebase's resolution instead of the full-range grid the rings were
   *        built with -- which is what used to flatten a millisecond capture of a dense source.
   */
  void retuneRings()
  {
    const auto cell = [](const TimeRing& ring, double window) {
      const auto capacity = ring.time.capacity();
      return 2.0 * window / static_cast<double>(capacity > 0 ? capacity : 1);
    };

    for (auto& r : front)
      r.retune(cell(r, activeWindow()));

    for (auto& r : back)
      r.retune(cell(r, activeWindow()));
  }

  /**
   * @brief Width of one captured sweep: the timebase override, else the full window.
   */
  [[nodiscard]] double activeWindow() const noexcept
  {
    return (timebaseSec > 0 && timebaseSec < windowSec) ? timebaseSec : windowSec;
  }

  /**
   * @brief Updates trigger parameters and re-arms a single-shot capture.
   */
  void setTrigger(double lvl, int edgeMode, int sweepMode, double holdoff, int curve)
  {
    level        = lvl;
    edge         = (edgeMode == kFalling) ? kFalling : kRising;
    mode         = (sweepMode == kNormal) ? kNormal : (sweepMode == kSingle ? kSingle : kAuto);
    holdoffSec   = holdoff > 0 ? holdoff : 0.0;
    triggerCurve = curve < 0 ? 0 : curve;
    armed        = true;
  }

  /**
   * @brief Sets the per-sweep timebase in seconds; 0 or >= window means full window. The rings
   *        follow, so the capture is decimated at the resolution the timebase implies.
   */
  void setTimebase(double seconds)
  {
    timebaseSec = seconds > 0 ? seconds : 0.0;
    retuneRings();
  }

  /**
   * @brief Re-arms a single-shot capture without touching retained data.
   */
  void arm() { armed = true; }

  /**
   * @brief Clears the decimation state and disarms an in-progress sweep.
   */
  void resetState()
  {
    for (auto& r : front)
      r.clear();

    for (auto& r : back)
      r.clear();

    t0             = 0.0;
    prevValue      = 0.0;
    prevTime       = 0.0;
    lastTriggerSec = -1.0;
    lastSweepSec   = 0.0;
    prevValid      = false;
    sweeping       = false;
    hasFront       = false;
    armed          = true;
    clearSegments();
  }

  /**
   * @brief Returns the ring to render. Short windows show the frozen, phase-locked
   *        completed sweep; long windows show the live partial trace so they grow
   *        in real time. Falls back to the live ring before the first completion.
   */
  [[nodiscard]] const TimeRing& display(std::size_t curve) const
  {
    static const TimeRing kEmpty{};
    if (curve >= front.size())
      return kEmpty;

    const bool live = !hasFront || activeWindow() > kLiveWindowSec;
    if (live && sweeping && curve < back.size())
      return back[curve];

    return front[curve];
  }

  /**
   * @brief Steps the trigger state with one trigger-source sample; returns the
   *        sweep time to append at, or a negative sentinel to skip this sample.
   */
  double advance(double now, double trigValue)
  {
    double sweepTime = -1.0;

    if (sweeping) {
      const double st = now - t0;
      if (st > activeWindow())
        completeSweep();
      else {
        sweepTime = st < 0 ? 0.0 : st;
        prevValue = trigValue;
        prevTime  = now;
        prevValid = true;
        return sweepTime;
      }
    }

    if (!sweeping && shouldStart(now, trigValue)) {
      t0           = triggerOrigin(now, trigValue);
      lastSweepSec = now;
      if (edgeDetected(trigValue))
        lastTriggerSec = now;

      for (auto& r : back)
        r.clear();

      sweeping  = true;
      sweepTime = (now - t0) < 0 ? 0.0 : (now - t0);
    }

    prevValue = trigValue;
    prevTime  = now;
    prevValid = true;
    return sweepTime;
  }

private:
  /**
   * @brief Returns true when the trigger source crossed the level in the armed direction.
   */
  [[nodiscard]] bool edgeDetected(double value) const
  {
    if (!prevValid)
      return false;

    if (edge == kFalling)
      return prevValue > level && value <= level;

    return prevValue < level && value >= level;
  }

  /**
   * @brief Decides whether an idle engine should begin a new sweep this sample.
   */
  [[nodiscard]] bool shouldStart(double now, double value)
  {
    const bool edgeOk =
      edgeDetected(value) && (lastTriggerSec < 0 || (now - lastTriggerSec) >= holdoffSec);

    if (mode == kSingle)
      return armed && edgeOk;

    if (mode == kNormal)
      return edgeOk;

    return edgeOk || (now - lastSweepSec) >= activeWindow();
  }

  /**
   * @brief Computes the sweep origin, interpolating the exact level crossing on an edge.
   */
  [[nodiscard]] double triggerOrigin(double now, double value) const
  {
    if (!edgeDetected(value))
      return now;

    const double denom = value - prevValue;
    double frac        = (std::abs(denom) > 1e-12) ? (level - prevValue) / denom : 0.0;
    frac               = std::clamp(frac, 0.0, 1.0);
    return prevTime + frac * (now - prevTime);
  }

  /**
   * @brief Publishes the filled sweep to `front` and ends the acquisition. `lastSweepSec` is
   *        left at the sweep's start time so the Auto free-run timer re-triggers immediately on
   *        the next sample (continuous refresh) instead of stalling for another full window.
   */
  void completeSweep()
  {
    const std::size_t n = std::min(front.size(), back.size());
    for (std::size_t i = 0; i < n; ++i)
      std::swap(front[i], back[i]);

    hasFront = true;
    sweeping = false;
    if (mode == kSingle)
      armed = false;

    retainFront();
  }

  /**
   * @brief Deep-copies the just-completed front rings into the next segment slot (FixedQueue
   *        copies alias their storage, so the slots are refilled element by element). One
   *        bounded walk per completed sweep, no allocation.
   */
  void retainFront()
  {
    if (segments.empty())
      return;

    SS_ASSERT(segmentHead < segments.size(), segmentHead = 0);

    SweepSegment& slot = segments[segmentHead];
    slot.triggerSec    = t0;
    slot.windowSec     = activeWindow();

    const std::size_t curves = std::min(slot.curves.size(), front.size());
    for (std::size_t c = 0; c < curves; ++c)
      copyRing(front[c], slot.curves[c]);

    segmentHead = (segmentHead + 1) % segments.size();
    segmentFill = std::min(segments.size(), segmentFill + 1);
  }

  /**
   * @brief Element-wise copy of a ring's retained samples and grid state into a same-capacity
   *        target that owns its own storage.
   */
  static void copyRing(const TimeRing& source, TimeRing& target)
  {
    target.clear();
    target.interval = source.interval;

    const std::size_t n = std::min(source.time.size(), source.value.size());
    for (std::size_t k = 0; k < n && k < target.time.capacity(); ++k) {
      target.time.push(source.time[k]);
      target.value.push(source.value[k]);
    }

    target.nextEmit  = source.nextEmit;
    target.cellSlots = 0;
  }
};

#ifdef BUILD_COMMERCIAL
/**
 * @typedef PlotData3D
 * @brief Represents a list of 3D points.
 */
typedef std::vector<QVector3D> LineSeries3D;
#endif

/**
 * @brief Represents a time-ordered sequence of GPS position data.
 */
typedef struct {
  FixedQueue<ssfp_t> latitudes;   ///< Latitude values (degrees)
  FixedQueue<ssfp_t> longitudes;  ///< Longitude values (degrees)
  FixedQueue<ssfp_t> altitudes;   ///< Altitude values (meters)
} GpsSeries;

}  // namespace DSP

//--------------------------------------------------------------------------------------------------
// Column-decimation kernels, split into a sibling header and included where they used to sit
//--------------------------------------------------------------------------------------------------

#include "DSPDownsample.h"

namespace DSP {

/**
 * @brief Check whether a numeric value is effectively zero (close to 0.0).
 */
template<Concepts::Numeric T>
[[nodiscard]] inline bool isZero(T value, T absEps = T(1e-12)) noexcept
{
  return std::abs(value) <= absEps;
}

/**
 * @brief Compare two numeric values for approximate equality using absolute + relative tolerances.
 */
template<Concepts::Numeric T>
[[nodiscard]] inline bool almostEqual(T a, T b, T relEps = T(1e-12), T absEps = T(1e-12)) noexcept
{
  if constexpr (std::floating_point<T>) {
    if (!std::isfinite(a) || !std::isfinite(b))
      return a == b;
  }

  const T diff = std::abs(a - b);

  if (diff <= absEps)
    return true;

  const T scale = std::max(std::abs(a), std::abs(b));
  return diff <= relEps * scale;
}

/**
 * @brief Explicit "not equal" companion to almostEqual().
 */
template<Concepts::Numeric T>
[[nodiscard]] inline bool notEqual(T a, T b, T relEps = T(1e-12), T absEps = T(1e-12)) noexcept
{
  return !almostEqual(a, b, relEps, absEps);
}

}  // namespace DSP
