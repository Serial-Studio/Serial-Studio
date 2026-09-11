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
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <QPointF>
#include <QtGlobal>

#include "Core/SimdLevel.h"
#include "Core/SSAssert.h"

/**
 * @file DSPSimd.h
 * @brief Portable SIMD kernels shared by the parse, render and export paths.
 *
 * Every kernel here has four lanes selected at runtime through DSP::activeSimdLevel() (spec 0081):
 * an SSE4 lane (SSE2..SSE4.2 intrinsics -- the product ships with an x86-64-v2 baseline per
 * cmake/Optimization.cmake, so SSSE3/SSE4.1 usage is legal even under MSVC cl.exe, which only
 * advertises _M_X64), an AVX2 lane whose bodies live out-of-line in DSPSimdAvx2.h behind a
 * never-inline call boundary, a NEON lane (aarch64 only, so float64x2 is always available), and a
 * scalar tail that is the reference semantics for all three and doubles as the selectable Scalar
 * lane: a kernel reads the level once per call, runs the AVX2 body only when a whole 256-bit block
 * exists, lets the inline 128-bit lane finish the residue (AVX2 implies SSE4 here), and closes
 * every span in the scalar loop. The reductions restart their seed, so they pick one lane instead.
 *
 * Hard rule, mirroring HotpathOptimization.h: no kernel may change observable values. Allowed op
 * classes are byte compares/masks, integer ops, IEEE min/max compares, per-lane add/mul in the
 * scalar's exact order, and per-lane f64->f32 converts. No horizontal float sums, no reassociation,
 * no approximate transcendentals. The single documented divergence is the sign of a min/max result
 * when -0.0 and +0.0 compare equal, which no caller observes.
 */

#if !defined(SS_SIMD_DISABLE)
#  if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
#    define SS_SIMD_X86 1
#    include <immintrin.h>
#  elif defined(__aarch64__) || defined(_M_ARM64)
#    define SS_SIMD_NEON 1
#    include <arm_neon.h>
#  endif
#endif

#ifndef SS_DSP_NAMESPACE
#  define SS_DSP_NAMESPACE DSP
#endif

#if defined(SS_SIMD_X86)
#  include "Core/DSPSimdAvx2.h"
#endif

namespace SS_DSP_NAMESPACE {

//--------------------------------------------------------------------------------------------------
// SSE4 lane (x86-64-v2 baseline)
//--------------------------------------------------------------------------------------------------

#if defined(SS_SIMD_X86)
namespace SimdSse4 {

/**
 * @brief Two-wide windowedRealSpan; leaves @p i at the first unprocessed element.
 */
inline void windowedRealSpan(const double* src,
                             const float* win,
                             float* out,
                             std::size_t n,
                             double offset,
                             double scale,
                             std::size_t& i)
{
  SS_ASSERT(src != nullptr || n == 0, return);
  SS_ASSERT(win != nullptr || n == 0, return);

  const __m128d sign_mask = _mm_set1_pd(-0.0);
  const __m128d plus_inf  = _mm_set1_pd(std::numeric_limits<double>::infinity());
  const __m128d v_offset  = _mm_set1_pd(offset);
  const __m128d v_scale   = _mm_set1_pd(scale);
  for (; i + 2 <= n; i += 2) {
    const __m128d raw    = _mm_loadu_pd(src + i);
    const __m128d finite = _mm_cmplt_pd(_mm_andnot_pd(sign_mask, raw), plus_inf);
    const __m128d scaled = _mm_mul_pd(_mm_add_pd(raw, v_offset), v_scale);
    const __m128 v       = _mm_cvtpd_ps(_mm_and_pd(scaled, finite));
    const __m128i w_bits = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(win + i));
    const __m128 vw      = _mm_mul_ps(v, _mm_castsi128_ps(w_bits));
    _mm_storel_epi64(reinterpret_cast<__m128i*>(out + i), _mm_castps_si128(vw));
  }
}

/**
 * @brief Two-wide interleaveSpan; leaves @p i at the first unprocessed element.
 */
inline void interleaveSpan(
  const double* xs, const double* ys, double* out, qsizetype n, qsizetype& i)
{
  SS_ASSERT(xs != nullptr || n == 0, return);
  SS_ASSERT(ys != nullptr || n == 0, return);

  for (; i + 2 <= n; i += 2) {
    const __m128d a = _mm_loadu_pd(xs + i);
    const __m128d b = _mm_loadu_pd(ys + i);
    _mm_storeu_pd(out + 2 * i, _mm_unpacklo_pd(a, b));
    _mm_storeu_pd(out + 2 * i + 2, _mm_unpackhi_pd(a, b));
  }
}

/**
 * @brief Four-wide widenF32Span; leaves @p i at the first unprocessed element.
 */
inline void widenF32Span(const float* src, double* out, std::size_t n, std::size_t& i)
{
  SS_ASSERT(src != nullptr || n == 0, return);
  SS_ASSERT(out != nullptr || n == 0, return);

  for (; i + 4 <= n; i += 4) {
    const __m128 v = _mm_loadu_ps(src + i);
    _mm_storeu_pd(out + i, _mm_cvtps_pd(v));
    _mm_storeu_pd(out + i + 2, _mm_cvtps_pd(_mm_movehl_ps(v, v)));
  }
}

/**
 * @brief 16-byte blocks of simdForEachByteMatch; false when @p onMatch aborted the scan.
 */
template<typename OnMatch>
[[nodiscard]] inline bool forEachByteMatch(
  const char* data, qsizetype len, char needle, OnMatch&& onMatch, qsizetype& i)
{
  SS_ASSERT(data != nullptr || len == 0, return true);
  SS_ASSERT(i >= 0 && i <= len, return true);

  const __m128i pattern = _mm_set1_epi8(needle);
  for (; i + 16 <= len; i += 16) {
    const __m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + i));
    unsigned mask       = static_cast<unsigned>(_mm_movemask_epi8(_mm_cmpeq_epi8(block, pattern)));
    for (int b = 0; b < 16 && mask != 0; ++b) {
      const qsizetype pos = i + std::countr_zero(mask);
      if (!onMatch(pos))
        return false;

      mask &= mask - 1;
    }
  }

  return true;
}

/**
 * @brief 16-byte blocks of simdFindAnyByte; the first hit index, or -1 with @p i at the first
 *        unprocessed byte.
 */
[[nodiscard]] inline qsizetype findAnyByte(
  const char* data, qsizetype len, const quint8* needles, int count, qsizetype& i)
{
  constexpr int kMaxNeedles = 8;
  SS_ASSERT(data != nullptr || len == 0, return -1);
  SS_ASSERT(needles != nullptr, return -1);
  SS_ASSERT(count >= 1 && count <= kMaxNeedles, return -1);

  __m128i patterns[kMaxNeedles];
  for (int k = 0; k < count && k < kMaxNeedles; ++k)
    patterns[k] = _mm_set1_epi8(static_cast<char>(needles[k]));

  for (; i + 16 <= len; i += 16) {
    const __m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + i));
    __m128i hit         = _mm_cmpeq_epi8(block, patterns[0]);
    for (int k = 1; k < count && k < kMaxNeedles; ++k)
      hit = _mm_or_si128(hit, _mm_cmpeq_epi8(block, patterns[k]));

    const unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(hit));
    if (mask != 0)
      return i + std::countr_zero(mask);
  }

  return -1;
}

/**
 * @brief 16-byte blocks of simdWidenAscii; returns the OR of every byte's high bit.
 */
[[nodiscard]] inline quint64 widenAscii(const char* src,
                                        char16_t* out,
                                        std::size_t n,
                                        std::size_t& i)
{
  SS_ASSERT(src != nullptr || n == 0, return 0);
  SS_ASSERT(out != nullptr || n == 0, return 0);

  quint64 high_bits  = 0;
  const __m128i zero = _mm_setzero_si128();
  for (; i + 16 <= n; i += 16) {
    const __m128i block  = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src + i));
    high_bits           |= static_cast<unsigned>(_mm_movemask_epi8(block));
    _mm_storeu_si128(reinterpret_cast<__m128i*>(out + i), _mm_unpacklo_epi8(block, zero));
    _mm_storeu_si128(reinterpret_cast<__m128i*>(out + i + 8), _mm_unpackhi_epi8(block, zero));
  }

  return high_bits;
}

/**
 * @brief Four-wide power stage of simdPowerSpectrumDb.
 */
inline void powerSpectrum(
  const float* interleaved, float* out, std::size_t n, float invNorm, float epsSq, std::size_t& i)
{
  SS_ASSERT(interleaved != nullptr || n == 0, return);
  SS_ASSERT(out != nullptr || n == 0, return);

  const __m128 v_norm = _mm_set1_ps(invNorm);
  const __m128 v_eps  = _mm_set1_ps(epsSq);
  for (; i + 4 <= n; i += 4) {
    const __m128 lo  = _mm_loadu_ps(interleaved + 2 * i);
    const __m128 hi  = _mm_loadu_ps(interleaved + 2 * i + 4);
    const __m128 sum = _mm_hadd_ps(_mm_mul_ps(lo, lo), _mm_mul_ps(hi, hi));
    _mm_storeu_ps(out + i, _mm_max_ps(v_eps, _mm_mul_ps(sum, v_norm)));
  }
}

/**
 * @brief Two-wide simdMinF64 body for n >= 2 * kChains: kChains independent accumulators
 *        (spec 0082) so the compare latency overlaps, folded chain 1 into chain 0 with the loop's
 *        own operand order. Legal because min is order-independent, a NaN element never wins in
 *        any chain and a NaN seed poisons every chain alike.
 */
template<int kChains>
inline void minF64(const double* p, std::size_t n, std::size_t& i, double& lo)
{
  static_assert(kChains == 1 || kChains == 2, "one or two accumulator chains");
  constexpr std::size_t kStride = 2 * kChains;
  SS_ASSERT(p != nullptr, return);
  SS_ASSERT(n >= kStride, return);

  __m128d acc0                  = _mm_set1_pd(p[0]);
  [[maybe_unused]] __m128d acc1 = acc0;
  for (i = 0; i + kStride <= n; i += kStride) {
    acc0 = _mm_min_pd(_mm_loadu_pd(p + i), acc0);
    if constexpr (kChains == 2)
      acc1 = _mm_min_pd(_mm_loadu_pd(p + i + 2), acc1);
  }

  if constexpr (kChains == 2)
    acc0 = _mm_min_pd(acc1, acc0);

  const __m128d swapped = _mm_unpackhi_pd(acc0, acc0);
  lo                    = _mm_cvtsd_f64(_mm_min_sd(swapped, acc0));
}

/**
 * @brief Two-wide simdMaxF64 body for n >= 2 * kChains, the maximum twin of minF64.
 */
template<int kChains>
inline void maxF64(const double* p, std::size_t n, std::size_t& i, double& hi)
{
  static_assert(kChains == 1 || kChains == 2, "one or two accumulator chains");
  constexpr std::size_t kStride = 2 * kChains;
  SS_ASSERT(p != nullptr, return);
  SS_ASSERT(n >= kStride, return);

  __m128d acc0                  = _mm_set1_pd(p[0]);
  [[maybe_unused]] __m128d acc1 = acc0;
  for (i = 0; i + kStride <= n; i += kStride) {
    acc0 = _mm_max_pd(_mm_loadu_pd(p + i), acc0);
    if constexpr (kChains == 2)
      acc1 = _mm_max_pd(_mm_loadu_pd(p + i + 2), acc1);
  }

  if constexpr (kChains == 2)
    acc0 = _mm_max_pd(acc1, acc0);

  const __m128d swapped = _mm_unpackhi_pd(acc0, acc0);
  hi                    = _mm_cvtsd_f64(_mm_max_sd(swapped, acc0));
}

/**
 * @brief Two-wide simdMinMaxF64 body for n >= 2 * kChains, both sides chained like minF64.
 */
template<int kChains>
inline void minMaxF64(const double* p, std::size_t n, std::size_t& i, double& lo, double& hi)
{
  static_assert(kChains == 1 || kChains == 2, "one or two accumulator chains");
  constexpr std::size_t kStride = 2 * kChains;
  SS_ASSERT(p != nullptr, return);
  SS_ASSERT(n >= kStride, return);

  __m128d acc_lo0                  = _mm_set1_pd(p[0]);
  __m128d acc_hi0                  = acc_lo0;
  [[maybe_unused]] __m128d acc_lo1 = acc_lo0;
  [[maybe_unused]] __m128d acc_hi1 = acc_lo0;
  for (i = 0; i + kStride <= n; i += kStride) {
    const __m128d v0 = _mm_loadu_pd(p + i);
    acc_lo0          = _mm_min_pd(v0, acc_lo0);
    acc_hi0          = _mm_max_pd(v0, acc_hi0);
    if constexpr (kChains == 2) {
      const __m128d v1 = _mm_loadu_pd(p + i + 2);
      acc_lo1          = _mm_min_pd(v1, acc_lo1);
      acc_hi1          = _mm_max_pd(v1, acc_hi1);
    }
  }

  if constexpr (kChains == 2) {
    acc_lo0 = _mm_min_pd(acc_lo1, acc_lo0);
    acc_hi0 = _mm_max_pd(acc_hi1, acc_hi0);
  }

  lo = _mm_cvtsd_f64(_mm_min_sd(_mm_unpackhi_pd(acc_lo0, acc_lo0), acc_lo0));
  hi = _mm_cvtsd_f64(_mm_max_sd(_mm_unpackhi_pd(acc_hi0, acc_hi0), acc_hi0));
}

/**
 * @brief Gathers one QPointF lane of two consecutive points into a vector.
 */
template<int kLane>
[[nodiscard]] inline __m128d pointLanePair(const double* base, qsizetype point)
{
  const __m128d a = _mm_loadu_pd(base + 2 * point);
  const __m128d b = _mm_loadu_pd(base + 2 * point + 2);
  return (kLane == 0) ? _mm_unpacklo_pd(a, b) : _mm_unpackhi_pd(a, b);
}

/**
 * @brief Two-point simdFiniteMinMaxPointF body for n >= 2 * kChains, chained like minMaxF64.
 */
template<int kLane, int kChains>
inline void finiteMinMaxPointF(
  const double* base, qsizetype n, qsizetype& i, double& lo, double& hi)
{
  static_assert(kLane == 0 || kLane == 1, "QPointF has exactly two lanes");
  static_assert(kChains == 1 || kChains == 2, "one or two accumulator chains");
  constexpr qsizetype kStride = 2 * kChains;
  SS_ASSERT(base != nullptr, return);
  SS_ASSERT(n >= kStride, return);

  const __m128d sign_mask          = _mm_set1_pd(-0.0);
  const __m128d plus_inf           = _mm_set1_pd(std::numeric_limits<double>::infinity());
  const __m128d minus_inf          = _mm_set1_pd(-std::numeric_limits<double>::infinity());
  __m128d acc_lo0                  = _mm_set1_pd(lo);
  __m128d acc_hi0                  = _mm_set1_pd(hi);
  [[maybe_unused]] __m128d acc_lo1 = acc_lo0;
  [[maybe_unused]] __m128d acc_hi1 = acc_hi0;
  for (; i + kStride <= n; i += kStride) {
    const __m128d v0      = pointLanePair<kLane>(base, i);
    const __m128d finite0 = _mm_cmplt_pd(_mm_andnot_pd(sign_mask, v0), plus_inf);
    acc_lo0               = _mm_min_pd(_mm_blendv_pd(plus_inf, v0, finite0), acc_lo0);
    acc_hi0               = _mm_max_pd(_mm_blendv_pd(minus_inf, v0, finite0), acc_hi0);
    if constexpr (kChains == 2) {
      const __m128d v1      = pointLanePair<kLane>(base, i + 2);
      const __m128d finite1 = _mm_cmplt_pd(_mm_andnot_pd(sign_mask, v1), plus_inf);
      acc_lo1               = _mm_min_pd(_mm_blendv_pd(plus_inf, v1, finite1), acc_lo1);
      acc_hi1               = _mm_max_pd(_mm_blendv_pd(minus_inf, v1, finite1), acc_hi1);
    }
  }

  if constexpr (kChains == 2) {
    acc_lo0 = _mm_min_pd(acc_lo1, acc_lo0);
    acc_hi0 = _mm_max_pd(acc_hi1, acc_hi0);
  }

  lo = _mm_cvtsd_f64(_mm_min_sd(_mm_unpackhi_pd(acc_lo0, acc_lo0), acc_lo0));
  hi = _mm_cvtsd_f64(_mm_max_sd(_mm_unpackhi_pd(acc_hi0, acc_hi0), acc_hi0));
}

/**
 * @brief simdAsciiDots16 over one 128-bit block.
 */
inline void asciiDots16(const quint8* src, char16_t* out)
{
  SS_ASSERT(src != nullptr, return);
  SS_ASSERT(out != nullptr, return);

  const __m128i block     = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
  const __m128i low       = _mm_set1_epi8(0x20);
  const __m128i high      = _mm_set1_epi8(0x7E);
  const __m128i ge_low    = _mm_cmpeq_epi8(_mm_max_epu8(block, low), block);
  const __m128i le_high   = _mm_cmpeq_epi8(_mm_min_epu8(block, high), block);
  const __m128i printable = _mm_and_si128(ge_low, le_high);
  const __m128i selected  = _mm_blendv_epi8(_mm_set1_epi8('.'), block, printable);
  const __m128i zero      = _mm_setzero_si128();
  _mm_storeu_si128(reinterpret_cast<__m128i*>(out), _mm_unpacklo_epi8(selected, zero));
  _mm_storeu_si128(reinterpret_cast<__m128i*>(out + 8), _mm_unpackhi_epi8(selected, zero));
}

}  // namespace SimdSse4
#endif

//--------------------------------------------------------------------------------------------------
// NEON lane (aarch64)
//--------------------------------------------------------------------------------------------------

#if defined(SS_SIMD_NEON)
namespace SimdNeon {

/**
 * @brief Two-wide windowedRealSpan; leaves @p i at the first unprocessed element.
 */
inline void windowedRealSpan(const double* src,
                             const float* win,
                             float* out,
                             std::size_t n,
                             double offset,
                             double scale,
                             std::size_t& i)
{
  SS_ASSERT(src != nullptr || n == 0, return);
  SS_ASSERT(win != nullptr || n == 0, return);

  const float64x2_t plus_inf = vdupq_n_f64(std::numeric_limits<double>::infinity());
  const float64x2_t v_offset = vdupq_n_f64(offset);
  const float64x2_t v_scale  = vdupq_n_f64(scale);
  for (; i + 2 <= n; i += 2) {
    const float64x2_t raw    = vld1q_f64(src + i);
    const uint64x2_t finite  = vcltq_f64(vabsq_f64(raw), plus_inf);
    const float64x2_t scaled = vmulq_f64(vaddq_f64(raw, v_offset), v_scale);
    const uint64x2_t bits    = vandq_u64(vreinterpretq_u64_f64(scaled), finite);
    const float32x2_t v      = vcvt_f32_f64(vreinterpretq_f64_u64(bits));
    vst1_f32(out + i, vmul_f32(v, vld1_f32(win + i)));
  }
}

/**
 * @brief Two-wide interleaveSpan; leaves @p i at the first unprocessed element.
 */
inline void interleaveSpan(
  const double* xs, const double* ys, double* out, qsizetype n, qsizetype& i)
{
  SS_ASSERT(xs != nullptr || n == 0, return);
  SS_ASSERT(ys != nullptr || n == 0, return);

  for (; i + 2 <= n; i += 2) {
    float64x2x2_t pair;
    pair.val[0] = vld1q_f64(xs + i);
    pair.val[1] = vld1q_f64(ys + i);
    vst2q_f64(out + 2 * i, pair);
  }
}

/**
 * @brief Four-wide widenF32Span; leaves @p i at the first unprocessed element.
 */
inline void widenF32Span(const float* src, double* out, std::size_t n, std::size_t& i)
{
  SS_ASSERT(src != nullptr || n == 0, return);
  SS_ASSERT(out != nullptr || n == 0, return);

  for (; i + 4 <= n; i += 4) {
    const float32x4_t v = vld1q_f32(src + i);
    vst1q_f64(out + i, vcvt_f64_f32(vget_low_f32(v)));
    vst1q_f64(out + i + 2, vcvt_f64_f32(vget_high_f32(v)));
  }
}

/**
 * @brief 16-byte blocks of simdForEachByteMatch; false when @p onMatch aborted the scan.
 */
template<typename OnMatch>
[[nodiscard]] inline bool forEachByteMatch(
  const char* data, qsizetype len, char needle, OnMatch&& onMatch, qsizetype& i)
{
  SS_ASSERT(data != nullptr || len == 0, return true);
  SS_ASSERT(i >= 0 && i <= len, return true);

  const uint8x16_t pattern = vdupq_n_u8(static_cast<uint8_t>(needle));
  for (; i + 16 <= len; i += 16) {
    const uint8x16_t block  = vld1q_u8(reinterpret_cast<const uint8_t*>(data + i));
    const uint8x8_t nibbles = vshrn_n_u16(vreinterpretq_u16_u8(vceqq_u8(block, pattern)), 4);
    uint64_t mask           = vget_lane_u64(vreinterpret_u64_u8(nibbles), 0);
    for (int b = 0; b < 16 && mask != 0; ++b) {
      const int bit       = std::countr_zero(mask);
      const qsizetype pos = i + (bit >> 2);
      if (!onMatch(pos))
        return false;

      mask &= ~(UINT64_C(0xF) << (bit & ~3));
    }
  }

  return true;
}

/**
 * @brief 16-byte blocks of simdFindAnyByte; the first hit index, or -1 with @p i at the first
 *        unprocessed byte.
 */
[[nodiscard]] inline qsizetype findAnyByte(
  const char* data, qsizetype len, const quint8* needles, int count, qsizetype& i)
{
  constexpr int kMaxNeedles = 8;
  SS_ASSERT(data != nullptr || len == 0, return -1);
  SS_ASSERT(needles != nullptr, return -1);
  SS_ASSERT(count >= 1 && count <= kMaxNeedles, return -1);

  uint8x16_t patterns[kMaxNeedles];
  for (int k = 0; k < count && k < kMaxNeedles; ++k)
    patterns[k] = vdupq_n_u8(needles[k]);

  for (; i + 16 <= len; i += 16) {
    const uint8x16_t block = vld1q_u8(reinterpret_cast<const uint8_t*>(data + i));
    uint8x16_t hit         = vceqq_u8(block, patterns[0]);
    for (int k = 1; k < count && k < kMaxNeedles; ++k)
      hit = vorrq_u8(hit, vceqq_u8(block, patterns[k]));

    const uint8x8_t nibbles = vshrn_n_u16(vreinterpretq_u16_u8(hit), 4);
    const uint64_t mask     = vget_lane_u64(vreinterpret_u64_u8(nibbles), 0);
    if (mask != 0)
      return i + (std::countr_zero(mask) >> 2);
  }

  return -1;
}

/**
 * @brief 16-byte blocks of simdWidenAscii; returns the OR of every byte's high bit.
 */
[[nodiscard]] inline quint64 widenAscii(const char* src,
                                        char16_t* out,
                                        std::size_t n,
                                        std::size_t& i)
{
  SS_ASSERT(src != nullptr || n == 0, return 0);
  SS_ASSERT(out != nullptr || n == 0, return 0);

  uint8x16_t acc = vdupq_n_u8(0);
  for (; i + 16 <= n; i += 16) {
    const uint8x16_t block = vld1q_u8(reinterpret_cast<const uint8_t*>(src + i));
    acc                    = vorrq_u8(acc, block);
    vst1q_u16(reinterpret_cast<uint16_t*>(out + i), vmovl_u8(vget_low_u8(block)));
    vst1q_u16(reinterpret_cast<uint16_t*>(out + i + 8), vmovl_u8(vget_high_u8(block)));
  }

  return (vmaxvq_u8(acc) & 0x80u);
}

/**
 * @brief Four-wide power stage of simdPowerSpectrumDb.
 */
inline void powerSpectrum(
  const float* interleaved, float* out, std::size_t n, float invNorm, float epsSq, std::size_t& i)
{
  SS_ASSERT(interleaved != nullptr || n == 0, return);
  SS_ASSERT(out != nullptr || n == 0, return);

  const float32x4_t v_norm = vdupq_n_f32(invNorm);
  const float32x4_t v_eps  = vdupq_n_f32(epsSq);
  for (; i + 4 <= n; i += 4) {
    const float32x4x2_t c = vld2q_f32(interleaved + 2 * i);
    const float32x4_t sum = vaddq_f32(vmulq_f32(c.val[0], c.val[0]), vmulq_f32(c.val[1], c.val[1]));
    vst1q_f32(out + i, vmaxq_f32(v_eps, vmulq_f32(sum, v_norm)));
  }
}

/**
 * @brief The NEON select-min step, acc = (v < acc) ? v : acc, shared by the chained loops and
 *        their folds so both use one operand order.
 */
[[nodiscard]] inline float64x2_t selectMin(float64x2_t v, float64x2_t acc)
{
  return vbslq_f64(vcltq_f64(v, acc), v, acc);
}

/**
 * @brief The NEON select-max step, acc = (v > acc) ? v : acc.
 */
[[nodiscard]] inline float64x2_t selectMax(float64x2_t v, float64x2_t acc)
{
  return vbslq_f64(vcgtq_f64(v, acc), v, acc);
}

/**
 * @brief Two-wide simdMinF64 body for n >= 2 * kChains: kChains independent accumulators
 *        (spec 0082), folded chain 1 into chain 0 with the loop's own operand order. Legal
 *        because min is order-independent and NaN never wins in any chain.
 */
template<int kChains>
inline void minF64(const double* p, std::size_t n, std::size_t& i, double& lo)
{
  static_assert(kChains == 1 || kChains == 2, "one or two accumulator chains");
  constexpr std::size_t kStride = 2 * kChains;
  SS_ASSERT(p != nullptr, return);
  SS_ASSERT(n >= kStride, return);

  float64x2_t acc0                  = vdupq_n_f64(p[0]);
  [[maybe_unused]] float64x2_t acc1 = acc0;
  for (i = 0; i + kStride <= n; i += kStride) {
    acc0 = selectMin(vld1q_f64(p + i), acc0);
    if constexpr (kChains == 2)
      acc1 = selectMin(vld1q_f64(p + i + 2), acc1);
  }

  if constexpr (kChains == 2)
    acc0 = selectMin(acc1, acc0);

  const double lane0 = vgetq_lane_f64(acc0, 0);
  const double lane1 = vgetq_lane_f64(acc0, 1);
  lo                 = (lane1 < lane0) ? lane1 : lane0;
}

/**
 * @brief Two-wide simdMaxF64 body for n >= 2 * kChains, the maximum twin of minF64.
 */
template<int kChains>
inline void maxF64(const double* p, std::size_t n, std::size_t& i, double& hi)
{
  static_assert(kChains == 1 || kChains == 2, "one or two accumulator chains");
  constexpr std::size_t kStride = 2 * kChains;
  SS_ASSERT(p != nullptr, return);
  SS_ASSERT(n >= kStride, return);

  float64x2_t acc0                  = vdupq_n_f64(p[0]);
  [[maybe_unused]] float64x2_t acc1 = acc0;
  for (i = 0; i + kStride <= n; i += kStride) {
    acc0 = selectMax(vld1q_f64(p + i), acc0);
    if constexpr (kChains == 2)
      acc1 = selectMax(vld1q_f64(p + i + 2), acc1);
  }

  if constexpr (kChains == 2)
    acc0 = selectMax(acc1, acc0);

  const double lane0 = vgetq_lane_f64(acc0, 0);
  const double lane1 = vgetq_lane_f64(acc0, 1);
  hi                 = (lane1 > lane0) ? lane1 : lane0;
}

/**
 * @brief Two-wide simdMinMaxF64 body for n >= 2 * kChains, both sides chained like minF64.
 */
template<int kChains>
inline void minMaxF64(const double* p, std::size_t n, std::size_t& i, double& lo, double& hi)
{
  static_assert(kChains == 1 || kChains == 2, "one or two accumulator chains");
  constexpr std::size_t kStride = 2 * kChains;
  SS_ASSERT(p != nullptr, return);
  SS_ASSERT(n >= kStride, return);

  float64x2_t acc_lo0                  = vdupq_n_f64(p[0]);
  float64x2_t acc_hi0                  = acc_lo0;
  [[maybe_unused]] float64x2_t acc_lo1 = acc_lo0;
  [[maybe_unused]] float64x2_t acc_hi1 = acc_lo0;
  for (i = 0; i + kStride <= n; i += kStride) {
    const float64x2_t v0 = vld1q_f64(p + i);
    acc_lo0              = selectMin(v0, acc_lo0);
    acc_hi0              = selectMax(v0, acc_hi0);
    if constexpr (kChains == 2) {
      const float64x2_t v1 = vld1q_f64(p + i + 2);
      acc_lo1              = selectMin(v1, acc_lo1);
      acc_hi1              = selectMax(v1, acc_hi1);
    }
  }

  if constexpr (kChains == 2) {
    acc_lo0 = selectMin(acc_lo1, acc_lo0);
    acc_hi0 = selectMax(acc_hi1, acc_hi0);
  }

  const double lo0 = vgetq_lane_f64(acc_lo0, 0);
  const double lo1 = vgetq_lane_f64(acc_lo0, 1);
  const double hi0 = vgetq_lane_f64(acc_hi0, 0);
  const double hi1 = vgetq_lane_f64(acc_hi0, 1);
  lo               = (lo1 < lo0) ? lo1 : lo0;
  hi               = (hi1 > hi0) ? hi1 : hi0;
}

/**
 * @brief Two-point simdFiniteMinMaxPointF body for n >= 2 * kChains, chained like minMaxF64.
 */
template<int kLane, int kChains>
inline void finiteMinMaxPointF(
  const double* base, qsizetype n, qsizetype& i, double& lo, double& hi)
{
  static_assert(kLane == 0 || kLane == 1, "QPointF has exactly two lanes");
  static_assert(kChains == 1 || kChains == 2, "one or two accumulator chains");
  constexpr qsizetype kStride = 2 * kChains;
  SS_ASSERT(base != nullptr, return);
  SS_ASSERT(n >= kStride, return);

  const float64x2_t plus_inf           = vdupq_n_f64(std::numeric_limits<double>::infinity());
  const float64x2_t minus_inf          = vnegq_f64(plus_inf);
  float64x2_t acc_lo0                  = vdupq_n_f64(lo);
  float64x2_t acc_hi0                  = vdupq_n_f64(hi);
  [[maybe_unused]] float64x2_t acc_lo1 = acc_lo0;
  [[maybe_unused]] float64x2_t acc_hi1 = acc_hi0;
  for (; i + kStride <= n; i += kStride) {
    const float64x2_t v0     = vld2q_f64(base + 2 * i).val[kLane];
    const uint64x2_t finite0 = vcltq_f64(vabsq_f64(v0), plus_inf);
    acc_lo0                  = selectMin(vbslq_f64(finite0, v0, plus_inf), acc_lo0);
    acc_hi0                  = selectMax(vbslq_f64(finite0, v0, minus_inf), acc_hi0);
    if constexpr (kChains == 2) {
      const float64x2_t v1     = vld2q_f64(base + 2 * i + 4).val[kLane];
      const uint64x2_t finite1 = vcltq_f64(vabsq_f64(v1), plus_inf);
      acc_lo1                  = selectMin(vbslq_f64(finite1, v1, plus_inf), acc_lo1);
      acc_hi1                  = selectMax(vbslq_f64(finite1, v1, minus_inf), acc_hi1);
    }
  }

  if constexpr (kChains == 2) {
    acc_lo0 = selectMin(acc_lo1, acc_lo0);
    acc_hi0 = selectMax(acc_hi1, acc_hi0);
  }

  const double lo0 = vgetq_lane_f64(acc_lo0, 0);
  const double lo1 = vgetq_lane_f64(acc_lo0, 1);
  const double hi0 = vgetq_lane_f64(acc_hi0, 0);
  const double hi1 = vgetq_lane_f64(acc_hi0, 1);
  lo               = (lo1 < lo0) ? lo1 : lo0;
  hi               = (hi1 > hi0) ? hi1 : hi0;
}

/**
 * @brief simdAsciiDots16 over one 128-bit block.
 */
inline void asciiDots16(const quint8* src, char16_t* out)
{
  SS_ASSERT(src != nullptr, return);
  SS_ASSERT(out != nullptr, return);

  const uint8x16_t block = vld1q_u8(src);
  const uint8x16_t printable =
    vandq_u8(vcgeq_u8(block, vdupq_n_u8(0x20)), vcleq_u8(block, vdupq_n_u8(0x7E)));
  const uint8x16_t selected = vbslq_u8(printable, block, vdupq_n_u8('.'));
  vst1q_u16(reinterpret_cast<uint16_t*>(out), vmovl_u8(vget_low_u8(selected)));
  vst1q_u16(reinterpret_cast<uint16_t*>(out) + 8, vmovl_u8(vget_high_u8(selected)));
}

}  // namespace SimdNeon
#endif

//--------------------------------------------------------------------------------------------------
// Internal contiguous-span kernels
//--------------------------------------------------------------------------------------------------

namespace SimdDetail {

/**
 * @brief Windowed FFT staging over one contiguous span: out[i] = finite(src[i]) ?
 *        f32((src[i]+offset)*scale)*win[i] : 0. Per-lane ops replicate the scalar order
 *        exactly (add, mul in f64, convert, mul in f32).
 */
inline void windowedRealSpan(
  const double* src, const float* win, float* out, std::size_t n, double offset, double scale)
{
  SS_ASSERT(src != nullptr || n == 0, return);
  SS_ASSERT(win != nullptr || n == 0, return);

  std::size_t i                                 = 0;
  [[maybe_unused]] const ::DSP::SimdLevel level = ::DSP::activeSimdLevel();

#if defined(SS_SIMD_X86)
  if (level == ::DSP::SimdLevel::Avx2 && n >= 8)
    SimdAvx2::windowedRealSpan(src, win, out, n, offset, scale, i);

  if (level != ::DSP::SimdLevel::Scalar)
    SimdSse4::windowedRealSpan(src, win, out, n, offset, scale, i);
#elif defined(SS_SIMD_NEON)
  if (level == ::DSP::SimdLevel::Neon)
    SimdNeon::windowedRealSpan(src, win, out, n, offset, scale, i);
#endif

  for (; i < n; ++i) {
    const double raw = src[i];
    const float v    = std::isfinite(raw) ? static_cast<float>((raw + offset) * scale) : 0.0f;
    out[i]           = v * win[i];
  }
}

/**
 * @brief Interleaves two contiguous f64 spans into (x, y) pairs: out[2i] = xs[i],
 *        out[2i+1] = ys[i]. Pure copy, bit-exact by construction.
 */
inline void interleaveSpan(const double* xs, const double* ys, double* out, qsizetype n)
{
  SS_ASSERT(xs != nullptr || n == 0, return);
  SS_ASSERT(ys != nullptr || n == 0, return);

  qsizetype i                                   = 0;
  [[maybe_unused]] const ::DSP::SimdLevel level = ::DSP::activeSimdLevel();

#if defined(SS_SIMD_X86)
  if (level == ::DSP::SimdLevel::Avx2 && n >= 4)
    SimdAvx2::interleaveSpan(xs, ys, out, n, i);

  if (level != ::DSP::SimdLevel::Scalar)
    SimdSse4::interleaveSpan(xs, ys, out, n, i);
#elif defined(SS_SIMD_NEON)
  if (level == ::DSP::SimdLevel::Neon)
    SimdNeon::interleaveSpan(xs, ys, out, n, i);
#endif

  for (; i < n; ++i) {
    out[2 * i]     = xs[i];
    out[2 * i + 1] = ys[i];
  }
}

/**
 * @brief Spreads the four bytes of @p v into the four 16-bit lanes of the result, so a
 *        little-endian store lands one UTF-16 code unit per source byte.
 */
[[nodiscard]] inline quint64 widenFourBytes(quint32 v)
{
  return (static_cast<quint64>(v) & 0xFF) | ((static_cast<quint64>(v) & 0xFF00) << 8)
       | ((static_cast<quint64>(v) & 0xFF0000) << 16)
       | ((static_cast<quint64>(v) & 0xFF000000) << 24);
}

/**
 * @brief Widens a contiguous f32 span to f64. Exact by construction: every finite or
 *        non-finite f32 has an exact f64 image, so the converts carry no rounding.
 */
inline void widenF32Span(const float* src, double* out, std::size_t n)
{
  SS_ASSERT(src != nullptr || n == 0, return);
  SS_ASSERT(out != nullptr || n == 0, return);

  std::size_t i                                 = 0;
  [[maybe_unused]] const ::DSP::SimdLevel level = ::DSP::activeSimdLevel();

#if defined(SS_SIMD_X86)
  if (level == ::DSP::SimdLevel::Avx2 && n >= 8)
    SimdAvx2::widenF32Span(src, out, n, i);

  if (level != ::DSP::SimdLevel::Scalar)
    SimdSse4::widenF32Span(src, out, n, i);
#elif defined(SS_SIMD_NEON)
  if (level == ::DSP::SimdLevel::Neon)
    SimdNeon::widenF32Span(src, out, n, i);
#endif

  for (; i < n; ++i)
    out[i] = static_cast<double>(src[i]);
}

}  // namespace SimdDetail

//--------------------------------------------------------------------------------------------------
// Byte scanning
//--------------------------------------------------------------------------------------------------

/**
 * @brief Invokes @p onMatch(pos) for every occurrence of @p needle in ascending order; a false
 *        return from the callback aborts the scan and makes this function return false.
 *        Allocation-free and bounded; safe on the 256 kHz span lane.
 */
template<typename OnMatch>
[[nodiscard]] inline bool simdForEachByteMatch(const char* data,
                                               qsizetype len,
                                               char needle,
                                               OnMatch&& onMatch)
{
  SS_ASSERT(data != nullptr || len == 0, return true);
  SS_ASSERT(len >= 0, return true);

  qsizetype i                                   = 0;
  [[maybe_unused]] const ::DSP::SimdLevel level = ::DSP::activeSimdLevel();

#if defined(SS_SIMD_X86)
  if (level == ::DSP::SimdLevel::Avx2 && len >= 32
      && !SimdAvx2::forEachByteMatch(data, len, needle, onMatch, i))
    return false;

  if (level != ::DSP::SimdLevel::Scalar
      && !SimdSse4::forEachByteMatch(data, len, needle, onMatch, i))
    return false;
#elif defined(SS_SIMD_NEON)
  if (level == ::DSP::SimdLevel::Neon && !SimdNeon::forEachByteMatch(data, len, needle, onMatch, i))
    return false;
#endif

  for (; i < len; ++i)
    if (data[i] == needle && !onMatch(i))
      return false;

  return true;
}

/**
 * @brief Returns the index of the first byte that matches any of @p count needles (max 8),
 *        or @p len when none matches. Byte-exact equivalent of a per-byte membership scan.
 */
[[nodiscard]] inline qsizetype simdFindAnyByte(const char* data,
                                               qsizetype len,
                                               const quint8* needles,
                                               int count)
{
  constexpr int kMaxNeedles = 8;
  SS_ASSERT(data != nullptr || len == 0, return len);
  SS_ASSERT(needles != nullptr, return len);
  SS_ASSERT(count >= 1 && count <= kMaxNeedles, return len);

  qsizetype i                                   = 0;
  [[maybe_unused]] const ::DSP::SimdLevel level = ::DSP::activeSimdLevel();

#if defined(SS_SIMD_X86)
  if (level == ::DSP::SimdLevel::Avx2 && len >= 32) {
    const qsizetype hit = SimdAvx2::findAnyByte(data, len, needles, count, i);
    if (hit >= 0)
      return hit;
  }

  if (level != ::DSP::SimdLevel::Scalar) {
    const qsizetype hit = SimdSse4::findAnyByte(data, len, needles, count, i);
    if (hit >= 0)
      return hit;
  }
#elif defined(SS_SIMD_NEON)
  if (level == ::DSP::SimdLevel::Neon) {
    const qsizetype hit = SimdNeon::findAnyByte(data, len, needles, count, i);
    if (hit >= 0)
      return hit;
  }
#endif

  for (; i < len; ++i) {
    const quint8 b = static_cast<quint8>(data[i]);
    for (int k = 0; k < count; ++k)
      if (b == needles[k])
        return i;
  }

  return len;
}

/**
 * @brief Widens @p n bytes to UTF-16 code units one-for-one and reports whether every byte was
 *        ASCII. All @p n units are written either way, so a false return lets the caller redo
 *        the span through a real UTF-8 decoder without first restoring anything.
 */
[[nodiscard]] inline bool simdWidenAscii(const char* src, char16_t* out, std::size_t n)
{
  SS_ASSERT(src != nullptr || n == 0, return true);
  SS_ASSERT(out != nullptr || n == 0, return true);

  std::size_t i                                 = 0;
  quint64 highBits                              = 0;
  [[maybe_unused]] const ::DSP::SimdLevel level = ::DSP::activeSimdLevel();

#if defined(SS_SIMD_X86)
  if (level == ::DSP::SimdLevel::Avx2 && n >= 32)
    highBits |= SimdAvx2::widenAscii(src, out, n, i);

  if (level != ::DSP::SimdLevel::Scalar)
    highBits |= SimdSse4::widenAscii(src, out, n, i);
#elif defined(SS_SIMD_NEON)
  if (level == ::DSP::SimdLevel::Neon)
    highBits |= SimdNeon::widenAscii(src, out, n, i);
#endif

#if !defined(SS_SIMD_DISABLE)
  if constexpr (std::endian::native == std::endian::little)
    for (; i + 8 <= n; i += 8) {
      quint64 v = 0;
      std::memcpy(&v, src + i, sizeof(v));
      highBits |= (v & UINT64_C(0x8080808080808080));

      const quint64 lo = SimdDetail::widenFourBytes(static_cast<quint32>(v));
      const quint64 hi = SimdDetail::widenFourBytes(static_cast<quint32>(v >> 32));
      std::memcpy(out + i, &lo, sizeof(lo));
      std::memcpy(out + i + 4, &hi, sizeof(hi));
    }
#endif

  for (; i < n; ++i) {
    const auto c  = static_cast<quint8>(src[i]);
    highBits     |= (c & 0x80u);
    out[i]        = static_cast<char16_t>(c);
  }

  return highBits == 0;
}

/**
 * @brief Extracts one channel of an interleaved f32 block into f64. A single-channel block is
 *        a contiguous widen; a multi-channel one is a strided gather that stays scalar at every
 *        level, since no lane improves it.
 */
inline void simdDeinterleaveToF64(
  const float* src, std::size_t frames, int channels, int channel, double* out)
{
  SS_ASSERT(src != nullptr || frames == 0, return);
  SS_ASSERT(out != nullptr || frames == 0, return);
  SS_ASSERT(channels >= 1, return);
  SS_ASSERT(channel >= 0 && channel < channels, return);

  if (channels == 1) {
    SimdDetail::widenF32Span(src, out, frames);
    return;
  }

  for (std::size_t i = 0; i < frames; ++i)
    out[i] = static_cast<double>(
      src[i * static_cast<std::size_t>(channels) + static_cast<std::size_t>(channel)]);
}

/**
 * @brief Converts an interleaved complex-f32 spectrum to display dB:
 *        out[i] = max(10*log10(max((re*re + im*im) * @p invNorm, @p epsSq)), @p floorDb).
 *        Only the power stage vectorizes -- same op order, max operands ordered so NaN
 *        propagates like std::max -- since no bit-exact vector log10 exists.
 */
inline void simdPowerSpectrumDb(
  const float* interleaved, float* out, std::size_t n, float invNorm, float epsSq, float floorDb)
{
  SS_ASSERT(interleaved != nullptr || n == 0, return);
  SS_ASSERT(out != nullptr || n == 0, return);

  std::size_t i                                 = 0;
  [[maybe_unused]] const ::DSP::SimdLevel level = ::DSP::activeSimdLevel();

#if defined(SS_SIMD_X86)
  if (level == ::DSP::SimdLevel::Avx2 && n >= 8)
    SimdAvx2::powerSpectrum(interleaved, out, n, invNorm, epsSq, i);

  if (level != ::DSP::SimdLevel::Scalar)
    SimdSse4::powerSpectrum(interleaved, out, n, invNorm, epsSq, i);
#elif defined(SS_SIMD_NEON)
  if (level == ::DSP::SimdLevel::Neon)
    SimdNeon::powerSpectrum(interleaved, out, n, invNorm, epsSq, i);
#endif

  for (; i < n; ++i) {
    const float re = interleaved[2 * i];
    const float im = interleaved[2 * i + 1];
    out[i]         = std::max((re * re + im * im) * invNorm, epsSq);
  }

  for (std::size_t k = 0; k < n; ++k)
    out[k] = std::max(10.0f * std::log10(out[k]), floorDb);
}

//--------------------------------------------------------------------------------------------------
// f64 reductions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Minimum of @p n doubles with the scalar predicate acc = (v < acc) ? v : acc seeded
 *        from p[0]: NaN elements never win and a NaN seed is sticky, exactly like the loop
 *        it replaces. Requires n >= 1.
 */
[[nodiscard]] inline double simdMinF64(const double* p, std::size_t n)
{
  SS_ASSERT(p != nullptr, return std::numeric_limits<double>::infinity());
  SS_ASSERT(n > 0, return std::numeric_limits<double>::infinity());

  std::size_t i                                 = 1;
  double lo                                     = p[0];
  [[maybe_unused]] const ::DSP::SimdLevel level = ::DSP::activeSimdLevel();

#if defined(SS_SIMD_X86)
  if (level == ::DSP::SimdLevel::Avx2 && n >= 8)
    SimdAvx2::minF64<2>(p, n, i, lo);
  else if (level != ::DSP::SimdLevel::Scalar && n >= 4)
    SimdSse4::minF64<2>(p, n, i, lo);
#elif defined(SS_SIMD_NEON)
  if (level == ::DSP::SimdLevel::Neon && n >= 4)
    SimdNeon::minF64<2>(p, n, i, lo);
#endif

  for (; i < n; ++i)
    if (p[i] < lo)
      lo = p[i];

  return lo;
}

/**
 * @brief Maximum twin of simdMinF64: acc = (v > acc) ? v : acc seeded from p[0], NaN-sticky,
 *        n >= 1 required.
 */
[[nodiscard]] inline double simdMaxF64(const double* p, std::size_t n)
{
  SS_ASSERT(p != nullptr, return -std::numeric_limits<double>::infinity());
  SS_ASSERT(n > 0, return -std::numeric_limits<double>::infinity());

  std::size_t i                                 = 1;
  double hi                                     = p[0];
  [[maybe_unused]] const ::DSP::SimdLevel level = ::DSP::activeSimdLevel();

#if defined(SS_SIMD_X86)
  if (level == ::DSP::SimdLevel::Avx2 && n >= 8)
    SimdAvx2::maxF64<2>(p, n, i, hi);
  else if (level != ::DSP::SimdLevel::Scalar && n >= 4)
    SimdSse4::maxF64<2>(p, n, i, hi);
#elif defined(SS_SIMD_NEON)
  if (level == ::DSP::SimdLevel::Neon && n >= 4)
    SimdNeon::maxF64<2>(p, n, i, hi);
#endif

  for (; i < n; ++i)
    if (p[i] > hi)
      hi = p[i];

  return hi;
}

/**
 * @brief One-pass min + max over @p n doubles, same predicate/seed/NaN semantics as
 *        simdMinF64 / simdMaxF64. Requires n >= 1.
 */
inline void simdMinMaxF64(const double* p, std::size_t n, double& lo, double& hi)
{
  SS_ASSERT(p != nullptr, {
    lo = std::numeric_limits<double>::infinity();
    hi = -lo;
    return;
  });
  SS_ASSERT(n > 0, {
    lo = std::numeric_limits<double>::infinity();
    hi = -lo;
    return;
  });

  std::size_t i                                 = 1;
  lo                                            = p[0];
  hi                                            = p[0];
  [[maybe_unused]] const ::DSP::SimdLevel level = ::DSP::activeSimdLevel();

#if defined(SS_SIMD_X86)
  if (level == ::DSP::SimdLevel::Avx2 && n >= 8)
    SimdAvx2::minMaxF64<2>(p, n, i, lo, hi);
  else if (level != ::DSP::SimdLevel::Scalar && n >= 4)
    SimdSse4::minMaxF64<2>(p, n, i, lo, hi);
#elif defined(SS_SIMD_NEON)
  if (level == ::DSP::SimdLevel::Neon && n >= 4)
    SimdNeon::minMaxF64<2>(p, n, i, lo, hi);
#endif

  for (; i < n; ++i) {
    if (p[i] < lo)
      lo = p[i];

    if (p[i] > hi)
      hi = p[i];
  }
}

//--------------------------------------------------------------------------------------------------
// QPointF lane reductions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Accumulates the finite min/max of one QPointF lane (kLane 0 = x, 1 = y) into the
 *        caller-seeded @p lo / @p hi: non-finite values are skipped, exactly like the
 *        isfinite-guarded scalar loops this replaces. Callers keep their sentinel-seed
 *        convention (lo > hi afterwards means no finite value was seen).
 */
template<int kLane>
inline void simdFiniteMinMaxPointF(const QPointF* pts, qsizetype n, double& lo, double& hi)
{
  static_assert(kLane == 0 || kLane == 1, "QPointF has exactly two lanes");
  static_assert(sizeof(QPointF) == 2 * sizeof(double), "QPointF must pack two doubles");
  SS_ASSERT(pts != nullptr || n == 0, return);
  SS_ASSERT(n >= 0, return);

  const double* base                            = reinterpret_cast<const double*>(pts);
  qsizetype i                                   = 0;
  [[maybe_unused]] const ::DSP::SimdLevel level = ::DSP::activeSimdLevel();

#if defined(SS_SIMD_X86)
  if (level == ::DSP::SimdLevel::Avx2 && n >= 8)
    SimdAvx2::finiteMinMaxPointF<kLane, 2>(base, n, i, lo, hi);
  else if (level != ::DSP::SimdLevel::Scalar && n >= 4)
    SimdSse4::finiteMinMaxPointF<kLane, 2>(base, n, i, lo, hi);
#elif defined(SS_SIMD_NEON)
  if (level == ::DSP::SimdLevel::Neon && n >= 4)
    SimdNeon::finiteMinMaxPointF<kLane, 2>(base, n, i, lo, hi);
#endif

  for (; i < n; ++i) {
    const double v = base[2 * i + kLane];
    if (std::isfinite(v)) {
      lo = (v < lo) ? v : lo;
      hi = (v > hi) ? v : hi;
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Ring-buffer bulk transforms
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fills @p n windowed f32 samples from a pow2-masked f64 ring for kiss_fftr staging:
 *        out[i] = finite(r) ? f32((r+offset)*scale)*window[i] : 0, with the ring resolved into
 *        at most two contiguous spans so the inner loops vectorize. Requires n <= mask + 1.
 */
inline void simdWindowedRealFill(const double* ring,
                                 std::size_t front,
                                 std::size_t mask,
                                 std::size_t n,
                                 double offset,
                                 double scale,
                                 const float* window,
                                 float* out)
{
  SS_ASSERT(ring != nullptr || n == 0, return);
  SS_ASSERT(out != nullptr || n == 0, return);
  SS_ASSERT(((mask + 1) & mask) == 0, return);
  SS_ASSERT(n <= mask + 1, return);
  SS_ASSERT(front <= mask, return);

  const std::size_t n0 = std::min(n, mask + 1 - front);
  SimdDetail::windowedRealSpan(ring + front, window, out, n0, offset, scale);
  SimdDetail::windowedRealSpan(ring, window + n0, out + n0, n - n0, offset, scale);
}

/**
 * @brief Gathers @p n logical elements from two pow2-masked f64 rings into interleaved
 *        QPointF (x, y) pairs, walking both rings segment-wise so every inner copy is a
 *        contiguous SIMD interleave. Pure copy: bit-exact versus the masked scalar loop.
 */
inline void simdRingsToPoints(const double* xs,
                              std::size_t xfront,
                              std::size_t xmask,
                              const double* ys,
                              std::size_t yfront,
                              std::size_t ymask,
                              qsizetype n,
                              QPointF* out)
{
  static_assert(sizeof(QPointF) == 2 * sizeof(double), "QPointF must pack two doubles");
  SS_ASSERT(xs != nullptr || n == 0, return);
  SS_ASSERT(ys != nullptr || n == 0, return);
  SS_ASSERT(out != nullptr || n == 0, return);
  SS_ASSERT(((xmask + 1) & xmask) == 0, return);
  SS_ASSERT(((ymask + 1) & ymask) == 0, return);

  double* dst    = reinterpret_cast<double*>(out);
  std::size_t xi = xfront;
  std::size_t yi = yfront;
  for (qsizetype done = 0; done < n;) {
    const auto x_run      = static_cast<qsizetype>(xmask + 1 - xi);
    const auto y_run      = static_cast<qsizetype>(ymask + 1 - yi);
    const qsizetype chunk = std::min({n - done, x_run, y_run});
    SS_ASSERT(chunk > 0, return);
    SimdDetail::interleaveSpan(xs + xi, ys + yi, dst + 2 * done, chunk);
    xi    = (xi + static_cast<std::size_t>(chunk)) & xmask;
    yi    = (yi + static_cast<std::size_t>(chunk)) & ymask;
    done += chunk;
  }
}

//--------------------------------------------------------------------------------------------------
// Text formatting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps 16 raw bytes to 16 UTF-16 code units for a hex-dump ASCII column: printable
 *        bytes (0x20..0x7E, the "C"-locale isprint range) pass through, everything else
 *        becomes '.'. A fixed 16-byte kernel cannot amortize a call boundary, so the AVX2 level
 *        runs the inline 128-bit lane here.
 */
inline void simdAsciiDots16(const quint8* src, char16_t* out)
{
  SS_ASSERT(src != nullptr, return);
  SS_ASSERT(out != nullptr, return);

  [[maybe_unused]] const ::DSP::SimdLevel level = ::DSP::activeSimdLevel();

#if defined(SS_SIMD_X86)
  if (level != ::DSP::SimdLevel::Scalar) {
    SimdSse4::asciiDots16(src, out);
    return;
  }
#elif defined(SS_SIMD_NEON)
  if (level == ::DSP::SimdLevel::Neon) {
    SimdNeon::asciiDots16(src, out);
    return;
  }
#endif

  for (int i = 0; i < 16; ++i) {
    const quint8 b = src[i];
    out[i]         = (b >= 0x20 && b <= 0x7E) ? static_cast<char16_t>(b) : u'.';
  }
}

}  // namespace SS_DSP_NAMESPACE
