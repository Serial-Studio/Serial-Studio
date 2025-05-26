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
  , m_minX(INT_MAX)
  , m_maxX(INT_MIN)
  , m_minY(INT_MAX)
  , m_maxY(INT_MIN)
  , m_minZ(INT_MAX)
  , m_maxZ(INT_MIN)
  , m_index(index)
  , m_zoom(0.05)
  , m_cameraAngleX(300)
  , m_cameraAngleY(0)
  , m_cameraAngleZ(225)
  , m_cameraOffsetX(0)
  , m_cameraOffsetY(0)
  , m_cameraOffsetZ(-10)
  , m_eyeSeparation(0.069)
  , m_anaglyph(false)
  , m_interpolate(true)
  , m_orbitNavigation(true)
  , m_invertEyePositions(false)
  , m_dirtyData(true)
  , m_dirtyGrid(true)
  , m_dirtyCameraIndicator(true)
{
  // Configure QML item behavior
  setMipmap(true);
  setAntialiasing(true);
  setOpaquePainting(true);
  setAcceptHoverEvents(true);
  setFiltersChildMouseEvents(true);

  // Configure item flags
  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);

  // Set rendering hints
  setMipmap(false);
  setAntialiasing(true);

  // Update the plot data
  connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
          &Widgets::Plot3D::updateData);

  // Mark everything as dirty when widget size changes
  connect(this, &Widgets::Plot3D::widthChanged, this,
          &Widgets::Plot3D::markDirty);
  connect(this, &Widgets::Plot3D::heightChanged, this,
          &Widgets::Plot3D::markDirty);
  connect(this, &Widgets::Plot3D::scaleChanged, this,
          &Widgets::Plot3D::markDirty);

  // Obtain group information
  if (VALIDATE_WIDGET(SerialStudio::DashboardPlot3D, m_index))
  {
    connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout24Hz,
            this, [=] {
              if (isVisible() && dirty())
                update();
            });
  }

  // Connect to the theme manager to update the curve colors
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Widgets::Plot3D::onThemeChanged);
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
  painter->setRenderHint(QPainter::Antialiasing);
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

  // Anaglyph processing
  if (anaglyphEnabled())
  {
    // Initialize left eye pixmap
    QPixmap left(m_backgroundPixmap[0].size());
    left.setDevicePixelRatio(qApp->devicePixelRatio());
    left.fill(Qt::transparent);

    // Initialize right eye pixmap
    QPixmap right(m_backgroundPixmap[1].size());
    right.setDevicePixelRatio(qApp->devicePixelRatio());
    right.fill(Qt::transparent);

    // Calculate additional image shift factor
    const float manualShift = qMin(m_eyeSeparation / 0.1, 1.0) * 10;
    const float lShift = m_invertEyePositions ? manualShift : 0;
    const float rShift = m_invertEyePositions ? 0 : manualShift;

    // Compose the left eye scene
    QPainter leftScene(&left);
    leftScene.drawPixmap(0, 0, m_backgroundPixmap[0]);
    leftScene.drawPixmap(lShift, 0, m_gridPixmap[0]);
    leftScene.drawPixmap(lShift, 0, m_plotPixmap[0]);
    leftScene.drawPixmap(0, 0, m_cameraIndicatorPixmap[0]);
    leftScene.end();

    // Compose the right eye scene
    QPainter rightScene(&right);
    rightScene.drawPixmap(0, 0, m_backgroundPixmap[1]);
    rightScene.drawPixmap(rShift, 0, m_gridPixmap[1]);
    rightScene.drawPixmap(rShift, 0, m_plotPixmap[1]);
    rightScene.drawPixmap(0, 0, m_cameraIndicatorPixmap[1]);
    rightScene.end();

    // Obtain two images from the scene
    QImage leftImg = left.toImage();
    QImage rightImg = right.toImage();

    // Build the anaglyph manually
    QImage finalImage(leftImg.size(), QImage::Format_RGB32);
    finalImage.setDevicePixelRatio(qApp->devicePixelRatio());
    for (int y = 0; y < leftImg.height(); ++y)
    {
      for (int x = 0; x < leftImg.width(); ++x)
      {
        // Obtain pixels from both left and right image
        const auto lRgb = leftImg.pixel(x, y);
        const auto rRgb = rightImg.pixel(x, y);

        // Preserve red from left and cyan (green + blue) from right
        const auto outR = qRed(lRgb);
        const auto outG = qGreen(rRgb);
        const auto outB = qBlue(rRgb);
        finalImage.setPixel(x, y, qRgb(outR, outG, outB));
      }
    }

    // Draw the final image
    painter->drawPixmap(0, 0, QPixmap::fromImage(finalImage));
  }

  // Standard processing
  else
  {
    painter->drawPixmap(0, 0, m_backgroundPixmap[0]);
    painter->drawPixmap(0, 0, m_gridPixmap[0]);
    painter->drawPixmap(0, 0, m_plotPixmap[0]);
    painter->drawPixmap(0, 0, m_cameraIndicatorPixmap[0]);
  }
}

//------------------------------------------------------------------------------
// Member access functions
//------------------------------------------------------------------------------

/**
 * @brief Returns the current zoom level of the 3D plot.
 * @return The zoom factor, where 1.0 is the default scale.
 */
qreal Widgets::Plot3D::zoom() const
{
  return m_zoom;
}

/**
 * @brief Returns the X-axis rotation.
 * @return The X-axis rotation angle in degrees.
 */
qreal Widgets::Plot3D::cameraAngleX() const
{
  return m_cameraAngleX;
}

/**
 * @brief Returns the Y-axis rotation.
 * @return The Y-axis rotation angle in degrees.
 */
qreal Widgets::Plot3D::cameraAngleY() const
{
  return m_cameraAngleY;
}

/**
 * @brief Returns the Z-axis rotation.
 * @return The Z-axis rotation angle in degrees.
 */
qreal Widgets::Plot3D::cameraAngleZ() const
{
  return m_cameraAngleZ;
}

/**
 * @brief Returns the X-axis camera offset.
 * @return The X-axis offset value.
 */
qreal Widgets::Plot3D::cameraOffsetX() const
{
  return m_cameraOffsetX;
}

/**
 * @brief Returns the Y-axis camera offset.
 * @return The Y-axis offset value.
 */
qreal Widgets::Plot3D::cameraOffsetY() const
{
  return m_cameraOffsetY;
}

/**
 * @brief Returns the Z-axis camera offset.
 * @return The Z-axis offset value.
 */
qreal Widgets::Plot3D::cameraOffsetZ() const
{
  return m_cameraOffsetZ;
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
void Widgets::Plot3D::setZoom(const qreal z)
{
  if (m_zoom != z)
  {
    m_zoom = z;
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
void Widgets::Plot3D::setCameraAngleX(const qreal angle)
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
void Widgets::Plot3D::setCameraAngleY(const qreal angle)
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
void Widgets::Plot3D::setCameraAngleZ(const qreal angle)
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
void Widgets::Plot3D::setCameraOffsetX(const qreal offset)
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
void Widgets::Plot3D::setCameraOffsetY(const qreal offset)
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
void Widgets::Plot3D::setCameraOffsetZ(const qreal offset)
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
  auto c = Misc::ThemeManager::instance().colors()["widget_colors"].toArray();
  if (c.count() > m_index)
    m_lineHeadColor = c.at(m_index).toString();
  else
    m_lineHeadColor = c.at(m_index % c.count()).toString();

  // Create gradient based on widget index
  QColor midCurve(m_lineHeadColor);
  m_lineTailColor = midCurve.darker(130);
  m_lineHeadColor = midCurve.lighter(130);

  // Obtain colors for XY plane & axes
  // clang-format off
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
  if (data.isEmpty())
    return;

  // Initialize camera matrix
  QMatrix4x4 matrix;
  matrix.perspective(45, float(width()) / height(), 0.1, 100);
  matrix.translate(m_cameraOffsetX, m_cameraOffsetY, m_cameraOffsetZ);
  matrix.rotate(m_cameraAngleX, 1, 0, 0);
  matrix.rotate(m_cameraAngleY, 0, 1, 0);
  matrix.rotate(m_cameraAngleZ, 0, 0, 1);
  matrix.scale(m_zoom);

  // Render 3D pixmaps
  if (anaglyphEnabled())
  {
    // Tweak left eye camera
    auto lMatrix = matrix;
    lMatrix.translate(-eyeShift(), 0, 0);
    lMatrix.rotate(convergenceAngle(), 0, 1, 0);

    // Tweak right eye camera
    auto rMatrix = matrix;
    rMatrix.translate(eyeShift(), 0, 0);
    rMatrix.rotate(-convergenceAngle(), 0, 1, 0);

    // Render pixmaps
    m_plotPixmap[0] = renderData(lMatrix, data);
    m_plotPixmap[1] = renderData(rMatrix, data);
  }

  // Render single pixmap
  else
    m_plotPixmap[0] = renderData(matrix, data);

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
  matrix.perspective(45, float(width()) / height(), 0.1, 100);
  matrix.translate(m_cameraOffsetX, m_cameraOffsetY, m_cameraOffsetZ);
  matrix.rotate(m_cameraAngleX, 1, 0, 0);
  matrix.rotate(m_cameraAngleY, 0, 1, 0);
  matrix.rotate(m_cameraAngleZ, 0, 0, 1);
  matrix.scale(m_zoom);

  // Render 3D pixmaps
  if (anaglyphEnabled())
  {
    // Tweak left eye camera
    auto lMatrix = matrix;
    lMatrix.translate(-eyeShift(), 0, 0);
    lMatrix.rotate(convergenceAngle(), 0, 1, 0);

    // Tweak right eye camera
    auto rMatrix = matrix;
    rMatrix.translate(eyeShift(), 0, 0);
    rMatrix.rotate(-convergenceAngle(), 0, 1, 0);

    // Render pixmaps
    m_gridPixmap[0] = renderGrid(lMatrix);
    m_gridPixmap[1] = renderGrid(rMatrix);
  }

  // Render 2D pixmap
  else
    m_gridPixmap[0] = renderGrid(matrix);

  // Mark dirty flag as false to avoid needless rendering
  m_dirtyGrid = false;
}

/**
 * @brief Renders the 3D plot background with optional anaglyph effect.
 *
 * This function draws the radial gradient background behind the 3D plot.
 * If anaglyph mode is enabled, it renders two slightly shifted backgrounds
 * to match the stereo effect of the scene. The shift is intentionally small
 * to maintain a cohesive look while avoiding visible artifacts, even though
 * backgrounds are usually static in depth.
 *
 * The result is stored in m_backgroundPixmap[0] (left eye) and
 * m_backgroundPixmap[1] (right eye) for anaglyph mode, or only in
 * m_backgroundPixmap[0] for normal rendering.
 *
 * Marks m_dirtyBackground as false to prevent unnecessary re-rendering.
 */
void Widgets::Plot3D::drawBackground()
{
  // Prepare common pixmap size
  const int pixWidth = static_cast<int>(width() * qApp->devicePixelRatio());
  const int pixHeight = static_cast<int>(height() * qApp->devicePixelRatio());

  // Lambda to create and paint background with optional horizontal shift
  auto renderBackground = [&](QPixmap &pixmap, float shiftX) {
    pixmap = QPixmap(pixWidth, pixHeight);
    pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
    pixmap.fill(Qt::transparent);

    QPointF center((width() * 0.5f) + shiftX, height() * 0.5f);
    qreal radius = qMax(width(), height()) * 0.25;
    QRadialGradient gradient(center, radius);
    gradient.setColorAt(0.0, m_innerBackgroundColor);
    gradient.setColorAt(1.0, m_outerBackgroundColor);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(boundingRect(), gradient);
  };

  // Render 3D pixmaps
  if (anaglyphEnabled())
  {
    constexpr float shiftAmount = 2.0f;
    renderBackground(m_backgroundPixmap[0], -shiftAmount);
    renderBackground(m_backgroundPixmap[1], shiftAmount);
  }

  // Single background with no shift
  else
    renderBackground(m_backgroundPixmap[0], 0.0f);

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
  matrix.rotate(m_cameraAngleX, 1, 0, 0);
  matrix.rotate(m_cameraAngleY, 0, 1, 0);
  matrix.rotate(m_cameraAngleZ, 0, 0, 1);

  // Render 3D pixmaps
  if (anaglyphEnabled())
  {
    // Tweak left eye camera
    auto lMatrix = matrix;
    lMatrix.translate(-eyeShift() * m_zoom, 0, 0);
    lMatrix.rotate(-convergenceAngle(), 0, 1, 0);

    // Tweak right eye camera
    auto rMatrix = matrix;
    rMatrix.translate(eyeShift() * m_zoom, 0, 0);
    rMatrix.rotate(-convergenceAngle(), 0, 1, 0);

    // Render pixmaps
    m_cameraIndicatorPixmap[0] = renderCameraIndicator(lMatrix);
    m_cameraIndicatorPixmap[1] = renderCameraIndicator(rMatrix);
  }

  // Render 2D pixmap
  else
    m_cameraIndicatorPixmap[0] = renderCameraIndicator(matrix);

  // Mark dirty flag as false to avoid needless rendering
  m_dirtyCameraIndicator = false;
}

//------------------------------------------------------------------------------
// Stereoscopic rendering eye position calculations
//------------------------------------------------------------------------------

/**
 * @brief Calculates the horizontal eye shift (parallax offset)
 *        for stereoscopic rendering.
 *
 * This value determines how far the left and right camera views are shifted
 * horizontally to simulate eye separation in the 3D scene.
 *
 * The offset is inversely proportional to zoom: as you zoom in, the shift
 * reduces to maintain a realistic parallax effect without over-exaggeration.
 *
 * @return The eye shift in scene units.
 */
float Widgets::Plot3D::eyeShift() const
{
  const float z = qMax(0.01f, m_zoom);
  return m_eyeSeparation / (2.0f * z) * (m_invertEyePositions ? -1 : 1);
}

/**
 * @brief Calculates the convergence angle for stereoscopic rendering.
 *
 * This angle rotates the left and right camera views inward so their frustums
 * converge at the focal plane (typically around the world center).
 *
 * It helps maintain depth realism and avoid divergence when looking at
 * near/far objects.
 *
 * The calculation is based on eye separation and zoom: at lower zoom, the
 * angle is small;  at higher zoom, the angle increases to keep the stereo
 * effect balanced.
 *
 * @return The convergence angle in degrees.
 */
float Widgets::Plot3D::convergenceAngle() const
{
  constexpr float gridStep = 10.0f;
  const float focalDistance = gridStep / qMax(0.01f, m_zoom);
  float rads = std::atan(m_eyeSeparation * 0.5f / focalDistance);
  return rads * 180.0f / float(M_PI) * (m_invertEyePositions ? -1 : 1);
}

//------------------------------------------------------------------------------
// Low-level scene rendering code
//------------------------------------------------------------------------------

/**
 * @brief Renders the background grid and axes of the 3D plot.
 *
 * This function creates a QPixmap the size of the widget, draws a fading grid
 * and the X, Y, Z axes using the provided transformation matrix, and returns
 * the resulting pixmap. The grid is drawn with dashed lines, and the axes are
 * drawn with solid lines. The intensity of the lines fades toward the edges
 * to create depth perception. The function supports zoom scaling and aspect
 * ratio adjustments.
 *
 * @param matrix The transformation matrix used to project the 3D scene into 2D.
 * @return QPixmap containing the rendered background grid and axes.
 */
QPixmap Widgets::Plot3D::renderGrid(const QMatrix4x4 &matrix)
{
  // Create the pixmap and initialize it to the widget's size
  // clang-format off
  QPixmap pixmap = QPixmap(static_cast<int>(width() * qApp->devicePixelRatio()),
                           static_cast<int>(height() * qApp->devicePixelRatio()));
  pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
  pixmap.fill(Qt::transparent);
  // clang-format on

  // Initialize paint device
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);

  // Grid configuration
  const int segments = 20;
  const float gridStep = 10;
  const float aspect = width() / float(height());
  const float worldSize = gridStep / qMax(0.01, m_zoom);

  // Calculate grid size
  const float gridWidth = qMin(worldSize, worldSize * aspect);
  const float gridHeight = qMin(worldSize, worldSize * aspect);

  // Calculate minimum and maximum values for XY plane
  const float maxX = gridWidth;
  const float minX = -gridWidth;
  const float maxY = gridHeight;
  const float minY = -gridHeight;

  // Calculate XY plane start/end points
  const float endX = std::ceil(maxX / gridStep) * gridStep;
  const float endY = std::ceil(maxY / gridStep) * gridStep;
  const float startX = std::floor(minX / gridStep) * gridStep;
  const float startY = std::floor(minY / gridStep) * gridStep;

  // Lambda function to draw a faded line
  auto drawFadedLine = [&](QVector3D p1, QVector3D p2, QColor color,
                           float lineWidth, Qt::PenStyle style) {
    // Transform 3D points into 4D homogeneous coordinates using the matrix
    QVector4D tp1 = matrix * QVector4D(p1, 1.0f);
    QVector4D tp2 = matrix * QVector4D(p2, 1.0f);

    // Project transformed points to 2D screen space
    QPointF sp1(width() / 2 + tp1.x() / tp1.w() * width() / 2,
                height() / 2 - tp1.y() / tp1.w() * height() / 2);
    QPointF sp2(width() / 2 + tp2.x() / tp2.w() * width() / 2,
                height() / 2 - tp2.y() / tp2.w() * height() / 2);

    // Determine the world space distance scale and fade thresholds
    auto distance = qMin(gridWidth, gridHeight);
    float fadeStart = 0.25f * distance;
    float fadeEnd = 0.5f * distance;

    // Calculate distances of points from origin
    float dist1 = p1.length();
    float dist2 = p2.length();
    float maxDist = std::max(dist1, dist2);

    // Compute normalized fade factor between fadeStart and fadeEnd
    float fade = 0.0f;
    if (maxDist >= fadeStart)
    {
      fade = (maxDist - fadeStart) / (fadeEnd - fadeStart);
      fade = std::clamp(fade, 0.0f, 1.0f);
    }

    // Apply fade to color alpha
    color.setAlphaF(1.0f - fade);

    // Set pen with computed color and draw the line
    painter.setPen(QPen(color, lineWidth, style));
    painter.drawLine(sp1, sp2);
  };

  // Calculate how many steps we need
  const int numXSteps = static_cast<int>((endX - startX) / gridStep);
  const int numYSteps = static_cast<int>((endY - startY) / gridStep);

  // Draw horizontal grid lines (Y-direction)
  for (int i = 0; i <= numYSteps; ++i)
  {
    const float y = startY + i * gridStep;
    for (int seg = 0; seg < segments; ++seg)
    {
      const float s1 = -1.0f + 2.0f * seg / segments;
      const float s2 = -1.0f + 2.0f * (seg + 1) / segments;
      const float x1 = s1 * gridWidth;
      const float x2 = s2 * gridWidth;
      drawFadedLine(QVector3D(x1, y, 0), QVector3D(x2, y, 0), m_gridMinorColor,
                    0.5, Qt::DashLine);
    }
  }

  // Draw vertical grid lines (X-direction)
  for (int i = 0; i <= numXSteps; ++i)
  {
    const float x = startX + i * gridStep;
    for (int seg = 0; seg < segments; ++seg)
    {
      const float s1 = -1.0f + 2.0f * seg / segments;
      const float s2 = -1.0f + 2.0f * (seg + 1) / segments;
      const float y1 = s1 * gridHeight;
      const float y2 = s2 * gridHeight;
      drawFadedLine(QVector3D(x, y1, 0), QVector3D(x, y2, 0), m_gridMinorColor,
                    0.5, Qt::DashLine);
    }
  }

  // Draw the axes
  QVector<QColor> axisColors = {m_xAxisColor, m_yAxisColor};
  for (int i = 0; i < axisColors.count(); ++i)
  {
    for (int seg = 0; seg < segments; ++seg)
    {
      const float s1 = -1.0f + 2.0f * seg / segments;
      const float s2 = -1.0f + 2.0f * (seg + 1) / segments;
      QVector3D p1, p2;

      // Draw x axis segment
      if (i == 0)
      {
        p1 = QVector3D(s1 * gridWidth, 0, 0);
        p2 = QVector3D(s2 * gridWidth, 0, 0);
      }

      // Draw y axis segment
      if (i == 1)
      {
        p1 = QVector3D(0, s1 * gridHeight, 0);
        p2 = QVector3D(0, s2 * gridHeight, 0);
      }

      // Draw z axis segment
      if (i == 2)
      {
        p1 = QVector3D(0, 0, s1 * gridHeight);
        p2 = QVector3D(0, 0, s2 * gridHeight);
      }

      // Draw the axis segment line
      drawFadedLine(p1, p2, axisColors[i], 1.5, Qt::SolidLine);
    }
  }

  // Return the obtained pixmap
  return pixmap;
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
QPixmap Widgets::Plot3D::renderCameraIndicator(const QMatrix4x4 &matrix)
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

  // Create the pixmap and initialize it to the widget's size
  // clang-format off
  QPixmap pixmap = QPixmap(static_cast<int>(width() * qApp->devicePixelRatio()),
                           static_cast<int>(height() * qApp->devicePixelRatio()));
  pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
  pixmap.fill(Qt::transparent);
  // clang-format on

  // Skip rendering if widget is too small
  if (width() < 240 || height() < 240)
    return pixmap;

  // Initialize paint device
  QPainter painter(&pixmap);
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
  return pixmap;
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
QPixmap Widgets::Plot3D::renderData(const QMatrix4x4 &matrix,
                                    const PlotData3D &data)
{
  // Create the pixmap and initialize it to the widget's size
  // clang-format off
  QPixmap pixmap = QPixmap(static_cast<int>(width() * qApp->devicePixelRatio()),
                           static_cast<int>(height() * qApp->devicePixelRatio()));
  pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
  pixmap.fill(Qt::transparent);
  // clang-format on

  // Initialize paint device
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);

  // Project 3D points to 2D screen space
  QVector<QPointF> points;
  const float halfWidth = width() * 0.5;
  const float halfHeight = height() * 0.5;
  for (const QVector3D &p : data)
  {
    QVector3D rearranged(p.x(), p.y(), p.z());
    QVector4D transformed = matrix * QVector4D(rearranged, 1);

    float x = halfWidth + (transformed.x() / transformed.w()) * halfWidth;
    float y = halfHeight - (transformed.y() / transformed.w()) * halfHeight;

    points.append(QPointF(x, y));
  }

  // Interpolate points by generated a gradient line
  if (m_interpolate)
  {
    const auto &endColor = m_lineTailColor;
    const auto &startColor = m_lineHeadColor;
    for (int i = 1; i < points.size(); ++i)
    {
      QColor c;
      qreal tVal = qreal(i) / points.size();
      c.setRedF(startColor.redF() * (1 - tVal) + endColor.redF() * tVal);
      c.setGreenF(startColor.greenF() * (1 - tVal) + endColor.greenF() * tVal);
      c.setBlueF(startColor.blueF() * (1 - tVal) + endColor.blueF() * tVal);

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
  return pixmap;
}

//------------------------------------------------------------------------------
// Event handling
//------------------------------------------------------------------------------

/**
 * @brief Handles mouse wheel events to zoom in or out of the 3D plot.
 * @param event The wheel event containing scroll delta.
 */
void Widgets::Plot3D::wheelEvent(QWheelEvent *event)
{
  if (event->angleDelta().y() != 0)
  {
    event->accept();
    if (event->angleDelta().y() > 0)
      m_zoom *= 1.1;
    else
      m_zoom /= 1.1;

    m_zoom = qBound(0.01, m_zoom, 100.0);

    Q_EMIT cameraChanged();
    markDirty();
    update();
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
  Q_EMIT cameraChanged();
  markDirty();
  update();
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
