/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
