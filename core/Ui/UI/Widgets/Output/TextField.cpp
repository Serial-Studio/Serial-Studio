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

#include <utility>

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a text field output widget.
 */
Widgets::Output::TextField::TextField(const DataModel::OutputWidget& config,
                                      TransmitTarget target,
                                      QQuickItem* parent)
  : Base(config, std::move(target), parent)
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

//--------------------------------------------------------------------------------------------------
// State feedback
//--------------------------------------------------------------------------------------------------

/**
 * @brief The text the source reports. A bound text field shows this beside the input rather than
 *        overwriting what the operator is typing, which is the same rule the slider follows.
 */
QString Widgets::Output::TextField::reportedText() const
{
  return m_reportedText;
}

/**
 * @brief Adopts the reported text without transmitting.
 */
void Widgets::Output::TextField::applyStateVerdict(const StateBinding::Verdict& verdict)
{
  if (!verdict.known || m_reportedText == verdict.text)
    return;

  m_reportedText = verdict.text;
  Q_EMIT reportedTextChanged();
}
