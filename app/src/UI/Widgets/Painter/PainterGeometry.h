/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * Pro feature -- requires the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <cmath>
#  include <QPainterPath>
#  include <QRectF>
#  include <QtMath>
#  include <QTransform>

/**
 * @brief Pure Canvas2D path constructors: they take the caller's QPainterPath and append the
 *        subpath a script asked for, with no drawing state involved.
 *
 * Header-inline on purpose: a script builds its whole path inside the widget's per-frame
 * paint(), so these have to stay visible to the caller's translation unit.
 */
namespace Widgets::PainterGeometry {

/**
 * @brief Normalises a Canvas2D (start, end, direction) triple into a signed sweep in radians.
 *        A span of a full turn or more saturates at one signed turn, matching the spec.
 */
[[nodiscard]] inline qreal normalizedSweep(qreal startRad, qreal endRad, bool counterClockwise)
{
  constexpr qreal kTau = 2.0 * M_PI;
  const qreal raw      = endRad - startRad;
  if (std::abs(raw) >= kTau)
    return counterClockwise ? -kTau : kTau;

  qreal sweepRad = std::fmod(raw, kTau);
  if (!counterClockwise && sweepRad < 0.0)
    sweepRad += kTau;
  else if (counterClockwise && sweepRad > 0.0)
    sweepRad -= kTau;

  return sweepRad;
}

/**
 * @brief Appends a rounded-rectangle subpath with the four per-corner radii already resolved.
 *        Radii are clamped to half the shorter side; an all-zero set degrades to a plain rect.
 */
inline void appendRoundRect(
  QPainterPath& path, qreal x, qreal y, qreal w, qreal h, qreal tl, qreal tr, qreal br, qreal bl)
{
  const qreal maxR = qMin(std::abs(w), std::abs(h)) * 0.5;
  tl               = qBound<qreal>(0.0, tl, maxR);
  tr               = qBound<qreal>(0.0, tr, maxR);
  br               = qBound<qreal>(0.0, br, maxR);
  bl               = qBound<qreal>(0.0, bl, maxR);

  if (qFuzzyIsNull(tl) && qFuzzyIsNull(tr) && qFuzzyIsNull(br) && qFuzzyIsNull(bl)) {
    path.addRect(x, y, w, h);
    return;
  }

  path.moveTo(x + tl, y);
  path.lineTo(x + w - tr, y);
  path.arcTo(x + w - 2 * tr, y, 2 * tr, 2 * tr, 90, -90);
  path.lineTo(x + w, y + h - br);
  path.arcTo(x + w - 2 * br, y + h - 2 * br, 2 * br, 2 * br, 0, -90);
  path.lineTo(x + bl, y + h);
  path.arcTo(x, y + h - 2 * bl, 2 * bl, 2 * bl, -90, -90);
  path.lineTo(x, y + tl);
  path.arcTo(x, y, 2 * tl, 2 * tl, 180, -90);
  path.closeSubpath();
}

/**
 * @brief Appends a circular arc to the path (Canvas2D semantics).
 */
inline void appendArc(QPainterPath& path,
                      qreal x,
                      qreal y,
                      qreal r,
                      qreal startRad,
                      qreal endRad,
                      bool counterClockwise)
{
  if (r <= 0.0)
    return;

  const qreal sweepRad = normalizedSweep(startRad, endRad, counterClockwise);
  const qreal startDeg = -qRadiansToDegrees(startRad);
  const qreal sweepDeg = -qRadiansToDegrees(sweepRad);
  const QRectF box(x - r, y - r, 2.0 * r, 2.0 * r);
  path.arcTo(box, startDeg, sweepDeg);
}

/**
 * @brief Appends an axis-rotated elliptical arc (Canvas2D semantics).
 */
inline void appendEllipse(QPainterPath& path,
                          qreal x,
                          qreal y,
                          qreal rx,
                          qreal ry,
                          qreal rotation,
                          qreal startRad,
                          qreal endRad,
                          bool counterClockwise)
{
  if (rx <= 0.0 || ry <= 0.0)
    return;

  const qreal sweepRad = normalizedSweep(startRad, endRad, counterClockwise);

  QPainterPath sub;
  const QRectF box(-rx, -ry, 2.0 * rx, 2.0 * ry);
  sub.arcMoveTo(box, -qRadiansToDegrees(startRad));
  sub.arcTo(box, -qRadiansToDegrees(startRad), -qRadiansToDegrees(sweepRad));

  QTransform t;
  t.translate(x, y);
  t.rotateRadians(rotation);
  path.connectPath(t.map(sub));
}

/**
 * @brief Appends a Canvas2D-style arcTo: the tangent arc of radius r joining the segment arriving
 *        at (x1, y1) to the one leaving towards (x2, y2). Degenerate geometry (empty path,
 *        zero-length leg, straight or reversed corner) falls back to a plain lineTo; the probe is
 *        on elementCount() because isEmpty() is true for a lone moveTo that still seeds the solve.
 */
inline void appendArcTo(QPainterPath& path, qreal x1, qreal y1, qreal x2, qreal y2, qreal r)
{
  if (r < 0.0)
    return;

  if (path.elementCount() == 0) {
    path.moveTo(x1, y1);
    return;
  }

  const QPointF p0 = path.currentPosition();
  const qreal v1x  = p0.x() - x1;
  const qreal v1y  = p0.y() - y1;
  const qreal v2x  = x2 - x1;
  const qreal v2y  = y2 - y1;
  const qreal len1 = std::hypot(v1x, v1y);
  const qreal len2 = std::hypot(v2x, v2y);
  if (qFuzzyIsNull(len1) || qFuzzyIsNull(len2) || qFuzzyIsNull(r)) {
    path.lineTo(x1, y1);
    return;
  }

  const qreal cos_a = (v1x * v2x + v1y * v2y) / (len1 * len2);
  const qreal a     = std::acos(qBound<qreal>(-1.0, cos_a, 1.0));
  if (qFuzzyCompare(a, M_PI) || qFuzzyIsNull(a)) {
    path.lineTo(x1, y1);
    return;
  }

  const qreal d       = r / std::tan(a * 0.5);
  const qreal invLen1 = 1.0 / len1;
  const qreal invLen2 = 1.0 / len2;
  const qreal n1x     = v1x * invLen1;
  const qreal n1y     = v1y * invLen1;
  const qreal n2x     = v2x * invLen2;
  const qreal n2y     = v2y * invLen2;
  const qreal tx1     = x1 + n1x * d;
  const qreal ty1     = y1 + n1y * d;
  const qreal tx2     = x1 + n2x * d;
  const qreal ty2     = y1 + n2y * d;

  const qreal sx = n1x + n2x;
  const qreal sy = n1y + n2y;
  const qreal sl = std::hypot(sx, sy);
  if (qFuzzyIsNull(sl)) {
    path.lineTo(x1, y1);
    return;
  }

  const qreal cR    = r / std::sin(a * 0.5);
  const qreal invSl = 1.0 / sl;
  const qreal cX    = x1 + (sx * invSl) * cR;
  const qreal cY    = y1 + (sy * invSl) * cR;
  const qreal cross = v1x * v2y - v1y * v2x;

  path.lineTo(tx1, ty1);

  const qreal startAng = std::atan2(ty1 - cY, tx1 - cX);
  const qreal endAng   = std::atan2(ty2 - cY, tx2 - cX);
  qreal sweep          = endAng - startAng;
  if (cross < 0.0 && sweep < 0.0)
    sweep += 2.0 * M_PI;
  else if (cross > 0.0 && sweep > 0.0)
    sweep -= 2.0 * M_PI;

  const QRectF box(cX - r, cY - r, 2.0 * r, 2.0 * r);
  path.arcTo(box, -qRadiansToDegrees(startAng), -qRadiansToDegrees(sweep));
}

}  // namespace Widgets::PainterGeometry

#endif  // BUILD_COMMERCIAL
