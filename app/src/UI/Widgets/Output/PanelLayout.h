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
 */
class PanelLayout {
public:
  struct Rect {
    qreal x = 0;
    qreal y = 0;
    qreal w = 0;
    qreal h = 0;
  };

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
