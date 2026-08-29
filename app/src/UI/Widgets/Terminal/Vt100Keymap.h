/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QByteArray>

class QKeyEvent;

namespace Widgets::Vt100Keymap {
/**
 * @brief The keyboard half of VT-100 emulation: a pure lookup from a Qt key event to the bytes a
 *        terminal would put on the wire. It holds no state and reaches no setting; the line
 *        ending that Return expands to is passed in, because that is a console preference the
 *        table itself has no business knowing.
 */
[[nodiscard]] QByteArray specialKey(int key);
[[nodiscard]] QByteArray translate(const QKeyEvent* event, const QByteArray& enterSequence);
}  // namespace Widgets::Vt100Keymap
