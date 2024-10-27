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

namespace Widgets
{
/**
 * @brief A widget that displays multiple plots on a single chart.
 */
class MultiPlot : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(qreal count READ count CONSTANT)
  Q_PROPERTY(QString yLabel READ yLabel CONSTANT)
  Q_PROPERTY(qreal minX READ minX NOTIFY rangeChanged)
  Q_PROPERTY(qreal maxX READ maxX NOTIFY rangeChanged)
  Q_PROPERTY(qreal minY READ minY NOTIFY rangeChanged)
  Q_PROPERTY(qreal maxY READ maxY NOTIFY rangeChanged)
  Q_PROPERTY(QStringList colors READ colors NOTIFY themeChanged)
  Q_PROPERTY(qreal xTickInterval READ xTickInterval NOTIFY rangeChanged)
  Q_PROPERTY(qreal yTickInterval READ yTickInterval NOTIFY rangeChanged)

signals:
  void rangeChanged();
  void themeChanged();

public:
  explicit MultiPlot(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] int count() const;
  [[nodiscard]] qreal minX() const;
  [[nodiscard]] qreal maxX() const;
  [[nodiscard]] qreal minY() const;
  [[nodiscard]] qreal maxY() const;
  [[nodiscard]] qreal xTickInterval() const;
  [[nodiscard]] qreal yTickInterval() const;
  [[nodiscard]] const QString &yLabel() const;
  [[nodiscard]] const QStringList &colors() const;

public slots:
  void draw(QLineSeries *series, const int index);

private slots:
  void updateData();
  void updateRange();
  void onThemeChanged();
  void calculateAutoScaleRange();

private:
  int m_index;
  qreal m_minX;
  qreal m_maxX;
  qreal m_minY;
  qreal m_maxY;
  QString m_yLabel;
  QStringList m_colors;
  QVector<QVector<QPointF>> m_data;
};
} // namespace Widgets
