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

  switch (map) {
    case Viridis: {
      static constexpr double r[] = {
        0.267, 0.282, 0.253, 0.207, 0.164, 0.135, 0.135, 0.267, 0.478, 0.741, 0.993};
      static constexpr double g[] = {
        0.005, 0.100, 0.265, 0.371, 0.471, 0.567, 0.659, 0.749, 0.821, 0.873, 0.906};
      static constexpr double b[] = {
        0.329, 0.529, 0.529, 0.553, 0.557, 0.553, 0.518, 0.440, 0.318, 0.150, 0.144};
      return interpolate_lut(r, g, b, 11, t);
    }

    case Inferno: {
      static constexpr double r[] = {0.001, 0.099, 0.301, 0.527, 0.733, 0.882, 0.973, 0.988};
      static constexpr double g[] = {0.000, 0.034, 0.064, 0.117, 0.214, 0.388, 0.626, 0.998};
      static constexpr double b[] = {0.014, 0.299, 0.434, 0.395, 0.276, 0.118, 0.034, 0.645};
      return interpolate_lut(r, g, b, 8, t);
    }

    case Magma: {
      static constexpr double r[] = {0.001, 0.146, 0.421, 0.715, 0.928, 0.987, 0.987};
      static constexpr double g[] = {0.000, 0.060, 0.139, 0.215, 0.473, 0.749, 0.991};
      static constexpr double b[] = {0.014, 0.347, 0.516, 0.475, 0.502, 0.622, 0.749};
      return interpolate_lut(r, g, b, 7, t);
    }

    case Plasma: {
      static constexpr double r[] = {0.050, 0.286, 0.530, 0.741, 0.892, 0.969, 0.940};
      static constexpr double g[] = {0.030, 0.010, 0.140, 0.347, 0.560, 0.789, 0.975};
      static constexpr double b[] = {0.527, 0.629, 0.586, 0.415, 0.227, 0.105, 0.131};
      return interpolate_lut(r, g, b, 7, t);
    }

    case Turbo: {
      static constexpr double r[] = {0.190, 0.275, 0.247, 0.085, 0.152, 0.617, 0.964, 0.974, 0.479};
      static constexpr double g[] = {0.072, 0.366, 0.703, 0.916, 0.988, 0.983, 0.787, 0.317, 0.016};
      static constexpr double b[] = {0.232, 0.804, 0.964, 0.757, 0.357, 0.141, 0.180, 0.108, 0.011};
      return interpolate_lut(r, g, b, 9, t);
    }

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
