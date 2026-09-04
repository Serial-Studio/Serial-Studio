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

#include "UI/Widgets/Plot3D/Plot3DOverlay.h"

#include <algorithm>
#include <QFontMetrics>
#include <QPainter>
#include <QRadialGradient>
#include <QVector4D>

namespace Plot3DOverlayDetail {
/**
 * @brief One axis of the camera orientation indicator, with its projected tip.
 */
struct IndicatorAxis {
  QVector3D direction;
  QColor color;
  QString label;
  QVector4D projected;
};
}  // namespace Plot3DOverlayDetail

/**
 * @brief Rasterizes the plot's radial background at a reduced resolution: the gradient is
 *        smooth, so a quarter-scale tile magnifies to the item without a visible difference,
 *        and the tile keeps the item's aspect ratio so the falloff stays circular rather than
 *        stretching into an ellipse. Rebuilt only on a theme or size change, never per frame.
 */
QImage UI::Widgets::Plot3DDetail::buildBackgroundTile(const QSize& itemSize,
                                                      const QColor& innerColor,
                                                      const QColor& outerColor)
{
  const int tw = qMax(32, itemSize.width() / 4);
  const int th = qMax(32, itemSize.height() / 4);

  QImage tile(tw, th, QImage::Format_ARGB32_Premultiplied);
  tile.fill(Qt::transparent);

  const QPointF center(tw * 0.5, th * 0.5);
  const double radius = qMax(tw, th) * 0.25;

  QRadialGradient gradient(center, radius);
  gradient.setColorAt(0.0, innerColor);
  gradient.setColorAt(1.0, outerColor);

  QPainter painter(&tile);
  painter.fillRect(QRectF(0, 0, tw, th), gradient);

  return tile;
}

/**
 * @brief Rasterizes the camera orientation indicator into a small fixed-size tile: three
 *        depth-sorted axis stubs, each capped by a filled dot carrying its label. Bounded to
 *        the corner region the indicator actually occupies, so a camera-driven rebuild costs a
 *        fraction of the full-item layer it replaces.
 */
QImage UI::Widgets::Plot3DDetail::buildCameraIndicatorTile(const int tileSize,
                                                           const qreal pixelRatio,
                                                           const QMatrix4x4& matrix,
                                                           const QFont& font,
                                                           const IndicatorColors& colors)
{
  QImage tile(QSize(tileSize, tileSize) * pixelRatio, QImage::Format_ARGB32_Premultiplied);
  tile.setDevicePixelRatio(pixelRatio);
  tile.fill(Qt::transparent);

  QPainter painter(&tile);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setFont(font);

  constexpr float kLineScale  = 18.0f;
  constexpr float kAxisLength = 2.0f;
  const QPointF origin(tileSize * 0.5, tileSize * 0.5);

  QList<Plot3DOverlayDetail::IndicatorAxis> axes = {
    {{1, 0, 0}, colors.xAxis, QStringLiteral("X"), {}},
    {{0, 1, 0}, colors.yAxis, QStringLiteral("Y"), {}},
    {{0, 0, 1}, colors.zAxis, QStringLiteral("Z"), {}}
  };

  for (auto& axis : axes)
    axis.projected = matrix * QVector4D(axis.direction * kAxisLength, 1.0f);

  std::sort(
    axes.begin(),
    axes.end(),
    [](const Plot3DOverlayDetail::IndicatorAxis& a, const Plot3DOverlayDetail::IndicatorAxis& b) {
      return a.projected.z() < b.projected.z();
    });

  const QFontMetrics metrics(font);
  const int textWidth      = metrics.horizontalAdvance("X");
  const int textHeight     = metrics.height();
  const float circleRadius = std::max(textWidth, textHeight) * 0.7f;

  for (const auto& axis : axes) {
    const QVector4D& t = axis.projected;
    const float invW   = t.w() != 0.0f ? 1.0f / t.w() : 0.0f;
    const QPointF tip(origin.x() + (t.x() * invW) * kLineScale,
                      origin.y() - (t.y() * invW) * kLineScale);

    painter.setPen(QPen(axis.color, 3));
    painter.drawLine(origin, tip);

    painter.setBrush(axis.color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(tip, circleRadius, circleRadius);

    const QRectF textRect(
      tip.x() - circleRadius, tip.y() - circleRadius, circleRadius * 2, circleRadius * 2);
    painter.setPen(colors.text);
    painter.drawText(textRect, Qt::AlignCenter, axis.label);
  }

  return tile;
}

/**
 * @brief Rasterizes a single line of text into a tile sized to its bounding box.
 */
QImage UI::Widgets::Plot3DDetail::buildTextTile(const QString& text,
                                                const QFont& font,
                                                const QColor& color,
                                                const qreal pixelRatio)
{
  if (text.isEmpty())
    return QImage();

  const QFontMetrics metrics(font);
  const QSize logical = metrics.size(Qt::TextSingleLine, text) + QSize(2, 2);
  if (logical.isEmpty())
    return QImage();

  QImage tile(logical * pixelRatio, QImage::Format_ARGB32_Premultiplied);
  tile.setDevicePixelRatio(pixelRatio);
  tile.fill(Qt::transparent);

  QPainter painter(&tile);
  painter.setRenderHint(QPainter::TextAntialiasing, true);
  painter.setFont(font);
  painter.setPen(color);
  painter.drawText(QRectF(0, 0, logical.width(), logical.height()), Qt::AlignCenter, text);

  return tile;
}
