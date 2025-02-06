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
/**
 * @brief A widget that displays the gyroscope data on an attitude indicator.
 */
class Gyroscope : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(qreal yaw READ yaw NOTIFY updated)
  Q_PROPERTY(qreal roll READ roll NOTIFY updated)
  Q_PROPERTY(qreal pitch READ pitch NOTIFY updated)

  Q_PROPERTY(qreal airspeed READ airspeed NOTIFY updated) // //NEW LINE FOR AIRSPEED EDITED
  Q_PROPERTY(qreal altitude READ altitude NOTIFY updated) // //NEW LINE FOR altitude EDITED

signals:
  void updated();

public:
  explicit Gyroscope(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] qreal yaw() const;
  [[nodiscard]] qreal roll() const;
  [[nodiscard]] qreal pitch() const;
  [[nodiscard]] qreal airspeed() const; // //NEW LINE FOR AIRSPEED EDITED
  [[nodiscard]] qreal altitude() const; // //NEW LINE FOR altitude EDITED

private slots:
  void updateData();

private:
  int m_index;
  // Store the index for the airspeed dataset
  qreal m_yaw;
  qreal m_roll;
  qreal m_pitch;
  qreal m_airspeed; //NEW LINE FOR AIRSPEED EDITED
  qreal m_altitude; //NEW LINE FOR altitude EDITED
  QElapsedTimer m_timer;
};

} // namespace Widgets
