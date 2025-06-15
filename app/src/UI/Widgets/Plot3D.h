/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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
  Q_PROPERTY(double worldScale
             READ worldScale
             WRITE setWorldScale
             NOTIFY cameraChanged)
  Q_PROPERTY(double cameraAngleX
             READ cameraAngleX
             WRITE setCameraAngleX
             NOTIFY cameraChanged)
  Q_PROPERTY(double cameraAngleY
             READ cameraAngleY
             WRITE setCameraAngleY
             NOTIFY cameraChanged)
  Q_PROPERTY(double cameraAngleZ
             READ cameraAngleZ
             WRITE setCameraAngleZ
             NOTIFY cameraChanged)
  Q_PROPERTY(double cameraOffsetX
             READ cameraOffsetX
             WRITE setCameraOffsetX
             NOTIFY cameraChanged)
  Q_PROPERTY(double cameraOffsetY
             READ cameraOffsetY
             WRITE setCameraOffsetY
             NOTIFY cameraChanged)
  Q_PROPERTY(double cameraOffsetZ
             READ cameraOffsetZ
             WRITE setCameraOffsetZ
             NOTIFY cameraChanged)
  Q_PROPERTY(double idealWorldScale
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

  [[nodiscard]] double worldScale() const;
  [[nodiscard]] double cameraAngleX() const;
  [[nodiscard]] double cameraAngleY() const;
  [[nodiscard]] double cameraAngleZ() const;
  [[nodiscard]] double cameraOffsetX() const;
  [[nodiscard]] double cameraOffsetY() const;
  [[nodiscard]] double cameraOffsetZ() const;
  [[nodiscard]] double idealWorldScale() const;

  [[nodiscard]] bool dirty() const;

  [[nodiscard]] float eyeSeparation() const;
  [[nodiscard]] bool anaglyphEnabled() const;
  [[nodiscard]] bool invertEyePositions() const;

  [[nodiscard]] bool orbitNavigation() const;
  [[nodiscard]] bool interpolationEnabled() const;

public slots:
  void setWorldScale(const double z);
  void setCameraAngleX(const double angle);
  void setCameraAngleY(const double angle);
  void setCameraAngleZ(const double angle);
  void setCameraOffsetX(const double offset);
  void setCameraOffsetY(const double offset);
  void setCameraOffsetZ(const double offset);
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
  double gridStep(const double scale = -1) const;
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

  double m_minX;
  double m_maxX;
  double m_minY;
  double m_maxY;
  double m_minZ;
  double m_maxZ;

  double m_worldScale;
  double m_cameraAngleX;
  double m_cameraAngleY;
  double m_cameraAngleZ;
  double m_cameraOffsetX;
  double m_cameraOffsetY;
  double m_cameraOffsetZ;

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

  double m_orbitOffsetX;
  double m_orbitOffsetY;
  QPointF m_lastMousePos;
  double m_minimumWorldScale;

  QVector3D m_minPoint;
  QVector3D m_maxPoint;

  QSettings m_settings;
};
} // namespace Widgets
