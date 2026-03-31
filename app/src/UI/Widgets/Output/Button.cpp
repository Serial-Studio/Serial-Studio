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

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a button output widget.
 */
Widgets::Output::Button::Button(const DataModel::OutputWidget& config, QQuickItem* parent)
  : Base(config, parent)
{}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Called from QML when the button is clicked.
 *
 * Sends value 1 through the transmit function.
 */
void Widgets::Output::Button::click()
{
  sendValue(1);
}
