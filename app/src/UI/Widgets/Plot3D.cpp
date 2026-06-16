/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/Plot3D.h"

#include <cmath>
#include <QCursor>

#include "DSP.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "UI/Dashboard.h"

/**
 * @brief Integer-exponent 10^n via table lookup; std::pow fallback for out-of-band values.
 */
static double fastPow10(double exponent) noexcept
{
  static constexpr double kTable[] = {
    1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5,
    1e-4,  1e-3,  1e-2,  1e-1,  1e0,   1e1,   1e2,  1e3,  1e4,  1e5,  1e6,
    1e7,   1e8,   1e9,   1e10,  1e11,  1e12,  1e13, 1e14, 1e15,
  };

  const int idx = static_cast<int>(exponent) + 15;
  if (idx < 0 || idx >= static_cast<int>(sizeof(kTable) / sizeof(kTable[0]))) [[unlikely]]
    return std::pow(10.0, exponent);

  return kTable[idx];
}

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Plot3D widget.
 */
Widgets::Plot3D::Plot3D(const int index, QQuickItem* parent)
  : QQuickPaintedItem(parent)
  , m_index(index)
  , m_minX(INT_MAX)
  , m_maxX(INT_MIN)
  , m_minY(INT_MAX)
  , m_maxY(INT_MIN)
  , m_minZ(INT_MAX)
  , m_maxZ(INT_MIN)
  , m_worldScale(0.05)
  , m_cameraAngleX(300)
  , m_cameraAngleY(0)
  , m_cameraAngleZ(225)
  , m_cameraOffsetX(0)
  , m_cameraOffsetY(0)
  , m_cameraOffsetZ(-10)
  , m_eyeSeparation(0.069f)
  , m_anaglyph(false)
  , m_autoCenter(false)
  , m_interpolate(true)
  , m_orbitNavigation(true)
  , m_invertEyePositions(false)
  , m_dirtyData(true)
  , m_dirtyGrid(true)
  , m_dirtyBackground(true)
  , m_dirtyCameraIndicator(true)
  , m_targetWorldScale(1.0)
  , m_centerInitialized(false)
{
  setOpaquePainting(true);
  setAcceptHoverEvents(true);
  setFiltersChildMouseEvents(true);

  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);

  setMipmap(true);
  setAntialiasing(false);

  connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this, &Widgets::Plot3D::updateData);

  connect(this, &Widgets::Plot3D::widthChanged, this, &Widgets::Plot3D::updateSize);
  connect(this, &Widgets::Plot3D::heightChanged, this, &Widgets::Plot3D::updateSize);
  connect(this, &Widgets::Plot3D::scaleChanged, this, &Widgets::Plot3D::updateSize);

  if (VALIDATE_WIDGET(SerialStudio::DashboardPlot3D, m_index)) {
    connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::uiTimeout, this, [=, this] {
      if (isVisible() && dirty())
        update();
    });
  }

  onThemeChanged();
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &Widgets::Plot3D::onThemeChanged);

  connect(
    this, &Widgets::Plot3D::rangeChanged, this, [this] { m_targetWorldScale = idealWorldScale(); });
}

//--------------------------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Renders the complete 3D plot scene, blending a red-cyan anaglyph when enabled.
 */
void Widgets::Plot3D::paint(QPainter* painter)
{
  painter->setBackground(m_outerBackgroundColor);

  if (m_dirtyGrid)
    drawGrid();

  if (m_dirtyData)
    drawData();

  if (m_dirtyCameraIndicator)
    drawCameraIndicator();

  if (m_dirtyBackground)
    drawBackground();

  QList<QImage*> images;
  images.append(m_bgImg);

  if (m_cameraAngleX <= 270 && m_cameraAngleX > 90.0) {
    images.append(m_plotImg);
    images.append(m_gridImg);
  }

  else {
    images.append(m_gridImg);
    images.append(m_plotImg);
  }

  images.append(m_cameraIndicatorImg);

  if (anaglyphEnabled()) {
    QImage left(widgetSize(), QImage::Format_ARGB32_Premultiplied);
    left.setDevicePixelRatio(qApp->devicePixelRatio());
    left.fill(Qt::transparent);

    QImage right(widgetSize(), QImage::Format_ARGB32_Premultiplied);
    right.setDevicePixelRatio(qApp->devicePixelRatio());
    right.fill(Qt::transparent);

    QPainter leftScene(&left);
    for (const auto* p : images)
      leftScene.drawImage(0, 0, p[0]);

    QPainter rightScene(&right);
    for (const auto* p : images)
      rightScene.drawImage(0, 0, p[1]);

    QImage finalImage(widgetSize(), QImage::Format_RGB32);
    finalImage.setDevicePixelRatio(qApp->devicePixelRatio());
    const int h = left.height();
    const int w = left.width();
    for (int y = 0; y < h; ++y) {
      const auto* lLine = reinterpret_cast<const QRgb*>(left.constScanLine(y));
      const auto* rLine = reinterpret_cast<const QRgb*>(right.constScanLine(y));
      auto* oLine       = reinterpret_cast<QRgb*>(finalImage.scanLine(y));

      for (int x = 0; x < w; ++x)
        oLine[x] = qRgb(qRed(lLine[x]), qGreen(rLine[x]), qBlue(rLine[x]));
    }

    painter->drawImage(0, 0, finalImage);
  }

  else {
    for (const auto* p : images)
      painter->drawImage(0, 0, p[0]);
  }
}

//--------------------------------------------------------------------------------------------------
// Camera control getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current worldScale level of the 3D plot.
 */
double Widgets::Plot3D::worldScale() const
{
  return m_worldScale;
}

/**
 * @brief Returns the X-axis rotation.
 */
double Widgets::Plot3D::cameraAngleX() const
{
  return m_cameraAngleX;
}

/**
 * @brief Returns the Y-axis rotation.
 */
double Widgets::Plot3D::cameraAngleY() const
{
  return m_cameraAngleY;
}

/**
 * @brief Returns the Z-axis rotation.
 */
double Widgets::Plot3D::cameraAngleZ() const
{
  return m_cameraAngleZ;
}

/**
 * @brief Returns the X-axis camera offset.
 */
double Widgets::Plot3D::cameraOffsetX() const
{
  return m_cameraOffsetX;
}

/**
 * @brief Returns the Y-axis camera offset.
 */
double Widgets::Plot3D::cameraOffsetY() const
{
  return m_cameraOffsetY;
}

/**
 * @brief Returns the Z-axis camera offset.
 */
double Widgets::Plot3D::cameraOffsetZ() const
{
  return m_cameraOffsetZ;
}

/**
 * @brief Returns the ideal zoom level for the plot.
 */
double Widgets::Plot3D::idealWorldScale() const
{
  const double dx = m_maxPoint.x() - m_minPoint.x();
  const double dy = m_maxPoint.y() - m_minPoint.y();
  const double dz = m_maxPoint.z() - m_minPoint.z();

  const double maxExtent = qMax(dz, qMax(dx, dy));
  if (maxExtent < 1e-9)
    return m_worldScale;

  constexpr double kInv6  = 1.0 / 6.0;
  const double targetStep = (maxExtent * 1.2) * kInv6;
  const double exponent   = std::floor(std::log10(targetStep));
  const double base       = fastPow10(exponent);
  double snappedStep;
  if (targetStep <= base)
    snappedStep = base;
  else if (targetStep <= base * 2.0)
    snappedStep = base * 2.0;
  else if (targetStep <= base * 5.0)
    snappedStep = base * 5.0;
  else
    snappedStep = base * 10.0;

  return qBound(1e-9, 1.0 / snappedStep, 1e9);
}

//--------------------------------------------------------------------------------------------------
// Display option getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks if any part of the plot needs to be redrawn.
 */
bool Widgets::Plot3D::dirty() const
{
  return m_dirtyGrid || m_dirtyData || m_dirtyBackground || m_dirtyCameraIndicator;
}

/**
 * @brief Returns the current eye separation value.
 */
float Widgets::Plot3D::eyeSeparation() const
{
  return m_eyeSeparation;
}

/**
 * @brief Checks if anaglyph (red/cyan 3D) rendering mode is enabled.
 */
bool Widgets::Plot3D::anaglyphEnabled() const
{
  return m_anaglyph;
}

/**
 * @brief Checks whether eye positions are inverted for stereo rendering.
 */
bool Widgets::Plot3D::invertEyePositions() const
{
  return m_invertEyePositions;
}

/**
 * @brief Returns whether auto-centering on incoming data is enabled.
 */
bool Widgets::Plot3D::autoCenter() const
{
  return m_autoCenter;
}

/**
 * @brief Returns whether orbit navigation mode is active.
 */
bool Widgets::Plot3D::orbitNavigation() const
{
  return m_orbitNavigation;
}

/**
 * @brief Checks if interpolation is enabled for the plot.
 */
bool Widgets::Plot3D::interpolationEnabled() const
{
  return m_interpolate;
}

/**
 * @brief Returns the widget size in device pixels.
 */
const QSize& Widgets::Plot3D::widgetSize() const
{
  return m_size;
}

//--------------------------------------------------------------------------------------------------
// Camera control setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the zoom level of the 3D plot.
 */
void Widgets::Plot3D::setWorldScale(const double z)
{
  auto limited = qBound(1e-9, z, 1e9);
  if (m_worldScale != limited) {
    m_worldScale = limited;
    markCameraDirty();

    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Sets the X-axis rotation.
 */
void Widgets::Plot3D::setCameraAngleX(const double angle)
{
  if (m_cameraAngleX != angle) {
    m_cameraAngleX = angle;
    markCameraDirty();

    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Sets the Y-axis rotation.
 */
void Widgets::Plot3D::setCameraAngleY(const double angle)
{
  if (m_cameraAngleY != angle) {
    m_cameraAngleY = angle;
    markCameraDirty();

    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Sets the Z-axis rotation.
 */
void Widgets::Plot3D::setCameraAngleZ(const double angle)
{
  if (m_cameraAngleZ != angle) {
    m_cameraAngleZ = angle;
    markCameraDirty();

    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Sets the X-axis camera offset.
 */
void Widgets::Plot3D::setCameraOffsetX(const double offset)
{
  if (m_cameraOffsetX != offset) {
    m_cameraOffsetX = offset;
    markCameraDirty();
    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Sets the Y-axis camera offset.
 */
void Widgets::Plot3D::setCameraOffsetY(const double offset)
{
  if (m_cameraOffsetY != offset) {
    m_cameraOffsetY = offset;
    markCameraDirty();
    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Sets the Z-axis camera offset.
 */
void Widgets::Plot3D::setCameraOffsetZ(const double offset)
{
  if (m_cameraOffsetZ != offset) {
    m_cameraOffsetZ = offset;
    markCameraDirty();
    Q_EMIT cameraChanged();
  }
}

//--------------------------------------------------------------------------------------------------
// Display option setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enables or disables automatic centering on incoming data.
 */
void Widgets::Plot3D::setAutoCenter(const bool enabled)
{
  if (m_autoCenter != enabled) {
    m_autoCenter = enabled;

    if (!enabled) {
      m_centerPoint  = QVector3D(0, 0, 0);
      m_targetCenter = QVector3D(0, 0, 0);
      markDirty();
    }

    Q_EMIT autoCenterChanged();
  }
}

/**
 * @brief Enables or disables anaglyph (stereo 3D) rendering.
 */
void Widgets::Plot3D::setAnaglyphEnabled(const bool enabled)
{
  if (m_anaglyph != enabled) {
    m_anaglyph = enabled;

    markDirty();

    Q_EMIT anaglyphEnabledChanged();
  }
}

/**
 * @brief Enables or disables orbit navigation mode.
 */
void Widgets::Plot3D::setOrbitNavigation(const bool enabled)
{
  if (m_orbitNavigation != enabled) {
    m_orbitNavigation = enabled;
    Q_EMIT orbitNavigationChanged();
  }
}

/**
 * @brief Sets the eye separation value for stereo rendering.
 */
void Widgets::Plot3D::setEyeSeparation(const float separation)
{
  m_eyeSeparation = separation;
  markCameraDirty();

  Q_EMIT eyeSeparationChanged();
}

/**
 * @brief Sets whether to invert the eye positions for stereo rendering.
 */
void Widgets::Plot3D::setInvertEyePositions(const bool enabled)
{
  if (m_invertEyePositions != enabled) {
    m_invertEyePositions = enabled;
    markCameraDirty();

    Q_EMIT invertEyePositionsChanged();
  }
}

/**
 * @brief Enables or disables interpolation for the plot.
 */
void Widgets::Plot3D::setInterpolationEnabled(const bool enabled)
{
  if (m_interpolate != enabled) {
    m_interpolate = enabled;
    m_dirtyData   = true;
    update();

    Q_EMIT interpolationEnabledChanged();
  }
}

//--------------------------------------------------------------------------------------------------
// Data updates & theme
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the 3D plot data and prepares it for rendering.
 */
void Widgets::Plot3D::updateData()
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardPlot3D, m_index))
    return;

  m_dirtyData = true;
}

/**
 * @brief Updates plot colors based on the current theme.
 */
void Widgets::Plot3D::onThemeChanged()
{
  const auto color = SerialStudio::getDatasetColor(m_index + 1);
  m_lineHeadColor  = color;

  QColor midCurve(m_lineHeadColor);
  m_lineHeadColor = midCurve.darker(130);
  m_lineTailColor = midCurve.lighter(130);
  m_lineTailColor.setAlpha(156);

  // clang-format off
  m_textColor = Misc::ThemeManager::instance().getColor("widget_text");
  m_xAxisColor = Misc::ThemeManager::instance().getColor("plot3d_x_axis");
  m_yAxisColor = Misc::ThemeManager::instance().getColor("plot3d_y_axis");
  m_zAxisColor = Misc::ThemeManager::instance().getColor("plot3d_z_axis");
  m_axisTextColor = Misc::ThemeManager::instance().getColor("plot3d_axis_text");
  m_gridMinorColor = Misc::ThemeManager::instance().getColor("plot3d_grid_minor");
  m_gridMajorColor = Misc::ThemeManager::instance().getColor("plot3d_grid_major");
  m_innerBackgroundColor = Misc::ThemeManager::instance().getColor("widget_base");
  m_outerBackgroundColor = Misc::ThemeManager::instance().getColor("widget_window");
  // clang-format on

  markDirty();
}

//--------------------------------------------------------------------------------------------------
// State management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks all plot layers as dirty and requests a repaint.
 */
void Widgets::Plot3D::markDirty()
{
  m_dirtyGrid            = true;
  m_dirtyData            = true;
  m_dirtyBackground      = true;
  m_dirtyCameraIndicator = true;
  update();
}

/**
 * @brief Marks projection-dependent layers dirty without touching the background.
 */
void Widgets::Plot3D::markCameraDirty()
{
  m_dirtyGrid            = true;
  m_dirtyData            = true;
  m_dirtyCameraIndicator = true;
  update();
}

/**
 * @brief Updates the internal size to match the current widget size in device pixels.
 */
void Widgets::Plot3D::updateSize()
{
  auto dpr = qApp->devicePixelRatio();
  m_size   = QSize(static_cast<int>(width() * dpr), static_cast<int>(height() * dpr));

  markDirty();
}

/**
 * @brief Renders the 3D plot foreground.
 */
void Widgets::Plot3D::drawData()
{
  const auto& data = UI::Dashboard::instance().plotData3D(m_index);
  if (data.size() <= 0)
    return;

  QVector3D min = data.front();
  QVector3D max = data.front();
  for (const auto& p : data) {
    min.setX(qMin(min.x(), p.x()));
    min.setY(qMin(min.y(), p.y()));
    min.setZ(qMin(min.z(), p.z()));
    max.setX(qMax(max.x(), p.x()));
    max.setY(qMax(max.y(), p.y()));
    max.setZ(qMax(max.z(), p.z()));
  }

  if (m_minPoint != min || m_maxPoint != max) {
    m_minPoint     = min;
    m_maxPoint     = max;
    m_targetCenter = (min + max) * 0.5f;
    Q_EMIT rangeChanged();
  }

  if (!m_centerInitialized) {
    m_centerPoint       = m_targetCenter;
    m_worldScale        = m_targetWorldScale;
    m_centerInitialized = true;
  }

  else if (m_autoCenter)
    m_centerPoint += (m_targetCenter - m_centerPoint) * 0.08f;

  QMatrix4x4 matrix;
  matrix.perspective(45.0f, float(width()) / height(), 0.1f, 100.0f);
  matrix.translate(m_cameraOffsetX, m_cameraOffsetY, m_cameraOffsetZ);

  if (anaglyphEnabled()) {
    auto eyes = eyeTransformations(matrix);

    eyes.first.rotate(m_cameraAngleX, 1, 0, 0);
    eyes.first.rotate(m_cameraAngleY, 0, 1, 0);
    eyes.first.rotate(m_cameraAngleZ, 0, 0, 1);
    eyes.first.scale(m_worldScale);
    eyes.first.translate(-m_centerPoint);

    eyes.second.rotate(m_cameraAngleX, 1, 0, 0);
    eyes.second.rotate(m_cameraAngleY, 0, 1, 0);
    eyes.second.rotate(m_cameraAngleZ, 0, 0, 1);
    eyes.second.scale(m_worldScale);
    eyes.second.translate(-m_centerPoint);

    m_plotImg[0] = renderData(eyes.first, data);
    m_plotImg[1] = renderData(eyes.second, data);
  }

  else {
    matrix.rotate(m_cameraAngleX, 1, 0, 0);
    matrix.rotate(m_cameraAngleY, 0, 1, 0);
    matrix.rotate(m_cameraAngleZ, 0, 0, 1);
    matrix.scale(m_worldScale);
    matrix.translate(-m_centerPoint);

    m_plotImg[0] = renderData(matrix, data);
  }

  m_dirtyData = false;
}

/**
 * @brief Renders the 3D plot background.
 */
void Widgets::Plot3D::drawGrid()
{
  QMatrix4x4 matrix;
  matrix.perspective(45.0f, float(width()) / height(), 0.1f, 100.0f);
  matrix.translate(m_cameraOffsetX, m_cameraOffsetY, m_cameraOffsetZ);

  if (anaglyphEnabled()) {
    auto eyes = eyeTransformations(matrix);

    eyes.first.rotate(m_cameraAngleX, 1, 0, 0);
    eyes.first.rotate(m_cameraAngleY, 0, 1, 0);
    eyes.first.rotate(m_cameraAngleZ, 0, 0, 1);
    eyes.first.scale(m_worldScale);
    eyes.first.translate(-m_centerPoint);

    eyes.second.rotate(m_cameraAngleX, 1, 0, 0);
    eyes.second.rotate(m_cameraAngleY, 0, 1, 0);
    eyes.second.rotate(m_cameraAngleZ, 0, 0, 1);
    eyes.second.scale(m_worldScale);
    eyes.second.translate(-m_centerPoint);

    m_gridImg[0] = renderGrid(eyes.first);
    m_gridImg[1] = renderGrid(eyes.second);
  }

  else {
    matrix.rotate(m_cameraAngleX, 1, 0, 0);
    matrix.rotate(m_cameraAngleY, 0, 1, 0);
    matrix.rotate(m_cameraAngleZ, 0, 0, 1);
    matrix.scale(m_worldScale);
    matrix.translate(-m_centerPoint);

    m_gridImg[0] = renderGrid(matrix);
  }

  m_dirtyGrid = false;
}

/**
 * @brief Renders the 3D plot background with optional anaglyph effect.
 */
void Widgets::Plot3D::drawBackground()
{
  QImage img(widgetSize(), QImage::Format_ARGB32_Premultiplied);
  img.setDevicePixelRatio(qApp->devicePixelRatio());
  img.fill(Qt::transparent);

  QPointF center(width() * 0.5f, height() * 0.5f);
  double radius = qMax(width(), height()) * 0.25;
  QRadialGradient gradient(center, radius);
  gradient.setColorAt(0.0, m_innerBackgroundColor);
  gradient.setColorAt(1.0, m_outerBackgroundColor);

  QPainter painter(&img);
  painter.fillRect(boundingRect(), gradient);

  m_bgImg[0] = img;
  if (anaglyphEnabled())
    m_bgImg[1] = img;

  m_dirtyBackground = false;
}

/**
 * @brief Renders the 3D camera indicator.
 */
void Widgets::Plot3D::drawCameraIndicator()
{
  QMatrix4x4 matrix;

  if (anaglyphEnabled()) {
    auto eyes = eyeTransformations(matrix);

    eyes.first.rotate(m_cameraAngleX, 1, 0, 0);
    eyes.first.rotate(m_cameraAngleY, 0, 1, 0);
    eyes.first.rotate(m_cameraAngleZ, 0, 0, 1);

    eyes.second.rotate(m_cameraAngleX, 1, 0, 0);
    eyes.second.rotate(m_cameraAngleY, 0, 1, 0);
    eyes.second.rotate(m_cameraAngleZ, 0, 0, 1);

    m_cameraIndicatorImg[0] = renderCameraIndicator(eyes.first);
    m_cameraIndicatorImg[1] = renderCameraIndicator(eyes.second);
  }

  else {
    matrix.rotate(m_cameraAngleX, 1, 0, 0);
    matrix.rotate(m_cameraAngleY, 0, 1, 0);
    matrix.rotate(m_cameraAngleZ, 0, 0, 1);
    m_cameraIndicatorImg[0] = renderCameraIndicator(matrix);
  }

  m_dirtyCameraIndicator = false;
}

//--------------------------------------------------------------------------------------------------
// Rendering pipeline helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes a clean grid step based on current world scale.
 */
double Widgets::Plot3D::gridStep(const double scale) const
{
  auto s = scale;
  if (s == -1)
    s = m_worldScale;

  const float rawStep  = 1.0f / s;
  const float exponent = std::floor(std::log10(rawStep));
  const float base     = static_cast<float>(fastPow10(exponent));

  if (rawStep >= base * 5)
    return base * 5;
  else if (rawStep >= base * 2)
    return base * 2;
  else
    return base;
}

/**
 * @brief Projects 3D world-space points into 2D screen-space coordinates.
 */
std::vector<QPointF> Widgets::Plot3D::screenProjection(const DSP::LineSeries3D& points,
                                                       const QMatrix4x4& matrix)
{
  std::vector<QPointF> projected;
  projected.reserve(points.size());

  const float halfW = width() * 0.5f;
  const float halfH = height() * 0.5f;

  for (const QVector3D& p : points) {
    QVector4D v = matrix * QVector4D(p, 1.0f);

    if (DSP::isZero(v.w()))
      continue;

    const float invW = 1.0f / v.w();
    const float ndcX = v.x() * invW;
    const float ndcY = v.y() * invW;

    const float screenX = halfW + ndcX * halfW;
    const float screenY = halfH - ndcY * halfH;
    projected.push_back(QPointF(screenX, screenY));
  }

  return projected;
}

/**
 * @brief Projects and renders a 3D line as a faded 2D segment on screen.
 */
void Widgets::Plot3D::drawLine3D(QPainter& painter,
                                 const QMatrix4x4& matrix,
                                 const QVector3D& p1,
                                 const QVector3D& p2,
                                 QColor color,
                                 float lineWidth,
                                 Qt::PenStyle style)
{
  constexpr int segmentCount = 40;

  const float w     = width();
  const float h     = height();
  const float halfW = w * 0.5f;
  const float halfH = h * 0.5f;
  const QPointF center(halfW, halfH);
  const float maxDist    = 0.5f * std::hypot(w, h);
  const float invMaxDist = maxDist > 0.0f ? 1.0f / maxDist : 0.0f;

  const float screenRatio = 0.4f;
  const float xLimit      = w * screenRatio;
  const float yLimit      = h * screenRatio;

  for (int i = 0; i < segmentCount; ++i) {
    float t1    = float(i) / segmentCount;
    float t2    = float(i + 1) / segmentCount;
    QVector3D a = (1.0f - t1) * p1 + t1 * p2;
    QVector3D b = (1.0f - t2) * p1 + t2 * p2;

    const auto projected = screenProjection({a, b}, matrix);
    if (projected.size() != 2)
      continue;

    const QPointF& pA = projected[0];
    const QPointF& pB = projected[1];

    const bool exceedPAx = std::abs(pA.x() - halfW) > xLimit;
    const bool exceedPBx = std::abs(pB.x() - halfW) > xLimit;
    const bool exceedPAy = std::abs(pA.y() - halfH) > yLimit;
    const bool exceedPBy = std::abs(pB.y() - halfH) > yLimit;
    if (exceedPAx || exceedPBx || exceedPAy || exceedPBy)
      continue;

    QPointF mid = 0.5f * (pA + pB);
    float dist  = QLineF(mid, center).length();
    float alpha = 1.0f - std::clamp(dist * invMaxDist, 0.0f, 1.0f);

    QColor faded = color;
    faded.setAlphaF(color.alphaF() * alpha);

    painter.setPen(QPen(faded, lineWidth, style));
    painter.drawLine(pA, pB);
  }
}

//--------------------------------------------------------------------------------------------------
// Grid computation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Renders the infinite grid overlay as a 2D pixmap.
 */
QImage Widgets::Plot3D::renderGrid(const QMatrix4x4& matrix)
{
  QImage img(widgetSize(), QImage::Format_ARGB32_Premultiplied);
  img.setDevicePixelRatio(qApp->devicePixelRatio());
  img.fill(Qt::transparent);

  QPainter painter(&img);
  painter.setRenderHint(QPainter::Antialiasing, true);

  const double numSteps = 10;
  const double step     = gridStep();
  const double l        = numSteps * step;
  const float cx        = std::round(m_centerPoint.x() / step) * step;
  const float cy        = std::round(m_centerPoint.y() / step) * step;

  QVector<QPair<QVector3D, QVector3D>> gridLines;
  for (int i = -numSteps; i <= numSteps; ++i) {
    if (i == 0)
      continue;

    float x       = cx + i * step;
    float y       = cy + i * step;
    const auto x1 = QVector3D(x, cy + l, 0);
    const auto x2 = QVector3D(x, cy - l, 0);
    const auto y1 = QVector3D(cx + l, y, 0);
    const auto y2 = QVector3D(cx - l, y, 0);

    gridLines.append({x1, x2});
    gridLines.append({y1, y2});
  }

  const float ax                    = m_centerPoint.x();
  const float ay                    = m_centerPoint.y();
  QPair<QVector3D, QVector3D> xAxis = {QVector3D(ax - l, ay, 0), QVector3D(ax + l, ay, 0)};
  QPair<QVector3D, QVector3D> yAxis = {QVector3D(ax, ay - l, 0), QVector3D(ax, ay + l, 0)};

  auto color = m_gridMinorColor;
  color.setAlpha(100);
  for (const auto& line : gridLines)
    drawLine3D(painter, matrix, line.first, line.second, color, 1, Qt::DashLine);

  drawLine3D(painter, matrix, xAxis.first, xAxis.second, m_xAxisColor, 1.5, Qt::SolidLine);
  drawLine3D(painter, matrix, yAxis.first, yAxis.second, m_yAxisColor, 1.5, Qt::SolidLine);

  const QString stepLabel = tr("Grid Interval: %1 unit(s)").arg(step);
  painter.setPen(m_textColor);
  painter.setFont(Misc::CommonFonts::instance().monoFont());
  painter.drawText(QPoint(8, height() - 8), stepLabel);

  return img;
}

/**
 * @brief Renders the camera orientation indicator as a 2D pixmap.
 */
QImage Widgets::Plot3D::renderCameraIndicator(const QMatrix4x4& matrix)
{
  struct Axis {
    QVector3D dir;
    QColor color;
    QString label;
  };

  struct TransformedAxis {
    Axis axis;
    QVector4D transformed;
  };

  QImage img(widgetSize(), QImage::Format_ARGB32_Premultiplied);
  img.setDevicePixelRatio(qApp->devicePixelRatio());
  img.fill(Qt::transparent);

  if (width() < 240 || height() < 240)
    return img;

  QPainter painter(&img);
  painter.setRenderHint(QPainter::Antialiasing, true);

  const float lineScale  = 18;
  const float axisLength = 2;
  const QPointF origin(50, 50);

  QVector<Axis> axes = {
    {{1, 0, 0}, m_xAxisColor, QStringLiteral("X")},
    {{0, 1, 0}, m_yAxisColor, QStringLiteral("Y")},
    {{0, 0, 1}, m_zAxisColor, QStringLiteral("Z")}
  };

  QVector<TransformedAxis> transformedAxes;
  for (const auto& ax : axes) {
    QVector4D t = matrix * QVector4D(ax.dir * axisLength, 1.0f);
    transformedAxes.append({ax, t});
  }

  std::sort(transformedAxes.begin(),
            transformedAxes.end(),
            [](const TransformedAxis& a, const TransformedAxis& b) {
              return a.transformed.z() < b.transformed.z();
            });

  painter.setFont(Misc::CommonFonts::instance().customMonoFont(0.8));
  QFontMetrics fm(painter.font());
  int textWidth      = fm.horizontalAdvance("X");
  int textHeight     = fm.height();
  float circleRadius = std::max(textWidth, textHeight) * 0.7f;

  for (const auto& ta : transformedAxes) {
    const QVector4D& t = ta.transformed;
    const float invW   = t.w() != 0.0f ? 1.0f / t.w() : 0.0f;
    QPointF endpoint(origin.x() + (t.x() * invW) * lineScale,
                     origin.y() - (t.y() * invW) * lineScale);

    painter.setPen(QPen(ta.axis.color, 3));
    painter.drawLine(origin, endpoint);

    painter.setBrush(ta.axis.color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(endpoint, circleRadius, circleRadius);

    QRectF textRect(
      endpoint.x() - circleRadius, endpoint.y() - circleRadius, circleRadius * 2, circleRadius * 2);

    painter.setPen(m_axisTextColor);
    painter.drawText(textRect, Qt::AlignCenter, ta.axis.label);
  }

  return img;
}

/**
 * @brief Renders the 3D plot foreground as a 2D pixmap.
 */
QImage Widgets::Plot3D::renderData(const QMatrix4x4& matrix, const DSP::LineSeries3D& data)
{
  QImage img(widgetSize(), QImage::Format_ARGB32_Premultiplied);
  img.setDevicePixelRatio(qApp->devicePixelRatio());
  img.fill(Qt::transparent);

  QPainter painter(&img);
  painter.setRenderHint(QPainter::Antialiasing, true);

  auto points = screenProjection(data, matrix);

  if (m_interpolate) {
    const auto& endColor   = m_lineHeadColor;
    const auto& startColor = m_lineTailColor;
    const auto numPoints   = static_cast<qsizetype>(points.size());
    const double invPoints = numPoints > 0 ? 1.0 / numPoints : 0.0;
    for (qsizetype i = 1; i < numPoints; ++i) {
      QColor c;
      double t = double(i) * invPoints;
      c.setRedF(startColor.redF() * (1 - t) + endColor.redF() * t);
      c.setGreenF(startColor.greenF() * (1 - t) + endColor.greenF() * t);
      c.setBlueF(startColor.blueF() * (1 - t) + endColor.blueF() * t);

      painter.setPen(QPen(c, 2));
      painter.drawLine(points[i - 1], points[i]);
    }
  }

  else {
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_lineHeadColor);
    for (const QPointF& pt : std::as_const(points))
      painter.drawEllipse(pt, 1, 1);
  }

  return img;
}

//--------------------------------------------------------------------------------------------------
// Projection & transformation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Calculates the left and right eye transformation matrices for 3D anaglyph rendering.
 */
QPair<QMatrix4x4, QMatrix4x4> Widgets::Plot3D::eyeTransformations(const QMatrix4x4& matrix)
{
  float shift = m_eyeSeparation / (2.0f) * (m_invertEyePositions ? -1 : 1);

  const float distance = 10.0f;
  const float angleRad = std::atan(shift / distance);
  const float angleDeg = angleRad * 180.0f / float(M_PI);

  QMatrix4x4 lMatrix = matrix;
  lMatrix.translate(shift, 0.0f, 0.0f);
  lMatrix.rotate(angleDeg, 0, 0, 1);

  QMatrix4x4 rMatrix = matrix;
  rMatrix.translate(-shift, 0.0f, 0.0f);
  rMatrix.rotate(-angleDeg, 0, 0, 1);

  return qMakePair(lMatrix, rMatrix);
}

//--------------------------------------------------------------------------------------------------
// Input event handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles mouse wheel events to worldScale in or out of the 3D plot.
 */
void Widgets::Plot3D::wheelEvent(QWheelEvent* event)
{
  if (event->angleDelta().y() != 0) {
    event->accept();

    const bool isTouchpad =
      !event->pixelDelta().isNull() || event->source() == Qt::MouseEventSynthesizedBySystem;
    const double zoomFactor  = isTouchpad ? 1.05 : 1.06;
    constexpr double kInv120 = 1.0 / 120.0;
    const double delta       = -event->angleDelta().y() * kInv120;
    const double factor      = qPow(zoomFactor, -delta);

    setWorldScale(worldScale() * factor);
  }
}

/**
 * @brief Handles mouse movement events to rotate or pan the 3D camera.
 */
void Widgets::Plot3D::mouseMoveEvent(QMouseEvent* event)
{
  QPointF delta  = event->pos() - m_lastMousePos;
  m_lastMousePos = event->pos();

  if (m_orbitNavigation) {
    m_cameraOffsetX  = m_orbitOffsetX;
    m_cameraOffsetY  = m_orbitOffsetY;
    m_cameraAngleZ  += delta.x() * 0.5;
    m_cameraAngleX  += delta.y() * 0.5;

    m_cameraAngleZ = fmod(m_cameraAngleZ, 360.0);
    if (m_cameraAngleZ < 0)
      m_cameraAngleZ += 360.0;

    m_cameraAngleX = fmod(m_cameraAngleX, 360.0);
    if (m_cameraAngleX < 0)
      m_cameraAngleX += 360.0;
  }

  else {
    m_cameraOffsetX += delta.x() * 0.01;
    m_cameraOffsetY -= delta.y() * 0.01;
  }

  event->accept();

  markCameraDirty();
  Q_EMIT cameraChanged();
}

/**
 * @brief Handles mouse press events to start dragging and change cursor.
 */
void Widgets::Plot3D::mousePressEvent(QMouseEvent* event)
{
  m_lastMousePos = event->pos();

  if (m_orbitNavigation) {
    float offsetFactor = 0.25;
    float biasX        = ((m_lastMousePos.x() / width()) - 0.5f) * 2.0f;
    float biasY        = (0.5f - (m_lastMousePos.y() / height())) * 2.0f;

    m_orbitOffsetX = m_cameraOffsetX - biasX * offsetFactor;
    m_orbitOffsetY = m_cameraOffsetY - biasY * offsetFactor;
  }

  else {
    m_orbitOffsetX = m_cameraOffsetX;
    m_orbitOffsetY = m_cameraOffsetY;
  }

  grabMouse();
  setCursor(Qt::ClosedHandCursor);
  event->accept();
}

/**
 * @brief Handles mouse release events to stop dragging and reset cursor.
 */
void Widgets::Plot3D::mouseReleaseEvent(QMouseEvent* event)
{
  unsetCursor();
  ungrabMouse();
  ungrabTouchPoints();
  event->accept();

  m_orbitOffsetX = m_cameraOffsetX;
  m_orbitOffsetY = m_cameraOffsetY;
}
