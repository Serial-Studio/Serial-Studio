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

#include <QQuickItem>

namespace Widgets
{
/**
 * @brief A widget that displays a gauge.
 */
class Gauge : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(QString units READ units CONSTANT)
  Q_PROPERTY(qreal value READ value NOTIFY updated)
  Q_PROPERTY(qreal minValue READ minValue CONSTANT)
  Q_PROPERTY(qreal maxValue READ maxValue CONSTANT)
  Q_PROPERTY(qreal alarmValue READ alarmValue CONSTANT)

signals:
  void updated();

public:
  explicit Gauge(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] const QString &units() const;

  [[nodiscard]] qreal value() const;
  [[nodiscard]] qreal minValue() const;
  [[nodiscard]] qreal maxValue() const;
  [[nodiscard]] qreal alarmValue() const;

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
