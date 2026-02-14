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
#include "UI/Widgets/Gyroscope.h"

#include <algorithm>
#include <cmath>

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
  , m_filteredYawRate(0)
  , m_filteredRollRate(0)
  , m_filteredPitchRate(0)
  , m_integrateValues(true)
  , m_rateFilterInitialized(false)
  , m_displayFilterInitialized(false)
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
  if (gyro.datasets.size() != 3)
    return;

  // Store previous orientation values
  const double previousYaw = m_yaw;
  const double previousRoll = m_roll;
  const double previousPitch = m_pitch;

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
  auto normalizeAngle = [](double angle) -> double {
    angle = std::fmod(angle + 180.0, 360.0);
    if (angle < 0)
      angle += 360.0;
    return angle - 180.0;
  };

  // Collect current axis values before integration/assignment
  double yawInput = 0;
  double rollInput = 0;
  double pitchInput = 0;
  bool hasYaw = false;
  bool hasRoll = false;
  bool hasPitch = false;

  for (int i = 0; i < 3; ++i)
  {
    const auto &dataset = gyro.datasets[i];
    const auto widget = dataset.widget.trimmed().toLower();
    const auto value = dataset.numericValue;

    if (isYaw(widget))
    {
      yawInput = value;
      hasYaw = true;
    }
    else if (isRoll(widget))
    {
      rollInput = value;
      hasRoll = true;
    }
    else if (isPitch(widget))
    {
      pitchInput = value;
      hasPitch = true;
    }
  }

  // Update the values with EMA filtering for smooth display
  if (!m_integrateValues)
  {
    constexpr double kAlpha = 0.4;
    if (!m_displayFilterInitialized)
    {
      if (hasYaw)
        m_yaw = yawInput;
      if (hasRoll)
        m_roll = rollInput;
      if (hasPitch)
        m_pitch = pitchInput;
      m_displayFilterInitialized = true;
    }
    else
    {
      auto angleEma = [](double input, double &state) {
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

    m_timer.restart();
    m_rateFilterInitialized = false;
  }

  // Integrate incoming angular rates into absolute orientation
  else
  {
    double deltaT = m_timer.nsecsElapsed() / 1e9;
    m_timer.restart();
    if (!std::isfinite(deltaT) || deltaT <= 0.0)
      deltaT = 0.0;
    else
      deltaT = std::clamp(deltaT, 0.001, 0.1);

    // Low-pass filter rates to reduce high-frequency vibration jitter.
    constexpr double kCutoffHz = 6.0;
    constexpr double kTwoPi = 6.28318530717958647692;
    const double rc = 1.0 / (kTwoPi * kCutoffHz);
    const double alpha = deltaT / (rc + deltaT);

    auto filteredRate = [&](const double input, double &state) {
      if (!m_rateFilterInitialized)
      {
        state = input;
        return state;
      }

      state += alpha * (input - state);
      if (std::abs(state) < 1e-5)
        state = 0.0;

      return state;
    };

    const double yawRate
        = hasYaw ? filteredRate(yawInput, m_filteredYawRate) : 0.0;
    const double rollRate
        = hasRoll ? filteredRate(rollInput, m_filteredRollRate) : 0.0;
    const double pitchRate
        = hasPitch ? filteredRate(pitchInput, m_filteredPitchRate) : 0.0;

    if (hasYaw)
      m_yaw += yawRate * deltaT;
    if (hasRoll)
      m_roll += rollRate * deltaT;
    if (hasPitch)
      m_pitch += pitchRate * deltaT;

    m_rateFilterInitialized = true;
  }

  // Normalize all angles between -180 and 180
  m_yaw = normalizeAngle(m_yaw);
  m_roll = normalizeAngle(m_roll);
  m_pitch = normalizeAngle(m_pitch);

  // Emit signal if orientation changed
  const bool yawChanged = DSP::notEqual(m_yaw, previousYaw);
  const bool rollChanged = DSP::notEqual(m_roll, previousRoll);
  const bool pitchChanged = DSP::notEqual(m_pitch, previousPitch);
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
    m_filteredYawRate = 0;
    m_filteredRollRate = 0;
    m_filteredPitchRate = 0;
    m_rateFilterInitialized = false;
    m_displayFilterInitialized = false;
    m_timer.restart();
    m_integrateValues = enabled;

    Q_EMIT updated();
    Q_EMIT integrateValuesChanged();
  }
}
