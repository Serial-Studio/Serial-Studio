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

#pragma once

#include <QVector>
#include <QXYSeries>
#include <QQuickItem>

namespace Widgets
{
/**
 * @brief A widget that displays multiple plots on a single chart.
 */
class MultiPlot : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(double count READ count CONSTANT)
  Q_PROPERTY(QString yLabel READ yLabel CONSTANT)
  Q_PROPERTY(QStringList labels READ labels CONSTANT)
  Q_PROPERTY(double minX READ minX NOTIFY rangeChanged)
  Q_PROPERTY(double maxX READ maxX NOTIFY rangeChanged)
  Q_PROPERTY(double minY READ minY NOTIFY rangeChanged)
  Q_PROPERTY(double maxY READ maxY NOTIFY rangeChanged)
  Q_PROPERTY(QStringList colors READ colors NOTIFY themeChanged)
  Q_PROPERTY(double xTickInterval READ xTickInterval NOTIFY rangeChanged)
  Q_PROPERTY(double yTickInterval READ yTickInterval NOTIFY rangeChanged)

signals:
  void rangeChanged();
  void themeChanged();

public:
  explicit MultiPlot(const int index = -1, QQuickItem *parent = nullptr);
  ~MultiPlot()
  {
    for (auto &curve : m_data)
    {
      curve.clear();
      curve.squeeze();
    }

    m_data.clear();
    m_data.squeeze();
  }

  [[nodiscard]] int count() const;
  [[nodiscard]] double minX() const;
  [[nodiscard]] double maxX() const;
  [[nodiscard]] double minY() const;
  [[nodiscard]] double maxY() const;
  [[nodiscard]] double xTickInterval() const;
  [[nodiscard]] double yTickInterval() const;
  [[nodiscard]] const QString &yLabel() const;
  [[nodiscard]] const QStringList &colors() const;
  [[nodiscard]] const QStringList &labels() const;

public slots:
  void draw(QXYSeries *series, const int index);

  void updateData();
  void updateRange();
  void calculateAutoScaleRange();

private slots:
  void onThemeChanged();

private:
  int m_index;
  double m_minX;
  double m_maxX;
  double m_minY;
  double m_maxY;
  QString m_yLabel;
  QStringList m_colors;
  QStringList m_labels;
  QVector<QVector<QPointF>> m_data;
};
} // namespace Widgets
