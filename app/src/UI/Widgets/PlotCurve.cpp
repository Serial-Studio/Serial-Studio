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

#include "UI/Widgets/PlotCurve.h"

#include <algorithm>
#include <QDebug>
#include <QElapsedTimer>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>

#include "DSPSimd.h"
#include "SSAssert.h"
#include "UI/Widgets/GpuStroke.h"

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
                                         qsizetype count,
                                         const double w,
                                         const double h)
{
  SS_ASSERT(pts != nullptr, return);
  SS_ASSERT(count >= 0, count = 0);

  const double sx = w / (m_xMax - m_xMin);
  const double sy = h / (m_yMax - m_yMin);

  if (m_px.size() != count)
    m_px.resize(count);

  QPointF* out = m_px.data();
  for (qsizetype i = 0; i < count; ++i)
    out[i] = QPointF((pts[i].x() - m_xMin) * sx, (m_yMax - pts[i].y()) * sy);
}

/**
 * @brief True when a run's X interval overlaps the visible window, so fully offscreen runs
 *        (zoom/pan slices) are skipped without extruding their geometry.
 */
bool Widgets::PlotCurve::runVisible(const QPointF* pts,
                                    const qsizetype start,
                                    const qsizetype len) const
{
  SS_ASSERT(pts != nullptr, return false);
  SS_ASSERT(len >= 1, return false);

  double lo = pts[start].x();
  double hi = pts[start].x();
  DSP::simdFiniteMinMaxPointF<0>(pts + start, len, lo, hi);

  return hi >= m_xMin && lo <= m_xMax;
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
  vertexCount = 0;
  indexCount  = 0;

  SS_ASSERT(pts != nullptr, return);
  SS_ASSERT(px != nullptr, return);
  SS_ASSERT(count >= 2, return);

  qsizetype start = 0;
  while (start < count) {
    const qsizetype len = GpuStroke::runLength(pts, count, start);
    if (len >= 2 && runVisible(pts, start, len))
      GpuStroke::countRun(px, start, len, vertexCount, indexCount);

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
  SS_ASSERT(vertices != nullptr, return);
  SS_ASSERT(indices != nullptr, return);

  int v           = 0;
  int idx         = 0;
  qsizetype start = 0;
  while (start < count) {
    const qsizetype len = GpuStroke::runLength(pts, count, start);
    if (len < 2 || !runVisible(pts, start, len)) {
      start += len;
      continue;
    }

    GpuStroke::emitRun(vertices, indices, v, idx, px, start, len, hw, m_color);
    start += len;
  }

  // code-verify off
  // Post-condition on a buffer already written: a soft assert cannot un-write an overrun, so the
  // countRibbon()/GpuStroke::emitRun() vertex-budget lockstep stays the real guard and this
  // stays debug-only.
  Q_ASSERT(v == vertexCount);
  Q_ASSERT(idx == indexCount);
  // code-verify on
  Q_UNUSED(vertexCount)
  Q_UNUSED(indexCount)
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

  if (vertexCount > GpuStroke::kMaxGeometry || indexCount > GpuStroke::kMaxGeometry) {
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
  SS_ASSERT(geometry != nullptr, return node);
  (void)GpuStroke::reserveGeometry(geometry, vertices, idxs);

  emitRibbon(geometry->vertexDataAsColoredPoint2D(),
             geometry->indexDataAsUInt(),
             vertices,
             idxs,
             pts,
             px,
             n,
             hw);

  GpuStroke::padGeometryTail(geometry, vertices, idxs);
  node->markDirty(QSGNode::DirtyGeometry);
  return node;
}
