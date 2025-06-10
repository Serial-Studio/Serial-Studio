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

#include "UI/Dashboard.h"
#include "UI/Widgets/Gyroscope.h"

/**
 * @brief Constructs a Gyroscope widget.
 *
 * Initializes the gyroscope with the given index and optional parent.
 * If the widget is valid, starts the timer and connects the update signal.
 *
 * @param index Index of the gyroscope in the Dashboard.
 * @param parent Optional parent QQuickItem.
 */
Widgets::Gyroscope::Gyroscope(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_yaw(0)
  , m_roll(0)
  , m_pitch(0)
  , m_integrateValues(true)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardGyroscope, m_index))
  {
    m_timer.start();
    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &Gyroscope::updateData);
  }
}

/**
 * @brief Gets the current yaw angle.
 *
 * @return Yaw angle in degrees, normalized to [-180, 180].
 */
qreal Widgets::Gyroscope::yaw() const
{
  return m_yaw;
}

/**
 * @brief Gets the current roll angle.
 *
 * @return Roll angle in degrees, normalized to [-180, 180].
 */
qreal Widgets::Gyroscope::roll() const
{
  return m_roll;
}

/**
 * @brief Gets the current pitch angle.
 *
 * @return Pitch angle in degrees, normalized to [-180, 180].
 */
qreal Widgets::Gyroscope::pitch() const
{
  return m_pitch;
}

/**
 * @brief Indicates whether values are being integrated over time.
 *
 * @return True if integration is enabled, false if values are used as-is.
 */
bool Widgets::Gyroscope::integrateValues() const
{
  return m_integrateValues;
}

/**
 * @brief Updates gyroscope orientation values.
 *
 * Pulls new data from the Dashboard and updates yaw, pitch, and roll
 * based on either direct values or time-integrated deltas.
 * Emits `updated()` if orientation changes.
 */
void Widgets::Gyroscope::updateData()
{
  // Exit early if widget is disabled
  if (!isEnabled())
    return;

  // Exit widget if its invalid
  if (!VALIDATE_WIDGET(SerialStudio::DashboardGyroscope, m_index))
    return;

  // Obtain group data from dashboard & validate widget count
  const auto &gyro = GET_GROUP(SerialStudio::DashboardGyroscope, m_index);
  if (gyro.datasetCount() != 3)
    return;

  // Store previous orientation values
  const qreal previousYaw = m_yaw;
  const qreal previousRoll = m_roll;
  const qreal previousPitch = m_pitch;

  // Calculate delta time for integration
  qreal deltaT = 0.0;
  if (m_integrateValues)
  {
    deltaT = qMax(1, m_timer.elapsed()) / 1000.0;
    m_timer.restart();
  }

  // Axis detection helpers
  auto isYaw = [](const QString &w) {
    return w == QStringLiteral("z") || w == QStringLiteral("yaw");
  };
  auto isRoll = [](const QString &w) {
    return w == QStringLiteral("y") || w == QStringLiteral("roll");
  };
  auto isPitch = [](const QString &w) {
    return w == QStringLiteral("x") || w == QStringLiteral("pitch");
  };

  // Normalize angle helper [-180, 180]
  auto normalizeAngle = [](qreal angle) -> qreal {
    angle = std::fmod(angle + 180.0, 360.0);
    if (angle < 0)
      angle += 360.0;
    return angle - 180.0;
  };

  // Update angles
  for (int i = 0; i < 3; ++i)
  {
    // Obtain dataset values & widget type
    const auto &dataset = gyro.getDataset(i);
    const auto &widget = dataset.widget();
    const auto angle = dataset.value().toDouble();

    // Continously integrate the values
    if (m_integrateValues)
    {
      if (isYaw(widget))
        m_yaw += angle * deltaT;
      else if (isRoll(widget))
        m_roll += angle * deltaT;
      else if (isPitch(widget))
        m_pitch += angle * deltaT;
    }

    // Update the values directly
    else
    {
      if (isYaw(widget))
        m_yaw = angle;
      else if (isRoll(widget))
        m_roll = angle;
      else if (isPitch(widget))
        m_pitch = angle;
    }
  }

  // Normalize all angles between -180 and 180
  m_yaw = normalizeAngle(m_yaw);
  m_roll = normalizeAngle(m_roll);
  m_pitch = normalizeAngle(m_pitch);

  // Emit signal if orientation changed
  const bool yawChanged = !qFuzzyCompare(m_yaw, previousYaw);
  const bool rollChanged = !qFuzzyCompare(m_roll, previousRoll);
  const bool pitchChanged = !qFuzzyCompare(m_pitch, previousPitch);
  if (yawChanged || rollChanged || pitchChanged)
    Q_EMIT updated();
}

/**
 * @brief Enables or disables value integration.
 *
 * Resets orientation state when toggled. Emits `updated()` and
 * `integrateValuesChanged()` signals.
 *
 * @param enabled True to enable integration, false for direct values.
 */
void Widgets::Gyroscope::setIntegrateValues(const bool enabled)
{
  if (m_integrateValues != enabled)
  {
    m_yaw = 0;
    m_roll = 0;
    m_pitch = 0;
    m_timer.restart();
    m_integrateValues = enabled;

    Q_EMIT updated();
    Q_EMIT integrateValuesChanged();
  }
}
