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

#include <QColor>
#include <QList>

namespace Widgets {
/**
 * @brief Owns every color decision the terminal's SGR lane makes: the theme-derived
 *        standard/bright base palettes, the xterm-256 index table, and the foreground and
 *        background selected by the escape sequences seen so far. The facade reads the two console
 *        colors and hands them to rebuild(), which keeps this unit pure and directly testable.
 */
class AnsiPalette {
public:
  AnsiPalette();

  [[nodiscard]] const QColor& foreground() const;
  [[nodiscard]] const QColor& background() const;
  [[nodiscard]] const QColor& brightColor(int index) const;
  [[nodiscard]] const QColor& standardColor(int index) const;

  [[nodiscard]] static QColor indexedColor(int index);

  void resetColors();
  void resetForeground();
  void rebuild(const QColor& consoleBase, const QColor& consoleText);
  void applySgr(const QList<int>& codes, const QColor& defaultForeground);

private:
  [[nodiscard]] int applySgrCode(const QList<int>& codes, int i, const QColor& defaultForeground);

private:
  QColor m_foreground;
  QColor m_background;
  QColor m_brightColors[8];
  QColor m_standardColors[8];
};
}  // namespace Widgets
