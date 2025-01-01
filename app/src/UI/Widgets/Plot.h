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

#pragma once

#include <QtQuick>
#include <QVector>
#include <QLineSeries>

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
  Q_PROPERTY(qreal minX READ minX NOTIFY rangeChanged)
  Q_PROPERTY(qreal maxX READ maxX NOTIFY rangeChanged)
  Q_PROPERTY(qreal minY READ minY NOTIFY rangeChanged)
  Q_PROPERTY(qreal maxY READ maxY NOTIFY rangeChanged)
  Q_PROPERTY(qreal xTickInterval READ xTickInterval NOTIFY rangeChanged)
  Q_PROPERTY(qreal yTickInterval READ yTickInterval NOTIFY rangeChanged)

signals:
  void rangeChanged();

public:
  explicit Plot(const int index = -1, QQuickItem *parent = nullptr);
  ~Plot()
  {
    m_data.clear();
    m_data.squeeze();
  }

  [[nodiscard]] qreal minX() const;
  [[nodiscard]] qreal maxX() const;
  [[nodiscard]] qreal minY() const;
  [[nodiscard]] qreal maxY() const;
  [[nodiscard]] qreal xTickInterval() const;
  [[nodiscard]] qreal yTickInterval() const;
  [[nodiscard]] const QString &yLabel() const;
  [[nodiscard]] const QString &xLabel() const;

public slots:
  void draw(QLineSeries *series);

private slots:
  void updateData();
  void updateRange();
  void calculateAutoScaleRange();

private:
  template<typename Extractor>
  bool computeMinMaxValues(qreal &min, qreal &max, const JSON::Dataset &dataset,
                           const bool addPadding, Extractor extractor);

private:
  int m_index;
  qreal m_minX;
  qreal m_maxX;
  qreal m_minY;
  qreal m_maxY;
  QString m_yLabel;
  QString m_xLabel;
  QVector<QPointF> m_data;
};
} // namespace Widgets
