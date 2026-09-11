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

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <limits>
#include <QElapsedTimer>
#include <QPointF>
#include <QTest>
#include <vector>

#include "Core/DSPSimd.h"
#include "Core/SimdLevel.h"
#include "dsp_scalar_ref.h"

/**
 * @file tst_dsp_reduction_bench.cpp
 * @brief Spec 0082 R3: the single-chain versus two-chain cost of every reduction lane helper,
 *        measured on the machine running the test and printed, never gated. The only assertion is
 *        that both chain counts stay bit-identical to the scalar oracle, so a "faster" number can
 *        never come from a changed result. The lane helpers are called directly through their
 *        chain template so the baseline is the same source as production, not a copy.
 */

namespace {

constexpr std::size_t kElements  = 65536;
constexpr int kRepetitions       = 200;
constexpr double kSeedOffset     = -37.25;
constexpr double kRampStep       = 1.5;
constexpr double kBenchTolerance = 0.0;

/**
 * @brief Ramp payload long enough that the chains dominate the tail.
 */
[[nodiscard]] std::vector<double> makeRamp()
{
  std::vector<double> out(kElements, 0.0);
  for (std::size_t i = 0; i < kElements; ++i)
    out[i] = kSeedOffset + kRampStep * static_cast<double>(i % 4099);

  return out;
}

/**
 * @brief Point payload sharing the ramp in both lanes with a few non-finite entries.
 */
[[nodiscard]] std::vector<QPointF> makePoints()
{
  const std::vector<double> ramp = makeRamp();
  std::vector<QPointF> out(kElements, QPointF());
  for (std::size_t i = 0; i < kElements; ++i) {
    const bool poison = (i % 997 == 13);
    const double v    = poison ? std::numeric_limits<double>::infinity() : ramp[i];
    out[i]            = QPointF(v, -ramp[i]);
  }

  return out;
}

[[nodiscard]] bool bitEqual(double a, double b)
{
  return std::bit_cast<quint64>(a) == std::bit_cast<quint64>(b);
}

/**
 * @brief Median of @p samples in nanoseconds; sorts a copy.
 */
[[nodiscard]] double medianNs(std::vector<qint64> samples)
{
  std::sort(samples.begin(), samples.end());
  return static_cast<double>(samples[samples.size() / 2]);
}

/**
 * @brief Times @p body kRepetitions times and returns the median nanoseconds per element.
 */
template<typename Body>
[[nodiscard]] double timePerElement(Body&& body)
{
  std::vector<qint64> samples;
  samples.reserve(kRepetitions);
  QElapsedTimer timer;
  for (int r = 0; r < kRepetitions; ++r) {
    timer.start();
    body();
    samples.push_back(timer.nsecsElapsed());
  }

  return medianNs(samples) / static_cast<double>(kElements);
}

/**
 * @brief Finishes a lane helper's partial minimum with the public kernel's scalar tail.
 */
[[nodiscard]] double finishMin(const double* p, std::size_t n, std::size_t i, double lo)
{
  for (; i < n; ++i)
    if (p[i] < lo)
      lo = p[i];

  return lo;
}

/**
 * @brief Maximum twin of finishMin.
 */
[[nodiscard]] double finishMax(const double* p, std::size_t n, std::size_t i, double hi)
{
  for (; i < n; ++i)
    if (p[i] > hi)
      hi = p[i];

  return hi;
}

/**
 * @brief Finishes a partial finite min/max over the x lane of @p pts.
 */
void finishFiniteX(const QPointF* pts, qsizetype n, qsizetype i, double& lo, double& hi)
{
  const double* base = reinterpret_cast<const double*>(pts);
  for (; i < n; ++i) {
    const double v = base[2 * i];
    if (std::isfinite(v)) {
      lo = (v < lo) ? v : lo;
      hi = (v > hi) ? v : hi;
    }
  }
}

/**
 * @brief Prints one benchmark line in the fixed column order the plan names.
 */
void report(const char* level, const char* kernel, double one, double two)
{
  const double ratio = (two > kBenchTolerance) ? one / two : 0.0;
  qInfo(
    "%-6s %-22s ns/elem chains=1 %.4f chains=2 %.4f ratio %.2f", level, kernel, one, two, ratio);
}

}  // namespace

/**
 * @brief Chain-count benchmark for the four reductions on every vector lane this machine has.
 */
class TstDspReductionBench : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanup();
  void sse4Reductions();
  void avx2Reductions();
  void neonReductions();

private:
  template<typename Lane>
  void benchLane(const char* name, Lane lane);
};

void TstDspReductionBench::initTestCase()
{
  qInfo("elements %zu, repetitions %d, median of per-call time", kElements, kRepetitions);
  QCOMPARE(QByteArray(DspRef::scalarLaneName()), QByteArray("scalar"));
}

void TstDspReductionBench::cleanup()
{
  QVERIFY(DSP::setActiveSimdLevel(DSP::bestSupportedSimdLevel()));
}

/**
 * @brief Runs the four reductions at one and two chains through @p lane, a struct of static
 *        entry points bound to one lane namespace, verifying both against the oracle first.
 */
template<typename Lane>
void TstDspReductionBench::benchLane(const char* name, Lane lane)
{
  const std::vector<double> data = makeRamp();
  const std::vector<QPointF> pts = makePoints();
  const double* p                = data.data();
  const std::size_t n            = kElements;
  const auto qn                  = static_cast<qsizetype>(kElements);
  const double inf               = std::numeric_limits<double>::infinity();

  std::size_t i = 1;
  double lo     = p[0];
  double hi     = p[0];
  lane.min1(p, n, i, lo);
  QVERIFY(bitEqual(finishMin(p, n, i, lo), DspRef::minF64(p, n)));
  i  = 1;
  lo = p[0];
  lane.min2(p, n, i, lo);
  QVERIFY(bitEqual(finishMin(p, n, i, lo), DspRef::minF64(p, n)));
  i  = 1;
  hi = p[0];
  lane.max1(p, n, i, hi);
  QVERIFY(bitEqual(finishMax(p, n, i, hi), DspRef::maxF64(p, n)));
  i  = 1;
  hi = p[0];
  lane.max2(p, n, i, hi);
  QVERIFY(bitEqual(finishMax(p, n, i, hi), DspRef::maxF64(p, n)));

  double ref_lo = 0.0;
  double ref_hi = 0.0;
  DspRef::minMaxF64(p, n, ref_lo, ref_hi);
  i  = 1;
  lo = p[0];
  hi = p[0];
  lane.minMax1(p, n, i, lo, hi);
  QVERIFY(bitEqual(finishMin(p, n, i, lo), ref_lo) && bitEqual(finishMax(p, n, i, hi), ref_hi));
  i  = 1;
  lo = p[0];
  hi = p[0];
  lane.minMax2(p, n, i, lo, hi);
  QVERIFY(bitEqual(finishMin(p, n, i, lo), ref_lo) && bitEqual(finishMax(p, n, i, hi), ref_hi));

  double pref_lo = inf;
  double pref_hi = -inf;
  DspRef::finiteMinMaxPointFX(pts.data(), qn, pref_lo, pref_hi);
  const double* base = reinterpret_cast<const double*>(pts.data());
  qsizetype qi       = 0;
  lo                 = inf;
  hi                 = -inf;
  lane.point1(base, qn, qi, lo, hi);
  finishFiniteX(pts.data(), qn, qi, lo, hi);
  QVERIFY(bitEqual(lo, pref_lo) && bitEqual(hi, pref_hi));
  qi = 0;
  lo = inf;
  hi = -inf;
  lane.point2(base, qn, qi, lo, hi);
  finishFiniteX(pts.data(), qn, qi, lo, hi);
  QVERIFY(bitEqual(lo, pref_lo) && bitEqual(hi, pref_hi));

  volatile double sink = 0.0;
  report(name,
         "minF64",
         timePerElement([&] {
           std::size_t k = 1;
           double v      = p[0];
           lane.min1(p, n, k, v);
           sink = v;
         }),
         timePerElement([&] {
           std::size_t k = 1;
           double v      = p[0];
           lane.min2(p, n, k, v);
           sink = v;
         }));
  report(name,
         "maxF64",
         timePerElement([&] {
           std::size_t k = 1;
           double v      = p[0];
           lane.max1(p, n, k, v);
           sink = v;
         }),
         timePerElement([&] {
           std::size_t k = 1;
           double v      = p[0];
           lane.max2(p, n, k, v);
           sink = v;
         }));
  report(name,
         "minMaxF64",
         timePerElement([&] {
           std::size_t k = 1;
           double a      = p[0];
           double b      = p[0];
           lane.minMax1(p, n, k, a, b);
           sink = a + b;
         }),
         timePerElement([&] {
           std::size_t k = 1;
           double a      = p[0];
           double b      = p[0];
           lane.minMax2(p, n, k, a, b);
           sink = a + b;
         }));
  report(name,
         "finiteMinMaxPointF<0>",
         timePerElement([&] {
           qsizetype k = 0;
           double a    = inf;
           double b    = -inf;
           lane.point1(base, qn, k, a, b);
           sink = a + b;
         }),
         timePerElement([&] {
           qsizetype k = 0;
           double a    = inf;
           double b    = -inf;
           lane.point2(base, qn, k, a, b);
           sink = a + b;
         }));
  (void)sink;
}

//--------------------------------------------------------------------------------------------------
// Lane bindings: one struct of thin forwarders per lane namespace
//--------------------------------------------------------------------------------------------------

#if defined(SS_SIMD_X86)
struct Sse4Lane {
  static void min1(const double* p, std::size_t n, std::size_t& i, double& lo)
  {
    DSP::SimdSse4::minF64<1>(p, n, i, lo);
  }

  static void min2(const double* p, std::size_t n, std::size_t& i, double& lo)
  {
    DSP::SimdSse4::minF64<2>(p, n, i, lo);
  }

  static void max1(const double* p, std::size_t n, std::size_t& i, double& hi)
  {
    DSP::SimdSse4::maxF64<1>(p, n, i, hi);
  }

  static void max2(const double* p, std::size_t n, std::size_t& i, double& hi)
  {
    DSP::SimdSse4::maxF64<2>(p, n, i, hi);
  }

  static void minMax1(const double* p, std::size_t n, std::size_t& i, double& lo, double& hi)
  {
    DSP::SimdSse4::minMaxF64<1>(p, n, i, lo, hi);
  }

  static void minMax2(const double* p, std::size_t n, std::size_t& i, double& lo, double& hi)
  {
    DSP::SimdSse4::minMaxF64<2>(p, n, i, lo, hi);
  }

  static void point1(const double* b, qsizetype n, qsizetype& i, double& lo, double& hi)
  {
    DSP::SimdSse4::finiteMinMaxPointF<0, 1>(b, n, i, lo, hi);
  }

  static void point2(const double* b, qsizetype n, qsizetype& i, double& lo, double& hi)
  {
    DSP::SimdSse4::finiteMinMaxPointF<0, 2>(b, n, i, lo, hi);
  }
};

struct Avx2Lane {
  static void min1(const double* p, std::size_t n, std::size_t& i, double& lo)
  {
    DSP::SimdAvx2::minF64<1>(p, n, i, lo);
  }

  static void min2(const double* p, std::size_t n, std::size_t& i, double& lo)
  {
    DSP::SimdAvx2::minF64<2>(p, n, i, lo);
  }

  static void max1(const double* p, std::size_t n, std::size_t& i, double& hi)
  {
    DSP::SimdAvx2::maxF64<1>(p, n, i, hi);
  }

  static void max2(const double* p, std::size_t n, std::size_t& i, double& hi)
  {
    DSP::SimdAvx2::maxF64<2>(p, n, i, hi);
  }

  static void minMax1(const double* p, std::size_t n, std::size_t& i, double& lo, double& hi)
  {
    DSP::SimdAvx2::minMaxF64<1>(p, n, i, lo, hi);
  }

  static void minMax2(const double* p, std::size_t n, std::size_t& i, double& lo, double& hi)
  {
    DSP::SimdAvx2::minMaxF64<2>(p, n, i, lo, hi);
  }

  static void point1(const double* b, qsizetype n, qsizetype& i, double& lo, double& hi)
  {
    DSP::SimdAvx2::finiteMinMaxPointF<0, 1>(b, n, i, lo, hi);
  }

  static void point2(const double* b, qsizetype n, qsizetype& i, double& lo, double& hi)
  {
    DSP::SimdAvx2::finiteMinMaxPointF<0, 2>(b, n, i, lo, hi);
  }
};
#endif

#if defined(SS_SIMD_NEON)
struct NeonLane {
  static void min1(const double* p, std::size_t n, std::size_t& i, double& lo)
  {
    DSP::SimdNeon::minF64<1>(p, n, i, lo);
  }

  static void min2(const double* p, std::size_t n, std::size_t& i, double& lo)
  {
    DSP::SimdNeon::minF64<2>(p, n, i, lo);
  }

  static void max1(const double* p, std::size_t n, std::size_t& i, double& hi)
  {
    DSP::SimdNeon::maxF64<1>(p, n, i, hi);
  }

  static void max2(const double* p, std::size_t n, std::size_t& i, double& hi)
  {
    DSP::SimdNeon::maxF64<2>(p, n, i, hi);
  }

  static void minMax1(const double* p, std::size_t n, std::size_t& i, double& lo, double& hi)
  {
    DSP::SimdNeon::minMaxF64<1>(p, n, i, lo, hi);
  }

  static void minMax2(const double* p, std::size_t n, std::size_t& i, double& lo, double& hi)
  {
    DSP::SimdNeon::minMaxF64<2>(p, n, i, lo, hi);
  }

  static void point1(const double* b, qsizetype n, qsizetype& i, double& lo, double& hi)
  {
    DSP::SimdNeon::finiteMinMaxPointF<0, 1>(b, n, i, lo, hi);
  }

  static void point2(const double* b, qsizetype n, qsizetype& i, double& lo, double& hi)
  {
    DSP::SimdNeon::finiteMinMaxPointF<0, 2>(b, n, i, lo, hi);
  }
};
#endif

//--------------------------------------------------------------------------------------------------
// One slot per lane; a lane the machine lacks skips
//--------------------------------------------------------------------------------------------------

void TstDspReductionBench::sse4Reductions()
{
#if defined(SS_SIMD_X86)
  if (!DSP::isSimdLevelSupported(DSP::SimdLevel::Sse4))
    QSKIP("SSE4 lane not available on this machine");

  benchLane("SSE4", Sse4Lane{});
#else
  QSKIP("not an x86-64 build");
#endif
}

void TstDspReductionBench::avx2Reductions()
{
#if defined(SS_SIMD_X86)
  if (!DSP::isSimdLevelSupported(DSP::SimdLevel::Avx2))
    QSKIP("AVX2 lane not available on this machine");

  benchLane("AVX2", Avx2Lane{});
#else
  QSKIP("not an x86-64 build");
#endif
}

void TstDspReductionBench::neonReductions()
{
#if defined(SS_SIMD_NEON)
  if (!DSP::isSimdLevelSupported(DSP::SimdLevel::Neon))
    QSKIP("NEON lane not available on this machine");

  benchLane("NEON", NeonLane{});
#else
  QSKIP("not an aarch64 build");
#endif
}

QTEST_APPLESS_MAIN(TstDspReductionBench)

#include "tst_dsp_reduction_bench.moc"
