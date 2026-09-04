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

#pragma once

#include <QColor>

namespace Widgets::Plot3DStereo {
/**
 * @brief Stereo eye a color is derived for: the left eye owns red, the right owns green and
 *        blue, matching the channel split of the per-pixel merge this replaced.
 */
enum class EyeMask {
  None,
  Left,
  Right
};

/**
 * @brief Channels one eye may write. What it does not own is never written, so the pixel
 *        already there survives instead of being blended over.
 */
struct EyeChannels {
  bool red;
  bool green;
  bool blue;
};

/**
 * @brief Alpha ceiling for the eye drawn second on the fallback path, where the two eyes do
 *        blend over each other. Half maximizes the first eye's surviving contrast, which is
 *        a(1-a) in the second eye's alpha.
 */
inline constexpr float kOverlaidEyeAlpha = 0.5f;

/**
 * @brief Accumulator and node slots the widget keeps: one per eye, with the mono picture
 *        reusing the left eye's.
 */
inline constexpr int kEyeSlots = 2;

/**
 * @brief Resource paths of the vendored stereo shaders, compiled into the binary by
 *        qt_add_shaders. Vendored so the uniform block and the code that fills it stay ours.
 */
inline constexpr char kVertexShader[]   = ":/serial-studio.com/shaders/plot3d_eye.vert.qsb";
inline constexpr char kFragmentShader[] = ":/serial-studio.com/shaders/plot3d_eye.frag.qsb";

[[nodiscard]] int eyeSlot(const EyeMask mask);
[[nodiscard]] bool shadersPresent();
[[nodiscard]] float luminance(const QColor& color);
[[nodiscard]] EyeChannels eyeChannels(const EyeMask mask);
[[nodiscard]] QColor midBackground(const QColor& inner, const QColor& outer);
[[nodiscard]] QColor isolatedEyeColor(const QColor& color, const EyeMask mask);
[[nodiscard]] QColor blendedEyeColor(const QColor& color,
                                     const QColor& background,
                                     const EyeMask mask);
}  // namespace Widgets::Plot3DStereo
