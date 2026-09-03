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

#include "UI/Widgets/GpuStroke.h"

#include <algorithm>
#include <cmath>
#include <QSGVertexColorMaterial>
#include <QtNumeric>

#include "SSAssert.h"

// Edge feather in logical pixels, straddling the stroke edge so perceived width stays lineWidth
constexpr double kFeatherPx = 1.0;

// Round-join fan: one arc segment per kFanStep of turn, capped at kFanMax (a 180deg reversal)
constexpr double kFanStep = 0.39269908;
constexpr int kFanMax     = 8;

//--------------------------------------------------------------------------------------------------
// Grow-only geometry buffers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Capacity to request for @p needed elements given @p held, never below either, never over
 *        the geometry ceiling.
 */
static int grownCapacity(const int needed, const int held)
{
  SS_ASSERT(needed >= 0, return held);
  SS_ASSERT(held >= 0, return needed);

  const qsizetype grown = static_cast<qsizetype>(needed)
                        * Widgets::GpuStroke::kGeometryHeadroomNumerator
                        / Widgets::GpuStroke::kGeometryHeadroomDenominator;
  const qsizetype capped =
    std::min(std::max(grown, static_cast<qsizetype>(needed)), Widgets::GpuStroke::kMaxGeometry);
  return static_cast<int>(std::max(capped, static_cast<qsizetype>(held)));
}

/**
 * @brief Grows @p geometry to hold at least @p vertices and @p indices, reusing the allocation
 *        whenever it already does, and answers whether it reallocated. Qt 6.11's qsggeometry.h
 *        carries ONE pair of counts over one buffer and no separate capacity, so any count change
 *        re-lays it out -- hundreds of KB per curve per frame once join fans move it (N4).
 */
bool Widgets::GpuStroke::reserveGeometry(QSGGeometry* geometry,
                                         const int vertices,
                                         const int indices)
{
  SS_ASSERT(geometry != nullptr, return false);
  SS_ASSERT(vertices > 0, return false);

  const int heldVertices = geometry->vertexCount();
  const int heldIndices  = geometry->indexCount();
  if (heldVertices >= vertices && heldIndices >= indices && heldVertices > 0)
    return false;

  int indexCapacity = grownCapacity(indices, heldIndices);
  if (indexCapacity > 0)
    indexCapacity += (3 - indexCapacity % 3) % 3;

  geometry->allocate(grownCapacity(vertices, heldVertices), indexCapacity);
  return true;
}

/**
 * @brief Makes everything past @p vertices / @p indices draw nothing: the spare index slots become
 *        zero-area triangles and the spare vertices repeat the last real one, which also keeps the
 *        renderer's batch bounds inside the curve instead of over uninitialised memory.
 */
void Widgets::GpuStroke::padGeometryTail(QSGGeometry* geometry,
                                         const int vertices,
                                         const int indices)
{
  SS_ASSERT(geometry != nullptr, return);
  SS_ASSERT(vertices > 0 && geometry->vertexCount() >= vertices, return);

  auto* vertexData         = geometry->vertexDataAsColoredPoint2D();
  const int vertexCapacity = geometry->vertexCount();
  for (int i = vertices; i < vertexCapacity; ++i)
    vertexData[i] = vertexData[vertices - 1];

  const int indexCapacity = geometry->indexCount();
  if (indexCapacity <= indices)
    return;

  quint32* indexData = geometry->indexDataAsUInt();
  for (int i = indices; i < indexCapacity; ++i)
    indexData[i] = 0;
}

/**
 * @brief Encodes a premultiplied vertex color for QSGVertexColorMaterial.
 */
static void setVertexColor(QSGGeometry::ColoredPoint2D& vertex,
                           const float x,
                           const float y,
                           const QColor& color,
                           double alpha)
{
  SS_ASSERT(alpha >= 0.0 && alpha <= 1.0, alpha = std::clamp(alpha, 0.0, 1.0));

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
                           double hw,
                           const QColor& color)
{
  SS_ASSERT(vertices != nullptr, return);
  SS_ASSERT(hw > 0.0, hw = 0.25);

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
  SS_ASSERT(vertices != nullptr, return);
  SS_ASSERT(indices != nullptr, return);
  SS_ASSERT(nFan >= 1, return);

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
  SS_ASSERT(indices != nullptr, return);
  SS_ASSERT(a >= 0 && b >= 0, return);

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
 * @brief Length of the maximal run of finite points starting at start: the polyline splits
 *        into connected strips at non-finite points, each mitered as one stroke. start advances
 *        past any leading non-finite point, and all the way to the end on a failed precondition
 *        so the caller's `start < count` loop terminates instead of spinning in release.
 */
qsizetype Widgets::GpuStroke::runLength(const QPointF* pts, const qsizetype count, qsizetype& start)
{
  SS_ASSERT_LOG(pts != nullptr);
  SS_ASSERT_LOG(count >= 0);
  if (!pts || count < 0) {
    start = count;
    return 0;
  }

  while (start < count && (!std::isfinite(pts[start].x()) || !std::isfinite(pts[start].y())))
    ++start;

  qsizetype end = start;
  while (end < count && std::isfinite(pts[end].x()) && std::isfinite(pts[end].y()))
    ++end;

  return end - start;
}

/**
 * @brief Linearly blends two colors in premultiplication-safe float space.
 */
static QColor lerpColor(const QColor& a, const QColor& b, const double t)
{
  QColor out;
  out.setRedF(a.redF() * (1.0 - t) + b.redF() * t);
  out.setGreenF(a.greenF() * (1.0 - t) + b.greenF() * t);
  out.setBlueF(a.blueF() * (1.0 - t) + b.blueF() * t);
  out.setAlphaF(a.alphaF() * (1.0 - t) + b.alphaF() * t);
  return out;
}

/**
 * @brief Appends one point and its color to the dashed output buffers.
 */
static void pushDashPoint(std::vector<QPointF>& outPx,
                          std::vector<QColor>& outColors,
                          const QPointF& p,
                          const QColor& color)
{
  outPx.push_back(p);
  outColors.push_back(color);
}

/**
 * @brief Walk state carried across segments so a dash spans a polyline corner unbroken.
 */
struct DashState {
  bool drawing;
  bool spanOpen;
  double remaining;
};

/**
 * @brief Closes the open dashed span with a non-finite separator, which is how runLength()
 *        breaks runs.
 */
static void closeDashSpan(std::vector<QPointF>& outPx,
                          std::vector<QColor>& outColors,
                          const QColor& color,
                          DashState& state)
{
  if (!state.spanOpen)
    return;

  pushDashPoint(outPx, outColors, QPointF(qQNaN(), qQNaN()), color);
  state.spanOpen = false;
}

/**
 * @brief Walks one segment, emitting the on-spans of the dash pattern and toggling the walk
 *        state at each pattern boundary. Colors are interpolated at every split.
 */
static void dashSegment(const QPointF& a,
                        const QPointF& b,
                        const QColor& ca,
                        const QColor& cb,
                        const double onLength,
                        const double offLength,
                        DashState& state,
                        std::vector<QPointF>& outPx,
                        std::vector<QColor>& outColors)
{
  const double segLength = std::hypot(b.x() - a.x(), b.y() - a.y());
  if (!(segLength > 0.0))
    return;

  double travelled = 0.0;
  while (travelled < segLength) {
    const double step = std::min(state.remaining, segLength - travelled);
    const double t0   = travelled / segLength;
    const double t1   = (travelled + step) / segLength;

    if (state.drawing && !state.spanOpen) {
      pushDashPoint(outPx, outColors, a + (b - a) * t0, lerpColor(ca, cb, t0));
      state.spanOpen = true;
    }

    if (state.drawing)
      pushDashPoint(outPx, outColors, a + (b - a) * t1, lerpColor(ca, cb, t1));

    travelled       += step;
    state.remaining -= step;
    if (state.remaining > 1e-9)
      continue;

    closeDashSpan(outPx, outColors, cb, state);
    state.drawing   = !state.drawing;
    state.remaining = state.drawing ? onLength : offLength;
  }
}

/**
 * @brief Rewrites a pixel-space polyline into dashed spans of onLength separated by gaps of
 *        offLength, walking arc length across segment boundaries. Gaps become non-finite
 *        points, which is already how runLength() breaks runs. Output buffers are cleared but
 *        keep their capacity, so repeated rebuilds do not allocate.
 */
void Widgets::GpuStroke::dashPolyline(const QPointF* px,
                                      const QColor* colors,
                                      const qsizetype count,
                                      const double onLength,
                                      const double offLength,
                                      std::vector<QPointF>& outPx,
                                      std::vector<QColor>& outColors)
{
  outPx.clear();
  outColors.clear();

  SS_ASSERT(px != nullptr, return);
  SS_ASSERT(colors != nullptr, return);
  SS_ASSERT(onLength > 0.0 && offLength > 0.0, return);
  if (count < 2)
    return;

  DashState state{true, false, onLength};
  for (qsizetype i = 0; i + 1 < count; ++i) {
    const QPointF a = px[i];
    const QPointF b = px[i + 1];
    const bool finite =
      std::isfinite(a.x()) && std::isfinite(a.y()) && std::isfinite(b.x()) && std::isfinite(b.y());
    if (!finite) {
      closeDashSpan(outPx, outColors, colors[i], state);
      continue;
    }

    dashSegment(a, b, colors[i], colors[i + 1], onLength, offLength, state, outPx, outColors);
  }

  closeDashSpan(outPx, outColors, colors[count - 1], state);
}

/**
 * @brief Counts one run's vertices and indices; the walk, direction fallback and fanSegments()
 *        call MUST stay textually identical to emitRun or the passes disagree and overrun the
 *        geometry buffer. qsizetype accumulators tolerate an undecimated raw series.
 */
void Widgets::GpuStroke::countRun(const QPointF* px,
                                  const qsizetype start,
                                  const qsizetype len,
                                  qsizetype& vertexCount,
                                  qsizetype& indexCount)
{
  SS_ASSERT(px != nullptr, return);
  SS_ASSERT(len >= 2, return);

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
 * @brief Emits one run [start, start+len) as flat-capped body quads extruded hw along each
 *        segment normal, with an outer-side round-join fan at every interior vertex. The
 *        optional per-point colors array (null for a uniform stroke) shades vertex payload
 *        only, so countRun()'s counts are unaffected and the passes stay in lockstep.
 */
void Widgets::GpuStroke::emitRun(QSGGeometry::ColoredPoint2D* vertices,
                                 quint32* indices,
                                 int& v,
                                 int& idx,
                                 const QPointF* px,
                                 const qsizetype start,
                                 const qsizetype len,
                                 const double hw,
                                 const QColor& color,
                                 const QColor* colors)
{
  SS_ASSERT(vertices != nullptr, return);
  SS_ASSERT(indices != nullptr, return);
  SS_ASSERT(len >= 2, return);

  auto colorAt = [&](const qsizetype i) -> const QColor& {
    return colors ? colors[start + i] : color;
  };

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
    emitCapSection(vertices, v, px[start + i], n, hw, colorAt(i));
    emitCapSection(vertices, v, px[start + i + 1], n, hw, colorAt(i + 1));
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
                colorAt(i + 1));
      }
    }

    prevDir = dir;
  }
}

/**
 * @brief Emits one axis-aligned quad centered on p, spanning halfSize in each direction.
 */
static void emitPointQuad(QSGGeometry::ColoredPoint2D* vertices,
                          quint32* indices,
                          int& v,
                          int& idx,
                          const QPointF& p,
                          const double halfSize,
                          const QColor& color)
{
  SS_ASSERT(vertices != nullptr, return);
  SS_ASSERT(indices != nullptr, return);

  const double alpha = color.alphaF();
  const int base     = v;

  setVertexColor(vertices[v++],
                 static_cast<float>(p.x() - halfSize),
                 static_cast<float>(p.y() - halfSize),
                 color,
                 alpha);
  setVertexColor(vertices[v++],
                 static_cast<float>(p.x() + halfSize),
                 static_cast<float>(p.y() - halfSize),
                 color,
                 alpha);
  setVertexColor(vertices[v++],
                 static_cast<float>(p.x() + halfSize),
                 static_cast<float>(p.y() + halfSize),
                 color,
                 alpha);
  setVertexColor(vertices[v++],
                 static_cast<float>(p.x() - halfSize),
                 static_cast<float>(p.y() + halfSize),
                 color,
                 alpha);

  indices[idx++] = static_cast<quint32>(base);
  indices[idx++] = static_cast<quint32>(base + 1);
  indices[idx++] = static_cast<quint32>(base + 2);
  indices[idx++] = static_cast<quint32>(base);
  indices[idx++] = static_cast<quint32>(base + 2);
  indices[idx++] = static_cast<quint32>(base + 3);
}

/**
 * @brief Builds or refreshes a vertex-colored geometry node for a pixel-space polyline whose
 *        runs are separated by non-finite points. One counting pass sizes the buffers (stable
 *        sizes reuse the allocation), one streaming pass fills them. Returns nullptr, deleting
 *        the node, when there is nothing to draw or the geometry ceiling is exceeded.
 */
QSGGeometryNode* Widgets::GpuStroke::buildStrokeNode(QSGGeometryNode* node,
                                                     const QPointF* px,
                                                     const QColor* colors,
                                                     const qsizetype count,
                                                     const double halfWidth,
                                                     MaterialFactory makeMaterial)
{
  SS_ASSERT_LOG(halfWidth > 0.0);

  if (!px || !colors || count < 2 || halfWidth <= 0.0) {
    delete node;
    return nullptr;
  }

  qsizetype vertexCount = 0;
  qsizetype indexCount  = 0;
  qsizetype start       = 0;
  while (start < count) {
    const qsizetype len = runLength(px, count, start);
    if (len >= 2)
      countRun(px, start, len, vertexCount, indexCount);

    start += len;
  }

  const bool drawable = vertexCount >= 8 && indexCount >= 18 && vertexCount <= kMaxGeometry
                     && indexCount <= kMaxGeometry;
  if (!drawable) {
    delete node;
    return nullptr;
  }

  const int vertices = static_cast<int>(vertexCount);
  const int idxs     = static_cast<int>(indexCount);

  if (!node) {
    node           = new QSGGeometryNode;
    auto* geometry = new QSGGeometry(
      QSGGeometry::defaultAttributes_ColoredPoint2D(), 0, 0, QSGGeometry::UnsignedIntType);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    QSGMaterial* material = makeMaterial ? makeMaterial() : nullptr;
    if (!material)
      material = new QSGVertexColorMaterial;

    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial);
  }

  auto* geometry = node->geometry();
  SS_ASSERT(geometry != nullptr, return node);
  (void)reserveGeometry(geometry, vertices, idxs);

  int v   = 0;
  int idx = 0;
  start   = 0;
  while (start < count) {
    const qsizetype len = runLength(px, count, start);
    if (len >= 2)
      emitRun(geometry->vertexDataAsColoredPoint2D(),
              geometry->indexDataAsUInt(),
              v,
              idx,
              px,
              start,
              len,
              halfWidth,
              colors[start],
              colors);

    start += len;
  }

  padGeometryTail(geometry, vertices, idxs);
  node->markDirty(QSGNode::DirtyGeometry);
  return node;
}

/**
 * @brief Builds or refreshes a vertex-colored node drawing one quad per finite point, sized
 *        halfSize about each point. Non-finite points are skipped, as they are on the stroke
 *        path. Returns nullptr, deleting the node, when there is nothing to draw.
 */
QSGGeometryNode* Widgets::GpuStroke::buildPointNode(QSGGeometryNode* node,
                                                    const QPointF* px,
                                                    const QColor* colors,
                                                    const qsizetype count,
                                                    const double halfSize,
                                                    MaterialFactory makeMaterial)
{
  SS_ASSERT_LOG(halfSize > 0.0);

  if (!px || !colors || count < 1 || halfSize <= 0.0) {
    delete node;
    return nullptr;
  }

  qsizetype points = 0;
  for (qsizetype i = 0; i < count; ++i)
    if (std::isfinite(px[i].x()) && std::isfinite(px[i].y()))
      ++points;

  const qsizetype vertexCount = points * 4;
  const qsizetype indexCount  = points * 6;
  const bool drawable = points >= 1 && vertexCount <= kMaxGeometry && indexCount <= kMaxGeometry;
  if (!drawable) {
    delete node;
    return nullptr;
  }

  const int vertices = static_cast<int>(vertexCount);
  const int idxs     = static_cast<int>(indexCount);

  if (!node) {
    node           = new QSGGeometryNode;
    auto* geometry = new QSGGeometry(
      QSGGeometry::defaultAttributes_ColoredPoint2D(), 0, 0, QSGGeometry::UnsignedIntType);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    QSGMaterial* material = makeMaterial ? makeMaterial() : nullptr;
    if (!material)
      material = new QSGVertexColorMaterial;

    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial);
  }

  auto* geometry = node->geometry();
  SS_ASSERT(geometry != nullptr, return node);
  (void)reserveGeometry(geometry, vertices, idxs);

  auto* vertexData = geometry->vertexDataAsColoredPoint2D();
  auto* indexData  = geometry->indexDataAsUInt();

  int v   = 0;
  int idx = 0;
  for (qsizetype i = 0; i < count; ++i) {
    if (!std::isfinite(px[i].x()) || !std::isfinite(px[i].y()))
      continue;

    emitPointQuad(vertexData, indexData, v, idx, px[i], halfSize, colors[i]);
  }

  padGeometryTail(geometry, vertices, idxs);
  node->markDirty(QSGNode::DirtyGeometry);
  return node;
}
