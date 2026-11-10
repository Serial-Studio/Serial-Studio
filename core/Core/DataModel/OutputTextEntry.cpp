/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "Core/DataModel/OutputTextEntry.h"

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Keypad
//--------------------------------------------------------------------------------------------------

/**
 * @brief A Number field with any decimals but zero takes a fraction; a Text field has no keypad.
 */
bool DataModel::keypadOffersDecimal(const OutputWidget& w)
{
  SS_ASSERT_LOG(w.decimals >= -1);
  SS_ASSERT_LOG(w.inputKind <= OutputInputKind::Number);
  return w.inputKind == OutputInputKind::Number && w.decimals != 0;
}

/**
 * @brief A field whose Min is zero or more can never take a negative value, so no sign key.
 */
bool DataModel::keypadOffersSign(const OutputWidget& w)
{
  SS_ASSERT_LOG(w.inputKind <= OutputInputKind::Number);
  SS_ASSERT_LOG(w.minValue <= w.maxValue);
  return w.inputKind == OutputInputKind::Number && w.minValue < 0;
}

/**
 * @brief Applies one key to the entry. The sign stays leading whatever else was typed, a point
 *        on an empty entry reads as "0.", and the entry never grows past kMaxKeypadEntry.
 */
QString DataModel::keypadApply(const QString& entry, const QChar key)
{
  SS_ASSERT(entry.size() <= kMaxKeypadEntry + 1, return QString());
  const bool negative = entry.startsWith(kKeypadSign);
  const QString body  = negative ? entry.mid(1) : entry;
  const QString sign  = negative ? QString(kKeypadSign) : QString();

  if (key == kKeypadClear)
    return QString();

  if (key == kKeypadBackspace)
    return entry.isEmpty() ? entry : entry.chopped(1);

  if (key == kKeypadSign)
    return negative ? body : QString(kKeypadSign) + body;

  if (entry.size() >= kMaxKeypadEntry)
    return entry;

  if (key == kKeypadPoint) {
    if (body.contains(kKeypadPoint))
      return entry;

    return sign + (body.isEmpty() ? QStringLiteral("0.") : body + kKeypadPoint);
  }

  if (!key.isDigit())
    return entry;

  SS_ASSERT(key.unicode() >= '0' && key.unicode() <= '9', return entry);
  if (body == QStringLiteral("0"))
    return sign + key;

  return entry + key;
}

//--------------------------------------------------------------------------------------------------
// Command history
//--------------------------------------------------------------------------------------------------

/**
 * @brief Newest first so Up walks back in time from index 0; duplicates further back are kept,
 *        since repeating an old command is itself part of the history.
 */
bool DataModel::pushCommandHistory(QStringList& history, const QString& line)
{
  SS_ASSERT(history.size() <= kMaxCommandHistory, history = history.mid(0, kMaxCommandHistory));
  if (line.trimmed().isEmpty())
    return false;

  if (!history.isEmpty() && history.first() == line)
    return false;

  history.prepend(line);
  while (history.size() > kMaxCommandHistory)
    history.removeLast();

  SS_ASSERT_LOG(history.size() <= kMaxCommandHistory);
  return true;
}
