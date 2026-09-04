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

#if !defined(SS_SIMD_DISABLE)
#  error "dsp_scalar_ref.cpp needs SS_SIMD_DISABLE; see set_source_files_properties in CMakeLists"
#endif

#include "dsp_scalar_ref.h"

#include "Core/DSPSimd.h"

/**
 * @file dsp_scalar_ref.cpp
 * @brief The scalar build of DSPSimd.h, wrapped in non-inline DspRef:: entry points (spec 0032).
 *
 * Compiled with SS_SIMD_DISABLE and SS_DSP_NAMESPACE=DspSimdScalar, set on this one source file in
 * app/tests/CMakeLists.txt. The kernels below are named through the literal DspSimdScalar namespace
 * rather than through the macro on purpose: if either definition ever stops being applied, the
 * namespace does not exist and this file fails to compile, instead of quietly forwarding every
 * "scalar" call into the vector lane and turning tst_dsp_kernels green for the wrong reason.
 */

namespace DspRef {

/**
 * @brief Scalar reference for DSP::simdForEachByteMatch.
 */
bool forEachByteMatch(const char* data,
                      qsizetype len,
                      char needle,
                      const ByteMatchCallback& on_match)
{
  return DspSimdScalar::simdForEachByteMatch(data, len, needle, on_match);
}

/**
 * @brief Scalar reference for DSP::simdFindAnyByte.
 */
qsizetype findAnyByte(const char* data, qsizetype len, const quint8* needles, int count)
{
  return DspSimdScalar::simdFindAnyByte(data, len, needles, count);
}

/**
 * @brief Scalar reference for DSP::simdMinF64.
 */
double minF64(const double* p, std::size_t n)
{
  return DspSimdScalar::simdMinF64(p, n);
}

/**
 * @brief Scalar reference for DSP::simdMaxF64.
 */
double maxF64(const double* p, std::size_t n)
{
  return DspSimdScalar::simdMaxF64(p, n);
}

/**
 * @brief Scalar reference for DSP::simdMinMaxF64.
 */
void minMaxF64(const double* p, std::size_t n, double& lo, double& hi)
{
  DspSimdScalar::simdMinMaxF64(p, n, lo, hi);
}

/**
 * @brief Scalar reference for DSP::simdFiniteMinMaxPointF<0>, the QPointF x lane.
 */
void finiteMinMaxPointFX(const QPointF* pts, qsizetype n, double& lo, double& hi)
{
  DspSimdScalar::simdFiniteMinMaxPointF<0>(pts, n, lo, hi);
}

/**
 * @brief Scalar reference for DSP::simdFiniteMinMaxPointF<1>, the QPointF y lane.
 */
void finiteMinMaxPointFY(const QPointF* pts, qsizetype n, double& lo, double& hi)
{
  DspSimdScalar::simdFiniteMinMaxPointF<1>(pts, n, lo, hi);
}

/**
 * @brief Scalar reference for DSP::simdWindowedRealFill.
 */
void windowedRealFill(const double* ring,
                      std::size_t front,
                      std::size_t mask,
                      std::size_t n,
                      double offset,
                      double scale,
                      const float* window,
                      float* out)
{
  DspSimdScalar::simdWindowedRealFill(ring, front, mask, n, offset, scale, window, out);
}

/**
 * @brief Scalar reference for DSP::simdRingsToPoints.
 */
void ringsToPoints(const double* xs,
                   std::size_t xfront,
                   std::size_t xmask,
                   const double* ys,
                   std::size_t yfront,
                   std::size_t ymask,
                   qsizetype n,
                   QPointF* out)
{
  DspSimdScalar::simdRingsToPoints(xs, xfront, xmask, ys, yfront, ymask, n, out);
}

/**
 * @brief Scalar reference for DSP::simdAsciiDots16.
 */
void asciiDots16(const quint8* src, char16_t* out)
{
  DspSimdScalar::simdAsciiDots16(src, out);
}

/**
 * @brief Scalar reference for DSP::simdWidenAscii.
 */
bool widenAscii(const char* src, char16_t* out, std::size_t n)
{
  return DspSimdScalar::simdWidenAscii(src, out, n);
}

/**
 * @brief Scalar reference for DSP::simdDeinterleaveToF64.
 */
void deinterleaveToF64(const float* src, std::size_t frames, int channels, int channel, double* out)
{
  DspSimdScalar::simdDeinterleaveToF64(src, frames, channels, channel, out);
}

/**
 * @brief Scalar reference for DSP::simdPowerSpectrumDb.
 */
void powerSpectrumDb(
  const float* interleaved, float* out, std::size_t n, float invNorm, float epsSq, float floorDb)
{
  DspSimdScalar::simdPowerSpectrumDb(interleaved, out, n, invNorm, epsSq, floorDb);
}

/**
 * @brief Scalar reference for DSP::SimdDetail::windowedRealSpan.
 */
void windowedRealSpan(
  const double* src, const float* win, float* out, std::size_t n, double offset, double scale)
{
  DspSimdScalar::SimdDetail::windowedRealSpan(src, win, out, n, offset, scale);
}

/**
 * @brief Scalar reference for DSP::SimdDetail::interleaveSpan.
 */
void interleaveSpan(const double* xs, const double* ys, double* out, qsizetype n)
{
  DspSimdScalar::SimdDetail::interleaveSpan(xs, ys, out, n);
}

/**
 * @brief Names the lane this translation unit actually compiled, so the suite can prove at runtime
 *        that the reference really is scalar and not a folded copy of the vector build.
 */
const char* scalarLaneName()
{
#if defined(SS_SIMD_X86)
  return "x86";
#elif defined(SS_SIMD_NEON)
  return "neon";
#else
  return "scalar";
#endif
}

}  // namespace DspRef
