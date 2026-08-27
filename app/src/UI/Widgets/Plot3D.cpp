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

#include <algorithm>
#include <cmath>
#include <QCursor>
#include <QFontMetrics>
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QSGSimpleTextureNode>
#include <QtNumeric>

#include "DataModel/HotpathOptimization.h"
#include "DSP.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"
#include "UI/Widgets/GpuStroke.h"
#include "UI/Widgets/Plot3D/Plot3DOverlay.h"

static constexpr float kNearPlane = 0.1f;
static constexpr float kFarPlane  = 100.0f;

static constexpr int kScaleShrinkDelay    = 30;
static constexpr double kFitPadding       = 1.2;
static constexpr double kInvFitSteps      = 1.0 / 6.0;
static constexpr double kScaleShrinkRatio = 0.35;

static constexpr int kIndicatorTileSize = 100;
static constexpr int kIndicatorMinSize  = 240;
static constexpr int kGridSteps         = 10;
static constexpr int kLineSegments      = 40;
static constexpr float kInvLineSegments = 1.0f / kLineSegments;
static constexpr float kScreenRatio     = 0.4f;
static constexpr double kGridDashOn     = 4.0;
static constexpr double kGridDashOff    = 2.0;

/**
 * @brief Appends a polyline to an accumulator, separated by a non-finite point so the two
 *        stay independent runs.
 */
static void appendPolyline(std::vector<QPointF>& dstPx,
                           std::vector<QColor>& dstColors,
                           const std::vector<QPointF>& srcPx,
                           const std::vector<QColor>& srcColors)
{
  if (srcPx.empty())
    return;

  if (!dstPx.empty()) {
    dstPx.push_back(QPointF(qQNaN(), qQNaN()));
    dstColors.push_back(srcColors.front());
  }

  dstPx.insert(dstPx.end(), srcPx.begin(), srcPx.end());
  dstColors.insert(dstColors.end(), srcColors.begin(), srcColors.end());
}

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
  : QQuickItem(parent)
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
  , m_dirtyLabel(true)
  , m_bgUpload(true)
  , m_labelUpload(false)
  , m_indicatorUpload(false)
  , m_bgNode(nullptr)
  , m_gridNode(nullptr)
  , m_axisNode(nullptr)
  , m_traceNode(nullptr)
  , m_labelNode(nullptr)
  , m_indicatorNode(nullptr)
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
  setAcceptHoverEvents(true);
  setFiltersChildMouseEvents(true);

  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);

  setAntialiasing(false);

  connect(&m_dashboard, &UI::Dashboard::updated, this, &Widgets::Plot3D::updateData);

  connect(this, &Widgets::Plot3D::widthChanged, this, &Widgets::Plot3D::updateSize);
  connect(this, &Widgets::Plot3D::heightChanged, this, &Widgets::Plot3D::updateSize);
  connect(this, &Widgets::Plot3D::scaleChanged, this, &Widgets::Plot3D::updateSize);

  if (VALIDATE_WIDGET(SerialStudio::DashboardPlot3D, m_index)) {
    connect(&m_timerEvents, &Misc::TimerEvents::uiTimeout, this, [=, this] {
      if (isVisible() && dirty()) {
        polish();
        update();
      }
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
 * @brief Builds the plot's scene-graph node tree. Runs on the render thread with the GUI
 *        thread blocked in the synchronization phase, which is what makes reading item and
 *        dashboard state here safe; no other render-thread callback may read that state, and
 *        the cached child pointers are never dereferenced outside this call.
 */
QSGNode* Widgets::Plot3D::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data)
{
  Q_UNUSED(data)

  const QRectF rect = boundingRect();
  if (rect.isEmpty() || !window()) {
    delete oldNode;
    m_bgNode        = nullptr;
    m_gridNode      = nullptr;
    m_axisNode      = nullptr;
    m_traceNode     = nullptr;
    m_labelNode     = nullptr;
    m_indicatorNode = nullptr;
    return nullptr;
  }

  auto* root = oldNode;
  if (!root) {
    root              = new QSGNode;
    m_bgNode          = nullptr;
    m_gridNode        = nullptr;
    m_axisNode        = nullptr;
    m_traceNode       = nullptr;
    m_labelNode       = nullptr;
    m_indicatorNode   = nullptr;
    m_labelUpload     = true;
    m_indicatorUpload = true;
    m_bgUpload        = true;
  }

  SS_ASSERT(root != nullptr, return nullptr);

  root->removeAllChildNodes();
  syncBackgroundNode(root, rect);
  syncStrokeNode(m_gridNode, m_gridPx, m_gridColors, 0.5);
  syncStrokeNode(m_axisNode, m_axisPx, m_axisColors, 0.75);
  syncTraceNode();

  syncTileNode(m_labelNode, m_labelTile, m_labelPos, m_labelUpload);
  syncTileNode(m_indicatorNode,
               m_indicatorTile,
               QPointF(50.0 - kIndicatorTileSize * 0.5, 50.0 - kIndicatorTileSize * 0.5),
               m_indicatorUpload);

  appendSceneNodes(root);

  return root;
}

/**
 * @brief Rebuilds the trace node, stroking the gradient polyline or emitting one dot per
 *        sample when interpolation is off.
 */
void Widgets::Plot3D::syncTraceNode()
{
  SS_ASSERT(m_tracePx.size() == m_traceColors.size(), return);

  const auto count = static_cast<qsizetype>(m_tracePx.size());
  if (m_interpolate)
    m_traceNode =
      GpuStroke::buildStrokeNode(m_traceNode, m_tracePx.data(), m_traceColors.data(), count, 1.0);
  else
    m_traceNode =
      GpuStroke::buildPointNode(m_traceNode, m_tracePx.data(), m_traceColors.data(), count, 1.0);
}

/**
 * @brief Rebuilds one tile node in place, re-uploading its texture only when the tile was
 *        rasterized again. A null tile releases the node.
 */
void Widgets::Plot3D::syncTileNode(QSGSimpleTextureNode*& slot,
                                   const QImage& tile,
                                   const QPointF& topLeft,
                                   bool& needsUpload)
{
  SS_ASSERT(window() != nullptr, return);

  if (tile.isNull()) {
    delete slot;
    slot        = nullptr;
    needsUpload = false;
    return;
  }

  if (!slot) {
    // code-verify off
    // Scene-graph nodes default to QSGNode::OwnedByParent, so appending transfers ownership to
    // the root and the slot is re-nulled whenever updatePaintNode is handed a null oldNode.
    slot = new QSGSimpleTextureNode;
    // code-verify on
    slot->setOwnsTexture(true);
    slot->setFiltering(QSGTexture::Linear);
    needsUpload = true;
  }

  if (needsUpload) {
    slot->setTexture(window()->createTextureFromImage(tile));
    needsUpload = false;
  }

  const QSizeF size = tile.deviceIndependentSize();
  slot->setRect(QRectF(topLeft, size));
}

/**
 * @brief Appends the scene nodes in draw order, flipping the trace and grid around the same
 *        camera-angle threshold the layered composite used.
 */
void Widgets::Plot3D::appendSceneNodes(QSGNode* root)
{
  SS_ASSERT(root != nullptr, return);

  const bool gridOnTop = m_cameraAngleX <= 270.0 && m_cameraAngleX > 90.0;
  if (gridOnTop && m_traceNode)
    root->appendChildNode(m_traceNode);

  if (m_gridNode)
    root->appendChildNode(m_gridNode);

  if (m_axisNode)
    root->appendChildNode(m_axisNode);

  if (!gridOnTop && m_traceNode)
    root->appendChildNode(m_traceNode);

  if (m_labelNode)
    root->appendChildNode(m_labelNode);

  if (m_indicatorNode)
    root->appendChildNode(m_indicatorNode);
}

/**
 * @brief Refreshes the background tile node, rebuilding the tile only when the theme or the
 *        item size marked it dirty.
 */
void Widgets::Plot3D::syncBackgroundNode(QSGNode* root, const QRectF& rect)
{
  SS_ASSERT(root != nullptr, return);
  SS_ASSERT(window() != nullptr, return);

  if (!m_bgNode) {
    // code-verify off
    // Scene-graph nodes default to QSGNode::OwnedByParent, so appending below transfers
    // ownership to the root and the slot is re-nulled whenever updatePaintNode is handed a
    // null oldNode. There is no destructor release path to add.
    m_bgNode = new QSGSimpleTextureNode;
    // code-verify on
    m_bgNode->setOwnsTexture(true);
    m_bgNode->setFiltering(QSGTexture::Linear);
  }

  if (m_bgUpload && !m_bgTile.isNull()) {
    m_bgNode->setTexture(window()->createTextureFromImage(m_bgTile));
    m_bgUpload = false;
  }

  m_bgNode->setRect(rect);
  root->appendChildNode(m_bgNode);
}

/**
 * @brief Rebuilds one stroke node in place from an accumulated polyline; the builder frees and
 *        nulls the slot when there is nothing left to draw.
 */
void Widgets::Plot3D::syncStrokeNode(QSGGeometryNode*& slot,
                                     const std::vector<QPointF>& px,
                                     const std::vector<QColor>& colors,
                                     const double halfWidth)
{
  SS_ASSERT(px.size() == colors.size(), return);
  SS_ASSERT(halfWidth > 0.0, return);

  slot = GpuStroke::buildStrokeNode(
    slot, px.data(), colors.data(), static_cast<qsizetype>(px.size()), halfWidth);
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
  return m_dirtyGrid || m_dirtyData || m_dirtyBackground || m_dirtyCameraIndicator || m_dirtyLabel;
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

  markDirty();
}

//--------------------------------------------------------------------------------------------------
// State management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks all plot layers as dirty and requests a repaint.
 */
void Widgets::Plot3D::updatePolish()
{
  if (m_dirtyBackground) {
    m_bgTile = UI::Widgets::Plot3DDetail::buildBackgroundTile(
      boundingRect().size().toSize(), m_innerBackgroundColor, m_outerBackgroundColor);
    m_dirtyBackground = false;
    m_bgUpload        = true;
  }

  if (m_dirtyGrid)
    drawGrid();

  if (m_dirtyData)
    drawData();

  if (m_dirtyCameraIndicator)
    drawCameraIndicator();

  if (m_dirtyLabel)
    drawGridLabel();
}

/**
 * @brief Marks every layer dirty and schedules a rebuild.
 */
void Widgets::Plot3D::markDirty()
{
  m_dirtyGrid            = true;
  m_dirtyData            = true;
  m_dirtyBackground      = true;
  m_dirtyCameraIndicator = true;
  m_dirtyLabel           = true;
  polish();
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
  polish();
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

  QQuickItem::itemChange(change, value);
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
 * @brief Rebuilds the trace polyline for the current camera transform. Stereo eyes are handled
 *        by the anaglyph pass, so this builds the mono transform only.
 */
void Widgets::Plot3D::drawData()
{
  const auto& data = m_dashboard.plotData3D(m_index);
  if (data.empty()) {
    m_tracePx.clear();
    m_traceColors.clear();
    m_dirtyData = false;
    return;
  }

  if (m_dataUpdated) {
    updateCamera(data);
    m_dataUpdated = false;
  }

  m_tracePx.clear();
  m_traceColors.clear();

  QMatrix4x4 matrix;
  matrix.perspective(45.0f, float(width()) / height(), kNearPlane, kFarPlane);
  matrix.translate(m_cameraOffsetX, m_cameraOffsetY, m_cameraOffsetZ);

  if (anaglyphEnabled()) {
    auto eyes = eyeTransformations(matrix);
    applyCameraTransform(eyes.first);
    applyCameraTransform(eyes.second);
    buildTracePolyline(eyes.first, data, EyeMask::Left);
    buildTracePolyline(eyes.second, data, EyeMask::Right);
  }

  else {
    applyCameraTransform(matrix);
    buildTracePolyline(matrix, data, EyeMask::None);
  }

  m_dirtyData = false;
}

/**
 * @brief Projects the series and shades it tail-to-head as per-vertex color, replacing the
 *        pen-per-segment gradient. With interpolation off every sample carries the head color
 *        and is drawn as a dot instead of a stroke.
 */
void Widgets::Plot3D::buildTracePolyline(const QMatrix4x4& matrix,
                                         const DSP::LineSeries3D& data,
                                         const EyeMask mask)
{
  const auto& points = screenProjection(data, matrix);
  const qsizetype n  = static_cast<qsizetype>(points.size());
  if (n < 1)
    return;

  const QColor head = maskEyeColor(m_lineHeadColor, mask);
  const QColor tail = maskEyeColor(m_lineTailColor, mask);

  if (!m_tracePx.empty()) {
    m_tracePx.push_back(QPointF(qQNaN(), qQNaN()));
    m_traceColors.push_back(head);
  }

  m_tracePx.insert(m_tracePx.end(), points.begin(), points.end());
  m_traceColors.reserve(m_traceColors.size() + static_cast<std::size_t>(n));

  if (!m_interpolate) {
    m_traceColors.insert(m_traceColors.end(), static_cast<std::size_t>(n), head);
    return;
  }

  const double inv = n > 1 ? 1.0 / static_cast<double>(n - 1) : 0.0;
  for (qsizetype i = 0; i < n; ++i) {
    const double t = static_cast<double>(i) * inv;
    QColor c;
    c.setRedF(tail.redF() * (1 - t) + head.redF() * t);
    c.setGreenF(tail.greenF() * (1 - t) + head.greenF() * t);
    c.setBlueF(tail.blueF() * (1 - t) + head.blueF() * t);
    m_traceColors.push_back(c);
  }
}

/**
 * @brief Applies a stereo eye's channel mask to a color, preserving alpha. Masking is
 *        per-channel linear, so a masked gradient endpoint interpolates exactly as the merged
 *        image did.
 */
QColor Widgets::Plot3D::maskEyeColor(const QColor& color, const EyeMask mask)
{
  if (mask == EyeMask::None)
    return color;

  QColor out = color;
  if (mask == EyeMask::Left) {
    out.setGreenF(0.0);
    out.setBlueF(0.0);
    return out;
  }

  out.setRedF(0.0);
  return out;
}

/**
 * @brief Applies the camera rotation, scale and centering to a projection matrix.
 */
void Widgets::Plot3D::applyCameraTransform(QMatrix4x4& matrix) const
{
  matrix.rotate(m_cameraAngleX, 1, 0, 0);
  matrix.rotate(m_cameraAngleY, 0, 1, 0);
  matrix.rotate(m_cameraAngleZ, 0, 0, 1);
  matrix.scale(m_worldScale);
  matrix.translate(-m_centerPoint);
}

/**
 * @brief Rebuilds the grid and axis polylines. With stereo on, both eyes accumulate into the
 *        same buffers under complementary channel masks, replacing the two extra full-screen
 *        layers and the per-pixel merge.
 */
void Widgets::Plot3D::drawGrid()
{
  m_gridPx.clear();
  m_gridColors.clear();
  m_axisPx.clear();
  m_axisColors.clear();

  QMatrix4x4 matrix;
  matrix.perspective(45.0f, float(width()) / height(), kNearPlane, kFarPlane);
  matrix.translate(m_cameraOffsetX, m_cameraOffsetY, m_cameraOffsetZ);

  if (anaglyphEnabled()) {
    auto eyes = eyeTransformations(matrix);
    applyCameraTransform(eyes.first);
    applyCameraTransform(eyes.second);
    buildGridPolylines(eyes.first, EyeMask::Left);
    buildGridPolylines(eyes.second, EyeMask::Right);
  }

  else {
    applyCameraTransform(matrix);
    buildGridPolylines(matrix, EyeMask::None);
  }

  m_dirtyGrid = false;
}

/**
 * @brief Rebuilds the camera orientation indicator tile. The indicator is hidden on small
 *        widgets, exactly as the layered version was.
 */
void Widgets::Plot3D::drawCameraIndicator()
{
  m_dirtyCameraIndicator = false;

  if (width() < kIndicatorMinSize || height() < kIndicatorMinSize) {
    m_indicatorTile = QImage();
    return;
  }

  QMatrix4x4 matrix;
  matrix.rotate(m_cameraAngleX, 1, 0, 0);
  matrix.rotate(m_cameraAngleY, 0, 1, 0);
  matrix.rotate(m_cameraAngleZ, 0, 0, 1);

  const UI::Widgets::Plot3DDetail::IndicatorColors colors{
    m_xAxisColor, m_yAxisColor, m_zAxisColor, m_axisTextColor};

  m_indicatorTile = UI::Widgets::Plot3DDetail::buildCameraIndicatorTile(
    kIndicatorTileSize, displayPixelRatio(), matrix, m_commonFonts.customMonoFont(0.8), colors);
  m_indicatorUpload = true;
}

/**
 * @brief Rebuilds the grid-interval label tile, which changes only when the grid step or the
 *        theme does.
 */
void Widgets::Plot3D::drawGridLabel()
{
  m_dirtyLabel     = false;
  const QFont font = m_commonFonts.monoFont();
  m_labelTile      = UI::Widgets::Plot3DDetail::buildTextTile(
    m_gridStepLabel, font, m_textColor, displayPixelRatio());
  m_labelPos    = QPointF(7.0, height() - 8.0 - QFontMetrics(font).ascent() - 1.0);
  m_labelUpload = true;
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

//--------------------------------------------------------------------------------------------------
// Grid computation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Projects a near-plane-clipped 3D line into a pixel-space polyline, fading each point
 *        toward the item edge and marking points beyond the visible ratio non-finite so the
 *        stroke breaks there, exactly where the old per-segment cull dropped geometry.
 */
void Widgets::Plot3D::projectLine3D(const QMatrix4x4& matrix,
                                    const QVector3D& p1,
                                    const QVector3D& p2,
                                    const QColor& color,
                                    std::vector<QPointF>& px,
                                    std::vector<QColor>& colors) const
{
  px.clear();
  colors.clear();

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
  const float xLimit     = w * kScreenRatio;
  const float yLimit     = h * kScreenRatio;

  const QPointF pStart(halfW + (a.x() / a.w()) * halfW, halfH - (a.y() / a.w()) * halfH);
  const QPointF pEnd(halfW + (b.x() / b.w()) * halfW, halfH - (b.y() / b.w()) * halfH);
  const QPointF span = pEnd - pStart;

  px.reserve(kLineSegments + 1);
  colors.reserve(kLineSegments + 1);
  for (int i = 0; i <= kLineSegments; ++i) {
    const QPointF p    = pStart + span * (float(i) * kInvLineSegments);
    const bool outside = std::abs(p.x() - halfW) > xLimit || std::abs(p.y() - halfH) > yLimit;
    if (outside) {
      px.push_back(QPointF(qQNaN(), qQNaN()));
      colors.push_back(color);
      continue;
    }

    const float dist  = QLineF(p, center).length();
    const float alpha = 1.0f - std::clamp(dist * invMaxDist, 0.0f, 1.0f);
    QColor faded      = color;
    faded.setAlphaF(color.alphaF() * alpha);

    px.push_back(p);
    colors.push_back(faded);
  }
}

/**
 * @brief Projects one grid line, dashes it, and appends it to the grid accumulator.
 */
void Widgets::Plot3D::appendGridLine(const QMatrix4x4& matrix,
                                     const QVector3D& p1,
                                     const QVector3D& p2,
                                     const QColor& color)
{
  projectLine3D(matrix, p1, p2, color, m_linePx, m_lineColors);
  if (m_linePx.empty())
    return;

  GpuStroke::dashPolyline(m_linePx.data(),
                          m_lineColors.data(),
                          static_cast<qsizetype>(m_linePx.size()),
                          kGridDashOn,
                          kGridDashOff,
                          m_dashPx,
                          m_dashColors);

  appendPolyline(m_gridPx, m_gridColors, m_dashPx, m_dashColors);
}

/**
 * @brief Projects one axis line and appends it, undashed, to the axis accumulator.
 */
void Widgets::Plot3D::appendAxisLine(const QMatrix4x4& matrix,
                                     const QVector3D& p1,
                                     const QVector3D& p2,
                                     const QColor& color)
{
  projectLine3D(matrix, p1, p2, color, m_linePx, m_lineColors);
  appendPolyline(m_axisPx, m_axisColors, m_linePx, m_lineColors);
}

/**
 * @brief Appends one eye's grid and axis polylines to the accumulators.
 */
void Widgets::Plot3D::buildGridPolylines(const QMatrix4x4& matrix, const EyeMask mask)
{
  const double step = gridStep();
  const double l    = kGridSteps * step;
  const float cx    = std::round(m_centerPoint.x() / step) * step;
  const float cy    = std::round(m_centerPoint.y() / step) * step;

  auto minor = m_gridMinorColor;
  minor.setAlpha(100);
  minor = maskEyeColor(minor, mask);

  for (int i = -kGridSteps; i <= kGridSteps; ++i) {
    if (i == 0)
      continue;

    const float x = cx + i * step;
    const float y = cy + i * step;
    appendGridLine(matrix, QVector3D(x, cy + l, 0), QVector3D(x, cy - l, 0), minor);
    appendGridLine(matrix, QVector3D(cx + l, y, 0), QVector3D(cx - l, y, 0), minor);
  }

  const float ax = m_centerPoint.x();
  const float ay = m_centerPoint.y();
  appendAxisLine(
    matrix, QVector3D(ax - l, ay, 0), QVector3D(ax + l, ay, 0), maskEyeColor(m_xAxisColor, mask));
  appendAxisLine(
    matrix, QVector3D(ax, ay - l, 0), QVector3D(ax, ay + l, 0), maskEyeColor(m_yAxisColor, mask));

  const QString label = tr("Grid Interval: %1 unit(s)").arg(step);
  if (label != m_gridStepLabel) {
    m_gridStepLabel = label;
    m_dirtyLabel    = true;
  }
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
