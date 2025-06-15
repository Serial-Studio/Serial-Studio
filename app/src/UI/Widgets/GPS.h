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
 * @brief A widget that displays the GPS data on a map.
 */
class GPS : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(double altitude READ altitude NOTIFY updated)
  Q_PROPERTY(double latitude READ latitude NOTIFY updated)
  Q_PROPERTY(double longitude READ longitude NOTIFY updated)

signals:
  void updated();

public:
  GPS(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] double altitude() const;
  [[nodiscard]] double latitude() const;
  [[nodiscard]] double longitude() const;

private slots:
  void updateData();

private:
  int m_index;
  double m_altitude;
  double m_latitude;
  double m_longitude;
};
} // namespace Widgets
