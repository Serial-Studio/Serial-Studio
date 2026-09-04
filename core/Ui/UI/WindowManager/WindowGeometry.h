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

#pragma once

#include <QList>
#include <QMap>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QVariantMap>
#include <QVector>

#include "UI/SnapGuides.h"

class QQuickItem;

namespace UI::WindowGeometry {

/**
 * @brief Pixel band around a window's border that starts a resize instead of a drag.
 */
constexpr int kResizeMargin = 8;

/**
 * @brief Edge gap still read as one seam by the shared-border merge (spacing -1 overlaps by a
 *        pixel).
 */
constexpr int kMergeTolerance = 1;

/**
 * @brief Perpendicular overlap two windows need before their edges are treated as adjacent.
 */
constexpr int kMergeOverlap = 2;

/**
 * @brief Which edge or corner of a window a manual gesture is moving.
 */
enum class ResizeEdge {
  None,
  Left,
  Right,
  Top,
  Bottom,
  TopLeft,
  TopRight,
  BottomLeft,
  BottomRight
};

[[nodiscard]] QRect extractGeometry(const QQuickItem* item);
[[nodiscard]] QRect liftSnapBottom(QRect rect, int canvasHeight);
[[nodiscard]] bool anyWindowMaximized(const QMap<int, QQuickItem*>& windows);
[[nodiscard]] bool hasRestorableHiddenWindow(const QMap<int, QQuickItem*>& windows);
[[nodiscard]] int idForWindow(const QMap<int, QQuickItem*>& windows, const QQuickItem* item);
[[nodiscard]] QList<QQuickItem*> normalWindowsInOrder(const QMap<int, QQuickItem*>& windows,
                                                      const QVector<int>& order);

void placeWindow(QQuickItem* window, const QRect& geometry);
void showRestorableWindows(const QMap<int, QQuickItem*>& windows);

[[nodiscard]] ResizeEdge edgeAtLocalPos(const QPointF& local, int width, int height, int margin);
[[nodiscard]] ResizeEdge detectResizeEdge(QQuickItem* target,
                                          const QQuickItem* origin,
                                          const QPointF& pos);

[[nodiscard]] Qt::CursorShape cursorForEdge(ResizeEdge edge);
[[nodiscard]] Snap::MovingEdges movingEdgesFor(ResizeEdge edge);

[[nodiscard]] QRect computeResizedGeometry(
  const QRect& initial, const QPoint& delta, ResizeEdge edge, int minWidth, int minHeight);
[[nodiscard]] QRect clampResizeToCanvas(QRect geometry, const QSize& canvas);
[[nodiscard]] QRect constrainGeometry(const QRect& geometry,
                                      const QSize& canvas,
                                      int minWidth,
                                      int minHeight);
[[nodiscard]] QRect cascadeGeometry(int index, const QSize& minSize, const QSize& canvas);

[[nodiscard]] QVariantMap mergedEdgeMasks(const QVector<int>& ids, const QVector<QRect>& rects);

[[nodiscard]] QQuickItem* findOverlapTarget(const QMap<int, QQuickItem*>& windows,
                                            const QRect& dragRect,
                                            const QQuickItem* exclude);
[[nodiscard]] QVector<QQuickItem*> sortedByVisualStacking(const QMap<int, QQuickItem*>& windows,
                                                          const QVector<int>& order);
[[nodiscard]] QQuickItem* topmostWindowAt(const QMap<int, QQuickItem*>& windows,
                                          const QVector<int>& order,
                                          const QPointF& pos,
                                          const QQuickItem* exclude);
[[nodiscard]] QQuickItem* manualResizeTargetAt(const QMap<int, QQuickItem*>& windows,
                                               const QVector<int>& order,
                                               const QQuickItem* origin,
                                               const QPointF& pos);

}  // namespace UI::WindowGeometry
