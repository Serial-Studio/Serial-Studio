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

#include <QList>
#include <QTimer>
#include <QQuickItem>

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
