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
#include "UI/Widgets/Gauge.h"

/**
 * @brief Constructs a Gauge widget.
 * @param index The index of the gauge in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::Gauge::Gauge(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_value(0)
  , m_minValue(0)
  , m_maxValue(100)
  , m_alarmValue(0)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardGauge, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardGauge, m_index);

    m_units = dataset.units();
    m_alarmValue = dataset.alarm();
    m_minValue = qMin(dataset.min(), dataset.max());
    m_maxValue = qMax(dataset.min(), dataset.max());

    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &Gauge::updateData);
  }
}

/**
 * @brief Returns the measurement units of the dataset.
 */
const QString &Widgets::Gauge::units() const
{
  return m_units;
}

/**
 * @brief Returns the current value of the gauge.
 * @return The current value of the gauge.
 */
qreal Widgets::Gauge::value() const
{
  return m_value;
}

/**
 * @brief Returns the minimum value of the gauge scale.
 * @return The minimum value of the gauge scale.
 */
qreal Widgets::Gauge::minValue() const
{
  return m_minValue;
}

/**
 * @brief Returns the maximum value of the gauge scale.
 * @return The maximum value of the gauge scale.
 */
qreal Widgets::Gauge::maxValue() const
{
  return m_maxValue;
}

/**
 * @brief Returns the alarm level of the gauge.
 * @return The alarm level of the gauge.
 */
qreal Widgets::Gauge::alarmValue() const
{
  return m_alarmValue;
}

/**
 * @brief Updates the gauge data from the Dashboard.
 *
 * This method retrieves the latest data for this gauge from the Dashboard
 * and updates the gauge's value and text display accordingly.
 */
void Widgets::Gauge::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardGauge, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardGauge, m_index);
    auto value = qMax(m_minValue, qMin(m_maxValue, dataset.value().toDouble()));
    if (!qFuzzyCompare(value, m_value))
    {
      m_value = value;
      Q_EMIT updated();
    }
  }
}
