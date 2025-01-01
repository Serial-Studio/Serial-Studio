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

#include "UI/Dashboard.h"
#include "UI/Widgets/Accelerometer.h"

/**
 * @brief Constructs an Accelerometer widget.
 * @param index The index of the accelerometer in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::Accelerometer::Accelerometer(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_theta(0)
  , m_magnitude(0)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardAccelerometer, m_index))
    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &Accelerometer::updateData);
}

/**
 * @brief Returns the current G-force magnitude of the accelerometer.
 * @return The current G-force magnitude.
 */
qreal Widgets::Accelerometer::magnitude() const
{
  return m_magnitude;
}

/**
 * @brief Returns the current theta of the G-Force vector.
 */
qreal Widgets::Accelerometer::theta() const
{
  return m_theta;
}

/**
 * @brief Updates the accelerometer data from the Dashboard.
 *
 * This method retrieves the latest data for this accelerometer from the
 * Dashboard and updates the G-force value and display accordingly.
 */
void Widgets::Accelerometer::updateData()
{
  // Widget not enabled, do nothing
  if (!isEnabled())
    return;

  // Get the dashboard instance and check if the index is valid
  if (VALIDATE_WIDGET(SerialStudio::DashboardAccelerometer, m_index))
    return;

  // Get the accelerometer data and validate the dataset count
  const auto &acc = GET_GROUP(SerialStudio::DashboardAccelerometer, m_index);
  if (acc.datasetCount() != 3)
    return;

  // Obtain the X, Y, and Z acceleration values
  qreal x = 0, y = 0;
  for (int i = 0; i < 3; ++i)
  {
    auto dataset = acc.getDataset(i);
    if (dataset.widget() == QStringLiteral("x"))
      x = dataset.value().toDouble();
    else if (dataset.widget() == QStringLiteral("y"))
      y = dataset.value().toDouble();
  }

  // Calculate the radius (magnitude) using only X and Y
  const qreal r = qSqrt(qPow(x / 9.81, 2) + qPow(y / 9.81, 2));

  // Calculate the angle using atan2 for the X-Y plane
  const qreal theta = qAtan2(y, x) * (180.0 / M_PI);

  // Redraw item if required
  if (!qFuzzyCompare(r, m_magnitude) || !qFuzzyCompare(theta, m_theta))
  {
    m_theta = theta;
    m_magnitude = r;
    Q_EMIT updated();
  }
}
