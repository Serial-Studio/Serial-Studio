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
class DataGrid : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(int count READ count CONSTANT)
  Q_PROPERTY(QStringList units READ units CONSTANT)
  Q_PROPERTY(QStringList titles READ titles CONSTANT)
  Q_PROPERTY(QStringList values READ values NOTIFY updated)
  Q_PROPERTY(QList<bool> alarms READ alarms NOTIFY updated)
  Q_PROPERTY(QStringList colors READ colors NOTIFY themeChanged)

signals:
  void updated();
  void themeChanged();

public:
  explicit DataGrid(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] int count() const;
  [[nodiscard]] const QList<bool> &alarms() const;

  [[nodiscard]] const QStringList &units() const;
  [[nodiscard]] const QStringList &colors() const;
  [[nodiscard]] const QStringList &titles() const;
  [[nodiscard]] const QStringList &values() const;

private slots:
  void updateData();
  void onThemeChanged();

private:
  int m_index;
  QList<bool> m_alarms;

  QStringList m_units;
  QStringList m_titles;
  QStringList m_values;
  QStringList m_colors;
};
} // namespace Widgets
