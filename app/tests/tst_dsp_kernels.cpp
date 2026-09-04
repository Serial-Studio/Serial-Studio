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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <bit>
#include <cstdint>
#include <limits>
#include <QByteArray>
#include <QList>
#include <QPointF>
#include <QTest>
#include <vector>

#include "Core/DSPSimd.h"
#include "dsp_scalar_ref.h"

/**
 * @file tst_dsp_kernels.cpp
 * @brief Per-lane bit-exactness contract for every kernel in DSPSimd.h (spec 0032, T8).
 *
 * This translation unit sees the ordinary vector build of the header (namespace DSP); the scalar
 * build lives behind DspRef:: in dsp_scalar_ref.cpp, which is the only source compiled with
 * SS_SIMD_DISABLE. Every comparison is on raw bit patterns via std::bit_cast, never on == or
 * qFuzzyCompare, because the contract CLAUDE.md and spec 0021 state is bit-exactness and not
 * numeric closeness. The single divergence the header documents -- the sign of a min/max result
 * when -0.0 and +0.0 compare equal -- is asserted explicitly in its own test rather than papered
 * over by a looser comparison everywhere else.
 *
 * Qt Test runs private slots in declaration order, so every test function here builds its own
 * inputs and carries no state to the next one.
 */

namespace {

//--------------------------------------------------------------------------------------------------
// Matrix constants
//--------------------------------------------------------------------------------------------------

constexpr qsizetype kLengths[] = {0, 1, 3, 4, 7, 8, 15, 16, 17, 31, 63, 64, 255, 1024};
constexpr int kOffsets         = 16;
constexpr char kNeedle         = '*';

enum PayloadKind {
  PayloadRamp = 0,
  PayloadNaN,
  PayloadInf,
  PayloadDenormal,
  PayloadMixed
};

constexpr int kPayloadKinds[] = {
  PayloadRamp, PayloadNaN, PayloadInf, PayloadDenormal, PayloadMixed};

/**
 * @brief Short label for a payload kind, used to build readable QTest row names.
 */
[[nodiscard]] const char* payloadName(int kind)
{
  switch (kind) {
    case PayloadRamp:
      return "ramp";
    case PayloadNaN:
      return "nan";
    case PayloadInf:
      return "inf";
    case PayloadDenormal:
      return "denormal";
    default:
      return "mixed";
  }
}

//--------------------------------------------------------------------------------------------------
// Bit-pattern comparison
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when two doubles carry the identical 64-bit pattern, so NaN payloads, signed zeros
 *        and denormals all compare as the distinct values they are.
 */
[[nodiscard]] bool bitEqual(double a, double b)
{
  return std::bit_cast<quint64>(a) == std::bit_cast<quint64>(b);
}

/**
 * @brief True when two floats carry the identical 32-bit pattern.
 */
[[nodiscard]] bool bitEqual(float a, float b)
{
  return std::bit_cast<quint32>(a) == std::bit_cast<quint32>(b);
}

/**
 * @brief The one divergence DSPSimd.h documents: both results are zero and differ only in the sign
 *        bit, which happens when -0.0 and +0.0 compare equal inside a min/max reduction.
 */
[[nodiscard]] bool zeroSignDivergence(double a, double b)
{
  const quint64 delta = std::bit_cast<quint64>(a) ^ std::bit_cast<quint64>(b);
  return a == 0.0 && b == 0.0 && delta == (UINT64_C(1) << 63);
}

/**
 * @brief Failure text naming the element and both raw patterns, so a red run says which lane of
 *        which kernel diverged instead of only that something did.
 */
[[nodiscard]] QByteArray mismatch(const char* what, qsizetype index, quint64 vec, quint64 ref)
{
  return QByteArray(what) + " diverged at index " + QByteArray::number(index) + ": vector 0x"
       + QByteArray::number(vec, 16) + " reference 0x" + QByteArray::number(ref, 16);
}

//--------------------------------------------------------------------------------------------------
// Input builders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Byte buffer of @p count usable bytes preceded by @p offset filler, so the caller can hand
 *        both kernels a pointer at every alignment 0..15 relative to the 16-byte block loop.
 */
[[nodiscard]] QByteArray makeBytes(qsizetype offset, qsizetype count, int stride)
{
  QByteArray bytes(offset + count + 16, 'z');
  for (qsizetype i = 0; i < count; ++i) {
    const bool hit    = (stride > 0) && (i % stride == 0);
    bytes[offset + i] = hit ? kNeedle : static_cast<char>('a' + (i % 24));
  }

  return bytes;
}

/**
 * @brief Doubles covering the awkward IEEE cases the reductions must carry through unchanged.
 *        Signed zeros are deliberately absent; they get their own test against the documented
 *        divergence instead of polluting the strict comparisons.
 */
[[nodiscard]] std::vector<double> makePayload(int kind, std::size_t count)
{
  const double denorm = std::numeric_limits<double>::denorm_min();
  const double inf    = std::numeric_limits<double>::infinity();
  const double nan    = std::numeric_limits<double>::quiet_NaN();

  std::vector<double> out(count, 0.0);
  for (std::size_t i = 0; i < count; ++i) {
    const double ramp = static_cast<double>(i) * 1.5 - 37.25;
    const auto phase  = static_cast<int>(i % 9);
    if (kind == PayloadRamp)
      out[i] = ramp;
    else if (kind == PayloadNaN)
      out[i] = (i % 7 == 3) ? nan : ramp;
    else if (kind == PayloadInf)
      out[i] = (i % 11 == 5) ? ((i % 22 == 5) ? inf : -inf) : ramp;
    else if (kind == PayloadDenormal)
      out[i] = denorm * static_cast<double>(i + 1) * ((i % 2 == 0) ? 1.0 : -1.0);
    else if (phase == 0)
      out[i] = nan;
    else if (phase == 1)
      out[i] = inf;
    else if (phase == 2)
      out[i] = -inf;
    else if (phase == 3)
      out[i] = denorm;
    else if (phase == 4)
      out[i] = -denorm;
    else
      out[i] = ramp;
  }

  return out;
}

/**
 * @brief Smallest power of two at or above @p n, floored at 2 so the ring kernels always get a
 *        legal mask even for an empty request.
 */
[[nodiscard]] std::size_t roundPow2(std::size_t n)
{
  std::size_t cap = 2;
  while (cap < n)
    cap <<= 1;

  return cap;
}

/**
 * @brief Window coefficients for the FFT staging kernels: finite, non-unit, and signed, so a
 *        dropped multiply or a lane swap shows up instead of cancelling out.
 */
[[nodiscard]] std::vector<float> makeWindow(std::size_t count)
{
  std::vector<float> win(count, 0.0f);
  for (std::size_t i = 0; i < count; ++i) {
    const double raw = 0.5 + 0.001 * static_cast<double>(i);
    win[i]           = static_cast<float>((i % 5 == 2) ? -raw : raw);
  }

  return win;
}

/**
 * @brief Float twin of makePayload for the f32 block kernels, over the same IEEE corner cases.
 */
[[nodiscard]] std::vector<float> makeF32Payload(int kind, std::size_t count)
{
  const auto payload = makePayload(kind, count);

  std::vector<float> out(count, 0.0f);
  for (std::size_t i = 0; i < count; ++i)
    out[i] = static_cast<float>(payload[i]);

  return out;
}

/**
 * @brief Fills a length/offset table shared by the byte-scanning and copy kernels.
 */
void addLengthOffsetRows()
{
  QTest::addColumn<qsizetype>("length");
  QTest::addColumn<int>("offset");

  for (const qsizetype length : kLengths)
    for (int offset = 0; offset < kOffsets; ++offset)
      QTest::addRow("len=%lld off=%d", static_cast<long long>(length), offset) << length << offset;
}

/**
 * @brief Fills the length/offset/payload table shared by the three f64 reductions, skipping the
 *        empty case because simdMinF64 and friends require n >= 1.
 */
void addReductionRows()
{
  QTest::addColumn<qsizetype>("length");
  QTest::addColumn<int>("offset");
  QTest::addColumn<int>("kind");

  for (const qsizetype length : kLengths) {
    if (length == 0)
      continue;

    for (int offset = 0; offset < kOffsets; ++offset)
      for (const int kind : kPayloadKinds)
        QTest::addRow(
          "%s len=%lld off=%d", payloadName(kind), static_cast<long long>(length), offset)
          << length << offset << kind;
  }
}

}  // namespace

/**
 * @brief SIMD-versus-scalar equivalence for every kernel in DSPSimd.h.
 */
class TstDspKernels : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void forEachByteMatch_data();
  void forEachByteMatch();
  void forEachByteMatchAborts();
  void findAnyByte_data();
  void findAnyByte();
  void minF64_data();
  void minF64();
  void maxF64_data();
  void maxF64();
  void minMaxF64_data();
  void minMaxF64();
  void signedZeroReductions();
  void finiteMinMaxPointF_data();
  void finiteMinMaxPointF();
  void interleaveSpan_data();
  void interleaveSpan();
  void windowedRealSpan_data();
  void windowedRealSpan();
  void windowedRealFill_data();
  void windowedRealFill();
  void ringsToPoints_data();
  void ringsToPoints();
  void asciiDots16_data();
  void asciiDots16();
  void widenAscii_data();
  void widenAscii();
  void deinterleaveToF64_data();
  void deinterleaveToF64();
  void powerSpectrumDb_data();
  void powerSpectrumDb();
};

/**
 * @brief Names the lane this binary compiled once per run, and proves the reference translation
 *        unit really is scalar: if SS_SIMD_DISABLE or SS_DSP_NAMESPACE stopped being applied the
 *        linker would fold both builds and every comparison below would be vector-versus-vector.
 */
void TstDspKernels::initTestCase()
{
#if defined(SS_SIMD_X86)
  const char* lane = "x86 SSE2..SSE4.2";
#elif defined(SS_SIMD_NEON)
  const char* lane = "aarch64 NEON";
#else
  const char* lane = "scalar (no vector lane compiled for this target)";
#endif

  qInfo("DSP vector lane under test: %s", lane);
  qInfo("DSP reference lane: %s", DspRef::scalarLaneName());
  QCOMPARE(QByteArray(DspRef::scalarLaneName()), QByteArray("scalar"));
}

//--------------------------------------------------------------------------------------------------
// Byte scanning
//--------------------------------------------------------------------------------------------------

void TstDspKernels::forEachByteMatch_data()
{
  addLengthOffsetRows();
}

/**
 * @brief Both lanes report the same match positions, in the same ascending order.
 */
void TstDspKernels::forEachByteMatch()
{
  QFETCH(qsizetype, length);
  QFETCH(int, offset);

  const QByteArray bytes = makeBytes(offset, length, 3);
  const char* data       = bytes.constData() + offset;

  QList<qsizetype> vec;
  QList<qsizetype> ref;
  const bool vec_ok = DSP::simdForEachByteMatch(data, length, kNeedle, [&vec](qsizetype pos) {
    vec.append(pos);
    return true;
  });
  const bool ref_ok = DspRef::forEachByteMatch(data, length, kNeedle, [&ref](qsizetype pos) {
    ref.append(pos);
    return true;
  });

  QCOMPARE(vec_ok, ref_ok);
  QCOMPARE(vec, ref);
}

/**
 * @brief A callback that refuses the third match stops both lanes at the same position and makes
 *        both return false.
 */
void TstDspKernels::forEachByteMatchAborts()
{
  const QByteArray bytes = makeBytes(0, 1024, 5);
  const char* data       = bytes.constData();

  QList<qsizetype> vec;
  QList<qsizetype> ref;
  const bool vec_ok = DSP::simdForEachByteMatch(data, 1024, kNeedle, [&vec](qsizetype pos) {
    vec.append(pos);
    return vec.size() < 3;
  });
  const bool ref_ok = DspRef::forEachByteMatch(data, 1024, kNeedle, [&ref](qsizetype pos) {
    ref.append(pos);
    return ref.size() < 3;
  });

  QCOMPARE(vec_ok, false);
  QCOMPARE(ref_ok, false);
  QCOMPARE(vec, ref);
  QCOMPARE(vec.size(), qsizetype(3));
}

void TstDspKernels::findAnyByte_data()
{
  QTest::addColumn<qsizetype>("length");
  QTest::addColumn<int>("offset");
  QTest::addColumn<int>("count");

  for (const qsizetype length : kLengths)
    for (int offset = 0; offset < kOffsets; ++offset)
      for (int count = 1; count <= 8; count += (count == 1 ? 1 : 3))
        QTest::addRow("len=%lld off=%d needles=%d", static_cast<long long>(length), offset, count)
          << length << offset << count;
}

/**
 * @brief First-membership index agrees between lanes for 1..8 needles, including the absent case
 *        where both must return the length.
 */
void TstDspKernels::findAnyByte()
{
  QFETCH(qsizetype, length);
  QFETCH(int, offset);
  QFETCH(int, count);

  const quint8 needles[8]  = {'*', 'Q', 'W', 0x00, 0x7F, 0xFE, 'K', 'M'};
  const QByteArray present = makeBytes(offset, length, 7);
  const QByteArray absent  = makeBytes(offset, length, 0);

  const char* hit  = present.constData() + offset;
  const char* miss = absent.constData() + offset;
  QCOMPARE(DSP::simdFindAnyByte(hit, length, needles, count),
           DspRef::findAnyByte(hit, length, needles, count));
  QCOMPARE(DSP::simdFindAnyByte(miss, length, needles, count),
           DspRef::findAnyByte(miss, length, needles, count));
}

//--------------------------------------------------------------------------------------------------
// f64 reductions
//--------------------------------------------------------------------------------------------------

void TstDspKernels::minF64_data()
{
  addReductionRows();
}

/**
 * @brief simdMinF64 is bit-identical to its scalar reference, NaN stickiness included.
 */
void TstDspKernels::minF64()
{
  QFETCH(qsizetype, length);
  QFETCH(int, offset);
  QFETCH(int, kind);

  const auto n                      = static_cast<std::size_t>(length);
  const std::vector<double> payload = makePayload(kind, n + static_cast<std::size_t>(offset));
  const double* p                   = payload.data() + offset;

  const double vec = DSP::simdMinF64(p, n);
  const double ref = DspRef::minF64(p, n);
  if (!bitEqual(vec, ref))
    QFAIL(mismatch("simdMinF64", length, std::bit_cast<quint64>(vec), std::bit_cast<quint64>(ref))
            .constData());
}

void TstDspKernels::maxF64_data()
{
  addReductionRows();
}

/**
 * @brief simdMaxF64 is bit-identical to its scalar reference.
 */
void TstDspKernels::maxF64()
{
  QFETCH(qsizetype, length);
  QFETCH(int, offset);
  QFETCH(int, kind);

  const auto n                      = static_cast<std::size_t>(length);
  const std::vector<double> payload = makePayload(kind, n + static_cast<std::size_t>(offset));
  const double* p                   = payload.data() + offset;

  const double vec = DSP::simdMaxF64(p, n);
  const double ref = DspRef::maxF64(p, n);
  if (!bitEqual(vec, ref))
    QFAIL(mismatch("simdMaxF64", length, std::bit_cast<quint64>(vec), std::bit_cast<quint64>(ref))
            .constData());
}

void TstDspKernels::minMaxF64_data()
{
  addReductionRows();
}

/**
 * @brief The one-pass kernel agrees with the scalar reference and with the two single-sided
 *        kernels on the same input, so a fused-loop bug cannot hide behind a matching reference.
 */
void TstDspKernels::minMaxF64()
{
  QFETCH(qsizetype, length);
  QFETCH(int, offset);
  QFETCH(int, kind);

  const auto n                      = static_cast<std::size_t>(length);
  const std::vector<double> payload = makePayload(kind, n + static_cast<std::size_t>(offset));
  const double* p                   = payload.data() + offset;

  double vec_lo = 0.0;
  double vec_hi = 0.0;
  double ref_lo = 0.0;
  double ref_hi = 0.0;
  DSP::simdMinMaxF64(p, n, vec_lo, vec_hi);
  DspRef::minMaxF64(p, n, ref_lo, ref_hi);

  if (!bitEqual(vec_lo, ref_lo))
    QFAIL(
      mismatch(
        "simdMinMaxF64 lo", length, std::bit_cast<quint64>(vec_lo), std::bit_cast<quint64>(ref_lo))
        .constData());

  if (!bitEqual(vec_hi, ref_hi))
    QFAIL(
      mismatch(
        "simdMinMaxF64 hi", length, std::bit_cast<quint64>(vec_hi), std::bit_cast<quint64>(ref_hi))
        .constData());

  QVERIFY(bitEqual(vec_lo, DSP::simdMinF64(p, n)));
  QVERIFY(bitEqual(vec_hi, DSP::simdMaxF64(p, n)));
}

/**
 * @brief The header's single documented divergence: with only signed zeros in play the two lanes
 *        may pick a different sign, so the contract is "equal, or differing only in the sign bit",
 *        and nothing weaker. Any other difference still fails.
 */
void TstDspKernels::signedZeroReductions()
{
  for (const qsizetype length : kLengths) {
    if (length == 0)
      continue;

    const auto n = static_cast<std::size_t>(length);
    std::vector<double> zeros(n, 0.0);
    for (std::size_t i = 0; i < n; ++i)
      zeros[i] = (i % 3 == 0) ? -0.0 : 0.0;

    const double* p      = zeros.data();
    const double vec_min = DSP::simdMinF64(p, n);
    const double ref_min = DspRef::minF64(p, n);
    const double vec_max = DSP::simdMaxF64(p, n);
    const double ref_max = DspRef::maxF64(p, n);

    QVERIFY(bitEqual(vec_min, ref_min) || zeroSignDivergence(vec_min, ref_min));
    QVERIFY(bitEqual(vec_max, ref_max) || zeroSignDivergence(vec_max, ref_max));
    QVERIFY(vec_min == 0.0 && vec_max == 0.0);
  }
}

//--------------------------------------------------------------------------------------------------
// QPointF lane reductions
//--------------------------------------------------------------------------------------------------

void TstDspKernels::finiteMinMaxPointF_data()
{
  addReductionRows();
}

/**
 * @brief Both QPointF lanes accumulate the same finite min/max into the caller's sentinel seeds,
 *        with non-finite points skipped identically.
 */
void TstDspKernels::finiteMinMaxPointF()
{
  QFETCH(qsizetype, length);
  QFETCH(int, offset);
  QFETCH(int, kind);

  const auto n                      = static_cast<std::size_t>(length);
  const std::vector<double> payload = makePayload(kind, 2 * (n + static_cast<std::size_t>(offset)));
  std::vector<QPointF> pts(n + static_cast<std::size_t>(offset), QPointF());
  for (std::size_t i = 0; i < pts.size(); ++i)
    pts[i] = QPointF(payload[2 * i], payload[2 * i + 1]);

  const double inf = std::numeric_limits<double>::infinity();
  const QPointF* p = pts.data() + offset;
  double vec_lo    = inf;
  double vec_hi    = -inf;
  double ref_lo    = inf;
  double ref_hi    = -inf;

  DSP::simdFiniteMinMaxPointF<0>(p, length, vec_lo, vec_hi);
  DspRef::finiteMinMaxPointFX(p, length, ref_lo, ref_hi);
  QVERIFY(bitEqual(vec_lo, ref_lo) || zeroSignDivergence(vec_lo, ref_lo));
  QVERIFY(bitEqual(vec_hi, ref_hi) || zeroSignDivergence(vec_hi, ref_hi));

  vec_lo = inf;
  vec_hi = -inf;
  ref_lo = inf;
  ref_hi = -inf;
  DSP::simdFiniteMinMaxPointF<1>(p, length, vec_lo, vec_hi);
  DspRef::finiteMinMaxPointFY(p, length, ref_lo, ref_hi);
  QVERIFY(bitEqual(vec_lo, ref_lo) || zeroSignDivergence(vec_lo, ref_lo));
  QVERIFY(bitEqual(vec_hi, ref_hi) || zeroSignDivergence(vec_hi, ref_hi));
}

//--------------------------------------------------------------------------------------------------
// SimdDetail contiguous-span helpers
//--------------------------------------------------------------------------------------------------

void TstDspKernels::interleaveSpan_data()
{
  addLengthOffsetRows();
}

/**
 * @brief The (x, y) interleave is a pure copy, so every output word must be bit-identical.
 */
void TstDspKernels::interleaveSpan()
{
  QFETCH(qsizetype, length);
  QFETCH(int, offset);

  const auto n                 = static_cast<std::size_t>(length);
  const auto pad               = static_cast<std::size_t>(offset);
  const std::vector<double> xs = makePayload(PayloadMixed, n + pad + 1);
  const std::vector<double> ys = makePayload(PayloadDenormal, n + pad + 1);
  std::vector<double> vec(2 * n + 2, 1.0);
  std::vector<double> ref(2 * n + 2, 1.0);

  DSP::SimdDetail::interleaveSpan(xs.data() + offset, ys.data() + offset, vec.data(), length);
  DspRef::interleaveSpan(xs.data() + offset, ys.data() + offset, ref.data(), length);

  for (std::size_t i = 0; i < vec.size(); ++i)
    if (!bitEqual(vec[i], ref[i]))
      QFAIL(mismatch("interleaveSpan",
                     static_cast<qsizetype>(i),
                     std::bit_cast<quint64>(vec[i]),
                     std::bit_cast<quint64>(ref[i]))
              .constData());
}

void TstDspKernels::windowedRealSpan_data()
{
  addReductionRows();
}

/**
 * @brief The kiss_fftr staging span: one windowed f32 per input sample, non-finite inputs
 *        folded to zero, compared word-for-word against the scalar lane.
 */
void TstDspKernels::windowedRealSpan()
{
  QFETCH(qsizetype, length);
  QFETCH(int, offset);
  QFETCH(int, kind);

  const auto n                      = static_cast<std::size_t>(length);
  const std::vector<double> payload = makePayload(kind, n + static_cast<std::size_t>(offset));
  const std::vector<float> win      = makeWindow(n + 1);
  std::vector<float> vec(n + 2, 1.0f);
  std::vector<float> ref(n + 2, 1.0f);

  const double* src = payload.data() + offset;
  DSP::SimdDetail::windowedRealSpan(src, win.data(), vec.data(), n, -3.25, 0.5);
  DspRef::windowedRealSpan(src, win.data(), ref.data(), n, -3.25, 0.5);

  for (std::size_t i = 0; i < vec.size(); ++i)
    if (!bitEqual(vec[i], ref[i]))
      QFAIL(mismatch("windowedRealSpan",
                     static_cast<qsizetype>(i),
                     std::bit_cast<quint32>(vec[i]),
                     std::bit_cast<quint32>(ref[i]))
              .constData());
}

//--------------------------------------------------------------------------------------------------
// Ring-buffer bulk transforms
//--------------------------------------------------------------------------------------------------

void TstDspKernels::windowedRealFill_data()
{
  addReductionRows();
}

void TstDspKernels::windowedRealFill()
{
  QFETCH(qsizetype, length);
  QFETCH(int, offset);
  QFETCH(int, kind);

  const auto n                   = static_cast<std::size_t>(length);
  const std::size_t cap          = roundPow2(n);
  const std::size_t mask         = cap - 1;
  const std::size_t front        = static_cast<std::size_t>(offset) & mask;
  const std::vector<double> ring = makePayload(kind, cap);
  const std::vector<float> win   = makeWindow(n + 1);
  std::vector<float> vec(n + 2, 1.0f);
  std::vector<float> ref(n + 2, 1.0f);

  DSP::simdWindowedRealFill(ring.data(), front, mask, n, -3.25, 0.5, win.data(), vec.data());
  DspRef::windowedRealFill(ring.data(), front, mask, n, -3.25, 0.5, win.data(), ref.data());

  for (std::size_t i = 0; i < vec.size(); ++i)
    if (!bitEqual(vec[i], ref[i]))
      QFAIL(mismatch("simdWindowedRealFill",
                     static_cast<qsizetype>(i),
                     std::bit_cast<quint32>(vec[i]),
                     std::bit_cast<quint32>(ref[i]))
              .constData());
}

void TstDspKernels::ringsToPoints_data()
{
  addLengthOffsetRows();
}

/**
 * @brief Two rings with different capacities and fronts gather into the same QPointF sequence,
 *        which is the segment-chunking case the plot path depends on.
 */
void TstDspKernels::ringsToPoints()
{
  QFETCH(qsizetype, length);
  QFETCH(int, offset);

  const auto n                 = static_cast<std::size_t>(length);
  const std::size_t xcap       = roundPow2(n);
  const std::size_t ycap       = roundPow2(n / 2 + 1);
  const std::size_t xmask      = xcap - 1;
  const std::size_t ymask      = ycap - 1;
  const std::size_t xfront     = static_cast<std::size_t>(offset) & xmask;
  const std::size_t yfront     = static_cast<std::size_t>(3 * offset) & ymask;
  const std::vector<double> xs = makePayload(PayloadMixed, xcap);
  const std::vector<double> ys = makePayload(PayloadDenormal, ycap);
  std::vector<QPointF> vec(n + 1, QPointF(1.0, 1.0));
  std::vector<QPointF> ref(n + 1, QPointF(1.0, 1.0));

  DSP::simdRingsToPoints(xs.data(), xfront, xmask, ys.data(), yfront, ymask, length, vec.data());
  DspRef::ringsToPoints(xs.data(), xfront, xmask, ys.data(), yfront, ymask, length, ref.data());

  for (std::size_t i = 0; i < vec.size(); ++i) {
    if (!bitEqual(vec[i].x(), ref[i].x()))
      QFAIL(mismatch("simdRingsToPoints x",
                     static_cast<qsizetype>(i),
                     std::bit_cast<quint64>(vec[i].x()),
                     std::bit_cast<quint64>(ref[i].x()))
              .constData());

    if (!bitEqual(vec[i].y(), ref[i].y()))
      QFAIL(mismatch("simdRingsToPoints y",
                     static_cast<qsizetype>(i),
                     std::bit_cast<quint64>(vec[i].y()),
                     std::bit_cast<quint64>(ref[i].y()))
              .constData());
  }
}

//--------------------------------------------------------------------------------------------------
// Text formatting
//--------------------------------------------------------------------------------------------------

void TstDspKernels::asciiDots16_data()
{
  QTest::addColumn<int>("base");

  for (int base = 0; base < 256; base += 16)
    QTest::addRow("bytes 0x%02X..0x%02X", base, base + 15) << base;

  QTest::addRow("printable boundary") << -1;
}

/**
 * @brief The hex-dump ASCII column agrees over the whole byte range, including the 0x20 and 0x7E
 *        edges of the printable window that the x86 lane reaches through unsigned min/max.
 */
void TstDspKernels::asciiDots16()
{
  QFETCH(int, base);

  const quint8 edges[16] = {
    0x00, 0x1F, 0x20, 0x21, 0x7D, 0x7E, 0x7F, 0x80, 0xFF, 0x41, 0x0A, 0x0D, 0x09, 0x2E, 0xC3, 0xA9};

  quint8 src[16] = {0};
  for (int i = 0; i < 16; ++i)
    src[i] = (base >= 0) ? static_cast<quint8>(base + i) : edges[i];

  char16_t vec[16] = {0};
  char16_t ref[16] = {0};
  DSP::simdAsciiDots16(src, vec);
  DspRef::asciiDots16(src, ref);

  for (int i = 0; i < 16; ++i)
    if (vec[i] != ref[i])
      QFAIL(mismatch("simdAsciiDots16", i, vec[i], ref[i]).constData());
}

/**
 * @brief Length/offset matrix plus a per-row switch between pure-ASCII and high-bit payloads,
 *        so both the all-ASCII verdict and the fallback verdict cross every lane boundary.
 */
void TstDspKernels::widenAscii_data()
{
  QTest::addColumn<qsizetype>("length");
  QTest::addColumn<int>("offset");
  QTest::addColumn<int>("highAt");

  for (const qsizetype length : kLengths)
    for (int offset = 0; offset < kOffsets; ++offset)
      for (const int highAt : {-1, 0, 1, 7, 8, 15, 16}) {
        if (static_cast<qsizetype>(highAt) >= length)
          continue;

        QTest::addRow("len=%lld off=%d high=%d", static_cast<long long>(length), offset, highAt)
          << length << offset << highAt;
      }
}

void TstDspKernels::widenAscii()
{
  QFETCH(qsizetype, length);
  QFETCH(int, offset);
  QFETCH(int, highAt);

  QByteArray bytes(offset + length + 16, 'z');
  for (qsizetype i = 0; i < length; ++i)
    bytes[offset + i] = static_cast<char>(1 + (i % 127));

  if (highAt >= 0)
    bytes[offset + highAt] = static_cast<char>(0xC3);

  const auto n          = static_cast<std::size_t>(length);
  constexpr auto kGuard = static_cast<char16_t>(0xABCD);
  std::vector<char16_t> vec(n + 8, kGuard);
  std::vector<char16_t> ref(n + 8, kGuard);

  const bool vecAscii = DSP::simdWidenAscii(bytes.constData() + offset, vec.data(), n);
  const bool refAscii = DspRef::widenAscii(bytes.constData() + offset, ref.data(), n);
  QCOMPARE(vecAscii, refAscii);
  QCOMPARE(vecAscii, highAt < 0);

  for (std::size_t i = 0; i < vec.size(); ++i)
    if (vec[i] != ref[i])
      QFAIL(mismatch("simdWidenAscii", static_cast<qsizetype>(i), vec[i], ref[i]).constData());
}

/**
 * @brief Frame-count matrix across every channel layout, so the contiguous single-channel lane
 *        and the strided gather are both walked over the IEEE corner payloads.
 */
void TstDspKernels::deinterleaveToF64_data()
{
  QTest::addColumn<qsizetype>("frames");
  QTest::addColumn<int>("channels");
  QTest::addColumn<int>("kind");

  for (const qsizetype frames : kLengths)
    for (const int channels : {1, 2, 3, 4, 8})
      for (const int kind : kPayloadKinds)
        QTest::addRow(
          "%s frames=%lld ch=%d", payloadName(kind), static_cast<long long>(frames), channels)
          << frames << channels << kind;
}

void TstDspKernels::deinterleaveToF64()
{
  QFETCH(qsizetype, frames);
  QFETCH(int, channels);
  QFETCH(int, kind);

  const auto n     = static_cast<std::size_t>(frames);
  const auto total = n * static_cast<std::size_t>(channels);
  const auto src   = makeF32Payload(kind, total + 8);

  for (int channel = 0; channel < channels; ++channel) {
    std::vector<double> vec(n + 4, -1.0);
    std::vector<double> ref(n + 4, -1.0);
    DSP::simdDeinterleaveToF64(src.data(), n, channels, channel, vec.data());
    DspRef::deinterleaveToF64(src.data(), n, channels, channel, ref.data());

    for (std::size_t i = 0; i < vec.size(); ++i)
      if (!bitEqual(vec[i], ref[i]))
        QFAIL(mismatch("simdDeinterleaveToF64",
                       static_cast<qsizetype>(i),
                       std::bit_cast<quint64>(vec[i]),
                       std::bit_cast<quint64>(ref[i]))
                .constData());
  }
}

/**
 * @brief Spectrum-length matrix over the IEEE corner payloads, so the vectorized power stage and
 *        the eps/floor clamps are compared bit-for-bit against the scalar lane.
 */
void TstDspKernels::powerSpectrumDb_data()
{
  QTest::addColumn<qsizetype>("length");
  QTest::addColumn<int>("kind");

  for (const qsizetype length : kLengths)
    for (const int kind : kPayloadKinds)
      QTest::addRow("%s len=%lld", payloadName(kind), static_cast<long long>(length))
        << length << kind;
}

void TstDspKernels::powerSpectrumDb()
{
  QFETCH(qsizetype, length);
  QFETCH(int, kind);

  const auto n     = static_cast<std::size_t>(length);
  const auto inter = makeF32Payload(kind, 2 * n + 8);
  const float norm = 1.0f / (256.0f * 256.0f);

  std::vector<float> vec(n + 4, -7.0f);
  std::vector<float> ref(n + 4, -7.0f);
  DSP::simdPowerSpectrumDb(inter.data(), vec.data(), n, norm, 1e-24f, -100.0f);
  DspRef::powerSpectrumDb(inter.data(), ref.data(), n, norm, 1e-24f, -100.0f);

  for (std::size_t i = 0; i < vec.size(); ++i)
    if (!bitEqual(vec[i], ref[i]))
      QFAIL(mismatch("simdPowerSpectrumDb",
                     static_cast<qsizetype>(i),
                     std::bit_cast<quint32>(vec[i]),
                     std::bit_cast<quint32>(ref[i]))
              .constData());
}

QTEST_APPLESS_MAIN(TstDspKernels)

#include "tst_dsp_kernels.moc"
