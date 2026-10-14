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
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>

#include "Core/DSPSimd.h"
#include "Core/SSAssert.h"
#include "Misc/HdrOutput.h"
#include "UI/Widgets/GpuStroke.h"
#include "UI/Widgets/SdfStroke.h"
#include "UI/Widgets/SdfStrokeMaterial.h"
#include "UI/Widgets/StrokeHdrMaterial.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Ancestors walked for inherited opacity; deeper than any dashboard scene, bounds the loop
constexpr int kMaxOpacityDepth = 64;

/**
 * @brief Builds the stroke material for the current mode; both node creation and the swap
 *        branch share it, and neither material back-face culls (the ribbon winding is mixed).
 */
static QSGMaterial* make_stroke_material(const bool hdr)
{
  if (hdr)
    return new Widgets::StrokeHdrMaterial;

  return new QSGVertexColorMaterial;
}

/**
 * @brief Constructs the curve item and enables scene-graph content.
 */
Widgets::PlotCurve::PlotCurve(QQuickItem* parent)
  : QQuickItem(parent)
  , m_lineWidth(2.0)
  , m_xMin(0)
  , m_xMax(1)
  , m_yMin(0)
  , m_yMax(1)
  , m_hdrMaterial(false)
  , m_sdfNode(false)
{
  setFlag(ItemHasContents, true);
  connect(this, &QQuickItem::opacityChanged, this, &QQuickItem::update);
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
 * @brief Streams each visible run as constant-width body quads with round joins, via emitRun.
 *        Sharp tips stay full-width, fans fill the outer notch at interior vertices, and the
 *        mixed triangle winding means the attached material must never back-face cull
 *        (QSGVertexColorMaterial and StrokeHdrMaterial both qualify).
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
 * @brief Counts signed-distance quads over each visible run of two or more points, mirroring
 *        countRibbon()'s walk so both paths cull the same offscreen runs.
 */
void Widgets::PlotCurve::countSdf(const QPointF* pts,
                                  const qsizetype count,
                                  qsizetype& vertexCount,
                                  qsizetype& indexCount) const
{
  vertexCount = 0;
  indexCount  = 0;

  SS_ASSERT(pts != nullptr, return);
  SS_ASSERT(count >= 2, return);

  qsizetype start = 0;
  while (start < count) {
    const qsizetype len = GpuStroke::runLength(pts, count, start);
    if (len >= 2 && runVisible(pts, start, len))
      SdfStroke::countRun(len, vertexCount, indexCount);

    start += len;
  }
}

/**
 * @brief True when the curve draws through the signed-distance path: only fully opaque
 *        strokes, since overlapping capsules would blend a translucent stroke twice at joins.
 *        Opacity is checked up the parent chain, because the shader applies the inherited one
 *        (a fading dashboard canvas must draw the ribbon). Read during sync, GUI blocked.
 */
bool Widgets::PlotCurve::useSdfStroke() const noexcept
{
  if (m_color.alphaF() < 1.0)
    return false;

  int depth = 0;
  for (const QQuickItem* item = this; item != nullptr && depth < kMaxOpacityDepth; ++depth) {
    if (item->opacity() < 1.0)
      return false;

    item = item->parentItem();
  }

  return true;
}

/**
 * @brief Rebuilds the curve node from the source series: project to pixels once, then the
 *        signed-distance path for opaque strokes or the tessellated ribbon otherwise. A path
 *        switch replaces the node, because the two vertex layouts differ.
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

  const bool sdf = useSdfStroke();
  if (oldNode && sdf != m_sdfNode) {
    delete oldNode;
    oldNode       = nullptr;
    m_hdrMaterial = false;
  }

  m_sdfNode = sdf;
  if (sdf)
    return updateSdfNode(oldNode, pts, px, n);

  return updateRibbonNode(oldNode, pts, px, n);
}

/**
 * @brief Signed-distance path (spec 0097): one quad per visible segment, a static index pattern
 *        written only when the grow-only buffer reallocates, a degenerate padded tail, and the
 *        style (color, face, feather, HDR boost) on the material instead of on every vertex.
 */
QSGNode* Widgets::PlotCurve::updateSdfNode(QSGNode* oldNode,
                                           const QPointF* pts,
                                           const QPointF* px,
                                           const qsizetype n)
{
  qsizetype vertexCount = 0;
  qsizetype indexCount  = 0;
  countSdf(pts, n, vertexCount, indexCount);
  if (vertexCount < SdfStroke::kVerticesPerSegment || vertexCount > GpuStroke::kMaxGeometry
      || indexCount > GpuStroke::kMaxGeometry) {
    delete oldNode;
    return nullptr;
  }

  const double hw     = std::max(0.5, m_lineWidth * 0.5);
  const double face   = std::max(0.25, hw - SdfStroke::kFeatherPx * 0.5);
  const double extent = face + SdfStroke::kFeatherPx;
  const float boost   = std::max(1.0f, Misc::HdrOutput::effectiveBoost(window()));
  const auto vertices = static_cast<int>(vertexCount);
  const auto indices  = static_cast<int>(indexCount);

  auto* node = static_cast<QSGGeometryNode*>(oldNode);
  if (!node) {
    node           = new QSGGeometryNode;
    auto* geometry = new QSGGeometry(SdfStroke::attributes(), 0, 0, QSGGeometry::UnsignedIntType);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(new SdfStrokeMaterial);
    node->setFlag(QSGNode::OwnsMaterial);
  }

  auto* material = static_cast<SdfStrokeMaterial*>(node->material());
  if (material->setStyle(m_color, static_cast<float>(face), SdfStroke::kFeatherPx, boost))
    node->markDirty(QSGNode::DirtyMaterial);

  auto* geometry = node->geometry();
  SS_ASSERT(geometry != nullptr, return node);
  if (GpuStroke::reserveGeometry(geometry, vertices, indices))
    SdfStroke::writeIndexPattern(geometry);

  int v           = 0;
  auto* data      = static_cast<SdfStroke::Vertex*>(geometry->vertexData());
  qsizetype start = 0;
  while (start < n) {
    const qsizetype len = GpuStroke::runLength(pts, n, start);
    if (len >= 2 && runVisible(pts, start, len))
      SdfStroke::emitRun(data, v, px, start, len, extent);

    start += len;
  }

  SdfStroke::padTail(geometry, v);
  node->markDirty(QSGNode::DirtyGeometry);
  return node;
}

/**
 * @brief Tessellated ribbon path (translucent strokes): one counting pass sizes the buffers
 *        (stable sizes reuse the allocation), one streaming pass fills them. Fully-offscreen runs
 *        are culled by run visibility, so zoomed curves cost the visible slice.
 */
QSGNode* Widgets::PlotCurve::updateRibbonNode(QSGNode* oldNode,
                                              const QPointF* pts,
                                              const QPointF* px,
                                              const qsizetype n)
{
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

  const float boost  = Misc::HdrOutput::effectiveBoost(window());
  const bool wantHdr = boost > 1.0f;

  auto* node = static_cast<QSGGeometryNode*>(oldNode);
  if (!node) {
    node           = new QSGGeometryNode;
    auto* geometry = new QSGGeometry(
      QSGGeometry::defaultAttributes_ColoredPoint2D(), 0, 0, QSGGeometry::UnsignedIntType);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(make_stroke_material(wantHdr));
    node->setFlag(QSGNode::OwnsMaterial);
    m_hdrMaterial = wantHdr;
  }

  if (m_hdrMaterial != wantHdr) {
    QSGMaterial* previous = node->material();
    node->setMaterial(make_stroke_material(wantHdr));
    delete previous;
    m_hdrMaterial = wantHdr;
    node->markDirty(QSGNode::DirtyMaterial);
  }

  if (m_hdrMaterial) {
    auto* material = static_cast<StrokeHdrMaterial*>(node->material());
    if (material->intensity() != boost) {
      material->setIntensity(boost);
      node->markDirty(QSGNode::DirtyMaterial);
    }
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
