/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

namespace UI::Layouts {

/**
 * @brief Default primary/secondary split, expressed in sixteenths (8 = 1/2).
 */
constexpr int kDefaultRatio = 8;

/**
 * @brief Denominator the stored split ratio is expressed in (the wrench ladder's finest rung).
 */
constexpr int kRatioDenominator = 16;

/**
 * @brief Largest distance between two manual-layout edges still read as one seam line.
 */
constexpr int kSeamTolerance = 6;

/**
 * @brief Arrangement a workspace tiles its widgets with. Grid reproduces the historical
 *        auto-layout exactly and is the default for every workspace that stores nothing.
 */
enum class Pattern {
  Grid,
  MasterStack,
  MasterGrid,
  Row,
  Column,
  Spiral,
};

/**
 * @brief Everything a tiling needs: the usable area, the chrome gaps, the size floor below
 *        which a pattern must stop subdividing, and the primary split in sixteenths.
 */
struct LayoutEnv {
  int margin       = 0;
  int spacing      = 0;
  int availW       = 0;
  int availH       = 0;
  bool isLandscape = true;
  int minWidth     = 1;
  int minHeight    = 1;
  int ratio        = kDefaultRatio;
};

/**
 * @brief Returns the geometry for @a count widgets under @a pattern, in widget order.
 *        Pure: identical inputs always yield identical rectangles, which is what lets the
 *        whole catalog be verified without a GUI.
 */
[[nodiscard]] QVector<QRect> tile(int count, Pattern pattern, const LayoutEnv& env);

/**
 * @brief Maps a manual layout built on @a refCanvas onto @a newCanvas, holding every join
 *        between two widgets at exactly @a spacing pixels and every outer edge flush with the
 *        canvas, whatever the size change. Widgets absorb the resize; the gaps do not scale.
 *        Pure and idempotent - rescaling onto the same canvas returns the input unchanged -
 *        which is what keeps repeated resizes from accumulating drift.
 */
[[nodiscard]] QVector<QRect> rescaleManual(const QVector<QRect>& rects,
                                           const QSize& refCanvas,
                                           const QSize& newCanvas,
                                           int spacing);

/**
 * @brief The split ratios the picker offers, in sixteenths: the wrench-ladder rungs between a
 *        quarter and three quarters, which is the range where a primary region still reads as
 *        primary. Shares the ladder the manual snapping uses rather than inventing a second one.
 */
[[nodiscard]] QVector<int> ratioStops();

[[nodiscard]] QString patternId(Pattern pattern);
[[nodiscard]] Pattern patternFromId(const QString& id);
[[nodiscard]] bool patternHasPrimary(Pattern pattern);

}  // namespace UI::Layouts
