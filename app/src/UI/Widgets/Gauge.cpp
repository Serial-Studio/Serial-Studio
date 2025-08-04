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
 *
 * Initializes a gauge-specific data model by bypassing the default Bar
 * initialization logic and loading values from the DashboardGauge dataset.
 *
 * This constructor disables the automatic Bar dataset loading by passing
 * `false` to the Bar constructor, and instead applies custom Gauge setup.
 *
 * @param index Dataset index for the gauge.
 * @param parent Optional QML parent item.
 */
Widgets::Gauge::Gauge(const int index, QQuickItem *parent)
  : Bar(index, parent, false)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardGauge, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardGauge, m_index);

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
            &Gauge::updateData);
  }
}

/**
 * @brief Updates the gauge value from the dashboard source.
 *
 * Reads the current numeric value from the DashboardBar dataset and clamps
 * it to the configured min/max range. Emits `updated()` if the value changes.
 *
 * This function overrides Bar's logic to retrieve data from a different source.
 */
void Widgets::Gauge::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardGauge, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardGauge, m_index);
    auto value = qMax(m_minValue, qMin(m_maxValue, dataset.numericValue));
    if (!qFuzzyCompare(value, m_value))
    {
      m_value = value;
      Q_EMIT updated();
    }
  }
}
