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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
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
    , m_data(std::shared_ptr<T[]>(new T[m_storageCapacity]))
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
    Q_ASSERT(index < m_size);
    return m_data[wrappedIndex(index < m_size ? index : 0)];
  }

  /**
   * @brief Provides mutable access to an element at a given index.
   */
  [[nodiscard]] T& operator[](std::size_t index)
  {
    Q_ASSERT(index < m_size);
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
    Q_ASSERT(index < m_capacity);
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
    std::shared_ptr<T[]> newData(new T[newStorage]);
    std::size_t elementsToCopy = std::min(m_size, newCapacity);
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
  double lastEmit;
  double accExtreme;
  double accTime;
  bool accValid;

  /**
   * @brief Constructs the ring with `capacity` slots covering `windowSec` seconds.
   */
  explicit TimeRing(int capacity = 1, double windowSec = 1.0)
    : time(static_cast<std::size_t>(capacity < 1 ? 1 : capacity))
    , value(static_cast<std::size_t>(capacity < 1 ? 1 : capacity))
    , interval(windowSec / std::max(1, capacity))
    , lastEmit(0.0)
    , accExtreme(0.0)
    , accTime(0.0)
    , accValid(false)
  {}

  /**
   * @brief Clears retained samples and the decimation accumulator.
   */
  void clear()
  {
    time.clear();
    value.clear();
    accValid = false;
    lastEmit = 0.0;
  }

  /**
   * @brief Appends one (time, value), decimating to one peak-preserving slot per interval.
   */
  void appendDecimated(double t, double v)
  {
    // Track the larger-magnitude sample so transients survive the decimation
    if (!accValid || std::abs(v) > std::abs(accExtreme)) {
      accExtreme = v;
      accTime    = t;
    }

    accValid = true;

    // Emit one representative point once an interval has elapsed
    if (time.size() == 0 || (t - lastEmit) >= interval) {
      time.push(accTime);
      value.push(accExtreme);
      lastEmit = t;
      accValid = false;
    }
  }
};

/**
 * @brief Oscilloscope-style sweep/trigger engine with a front/back ring per curve.
 */
struct SweepEngine {
  static constexpr int kAuto    = 0;
  static constexpr int kNormal  = 1;
  static constexpr int kSingle  = 2;
  static constexpr int kRising  = 0;
  static constexpr int kFalling = 1;

  std::vector<TimeRing> front;
  std::vector<TimeRing> back;

  double windowSec;
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
    : windowSec(1.0)
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
   * @brief Allocates `curveCount` front/back rings of `capacity` over `window` seconds.
   */
  void configure(int curveCount, int capacity, double window)
  {
    windowSec = window > 0 ? window : 1.0;
    front.assign(static_cast<std::size_t>(curveCount < 0 ? 0 : curveCount),
                 TimeRing(capacity, windowSec));
    back.assign(static_cast<std::size_t>(curveCount < 0 ? 0 : curveCount),
                TimeRing(capacity, windowSec));
    resetState();
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
  }

  /**
   * @brief Returns the ring to render: the live trace while acquiring, else the
   *        last completed sweep. Lets the trace grow in real time like a scope.
   */
  [[nodiscard]] const TimeRing& display(std::size_t curve) const
  {
    static const TimeRing kEmpty{};
    if (curve >= front.size())
      return kEmpty;

    if (sweeping && curve < back.size())
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

    // Already filling: append until the window elapses, then publish the sweep
    if (sweeping) {
      const double st = now - t0;
      if (st > windowSec)
        completeSweep(now);
      else
        sweepTime = st < 0 ? 0.0 : st;
    }

    // Idle: decide whether a trigger (or auto free-run) starts a new sweep
    else if (shouldStart(now, trigValue)) {
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

    // Auto: trigger when possible, otherwise free-run after a quiet window
    return edgeOk || (now - lastSweepSec) >= windowSec;
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
   * @brief Publishes the filled sweep to `front` and ends the acquisition.
   */
  void completeSweep(double now)
  {
    const std::size_t n = std::min(front.size(), back.size());
    for (std::size_t i = 0; i < n; ++i)
      std::swap(front[i], back[i]);

    hasFront     = true;
    sweeping     = false;
    lastSweepSec = now;
    if (mode == kSingle)
      armed = false;
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

//--------------------------------------------------------------------------------------------------
// Downsampling workspace
//--------------------------------------------------------------------------------------------------

/**
 * @brief Scratch buffers for column-wise downsampling.
 */
struct DownsampleWorkspace {
  // Number of valid samples that landed in each column
  std::vector<unsigned int> cnt;

  // Column-wise vertical extrema
  std::vector<ssfp_t> minY;
  std::vector<ssfp_t> maxY;

  // Logical indices associated with extrema and endpoints per column
  std::vector<std::size_t> minI;
  std::vector<std::size_t> maxI;
  std::vector<std::size_t> firstI;
  std::vector<std::size_t> lastI;

  /**
   * @brief Prepare the workspace for a render pass with C columns.
   */
  void reset(std::size_t C)
  {
    if (cnt.size() != C) {
      cnt.resize(C);
      minY.resize(C);
      maxY.resize(C);
      minI.resize(C);
      maxI.resize(C);
      lastI.resize(C);
      firstI.resize(C);
    }

    // Clear per-column tallies and extrema
    std::fill(cnt.begin(), cnt.end(), 0u);
    std::fill(minY.begin(), minY.end(), std::numeric_limits<ssfp_t>::infinity());
    std::fill(maxY.begin(), maxY.end(), -std::numeric_limits<ssfp_t>::infinity());
  }
};

//--------------------------------------------------------------------------------------------------
// Ring helper
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get two contiguous spans from a ring-buffered queue.
 */
template<typename T>
  requires std::copy_constructible<T> && std::is_copy_assignable_v<T>
inline void spanFromFixedQueue(
  const FixedQueue<T>& q, const T*& p0, std::size_t& n0, const T*& p1, std::size_t& n1)
{
  const T* base = q.raw();

  const std::size_t n    = q.size();
  const std::size_t scap = q.storageCapacity();
  const std::size_t i0   = q.frontIndex();
  const std::size_t tail = std::min<std::size_t>(n, scap - i0);

  p0 = base + i0;
  n0 = tail;

  p1 = base;
  n1 = n - tail;
}

//--------------------------------------------------------------------------------------------------
// Downsample helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Bounds and finite-range result returned by dsScanFiniteRange().
 */
struct DownsampleBounds {
  ssfp_t xmin;
  ssfp_t xmax;
  ssfp_t ymin;
  ssfp_t ymax;
  std::size_t firstFinite;
  std::size_t lastFinite;
};

/**
 * @brief Scan ring-buffer spans to compute finite (X,Y) bounds and endpoints.
 */
template<typename XAt, typename YAt>
inline DownsampleBounds dsScanFiniteRange(std::size_t n, XAt xAt, YAt yAt)
{
  DownsampleBounds b;
  b.firstFinite = n;
  b.lastFinite  = n;
  b.xmin        = std::numeric_limits<ssfp_t>::infinity();
  b.ymin        = std::numeric_limits<ssfp_t>::infinity();
  b.xmax        = -std::numeric_limits<ssfp_t>::infinity();
  b.ymax        = -std::numeric_limits<ssfp_t>::infinity();

  for (std::size_t i = 0; i < n; ++i) {
    const ssfp_t xv = xAt(i);
    const ssfp_t yv = yAt(i);
    if (!std::isfinite(xv) || !std::isfinite(yv))
      continue;

    if (xv < b.xmin)
      b.xmin = xv;

    if (xv > b.xmax)
      b.xmax = xv;

    if (yv < b.ymin)
      b.ymin = yv;

    if (yv > b.ymax)
      b.ymax = yv;

    if (b.firstFinite == n)
      b.firstFinite = i;

    b.lastFinite = i;
  }

  return b;
}

/**
 * @brief Emit a stepped polyline for the degenerate xmin==xmax case.
 */
template<typename XAt, typename YAt>
inline void dsEmitSteppedFallback(std::size_t n,
                                  int w,
                                  std::size_t firstFinite,
                                  std::size_t lastFinite,
                                  XAt xAt,
                                  YAt yAt,
                                  QList<QPointF>& out)
{
  std::size_t step = n / (std::max(1, w));
  if (step < 1)
    step = 1;

  for (std::size_t i = firstFinite; i <= lastFinite; i += step)
    out.append(QPointF(xAt(i), yAt(i)));

  if (out.isEmpty() || out.back().x() != xAt(lastFinite))
    out.append(QPointF(xAt(lastFinite), yAt(lastFinite)));
}

/**
 * @brief Accumulate per-column min/max/first/last indices into the workspace.
 */
template<typename XAt, typename YAt>
inline void dsAccumulateBuckets(
  std::size_t n, int w, ssfp_t xmin, ssfp_t scaleX, XAt xAt, YAt yAt, DownsampleWorkspace* ws)
{
  auto getColFromX = [&](ssfp_t x) -> std::size_t {
    auto c = static_cast<long>((x - xmin) * scaleX);
    if (c < 0)
      c = 0;

    else if (c >= w)
      c = w - 1;

    return std::size_t(c);
  };

  for (std::size_t i = 0; i < n; ++i) {
    const ssfp_t xv = xAt(i);
    const ssfp_t yv = yAt(i);
    if (!std::isfinite(xv) || !std::isfinite(yv))
      continue;

    const std::size_t c = getColFromX(xv);

    if (ws->cnt[c] == 0) {
      ws->firstI[c] = ws->lastI[c] = i;
      ws->minI[c] = ws->maxI[c] = i;
      ws->minY[c] = ws->maxY[c] = yv;
      ws->cnt[c]                = 1;
      continue;
    }

    if (yv < ws->minY[c]) {
      ws->minY[c] = yv;
      ws->minI[c] = i;
    }

    if (yv > ws->maxY[c]) {
      ws->maxY[c] = yv;
      ws->maxI[c] = i;
    }

    ws->lastI[c] = i;
    ++ws->cnt[c];
  }
}

/**
 * @brief Emit time-ordered first/min/max/last points for a single column.
 */
template<typename XAt, typename YAt>
inline void dsEmitColumnPoints(std::size_t c,
                               ssfp_t scaleY,
                               const DownsampleWorkspace* ws,
                               XAt xAt,
                               YAt yAt,
                               QList<QPointF>& out)
{
  int k = 0;
  std::size_t tmp[4];
  auto push_unique = [&](std::size_t v) {
    for (int j = 0; j < k; ++j)
      if (tmp[j] == v)
        return;

    tmp[k++] = v;
  };

  push_unique(ws->firstI[c]);

  const ssfp_t vspan_px = (ws->maxY[c] - ws->minY[c]) * scaleY;
  if (vspan_px >= 1.0) {
    push_unique(ws->minI[c]);
    push_unique(ws->maxI[c]);
  }

  push_unique(ws->lastI[c]);

  for (int a = 1; a < k; ++a) {
    int b         = a - 1;
    std::size_t v = tmp[a];
    while (b >= 0 && tmp[b] > v) {
      tmp[b + 1] = tmp[b];
      --b;
    }

    tmp[b + 1] = v;
  }

  for (int j = 0; j < k; ++j)
    out.append(QPointF(xAt(tmp[j]), yAt(tmp[j])));
}

//--------------------------------------------------------------------------------------------------
// Downsample 2D series into screen-space pixels
//--------------------------------------------------------------------------------------------------

/**
 * @brief Downsample a 2D series (X,Y) into screen-space pixels, preserving extremes.
 */
inline bool downsampleMonotonic(
  const AxisData& X, const AxisData& Y, int w, int h, QList<QPointF>& out, DownsampleWorkspace* ws)
{
  out.clear();
  const std::size_t n = std::min<std::size_t>(X.size(), Y.size());
  if (n == 0 || w <= 0 || h <= 0)
    return true;

  std::size_t xn0, xn1, yn0, yn1;
  const ssfp_t *xp0, *xp1, *yp0, *yp1;
  spanFromFixedQueue(X, xp0, xn0, xp1, xn1);
  spanFromFixedQueue(Y, yp0, yn0, yp1, yn1);

  auto xAt = [&](std::size_t i) -> ssfp_t {
    return (i < xn0) ? xp0[i] : xp1[i - xn0];
  };
  auto yAt = [&](std::size_t i) -> ssfp_t {
    return (i < yn0) ? yp0[i] : yp1[i - yn0];
  };

  const DownsampleBounds b = dsScanFiniteRange(n, xAt, yAt);
  if (b.firstFinite == n)
    return false;

  if (!(b.xmin < b.xmax)) {
    dsEmitSteppedFallback(n, w, b.firstFinite, b.lastFinite, xAt, yAt, out);
    return true;
  }

  const std::size_t C = std::size_t(w);
  ws->reset(C);

  const auto scaleX = static_cast<ssfp_t>(w - 1) / std::max(1e-12, b.xmax - b.xmin);
  const auto scaleY = static_cast<ssfp_t>(h) / std::max(1e-12, b.ymax - b.ymin);

  dsAccumulateBuckets(n, w, b.xmin, scaleX, xAt, yAt, ws);

  out.reserve(w * 3 / 2 + 8);
  for (std::size_t c = 0; c < C; ++c) {
    if (ws->cnt[c] == 0)
      continue;

    dsEmitColumnPoints(c, scaleY, ws, xAt, yAt, out);
  }

  return true;
}

/**
 * @brief Downsample a LineSeries (paired X and Y AxisData) for rendering.
 */
[[nodiscard]] inline bool downsampleMonotonic(
  const LineSeries& in, int width, int height, QList<QPointF>& out, DownsampleWorkspace* ws)
{
  return downsampleMonotonic(*in.x, *in.y, width, height, out, ws);
}

/**
 * @brief Decimate the visible [xLo, xHi] slice of a monotonic time ring to render columns.
 */
inline bool downsampleTimeWindow(const AxisData& timeX,
                                 const AxisData& valueY,
                                 ssfp_t xLo,
                                 ssfp_t xHi,
                                 int w,
                                 int h,
                                 QList<QPointF>& out,
                                 DownsampleWorkspace* ws)
{
  out.clear();
  const std::size_t n = std::min<std::size_t>(timeX.size(), valueY.size());
  if (n == 0 || w <= 0 || h <= 0 || !(xLo < xHi))
    return true;

  std::size_t xn0, xn1, yn0, yn1;
  const ssfp_t *xp0, *xp1, *yp0, *yp1;
  spanFromFixedQueue(timeX, xp0, xn0, xp1, xn1);
  spanFromFixedQueue(valueY, yp0, yn0, yp1, yn1);

  auto xAbs = [&](std::size_t i) -> ssfp_t {
    return (i < xn0) ? xp0[i] : xp1[i - xn0];
  };
  auto yAt = [&](std::size_t i) -> ssfp_t {
    return (i < yn0) ? yp0[i] : yp1[i - yn0];
  };

  // Rebase absolute ring time so the newest sample sits at 0 and the axis is [-T, 0]
  const ssfp_t newest = xAbs(n - 1);
  auto tRel           = [&](std::size_t i) -> ssfp_t {
    return xAbs(i) - newest;
  };

  // Locate the visible index span; the time ring is monotonic non-decreasing
  std::size_t lo = 0;
  while (lo < n && tRel(lo) < xLo)
    ++lo;

  std::size_t hi = lo;
  while (hi < n && tRel(hi) <= xHi)
    ++hi;

  const std::size_t visible = (hi > lo) ? (hi - lo) : 0;
  if (visible == 0)
    return true;

  // Re-base the accessors onto the visible slice so helpers see [0, visible)
  auto xWin = [&](std::size_t i) -> ssfp_t {
    return tRel(lo + i);
  };
  auto yWin = [&](std::size_t i) -> ssfp_t {
    return yAt(lo + i);
  };

  const std::size_t C = std::size_t(w);
  ws->reset(C);

  // Columns are bound to the viewport, not the data extent, so zoom maps 1:1
  const ssfp_t span        = std::max<ssfp_t>(1e-12, xHi - xLo);
  const auto scaleX        = static_cast<ssfp_t>(w - 1) / span;
  const DownsampleBounds b = dsScanFiniteRange(visible, xWin, yWin);
  if (b.firstFinite == visible)
    return false;

  const auto scaleY = static_cast<ssfp_t>(h) / std::max<ssfp_t>(1e-12, b.ymax - b.ymin);

  dsAccumulateBuckets(visible, w, xLo, scaleX, xWin, yWin, ws);

  out.reserve(w * 3 / 2 + 8);
  for (std::size_t c = 0; c < C; ++c) {
    if (ws->cnt[c] == 0)
      continue;

    dsEmitColumnPoints(c, scaleY, ws, xWin, yWin, out);
  }

  return true;
}

/**
 * @brief Decimate the [xLo, xHi] slice of a ring whose times are already window-relative.
 */
inline bool downsampleWindowAbsolute(const AxisData& timeX,
                                     const AxisData& valueY,
                                     ssfp_t xLo,
                                     ssfp_t xHi,
                                     int w,
                                     int h,
                                     QList<QPointF>& out,
                                     DownsampleWorkspace* ws)
{
  out.clear();
  const std::size_t n = std::min<std::size_t>(timeX.size(), valueY.size());
  if (n == 0 || w <= 0 || h <= 0 || !(xLo < xHi))
    return true;

  std::size_t xn0, xn1, yn0, yn1;
  const ssfp_t *xp0, *xp1, *yp0, *yp1;
  spanFromFixedQueue(timeX, xp0, xn0, xp1, xn1);
  spanFromFixedQueue(valueY, yp0, yn0, yp1, yn1);

  auto xAbs = [&](std::size_t i) -> ssfp_t {
    return (i < xn0) ? xp0[i] : xp1[i - xn0];
  };
  auto yAt = [&](std::size_t i) -> ssfp_t {
    return (i < yn0) ? yp0[i] : yp1[i - yn0];
  };

  // Locate the visible index span; sweep time is monotonic non-decreasing
  std::size_t lo = 0;
  while (lo < n && xAbs(lo) < xLo)
    ++lo;

  std::size_t hi = lo;
  while (hi < n && xAbs(hi) <= xHi)
    ++hi;

  const std::size_t visible = (hi > lo) ? (hi - lo) : 0;
  if (visible == 0)
    return true;

  auto xWin = [&](std::size_t i) -> ssfp_t {
    return xAbs(lo + i);
  };
  auto yWin = [&](std::size_t i) -> ssfp_t {
    return yAt(lo + i);
  };

  const std::size_t C = std::size_t(w);
  ws->reset(C);

  const ssfp_t span        = std::max<ssfp_t>(1e-12, xHi - xLo);
  const auto scaleX        = static_cast<ssfp_t>(w - 1) / span;
  const DownsampleBounds b = dsScanFiniteRange(visible, xWin, yWin);
  if (b.firstFinite == visible)
    return false;

  const auto scaleY = static_cast<ssfp_t>(h) / std::max<ssfp_t>(1e-12, b.ymax - b.ymin);

  dsAccumulateBuckets(visible, w, xLo, scaleX, xWin, yWin, ws);

  out.reserve(w * 3 / 2 + 8);
  for (std::size_t c = 0; c < C; ++c) {
    if (ws->cnt[c] == 0)
      continue;

    dsEmitColumnPoints(c, scaleY, ws, xWin, yWin, out);
  }

  return true;
}

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
  // Fast reject for NaNs (only applicable to floating-point types)
  if constexpr (std::floating_point<T>) {
    if (!std::isfinite(a) || !std::isfinite(b))
      return a == b;
  }

  const T diff = std::abs(a - b);

  // Absolute tolerance handles exact/near-zero and very small magnitudes.
  if (diff <= absEps)
    return true;

  // Relative tolerance handles larger magnitudes.
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
