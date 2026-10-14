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

#include "Core/Concepts.h"
#include "Core/DSPSimd.h"
#include "Core/SSAssert.h"
#include "DSP/FixedQueue.h"

namespace DSP {
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
 * @brief Slot count of a time ring. The only source is a sample rate over a window: a ring
 *        bounded in samples alone runs out of history as soon as the rate is high (44.1 kHz
 *        filled a 10 s axis to 5.9 s), so a bare count does not convert into one.
 */
class RingCapacity {
public:
  static constexpr int kMaxSamples = 1 << 22;

  /**
   * @brief Slots that hold @p windowSec seconds at @p rateHz, truncated, within [1, kMaxSamples].
   */
  [[nodiscard]] static RingCapacity fromRate(double windowSec, double rateHz) noexcept
  {
    const double want = windowSec * rateHz;
    if (!(want >= 1.0))
      return RingCapacity(1);

    return RingCapacity(static_cast<int>(std::min(want, static_cast<double>(kMaxSamples))));
  }

  /**
   * @brief This capacity raised to at least @p floor slots (a policy bound, never a source).
   */
  [[nodiscard]] RingCapacity atLeast(int floor) const noexcept
  {
    return RingCapacity(std::clamp(floor, m_slots, kMaxSamples));
  }

  /**
   * @brief This capacity lowered to at most @p ceiling slots (a policy bound, never a source).
   */
  [[nodiscard]] RingCapacity atMost(int ceiling) const noexcept
  {
    return RingCapacity(std::clamp(ceiling, 1, m_slots));
  }

  [[nodiscard]] int value() const noexcept { return m_slots; }

private:
  friend struct TimeRing;

  explicit RingCapacity(int slotCount) noexcept : m_slots(slotCount) {}

  int m_slots;
};

/**
 * @brief How a TimeRing stores what it is given: every sample in its own slot (Exact), or a
 *        min/max pair per absolute-time grid cell so a fixed capacity spans the window
 *        (Decimating). A ring is exact while it can afford every sample (spec 0097).
 */
enum class RingMode {
  Decimating,
  Exact
};

/**
 * @brief Bounded (time, value) ring over a fixed window: exact (one slot per sample) or
 *        decimating on ingest to a min/max envelope per grid cell.
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
  RingMode mode;

  /**
   * @brief Constructs the ring with `capacity` slots covering `windowSec` seconds. Each
   *        grid cell may retain two slots (min + max), so the interval reserves both to
   *        keep a saturated source spanning the full window. Storage is exactly the capacity.
   */
  explicit TimeRing(RingCapacity capacity = RingCapacity::fromRate(1.0, 1.0),
                    double windowSec      = 1.0,
                    RingMode ringMode     = RingMode::Decimating)
    : time(static_cast<std::size_t>(capacity.value()), ExactStorage{})
    , value(static_cast<std::size_t>(capacity.value()), ExactStorage{})
    , interval(2.0 * windowSec / capacity.value())
    , nextEmit(0.0)
    , accMin(0.0)
    , accMax(0.0)
    , accMinTime(0.0)
    , accMaxTime(0.0)
    , cellSlots(0)
    , mode(ringMode)
  {}

  /**
   * @brief True while every appended sample keeps its own slot.
   */
  [[nodiscard]] bool exact() const noexcept { return mode == RingMode::Exact; }

  /**
   * @brief Bytes held by the sample storage.
   */
  [[nodiscard]] std::size_t bytes() const noexcept
  {
    return (time.storageCapacity() + value.storageCapacity()) * sizeof(ssfp_t);
  }

  /**
   * @brief Switches an exact ring to min/max decimation on a grid sized so the current
   *        capacity spans @p windowSec. Retained samples stay; the next append opens a cell.
   */
  void setDecimating(double windowSec)
  {
    SS_ASSERT(windowSec > 0.0, return);
    SS_ASSERT(time.capacity() > 0, return);

    mode      = RingMode::Decimating;
    interval  = 2.0 * windowSec / static_cast<double>(time.capacity());
    nextEmit  = 0.0;
    cellSlots = 0;
  }

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
  void resizeCapacity(RingCapacity capacity, double windowSec)
  {
    const int slotCount = capacity.value();
    if (!(windowSec > 0.0) || static_cast<std::size_t>(slotCount) == time.capacity())
      return;

    time.resize(static_cast<std::size_t>(slotCount));
    value.resize(static_cast<std::size_t>(slotCount));

    interval  = 2.0 * windowSec / static_cast<double>(slotCount);
    nextEmit  = 0.0;
    cellSlots = 0;
  }

  /**
   * @brief This ring's capacity, for sizing a sibling ring identically: a copy of a capacity that
   *        was already derived from a rate, never a new sample count.
   */
  [[nodiscard]] RingCapacity ringCapacity() const noexcept
  {
    return RingCapacity(static_cast<int>(time.capacity()));
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
   * @brief Appends one (time, value): its own slot in exact mode, else a per-cell min/max pair
   *        on an absolute time grid with the open cell updated in place. Sub-cell backward
   *        jitter clamps forward, but a jump back over a whole cell is a restarted clock and
   *        drops the retained span: clamping it wedges the ring shut until wall time catches up.
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

    if (mode == RingMode::Exact) {
      time.push(t);
      value.push(v);
      return;
    }

    appendChecked(t, v);
  }

  /**
   * @brief appendDecimated() past its input guards: @p t and @p v must be finite and @p t
   *        already clamped. EnvelopeRing runs those guards to place its open cell, so it
   *        enters here rather than paying them twice. The min/max update selects into
   *        registers: on a rate-sized ring, which extreme a sample replaces is a coin flip.
   */
  void appendChecked(double t, double v)
  {
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

    const bool lowers = v < accMin;
    const bool raises = v > accMax;
    accMin            = lowers ? v : accMin;
    accMinTime        = lowers ? t : accMinTime;
    accMax            = raises ? v : accMax;
    accMaxTime        = raises ? t : accMaxTime;

    if (!(lowers | raises))
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
    : cells(capacity < 1 ? 1 : capacity, ExactStorage{})
    , openIndex(0)
    , shift(shiftBits)
    , openValid(false)
  {}
};

/**
 * @brief Byte budget shared by every time-plot history ring (spec 0097 R7). Growth asks for a
 *        grant; a refused grant leaves that ring at its size, decimating. Nothing shrinks to
 *        make room. Owned by the dashboard and recomputed from the live rings on every rebuild.
 */
struct RingBudget {
  std::size_t used;
  std::size_t limit;
  bool binding;

  /**
   * @brief Constructs an empty budget capped at @p limitBytes.
   */
  explicit RingBudget(std::size_t limitBytes = std::numeric_limits<std::size_t>::max())
    : used(0), limit(limitBytes), binding(false)
  {}

  /**
   * @brief Grants up to @p wanted bytes; anything short of it marks the rail binding.
   */
  [[nodiscard]] std::size_t grant(std::size_t wanted) noexcept
  {
    const std::size_t room = used < limit ? limit - used : 0;
    if (wanted <= room)
      return wanted;

    binding = true;
    return room;
  }

  /**
   * @brief Records a ring's footprint moving from @p before to @p after bytes.
   */
  void charge(std::size_t before, std::size_t after) noexcept
  {
    if (after >= before) {
      used += after - before;
      return;
    }

    const std::size_t freed = before - after;
    used                    = used > freed ? used - freed : 0;
  }
};

/**
 * @brief Why a history ring stopped keeping every sample.
 */
enum class RingLimit {
  None,
  Ceiling,
  Rail
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

  // Coverage growth tuning (spec 0097); byte bound = level 0 + pyramid share per slot, plus slack
  static constexpr double kGrowthHeadroom        = 1.1;
  static constexpr double kMinGrowthFactor       = 1.25;
  static constexpr double kTrustedSpanFraction   = 0.1;
  static constexpr std::size_t kSlotBytesBound   = 18;
  static constexpr std::size_t kGrowthSlackBytes = 1024;

  TimeRing level0;
  std::vector<EnvelopeLevel> levels;
  RingLimit limit;
  double windowSec;
  double openNextEmit;
  double openMin;
  double openMax;
  double openMinTime;
  double openMaxTime;
  std::int64_t openCell;
  bool openCellValid;

  /**
   * @brief Constructs level 0 with `capacity` slots over `window` seconds in @p mode and
   *        derives the coarse levels from that capacity. The pyramid keeps its own open-cell
   *        accumulator on level 0's grid, so it folds the same cells whether level 0 decimates
   *        or keeps every sample (spec 0097).
   */
  explicit EnvelopeRing(RingCapacity capacity = RingCapacity::fromRate(1.0, 1.0),
                        double window         = 1.0,
                        RingMode mode         = RingMode::Decimating)
    : level0(capacity, window, mode)
    , levels()
    , limit(RingLimit::None)
    , windowSec(window)
    , openNextEmit(0.0)
    , openMin(0.0)
    , openMax(0.0)
    , openMinTime(0.0)
    , openMaxTime(0.0)
    , openCell(0)
    , openCellValid(false)
  {
    buildLevels();
  }

  /**
   * @brief True while level 0 keeps every appended sample.
   */
  [[nodiscard]] bool exact() const noexcept { return level0.exact(); }

  /**
   * @brief Samples per second level 0 currently holds: retained count over retained span.
   */
  [[nodiscard]] double retainedRateHz() const noexcept
  {
    const std::size_t n = level0.time.size();
    if (n < 2)
      return 0.0;

    const double span = level0.time[n - 1] - level0.time[0];
    return span > 0.0 ? static_cast<double>(n - 1) / span : 0.0;
  }

  /**
   * @brief Bytes held by level 0 and every coarse level.
   */
  [[nodiscard]] std::size_t bytes() const noexcept
  {
    std::size_t total = level0.bytes();
    for (const auto& level : levels)
      total += level.cells.storageCapacity() * sizeof(EnvelopeCell);

    return total;
  }

  /**
   * @brief Switches level 0 to min/max decimation at its current capacity over the ring's
   *        window, recording @p reason. The grid is the one the pyramid already folds on, so
   *        coarse levels continue.
   */
  void setDecimating(RingLimit reason)
  {
    level0.setDecimating(windowSec);
    limit = reason;
  }

  /**
   * @brief Coverage growth (spec 0097) before @p incoming samples ending at @p tNewest: grows an
   *        exact ring that would evict an in-window sample to coverageNeed() within @p budget.
   *        Never shrinks; decimates only if the grown ring still cannot hold the window's
   *        samples, recording whether the per-ring ceiling or the rail stopped it.
   */
  bool growFor(std::size_t incoming, double tNewest, RingBudget& budget)
  {
    SS_ASSERT(windowSec > 0.0, return false);
    SS_ASSERT(level0.time.capacity() > 0, return false);

    const std::size_t size = level0.time.size();
    const std::size_t cap  = level0.time.capacity();
    if (!level0.exact() || size + incoming <= cap)
      return false;

    const std::size_t evicted = size + incoming - cap;
    if (evicted <= size && level0.time[evicted - 1] < tNewest - windowSec)
      return false;

    const std::size_t total = size + incoming;
    const double density    = coverageDensity(total, tNewest);
    const double required   = density > 0.0 ? density * windowSec : static_cast<double>(total);
    const RingCapacity need = coverageNeed(total, density);
    const auto needSlots    = static_cast<std::size_t>(need.value());
    const std::size_t extra = needSlots > cap ? needSlots - cap : 0;
    const std::size_t room =
      budget.grant(extra > 0 ? extra * kSlotBytesBound + kGrowthSlackBytes : 0);
    const std::size_t usable  = room > kGrowthSlackBytes ? room - kGrowthSlackBytes : 0;
    const RingCapacity target = need.atMost(static_cast<int>(cap + usable / kSlotBytesBound));
    const bool grows          = static_cast<std::size_t>(target.value()) > cap;

    if (grows) {
      const std::size_t before = bytes();
      resizeCapacity(target, windowSec);
      budget.charge(before, bytes());
    }

    if (static_cast<double>(target.value()) < required)
      setDecimating(target.value() >= RingCapacity::kMaxSamples ? RingLimit::Ceiling
                                                                : RingLimit::Rail);

    return grows;
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
  void resizeCapacity(RingCapacity capacity, double window)
  {
    if (!(window > 0.0) || static_cast<std::size_t>(capacity.value()) == level0.time.capacity())
      return;

    windowSec = window;
    level0.resizeCapacity(capacity, window);
    buildLevels();
    rebuildLevelsFromLevel0();
  }

  /**
   * @brief Appends one (time, value): rejects non-finite input, folds the grid cell this sample
   *        closes (same boundaries as TimeRing::opensCell(), so a decimating ring folds the same
   *        cells as before), then stores it in level 0: its own slot when exact, else through
   *        appendChecked(). A restarted producer clock drops the pyramid with level 0.
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

    accumulateOpenCell(t, v);

    if (level0.mode == RingMode::Exact) {
      level0.time.push(t);
      level0.value.push(v);
      return;
    }

    level0.appendChecked(t, v);
  }

private:
  /**
   * @brief Measured samples per second of @p total samples ending at @p tNewest, or 0 while the
   *        retained span is under a tenth of the window (too short to trust).
   */
  [[nodiscard]] double coverageDensity(std::size_t total, double tNewest) const
  {
    SS_ASSERT_LOG(windowSec > 0.0);
    SS_ASSERT_LOG(total > 0);

    const double span = level0.time.size() > 0 ? tNewest - level0.time[0] : 0.0;
    return span >= windowSec * kTrustedSpanFraction ? static_cast<double>(total) / span : 0.0;
  }

  /**
   * @brief Capacity an exact ring grows to for @p total samples at @p density (0 = untrusted):
   *        the window at that density plus headroom, else double; at least one step (none for
   *        an empty ring) and the samples at hand, at most the per-ring ceiling.
   */
  [[nodiscard]] RingCapacity coverageNeed(std::size_t total, double density) const
  {
    SS_ASSERT_LOG(windowSec > 0.0);
    SS_ASSERT_LOG(density >= 0.0);

    const auto cap      = static_cast<double>(level0.time.capacity());
    const auto ceiling  = static_cast<double>(RingCapacity::kMaxSamples);
    const double factor = density > 0.0 ? kMinGrowthFactor : 2.0;
    const double step   = level0.time.size() > 0 ? cap * factor : 0.0;

    return RingCapacity::fromRate(windowSec * kGrowthHeadroom, density)
      .atLeast(static_cast<int>(std::min(step, ceiling)))
      .atLeast(static_cast<int>(std::min(static_cast<double>(total), ceiling)));
  }

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
   * @brief Opens a fresh grid cell for (t, v) when t crosses the open cell's boundary, folding
   *        the closed one into the coarse levels first; otherwise widens the open cell's
   *        extremes. Same boundary and tie rules as TimeRing::appendChecked().
   */
  void accumulateOpenCell(double t, double v)
  {
    if (!openCellValid || level0.time.size() == 0 || t >= openNextEmit) {
      if (openCellValid)
        foldOpenCell();

      openCell      = cellIndex(t);
      openNextEmit  = (std::floor(t / level0.interval) + 1.0) * level0.interval;
      openMin       = v;
      openMax       = v;
      openMinTime   = t;
      openMaxTime   = t;
      openCellValid = true;
      return;
    }

    const bool lowers = v < openMin;
    const bool raises = v > openMax;
    openMin           = lowers ? v : openMin;
    openMinTime       = lowers ? t : openMinTime;
    openMax           = raises ? v : openMax;
    openMaxTime       = raises ? t : openMaxTime;
  }

  /**
   * @brief Folds the closed grid cell (the open-cell extremes) into every coarse level.
   */
  void foldOpenCell()
  {
    SS_ASSERT(openCellValid, return);

    const bool minFirst = openMinTime <= openMaxTime;
    EnvelopeCell cell;
    cell.t0 = minFirst ? openMinTime : openMaxTime;
    cell.v0 = minFirst ? openMin : openMax;
    cell.t1 = minFirst ? openMaxTime : openMinTime;
    cell.v1 = minFirst ? openMax : openMin;
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

  RingCapacity shapeCapacity;
  int shapeCurves;

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
    , shapeCapacity(RingCapacity::fromRate(1.0, 1.0))
    , shapeCurves(0)
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
   * @brief Records the shape and allocates it: setShape() then allocate().
   */
  void configure(int curveCount, RingCapacity capacity, double window)
  {
    setShape(curveCount, capacity, window);
    allocate();
  }

  /**
   * @brief Records `curveCount` curves of `capacity` slots over `window` seconds without
   *        allocating, releasing any rings held (spec 0097 R4: a sweep that is off costs nothing).
   */
  void setShape(int curveCount, RingCapacity capacity, double window)
  {
    windowSec     = window > 0 ? window : 1.0;
    shapeCapacity = capacity;
    shapeCurves   = curveCount < 0 ? 0 : curveCount;
    release();
  }

  /**
   * @brief Allocates the recorded shape's front/back rings and retained segments, each ring
   *        constructed individually: FixedQueue copies share their backing array, so a
   *        copy-fill would alias every curve. No-op when already allocated.
   */
  void allocate()
  {
    SS_ASSERT(shapeCurves >= 0, return);
    SS_ASSERT_LOG(windowSec > 0.0);

    if (allocated())
      return;

    const auto curves = static_cast<std::size_t>(shapeCurves);
    front.reserve(curves);
    back.reserve(curves);
    for (std::size_t i = 0; i < curves; ++i) {
      front.emplace_back(shapeCapacity, windowSec);
      back.emplace_back(shapeCapacity, windowSec);
    }

    retuneRings();
    resetState();
    setSegmentRetention(segmentRetention);
  }

  /**
   * @brief Grows an allocated engine's rings to @p capacity in place (TimeRing::resizeCapacity
   *        keeps the newest samples), so a capture in progress, a frozen single-shot trace and
   *        the trigger state survive. Never shrinks. The retained-segment store is re-sized
   *        under its memory bound, so it restarts: a smaller slot would truncate new captures.
   */
  void growCapacity(RingCapacity capacity)
  {
    SS_ASSERT_LOG(windowSec > 0.0);
    if (!allocated() || capacity.value() <= shapeCapacity.value())
      return;

    shapeCapacity = capacity;
    for (auto& r : front)
      r.resizeCapacity(capacity, activeWindow());

    for (auto& r : back)
      r.resizeCapacity(capacity, activeWindow());

    setSegmentRetention(segmentRetention);
  }

  /**
   * @brief Frees every ring and retained segment; the trigger configuration, the shape and the
   *        requested retention stay, so allocate() restores the same engine.
   */
  void release()
  {
    SS_ASSERT_LOG(front.size() == back.size());
    front.clear();
    front.shrink_to_fit();
    back.clear();
    back.shrink_to_fit();
    segments.clear();
    segments.shrink_to_fit();
    resetState();
  }

  /**
   * @brief True while the rings for the recorded shape exist.
   */
  [[nodiscard]] bool allocated() const noexcept
  {
    return shapeCurves > 0 && front.size() == static_cast<std::size_t>(shapeCurves);
  }

  /**
   * @brief Bytes held by the front/back rings and every retained segment.
   */
  [[nodiscard]] std::size_t bytes() const noexcept
  {
    std::size_t total = 0;
    for (const auto& ring : front)
      total += ring.bytes();

    for (const auto& ring : back)
      total += ring.bytes();

    for (const auto& segment : segments)
      for (const auto& ring : segment.curves)
        total += ring.bytes();

    return total;
  }

  /**
   * @brief Requests retention of the last @p count completed sweeps (0 = off). The effective
   *        count is clamped so `count * curves * capacity * 16 B` stays under kMaxSegmentBytes;
   *        segments are pre-sized here so completeSweep() never allocates. An unallocated
   *        engine only records the request.
   */
  void setSegmentRetention(int count)
  {
    segmentRetention = std::clamp(count, 0, kMaxSegments);
    if (front.empty()) {
      segments.clear();
      segmentHead = 0;
      segmentFill = 0;
      return;
    }

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
        segment.curves.emplace_back(front.front().ringCapacity(), windowSec);

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
