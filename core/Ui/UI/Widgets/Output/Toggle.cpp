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

#include "UI/Widgets/Output/Toggle.h"

#include <utility>

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a toggle output widget.
 */
Widgets::Output::Toggle::Toggle(const DataModel::OutputWidget& config,
                                TransmitTarget target,
                                QQuickItem* parent)
  : Base(config, std::move(target), parent), m_checked(config.initialValue != 0)
{}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current toggle state.
 */
bool Widgets::Output::Toggle::isChecked() const noexcept
{
  return m_checked;
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the toggle and transmits 1 (on) or 0 (off). The operator is acting, so it always
 *        transmits: once feedback moved the switch to the reported state, guarding the send on a
 *        change swallowed the re-assert of a state the plant already claims (spec 0080 R3).
 */
void Widgets::Output::Toggle::setChecked(bool checked)
{
  if (m_checked != checked) {
    m_checked = checked;
    Q_EMIT checkedChanged();
  }

  sendValue(m_checked ? 1 : 0);
}

//--------------------------------------------------------------------------------------------------
// State feedback
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adopts what the state source reports, without transmitting. A bound toggle shows the
 *        equipment's state, so an interlock that stopped the fan stops claiming it is running
 *        (spec 0080). Assigning m_checked directly is the point: setChecked() would transmit and
 *        the control would command its own equipment from its own feedback.
 */
void Widgets::Output::Toggle::applyStateVerdict(const StateBinding::Verdict& verdict)
{
  if (!verdict.known || m_checked == verdict.on)
    return;

  m_checked = verdict.on;
  Q_EMIT checkedChanged();
}
