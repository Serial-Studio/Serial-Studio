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

#include <QQuickItem>

namespace Widgets
{
/**
 * @brief A widget that displays the GPS data on a map.
 */
class GPS : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(qreal altitude READ altitude NOTIFY updated)
  Q_PROPERTY(qreal latitude READ latitude NOTIFY updated)
  Q_PROPERTY(qreal longitude READ longitude NOTIFY updated)

signals:
  void updated();

public:
  GPS(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] qreal altitude() const;
  [[nodiscard]] qreal latitude() const;
  [[nodiscard]] qreal longitude() const;

private slots:
  void updateData();

private:
  int m_index;
  qreal m_altitude;
  qreal m_latitude;
  qreal m_longitude;
};
} // namespace Widgets
