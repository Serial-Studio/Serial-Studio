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
#include "UI/Widgets/Bar.h"

/**
 * @brief Constructs a Bar widget.
 *
 * Initializes the bar widget with data from the specified dataset index,
 * including value thresholds, units, and pltMin/pltMax limits. Connects the
 * Dashboard update signal to trigger value refreshes.
 *
 * @param index The index of the dataset in the dashboard.
 * @param parent Optional parent QQuickItem.
 * @param autoInitFromBarDataset If true, initializes dataset from DashboardBar.
 *        Set to false if subclass wants to handle dataset manually (e.g.,
 *        Gauge).
 */
Widgets::Bar::Bar(const int index, QQuickItem *parent,
                  bool autoInitFromBarDataset)
  : QQuickItem(parent)
  , m_index(index)
  , m_value(std::nan(""))
  , m_minValue(std::nan(""))
  , m_maxValue(std::nan(""))
  , m_alarmLow(std::nan(""))
  , m_alarmHigh(std::nan(""))
  , m_alarmsDefined(false)
{
  if (autoInitFromBarDataset
      && VALIDATE_WIDGET(SerialStudio::DashboardBar, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardBar, m_index);

    m_units = dataset.units;
    m_minValue = qMin(dataset.wgtMin, dataset.wgtMax);
    m_maxValue = qMax(dataset.wgtMin, dataset.wgtMax);
    m_alarmLow = qMin(dataset.alarmLow, dataset.alarmHigh);
    m_alarmHigh = qMax(dataset.alarmLow, dataset.alarmHigh);
    m_alarmLow = qBound(m_minValue, m_alarmLow, m_maxValue);
    m_alarmHigh = qBound(m_minValue, m_alarmHigh, m_maxValue);

    if (dataset.alarmEnabled)
    {
      if (m_alarmHigh == m_alarmLow)
        m_alarmLow = m_minValue;

      m_alarmsDefined
          = (m_alarmLow > m_minValue && m_alarmLow < m_maxValue)
            || (m_alarmHigh < m_maxValue && m_alarmHigh > m_minValue);
    }

    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &Bar::updateData);
  }
}

/**
 * @brief Checks if the dataset options indicate that a valid alarm range
 *        is available.
 *
 * @return `true` if the dataset configuration permits enabling alarm values.
 */
bool Widgets::Bar::alarmsDefined() const
{
  return m_alarmsDefined;
}

/**
 * @brief Checks if the current value is in an alarm state.
 *
 * This function determines whether the current value (`m_value`)
 * falls outside the defined safe range. It compares the value against
 * the alarm thresholds `m_alarmLow` and `m_alarmHigh`, if they are defined.
 *
 * @return `true` if the value is less than or equal to `m_alarmLow`, or
 *         greater than or equal to `m_alarmHigh`, and those thresholds are
 *         valid (not NaN). Returns `false` if the value is within the safe
 *         range or no thresholds are defined.
 */
bool Widgets::Bar::alarmTriggered() const
{
  if (m_alarmsDefined)
  {
    if (!std::isnan(m_alarmLow) && m_alarmLow > m_minValue)
    {
      if (m_value <= m_alarmLow)
        return true;
    }

    if (!std::isnan(m_alarmHigh) && m_alarmHigh < m_maxValue)
    {
      if (m_value >= m_alarmHigh)
        return true;
    }
  }

  return false;
}

/**
 * @brief Returns the measurement units associated with the dataset.
 *
 * This value is displayed alongside the bar for context (e.g., "°C", "V").
 *
 * @return A constant reference to the units string.
 */
const QString &Widgets::Bar::units() const
{
  return m_units;
}

/**
 * @brief Returns the current numeric value of the dataset.
 *
 * This value reflects the most recently received and validated data
 * from the dashboard source.
 *
 * @return The current value displayed by the bar.
 */
double Widgets::Bar::value() const
{
  return m_value;
}

/**
 * @brief Returns the minimum scale value for the bar.
 *
 * Used to define the lower bound of the bar's value range.
 *
 * @return The minimum value on the bar's scale.
 */
double Widgets::Bar::minValue() const
{
  return m_minValue;
}

/**
 * @brief Returns the maximum scale value for the bar.
 *
 * Used to define the upper bound of the bar's value range.
 *
 * @return The maximum value on the bar's scale.
 */
double Widgets::Bar::maxValue() const
{
  return m_maxValue;
}

/**
 * @brief Returns the configured low alarm threshold.
 *
 * If the dataset value falls below this threshold, the UI may visually indicate
 * a warning state.
 *
 * @return The low alarm threshold.
 */
double Widgets::Bar::alarmLow() const
{
  return m_alarmLow;
}

/**
 * @brief Returns the configured high alarm threshold.
 *
 * If the dataset value exceeds this threshold, the UI may visually indicate a
 * warning state.
 *
 * @return The high alarm threshold.
 */
double Widgets::Bar::alarmHigh() const
{
  return m_alarmHigh;
}

/**
 * @brief Returns the normalized fractional position of the current value.
 *
 * Computes a value between 0.0 and 1.0 indicating the relative position
 * of the dataset value between the min and max bounds.
 *
 * @return Fractional position of the current value.
 */
double Widgets::Bar::normalizedValue() const
{
  return computeFractional(m_value);
}

/**
 * @brief Returns the normalized fractional position of the low alarm threshold.
 *
 * Indicates where, within the bar's scale, the low alarm should appear
 * visually.
 *
 * @return Fractional position of the low alarm.
 */
double Widgets::Bar::normalizedAlarmLow() const
{
  return computeFractional(m_alarmLow);
}

/**
 * @brief Returns the normalized fractional position of the high alarm
 *        threshold.
 *
 * Indicates where, within the bar's scale, the high alarm should appear
 * visually.
 *
 * @return Fractional position of the high alarm.
 */
double Widgets::Bar::normalizedAlarmHigh() const
{
  return computeFractional(m_alarmHigh);
}

/**
 * @brief Updates the current dataset value from the dashboard source.
 *
 * Retrieves the latest numeric value associated with this bar's index,
 * clamps it to the valid min/max range, and emits the `updated()` signal
 * if the value has changed.
 */
void Widgets::Bar::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardBar, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardBar, m_index);
    auto value = qMax(m_minValue, qMin(m_maxValue, dataset.numericValue));
    if (!qFuzzyCompare(value, m_value))
    {
      m_value = value;
      Q_EMIT updated();
    }
  }
}
