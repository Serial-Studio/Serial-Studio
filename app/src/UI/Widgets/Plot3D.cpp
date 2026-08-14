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
#include <QQuickWindow>
#include <QtNumeric>

#include "DataModel/HotpathOptimization.h"
#include "DSP.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"

static constexpr float kNearPlane = 0.1f;
static constexpr float kFarPlane  = 100.0f;

static constexpr int kScaleShrinkDelay    = 30;
static constexpr double kFitPadding       = 1.2;
static constexpr double kInvFitSteps      = 1.0 / 6.0;
static constexpr double kScaleShrinkRatio = 0.35;

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
  , m_worldScale(0.05)
  , m_cameraAngleX(300)
  , m_cameraAngleY(0)
  , m_cameraAngleZ(225)
  , m_cameraOffsetX(0)
  , m_cameraOffsetY(0)
  , m_cameraOffsetZ(-10)
  , m_eyeSeparation(0.069f)
  , m_anaglyph(false)
  , m_autoScale(true)
  , m_autoCenter(false)
  , m_interpolate(true)
  , m_orbitNavigation(true)
  , m_invertEyePositions(false)
  , m_dirtyData(true)
  , m_dirtyGrid(true)
  , m_dirtyBackground(true)
  , m_dirtyCameraIndicator(true)
  , m_dataUpdated(true)
  , m_orbitOffsetX(0)
  , m_orbitOffsetY(0)
  , m_targetWorldScale(1.0)
  , m_shrinkTicks(0)
  , m_centerInitialized(false)
  , m_dashboard(UI::Dashboard::instance())
  , m_timerEvents(Misc::TimerEvents::instance())
  , m_themeManager(Misc::ThemeManager::instance())
  , m_commonFonts(Misc::CommonFonts::instance())
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

  connect(&m_dashboard, &UI::Dashboard::updated, this, &Widgets::Plot3D::updateData);

  connect(this, &Widgets::Plot3D::widthChanged, this, &Widgets::Plot3D::updateSize);
  connect(this, &Widgets::Plot3D::heightChanged, this, &Widgets::Plot3D::updateSize);
  connect(this, &Widgets::Plot3D::scaleChanged, this, &Widgets::Plot3D::updateSize);

  if (VALIDATE_WIDGET(SerialStudio::DashboardPlot3D, m_index)) {
    connect(&m_timerEvents, &Misc::TimerEvents::uiTimeout, this, [=, this] {
      if (isVisible() && dirty())
        update();
    });
  }

  onThemeChanged();
  connect(
    &m_themeManager, &Misc::ThemeManager::themeChanged, this, &Widgets::Plot3D::onThemeChanged);
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

  if (m_dirtyData)
    drawData();

  if (m_dirtyGrid)
    drawGrid();

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
    const qreal dpr = displayPixelRatio();
    if (m_anaglyphImg[0].size() != widgetSize() || m_anaglyphImg[0].devicePixelRatio() != dpr) {
      m_anaglyphImg[0] = QImage(widgetSize(), QImage::Format_ARGB32_Premultiplied);
      m_anaglyphImg[1] = QImage(widgetSize(), QImage::Format_ARGB32_Premultiplied);
      m_anaglyphMerged = QImage(widgetSize(), QImage::Format_RGB32);
      m_anaglyphImg[0].setDevicePixelRatio(dpr);
      m_anaglyphImg[1].setDevicePixelRatio(dpr);
      m_anaglyphMerged.setDevicePixelRatio(dpr);
    }

    m_anaglyphImg[0].fill(Qt::transparent);
    m_anaglyphImg[1].fill(Qt::transparent);

    QPainter leftScene(&m_anaglyphImg[0]);
    for (const auto* p : images)
      leftScene.drawImage(0, 0, p[0]);

    QPainter rightScene(&m_anaglyphImg[1]);
    for (const auto* p : images)
      rightScene.drawImage(0, 0, p[1]);

    const int h = m_anaglyphImg[0].height();
    const int w = m_anaglyphImg[0].width();
    for (int y = 0; y < h; ++y) {
      const QRgb* SS_RESTRICT lLine =
        reinterpret_cast<const QRgb*>(m_anaglyphImg[0].constScanLine(y));
      const QRgb* SS_RESTRICT rLine =
        reinterpret_cast<const QRgb*>(m_anaglyphImg[1].constScanLine(y));
      QRgb* SS_RESTRICT oLine = reinterpret_cast<QRgb*>(m_anaglyphMerged.scanLine(y));

      for (int x = 0; x < w; ++x)
        oLine[x] = qRgb(qRed(lLine[x]), qGreen(rLine[x]), qBlue(rLine[x]));
    }

    painter->drawImage(0, 0, m_anaglyphMerged);
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

  const double targetStep = maxExtent * kFitPadding * kInvFitSteps;
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
 * @brief Returns whether automatic world-scale tracking of the data extent is enabled.
 */
bool Widgets::Plot3D::autoScale() const
{
  return m_autoScale;
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
 * @brief Sets the zoom level of the 3D plot; any explicit zoom disables automatic scaling.
 */
void Widgets::Plot3D::setWorldScale(const double z)
{
  setAutoScale(false);

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
 * @brief Enables or disables automatic world-scale tracking of the data extent.
 */
void Widgets::Plot3D::setAutoScale(const bool enabled)
{
  if (m_autoScale != enabled) {
    m_autoScale = enabled;
    Q_EMIT autoScaleChanged();
  }
}

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
  if (!isVisible())
    return;

  if (!VALIDATE_WIDGET(SerialStudio::DashboardPlot3D, m_index))
    return;

  m_dirtyData   = true;
  m_dataUpdated = true;
}

/**
 * @brief Updates plot colors based on the current theme; a valid per-dataset override in the
 *        group is drawn verbatim as the head color, with only the tail derived from it.
 */
void Widgets::Plot3D::onThemeChanged()
{
  QColor custom;
  if (VALIDATE_WIDGET(SerialStudio::DashboardPlot3D, m_index))
    custom = SerialStudio::getGroupColorOverride(GET_GROUP(SerialStudio::DashboardPlot3D, m_index));

  const QColor base = custom.isValid() ? custom : SerialStudio::getDatasetColor(m_index + 1);
  m_lineHeadColor   = custom.isValid() ? base : base.darker(130);
  m_lineTailColor   = base.lighter(130);
  m_lineTailColor.setAlpha(156);

  // clang-format off
  m_textColor = m_themeManager.getColor("widget_text");
  m_xAxisColor = m_themeManager.getColor("plot3d_x_axis");
  m_yAxisColor = m_themeManager.getColor("plot3d_y_axis");
  m_zAxisColor = m_themeManager.getColor("plot3d_z_axis");
  m_axisTextColor = m_themeManager.getColor("plot3d_axis_text");
  m_gridMinorColor = m_themeManager.getColor("plot3d_grid_minor");
  m_gridMajorColor = m_themeManager.getColor("plot3d_grid_major");
  m_innerBackgroundColor = m_themeManager.getColor("widget_base");
  m_outerBackgroundColor = m_themeManager.getColor("widget_window");
  // clang-format on

  const double invSteps = 1.0 / static_cast<double>(m_gradientPens.size() - 1);
  for (std::size_t i = 0; i < m_gradientPens.size(); ++i) {
    const double t = static_cast<double>(i) * invSteps;
    QColor c;
    c.setRedF(m_lineTailColor.redF() * (1 - t) + m_lineHeadColor.redF() * t);
    c.setGreenF(m_lineTailColor.greenF() * (1 - t) + m_lineHeadColor.greenF() * t);
    c.setBlueF(m_lineTailColor.blueF() * (1 - t) + m_lineHeadColor.blueF() * t);
    m_gradientPens[i] = QPen(c, 2);
  }

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
  auto dpr = displayPixelRatio();
  m_size   = QSize(static_cast<int>(width() * dpr), static_cast<int>(height() * dpr));

  markDirty();
}

/**
 * @brief Returns the device pixel ratio of the hosting window, with an app-level fallback.
 */
qreal Widgets::Plot3D::displayPixelRatio() const
{
  const auto* win = window();
  return win ? win->devicePixelRatio() : qApp->devicePixelRatio();
}

/**
 * @brief Re-derives the cached device-pixel size when the effective pixel ratio changes.
 */
void Widgets::Plot3D::itemChange(ItemChange change, const ItemChangeData& value)
{
  if (change == ItemDevicePixelRatioHasChanged)
    updateSize();

  if (change == ItemVisibleHasChanged && value.boolValue)
    updateData();

  QQuickPaintedItem::itemChange(change, value);
}

/**
 * @brief Re-targets the world scale with hysteresis: refit immediately when data overflows
 *        the fitted view, refit smaller only after a persistent shrink well below it. The
 *        0.35 shrink ratio sits below the worst post-refit ideal/fitted ratio (2/5), so a
 *        refit always lands inside the keep band and re-targeting can never oscillate.
 */
void Widgets::Plot3D::updateTargetScale()
{
  if (!m_centerInitialized) {
    m_targetWorldScale = idealWorldScale();
    return;
  }

  const double dx        = m_maxPoint.x() - m_minPoint.x();
  const double dy        = m_maxPoint.y() - m_minPoint.y();
  const double dz        = m_maxPoint.z() - m_minPoint.z();
  const double maxExtent = qMax(dz, qMax(dx, dy));
  if (maxExtent < 1e-9)
    return;

  const double idealStep  = maxExtent * kFitPadding * kInvFitSteps;
  const double fittedStep = 1.0 / m_targetWorldScale;
  const double ratio      = idealStep / fittedStep;

  if (ratio > 1.0) {
    m_targetWorldScale = idealWorldScale();
    m_shrinkTicks      = 0;
  }

  else if (ratio < kScaleShrinkRatio) {
    if (++m_shrinkTicks >= kScaleShrinkDelay) {
      m_targetWorldScale = idealWorldScale();
      m_shrinkTicks      = 0;
    }
  }

  else
    m_shrinkTicks = 0;
}

/**
 * @brief Scans the data extent and converges the camera center and world scale toward it,
 *        stepping once per dashboard data update so pointer-driven repaints (orbit, pan)
 *        cannot fast-forward the easing into a visible scale jump.
 */
void Widgets::Plot3D::updateCamera(const DSP::LineSeries3D& data)
{
  SS_ASSERT(!data.empty(), return);

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

  updateTargetScale();

  bool moved = false;
  if (!m_centerInitialized) {
    m_centerPoint       = m_targetCenter;
    m_worldScale        = m_targetWorldScale;
    m_centerInitialized = true;
    moved               = true;
  }

  else {
    if (m_autoCenter && m_centerPoint != m_targetCenter) {
      const QVector3D delta = m_targetCenter - m_centerPoint;
      if (delta.lengthSquared() <= (max - min).lengthSquared() * 1e-8f)
        m_centerPoint = m_targetCenter;
      else
        m_centerPoint += delta * 0.08f;

      moved = true;
    }

    if (m_autoScale && m_worldScale != m_targetWorldScale) {
      const double delta = m_targetWorldScale - m_worldScale;
      if (std::abs(delta) <= m_targetWorldScale * 1e-3)
        m_worldScale = m_targetWorldScale;
      else
        m_worldScale += delta * 0.15;

      moved = true;
    }
  }

  if (moved) {
    m_dirtyGrid = true;
    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Renders the 3D plot foreground.
 */
void Widgets::Plot3D::drawData()
{
  const auto& data = m_dashboard.plotData3D(m_index);
  if (data.empty()) {
    m_plotImg[0] = QImage();
    m_plotImg[1] = QImage();
    m_dirtyData  = false;
    return;
  }

  if (m_dataUpdated) {
    updateCamera(data);
    m_dataUpdated = false;
  }

  QMatrix4x4 matrix;
  matrix.perspective(45.0f, float(width()) / height(), kNearPlane, kFarPlane);
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
  matrix.perspective(45.0f, float(width()) / height(), kNearPlane, kFarPlane);
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
  img.setDevicePixelRatio(displayPixelRatio());
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

  const double rawStep  = 1.0 / s;
  const double exponent = std::floor(std::log10(rawStep));
  const double base     = fastPow10(exponent);

  if (rawStep >= base * 5)
    return base * 5;
  else if (rawStep >= base * 2)
    return base * 2;
  else
    return base;
}

/**
 * @brief Projects 3D world-space points into 2D screen-space coordinates, writing a NaN
 *        sentinel for points behind the near plane so callers can skip broken segments.
 */
const std::vector<QPointF>& Widgets::Plot3D::screenProjection(const DSP::LineSeries3D& points,
                                                              const QMatrix4x4& matrix)
{
  m_projected.clear();
  m_projected.reserve(points.size());

  const float halfW = width() * 0.5f;
  const float halfH = height() * 0.5f;

  for (const QVector3D& p : points) {
    QVector4D v = matrix * QVector4D(p, 1.0f);

    if (v.w() < kNearPlane) {
      m_projected.push_back(QPointF(qQNaN(), qQNaN()));
      continue;
    }

    const float invW = 1.0f / v.w();
    const float ndcX = v.x() * invW;
    const float ndcY = v.y() * invW;

    const float screenX = halfW + ndcX * halfW;
    const float screenY = halfH - ndcY * halfH;
    m_projected.push_back(QPointF(screenX, screenY));
  }

  return m_projected;
}

/**
 * @brief Renders a near-plane-clipped 3D line as faded 2D segments, projecting only its two
 *        endpoints since a projective transform maps straight lines to straight lines.
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

  QVector4D a = matrix * QVector4D(p1, 1.0f);
  QVector4D b = matrix * QVector4D(p2, 1.0f);
  if (a.w() < kNearPlane && b.w() < kNearPlane)
    return;

  if (a.w() < kNearPlane)
    a += (b - a) * ((kNearPlane - a.w()) / (b.w() - a.w()));
  else if (b.w() < kNearPlane)
    b += (a - b) * ((kNearPlane - b.w()) / (a.w() - b.w()));

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

  const QPointF pStart(halfW + (a.x() / a.w()) * halfW, halfH - (a.y() / a.w()) * halfH);
  const QPointF pEnd(halfW + (b.x() / b.w()) * halfW, halfH - (b.y() / b.w()) * halfH);
  const QPointF span = pEnd - pStart;

  QPen pen(color, lineWidth, style);
  constexpr float kInvSegments = 1.0f / segmentCount;
  for (int i = 0; i < segmentCount; ++i) {
    const QPointF pA = pStart + span * (float(i) * kInvSegments);
    const QPointF pB = pStart + span * (float(i + 1) * kInvSegments);

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

    pen.setColor(faded);
    painter.setPen(pen);
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
  img.setDevicePixelRatio(displayPixelRatio());
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
  painter.setFont(m_commonFonts.monoFont());
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
  img.setDevicePixelRatio(displayPixelRatio());
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

  painter.setFont(m_commonFonts.customMonoFont(0.8));
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
  img.setDevicePixelRatio(displayPixelRatio());
  img.fill(Qt::transparent);

  QPainter painter(&img);
  painter.setRenderHint(QPainter::Antialiasing, true);

  const auto& points = screenProjection(data, matrix);

  if (m_interpolate) {
    const auto numPoints   = static_cast<qsizetype>(points.size());
    const double invPoints = numPoints > 0 ? 1.0 / numPoints : 0.0;
    const double lutScale  = static_cast<double>(m_gradientPens.size() - 1);
    for (qsizetype i = 1; i < numPoints; ++i) {
      const QPointF& a = points[i - 1];
      const QPointF& b = points[i];
      if (qIsNaN(a.x()) || qIsNaN(b.x()))
        continue;

      const auto idx = static_cast<std::size_t>(double(i) * invPoints * lutScale);
      painter.setPen(m_gradientPens[idx]);
      painter.drawLine(a, b);
    }
  }

  else {
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_lineHeadColor);
    for (const QPointF& pt : points)
      if (!qIsNaN(pt.x()))
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
  lMatrix.rotate(angleDeg, 0, 1, 0);

  QMatrix4x4 rMatrix = matrix;
  rMatrix.translate(-shift, 0.0f, 0.0f);
  rMatrix.rotate(-angleDeg, 0, 1, 0);

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
