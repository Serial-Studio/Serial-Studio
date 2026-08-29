/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "UI/WindowManager/WindowLayoutStore.h"

#include <QJsonArray>
#include <QPair>
#include <QQuickItem>
#include <QSet>

#include "SerialStudio.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"
#include "UI/LayoutPatterns.h"
#include "UI/WindowManager/WindowGeometry.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Manual-mode window size floor; mirrors the fallback WindowManager::constrainWindows() applies.
constexpr int kLayoutManualMinSize = 48;

namespace detail {

/**
 * @brief Stable (widgetType, relativeIndex) identity used as the on-disk key for saved layouts.
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

}  // namespace detail

using detail::StableKey;

//--------------------------------------------------------------------------------------------------
// Stable key resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a (widgetType, relativeIndex) -> windowId map from the live Dashboard widget map.
 */
[[nodiscard]] static QHash<StableKey, int> buildStableKeyToWindowId(const UI::Dashboard& dashboard)
{
  QHash<StableKey, int> out;
  const auto& widgetMap = dashboard.widgetMap();
  out.reserve(widgetMap.size());
  for (auto it = widgetMap.cbegin(); it != widgetMap.cend(); ++it) {
    const StableKey key{static_cast<int>(it.value().first), it.value().second};
    out.insert(key, it.key());
  }

  return out;
}

/**
 * @brief Returns the (widgetType, relativeIndex) identity for the given live windowId, or an
 *        invalid key when the windowId is unknown.
 */
[[nodiscard]] static StableKey stableKeyForWindowId(const UI::Dashboard& dashboard, int windowId)
{
  const auto& widgetMap = dashboard.widgetMap();
  const auto it         = widgetMap.constFind(windowId);
  if (it == widgetMap.cend())
    return {};

  return {static_cast<int>(it.value().first), it.value().second};
}

/**
 * @brief Reads the stable key from a serialized geometry entry. Returns an invalid key for legacy
 *        entries that only carry the windowId.
 */
[[nodiscard]] static StableKey readStableKey(const QJsonObject& entry)
{
  if (!entry.contains("widgetType") || !entry.contains("relativeIndex"))
    return {};

  return {entry["widgetType"].toInt(-1), entry["relativeIndex"].toInt(-1)};
}

/**
 * @brief Resolves a serialized geometry/order entry to the current session's windowId, preferring
 *        the stable key over the legacy integer.
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

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an empty store bound to the dashboard whose widget map resolves stable keys.
 */
UI::WindowLayoutStore::WindowLayoutStore(UI::Dashboard& dashboard)
  : m_dashboard(dashboard), m_manualCanvasWidth(0), m_manualCanvasHeight(0)
{}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the canvas size the stored manual reference was measured on; a zero extent means
 *        no reference has been captured yet.
 */
QSize UI::WindowLayoutStore::referenceCanvas() const
{
  return QSize(m_manualCanvasWidth, m_manualCanvasHeight);
}

//--------------------------------------------------------------------------------------------------
// Serialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes the current window layout to a JSON object.
 */
QJsonObject UI::WindowLayoutStore::serialize(const QVector<int>& order,
                                             const QMap<int, QQuickItem*>& windows,
                                             const QSize& canvas,
                                             const bool autoLayout,
                                             const bool userReordered) const
{
  QJsonObject layout;
  QJsonArray geometries;
  for (int id : order) {
    auto* win = windows.value(id);
    if (!win)
      continue;

    const StableKey key = stableKeyForWindowId(m_dashboard, id);

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

  layout["geometries"]   = geometries;
  layout["canvasWidth"]  = canvas.width();
  layout["canvasHeight"] = canvas.height();

  QJsonArray orderArray;
  QJsonArray orderTypedArray;
  for (int id : order) {
    orderArray.append(id);

    const StableKey key = stableKeyForWindowId(m_dashboard, id);
    if (!key.isValid())
      continue;

    QJsonObject entry;
    entry["widgetType"]    = key.widgetType;
    entry["relativeIndex"] = key.relativeIndex;
    orderTypedArray.append(entry);
  }

  layout["windowOrder"]      = orderArray;
  layout["windowOrderTyped"] = orderTypedArray;
  layout["autoLayout"]       = autoLayout;
  layout["userReordered"]    = userReordered;

  return layout;
}

/**
 * @brief Reads the per-window chrome state ("normal"/"minimized"/"closed") stored with the layout,
 *        resolved to this session's window ids. The window manager only tiles what is normal, so
 *        the taskbar owns applying these back onto its model.
 */
QMap<int, QString> UI::WindowLayoutStore::savedWindowStates(
  const QJsonObject& layout, const QMap<int, QQuickItem*>& windows) const
{
  QMap<int, QString> states;
  if (layout.isEmpty() || !layout.contains("geometries"))
    return states;

  const QHash<StableKey, int> stableLookup = buildStableKeyToWindowId(m_dashboard);
  const QJsonArray geometries              = layout["geometries"].toArray();

  for (const auto& val : std::as_const(geometries)) {
    const QJsonObject winGeom = val.toObject();
    const int id              = resolveSavedWindowId(winGeom, stableLookup, windows);
    const QString state       = winGeom.value("state").toString();
    if (id < 0 || state.isEmpty())
      continue;

    states.insert(id, state);
  }

  return states;
}

/**
 * @brief Resolves a saved window-order array into live windowIds, appending whatever the saved
 *        order does not mention in its current relative order.
 */
QVector<int> UI::WindowLayoutStore::resolveSavedOrder(const QJsonObject& layout,
                                                      const QMap<int, QQuickItem*>& windows,
                                                      const QVector<int>& currentOrder) const
{
  const QHash<StableKey, int> stableLookup = buildStableKeyToWindowId(m_dashboard);

  QVector<int> newOrder;
  QSet<int> seen;
  const bool typed     = layout.contains("windowOrderTyped");
  const QJsonArray src = (typed ? layout["windowOrderTyped"] : layout["windowOrder"]).toArray();

  for (const auto& val : std::as_const(src)) {
    int id = -1;
    if (typed)
      id = resolveSavedWindowId(val.toObject(), stableLookup, windows);
    else
      id = val.toInt(-1);

    if (id < 0 || seen.contains(id) || !windows.contains(id))
      continue;

    newOrder.append(id);
    seen.insert(id);
  }

  for (int id : currentOrder) {
    if (!seen.contains(id)) {
      newOrder.append(id);
      seen.insert(id);
    }
  }

  return newOrder;
}

//--------------------------------------------------------------------------------------------------
// Reference lifetime
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drops the whole reference: stored rectangles, pending rectangles and reference canvas.
 */
void UI::WindowLayoutStore::clear()
{
  m_manualCanvasWidth  = 0;
  m_manualCanvasHeight = 0;
  m_manualGeometries.clear();
  m_pendingGeometries.clear();
}

/**
 * @brief Drops the stored and pending rectangles, keeping the reference canvas.
 */
void UI::WindowLayoutStore::clearGeometries()
{
  m_manualGeometries.clear();
  m_pendingGeometries.clear();
}

/**
 * @brief Drops the manual reference (rectangles plus canvas) while leaving geometries that have
 *        not been claimed by a window yet in place.
 */
void UI::WindowLayoutStore::clearManualReference()
{
  m_manualCanvasWidth  = 0;
  m_manualCanvasHeight = 0;
  m_manualGeometries.clear();
}

/**
 * @brief Applies and consumes the geometry stashed for @a id, if any, so a window can be placed
 *        before its first paint instead of flashing at its minimum size.
 */
void UI::WindowLayoutStore::applyPendingGeometry(const int id, QQuickItem* item)
{
  SS_ASSERT(item != nullptr, return);

  const auto pending = m_pendingGeometries.constFind(id);
  if (pending == m_pendingGeometries.cend())
    return;

  WindowGeometry::placeWindow(item, pending.value());
  m_pendingGeometries.remove(id);
}

//--------------------------------------------------------------------------------------------------
// Manual reference capture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores one window's geometry as the reference the manual layout is re-derived from.
 */
void UI::WindowLayoutStore::storeManualGeometry(const int id, QQuickItem* item, const QSize& canvas)
{
  if (!item)
    return;

  m_manualGeometries.insert(id, WindowGeometry::extractGeometry(item));
  if (canvas.width() <= 0 || canvas.height() <= 0)
    return;

  m_manualCanvasWidth  = canvas.width();
  m_manualCanvasHeight = canvas.height();
}

/**
 * @brief Snapshots every normal window as the new manual reference. The reference canvas size is
 *        shared, so storing one window alone would leave the rest read against a size they were
 *        never laid out on - which is how editing one widget used to move the others.
 */
void UI::WindowLayoutStore::storeManualLayout(const QMap<int, QQuickItem*>& windows,
                                              const QSize& canvas)
{
  for (auto it = windows.constBegin(); it != windows.constEnd(); ++it)
    if (it.value() && it.value()->state() == QStringLiteral("normal"))
      storeManualGeometry(it.key(), it.value(), canvas);
}

//--------------------------------------------------------------------------------------------------
// Restore
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pre-stashes saved geometries so registerWindow can apply them per-window before first
 *        paint, avoiding the minimum-size flash.
 */
void UI::WindowLayoutStore::preload(const QJsonObject& layout,
                                    const QMap<int, QQuickItem*>& windows,
                                    const QSize& canvas,
                                    const int spacing)
{
  clearGeometries();
  if (layout.isEmpty() || !layout.contains("geometries"))
    return;

  if (layout["autoLayout"].toBool(true))
    return;

  const int savedCanvasW  = layout["canvasWidth"].toInt(0);
  const int savedCanvasH  = layout["canvasHeight"].toInt(0);
  const int marginCanvasW = savedCanvasW > 0 ? savedCanvasW : canvas.width();
  const int marginCanvasH = savedCanvasH > 0 ? savedCanvasH : canvas.height();
  m_manualCanvasWidth     = marginCanvasW;
  m_manualCanvasHeight    = marginCanvasH;

  const QHash<StableKey, int> stableLookup = buildStableKeyToWindowId(m_dashboard);
  const auto saved                         = parseSavedGeometries(layout, stableLookup, windows);
  if (saved.isEmpty())
    return;

  QVector<QRect> reference;
  reference.reserve(saved.size());
  for (const auto& entry : std::as_const(saved))
    reference.append(entry.second);

  const auto placed =
    Layouts::rescaleManual(reference, QSize(marginCanvasW, marginCanvasH), canvas, spacing);

  for (int i = 0; i < saved.size(); ++i) {
    m_manualGeometries.insert(saved[i].first, saved[i].second);
    m_pendingGeometries.insert(saved[i].first, placed[i]);
  }
}

/**
 * @brief Applies saved manual-mode geometries to live windows and stashes the rescaled rects for
 *        any window that hasn't registered yet. The saved rects stay the reference the layout is
 *        re-derived from; only the placement follows the current canvas.
 */
void UI::WindowLayoutStore::applySavedGeometries(const QJsonObject& layout,
                                                 const QMap<int, QQuickItem*>& windows,
                                                 const QSize& savedCanvas,
                                                 const QSize& canvas,
                                                 const int spacing)
{
  m_manualCanvasWidth  = savedCanvas.width();
  m_manualCanvasHeight = savedCanvas.height();

  const QHash<StableKey, int> stableLookup = buildStableKeyToWindowId(m_dashboard);
  const auto saved                         = parseSavedGeometries(layout, stableLookup, windows);
  if (saved.isEmpty())
    return;

  QVector<QRect> reference;
  reference.reserve(saved.size());
  for (const auto& entry : std::as_const(saved))
    reference.append(entry.second);

  const auto placed = Layouts::rescaleManual(reference, savedCanvas, canvas, spacing);

  for (int i = 0; i < saved.size(); ++i) {
    if (auto* win = windows.value(saved[i].first))
      WindowGeometry::placeWindow(win, placed[i]);

    m_manualGeometries.insert(saved[i].first, saved[i].second);
    m_pendingGeometries.insert(saved[i].first, placed[i]);
  }
}

/**
 * @brief Lays the stored manual reference out on a canvas of the given size, holding every join at
 *        the configured spacing and every outer edge flush. Reads the reference and never its own
 *        output, so repeated resizes cannot accumulate drift.
 */
void UI::WindowLayoutStore::applyManualLayout(const QMap<int, QQuickItem*>& windows,
                                              const QSize& canvas,
                                              const QSize& lastCanvas,
                                              const int spacing)
{
  if (canvas.width() <= 0 || canvas.height() <= 0)
    return;

  int refWidth  = m_manualCanvasWidth > 0 ? m_manualCanvasWidth : lastCanvas.width();
  int refHeight = m_manualCanvasHeight > 0 ? m_manualCanvasHeight : lastCanvas.height();
  refWidth      = refWidth > 0 ? refWidth : canvas.width();
  refHeight     = refHeight > 0 ? refHeight : canvas.height();

  m_manualCanvasWidth  = refWidth;
  m_manualCanvasHeight = refHeight;

  QVector<int> ids;
  QVector<QRect> reference;
  for (auto it = windows.constBegin(); it != windows.constEnd(); ++it) {
    auto* win = it.value();
    if (!win || win->state() != QStringLiteral("normal"))
      continue;

    if (!m_manualGeometries.contains(it.key()))
      m_manualGeometries.insert(it.key(), WindowGeometry::extractGeometry(win));

    ids.append(it.key());
    reference.append(m_manualGeometries.value(it.key()));
  }

  if (ids.isEmpty())
    return;

  const auto placed =
    Layouts::rescaleManual(reference, QSize(refWidth, refHeight), canvas, spacing);

  for (int i = 0; i < ids.size(); ++i) {
    auto* win = windows.value(ids[i]);
    if (!win)
      continue;

    WindowGeometry::placeWindow(win,
                                QRect(placed[i].x(),
                                      placed[i].y(),
                                      qMax(kLayoutManualMinSize, placed[i].width()),
                                      qMax(kLayoutManualMinSize, placed[i].height())));
  }
}
