/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/Output/PanelLayout.h"

#include <algorithm>
#include <cmath>
#include <QFontMetricsF>

#include "Misc/CommonFonts.h"

using namespace Widgets::Output;
using OWT = DataModel::OutputWidgetType;

//--------------------------------------------------------------------------------------------------
// Size classification helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Classifies a widget type as small (stackable) or tall (needs height).
 */
PanelLayout::SizeClass PanelLayout::classify(OWT type)
{
  switch (type) {
    case OWT::Knob:
      return Tall;
    default:
      return Small;
  }
}

/**
 * @brief Returns the minimum size (width, height) for a widget type.
 *
 * Computes sizes from actual font metrics so they scale correctly with
 * system DPI and font settings. Each height accounts for the actual
 * content the widget renders:
 *
 *   Section label:  fontH * 0.75 (small caps) + 1px separator + spacing
 *   Button:         centered button height + vertical margins
 *   Slider:         label row + slider track + range labels + value
 *   Toggle:         label + switch control
 *   TextField:      label + input row (textfield + send button)
 *   Knob:           label + square dial area + value label
 *   Ramp:           label + progress bar + value row + button row
 */
QSizeF PanelLayout::minSize(OWT type)
{
  static const auto& fonts = Misc::CommonFonts::instance();
  const QFontMetricsF fm(fonts.uiFont());
  const QFontMetricsF fmMono(fonts.monoFont());

  // Common measurements from actual fonts
  const qreal fontH = fm.height();
  const qreal monoH = fmMono.height();
  const qreal charW = fm.averageCharWidth();

  // Widget element heights
  const qreal labelH  = std::ceil(fontH * 0.75) + 1 + 2;
  const qreal btnH    = qMax(fontH + 12, 32.0);
  const qreal inputH  = qMax(monoH + 10, 28.0);
  const qreal switchH = 28.0;
  const qreal sliderH = 24.0;
  const qreal progH   = 16.0;
  const qreal rangeH  = std::ceil(fontH * 0.7);
  const qreal valueH  = std::ceil(monoH);

  // Layout constants (must match QML anchors.margins and spacing)
  const qreal m = 8;  // cell inner margin (top + bottom = 2*m)
  const qreal s = 4;  // spacing between items in ColumnLayout

  switch (type) {
    case OWT::Button:
      // [margin] button [margin]
      return {charW * 14 + 2 * m, 2 * m + btnH};

    case OWT::Slider:
      // [m] label [s] slider [s] rangeRow [s] value [m]
      return {charW * 20 + 2 * m, 2 * m + labelH + s + sliderH + s + rangeH + s + valueH};

    case OWT::Toggle:
      // [m] label [s] switch [m]
      return {charW * 12 + 2 * m, 2 * m + labelH + s + switchH};

    case OWT::TextField:
      // [m] label [s] inputRow [m]
      return {charW * 24 + 2 * m, 2 * m + labelH + s + qMax(inputH, btnH)};

    case OWT::Knob:
      // [m] label [s] dial(square) [s] value [m]
      return {charW * 16 + 2 * m, 2 * m + labelH + s + charW * 10 + s + valueH};

    case OWT::RampGenerator:
      // [m] label [s] progBar [s] valueRow [s] buttonRow [m]
      return {charW * 24 + 2 * m, 2 * m + labelH + s + progH + s + qMax(rangeH, valueH) + s + btnH};

    default:
      return {charW * 14 + 2 * m, 2 * m + btnH};
  }
}

//--------------------------------------------------------------------------------------------------
// Layout algorithm
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds columns by grouping widgets in order.
 *
 * Tall widgets get their own column (full height). Small widgets stack
 * vertically in shared columns, with the stack size computed from the
 * available height.
 */
QVector<PanelLayout::Column> PanelLayout::buildColumns(const QVector<Item>& items,
                                                       int n,
                                                       qreal height,
                                                       qreal gap)
{
  // Separate tall and small items (preserving order)
  QVector<int> tall_idx;
  QVector<int> small_idx;
  for (int i = 0; i < n; ++i)
    if (items[i].sc == Tall)
      tall_idx.append(i);
    else
      small_idx.append(i);

  // How many small items fit in one stack column?
  qreal small_avg_h = 0;
  for (int i : small_idx)
    small_avg_h += items[i].mh;

  if (!small_idx.isEmpty())
    small_avg_h /= small_idx.size();
  else
    small_avg_h = 52.0;

  const int small_per_stack =
    qMax(1, static_cast<int>(std::floor((height + gap) / (small_avg_h + gap))));

  // Build columns preserving original widget order
  QVector<Column> columns;
  int si = 0;
  int ti = 0;

  while (si < small_idx.size() || ti < tall_idx.size()) {
    int next_small = si < small_idx.size() ? small_idx[si] : n;
    int next_tall  = ti < tall_idx.size() ? tall_idx[ti] : n;

    if (next_tall < next_small) {
      Column col;
      col.itemIndices.append(next_tall);
      col.minW      = items[next_tall].mw;
      col.totalMinH = items[next_tall].mh;
      columns.append(col);
      ti++;
      continue;
    }

    Column col;
    int count = 0;
    while (si < small_idx.size() && count < small_per_stack) {
      int idx = small_idx[si];
      if (ti < tall_idx.size() && tall_idx[ti] < idx)
        break;

      col.itemIndices.append(idx);
      col.minW = qMax(col.minW, items[idx].mw);
      col.totalMinH += items[idx].mh;
      si++;
      count++;
    }

    if (!col.itemIndices.isEmpty())
      columns.append(col);
  }

  return columns;
}

/**
 * @brief Places one row of columns into the result vector.
 *
 * Distributes width proportionally to column min-widths and distributes
 * height proportionally to item min-heights within each column.
 */
void PanelLayout::layoutRow(QVector<Rect>& result,
                            const QVector<Column>& columns,
                            const QVector<Item>& items,
                            int col_start,
                            int col_end,
                            qreal row_y,
                            qreal row_h,
                            qreal width,
                            qreal gap)
{
  // Width distribution
  const int row_col_count = col_end - col_start;
  qreal row_min_w         = 0;
  for (int c = col_start; c < col_end; ++c)
    row_min_w += columns[c].minW;

  const qreal row_gap_w = (row_col_count - 1) * gap;
  const qreal avail_w   = width - row_gap_w;
  const qreal scale     = (row_min_w > 0) ? avail_w / row_min_w : 1.0;

  // Place each column
  qreal x = 0;
  for (int c = col_start; c < col_end; ++c) {
    const auto& col   = columns[c];
    qreal col_w       = qMax(col.minW, col.minW * scale);
    const int n_items = col.itemIndices.size();
    const qreal gap_h = (n_items - 1) * gap;
    const qreal avail = row_h - gap_h;

    // Heights proportional to min-heights
    qreal y = 0;
    for (int i = 0; i < n_items; ++i) {
      int idx = col.itemIndices[i];
      qreal itemH =
        (n_items == 1) ? row_h : qMax(items[idx].mh, avail * items[idx].mh / col.totalMinH);

      if (i == n_items - 1)
        itemH = row_h - y;

      result[idx] = {x, row_y + y, col_w, itemH};
      y += itemH + gap;
    }

    // Last column absorbs remaining width
    if (c == col_end - 1) {
      qreal final_w = width - x;
      for (int i = 0; i < n_items; ++i)
        result[col.itemIndices[i]].w = final_w;
    }

    x += col_w + gap;
  }
}

/**
 * @brief Computes optimal 2D placement for output widgets.
 *
 * Builds columns from the widget list, determines how many columns fit
 * per row, then lays out each row distributing width and height
 * proportionally.
 */
QVector<PanelLayout::Rect> PanelLayout::compute(const std::vector<DataModel::OutputWidget>& widgets,
                                                qreal width,
                                                qreal height,
                                                qreal gap)
{
  const int n = static_cast<int>(widgets.size());
  if (n == 0 || width <= 0 || height <= 0)
    return {};

  // Classify and get min sizes
  QVector<Item> items;
  items.reserve(n);
  for (int i = 0; i < n; ++i) {
    auto sc = classify(widgets[i].type);
    auto ms = minSize(widgets[i].type);
    items.append({i, sc, widgets[i].type, ms.width(), ms.height()});
  }

  // Build columns from items
  auto columns = buildColumns(items, n, height, gap);

  // Determine how many columns fit per row
  const int num_cols = columns.size();
  qreal total_min_w  = 0;
  for (const auto& col : columns)
    total_min_w += col.minW;

  const qreal total_with_gaps = total_min_w + (num_cols - 1) * gap;
  int cols_per_row            = num_cols;
  if (total_with_gaps > width && num_cols > 1) {
    qreal accum  = 0;
    cols_per_row = 0;
    for (int c = 0; c < num_cols; ++c) {
      qreal needed = columns[c].minW + (cols_per_row > 0 ? gap : 0);
      if (accum + needed > width && cols_per_row > 0)
        break;

      accum += needed;
      cols_per_row++;
    }

    cols_per_row = qMax(1, cols_per_row);
  }

  // Lay out in rows of columns
  QVector<Rect> result(n);
  int col_idx = 0;
  qreal row_y = 0;
  while (col_idx < num_cols) {
    int row_end = qMin(col_idx + cols_per_row, num_cols);

    // Row height from tallest column
    qreal row_min_h = 0;
    for (int c = col_idx; c < row_end; ++c) {
      qreal h   = columns[c].totalMinH + (columns[c].itemIndices.size() - 1) * gap;
      row_min_h = qMax(row_min_h, h);
    }

    // Last row stretches to fill remaining space
    qreal row_h = row_min_h;
    if (col_idx + cols_per_row >= num_cols)
      row_h = qMax(row_min_h, height - row_y);

    layoutRow(result, columns, items, col_idx, row_end, row_y, row_h, width, gap);

    row_y += row_h + gap;
    col_idx = row_end;
  }

  return result;
}
