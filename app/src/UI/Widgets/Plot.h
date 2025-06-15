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

#include "JSON/Dataset.h"

namespace Widgets
{
/**
 * @brief A widget that displays a real-time plot of data points.
 */
class Plot : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(QString yLabel READ yLabel CONSTANT)
  Q_PROPERTY(QString xLabel READ xLabel CONSTANT)
  Q_PROPERTY(double minX READ minX NOTIFY rangeChanged)
  Q_PROPERTY(double maxX READ maxX NOTIFY rangeChanged)
  Q_PROPERTY(double minY READ minY NOTIFY rangeChanged)
  Q_PROPERTY(double maxY READ maxY NOTIFY rangeChanged)
  Q_PROPERTY(double xTickInterval READ xTickInterval NOTIFY rangeChanged)
  Q_PROPERTY(double yTickInterval READ yTickInterval NOTIFY rangeChanged)

signals:
  void rangeChanged();

public:
  explicit Plot(const int index = -1, QQuickItem *parent = nullptr);
  ~Plot()
  {
    m_data.clear();
    m_data.squeeze();
  }

  [[nodiscard]] double minX() const;
  [[nodiscard]] double maxX() const;
  [[nodiscard]] double minY() const;
  [[nodiscard]] double maxY() const;
  [[nodiscard]] double xTickInterval() const;
  [[nodiscard]] double yTickInterval() const;
  [[nodiscard]] const QString &yLabel() const;
  [[nodiscard]] const QString &xLabel() const;

public slots:
  void draw(QXYSeries *series);

private slots:
  void updateData();
  void updateRange();
  void calculateAutoScaleRange();

private:
  template<typename Extractor>
  bool computeMinMaxValues(double &min, double &max,
                           const JSON::Dataset &dataset, const bool addPadding,
                           Extractor extractor);

private:
  int m_index;
  double m_minX;
  double m_maxX;
  double m_minY;
  double m_maxY;
  QString m_yLabel;
  QString m_xLabel;
  QVector<QPointF> m_data;
};
} // namespace Widgets
