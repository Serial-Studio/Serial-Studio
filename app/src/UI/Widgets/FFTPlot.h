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

#include <QtQuick>
#include <QVector>
#include <QLineSeries>
#include <qfouriertransformer.h>

namespace Widgets
{
/**
 * @brief A widget that plots the FFT of a dataset.
 */
class FFTPlot : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(qreal minX READ minX CONSTANT)
  Q_PROPERTY(qreal maxX READ maxX CONSTANT)
  Q_PROPERTY(qreal minY READ minY CONSTANT)
  Q_PROPERTY(qreal maxY READ maxY CONSTANT)
  Q_PROPERTY(qreal xTickInterval READ xTickInterval CONSTANT)
  Q_PROPERTY(qreal yTickInterval READ yTickInterval CONSTANT)

public:
  explicit FFTPlot(const int index = -1, QQuickItem *parent = nullptr);
  ~FFTPlot()
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

public slots:
  void draw(QLineSeries *series);

private slots:
  void updateData();

private:
  int m_size;
  int m_index;
  int m_samplingRate;

  qreal m_minX;
  qreal m_maxX;
  qreal m_minY;
  qreal m_maxY;

  QFourierTransformer m_transformer;

  QList<QPointF> m_data;
  QScopedArrayPointer<float> m_fft;
  QScopedArrayPointer<float> m_samples;
};
} // namespace Widgets
