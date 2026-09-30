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

#include <algorithm>
#include <cmath>

/**
 * The extended sRGB transfer pair shared by every HDR shader (spec 0089). The GPU copies in
 * app/shaders/hdr_output.frag, hdr_boost.frag, stroke_hdr.frag and waterfall_hdr.frag mirror
 * these bodies line for line; tst_hdr_transfer pins the contract (exact continuity at 1.0,
 * roundtrip identity, monotonicity), so a drifted shader copy is caught by reading it against
 * this header, never by eye. Pure float math, no Qt, no allocation.
 */
namespace Misc::HdrTransfer {

inline constexpr float kSrgbLinearCut  = 0.0031308f;
inline constexpr float kSrgbEncodedCut = 0.04045f;
inline constexpr float kInvSrgbSlope   = 1.0f / 12.92f;
inline constexpr float kInvSrgbScale   = 1.0f / 1.055f;
inline constexpr float kInvSrgbGamma   = 1.0f / 2.4f;

// code-verify off -- pow(x, 2.4) IS the sRGB definition (no exact non-transcendental form),
// and these CPU bodies run only in tests: the per-pixel work happens in the shader copies.

/**
 * @brief Decodes an extended-sRGB encoded value to linear light: piecewise sRGB on [0, 1],
 *        identity above 1 (continuous at 1 since the sRGB curve maps 1 to 1), 0 below 0.
 */
[[nodiscard]] inline float eotfExt(const float encoded) noexcept
{
  if (encoded <= 0.0f)
    return 0.0f;

  if (encoded <= kSrgbEncodedCut)
    return encoded * kInvSrgbSlope;

  if (encoded <= 1.0f)
    return std::pow((encoded + 0.055f) * kInvSrgbScale, 2.4f);

  return encoded;
}

/**
 * @brief Encodes linear light to extended sRGB: exact inverse of eotfExt over both ranges.
 */
[[nodiscard]] inline float oetfExt(const float linear) noexcept
{
  if (linear <= 0.0f)
    return 0.0f;

  if (linear <= kSrgbLinearCut)
    return linear * 12.92f;

  if (linear <= 1.0f)
    return 1.055f * std::pow(linear, kInvSrgbGamma) - 0.055f;

  return linear;
}

// code-verify on

/**
 * @brief Boosts an encoded SDR value by @p intensity in linear light (the emissive contract:
 *        the output transform maps the result to intensity x SDR white). Intensity below 1
 *        clamps to 1 so a boost can never dim.
 */
[[nodiscard]] inline float boostEncoded(const float encoded, const float intensity) noexcept
{
  return oetfExt(eotfExt(encoded) * std::max(1.0f, intensity));
}

}  // namespace Misc::HdrTransfer
