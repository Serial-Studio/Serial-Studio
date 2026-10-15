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

#include <QString>

#include "Core/Translator.h"

namespace Console {

/**
 * @brief Returns the ASCII-art Serial Studio banner that opens the console welcome guide.
 */
[[nodiscard]] QString welcomeLogo();

/**
 * @brief Returns the one-line start hint that closes the welcome guide.
 */
[[nodiscard]] QString commandPaletteTip();

/**
 * @brief Returns the console hint shown when a connection stays silent (spec 0100 R5): one line
 *        for "no bytes at all", another for "bytes but no complete frame".
 */
[[nodiscard]] QString noDataHint(bool bytesReceived);

/**
 * @brief Returns the localized welcome guide the console shows on startup and on every language
 *        change: the GPL text, or on a commercial build the trial text unless the published
 *        licence tier is above Trial (ordinal 2), in which case the Pro text.
 */
[[nodiscard]] QString welcomeConsoleText(Misc::Translator::Language language);

}  // namespace Console
