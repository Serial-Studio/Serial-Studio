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

#include "UI/Widgets/Gauge.h"

#include "DataModel/Frame.h"
#include "DSP.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Gauge widget.
 */
Widgets::Gauge::Gauge(const int index, QQuickItem* parent) : Bar(index, parent, false)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardGauge, m_index)) {
    const auto& dataset = GET_DATASET(SerialStudio::DashboardGauge, m_index);

    m_title            = dataset.title;
    m_units            = dataset.units;
    m_displayFormat    = dataset.displayFormat;
    m_displayTickCount = dataset.displayTickCount;
    m_decimalPoints    = dataset.decimalPoints;
    m_minValue         = qMin(dataset.wgtMin, dataset.wgtMax);
    m_maxValue         = qMax(dataset.wgtMin, dataset.wgtMax);
    buildBands(dataset.alarmBands);

    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this, &Gauge::updateData);
  }
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the gauge value from the dashboard source.
 */
void Widgets::Gauge::updateData()
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardGauge, m_index)) {
    const auto& dataset = GET_DATASET(SerialStudio::DashboardGauge, m_index);
    if (!std::isfinite(dataset.numericValue))
      return;

    auto value = qMax(m_minValue, qMin(m_maxValue, dataset.numericValue));
    if (DSP::notEqual(value, m_value)) {
      m_value = value;
      recomputeActiveBand(value);
      if (isEnabled())
        Q_EMIT updated();
    }
  }
}
