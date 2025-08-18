/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include <QCursor>

#include "UI/Dashboard.h"

#include "Misc/TimerEvents.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"

#include "UI/Widgets/Plot3D.h"

/**
 * @brief Constructs a Plot3D widget.
 * @param index The index of the Plot3D in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::Plot3D::Plot3D(const int index, QQuickItem *parent)
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
  , m_interpolate(true)
  , m_orbitNavigation(true)
  , m_invertEyePositions(false)
  , m_dirtyData(true)
  , m_dirtyGrid(true)
  , m_dirtyCameraIndicator(true)
{
  // Read settings
  m_anaglyph = m_settings.value("Plot3D_Anaglyph", false).toBool();
  m_interpolate = m_settings.value("Plot3D_Interpolate", true).toBool();
  m_eyeSeparation = m_settings.value("Plot3D_EyeSeparation", 0.069).toFloat();
  m_invertEyePositions = m_settings.value("Plot3D_InvertEyes", false).toBool();

  // Configure QML item behavior
  setOpaquePainting(true);
  setAcceptHoverEvents(true);
  setFiltersChildMouseEvents(true);

  // Configure item flags
  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);

  // Set rendering hints
  setMipmap(true);
  setAntialiasing(false);

  // Update the plot data
  connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
          &Widgets::Plot3D::updateData);

  // Mark everything as dirty when widget size changes
  connect(this, &Widgets::Plot3D::widthChanged, this,
          &Widgets::Plot3D::updateSize);
  connect(this, &Widgets::Plot3D::heightChanged, this,
          &Widgets::Plot3D::updateSize);
  connect(this, &Widgets::Plot3D::scaleChanged, this,
          &Widgets::Plot3D::updateSize);

  // Obtain group information
  if (VALIDATE_WIDGET(SerialStudio::DashboardPlot3D, m_index))
  {
    connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::uiTimeout, this,
            [=, this] {
              if (isVisible() && dirty())
                update();
            });
  }

  // Connect to the theme manager to update the curve colors
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Widgets::Plot3D::onThemeChanged);

  // Set zoom automatically
  connect(this, &Widgets::Plot3D::rangeChanged, this, [=, this] {
    if (m_worldScale < idealWorldScale())
      setWorldScale(idealWorldScale());
  });
}

//------------------------------------------------------------------------------
// Main rendering/painting code
//------------------------------------------------------------------------------

/**
 * @brief Renders the complete 3D plot scene, including optional anaglyph.
 *
 * This method composites all plot layers—background, grid, plot data,
 * and camera indicator—into the provided QPainter target. It handles
 * dirty flags to re-render components as needed.
 *
 * If anaglyph mode is enabled, it:
 * - Composes the full scene into a single image.
 * - Generates left and right eye views (right view shifted horizontally).
 * - Manually blends the two views into a red-cyan anaglyph using per-pixel
 *   channel mixing (red from left, green+blue from right).
 *
 * If anaglyph is disabled, it simply draws the pre-rendered layers as-is.
 *
 * @param painter The QPainter to render the scene onto.
 */
void Widgets::Plot3D::paint(QPainter *painter)
{
  // Configure render hints
  painter->setBackground(m_outerBackgroundColor);

  // Re-draw background (if required)
  if (m_dirtyGrid)
    drawGrid();

  // Re-draw foreground (if required)
  if (m_dirtyData)
    drawData();

  // Re-draw camera indicator (if required)
  if (m_dirtyCameraIndicator)
    drawCameraIndicator();

  // Re-draw background (if required)
  if (m_dirtyBackground)
    drawBackground();

  // Generate list of images
  QList<QImage *> images;
  images.append(m_bgImg);

  // User is below axis/plane...draw plot first then grid
  if (m_cameraAngleX <= 270 && m_cameraAngleX > 90.0)
  {
    images.append(m_plotImg);
    images.append(m_gridImg);
  }

  // User is above axis/plane...draw grid first then plot
  else
  {
    images.append(m_gridImg);
    images.append(m_plotImg);
  }

  // Add camera indicator
  images.append(m_cameraIndicatorImg);

  // Anaglyph processing
  if (anaglyphEnabled())
  {
    // Initialize left eye pixmap
    QImage left(widgetSize(), QImage::Format_ARGB32_Premultiplied);
    left.setDevicePixelRatio(qApp->devicePixelRatio());
    left.fill(Qt::transparent);

    // Initialize right eye pixmap
    QImage right(widgetSize(), QImage::Format_ARGB32_Premultiplied);
    right.setDevicePixelRatio(qApp->devicePixelRatio());
    right.fill(Qt::transparent);

    // Compose the left eye scene
    QPainter leftScene(&left);
    for (const auto *p : images)
      leftScene.drawImage(0, 0, p[0]);

    // Compose the right eye scene
    QPainter rightScene(&right);
    for (const auto *p : images)
      rightScene.drawImage(0, 0, p[1]);

    // Build the anaglyph manually
    QImage finalImage(widgetSize(), QImage::Format_RGB32);
    finalImage.setDevicePixelRatio(qApp->devicePixelRatio());
    for (int y = 0; y < left.height(); ++y)
    {
      for (int x = 0; x < left.width(); ++x)
      {
        // Obtain pixels from both left and right image
        const auto lRgb = left.pixel(x, y);
        const auto rRgb = right.pixel(x, y);

        // Preserve red from left and cyan (green + blue) from right
        const auto outR = qRed(lRgb);
        const auto outG = qGreen(rRgb);
        const auto outB = qBlue(rRgb);
        finalImage.setPixel(x, y, qRgb(outR, outG, outB));
      }
    }

    // Draw the final image
    painter->drawImage(0, 0, finalImage);
  }

  // Standard processing
  else
  {
    for (const auto *p : images)
      painter->drawImage(0, 0, p[0]);
  }
}

//------------------------------------------------------------------------------
// Member access functions
//------------------------------------------------------------------------------

/**
 * @brief Returns the current worldScale level of the 3D plot.
 * @return The worldScale factor, where 1.0 is the default scale.
 */
double Widgets::Plot3D::worldScale() const
{
  return m_worldScale;
}

/**
 * @brief Returns the X-axis rotation.
 * @return The X-axis rotation angle in degrees.
 */
double Widgets::Plot3D::cameraAngleX() const
{
  return m_cameraAngleX;
}

/**
 * @brief Returns the Y-axis rotation.
 * @return The Y-axis rotation angle in degrees.
 */
double Widgets::Plot3D::cameraAngleY() const
{
  return m_cameraAngleY;
}

/**
 * @brief Returns the Z-axis rotation.
 * @return The Z-axis rotation angle in degrees.
 */
double Widgets::Plot3D::cameraAngleZ() const
{
  return m_cameraAngleZ;
}

/**
 * @brief Returns the X-axis camera offset.
 * @return The X-axis offset value.
 */
double Widgets::Plot3D::cameraOffsetX() const
{
  return m_cameraOffsetX;
}

/**
 * @brief Returns the Y-axis camera offset.
 * @return The Y-axis offset value.
 */
double Widgets::Plot3D::cameraOffsetY() const
{
  return m_cameraOffsetY;
}

/**
 * @brief Returns the Z-axis camera offset.
 * @return The Z-axis offset value.
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

  const double distance = qMax(1e-9, std::sqrt(dx * dx + dy * dy + dz * dz));
  const double targetWorldSize = distance * 2.0;
  const double targetStep = targetWorldSize / 10.0;

  const double estimatedScale = 1.0 / targetStep;
  const int base = static_cast<int>(std::log(estimatedScale) / std::log(0.95));

  const int minIndex = base - 20;
  const int maxIndex = base + 20;

  double scale = 1e-9;
  for (int i = minIndex; i <= maxIndex; ++i)
  {
    scale = std::pow(0.95, i);
    const double step = gridStep(scale);
    const double totalVisibleWorld = 10.0 * step;

    if (totalVisibleWorld >= targetWorldSize)
    {
      scale = qBound(1e-9, scale, 1e9);
      break;
    }
  }

  return scale;
}
/**
 * @brief Checks if any part of the plot needs to be redrawn.
 *
 * @return true if the background, foreground, or camera indicator
 *         is marked as dirty and requires a repaint; false otherwise.
 */
bool Widgets::Plot3D::dirty() const
{
  return m_dirtyGrid || m_dirtyData || m_dirtyCameraIndicator;
}

/**
 * @brief Returns the current eye separation value.
 *
 * This value controls the virtual distance between the left and right
 * camera views when rendering in anaglyph (stereo 3D) mode. It affects
 * the strength of the depth/parallax effect.
 *
 * @return The eye separation distance as a float.
 */
float Widgets::Plot3D::eyeSeparation() const
{
  return m_eyeSeparation;
}

/**
 * @brief Checks if anaglyph (red/cyan 3D) rendering mode is enabled.
 *
 * @return True if anaglyph mode is currently active; false otherwise.
 */
bool Widgets::Plot3D::anaglyphEnabled() const
{
  return m_anaglyph;
}

/**
 * @brief Checks whether eye positions are inverted for stereo rendering.
 *
 * In stereo/anaglyph mode, this flag determines if the left and right
 * eye positions should be swapped. It's useful for cases where the
 * stereo effect appears reversed (e.g., objects look "inside out").
 *
 * @return True if eye positions are inverted; otherwise false.
 */
bool Widgets::Plot3D::invertEyePositions() const
{
  return m_invertEyePositions;
}

/**
 * @brief Checks if orbit navigation is enabled.
 *
 * When orbit navigation is enabled, the view allows rotating (orbiting)
 * around the 3D plot. If disabled, the navigation mode switches to panning.
 *
 * @return true if orbit navigation is enabled; false if panning mode is active.
 */
bool Widgets::Plot3D::orbitNavigation() const
{
  return m_orbitNavigation;
}

/**
 * @brief Checks if interpolation is enabled for the plot.
 *
 * If interpolation is enabled, points in the plot are connected via lines.
 * If disabled, only discrete points are displayed without connecting lines.
 *
 * @return true if interpolation is enabled; false if only points are shown.
 */
bool Widgets::Plot3D::interpolationEnabled() const
{
  return m_interpolate;
}

/**
 * @brief Returns the widget size in device pixels.
 *
 * Computes the widget's size multiplied by the current device pixel ratio,
 * accounting for high-DPI displays.
 *
 * @return QSize representing the width and height in device pixels.
 */
const QSize &Widgets::Plot3D::widgetSize() const
{
  return m_size;
}

//------------------------------------------------------------------------------
// Public slots
//------------------------------------------------------------------------------

/**
 * @brief Sets the zoom level of the 3D plot.
 * @param z The zoom factor; values greater than 1.0 zoom in,
 *          less than 1.0 zoom out.
 *
 * Emits the cameraChanged() signal if the value is updated.
 */
void Widgets::Plot3D::setWorldScale(const double z)
{
  auto limited = qBound(idealWorldScale(), z, 10e9);
  if (m_worldScale != limited)
  {
    m_worldScale = limited;
    markDirty();

    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Sets the X-axis rotation.
 * @param angle The X-axis rotation angle in degrees.
 *
 * Emits the cameraChanged() signal if the value is updated.
 */
void Widgets::Plot3D::setCameraAngleX(const double angle)
{
  if (m_cameraAngleX != angle)
  {
    m_cameraAngleX = angle;
    markDirty();

    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Sets the Y-axis rotation.
 * @param angle The Y-axis rotation angle in degrees.
 *
 * Emits the cameraChanged() signal if the value is updated.
 */
void Widgets::Plot3D::setCameraAngleY(const double angle)
{
  if (m_cameraAngleY != angle)
  {
    m_cameraAngleY = angle;
    markDirty();

    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Sets the Z-axis rotation.
 * @param angle The Z-axis rotation angle in degrees.
 *
 * Emits the cameraChanged() signal if the value is updated.
 */
void Widgets::Plot3D::setCameraAngleZ(const double angle)
{
  if (m_cameraAngleZ != angle)
  {
    m_cameraAngleZ = angle;
    markDirty();

    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Sets the X-axis camera offset.
 * @param offset The new X-axis offset value.
 */
void Widgets::Plot3D::setCameraOffsetX(const double offset)
{
  if (m_cameraOffsetX != offset)
  {
    m_cameraOffsetX = offset;
    markDirty();
    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Sets the Y-axis camera offset.
 * @param offset The new Y-axis offset value.
 */
void Widgets::Plot3D::setCameraOffsetY(const double offset)
{
  if (m_cameraOffsetY != offset)
  {
    m_cameraOffsetY = offset;
    markDirty();
    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Sets the Z-axis camera offset.
 * @param offset The new Z-axis offset value.
 */
void Widgets::Plot3D::setCameraOffsetZ(const double offset)
{
  if (m_cameraOffsetZ != offset)
  {
    m_cameraOffsetZ = offset;
    markDirty();
    Q_EMIT cameraChanged();
  }
}

/**
 * @brief Enables or disables anaglyph (red/cyan 3D) rendering mode.
 *
 * This function updates the internal anaglyph mode flag. If the value
 * changes, it marks all layers as dirty to trigger re-rendering and emits
 * the anaglyphEnabledChanged() signal to notify connected components.
 *
 * @param enabled True to enable anaglyph mode; false to disable it.
 */
void Widgets::Plot3D::setAnaglyphEnabled(const bool enabled)
{
  if (m_anaglyph != enabled)
  {
    m_anaglyph = enabled;
    m_settings.setValue("Plot3D_Anaglyph", enabled);

    markDirty();

    Q_EMIT anaglyphEnabledChanged();
  }
}

/**
 * @brief Enables or disables orbit navigation mode.
 *
 * When enabled, the view allows rotating (orbiting) around the 3D model.
 * When disabled, the navigation switches to panning mode.
 *
 * Emits the orbitNavigationChanged() signal if the state changes.
 *
 * @param enabled Set to true to enable orbit navigation; false to enable
 *        panning.
 */
void Widgets::Plot3D::setOrbitNavigation(const bool enabled)
{
  if (m_orbitNavigation != enabled)
  {
    m_orbitNavigation = enabled;
    Q_EMIT orbitNavigationChanged();
  }
}

/**
 * @brief Sets the eye separation value for stereo rendering.
 *
 * This defines the virtual distance between the left and right eye views
 * when rendering in anaglyph mode. A larger separation increases the 3D
 * depth effect, while a smaller separation reduces it.
 *
 * Marks the plot as dirty and emits eyeSeparationChanged().
 *
 * @param separation The new eye separation distance.
 */
void Widgets::Plot3D::setEyeSeparation(const float separation)
{
  m_eyeSeparation = separation;
  m_settings.setValue("Plot3D_EyeSeparation", separation);
  markDirty();

  Q_EMIT eyeSeparationChanged();
}

/**
 * @brief Sets whether to invert the eye positions for stereo rendering.
 *
 * This controls if the left and right eye positions are swapped when rendering
 * in stereo/anaglyph mode. Use this if the depth effect appears incorrect or
 * reversed. Triggers a re-render if the value changes.
 *
 * @param enabled True to invert eye positions; false to use standard positions.
 */
void Widgets::Plot3D::setInvertEyePositions(const bool enabled)
{
  if (m_invertEyePositions != enabled)
  {
    m_invertEyePositions = enabled;
    m_settings.setValue("Plot3D_InvertEyes", enabled);
    markDirty();

    Q_EMIT invertEyePositionsChanged();
  }
}

/**
 * @brief Enables or disables interpolation for the plot.
 *
 * When enabled, points in the plot are connected with lines.
 * When disabled, only discrete points are shown.
 *
 * Triggers a re-render by calling markDirty() and emits the
 * interpolationEnabledChanged() signal if the state changes.
 *
 * @param enabled Set to true to connect points with lines; false to show only
 *        points.
 */
void Widgets::Plot3D::setInterpolationEnabled(const bool enabled)
{
  if (m_interpolate != enabled)
  {
    m_interpolate = enabled;
    m_settings.setValue("Plot3D_Interpolate", enabled);
    markDirty();

    Q_EMIT interpolationEnabledChanged();
  }
}

//------------------------------------------------------------------------------
// Private slots
//------------------------------------------------------------------------------

/**
 * @brief Updates the 3D plot data and prepares it for rendering.
 *
 * This function retrieves 3D data points from the dashboard, applies a
 * perspective projection and view transformation, and projects the points to
 * 2D screen space.
 *
 * The result is stored in m_points, and the foreground is marked dirty to
 * trigger a redraw.
 */
void Widgets::Plot3D::updateData()
{
  // Validate that the widget exists
  if (!VALIDATE_WIDGET(SerialStudio::DashboardPlot3D, m_index))
    return;

  // Re-draw line/curve on next pain event
  m_dirtyData = true;
}

/**
 * @brief Updates plot colors based on the current theme.
 *
 * Sets line head/tail colors as a brightness gradient around the base color.
 * Also updates grid and axis colors from the theme manager.
 */
void Widgets::Plot3D::onThemeChanged()
{
  // Obtain color for latest line data
  const auto &colors = Misc::ThemeManager::instance().widgetColors();
  if (colors.count() > m_index)
    m_lineHeadColor = colors.at(m_index);
  else
    m_lineHeadColor = colors.at(m_index % colors.count());

  // Create gradient based on widget index
  QColor midCurve(m_lineHeadColor);
  m_lineHeadColor = midCurve.darker(130);
  m_lineTailColor = midCurve.lighter(130);
  m_lineTailColor.setAlpha(156);

  // Obtain colors for XY plane & axes
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

  // Mark all widget as dirty to force re-rendering
  markDirty();
}

//------------------------------------------------------------------------------
// High-level scene rendering code
//------------------------------------------------------------------------------

/**
 * @brief Marks all plot layers as dirty and requests a repaint.
 *
 * This sets the background, foreground, and camera indicator flags to true,
 * ensuring they will be redrawn on the next update.
 * It then calls update() to schedule a repaint event.
 */
void Widgets::Plot3D::markDirty()
{
  m_dirtyGrid = true;
  m_dirtyData = true;
  m_dirtyBackground = true;
  m_dirtyCameraIndicator = true;
  update();
}

/**
 * @brief Updates the internal size to match the current widget size in device
 * pixels.
 *
 * Recalculates and stores the size using the device pixel ratio to support
 * high-DPI scaling.
 */
void Widgets::Plot3D::updateSize()
{
  auto dpr = qApp->devicePixelRatio();
  m_size = QSize(static_cast<int>(width() * dpr),
                 static_cast<int>(height() * dpr));

  markDirty();
}

/**
 * @brief Renders the 3D plot foreground.
 *
 * This function draws the main 3D data points or objects in the plot.
 *
 * Marks m_dirtyData as false to avoid unnecessary re-rendering.
 */
void Widgets::Plot3D::drawData()
{
  // Obtain data from dashboard
  const auto &data = UI::Dashboard::instance().plotData3D(m_index);
  if (data.size() <= 0)
    return;

  // Get min/max values
  QVector3D min, max;
  for (const auto &p : data)
  {
    min.setX(qMin(m_minPoint.x(), p.x()));
    min.setY(qMin(m_minPoint.y(), p.y()));
    min.setZ(qMin(m_minPoint.z(), p.z()));
    max.setX(qMax(m_maxPoint.x(), p.x()));
    max.setY(qMax(m_maxPoint.y(), p.y()));
    max.setZ(qMax(m_maxPoint.z(), p.z()));
  }

  // Min/max values changed
  if (m_minPoint != min || m_maxPoint != max)
  {
    m_minPoint = min;
    m_maxPoint = max;
    Q_EMIT rangeChanged();
  }

  // Initialize camera matrix
  QMatrix4x4 matrix;
  matrix.perspective(45.0f, float(width()) / height(), 0.1f, 100.0f);
  matrix.translate(m_cameraOffsetX, m_cameraOffsetY, m_cameraOffsetZ);

  // Render 3D pixmaps
  if (anaglyphEnabled())
  {
    // Slightly rotate & shift view for each eye
    auto eyes = eyeTransformations(matrix);

    // Apply world transformations for left eye
    eyes.first.rotate(m_cameraAngleX, 1, 0, 0);
    eyes.first.rotate(m_cameraAngleY, 0, 1, 0);
    eyes.first.rotate(m_cameraAngleZ, 0, 0, 1);
    eyes.first.scale(m_worldScale);

    // Apply world transformations for second eye
    eyes.second.rotate(m_cameraAngleX, 1, 0, 0);
    eyes.second.rotate(m_cameraAngleY, 0, 1, 0);
    eyes.second.rotate(m_cameraAngleZ, 0, 0, 1);
    eyes.second.scale(m_worldScale);

    // Render data
    m_plotImg[0] = renderData(eyes.first, data);
    m_plotImg[1] = renderData(eyes.second, data);
  }

  // Render single pixmap
  else
  {
    // Apply world transformations
    matrix.rotate(m_cameraAngleX, 1, 0, 0);
    matrix.rotate(m_cameraAngleY, 0, 1, 0);
    matrix.rotate(m_cameraAngleZ, 0, 0, 1);
    matrix.scale(m_worldScale);

    // Render data
    m_plotImg[0] = renderData(matrix, data);
  }

  // Mark dirty flag as false to avoid needless rendering
  m_dirtyData = false;
}

/**
 * @brief Renders the 3D plot background.
 *
 * This function draws the infinite grid and background axes behind the 3D plot.
 * Marks m_dirtyGrid as false to prevent unnecessary re-rendering.
 */
void Widgets::Plot3D::drawGrid()
{
  // Initialize camera matrix
  QMatrix4x4 matrix;
  matrix.perspective(45.0f, float(width()) / height(), 0.1f, 100.0f);
  matrix.translate(m_cameraOffsetX, m_cameraOffsetY, m_cameraOffsetZ);

  // Render 3D pixmaps
  if (anaglyphEnabled())
  {
    // Slightly rotate & shift view for each eye
    auto eyes = eyeTransformations(matrix);

    // Apply world transformations for left eye
    eyes.first.rotate(m_cameraAngleX, 1, 0, 0);
    eyes.first.rotate(m_cameraAngleY, 0, 1, 0);
    eyes.first.rotate(m_cameraAngleZ, 0, 0, 1);
    eyes.first.scale(m_worldScale);

    // Apply world transformations for second eye
    eyes.second.rotate(m_cameraAngleX, 1, 0, 0);
    eyes.second.rotate(m_cameraAngleY, 0, 1, 0);
    eyes.second.rotate(m_cameraAngleZ, 0, 0, 1);
    eyes.second.scale(m_worldScale);

    // Render grid
    m_gridImg[0] = renderGrid(eyes.first);
    m_gridImg[1] = renderGrid(eyes.second);
  }

  // Render 2D pixmap
  else
  {
    // Apply world transformations
    matrix.rotate(m_cameraAngleX, 1, 0, 0);
    matrix.rotate(m_cameraAngleY, 0, 1, 0);
    matrix.rotate(m_cameraAngleZ, 0, 0, 1);
    matrix.scale(m_worldScale);

    // Render grid
    m_gridImg[0] = renderGrid(matrix);
  }

  // Mark dirty flag as false to avoid needless rendering
  m_dirtyGrid = false;
}

/**
 * @brief Renders the 3D plot background with optional anaglyph effect.
 *
 * This function draws the radial gradient background behind the 3D plot.
 *
 * The result is stored in m_bgImg[0] (left eye) and
 * m_bgImg[1] (right eye) for anaglyph mode, or only in
 * m_bgImg[0] for normal rendering.
 *
 * Marks m_dirtyBackground as false to prevent unnecessary re-rendering.
 */
void Widgets::Plot3D::drawBackground()
{
  // Create a transparent image at high DPI resolution
  QImage img(widgetSize(), QImage::Format_ARGB32_Premultiplied);
  img.setDevicePixelRatio(qApp->devicePixelRatio());
  img.fill(Qt::transparent);

  // Set up a radial gradient centered in the widget
  QPointF center(width() * 0.5f, height() * 0.5f);
  double radius = qMax(width(), height()) * 0.25;
  QRadialGradient gradient(center, radius);
  gradient.setColorAt(0.0, m_innerBackgroundColor);
  gradient.setColorAt(1.0, m_outerBackgroundColor);

  // Paint the gradient onto the pixmap with antialiasing enabled
  QPainter painter(&img);
  painter.fillRect(boundingRect(), gradient);

  // Assign background pixmaps
  m_bgImg[0] = img;
  if (anaglyphEnabled())
    m_bgImg[1] = img;

  // Mark dirty flag as false to avoid needless rendering
  m_dirtyBackground = false;
}

/**
 * @brief Renders the 3D camera indicator.
 *
 * This function draws a small 3D axis indicator that shows the camera’s
 * orientation.
 *
 * Marks m_dirtyCameraIndicator as false to avoid unnecessary re-rendering.
 */
void Widgets::Plot3D::drawCameraIndicator()
{
  // Initialize camera matrix
  QMatrix4x4 matrix;

  // Render 3D pixmaps
  if (anaglyphEnabled())
  {
    // Slightly rotate & shift view for each eye
    auto eyes = eyeTransformations(matrix);

    // Apply rotation transformations for left eye
    eyes.first.rotate(m_cameraAngleX, 1, 0, 0);
    eyes.first.rotate(m_cameraAngleY, 0, 1, 0);
    eyes.first.rotate(m_cameraAngleZ, 0, 0, 1);

    // Apply rotation transformations for second eye
    eyes.second.rotate(m_cameraAngleX, 1, 0, 0);
    eyes.second.rotate(m_cameraAngleY, 0, 1, 0);
    eyes.second.rotate(m_cameraAngleZ, 0, 0, 1);

    // Render camera indicator
    m_cameraIndicatorImg[0] = renderCameraIndicator(eyes.first);
    m_cameraIndicatorImg[1] = renderCameraIndicator(eyes.second);
  }

  // Render 2D pixmap
  else
  {
    matrix.rotate(m_cameraAngleX, 1, 0, 0);
    matrix.rotate(m_cameraAngleY, 0, 1, 0);
    matrix.rotate(m_cameraAngleZ, 0, 0, 1);
    m_cameraIndicatorImg[0] = renderCameraIndicator(matrix);
  }

  // Mark dirty flag as false to avoid needless rendering
  m_dirtyCameraIndicator = false;
}

//------------------------------------------------------------------------------
// Low-level 3D->2D projection code
//------------------------------------------------------------------------------

/**
 * @brief Computes a clean grid step based on current world scale.
 *
 * This function returns a visually consistent grid step in world units by
 * snapping the scale-adjusted unit size to a clean, human-readable value—
 * specifically 1, 2, or 5 multiplied by a power of 10.
 *
 * The result is based on the inverse of the world scale (i.e., how many world
 * units are visible on screen), and is not tied to screen pixels. This ensures
 * that grid lines stay readable and logically spaced across all zoom levels,
 * without relying on fixed screen-space distances.
 *
 * @return Grid step size in world units (e.g., 1, 2, 5, 10, 20, 50, etc.).
 */
double Widgets::Plot3D::gridStep(const double scale) const
{
  auto s = scale;
  if (s == -1)
    s = m_worldScale;

  const float rawStep = 1.0f / s;
  const float exponent = std::floor(std::log10(rawStep));
  const float base = std::pow(10.0f, exponent);

  if (rawStep >= base * 5)
    return base * 5;
  else if (rawStep >= base * 2)
    return base * 2;
  else
    return base;
}

/**
 * @brief Projects 3D world-space points into 2D screen-space coordinates.
 *
 * Applies the given model-view-projection (MVP) matrix to each 3D point,
 * performs perspective divide, and maps normalized device coordinates (NDC)
 * from [-1, 1] range to actual screen pixels.
 *
 * This function assumes a standard right-handed coordinate system with
 * Y-up and a perspective or orthographic projection already applied.
 *
 * @param points List of 3D points in world space.
 * @param matrix The combined MVP matrix.
 * @return Vector of 2D QPointF in screen coordinates.
 */
std::vector<QPointF>
Widgets::Plot3D::screenProjection(const DSP::LineSeries3D &points,
                                  const QMatrix4x4 &matrix)
{
  std::vector<QPointF> projected;
  projected.reserve(points.size());

  const float halfW = width() * 0.5f;
  const float halfH = height() * 0.5f;

  for (const QVector3D &p : points)
  {
    // Project the point
    QVector4D v = matrix * QVector4D(p, 1.0f);

    // Avoid invalid perspective divide
    if (qFuzzyIsNull(v.w()))
      continue;

    // Normalized Device Coordinates [-1, 1]
    const float ndcX = v.x() / v.w();
    const float ndcY = v.y() / v.w();

    // Convert NDC to screen-space
    const float screenX = halfW + ndcX * halfW;
    const float screenY = halfH - ndcY * halfH;
    projected.push_back(QPointF(screenX, screenY));
  }

  return projected;
}

/**
 * @brief Projects and renders a 3D line as a faded 2D segment on screen.
 *
 * This function subdivides a 3D line into smaller segments, projects them
 * onto screen space using the provided view-projection matrix, and renders
 * them with distance-based fading. Segments that fall outside the central
 * view region or screen bounds are discarded to avoid clutter and improve
 * performance.
 *
 * @param painter The QPainter used to render the 2D line segments.
 * @param matrix The 4x4 view-projection matrix for 3D to 2D projection.
 * @param p1 The starting point of the 3D line.
 * @param p2 The ending point of the 3D line.
 * @param color The base color of the line before fading is applied.
 * @param lineWidth The width of the rendered line in pixels.
 * @param style The pen style used for the rendered segments.
 */
void Widgets::Plot3D::drawLine3D(QPainter &painter, const QMatrix4x4 &matrix,
                                 const QVector3D &p1, const QVector3D &p2,
                                 QColor color, float lineWidth,
                                 Qt::PenStyle style)
{
  // Define number of segments per line
  constexpr int segmentCount = 40;

  // Calculate drawable area and center
  const float w = width();
  const float h = height();
  const float halfW = w * 0.5f;
  const float halfH = h * 0.5f;
  const QPointF center(halfW, halfH);
  const float maxDist = 0.5f * std::hypot(w, h);

  // Define screen region beyond which we skip segments
  const float screenRatio = 0.4f;
  const float xLimit = w * screenRatio;
  const float yLimit = h * screenRatio;

  // Subdivide and render the line with fading
  for (int i = 0; i < segmentCount; ++i)
  {
    // Interpolate points along the 3D line
    float t1 = float(i) / segmentCount;
    float t2 = float(i + 1) / segmentCount;
    QVector3D a = (1.0f - t1) * p1 + t1 * p2;
    QVector3D b = (1.0f - t2) * p1 + t2 * p2;

    // Project to screen space
    const auto projected = screenProjection({a, b}, matrix);
    if (projected.size() != 2)
      continue;

    // Obtain start & end points
    const QPointF &pA = projected[0];
    const QPointF &pB = projected[1];

    // Discard segments far from center horizontally or vertically
    const bool exceedPAx = std::abs(pA.x() - halfW) > xLimit;
    const bool exceedPBx = std::abs(pB.x() - halfW) > xLimit;
    const bool exceedPAy = std::abs(pA.y() - halfH) > yLimit;
    const bool exceedPBy = std::abs(pB.y() - halfH) > yLimit;
    if (exceedPAx || exceedPBx || exceedPAy || exceedPBy)
      continue;

    // Compute fade factor based on distance from center
    QPointF mid = 0.5f * (pA + pB);
    float dist = QLineF(mid, center).length();
    float alpha = 1.0f - std::clamp(dist / maxDist, 0.0f, 1.0f);

    // Apply fade and draw the segment
    QColor faded = color;
    faded.setAlphaF(color.alphaF() * alpha);

    // Draw line segment
    painter.setPen(QPen(faded, lineWidth, style));
    painter.drawLine(pA, pB);
  }
}

//------------------------------------------------------------------------------
// Scene rendering code
//------------------------------------------------------------------------------

/**
 * @brief Renders the infinite grid overlay as a 2D pixmap.
 *
 * This method generates a QPixmap representing a perspective-projected
 * grid based on the current camera transformation. Grid lines and axes
 * are rendered with distance-based fading and clipping to preserve clarity
 * at varying zoom levels. A label indicating the current grid step is
 * drawn in the lower-left corner.
 *
 * @param matrix The camera view-projection matrix used for 3D to 2D projection.
 * @return A QPixmap containing the rendered grid overlay.
 */
QImage Widgets::Plot3D::renderGrid(const QMatrix4x4 &matrix)
{
  // Create the image and initialize it to the widget's size
  QImage img(widgetSize(), QImage::Format_ARGB32_Premultiplied);
  img.setDevicePixelRatio(qApp->devicePixelRatio());
  img.fill(Qt::transparent);

  // Initialize paint device
  QPainter painter(&img);
  painter.setRenderHint(QPainter::Antialiasing, true);

  // Obtain grid interval
  const double numSteps = 10;
  const double step = gridStep();
  const double l = numSteps * step;

  // Construct grid lines
  QVector<QPair<QVector3D, QVector3D>> gridLines;
  for (int i = -numSteps; i <= numSteps; ++i)
  {
    if (i == 0)
      continue;

    float y = i * step;
    float x = i * step;
    const auto x1 = QVector3D(x, l, 0);
    const auto x2 = QVector3D(x, -l, 0);
    const auto y1 = QVector3D(l, y, 0);
    const auto y2 = QVector3D(-l, y, 0);

    gridLines.append({x1, x2});
    gridLines.append({y1, y2});
  }

  // Construct axis lines
  QPair<QVector3D, QVector3D> xAxis = {QVector3D(-l, 0, 0), QVector3D(l, 0, 0)};
  QPair<QVector3D, QVector3D> yAxis = {QVector3D(0, -l, 0), QVector3D(0, l, 0)};

  // Render horizontal & vertical lines
  auto color = m_gridMinorColor;
  color.setAlpha(100);
  for (const auto &line : gridLines)
  {
    drawLine3D(painter, matrix, line.first, line.second, color, 1,
               Qt::DashLine);
  }

  // Render axis lines
  drawLine3D(painter, matrix, xAxis.first, xAxis.second, m_xAxisColor, 1.5,
             Qt::SolidLine);
  drawLine3D(painter, matrix, yAxis.first, yAxis.second, m_yAxisColor, 1.5,
             Qt::SolidLine);
  // drawLine3D(painter, matrix, zAxis.first, zAxis.second, m_zAxisColor, 1.5,
  //            Qt::SolidLine);

  // Render label with grid step
  const QString stepLabel = tr("Grid Interval: %1 unit(s)").arg(step);
  painter.setPen(m_textColor);
  painter.setFont(Misc::CommonFonts::instance().monoFont());
  painter.drawText(QPoint(8, height() - 8), stepLabel);

  // Return the result
  return img;
}

/**
 * @brief Renders the camera orientation indicator as a 2D pixmap.
 *
 * Draws X, Y, Z axes with labels, projecting them from 3D space
 * using the provided matrix. Axes are depth-sorted for proper overlap.
 *
 * @param matrix Transform matrix for projection.
 * @return Rendered camera indicator pixmap.
 */
QImage Widgets::Plot3D::renderCameraIndicator(const QMatrix4x4 &matrix)
{
  // Define a structure to hold axis data
  struct Axis
  {
    QVector3D dir;
    QColor color;
    QString label;
  };

  // Define a structure to hold transformed axis data
  struct TransformedAxis
  {
    Axis axis;
    QVector4D transformed;
  };

  // Create the image and initialize it to the widget's size
  QImage img(widgetSize(), QImage::Format_ARGB32_Premultiplied);
  img.setDevicePixelRatio(qApp->devicePixelRatio());
  img.fill(Qt::transparent);

  // Skip rendering if widget is too small
  if (width() < 240 || height() < 240)
    return img;

  // Initialize paint device
  QPainter painter(&img);
  painter.setRenderHint(QPainter::Antialiasing, true);

  // Define widget constants & set origin to bottom left
  const float lineScale = 18;
  const float axisLength = 2;
  const QPointF origin(50, 50);

  // Define axes names & colors
  QVector<Axis> axes = {{{1, 0, 0}, m_xAxisColor, QStringLiteral("X")},
                        {{0, 1, 0}, m_yAxisColor, QStringLiteral("Y")},
                        {{0, 0, 1}, m_zAxisColor, QStringLiteral("Z")}};

  // Apply projection transformations to the axes
  QVector<TransformedAxis> transformedAxes;
  for (const auto &ax : axes)
  {
    QVector4D t = matrix * QVector4D(ax.dir * axisLength, 1.0f);
    transformedAxes.append({ax, t});
  }

  // Sort axes so we avoid drawing items that should go back in the front
  std::sort(transformedAxes.begin(), transformedAxes.end(),
            [](const TransformedAxis &a, const TransformedAxis &b) {
              return a.transformed.z() < b.transformed.z();
            });

  // Set axis text font
  painter.setFont(Misc::CommonFonts::instance().customMonoFont(0.8));

  // Calculate size of circles
  QFontMetrics fm(painter.font());
  int textWidth = fm.horizontalAdvance("X");
  int textHeight = fm.height();
  float circleRadius = std::max(textWidth, textHeight) * 0.7f;

  // Draw widget
  for (const auto &ta : transformedAxes)
  {
    // Obtain en position of axis
    const QVector4D &t = ta.transformed;
    QPointF endpoint(origin.x() + (t.x() / t.w()) * lineScale,
                     origin.y() - (t.y() / t.w()) * lineScale);

    // Draw line
    painter.setPen(QPen(ta.axis.color, 3));
    painter.drawLine(origin, endpoint);

    // Draw filled circle
    painter.setBrush(ta.axis.color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(endpoint, circleRadius, circleRadius);

    // Find where to draw the text
    QRectF textRect(endpoint.x() - circleRadius, endpoint.y() - circleRadius,
                    circleRadius * 2, circleRadius * 2);

    // Draw axis label in circle
    painter.setPen(m_axisTextColor);
    painter.drawText(textRect, Qt::AlignCenter, ta.axis.label);
  }

  // Return obtained pixmap
  return img;
}

/**
 * @brief Renders the 3D plot foreground as a 2D pixmap.
 *
 * Projects 3D plot points to 2D using the given matrix and draws a
 * gradient line from head to tail.
 *
 * @param matrix Transform matrix for projection.
 * @param data 3D plot points.
 * @return Rendered foreground pixmap.
 */
QImage Widgets::Plot3D::renderData(const QMatrix4x4 &matrix,
                                   const DSP::LineSeries3D &data)
{
  // Create the pixmap and initialize it to the widget's size
  QImage img(widgetSize(), QImage::Format_ARGB32_Premultiplied);
  img.setDevicePixelRatio(qApp->devicePixelRatio());
  img.fill(Qt::transparent);

  // Initialize paint device
  QPainter painter(&img);
  painter.setRenderHint(QPainter::Antialiasing, true);

  // Project 3D points to 2D screen space
  auto points = screenProjection(data, matrix);

  // Interpolate points by generated a gradient line
  if (m_interpolate)
  {
    const auto &endColor = m_lineHeadColor;
    const auto &startColor = m_lineTailColor;
    const auto numPoints = static_cast<qsizetype>(points.size());
    for (qsizetype i = 1; i < numPoints; ++i)
    {
      QColor c;
      double t = double(i) / numPoints;
      c.setRedF(startColor.redF() * (1 - t) + endColor.redF() * t);
      c.setGreenF(startColor.greenF() * (1 - t) + endColor.greenF() * t);
      c.setBlueF(startColor.blueF() * (1 - t) + endColor.blueF() * t);

      painter.setPen(QPen(c, 2));
      painter.drawLine(points[i - 1], points[i]);
    }
  }

  // Draw individual points only
  else
  {
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_lineHeadColor);
    for (const QPointF &pt : std::as_const(points))
      painter.drawEllipse(pt, 1, 1);
  }

  // Return the rendered pixmap
  return img;
}

//------------------------------------------------------------------------------
// Stereoscopic rendering eye position calculations
//------------------------------------------------------------------------------

/**
 * @brief Calculates the left and right eye transformation matrices for 3D
 *        anaglyph rendering.
 *
 * Simulates realistic human stereoscopic vision by combining:
 * - Horizontal eye separation (scaled by zoom) to generate parallax.
 * - Z-axis rotation (toe-in) to simulate convergence on a focal point.
 * - Micro-jittered rotation axes to reflect natural human eye asymmetry.
 *
 * This improves stereo depth perception and avoids the unnatural feel of
 * mathematically perfect symmetry.
 *
 * @param matrix The original view matrix to transform for stereo rendering.
 * @param staticView If true, disables zoom scaling for eye separation.
 * @return A QPair with the left and right eye matrices.
 */
QPair<QMatrix4x4, QMatrix4x4>
Widgets::Plot3D::eyeTransformations(const QMatrix4x4 &matrix)
{
  // Calculate horizontal eye separation
  float shift = m_eyeSeparation / (2.0f) * (m_invertEyePositions ? -1 : 1);

  // Calculate convergence angle
  const float distance = 10.0f;
  const float angleRad = std::atan(shift / distance);
  const float angleDeg = angleRad * 180.0f / float(M_PI);

  // Generate left eye camera
  QMatrix4x4 lMatrix = matrix;
  lMatrix.translate(shift, 0.0f, 0.0f);
  lMatrix.rotate(angleDeg, 0, 0, 1);

  // Generate right eye camera
  QMatrix4x4 rMatrix = matrix;
  rMatrix.translate(-shift, 0.0f, 0.0f);
  rMatrix.rotate(-angleDeg, 0, 0, 1);

  // Return both cameras
  return qMakePair(lMatrix, rMatrix);
}

//------------------------------------------------------------------------------
// Event handling
//------------------------------------------------------------------------------

/**
 * @brief Handles mouse wheel events to worldScale in or out of the 3D plot.
 * @param event The wheel event containing scroll delta.
 */
void Widgets::Plot3D::wheelEvent(QWheelEvent *event)
{
  if (event->angleDelta().y() != 0)
  {
    event->accept();
    if (event->angleDelta().y() > 0)
      setWorldScale(worldScale() * 1.1);
    else
      setWorldScale(worldScale() / 1.1);
  }
}

/**
 * @brief Handles mouse movement events to rotate or pan the 3D camera.
 *
 * If orbit navigation is enabled, this rotates the camera around the plot.
 * If orbit navigation is disabled, this pans the camera view horizontally and
 * vertically.
 *
 * Emits cameraChanged and redraws after movement.
 *
 * @param event The mouse move event containing the current cursor position.
 */
void Widgets::Plot3D::mouseMoveEvent(QMouseEvent *event)
{
  // Calculate delta & update last postion
  QPointF delta = event->pos() - m_lastMousePos;
  m_lastMousePos = event->pos();

  // Orbit mode
  if (m_orbitNavigation)
  {
    m_cameraOffsetX = m_orbitOffsetX;
    m_cameraOffsetY = m_orbitOffsetY;
    m_cameraAngleZ += delta.x() * 0.5;
    m_cameraAngleX += delta.y() * 0.5;

    m_cameraAngleZ = fmod(m_cameraAngleZ, 360.0);
    if (m_cameraAngleZ < 0)
      m_cameraAngleZ += 360.0;

    m_cameraAngleX = fmod(m_cameraAngleX, 360.0);
    if (m_cameraAngleX < 0)
      m_cameraAngleX += 360.0;
  }

  // Pan mode
  else
  {
    m_cameraOffsetX += delta.x() * 0.01;
    m_cameraOffsetY -= delta.y() * 0.01;
  }

  // Accept event
  event->accept();

  // Re-render everything
  markDirty();
  Q_EMIT cameraChanged();
}

/**
 * @brief Handles mouse press events to start dragging and change cursor.
 * @param event The mouse press event containing the cursor position.
 */
void Widgets::Plot3D::mousePressEvent(QMouseEvent *event)
{
  m_lastMousePos = event->pos();

  if (m_orbitNavigation)
  {
    float offsetFactor = 0.25;
    float biasX = ((m_lastMousePos.x() / width()) - 0.5f) * 2.0f;
    float biasY = (0.5f - (m_lastMousePos.y() / height())) * 2.0f;

    m_orbitOffsetX = m_cameraOffsetX - biasX * offsetFactor;
    m_orbitOffsetY = m_cameraOffsetY - biasY * offsetFactor;
  }

  else
  {
    m_orbitOffsetX = m_cameraOffsetX;
    m_orbitOffsetY = m_cameraOffsetY;
  }

  grabMouse();
  setCursor(Qt::ClosedHandCursor);
  event->accept();
}

/**
 * @brief Handles mouse release events to stop dragging and reset cursor.
 * @param event The mouse release event.
 */
void Widgets::Plot3D::mouseReleaseEvent(QMouseEvent *event)
{
  unsetCursor();
  ungrabMouse();
  ungrabTouchPoints();
  event->accept();

  m_orbitOffsetX = m_cameraOffsetX;
  m_orbitOffsetY = m_cameraOffsetY;
}
