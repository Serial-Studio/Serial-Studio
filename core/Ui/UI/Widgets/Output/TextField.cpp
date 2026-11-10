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

#include "Core/DataModel/OutputTextEntry.h"

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
  , m_keypadSign(DataModel::keypadOffersSign(config))
  , m_keypadDecimal(DataModel::keypadOffersDecimal(config))
{}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether the keypad offers the sign key (spec 0118 R3): Min allows a negative value.
 */
bool Widgets::Output::TextField::keypadSign() const noexcept
{
  return m_keypadSign;
}

/**
 * @brief Whether the keypad offers a decimal point: the field does not take whole numbers only.
 */
bool Widgets::Output::TextField::keypadDecimal() const noexcept
{
  return m_keypadDecimal;
}

/**
 * @brief How many sent lines the command line can recall this session (spec 0118 R4).
 */
int Widgets::Output::TextField::historyCount() const noexcept
{
  return static_cast<int>(m_history.size());
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Called from QML when the user presses Enter or clicks Send. Returns false when the control
 *        refused the value as out of range, so QML keeps the typed text for correction.
 */
bool Widgets::Output::TextField::sendText(const QString& text)
{
  sendValue(text);
  const bool admitted = rangeMessage().isEmpty();
  if (admitted && !numeric() && DataModel::pushCommandHistory(m_history, text))
    Q_EMIT historyChanged();

  return admitted;
}

/**
 * @brief The sent line @p index steps back, 0 being the newest; empty past either end. Reading it
 *        never sends: only sendText() does.
 */
QString Widgets::Output::TextField::historyAt(const int index) const
{
  if (index < 0 || index >= m_history.size())
    return QString();

  return m_history.at(index);
}

/**
 * @brief The keypad entry after the key named by @p key's first character; the keypad edits a
 *        local entry and only its Enter key sends.
 */
QString Widgets::Output::TextField::keypadApply(const QString& entry, const QString& key) const
{
  if (key.isEmpty())
    return entry;

  return DataModel::keypadApply(entry, key.at(0));
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
