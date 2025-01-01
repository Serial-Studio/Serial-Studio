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
