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
 * @brief Multi-curve plotting widget for visualizing multiple datasets on a
 *        shared chart.
 */
class MultiPlot : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int count
             READ count
             CONSTANT)
  Q_PROPERTY(bool running
             READ running
             WRITE setRunning
             NOTIFY runningChanged)
  Q_PROPERTY(int dataW
             READ dataW
             WRITE setDataW
             NOTIFY dataSizeChanged)
  Q_PROPERTY(int dataH
             READ dataH
             WRITE setDataH
             NOTIFY dataSizeChanged)
  Q_PROPERTY(double minX
             READ minX
             NOTIFY rangeChanged)
  Q_PROPERTY(double maxX
             READ maxX
             NOTIFY rangeChanged)
  Q_PROPERTY(double minY
             READ minY
             NOTIFY rangeChanged)
  Q_PROPERTY(double maxY
             READ maxY
             NOTIFY rangeChanged)
  Q_PROPERTY(QString yLabel
             READ yLabel
             CONSTANT)
  Q_PROPERTY(QStringList labels
             READ labels
             CONSTANT)
  Q_PROPERTY(QStringList colors
             READ colors
             NOTIFY themeChanged)
  Q_PROPERTY(QList<bool> visibleCurves
             READ visibleCurves
             NOTIFY curvesChanged)
  // clang-format on

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

  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] int dataW() const noexcept;
  [[nodiscard]] int dataH() const noexcept;
  [[nodiscard]] double minX() const noexcept;
  [[nodiscard]] double maxX() const noexcept;
  [[nodiscard]] double minY() const noexcept;
  [[nodiscard]] double maxY() const noexcept;
  [[nodiscard]] bool running() const noexcept;
  [[nodiscard]] const QString& yLabel() const noexcept;
  [[nodiscard]] const QStringList& colors() const noexcept;
  [[nodiscard]] const QStringList& labels() const noexcept;
  [[nodiscard]] const QList<bool>& visibleCurves() const noexcept;

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
