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

#include "UI/Widgets/SdfStroke.h"

#include <algorithm>
#include <cmath>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Segments shorter than this keep a fixed direction so a repeated point still draws a dot
constexpr double kMinSegmentPx = 1e-6;

//--------------------------------------------------------------------------------------------------
// Vertex layout
//--------------------------------------------------------------------------------------------------

/**
 * @brief Attribute layout of SdfStroke::Vertex: corner position (location 0), segment endpoints
 *        (location 1, a.xy b.xy) and the cap flags (location 2). Static storage, because a
 *        QSGGeometry keeps a reference to its attribute set for its whole life.
 */
const QSGGeometry::AttributeSet& Widgets::SdfStroke::attributes()
{
  static const QSGGeometry::Attribute kAttributes[] = {
    QSGGeometry::Attribute::createWithAttributeType(
      0, 2, QSGGeometry::FloatType, QSGGeometry::PositionAttribute),
    QSGGeometry::Attribute::createWithAttributeType(
      1, 4, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute),
    QSGGeometry::Attribute::createWithAttributeType(
      2, 1, QSGGeometry::FloatType, QSGGeometry::UnknownAttribute),
  };

  static const QSGGeometry::AttributeSet kSet = {3, sizeof(Vertex), kAttributes};
  return kSet;
}

/**
 * @brief Writes one vertex.
 */
static void setVertex(Widgets::SdfStroke::Vertex& vertex,
                      const double x,
                      const double y,
                      const QPointF& a,
                      const QPointF& b,
                      const float caps)
{
  vertex.x    = static_cast<float>(x);
  vertex.y    = static_cast<float>(y);
  vertex.ax   = static_cast<float>(a.x());
  vertex.ay   = static_cast<float>(a.y());
  vertex.bx   = static_cast<float>(b.x());
  vertex.by   = static_cast<float>(b.y());
  vertex.caps = caps;
}

//--------------------------------------------------------------------------------------------------
// Geometry
//--------------------------------------------------------------------------------------------------

/**
 * @brief Vertices and indices one run of @p len points needs: one quad per segment, whatever
 *        the turn angles (spec 0097 R9); the round joins come from the shader, not from fans.
 */
void Widgets::SdfStroke::countRun(const qsizetype len,
                                  qsizetype& vertexCount,
                                  qsizetype& indexCount)
{
  SS_ASSERT(len >= 0, return);
  SS_ASSERT(vertexCount >= 0 && indexCount >= 0, return);

  const qsizetype segments  = len >= 2 ? len - 1 : 0;
  vertexCount              += segments * kVerticesPerSegment;
  indexCount               += segments * kIndicesPerSegment;
}

/**
 * @brief Emits one run [start, start+len) as one quad per segment, extended @p extent pixels
 *        along and across the segment so the fragment stage has room for the round cap and the
 *        feather. The run's first and last segments flag their outer ends flat, which keeps the
 *        flat caps the tessellated stroke draws at run ends and NaN gaps.
 */
void Widgets::SdfStroke::emitRun(Vertex* vertices,
                                 int& v,
                                 const QPointF* px,
                                 const qsizetype start,
                                 const qsizetype len,
                                 const double extent)
{
  SS_ASSERT(vertices != nullptr && px != nullptr, return);
  SS_ASSERT(len >= 2 && extent > 0.0, return);

  const qsizetype last = start + len - 2;
  for (qsizetype i = start; i <= last; ++i) {
    const QPointF& a    = px[i];
    const QPointF& b    = px[i + 1];
    const double dx     = b.x() - a.x();
    const double dy     = b.y() - a.y();
    const double length = std::sqrt((dx * dx) + (dy * dy));
    const double ux     = length > kMinSegmentPx ? dx / length : 1.0;
    const double uy     = length > kMinSegmentPx ? dy / length : 0.0;
    const double ex     = ux * extent;
    const double ey     = uy * extent;
    const auto caps     = static_cast<float>((i == start ? 1 : 0) + (i == last ? 2 : 0));

    setVertex(vertices[v++], a.x() - ex - ey, a.y() - ey + ex, a, b, caps);
    setVertex(vertices[v++], a.x() - ex + ey, a.y() - ey - ex, a, b, caps);
    setVertex(vertices[v++], b.x() + ex - ey, b.y() + ey + ex, a, b, caps);
    setVertex(vertices[v++], b.x() + ex + ey, b.y() + ey - ex, a, b, caps);
  }
}

/**
 * @brief Fills the whole index buffer with the per-quad pattern (0 1 2, 2 1 3). The pattern does
 *        not depend on the curve, so it is written only when the buffer is (re)allocated, never
 *        per frame; slots past the last whole quad are zero, i.e. degenerate.
 */
void Widgets::SdfStroke::writeIndexPattern(QSGGeometry* geometry)
{
  SS_ASSERT(geometry != nullptr, return);
  SS_ASSERT(geometry->indexType() == QSGGeometry::UnsignedIntType, return);

  quint32* indices    = geometry->indexDataAsUInt();
  const int quads     = std::min(geometry->vertexCount() / kVerticesPerSegment,
                                 geometry->indexCount() / kIndicesPerSegment);
  const int written   = quads * kIndicesPerSegment;
  const int available = geometry->indexCount();

  for (int q = 0; q < quads; ++q) {
    const auto base = static_cast<quint32>(q * kVerticesPerSegment);
    quint32* out    = indices + (q * kIndicesPerSegment);
    out[0]          = base;
    out[1]          = base + 1;
    out[2]          = base + 2;
    out[3]          = base + 2;
    out[4]          = base + 1;
    out[5]          = base + 3;
  }

  std::fill(indices + written, indices + available, 0u);
}

/**
 * @brief Makes every vertex past @p vertices repeat the last real one, so the spare quads the
 *        static index pattern still references have zero area and draw nothing, and the batch
 *        bounds stay inside the curve.
 */
void Widgets::SdfStroke::padTail(QSGGeometry* geometry, const int vertices)
{
  SS_ASSERT(geometry != nullptr, return);
  SS_ASSERT(vertices > 0 && geometry->vertexCount() >= vertices, return);

  auto* data         = static_cast<Vertex*>(geometry->vertexData());
  const int capacity = geometry->vertexCount();
  std::fill(data + vertices, data + capacity, data[vertices - 1]);
}
