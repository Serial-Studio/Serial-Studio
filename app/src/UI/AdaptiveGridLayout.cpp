/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "AdaptiveGridLayout.h"

/**
 * @brief Constructs an AdaptiveGridLayout instance.
 * @param parent The parent QQuickItem.
 */
UI::AdaptiveGridLayout::AdaptiveGridLayout(QQuickItem *parent)
  : QQuickItem(parent)
  , m_columns(1)
  , m_cellWidth(0)
  , m_cellHeight(0)
  , m_cellCount(0)
  , m_minColumns(1)
  , m_maxColumns(1)
  , m_rowSpacing(0)
  , m_minCellWidth(0)
  , m_columnSpacing(0)
  , m_containerWidth(0)
  , m_containerHeight(0)
  , m_cellHeightFactor(0.66)
{
  connect(this, &UI::AdaptiveGridLayout::inputChanged, this,
          &UI::AdaptiveGridLayout::computeLayout);
}

/**
 * @brief Destroys the AdaptiveGridLayout instance.
 */
UI::AdaptiveGridLayout::~AdaptiveGridLayout() {}

/**
 * @brief Returns the number of columns in the layout.
 */
int UI::AdaptiveGridLayout::columns() const
{
  return m_columns;
}

/**
 * @brief Returns the computed cell width.
 */
int UI::AdaptiveGridLayout::cellWidth() const
{
  return m_cellWidth;
}

/**
 * @brief Returns the computed cell height.
 */
int UI::AdaptiveGridLayout::cellHeight() const
{
  return m_cellHeight;
}

/**
 * @brief Returns the total number of cells.
 */
int UI::AdaptiveGridLayout::cellCount() const
{
  return m_cellCount;
}

/**
 * @brief Returns the minimum allowed number of columns.
 */
int UI::AdaptiveGridLayout::minColumns() const
{
  return m_minColumns;
}

/**
 * @brief Returns the maximum allowed number of columns.
 */
int UI::AdaptiveGridLayout::maxColumns() const
{
  return m_maxColumns;
}

/**
 * @brief Returns the spacing between rows.
 */
int UI::AdaptiveGridLayout::rowSpacing() const
{
  return m_rowSpacing;
}

/**
 * @brief Returns the minimum allowed cell width.
 */
int UI::AdaptiveGridLayout::minCellWidth() const
{
  return m_minCellWidth;
}

/**
 * @brief Returns the spacing between columns.
 */
int UI::AdaptiveGridLayout::columnSpacing() const
{
  return m_columnSpacing;
}

/**
 * @brief Returns the container width.
 */
int UI::AdaptiveGridLayout::containerWidth() const
{
  return m_containerWidth;
}

/**
 * @brief Returns the container height.
 */
int UI::AdaptiveGridLayout::containerHeight() const
{
  return m_containerHeight;
}

/**
 * @brief Returns the factor used to compute cell height based on cell width.
 */
qreal UI::AdaptiveGridLayout::cellHeightFactor() const
{
  return m_cellHeightFactor;
}

/**
 * @brief Sets the total number of cells and triggers layout recalculation.
 * @param count The new number of cells.
 */
void UI::AdaptiveGridLayout::setCellCount(const int count)
{
  if (m_cellCount != count)
  {
    m_cellCount = count;
    Q_EMIT inputChanged();
  }
}

/**
 * @brief Sets the minimum number of columns and triggers layout recalculation.
 * @param columns The new minimum column count.
 */
void UI::AdaptiveGridLayout::setMinColumns(const int columns)
{
  if (m_minColumns != columns && columns >= 1)
  {
    m_minColumns = columns;
    Q_EMIT inputChanged();
  }
}

/**
 * @brief Sets the maximum number of columns and triggers layout recalculation.
 * @param columns The new maximum column count.
 */
void UI::AdaptiveGridLayout::setMaxColumns(const int columns)
{
  if (m_maxColumns != columns && columns >= 1)
  {
    m_maxColumns = columns;
    Q_EMIT inputChanged();
  }
}

/**
 * @brief Sets the maximum number of columns and triggers layout recalculation.
 * @param columns The new maximum column count.
 */
void UI::AdaptiveGridLayout::setRowSpacing(const int spacing)
{
  if (m_rowSpacing != spacing)
  {
    m_rowSpacing = spacing;
    Q_EMIT inputChanged();
  }
}

/**
 * @brief Sets the row spacing and triggers layout recalculation.
 * @param spacing The new row spacing.
 */
void UI::AdaptiveGridLayout::setMinCellWidth(const int minWidth)
{
  if (m_minCellWidth != minWidth)
  {
    m_minCellWidth = minWidth;
    Q_EMIT inputChanged();
  }
}

/**
 * @brief Sets the minimum cell width and triggers layout recalculation.
 * @param minWidth The new minimum cell width.
 */
void UI::AdaptiveGridLayout::setColumnSpacing(const int spacing)
{
  if (m_columnSpacing != spacing)
  {
    m_columnSpacing = spacing;
    Q_EMIT inputChanged();
  }
}

/**
 * @brief Sets the container width and triggers layout recalculation.
 * @param containerWidth The new container width.
 */
void UI::AdaptiveGridLayout::setContainerWidth(const int containerWidth)
{
  if (m_containerWidth != containerWidth)
  {
    m_containerWidth = containerWidth;
    Q_EMIT inputChanged();
  }
}

/**
 * @brief Sets the container height and triggers layout recalculation.
 * @param containerHeight The new container height.
 */
void UI::AdaptiveGridLayout::setContainerHeight(const int containerHeight)
{
  if (m_containerHeight != containerHeight)
  {
    m_containerHeight = containerHeight;
    Q_EMIT inputChanged();
  }
}

/**
 * @brief Sets the factor used to compute cell height based on cell width.
 * @param factor The new height factor.
 */
void UI::AdaptiveGridLayout::setCellHeightFactor(const qreal cellHeightFactor)
{
  if (m_cellHeightFactor != cellHeightFactor)
  {
    m_cellHeightFactor = cellHeightFactor;
    Q_EMIT inputChanged();
  }
}

/**
 * @brief Computes the optimal grid layout based on the given constraints.
 *
 * This function determines the best number of columns and cell sizes
 * to minimize wasted space within the container.
 *
 * It updates the computed cell width, height, and column count accordingly.
 */
void UI::AdaptiveGridLayout::computeLayout()
{
  // Initialize layout counters
  int bestAreaDiff = INT_MAX, bestColumns = 1, bestCellWidth = m_minCellWidth;

  // Calculate setup that obtains the least wasted space
  const int containerArea = m_containerWidth * m_containerHeight;
  for (int c = m_minColumns; c <= m_maxColumns; ++c)
  {
    // Compute cell width, height & area
    const int cW = (m_containerWidth / c);
    const int cH = qCeil(cW * m_cellHeightFactor);
    const int cA = cW * cH + (m_columnSpacing * m_rowSpacing);

    // Obtain difference between container area & area ocuppied by cells
    const int diff = qAbs(containerArea - cA * m_cellCount);

    // Early stop if cell width is smaller than minimum cell width
    if (cW <= m_minCellWidth && bestCellWidth != INT_MAX)
      break;

    // Stop at first iteration if required
    else if (cW <= m_minCellWidth)
    {
      bestColumns = 1;
      bestCellWidth = m_containerWidth;
      break;
    }

    // Store the setup that wastes least space
    else if (diff <= bestAreaDiff)
    {
      bestColumns = c;
      bestAreaDiff = diff;
      bestCellWidth = cW - m_columnSpacing;
    }
  }

  // Update column count
  m_columns = bestColumns;

  // Update cell width
  if (m_columns > 1)
    m_cellWidth = bestCellWidth;
  else
    m_cellWidth = m_containerWidth;

  // Update cell height
  m_cellHeight = qCeil(m_cellWidth * m_cellHeightFactor);

  // Update user interface
  Q_EMIT layoutChanged();
}
