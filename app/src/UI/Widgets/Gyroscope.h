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
 * @brief A widget that displays the gyroscope data on an attitude indicator.
 */
class Gyroscope : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(qreal yaw READ yaw NOTIFY updated)
  Q_PROPERTY(qreal roll READ roll NOTIFY updated)
  Q_PROPERTY(qreal pitch READ pitch NOTIFY updated)

signals:
  void updated();

public:
  explicit Gyroscope(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] qreal yaw() const;
  [[nodiscard]] qreal roll() const;
  [[nodiscard]] qreal pitch() const;

private slots:
  void updateData();

private:
  int m_index;
  qreal m_yaw;
  qreal m_roll;
  qreal m_pitch;
  QElapsedTimer m_timer;
};

} // namespace Widgets
