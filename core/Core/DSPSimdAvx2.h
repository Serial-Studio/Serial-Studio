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

#include <immintrin.h>

#include <bit>
#include <cstddef>
#include <limits>
#include <QtGlobal>

#include "Core/HotpathOptimization.h"
#include "Core/SSAssert.h"

/**
 * @file DSPSimdAvx2.h
 * @brief The AVX2 lane of every DSPSimd.h kernel (spec 0081). Each body is SS_NEVER_INLINE and
 *        SS_TARGET_AVX2, so the wide code exists only behind the runtime level check in its
 *        DSPSimd.h caller and no translation unit is compiled wide. A caller enters a body only
 *        when at least one whole 256-bit block exists; the body consumes whole blocks and hands
 *        back the index where the caller's 128-bit lane resumes for the residue, so the scalar
 *        tail is the same as the SSE lane's. Same op classes as the 128-bit lane, same per-lane
 *        order, never FMA, never a horizontal float sum; one _mm256_zeroupper() closes every body
 *        once no 256-bit value is live and before any 128-bit op, since cl.exe does not insert it.
 *        The 16-byte hex-dump kernel has no body here: it cannot amortize a call boundary.
 */

#ifndef SS_DSP_NAMESPACE
#  define SS_DSP_NAMESPACE DSP
#endif

namespace SS_DSP_NAMESPACE::SimdAvx2 {

//--------------------------------------------------------------------------------------------------
// Byte scanning
//--------------------------------------------------------------------------------------------------

/**
 * @brief 32-byte blocks of simdForEachByteMatch: invokes @p onMatch(pos) ascending, aborts on a
 *        false return (and returns false itself). @p i is left at the first unprocessed byte.
 */
template<typename OnMatch>
[[nodiscard]] SS_NEVER_INLINE SS_TARGET_AVX2 bool forEachByteMatch(
  const char* data, qsizetype len, char needle, OnMatch&& onMatch, qsizetype& i)
{
  SS_ASSERT(data != nullptr || len == 0, return true);
  SS_ASSERT(i >= 0 && i <= len, return true);

  const __m256i pattern = _mm256_set1_epi8(needle);
  for (; i + 32 <= len; i += 32) {
    const __m256i block = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data + i));
    quint32 mask = static_cast<quint32>(_mm256_movemask_epi8(_mm256_cmpeq_epi8(block, pattern)));
    for (int b = 0; b < 32 && mask != 0; ++b) {
      const qsizetype pos = i + std::countr_zero(mask);
      if (!onMatch(pos)) {
        _mm256_zeroupper();
        return false;
      }

      mask &= mask - 1;
    }
  }

  _mm256_zeroupper();
  return true;
}

/**
 * @brief 32-byte blocks of simdFindAnyByte: the index of the first byte matching any of @p count
 *        needles (max 8), or -1 with @p i at the first unprocessed byte.
 */
[[nodiscard]] SS_NEVER_INLINE SS_TARGET_AVX2 inline qsizetype findAnyByte(
  const char* data, qsizetype len, const quint8* needles, int count, qsizetype& i)
{
  constexpr int kMaxNeedles = 8;
  SS_ASSERT(data != nullptr || len == 0, return -1);
  SS_ASSERT(needles != nullptr, return -1);
  SS_ASSERT(count >= 1 && count <= kMaxNeedles, return -1);

  __m256i patterns[kMaxNeedles];
  for (int k = 0; k < count && k < kMaxNeedles; ++k)
    patterns[k] = _mm256_set1_epi8(static_cast<char>(needles[k]));

  for (; i + 32 <= len; i += 32) {
    const __m256i block = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data + i));
    __m256i hit         = _mm256_cmpeq_epi8(block, patterns[0]);
    for (int k = 1; k < count && k < kMaxNeedles; ++k)
      hit = _mm256_or_si256(hit, _mm256_cmpeq_epi8(block, patterns[k]));

    const quint32 mask = static_cast<quint32>(_mm256_movemask_epi8(hit));
    if (mask != 0) {
      const qsizetype pos = i + std::countr_zero(mask);
      _mm256_zeroupper();
      return pos;
    }
  }

  _mm256_zeroupper();
  return -1;
}

/**
 * @brief 32-byte blocks of simdWidenAscii: widens bytes to UTF-16 units one-for-one and returns
 *        the OR of every byte's high bit (non-zero means a non-ASCII byte was seen). @p i is left
 *        at the first unprocessed byte.
 */
[[nodiscard]] SS_NEVER_INLINE SS_TARGET_AVX2 inline quint64 widenAscii(const char* src,
                                                                       char16_t* out,
                                                                       std::size_t n,
                                                                       std::size_t& i)
{
  SS_ASSERT(src != nullptr || n == 0, return 0);
  SS_ASSERT(out != nullptr || n == 0, return 0);

  quint64 high_bits = 0;
  for (; i + 32 <= n; i += 32) {
    const __m256i block  = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + i));
    high_bits           |= static_cast<quint32>(_mm256_movemask_epi8(block));
    const __m256i lo     = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(block));
    const __m256i hi     = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(block, 1));
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(out + i), lo);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(out + i + 16), hi);
  }

  _mm256_zeroupper();
  return high_bits;
}

//--------------------------------------------------------------------------------------------------
// Contiguous-span transforms
//--------------------------------------------------------------------------------------------------

/**
 * @brief Eight-wide SimdDetail::windowedRealSpan: out[k] = finite(src[k]) ?
 *        f32((src[k]+offset)*scale)*win[k] : 0, add and mul in f64, convert, mul in f32, per lane
 *        in that order. Both f32 stages stay 256-bit so no legacy-encoded op runs mid-loop.
 */
SS_NEVER_INLINE SS_TARGET_AVX2 inline void windowedRealSpan(const double* src,
                                                            const float* win,
                                                            float* out,
                                                            std::size_t n,
                                                            double offset,
                                                            double scale,
                                                            std::size_t& i)
{
  SS_ASSERT(src != nullptr || n == 0, return);
  SS_ASSERT(win != nullptr || n == 0, return);

  const __m256d sign_mask = _mm256_set1_pd(-0.0);
  const __m256d plus_inf  = _mm256_set1_pd(std::numeric_limits<double>::infinity());
  const __m256d v_offset  = _mm256_set1_pd(offset);
  const __m256d v_scale   = _mm256_set1_pd(scale);
  for (; i + 8 <= n; i += 8) {
    const __m256d raw_lo    = _mm256_loadu_pd(src + i);
    const __m256d raw_hi    = _mm256_loadu_pd(src + i + 4);
    const __m256d abs_lo    = _mm256_andnot_pd(sign_mask, raw_lo);
    const __m256d abs_hi    = _mm256_andnot_pd(sign_mask, raw_hi);
    const __m256d finite_lo = _mm256_cmp_pd(abs_lo, plus_inf, _CMP_LT_OS);
    const __m256d finite_hi = _mm256_cmp_pd(abs_hi, plus_inf, _CMP_LT_OS);
    const __m256d scaled_lo = _mm256_mul_pd(_mm256_add_pd(raw_lo, v_offset), v_scale);
    const __m256d scaled_hi = _mm256_mul_pd(_mm256_add_pd(raw_hi, v_offset), v_scale);
    const __m128 v_lo       = _mm256_cvtpd_ps(_mm256_and_pd(scaled_lo, finite_lo));
    const __m128 v_hi       = _mm256_cvtpd_ps(_mm256_and_pd(scaled_hi, finite_hi));
    const __m256 v          = _mm256_insertf128_ps(_mm256_castps128_ps256(v_lo), v_hi, 1);
    _mm256_storeu_ps(out + i, _mm256_mul_ps(v, _mm256_loadu_ps(win + i)));
  }

  _mm256_zeroupper();
}

/**
 * @brief Four-wide SimdDetail::interleaveSpan: out[2k] = xs[k], out[2k+1] = ys[k]. Pure copy.
 */
SS_NEVER_INLINE SS_TARGET_AVX2 inline void interleaveSpan(
  const double* xs, const double* ys, double* out, qsizetype n, qsizetype& i)
{
  SS_ASSERT(xs != nullptr || n == 0, return);
  SS_ASSERT(ys != nullptr || n == 0, return);

  for (; i + 4 <= n; i += 4) {
    const __m256d a  = _mm256_loadu_pd(xs + i);
    const __m256d b  = _mm256_loadu_pd(ys + i);
    const __m256d lo = _mm256_unpacklo_pd(a, b);
    const __m256d hi = _mm256_unpackhi_pd(a, b);
    _mm256_storeu_pd(out + 2 * i, _mm256_permute2f128_pd(lo, hi, 0x20));
    _mm256_storeu_pd(out + 2 * i + 4, _mm256_permute2f128_pd(lo, hi, 0x31));
  }

  _mm256_zeroupper();
}

/**
 * @brief Eight-wide SimdDetail::widenF32Span: exact f32 to f64 converts.
 */
SS_NEVER_INLINE SS_TARGET_AVX2 inline void widenF32Span(const float* src,
                                                        double* out,
                                                        std::size_t n,
                                                        std::size_t& i)
{
  SS_ASSERT(src != nullptr || n == 0, return);
  SS_ASSERT(out != nullptr || n == 0, return);

  for (; i + 8 <= n; i += 8) {
    const __m256 v = _mm256_loadu_ps(src + i);
    _mm256_storeu_pd(out + i, _mm256_cvtps_pd(_mm256_castps256_ps128(v)));
    _mm256_storeu_pd(out + i + 4, _mm256_cvtps_pd(_mm256_extractf128_ps(v, 1)));
  }

  _mm256_zeroupper();
}

/**
 * @brief Eight-wide power stage of simdPowerSpectrumDb: out[k] = max((re*re + im*im) * invNorm,
 *        epsSq), re*re + im*im through hadd in that operand order, max operands ordered so NaN
 *        propagates like std::max. The log10 stage stays with the caller.
 */
SS_NEVER_INLINE SS_TARGET_AVX2 inline void powerSpectrum(
  const float* interleaved, float* out, std::size_t n, float invNorm, float epsSq, std::size_t& i)
{
  SS_ASSERT(interleaved != nullptr || n == 0, return);
  SS_ASSERT(out != nullptr || n == 0, return);

  const __m256 v_norm = _mm256_set1_ps(invNorm);
  const __m256 v_eps  = _mm256_set1_ps(epsSq);
  for (; i + 8 <= n; i += 8) {
    const __m256 lo     = _mm256_loadu_ps(interleaved + 2 * i);
    const __m256 hi     = _mm256_loadu_ps(interleaved + 2 * i + 8);
    const __m256 paired = _mm256_hadd_ps(_mm256_mul_ps(lo, lo), _mm256_mul_ps(hi, hi));
    const __m256 sum =
      _mm256_castpd_ps(_mm256_permute4x64_pd(_mm256_castps_pd(paired), _MM_SHUFFLE(3, 1, 2, 0)));
    _mm256_storeu_ps(out + i, _mm256_max_ps(v_eps, _mm256_mul_ps(sum, v_norm)));
  }

  _mm256_zeroupper();
}

//--------------------------------------------------------------------------------------------------
// f64 reductions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Folds the two 128-bit halves of an accumulator with the SSE lane's tail: min_pd of the
 *        halves, then the same min_sd(swapped, acc) step, so NaN and signed zero see the same
 *        operand order. Runs after the caller's _mm256_zeroupper(), when no 256-bit value is live.
 */
[[nodiscard]] SS_TARGET_AVX2 inline double fold128Min(const __m128d& hi, const __m128d& lo)
{
  const __m128d m = _mm_min_pd(hi, lo);
  return _mm_cvtsd_f64(_mm_min_sd(_mm_unpackhi_pd(m, m), m));
}

/**
 * @brief Maximum twin of fold128Min.
 */
[[nodiscard]] SS_TARGET_AVX2 inline double fold128Max(const __m128d& hi, const __m128d& lo)
{
  const __m128d m = _mm_max_pd(hi, lo);
  return _mm_cvtsd_f64(_mm_max_sd(_mm_unpackhi_pd(m, m), m));
}

/**
 * @brief Four-wide simdMinF64 body for n >= 4 * kChains: kChains independent accumulators
 *        (spec 0082) seeded from p[0], NaN elements never win, a NaN seed poisons every chain
 *        alike; chain 1 folds into chain 0 with the loop's operand order. Leaves @p i at the first
 *        unprocessed element.
 */
template<int kChains>
SS_NEVER_INLINE SS_TARGET_AVX2 void minF64(const double* p,
                                           std::size_t n,
                                           std::size_t& i,
                                           double& lo)
{
  static_assert(kChains == 1 || kChains == 2, "one or two accumulator chains");
  constexpr std::size_t kStride = 4 * kChains;
  SS_ASSERT(p != nullptr, return);
  SS_ASSERT(n >= kStride, return);

  __m256d acc0                  = _mm256_set1_pd(p[0]);
  [[maybe_unused]] __m256d acc1 = acc0;
  for (i = 0; i + kStride <= n; i += kStride) {
    acc0 = _mm256_min_pd(_mm256_loadu_pd(p + i), acc0);
    if constexpr (kChains == 2)
      acc1 = _mm256_min_pd(_mm256_loadu_pd(p + i + 4), acc1);
  }

  if constexpr (kChains == 2)
    acc0 = _mm256_min_pd(acc1, acc0);

  const __m128d half_lo = _mm256_castpd256_pd128(acc0);
  const __m128d half_hi = _mm256_extractf128_pd(acc0, 1);
  _mm256_zeroupper();
  lo = fold128Min(half_hi, half_lo);
}

/**
 * @brief Maximum twin of minF64.
 */
template<int kChains>
SS_NEVER_INLINE SS_TARGET_AVX2 void maxF64(const double* p,
                                           std::size_t n,
                                           std::size_t& i,
                                           double& hi)
{
  static_assert(kChains == 1 || kChains == 2, "one or two accumulator chains");
  constexpr std::size_t kStride = 4 * kChains;
  SS_ASSERT(p != nullptr, return);
  SS_ASSERT(n >= kStride, return);

  __m256d acc0                  = _mm256_set1_pd(p[0]);
  [[maybe_unused]] __m256d acc1 = acc0;
  for (i = 0; i + kStride <= n; i += kStride) {
    acc0 = _mm256_max_pd(_mm256_loadu_pd(p + i), acc0);
    if constexpr (kChains == 2)
      acc1 = _mm256_max_pd(_mm256_loadu_pd(p + i + 4), acc1);
  }

  if constexpr (kChains == 2)
    acc0 = _mm256_max_pd(acc1, acc0);

  const __m128d half_lo = _mm256_castpd256_pd128(acc0);
  const __m128d half_hi = _mm256_extractf128_pd(acc0, 1);
  _mm256_zeroupper();
  hi = fold128Max(half_hi, half_lo);
}

/**
 * @brief One-pass four-wide simdMinMaxF64 body for n >= 4 * kChains, both sides chained like
 *        minF64, same seed and NaN semantics.
 */
template<int kChains>
SS_NEVER_INLINE SS_TARGET_AVX2 void minMaxF64(
  const double* p, std::size_t n, std::size_t& i, double& lo, double& hi)
{
  static_assert(kChains == 1 || kChains == 2, "one or two accumulator chains");
  constexpr std::size_t kStride = 4 * kChains;
  SS_ASSERT(p != nullptr, return);
  SS_ASSERT(n >= kStride, return);

  __m256d acc_lo0                  = _mm256_set1_pd(p[0]);
  __m256d acc_hi0                  = acc_lo0;
  [[maybe_unused]] __m256d acc_lo1 = acc_lo0;
  [[maybe_unused]] __m256d acc_hi1 = acc_lo0;
  for (i = 0; i + kStride <= n; i += kStride) {
    const __m256d v0 = _mm256_loadu_pd(p + i);
    acc_lo0          = _mm256_min_pd(v0, acc_lo0);
    acc_hi0          = _mm256_max_pd(v0, acc_hi0);
    if constexpr (kChains == 2) {
      const __m256d v1 = _mm256_loadu_pd(p + i + 4);
      acc_lo1          = _mm256_min_pd(v1, acc_lo1);
      acc_hi1          = _mm256_max_pd(v1, acc_hi1);
    }
  }

  if constexpr (kChains == 2) {
    acc_lo0 = _mm256_min_pd(acc_lo1, acc_lo0);
    acc_hi0 = _mm256_max_pd(acc_hi1, acc_hi0);
  }

  const __m128d lo_l = _mm256_castpd256_pd128(acc_lo0);
  const __m128d lo_h = _mm256_extractf128_pd(acc_lo0, 1);
  const __m128d hi_l = _mm256_castpd256_pd128(acc_hi0);
  const __m128d hi_h = _mm256_extractf128_pd(acc_hi0, 1);
  _mm256_zeroupper();
  lo = fold128Min(lo_h, lo_l);
  hi = fold128Max(hi_h, hi_l);
}

/**
 * @brief Gathers one QPointF lane of four consecutive points into a vector (lane order is
 *        scrambled across the two 128-bit halves, which a min/max reduction never observes).
 */
template<int kLane>
[[nodiscard]] SS_TARGET_AVX2 inline __m256d pointLaneQuad(const double* base, qsizetype point)
{
  const __m256d a = _mm256_loadu_pd(base + 2 * point);
  const __m256d b = _mm256_loadu_pd(base + 2 * point + 4);
  return (kLane == 0) ? _mm256_unpacklo_pd(a, b) : _mm256_unpackhi_pd(a, b);
}

/**
 * @brief Four-point simdFiniteMinMaxPointF body for n >= 4 * kChains (kLane 0 = x, 1 = y):
 *        non-finite lanes are replaced by the opposite infinity before the min/max, exactly as
 *        the SSE lane does, so they can never win. Accumulators start from the caller's seeds
 *        and chain like minMaxF64.
 */
template<int kLane, int kChains>
SS_NEVER_INLINE SS_TARGET_AVX2 void finiteMinMaxPointF(
  const double* base, qsizetype n, qsizetype& i, double& lo, double& hi)
{
  static_assert(kLane == 0 || kLane == 1, "QPointF has exactly two lanes");
  static_assert(kChains == 1 || kChains == 2, "one or two accumulator chains");
  constexpr qsizetype kStride = 4 * kChains;
  SS_ASSERT(base != nullptr, return);
  SS_ASSERT(n >= kStride, return);

  const __m256d sign_mask          = _mm256_set1_pd(-0.0);
  const __m256d plus_inf           = _mm256_set1_pd(std::numeric_limits<double>::infinity());
  const __m256d minus_inf          = _mm256_set1_pd(-std::numeric_limits<double>::infinity());
  __m256d acc_lo0                  = _mm256_set1_pd(lo);
  __m256d acc_hi0                  = _mm256_set1_pd(hi);
  [[maybe_unused]] __m256d acc_lo1 = acc_lo0;
  [[maybe_unused]] __m256d acc_hi1 = acc_hi0;
  for (; i + kStride <= n; i += kStride) {
    const __m256d v0      = pointLaneQuad<kLane>(base, i);
    const __m256d finite0 = _mm256_cmp_pd(_mm256_andnot_pd(sign_mask, v0), plus_inf, _CMP_LT_OS);
    acc_lo0               = _mm256_min_pd(_mm256_blendv_pd(plus_inf, v0, finite0), acc_lo0);
    acc_hi0               = _mm256_max_pd(_mm256_blendv_pd(minus_inf, v0, finite0), acc_hi0);
    if constexpr (kChains == 2) {
      const __m256d v1      = pointLaneQuad<kLane>(base, i + 4);
      const __m256d finite1 = _mm256_cmp_pd(_mm256_andnot_pd(sign_mask, v1), plus_inf, _CMP_LT_OS);
      acc_lo1               = _mm256_min_pd(_mm256_blendv_pd(plus_inf, v1, finite1), acc_lo1);
      acc_hi1               = _mm256_max_pd(_mm256_blendv_pd(minus_inf, v1, finite1), acc_hi1);
    }
  }

  if constexpr (kChains == 2) {
    acc_lo0 = _mm256_min_pd(acc_lo1, acc_lo0);
    acc_hi0 = _mm256_max_pd(acc_hi1, acc_hi0);
  }

  const __m128d lo_l = _mm256_castpd256_pd128(acc_lo0);
  const __m128d lo_h = _mm256_extractf128_pd(acc_lo0, 1);
  const __m128d hi_l = _mm256_castpd256_pd128(acc_hi0);
  const __m128d hi_h = _mm256_extractf128_pd(acc_hi0, 1);
  _mm256_zeroupper();
  lo = fold128Min(lo_h, lo_l);
  hi = fold128Max(hi_h, hi_l);
}

}  // namespace SS_DSP_NAMESPACE::SimdAvx2
