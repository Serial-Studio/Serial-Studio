/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QPainter>
#include <QSettings>
#include <QVector3D>
#include <QMatrix4x4>
#include <QQuickPaintedItem>

#include "SerialStudio.h"

namespace Widgets
{
/**
 * @class Plot3D
 * @brief A 3D plotting widget with optional anaglyph (stereo) rendering.
 *
 * Renders a 3D plot with grid, data, and camera indicator.
 * Supports zoom, camera rotation, and anaglyph mode for red/cyan 3D effect.
 *
 * Exposed properties:
 * - zoom
 * - anaglyphEnabled
 * - cameraAngleX, cameraAngleY, cameraAngleZ
 */
class Plot3D : public QQuickPaintedItem
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool anaglyphEnabled
             READ anaglyphEnabled
             WRITE setAnaglyphEnabled
             NOTIFY anaglyphEnabledChanged)
  Q_PROPERTY(bool orbitNavigation
             READ orbitNavigation
             WRITE setOrbitNavigation
             NOTIFY orbitNavigationChanged)
  Q_PROPERTY(bool interpolationEnabled
             READ interpolationEnabled
             WRITE setInterpolationEnabled
             NOTIFY interpolationEnabledChanged)
  Q_PROPERTY(qreal worldScale
             READ worldScale
             WRITE setWorldScale
             NOTIFY cameraChanged)
  Q_PROPERTY(qreal cameraAngleX
             READ cameraAngleX
             WRITE setCameraAngleX
             NOTIFY cameraChanged)
  Q_PROPERTY(qreal cameraAngleY
             READ cameraAngleY
             WRITE setCameraAngleY
             NOTIFY cameraChanged)
  Q_PROPERTY(qreal cameraAngleZ
             READ cameraAngleZ
             WRITE setCameraAngleZ
             NOTIFY cameraChanged)
  Q_PROPERTY(qreal cameraOffsetX
             READ cameraOffsetX
             WRITE setCameraOffsetX
             NOTIFY cameraChanged)
  Q_PROPERTY(qreal cameraOffsetY
             READ cameraOffsetY
             WRITE setCameraOffsetY
             NOTIFY cameraChanged)
  Q_PROPERTY(qreal cameraOffsetZ
             READ cameraOffsetZ
             WRITE setCameraOffsetZ
             NOTIFY cameraChanged)
  Q_PROPERTY(qreal idealWorldScale
             READ idealWorldScale
             NOTIFY rangeChanged)
  Q_PROPERTY(float eyeSeparation
             READ eyeSeparation
             WRITE setEyeSeparation
             NOTIFY eyeSeparationChanged)
  Q_PROPERTY(bool invertEyePositions
             READ invertEyePositions
             WRITE setInvertEyePositions
             NOTIFY invertEyePositionsChanged)
  // clang-format on

signals:
  void rangeChanged();
  void cameraChanged();
  void eyeSeparationChanged();
  void anaglyphEnabledChanged();
  void orbitNavigationChanged();
  void invertEyePositionsChanged();
  void interpolationEnabledChanged();

public:
  explicit Plot3D(const int index = -1, QQuickItem *parent = nullptr);
  void paint(QPainter *painter) override;

  [[nodiscard]] qreal worldScale() const;
  [[nodiscard]] qreal cameraAngleX() const;
  [[nodiscard]] qreal cameraAngleY() const;
  [[nodiscard]] qreal cameraAngleZ() const;
  [[nodiscard]] qreal cameraOffsetX() const;
  [[nodiscard]] qreal cameraOffsetY() const;
  [[nodiscard]] qreal cameraOffsetZ() const;
  [[nodiscard]] qreal idealWorldScale() const;

  [[nodiscard]] bool dirty() const;

  [[nodiscard]] float eyeSeparation() const;
  [[nodiscard]] bool anaglyphEnabled() const;
  [[nodiscard]] bool invertEyePositions() const;

  [[nodiscard]] bool orbitNavigation() const;
  [[nodiscard]] bool interpolationEnabled() const;

public slots:
  void setWorldScale(const qreal z);
  void setCameraAngleX(const qreal angle);
  void setCameraAngleY(const qreal angle);
  void setCameraAngleZ(const qreal angle);
  void setCameraOffsetX(const qreal offset);
  void setCameraOffsetY(const qreal offset);
  void setCameraOffsetZ(const qreal offset);
  void setAnaglyphEnabled(const bool enabled);
  void setOrbitNavigation(const bool enabled);
  void setEyeSeparation(const float separation);
  void setInvertEyePositions(const bool enabled);
  void setInterpolationEnabled(const bool enabled);

private slots:
  void updateData();
  void onThemeChanged();

private:
  void markDirty();

  void drawData();
  void drawGrid();
  void drawBackground();
  void drawCameraIndicator();

private:
  qreal gridStep(const qreal scale = -1) const;
  std::vector<QPointF> screenProjection(const PlotData3D &points,
                                        const QMatrix4x4 &matrix);
  void drawLine3D(QPainter &painter, const QMatrix4x4 &matrix,
                  const QVector3D &p1, const QVector3D &p2, QColor color,
                  float lineWidth, Qt::PenStyle style);

  QPixmap renderGrid(const QMatrix4x4 &matrix);
  QPixmap renderCameraIndicator(const QMatrix4x4 &matrix);
  QPixmap renderData(const QMatrix4x4 &matrix, const PlotData3D &data);
  QPair<QMatrix4x4, QMatrix4x4> eyeTransformations(const QMatrix4x4 &matrix);

protected:
  void wheelEvent(QWheelEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  int m_index;

  qreal m_minX;
  qreal m_maxX;
  qreal m_minY;
  qreal m_maxY;
  qreal m_minZ;
  qreal m_maxZ;

  qreal m_worldScale;
  qreal m_cameraAngleX;
  qreal m_cameraAngleY;
  qreal m_cameraAngleZ;
  qreal m_cameraOffsetX;
  qreal m_cameraOffsetY;
  qreal m_cameraOffsetZ;

  float m_eyeSeparation;

  bool m_anaglyph;
  bool m_interpolate;
  bool m_orbitNavigation;
  bool m_invertEyePositions;

  bool m_dirtyData;
  bool m_dirtyGrid;
  bool m_dirtyBackground;
  bool m_dirtyCameraIndicator;

  QColor m_textColor;
  QColor m_xAxisColor;
  QColor m_yAxisColor;
  QColor m_zAxisColor;
  QColor m_axisTextColor;
  QColor m_lineHeadColor;
  QColor m_lineTailColor;
  QColor m_gridMinorColor;
  QColor m_gridMajorColor;
  QColor m_innerBackgroundColor;
  QColor m_outerBackgroundColor;

  QPixmap m_plotPixmap[2];
  QPixmap m_gridPixmap[2];
  QPixmap m_backgroundPixmap[2];
  QPixmap m_cameraIndicatorPixmap[2];

  qreal m_orbitOffsetX;
  qreal m_orbitOffsetY;
  QPointF m_lastMousePos;
  qreal m_minimumWorldScale;

  QVector3D m_minPoint;
  QVector3D m_maxPoint;

  QSettings m_settings;
};
} // namespace Widgets
