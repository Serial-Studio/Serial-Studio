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

#include <QList>
#include <QPointF>
#include <QObject>
#include <QVector>
#include <QVector3D>

#include <cmath>
#include <cstddef>
#include <cstring>
#include <cstdlib>

#include <memory>
#include <vector>
#include <limits>
#include <stdexcept>

namespace DSP
{
//------------------------------------------------------------------------------
// Fixed queue implementation with a circular buffer
//------------------------------------------------------------------------------

/**
 * @brief A fixed-capacity, auto-overwriting circular buffer (FIFO queue).
 *
 * This container provides constant-time insertion and access operations.
 * Once full, new elements overwrite the oldest ones. Internally backed by
 * a shared pointer to a heap-allocated array.
 *
 * @tparam T Type of elements stored in the queue.
 */
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
    if (index >= m_size)
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
    if (index >= m_size)
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

//------------------------------------------------------------------------------
// Type Aliases
//------------------------------------------------------------------------------

/**
 * @typedef PlotDataX
 * @brief Represents the unique X-axis data points for a plot.
 */
typedef FixedQueue<double> AxisData;

/**
 * @typedef MultiPlotDataY
 * @brief Represents Y-axis data for multiple curves in a multiplot.
 *
 * A vector of `PlotDataY` structures, where each element represents
 * the Y-axis data for one curve in a multiplot widget. This allows
 * managing multiple curves independently.
 */
typedef std::vector<AxisData> MultiPlotDataY;

//------------------------------------------------------------------------------
// Composite Data Structures
//------------------------------------------------------------------------------

/**
 * @typedef LineSeries
 * @brief Represents a paired series of X-axis and Y-axis data for a plot.
 *
 * The `LineSeries` type is defined as a `QPair` containing:
 * - A pointer to `PlotDataX`, which holds the unique X-axis values.
 * - A pointer to `PlotDataY`, which holds the Y-axis values.
 *
 * This type simplifies data processing by tightly coupling the related X and Y
 * data for a plot, ensuring that they are always accessed and managed together.
 */
typedef struct
{
  AxisData *x; ///< X-axis data (e.g., time or samples)
  AxisData *y; ///< Y-axis data (e.g., sensor readings)
} LineSeries;

/**
 * @struct MultiLineSeries
 * @brief Represents a set of line plots sharing a common X-axis.
 *
 * This structure is used for rendering multi-curve plots, such as when multiple
 * sensors or variables are plotted against the same time base or domain.
 *
 * - `x`: Pointer to the shared X-axis data (e.g., time).
 * - `y`: A list of Y-axis data vectors, where each entry represents one curve.
 *
 * All Y-series are expected to align with the length and indexing of the
 * shared X-axis.
 */
typedef struct
{
  AxisData *x;             ///< Shared X-axis data (e.g., time or index)
  std::vector<AxisData> y; ///< Y-axis data for each individual curve
} MultiLineSeries;

#ifdef BUILD_COMMERCIAL
/**
 * @typedef PlotData3D
 * @brief Represents a list of 3D points.
 */
typedef std::vector<QVector3D> LineSeries3D;
#endif

/**
 * @struct GpsSeries
 * @brief Represents a time-ordered sequence of GPS position data.
 *
 * This structure stores a trajectory as three parallel vectors:
 * - `latitudes`: The latitude values in degrees.
 * - `longitudes`: The longitude values in degrees.
 * - `altitudes`: The altitude values in meters.
 *
 * Each index across the three vectors corresponds to a single
 * recorded GPS point in time. This format is optimized for
 * visualization, analysis, and storage of path-based geospatial data.
 */
typedef struct
{
  FixedQueue<double> latitudes;  ///< Latitude values (degrees)
  FixedQueue<double> longitudes; ///< Longitude values (degrees)
  FixedQueue<double> altitudes;  ///< Altitude values (meters)
} GpsSeries;

//------------------------------------------------------------------------------
// Downsampling workspace
//------------------------------------------------------------------------------

/**
 * @brief Scratch buffers for column-wise downsampling.
 *
 * This workspace holds per-column accumulators that the downsampler reuses
 * across frames. Reusing these vectors avoids heap churn and keeps the hot
 * path O(n + width) with minimal allocations.
 *
 * Each column c in [0, C) tracks:
 *  - cnt[c]: how many valid samples fell into the column
 *  - firstI[c]: logical index of the first sample seen in the column
 *  - lastI[c]:  logical index of the last sample seen in the column
 *  - minY[c]: smallest Y value seen in the column
 *  - maxY[c]: largest  Y value seen in the column
 *  - minI[c]: logical index where minY[c] occurred
 *  - maxI[c]: logical index where maxY[c] occurred
 *
 * The downsampler uses these to emit up to four ordered points per column:
 * first, min, max, last. It then de-duplicates and time-sorts them to keep
 * the polyline stable and spike-free.
 *
 * @note Thread safety
 *       One instance is not thread-safe. Use one workspace per rendering
 *       thread or guard access externally.
 *
 * @note Lifetime
 *       You can keep a workspace as a member of your widget and pass it
 *       on every call. The downsampler will resize and reset it as needed.
 */
struct DownsampleWorkspace
{
  // Number of valid samples that landed in each column
  std::vector<unsigned int> cnt;

  // Column-wise vertical extrema
  std::vector<double> minY;
  std::vector<double> maxY;

  // Logical indices associated with extrema and endpoints per column
  std::vector<std::size_t> minI;
  std::vector<std::size_t> maxI;
  std::vector<std::size_t> firstI;
  std::vector<std::size_t> lastI;

  /**
   * @brief Prepare the workspace for a render pass with C columns.
   *
   * Resizes internal arrays to exactly C elements if needed, then resets
   * counters and extrema for a fresh accumulation pass. Existing capacity
   * is reused when possible to avoid allocations.
   *
   * @param C Number of horizontal pixel columns in the target plot.
   *
   * @post
   *  - cnt[c] is zero for all c
   *  - minY[c] is +inf and maxY[c] is −inf for all c
   *  - index arrays keep their size C and will be assigned by the caller
   */
  void reset(std::size_t C)
  {
    if (cnt.size() != C)
    {
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
    std::fill(minY.begin(), minY.end(),
              std::numeric_limits<double>::infinity());
    std::fill(maxY.begin(), maxY.end(),
              -std::numeric_limits<double>::infinity());
  }
};

//------------------------------------------------------------------------------
// Ring helper
//------------------------------------------------------------------------------

/**
 * @brief Get two contiguous spans from a ring-buffered queue.
 *
 * A `FixedQueue` (ring buffer) stores elements in a raw array with wraparound.
 * At any given time, the logical data may be split into two contiguous spans:
 *
 *   [p0, p0 + n0)  followed by  [p1, p1 + n1)
 *
 * Together these cover exactly q.size() elements in logical order.
 * One of the spans may be empty if the data is already contiguous.
 *
 * This routine exposes raw pointers and lengths into the queue’s storage.
 * Callers can then scan elements linearly without worrying about wraparound.
 *
 * Example:
 * ```
 * const double* p0; size_t n0;
 * const double* p1; size_t n1;
 * spanFromFixedQueue(myQueue, p0, n0, p1, n1);
 * for (size_t i = 0; i < n0; ++i) use(p0[i]);
 * for (size_t i = 0; i < n1; ++i) use(p1[i]);
 * ```
 *
 * @param q   Ring-buffer queue to extract spans from
 * @param p0  Output pointer to first contiguous block
 * @param n0  Output number of elements in first block
 * @param p1  Output pointer to second contiguous block
 * @param n1  Output number of elements in second block
 */
inline void spanFromFixedQueue(const AxisData &q, const double *&p0,
                               std::size_t &n0, const double *&p1,
                               std::size_t &n1)
{
  const double *base = q.raw();

  const std::size_t n = q.size();
  const std::size_t cap = q.capacity();
  const std::size_t i0 = q.frontIndex();
  const std::size_t tail = std::min<std::size_t>(n, cap - i0);

  p0 = base + i0;
  n0 = tail;

  p1 = base;
  n1 = n - tail;
}

//------------------------------------------------------------------------------
// Downsample 2D series into screen-space pixels
//------------------------------------------------------------------------------

/**
 * @brief Downsample a 2D series (X,Y) into screen-space pixels, preserving
 *        extremes.
 *
 * This algorithm assumes that X is monotonic (non-decreasing), which is the
 * common case for time series data.
 *
 * It reduces `n` raw points down to ~w representative points suitable for
 * plotting without aliasing.
 *
 * Procedure:
 *  1. Scan all points to find finite range (xmin..xmax, ymin..ymax).
 *  2. Divide the X span into `w` vertical buckets (screen columns).
 *  3. For each bucket, track:
 *       - First point index
 *       - Last point index
 *       - Min-Y point index
 *       - Max-Y point index
 *  4. Emit at most 4 points per bucket, in chronological order:
 *     first, (min,max if Y span crosses >= 1 pixel) and last.
 *
 * The output preserves important vertical features (peaks/valleys) while
 * reducing density enough to render interactively.
 *
 * @param X   Ring-buffer of X values (must be monotonic)
 * @param Y   Ring-buffer of Y values (same length as X)
 * @param w   Target plot width in pixels
 * @param h   Target plot height in pixels
 * @param out Output polyline of downsampled points (cleared before use)
 * @param ws  Workspace for temporary bucket storage (reused across calls)
 *
 * @return true always, false only if inputs invalid.
 */
inline bool downsampleMonotonic(const AxisData &X, const AxisData &Y, int w,
                                int h, QList<QPointF> &out,
                                DownsampleWorkspace *ws)
{
  // Clear the buffer and validate input data
  out.clear();
  const std::size_t n = std::min<std::size_t>(X.size(), Y.size());
  if (n == 0 || w <= 0 || h <= 0)
    return true;

  // Extract ring buffer spans from data containers
  std::size_t xn0, xn1, yn0, yn1;
  const double *xp0, *xp1, *yp0, *yp1;
  spanFromFixedQueue(X, xp0, xn0, xp1, xn1);
  spanFromFixedQueue(Y, yp0, yn0, yp1, yn1);

  // Functions to map logical indexes to X and Y independently
  auto xAt = [&](std::size_t i) -> double {
    return (i < xn0) ? xp0[i] : xp1[i - xn0];
  };
  auto yAt = [&](std::size_t i) -> double {
    return (i < yn0) ? yp0[i] : yp1[i - yn0];
  };

  // Find data bounds
  std::size_t lastFinite = n;
  std::size_t firstFinite = n;
  double xmin = std::numeric_limits<double>::infinity();
  double ymin = std::numeric_limits<double>::infinity();
  double xmax = -std::numeric_limits<double>::infinity();
  double ymax = -std::numeric_limits<double>::infinity();
  for (std::size_t i = 0; i < n; ++i)
  {
    // Get & validate raw points
    const double xv = xAt(i);
    const double yv = yAt(i);
    if (!std::isfinite(xv) || !std::isfinite(yv))
      continue;

    // Update data boundaries
    if (xv < xmin)
      xmin = xv;
    if (xv > xmax)
      xmax = xv;
    if (yv < ymin)
      ymin = yv;
    if (yv > ymax)
      ymax = yv;

    // Set first valid point
    if (firstFinite == n)
      firstFinite = i;

    // Update last valid point
    lastFinite = i;
  }

  // Catch edge cases where all the data is invalid
  if (firstFinite == n)
    return false;

  // If X axis is not monotonic, fallback to index sampling
  if (!(xmin < xmax))
  {
    // Calculate step size
    std::size_t step = n / (std::max(1, w));
    if (step < 1)
      step = 1;

    // Add a point for each step
    for (std::size_t i = firstFinite; i <= lastFinite; i += step)
      out.append(QPointF(xAt(i), yAt(i)));

    // Add last point if needed
    if (out.isEmpty() || out.back().x() != xAt(lastFinite))
      out.append(QPointF(xAt(lastFinite), yAt(lastFinite)));

    // Early exit
    return true;
  }

  // Prepare buckets
  const std::size_t C = std::size_t(w);
  ws->reset(C);

  // Scaling constants
  const auto scaleX = static_cast<double>(w - 1) / std::max(1e-12, xmax - xmin);
  const auto scaleY = static_cast<double>(h) / std::max(1e-12, ymax - ymin);

  // Lambda function to obtain the column index for a "raw" x index
  auto getColFromX = [&](double x) -> std::size_t {
    auto c = static_cast<long>((x - xmin) * scaleX);
    if (c < 0)
      c = 0;

    else if (c >= w)
      c = w - 1;

    return std::size_t(c);
  };

  // Fill buckets
  for (std::size_t i = 0; i < n; ++i)
  {
    // Obtain raw points & validate them
    const double xv = xAt(i);
    const double yv = yAt(i);
    if (!std::isfinite(xv) || !std::isfinite(yv))
      continue;

    // Get column
    const std::size_t c = getColFromX(xv);

    // Register first point for column
    if (ws->cnt[c] == 0)
    {
      ws->firstI[c] = ws->lastI[c] = i;
      ws->minI[c] = ws->maxI[c] = i;
      ws->minY[c] = ws->maxY[c] = yv;
      ws->cnt[c] = 1;
    }

    // Register min/max values for column
    else
    {
      if (yv < ws->minY[c])
      {
        ws->minY[c] = yv;
        ws->minI[c] = i;
      }

      if (yv > ws->maxY[c])
      {
        ws->maxY[c] = yv;
        ws->maxI[c] = i;
      }

      ws->lastI[c] = i;
      ++ws->cnt[c];
    }
  }

  // Register time-ordered points per column: first, min, max, last
  out.reserve(w * 3 / 2 + 8);
  for (std::size_t c = 0; c < C; ++c)
  {
    // Skip columns widhout data
    if (ws->cnt[c] == 0)
      continue;

    // Utility lambda to avoid adding duplicated points
    int k = 0;
    std::size_t tmp[4];
    auto push_unique = [&](std::size_t v) {
      for (int j = 0; j < k; ++j)
        if (tmp[j] == v)
          return;

      tmp[k++] = v;
    };

    // Add first point
    push_unique(ws->firstI[c]);

    // Add minimum & maximum points (if needed)
    const double vspan_px = (ws->maxY[c] - ws->minY[c]) * scaleY;
    if (vspan_px >= 1.0)
    {
      push_unique(ws->minI[c]);
      push_unique(ws->maxI[c]);
    }

    // Add last point
    push_unique(ws->lastI[c]);

    // Sort the column points into ascending order
    for (int a = 1; a < k; ++a)
    {
      int b = a - 1;
      std::size_t v = tmp[a];
      while (b >= 0 && tmp[b] > v)
      {
        tmp[b + 1] = tmp[b];
        --b;
      }

      tmp[b + 1] = v;
    }

    // Append the generated points
    for (int j = 0; j < k; ++j)
      out.append(QPointF(xAt(tmp[j]), yAt(tmp[j])));
  }

  // Success
  return true;
}

/**
 * Downsample a LineSeries (paired X and Y AxisData) for rendering.
 *
 * Convenience overload that just forwards to the main
 * downsampleMonotonic() implementation.
 *
 * @param in     Input line series with X and Y data.
 * @param width  Target pixel width of output.
 * @param height Target pixel height of output.
 * @param out    Output vector of downsampled points (cleared first).
 * @param ws     Optional workspace for bucket storage. Pass nullptr to
 *               use a temporary.
 *
 * @return true on success, false on parameter mismatch.
 */
inline bool downsampleMonotonic(const LineSeries &in, int width, int height,
                                QList<QPointF> &out, DownsampleWorkspace *ws)
{
  return downsampleMonotonic(*in.x, *in.y, width, height, out, ws);
}

} // namespace DSP
