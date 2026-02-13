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

#include "DSP.h"
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
  , m_x(0)
  , m_y(0)
  , m_z(0)
  , m_g(0)
  , m_pitch(0)
  , m_roll(0)
  , m_peakG(0)
  , m_theta(0)
  , m_magnitude(0)
  , m_maxG(16.0)
  , m_inputInG(false)
  , m_filterInitialized(false)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardAccelerometer, m_index))
    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &Accelerometer::updateData);
}

/**
 * @brief Returns the current X-axis acceleration in G.
 */
double Widgets::Accelerometer::accelX() const
{
  return m_x;
}

/**
 * @brief Returns the current Y-axis acceleration in G.
 */
double Widgets::Accelerometer::accelY() const
{
  return m_y;
}

/**
 * @brief Returns the current Z-axis acceleration in G.
 */
double Widgets::Accelerometer::accelZ() const
{
  return m_z;
}

/**
 * @brief Returns the total 3D G-force magnitude sqrt(x^2+y^2+z^2).
 */
double Widgets::Accelerometer::g() const
{
  return m_g;
}

/**
 * @brief Returns the pitch angle in degrees.
 *
 * Computed as atan2(x, sqrt(y^2 + z^2)).
 */
double Widgets::Accelerometer::pitch() const
{
  return m_pitch;
}

/**
 * @brief Returns the roll angle in degrees.
 *
 * Computed as atan2(y, sqrt(x^2 + z^2)).
 */
double Widgets::Accelerometer::roll() const
{
  return m_roll;
}

/**
 * @brief Returns the session peak G-force value.
 */
double Widgets::Accelerometer::peakG() const
{
  return m_peakG;
}

/**
 * @brief Returns the 2D polar angle (theta) for the X-Y plane.
 */
double Widgets::Accelerometer::theta() const
{
  return m_theta;
}

/**
 * @brief Returns the 2D polar magnitude in the X-Y plane in G.
 */
double Widgets::Accelerometer::magnitude() const
{
  return m_magnitude;
}

/**
 * @brief Returns the maximum G value for the polar plot range.
 */
double Widgets::Accelerometer::maxG() const
{
  return m_maxG;
}

/**
 * @brief Returns whether the input values are already in G-force units.
 *
 * When false (default), input values are assumed to be in m/s^2 and are
 * divided by 9.81 to convert to G.
 */
bool Widgets::Accelerometer::inputInG() const
{
  return m_inputInG;
}

/**
 * @brief Resets the peak G-force tracker to zero.
 */
void Widgets::Accelerometer::resetPeakG()
{
  m_peakG = 0;
  Q_EMIT updated();
}

/**
 * @brief Updates the accelerometer data from the Dashboard.
 *
 * Reads X, Y, Z axes from group datasets, applies unit conversion,
 * computes all derived values (magnitude, theta, pitch, roll, total G),
 * and tracks peak G-force.
 */
void Widgets::Accelerometer::updateData()
{
  if (!isEnabled())
    return;

  if (!VALIDATE_WIDGET(SerialStudio::DashboardAccelerometer, m_index))
    return;

  const auto &acc = GET_GROUP(SerialStudio::DashboardAccelerometer, m_index);
  if (acc.datasets.size() != 3)
    return;

  // Read raw axis values
  double rawX = 0, rawY = 0, rawZ = 0;
  for (int i = 0; i < 3; ++i)
  {
    const auto &dataset = acc.datasets[i];
    if (dataset.widget == QStringLiteral("x"))
      rawX = dataset.numericValue;
    else if (dataset.widget == QStringLiteral("y"))
      rawY = dataset.numericValue;
    else if (dataset.widget == QStringLiteral("z"))
      rawZ = dataset.numericValue;
  }

  // Convert to G-force
  const double factor = m_inputInG ? 1.0 : (1.0 / 9.81);
  const double gx = rawX * factor;
  const double gy = rawY * factor;
  const double gz = rawZ * factor;

  // 2D polar (X-Y plane)
  const double mag = qSqrt(gx * gx + gy * gy);
  const double theta = qAtan2(gy, gx) * (180.0 / M_PI);

  // 3D total G-force
  const double totalG = qSqrt(gx * gx + gy * gy + gz * gz);

  // Tilt angles
  const double pitchVal = qAtan2(gx, qSqrt(gy * gy + gz * gz)) * (180.0 / M_PI);
  const double rollVal = qAtan2(gy, qSqrt(gx * gx + gz * gz)) * (180.0 / M_PI);

  // Store previous filtered values for change detection
  const double prevX = m_x;
  const double prevY = m_y;
  const double prevZ = m_z;
  const double prevG = m_g;
  const double prevMag = m_magnitude;
  const double prevTheta = m_theta;
  const double prevPitch = m_pitch;
  const double prevRoll = m_roll;
  const double prevPeak = m_peakG;

  // Apply EMA filter for smooth display
  constexpr double kAlpha = 0.4;
  if (!m_filterInitialized)
  {
    m_x = gx;
    m_y = gy;
    m_z = gz;
    m_g = totalG;
    m_magnitude = mag;
    m_theta = theta;
    m_pitch = pitchVal;
    m_roll = rollVal;
    m_filterInitialized = true;
  }
  else
  {
    m_x += kAlpha * (gx - m_x);
    m_y += kAlpha * (gy - m_y);
    m_z += kAlpha * (gz - m_z);
    m_g += kAlpha * (totalG - m_g);
    m_magnitude += kAlpha * (mag - m_magnitude);
    m_pitch += kAlpha * (pitchVal - m_pitch);
    m_roll += kAlpha * (rollVal - m_roll);

    // Angle-aware EMA for theta (handles wrapping at +/-180)
    double thetaDelta = theta - m_theta;
    if (thetaDelta > 180.0)
      thetaDelta -= 360.0;
    else if (thetaDelta < -180.0)
      thetaDelta += 360.0;
    m_theta += kAlpha * thetaDelta;
  }

  // Peak tracking uses raw (unfiltered) value
  m_peakG = qMax(m_peakG, totalG);

  // Check for changes
  const bool changed = DSP::notEqual(m_x, prevX) || DSP::notEqual(m_y, prevY)
                       || DSP::notEqual(m_z, prevZ) || DSP::notEqual(m_g, prevG)
                       || DSP::notEqual(m_magnitude, prevMag)
                       || DSP::notEqual(m_theta, prevTheta)
                       || DSP::notEqual(m_pitch, prevPitch)
                       || DSP::notEqual(m_roll, prevRoll)
                       || DSP::notEqual(m_peakG, prevPeak);

  if (changed)
    Q_EMIT updated();
}

/**
 * @brief Sets the maximum G value for the polar plot range.
 *
 * @param maxG The new maximum G value (must be >= 0.5).
 */
void Widgets::Accelerometer::setMaxG(const double maxG)
{
  const double clamped = qMax(0.5, maxG);
  if (DSP::notEqual(clamped, m_maxG))
  {
    m_maxG = clamped;
    Q_EMIT configChanged();
  }
}

/**
 * @brief Enables or disables the input-in-G mode.
 *
 * When toggled, the peak tracker is reset since the unit change
 * invalidates the previous peak value.
 *
 * @param enabled True if input values are already in G-force units.
 */
void Widgets::Accelerometer::setInputInG(const bool enabled)
{
  if (m_inputInG != enabled)
  {
    m_inputInG = enabled;
    m_peakG = 0;
    m_filterInitialized = false;
    Q_EMIT configChanged();
    Q_EMIT updated();
  }
}
