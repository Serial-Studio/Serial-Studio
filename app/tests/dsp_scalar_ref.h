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

#pragma once

#include <cstddef>
#include <functional>
#include <QPointF>
#include <QtGlobal>

/**
 * @file dsp_scalar_ref.h
 * @brief Scalar-lane reference entry points for the DSPSimd.h kernels (spec 0032, T7).
 *
 * This header deliberately does NOT include DSPSimd.h. The suite that consumes it
 * (tst_dsp_kernels.cpp) must see the ordinary vector build of that header in namespace DSP; the
 * scalar build lives alone in dsp_scalar_ref.cpp, which is the only translation unit compiled with
 * SS_SIMD_DISABLE and SS_DSP_NAMESPACE=DspSimdScalar.
 *
 * The two builds must not share the header's inline symbols. DSP:: kernels are `inline` with
 * external linkage, so compiling the same header twice under the same namespace lets the linker
 * fold the two definitions (an ODR violation), and the "scalar" wrapper would silently call the
 * vector code -- a green test that checks nothing. The namespace macro is what keeps them distinct,
 * and these wrappers are non-inline so the call can never be devirtualized back into the vector
 * lane either.
 */

namespace DspRef {

using ByteMatchCallback = std::function<bool(qsizetype)>;

//--------------------------------------------------------------------------------------------------
// Byte scanning
//--------------------------------------------------------------------------------------------------

[[nodiscard]] bool forEachByteMatch(const char* data,
                                    qsizetype len,
                                    char needle,
                                    const ByteMatchCallback& on_match);

[[nodiscard]] qsizetype findAnyByte(const char* data,
                                    qsizetype len,
                                    const quint8* needles,
                                    int count);

//--------------------------------------------------------------------------------------------------
// f64 reductions
//--------------------------------------------------------------------------------------------------

[[nodiscard]] double minF64(const double* p, std::size_t n);
[[nodiscard]] double maxF64(const double* p, std::size_t n);
void minMaxF64(const double* p, std::size_t n, double& lo, double& hi);

//--------------------------------------------------------------------------------------------------
// QPointF lane reductions
//--------------------------------------------------------------------------------------------------

void finiteMinMaxPointFX(const QPointF* pts, qsizetype n, double& lo, double& hi);
void finiteMinMaxPointFY(const QPointF* pts, qsizetype n, double& lo, double& hi);

//--------------------------------------------------------------------------------------------------
// Ring-buffer bulk transforms
//--------------------------------------------------------------------------------------------------

void windowedComplexFill(const double* ring,
                         std::size_t front,
                         std::size_t mask,
                         std::size_t n,
                         double offset,
                         double scale,
                         const float* window,
                         float* out);

void ringsToPoints(const double* xs,
                   std::size_t xfront,
                   std::size_t xmask,
                   const double* ys,
                   std::size_t yfront,
                   std::size_t ymask,
                   qsizetype n,
                   QPointF* out);

//--------------------------------------------------------------------------------------------------
// Text formatting
//--------------------------------------------------------------------------------------------------

void asciiDots16(const quint8* src, char16_t* out);

//--------------------------------------------------------------------------------------------------
// SimdDetail contiguous-span helpers
//--------------------------------------------------------------------------------------------------

void windowedComplexSpan(
  const double* src, const float* win, float* out, std::size_t n, double offset, double scale);

void interleaveSpan(const double* xs, const double* ys, double* out, qsizetype n);

//--------------------------------------------------------------------------------------------------
// Build introspection
//--------------------------------------------------------------------------------------------------

[[nodiscard]] const char* scalarLaneName();

}  // namespace DspRef
