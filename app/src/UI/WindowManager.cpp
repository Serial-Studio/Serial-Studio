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
#include <QFile>
#include <QFileDialog>
#include <QPair>
#include <QSet>
#include <QSize>
#include <QStandardPaths>
#include <QUrl>

#include "DataModel/ProjectModel.h"
#include "SerialStudio.h"
#include "UI/Dashboard.h"
#include "UI/LayoutPatterns.h"
#include "UI/SnapGuides.h"
#include "UI/Taskbar.h"
#include "UI/UISessionRegistry.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Edge gap still read as one seam by the shared-border merge (spacing -1 overlaps by a pixel).
constexpr int kMergeTolerance = 1;

// Perpendicular overlap two windows need before their edges are treated as adjacent.
constexpr int kMergeOverlap = 2;

// Manual-mode window size floor; mirrors the fallback constrainWindows() applies.
constexpr int kManualMinSize = 48;

// Auto-layout size floor; mirrors the auto-mode floor constrainWindows() applies.
constexpr int kAutoLayoutMinWidth  = 100;
constexpr int kAutoLayoutMinHeight = 80;

namespace detail {

/**
 * @brief Stable (widgetType, relativeIndex) identity used as the on-disk key
 *        for saved layouts.
 */
struct StableKey {
  int widgetType    = -1;
  int relativeIndex = -1;

  /**
   * @brief Whether the key references a real widget.
   */
  [[nodiscard]] bool isValid() const { return widgetType >= 0 && relativeIndex >= 0; }

  /**
   * @brief Value equality on the (widgetType, relativeIndex) pair.
   */
  [[nodiscard]] bool operator==(const StableKey& other) const
  {
    return widgetType == other.widgetType && relativeIndex == other.relativeIndex;
  }
};

/**
 * @brief QHash hook for StableKey so it can index QHash<StableKey, int>.
 */
[[nodiscard]] inline size_t qHash(const StableKey& k, size_t seed = 0) noexcept
{
  return ::qHashMulti(seed, k.widgetType, k.relativeIndex);
}

/**
 * @brief Tiling environment shared by the auto-layout tiling helpers.
 */
}  // namespace detail

using detail::StableKey;

/**
 * @brief Builds a (widgetType, relativeIndex) -> windowId map from the live
 *        Dashboard widget map.
 */
[[nodiscard]] static QHash<StableKey, int> buildStableKeyToWindowId()
{
  QHash<StableKey, int> out;
  static auto& dashboard = UI::Dashboard::instance();
  const auto& widgetMap  = dashboard.widgetMap();
  out.reserve(widgetMap.size());
  for (auto it = widgetMap.cbegin(); it != widgetMap.cend(); ++it) {
    const StableKey key{static_cast<int>(it.value().first), it.value().second};
    out.insert(key, it.key());
  }
  return out;
}

/**
 * @brief Reads the stable key from a serialized geometry entry. Returns an
 *        invalid key for legacy entries that only carry the windowId.
 */
[[nodiscard]] static StableKey readStableKey(const QJsonObject& entry)
{
  if (!entry.contains("widgetType") || !entry.contains("relativeIndex"))
    return {};

  return {entry["widgetType"].toInt(-1), entry["relativeIndex"].toInt(-1)};
}

/**
 * @brief Resolves a serialized geometry/order entry to the current session's
 *        windowId, preferring the stable key over the legacy integer.
 */
[[nodiscard]] static int resolveSavedWindowId(const QJsonObject& entry,
                                              const QHash<StableKey, int>& lookup,
                                              const QMap<int, QQuickItem*>& liveWindows)
{
  const StableKey key = readStableKey(entry);
  if (key.isValid()) {
    const auto it = lookup.constFind(key);
    if (it != lookup.cend())
      return it.value();

    return -1;
  }

  const int legacyId = entry.value("id").toInt(-1);
  if (legacyId >= 0 && liveWindows.contains(legacyId))
    return legacyId;

  return -1;
}

/**
 * @brief Returns the (widgetType, relativeIndex) identity for the given live
 *        windowId, or an invalid key when the windowId is unknown.
 */
[[nodiscard]] static StableKey stableKeyForWindowId(int windowId)
{
  static auto& dashboard = UI::Dashboard::instance();
  const auto& widgetMap  = dashboard.widgetMap();
  const auto it          = widgetMap.constFind(windowId);
  if (it == widgetMap.cend())
    return {};

  return {static_cast<int>(it.value().first), it.value().second};
}

/**
 * @brief Shrinks a snap rectangle when its bottom reaches the canvas floor,
 *        keeping the border clear of the taskbar (canvas overlaps it 1px).
 */
static QRect liftSnapBottom(QRect rect, int ch)
{
  if (rect.y() + rect.height() >= ch)
    rect.setHeight(rect.height() - 2);

  return rect;
}

/**
 * @brief Maps a resize edge to the axis edges the gesture is moving.
 */
static UI::Snap::MovingEdges movingEdgesFor(const UI::WindowManager::ResizeEdge edge)
{
  using ResizeEdge = UI::WindowManager::ResizeEdge;
  const bool left =
    edge == ResizeEdge::Left || edge == ResizeEdge::TopLeft || edge == ResizeEdge::BottomLeft;
  const bool right =
    edge == ResizeEdge::Right || edge == ResizeEdge::TopRight || edge == ResizeEdge::BottomRight;
  const bool top =
    edge == ResizeEdge::Top || edge == ResizeEdge::TopLeft || edge == ResizeEdge::TopRight;
  const bool bottom =
    edge == ResizeEdge::Bottom || edge == ResizeEdge::BottomLeft || edge == ResizeEdge::BottomRight;
  return {left, right, top, bottom};
}

/**
 * @brief Serializes guide lines and spacing gaps into the QVariantList shape the
 *        QML overlay consumes.
 */
static void snapVisualsToVariants(const UI::Snap::SnapResult& result,
                                  QVariantList& guides,
                                  QVariantList& spacings)
{
  guides.reserve(result.guides.size());
  for (const auto& guide : result.guides) {
    QVariantMap entry;
    entry["x"]      = guide.rect.x();
    entry["y"]      = guide.rect.y();
    entry["width"]  = guide.rect.width();
    entry["height"] = guide.rect.height();
    entry["center"] = guide.kind == UI::Snap::GuideKind::Center;
    guides.append(entry);
  }

  spacings.reserve(result.spacings.size());
  for (const auto& spacing : result.spacings) {
    QVariantMap entry;
    entry["x"]      = spacing.rect.x();
    entry["y"]      = spacing.rect.y();
    entry["width"]  = spacing.rect.width();
    entry["height"] = spacing.rect.height();
    entry["gap"]    = spacing.gap;
    spacings.append(entry);
  }
}

/**
 * @brief Returns true if any tracked window is currently in the maximized state.
 */
static bool anyWindowMaximized(const QMap<int, QQuickItem*>& windows)
{
  for (auto* win : std::as_const(windows))
    if (win && win->state() == "maximized")
      return true;

  return false;
}

/**
 * @brief Sets a window's geometry to the given rectangle.
 */
static void placeWindow(QQuickItem* win, int x, int y, int w, int h)
{
  win->setX(x);
  win->setY(y);
  win->setWidth(w);
  win->setHeight(h);
}

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the WindowManager singleton.
 */
UI::WindowManager::WindowManager(QQuickItem* parent)
  : QQuickItem(parent)
  , m_dashboard(UI::Dashboard::instance())
  , m_sessionRegistry(UISessionRegistry::instance())
  , m_zCounter(1)
  , m_layoutRestored(false)
  , m_autoLayoutEnabled(true)
  , m_frozen(false)
  , m_userReordered(false)
  , m_suppressGeometrySignal(false)
  , m_manualCanvasWidth(0)
  , m_manualCanvasHeight(0)
  , m_lastCanvasWidth(0)
  , m_lastCanvasHeight(0)
  , m_resizeEdge(ResizeEdge::None)
  , m_snapIndicatorVisible(false)
  , m_gridEnabled(false)
  , m_gridSize(16)
  , m_layoutRatio(Layouts::kDefaultRatio)
  , m_manualGestureActive(false)
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

  connect(this, &UI::WindowManager::widthChanged, this, &UI::WindowManager::triggerLayoutUpdate);
  connect(this, &UI::WindowManager::heightChanged, this, &UI::WindowManager::triggerLayoutUpdate);

  connect(&m_dashboard, &UI::Dashboard::layoutSpacingChanged, this, [this] {
    if (!m_autoLayoutEnabled)
      applyManualLayout(static_cast<int>(width()), static_cast<int>(height()));
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
  return m_sizeMatchRect.isValid();
}

/**
 * @brief Returns whether a manual move/resize gesture is currently in progress.
 */
bool UI::WindowManager::manualGestureActive() const
{
  return m_manualGestureActive;
}

/**
 * @brief Returns the geometry of the sibling whose size the resize gesture matched.
 */
const QRect& UI::WindowManager::sizeMatchRect() const
{
  return m_sizeMatchRect;
}

/**
 * @brief Returns the footprint of the wrench-fraction resize preview; invalid when hidden.
 */
const QRect& UI::WindowManager::fractionPreviewRect() const
{
  return m_fractionPreviewRect;
}

/**
 * @brief Returns the fraction preview's label ("1/2 x 1/4"); empty when hidden.
 */
const QString& UI::WindowManager::fractionPreviewLabel() const
{
  return m_fractionPreviewLabel;
}

/**
 * @brief Returns the live geometry of the window being moved or resized.
 */
const QRect& UI::WindowManager::manualGestureGeometry() const
{
  return m_manualGestureGeometry;
}

/**
 * @brief Returns the alignment guide lines for the active gesture.
 */
const QVariantList& UI::WindowManager::alignmentGuides() const
{
  return m_alignmentGuides;
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
 * @brief Returns the equal-spacing indicators for the active gesture.
 */
const QVariantList& UI::WindowManager::spacingIndicators() const
{
  return m_spacingIndicators;
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

//--------------------------------------------------------------------------------------------------
// Layout management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes the current window layout to a JSON object.
 */
QJsonObject UI::WindowManager::serializeLayout() const
{
  QJsonObject layout;
  QJsonArray geometries;
  for (int id : m_windowOrder) {
    auto* win = m_windows.value(id);
    if (!win)
      continue;

    const StableKey key = stableKeyForWindowId(id);

    QJsonObject winGeom;
    winGeom["id"]     = id;
    winGeom["x"]      = win->x();
    winGeom["y"]      = win->y();
    winGeom["width"]  = win->width();
    winGeom["height"] = win->height();
    winGeom["state"]  = win->state();
    if (key.isValid()) {
      winGeom["widgetType"]    = key.widgetType;
      winGeom["relativeIndex"] = key.relativeIndex;
    }
    geometries.append(winGeom);
  }

  layout["geometries"] = geometries;

  layout["canvasWidth"]  = static_cast<int>(width());
  layout["canvasHeight"] = static_cast<int>(height());

  QJsonArray orderArray;
  QJsonArray orderTypedArray;
  for (int id : m_windowOrder) {
    orderArray.append(id);

    const StableKey key = stableKeyForWindowId(id);
    if (!key.isValid())
      continue;

    QJsonObject entry;
    entry["widgetType"]    = key.widgetType;
    entry["relativeIndex"] = key.relativeIndex;
    orderTypedArray.append(entry);
  }

  layout["windowOrder"]      = orderArray;
  layout["windowOrderTyped"] = orderTypedArray;
  layout["autoLayout"]       = m_autoLayoutEnabled;
  layout["userReordered"]    = m_userReordered;

  return layout;
}

/**
 * @brief Resolves a saved window-order array into live windowIds.
 */
QVector<int> UI::WindowManager::resolveSavedOrder(const QJsonObject& layout,
                                                  const QHash<StableKey, int>& stableLookup) const
{
  QVector<int> newOrder;
  QSet<int> seen;
  const bool typed     = layout.contains("windowOrderTyped");
  const QJsonArray src = (typed ? layout["windowOrderTyped"] : layout["windowOrder"]).toArray();

  for (const auto& val : std::as_const(src)) {
    int id = -1;
    if (typed)
      id = resolveSavedWindowId(val.toObject(), stableLookup, m_windows);
    else
      id = val.toInt(-1);

    if (id < 0 || seen.contains(id) || !m_windows.contains(id))
      continue;

    newOrder.append(id);
    seen.insert(id);
  }

  for (int id : std::as_const(m_windowOrder)) {
    if (!seen.contains(id)) {
      newOrder.append(id);
      seen.insert(id);
    }
  }

  return newOrder;
}

/**
 * @brief Reads the saved manual geometries, resolving each entry to a live window id.
 */
[[nodiscard]] static QVector<QPair<int, QRect>> parseSavedGeometries(
  const QJsonObject& layout,
  const QHash<StableKey, int>& stableLookup,
  const QMap<int, QQuickItem*>& windows)
{
  QVector<QPair<int, QRect>> out;
  const QJsonArray geometries = layout["geometries"].toArray();
  out.reserve(geometries.size());

  for (const auto& val : std::as_const(geometries)) {
    const QJsonObject winGeom = val.toObject();
    const int id              = resolveSavedWindowId(winGeom, stableLookup, windows);
    if (id < 0)
      continue;

    const int x = static_cast<int>(SerialStudio::toDouble(winGeom["x"], 0.0));
    const int y = static_cast<int>(SerialStudio::toDouble(winGeom["y"], 0.0));
    const int w = static_cast<int>(SerialStudio::toDouble(winGeom["width"], 200.0));
    const int h = static_cast<int>(SerialStudio::toDouble(winGeom["height"], 150.0));
    out.append(qMakePair(id, QRect(x, y, w, h)));
  }

  return out;
}

/**
 * @brief Applies saved manual-mode geometries to live windows and stashes the rescaled rects
 *        for any window that hasn't registered yet. The saved rects stay the reference the
 *        layout is re-derived from; only the placement follows the current canvas.
 */
void UI::WindowManager::applySavedGeometries(const QJsonObject& layout,
                                             const QHash<StableKey, int>& stableLookup,
                                             int marginCanvasW,
                                             int marginCanvasH)
{
  const auto saved = parseSavedGeometries(layout, stableLookup, m_windows);
  if (saved.isEmpty())
    return;

  QVector<QRect> reference;
  reference.reserve(saved.size());
  for (const auto& entry : std::as_const(saved))
    reference.append(entry.second);

  const QSize canvas(static_cast<int>(width()), static_cast<int>(height()));
  const auto placed = Layouts::rescaleManual(
    reference, QSize(marginCanvasW, marginCanvasH), canvas, m_dashboard.layoutSpacing());

  for (int i = 0; i < saved.size(); ++i) {
    if (auto* win = m_windows.value(saved[i].first))
      placeWindow(win, placed[i].x(), placed[i].y(), placed[i].width(), placed[i].height());

    m_manualGeometries.insert(saved[i].first, saved[i].second);
    m_pendingGeometries.insert(saved[i].first, placed[i]);
  }
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

  const QHash<StableKey, int> stableLookup = buildStableKeyToWindowId();

  if (layout.contains("windowOrderTyped") || layout.contains("windowOrder"))
    m_windowOrder = resolveSavedOrder(layout, stableLookup);

  m_manualGeometries.clear();
  m_pendingGeometries.clear();
  if (!autoLayout && layout.contains("geometries")) {
    const int canvasW       = static_cast<int>(width());
    const int canvasH       = static_cast<int>(height());
    const int marginCanvasW = savedCanvasW > 0 ? savedCanvasW : canvasW;
    const int marginCanvasH = savedCanvasH > 0 ? savedCanvasH : canvasH;
    m_manualCanvasWidth     = marginCanvasW;
    m_manualCanvasHeight    = marginCanvasH;

    applySavedGeometries(layout, stableLookup, marginCanvasW, marginCanvasH);
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
 * @brief Pre-stashes saved geometries so registerWindow can apply them per-window before first
 * paint, avoiding the minimumSize flash.
 */
void UI::WindowManager::preloadPendingGeometries(const QJsonObject& layout)
{
  m_pendingGeometries.clear();
  m_manualGeometries.clear();
  if (layout.isEmpty() || !layout.contains("geometries"))
    return;

  if (layout["autoLayout"].toBool(true))
    return;

  const int savedCanvasW  = layout["canvasWidth"].toInt(0);
  const int savedCanvasH  = layout["canvasHeight"].toInt(0);
  const int canvasW       = static_cast<int>(width());
  const int canvasH       = static_cast<int>(height());
  const int marginCanvasW = savedCanvasW > 0 ? savedCanvasW : canvasW;
  const int marginCanvasH = savedCanvasH > 0 ? savedCanvasH : canvasH;
  m_manualCanvasWidth     = marginCanvasW;
  m_manualCanvasHeight    = marginCanvasH;

  const QHash<StableKey, int> stableLookup = buildStableKeyToWindowId();
  const auto saved                         = parseSavedGeometries(layout, stableLookup, m_windows);
  if (saved.isEmpty())
    return;

  QVector<QRect> reference;
  reference.reserve(saved.size());
  for (const auto& entry : std::as_const(saved))
    reference.append(entry.second);

  const auto placed = Layouts::rescaleManual(reference,
                                             QSize(marginCanvasW, marginCanvasH),
                                             QSize(canvasW, canvasH),
                                             m_dashboard.layoutSpacing());

  for (int i = 0; i < saved.size(); ++i) {
    m_manualGeometries.insert(saved[i].first, saved[i].second);
    m_pendingGeometries.insert(saved[i].first, placed[i]);
  }
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
  m_manualCanvasWidth      = 0;
  m_manualCanvasHeight     = 0;
  m_lastCanvasWidth        = 0;
  m_lastCanvasHeight       = 0;
  m_snapIndicatorVisible   = false;
  m_manualGeometries.clear();
  m_pendingGeometries.clear();
  m_snapSiblings.clear();
  clearManualGesture();

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
  const int canvasW = static_cast<int>(width());
  const int canvasH = static_cast<int>(height());
  if (canvasW <= 0 || canvasH <= 0)
    return;

  if (anyWindowMaximized(m_windows))
    return;

  QList<QQuickItem*> windows;
  for (int id : std::as_const(m_windowOrder)) {
    auto* win = m_windows.value(id);
    if (win && win->state() == "normal")
      windows.append(win);
  }

  if (windows.isEmpty())
    return;

  Layouts::LayoutEnv env;
  env.margin      = 0;
  env.spacing     = qMax(-1, m_dashboard.layoutSpacing());
  env.availW      = canvasW;
  env.availH      = canvasH;
  env.isLandscape = env.availW >= env.availH;
  env.minWidth    = kAutoLayoutMinWidth;
  env.minHeight   = kAutoLayoutMinHeight;
  env.ratio       = m_layoutRatio;

  const auto rects = Layouts::tile(windows.size(), Layouts::patternFromId(m_layoutPattern), env);
  const int placed = qMin(rects.size(), windows.size());
  for (int i = 0; i < placed; ++i)
    placeWindow(windows[i], rects[i].x(), rects[i].y(), rects[i].width(), rects[i].height());

  for (auto* win : std::as_const(m_windows))
    if (win && !win->isVisible() && (win->state() == "normal" || win->state() == "maximized"))
      win->setVisible(true);

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

  static auto& project = DataModel::ProjectModel::instance();
  const auto choice    = project.layoutChoice(m_taskbar->layoutScope(), m_taskbar->activeGroupId());

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
 * @brief Returns which edges of each window coincide with a sibling's opposite edge, as a
 *        per-window bitmask (1 left, 2 right, 4 top, 8 bottom). Presentation only: QML draws a
 *        coincident pair as one border instead of two, and nothing here touches geometry.
 */
void UI::WindowManager::computeMergedEdges()
{
  QVector<int> ids;
  QVector<QRect> rects;
  for (auto it = m_windows.constBegin(); it != m_windows.constEnd(); ++it) {
    if (!it.value() || it.value()->state() != "normal")
      continue;

    ids.append(it.key());
    rects.append(extractGeometry(it.value()));
  }

  QVariantMap merged;
  for (int i = 0; i < ids.size(); ++i) {
    int mask = 0;
    for (int j = 0; j < ids.size(); ++j) {
      if (i == j)
        continue;

      const QRect a = rects[i];
      const QRect b = rects[j];
      const bool rows =
        qMin(a.y() + a.height(), b.y() + b.height()) - qMax(a.y(), b.y()) > kMergeOverlap;
      const bool cols =
        qMin(a.x() + a.width(), b.x() + b.width()) - qMax(a.x(), b.x()) > kMergeOverlap;

      if (rows && qAbs(a.x() - (b.x() + b.width())) <= kMergeTolerance)
        mask |= 1;

      if (rows && qAbs(b.x() - (a.x() + a.width())) <= kMergeTolerance)
        mask |= 2;

      if (cols && qAbs(a.y() - (b.y() + b.height())) <= kMergeTolerance)
        mask |= 4;

      if (cols && qAbs(b.y() - (a.y() + a.height())) <= kMergeTolerance)
        mask |= 8;
    }

    merged.insert(QString::number(ids[i]), mask);
  }

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
    static auto& project = DataModel::ProjectModel::instance();
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
  const int canvasW = static_cast<int>(width());
  const int canvasH = static_cast<int>(height());

  if (canvasW <= 0 || canvasH <= 0)
    return;

  if (anyWindowMaximized(m_windows))
    return;

  QList<QQuickItem*> visibleWindows;
  for (int id : std::as_const(m_windowOrder)) {
    auto* win = m_windows.value(id);
    if (win && win->state() == "normal")
      visibleWindows.append(win);
  }

  if (visibleWindows.isEmpty())
    return;

  const int margin         = 8;
  const int cascadeOffsetX = 26;
  const int cascadeOffsetY = 26;

  const int availableW = canvasW - 2 * margin;
  const int availableH = canvasH - 2 * margin;

  for (int i = 0; i < visibleWindows.size(); ++i) {
    QQuickItem* win = visibleWindows[i];
    if (!win)
      continue;

    int minW = qMax(static_cast<int>(win->implicitWidth()), 200);
    int minH = qMax(static_cast<int>(win->implicitHeight()), 150);

    int winW = qMax(minW, availableW * 55 / 100);
    int winH = qMax(minH, availableH * 60 / 100);

    winW = qMin(winW, availableW);
    winH = qMin(winH, availableH);

    int baseX = margin + (availableW - winW) / 2;
    int baseY = margin + (availableH - winH) / 2;

    int offsetX = i * cascadeOffsetX;
    int offsetY = i * cascadeOffsetY;

    int wrapCount = 0;
    while (baseY + offsetY + winH > canvasH - margin && wrapCount < 10) {
      offsetY -= (availableH - winH);
      offsetX += cascadeOffsetX * 2;
      wrapCount++;
    }

    while (baseX + offsetX + winW > canvasW - margin && wrapCount < 20) {
      offsetX -= (availableW - winW);
      wrapCount++;
    }

    int winX = baseX + offsetX;
    int winY = baseY + offsetY;

    winX = qBound(margin, winX, canvasW - winW - margin);
    winY = qBound(margin, winY, canvasH - winH - margin);

    if (winW > availableW) {
      winW = availableW;
      winX = margin;
    }

    if (winH > availableH) {
      winH = availableH;
      winY = margin;
    }

    win->setX(winX);
    win->setY(winY);
    win->setWidth(winW);
    win->setHeight(winH);
  }

  for (auto* win : std::as_const(m_windows)) {
    if (win && !win->isVisible()) {
      if (win->state() == "normal" || win->state() == "maximized")
        win->setVisible(true);
    }
  }

  Q_EMIT geometryChanged(nullptr);
}

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

  auto pending = m_pendingGeometries.find(id);
  if (pending != m_pendingGeometries.end()) {
    item->setX(pending.value().x());
    item->setY(pending.value().y());
    item->setWidth(pending.value().width());
    item->setHeight(pending.value().height());
    m_pendingGeometries.erase(pending);
  }

  Q_EMIT zCounterChanged();
  Q_EMIT zOrderChanged(item);
}

/**
 * @brief Unregisters a window, removing its z-order and geometry tracking.
 */
void UI::WindowManager::unregisterWindow(QQuickItem* item)
{
  m_windowZ.remove(item);
  m_windowOrder.removeAll(getIdForWindow(item));
  for (auto it = m_windows.begin(); it != m_windows.end(); ++it) {
    if (it.value() == item) {
      m_windows.remove(it.key());
      break;
    }
  }

  if (m_dragWindow || m_resizeWindow)
    cacheSnapSiblings(m_dragWindow ? m_dragWindow : m_resizeWindow);

  triggerLayoutUpdate();
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
    clearManualGesture();

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
 * @brief Enables or disables automatic window layout.
 */
void UI::WindowManager::setAutoLayoutEnabled(const bool enabled)
{
  if (m_autoLayoutEnabled != enabled) {
    m_layoutRestored    = false;
    m_autoLayoutEnabled = enabled;

    if (enabled) {
      m_manualGeometries.clear();
      m_manualCanvasWidth  = 0;
      m_manualCanvasHeight = 0;
    }

    for (auto* win : std::as_const(m_windows))
      if (win->state() == "maximized")
        QMetaObject::invokeMethod(win, "restoreClicked");

    loadLayout();

    if (!m_autoLayoutEnabled)
      storeManualLayout();

    Q_EMIT autoLayoutEnabledChanged();
  }
}

/**
 * @brief Stores one window's geometry as the reference the manual layout is re-derived from.
 */
void UI::WindowManager::storeManualGeometry(const int id, QQuickItem* item)
{
  if (!item)
    return;

  m_manualGeometries.insert(id, extractGeometry(item));

  const int canvasW = static_cast<int>(width());
  const int canvasH = static_cast<int>(height());
  if (canvasW <= 0 || canvasH <= 0)
    return;

  m_manualCanvasWidth  = canvasW;
  m_manualCanvasHeight = canvasH;
}

/**
 * @brief Snapshots every normal window as the new manual reference. The reference canvas size
 *        is shared, so storing one window alone would leave the rest read against a size they
 *        were never laid out on - which is how editing one widget used to move the others.
 */
void UI::WindowManager::storeManualLayout()
{
  for (auto it = m_windows.constBegin(); it != m_windows.constEnd(); ++it)
    if (it.value() && it.value()->state() == "normal")
      storeManualGeometry(it.key(), it.value());
}

/**
 * @brief Lays the stored manual reference out on a canvas of the given size, holding every
 *        join at the configured spacing and every outer edge flush. Reads the reference and
 *        never its own output, so repeated resizes cannot accumulate drift.
 */
void UI::WindowManager::applyManualLayout(const int newWidth, const int newHeight)
{
  if (newWidth <= 0 || newHeight <= 0)
    return;

  int refWidth  = m_manualCanvasWidth > 0 ? m_manualCanvasWidth : m_lastCanvasWidth;
  int refHeight = m_manualCanvasHeight > 0 ? m_manualCanvasHeight : m_lastCanvasHeight;
  refWidth      = refWidth > 0 ? refWidth : newWidth;
  refHeight     = refHeight > 0 ? refHeight : newHeight;

  m_manualCanvasWidth  = refWidth;
  m_manualCanvasHeight = refHeight;

  QVector<int> ids;
  QVector<QRect> reference;
  for (auto it = m_windows.constBegin(); it != m_windows.constEnd(); ++it) {
    auto* win = it.value();
    if (!win || win->state() != "normal")
      continue;

    if (!m_manualGeometries.contains(it.key()))
      m_manualGeometries.insert(it.key(), extractGeometry(win));

    ids.append(it.key());
    reference.append(m_manualGeometries.value(it.key()));
  }

  if (ids.isEmpty())
    return;

  const auto placed = Layouts::rescaleManual(
    reference, QSize(refWidth, refHeight), QSize(newWidth, newHeight), m_dashboard.layoutSpacing());

  for (int i = 0; i < ids.size(); ++i) {
    auto* win = m_windows.value(ids[i]);
    if (!win)
      continue;

    placeWindow(win,
                placed[i].x(),
                placed[i].y(),
                qMax(kManualMinSize, placed[i].width()),
                qMax(kManualMinSize, placed[i].height()));
  }
}

/**
 * @brief Caches the geometry of every visible normal window except the gesture
 *        target, so per-move snap resolution never re-walks the window map.
 */
void UI::WindowManager::cacheSnapSiblings(QQuickItem* target)
{
  m_snapSiblings.clear();
  m_snapSiblings.reserve(m_windows.size());
  for (auto it = m_windows.constBegin(); it != m_windows.constEnd(); ++it) {
    auto* win = it.value();
    if (!win || win == target || !win->isVisible() || win->state() != "normal")
      continue;

    m_snapSiblings.append(extractGeometry(win));
  }
}

/**
 * @brief Publishes the guide/spacing/size-match visuals of a snap resolution,
 *        emitting only for the properties that actually changed.
 */
void UI::WindowManager::publishSnapGuides(const Snap::SnapResult& result)
{
  QVariantList guides;
  QVariantList spacings;
  snapVisualsToVariants(result, guides, spacings);

  if (m_alignmentGuides != guides) {
    m_alignmentGuides = std::move(guides);
    Q_EMIT alignmentGuidesChanged();
  }

  if (m_spacingIndicators != spacings) {
    m_spacingIndicators = std::move(spacings);
    Q_EMIT spacingIndicatorsChanged();
  }

  if (m_sizeMatchRect != result.sizeMatch) {
    m_sizeMatchRect = result.sizeMatch;
    Q_EMIT sizeMatchRectChanged();
  }
}

/**
 * @brief Publishes the wrench-fraction preview for a resize in progress: the footprint the
 *        window occupies plus its width/height as canvas fractions. Only axes actually
 *        snapped onto a stop are named, so the panel confirms a real snap instead of
 *        approximating one; it describes the widget's size, never where it sits.
 */
void UI::WindowManager::publishFractionPreview(const QRect& geometry)
{
  const bool guidesOn = m_dashboard.showAlignmentGuides();
  const QString widthLabel =
    guidesOn ? Snap::fractionLabel(geometry.width(), static_cast<int>(width())) : QString();
  const QString heightLabel =
    guidesOn ? Snap::fractionLabel(geometry.height(), static_cast<int>(height())) : QString();

  QRect rect;
  QString label;
  if (!widthLabel.isEmpty() && !heightLabel.isEmpty())
    label = tr("Width: %1    Height: %2").arg(widthLabel, heightLabel);
  else if (!widthLabel.isEmpty())
    label = tr("Width: %1").arg(widthLabel);
  else if (!heightLabel.isEmpty())
    label = tr("Height: %1").arg(heightLabel);

  if (!label.isEmpty())
    rect = geometry;

  if (m_fractionPreviewRect == rect && m_fractionPreviewLabel == label)
    return;

  m_fractionPreviewRect  = rect;
  m_fractionPreviewLabel = label;
  Q_EMIT fractionPreviewChanged();
}

/**
 * @brief Clears the guide/spacing/size-match visuals, emitting only when needed.
 */
void UI::WindowManager::clearSnapGuides()
{
  if (m_fractionPreviewRect.isValid() || !m_fractionPreviewLabel.isEmpty()) {
    m_fractionPreviewRect = QRect();
    m_fractionPreviewLabel.clear();
    Q_EMIT fractionPreviewChanged();
  }

  if (!m_alignmentGuides.isEmpty()) {
    m_alignmentGuides.clear();
    Q_EMIT alignmentGuidesChanged();
  }

  if (!m_spacingIndicators.isEmpty()) {
    m_spacingIndicators.clear();
    Q_EMIT spacingIndicatorsChanged();
  }

  if (m_sizeMatchRect.isValid()) {
    m_sizeMatchRect = QRect();
    Q_EMIT sizeMatchRectChanged();
  }
}

/**
 * @brief Publishes the live geometry of the active manual gesture for the badge.
 */
void UI::WindowManager::publishManualGesture(const QRect& geometry)
{
  if (m_manualGestureActive && m_manualGestureGeometry == geometry)
    return;

  m_manualGestureActive   = true;
  m_manualGestureGeometry = geometry;
  Q_EMIT manualGestureChanged();
}

/**
 * @brief Ends the manual gesture: hides the badge and clears every snap visual.
 */
void UI::WindowManager::clearManualGesture()
{
  clearSnapGuides();
  if (!m_manualGestureActive)
    return;

  m_manualGestureActive = false;
  Q_EMIT manualGestureChanged();
}

/**
 * @brief Constrains all windows to fit within the current canvas bounds.
 */
void UI::WindowManager::constrainWindows()
{
  const int canvasW = static_cast<int>(width());
  const int canvasH = static_cast<int>(height());

  if (canvasW <= 0 || canvasH <= 0)
    return;

  for (auto* win : std::as_const(m_windows)) {
    if (!win)
      continue;

    if (win->state() != "normal")
      continue;

    int winX = static_cast<int>(win->x());
    int winY = static_cast<int>(win->y());
    int winW = static_cast<int>(win->width());
    int winH = static_cast<int>(win->height());

    const int floorW = autoLayoutEnabled() ? 100 : 48;
    const int floorH = autoLayoutEnabled() ? 80 : 48;
    const int minW   = qMax(static_cast<int>(win->implicitWidth()), floorW);
    const int minH   = qMax(static_cast<int>(win->implicitHeight()), floorH);

    bool changed = false;

    if (winW > canvasW) {
      winW    = canvasW;
      changed = true;
    }

    if (winH > canvasH) {
      winH    = canvasH;
      changed = true;
    }

    if (winW < minW && canvasW >= minW) {
      winW    = minW;
      changed = true;
    }

    if (winH < minH && canvasH >= minH) {
      winH    = minH;
      changed = true;
    }

    if (winX < 0) {
      winX    = 0;
      changed = true;
    }

    if (winY < 0) {
      winY    = 0;
      changed = true;
    }

    if (winX + winW > canvasW) {
      winX = canvasW - winW;
      if (winX < 0) {
        winX = 0;
        winW = canvasW;
      }
      changed = true;
    }

    if (winY + winH > canvasH) {
      winY = canvasH - winH;
      if (winY < 0) {
        winY = 0;
        winH = canvasH;
      }
      changed = true;
    }

    if (changed) {
      win->setX(winX);
      win->setY(winY);
      win->setWidth(winW);
      win->setHeight(winH);
      if (!m_suppressGeometrySignal)
        Q_EMIT geometryChanged(win);
    }
  }

  for (auto* win : std::as_const(m_windows)) {
    if (win && !win->isVisible()) {
      if (win->state() == "normal" || win->state() == "maximized")
        win->setVisible(true);
    }
  }
}

/**
 * @brief Reacts to changes in the desktop or available layout area.
 */
void UI::WindowManager::triggerLayoutUpdate()
{
  const int canvasW    = static_cast<int>(width());
  const int canvasH    = static_cast<int>(height());
  const bool sizeValid = canvasW > 0 && canvasH > 0;
  const bool sizeChanged =
    sizeValid && (canvasW != m_lastCanvasWidth || canvasH != m_lastCanvasHeight);

  if (autoLayoutEnabled())
    autoLayout();

  else {
    bool hasUninitializedWindows = false;
    for (auto* win : std::as_const(m_windows)) {
      if (win && !win->isVisible() && (win->state() == "normal" || win->state() == "maximized")) {
        hasUninitializedWindows = true;
        break;
      }
    }

    if (sizeChanged)
      applyManualLayout(canvasW, canvasH);

    m_suppressGeometrySignal = sizeChanged;
    const bool shouldCascade = hasUninitializedWindows && !m_layoutRestored;
    if (shouldCascade)
      cascadeLayout();

    if (!shouldCascade)
      constrainWindows();

    m_suppressGeometrySignal = false;
  }

  computeMergedEdges();

  if (sizeValid) {
    m_lastCanvasWidth  = canvasW;
    m_lastCanvasHeight = canvasH;
  }
}

/**
 * @brief Retrieves the ID associated with a registered window item.
 */
int UI::WindowManager::getIdForWindow(QQuickItem* item) const
{
  for (auto it = m_windows.constBegin(); it != m_windows.constEnd(); ++it)
    if (it.value() == item)
      return it.key();

  return -1;
}

/**
 * @brief Finds the non-drag window with the largest area overlap with the dragged rect.
 */
QQuickItem* UI::WindowManager::findOverlapTarget(const QRect& dragRect) const
{
  QQuickItem* best = nullptr;
  qint64 bestArea  = 0;

  for (auto it = m_windows.constBegin(); it != m_windows.constEnd(); ++it) {
    QQuickItem* win = it.value();
    if (!win || win == m_dragWindow || !win->isVisible())
      continue;

    if (win->state() != "normal")
      continue;

    const QRect winRect(static_cast<int>(win->x()),
                        static_cast<int>(win->y()),
                        static_cast<int>(win->width()),
                        static_cast<int>(win->height()));

    const QRect inter = dragRect.intersected(winRect);
    const qint64 area = static_cast<qint64>(inter.width()) * static_cast<qint64>(inter.height());
    if (area > bestArea) {
      bestArea = area;
      best     = win;
    }
  }

  return best;
}

/**
 * @brief Utility function to extract a window's actual geometry from its QQuickItem.
 */
QRect UI::WindowManager::extractGeometry(QQuickItem* item) const
{
  return QRect(item->x(),
               item->y(),
               item->width() > 0 ? item->width() : item->implicitWidth(),
               item->height() > 0 ? item->height() : item->implicitHeight());
}

/**
 * @brief Determines which edge or corner of a window is being hovered for resizing.
 */
UI::WindowManager::ResizeEdge UI::WindowManager::detectResizeEdge(QQuickItem* target,
                                                                  const QPointF& pos) const
{
  if (target->state() != "normal")
    return ResizeEdge::None;

  const int kResizeMargin = 8;
  QPointF localPos        = target->mapFromItem(this, pos);
  const int x             = static_cast<int>(localPos.x());
  const int y             = static_cast<int>(localPos.y());
  const int w             = static_cast<int>(target->width());
  const int h             = static_cast<int>(target->height());

  const bool nearLeft   = x <= kResizeMargin;
  const bool nearRight  = x >= w - kResizeMargin;
  const bool nearTop    = y <= kResizeMargin;
  const bool nearBottom = y >= h - kResizeMargin;

  if (nearLeft && nearTop)
    return ResizeEdge::TopLeft;

  if (nearRight && nearTop)
    return ResizeEdge::TopRight;

  if (nearLeft && nearBottom)
    return ResizeEdge::BottomLeft;

  if (nearRight && nearBottom)
    return ResizeEdge::BottomRight;

  if (nearLeft)
    return ResizeEdge::Left;

  if (nearRight)
    return ResizeEdge::Right;

  if (nearTop)
    return ResizeEdge::Top;

  if (nearBottom)
    return ResizeEdge::Bottom;

  return ResizeEdge::None;
}

/**
 * @brief Sets the manual-mode resize cursor matching the hovered edge.
 */
void UI::WindowManager::applyResizeCursor(ResizeEdge edge)
{
  switch (edge) {
    case ResizeEdge::Left:
    case ResizeEdge::Right:
      setCursor(Qt::SizeHorCursor);
      break;
    case ResizeEdge::Top:
    case ResizeEdge::Bottom:
      setCursor(Qt::SizeVerCursor);
      break;
    case ResizeEdge::TopRight:
    case ResizeEdge::BottomLeft:
      setCursor(Qt::SizeBDiagCursor);
      break;
    case ResizeEdge::TopLeft:
    case ResizeEdge::BottomRight:
      setCursor(Qt::SizeFDiagCursor);
      break;
    default:
      unsetCursor();
      break;
  }
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

  auto* target = topmostWindowAt(pos);
  if (!target || target->state() != "normal") {
    unsetCursor();
    return;
  }

  applyResizeCursor(detectResizeEdge(target, pos));
}

/**
 * @brief Freeze-mode focus follow: makes the window under the cursor active
 *        (raising it), since click-to-focus is suppressed while frozen.
 */
void UI::WindowManager::focusWindowUnderCursor(const QPointF& pos)
{
  if (!m_frozen || !m_taskbar)
    return;

  auto* target = topmostWindowAt(pos);
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

/**
 * @brief Returns m_windows sorted topmost-first: higher z wins, then a smaller
 *        m_windowOrder index breaks the tie so equal-z stacks stay deterministic.
 */
QVector<QQuickItem*> UI::WindowManager::sortedByVisualStacking() const
{
  QVector<QPair<int, QQuickItem*>> entries;
  entries.reserve(m_windows.size());
  for (auto it = m_windows.cbegin(); it != m_windows.cend(); ++it)
    entries.append({it.key(), it.value()});

  std::sort(entries.begin(),
            entries.end(),
            [this](const QPair<int, QQuickItem*>& a, const QPair<int, QQuickItem*>& b) {
              if (!a.second || !b.second)
                return a.second != nullptr;

              if (a.second->z() != b.second->z())
                return a.second->z() > b.second->z();

              const int ai = m_windowOrder.indexOf(a.first);
              const int bi = m_windowOrder.indexOf(b.first);
              if (ai < 0)
                return false;

              if (bi < 0)
                return true;

              return ai < bi;
            });

  QVector<QQuickItem*> out;
  out.reserve(entries.size());
  for (const auto& entry : std::as_const(entries))
    out.append(entry.second);

  return out;
}

/**
 * @brief Returns the topmost visible normal/maximized window whose bounding
 *        rect contains pos.
 */
QQuickItem* UI::WindowManager::topmostWindowAt(const QPointF& pos) const
{
  for (QQuickItem* window : sortedByVisualStacking()) {
    if (!window || !window->isVisible() || window == m_dragWindow)
      continue;

    const auto state = window->state();
    if (state != "normal" && state != "maximized")
      continue;

    const QRectF bounds(window->x(), window->y(), window->width(), window->height());
    if (bounds.contains(pos))
      return window;
  }

  return nullptr;
}

/**
 * @brief Manual-mode press pre-pass: returns the topmost normal window whose
 *        bounding rect contains pos and whose edge would start a resize. Lets
 *        a top window's corner win over a lower window's body at the same pixel.
 */
QQuickItem* UI::WindowManager::manualResizeTargetAt(const QPointF& pos) const
{
  for (QQuickItem* window : sortedByVisualStacking()) {
    if (!window || !window->isVisible() || window->state() != "normal")
      continue;

    const QRectF bounds(window->x(), window->y(), window->width(), window->height());
    if (!bounds.contains(pos))
      continue;

    if (detectResizeEdge(window, pos) != ResizeEdge::None)
      return window;
  }

  return nullptr;
}

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

  if (m_dragWindow && dragDistance >= 20) {
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
      clearSnapGuides();

    else {
      const Snap::SnapInput in{rect,
                               QSize(canvasW, canvasH),
                               m_snapSiblings,
                               Snap::kSnapThreshold,
                               0,
                               m_gridEnabled,
                               m_gridSize,
                               m_dashboard.layoutSpacing(),
                               m_dashboard.showAlignmentGuides()};
      const Snap::SnapResult res = Snap::resolveMoveSnap(in);
      rect                       = res.rect;
      publishSnapGuides(res);
    }

    const int x = qBound(0, rect.x(), qMax(0, canvasW - w));
    const int y = qBound(0, rect.y(), qMax(0, canvasH - h));
    m_dragWindow->setX(x);
    m_dragWindow->setY(y);
    publishManualGesture(QRect(x, y, w, h));
    event->accept();
    return;
  }

  newX = qBound(0, newX, canvasW - w);
  newY = qBound(0, newY, canvasH - h);
  m_dragWindow->setX(newX);
  m_dragWindow->setY(newY);

  const QRect dragRect(newX, newY, w, h);
  m_targetWindow = findOverlapTarget(dragRect);

  if (m_targetWindow && m_targetWindow != m_dragWindow) {
    m_dragWindow->setWidth(qMin(w, static_cast<int>(m_targetWindow->width())));
    m_dragWindow->setHeight(qMin(h, static_cast<int>(m_targetWindow->height())));
    m_snapIndicator        = liftSnapBottom(extractGeometry(m_targetWindow), canvasH);
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
 * @brief Computes the new geometry for the active resize window from the mouse delta.
 */
QRect UI::WindowManager::computeResizedGeometry(const QPoint& delta) const
{
  QRect geometry = m_initialGeometry;
  const int minW = m_resizeWindow->implicitWidth();
  const int minH = m_resizeWindow->implicitHeight();

  switch (m_resizeEdge) {
    case ResizeEdge::Right:
      geometry.setWidth(qMax(minW, m_initialGeometry.width() + delta.x()));
      return geometry;
    case ResizeEdge::Bottom:
      geometry.setHeight(qMax(minH, m_initialGeometry.height() + delta.y()));
      return geometry;
    case ResizeEdge::Left: {
      const int w = qMax(minW, m_initialGeometry.width() - delta.x());
      geometry.setX(m_initialGeometry.right() - w);
      geometry.setWidth(w);
      return geometry;
    }
    case ResizeEdge::Top: {
      const int h = qMax(minH, m_initialGeometry.height() - delta.y());
      geometry.setY(m_initialGeometry.bottom() - h);
      geometry.setHeight(h);
      return geometry;
    }
    case ResizeEdge::TopLeft: {
      const int w = qMax(minW, m_initialGeometry.width() - delta.x());
      const int h = qMax(minH, m_initialGeometry.height() - delta.y());
      geometry.setX(m_initialGeometry.right() - w);
      geometry.setWidth(w);
      geometry.setY(m_initialGeometry.bottom() - h);
      geometry.setHeight(h);
      return geometry;
    }
    case ResizeEdge::TopRight: {
      const int w = qMax(minW, m_initialGeometry.width() + delta.x());
      const int h = qMax(minH, m_initialGeometry.height() - delta.y());
      geometry.setY(m_initialGeometry.bottom() - h);
      geometry.setHeight(h);
      geometry.setWidth(w);
      return geometry;
    }
    case ResizeEdge::BottomLeft: {
      const int w = qMax(minW, m_initialGeometry.width() - delta.x());
      const int h = qMax(minH, m_initialGeometry.height() + delta.y());
      geometry.setX(m_initialGeometry.right() - w);
      geometry.setWidth(w);
      geometry.setHeight(h);
      return geometry;
    }
    case ResizeEdge::BottomRight:
      geometry.setWidth(qMax(minW, m_initialGeometry.width() + delta.x()));
      geometry.setHeight(qMax(minH, m_initialGeometry.height() + delta.y()));
      return geometry;
    case ResizeEdge::None:
      return geometry;
  }

  return geometry;
}

/**
 * @brief Applies a resize delta to the focused window, clamped to canvas bounds.
 */
void UI::WindowManager::handleResizeMove(QMouseEvent* event, const QPoint& delta)
{
  QRect geometry = computeResizedGeometry(delta);

  if (event->modifiers() & Qt::AltModifier)
    clearSnapGuides();

  else {
    const int minSize = qMax(
      1, static_cast<int>(qMin(m_resizeWindow->implicitWidth(), m_resizeWindow->implicitHeight())));
    const Snap::SnapInput in{geometry,
                             QSize(static_cast<int>(width()), static_cast<int>(height())),
                             m_snapSiblings,
                             Snap::kSnapThreshold,
                             minSize,
                             m_gridEnabled,
                             m_gridSize,
                             m_dashboard.layoutSpacing(),
                             m_dashboard.showAlignmentGuides()};
    const Snap::SnapResult res = Snap::resolveResizeSnap(in, movingEdgesFor(m_resizeEdge));
    geometry                   = res.rect;
    publishSnapGuides(res);
    publishFractionPreview(geometry);
  }

  const QRect unclamped = geometry;
  geometry.setX(qMax(0, geometry.x()));
  geometry.setY(qMax(0, geometry.y()));
  if (geometry.right() > int(width()) - 1)
    geometry.setWidth(int(width()) - geometry.x());

  if (geometry.bottom() > int(height()) - 1)
    geometry.setHeight(int(height()) - geometry.y());

  if (geometry == unclamped) {
    m_resizeWindow->setX(geometry.x());
    m_resizeWindow->setY(geometry.y());
    m_resizeWindow->setWidth(geometry.width());
    m_resizeWindow->setHeight(geometry.height());
    publishManualGesture(geometry);
    event->accept();
  }
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

  m_focusedWindow = manualResizeTargetAt(m_initialMousePos);
  if (!m_focusedWindow)
    m_focusedWindow = topmostWindowAt(m_initialMousePos);

  if (!m_focusedWindow)
    return false;

  const bool wasFocused = m_taskbar && m_taskbar->activeWindow() == m_focusedWindow;

  if (m_taskbar)
    m_taskbar->setActiveWindow(m_focusedWindow);

  bringToFront(m_focusedWindow);

  if (button != Qt::LeftButton || m_focusedWindow->state() != "normal")
    return false;

  m_resizeEdge = detectResizeEdge(m_focusedWindow, m_initialMousePos);
  if (m_resizeEdge != ResizeEdge::None) {
    m_resizeWindow    = m_focusedWindow;
    m_initialGeometry = extractGeometry(m_focusedWindow);
    cacheSnapSiblings(m_focusedWindow);
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
  m_initialGeometry = extractGeometry(m_focusedWindow);
  cacheSnapSiblings(m_focusedWindow);
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

  m_focusedWindow = topmostWindowAt(m_initialMousePos);

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
    m_resizeEdge = detectResizeEdge(m_focusedWindow, m_initialMousePos);
    if (m_resizeEdge != ResizeEdge::None && !autoLayoutEnabled()) {
      grabMouse();
      applyResizeCursor(m_resizeEdge);
      m_resizeWindow    = m_focusedWindow;
      m_initialGeometry = extractGeometry(m_focusedWindow);
      event->accept();
      return;
    }

    if (captionClick) {
      grabMouse();
      setCursor(Qt::ClosedHandCursor);
      m_dragWindow      = m_focusedWindow;
      m_initialGeometry = extractGeometry(m_focusedWindow);
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

  const int draggedId    = getIdForWindow(m_dragWindow);
  const int targetId     = getIdForWindow(m_targetWindow);
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

  storeManualLayout();
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
  clearManualGesture();

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

  m_focusedWindow = topmostWindowAt(event->pos());
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
