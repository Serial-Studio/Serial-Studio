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

#include <QList>
#include <QTimer>
#include <QtQuick>

namespace Widgets
{
/**
 * @brief A widget that displays a panel of LEDs.
 */
class LEDPanel : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(int count READ count CONSTANT)
  Q_PROPERTY(QStringList titles READ titles CONSTANT)
  Q_PROPERTY(QList<bool> states READ states NOTIFY updated)
  Q_PROPERTY(QList<bool> alarms READ alarms NOTIFY updated)
  Q_PROPERTY(QStringList colors READ colors NOTIFY themeChanged)

signals:
  void updated();
  void themeChanged();

public:
  explicit LEDPanel(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] int count() const;
  [[nodiscard]] const QList<bool> &alarms() const;
  [[nodiscard]] const QList<bool> &states() const;
  [[nodiscard]] const QStringList &colors() const;
  [[nodiscard]] const QStringList &titles() const;

private slots:
  void updateData();
  void onAlarmTimeout();
  void onThemeChanged();

private:
  int m_index;
  QTimer m_alarmTimer;
  QList<bool> m_alarms;
  QList<bool> m_states;
  QStringList m_titles;
  QStringList m_colors;
};

} // namespace Widgets
