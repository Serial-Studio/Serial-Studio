/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <QPoint>
#include <QRect>
#include <QSize>
#include <QTest>

#include "UI/WindowManager/WindowGeometry.h"

using ResizeEdge = UI::WindowGeometry::ResizeEdge;

Q_DECLARE_METATYPE(ResizeEdge)

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Pins the stateless window-geometry math the dashboard's manual layout mode runs on:
 *        edge detection, resize deltas, canvas fitting, shared-border masks and the cascade.
 */
class TstWindowGeometry : public QObject {
  Q_OBJECT

private slots:
  void edgeAtLocalPos_data();
  void edgeAtLocalPos();

  void cursorForEdge_data();
  void cursorForEdge();

  void movingEdgesFor_data();
  void movingEdgesFor();

  void computeResizedGeometry_data();
  void computeResizedGeometry();

  void constrainGeometry_data();
  void constrainGeometry();

  void mergedEdgeMasks_sideBySide();
  void mergedEdgeMasks_stacked();
  void mergedEdgeMasks_apart();

  void cascadeGeometry_steps();

  void clampResizeToCanvas_insideIsUntouched();
  void clampResizeToCanvas_appliesTheClampedRect();
};

//--------------------------------------------------------------------------------------------------
// edgeAtLocalPos
//--------------------------------------------------------------------------------------------------

void TstWindowGeometry::edgeAtLocalPos_data()
{
  QTest::addColumn<QPointF>("local");
  QTest::addColumn<ResizeEdge>("expected");

  QTest::newRow("origin is the top-left corner") << QPointF(0, 0) << ResizeEdge::TopLeft;
  QTest::newRow("inside the top-left band") << QPointF(4, 4) << ResizeEdge::TopLeft;
  QTest::newRow("corner band is inclusive") << QPointF(8, 8) << ResizeEdge::TopLeft;
  QTest::newRow("one pixel past the corner") << QPointF(9, 9) << ResizeEdge::None;
  QTest::newRow("top edge") << QPointF(50, 4) << ResizeEdge::Top;
  QTest::newRow("top-right corner") << QPointF(96, 4) << ResizeEdge::TopRight;
  QTest::newRow("right edge") << QPointF(96, 40) << ResizeEdge::Right;
  QTest::newRow("bottom-right corner") << QPointF(96, 76) << ResizeEdge::BottomRight;
  QTest::newRow("bottom-right band is inclusive") << QPointF(92, 72) << ResizeEdge::BottomRight;
  QTest::newRow("one pixel inside the bottom-right") << QPointF(91, 71) << ResizeEdge::None;
  QTest::newRow("bottom edge") << QPointF(50, 76) << ResizeEdge::Bottom;
  QTest::newRow("bottom-left corner") << QPointF(4, 76) << ResizeEdge::BottomLeft;
  QTest::newRow("left edge") << QPointF(4, 40) << ResizeEdge::Left;
  QTest::newRow("dead zone in the middle") << QPointF(50, 40) << ResizeEdge::None;
}

/**
 * @brief Sweeps a 100x80 window with the shipping 8px margin: corners must beat edges, and the
 *        body must resolve to no edge at all so a press there drags instead of resizing.
 */
void TstWindowGeometry::edgeAtLocalPos()
{
  QFETCH(QPointF, local);
  QFETCH(ResizeEdge, expected);

  QCOMPARE(UI::WindowGeometry::edgeAtLocalPos(local, 100, 80, 8), expected);
}

//--------------------------------------------------------------------------------------------------
// cursorForEdge
//--------------------------------------------------------------------------------------------------

void TstWindowGeometry::cursorForEdge_data()
{
  QTest::addColumn<ResizeEdge>("edge");
  QTest::addColumn<int>("expected");

  QTest::newRow("left is horizontal") << ResizeEdge::Left << int(Qt::SizeHorCursor);
  QTest::newRow("right is horizontal") << ResizeEdge::Right << int(Qt::SizeHorCursor);
  QTest::newRow("top is vertical") << ResizeEdge::Top << int(Qt::SizeVerCursor);
  QTest::newRow("bottom is vertical") << ResizeEdge::Bottom << int(Qt::SizeVerCursor);
  QTest::newRow("top-right is back-diagonal") << ResizeEdge::TopRight << int(Qt::SizeBDiagCursor);
  QTest::newRow("bottom-left is back-diagonal")
    << ResizeEdge::BottomLeft << int(Qt::SizeBDiagCursor);
  QTest::newRow("top-left is forward-diagonal") << ResizeEdge::TopLeft << int(Qt::SizeFDiagCursor);
  QTest::newRow("bottom-right is forward-diagonal")
    << ResizeEdge::BottomRight << int(Qt::SizeFDiagCursor);
  QTest::newRow("no edge falls back to the arrow") << ResizeEdge::None << int(Qt::ArrowCursor);
}

/**
 * @brief The arrow shape is the sentinel the window manager reads as "unset the cursor", so it
 *        must be reachable only from ResizeEdge::None.
 */
void TstWindowGeometry::cursorForEdge()
{
  QFETCH(ResizeEdge, edge);
  QFETCH(int, expected);

  QCOMPARE(int(UI::WindowGeometry::cursorForEdge(edge)), expected);
}

//--------------------------------------------------------------------------------------------------
// movingEdgesFor
//--------------------------------------------------------------------------------------------------

void TstWindowGeometry::movingEdgesFor_data()
{
  QTest::addColumn<ResizeEdge>("edge");
  QTest::addColumn<bool>("left");
  QTest::addColumn<bool>("right");
  QTest::addColumn<bool>("top");
  QTest::addColumn<bool>("bottom");

  QTest::newRow("none moves nothing") << ResizeEdge::None << false << false << false << false;
  QTest::newRow("left") << ResizeEdge::Left << true << false << false << false;
  QTest::newRow("right") << ResizeEdge::Right << false << true << false << false;
  QTest::newRow("top") << ResizeEdge::Top << false << false << true << false;
  QTest::newRow("bottom") << ResizeEdge::Bottom << false << false << false << true;
  QTest::newRow("top-left") << ResizeEdge::TopLeft << true << false << true << false;
  QTest::newRow("top-right") << ResizeEdge::TopRight << false << true << true << false;
  QTest::newRow("bottom-left") << ResizeEdge::BottomLeft << true << false << false << true;
  QTest::newRow("bottom-right") << ResizeEdge::BottomRight << false << true << false << true;
}

/**
 * @brief The snap resolver only considers the edges a gesture actually moves; a corner moves one
 *        edge per axis and never its opposite.
 */
void TstWindowGeometry::movingEdgesFor()
{
  QFETCH(ResizeEdge, edge);
  QFETCH(bool, left);
  QFETCH(bool, right);
  QFETCH(bool, top);
  QFETCH(bool, bottom);

  const auto edges = UI::WindowGeometry::movingEdgesFor(edge);
  QCOMPARE(edges.left, left);
  QCOMPARE(edges.right, right);
  QCOMPARE(edges.top, top);
  QCOMPARE(edges.bottom, bottom);
}

//--------------------------------------------------------------------------------------------------
// computeResizedGeometry
//--------------------------------------------------------------------------------------------------

void TstWindowGeometry::computeResizedGeometry_data()
{
  QTest::addColumn<ResizeEdge>("edge");
  QTest::addColumn<QPoint>("delta");
  QTest::addColumn<QRect>("expected");

  QTest::newRow("no edge ignores the delta")
    << ResizeEdge::None << QPoint(40, 40) << QRect(10, 20, 100, 80);
  QTest::newRow("right grows the width")
    << ResizeEdge::Right << QPoint(30, 0) << QRect(10, 20, 130, 80);
  QTest::newRow("right clamps at the minimum width")
    << ResizeEdge::Right << QPoint(-90, 0) << QRect(10, 20, 40, 80);
  QTest::newRow("bottom grows the height")
    << ResizeEdge::Bottom << QPoint(0, 25) << QRect(10, 20, 100, 105);
  QTest::newRow("bottom clamps at the minimum height")
    << ResizeEdge::Bottom << QPoint(0, -70) << QRect(10, 20, 100, 30);
  QTest::newRow("left grows leftwards")
    << ResizeEdge::Left << QPoint(-15, 0) << QRect(-6, 20, 115, 80);
  QTest::newRow("left clamps and pins the right border")
    << ResizeEdge::Left << QPoint(80, 0) << QRect(69, 20, 40, 80);
  QTest::newRow("top grows upwards") << ResizeEdge::Top << QPoint(0, -10) << QRect(10, 9, 100, 90);
  QTest::newRow("top-left grows both axes")
    << ResizeEdge::TopLeft << QPoint(-10, -10) << QRect(-1, 9, 110, 90);
  QTest::newRow("top-right grows both axes")
    << ResizeEdge::TopRight << QPoint(10, -10) << QRect(10, 9, 110, 90);
  QTest::newRow("bottom-left grows both axes")
    << ResizeEdge::BottomLeft << QPoint(-10, 10) << QRect(-1, 20, 110, 90);
  QTest::newRow("bottom-right grows both axes")
    << ResizeEdge::BottomRight << QPoint(10, 10) << QRect(10, 20, 110, 90);
}

/**
 * @brief Known answers for a 100x80 window at (10,20) with a 40x30 floor. An edge that would
 *        shrink the window past the floor stops growing instead of dragging the opposite border
 *        along, which is what keeps a clamped resize from walking the window across the canvas.
 */
void TstWindowGeometry::computeResizedGeometry()
{
  QFETCH(ResizeEdge, edge);
  QFETCH(QPoint, delta);
  QFETCH(QRect, expected);

  const QRect initial(10, 20, 100, 80);
  QCOMPARE(UI::WindowGeometry::computeResizedGeometry(initial, delta, edge, 40, 30), expected);
}

//--------------------------------------------------------------------------------------------------
// constrainGeometry
//--------------------------------------------------------------------------------------------------

void TstWindowGeometry::constrainGeometry_data()
{
  QTest::addColumn<QRect>("geometry");
  QTest::addColumn<QRect>("expected");

  QTest::newRow("a window that already fits is untouched")
    << QRect(10, 10, 50, 50) << QRect(10, 10, 50, 50);
  QTest::newRow("width beyond the canvas is capped and pulled back")
    << QRect(10, 10, 300, 50) << QRect(0, 10, 200, 50);
  QTest::newRow("height beyond the canvas is capped and pulled back")
    << QRect(10, 10, 50, 300) << QRect(10, 0, 50, 100);
  QTest::newRow("negative origin is pushed onto the canvas")
    << QRect(-5, -5, 50, 50) << QRect(0, 0, 50, 50);
  QTest::newRow("a window below the floor grows to it")
    << QRect(10, 10, 20, 20) << QRect(10, 10, 40, 30);
  QTest::newRow("a window past the far edge slides back")
    << QRect(180, 90, 50, 50) << QRect(150, 50, 50, 50);
}

/**
 * @brief Fits a window into a 200x100 canvas with a 40x30 floor. The canvas always wins: the
 *        floor is only applied while the canvas can still hold it.
 */
void TstWindowGeometry::constrainGeometry()
{
  QFETCH(QRect, geometry);
  QFETCH(QRect, expected);

  QCOMPARE(UI::WindowGeometry::constrainGeometry(geometry, QSize(200, 100), 40, 30), expected);
}

//--------------------------------------------------------------------------------------------------
// mergedEdgeMasks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Two windows sharing a vertical seam: the left one reports a merged right edge (2) and
 *        the right one a merged left edge (1).
 */
void TstWindowGeometry::mergedEdgeMasks_sideBySide()
{
  const QVector<int> ids{7, 9};
  const QVector<QRect> rects{QRect(0, 0, 100, 100), QRect(100, 0, 100, 100)};

  const auto masks = UI::WindowGeometry::mergedEdgeMasks(ids, rects);
  QCOMPARE(masks.value(QStringLiteral("7")).toInt(), 2);
  QCOMPARE(masks.value(QStringLiteral("9")).toInt(), 1);
}

/**
 * @brief Two windows sharing a horizontal seam: the top one reports a merged bottom edge (8) and
 *        the bottom one a merged top edge (4).
 */
void TstWindowGeometry::mergedEdgeMasks_stacked()
{
  const QVector<int> ids{1, 2};
  const QVector<QRect> rects{QRect(0, 0, 100, 100), QRect(0, 100, 100, 100)};

  const auto masks = UI::WindowGeometry::mergedEdgeMasks(ids, rects);
  QCOMPARE(masks.value(QStringLiteral("1")).toInt(), 8);
  QCOMPARE(masks.value(QStringLiteral("2")).toInt(), 4);
}

/**
 * @brief Windows further apart than the one-pixel seam tolerance share no border at all.
 */
void TstWindowGeometry::mergedEdgeMasks_apart()
{
  const QVector<int> ids{1, 2};
  const QVector<QRect> rects{QRect(0, 0, 100, 100), QRect(140, 0, 100, 100)};

  const auto masks = UI::WindowGeometry::mergedEdgeMasks(ids, rects);
  QCOMPARE(masks.value(QStringLiteral("1")).toInt(), 0);
  QCOMPARE(masks.value(QStringLiteral("2")).toInt(), 0);
}

//--------------------------------------------------------------------------------------------------
// cascadeGeometry
//--------------------------------------------------------------------------------------------------

/**
 * @brief The first cascade tile is centered and every following one steps down-right by a fixed
 *        offset while keeping the same size.
 */
void TstWindowGeometry::cascadeGeometry_steps()
{
  const QSize canvas(1000, 800);
  const QSize minSize(200, 150);

  const QRect first  = UI::WindowGeometry::cascadeGeometry(0, minSize, canvas);
  const QRect second = UI::WindowGeometry::cascadeGeometry(1, minSize, canvas);

  QCOMPARE(first, QRect(229, 165, 541, 470));
  QCOMPARE(second, QRect(255, 191, 541, 470));
  QCOMPARE(second.size(), first.size());
}

//--------------------------------------------------------------------------------------------------
// clampResizeToCanvas (spec 0075, F19)
//--------------------------------------------------------------------------------------------------

/**
 * @brief A candidate already inside the canvas comes back unchanged.
 */
void TstWindowGeometry::clampResizeToCanvas_insideIsUntouched()
{
  const QRect inside(10, 20, 300, 200);
  QCOMPARE(UI::WindowGeometry::clampResizeToCanvas(inside, QSize(1000, 800)), inside);
}

/**
 * @brief A candidate that runs past an edge comes back clamped, not rejected: the gesture used
 *        to be discarded whole, so the window stopped following the pointer at the canvas edge.
 */
void TstWindowGeometry::clampResizeToCanvas_appliesTheClampedRect()
{
  const QSize canvas(1000, 800);

  const QRect right = UI::WindowGeometry::clampResizeToCanvas(QRect(900, 10, 300, 100), canvas);
  QCOMPARE(right, QRect(900, 10, 100, 100));

  const QRect bottom = UI::WindowGeometry::clampResizeToCanvas(QRect(10, 700, 100, 300), canvas);
  QCOMPARE(bottom, QRect(10, 700, 100, 100));

  const QRect origin = UI::WindowGeometry::clampResizeToCanvas(QRect(-40, -30, 200, 150), canvas);
  QCOMPARE(origin.x(), 0);
  QCOMPARE(origin.y(), 0);
}

QTEST_APPLESS_MAIN(TstWindowGeometry)

#include "tst_window_geometry.moc"
