/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QtTest>

#include "UI/Widgets/Painter/PainterGeometry.h"

using namespace Widgets::PainterGeometry;

//--------------------------------------------------------------------------------------------------
// Canvas2D path constructors (spec 0070)
//--------------------------------------------------------------------------------------------------
//
// The Canvas widget's arc/arcTo/ellipse/roundRect are Canvas2D semantics layered on Qt's
// clockwise, y-down, degree-based QPainterPath::arcTo. The suite pins the two places that layering
// can silently invert: the sweep normalisation (direction, full-turn saturation) and the tangent
// solve in arcTo, plus every degenerate branch that must fall back to a lineTo instead of emitting
// a NaN.
//
//--------------------------------------------------------------------------------------------------

class TstPainterGeometry : public QObject {
  Q_OBJECT

private slots:
  void sweepKeepsDirection();
  void sweepSaturatesAtOneTurn();
  void arcRejectsNonPositiveRadius();
  void arcLandsOnCanvasEndpoint();
  void arcHonoursDirection();
  void roundRectWithoutRadiiIsAPlainRect();
  void roundRectClampsRadiiToHalfTheShortSide();
  void arcToSeedsAnEmptyPath();
  void arcToRejectsNegativeRadius();
  void arcToDegradesToALine();
  void arcToSolvesARightAngleCorner();
  void ellipseRejectsNonPositiveRadii();
  void ellipseRotatesItsBoundingBox();
};

//--------------------------------------------------------------------------------------------------
// Sweep normalisation
//--------------------------------------------------------------------------------------------------

void TstPainterGeometry::sweepKeepsDirection()
{
  constexpr qreal kQuarter = M_PI * 0.5;

  QVERIFY(qAbs(normalizedSweep(0.0, kQuarter, false) - kQuarter) < 1e-9);
  QVERIFY(qAbs(normalizedSweep(0.0, kQuarter, true) - (kQuarter - 2.0 * M_PI)) < 1e-9);
  QVERIFY(qAbs(normalizedSweep(0.0, -kQuarter, false) - (2.0 * M_PI - kQuarter)) < 1e-9);
  QVERIFY(qAbs(normalizedSweep(0.0, -kQuarter, true) + kQuarter) < 1e-9);
  QVERIFY(qAbs(normalizedSweep(1.25, 1.25, false)) < 1e-9);
}

void TstPainterGeometry::sweepSaturatesAtOneTurn()
{
  constexpr qreal kTau = 2.0 * M_PI;

  QVERIFY(qAbs(normalizedSweep(0.0, kTau, false) - kTau) < 1e-9);
  QVERIFY(qAbs(normalizedSweep(0.0, 3.0 * M_PI, false) - kTau) < 1e-9);
  QVERIFY(qAbs(normalizedSweep(0.0, 40.0, false) - kTau) < 1e-9);
  QVERIFY(qAbs(normalizedSweep(0.0, -40.0, true) + kTau) < 1e-9);
}

//--------------------------------------------------------------------------------------------------
// Arcs
//--------------------------------------------------------------------------------------------------

void TstPainterGeometry::arcRejectsNonPositiveRadius()
{
  QPainterPath path;
  appendArc(path, 10.0, 10.0, 0.0, 0.0, M_PI, false);
  appendArc(path, 10.0, 10.0, -5.0, 0.0, M_PI, false);

  QCOMPARE(path.elementCount(), 0);
}

void TstPainterGeometry::arcLandsOnCanvasEndpoint()
{
  QPainterPath path;
  appendArc(path, 0.0, 0.0, 10.0, 0.0, M_PI * 0.5, false);

  const QPointF end = path.currentPosition();
  QVERIFY(qAbs(end.x()) < 1e-4);
  QVERIFY(qAbs(end.y() - 10.0) < 1e-4);
}

void TstPainterGeometry::arcHonoursDirection()
{
  QPainterPath cw;
  appendArc(cw, 0.0, 0.0, 10.0, 0.0, M_PI * 0.5, false);

  QPainterPath ccw;
  appendArc(ccw, 0.0, 0.0, 10.0, 0.0, M_PI * 0.5, true);

  const QPointF cwEnd  = cw.currentPosition();
  const QPointF ccwEnd = ccw.currentPosition();
  QVERIFY(qAbs(cwEnd.x() - ccwEnd.x()) < 1e-4);
  QVERIFY(qAbs(cwEnd.y() - ccwEnd.y()) < 1e-4);

  QVERIFY2(cw.boundingRect().width() < 12.0, "clockwise quarter arc left its quadrant");
  QVERIFY2(ccw.boundingRect().width() > 15.0, "counter-clockwise arc took the short way round");
}

//--------------------------------------------------------------------------------------------------
// Rounded rectangles
//--------------------------------------------------------------------------------------------------

void TstPainterGeometry::roundRectWithoutRadiiIsAPlainRect()
{
  QPainterPath rounded;
  appendRoundRect(rounded, 5.0, 7.0, 100.0, 50.0, 0.0, 0.0, 0.0, 0.0);

  QPainterPath plain;
  plain.addRect(5.0, 7.0, 100.0, 50.0);

  QCOMPARE(rounded.elementCount(), plain.elementCount());
  QCOMPARE(rounded.boundingRect(), plain.boundingRect());
}

void TstPainterGeometry::roundRectClampsRadiiToHalfTheShortSide()
{
  QPainterPath path;
  appendRoundRect(path, 0.0, 0.0, 100.0, 50.0, 1000.0, 1000.0, 1000.0, 1000.0);

  const QRectF bounds = path.boundingRect();
  QVERIFY(qAbs(bounds.left()) < 0.5);
  QVERIFY(qAbs(bounds.top()) < 0.5);
  QVERIFY(qAbs(bounds.width() - 100.0) < 0.5);
  QVERIFY(qAbs(bounds.height() - 50.0) < 0.5);
}

//--------------------------------------------------------------------------------------------------
// Tangent arcTo
//--------------------------------------------------------------------------------------------------

void TstPainterGeometry::arcToSeedsAnEmptyPath()
{
  QPainterPath path;
  appendArcTo(path, 30.0, 40.0, 90.0, 40.0, 5.0);

  QCOMPARE(path.elementCount(), 1);
  QVERIFY(qAbs(path.currentPosition().x() - 30.0) < 1e-9);
  QVERIFY(qAbs(path.currentPosition().y() - 40.0) < 1e-9);
}

void TstPainterGeometry::arcToRejectsNegativeRadius()
{
  QPainterPath path;
  appendArcTo(path, 30.0, 40.0, 90.0, 40.0, -1.0);

  QCOMPARE(path.elementCount(), 0);
}

void TstPainterGeometry::arcToDegradesToALine()
{
  QPainterPath collinear;
  collinear.moveTo(0.0, 0.0);
  appendArcTo(collinear, 10.0, 0.0, 20.0, 0.0, 5.0);
  QVERIFY(qAbs(collinear.currentPosition().x() - 10.0) < 1e-9);
  QVERIFY(qAbs(collinear.currentPosition().y()) < 1e-9);

  QPainterPath degenerate;
  degenerate.moveTo(10.0, 0.0);
  appendArcTo(degenerate, 10.0, 0.0, 20.0, 0.0, 5.0);
  QVERIFY(qAbs(degenerate.currentPosition().x() - 10.0) < 1e-9);
  QVERIFY(qAbs(degenerate.currentPosition().y()) < 1e-9);

  QPainterPath zeroRadius;
  zeroRadius.moveTo(0.0, 0.0);
  appendArcTo(zeroRadius, 100.0, 0.0, 100.0, 100.0, 0.0);
  QVERIFY(qAbs(zeroRadius.currentPosition().x() - 100.0) < 1e-9);
  QVERIFY(qAbs(zeroRadius.currentPosition().y()) < 1e-9);
}

void TstPainterGeometry::arcToSolvesARightAngleCorner()
{
  QPainterPath path;
  path.moveTo(0.0, 0.0);
  appendArcTo(path, 100.0, 0.0, 100.0, 100.0, 20.0);

  const QPointF end = path.currentPosition();
  QVERIFY2(qAbs(end.x() - 100.0) < 1e-4, "tangent arc missed the outgoing leg");
  QVERIFY2(qAbs(end.y() - 20.0) < 1e-4, "tangent arc ended at the wrong distance from the corner");

  QVERIFY2(path.boundingRect().right() < 100.5, "tangent arc overshot the corner");
}

//--------------------------------------------------------------------------------------------------
// Ellipses
//--------------------------------------------------------------------------------------------------

void TstPainterGeometry::ellipseRejectsNonPositiveRadii()
{
  QPainterPath path;
  appendEllipse(path, 0.0, 0.0, 0.0, 10.0, 0.0, 0.0, 2.0 * M_PI, false);
  appendEllipse(path, 0.0, 0.0, 10.0, -1.0, 0.0, 0.0, 2.0 * M_PI, false);

  QCOMPARE(path.elementCount(), 0);
}

void TstPainterGeometry::ellipseRotatesItsBoundingBox()
{
  QPainterPath flat;
  appendEllipse(flat, 0.0, 0.0, 20.0, 10.0, 0.0, 0.0, 2.0 * M_PI, false);

  QPainterPath upright;
  appendEllipse(upright, 0.0, 0.0, 20.0, 10.0, M_PI * 0.5, 0.0, 2.0 * M_PI, false);

  QVERIFY2(flat.boundingRect().width() > flat.boundingRect().height(),
           "unrotated ellipse lost its major axis");
  QVERIFY2(upright.boundingRect().height() > upright.boundingRect().width(),
           "quarter-turn rotation did not swap the axes");
}

QTEST_GUILESS_MAIN(TstPainterGeometry)
#include "tst_painter_geometry.moc"
