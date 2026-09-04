/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QRect>
#include <QSize>
#include <QString>
#include <QVector>

namespace UI::Snap {

/**
 * @brief Pixel distance within which a smart-guide candidate captures the gesture.
 */
constexpr int kSnapThreshold = 6;

/**
 * @brief Visual classification of an alignment guide line.
 */
enum class GuideKind {
  Edge,
  Center
};

/**
 * @brief One guide line to render, expressed as a 1-px rect in canvas coordinates.
 */
struct GuideLine {
  QRect rect;
  GuideKind kind;
};

/**
 * @brief One equal-spacing gap to highlight, with its pixel value for the label.
 */
struct SpacingIndicator {
  QRect rect;
  int gap;
};

/**
 * @brief Inputs for one snap resolution: candidate geometry, canvas extents,
 *        sibling rects and snap settings. minSize bounds resize snaps only;
 *        siblingSpacing is the flush-snap gap against a neighbor. When
 *        smartGuidesEnabled is off, only grid quantization applies.
 */
struct SnapInput {
  QRect rect;
  QSize canvas;
  QVector<QRect> siblings;
  int threshold;
  int minSize;
  bool gridEnabled;
  int gridSize;
  int siblingSpacing;
  bool smartGuidesEnabled;
};

/**
 * @brief Which edges of the geometry a resize gesture is currently moving.
 */
struct MovingEdges {
  bool left;
  bool right;
  bool top;
  bool bottom;
};

/**
 * @brief Snap resolution output: the snapped geometry plus the visuals that
 *        justify it (guides, spacing gaps, matched-size sibling).
 */
struct SnapResult {
  QRect rect;
  QRect sizeMatch;
  QVector<GuideLine> guides;
  QVector<SpacingIndicator> spacings;
};

[[nodiscard]] int snapToGrid(int value, int gridSize);
[[nodiscard]] int snapToFraction(int value, int extent, int tolerance);
[[nodiscard]] QString fractionLabel(int size, int extent);
[[nodiscard]] SnapResult resolveMoveSnap(const SnapInput& input);
[[nodiscard]] SnapResult resolveResizeSnap(const SnapInput& input, const MovingEdges& edges);

}  // namespace UI::Snap
