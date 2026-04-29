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

#include "UI/Widgets/Gyroscope.h"

#include <cmath>

#include "DSP.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Gyroscope widget.
 *
 * Initializes the gyroscope with the given index and optional parent.
 * If the widget is valid, connects the update signal.
 *
 * @param index Index of the gyroscope in the Dashboard.
 * @param parent Optional parent QQuickItem.
 */
Widgets::Gyroscope::Gyroscope(const int index, QQuickItem* parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_yaw(0)
  , m_roll(0)
  , m_pitch(0)
  , m_displayFilterInitialized(false)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardGyroscope, m_index))
    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this, &Gyroscope::updateData);
}

//--------------------------------------------------------------------------------------------------
// Angle getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Gets the current yaw angle.
 *
 * @return Yaw angle in degrees, normalized to [-180, 180].
 */
double Widgets::Gyroscope::yaw() const
{
  return m_yaw;
}

/**
 * @brief Gets the current roll angle.
 *
 * @return Roll angle in degrees, normalized to [-180, 180].
 */
double Widgets::Gyroscope::roll() const
{
  return m_roll;
}

/**
 * @brief Gets the current pitch angle.
 *
 * @return Pitch angle in degrees, normalized to [-180, 180].
 */
double Widgets::Gyroscope::pitch() const
{
  return m_pitch;
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates gyroscope orientation values.
 *
 * Pulls new absolute-angle data from the Dashboard and drives a per-axis
 * EMA to smooth the displayed orientation. Emits `updated()` when any
 * axis changes. If the source only provides angular rates (deg/s), users
 * should integrate them to absolute angles via a dataset transform.
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
  const auto& gyro = GET_GROUP(SerialStudio::DashboardGyroscope, m_index);
  if (gyro.datasets.size() != 3)
    return;

  // Store previous orientation values
  const double previousYaw   = m_yaw;
  const double previousRoll  = m_roll;
  const double previousPitch = m_pitch;

  // Axis detection helpers
  auto isYaw = [](const QString& w) {
    return w == QStringLiteral("z") || w == QStringLiteral("yaw");
  };
  auto isRoll = [](const QString& w) {
    return w == QStringLiteral("y") || w == QStringLiteral("roll");
  };
  auto isPitch = [](const QString& w) {
    return w == QStringLiteral("x") || w == QStringLiteral("pitch");
  };

  // Normalize angle helper [-180, 180]
  auto normalizeAngle = [](double angle) -> double {
    angle = std::fmod(angle + 180.0, 360.0);
    if (angle < 0)
      angle += 360.0;

    return angle - 180.0;
  };

  // Collect current axis values before assignment
  double yawInput   = 0;
  double rollInput  = 0;
  double pitchInput = 0;
  bool hasYaw       = false;
  bool hasRoll      = false;
  bool hasPitch     = false;

  for (int i = 0; i < 3; ++i) {
    const auto& dataset = gyro.datasets[i];
    const auto widget   = dataset.widget.trimmed().toLower();
    const auto value    = dataset.numericValue;

    if (isYaw(widget)) {
      yawInput = value;
      hasYaw   = true;
    } else if (isRoll(widget)) {
      rollInput = value;
      hasRoll   = true;
    } else if (isPitch(widget)) {
      pitchInput = value;
      hasPitch   = true;
    }
  }

  // Update the values with EMA filtering for smooth display
  constexpr double kAlpha = 0.4;
  if (!m_displayFilterInitialized) {
    if (hasYaw)
      m_yaw = yawInput;

    if (hasRoll)
      m_roll = rollInput;

    if (hasPitch)
      m_pitch = pitchInput;

    m_displayFilterInitialized = true;
  } else {
    auto angleEma = [](double input, double& state) {
      double delta = input - state;
      if (delta > 180.0)
        delta -= 360.0;
      else if (delta < -180.0)
        delta += 360.0;

      state += kAlpha * delta;
    };

    if (hasYaw)
      angleEma(yawInput, m_yaw);

    if (hasRoll)
      angleEma(rollInput, m_roll);

    if (hasPitch)
      angleEma(pitchInput, m_pitch);
  }

  // Normalize all angles between -180 and 180
  m_yaw   = normalizeAngle(m_yaw);
  m_roll  = normalizeAngle(m_roll);
  m_pitch = normalizeAngle(m_pitch);

  // Emit signal if orientation changed
  const bool yawChanged   = DSP::notEqual(m_yaw, previousYaw);
  const bool rollChanged  = DSP::notEqual(m_roll, previousRoll);
  const bool pitchChanged = DSP::notEqual(m_pitch, previousPitch);
  if (yawChanged || rollChanged || pitchChanged)
    Q_EMIT updated();
}
