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

#include "UI/Widgets/Terminal/TerminalHighlighter.h"

#include <algorithm>
#include <QFontMetrics>
#include <QPainter>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Span collection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether [start, start + length) overlaps a span already taken.
 */
[[nodiscard]] static bool overlapsTaken(const std::vector<Widgets::KeywordSpan>& taken,
                                        const int start,
                                        const int length) noexcept
{
  for (const auto& span : taken)
    if (start < span.start + span.length && span.start < start + length)
      return true;

  return false;
}

/**
 * @brief Finds every occurrence of every rule's keyword in @p line, sorted by position. Rules are
 *        tried in table order and an occurrence that overlaps one already taken is dropped, so
 *        an earlier rule wins. Bounded by kMaxKeywordSpans and by the first kMaxKeywordScan
 *        characters of the row.
 */
void Widgets::collectKeywordSpans(const QStringView line,
                                  const std::vector<Console::KeywordRule>& rules,
                                  std::vector<KeywordSpan>& out)
{
  out.clear();
  const auto head = line.first(qMin(line.size(), kMaxKeywordScan));
  if (head.isEmpty())
    return;

  for (size_t r = 0; r < rules.size(); ++r) {
    const auto& rule       = rules[r];
    const qsizetype length = rule.keyword.size();
    if (length == 0 || length > head.size())
      continue;

    qsizetype at = head.indexOf(rule.keyword, 0, rule.sensitivity);
    while (at >= 0 && out.size() < static_cast<size_t>(kMaxKeywordSpans)) {
      const int start = static_cast<int>(at);
      if (!overlapsTaken(out, start, static_cast<int>(length)))
        out.push_back({start, static_cast<int>(length), static_cast<int>(r)});

      at = head.indexOf(rule.keyword, at + length, rule.sensitivity);
    }
  }

  std::sort(out.begin(), out.end(), [](const KeywordSpan& a, const KeywordSpan& b) {
    return a.start < b.start;
  });
}

//--------------------------------------------------------------------------------------------------
// Painting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Paints the part of @p span that falls inside the wrapped segment [segStart, segEnd) of
 *        @p line: the rule's background first, then the same characters in its text colour.
 */
static void paintSpan(QPainter* painter,
                      const QFontMetrics& fm,
                      const QString& line,
                      const std::vector<Console::KeywordRule>& rules,
                      const Widgets::KeywordSpan& span,
                      const int segStart,
                      const int segEnd,
                      const int originX,
                      const int y,
                      const int ascent,
                      const int lineHeight)
{
  SS_ASSERT(span.rule >= 0 && static_cast<size_t>(span.rule) < rules.size(), return);

  const int from = qMax(span.start, segStart);
  const int to   = qMin(span.start + span.length, segEnd);
  if (from >= to)
    return;

  const auto& rule = rules[static_cast<size_t>(span.rule)];
  const auto text  = line.mid(from, to - from);
  const int x      = originX + fm.horizontalAdvance(line.mid(segStart, from - segStart));
  painter->fillRect(x, y, fm.horizontalAdvance(text), lineHeight, rule.background);
  painter->setPen(rule.foreground);
  painter->drawText(QPointF(x, y + ascent), text);
}

/**
 * @brief Reserves the span scratch once, so the paint pass does not allocate per row.
 */
Widgets::TerminalHighlighter::TerminalHighlighter()
{
  m_spans.reserve(static_cast<size_t>(kMaxKeywordSpans));
}

/**
 * @brief Repaints each keyword occurrence in the visible rows with its rule's background and
 *        text colour, walking the same wrapped-segment geometry as the text pass. An occurrence
 *        split by a wrap is painted in both segments.
 */
void Widgets::TerminalHighlighter::paint(QPainter* painter,
                                         const QStringList& lines,
                                         const std::vector<Console::KeywordRule>& rules,
                                         const KeywordPaintGeometry& geometry)
{
  SS_ASSERT(painter != nullptr, return);
  SS_ASSERT(geometry.charsPerLine > 0 && geometry.lineHeight > 0, return);
  if (rules.empty())
    return;

  const QFontMetrics fm = painter->fontMetrics();
  const int ascent      = fm.ascent();
  const int bottom      = geometry.viewHeight - geometry.borderY;
  const int last        = qMin(geometry.lastLine, static_cast<int>(lines.size()) - 1);

  int y = geometry.borderY;
  for (int i = geometry.firstLine; i <= last && y < bottom; ++i) {
    const QString& line = lines[i];
    if (line.isEmpty()) {
      y += geometry.lineHeight;
      continue;
    }

    collectKeywordSpans(line, rules, m_spans);
    const int length = static_cast<int>(line.length());
    for (int start = 0; start < length && y < bottom; start += geometry.charsPerLine) {
      const int end = qMin(start + geometry.charsPerLine, length);
      for (const auto& span : m_spans)
        paintSpan(painter,
                  fm,
                  line,
                  rules,
                  span,
                  start,
                  end,
                  geometry.borderX,
                  y,
                  ascent,
                  geometry.lineHeight);

      y += geometry.lineHeight;
    }
  }
}
