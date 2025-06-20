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
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <QQuickItem>

namespace Widgets
{
/**
 * @brief A widget that displays a bar/level indicator.
 */
class Bar : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(QString units READ units CONSTANT)
  Q_PROPERTY(double value READ value NOTIFY updated)
  Q_PROPERTY(double minValue READ minValue CONSTANT)
  Q_PROPERTY(double maxValue READ maxValue CONSTANT)
  Q_PROPERTY(double alarmValue READ alarmValue CONSTANT)
  Q_PROPERTY(double fractionalValue READ fractionalValue NOTIFY updated)
  Q_PROPERTY(double alarmFractionalValue READ alarmFractionalValue CONSTANT)

signals:
  void updated();

public:
  explicit Bar(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] const QString &units() const;

  [[nodiscard]] double value() const;
  [[nodiscard]] double minValue() const;
  [[nodiscard]] double maxValue() const;
  [[nodiscard]] double alarmValue() const;
  [[nodiscard]] double fractionalValue() const;
  [[nodiscard]] double alarmFractionalValue() const;

private slots:
  void updateData();

private:
  int m_index;
  QString m_units;
  double m_value;
  double m_minValue;
  double m_maxValue;
  double m_alarmValue;
};
} // namespace Widgets
