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

#pragma once

#include <QChar>
#include <QString>
#include <QStringList>

#include "Core/DataModel/OutputWidget.h"

namespace DataModel {

/**
 * @brief Most lines a command-line text field recalls (spec 0118 R4).
 */
inline constexpr int kMaxCommandHistory = 20;

/**
 * @brief Longest entry the keypad builds; past it a digit is ignored rather than truncated.
 */
inline constexpr int kMaxKeypadEntry = 24;

/**
 * @brief Keys the keypad sends to keypadApply besides the digits '0' to '9'.
 */
inline constexpr QChar kKeypadPoint     = QLatin1Char('.');
inline constexpr QChar kKeypadSign      = QLatin1Char('-');
inline constexpr QChar kKeypadBackspace = QLatin1Char('<');
inline constexpr QChar kKeypadClear     = QLatin1Char('C');

/**
 * @brief Whether the keypad offers a decimal point: a Number field that does not take whole
 *        numbers only (decimals 0).
 */
[[nodiscard]] bool keypadOffersDecimal(const OutputWidget& w);

/**
 * @brief Whether the keypad offers the sign key: only when Min allows a negative value.
 */
[[nodiscard]] bool keypadOffersSign(const OutputWidget& w);

/**
 * @brief The keypad entry after @p key: a digit appends (replacing a lone zero), the point is
 *        added once, the sign toggles a leading minus, backspace drops one character and clear
 *        empties it. Unknown keys leave the entry as it is.
 */
[[nodiscard]] QString keypadApply(const QString& entry, QChar key);

/**
 * @brief Records a sent command line newest first (spec 0118 R4): an empty line or a repeat of
 *        the newest one is not stored, and the oldest falls off past kMaxCommandHistory.
 *        Returns whether @p history changed.
 */
bool pushCommandHistory(QStringList& history, const QString& line);

}  // namespace DataModel
