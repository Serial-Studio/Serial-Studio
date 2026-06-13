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
 * @brief Emits one four-vertex cross-section along the segment normal: feather edge,
 *        core edge, core edge, feather edge. The solid core ends at hw - feather/2 and
 *        the alpha ramp at hw + feather/2, so the 50%-alpha point sits exactly on the
 *        stroke edge.
 */
static void emitCrossSection(QSGGeometry::ColoredPoint2D* vertices,
                             int& v,
                             const QPointF& p,
                             const QPointF& ext,
                             const double hw,
                             const QColor& color)
{
  Q_ASSERT(vertices != nullptr);
  Q_ASSERT(hw > 0.0);

  const double alpha = color.alphaF();
  const double face  = std::max(0.25, hw - kFeatherPx * 0.5);
  const double edge  = face + kFeatherPx;
  const double fx    = ext.x() * edge;
  const double fy    = ext.y() * edge;
  const double cx    = ext.x() * face;
  const double cy    = ext.y() * face;

  setVertexColor(
    vertices[v++], static_cast<float>(p.x() + fx), static_cast<float>(p.y() + fy), color, 0.0);
  setVertexColor(
    vertices[v++], static_cast<float>(p.x() + cx), static_cast<float>(p.y() + cy), color, alpha);
  setVertexColor(
    vertices[v++], static_cast<float>(p.x() - cx), static_cast<float>(p.y() - cy), color, alpha);
  setVertexColor(
    vertices[v++], static_cast<float>(p.x() - fx), static_cast<float>(p.y() - fy), color, 0.0);
}

/**
 * @brief Emits the 18 indices (three quad bands, two triangles each) joining two
 *        four-vertex cross-sections.
 */
static void emitSegmentIndices(quint32* indices, int& idx, const int a, const int b)
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
 * @brief True when the segment from point i to i+1 has finite endpoints and its X
 *        interval overlaps the visible window (works for non-monotonic X too).
 */
bool Widgets::PlotCurve::segmentVisible(const QPointF* pts,
                                        const qsizetype count,
                                        const qsizetype i) const
{
  Q_ASSERT(pts != nullptr);
  Q_ASSERT(count >= 0);

  if (i < 0 || i + 1 >= count)
    return false;

  const double x0 = pts[i].x();
  const double y0 = pts[i].y();
  const double x1 = pts[i + 1].x();
  const double y1 = pts[i + 1].y();
  if (!std::isfinite(x0) || !std::isfinite(y0) || !std::isfinite(x1) || !std::isfinite(y1))
    return false;

  return std::max(x0, x1) >= m_xMin && std::min(x0, x1) <= m_xMax;
}

/**
 * @brief Counts ribbon vertices and indices: eight vertices and 18 indices per visible
 *        segment. Must stay in lockstep with emitRibbon().
 */
void Widgets::PlotCurve::countRibbon(const QPointF* pts,
                                     const qsizetype count,
                                     int& vertexCount,
                                     int& indexCount) const
{
  Q_ASSERT(pts != nullptr);
  Q_ASSERT(count >= 2);

  vertexCount = 0;
  indexCount  = 0;

  for (qsizetype i = 0; i + 1 < count; ++i) {
    if (segmentVisible(pts, count, i)) {
      vertexCount += 8;
      indexCount  += 18;
    }
  }
}

/**
 * @brief Streams independent per-segment quads: each visible segment extrudes both of
 *        its cross-sections along its own perpendicular. Sharing join cross-sections
 *        instead collapses the ribbon to a hairline on near-reversals (every spike tip
 *        in dense data); overlapping butt joints are invisible with an opaque core.
 */
void Widgets::PlotCurve::emitRibbon(QSGGeometry::ColoredPoint2D* vertices,
                                    quint32* indices,
                                    const int vertexCount,
                                    const int indexCount,
                                    const QPointF* pts,
                                    const qsizetype count,
                                    const double w,
                                    const double h) const
{
  Q_ASSERT(vertices != nullptr);
  Q_ASSERT(indices != nullptr);

  const double sx = w / (m_xMax - m_xMin);
  const double sy = h / (m_yMax - m_yMin);
  const double hw = std::max(0.5, m_lineWidth * 0.5);

  auto toPx = [&](const QPointF& pt) -> QPointF {
    return QPointF((pt.x() - m_xMin) * sx, (m_yMax - pt.y()) * sy);
  };

  int v   = 0;
  int idx = 0;
  for (qsizetype i = 0; i + 1 < count; ++i) {
    if (!segmentVisible(pts, count, i))
      continue;

    const QPointF p0 = toPx(pts[i]);
    const QPointF p1 = toPx(pts[i + 1]);

    QPointF dir = QPointF(1, 0);
    (void)unitDir(p0, p1, dir);
    const QPointF normal(-dir.y(), dir.x());

    const int base = v;
    emitCrossSection(vertices, v, p0, normal, hw, m_color);
    emitCrossSection(vertices, v, p1, normal, hw, m_color);
    emitSegmentIndices(indices, idx, base, base + 4);
  }

  Q_ASSERT(v == vertexCount);
  Q_ASSERT(idx == indexCount);
  Q_UNUSED(vertexCount)
  Q_UNUSED(indexCount)
}

/**
 * @brief Rebuilds the ribbon node from the source series: one counting pass sizes the
 *        vertex/index buffers (stable sizes reuse the allocation), one streaming pass
 *        fills them. Offscreen stretches are culled by segment visibility, so zoomed
 *        curves cost the visible slice, not the full series.
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

  int vertexCount = 0;
  int indexCount  = 0;
  countRibbon(points.constData(), points.size(), vertexCount, indexCount);
  if (vertexCount < 8 || indexCount < 18) {
    delete oldNode;
    return nullptr;
  }

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
  if (geometry->vertexCount() != vertexCount || geometry->indexCount() != indexCount)
    geometry->allocate(vertexCount, indexCount);

  emitRibbon(geometry->vertexDataAsColoredPoint2D(),
             geometry->indexDataAsUInt(),
             vertexCount,
             indexCount,
             points.constData(),
             points.size(),
             w,
             h);

  node->markDirty(QSGNode::DirtyGeometry);
  return node;
}
