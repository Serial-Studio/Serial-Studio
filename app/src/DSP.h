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
/**
 * @brief Fixed-capacity, auto-overwriting circular buffer (FIFO queue).
 */
template<typename T>
  requires std::copy_constructible<T> && std::is_copy_assignable_v<T>
class FixedQueue {
public:
  explicit FixedQueue(std::size_t capacity = 100)
    : m_capacity(capacity < 1 ? 1 : capacity)
    , m_data(std::shared_ptr<T[]>(new T[capacity < 1 ? 1 : capacity]))
    , m_start(0)
    , m_size(0)
  {}

  FixedQueue(const FixedQueue&) = default;
  FixedQueue& operator=(const FixedQueue&) = default;
  FixedQueue(FixedQueue&&) noexcept = default;
  FixedQueue& operator=(FixedQueue&&) noexcept = default;

  [[nodiscard]] const T& operator[](std::size_t index) const
  {
    Q_ASSERT(index < m_size);
    return m_data[wrappedIndex(index < m_size ? index : 0)];
  }

  [[nodiscard]] T& operator[](std::size_t index)
  {
    Q_ASSERT(index < m_size);
    return m_data[wrappedIndex(index < m_size ? index : 0)];
  }

  [[nodiscard]] std::size_t size() const { return m_size; }
  [[nodiscard]] std::size_t capacity() const { return m_capacity; }
  [[nodiscard]] bool full() const { return m_size == m_capacity; }
  [[nodiscard]] bool empty() const { return m_size == 0; }
  [[nodiscard]] std::size_t frontIndex() const { return m_start; }

  [[nodiscard]] const T& at(std::size_t index) const
  {
    Q_ASSERT(index < m_capacity);
    return m_data[wrappedIndex(index < m_capacity ? index : 0)];
  }

  [[nodiscard]] const T& front() const
  {
    if (m_size == 0) [[unlikely]] {
      qWarning() << "DSP::RingBuffer::front() called on empty buffer";
      return m_data[0];
    }

    return m_data[m_start];
  }

  [[nodiscard]] const T& back() const
  {
    if (m_size == 0) [[unlikely]] {
      qWarning() << "DSP::RingBuffer::back() called on empty buffer";
      return m_data[0];
    }

    return m_data[wrappedIndex(m_size - 1)];
  }

  [[nodiscard]] const T* raw() const { return m_data.get(); }
  [[nodiscard]] T* raw() { return m_data.get(); }

  void clear()
  {
    m_start = 0;
    m_size  = 0;
  }

  void fill(const T& value)
  {
    clear();
    for (std::size_t i = 0; i < m_capacity; ++i)
      push(value);
  }

  void fillRange(const T& start, const T& step = 1)
    requires Concepts::Numeric<T>
  {
    clear();
    for (std::size_t i = 0; i < m_capacity; ++i)
      push(start + static_cast<T>(i) * step);
  }

  void push(const T& item)
  {
    m_data[endIndex()] = item;
    advance();
  }

  void push(T&& item)
  {
    m_data[endIndex()] = std::move(item);
    advance();
  }

  void resize(std::size_t newCapacity)
  {
    if (newCapacity < 1)
      newCapacity = 1;

    if (newCapacity == m_capacity)
      return;

    std::shared_ptr<T[]> newData(new T[newCapacity]);
    std::size_t elementsToCopy = std::min(m_size, newCapacity);
    for (std::size_t i = 0; i < elementsToCopy; ++i)
      newData[i] = std::move((*this)[m_size - elementsToCopy + i]);

    m_start    = 0;
    m_size     = elementsToCopy;
    m_capacity = newCapacity;
    m_data     = std::move(newData);
  }

private:
  std::size_t endIndex() const { return (m_start + m_size) % m_capacity; }
  std::size_t wrappedIndex(std::size_t index) const { return (m_start + index) % m_capacity; }

  void advance()
  {
    if (m_size < m_capacity)
      ++m_size;
    else
      m_start = (m_start + 1) % m_capacity;
  }

private:
  std::size_t m_capacity;
  std::shared_ptr<T[]> m_data;
  std::size_t m_start;
  std::size_t m_size;
};

typedef double ssfp_t;
typedef FixedQueue<ssfp_t> AxisData;
typedef std::vector<AxisData> MultiPlotDataY;

/**
 * @brief Paired X and Y axis data for a single plot line.
 */
typedef struct {
  AxisData* x;
  AxisData* y;
} LineSeries;

/**
 * @brief Set of plot lines sharing a single X axis.
 */
typedef struct {
  AxisData* x;
  std::vector<AxisData> y;
} MultiLineSeries;

#ifdef BUILD_COMMERCIAL
typedef std::vector<QVector3D> LineSeries3D;
#endif

/**
 * @brief Time-ordered sequence of GPS position data (lat/lon/alt).
 */
typedef struct {
  FixedQueue<ssfp_t> latitudes;
  FixedQueue<ssfp_t> longitudes;
  FixedQueue<ssfp_t> altitudes;
} GpsSeries;

/**
 * @brief Scratch buffers for column-wise downsampling.
 */
struct DownsampleWorkspace {
  std::vector<unsigned int> cnt;
  std::vector<ssfp_t> minY;
  std::vector<ssfp_t> maxY;
  std::vector<std::size_t> minI;
  std::vector<std::size_t> maxI;
  std::vector<std::size_t> firstI;
  std::vector<std::size_t> lastI;

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

    std::fill(cnt.begin(), cnt.end(), 0u);
    std::fill(minY.begin(), minY.end(), std::numeric_limits<ssfp_t>::infinity());
    std::fill(maxY.begin(), maxY.end(), -std::numeric_limits<ssfp_t>::infinity());
  }
};

template<typename T>
  requires std::copy_constructible<T> && std::is_copy_assignable_v<T>
inline void spanFromFixedQueue(
  const FixedQueue<T>& q, const T*& p0, std::size_t& n0, const T*& p1, std::size_t& n1)
{
  const T* base = q.raw();

  const std::size_t n    = q.size();
  const std::size_t cap  = q.capacity();
  const std::size_t i0   = q.frontIndex();
  const std::size_t tail = std::min<std::size_t>(n, cap - i0);

  p0 = base + i0;
  n0 = tail;

  p1 = base;
  n1 = n - tail;
}

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

  std::size_t lastFinite  = n;
  std::size_t firstFinite = n;
  ssfp_t xmin             = std::numeric_limits<ssfp_t>::infinity();
  ssfp_t ymin             = std::numeric_limits<ssfp_t>::infinity();
  ssfp_t xmax             = -std::numeric_limits<ssfp_t>::infinity();
  ssfp_t ymax             = -std::numeric_limits<ssfp_t>::infinity();
  for (std::size_t i = 0; i < n; ++i) {
    const ssfp_t xv = xAt(i);
    const ssfp_t yv = yAt(i);
    if (!std::isfinite(xv) || !std::isfinite(yv))
      continue;

    if (xv < xmin)
      xmin = xv;
    if (xv > xmax)
      xmax = xv;
    if (yv < ymin)
      ymin = yv;
    if (yv > ymax)
      ymax = yv;

    if (firstFinite == n)
      firstFinite = i;

    lastFinite = i;
  }

  if (firstFinite == n)
    return false;

  if (!(xmin < xmax)) {
    std::size_t step = n / (std::max(1, w));
    if (step < 1)
      step = 1;

    for (std::size_t i = firstFinite; i <= lastFinite; i += step)
      out.append(QPointF(xAt(i), yAt(i)));

    if (out.isEmpty() || out.back().x() != xAt(lastFinite))
      out.append(QPointF(xAt(lastFinite), yAt(lastFinite)));

    return true;
  }

  const std::size_t C = std::size_t(w);
  ws->reset(C);

  const auto scaleX = static_cast<ssfp_t>(w - 1) / std::max(1e-12, xmax - xmin);
  const auto scaleY = static_cast<ssfp_t>(h) / std::max(1e-12, ymax - ymin);

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
    }

    else {
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

  out.reserve(w * 3 / 2 + 8);
  for (std::size_t c = 0; c < C; ++c) {
    if (ws->cnt[c] == 0)
      continue;

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

  return true;
}

[[nodiscard]] inline bool downsampleMonotonic(
  const LineSeries& in, int width, int height, QList<QPointF>& out, DownsampleWorkspace* ws)
{
  return downsampleMonotonic(*in.x, *in.y, width, height, out, ws);
}

template<Concepts::Numeric T>
[[nodiscard]] inline bool isZero(T value, T absEps = T(1e-12)) noexcept
{
  return std::abs(value) <= absEps;
}

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

template<Concepts::Numeric T>
[[nodiscard]] inline bool notEqual(T a, T b, T relEps = T(1e-12), T absEps = T(1e-12)) noexcept
{
  return !almostEqual(a, b, relEps, absEps);
}

}  // namespace DSP
