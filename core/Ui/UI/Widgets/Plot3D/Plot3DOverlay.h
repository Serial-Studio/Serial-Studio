/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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
#include <QFont>
#include <QImage>
#include <QMatrix4x4>
#include <QSize>
#include <QString>

namespace UI::Widgets::Plot3DDetail {
/**
 * @brief Palette the camera indicator is drawn with, kept together so the tile builder does
 *        not grow a per-color parameter list.
 */
struct IndicatorColors {
  QColor xAxis;
  QColor yAxis;
  QColor zAxis;
  QColor text;
};

[[nodiscard]] QImage buildBackgroundTile(const QSize& itemSize,
                                         const QColor& innerColor,
                                         const QColor& outerColor);

[[nodiscard]] QImage buildCameraIndicatorTile(const int tileSize,
                                              const qreal pixelRatio,
                                              const QMatrix4x4& matrix,
                                              const QFont& font,
                                              const IndicatorColors& colors);

[[nodiscard]] QImage buildTextTile(const QString& text,
                                   const QFont& font,
                                   const QColor& color,
                                   const qreal pixelRatio);
}  // namespace UI::Widgets::Plot3DDetail
