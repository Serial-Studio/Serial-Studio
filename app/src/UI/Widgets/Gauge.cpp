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
double Widgets::Gauge::value() const
{
  return m_value;
}

/**
 * @brief Returns the minimum value of the gauge scale.
 * @return The minimum value of the gauge scale.
 */
double Widgets::Gauge::minValue() const
{
  return m_minValue;
}

/**
 * @brief Returns the maximum value of the gauge scale.
 * @return The maximum value of the gauge scale.
 */
double Widgets::Gauge::maxValue() const
{
  return m_maxValue;
}

/**
 * @brief Returns the alarm level of the gauge.
 * @return The alarm level of the gauge.
 */
double Widgets::Gauge::alarmValue() const
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
