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

#include "UI/Widgets/Meter.h"

#include "DSP.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Meter widget.
 */
Widgets::Meter::Meter(const int index, QQuickItem* parent) : Bar(index, parent, false)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardMeter, m_index)) {
    const auto& dataset = GET_DATASET(SerialStudio::DashboardMeter, m_index);

    m_title            = dataset.title;
    m_units            = dataset.units;
    m_displayFormat    = dataset.displayFormat;
    m_displayTickCount = dataset.displayTickCount;
    m_showValueDisplay = dataset.showValueDisplay;
    m_minValue         = qMin(dataset.wgtMin, dataset.wgtMax);
    m_maxValue         = qMax(dataset.wgtMin, dataset.wgtMax);
    m_alarmLow         = qMin(dataset.alarmLow, dataset.alarmHigh);
    m_alarmHigh        = qMax(dataset.alarmLow, dataset.alarmHigh);
    m_alarmLow         = qBound(m_minValue, m_alarmLow, m_maxValue);
    m_alarmHigh        = qBound(m_minValue, m_alarmHigh, m_maxValue);

    if (dataset.alarmEnabled) {
      if (m_alarmHigh == m_alarmLow)
        m_alarmLow = m_minValue;

      m_alarmsDefined = (m_alarmLow > m_minValue && m_alarmLow < m_maxValue)
                     || (m_alarmHigh < m_maxValue && m_alarmHigh > m_minValue);
    }

    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this, &Meter::updateData);
  }
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the meter value from the dashboard source.
 */
void Widgets::Meter::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardMeter, m_index)) {
    const auto& dataset = GET_DATASET(SerialStudio::DashboardMeter, m_index);
    if (!std::isfinite(dataset.numericValue))
      return;

    auto value = qMax(m_minValue, qMin(m_maxValue, dataset.numericValue));
    if (DSP::notEqual(value, m_value)) {
      m_value = value;
      notifyOnAlarmEdge();
      Q_EMIT updated();
    }
  }
}
