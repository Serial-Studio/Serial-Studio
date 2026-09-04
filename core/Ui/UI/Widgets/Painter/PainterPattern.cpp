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

#ifdef BUILD_COMMERCIAL

#  include "UI/Widgets/Painter/PainterPattern.h"

#  include <QImage>
#  include <QPainter>

/**
 * @brief Stores the tile pixmap and repetition string.
 */
Widgets::PainterPattern::PainterPattern(const QPixmap& tile,
                                        const QString& repetition,
                                        QObject* parent)
  : QObject(parent), m_tile(tile), m_repetition(repetition)
{}

/**
 * @brief Returns a tiled QBrush. Non-repeat modes blank the off-axis bands.
 */
QBrush Widgets::PainterPattern::brush() const
{
  if (m_tile.isNull())
    return QBrush();

  if (m_repetition == QLatin1String("repeat") || m_repetition.isEmpty())
    return QBrush(m_tile);

  const int tw = m_tile.width();
  const int th = m_tile.height();
  if (tw <= 0 || th <= 0)
    return QBrush();

  const bool noX =
    (m_repetition == QLatin1String("no-repeat") || m_repetition == QLatin1String("repeat-y"));
  const bool noY =
    (m_repetition == QLatin1String("no-repeat") || m_repetition == QLatin1String("repeat-x"));

  const int outW = noX ? tw * 2 : tw;
  const int outH = noY ? th * 2 : th;
  QImage img(outW, outH, QImage::Format_ARGB32_Premultiplied);
  img.fill(Qt::transparent);
  QPainter p(&img);
  p.drawPixmap(0, 0, m_tile);
  p.end();
  return QBrush(QPixmap::fromImage(img));
}

#endif  // BUILD_COMMERCIAL
