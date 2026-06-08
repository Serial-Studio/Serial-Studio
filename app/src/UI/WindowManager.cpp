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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "WindowManager.h"

#include <optional>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QSet>
#include <QStandardPaths>
#include <QtMath>
#include <QUrl>

#include "SerialStudio.h"
#include "UI/Dashboard.h"
#include "UI/Taskbar.h"
#include "UI/UISessionRegistry.h"

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
 * @brief Tiling environment shared by every per-count auto-layout helper.
 */
struct TileEnv {
  int margin;
  int spacing;
  int availW;
  int availH;
  bool isLandscape;
};

}  // namespace detail

using detail::StableKey;
using detail::TileEnv;

/**
 * @brief Builds a (widgetType, relativeIndex) -> windowId map from the live
 *        Dashboard widget map.
 */
[[nodiscard]] static QHash<StableKey, int> buildStableKeyToWindowId()
{
  QHash<StableKey, int> out;
  const auto& widgetMap = UI::Dashboard::instance().widgetMap();
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
  const auto& widgetMap = UI::Dashboard::instance().widgetMap();
  const auto it         = widgetMap.constFind(windowId);
  if (it == widgetMap.cend())
    return {};

  return {static_cast<int>(it.value().first), it.value().second};
}

/**
 * @brief Returns the manual-mode snap rectangle for the dragged window's edges.
 */
static std::optional<QRect> computeSnapRect(
  int left, int top, int right, int bottom, int cw, int ch)
{
  if (left <= 0 && top <= 0)
    return QRect(0, 0, cw / 2, ch / 2);

  if (right >= cw && top <= 0)
    return QRect(cw / 2, 0, cw / 2, ch / 2);

  if (left <= 0 && bottom >= ch)
    return QRect(0, ch / 2, cw / 2, ch / 2);

  if (right >= cw && bottom >= ch)
    return QRect(cw / 2, ch / 2, cw / 2, ch / 2);

  if (top <= 0)
    return QRect(0, 0, cw, ch);

  if (left <= 0)
    return QRect(0, 0, cw / 2, ch);

  if (right >= cw)
    return QRect(cw / 2, 0, cw / 2, ch);

  return std::nullopt;
}

/**
 * @brief Returns manual anchor margins for a geometry within a canvas.
 */
static QMargins manualMarginsForGeometry(const QRect& geom, const int canvasW, const int canvasH)
{
  if (canvasW <= 0 || canvasH <= 0)
    return QMargins();

  const int left   = qMax(0, geom.x());
  const int top    = qMax(0, geom.y());
  const int right  = qMax(0, canvasW - (geom.x() + geom.width()));
  const int bottom = qMax(0, canvasH - (geom.y() + geom.height()));
  return QMargins(left, top, right, bottom);
}

/**
 * @brief Returns the anchored geometry for a preferred size within a canvas.
 */
static QRect anchoredGeometry(const QRect& preferred,
                              const QMargins& margins,
                              const int canvasW,
                              const int canvasH)
{
  if (canvasW <= 0 || canvasH <= 0)
    return preferred;

  const bool anchorLeft = margins.left() <= margins.right();
  const bool anchorTop  = margins.top() <= margins.bottom();
  const int x = anchorLeft ? margins.left() : canvasW - (margins.right() + preferred.width());
  const int y = anchorTop ? margins.top() : canvasH - (margins.bottom() + preferred.height());
  return QRect(x, y, preferred.width(), preferred.height());
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

/**
 * @brief One window fills the available canvas area.
 */
static void tileOne(const QList<QQuickItem*>& wins, const TileEnv& env)
{
  placeWindow(wins[0], env.margin, env.margin, env.availW, env.availH);
}

/**
 * @brief Two windows split side-by-side or stacked depending on orientation.
 */
static void tileTwo(const QList<QQuickItem*>& wins, const TileEnv& env)
{
  if (env.isLandscape) {
    const int w = (env.availW - env.spacing) / 2;
    placeWindow(wins[0], env.margin, env.margin, w, env.availH);
    placeWindow(wins[1], env.margin + w + env.spacing, env.margin, w, env.availH);
    return;
  }

  const int h = (env.availH - env.spacing) / 2;
  placeWindow(wins[0], env.margin, env.margin, env.availW, h);
  placeWindow(wins[1], env.margin, env.margin + h + env.spacing, env.availW, h);
}

/**
 * @brief Three windows in a master + 2-stack arrangement.
 */
static void tileThree(const QList<QQuickItem*>& wins, const TileEnv& env)
{
  if (env.isLandscape) {
    const int masterW = env.availW / 2;
    const int stackW  = env.availW - masterW - env.spacing;
    const int stackH  = (env.availH - env.spacing) / 2;
    placeWindow(wins[0], env.margin, env.margin, masterW, env.availH);
    placeWindow(wins[1], env.margin + masterW + env.spacing, env.margin, stackW, stackH);
    placeWindow(wins[2],
                env.margin + masterW + env.spacing,
                env.margin + stackH + env.spacing,
                stackW,
                stackH);
    return;
  }

  const int masterH = env.availH / 2;
  const int stackH  = env.availH - masterH - env.spacing;
  const int stackW  = (env.availW - env.spacing) / 2;
  placeWindow(wins[0], env.margin, env.margin, env.availW, masterH);
  placeWindow(wins[1], env.margin, env.margin + masterH + env.spacing, stackW, stackH);
  placeWindow(
    wins[2], env.margin + stackW + env.spacing, env.margin + masterH + env.spacing, stackW, stackH);
}

/**
 * @brief Four windows in a 2x2 grid.
 */
static void tileFour(const QList<QQuickItem*>& wins, const TileEnv& env)
{
  const int w = (env.availW - env.spacing) / 2;
  const int h = (env.availH - env.spacing) / 2;
  placeWindow(wins[0], env.margin, env.margin, w, h);
  placeWindow(wins[1], env.margin + w + env.spacing, env.margin, w, h);
  placeWindow(wins[2], env.margin, env.margin + h + env.spacing, w, h);
  placeWindow(wins[3], env.margin + w + env.spacing, env.margin + h + env.spacing, w, h);
}

/**
 * @brief Five windows in an asymmetric 2+3 arrangement.
 */
static void tileFive(const QList<QQuickItem*>& wins, const TileEnv& env)
{
  if (env.isLandscape) {
    const int topW = (env.availW - env.spacing) / 2;
    const int botW = (env.availW - 2 * env.spacing) / 3;
    const int h    = (env.availH - env.spacing) / 2;
    placeWindow(wins[0], env.margin, env.margin, topW, h);
    placeWindow(wins[1], env.margin + topW + env.spacing, env.margin, topW, h);
    placeWindow(wins[2], env.margin, env.margin + h + env.spacing, botW, h);
    placeWindow(wins[3], env.margin + botW + env.spacing, env.margin + h + env.spacing, botW, h);
    placeWindow(
      wins[4], env.margin + 2 * (botW + env.spacing), env.margin + h + env.spacing, botW, h);
    return;
  }

  const int leftH  = (env.availH - env.spacing) / 2;
  const int rightH = (env.availH - 2 * env.spacing) / 3;
  const int w      = (env.availW - env.spacing) / 2;
  placeWindow(wins[0], env.margin, env.margin, w, leftH);
  placeWindow(wins[1], env.margin, env.margin + leftH + env.spacing, w, leftH);
  placeWindow(wins[2], env.margin + w + env.spacing, env.margin, w, rightH);
  placeWindow(wins[3], env.margin + w + env.spacing, env.margin + rightH + env.spacing, w, rightH);
  placeWindow(
    wins[4], env.margin + w + env.spacing, env.margin + 2 * (rightH + env.spacing), w, rightH);
}

/**
 * @brief Six windows in a 3x2 or 2x3 grid based on canvas orientation.
 */
static void tileSix(const QList<QQuickItem*>& wins, const TileEnv& env)
{
  const int cols = env.isLandscape ? 3 : 2;
  const int rows = env.isLandscape ? 2 : 3;
  const int w    = (env.availW - (cols - 1) * env.spacing) / cols;
  const int h    = (env.availH - (rows - 1) * env.spacing) / rows;

  for (int i = 0; i < 6; ++i) {
    const int col = i % cols;
    const int row = i / cols;
    placeWindow(
      wins[i], env.margin + col * (w + env.spacing), env.margin + row * (h + env.spacing), w, h);
  }
}

/**
 * @brief Seven or more windows distributed in an aspect-aware optimal grid.
 */
static void tileGrid(const QList<QQuickItem*>& wins, const TileEnv& env)
{
  const int n = wins.size();
  if (n <= 0)
    return;

  const double safeW = qMax(1, env.availW);
  const double safeH = qMax(1, env.availH);

  int cols, rows;
  if (env.isLandscape) {
    cols = qCeil(qSqrt(static_cast<double>(n) * safeW / safeH));
    cols = qBound(1, cols, n);
    rows = qCeil(static_cast<double>(n) / cols);
  } else {
    rows = qCeil(qSqrt(static_cast<double>(n) * safeH / safeW));
    rows = qBound(1, rows, n);
    cols = qCeil(static_cast<double>(n) / rows);
  }

  cols = qBound(1, cols, n);
  rows = qBound(1, rows, n);

  for (int guard = 0; guard < 2 * n && cols * rows < n; ++guard)
    if (env.isLandscape)
      cols++;
    else
      rows++;

  const int spacingForSizing = qMax(env.spacing, 0);
  const int totalSpacingW    = (cols - 1) * spacingForSizing;
  const int totalSpacingH    = (rows - 1) * spacingForSizing;
  const int totalCellsW      = qMax(1, env.availW - totalSpacingW);
  const int totalCellsH      = qMax(1, env.availH - totalSpacingH);

  const int baseCellW = totalCellsW / cols;
  const int baseCellH = totalCellsH / rows;
  const int extraW    = totalCellsW % cols;
  const int extraH    = totalCellsH % rows;

  QVector<int> colWidths(cols), colXs(cols);
  QVector<int> rowHeights(rows), rowYs(rows);

  int runningX = env.margin;
  for (int c = 0; c < cols; ++c) {
    colWidths[c]  = baseCellW + (c < extraW ? 1 : 0);
    colXs[c]      = runningX;
    runningX     += colWidths[c] + env.spacing;
  }

  int runningY = env.margin;
  for (int r = 0; r < rows; ++r) {
    rowHeights[r]  = baseCellH + (r < extraH ? 1 : 0);
    rowYs[r]       = runningY;
    runningY      += rowHeights[r] + env.spacing;
  }

  const int windowsInLastRow   = n - (rows - 1) * cols;
  const bool hasPartialLastRow = windowsInLastRow > 0 && windowsInLastRow < cols;
  QVector<int> lastRowWidths, lastRowXs;

  if (hasPartialLastRow) {
    const int totalSpacingLast = (windowsInLastRow - 1) * spacingForSizing;
    const int totalCellsLast   = qMax(1, env.availW - totalSpacingLast);
    const int baseLastW        = totalCellsLast / windowsInLastRow;
    const int extraLastW       = totalCellsLast % windowsInLastRow;

    lastRowWidths.resize(windowsInLastRow);
    lastRowXs.resize(windowsInLastRow);

    int runningLastX = env.margin;
    for (int c = 0; c < windowsInLastRow; ++c) {
      lastRowWidths[c]  = baseLastW + (c < extraLastW ? 1 : 0);
      lastRowXs[c]      = runningLastX;
      runningLastX     += lastRowWidths[c] + env.spacing;
    }
  }

  for (int i = 0; i < n; ++i) {
    const int row = i / cols;
    const int col = i % cols;

    if (hasPartialLastRow && row == rows - 1)
      placeWindow(wins[i], lastRowXs[col], rowYs[row], lastRowWidths[col], rowHeights[row]);
    else
      placeWindow(wins[i], colXs[col], rowYs[row], colWidths[col], rowHeights[row]);
  }
}

/**
 * @brief Dispatches to the appropriate tiling helper based on window count.
 */
static void dispatchTile(const QList<QQuickItem*>& wins, const TileEnv& env)
{
  switch (wins.size()) {
    case 1:
      tileOne(wins, env);
      return;
    case 2:
      tileTwo(wins, env);
      return;
    case 3:
      tileThree(wins, env);
      return;
    case 4:
      tileFour(wins, env);
      return;
    case 5:
      tileFive(wins, env);
      return;
    case 6:
      tileSix(wins, env);
      return;
    default:
      tileGrid(wins, env);
      return;
  }
}

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the WindowManager singleton.
 */
UI::WindowManager::WindowManager(QQuickItem* parent)
  : QQuickItem(parent)
  , m_zCounter(1)
  , m_layoutRestored(false)
  , m_autoLayoutEnabled(true)
  , m_userReordered(false)
  , m_suppressGeometrySignal(false)
  , m_manualCanvasWidth(0)
  , m_manualCanvasHeight(0)
  , m_lastCanvasWidth(0)
  , m_lastCanvasHeight(0)
  , m_resizeEdge(ResizeEdge::None)
  , m_snapIndicatorVisible(false)
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

  connect(this, &UI::WindowManager::widthChanged, this, &UI::WindowManager::triggerLayoutUpdate);
  connect(this, &UI::WindowManager::heightChanged, this, &UI::WindowManager::triggerLayoutUpdate);

  UISessionRegistry::instance().registerWindowManager(this);
}

/**
 * @brief Destroys the WindowManager and unregisters it from the UI session registry.
 */
UI::WindowManager::~WindowManager()
{
  UISessionRegistry::instance().unregisterWindowManager(this);
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
 * @brief Applies saved manual-mode geometries to live windows and stashes the
 *        anchored rects for any window that hasn't registered yet.
 */
void UI::WindowManager::applySavedGeometries(const QJsonObject& layout,
                                             const QHash<StableKey, int>& stableLookup,
                                             int marginCanvasW,
                                             int marginCanvasH)
{
  const int canvasW           = static_cast<int>(width());
  const int canvasH           = static_cast<int>(height());
  const QJsonArray geometries = layout["geometries"].toArray();

  for (const auto& val : std::as_const(geometries)) {
    const QJsonObject winGeom = val.toObject();
    const int id              = resolveSavedWindowId(winGeom, stableLookup, m_windows);
    if (id < 0)
      continue;

    const int x = static_cast<int>(SerialStudio::toDouble(winGeom["x"], 0.0));
    const int y = static_cast<int>(SerialStudio::toDouble(winGeom["y"], 0.0));
    const int w = static_cast<int>(SerialStudio::toDouble(winGeom["width"], 200.0));
    const int h = static_cast<int>(SerialStudio::toDouble(winGeom["height"], 150.0));
    const QRect geom(x, y, w, h);
    const QMargins margins = manualMarginsForGeometry(geom, marginCanvasW, marginCanvasH);
    const QRect anchored   = anchoredGeometry(geom, margins, canvasW, canvasH);

    auto* win = m_windows.value(id);
    if (win) {
      win->setX(anchored.x());
      win->setY(anchored.y());
      win->setWidth(anchored.width());
      win->setHeight(anchored.height());
    }

    m_manualGeometries.insert(id, geom);
    m_manualMargins.insert(id, margins);
    m_pendingGeometries.insert(id, anchored);
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
  m_manualMargins.clear();
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
  m_manualMargins.clear();
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

  const QJsonArray geometries = layout["geometries"].toArray();
  for (const auto& val : std::as_const(geometries)) {
    const QJsonObject winGeom = val.toObject();
    const int id              = resolveSavedWindowId(winGeom, stableLookup, m_windows);
    if (id < 0)
      continue;

    const int x = static_cast<int>(SerialStudio::toDouble(winGeom["x"], 0.0));
    const int y = static_cast<int>(SerialStudio::toDouble(winGeom["y"], 0.0));
    const int w = static_cast<int>(SerialStudio::toDouble(winGeom["width"], 200.0));
    const int h = static_cast<int>(SerialStudio::toDouble(winGeom["height"], 150.0));
    const QRect geom(x, y, w, h);
    const QMargins margins = manualMarginsForGeometry(geom, marginCanvasW, marginCanvasH);
    const QRect anchored   = anchoredGeometry(geom, margins, canvasW, canvasH);

    m_manualGeometries.insert(id, geom);
    m_manualMargins.insert(id, margins);
    m_pendingGeometries.insert(id, anchored);
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
  m_manualMargins.clear();
  m_pendingGeometries.clear();

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
 * @brief Automatically tiles visible windows using a smart grid-based layout.
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

  TileEnv env;
  env.margin      = 0;
  env.spacing     = -1;
  env.availW      = canvasW - 2 * env.margin;
  env.availH      = canvasH - 2 * env.margin;
  env.isLandscape = env.availW >= env.availH;
  dispatchTile(windows, env);

  for (auto* win : std::as_const(m_windows))
    if (win && !win->isVisible() && (win->state() == "normal" || win->state() == "maximized"))
      win->setVisible(true);

  Q_EMIT geometryChanged(nullptr);
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
  m_taskbar = qobject_cast<UI::Taskbar*>(taskbar);
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
 * @brief Enables or disables automatic window layout.
 */
void UI::WindowManager::setAutoLayoutEnabled(const bool enabled)
{
  if (m_autoLayoutEnabled != enabled) {
    m_layoutRestored    = false;
    m_autoLayoutEnabled = enabled;

    if (enabled) {
      m_manualGeometries.clear();
      m_manualMargins.clear();
      m_manualCanvasWidth  = 0;
      m_manualCanvasHeight = 0;
    }

    for (auto* win : std::as_const(m_windows))
      if (win->state() == "maximized")
        QMetaObject::invokeMethod(win, "restoreClicked");

    loadLayout();

    if (!m_autoLayoutEnabled)
      for (auto it = m_windows.constBegin(); it != m_windows.constEnd(); ++it)
        storeManualGeometry(it.key(), it.value());

    Q_EMIT autoLayoutEnabledChanged();
  }
}

/**
 * @brief Stores the preferred manual geometry and anchor margins for a window.
 */
void UI::WindowManager::storeManualGeometry(const int id,
                                            QQuickItem* item,
                                            const int canvasWidth,
                                            const int canvasHeight)
{
  if (!item)
    return;

  const QRect geom = extractGeometry(item);
  m_manualGeometries.insert(id, geom);

  const int canvasW = canvasWidth > 0 ? canvasWidth : static_cast<int>(width());
  const int canvasH = canvasHeight > 0 ? canvasHeight : static_cast<int>(height());
  if (canvasW <= 0 || canvasH <= 0)
    return;

  m_manualCanvasWidth  = canvasW;
  m_manualCanvasHeight = canvasH;
  m_manualMargins.insert(id, manualMarginsForGeometry(geom, canvasW, canvasH));
}

/**
 * @brief Repositions manual-layout windows to preserve edge anchoring on resize.
 */
void UI::WindowManager::applyManualAnchors(const int newWidth, const int newHeight)
{
  if (newWidth <= 0 || newHeight <= 0)
    return;

  int refWidth  = m_manualCanvasWidth > 0 ? m_manualCanvasWidth : newWidth;
  int refHeight = m_manualCanvasHeight > 0 ? m_manualCanvasHeight : newHeight;

  if (m_manualCanvasWidth <= 0 && m_lastCanvasWidth > 0)
    refWidth = m_lastCanvasWidth;

  if (m_manualCanvasHeight <= 0 && m_lastCanvasHeight > 0)
    refHeight = m_lastCanvasHeight;

  if (m_manualCanvasWidth <= 0 && refWidth > 0)
    m_manualCanvasWidth = refWidth;

  if (m_manualCanvasHeight <= 0 && refHeight > 0)
    m_manualCanvasHeight = refHeight;

  const double scaleX = refWidth > 0 ? double(newWidth) / double(refWidth) : 1.0;
  const double scaleY = refHeight > 0 ? double(newHeight) / double(refHeight) : 1.0;

  for (auto it = m_windows.constBegin(); it != m_windows.constEnd(); ++it) {
    const int id = it.key();
    auto* win    = it.value();
    if (!win || win->state() != "normal")
      continue;

    if (!m_manualGeometries.contains(id) || !m_manualMargins.contains(id))
      storeManualGeometry(id, win);

    const QRect prefGeom   = m_manualGeometries.value(id, extractGeometry(win));
    const QMargins margins = m_manualMargins.value(id, QMargins());
    int scaledW            = qRound(prefGeom.width() * scaleX);
    int scaledH            = qRound(prefGeom.height() * scaleY);
    const QMargins scaledMargins(qRound(margins.left() * scaleX),
                                 qRound(margins.top() * scaleY),
                                 qRound(margins.right() * scaleX),
                                 qRound(margins.bottom() * scaleY));
    const int maxW = qMax(0, newWidth - scaledMargins.left() - scaledMargins.right());
    const int maxH = qMax(0, newHeight - scaledMargins.top() - scaledMargins.bottom());
    scaledW        = qMin(scaledW, maxW);
    scaledH        = qMin(scaledH, maxH);
    const QRect scaledPref(0, 0, scaledW, scaledH);
    const QRect anchored = anchoredGeometry(scaledPref, scaledMargins, newWidth, newHeight);

    win->setX(anchored.x());
    win->setY(anchored.y());
    win->setWidth(anchored.width());
    win->setHeight(anchored.height());
  }
}

/**
 * @brief Applies the manual snap indicator to the dragged window.
 */
void UI::WindowManager::applyManualSnap()
{
  if (!m_dragWindow || !m_snapIndicatorVisible)
    return;

  const int x = m_snapIndicator.x();
  const int y = m_snapIndicator.y();
  const int w = m_snapIndicator.width();
  const int h = m_snapIndicator.height();

  if (x == 0 && y == 0 && w >= width() && h >= height()) {
    QMetaObject::invokeMethod(m_dragWindow, "maximizeClicked");
    return;
  }

  m_dragWindow->setX(x);
  m_dragWindow->setY(y);
  m_dragWindow->setWidth(w);
  m_dragWindow->setHeight(h);
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

    const int minW = qMax(static_cast<int>(win->implicitWidth()), 100);
    const int minH = qMax(static_cast<int>(win->implicitHeight()), 80);

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
      applyManualAnchors(canvasW, canvasH);

    m_suppressGeometrySignal = sizeChanged;
    const bool shouldCascade = hasUninitializedWindows && !m_layoutRestored;
    if (shouldCascade)
      cascadeLayout();

    if (!shouldCascade)
      constrainWindows();

    m_suppressGeometrySignal = false;
  }

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
  if (autoLayoutEnabled()) {
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
 * @brief Updates the manual-mode edge snap indicator based on the dragged window position.
 */
void UI::WindowManager::updateManualSnapIndicator(
  int newX, int newY, int w, int h, int canvasW, int canvasH)
{
  const auto snap = computeSnapRect(newX, newY, newX + w, newY + h, canvasW, canvasH);
  if (snap.has_value()) {
    m_snapIndicator        = *snap;
    m_snapIndicatorVisible = true;
    Q_EMIT snapIndicatorChanged();
    return;
  }

  if (m_snapIndicatorVisible) {
    m_snapIndicatorVisible = false;
    Q_EMIT snapIndicatorChanged();
  }
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

  if ((w >= canvasW - 20 || h >= canvasH - 20) && !autoLayoutEnabled()) {
    w = static_cast<int>(m_dragWindow->implicitWidth());
    h = static_cast<int>(m_dragWindow->implicitHeight());
    m_dragWindow->setWidth(w);
    m_dragWindow->setHeight(h);
  }

  newX = qBound(0, newX, canvasW - w);
  newY = qBound(0, newY, canvasH - h);
  m_dragWindow->setX(newX);
  m_dragWindow->setY(newY);

  if (!autoLayoutEnabled()) {
    updateManualSnapIndicator(newX, newY, w, h, canvasW, canvasH);
    event->accept();
    return;
  }

  const QRect dragRect(newX, newY, w, h);
  m_targetWindow = findOverlapTarget(dragRect);

  if (m_targetWindow && m_targetWindow != m_dragWindow) {
    m_dragWindow->setWidth(qMin(w, static_cast<int>(m_targetWindow->width())));
    m_dragWindow->setHeight(qMin(h, static_cast<int>(m_targetWindow->height())));
    m_snapIndicator        = extractGeometry(m_targetWindow);
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
    event->accept();
  }
}

/**
 * @brief Manual-mode press logic: hit-tests the topmost window at pos, raises it,
 *        and starts a resize or drag. Returns true when the press is consumed.
 */
bool UI::WindowManager::startManualPress(const QPointF& pos, Qt::MouseButton button)
{
  if (autoLayoutEnabled())
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
    grabMouse();
    return true;
  }

  const auto local     = m_focusedWindow->mapFromItem(this, m_initialMousePos);
  const int captionH   = m_focusedWindow->property("captionHeight").toInt();
  const int externcW   = m_focusedWindow->property("externControlWidth").toInt();
  const int buttonsW   = m_focusedWindow->property("windowControlsWidth").toInt();
  const bool onCaption = local.y() <= captionH && local.x() > externcW
                      && local.x() <= m_focusedWindow->width() - buttonsW;
  const bool onControls = local.y() <= captionH && !onCaption;
  if (onControls)
    return false;

  if (wasFocused && local.y() > captionH)
    return false;

  m_dragWindow      = m_focusedWindow;
  m_initialGeometry = extractGeometry(m_focusedWindow);
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
  if (autoLayoutEnabled() || event->type() != QEvent::MouseButtonPress)
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
  if (!autoLayoutEnabled()) {
    if (startManualPress(event->pos(), event->button())) {
      event->accept();
      return;
    }

    if (event->button() == Qt::RightButton && !m_focusedWindow) {
      if (m_taskbar)
        m_taskbar->setActiveWindow(nullptr);

      Q_EMIT rightClicked(event->pos().x(), event->pos().y());
      event->accept();
      return;
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
  const int externcW    = m_focusedWindow->property("externControlWidth").toInt();
  const int buttonsW    = m_focusedWindow->property("windowControlsWidth").toInt();
  const auto mouseClick = m_focusedWindow->mapFromItem(this, m_initialMousePos);
  if (mouseClick.y() <= captionH) {
    if (mouseClick.x() <= m_focusedWindow->width() - buttonsW && mouseClick.x() > externcW)
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
 * @brief Persists window geometry under its id and emits geometryChanged.
 */
void UI::WindowManager::commitManualGeometry(QQuickItem* window)
{
  if (!window)
    return;

  const int id = getIdForWindow(window);
  if (id >= 0)
    storeManualGeometry(id, window);

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
    applyManualSnap();
    if (m_dragWindow)
      commitManualGeometry(m_dragWindow);
    else if (m_resizeWindow)
      commitManualGeometry(m_resizeWindow);
  }

  ungrabMouse();
  unsetCursor();

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
  m_focusedWindow = topmostWindowAt(event->pos());
  if (!m_focusedWindow) {
    QQuickItem::mouseDoubleClickEvent(event);
    return;
  }

  const int captionH  = m_focusedWindow->property("captionHeight").toInt();
  const int externcW  = m_focusedWindow->property("externControlWidth").toInt();
  const int buttonsW  = m_focusedWindow->property("windowControlsWidth").toInt();
  const auto localPos = m_focusedWindow->mapFromItem(this, event->pos());
  if (localPos.y() <= captionH && localPos.x() <= m_focusedWindow->width() - buttonsW
      && localPos.x() > externcW) {
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
