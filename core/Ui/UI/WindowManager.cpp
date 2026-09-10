/*
 * Serial Studio
 * https://serial-studio.com/
 *

 * Copyright (C) 2020-2025 Alex Spataru
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

#include "WindowManager.h"

#include <algorithm>
#include <QApplication>
#include <QFileDialog>
#include <QSet>
#include <QStandardPaths>
#include <QUrl>

#include "Core/LayoutPatterns.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "UI/Dashboard.h"
#include "UI/SnapGuides.h"
#include "UI/Taskbar.h"
#include "UI/UISessionRegistry.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Manual-mode window size floor; mirrors the fallback WindowLayoutStore applies.
constexpr int kManualMinSize = 48;

// Auto-layout size floor; mirrors the auto-mode floor constrainWindows() applies.
constexpr int kAutoLayoutMinWidth  = 100;
constexpr int kAutoLayoutMinHeight = 80;

// Cascade-mode size floor, below which a tile stops reading as a window.
constexpr int kCascadeMinWidth  = 200;
constexpr int kCascadeMinHeight = 150;

// Drag distance a press has to travel before it counts as a move gesture.
constexpr int kDragThreshold = 20;

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the WindowManager and re-publishes the overlay's change signals under the
 *        property names QML binds to.
 */
UI::WindowManager::WindowManager(QQuickItem* parent)
  : QQuickItem(parent)
  , m_dashboard(UI::Dashboard::instance())
  , m_sessionRegistry(UISessionRegistry::instance())
  , m_snapOverlay()
  , m_layoutStore(m_dashboard)
  , m_zCounter(1)
  , m_layoutRestored(false)
  , m_autoLayoutEnabled(true)
  , m_frozen(false)
  , m_userReordered(false)
  , m_suppressGeometrySignal(false)
  , m_lastCanvasWidth(0)
  , m_lastCanvasHeight(0)
  , m_resizeEdge(ResizeEdge::None)
  , m_snapIndicatorVisible(false)
  , m_gridEnabled(false)
  , m_gridSize(16)
  , m_layoutRatio(Layouts::kDefaultRatio)
  , m_taskbar(nullptr)
  , m_dragWindow(nullptr)
  , m_targetWindow(nullptr)
  , m_resizeWindow(nullptr)
  , m_focusedWindow(nullptr)
{
  setEnabled(true);
  setAcceptHoverEvents(true);
  setFlag(ItemHasContents, false);
  setFiltersChildMouseEvents(true);
  setAcceptedMouseButtons(Qt::AllButtons);

  m_backgroundImage = m_settings.value("WindowManager_Wallpaper").toString();
  m_gridEnabled     = m_settings.value("WindowManager_GridEnabled", false).toBool();
  m_gridSize        = qBound(2, m_settings.value("WindowManager_GridSize", 16).toInt(), 256);

  connect(&m_snapOverlay,
          &UI::SnapOverlay::sizeMatchRectChanged,
          this,
          &UI::WindowManager::sizeMatchRectChanged);
  connect(&m_snapOverlay,
          &UI::SnapOverlay::manualGestureChanged,
          this,
          &UI::WindowManager::manualGestureChanged);
  connect(&m_snapOverlay,
          &UI::SnapOverlay::fractionPreviewChanged,
          this,
          &UI::WindowManager::fractionPreviewChanged);
  connect(&m_snapOverlay,
          &UI::SnapOverlay::alignmentGuidesChanged,
          this,
          &UI::WindowManager::alignmentGuidesChanged);
  connect(&m_snapOverlay,
          &UI::SnapOverlay::spacingIndicatorsChanged,
          this,
          &UI::WindowManager::spacingIndicatorsChanged);

  connect(this, &UI::WindowManager::widthChanged, this, &UI::WindowManager::triggerLayoutUpdate);
  connect(this, &UI::WindowManager::heightChanged, this, &UI::WindowManager::triggerLayoutUpdate);

  connect(&m_dashboard, &UI::Dashboard::layoutSpacingChanged, this, [this] {
    if (!m_autoLayoutEnabled)
      m_layoutStore.applyManualLayout(m_windows,
                                      canvasSize(),
                                      QSize(m_lastCanvasWidth, m_lastCanvasHeight),
                                      m_dashboard.layoutSpacing());
  });

  m_sessionRegistry.registerWindowManager(this);
}

/**
 * @brief Destroys the WindowManager and unregisters it from the UI session registry.
 */
UI::WindowManager::~WindowManager()
{
  m_sessionRegistry.unregisterWindowManager(this);
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Gets the current z-order counter.
 */
int UI::WindowManager::zCounter() const
{
  return m_zCounter;
}

/**
 * @brief Returns whether the dashboard is frozen (all window manipulation disabled).
 */
bool UI::WindowManager::frozen() const
{
  return m_frozen;
}

/**
 * @brief Returns whether automatic layout is enabled.
 */
bool UI::WindowManager::autoLayoutEnabled() const
{
  return m_autoLayoutEnabled;
}

/**
 * @brief Returns the currently set background image path.
 */
const QString& UI::WindowManager::backgroundImage() const
{
  return m_backgroundImage;
}

/**
 * @brief Indicates whether the snap indicator is currently visible.
 */
bool UI::WindowManager::snapIndicatorVisible() const
{
  return m_snapIndicatorVisible;
}

/**
 * @brief Returns the geometry of the current snap indicator.
 */
const QRect& UI::WindowManager::snapIndicator() const
{
  return m_snapIndicator;
}

/**
 * @brief Returns the grid cell size in pixels for manual-mode grid snapping.
 */
int UI::WindowManager::gridSize() const
{
  return m_gridSize;
}

/**
 * @brief Returns whether the manual-mode layout grid (render + snap) is enabled.
 */
bool UI::WindowManager::gridEnabled() const
{
  return m_gridEnabled;
}

/**
 * @brief Returns whether a size-matched sibling should be highlighted.
 */
bool UI::WindowManager::sizeMatchVisible() const
{
  return m_snapOverlay.sizeMatchVisible();
}

/**
 * @brief Returns whether a manual move/resize gesture is currently in progress.
 */
bool UI::WindowManager::manualGestureActive() const
{
  return m_snapOverlay.manualGestureActive();
}

/**
 * @brief Returns the geometry of the sibling whose size the resize gesture matched.
 */
const QRect& UI::WindowManager::sizeMatchRect() const
{
  return m_snapOverlay.sizeMatchRect();
}

/**
 * @brief Returns the footprint of the wrench-fraction resize preview; invalid when hidden.
 */
const QRect& UI::WindowManager::fractionPreviewRect() const
{
  return m_snapOverlay.fractionPreviewRect();
}

/**
 * @brief Returns the fraction preview's label ("1/2 x 1/4"); empty when hidden.
 */
const QString& UI::WindowManager::fractionPreviewLabel() const
{
  return m_snapOverlay.fractionPreviewLabel();
}

/**
 * @brief Returns the live geometry of the window being moved or resized.
 */
const QRect& UI::WindowManager::manualGestureGeometry() const
{
  return m_snapOverlay.manualGestureGeometry();
}

/**
 * @brief Returns the alignment guide lines for the active gesture.
 */
const QVariantList& UI::WindowManager::alignmentGuides() const
{
  return m_snapOverlay.alignmentGuides();
}

/**
 * @brief Returns the equal-spacing indicators for the active gesture.
 */
const QVariantList& UI::WindowManager::spacingIndicators() const
{
  return m_snapOverlay.spacingIndicators();
}

/**
 * @brief Split ratio of the layout pattern in force, in sixteenths.
 */
int UI::WindowManager::layoutRatio() const
{
  return m_layoutRatio;
}

/**
 * @brief Id of the layout pattern in force; empty is Grid.
 */
const QString& UI::WindowManager::layoutPattern() const
{
  return m_layoutPattern;
}

/**
 * @brief Per-window bitmask of the edges that coincide with a sibling's, keyed by window id.
 */
const QVariantMap& UI::WindowManager::mergedEdges() const
{
  return m_mergedEdges;
}

/**
 * @brief Retrieves the z-order for a given window item.
 */
int UI::WindowManager::zOrder(QQuickItem* item) const
{
  if (m_windowZ.contains(item))
    return m_windowZ.value(item);

  return -1;
}

/**
 * @brief Returns the windowId of the visually-first tile, or -1 if none.
 */
int UI::WindowManager::firstTileWindowId() const
{
  if (m_windowOrder.isEmpty())
    return -1;

  return m_windowOrder.first();
}

/**
 * @brief Returns the current visual window order (front-to-back tile sequence).
 */
const QVector<int>& UI::WindowManager::windowOrder() const
{
  return m_windowOrder;
}

/**
 * @brief Returns the canvas extents every layout and snap resolution is measured against.
 */
QSize UI::WindowManager::canvasSize() const
{
  return QSize(static_cast<int>(width()), static_cast<int>(height()));
}

//--------------------------------------------------------------------------------------------------
// Layout persistence
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes the current window layout to a JSON object.
 */
QJsonObject UI::WindowManager::serializeLayout() const
{
  return m_layoutStore.serialize(
    m_windowOrder, m_windows, canvasSize(), m_autoLayoutEnabled, m_userReordered);
}

/**
 * @brief Restores a previously serialized window layout.
 */
bool UI::WindowManager::restoreLayout(const QJsonObject& layout)
{
  if (layout.isEmpty())
    return false;

  bool autoLayout        = layout["autoLayout"].toBool(true);
  const int savedCanvasW = layout["canvasWidth"].toInt(0);
  const int savedCanvasH = layout["canvasHeight"].toInt(0);
  m_userReordered        = layout["userReordered"].toBool(false);

  if (layout.contains("windowOrderTyped") || layout.contains("windowOrder"))
    m_windowOrder = m_layoutStore.resolveSavedOrder(layout, m_windows, m_windowOrder);

  m_layoutStore.clearGeometries();
  if (!autoLayout && layout.contains("geometries")) {
    const QSize canvas = canvasSize();
    const QSize savedCanvas(savedCanvasW > 0 ? savedCanvasW : canvas.width(),
                            savedCanvasH > 0 ? savedCanvasH : canvas.height());

    m_layoutStore.applySavedGeometries(
      layout, m_windows, savedCanvas, canvas, m_dashboard.layoutSpacing());
    constrainWindows();
    Q_EMIT geometryChanged(nullptr);
  }

  if (m_autoLayoutEnabled != autoLayout) {
    m_autoLayoutEnabled = autoLayout;
    Q_EMIT autoLayoutEnabledChanged();
  }

  if (autoLayout)
    loadLayout();
  else
    m_layoutRestored = true;

  return true;
}

/**
 * @brief Reads the per-window chrome state ("normal"/"minimized"/"closed") stored with the
 *        layout, resolved to this session's window ids. The window manager only tiles what is
 *        normal, so the taskbar owns applying these back onto its model.
 */
QMap<int, QString> UI::WindowManager::savedWindowStates(const QJsonObject& layout) const
{
  return m_layoutStore.savedWindowStates(layout, m_windows);
}

/**
 * @brief Pre-stashes saved geometries so registerWindow can apply them per-window before first
 *        paint, avoiding the minimumSize flash.
 */
void UI::WindowManager::preloadPendingGeometries(const QJsonObject& layout)
{
  m_layoutStore.preload(layout, m_windows, canvasSize(), m_dashboard.layoutSpacing());
}

/**
 * @brief Reconciles m_windowOrder against the authoritative taskbar order.
 */
void UI::WindowManager::reconcileWindowOrder(const QVector<int>& taskbarOrder)
{
  QSet<int> taskbarSet;
  taskbarSet.reserve(taskbarOrder.size());
  for (int id : taskbarOrder)
    taskbarSet.insert(id);

  QVector<int> reconciled;
  reconciled.reserve(taskbarOrder.size());
  QSet<int> seen;

  if (m_userReordered) {
    for (int id : std::as_const(m_windowOrder)) {
      if (taskbarSet.contains(id) && !seen.contains(id)) {
        reconciled.append(id);
        seen.insert(id);
      }
    }
  }

  for (int id : taskbarOrder) {
    if (!seen.contains(id)) {
      reconciled.append(id);
      seen.insert(id);
    }
  }

  if (reconciled == m_windowOrder)
    return;

  m_windowOrder = std::move(reconciled);

  if (m_windowOrder.size() == taskbarOrder.size())
    triggerLayoutUpdate();
}

//--------------------------------------------------------------------------------------------------
// Layout management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clears all tracked windows, z-order, and geometry.
 *        Resets the z-order counter.
 */
void UI::WindowManager::clear()
{
  m_zCounter = 1;
  m_windowZ.clear();
  m_windows.clear();
  m_windowOrder.clear();
  m_dragWindow             = nullptr;
  m_targetWindow           = nullptr;
  m_resizeWindow           = nullptr;
  m_focusedWindow          = nullptr;
  m_layoutRestored         = false;
  m_userReordered          = false;
  m_suppressGeometrySignal = false;
  m_lastCanvasWidth        = 0;
  m_lastCanvasHeight       = 0;
  m_snapIndicatorVisible   = false;
  m_layoutStore.clear();
  m_snapOverlay.clearSnapSiblings();
  m_snapOverlay.clearManualGesture();

  Q_EMIT zCounterChanged();
  Q_EMIT snapIndicatorChanged();
}

/**
 * @brief Loads the appropriate layout based on current settings.
 */
void UI::WindowManager::loadLayout()
{
  if (m_layoutRestored)
    constrainWindows();

  else if (autoLayoutEnabled())
    autoLayout();
  else
    cascadeLayout();
}

/**
 * @brief Automatically tiles visible windows using a smart grid-based layout. The layout
 *        margin is not applied here: the QML canvas insets this item by it, so both layout
 *        modes share one application point.
 */
void UI::WindowManager::autoLayout()
{
  const QSize canvas = canvasSize();
  if (canvas.width() <= 0 || canvas.height() <= 0)
    return;

  if (WindowGeometry::anyWindowMaximized(m_windows))
    return;

  const auto windows = WindowGeometry::normalWindowsInOrder(m_windows, m_windowOrder);
  if (windows.isEmpty())
    return;

  Layouts::LayoutEnv env;
  env.margin      = 0;
  env.spacing     = qMax(-1, m_dashboard.layoutSpacing());
  env.availW      = canvas.width();
  env.availH      = canvas.height();
  env.isLandscape = env.availW >= env.availH;
  env.minWidth    = kAutoLayoutMinWidth;
  env.minHeight   = kAutoLayoutMinHeight;
  env.ratio       = m_layoutRatio;

  const auto rects = Layouts::tile(windows.size(), Layouts::patternFromId(m_layoutPattern), env);
  const int placed = qMin(rects.size(), windows.size());
  for (int i = 0; i < placed; ++i)
    WindowGeometry::placeWindow(windows[i], rects[i]);

  WindowGeometry::showRestorableWindows(m_windows);
  computeMergedEdges();
  Q_EMIT geometryChanged(nullptr);
}

/**
 * @brief Re-reads the layout choice of whatever group or workspace the taskbar is showing.
 *        Called on every workspace switch, so the pattern can never be a leftover from the
 *        workspace the user just left.
 */
void UI::WindowManager::refreshLayoutChoice()
{
  if (!m_taskbar)
    return;

  auto& project     = DataModel::pipelineModules().projectModel;
  const auto choice = project.layoutChoice(m_taskbar->layoutScope(), m_taskbar->activeGroupId());

  m_layoutPattern = choice.value(QStringLiteral("pattern")).toString();
  m_layoutRatio   = qBound(1, choice.value(QStringLiteral("ratio")).toInt(), 15);
}

/**
 * @brief Returns the rectangles @a pattern would produce, normalized to a @a width x @a height
 *        preview box. The picker draws its artwork from this, so a tile can never disagree with
 *        what applying it does.
 */
QVariantList UI::WindowManager::patternPreview(
  const int pattern, const int count, const int width, const int height, const int ratio) const
{
  Layouts::LayoutEnv env;
  env.margin      = 0;
  env.spacing     = 1;
  env.availW      = qMax(1, width);
  env.availH      = qMax(1, height);
  env.isLandscape = env.availW >= env.availH;
  env.minWidth    = 1;
  env.minHeight   = 1;
  env.ratio       = qBound(1, ratio, Layouts::kRatioDenominator - 1);

  QVariantList out;
  const auto rects = Layouts::tile(qMax(1, count), static_cast<Layouts::Pattern>(pattern), env);
  for (const auto& rect : rects) {
    QVariantMap entry;
    entry["x"]      = rect.x();
    entry["y"]      = rect.y();
    entry["width"]  = rect.width();
    entry["height"] = rect.height();
    out.append(entry);
  }

  return out;
}

/**
 * @brief Recomputes the shared-border mask of every normal window and publishes it when it
 *        actually changed.
 */
void UI::WindowManager::computeMergedEdges()
{
  QVector<int> ids;
  QVector<QRect> rects;
  for (auto it = m_windows.constBegin(); it != m_windows.constEnd(); ++it) {
    if (!it.value() || it.value()->state() != "normal")
      continue;

    ids.append(it.key());
    rects.append(WindowGeometry::extractGeometry(it.value()));
  }

  const QVariantMap merged = WindowGeometry::mergedEdgeMasks(ids, rects);
  if (m_mergedEdges == merged)
    return;

  m_mergedEdges = merged;
  Q_EMIT mergedEdgesChanged();
}

/**
 * @brief Whether a pattern has a primary region, i.e. whether the picker should offer its
 *        split ratio.
 */
bool UI::WindowManager::patternHasPrimary(const int pattern) const
{
  return Layouts::patternHasPrimary(static_cast<Layouts::Pattern>(pattern));
}

/**
 * @brief The split ratios the picker offers, in sixteenths.
 */
QVariantList UI::WindowManager::layoutRatioStops() const
{
  QVariantList out;
  for (const int stop : Layouts::ratioStops())
    out.append(stop);

  return out;
}

/**
 * @brief Applies a pattern to the active workspace and re-tiles. An empty id is Grid, not
 *        manual mode: manual is the auto-layout switch turned off, which the picker's Manual
 *        entry does directly.
 */
void UI::WindowManager::selectLayoutPattern(const QString& pattern, const int ratio)
{
  m_layoutPattern = pattern.trimmed().toLower();
  m_layoutRatio   = qBound(1, ratio, Layouts::kRatioDenominator - 1);

  if (m_taskbar) {
    auto& project = DataModel::pipelineModules().projectModel;
    project.setLayoutChoice(
      m_taskbar->layoutScope(), m_taskbar->activeGroupId(), m_layoutPattern, m_layoutRatio);
  }

  Q_EMIT layoutChoiceChanged();

  if (!m_autoLayoutEnabled)
    setAutoLayoutEnabled(true);

  else
    autoLayout();
}

/**
 * @brief Arranges windows using a macOS-inspired smart cascade layout.
 */
void UI::WindowManager::cascadeLayout()
{
  const QSize canvas = canvasSize();
  if (canvas.width() <= 0 || canvas.height() <= 0)
    return;

  if (WindowGeometry::anyWindowMaximized(m_windows))
    return;

  const auto windows = WindowGeometry::normalWindowsInOrder(m_windows, m_windowOrder);
  if (windows.isEmpty())
    return;

  for (int i = 0; i < windows.size(); ++i) {
    const QSize minSize(qMax(static_cast<int>(windows[i]->implicitWidth()), kCascadeMinWidth),
                        qMax(static_cast<int>(windows[i]->implicitHeight()), kCascadeMinHeight));
    WindowGeometry::placeWindow(windows[i], WindowGeometry::cascadeGeometry(i, minSize, canvas));
  }

  WindowGeometry::showRestorableWindows(m_windows);
  Q_EMIT geometryChanged(nullptr);
}

/**
 * @brief Constrains all windows to fit within the current canvas bounds.
 */
void UI::WindowManager::constrainWindows()
{
  const QSize canvas = canvasSize();
  if (canvas.width() <= 0 || canvas.height() <= 0)
    return;

  const int floorW = autoLayoutEnabled() ? kAutoLayoutMinWidth : kManualMinSize;
  const int floorH = autoLayoutEnabled() ? kAutoLayoutMinHeight : kManualMinSize;

  for (auto* win : std::as_const(m_windows)) {
    if (!win || win->state() != "normal")
      continue;

    const QRect current(static_cast<int>(win->x()),
                        static_cast<int>(win->y()),
                        static_cast<int>(win->width()),
                        static_cast<int>(win->height()));
    const QRect fitted =
      WindowGeometry::constrainGeometry(current,
                                        canvas,
                                        qMax(static_cast<int>(win->implicitWidth()), floorW),
                                        qMax(static_cast<int>(win->implicitHeight()), floorH));
    if (fitted == current)
      continue;

    WindowGeometry::placeWindow(win, fitted);
    if (!m_suppressGeometrySignal)
      Q_EMIT geometryChanged(win);
  }

  WindowGeometry::showRestorableWindows(m_windows);
}

/**
 * @brief Reacts to changes in the desktop or available layout area.
 */
void UI::WindowManager::triggerLayoutUpdate()
{
  const QSize canvas   = canvasSize();
  const bool sizeValid = canvas.width() > 0 && canvas.height() > 0;
  const bool sizeChanged =
    sizeValid && (canvas.width() != m_lastCanvasWidth || canvas.height() != m_lastCanvasHeight);

  if (autoLayoutEnabled())
    autoLayout();

  else {
    const bool uninitialized = WindowGeometry::hasRestorableHiddenWindow(m_windows);
    if (sizeChanged)
      m_layoutStore.applyManualLayout(m_windows,
                                      canvas,
                                      QSize(m_lastCanvasWidth, m_lastCanvasHeight),
                                      m_dashboard.layoutSpacing());

    m_suppressGeometrySignal = sizeChanged;
    const bool shouldCascade = uninitialized && !m_layoutRestored;
    if (shouldCascade)
      cascadeLayout();

    if (!shouldCascade)
      constrainWindows();

    m_suppressGeometrySignal = false;
  }

  computeMergedEdges();

  if (sizeValid) {
    m_lastCanvasWidth  = canvas.width();
    m_lastCanvasHeight = canvas.height();
  }
}

//--------------------------------------------------------------------------------------------------
// Window registry
//--------------------------------------------------------------------------------------------------

/**
 * @brief Brings a window to the front by increasing its z-order.
 */
void UI::WindowManager::bringToFront(QQuickItem* item)
{
  if (!item)
    return;

  m_windowZ[item] = ++m_zCounter;
  item->setZ(m_windowZ[item]);

  Q_EMIT zCounterChanged();
  Q_EMIT zOrderChanged(item);
}

/**
 * @brief Sets the associated Taskbar instance for window management.
 */
void UI::WindowManager::setTaskbar(QQuickItem* taskbar)
{
  if (m_taskbar)
    disconnect(m_workspaceConnection);

  m_taskbar = qobject_cast<UI::Taskbar*>(taskbar);
  if (!m_taskbar)
    return;

  refreshLayoutChoice();
  m_workspaceConnection = connect(m_taskbar, &UI::Taskbar::activeGroupIdChanged, this, [this] {
    refreshLayoutChoice();
    Q_EMIT layoutChoiceChanged();
    if (m_autoLayoutEnabled)
      triggerLayoutUpdate();
  });
}

/**
 * @brief Registers a new window item with the manager, assigning initial z-order and geometry.
 */
void UI::WindowManager::registerWindow(const int id, QQuickItem* item)
{
  if (!item)
    return;

  m_windows[id] = item;
  m_windowOrder.append(id);
  m_windowZ[item] = ++m_zCounter;
  item->setZ(m_windowZ[item]);

  m_layoutStore.applyPendingGeometry(id, item);

  Q_EMIT zCounterChanged();
  Q_EMIT zOrderChanged(item);
}

/**
 * @brief Unregisters a window, removing its z-order and geometry tracking.
 */
void UI::WindowManager::unregisterWindow(QQuickItem* item)
{
  m_windowZ.remove(item);
  m_windowOrder.removeAll(WindowGeometry::idForWindow(m_windows, item));
  for (auto it = m_windows.begin(); it != m_windows.end(); ++it) {
    if (it.value() == item) {
      m_windows.remove(it.key());
      break;
    }
  }

  if (m_dragWindow || m_resizeWindow)
    m_snapOverlay.cacheSnapSiblings(m_windows, m_dragWindow ? m_dragWindow : m_resizeWindow);

  triggerLayoutUpdate();
}

//--------------------------------------------------------------------------------------------------
// Session state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Removes the background image and clears the settings.
 */
void UI::WindowManager::clearBackgroundImage()
{
  setBackgroundImage("");
}

/**
 * @brief Opens a file dialog to allow the user to select a background image.
 */
void UI::WindowManager::selectBackgroundImage()
{
  auto* dialog = new QFileDialog(qApp->activeWindow(),
                                 tr("Select Background Image"),
                                 QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                 tr("Images (*.png *.jpg *.jpeg *.bmp)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this,
      [this, path]() { setBackgroundImage(QUrl::fromLocalFile(path).toString()); },
      Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Sets the background image to be used for the container.
 */
void UI::WindowManager::setBackgroundImage(const QString& path)
{
  if (m_backgroundImage != path) {
    m_backgroundImage = path;
    m_settings.setValue("WindowManager_Wallpaper", path);
    Q_EMIT backgroundImageChanged();
  }
}

/**
 * @brief Freezes or unfreezes all window manipulation. Freezing aborts any in-flight
 *        drag/resize without committing geometry, so an async freeze (late license
 *        activation, project reload) cannot mutate a locked layout mid-gesture.
 */
void UI::WindowManager::setFrozen(const bool frozen)
{
  if (m_frozen == frozen)
    return;

  m_frozen = frozen;
  if (m_frozen) {
    ungrabMouse();
    unsetCursor();
    m_snapOverlay.clearManualGesture();

    m_dragWindow    = nullptr;
    m_targetWindow  = nullptr;
    m_resizeWindow  = nullptr;
    m_focusedWindow = nullptr;
    m_resizeEdge    = ResizeEdge::None;

    if (m_snapIndicatorVisible) {
      m_snapIndicatorVisible = false;
      Q_EMIT snapIndicatorChanged();
    }
  }

  Q_EMIT frozenChanged();
}

/**
 * @brief Enables or disables the manual-mode layout grid, persisting the choice.
 */
void UI::WindowManager::setGridEnabled(const bool enabled)
{
  if (m_gridEnabled == enabled)
    return;

  m_gridEnabled = enabled;
  m_settings.setValue("WindowManager_GridEnabled", enabled);
  Q_EMIT gridEnabledChanged();
}

/**
 * @brief Sets the grid cell size in pixels (bounded to 2..256), persisting the choice.
 */
void UI::WindowManager::setGridSize(const int size)
{
  const int bounded = qBound(2, size, 256);
  if (m_gridSize == bounded)
    return;

  m_gridSize = bounded;
  m_settings.setValue("WindowManager_GridSize", bounded);
  Q_EMIT gridSizeChanged();
}

/**
 * @brief Rebinds the manager to the layout universe named by @a key (mode, project, scope,
 *        group). Auto-versus-manual lives inside each group's layout blob, so a context with
 *        no blob falls back to auto instead of inheriting the mode of the workspace the user
 *        just left. Resets state only; the caller applies the layout right afterwards.
 */
void UI::WindowManager::setLayoutContext(const QString& key)
{
  if (m_layoutContextKey == key)
    return;

  m_layoutContextKey = key;
  if (m_autoLayoutEnabled)
    return;

  m_autoLayoutEnabled = true;
  m_layoutRestored    = false;
  m_layoutStore.clear();

  Q_EMIT autoLayoutEnabledChanged();
}

/**
 * @brief Enables or disables automatic window layout.
 */
void UI::WindowManager::setAutoLayoutEnabled(const bool enabled)
{
  if (m_autoLayoutEnabled != enabled) {
    m_layoutRestored    = false;
    m_autoLayoutEnabled = enabled;

    if (enabled)
      m_layoutStore.clearManualReference();

    for (auto* win : std::as_const(m_windows))
      if (win->state() == "maximized")
        QMetaObject::invokeMethod(win, "restoreClicked");

    loadLayout();

    if (!m_autoLayoutEnabled)
      m_layoutStore.storeManualLayout(m_windows, canvasSize());

    Q_EMIT autoLayoutEnabledChanged();
  }
}

//--------------------------------------------------------------------------------------------------
// Hover feedback
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the manual-mode resize cursor matching the hovered edge; an edge-less point
 *        unsets the cursor instead.
 */
void UI::WindowManager::applyResizeCursor(const ResizeEdge edge)
{
  const Qt::CursorShape shape = WindowGeometry::cursorForEdge(edge);
  if (shape == Qt::ArrowCursor)
    unsetCursor();
  else
    setCursor(shape);
}

/**
 * @brief Updates the resize cursor for a canvas-local point in manual mode.
 */
void UI::WindowManager::updateHoverCursor(const QPointF& pos)
{
  if (m_frozen || autoLayoutEnabled()) {
    unsetCursor();
    return;
  }

  auto* target = WindowGeometry::topmostWindowAt(m_windows, m_windowOrder, pos, m_dragWindow);
  if (!target || target->state() != "normal") {
    unsetCursor();
    return;
  }

  applyResizeCursor(WindowGeometry::detectResizeEdge(target, this, pos));
}

/**
 * @brief Freeze-mode focus follow: makes the window under the cursor active
 *        (raising it), since click-to-focus is suppressed while frozen.
 */
void UI::WindowManager::focusWindowUnderCursor(const QPointF& pos)
{
  if (!m_frozen || !m_taskbar)
    return;

  auto* target = WindowGeometry::topmostWindowAt(m_windows, m_windowOrder, pos, m_dragWindow);
  if (target && m_taskbar->activeWindow() != target)
    m_taskbar->setActiveWindow(target);
}

/**
 * @brief Updates the cursor when hovering over resizable edges in manual layout mode.
 */
void UI::WindowManager::hoverMoveEvent(QHoverEvent* event)
{
  updateHoverCursor(event->position());
  QQuickItem::hoverMoveEvent(event);
}

/**
 * @brief Clears the resize cursor when the pointer leaves the canvas.
 */
void UI::WindowManager::hoverLeaveEvent(QHoverEvent* event)
{
  unsetCursor();
  QQuickItem::hoverLeaveEvent(event);
}

//--------------------------------------------------------------------------------------------------
// Drag & resize gestures
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles mouse movement during drag or resize operations.
 */
void UI::WindowManager::mouseMoveEvent(QMouseEvent* event)
{
  const QPoint currentPos = event->pos();
  const QPoint delta      = currentPos - m_initialMousePos;
  const int dragDistance  = delta.manhattanLength();

  if (!m_focusedWindow || m_focusedWindow->state() != "normal") {
    QQuickItem::mouseMoveEvent(event);
    return;
  }

  if (m_dragWindow && dragDistance >= kDragThreshold) {
    handleDragMove(event, delta);
    return;
  }

  if (m_resizeWindow) {
    handleResizeMove(event, delta);
    return;
  }

  QQuickItem::mouseMoveEvent(event);
}

/**
 * @brief Applies a drag delta to the focused window and updates snap indicators.
 */
void UI::WindowManager::handleDragMove(QMouseEvent* event, const QPoint& delta)
{
  int newX          = m_initialGeometry.x() + delta.x();
  int newY          = m_initialGeometry.y() + delta.y();
  int w             = static_cast<int>(m_dragWindow->width());
  int h             = static_cast<int>(m_dragWindow->height());
  const int canvasW = static_cast<int>(width());
  const int canvasH = static_cast<int>(height());

  if (!autoLayoutEnabled()) {
    QRect rect(newX, newY, w, h);
    if (event->modifiers() & Qt::AltModifier)
      m_snapOverlay.clearSnapGuides();

    else {
      const Snap::SnapInput in{rect,
                               QSize(canvasW, canvasH),
                               m_snapOverlay.snapSiblings(),
                               Snap::kSnapThreshold,
                               0,
                               m_gridEnabled,
                               m_gridSize,
                               m_dashboard.layoutSpacing(),
                               m_dashboard.showAlignmentGuides()};
      const Snap::SnapResult res = Snap::resolveMoveSnap(in);
      rect                       = res.rect;
      m_snapOverlay.publishSnapGuides(res);
    }

    const int x = qBound(0, rect.x(), qMax(0, canvasW - w));
    const int y = qBound(0, rect.y(), qMax(0, canvasH - h));
    m_dragWindow->setX(x);
    m_dragWindow->setY(y);
    m_snapOverlay.publishManualGesture(QRect(x, y, w, h));
    event->accept();
    return;
  }

  newX = qBound(0, newX, canvasW - w);
  newY = qBound(0, newY, canvasH - h);
  m_dragWindow->setX(newX);
  m_dragWindow->setY(newY);

  const QRect dragRect(newX, newY, w, h);
  m_targetWindow = WindowGeometry::findOverlapTarget(m_windows, dragRect, m_dragWindow);

  if (m_targetWindow && m_targetWindow != m_dragWindow) {
    m_dragWindow->setWidth(qMin(w, static_cast<int>(m_targetWindow->width())));
    m_dragWindow->setHeight(qMin(h, static_cast<int>(m_targetWindow->height())));
    const QRect targetRect = WindowGeometry::extractGeometry(m_targetWindow);
    m_snapIndicator        = WindowGeometry::liftSnapBottom(targetRect, canvasH);
    m_snapIndicatorVisible = true;
    Q_EMIT snapIndicatorChanged();
    event->accept();
    return;
  }

  if (m_snapIndicatorVisible) {
    m_snapIndicatorVisible = false;
    Q_EMIT snapIndicatorChanged();
  }

  event->accept();
}

/**
 * @brief Applies a resize delta to the focused window, clamped to canvas bounds. The clamped
 *        rectangle is what lands: discarding the whole gesture the moment an edge reached the
 *        canvas made the window stop following the pointer entirely (F19).
 */
void UI::WindowManager::handleResizeMove(QMouseEvent* event, const QPoint& delta)
{
  QRect geometry =
    WindowGeometry::computeResizedGeometry(m_initialGeometry,
                                           delta,
                                           m_resizeEdge,
                                           static_cast<int>(m_resizeWindow->implicitWidth()),
                                           static_cast<int>(m_resizeWindow->implicitHeight()));

  if (event->modifiers() & Qt::AltModifier)
    m_snapOverlay.clearSnapGuides();

  else {
    const int minSize = qMax(
      1, static_cast<int>(qMin(m_resizeWindow->implicitWidth(), m_resizeWindow->implicitHeight())));
    const Snap::SnapInput in{geometry,
                             canvasSize(),
                             m_snapOverlay.snapSiblings(),
                             Snap::kSnapThreshold,
                             minSize,
                             m_gridEnabled,
                             m_gridSize,
                             m_dashboard.layoutSpacing(),
                             m_dashboard.showAlignmentGuides()};
    const Snap::SnapResult res =
      Snap::resolveResizeSnap(in, WindowGeometry::movingEdgesFor(m_resizeEdge));
    geometry = res.rect;
    m_snapOverlay.publishSnapGuides(res);
    m_snapOverlay.publishFractionPreview(geometry, canvasSize(), m_dashboard.showAlignmentGuides());
  }

  geometry = WindowGeometry::clampResizeToCanvas(geometry, canvasSize());

  WindowGeometry::placeWindow(m_resizeWindow, geometry);
  m_snapOverlay.publishManualGesture(geometry);
  event->accept();
}

/**
 * @brief Manual-mode press logic: hit-tests the topmost window at pos, raises it,
 *        and starts a resize or drag. Returns true when the press is consumed.
 */
bool UI::WindowManager::startManualPress(const QPointF& pos, Qt::MouseButton button)
{
  if (m_frozen || autoLayoutEnabled())
    return false;

  m_dragWindow      = nullptr;
  m_targetWindow    = nullptr;
  m_resizeWindow    = nullptr;
  m_focusedWindow   = nullptr;
  m_resizeEdge      = ResizeEdge::None;
  m_initialMousePos = pos.toPoint();

  m_focusedWindow =
    WindowGeometry::manualResizeTargetAt(m_windows, m_windowOrder, this, m_initialMousePos);
  if (!m_focusedWindow)
    m_focusedWindow =
      WindowGeometry::topmostWindowAt(m_windows, m_windowOrder, m_initialMousePos, m_dragWindow);

  if (!m_focusedWindow)
    return false;

  const bool wasFocused = m_taskbar && m_taskbar->activeWindow() == m_focusedWindow;

  if (m_taskbar)
    m_taskbar->setActiveWindow(m_focusedWindow);

  bringToFront(m_focusedWindow);

  if (button != Qt::LeftButton || m_focusedWindow->state() != "normal")
    return false;

  m_resizeEdge = WindowGeometry::detectResizeEdge(m_focusedWindow, this, m_initialMousePos);
  if (m_resizeEdge != ResizeEdge::None) {
    m_resizeWindow    = m_focusedWindow;
    m_initialGeometry = WindowGeometry::extractGeometry(m_focusedWindow);
    m_snapOverlay.cacheSnapSiblings(m_windows, m_focusedWindow);
    grabMouse();
    return true;
  }

  const auto local     = m_focusedWindow->mapFromItem(this, m_initialMousePos);
  const int captionH   = m_focusedWindow->property("captionHeight").toInt();
  const int menuCtlW   = m_focusedWindow->property("menuControlWidth").toInt();
  const int buttonsW   = m_focusedWindow->property("windowControlsWidth").toInt();
  const bool onCaption = local.y() <= captionH && local.x() > menuCtlW
                      && local.x() <= m_focusedWindow->width() - buttonsW;
  const bool onControls = local.y() <= captionH && !onCaption;
  if (onControls)
    return false;

  if (wasFocused && local.y() > captionH)
    return false;

  m_dragWindow      = m_focusedWindow;
  m_initialGeometry = WindowGeometry::extractGeometry(m_focusedWindow);
  m_snapOverlay.cacheSnapSiblings(m_windows, m_focusedWindow);
  if (m_snapIndicatorVisible) {
    m_snapIndicatorVisible = false;
    Q_EMIT snapIndicatorChanged();
  }

  grabMouse();
  setCursor(Qt::ClosedHandCursor);
  return true;
}

/**
 * @brief Intercepts child-window presses so the manager owns focus/raise/drag in
 *        manual mode, passing non-management presses through to the widget.
 */
bool UI::WindowManager::childMouseEventFilter(QQuickItem* item, QEvent* event)
{
  if (m_frozen || autoLayoutEnabled() || event->type() != QEvent::MouseButtonPress)
    return false;

  auto* mouse       = static_cast<QMouseEvent*>(event);
  const QPointF pos = mapFromItem(item, mouse->position());
  return startManualPress(pos, mouse->button());
}

/**
 * @brief Handles mouse press interactions for initiating window drag or resize.
 */
void UI::WindowManager::mousePressEvent(QMouseEvent* event)
{
  if (m_frozen) {
    QQuickItem::mousePressEvent(event);
    return;
  }

  if (!autoLayoutEnabled()) {
    if (startManualPress(event->pos(), event->button())) {
      event->accept();
      return;
    }

    if (!m_focusedWindow) {
      if (m_taskbar)
        m_taskbar->setActiveWindow(nullptr);

      if (event->button() == Qt::RightButton) {
        Q_EMIT rightClicked(event->pos().x(), event->pos().y());
        event->accept();
        return;
      }
    }

    QQuickItem::mousePressEvent(event);
    return;
  }

  m_dragWindow      = nullptr;
  m_targetWindow    = nullptr;
  m_resizeWindow    = nullptr;
  m_focusedWindow   = nullptr;
  m_resizeEdge      = ResizeEdge::None;
  m_initialMousePos = event->pos();

  if (m_snapIndicatorVisible) {
    m_snapIndicatorVisible = false;
    Q_EMIT snapIndicatorChanged();
  }

  m_focusedWindow =
    WindowGeometry::topmostWindowAt(m_windows, m_windowOrder, m_initialMousePos, m_dragWindow);

  if (!m_focusedWindow) {
    if (m_taskbar)
      m_taskbar->setActiveWindow(nullptr);

    if (event->button() == Qt::RightButton)
      Q_EMIT rightClicked(m_initialMousePos.x(), m_initialMousePos.y());

    return;
  }

  if (m_taskbar)
    m_taskbar->setActiveWindow(m_focusedWindow);

  bool captionClick     = false;
  const int captionH    = m_focusedWindow->property("captionHeight").toInt();
  const int menuCtlW    = m_focusedWindow->property("menuControlWidth").toInt();
  const int buttonsW    = m_focusedWindow->property("windowControlsWidth").toInt();
  const auto mouseClick = m_focusedWindow->mapFromItem(this, m_initialMousePos);
  if (mouseClick.y() <= captionH) {
    if (mouseClick.x() <= m_focusedWindow->width() - buttonsW && mouseClick.x() > menuCtlW)
      captionClick = true;
    else {
      QQuickItem::mousePressEvent(event);
      return;
    }
  }

  if (m_focusedWindow->state() == "normal") {
    m_resizeEdge = WindowGeometry::detectResizeEdge(m_focusedWindow, this, m_initialMousePos);
    if (m_resizeEdge != ResizeEdge::None && !autoLayoutEnabled()) {
      grabMouse();
      applyResizeCursor(m_resizeEdge);
      m_resizeWindow    = m_focusedWindow;
      m_initialGeometry = WindowGeometry::extractGeometry(m_focusedWindow);
      event->accept();
      return;
    }

    if (captionClick) {
      grabMouse();
      setCursor(Qt::ClosedHandCursor);
      m_dragWindow      = m_focusedWindow;
      m_initialGeometry = WindowGeometry::extractGeometry(m_focusedWindow);
      event->accept();
    }
  }

  if (!captionClick)
    QQuickItem::mousePressEvent(event);
}

/**
 * @brief Swaps the dragged window with the snap target in m_windowOrder.
 */
bool UI::WindowManager::tryReorderDraggedWindow()
{
  if (!m_dragWindow || !m_targetWindow || !m_snapIndicatorVisible)
    return false;

  const int draggedId    = WindowGeometry::idForWindow(m_windows, m_dragWindow);
  const int targetId     = WindowGeometry::idForWindow(m_windows, m_targetWindow);
  const int newIndex     = m_windowOrder.indexOf(targetId);
  const int currentIndex = m_windowOrder.indexOf(draggedId);
  if (draggedId < 0 || targetId < 0 || currentIndex < 0 || newIndex < 0 || newIndex == currentIndex)
    return false;

  std::swap(m_windowOrder[currentIndex], m_windowOrder[newIndex]);
  m_userReordered = true;
  return true;
}

/**
 * @brief Re-bases the manual reference on the finished gesture and emits geometryChanged. The
 *        whole layout is snapshotted, not just the edited window: the canvas the reference is
 *        measured against is shared by all of them.
 */
void UI::WindowManager::commitManualGeometry(QQuickItem* window)
{
  if (!window)
    return;

  m_layoutStore.storeManualLayout(m_windows, canvasSize());
  computeMergedEdges();
  Q_EMIT geometryChanged(window);
}

/**
 * @brief Handles mouse release to finalize window drag or resize operations.
 */
void UI::WindowManager::mouseReleaseEvent(QMouseEvent* event)
{
  if (autoLayoutEnabled()) {
    const bool reordered = tryReorderDraggedWindow();
    loadLayout();

    if (reordered && m_dragWindow)
      Q_EMIT geometryChanged(m_dragWindow);
  }

  else {
    if (m_dragWindow)
      commitManualGeometry(m_dragWindow);
    else if (m_resizeWindow)
      commitManualGeometry(m_resizeWindow);
  }

  ungrabMouse();
  unsetCursor();
  m_snapOverlay.clearManualGesture();

  if (m_snapIndicatorVisible) {
    m_snapIndicatorVisible = false;
    Q_EMIT snapIndicatorChanged();
  }

  m_dragWindow      = nullptr;
  m_targetWindow    = nullptr;
  m_resizeWindow    = nullptr;
  m_focusedWindow   = nullptr;
  m_resizeEdge      = ResizeEdge::None;
  m_initialMousePos = event->pos();

  QQuickItem::mouseReleaseEvent(event);
}

/**
 * @brief Handles double-click events on window title bars to toggle maximize/restore.
 */
void UI::WindowManager::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (m_frozen) {
    QQuickItem::mouseDoubleClickEvent(event);
    return;
  }

  m_focusedWindow =
    WindowGeometry::topmostWindowAt(m_windows, m_windowOrder, event->pos(), m_dragWindow);
  if (!m_focusedWindow) {
    QQuickItem::mouseDoubleClickEvent(event);
    return;
  }

  const int captionH  = m_focusedWindow->property("captionHeight").toInt();
  const int menuCtlW  = m_focusedWindow->property("menuControlWidth").toInt();
  const int buttonsW  = m_focusedWindow->property("windowControlsWidth").toInt();
  const auto localPos = m_focusedWindow->mapFromItem(this, event->pos());
  if (localPos.y() <= captionH && localPos.x() <= m_focusedWindow->width() - buttonsW
      && localPos.x() > menuCtlW) {
    const QString state = m_focusedWindow->property("state").toString();

    if (state == "maximized")
      QMetaObject::invokeMethod(m_focusedWindow, "restoreClicked");

    else if (state == "normal")
      QMetaObject::invokeMethod(m_focusedWindow, "maximizeClicked");

    event->accept();
    return;
  }

  QQuickItem::mouseDoubleClickEvent(event);
}
