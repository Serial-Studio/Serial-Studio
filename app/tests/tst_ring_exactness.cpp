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

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <QTest>
#include <vector>

#include "DSP.h"

/**
 * @file tst_ring_exactness.cpp
 * @brief Exact retention contract of the time-plot history rings (spec 0097, AC1/AC2/AC5).
 *
 * A ring in exact mode keeps every appended sample in its own slot with its exact time and
 * value: no grid snapping, no collapse of equal neighbours, no loss under timestamp jitter. The
 * decimating mode it falls back to must stay today's min/max grid. Comparisons are exact: the
 * rings move doubles, they never compute with them.
 */

namespace {

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

/**
 * @brief One appended sample, kept so a test can compare the ring against its input.
 */
struct Sample {
  double t;
  double v;
};

/**
 * @brief Deterministic jitter in [-amplitude, +amplitude] periods, without an RNG dependency.
 */
[[nodiscard]] double jitter(std::int64_t i, double amplitude)
{
  const double phase = static_cast<double>((i * 7919) % 1000) / 1000.0;
  return (2.0 * phase - 1.0) * amplitude;
}

/**
 * @brief Builds @p count samples at @p rateHz with optional jitter and runs of equal values,
 *        so equal neighbours and uneven spacing both occur. Times stay strictly increasing.
 */
[[nodiscard]] std::vector<Sample> makeSamples(std::int64_t count,
                                              double rateHz,
                                              double jitterPeriods)
{
  std::vector<Sample> out;
  out.reserve(static_cast<std::size_t>(count));

  const double period = 1.0 / rateHz;
  double last         = -1.0;
  for (std::int64_t i = 0; i < count; ++i) {
    double t = 10.0 + static_cast<double>(i) * period + jitter(i, jitterPeriods) * period;
    if (t <= last)
      t = std::nextafter(last, 1e300);

    const double v = (i % 4 < 2) ? 1.5 : static_cast<double>(i % 11) - 5.0;
    out.push_back({t, v});
    last = t;
  }

  return out;
}

/**
 * @brief True when the ring's retained slots equal the last ring.size() input samples exactly.
 */
[[nodiscard]] bool retainsTail(const DSP::TimeRing& ring, const std::vector<Sample>& input)
{
  const std::size_t held = ring.time.size();
  if (held != ring.value.size() || held > input.size())
    return false;

  const std::size_t first = input.size() - held;
  for (std::size_t k = 0; k < held; ++k)
    if (ring.time[k] != input[first + k].t || ring.value[k] != input[first + k].v)
      return false;

  return true;
}

/**
 * @brief True when the envelope ring's level 0 equals the last level0.size() input samples.
 */
[[nodiscard]] bool retainsTail(const DSP::EnvelopeRing& ring, const std::vector<Sample>& input)
{
  return retainsTail(ring.level0, input);
}

/**
 * @brief Brute-force check of every coarse cell against the extremes of the logged samples in
 *        its grid range; the newest cell of each level is skipped because it may still be open.
 *        Returns the number of cells checked, or -1 on the first mismatch.
 */
[[nodiscard]] int checkLevels(const DSP::EnvelopeRing& ring, const std::vector<Sample>& log)
{
  int checked = 0;
  for (const auto& level : ring.levels) {
    for (std::size_t c = 0; c + 1 < level.cells.size(); ++c) {
      const auto& cell         = level.cells[c];
      const std::int64_t index = ring.cellIndex(cell.t0) >> level.shift;

      double lo = std::numeric_limits<double>::max();
      double hi = std::numeric_limits<double>::lowest();
      for (const auto& s : log) {
        if ((ring.cellIndex(s.t) >> level.shift) != index)
          continue;

        lo = std::min(lo, s.v);
        hi = std::max(hi, s.v);
      }

      if (std::min(cell.v0, cell.v1) != lo || std::max(cell.v0, cell.v1) != hi)
        return -1;

      ++checked;
    }
  }

  return checked;
}

/**
 * @brief Feeds @p input to @p ring in blocks of @p block samples, asking for coverage growth
 *        before each block exactly as the dashboard ingest does.
 */
void feedBlocks(DSP::EnvelopeRing& ring,
                const std::vector<Sample>& input,
                std::size_t block,
                DSP::RingBudget& budget)
{
  for (std::size_t i = 0; i < input.size(); i += block) {
    const std::size_t n = std::min(block, input.size() - i);
    if (ring.level0.time.size() + n > ring.level0.time.capacity())
      (void)ring.growFor(n, input[i + n - 1].t, budget);

    for (std::size_t k = 0; k < n; ++k)
      ring.appendDecimated(input[i + k].t, input[i + k].v);
  }
}

/**
 * @brief Number of logged samples inside the last @p windowSec seconds of @p input.
 */
[[nodiscard]] std::size_t samplesInWindow(const std::vector<Sample>& input, double windowSec)
{
  const double cutoff = input.back().t - windowSec;
  std::size_t n       = 0;
  for (auto it = input.rbegin(); it != input.rend() && it->t >= cutoff; ++it)
    ++n;

  return n;
}

}  // namespace

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class RingExactnessTest : public QObject {
  Q_OBJECT

private slots:
  void exactRingKeepsEverySample();
  void exactRingKeepsEqualNeighboursAndJitter();
  void exactRingWrapsToTheNewestSamples();
  void exactRingClampsSubCellJitterAndDropsARestartedClock();
  void exactStorageMatchesCapacity();
  void decimatingFallbackKeepsTheGrid();
  void exactPyramidCarriesTheGridExtremes();
  void coverageGrowthKeepsTheWindowExact_data();
  void coverageGrowthKeepsTheWindowExact();
  void rateStepUpGrowsWithoutLoss();
  void rateStepDownNeverShrinks();
  void railRefusalDecimatesAndFlags();
  void ceilingDecimatesAndFlags();
};

/**
 * @brief Every appended sample lands in its own slot with its own time and value.
 */
void RingExactnessTest::exactRingKeepsEverySample()
{
  const auto capacity = DSP::RingCapacity::fromRate(12.5, 1000.0);
  DSP::TimeRing ring(capacity, 12.5, DSP::RingMode::Exact);
  QVERIFY(ring.exact());

  const auto input = makeSamples(10000, 1000.0, 0.0);
  for (const auto& s : input)
    ring.appendDecimated(s.t, s.v);

  QCOMPARE(ring.time.size(), input.size());
  QVERIFY(retainsTail(ring, input));
}

/**
 * @brief Equal adjacent values and +-20% timestamp jitter cost no sample: the grid-based mode
 *        collapses both, the exact mode keeps both.
 */
void RingExactnessTest::exactRingKeepsEqualNeighboursAndJitter()
{
  const auto capacity = DSP::RingCapacity::fromRate(12.5, 1000.0);
  DSP::TimeRing ring(capacity, 12.5, DSP::RingMode::Exact);

  const auto input = makeSamples(12000, 1000.0, 0.2);
  for (const auto& s : input)
    ring.appendDecimated(s.t, s.v);

  QCOMPARE(ring.time.size(), input.size());
  QVERIFY(retainsTail(ring, input));
}

/**
 * @brief Past capacity the ring keeps exactly the newest capacity() samples.
 */
void RingExactnessTest::exactRingWrapsToTheNewestSamples()
{
  const auto capacity = DSP::RingCapacity::fromRate(1.0, 1000.0);
  DSP::TimeRing ring(capacity, 1.0, DSP::RingMode::Exact);

  const auto input = makeSamples(3500, 1000.0, 0.1);
  for (const auto& s : input)
    ring.appendDecimated(s.t, s.v);

  QCOMPARE(ring.time.size(), ring.time.capacity());
  QVERIFY(retainsTail(ring, input));
}

/**
 * @brief A sub-cell step back is clamped forward; a jump back over a whole cell is a restarted
 *        clock and drops the retained span, as in decimating mode.
 */
void RingExactnessTest::exactRingClampsSubCellJitterAndDropsARestartedClock()
{
  const auto capacity = DSP::RingCapacity::fromRate(1.0, 1000.0);
  DSP::TimeRing ring(capacity, 1.0, DSP::RingMode::Exact);

  ring.appendDecimated(5.000, 1.0);
  ring.appendDecimated(5.001, 2.0);
  ring.appendDecimated(5.0005, 3.0);
  QCOMPARE(ring.time.size(), std::size_t(3));
  QCOMPARE(ring.time[2], 5.001);
  QCOMPARE(ring.value[2], 3.0);

  ring.appendDecimated(1.0, 4.0);
  QCOMPARE(ring.time.size(), std::size_t(1));
  QCOMPARE(ring.time[0], 1.0);
  QCOMPARE(ring.value[0], 4.0);
}

/**
 * @brief Time-ring storage is exactly the capacity: no power-of-two rounding (R5), and the
 *        byte count reports what is held.
 */
void RingExactnessTest::exactStorageMatchesCapacity()
{
  const auto capacity = DSP::RingCapacity::fromRate(12.5, 48000.0);
  DSP::TimeRing ring(capacity, 12.5, DSP::RingMode::Exact);

  QCOMPARE(ring.time.capacity(), std::size_t(600000));
  QCOMPARE(ring.time.storageCapacity(), std::size_t(600000));
  QVERIFY(ring.time.exactStorage());
  QCOMPARE(ring.bytes(), std::size_t(600000) * 2 * sizeof(double));
}

/**
 * @brief setDecimating() leaves the exact samples in place and grids everything after it, so
 *        a saturated source spans the window at the ring's capacity again.
 */
void RingExactnessTest::decimatingFallbackKeepsTheGrid()
{
  const auto capacity = DSP::RingCapacity::fromRate(1.0, 100.0);
  DSP::TimeRing ring(capacity, 1.0, DSP::RingMode::Exact);

  ring.appendDecimated(0.000, 1.0);
  ring.appendDecimated(0.001, 2.0);
  ring.setDecimating(1.0);
  QVERIFY(!ring.exact());
  QCOMPARE(ring.time.size(), std::size_t(2));

  const double cell  = ring.interval;
  const double start = 1.0 + cell * 0.25;
  for (int i = 0; i < 50; ++i)
    ring.appendDecimated(start + static_cast<double>(i) * cell / 100.0, static_cast<double>(i % 7));

  QCOMPARE(ring.time.size(), std::size_t(4));
}

/**
 * @brief An exact level 0 no longer runs the grid, so the pyramid folds its own accumulator:
 *        every coarse cell must still carry the extremes of its grid range, through a wrap.
 */
void RingExactnessTest::exactPyramidCarriesTheGridExtremes()
{
  const auto capacity = DSP::RingCapacity::fromRate(1.0, 4096.0);
  DSP::EnvelopeRing ring(capacity, 1.0, DSP::RingMode::Exact);

  const auto input = makeSamples(20000, 4096.0, 0.2);
  for (const auto& s : input)
    ring.appendDecimated(s.t, s.v);

  QVERIFY(ring.exact());
  QVERIFY(retainsTail(ring, input));
  QVERIFY(ring.coarseLevelCount() >= 2);
  QVERIFY(checkLevels(ring, input) > 0);
}

/**
 * @brief AC1: across rates, windows and both lanes' starting sizes, a ring fed a full window
 *        in blocks retains every sample of the window exactly while rate x window fits under
 *        the ceiling, and holds at most 1.25x the bytes that window needs (or the small
 *        starting floor). Past the ceiling it decimates and says so.
 */
void RingExactnessTest::coverageGrowthKeepsTheWindowExact_data()
{
  QTest::addColumn<double>("rateHz");
  QTest::addColumn<double>("windowSec");
  QTest::addColumn<bool>("streamLane");

  const double rates[]   = {10.0, 1000.0, 10000.0, 48000.0, 192000.0};
  const double windows[] = {1.0, 10.0, 300.0};
  for (const double rate : rates) {
    for (const double window : windows) {
      for (const bool stream : {false, true}) {
        const auto name = QStringLiteral("%1Hz-%2s-%3")
                            .arg(rate)
                            .arg(window)
                            .arg(stream ? QStringLiteral("stream") : QStringLiteral("frame"));
        QTest::newRow(qPrintable(name)) << rate << window << stream;
      }
    }
  }
}

void RingExactnessTest::coverageGrowthKeepsTheWindowExact()
{
  QFETCH(double, rateHz);
  QFETCH(double, windowSec);
  QFETCH(bool, streamLane);

  const double history  = windowSec * 1.25;
  const double need     = rateHz * history;
  const double ceiling  = static_cast<double>(DSP::RingCapacity::kMaxSamples);
  const bool exceeds    = need > ceiling;
  const double headroom = DSP::EnvelopeRing::kGrowthHeadroom;
  const auto start      = streamLane ? DSP::RingCapacity::fromRate(history * headroom, rateHz)
                                     : DSP::RingCapacity::fromRate(1.0, 1024.0);
  const auto mode       = streamLane && exceeds ? DSP::RingMode::Decimating : DSP::RingMode::Exact;
  const auto total      = static_cast<std::int64_t>(std::min(need, ceiling * 1.5));
  const std::size_t blk = streamLane ? 4096 : 64;

  DSP::EnvelopeRing ring(start, history, mode);
  DSP::RingBudget budget;
  budget.used      = ring.bytes();
  const auto input = makeSamples(total, rateHz, streamLane ? 0.0 : 0.2);
  feedBlocks(ring, input, blk, budget);

  if (exceeds) {
    QVERIFY(!ring.exact());
    return;
  }

  QVERIFY(ring.exact());
  QVERIFY(ring.limit == DSP::RingLimit::None);
  QVERIFY(retainsTail(ring, input));
  QVERIFY(ring.level0.time.size() >= samplesInWindow(input, history * 0.999));

  const double needBytes  = static_cast<double>(samplesInWindow(input, history)) * 16.0;
  const double floorBytes = 1024.0 * 18.0;
  QVERIFY2(static_cast<double>(ring.bytes()) <= std::max(needBytes * 1.25, floorBytes),
           qPrintable(QStringLiteral("bytes %1, need %2").arg(ring.bytes()).arg(needBytes)));
}

/**
 * @brief AC2: a source that speeds up mid-run grows its ring with no sample lost.
 */
void RingExactnessTest::rateStepUpGrowsWithoutLoss()
{
  const double history = 12.5;
  DSP::EnvelopeRing ring(
    DSP::RingCapacity::fromRate(history, 1000.0), history, DSP::RingMode::Exact);
  DSP::RingBudget budget;

  auto slow       = makeSamples(15000, 1000.0, 0.1);
  const double t0 = slow.back().t;
  auto fast       = makeSamples(150000, 10000.0, 0.1);
  for (auto& s : fast)
    s.t += t0;

  std::vector<Sample> input = slow;
  input.insert(input.end(), fast.begin(), fast.end());
  const std::size_t before = ring.level0.time.capacity();
  feedBlocks(ring, input, 64, budget);

  QVERIFY(ring.exact());
  QVERIFY(ring.level0.time.capacity() > before);
  QVERIFY(retainsTail(ring, input));
  QVERIFY(ring.level0.time.size() >= samplesInWindow(input, history * 0.999));
}

/**
 * @brief AC2: a source that slows down keeps its ring; nothing shrinks mid-session. The fast phase
 *        may grow the ring (a sample exactly on the window edge counts as in-window), so the
 *        capacity is pinned after it and must hold through the slow phase.
 */
void RingExactnessTest::rateStepDownNeverShrinks()
{
  const double history = 12.5;
  DSP::EnvelopeRing ring(
    DSP::RingCapacity::fromRate(history, 10000.0), history, DSP::RingMode::Exact);
  DSP::RingBudget budget;

  auto fast       = makeSamples(150000, 10000.0, 0.0);
  const double t0 = fast.back().t;
  auto slow       = makeSamples(15000, 1000.0, 0.0);
  for (auto& s : slow)
    s.t += t0;

  std::vector<Sample> input = fast;
  input.insert(input.end(), slow.begin(), slow.end());

  const std::size_t initial = ring.level0.time.capacity();
  feedBlocks(ring, fast, 64, budget);
  const std::size_t afterFast = ring.level0.time.capacity();
  QVERIFY(afterFast >= initial);

  feedBlocks(ring, slow, 64, budget);
  QVERIFY(ring.exact());
  QCOMPARE(ring.level0.time.capacity(), afterFast);
  QVERIFY(retainsTail(ring, input));
}

/**
 * @brief AC5: a growth the rail cannot grant leaves the ring at its size, decimating, with the
 *        reason recorded and the budget marked binding; the samples it held stay.
 */
void RingExactnessTest::railRefusalDecimatesAndFlags()
{
  const double history = 12.5;
  DSP::EnvelopeRing ring(
    DSP::RingCapacity::fromRate(history, 100.0), history, DSP::RingMode::Exact);
  DSP::RingBudget budget(ring.bytes() + 4096);
  budget.used = ring.bytes();

  const auto input = makeSamples(200000, 48000.0, 0.0);
  feedBlocks(ring, input, 4096, budget);

  QVERIFY(!ring.exact());
  QVERIFY(ring.limit == DSP::RingLimit::Rail);
  QVERIFY(budget.binding);
  QVERIFY(budget.used <= budget.limit);
  QVERIFY(ring.level0.time.size() > 0);
}

/**
 * @brief Past the per-ring ceiling the ring decimates, flags the ceiling, and still spans the
 *        window.
 */
void RingExactnessTest::ceilingDecimatesAndFlags()
{
  const double history = 30.0;
  DSP::EnvelopeRing ring(
    DSP::RingCapacity::fromRate(history, 100.0), history, DSP::RingMode::Exact);
  DSP::RingBudget budget;

  const auto input = makeSamples(9000000, 192000.0, 0.0);
  feedBlocks(ring, input, 4096, budget);

  QVERIFY(!ring.exact());
  QVERIFY(ring.limit == DSP::RingLimit::Ceiling);
  QVERIFY(ring.level0.time.capacity() <= std::size_t(DSP::RingCapacity::kMaxSamples));
  QVERIFY(ring.level0.time[0] <= input.back().t - history * 0.9);
}

QTEST_APPLESS_MAIN(RingExactnessTest)

#include "tst_ring_exactness.moc"
