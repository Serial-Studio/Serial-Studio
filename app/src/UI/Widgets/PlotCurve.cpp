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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/PlotCurve.h"

#include <algorithm>
#include <cmath>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>

// Edge feather in logical pixels, straddling the stroke edge so perceived width stays lineWidth
constexpr double kFeatherPx = 1.0;

// Round-join fan: one arc segment per kFanStep of turn, capped at kFanMax (a 180deg reversal)
constexpr double kFanStep = 0.39269908;
constexpr int kFanMax     = 8;

// Geometry ceiling (16M): past it the curve is dropped so int scene-graph buffers can't overflow
constexpr qsizetype kMaxGeometry = 1 << 24;

/**
 * @brief Encodes a premultiplied vertex color for QSGVertexColorMaterial.
 */
static void setVertexColor(QSGGeometry::ColoredPoint2D& vertex,
                           const float x,
                           const float y,
                           const QColor& color,
                           const double alpha)
{
  Q_ASSERT(alpha >= 0.0);
  Q_ASSERT(alpha <= 1.0);

  vertex.set(x,
             y,
             static_cast<unsigned char>(std::lround(color.redF() * alpha * 255.0)),
             static_cast<unsigned char>(std::lround(color.greenF() * alpha * 255.0)),
             static_cast<unsigned char>(std::lround(color.blueF() * alpha * 255.0)),
             static_cast<unsigned char>(std::lround(alpha * 255.0)));
}

/**
 * @brief Computes the unit direction between two pixel-space points; returns false for
 *        degenerate (near zero-length) segments.
 */
static bool unitDir(const QPointF& from, const QPointF& to, QPointF& dir)
{
  const double dx  = to.x() - from.x();
  const double dy  = to.y() - from.y();
  const double len = std::sqrt(dx * dx + dy * dy);
  if (!(len > 1e-6))
    return false;

  dir = QPointF(dx / len, dy / len);
  return true;
}

/**
 * @brief Arc-segment count for the round join between two unit segment directions: the exterior
 *        turn angle bucketed by kFanStep and capped at kFanMax. Pure in the directions, so the
 *        counting and emitting passes derive an identical count without shared state. Zero for a
 *        near-collinear join (no fan).
 */
static int fanSegments(const QPointF& dIn, const QPointF& dOut)
{
  const double dot   = std::clamp(dIn.x() * dOut.x() + dIn.y() * dOut.y(), -1.0, 1.0);
  const double cross = dIn.x() * dOut.y() - dIn.y() * dOut.x();
  const double phi   = std::atan2(std::abs(cross), dot);
  return std::clamp(static_cast<int>(std::ceil(phi / kFanStep)), 0, kFanMax);
}

/**
 * @brief Emits a symmetric four-vertex cross-section along unit normal n at half-width hw: feather
 *        edge, core edge, core edge, feather edge. The feather adds kFeatherPx beyond the core, so
 *        the 50%-alpha point lands on the stroke edge and the alpha ramp anti-aliases. Used at run
 *        ends (flat caps) where both sides are exposed.
 */
static void emitCapSection(QSGGeometry::ColoredPoint2D* vertices,
                           int& v,
                           const QPointF& p,
                           const QPointF& n,
                           const double hw,
                           const QColor& color)
{
  Q_ASSERT(vertices != nullptr);
  Q_ASSERT(hw > 0.0);

  const double alpha = color.alphaF();
  const double face  = std::max(0.25, hw - kFeatherPx * 0.5);
  const double edge  = face + kFeatherPx;

  setVertexColor(vertices[v++],
                 static_cast<float>(p.x() + n.x() * edge),
                 static_cast<float>(p.y() + n.y() * edge),
                 color,
                 0.0);
  setVertexColor(vertices[v++],
                 static_cast<float>(p.x() + n.x() * face),
                 static_cast<float>(p.y() + n.y() * face),
                 color,
                 alpha);
  setVertexColor(vertices[v++],
                 static_cast<float>(p.x() - n.x() * face),
                 static_cast<float>(p.y() - n.y() * face),
                 color,
                 alpha);
  setVertexColor(vertices[v++],
                 static_cast<float>(p.x() - n.x() * edge),
                 static_cast<float>(p.y() - n.y() * edge),
                 color,
                 0.0);
}

/**
 * @brief Emits a round-join fan filling the outer notch at vertex p, sweeping outer-side unit
 *        normals n0 to n1 across nFan>=1 arc steps. A center vertex plus nFan+1 feathered rim
 *        pairs (core at face, feather at edge) tile the arc as one core plus two feather triangles
 *        per step, so the join is solid with an anti-aliased rim.
 */
static void emitFan(QSGGeometry::ColoredPoint2D* vertices,
                    quint32* indices,
                    int& v,
                    int& idx,
                    const QPointF& p,
                    const QPointF& n0,
                    const QPointF& n1,
                    const int nFan,
                    const double hw,
                    const QColor& color)
{
  Q_ASSERT(vertices != nullptr);
  Q_ASSERT(indices != nullptr);
  Q_ASSERT(nFan >= 1);

  const double alpha = color.alphaF();
  const double face  = std::max(0.25, hw - kFeatherPx * 0.5);
  const double edge  = face + kFeatherPx;

  const double dot   = std::clamp(n0.x() * n1.x() + n0.y() * n1.y(), -1.0, 1.0);
  const double cross = n0.x() * n1.y() - n0.y() * n1.x();
  const double sweep = std::atan2(cross, dot) / nFan;
  const double cs    = std::cos(sweep);
  const double sn    = std::sin(sweep);

  const int center = v;
  setVertexColor(vertices[v++], static_cast<float>(p.x()), static_cast<float>(p.y()), color, alpha);

  double rx = n0.x();
  double ry = n0.y();
  for (int j = 0; j <= nFan; ++j) {
    const double dx = (j == nFan) ? n1.x() : rx;
    const double dy = (j == nFan) ? n1.y() : ry;
    setVertexColor(vertices[v++],
                   static_cast<float>(p.x() + dx * face),
                   static_cast<float>(p.y() + dy * face),
                   color,
                   alpha);
    setVertexColor(vertices[v++],
                   static_cast<float>(p.x() + dx * edge),
                   static_cast<float>(p.y() + dy * edge),
                   color,
                   0.0);
    const double nx = rx * cs - ry * sn;
    ry              = rx * sn + ry * cs;
    rx              = nx;
  }

  for (int j = 0; j < nFan; ++j) {
    const int c0   = center + 1 + 2 * j;
    const int c1   = center + 1 + 2 * (j + 1);
    indices[idx++] = static_cast<quint32>(center);
    indices[idx++] = static_cast<quint32>(c0);
    indices[idx++] = static_cast<quint32>(c1);
    indices[idx++] = static_cast<quint32>(c0);
    indices[idx++] = static_cast<quint32>(c0 + 1);
    indices[idx++] = static_cast<quint32>(c1 + 1);
    indices[idx++] = static_cast<quint32>(c0);
    indices[idx++] = static_cast<quint32>(c1 + 1);
    indices[idx++] = static_cast<quint32>(c1);
  }
}

/**
 * @brief Emits the 18 indices (three quad bands, two triangles each) joining two consecutive
 *        four-vertex cross-sections along a run.
 */
static void emitJoinIndices(quint32* indices, int& idx, const int a, const int b)
{
  Q_ASSERT(indices != nullptr);
  Q_ASSERT(a >= 0 && b >= 0);

  for (int k = 0; k < 3; ++k) {
    indices[idx++] = static_cast<quint32>(a + k);
    indices[idx++] = static_cast<quint32>(b + k);
    indices[idx++] = static_cast<quint32>(a + k + 1);
    indices[idx++] = static_cast<quint32>(b + k);
    indices[idx++] = static_cast<quint32>(b + k + 1);
    indices[idx++] = static_cast<quint32>(a + k + 1);
  }
}

/**
 * @brief Constructs the curve item and enables scene-graph content.
 */
Widgets::PlotCurve::PlotCurve(QQuickItem* parent)
  : QQuickItem(parent), m_lineWidth(2.0), m_xMin(0), m_xMax(1), m_yMin(0), m_yMax(1)
{
  setFlag(ItemHasContents, true);
}

/**
 * @brief Returns the curve series rendered by this item.
 */
QXYSeries* Widgets::PlotCurve::source() const noexcept
{
  return m_source.data();
}

/**
 * @brief Returns the stroke color.
 */
const QColor& Widgets::PlotCurve::color() const noexcept
{
  return m_color;
}

/**
 * @brief Returns the stroke width in logical pixels.
 */
double Widgets::PlotCurve::lineWidth() const noexcept
{
  return m_lineWidth;
}

/**
 * @brief Returns the lower bound of the visible X window.
 */
double Widgets::PlotCurve::xMin() const noexcept
{
  return m_xMin;
}

/**
 * @brief Returns the upper bound of the visible X window.
 */
double Widgets::PlotCurve::xMax() const noexcept
{
  return m_xMax;
}

/**
 * @brief Returns the lower bound of the visible Y window.
 */
double Widgets::PlotCurve::yMin() const noexcept
{
  return m_yMin;
}

/**
 * @brief Returns the upper bound of the visible Y window.
 */
double Widgets::PlotCurve::yMax() const noexcept
{
  return m_yMax;
}

/**
 * @brief Follows the given series; the curve regenerates on every series update().
 */
void Widgets::PlotCurve::setSource(QXYSeries* series)
{
  if (m_source.data() == series)
    return;

  if (m_sourceConnection)
    disconnect(m_sourceConnection);

  m_source = series;
  if (m_source)
    m_sourceConnection =
      connect(m_source.data(), &QXYSeries::update, this, &QQuickItem::update, Qt::DirectConnection);

  Q_EMIT sourceChanged();
  update();
}

/**
 * @brief Sets the stroke color.
 */
void Widgets::PlotCurve::setColor(const QColor& color)
{
  if (m_color == color)
    return;

  m_color = color;
  Q_EMIT colorChanged();
  update();
}

/**
 * @brief Sets the stroke width in logical pixels.
 */
void Widgets::PlotCurve::setLineWidth(const double width)
{
  if (m_lineWidth == width)
    return;

  m_lineWidth = width;
  Q_EMIT lineWidthChanged();
  update();
}

/**
 * @brief Sets the lower bound of the visible X window.
 */
void Widgets::PlotCurve::setXMin(const double value)
{
  if (m_xMin == value)
    return;

  m_xMin = value;
  Q_EMIT rangeChanged();
  update();
}

/**
 * @brief Sets the upper bound of the visible X window.
 */
void Widgets::PlotCurve::setXMax(const double value)
{
  if (m_xMax == value)
    return;

  m_xMax = value;
  Q_EMIT rangeChanged();
  update();
}

/**
 * @brief Sets the lower bound of the visible Y window.
 */
void Widgets::PlotCurve::setYMin(const double value)
{
  if (m_yMin == value)
    return;

  m_yMin = value;
  Q_EMIT rangeChanged();
  update();
}

/**
 * @brief Sets the upper bound of the visible Y window.
 */
void Widgets::PlotCurve::setYMax(const double value)
{
  if (m_yMax == value)
    return;

  m_yMax = value;
  Q_EMIT rangeChanged();
  update();
}

/**
 * @brief Projects the data points into pixel space once, into m_px, so the miter pass reads
 *        each vertex's pixel coordinate without re-deriving it. Non-finite inputs map to
 *        non-finite pixels and stay run boundaries downstream.
 */
void Widgets::PlotCurve::projectToPixels(const QPointF* pts,
                                         const qsizetype count,
                                         const double w,
                                         const double h)
{
  Q_ASSERT(pts != nullptr);
  Q_ASSERT(count >= 0);

  const double sx = w / (m_xMax - m_xMin);
  const double sy = h / (m_yMax - m_yMin);

  if (m_px.size() != count)
    m_px.resize(count);

  QPointF* out = m_px.data();
  for (qsizetype i = 0; i < count; ++i)
    out[i] = QPointF((pts[i].x() - m_xMin) * sx, (m_yMax - pts[i].y()) * sy);
}

/**
 * @brief Length of the maximal run of finite points starting at start: the polyline is
 *        split into connected strips at non-finite points (NaN/inf gaps), and each strip
 *        is mitered as one continuous stroke. Returns the vertex count of the run; start
 *        advances past any leading non-finite point.
 */
qsizetype Widgets::PlotCurve::runLength(const QPointF* pts,
                                        const qsizetype count,
                                        qsizetype& start) const
{
  Q_ASSERT(pts != nullptr);
  Q_ASSERT(count >= 0);

  while (start < count && (!std::isfinite(pts[start].x()) || !std::isfinite(pts[start].y())))
    ++start;

  qsizetype end = start;
  while (end < count && std::isfinite(pts[end].x()) && std::isfinite(pts[end].y()))
    ++end;

  return end - start;
}

/**
 * @brief True when a run's X interval overlaps the visible window, so fully offscreen runs
 *        (zoom/pan slices) are skipped without extruding their geometry.
 */
bool Widgets::PlotCurve::runVisible(const QPointF* pts,
                                    const qsizetype start,
                                    const qsizetype len) const
{
  Q_ASSERT(pts != nullptr);
  Q_ASSERT(len >= 1);

  double lo = pts[start].x();
  double hi = pts[start].x();
  for (qsizetype i = start + 1; i < start + len; ++i) {
    lo = std::min(lo, pts[i].x());
    hi = std::max(hi, pts[i].x());
  }

  return hi >= m_xMin && lo <= m_xMax;
}

/**
 * @brief Counts one run's vertices and indices; the walk, direction fallback and fanSegments()
 *        call MUST stay textually identical to emitRun or the passes disagree and overrun the
 *        geometry buffer. qsizetype accumulators tolerate an undecimated raw series.
 */
void Widgets::PlotCurve::countRun(const QPointF* px,
                                  const qsizetype start,
                                  const qsizetype len,
                                  qsizetype& vertexCount,
                                  qsizetype& indexCount) const
{
  Q_ASSERT(px != nullptr);
  Q_ASSERT(len >= 2);

  auto dirAt = [&](const qsizetype i, const QPointF& fallback) {
    QPointF dir = fallback;
    (void)unitDir(px[start + i], px[start + i + 1], dir);
    return dir;
  };

  QPointF prevDir = dirAt(0, QPointF(1, 0));
  for (qsizetype i = 0; i + 1 < len; ++i) {
    const QPointF dir  = dirAt(i, prevDir);
    vertexCount       += 8;
    indexCount        += 18;

    if (i + 2 < len) {
      const QPointF dOut = dirAt(i + 1, dir);
      const int nFan     = fanSegments(dir, dOut);
      if (nFan >= 1) {
        vertexCount += 2 * nFan + 3;
        indexCount  += 9 * nFan;
      }
    }

    prevDir = dir;
  }
}

/**
 * @brief Counts ribbon vertices and indices over each visible run of two or more points, by
 *        summing countRun. Must stay in lockstep with emitRibbon().
 */
void Widgets::PlotCurve::countRibbon(const QPointF* pts,
                                     const QPointF* px,
                                     const qsizetype count,
                                     qsizetype& vertexCount,
                                     qsizetype& indexCount) const
{
  Q_ASSERT(pts != nullptr);
  Q_ASSERT(px != nullptr);
  Q_ASSERT(count >= 2);

  vertexCount = 0;
  indexCount  = 0;

  qsizetype start = 0;
  while (start < count) {
    const qsizetype len = runLength(pts, count, start);
    if (len >= 2 && runVisible(pts, start, len))
      countRun(px, start, len, vertexCount, indexCount);

    start += len;
  }
}

/**
 * @brief Streams each visible run as constant-width body quads with round joins, via emitRun. Sharp
 *        tips stay full-width and the fans fill the outer notch at each interior vertex. Triangle
 *        winding is mixed (fan side flips with turn direction), so correctness relies on
 *        QSGVertexColorMaterial not back-face culling: do not enable culling or swap the material.
 */
void Widgets::PlotCurve::emitRibbon(QSGGeometry::ColoredPoint2D* vertices,
                                    quint32* indices,
                                    const int vertexCount,
                                    const int indexCount,
                                    const QPointF* pts,
                                    const QPointF* px,
                                    const qsizetype count,
                                    const double hw) const
{
  Q_ASSERT(vertices != nullptr);
  Q_ASSERT(indices != nullptr);

  int v           = 0;
  int idx         = 0;
  qsizetype start = 0;
  while (start < count) {
    const qsizetype len = runLength(pts, count, start);
    if (len < 2 || !runVisible(pts, start, len)) {
      start += len;
      continue;
    }

    emitRun(vertices, indices, v, idx, px, start, len, hw);
    start += len;
  }

  Q_ASSERT(v == vertexCount);
  Q_ASSERT(idx == indexCount);
  Q_UNUSED(vertexCount)
  Q_UNUSED(indexCount)
}

/**
 * @brief Emits one run [start, start+len) as constant-width body quads with round joins: each
 *        segment is a flat-capped quad extruded hw along its own normal (width never varies with
 *        bend angle, sharp tips stay full-width), and each interior vertex gets an outer-side
 *        round-join fan. Carries the previous direction across degenerate (zero-length) segments.
 */
void Widgets::PlotCurve::emitRun(QSGGeometry::ColoredPoint2D* vertices,
                                 quint32* indices,
                                 int& v,
                                 int& idx,
                                 const QPointF* px,
                                 const qsizetype start,
                                 const qsizetype len,
                                 const double hw) const
{
  Q_ASSERT(vertices != nullptr);
  Q_ASSERT(indices != nullptr);
  Q_ASSERT(len >= 2);

  auto dirAt = [&](const qsizetype i, const QPointF& fallback) {
    QPointF dir = fallback;
    (void)unitDir(px[start + i], px[start + i + 1], dir);
    return dir;
  };

  QPointF prevDir = dirAt(0, QPointF(1, 0));
  for (qsizetype i = 0; i + 1 < len; ++i) {
    const QPointF dir = dirAt(i, prevDir);
    const QPointF n(-dir.y(), dir.x());

    const int body = v;
    emitCapSection(vertices, v, px[start + i], n, hw, m_color);
    emitCapSection(vertices, v, px[start + i + 1], n, hw, m_color);
    emitJoinIndices(indices, idx, body, body + 4);

    if (i + 2 < len) {
      const QPointF dOut = dirAt(i + 1, dir);
      const int nFan     = fanSegments(dir, dOut);
      if (nFan >= 1) {
        const double s = (dir.x() * dOut.y() - dir.y() * dOut.x()) < 0.0 ? 1.0 : -1.0;
        const QPointF nOut(-dOut.y(), dOut.x());
        emitFan(vertices,
                indices,
                v,
                idx,
                px[start + i + 1],
                QPointF(n.x() * s, n.y() * s),
                QPointF(nOut.x() * s, nOut.y() * s),
                nFan,
                hw,
                m_color);
      }
    }

    prevDir = dir;
  }
}

/**
 * @brief Rebuilds the ribbon node from the source series: project to pixels once, one
 *        counting pass sizes the vertex/index buffers (stable sizes reuse the allocation),
 *        one streaming pass fills them. Fully-offscreen runs are culled by run visibility,
 *        so zoomed curves cost the visible slice, not the full series.
 */
QSGNode* Widgets::PlotCurve::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data)
{
  Q_UNUSED(data)

  const double w      = width();
  const double h      = height();
  const double xRange = m_xMax - m_xMin;
  const double yRange = m_yMax - m_yMin;

  const QList<QPointF> points = m_source ? m_source->points() : QList<QPointF>();

  const bool valid =
    points.size() >= 2 && xRange > 0 && yRange > 0 && w > 0 && h > 0 && m_lineWidth > 0;
  if (!valid) {
    delete oldNode;
    return nullptr;
  }

  const QPointF* pts = points.constData();
  const qsizetype n  = points.size();
  projectToPixels(pts, n, w, h);
  const QPointF* px = m_px.constData();

  qsizetype vertexCount = 0;
  qsizetype indexCount  = 0;
  countRibbon(pts, px, n, vertexCount, indexCount);
  if (vertexCount < 8 || indexCount < 18) {
    delete oldNode;
    return nullptr;
  }

  if (vertexCount > kMaxGeometry || indexCount > kMaxGeometry) {
    delete oldNode;
    return nullptr;
  }

  const int vertices = static_cast<int>(vertexCount);
  const int idxs     = static_cast<int>(indexCount);
  const double hw    = std::max(0.5, m_lineWidth * 0.5);

  auto* node = static_cast<QSGGeometryNode*>(oldNode);
  if (!node) {
    node           = new QSGGeometryNode;
    auto* geometry = new QSGGeometry(
      QSGGeometry::defaultAttributes_ColoredPoint2D(), 0, 0, QSGGeometry::UnsignedIntType);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(new QSGVertexColorMaterial);
    node->setFlag(QSGNode::OwnsMaterial);
  }

  auto* geometry = node->geometry();
  Q_ASSERT(geometry != nullptr);
  if (geometry->vertexCount() != vertices || geometry->indexCount() != idxs)
    geometry->allocate(vertices, idxs);

  emitRibbon(geometry->vertexDataAsColoredPoint2D(),
             geometry->indexDataAsUInt(),
             vertices,
             idxs,
             pts,
             px,
             n,
             hw);

  node->markDirty(QSGNode::DirtyGeometry);
  return node;
}
