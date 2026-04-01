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

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a toggle output widget.
 */
Widgets::Output::Toggle::Toggle(const DataModel::OutputWidget& config, QQuickItem* parent)
  : Base(config, parent), m_checked(config.initialValue != 0)
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
 * @brief Sets the toggle state and transmits 1 (on) or 0 (off).
 */
void Widgets::Output::Toggle::setChecked(bool checked)
{
  // Update state and transmit the new on/off value
  if (m_checked != checked) {
    m_checked = checked;
    Q_EMIT checkedChanged();
    sendValue(m_checked ? 1 : 0);
  }
}
