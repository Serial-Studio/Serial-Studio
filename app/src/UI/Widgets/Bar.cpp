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
 * @param index The index of the bar in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::Bar::Bar(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_value(0)
  , m_minValue(0)
  , m_maxValue(100)
  , m_alarmValue(0)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardBar, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardBar, m_index);

    m_units = dataset.units();
    m_alarmValue = dataset.alarm();
    m_minValue = qMin(dataset.min(), dataset.max());
    m_maxValue = qMax(dataset.min(), dataset.max());

    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &Bar::updateData);
  }
}

/**
 * @brief Returns the measurement units of the dataset.
 */
const QString &Widgets::Bar::units() const
{
  return m_units;
}

/**
 * @brief Returns the current value of the bar.
 * @return The current value of the bar.
 */
double Widgets::Bar::value() const
{
  return m_value;
}

/**
 * @brief Returns the minimum value of the bar scale.
 * @return The minimum value of the bar scale.
 */
double Widgets::Bar::minValue() const
{
  return m_minValue;
}

/**
 * @brief Returns the maximum value of the bar scale.
 * @return The maximum value of the bar scale.
 */
double Widgets::Bar::maxValue() const
{
  return m_maxValue;
}

/**
 * @brief Returns the alarm level of the bar.
 * @return The alarm level of the bar.
 */
double Widgets::Bar::alarmValue() const
{
  return m_alarmValue;
}

/**
 * @brief Calculates the fractional value of the bar's current level.
 *
 * This function computes the relative position of the current value within
 * the range defined by minValue() and maxValue(). It is used to determine
 * the visual height of the bar in the UI.
 */
double Widgets::Bar::fractionalValue() const
{
  double min = qMin(minValue(), maxValue());
  double max = qMax(minValue(), maxValue());
  const double range = max - min;
  if (qFuzzyIsNull(range))
    return 0.0;

  const double level = value() - min;
  return qBound(0.0, level / range, 1.0);
}

/**
 * @brief Calculates the fractional value of the bar's alarm level.
 *
 * This function computes the relative position of the alarm level within
 * the range defined by minValue() and maxValue(). It is used to determine
 * the position where the bar's color should change to indicate an alarm state.
 */
double Widgets::Bar::alarmFractionalValue() const
{
  const double range = maxValue() - minValue();
  if (qFuzzyIsNull(range))
    return 0.0;

  const double clampedAlarmLevel = qBound(minValue(), m_alarmValue, maxValue());
  const double alarmPosition = clampedAlarmLevel - minValue();
  return qBound(0.0, alarmPosition / range, 1.0);
}

/**
 * @brief Updates the bar data from the Dashboard.
 *
 * This method retrieves the latest data for this bar from the Dashboard
 * and updates the bar's value and text display accordingly.
 */
void Widgets::Bar::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardBar, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardBar, m_index);
    auto value = qMax(m_minValue, qMin(m_maxValue, dataset.value().toDouble()));
    if (!qFuzzyCompare(value, m_value))
    {
      m_value = value;
      Q_EMIT updated();
    }
  }
}
