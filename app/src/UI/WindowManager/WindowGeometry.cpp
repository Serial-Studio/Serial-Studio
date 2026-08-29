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

#include "UI/WindowManager/WindowGeometry.h"

#include <algorithm>
#include <QPair>
#include <QQuickItem>
#include <QString>

#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Window state helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Extracts a window's actual geometry, falling back to its implicit size while the item
 *        has not been laid out yet.
 */
QRect UI::WindowGeometry::extractGeometry(const QQuickItem* item)
{
  SS_ASSERT(item != nullptr, return QRect());

  const qreal w = item->width() > 0 ? item->width() : item->implicitWidth();
  const qreal h = item->height() > 0 ? item->height() : item->implicitHeight();
  return QRect(static_cast<int>(item->x()),
               static_cast<int>(item->y()),
               static_cast<int>(w),
               static_cast<int>(h));
}

/**
 * @brief Shrinks a snap rectangle when its bottom reaches the canvas floor, keeping the border
 *        clear of the taskbar (the canvas overlaps it by one pixel).
 */
QRect UI::WindowGeometry::liftSnapBottom(QRect rect, const int canvasHeight)
{
  if (rect.y() + rect.height() >= canvasHeight)
    rect.setHeight(rect.height() - 2);

  return rect;
}

/**
 * @brief Returns true if any tracked window is currently in the maximized state.
 */
bool UI::WindowGeometry::anyWindowMaximized(const QMap<int, QQuickItem*>& windows)
{
  for (auto* win : std::as_const(windows))
    if (win && win->state() == QStringLiteral("maximized"))
      return true;

  return false;
}

/**
 * @brief Returns true when a window that should be on screen has never been shown, which is what
 *        marks a layout pass as running before the windows finished initializing.
 */
bool UI::WindowGeometry::hasRestorableHiddenWindow(const QMap<int, QQuickItem*>& windows)
{
  for (auto* win : std::as_const(windows)) {
    if (!win || win->isVisible())
      continue;

    if (win->state() == QStringLiteral("normal") || win->state() == QStringLiteral("maximized"))
      return true;
  }

  return false;
}

/**
 * @brief Retrieves the id associated with a registered window item, or -1 when unknown.
 */
int UI::WindowGeometry::idForWindow(const QMap<int, QQuickItem*>& windows, const QQuickItem* item)
{
  for (auto it = windows.constBegin(); it != windows.constEnd(); ++it)
    if (it.value() == item)
      return it.key();

  return -1;
}

/**
 * @brief Returns the windows in @a order that are registered, normal and therefore tileable.
 */
QList<QQuickItem*> UI::WindowGeometry::normalWindowsInOrder(const QMap<int, QQuickItem*>& windows,
                                                            const QVector<int>& order)
{
  QList<QQuickItem*> out;
  out.reserve(order.size());
  for (int id : order) {
    auto* win = windows.value(id);
    if (win && win->state() == QStringLiteral("normal"))
      out.append(win);
  }

  return out;
}

/**
 * @brief Sets a window's geometry to the given rectangle.
 */
void UI::WindowGeometry::placeWindow(QQuickItem* window, const QRect& geometry)
{
  SS_ASSERT(window != nullptr, return);

  window->setX(geometry.x());
  window->setY(geometry.y());
  window->setWidth(geometry.width());
  window->setHeight(geometry.height());
}

/**
 * @brief Shows every window that a layout pass has just placed but that has never been made
 *        visible; a minimized or closed window keeps its state.
 */
void UI::WindowGeometry::showRestorableWindows(const QMap<int, QQuickItem*>& windows)
{
  for (auto* win : std::as_const(windows)) {
    if (!win || win->isVisible())
      continue;

    if (win->state() == QStringLiteral("normal") || win->state() == QStringLiteral("maximized"))
      win->setVisible(true);
  }
}

//--------------------------------------------------------------------------------------------------
// Resize edge detection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Classifies a window-local point into the edge or corner it would resize. Corners win
 *        over edges, and a point further than @a margin from every border resizes nothing.
 */
UI::WindowGeometry::ResizeEdge UI::WindowGeometry::edgeAtLocalPos(const QPointF& local,
                                                                  const int width,
                                                                  const int height,
                                                                  const int margin)
{
  SS_ASSERT(margin >= 0, return ResizeEdge::None);

  const int x = static_cast<int>(local.x());
  const int y = static_cast<int>(local.y());

  const bool nearLeft   = x <= margin;
  const bool nearRight  = x >= width - margin;
  const bool nearTop    = y <= margin;
  const bool nearBottom = y >= height - margin;

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
 * @brief Determines which edge or corner of @a target the canvas-local point @a pos hovers;
 *        only a normal (non-maximized, non-minimized) window is resizable.
 */
UI::WindowGeometry::ResizeEdge UI::WindowGeometry::detectResizeEdge(QQuickItem* target,
                                                                    const QQuickItem* origin,
                                                                    const QPointF& pos)
{
  SS_ASSERT(target != nullptr, return ResizeEdge::None);
  SS_ASSERT(origin != nullptr, return ResizeEdge::None);

  if (target->state() != QStringLiteral("normal"))
    return ResizeEdge::None;

  const QPointF local = target->mapFromItem(origin, pos);
  return edgeAtLocalPos(
    local, static_cast<int>(target->width()), static_cast<int>(target->height()), kResizeMargin);
}

/**
 * @brief Maps a resize edge to the cursor that advertises it; Qt::ArrowCursor means the pointer
 *        is over no edge at all and the caller should unset its cursor.
 */
Qt::CursorShape UI::WindowGeometry::cursorForEdge(const ResizeEdge edge)
{
  switch (edge) {
    case ResizeEdge::Left:
    case ResizeEdge::Right:
      return Qt::SizeHorCursor;
    case ResizeEdge::Top:
    case ResizeEdge::Bottom:
      return Qt::SizeVerCursor;
    case ResizeEdge::TopRight:
    case ResizeEdge::BottomLeft:
      return Qt::SizeBDiagCursor;
    case ResizeEdge::TopLeft:
    case ResizeEdge::BottomRight:
      return Qt::SizeFDiagCursor;
    default:
      return Qt::ArrowCursor;
  }
}

/**
 * @brief Maps a resize edge to the axis edges the gesture is moving.
 */
UI::Snap::MovingEdges UI::WindowGeometry::movingEdgesFor(const ResizeEdge edge)
{
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

//--------------------------------------------------------------------------------------------------
// Geometry math
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies a mouse delta to the geometry a gesture started from. An edge that shrinks the
 *        window past its minimum pins the opposite border instead of moving it.
 */
QRect UI::WindowGeometry::computeResizedGeometry(const QRect& initial,
                                                 const QPoint& delta,
                                                 const ResizeEdge edge,
                                                 const int minWidth,
                                                 const int minHeight)
{
  SS_ASSERT(minWidth >= 0, return initial);
  SS_ASSERT(minHeight >= 0, return initial);

  QRect geometry = initial;
  switch (edge) {
    case ResizeEdge::Right:
      geometry.setWidth(qMax(minWidth, initial.width() + delta.x()));
      return geometry;
    case ResizeEdge::Bottom:
      geometry.setHeight(qMax(minHeight, initial.height() + delta.y()));
      return geometry;
    case ResizeEdge::Left: {
      const int w = qMax(minWidth, initial.width() - delta.x());
      geometry.setX(initial.right() - w);
      geometry.setWidth(w);
      return geometry;
    }
    case ResizeEdge::Top: {
      const int h = qMax(minHeight, initial.height() - delta.y());
      geometry.setY(initial.bottom() - h);
      geometry.setHeight(h);
      return geometry;
    }
    case ResizeEdge::TopLeft: {
      const int w = qMax(minWidth, initial.width() - delta.x());
      const int h = qMax(minHeight, initial.height() - delta.y());
      geometry.setX(initial.right() - w);
      geometry.setWidth(w);
      geometry.setY(initial.bottom() - h);
      geometry.setHeight(h);
      return geometry;
    }
    case ResizeEdge::TopRight: {
      const int w = qMax(minWidth, initial.width() + delta.x());
      const int h = qMax(minHeight, initial.height() - delta.y());
      geometry.setY(initial.bottom() - h);
      geometry.setHeight(h);
      geometry.setWidth(w);
      return geometry;
    }
    case ResizeEdge::BottomLeft: {
      const int w = qMax(minWidth, initial.width() - delta.x());
      const int h = qMax(minHeight, initial.height() + delta.y());
      geometry.setX(initial.right() - w);
      geometry.setWidth(w);
      geometry.setHeight(h);
      return geometry;
    }
    case ResizeEdge::BottomRight:
      geometry.setWidth(qMax(minWidth, initial.width() + delta.x()));
      geometry.setHeight(qMax(minHeight, initial.height() + delta.y()));
      return geometry;
    case ResizeEdge::None:
      return geometry;
  }

  return geometry;
}

/**
 * @brief Fits one window inside the canvas: the size floor loses to the canvas, the canvas wins
 *        over the position, and a rect equal to the input means nothing had to move.
 */
QRect UI::WindowGeometry::constrainGeometry(const QRect& geometry,
                                            const QSize& canvas,
                                            const int minWidth,
                                            const int minHeight)
{
  SS_ASSERT(canvas.width() > 0, return geometry);
  SS_ASSERT(canvas.height() > 0, return geometry);

  int winX = geometry.x();
  int winY = geometry.y();
  int winW = geometry.width();
  int winH = geometry.height();

  winW = qMin(winW, canvas.width());
  winH = qMin(winH, canvas.height());

  if (winW < minWidth && canvas.width() >= minWidth)
    winW = minWidth;

  if (winH < minHeight && canvas.height() >= minHeight)
    winH = minHeight;

  winX = qMax(0, winX);
  winY = qMax(0, winY);

  if (winX + winW > canvas.width()) {
    winX = canvas.width() - winW;
    if (winX < 0) {
      winX = 0;
      winW = canvas.width();
    }
  }

  if (winY + winH > canvas.height()) {
    winY = canvas.height() - winH;
    if (winY < 0) {
      winY = 0;
      winH = canvas.height();
    }
  }

  return QRect(winX, winY, winW, winH);
}

/**
 * @brief Places the window at position @a index of a macOS-inspired cascade: every tile is
 *        centered, stepped down-right by its index, and wrapped back into the canvas once the
 *        step would push it past an edge. @a minSize is the floor the tile never shrinks below.
 */
QRect UI::WindowGeometry::cascadeGeometry(const int index,
                                          const QSize& minSize,
                                          const QSize& canvas)
{
  SS_ASSERT(index >= 0, return QRect());
  SS_ASSERT(canvas.isValid(), return QRect());

  constexpr int margin  = 8;
  constexpr int offsetX = 26;
  constexpr int offsetY = 26;

  const int availableW = canvas.width() - 2 * margin;
  const int availableH = canvas.height() - 2 * margin;

  int winW = qMin(qMax(minSize.width(), availableW * 55 / 100), availableW);
  int winH = qMin(qMax(minSize.height(), availableH * 60 / 100), availableH);

  const int baseX = margin + (availableW - winW) / 2;
  const int baseY = margin + (availableH - winH) / 2;

  int stepX     = index * offsetX;
  int stepY     = index * offsetY;
  int wrapCount = 0;
  while (baseY + stepY + winH > canvas.height() - margin && wrapCount < 10) {
    stepY -= (availableH - winH);
    stepX += offsetX * 2;
    wrapCount++;
  }

  while (baseX + stepX + winW > canvas.width() - margin && wrapCount < 20) {
    stepX -= (availableW - winW);
    wrapCount++;
  }

  int winX = qBound(margin, baseX + stepX, canvas.width() - winW - margin);
  int winY = qBound(margin, baseY + stepY, canvas.height() - winH - margin);

  if (winW > availableW) {
    winW = availableW;
    winX = margin;
  }

  if (winH > availableH) {
    winH = availableH;
    winY = margin;
  }

  return QRect(winX, winY, winW, winH);
}

/**
 * @brief Returns which edges of each window coincide with a sibling's opposite edge, as a
 *        per-window bitmask (1 left, 2 right, 4 top, 8 bottom) keyed by window id. Presentation
 *        only: QML draws a coincident pair as one border instead of two.
 */
QVariantMap UI::WindowGeometry::mergedEdgeMasks(const QVector<int>& ids,
                                                const QVector<QRect>& rects)
{
  SS_ASSERT(ids.size() == rects.size(), return QVariantMap());

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

  return merged;
}

//--------------------------------------------------------------------------------------------------
// Hit testing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Finds the window with the largest area overlap with @a dragRect, ignoring @a exclude.
 */
QQuickItem* UI::WindowGeometry::findOverlapTarget(const QMap<int, QQuickItem*>& windows,
                                                  const QRect& dragRect,
                                                  const QQuickItem* exclude)
{
  QQuickItem* best = nullptr;
  qint64 bestArea  = 0;

  for (auto it = windows.constBegin(); it != windows.constEnd(); ++it) {
    QQuickItem* win = it.value();
    if (!win || win == exclude || !win->isVisible())
      continue;

    if (win->state() != QStringLiteral("normal"))
      continue;

    const QRect inter = dragRect.intersected(QRect(static_cast<int>(win->x()),
                                                   static_cast<int>(win->y()),
                                                   static_cast<int>(win->width()),
                                                   static_cast<int>(win->height())));
    const qint64 area = static_cast<qint64>(inter.width()) * static_cast<qint64>(inter.height());
    if (area > bestArea) {
      bestArea = area;
      best     = win;
    }
  }

  return best;
}

/**
 * @brief Returns @a windows sorted topmost-first: higher z wins, then a smaller index in
 *        @a order breaks the tie so equal-z stacks stay deterministic.
 */
QVector<QQuickItem*> UI::WindowGeometry::sortedByVisualStacking(
  const QMap<int, QQuickItem*>& windows, const QVector<int>& order)
{
  QVector<QPair<int, QQuickItem*>> entries;
  entries.reserve(windows.size());
  for (auto it = windows.cbegin(); it != windows.cend(); ++it)
    entries.append({it.key(), it.value()});

  std::sort(entries.begin(),
            entries.end(),
            [&order](const QPair<int, QQuickItem*>& a, const QPair<int, QQuickItem*>& b) {
              if (!a.second || !b.second)
                return a.second != nullptr;

              if (a.second->z() != b.second->z())
                return a.second->z() > b.second->z();

              const int ai = order.indexOf(a.first);
              const int bi = order.indexOf(b.first);
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
 * @brief Returns the topmost visible normal/maximized window whose bounding rect contains
 *        @a pos, ignoring @a exclude.
 */
QQuickItem* UI::WindowGeometry::topmostWindowAt(const QMap<int, QQuickItem*>& windows,
                                                const QVector<int>& order,
                                                const QPointF& pos,
                                                const QQuickItem* exclude)
{
  for (QQuickItem* window : sortedByVisualStacking(windows, order)) {
    if (!window || !window->isVisible() || window == exclude)
      continue;

    const auto state = window->state();
    if (state != QStringLiteral("normal") && state != QStringLiteral("maximized"))
      continue;

    const QRectF bounds(window->x(), window->y(), window->width(), window->height());
    if (bounds.contains(pos))
      return window;
  }

  return nullptr;
}

/**
 * @brief Manual-mode press pre-pass: returns the topmost normal window containing @a pos whose
 *        edge would start a resize, so a top window's corner wins over a lower window's body.
 */
QQuickItem* UI::WindowGeometry::manualResizeTargetAt(const QMap<int, QQuickItem*>& windows,
                                                     const QVector<int>& order,
                                                     const QQuickItem* origin,
                                                     const QPointF& pos)
{
  for (QQuickItem* window : sortedByVisualStacking(windows, order)) {
    if (!window || !window->isVisible() || window->state() != QStringLiteral("normal"))
      continue;

    const QRectF bounds(window->x(), window->y(), window->width(), window->height());
    if (!bounds.contains(pos))
      continue;

    if (detectResizeEdge(window, origin, pos) != ResizeEdge::None)
      return window;
  }

  return nullptr;
}
