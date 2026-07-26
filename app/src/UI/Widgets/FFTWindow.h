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

#pragma once

#include <cmath>
#include <QtGlobal>

#include "SerialStudio.h"
#include "SSAssert.h"

// Frame.h defaults Dataset::fftWindow to the literal 5 (it cannot name the enumerator).
static_assert(static_cast<int>(SerialStudio::FFTWindowBlackmanHarris) == 5,
              "Dataset::fftWindow default literal must match FFTWindowBlackmanHarris");

namespace Widgets {
//--------------------------------------------------------------------------------------------------
// FFT window coefficient tables
//--------------------------------------------------------------------------------------------------

inline constexpr float kFftWindowPi    = 3.14159265358979323846f;
inline constexpr float kFftWindowTwoPi = 2.0f * kFftWindowPi;

/**
 * @brief Evaluates the coefficient at sample @a i (of @a N) for the given window
 *        function. Symmetric windows use the (N-1) span; N <= 1 degenerates to 1.
 */
[[nodiscard]] inline float fftWindowValue(SerialStudio::FFTWindow type,
                                          unsigned int i,
                                          unsigned int N)
{
  if (N <= 1)
    return 1.0f;

  SS_ASSERT(i < N, return 1.0f);
  const float d    = static_cast<float>(N - 1);
  const float x    = kFftWindowTwoPi * static_cast<float>(i) / d;
  const float t    = static_cast<float>(i) / d;
  const float half = 0.5f * d;
  const float n    = static_cast<float>(i) - half;

  switch (type) {
    case SerialStudio::FFTWindowRectangular:
      return 1.0f;
    case SerialStudio::FFTWindowBartlett:
      return 1.0f - std::fabs(n / half);
    case SerialStudio::FFTWindowWelch: {
      const float r = n / half;
      return 1.0f - r * r;
    }
    case SerialStudio::FFTWindowHann:
      return 0.5f - 0.5f * std::cos(x);
    case SerialStudio::FFTWindowHamming:
      return 0.54f - 0.46f * std::cos(x);
    case SerialStudio::FFTWindowBlackman:
      return 0.42f - 0.5f * std::cos(x) + 0.08f * std::cos(2.0f * x);
    case SerialStudio::FFTWindowBlackmanHarris:
      return 0.35875f - 0.48829f * std::cos(x) + 0.14128f * std::cos(2.0f * x)
           - 0.01168f * std::cos(3.0f * x);
    case SerialStudio::FFTWindowNuttall:
      return 0.355768f - 0.487396f * std::cos(x) + 0.144232f * std::cos(2.0f * x)
           - 0.012604f * std::cos(3.0f * x);
    case SerialStudio::FFTWindowBlackmanNuttall:
      return 0.3635819f - 0.4891775f * std::cos(x) + 0.1365995f * std::cos(2.0f * x)
           - 0.0106411f * std::cos(3.0f * x);
    case SerialStudio::FFTWindowFlatTop:
      return 0.21557895f - 0.41663158f * std::cos(x) + 0.277263158f * std::cos(2.0f * x)
           - 0.083578947f * std::cos(3.0f * x) + 0.006947368f * std::cos(4.0f * x);
    case SerialStudio::FFTWindowBartlettHann:
      return 0.62f - 0.48f * std::fabs(t - 0.5f) - 0.38f * std::cos(x);
    case SerialStudio::FFTWindowBohman: {
      const float a  = std::fabs(2.0f * t - 1.0f);
      const float pa = kFftWindowPi * a;
      return (1.0f - a) * std::cos(pa) + (1.0f / kFftWindowPi) * std::sin(pa);
    }
    case SerialStudio::FFTWindowCosine:
      return std::sin(kFftWindowPi * t);
    case SerialStudio::FFTWindowLanczos: {
      const float z = 2.0f * t - 1.0f;
      if (z == 0.0f)
        return 1.0f;

      const float px = kFftWindowPi * z;
      return std::sin(px) / px;
    }
    case SerialStudio::FFTWindowParzen: {
      const float a = std::fabs(n) / half;
      if (std::fabs(n) <= 0.5f * half)
        return 1.0f - 6.0f * a * a * (1.0f - a);

      const float b = 1.0f - a;
      return 2.0f * b * b * b;
    }
  }

  return 1.0f;
}

/**
 * @brief Fills @a out[0..N) with the coefficients of the given window function.
 */
inline void fillFftWindow(SerialStudio::FFTWindow type, float* out, unsigned int N)
{
  SS_ASSERT(out != nullptr || N == 0, return);
  for (unsigned int i = 0; i < N; ++i)
    out[i] = fftWindowValue(type, i, N);
}

//--------------------------------------------------------------------------------------------------
// Transform size contract
//--------------------------------------------------------------------------------------------------

inline constexpr int kMaxFftTransformSize = 262144;

/**
 * @brief Transform ceiling for the waterfall, deliberately below kMaxFftTransformSize: the
 *        spectrogram materializes a QImage of (size / 2) x historySize 32-bit pixels, so the
 *        full 262144 would ask for a 131072-column image (gigabytes at the 4096-row history
 *        limit). FFTPlot has no such backing image and keeps the higher ceiling.
 */
inline constexpr int kMaxWaterfallFftSize = 65536;

/**
 * @brief Normalizes a dataset's sample count to the size the transform runs at: the largest
 *        power of two not exceeding it, >= 8 bins, capped at @a maxSize. Ring capacity and
 *        widget plan must derive from this under the same cap, or a hand-written fftSamples
 *        leaves the widget re-planning against a size it never validated.
 */
[[nodiscard]] inline int normalizedFftSize(int samples, int maxSize)
{
  SS_ASSERT(maxSize >= 8, return 8);
  const int clamped = qBound(8, samples, qMin(maxSize, kMaxFftTransformSize));
  return 1 << static_cast<int>(std::log2(clamped));
}

/**
 * @brief Normalizes a dataset's configured sample count against the global transform ceiling.
 */
[[nodiscard]] inline int normalizedFftSize(int samples)
{
  return normalizedFftSize(samples, kMaxFftTransformSize);
}
}  // namespace Widgets
