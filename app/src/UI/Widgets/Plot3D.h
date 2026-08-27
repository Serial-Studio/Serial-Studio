/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QMatrix4x4>
#include <QPainter>
#include <QQuickItem>
#include <QVector3D>
#include <vector>

#include "DSP.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "UI/Dashboard.h"

QT_FORWARD_DECLARE_CLASS(QSGGeometryNode)
QT_FORWARD_DECLARE_CLASS(QSGSimpleTextureNode)

namespace Widgets {
/**
 * @brief 3D plotting widget with optional anaglyph (stereo) rendering.
 */
class Plot3D : public QQuickItem {
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
  Q_PROPERTY(bool autoCenter
             READ autoCenter
             WRITE setAutoCenter
             NOTIFY autoCenterChanged)
  Q_PROPERTY(bool autoScale
             READ autoScale
             WRITE setAutoScale
             NOTIFY autoScaleChanged)
  // clang-format on

signals:
  void rangeChanged();
  void cameraChanged();
  void autoScaleChanged();
  void autoCenterChanged();
  void eyeSeparationChanged();
  void anaglyphEnabledChanged();
  void orbitNavigationChanged();
  void invertEyePositionsChanged();
  void interpolationEnabledChanged();

public:
  explicit Plot3D(const int index = -1, QQuickItem* parent = nullptr);

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

  [[nodiscard]] bool autoScale() const;
  [[nodiscard]] bool autoCenter() const;
  [[nodiscard]] bool orbitNavigation() const;
  [[nodiscard]] bool interpolationEnabled() const;

  [[nodiscard]] const QSize& widgetSize() const;

public slots:
  void setWorldScale(const double z);
  void setCameraAngleX(const double angle);
  void setCameraAngleY(const double angle);
  void setCameraAngleZ(const double angle);
  void setCameraOffsetX(const double offset);
  void setCameraOffsetY(const double offset);
  void setCameraOffsetZ(const double offset);
  void setAutoScale(const bool enabled);
  void setAutoCenter(const bool enabled);
  void setAnaglyphEnabled(const bool enabled);
  void setOrbitNavigation(const bool enabled);
  void setEyeSeparation(const float separation);
  void setInvertEyePositions(const bool enabled);
  void setInterpolationEnabled(const bool enabled);

private slots:
  void updateData();
  void onThemeChanged();

private:
  /**
   * @brief Channel mask applied to a stereo eye: the left eye keeps red, the right keeps
   *        green and blue, matching the channel split of the retired per-pixel merge.
   */
  enum class EyeMask {
    None,
    Left,
    Right
  };

  void markDirty();
  void markCameraDirty();
  void updateSize();
  void updateTargetScale();
  void updateCamera(const DSP::LineSeries3D& data);

  void drawData();
  void drawGrid();
  void drawCameraIndicator();

  void projectLine3D(const QMatrix4x4& matrix,
                     const QVector3D& p1,
                     const QVector3D& p2,
                     const QColor& color,
                     std::vector<QPointF>& px,
                     std::vector<QColor>& colors) const;
  void appendGridLine(const QMatrix4x4& matrix,
                      const QVector3D& p1,
                      const QVector3D& p2,
                      const QColor& color);
  void appendAxisLine(const QMatrix4x4& matrix,
                      const QVector3D& p1,
                      const QVector3D& p2,
                      const QColor& color);
  [[nodiscard]] static QColor maskEyeColor(const QColor& color, const EyeMask mask);
  void applyCameraTransform(QMatrix4x4& matrix) const;
  void buildGridPolylines(const QMatrix4x4& matrix, const EyeMask mask);
  void buildTracePolyline(const QMatrix4x4& matrix,
                          const DSP::LineSeries3D& data,
                          const EyeMask mask);
  void syncBackgroundNode(QSGNode* root, const QRectF& rect);
  void drawGridLabel();
  void syncTraceNode();
  void syncTileNode(QSGSimpleTextureNode*& slot,
                    const QImage& tile,
                    const QPointF& topLeft,
                    bool& needsUpload);
  void appendSceneNodes(QSGNode* root);
  void syncStrokeNode(QSGGeometryNode*& slot,
                      const std::vector<QPointF>& px,
                      const std::vector<QColor>& colors,
                      const double halfWidth);

private:
  [[nodiscard]] qreal displayPixelRatio() const;
  [[nodiscard]] double gridStep(const double scale = -1) const;
  const std::vector<QPointF>& screenProjection(const DSP::LineSeries3D& points,
                                               const QMatrix4x4& matrix);

  QPair<QMatrix4x4, QMatrix4x4> eyeTransformations(const QMatrix4x4& matrix);

protected:
  QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) override;
  void updatePolish() override;
  void wheelEvent(QWheelEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void itemChange(ItemChange change, const ItemChangeData& value) override;

private:
  int m_index;

  double m_worldScale;
  double m_cameraAngleX;
  double m_cameraAngleY;
  double m_cameraAngleZ;
  double m_cameraOffsetX;
  double m_cameraOffsetY;
  double m_cameraOffsetZ;

  float m_eyeSeparation;

  bool m_anaglyph;
  bool m_autoScale;
  bool m_autoCenter;
  bool m_interpolate;
  bool m_orbitNavigation;
  bool m_invertEyePositions;

  bool m_dirtyData;
  bool m_dirtyGrid;
  bool m_dirtyBackground;
  bool m_dirtyCameraIndicator;
  bool m_dataUpdated;

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

  std::vector<QPointF> m_projected;

  QImage m_bgTile;
  QImage m_labelTile;
  QImage m_indicatorTile;
  QString m_gridStepLabel;
  QPointF m_labelPos;
  bool m_dirtyLabel;
  bool m_bgUpload;
  bool m_labelUpload;
  bool m_indicatorUpload;

  std::vector<QPointF> m_linePx;
  std::vector<QColor> m_lineColors;
  std::vector<QPointF> m_dashPx;
  std::vector<QColor> m_dashColors;
  std::vector<QPointF> m_gridPx;
  std::vector<QColor> m_gridColors;
  std::vector<QPointF> m_axisPx;
  std::vector<QColor> m_axisColors;
  std::vector<QPointF> m_tracePx;
  std::vector<QColor> m_traceColors;

  QSGSimpleTextureNode* m_bgNode;
  QSGGeometryNode* m_gridNode;
  QSGGeometryNode* m_axisNode;
  QSGGeometryNode* m_traceNode;
  QSGSimpleTextureNode* m_labelNode;
  QSGSimpleTextureNode* m_indicatorNode;

  double m_orbitOffsetX;
  double m_orbitOffsetY;
  QPointF m_lastMousePos;

  QVector3D m_minPoint;
  QVector3D m_maxPoint;
  QVector3D m_centerPoint;
  QVector3D m_targetCenter;
  double m_targetWorldScale;
  int m_shrinkTicks;
  bool m_centerInitialized;

  QSize m_size;

  UI::Dashboard& m_dashboard;
  Misc::TimerEvents& m_timerEvents;
  Misc::ThemeManager& m_themeManager;
  Misc::CommonFonts& m_commonFonts;
};
}  // namespace Widgets
