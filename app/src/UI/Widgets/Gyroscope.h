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
 * @brief A widget that displays the gyroscope data on an attitude indicator.
 */
class Gyroscope : public QQuickItem
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(double yaw
             READ yaw
             NOTIFY updated)
  Q_PROPERTY(double roll
             READ roll
             NOTIFY updated)
  Q_PROPERTY(double pitch
             READ pitch
             NOTIFY updated)
  Q_PROPERTY(bool integrateValues
             READ integrateValues
             WRITE setIntegrateValues
             NOTIFY integrateValuesChanged)
  // clang-format on

signals:
  void updated();
  void integrateValuesChanged();

public:
  explicit Gyroscope(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] double yaw() const;
  [[nodiscard]] double roll() const;
  [[nodiscard]] double pitch() const;
  [[nodiscard]] bool integrateValues() const;

private slots:
  void updateData();
  void setIntegrateValues(const bool enabled);

private:
  int m_index;

  double m_yaw;
  double m_roll;
  double m_pitch;
  QElapsedTimer m_timer;

  bool m_integrateValues;
};

} // namespace Widgets
