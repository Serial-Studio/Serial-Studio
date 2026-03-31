/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QRectF>
#include <QSizeF>
#include <QVector>

#include "DataModel/Frame.h"

namespace Widgets {
namespace Output {

/**
 * @brief Computes 2D bin-packed layout for output panel widgets.
 *
 * Instead of a rigid grid, this engine places widgets into columns that
 * adapt to the available space. Tall widgets (knob, ramp) can fill a
 * full column while small widgets (button, slider, toggle, textfield)
 * stack vertically beside them.
 *
 * Each widget type has specific minimum width and height requirements.
 * The engine respects these minimums and overflows vertically when the
 * available width cannot fit all columns at their minimum widths.
 */
class PanelLayout {
public:
  struct Rect {
    qreal x = 0;
    qreal y = 0;
    qreal w = 0;
    qreal h = 0;
  };

  /**
   * @brief Computes layout geometry for the given widgets.
   * @param widgets The output widgets to lay out.
   * @param width Available width in pixels.
   * @param height Available height in pixels.
   * @param gap Spacing between items in pixels.
   * @return Vector of rectangles, one per widget, in input order.
   */
  [[nodiscard]] static QVector<Rect> compute(const std::vector<DataModel::OutputWidget>& widgets,
                                             qreal width,
                                             qreal height,
                                             qreal gap = 4);

private:
  enum SizeClass {
    Small,
    Tall
  };

  struct Item {
    int originalIndex;
    SizeClass sc;
    DataModel::OutputWidgetType type;
    qreal mw;
    qreal mh;
  };

  struct Column {
    QVector<int> itemIndices;
    qreal minW      = 0;
    qreal totalMinH = 0;
  };

  [[nodiscard]] static SizeClass classify(DataModel::OutputWidgetType type);
  [[nodiscard]] static QSizeF minSize(DataModel::OutputWidgetType type);

  static QVector<Column> buildColumns(const QVector<Item>& items, int n, qreal height, qreal gap);

  static void layoutRow(QVector<Rect>& result,
                        const QVector<Column>& columns,
                        const QVector<Item>& items,
                        int colStart,
                        int colEnd,
                        qreal rowY,
                        qreal rowH,
                        qreal width,
                        qreal gap);
};

}  // namespace Output
}  // namespace Widgets
