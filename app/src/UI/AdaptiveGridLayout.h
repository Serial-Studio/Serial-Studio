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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

#include <QQuickItem>

namespace UI
{
/**
 * @class UI::AdaptiveGridLayout
 * @brief A dynamic grid layout that adapts to fit a container with optimal cell
 *        sizes.
 *
 * This class computes an adaptive grid layout that ensures an efficient use of
 * space while maintaining configurable constraints such as minimum cell width,
 * column count, and spacing.
 */
class AdaptiveGridLayout : public QQuickItem
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int columns
             READ columns
             NOTIFY layoutChanged)
  Q_PROPERTY(int cellWidth
             READ cellWidth
             NOTIFY layoutChanged)
  Q_PROPERTY(int cellHeight
             READ cellHeight
             NOTIFY layoutChanged)
  Q_PROPERTY(int cellCount
             READ cellCount
             WRITE setCellCount
             NOTIFY inputChanged)
  Q_PROPERTY(int minColumns
             READ minColumns
             WRITE setMinColumns
             NOTIFY inputChanged)
  Q_PROPERTY(int maxColumns
             READ maxColumns
             WRITE setMaxColumns
             NOTIFY inputChanged)
  Q_PROPERTY(int rowSpacing
             READ rowSpacing
             WRITE setRowSpacing
             NOTIFY inputChanged)
  Q_PROPERTY(int minCellWidth
             READ minCellWidth
             WRITE setMinCellWidth
             NOTIFY inputChanged)
  Q_PROPERTY(int columnSpacing
             READ columnSpacing
             WRITE setColumnSpacing
             NOTIFY inputChanged)
  Q_PROPERTY(int containerWidth
             READ containerWidth
             WRITE setContainerWidth
             NOTIFY inputChanged)
  Q_PROPERTY(int containerHeight
             READ containerHeight
             WRITE setContainerHeight
             NOTIFY inputChanged)
  Q_PROPERTY(qreal cellHeightFactor
             READ cellHeightFactor
             WRITE setCellHeightFactor
             NOTIFY inputChanged)
  // clang-format on

signals:
  void inputChanged();
  void layoutChanged();

public:
  explicit AdaptiveGridLayout(QQuickItem *parent = nullptr);
  ~AdaptiveGridLayout();

  int columns() const;
  int cellWidth() const;
  int cellHeight() const;

  int cellCount() const;
  int minColumns() const;
  int maxColumns() const;
  int rowSpacing() const;
  int minCellWidth() const;
  int columnSpacing() const;
  int containerWidth() const;
  int containerHeight() const;
  qreal cellHeightFactor() const;

public slots:
  void setCellCount(const int count);
  void setMinColumns(const int columns);
  void setMaxColumns(const int columns);
  void setRowSpacing(const int spacing);
  void setMinCellWidth(const int minWidth);
  void setColumnSpacing(const int spacing);
  void setContainerWidth(const int containerWidth);
  void setContainerHeight(const int containerHeight);
  void setCellHeightFactor(const qreal cellHeightFactor);

private slots:
  void computeLayout();

private:
  int m_columns;
  int m_cellWidth;
  int m_cellHeight;

  int m_cellCount;
  int m_minColumns;
  int m_maxColumns;
  int m_rowSpacing;
  int m_minCellWidth;
  int m_columnSpacing;
  int m_containerWidth;
  int m_containerHeight;
  qreal m_cellHeightFactor;
};
} // namespace UI
