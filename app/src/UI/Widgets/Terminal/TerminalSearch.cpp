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

#include "UI/Widgets/Terminal/TerminalSearch.h"

#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an idle search: no query, no matches, and a current index of -1 meaning
 *        "nothing selected".
 */
Widgets::TerminalSearch::TerminalSearch() : m_current(-1), m_dirty(false), m_caseSensitive(false) {}

//--------------------------------------------------------------------------------------------------
// State
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the buffer changed since the last scan and the match list is
 *        therefore stale.
 */
bool Widgets::TerminalSearch::dirty() const
{
  return m_dirty;
}

/**
 * @brief Returns true while a query is set.
 */
bool Widgets::TerminalSearch::active() const
{
  return !m_query.isEmpty();
}

/**
 * @brief Returns the buffer row of the current match, or -1 when there is none.
 */
int Widgets::TerminalSearch::currentRow() const
{
  if (m_current < 0 || m_current >= m_matches.size())
    return -1;

  return m_matches[m_current].y();
}

/**
 * @brief Returns the number of matches for the active query.
 */
int Widgets::TerminalSearch::matchCount() const
{
  return static_cast<int>(m_matches.size());
}

/**
 * @brief Returns the zero-based index of the current match (-1 when there is none).
 */
int Widgets::TerminalSearch::currentIndex() const
{
  return m_current;
}

/**
 * @brief Returns the one-based index of the current match, which is what the search bar
 *        shows; zero when there is no match.
 */
int Widgets::TerminalSearch::currentMatchNumber() const
{
  return m_current + 1;
}

/**
 * @brief Returns the active query string.
 */
const QString& Widgets::TerminalSearch::query() const
{
  return m_query;
}

/**
 * @brief Returns the match list, ordered by row and then by column.
 */
const QList<QPoint>& Widgets::TerminalSearch::matches() const
{
  return m_matches;
}

/**
 * @brief Marks the match list stale; every buffer mutation on the terminal side must call
 *        this or navigation keeps pointing at rows that have moved or vanished.
 */
void Widgets::TerminalSearch::markDirty()
{
  m_dirty = true;
}

//--------------------------------------------------------------------------------------------------
// Query & scanning
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the query and case mode, resetting navigation to the first match; returns
 *        false when neither changed, so the facade can skip the rescan.
 */
bool Widgets::TerminalSearch::setQuery(const QString& query, bool caseSensitive)
{
  if (m_query == query && m_caseSensitive == caseSensitive)
    return false;

  m_query         = query;
  m_caseSensitive = caseSensitive;
  m_current       = 0;
  return true;
}

/**
 * @brief Drops the query and every match; returns false when there was nothing to clear.
 */
bool Widgets::TerminalSearch::clear()
{
  if (m_query.isEmpty() && m_matches.isEmpty())
    return false;

  m_query.clear();
  m_matches.clear();
  m_current = -1;
  m_dirty   = false;
  return true;
}

/**
 * @brief Rescans @p lines for the active query, clamping the current-match index so
 *        navigation stays valid after rows are trimmed, erased, or collapsed.
 */
void Widgets::TerminalSearch::refresh(const QStringList& lines)
{
  m_dirty = false;
  m_matches.clear();

  if (m_query.isEmpty()) {
    m_current = -1;
    return;
  }

  const auto cs = m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
  for (int i = 0; i < lines.size(); ++i) {
    const QString& line = lines[i];

    qsizetype from = 0;
    while (from < line.length()) {
      const qsizetype index = line.indexOf(m_query, from, cs);
      if (index < 0)
        break;

      m_matches.append(QPoint(static_cast<int>(index), i));
      from = index + qMax<qsizetype>(1, m_query.length());
    }
  }

  if (m_matches.isEmpty())
    m_current = -1;
  else
    m_current = qBound(0, m_current, static_cast<int>(m_matches.size()) - 1);

  SS_ASSERT(m_current >= -1, m_current = -1);
  SS_ASSERT(m_current < m_matches.size(), m_current = static_cast<int>(m_matches.size()) - 1);
}

//--------------------------------------------------------------------------------------------------
// Navigation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Advances to the next match, wrapping at the end; false when there is none.
 */
bool Widgets::TerminalSearch::next()
{
  if (m_matches.isEmpty())
    return false;

  m_current = (m_current + 1) % static_cast<int>(m_matches.size());
  return true;
}

/**
 * @brief Steps back to the previous match, wrapping at the start; false when there is none.
 */
bool Widgets::TerminalSearch::previous()
{
  if (m_matches.isEmpty())
    return false;

  const int count = static_cast<int>(m_matches.size());
  m_current       = (m_current <= 0) ? count - 1 : m_current - 1;
  return true;
}
