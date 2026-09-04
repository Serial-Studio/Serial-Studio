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

#include "UI/Widgets/Output/TextField.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a text field output widget.
 */
Widgets::Output::TextField::TextField(const DataModel::OutputWidget& config, QQuickItem* parent)
  : Base(config, parent)
{}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Called from QML when the user presses Enter or clicks Send.
 */
void Widgets::Output::TextField::sendText(const QString& text)
{
  sendValue(text);
}
