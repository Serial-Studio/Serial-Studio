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

#include <QStringList>
#include <QStringView>
#include <vector>

#include "Console/HighlightRules.h"

class QPainter;

namespace Widgets {

/**
 * @brief Most keyword occurrences coloured in one row, and how far into a row they are looked
 *        for; both bound the paint pass on a pathological line.
 */
inline constexpr int kMaxKeywordSpans      = 64;
inline constexpr qsizetype kMaxKeywordScan = 4096;

/**
 * @brief One coloured occurrence in a row: where it starts, how long it is and which rule's
 *        colours it takes.
 */
struct KeywordSpan {
  int start  = 0;
  int length = 0;
  int rule   = 0;
};

/**
 * @brief Where the terminal draws its rows, as the keyword overlay needs it.
 */
struct KeywordPaintGeometry {
  int firstLine    = 0;
  int lastLine     = -1;
  int lineHeight   = 0;
  int borderX      = 0;
  int borderY      = 0;
  int viewHeight   = 0;
  int charsPerLine = 1;
};

void collectKeywordSpans(QStringView line,
                         const std::vector<Console::KeywordRule>& rules,
                         std::vector<KeywordSpan>& out);

/**
 * @brief Draws the console's keyword colour rules (spec 0127 R8) over the visible rows. It runs
 *        at paint time and only on what is on screen, so the row store is never rewritten: the
 *        text the console copies, logs or searches stays exactly what was received.
 */
class TerminalHighlighter {
public:
  TerminalHighlighter();

  void paint(QPainter* painter,
             const QStringList& lines,
             const std::vector<Console::KeywordRule>& rules,
             const KeywordPaintGeometry& geometry);

private:
  std::vector<KeywordSpan> m_spans;
};

}  // namespace Widgets
