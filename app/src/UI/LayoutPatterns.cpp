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

#include "LayoutPatterns.h"

#include <QtGlobal>
#include <QtMath>

//--------------------------------------------------------------------------------------------------
// Grid: the historical auto-layout, moved here verbatim
//--------------------------------------------------------------------------------------------------

/**
 * @brief Picks the auto-layout column count for a window count: fixed choices
 *        up to six windows, aspect-aware square-root fit beyond that.
 */
[[nodiscard]] static int autoLayoutColumnCount(const int n, const UI::Layouts::LayoutEnv& env)
{
  static constexpr int kLandscapeCols[] = {1, 1, 2, 2, 2, 3, 3};
  static constexpr int kPortraitCols[]  = {1, 1, 1, 1, 2, 2, 2};
  if (n <= 6)
    return env.isLandscape ? kLandscapeCols[n] : kPortraitCols[n];

  const double safeW = qMax(1, env.availW);
  const double safeH = qMax(1, env.availH);

  int cols;
  if (env.isLandscape) {
    cols = qCeil(qSqrt(static_cast<double>(n) * safeW / safeH));
  } else {
    const int rows = qBound(1, qCeil(qSqrt(static_cast<double>(n) * safeH / safeW)), n);
    cols           = qCeil(static_cast<double>(n) / rows);
  }

  return qBound(1, cols, n);
}

/**
 * @brief Lays out windows in a uniform cols x rows grid in row-major order;
 *        requires the window count to be an exact multiple of cols.
 */
[[nodiscard]] static QVector<QRect> tileExactGrid(const int n,
                                                  const UI::Layouts::LayoutEnv& env,
                                                  const int cols)
{
  const int rows             = n / cols;
  const int spacingForSizing = qMax(env.spacing, 0);
  const int totalCellsW      = qMax(1, env.availW - (cols - 1) * spacingForSizing);
  const int totalCellsH      = qMax(1, env.availH - (rows - 1) * spacingForSizing);
  const int baseCellW        = totalCellsW / cols;
  const int baseCellH        = totalCellsH / rows;
  const int extraW           = totalCellsW % cols;
  const int extraH           = totalCellsH % rows;

  QVector<int> colWidths(cols), colXs(cols);
  QVector<int> rowHeights(rows), rowYs(rows);

  int runningX = env.margin;
  for (int c = 0; c < cols; ++c) {
    colWidths[c]  = baseCellW + (c < extraW ? 1 : 0);
    colXs[c]      = runningX;
    runningX     += colWidths[c] + env.spacing;
  }

  int runningY = env.margin;
  for (int r = 0; r < rows; ++r) {
    rowHeights[r]  = baseCellH + (r < extraH ? 1 : 0);
    rowYs[r]       = runningY;
    runningY      += rowHeights[r] + env.spacing;
  }

  QVector<QRect> out;
  out.reserve(n);
  for (int i = 0; i < n; ++i)
    out.append(QRect(colXs[i % cols], rowYs[i / cols], colWidths[i % cols], rowHeights[i / cols]));

  return out;
}

/**
 * @brief Lays out windows in equal-width columns, column-major, when the count
 *        does not fill a grid evenly: every widget keeps the same width, the
 *        extra windows stack in the trailing columns, and each column's windows
 *        split the full canvas height evenly.
 */
[[nodiscard]] static QVector<QRect> tileUnevenColumns(const int n,
                                                      const UI::Layouts::LayoutEnv& env,
                                                      const int cols)
{
  const int spacingForSizing = qMax(env.spacing, 0);
  const int totalCellsW      = qMax(1, env.availW - (cols - 1) * spacingForSizing);
  const int baseCellW        = totalCellsW / cols;
  const int extraW           = totalCellsW % cols;
  const int baseCount        = n / cols;
  const int extraCount       = n % cols;

  QVector<QRect> out;
  out.reserve(n);

  int runningX = env.margin;
  for (int c = 0; c < cols; ++c) {
    const int colW        = baseCellW + (c < extraW ? 1 : 0);
    const int count       = baseCount + (c >= cols - extraCount ? 1 : 0);
    const int totalCellsH = qMax(1, env.availH - (count - 1) * spacingForSizing);
    const int baseCellH   = totalCellsH / count;
    const int extraH      = totalCellsH % count;

    int runningY = env.margin;
    for (int r = 0; r < count; ++r) {
      const int cellH = baseCellH + (r < extraH ? 1 : 0);
      out.append(QRect(runningX, runningY, colW, cellH));
      runningY += cellH + env.spacing;
    }

    runningX += colW + env.spacing;
  }

  return out;
}

/**
 * @brief Tiles into equal-width columns so every widget shares the same width regardless of
 *        count; exact grids keep row-major order.
 */
[[nodiscard]] static QVector<QRect> tileGrid(const int n, const UI::Layouts::LayoutEnv& env)
{
  const int cols = autoLayoutColumnCount(n, env);
  if (n % cols == 0)
    return tileExactGrid(n, env, cols);

  return tileUnevenColumns(n, env, cols);
}

//--------------------------------------------------------------------------------------------------
// Public API
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the geometry for @a count widgets under @a pattern, in widget order.
 */
QVector<QRect> UI::Layouts::tile(const int count, const Pattern pattern, const LayoutEnv& env)
{
  if (count <= 0 || env.availW <= 0 || env.availH <= 0)
    return {};

  Q_UNUSED(pattern);
  return tileGrid(count, env);
}

/**
 * @brief Returns the persisted identifier for a pattern; Grid is the empty default.
 */
QString UI::Layouts::patternId(const Pattern pattern)
{
  switch (pattern) {
    case Pattern::MasterStack:
      return QStringLiteral("master-stack");
    case Pattern::MasterGrid:
      return QStringLiteral("master-grid");
    case Pattern::Row:
      return QStringLiteral("row");
    case Pattern::Column:
      return QStringLiteral("column");
    case Pattern::Spiral:
      return QStringLiteral("spiral");
    case Pattern::Grid:
      return {};
  }

  return {};
}

/**
 * @brief Resolves a persisted identifier; anything unknown degrades to Grid rather than
 *        failing the load, so a project written by a newer build still opens.
 */
UI::Layouts::Pattern UI::Layouts::patternFromId(const QString& id)
{
  const auto key = id.trimmed().toLower();
  if (key == QLatin1String("master-stack"))
    return Pattern::MasterStack;

  if (key == QLatin1String("master-grid"))
    return Pattern::MasterGrid;

  if (key == QLatin1String("row"))
    return Pattern::Row;

  if (key == QLatin1String("column"))
    return Pattern::Column;

  if (key == QLatin1String("spiral"))
    return Pattern::Spiral;

  return Pattern::Grid;
}

/**
 * @brief Returns whether the pattern has a primary region, i.e. whether its split ratio is
 *        meaningful and the picker should offer the ratio control.
 */
bool UI::Layouts::patternHasPrimary(const Pattern pattern)
{
  return pattern == Pattern::MasterStack || pattern == Pattern::MasterGrid
      || pattern == Pattern::Spiral;
}
