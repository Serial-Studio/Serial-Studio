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

#include <QByteArrayView>
#include <QString>

namespace Console::TextFormat {

/**
 * @brief Line-continuation state of one console stream: whether the next character opens a line
 *        (so a timestamp belongs in front of it) and whether the previous chunk ended on a CR
 *        whose LF may still arrive. One instance per stream -- the shared view and each device.
 */
struct LineState {
  bool isStartingLine = true;
  bool lastCharWasCR  = false;
};

/**
 * @brief Normalizes @p text's line endings to LF and prefixes @p timestamp to every line that
 *        starts here and carries something other than whitespace, advancing @p state across the
 *        chunk boundary. An empty @p timestamp stamps nothing.
 */
[[nodiscard]] QString formatIncoming(const QString& text,
                                     LineState& state,
                                     const QString& timestamp);

/**
 * @brief Replaces every non-printable character with '.', keeping CR, LF, TAB and ESC. Used
 *        when VT-100 emulation is off and the terminal renders the bytes literally.
 */
[[nodiscard]] QString filterControlChars(const QString& text);

/**
 * @brief Renders @p data as a classic hex dump: offset, 16 hex bytes, ASCII column.
 */
[[nodiscard]] QString hexDump(QByteArrayView data);

}  // namespace Console::TextFormat
