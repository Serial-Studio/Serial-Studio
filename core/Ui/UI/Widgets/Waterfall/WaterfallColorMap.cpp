/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/Waterfall/WaterfallColorMap.h"

#include <QtGlobal>

//--------------------------------------------------------------------------------------------------
// Anchor tables
//--------------------------------------------------------------------------------------------------

namespace detail::waterfallmaps {

/**
 * @brief Evenly spaced per-channel control points for one ramp, walked linearly by sample().
 */
struct Ramp {
  const double* r;
  const double* g;
  const double* b;
  int n;
};

static constexpr double kViridisR[] = {
  0.267, 0.282, 0.253, 0.207, 0.164, 0.135, 0.135, 0.267, 0.478, 0.741, 0.993};
static constexpr double kViridisG[] = {
  0.005, 0.100, 0.265, 0.371, 0.471, 0.567, 0.659, 0.749, 0.821, 0.873, 0.906};
static constexpr double kViridisB[] = {
  0.329, 0.529, 0.529, 0.553, 0.557, 0.553, 0.518, 0.440, 0.318, 0.150, 0.144};

static constexpr double kInfernoR[] = {0.001, 0.099, 0.301, 0.527, 0.733, 0.882, 0.973, 0.988};
static constexpr double kInfernoG[] = {0.000, 0.034, 0.064, 0.117, 0.214, 0.388, 0.626, 0.998};
static constexpr double kInfernoB[] = {0.014, 0.299, 0.434, 0.395, 0.276, 0.118, 0.034, 0.645};

static constexpr double kMagmaR[] = {0.001, 0.146, 0.421, 0.715, 0.928, 0.987, 0.987};
static constexpr double kMagmaG[] = {0.000, 0.060, 0.139, 0.215, 0.473, 0.749, 0.991};
static constexpr double kMagmaB[] = {0.014, 0.347, 0.516, 0.475, 0.502, 0.622, 0.749};

static constexpr double kPlasmaR[] = {0.050, 0.286, 0.530, 0.741, 0.892, 0.969, 0.940};
static constexpr double kPlasmaG[] = {0.030, 0.010, 0.140, 0.347, 0.560, 0.789, 0.975};
static constexpr double kPlasmaB[] = {0.527, 0.629, 0.586, 0.415, 0.227, 0.105, 0.131};

static constexpr double kTurboR[] = {0.190, 0.275, 0.247, 0.085, 0.152, 0.617, 0.964, 0.974, 0.479};
static constexpr double kTurboG[] = {0.072, 0.366, 0.703, 0.916, 0.988, 0.983, 0.787, 0.317, 0.016};
static constexpr double kTurboB[] = {0.232, 0.804, 0.964, 0.757, 0.357, 0.141, 0.180, 0.108, 0.011};

static constexpr double kCividisR[] = {
  0.000, 0.000, 0.165, 0.264, 0.342, 0.415, 0.489, 0.567, 0.648, 0.732, 0.819, 0.914, 0.996};
static constexpr double kCividisG[] = {
  0.135, 0.191, 0.248, 0.308, 0.365, 0.423, 0.485, 0.547, 0.611, 0.677, 0.748, 0.826, 0.909};
static constexpr double kCividisB[] = {
  0.305, 0.441, 0.429, 0.423, 0.429, 0.445, 0.471, 0.470, 0.455, 0.426, 0.380, 0.304, 0.218};

static constexpr double kCubehelixR[] = {
  0.000, 0.095, 0.096, 0.085, 0.170, 0.375, 0.633, 0.798, 0.830, 0.780, 0.762, 0.851, 1.000};
static constexpr double kCubehelixG[] = {
  0.000, 0.063, 0.177, 0.327, 0.437, 0.479, 0.475, 0.487, 0.563, 0.700, 0.844, 0.948, 1.000};
static constexpr double kCubehelixB[] = {
  0.000, 0.154, 0.283, 0.297, 0.224, 0.185, 0.291, 0.524, 0.776, 0.928, 0.952, 0.936, 1.000};

static constexpr double kSdrClassicR[] = {
  0.000, 0.000, 0.000, 0.000, 0.145, 0.949, 0.980, 0.902, 1.000};
static constexpr double kSdrClassicG[] = {
  0.000, 0.000, 0.235, 0.706, 0.902, 0.949, 0.514, 0.000, 1.000};
static constexpr double kSdrClassicB[] = {
  0.000, 0.376, 0.702, 0.902, 0.361, 0.000, 0.000, 0.000, 1.000};

static constexpr double kSpectralR[] = {
  0.620, 0.838, 0.957, 0.991, 0.996, 0.998, 0.902, 0.675, 0.400, 0.199, 0.369};
static constexpr double kSpectralG[] = {
  0.004, 0.247, 0.427, 0.677, 0.878, 0.999, 0.961, 0.869, 0.761, 0.529, 0.310};
static constexpr double kSpectralB[] = {
  0.259, 0.309, 0.263, 0.378, 0.545, 0.746, 0.596, 0.642, 0.647, 0.739, 0.635};

static constexpr double kRedBlueR[] = {
  0.404, 0.701, 0.839, 0.955, 0.992, 0.966, 0.820, 0.577, 0.263, 0.127, 0.020};
static constexpr double kRedBlueG[] = {
  0.000, 0.100, 0.376, 0.642, 0.859, 0.967, 0.898, 0.775, 0.576, 0.396, 0.188};
static constexpr double kRedBlueB[] = {
  0.122, 0.171, 0.302, 0.506, 0.780, 0.968, 0.941, 0.872, 0.765, 0.669, 0.380};

static constexpr double kCoolwarmR[] = {
  0.230, 0.329, 0.436, 0.554, 0.667, 0.773, 0.867, 0.938, 0.968, 0.958, 0.909, 0.820, 0.706};
static constexpr double kCoolwarmG[] = {
  0.299, 0.440, 0.571, 0.690, 0.779, 0.839, 0.864, 0.809, 0.721, 0.604, 0.462, 0.287, 0.016};
static constexpr double kCoolwarmB[] = {
  0.754, 0.870, 0.952, 0.996, 0.993, 0.949, 0.863, 0.741, 0.612, 0.483, 0.361, 0.245, 0.150};

/**
 * @brief Returns the anchor table for a table-driven map, or a null ramp for the maps sample()
 *        evaluates in closed form (Jet, Hot and the two grayscales).
 */
static Ramp ramp_for(const int map)
{
  using namespace Widgets::WaterfallColorMap;

  switch (map) {
    case Viridis:
      return {kViridisR, kViridisG, kViridisB, 11};
    case Inferno:
      return {kInfernoR, kInfernoG, kInfernoB, 8};
    case Magma:
      return {kMagmaR, kMagmaG, kMagmaB, 7};
    case Plasma:
      return {kPlasmaR, kPlasmaG, kPlasmaB, 7};
    case Turbo:
      return {kTurboR, kTurboG, kTurboB, 9};
    case Cividis:
      return {kCividisR, kCividisG, kCividisB, 13};
    case Cubehelix:
      return {kCubehelixR, kCubehelixG, kCubehelixB, 13};
    case SdrClassic:
      return {kSdrClassicR, kSdrClassicG, kSdrClassicB, 9};
    case Spectral:
      return {kSpectralR, kSpectralG, kSpectralB, 11};
    case RedBlue:
      return {kRedBlueR, kRedBlueG, kRedBlueB, 11};
    case Coolwarm:
      return {kCoolwarmR, kCoolwarmG, kCoolwarmB, 13};
    default:
      return {nullptr, nullptr, nullptr, 0};
  }
}

}  // namespace detail::waterfallmaps

//--------------------------------------------------------------------------------------------------
// Sampling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Linearly interpolates a color from per-channel control-point arrays of length n.
 */
static QRgb interpolate_lut(
  const double* r, const double* g, const double* b, const int n, const double t)
{
  const double f = t * (n - 1);
  const int i    = qBound(0, static_cast<int>(f), n - 2);
  const double s = f - i;
  const int rr   = static_cast<int>((r[i] + (r[i + 1] - r[i]) * s) * 255.0);
  const int gg   = static_cast<int>((g[i] + (g[i + 1] - g[i]) * s) * 255.0);
  const int bb   = static_cast<int>((b[i] + (b[i + 1] - b[i]) * s) * 255.0);

  return qRgb(qBound(0, rr, 255), qBound(0, gg, 255), qBound(0, bb, 255));
}

/**
 * @brief Returns the RGB color for a color map and a normalized magnitude in [0, 1]. Reserved for
 *        the colorbar and single-color queries: the spectrogram colorizes through the baked LUT,
 *        which is what keeps a 32768-bin row off this switch.
 */
QRgb Widgets::WaterfallColorMap::sample(int map, double t)
{
  t = qBound(0.0, t, 1.0);

  const detail::waterfallmaps::Ramp ramp = detail::waterfallmaps::ramp_for(map);
  if (ramp.r != nullptr)
    return interpolate_lut(ramp.r, ramp.g, ramp.b, ramp.n, t);

  switch (map) {
    case Jet: {
      const double v = t;
      const double r = qBound(0.0, qMin(4.0 * v - 1.5, 4.5 - 4.0 * v), 1.0);
      const double g = qBound(0.0, qMin(4.0 * v - 0.5, 3.5 - 4.0 * v), 1.0);
      const double b = qBound(0.0, qMin(4.0 * v + 0.5, 2.5 - 4.0 * v), 1.0);
      return qRgb(
        static_cast<int>(r * 255.0), static_cast<int>(g * 255.0), static_cast<int>(b * 255.0));
    }

    case Hot: {
      const double v = t;
      const double r = qBound(0.0, 3.0 * v, 1.0);
      const double g = qBound(0.0, 3.0 * v - 1.0, 1.0);
      const double b = qBound(0.0, 3.0 * v - 2.0, 1.0);
      return qRgb(
        static_cast<int>(r * 255.0), static_cast<int>(g * 255.0), static_cast<int>(b * 255.0));
    }

    case GrayscaleInverted: {
      const int v = static_cast<int>((1.0 - t) * 255.0);
      return qRgb(v, v, v);
    }

    case Grayscale:
    default: {
      const int v = static_cast<int>(t * 255.0);
      return qRgb(v, v, v);
    }
  }
}

/**
 * @brief Bakes one color map into a 256-entry table indexed by the quantized magnitude. Rebuilt
 *        only when the map changes, so colorizing a spectrum row is one array read per bin
 *        instead of a switch, three multiplies and three clamps in the double domain.
 */
std::vector<QRgb> Widgets::WaterfallColorMap::buildLut(const int map)
{
  std::vector<QRgb> lut(static_cast<std::size_t>(kLutSize));
  const double last = static_cast<double>(kLutSize - 1);
  for (int i = 0; i < kLutSize; ++i)
    lut[static_cast<std::size_t>(i)] = sample(map, static_cast<double>(i) / last);

  return lut;
}
