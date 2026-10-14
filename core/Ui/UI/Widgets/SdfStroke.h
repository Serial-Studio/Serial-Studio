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

#pragma once

#include <QPointF>
#include <QSGGeometry>

namespace Widgets::SdfStroke {
/**
 * @brief One stroke vertex (spec 0097): the quad corner in item pixels, the segment it belongs
 *        to, and the flat-cap flags of the segment's ends (bit 0 = a, bit 1 = b). The fragment
 *        stage turns the distance to the segment into coverage, so one quad of four of these
 *        draws an anti-aliased, round-joined segment at any turn angle.
 */
struct Vertex {
  float x;
  float y;
  float ax;
  float ay;
  float bx;
  float by;
  float caps;
};

// Same 1 px anti-aliasing ramp as the tessellated stroke (GpuStroke), so both paths look alike
inline constexpr double kFeatherPx = 1.0;

inline constexpr int kVerticesPerSegment = 4;
inline constexpr int kIndicesPerSegment  = 6;

[[nodiscard]] const QSGGeometry::AttributeSet& attributes();

void countRun(const qsizetype len, qsizetype& vertexCount, qsizetype& indexCount);

void emitRun(Vertex* vertices,
             int& v,
             const QPointF* px,
             const qsizetype start,
             const qsizetype len,
             const double extent);

void writeIndexPattern(QSGGeometry* geometry);

void padTail(QSGGeometry* geometry, const int vertices);
}  // namespace Widgets::SdfStroke
