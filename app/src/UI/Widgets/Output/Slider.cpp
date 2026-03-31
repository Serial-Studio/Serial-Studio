/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/Output/Slider.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a slider output widget.
 */
Widgets::Output::Slider::Slider(const DataModel::OutputWidget& config, QQuickItem* parent)
  : Base(config, parent), m_currentValue(config.minValue)
{}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current slider value.
 */
double Widgets::Output::Slider::currentValue() const noexcept
{
  return m_currentValue;
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the slider value and transmits it.
 *
 * The value is clamped to [minValue, maxValue] before transmission.
 */
void Widgets::Output::Slider::setCurrentValue(double value)
{
  value = qBound(minValue(), value, maxValue());

  if (!qFuzzyCompare(m_currentValue, value)) {
    m_currentValue = value;
    Q_EMIT currentValueChanged();
    sendValue(m_currentValue);
  }
}
