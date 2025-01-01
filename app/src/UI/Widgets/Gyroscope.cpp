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
#include "UI/Widgets/Gyroscope.h"

/**
 * @brief Constructs a Gyroscope widget.
 * @param index The index of the gyroscope in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::Gyroscope::Gyroscope(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_yaw(0)
  , m_roll(0)
  , m_pitch(0)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardGyroscope, m_index))
  {
    m_timer.start();
    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &Gyroscope::updateData);
  }
}

/**
 * @brief Returns the yaw value of the gyroscope.
 * @return The yaw value in degrees.
 */
qreal Widgets::Gyroscope::yaw() const
{
  return m_yaw;
}

/**
 * @brief Returns the roll value of the gyroscope.
 * @return The roll value in degrees.
 */
qreal Widgets::Gyroscope::roll() const
{
  return m_roll;
}

/**
 * @brief Returns the pitch value of the gyroscope.
 * @return The pitch value in degrees.
 */
qreal Widgets::Gyroscope::pitch() const
{
  return m_pitch;
}

/**
 * @brief Updates the gyroscope data from the Dashboard.
 *
 * This method retrieves the latest data for this gyroscope from the
 * Dashboard and updates the pitch, roll, and yaw values accordingly
 * by integrating the data over time.
 */
void Widgets::Gyroscope::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardGyroscope, m_index))
  {
    // Get the gyroscope data and validate the dataset count
    const auto &gyro = GET_GROUP(SerialStudio::DashboardGyroscope, m_index);
    if (gyro.datasetCount() != 3)
      return;

    // Backup previous readings
    const qreal previousYaw = m_yaw;
    const qreal previousRoll = m_roll;
    const qreal previousPitch = m_pitch;

    // Obtain delta-T for integration
    const qreal deltaT = qMax(1, m_timer.elapsed()) / 1000.0;
    m_timer.restart();

    // Update the pitch, roll, and yaw values by integration
    for (int i = 0; i < 3; ++i)
    {
      // Obtain dataset
      const auto &dataset = gyro.getDataset(i);

      // clang-format off
      const qreal angle = dataset.value().toDouble();
      const bool isYaw = (dataset.widget() == QStringLiteral("z")) ||
                         (dataset.widget() == QStringLiteral("yaw"));
      const bool isRoll = (dataset.widget() == QStringLiteral("y")) ||
                          (dataset.widget() == QStringLiteral("roll"));
      const bool isPitch = (dataset.widget() == QStringLiteral("x")) ||
                           (dataset.widget() == QStringLiteral("pitch"));
      // clang-format on

      // Update orientation angles
      if (isYaw)
        m_yaw += angle * deltaT;
      else if (isRoll)
        m_roll += angle * deltaT;
      else if (isPitch)
        m_pitch += angle * deltaT;
    }

    // Normalize yaw angle from -180 to 180
    m_yaw = std::fmod(m_yaw + 180.0, 360.0);
    if (m_yaw < 0)
      m_yaw += 360.0;
    m_yaw -= 180.0;

    // Normalize roll angle from -180 to 180
    m_roll = std::fmod(m_roll + 180.0, 360.0);
    if (m_roll < 0)
      m_roll += 360.0;
    m_roll -= 180.0;

    // Normalize pitch angle from -180 to 180
    m_pitch = std::fmod(m_pitch + 180.0, 360.0);
    if (m_pitch < 0)
      m_pitch += 360.0;
    m_pitch -= 180.0;

    // Request a repaint of the widget
    if (!qFuzzyCompare(m_yaw, previousYaw)
        || !qFuzzyCompare(m_roll, previousRoll)
        || !qFuzzyCompare(m_pitch, previousPitch))
      Q_EMIT updated();
  }
}
