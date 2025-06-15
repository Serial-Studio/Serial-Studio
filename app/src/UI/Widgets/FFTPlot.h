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
#include <QQuickItem>
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
  Q_PROPERTY(double minX READ minX CONSTANT)
  Q_PROPERTY(double maxX READ maxX CONSTANT)
  Q_PROPERTY(double minY READ minY CONSTANT)
  Q_PROPERTY(double maxY READ maxY CONSTANT)
  Q_PROPERTY(double xTickInterval READ xTickInterval CONSTANT)
  Q_PROPERTY(double yTickInterval READ yTickInterval CONSTANT)

public:
  explicit FFTPlot(const int index = -1, QQuickItem *parent = nullptr);
  ~FFTPlot()
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

public slots:
  void draw(QLineSeries *series);

private slots:
  void updateData();

private:
  int m_size;
  int m_index;
  int m_samplingRate;

  double m_minX;
  double m_maxX;
  double m_minY;
  double m_maxY;

  QFourierTransformer m_transformer;

  QList<QPointF> m_data;
  QScopedArrayPointer<float> m_fft;
  QScopedArrayPointer<float> m_samples;
};
} // namespace Widgets
