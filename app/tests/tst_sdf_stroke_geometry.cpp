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

#include <algorithm>
#include <cmath>
#include <QPointF>
#include <QSGGeometry>
#include <QTest>
#include <vector>

#include "UI/Widgets/GpuStroke.h"
#include "UI/Widgets/SdfStroke.h"

/**
 * @file tst_sdf_stroke_geometry.cpp
 * @brief Signed-distance stroke geometry (spec 0097 AC8): one quad per segment at any turn angle,
 *        so a dense min/max envelope (which reverses at almost every point) costs no more per
 *        segment than a straight line; flat caps flagged at run ends; a static index pattern and
 *        a degenerate padded tail on the shared grow-only buffer.
 */

class SdfStrokeGeometryTest : public QObject {
  Q_OBJECT

private slots:
  void segmentCostIgnoresTheTurnAngle();
  void denseEnvelopeIsBoundedPerColumn();
  void quadsEncloseTheirSegment();
  void runEndsAreFlaggedFlat();
  void indexPatternAndDegenerateTail();
  void steadyFramesNeverReallocate();

private:
  [[nodiscard]] static std::vector<Widgets::SdfStroke::Vertex> emitVertices(
    const std::vector<QPointF>& px, double extent);
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Emits one run over @p px and returns the vertices.
 */
std::vector<Widgets::SdfStroke::Vertex> SdfStrokeGeometryTest::emitVertices(
  const std::vector<QPointF>& px, const double extent)
{
  qsizetype vertices = 0;
  qsizetype indices  = 0;
  Widgets::SdfStroke::countRun(static_cast<qsizetype>(px.size()), vertices, indices);

  std::vector<Widgets::SdfStroke::Vertex> out(static_cast<std::size_t>(vertices));
  int v = 0;
  if (px.size() >= 2)
    Widgets::SdfStroke::emitRun(
      out.data(), v, px.data(), 0, static_cast<qsizetype>(px.size()), extent);

  return out;
}

//--------------------------------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------------------------------

/**
 * @brief A straight line, a right angle and a full reversal all cost four vertices and six
 *        indices per segment: the round join comes from the fragment stage, not from fans.
 */
void SdfStrokeGeometryTest::segmentCostIgnoresTheTurnAngle()
{
  const std::vector<std::vector<QPointF>> runs = {
    {{0, 0}, {10, 0},    {20, 0}},
    {{0, 0}, {10, 0},   {10, 10}},
    {{0, 0}, {10, 0}, {0, 0.001}},
  };

  for (const auto& run : runs) {
    qsizetype vertices = 0;
    qsizetype indices  = 0;
    Widgets::SdfStroke::countRun(static_cast<qsizetype>(run.size()), vertices, indices);
    QCOMPARE(vertices, qsizetype(8));
    QCOMPARE(indices, qsizetype(12));
    QCOMPARE(emitVertices(run, 2.0).size(), std::size_t(8));
  }
}

/**
 * @brief A min/max envelope over W columns (2W points, a reversal at nearly every one) emits at
 *        most 8W vertices: the bound is per column, independent of how dense the data is.
 */
void SdfStrokeGeometryTest::denseEnvelopeIsBoundedPerColumn()
{
  constexpr int kColumns = 1000;
  std::vector<QPointF> envelope;
  envelope.reserve(2 * kColumns);
  for (int c = 0; c < kColumns; ++c) {
    envelope.emplace_back(c + 0.25, 10.0 + (c % 7));
    envelope.emplace_back(c + 0.75, 90.0 - (c % 5));
  }

  qsizetype vertices = 0;
  qsizetype indices  = 0;
  Widgets::SdfStroke::countRun(static_cast<qsizetype>(envelope.size()), vertices, indices);
  QVERIFY(vertices <= qsizetype(8 * kColumns));
  QVERIFY(indices <= qsizetype(12 * kColumns));
}

/**
 * @brief Every quad carries its own segment and spans it plus the extent on every side, so the
 *        fragment stage has room for the cap and the feather.
 */
void SdfStrokeGeometryTest::quadsEncloseTheirSegment()
{
  const std::vector<QPointF> run = {
    { 5,  5},
    {25, 15}
  };
  const double extent = 3.0;
  const auto vertices = emitVertices(run, extent);
  QCOMPARE(vertices.size(), std::size_t(4));

  double minX = 1e9;
  double maxX = -1e9;
  double minY = 1e9;
  double maxY = -1e9;
  for (const auto& vertex : vertices) {
    QCOMPARE(vertex.ax, 5.0f);
    QCOMPARE(vertex.ay, 5.0f);
    QCOMPARE(vertex.bx, 25.0f);
    QCOMPARE(vertex.by, 15.0f);
    minX = std::min(minX, static_cast<double>(vertex.x));
    maxX = std::max(maxX, static_cast<double>(vertex.x));
    minY = std::min(minY, static_cast<double>(vertex.y));
    maxY = std::max(maxY, static_cast<double>(vertex.y));
  }

  QVERIFY(minX <= 5.0 - extent * 0.5 && maxX >= 25.0 + extent * 0.5);
  QVERIFY(minY <= 5.0 - extent * 0.5 && maxY >= 15.0 + extent * 0.5);
}

/**
 * @brief A run's first segment flags its start flat, its last flags its end flat, interior
 *        segments flag nothing (round joins), and a single-segment run flags both ends.
 */
void SdfStrokeGeometryTest::runEndsAreFlaggedFlat()
{
  const auto vertices = emitVertices(
    {
      { 0, 0},
      {10, 0},
      {20, 5},
      {30, 0}
  },
    2.0);
  QCOMPARE(vertices.size(), std::size_t(12));
  QCOMPARE(vertices[0].caps, 1.0f);
  QCOMPARE(vertices[4].caps, 0.0f);
  QCOMPARE(vertices[8].caps, 2.0f);

  const auto single = emitVertices(
    {
      { 0, 0},
      {10, 0}
  },
    2.0);
  QCOMPARE(single[0].caps, 3.0f);
}

/**
 * @brief The static pattern draws each quad as two triangles, slots past the last whole quad
 *        are zero, and the padded tail repeats the last real vertex so spare quads have no area.
 */
void SdfStrokeGeometryTest::indexPatternAndDegenerateTail()
{
  QSGGeometry geometry(Widgets::SdfStroke::attributes(), 0, 0, QSGGeometry::UnsignedIntType);
  QCOMPARE(geometry.sizeOfVertex(), int(sizeof(Widgets::SdfStroke::Vertex)));

  QVERIFY(Widgets::GpuStroke::reserveGeometry(&geometry, 8, 12));
  Widgets::SdfStroke::writeIndexPattern(&geometry);

  const quint32* indices   = geometry.indexDataAsUInt();
  const quint32 expected[] = {0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7};
  for (int i = 0; i < 12; ++i)
    QCOMPARE(indices[i], expected[i]);

  const int quads = std::min(geometry.vertexCount() / 4, geometry.indexCount() / 6);
  for (int i = quads * 6; i < geometry.indexCount(); ++i)
    QCOMPARE(indices[i], quint32(0));

  auto* data                     = static_cast<Widgets::SdfStroke::Vertex*>(geometry.vertexData());
  int v                          = 0;
  const std::vector<QPointF> run = {
    { 0, 0},
    {10, 0}
  };
  Widgets::SdfStroke::emitRun(data, v, run.data(), 0, 2, 2.0);
  Widgets::SdfStroke::padTail(&geometry, v);
  for (int i = v; i < geometry.vertexCount(); ++i) {
    QCOMPARE(data[i].x, data[v - 1].x);
    QCOMPARE(data[i].y, data[v - 1].y);
  }
}

/**
 * @brief The shared grow-only reserve holds steady across 100 frames whose segment count wobbles
 *        inside the headroom, so the per-frame path reallocates nothing.
 */
void SdfStrokeGeometryTest::steadyFramesNeverReallocate()
{
  QSGGeometry geometry(Widgets::SdfStroke::attributes(), 0, 0, QSGGeometry::UnsignedIntType);
  QVERIFY(Widgets::GpuStroke::reserveGeometry(&geometry, 4000, 6000));

  for (int frame = 0; frame < 100; ++frame) {
    const int segments = 1000 - (frame % 7);
    QVERIFY(!Widgets::GpuStroke::reserveGeometry(&geometry, segments * 4, segments * 6));
  }
}

QTEST_APPLESS_MAIN(SdfStrokeGeometryTest)

#include "tst_sdf_stroke_geometry.moc"
