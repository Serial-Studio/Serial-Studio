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

#include <cmath>
#include <cstdint>
#include <limits>
#include <QList>
#include <QPointF>
#include <QTest>
#include <utility>
#include <vector>

#include "DSP.h"

/**
 * @file tst_envelope_ring.cpp
 * @brief Level consistency contract of DSP::EnvelopeRing (spec 0057, AC1-AC4).
 *
 * Every coarse cell must carry exactly the min and max of the level-0 grid range it covers, at
 * every level, before and after the rings wrap; level selection must follow seconds-per-pixel and
 * fall back to level 0 when a coarse level cannot cover the requested span; non-finite input must
 * be rejected before it touches any level. Comparisons are exact: the pyramid moves doubles, it
 * never computes with them.
 */

namespace {

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

/**
 * @brief One appended sample, kept so a test can brute-force any grid range afterwards.
 */
struct Sample {
  double t;
  double v;
};

/**
 * @brief Deterministic value that both drifts and oscillates, so a cell's min and max land on
 *        different samples and every coarse cell's extremes differ from its neighbours'.
 */
[[nodiscard]] double waveform(std::int64_t i)
{
  const double saw = static_cast<double>(i % 37) - 18.0;
  const double alt = (i % 5 == 0) ? 40.0 : 0.0;
  return saw + alt * ((i / 5) % 2 == 0 ? 1.0 : -1.0) + static_cast<double>(i) * 1e-3;
}

/**
 * @brief Appends @p cells worth of level-0 cells, @p perCell samples each, at a fixed phase inside
 *        the ring's grid, and records every sample.
 */
void feedCells(DSP::EnvelopeRing& ring,
               std::int64_t firstCell,
               std::int64_t cells,
               int perCell,
               std::vector<Sample>& log)
{
  const double interval = ring.level0.interval;
  for (std::int64_t c = 0; c < cells; ++c) {
    for (int s = 0; s < perCell; ++s) {
      const std::int64_t i = c * perCell + s;
      const double t       = (static_cast<double>(firstCell + c) + (s + 0.5) / perCell) * interval;
      const double v       = waveform(i);
      ring.appendDecimated(t, v);
      log.push_back({t, v});
    }
  }
}

/**
 * @brief Brute-force min/max over every logged sample whose level-0 index falls inside the
 *        half-open range [@p first, @p last); false when the range holds no sample.
 */
[[nodiscard]] bool bruteForce(const DSP::EnvelopeRing& ring,
                              const std::vector<Sample>& log,
                              std::int64_t first,
                              std::int64_t last,
                              double& outMin,
                              double& outMax)
{
  bool any = false;
  outMin   = std::numeric_limits<double>::max();
  outMax   = std::numeric_limits<double>::lowest();
  for (const auto& s : log) {
    const std::int64_t idx = ring.cellIndex(s.t);
    if (idx < first || idx >= last)
      continue;

    any    = true;
    outMin = std::min(outMin, s.v);
    outMax = std::max(outMax, s.v);
  }

  return any;
}

/**
 * @brief Checks every cell of every coarse level against the brute-force extremes of the level-0
 *        range it covers, skipping cells that cover @p openIndex0 (the level-0 cell still open,
 *        which by contract has not folded yet). Returns the number of cells checked.
 */
[[nodiscard]] int checkLevels(const DSP::EnvelopeRing& ring,
                              const std::vector<Sample>& log,
                              std::int64_t openIndex0)
{
  int checked = 0;
  for (std::size_t k = 0; k < ring.levels.size(); ++k) {
    const auto& level = ring.levels[k];
    double prevT      = std::numeric_limits<double>::lowest();
    for (std::size_t c = 0; c < level.cells.size(); ++c) {
      const auto& cell         = level.cells[c];
      const std::int64_t index = ring.cellIndex(cell.t0) >> level.shift;
      const std::int64_t first = index << level.shift;
      const std::int64_t last  = (index + 1) << level.shift;

      if (!(cell.t0 <= cell.t1))
        return -1;

      if (cell.t0 < prevT)
        return -1;

      prevT = cell.t1;
      if (openIndex0 >= first && openIndex0 < last)
        continue;

      double bMin = 0;
      double bMax = 0;
      if (!bruteForce(ring, log, first, last, bMin, bMax))
        return -1;

      if (std::min(cell.v0, cell.v1) != bMin || std::max(cell.v0, cell.v1) != bMax)
        return -1;

      ++checked;
    }
  }

  return checked;
}

}  // namespace

/**
 * @brief Level consistency, selection and input rejection of DSP::EnvelopeRing.
 */
class TstEnvelopeRing : public QObject {
  Q_OBJECT

private slots:
  void sizing();
  void rampMatchesBruteForce();
  void wrapKeepsLevelsConsistent();
  void resizeRebuildsLevels();
  void levelSelection();
  void coverageFallback();
  void rejectsNonFinite();
  void downsampleReadsCoarseLevel();
};

/**
 * @brief Coarse levels follow ceil(cells0 / 16^k) + 1 while at least three cells remain, and the
 *        byte total stays near 1/15 of level 0 (the +1 open cell per level pushes it to 1/14).
 */
void TstEnvelopeRing::sizing()
{
  const DSP::EnvelopeRing tiny(1, 1.0);
  QCOMPARE(tiny.coarseLevelCount(), 0);

  const DSP::EnvelopeRing small(1024, 1.0);
  QCOMPARE(small.coarseLevelCount(), 2);
  QCOMPARE(small.levels[0].cells.capacity(), std::size_t(33));
  QCOMPARE(small.levels[1].cells.capacity(), std::size_t(3));

  const DSP::EnvelopeRing large(262144, 10.0);
  QCOMPARE(large.coarseLevelCount(), 4);
  QCOMPARE(large.levels[0].cells.capacity(), std::size_t(8193));
  QCOMPARE(large.levels[3].cells.capacity(), std::size_t(3));

  const std::size_t level0Bytes = 262144 * 2 * sizeof(double);
  std::size_t coarseBytes       = 0;
  for (const auto& level : large.levels)
    coarseBytes += level.cells.capacity() * sizeof(DSP::EnvelopeCell);

  QVERIFY(coarseBytes * 14 <= level0Bytes);
  QCOMPARE(large.levelSpanSec(1), large.level0.interval * 16.0);
  QCOMPARE(large.levelSpanSec(3), large.level0.interval * 4096.0);
}

/**
 * @brief AC1: on an unwrapped ring every closed coarse cell holds the brute-force min/max of the
 *        level-0 grid range it covers, at every level.
 */
void TstEnvelopeRing::rampMatchesBruteForce()
{
  DSP::EnvelopeRing ring(4096, 1.0);
  QCOMPARE(ring.coarseLevelCount(), 2);

  std::vector<Sample> log;
  feedCells(ring, 0, 1500, 5, log);
  QCOMPARE(ring.level0.time.size(), std::size_t(3000));

  const int checked = checkLevels(ring, log, ring.openCell);
  QVERIFY(checked > 0);
  QCOMPARE(ring.levels[0].cells.size(), std::size_t(94));
  QCOMPARE(ring.levels[1].cells.size(), std::size_t(6));

  const std::int64_t sentinel = 4096LL * 64;
  ring.appendDecimated(static_cast<double>(sentinel) * ring.level0.interval, 0.0);
  log.push_back({static_cast<double>(sentinel) * ring.level0.interval, 0.0});
  QCOMPARE(checkLevels(ring, log, sentinel), 100);
}

/**
 * @brief AC3: after level 0 and both coarse levels wrapped several times, every closed coarse cell
 *        still matches the brute-force extremes of its range, cells stay time-ordered, and the
 *        coarsest retained history is never shorter than level 0's.
 */
void TstEnvelopeRing::wrapKeepsLevelsConsistent()
{
  DSP::EnvelopeRing ring(1024, 1.0);
  QCOMPARE(ring.coarseLevelCount(), 2);

  std::vector<Sample> log;
  feedCells(ring, 0, 5000, 3, log);
  QVERIFY(ring.level0.time.full());
  QVERIFY(ring.levels[0].cells.full());
  QVERIFY(ring.levels[1].cells.full());

  QVERIFY(checkLevels(ring, log, ring.openCell) > 0);

  const std::int64_t oldest0 = ring.cellIndex(ring.level0.time[0]);
  for (const auto& level : ring.levels) {
    const std::int64_t front = ring.cellIndex(level.cells.front().t0) >> level.shift;
    QVERIFY((front << level.shift) <= oldest0);
  }
}

/**
 * @brief Growing level 0 rebuilds the coarse levels from what it retained; the rebuilt cells match
 *        the brute-force extremes of the retained slots and later appends keep folding correctly.
 */
void TstEnvelopeRing::resizeRebuildsLevels()
{
  DSP::EnvelopeRing ring(1024, 1.0);
  std::vector<Sample> log;
  feedCells(ring, 0, 700, 3, log);

  ring.resizeCapacity(4096, 1.0);
  QCOMPARE(ring.level0.time.capacity(), std::size_t(4096));
  QCOMPARE(ring.coarseLevelCount(), 2);
  QVERIFY(!ring.openCellValid);

  std::vector<Sample> retained;
  for (std::size_t k = 0; k < ring.level0.time.size(); ++k)
    retained.push_back({ring.level0.time[k], ring.level0.value[k]});

  QVERIFY(checkLevels(ring, retained, std::numeric_limits<std::int64_t>::min()) > 0);

  const std::int64_t nextCell = ring.cellIndex(ring.level0.time[ring.level0.time.size() - 1]) + 1;
  feedCells(ring, nextCell, 200, 5, retained);
  QVERIFY(ring.openCellValid);
  QVERIFY(checkLevels(ring, retained, ring.openCell) > 0);
}

/**
 * @brief AC2: the chosen level is the coarsest whose cell span fits under one pixel of time, and
 *        narrow windows fall back to level 0.
 */
void TstEnvelopeRing::levelSelection()
{
  DSP::EnvelopeRing ring(4096, 1.0);
  std::vector<Sample> log;
  feedCells(ring, 0, 1500, 5, log);

  const double interval = ring.level0.interval;
  const double oldest   = ring.level0.time[0];

  QCOMPARE(ring.selectLevel(100.0 * interval * 8.0, 100, oldest), 0);
  QCOMPARE(ring.selectLevel(100.0 * interval * 16.0, 100, oldest), 1);
  QCOMPARE(ring.selectLevel(100.0 * interval * 255.0, 100, oldest), 1);
  QCOMPARE(ring.selectLevel(100.0 * interval * 256.0, 100, oldest), 2);
  QCOMPARE(ring.selectLevel(100.0 * interval * 100000.0, 100, oldest), 2);

  QCOMPARE(ring.selectLevel(0.0, 100, oldest), 0);
  QCOMPARE(ring.selectLevel(1.0, 0, oldest), 0);
  QCOMPARE(ring.selectLevel(1.0, 100, std::numeric_limits<double>::quiet_NaN()), 0);

  const DSP::EnvelopeRing empty(4096, 1.0);
  QCOMPARE(empty.selectLevel(1.0, 10, 0.0), 0);
}

/**
 * @brief AC2: a coarse level whose oldest cell starts after the requested oldest time cannot
 *        serve the window, so selection falls to a finer level even when the pixel budget allows.
 */
void TstEnvelopeRing::coverageFallback()
{
  DSP::EnvelopeRing ring(4096, 1.0);
  std::vector<Sample> log;
  feedCells(ring, 0, 3000, 5, log);
  QVERIFY(ring.levels[0].cells.full());

  const double interval = ring.level0.interval;
  const double wide     = 100.0 * interval * 16.0;

  QCOMPARE(ring.selectLevel(wide, 100, ring.level0.time[0]), 1);

  const std::int64_t front1 = ring.cellIndex(ring.levels[0].cells.front().t0) >> 4;
  const double tooOld       = static_cast<double>((front1 - 1) << 4) * interval;
  QCOMPARE(ring.selectLevel(wide, 100, tooOld), 0);
}

/**
 * @brief AC4: NaN and infinite times or values leave level 0, every coarse level and the open-cell
 *        bookkeeping untouched.
 */
void TstEnvelopeRing::rejectsNonFinite()
{
  DSP::EnvelopeRing ring(1024, 1.0);
  std::vector<Sample> log;
  feedCells(ring, 0, 100, 4, log);

  const std::size_t n0    = ring.level0.time.size();
  const std::size_t n1    = ring.levels[0].cells.size();
  const std::size_t n2    = ring.levels[1].cells.size();
  const std::int64_t open = ring.openCell;
  const double nan        = std::numeric_limits<double>::quiet_NaN();
  const double inf        = std::numeric_limits<double>::infinity();
  const double t          = ring.level0.time[n0 - 1] + 100.0;
  const double values[]   = {nan, inf, -inf};
  for (const double bad : values) {
    ring.appendDecimated(bad, 1.0);
    ring.appendDecimated(t, bad);
  }

  QCOMPARE(ring.level0.time.size(), n0);
  QCOMPARE(ring.levels[0].cells.size(), n1);
  QCOMPARE(ring.levels[1].cells.size(), n2);
  QCOMPARE(ring.openCell, open);
  QVERIFY(ring.openCellValid);
}

/**
 * @brief The pyramid overload of downsampleTimeWindow reads level 0 for a narrow window
 *        (identical output to the plain overload) and a coarse level for a wide one, with every
 *        emitted point inside the requested window and rebased to the newest sample.
 */
void TstEnvelopeRing::downsampleReadsCoarseLevel()
{
  DSP::EnvelopeRing ring(4096, 1.0);
  std::vector<Sample> log;
  feedCells(ring, 0, 1500, 5, log);

  const double interval = ring.level0.interval;
  DSP::DownsampleWorkspace ws;
  QList<QPointF> viaRing;
  QList<QPointF> viaLevel0;

  const double narrow = 50.0 * interval;
  QVERIFY(DSP::downsampleTimeWindow(ring, -narrow, 0.0, 100, 100, viaRing, &ws));
  QVERIFY(DSP::downsampleTimeWindow(
    ring.level0.time, ring.level0.value, -narrow, 0.0, 100, 100, viaLevel0, &ws));
  QCOMPARE(viaRing, viaLevel0);

  const double wide = 1400.0 * interval;
  QCOMPARE(ring.selectLevel(wide, 50, ring.level0.time[0]), 1);
  QVERIFY(DSP::downsampleTimeWindow(ring, -wide, 0.0, 50, 100, viaRing, &ws));
  QVERIFY(!viaRing.isEmpty());
  QVERIFY(viaRing.size() <= 50 * 4);
  for (const auto& p : viaRing) {
    QVERIFY(p.x() >= -wide);
    QVERIFY(p.x() <= 0.0);
    QVERIFY(std::isfinite(p.y()));
  }
}

QTEST_APPLESS_MAIN(TstEnvelopeRing)

#include "tst_envelope_ring.moc"
