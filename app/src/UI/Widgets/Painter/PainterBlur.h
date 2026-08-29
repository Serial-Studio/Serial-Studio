/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * Pro feature -- requires the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QImage>

namespace Widgets::PainterBlur {

/**
 * @brief Largest radius the fixed reciprocal table is sized for; callers clamp to it.
 */
constexpr int kMaxBlurRadius = 32;

/**
 * @brief Three-pass separable box-blur applied in place to the given image.
 *        The image must be QImage::Format_ARGB32_Premultiplied.
 */
void applyBoxBlur(QImage& image, int radius);

}  // namespace Widgets::PainterBlur

#endif  // BUILD_COMMERCIAL
