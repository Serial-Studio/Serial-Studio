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

#include <utility>

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a slider output widget.
 */
Widgets::Output::Slider::Slider(const DataModel::OutputWidget& config,
                                TransmitTarget target,
                                QQuickItem* parent)
  : Base(config, std::move(target), parent)
  , m_currentValue(qBound(config.minValue, config.initialValue, config.maxValue))
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
 * @brief Sets the slider value and transmits it. The send is unguarded because the operator is
 *        acting: once feedback moved the slider to the reported setpoint, guarding on a change
 *        swallowed a drag back to that same setpoint. Only the notify is guarded.
 */
void Widgets::Output::Slider::setCurrentValue(double value)
{
  value = qBound(minValue(), value, maxValue());

  if (!qFuzzyCompare(m_currentValue, value)) {
    m_currentValue = value;
    Q_EMIT currentValueChanged();
  }

  sendValue(m_currentValue);
}

//--------------------------------------------------------------------------------------------------
// State feedback
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adopts the reported setpoint without transmitting. Base only calls this when the operator
 *        is neither holding the control nor waiting on a request, so feedback can never move a
 *        slider under a dragging finger (spec 0080 R14).
 */
void Widgets::Output::Slider::applyStateVerdict(const StateBinding::Verdict& verdict)
{
  if (!verdict.known || qFuzzyCompare(1.0 + m_currentValue, 1.0 + verdict.value))
    return;

  m_currentValue = verdict.value;
  Q_EMIT currentValueChanged();
}
