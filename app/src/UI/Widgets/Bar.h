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

namespace Widgets
{
/**
 * @brief A widget that displays a bar/level indicator.
 */
class Bar : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(QString units READ units CONSTANT)
  Q_PROPERTY(qreal value READ value NOTIFY updated)
  Q_PROPERTY(qreal minValue READ minValue CONSTANT)
  Q_PROPERTY(qreal maxValue READ maxValue CONSTANT)
  Q_PROPERTY(qreal alarmValue READ alarmValue CONSTANT)
  Q_PROPERTY(qreal fractionalValue READ fractionalValue NOTIFY updated)
  Q_PROPERTY(qreal alarmFractionalValue READ alarmFractionalValue CONSTANT)

signals:
  void updated();

public:
  explicit Bar(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] const QString &units() const;

  [[nodiscard]] qreal value() const;
  [[nodiscard]] qreal minValue() const;
  [[nodiscard]] qreal maxValue() const;
  [[nodiscard]] qreal alarmValue() const;
  [[nodiscard]] qreal fractionalValue() const;
  [[nodiscard]] qreal alarmFractionalValue() const;

private slots:
  void updateData();

private:
  int m_index;
  QString m_units;
  qreal m_value;
  qreal m_minValue;
  qreal m_maxValue;
  qreal m_alarmValue;
};
} // namespace Widgets
