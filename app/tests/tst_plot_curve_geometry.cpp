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

#include <QSGGeometry>
#include <QTest>

#include "UI/Widgets/GpuStroke.h"

// Grow-only plot geometry (spec 0075, N4). A QSGGeometry has one pair of counts over one buffer
// and no separate capacity, so the old "allocate whenever the count moved" reallocated every
// frame: the round-join fan count is data dependent, so a live curve moved it constantly and Qt
// freed and malloc'd a few hundred KB per curve per frame. reserveGeometry() keeps the counts
// still at a headroom capacity and padGeometryTail() makes the unused tail draw nothing. The
// reallocation is the thing to count, which is why reserveGeometry() answers whether it happened
// -- that boolean is the seam this suite drives.

class PlotCurveGeometryTest : public QObject {
  Q_OBJECT

private slots:
  void firstReserveAllocatesOnce();
  void stationaryCountsNeverReallocate();
  void wobblingCountsStayInsideTheHeadroom();
  void exceedingTheHeadroomGrowsOnce();
  void capacityNeverShrinks();
  void indexCapacityHoldsWholeTriangles();
  void paddedIndicesAreDegenerate();
  void paddedVerticesRepeatTheLastRealOne();

private:
  [[nodiscard]] static QSGGeometry* makeIndexedGeometry();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the exact geometry shape every stroke node uses: colored 2D points, 32-bit
 *        indices, drawn as triangles.
 */
QSGGeometry* PlotCurveGeometryTest::makeIndexedGeometry()
{
  auto* geometry = new QSGGeometry(
    QSGGeometry::defaultAttributes_ColoredPoint2D(), 0, 0, QSGGeometry::UnsignedIntType);
  geometry->setDrawingMode(QSGGeometry::DrawTriangles);
  return geometry;
}

//--------------------------------------------------------------------------------------------------
// Allocation behaviour
//--------------------------------------------------------------------------------------------------

/**
 * @brief The first frame allocates, and it allocates headroom rather than the exact count.
 */
void PlotCurveGeometryTest::firstReserveAllocatesOnce()
{
  const QScopedPointer<QSGGeometry> geometry(makeIndexedGeometry());

  QVERIFY(Widgets::GpuStroke::reserveGeometry(geometry.data(), 800, 1800));
  QVERIFY(geometry->vertexCount() >= 800);
  QVERIFY(geometry->indexCount() >= 1800);
  QVERIFY(geometry->vertexCount() > 800);
}

/**
 * @brief A hundred frames of an unchanged point count allocate nothing after the first. This is
 *        the finding's scenario: a live curve whose sample count is stable.
 */
void PlotCurveGeometryTest::stationaryCountsNeverReallocate()
{
  const QScopedPointer<QSGGeometry> geometry(makeIndexedGeometry());

  int reallocations = 0;
  for (int frame = 0; frame < 100; ++frame)
    if (Widgets::GpuStroke::reserveGeometry(geometry.data(), 800, 1800))
      ++reallocations;

  QCOMPARE(reallocations, 1);
}

/**
 * @brief The data-dependent wobble the round-join fans produce stays inside the headroom, which is
 *        what the headroom exists for.
 */
void PlotCurveGeometryTest::wobblingCountsStayInsideTheHeadroom()
{
  const QScopedPointer<QSGGeometry> geometry(makeIndexedGeometry());

  int reallocations = 0;
  for (int frame = 0; frame < 100; ++frame) {
    const int vertices = 800 + (frame % 7) * 3;
    const int indices  = 1800 + (frame % 7) * 9;
    if (Widgets::GpuStroke::reserveGeometry(geometry.data(), vertices, indices))
      ++reallocations;
  }

  QCOMPARE(reallocations, 1);
}

/**
 * @brief A genuine growth past the headroom reallocates once and then settles again.
 */
void PlotCurveGeometryTest::exceedingTheHeadroomGrowsOnce()
{
  const QScopedPointer<QSGGeometry> geometry(makeIndexedGeometry());

  QVERIFY(Widgets::GpuStroke::reserveGeometry(geometry.data(), 100, 300));
  QVERIFY(Widgets::GpuStroke::reserveGeometry(geometry.data(), 5000, 15000));
  QVERIFY(!Widgets::GpuStroke::reserveGeometry(geometry.data(), 5000, 15000));
  QVERIFY(!Widgets::GpuStroke::reserveGeometry(geometry.data(), 4800, 14400));
}

/**
 * @brief A shrinking curve keeps the buffer it grew, so a zoom-out followed by a zoom-in costs no
 *        allocation at all.
 */
void PlotCurveGeometryTest::capacityNeverShrinks()
{
  const QScopedPointer<QSGGeometry> geometry(makeIndexedGeometry());

  QVERIFY(Widgets::GpuStroke::reserveGeometry(geometry.data(), 5000, 15000));
  const int vertexCapacity = geometry->vertexCount();
  const int indexCapacity  = geometry->indexCount();

  QVERIFY(!Widgets::GpuStroke::reserveGeometry(geometry.data(), 8, 18));
  QCOMPARE(geometry->vertexCount(), vertexCapacity);
  QCOMPARE(geometry->indexCount(), indexCapacity);
}

/**
 * @brief The index capacity is a whole number of triangles, so the padded tail can never leave a
 *        partial primitive at the end of the buffer.
 */
void PlotCurveGeometryTest::indexCapacityHoldsWholeTriangles()
{
  const QScopedPointer<QSGGeometry> geometry(makeIndexedGeometry());

  QVERIFY(Widgets::GpuStroke::reserveGeometry(geometry.data(), 800, 1801));
  QCOMPARE(geometry->indexCount() % 3, 0);
}

//--------------------------------------------------------------------------------------------------
// Tail padding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every index past the used count collapses its triangle to zero area, so the spare
 *        capacity draws nothing even though the renderer walks it.
 */
void PlotCurveGeometryTest::paddedIndicesAreDegenerate()
{
  const QScopedPointer<QSGGeometry> geometry(makeIndexedGeometry());
  QVERIFY(Widgets::GpuStroke::reserveGeometry(geometry.data(), 8, 18));

  auto* vertices = geometry->vertexDataAsColoredPoint2D();
  for (int i = 0; i < geometry->vertexCount(); ++i)
    vertices[i].set(static_cast<float>(i), static_cast<float>(i), 255, 255, 255, 255);

  quint32* indices = geometry->indexDataAsUInt();
  for (int i = 0; i < geometry->indexCount(); ++i)
    indices[i] = 7;

  Widgets::GpuStroke::padGeometryTail(geometry.data(), 8, 18);

  for (int i = 0; i < 18; ++i)
    QCOMPARE(indices[i], 7u);

  for (int i = 18; i < geometry->indexCount(); ++i)
    QCOMPARE(indices[i], 0u);
}

/**
 * @brief Spare vertices repeat the last real one instead of holding uninitialised memory, which
 *        keeps the renderer's batch bounds inside the curve.
 */
void PlotCurveGeometryTest::paddedVerticesRepeatTheLastRealOne()
{
  const QScopedPointer<QSGGeometry> geometry(makeIndexedGeometry());
  QVERIFY(Widgets::GpuStroke::reserveGeometry(geometry.data(), 8, 18));

  auto* vertices = geometry->vertexDataAsColoredPoint2D();
  for (int i = 0; i < geometry->vertexCount(); ++i)
    vertices[i].set(static_cast<float>(1000 + i), static_cast<float>(2000 + i), 1, 2, 3, 4);

  Widgets::GpuStroke::padGeometryTail(geometry.data(), 8, 18);

  for (int i = 8; i < geometry->vertexCount(); ++i) {
    QCOMPARE(vertices[i].x, vertices[7].x);
    QCOMPARE(vertices[i].y, vertices[7].y);
  }
}

QTEST_GUILESS_MAIN(PlotCurveGeometryTest)

#include "tst_plot_curve_geometry.moc"
