/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
