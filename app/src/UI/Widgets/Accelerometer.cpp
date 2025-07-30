/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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
double Widgets::Accelerometer::magnitude() const
{
  return m_magnitude;
}

/**
 * @brief Returns the current theta of the G-Force vector.
 */
double Widgets::Accelerometer::theta() const
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
  if (!VALIDATE_WIDGET(SerialStudio::DashboardAccelerometer, m_index))
    return;

  // Get the accelerometer data and validate the dataset count
  const auto &acc = GET_GROUP(SerialStudio::DashboardAccelerometer, m_index);
  if (acc.datasets.size() != 3)
    return;

  // Obtain the X, Y, and Z acceleration values
  double x = 0, y = 0;
  for (int i = 0; i < 3; ++i)
  {
    auto dataset = acc.datasets[i];
    if (dataset.widget == QStringLiteral("x"))
      x = dataset.numericValue;
    else if (dataset.widget == QStringLiteral("y"))
      y = dataset.numericValue;
  }

  // Calculate the radius (magnitude) using only X and Y
  const double r = qSqrt(qPow(x / 9.81, 2) + qPow(y / 9.81, 2));

  // Calculate the angle using atan2 for the X-Y plane
  const double theta = qAtan2(y, x) * (180.0 / M_PI);

  // Redraw item if required
  if (!qFuzzyCompare(r, m_magnitude) || !qFuzzyCompare(theta, m_theta))
  {
    m_theta = theta;
    m_magnitude = r;
    Q_EMIT updated();
  }
}
