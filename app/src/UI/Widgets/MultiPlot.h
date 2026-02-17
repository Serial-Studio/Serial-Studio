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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QQuickItem>
#include <QVector>
#include <QXYSeries>

namespace Widgets {
/**
 * @class Widgets::MultiPlot
 * @brief Multi-curve plotting widget for visualizing multiple datasets on a
 *        shared chart.
 *
 * The MultiPlot class extends the basic plotting functionality to support
 * multiple data series (curves) displayed simultaneously on a single chart.
 * Each curve can be individually toggled, has its own color, and shares common
 * X/Y axis ranges for easy comparison.
 *
 * Key Features:
 * - **Multiple Curves**: Display multiple related datasets on one chart
 * - **Individual Control**: Toggle visibility of each curve independently
 * - **Shared Scaling**: All curves use common axis ranges for direct
 *   comparison
 * - **Color Management**: Automatic theme-aware color assignment for each
 *   curve
 * - **Performance**: Efficient rendering of multiple data series
 *   simultaneously
 * - **Dynamic Labels**: Each curve has an associated label for identification
 *
 * Typical Use Cases:
 * - Multi-sensor comparisons (e.g., temperature from multiple sources)
 * - Multi-axis data (X, Y, Z accelerometer readings)
 * - Related measurements with shared time bases
 *
 * @note The curve count is determined by the associated dataset group and
 *       cannot be changed after initialization.
 */
class MultiPlot : public QQuickItem {
  Q_OBJECT
  Q_PROPERTY(double count READ count CONSTANT)
  Q_PROPERTY(QString yLabel READ yLabel CONSTANT)
  Q_PROPERTY(QStringList labels READ labels CONSTANT)
  Q_PROPERTY(double minX READ minX NOTIFY rangeChanged)
  Q_PROPERTY(double maxX READ maxX NOTIFY rangeChanged)
  Q_PROPERTY(double minY READ minY NOTIFY rangeChanged)
  Q_PROPERTY(double maxY READ maxY NOTIFY rangeChanged)
  Q_PROPERTY(QStringList colors READ colors NOTIFY themeChanged)
  Q_PROPERTY(int dataW READ dataW WRITE setDataW NOTIFY dataSizeChanged)
  Q_PROPERTY(int dataH READ dataH WRITE setDataH NOTIFY dataSizeChanged)
  Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
  Q_PROPERTY(QList<bool> visibleCurves READ visibleCurves NOTIFY curvesChanged)

signals:
  void rangeChanged();
  void themeChanged();
  void curvesChanged();
  void runningChanged();
  void dataSizeChanged();

public:
  explicit MultiPlot(const int index = -1, QQuickItem* parent = nullptr);

  ~MultiPlot()
  {
    for (auto& curve : m_data) {
      curve.clear();
      curve.squeeze();
    }

    m_data.clear();
    m_data.squeeze();
  }

  [[nodiscard]] int count() const;
  [[nodiscard]] int dataW() const;
  [[nodiscard]] int dataH() const;
  [[nodiscard]] double minX() const;
  [[nodiscard]] double maxX() const;
  [[nodiscard]] double minY() const;
  [[nodiscard]] double maxY() const;
  [[nodiscard]] bool running() const;
  [[nodiscard]] const QString& yLabel() const;
  [[nodiscard]] const QStringList& colors() const;
  [[nodiscard]] const QStringList& labels() const;
  [[nodiscard]] const QList<bool>& visibleCurves() const;

public slots:
  void draw(QXYSeries* series, const int index);

  void setDataW(const int width);
  void setDataH(const int height);
  void setRunning(const bool enabled);

  void updateData();
  void updateRange();
  void calculateAutoScaleRange();
  void modifyCurveVisibility(const int index, const bool visible);

private slots:
  void onThemeChanged();

private:
  int m_index;
  int m_dataW;
  int m_dataH;
  double m_minX;
  double m_maxX;
  double m_minY;
  double m_maxY;
  QString m_yLabel;
  QStringList m_colors;
  QStringList m_labels;
  QList<int> m_drawOrders;
  QList<bool> m_visibleCurves;
  QList<QList<QPointF>> m_data;
};
}  // namespace Widgets
