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

#include "UI/Widgets/Output/Button.h"

#include <utility>

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a button output widget.
 */
Widgets::Output::Button::Button(const DataModel::OutputWidget& config,
                                TransmitTarget target,
                                QQuickItem* parent)
  : Base(config, std::move(target), parent)
  , m_checked(config.checkable && config.initialValue != 0)
  , m_checkable(config.checkable)
{}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the latched state of a checkable button.
 */
bool Widgets::Output::Button::isChecked() const noexcept
{
  return m_checked;
}

/**
 * @brief Returns true when the button latches instead of pulsing a single value.
 */
bool Widgets::Output::Button::isCheckable() const noexcept
{
  return m_checkable;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Called from QML when a momentary button is clicked.
 */
void Widgets::Output::Button::click()
{
  sendValue(1);
}

/**
 * @brief Latches the button and transmits 1 (on) or 0 (off). The operator is acting, so it
 *        always transmits: once feedback moved the latch to the reported state, guarding the send
 *        on a change swallowed the re-assert of a state the plant already claims. Only the notify
 *        is guarded; feedback assigns through applyStateVerdict instead (spec 0080 R3).
 */
void Widgets::Output::Button::setChecked(bool checked)
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
 * @brief Adopts what the state source reports, without transmitting. Only a latching button has a
 *        state to correct; a momentary one pulses and holds nothing. Assigning m_checked directly
 *        rather than through setChecked() is what keeps feedback from commanding (spec 0080 R3).
 */
void Widgets::Output::Button::applyStateVerdict(const StateBinding::Verdict& verdict)
{
  if (!m_checkable || !verdict.known || m_checked == verdict.on)
    return;

  m_checked = verdict.on;
  Q_EMIT checkedChanged();
}
