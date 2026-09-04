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
#include <cstddef>
#include <limits>
#include <QList>
#include <QPointF>
#include <vector>

#include "Core/DSPSimd.h"
#include "Core/SSAssert.h"
#include "DSP.h"

namespace DSP {
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
 * @brief First index in [0, n) whose X value is >= bound. Binary search: X must be
 *        monotonic non-decreasing (len halves per step, so 64 iterations suffice).
 */
template<typename XAt>
[[nodiscard]] inline std::size_t dsLowerBound(std::size_t n, XAt xAt, const ssfp_t bound)
{
  std::size_t lo  = 0;
  std::size_t len = n;
  for (int step = 0; step < 64 && len > 0; ++step) {
    const std::size_t half = len / 2;
    const std::size_t mid  = lo + half;
    if (xAt(mid) < bound) {
      lo  = mid + 1;
      len = len - half - 1;
    }

    else
      len = half;
  }

  return lo;
}

/**
 * @brief First index in [0, n) whose X value is > bound. Binary search: X must be
 *        monotonic non-decreasing (len halves per step, so 64 iterations suffice).
 */
template<typename XAt>
[[nodiscard]] inline std::size_t dsUpperBound(std::size_t n, XAt xAt, const ssfp_t bound)
{
  std::size_t lo  = 0;
  std::size_t len = n;
  for (int step = 0; step < 64 && len > 0; ++step) {
    const std::size_t half = len / 2;
    const std::size_t mid  = lo + half;
    if (xAt(mid) <= bound) {
      lo  = mid + 1;
      len = len - half - 1;
    }

    else
      len = half;
  }

  return lo;
}

/**
 * @brief Finds the first and last indices whose (X, Y) pair is fully finite. Returns
 *        false when no finite pair exists. O(1) for clean data, O(n) worst case.
 */
template<typename XAt, typename YAt>
[[nodiscard]] inline bool dsFiniteEnds(
  std::size_t n, XAt xAt, YAt yAt, std::size_t& first, std::size_t& last)
{
  first = n;
  for (std::size_t i = 0; i < n; ++i) {
    if (std::isfinite(xAt(i)) && std::isfinite(yAt(i))) {
      first = i;
      break;
    }
  }

  if (first == n)
    return false;

  last = first;
  for (std::size_t i = n; i > first; --i) {
    if (std::isfinite(xAt(i - 1)) && std::isfinite(yAt(i - 1))) {
      last = i - 1;
      break;
    }
  }

  return true;
}

/**
 * @brief Extracts the global Y bounds from the filled workspace columns; returns false
 *        when no column received a finite sample. Empty columns hold the +/-inf
 *        identities reset() installed, so the branch-free SIMD reduction matches the old
 *        cnt-guarded loop and "no finite sample" becomes ymin > ymax.
 */
[[nodiscard]] inline bool dsColumnYBounds(const DownsampleWorkspace* ws,
                                          std::size_t C,
                                          ssfp_t& ymin,
                                          ssfp_t& ymax)
{
  SS_ASSERT(ws != nullptr, return false);
  SS_ASSERT(C > 0, return false);
  SS_ASSERT(ws->minY.size() >= C, return false);
  SS_ASSERT(ws->maxY.size() >= C, return false);

  ymin = simdMinF64(ws->minY.data(), C);
  ymax = simdMaxF64(ws->maxY.data(), C);
  return ymin <= ymax;
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
 *        X must be monotonic non-decreasing: the X bounds come from the finite
 *        endpoints and the Y bounds from the filled columns, so the samples are
 *        walked once (bucket accumulation) instead of twice.
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

  std::size_t firstFinite = 0;
  std::size_t lastFinite  = 0;
  if (!dsFiniteEnds(n, xAt, yAt, firstFinite, lastFinite))
    return false;

  const ssfp_t xmin = xAt(firstFinite);
  const ssfp_t xmax = xAt(lastFinite);
  if (!(xmin < xmax)) {
    dsEmitSteppedFallback(n, w, firstFinite, lastFinite, xAt, yAt, out);
    return true;
  }

  const std::size_t C = std::size_t(w);
  ws->reset(C);

  const auto scaleX = static_cast<ssfp_t>(w - 1) / std::max(1e-12, xmax - xmin);
  dsAccumulateBuckets(n, w, xmin, scaleX, xAt, yAt, ws);

  ssfp_t ymin = 0;
  ssfp_t ymax = 0;
  if (!dsColumnYBounds(ws, C, ymin, ymax))
    return false;

  const auto scaleY = static_cast<ssfp_t>(h) / std::max(1e-12, ymax - ymin);

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
 * @brief Shared body of the time-window downsamplers over @p n monotonic (time, value) points
 *        rebased so @p newest sits at 0 (axis [-T, 0]): the window resolves as two binary
 *        searches, the slice is walked once, and buckets sit on an absolute newest-anchored
 *        column-width lattice (shimmer fix).
 */
template<typename XAt, typename YAt>
inline bool dsTimeWindowCore(std::size_t n,
                             XAt xAbs,
                             YAt yAt,
                             ssfp_t newest,
                             ssfp_t xLo,
                             ssfp_t xHi,
                             int w,
                             int h,
                             QList<QPointF>& out,
                             DownsampleWorkspace* ws)
{
  SS_ASSERT(ws != nullptr, return false);
  SS_ASSERT(n > 0, return true);

  auto tRel = [&](std::size_t i) -> ssfp_t {
    return xAbs(i) - newest;
  };

  const std::size_t lo = dsLowerBound(n, xAbs, xLo + newest);
  const std::size_t hi = dsUpperBound(n, xAbs, xHi + newest);

  const std::size_t visible = (hi > lo) ? (hi - lo) : 0;
  if (visible == 0)
    return true;

  auto xWin = [&](std::size_t i) -> ssfp_t {
    return tRel(lo + i);
  };
  auto yWin = [&](std::size_t i) -> ssfp_t {
    return yAt(lo + i);
  };

  const std::size_t C = std::size_t(w);
  ws->reset(C);

  const ssfp_t span        = std::max<ssfp_t>(1e-12, xHi - xLo);
  const auto scaleX        = static_cast<ssfp_t>(w - 1) / span;
  const ssfp_t colWidth    = span / static_cast<ssfp_t>(std::max(1, w - 1));
  const ssfp_t anchorShift = newest - std::floor(newest / colWidth) * colWidth;

  auto xBkt = [&](std::size_t i) -> ssfp_t {
    return tRel(lo + i) + anchorShift;
  };

  dsAccumulateBuckets(visible, w, xLo, scaleX, xBkt, yWin, ws);

  ssfp_t ymin = 0;
  ssfp_t ymax = 0;
  if (!dsColumnYBounds(ws, C, ymin, ymax))
    return false;

  const auto scaleY = static_cast<ssfp_t>(h) / std::max<ssfp_t>(1e-12, ymax - ymin);

  out.reserve(w * 3 / 2 + 8);
  for (std::size_t c = 0; c < C; ++c) {
    if (ws->cnt[c] == 0)
      continue;

    dsEmitColumnPoints(c, scaleY, ws, xWin, yWin, out);
  }

  return true;
}

/**
 * @brief Decimate the visible [xLo, xHi] slice of a monotonic time ring to render columns; the
 *        newest sample is the axis' 0. See dsTimeWindowCore() for the bucket lattice.
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

  return dsTimeWindowCore(n, xAbs, yAt, xAbs(n - 1), xLo, xHi, w, h, out, ws);
}

/**
 * @brief Decimate the visible [xLo, xHi] slice of an envelope pyramid (spec 0057): the coarsest
 *        level whose cells fit under one render column and still cover the window feeds the
 *        same column lattice with its extreme pairs; level 0 reads as the plain overload. The
 *        axis' 0 is always level 0's newest sample, so a coarse trace never shifts right.
 */
inline bool downsampleTimeWindow(const EnvelopeRing& ring,
                                 ssfp_t xLo,
                                 ssfp_t xHi,
                                 int w,
                                 int h,
                                 QList<QPointF>& out,
                                 DownsampleWorkspace* ws)
{
  out.clear();
  const AxisData& timeX  = ring.level0.time;
  const AxisData& valueY = ring.level0.value;
  const std::size_t n0   = std::min<std::size_t>(timeX.size(), valueY.size());
  if (n0 == 0 || w <= 0 || h <= 0 || !(xLo < xHi))
    return true;

  const ssfp_t newest = timeX[n0 - 1];
  const ssfp_t oldest = std::max<ssfp_t>(xLo + newest, timeX[0]);
  const int level     = ring.selectLevel(xHi - xLo, w, oldest);
  if (level <= 0 || level > ring.coarseLevelCount())
    return downsampleTimeWindow(timeX, valueY, xLo, xHi, w, h, out, ws);

  const auto& cells   = ring.levels[static_cast<std::size_t>(level - 1)].cells;
  const std::size_t n = 2 * cells.size();
  if (n == 0)
    return true;

  std::size_t cn0, cn1;
  const EnvelopeCell *cp0, *cp1;
  spanFromFixedQueue(cells, cp0, cn0, cp1, cn1);

  auto cellAt = [&](std::size_t i) -> const EnvelopeCell& {
    return (i < cn0) ? cp0[i] : cp1[i - cn0];
  };
  auto xAbs = [&](std::size_t i) -> ssfp_t {
    const EnvelopeCell& c = cellAt(i >> 1);
    return (i & 1) ? c.t1 : c.t0;
  };
  auto yAt = [&](std::size_t i) -> ssfp_t {
    const EnvelopeCell& c = cellAt(i >> 1);
    return (i & 1) ? c.v1 : c.v0;
  };

  return dsTimeWindowCore(n, xAbs, yAt, newest, xLo, xHi, w, h, out, ws);
}

/**
 * @brief Decimate the [xLo, xHi] slice of a ring whose times are already window-relative.
 *        Monotonic non-decreasing sweep time lets the visible span resolve as two binary
 *        searches; the slice is walked once (bucket accumulation; Y bounds come from the
 *        filled columns).
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

  const std::size_t lo = dsLowerBound(n, xAbs, xLo);
  const std::size_t hi = dsUpperBound(n, xAbs, xHi);

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

  const ssfp_t span = std::max<ssfp_t>(1e-12, xHi - xLo);
  const auto scaleX = static_cast<ssfp_t>(w - 1) / span;

  dsAccumulateBuckets(visible, w, xLo, scaleX, xWin, yWin, ws);

  ssfp_t ymin = 0;
  ssfp_t ymax = 0;
  if (!dsColumnYBounds(ws, C, ymin, ymax))
    return false;

  const auto scaleY = static_cast<ssfp_t>(h) / std::max<ssfp_t>(1e-12, ymax - ymin);

  out.reserve(w * 3 / 2 + 8);
  for (std::size_t c = 0; c < C; ++c) {
    if (ws->cnt[c] == 0)
      continue;

    dsEmitColumnPoints(c, scaleY, ws, xWin, yWin, out);
  }

  return true;
}

}  // namespace DSP
