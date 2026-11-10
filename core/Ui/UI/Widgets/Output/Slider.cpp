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

#include <algorithm>
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
  , m_selector(config.style == DataModel::OutputWidgetStyle::Selector)
  , m_springReturn(config.springReturn)
  , m_currentValue(qBound(config.minValue, config.initialValue, config.maxValue))
  , m_initialValue(config.initialValue)
  , m_labels(config.positionLabels)
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

/**
 * @brief Called from QML when the operator lets go of a selector (spec 0113 R22): a spring-return
 *        selector goes back to its rest position and sends that value once; any other control,
 *        or one already at rest, does nothing.
 */
void Widgets::Output::Slider::release()
{
  const DataModel::OutputLimits limits{minValue(), maxValue(), stepSize(), false};
  double rest = 0;
  if (!m_selector
      || !DataModel::selectorReleaseValue(
        limits, m_springReturn, m_initialValue, m_currentValue, rest))
    return;

  setCurrentValue(rest);
}

//--------------------------------------------------------------------------------------------------
// State feedback
//--------------------------------------------------------------------------------------------------

/**
 * @brief A selector shows a report only when it names one of its positions, by label or by a
 *        detent's value (spec 0113 R23); anything else reads as unknown instead of turning the
 *        pointer to a place between marks. Showing a position never transmits.
 */
void Widgets::Output::Slider::adjustVerdict(StateBinding::Verdict& verdict) const
{
  if (!m_selector || !verdict.known)
    return;

  const DataModel::OutputLimits limits{minValue(), maxValue(), stepSize(), false};
  const int index = DataModel::selectorPosition(limits, m_labels, verdict.value, verdict.text);
  if (index < 0) {
    verdict.known = false;
    return;
  }

  verdict.value = minValue() + index * std::max(stepSize(), 1e-9);
}

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
