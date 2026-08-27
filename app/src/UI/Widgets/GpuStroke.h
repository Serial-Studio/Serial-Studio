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

#include <QColor>
#include <QPointF>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <vector>

QT_FORWARD_DECLARE_CLASS(QSGMaterial)

namespace Widgets::GpuStroke {
/**
 * @brief Geometry ceiling (16M): past it a caller drops the stroke so int scene-graph buffers
 *        can't overflow.
 */
inline constexpr qsizetype kMaxGeometry = 1 << 24;

/**
 * @brief Optional material hook, consulted only when a node is first built. Null means the
 *        stock vertex-color material, which is what every caller but the 3D plot's stereo
 *        passes wants.
 */
using MaterialFactory = QSGMaterial* (*)();

[[nodiscard]] qsizetype runLength(const QPointF* pts, const qsizetype count, qsizetype& start);

void dashPolyline(const QPointF* px,
                  const QColor* colors,
                  const qsizetype count,
                  const double onLength,
                  const double offLength,
                  std::vector<QPointF>& outPx,
                  std::vector<QColor>& outColors);

void countRun(const QPointF* px,
              const qsizetype start,
              const qsizetype len,
              qsizetype& vertexCount,
              qsizetype& indexCount);

[[nodiscard]] QSGGeometryNode* buildStrokeNode(QSGGeometryNode* node,
                                               const QPointF* px,
                                               const QColor* colors,
                                               const qsizetype count,
                                               const double halfWidth,
                                               MaterialFactory makeMaterial = nullptr);

[[nodiscard]] QSGGeometryNode* buildPointNode(QSGGeometryNode* node,
                                              const QPointF* px,
                                              const QColor* colors,
                                              const qsizetype count,
                                              const double halfSize,
                                              MaterialFactory makeMaterial = nullptr);

void emitRun(QSGGeometry::ColoredPoint2D* vertices,
             quint32* indices,
             int& v,
             int& idx,
             const QPointF* px,
             const qsizetype start,
             const qsizetype len,
             const double hw,
             const QColor& color,
             const QColor* colors = nullptr);
}  // namespace Widgets::GpuStroke
