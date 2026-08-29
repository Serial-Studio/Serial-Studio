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

#include "DataModel/Project/ProjectNavHistory.h"

#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an empty history parked before the first slot.
 */
DataModel::ProjectNavHistory::ProjectNavHistory()
  : m_cursor(-1), m_direction(0), m_navigating(false)
{}

//--------------------------------------------------------------------------------------------------
// Entry identity
//--------------------------------------------------------------------------------------------------

/**
 * @brief Equality over the navigable target of two entries (identity, not cursor position).
 */
bool DataModel::ProjectNavHistory::sameTarget(const Entry& a, const Entry& b) noexcept
{
  if (a.container != b.container)
    return false;

  if (a.container)
    return a.view == b.view;

  return a.kind == b.kind && a.id == b.id && a.parentId == b.parentId && a.key == b.key;
}

//--------------------------------------------------------------------------------------------------
// Cursor state
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when a previously visited node is available behind the cursor.
 */
bool DataModel::ProjectNavHistory::canGoBack() const noexcept
{
  return m_cursor > 0 && !m_entries.empty();
}

/**
 * @brief True when a visited node is available ahead of the cursor.
 */
bool DataModel::ProjectNavHistory::canGoForward() const noexcept
{
  return m_cursor >= 0 && (m_cursor + 1) < static_cast<int>(m_entries.size());
}

/**
 * @brief Index of the entry the editor is currently parked on, or -1 when the history is empty.
 */
int DataModel::ProjectNavHistory::cursor() const noexcept
{
  return m_cursor;
}

/**
 * @brief Number of recorded entries.
 */
int DataModel::ProjectNavHistory::size() const noexcept
{
  return static_cast<int>(m_entries.size());
}

/**
 * @brief Entry at @p index; an out-of-range index yields a shared invalid entry.
 */
const DataModel::ProjectNavHistory::Entry& DataModel::ProjectNavHistory::entryAt(int index) const
{
  static const Entry kInvalid;
  SS_ASSERT(index >= 0 && index < static_cast<int>(m_entries.size()), return kInvalid);

  return m_entries[static_cast<size_t>(index)];
}

/**
 * @brief Direction of the in-flight selection change: -1 back, +1 forward, 0 normal.
 */
int DataModel::ProjectNavHistory::direction() const noexcept
{
  return m_direction;
}

/**
 * @brief True while a back/forward replay is driving the selection, so the resulting selection
 *        change must not be recorded as a new visit.
 */
bool DataModel::ProjectNavHistory::navigating() const noexcept
{
  return m_navigating;
}

//--------------------------------------------------------------------------------------------------
// Mutation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends a visited node and returns whether the history changed: invalid entries and a
 *        repeat of the entry under the cursor are dropped, the forward tail is truncated, and the
 *        ring is capped at kMaxEntries. The dedup is load-bearing: currentChanged and
 *        selectionChanged both reach the editor's selection handler, so one click arrives twice.
 */
bool DataModel::ProjectNavHistory::push(const Entry& entry)
{
  if (!entry.valid)
    return false;

  const int count = static_cast<int>(m_entries.size());
  if (m_cursor >= 0 && m_cursor < count
      && sameTarget(m_entries[static_cast<size_t>(m_cursor)], entry))
    return false;

  if (m_cursor + 1 < count)
    m_entries.erase(m_entries.begin() + (m_cursor + 1), m_entries.end());

  m_entries.push_back(entry);
  m_cursor = static_cast<int>(m_entries.size()) - 1;

  if (static_cast<int>(m_entries.size()) > kMaxEntries) {
    const int drop = static_cast<int>(m_entries.size()) - kMaxEntries;
    m_entries.erase(m_entries.begin(), m_entries.begin() + drop);
    m_cursor -= drop;
  }

  SS_ASSERT_LOG(static_cast<int>(m_entries.size()) <= kMaxEntries);
  SS_ASSERT_LOG(m_cursor >= 0 && m_cursor < static_cast<int>(m_entries.size()));
  return true;
}

/**
 * @brief Drops the whole history (used when a different project is loaded); returns whether
 *        anything was actually discarded.
 */
bool DataModel::ProjectNavHistory::clear()
{
  if (m_entries.empty() && m_cursor == -1)
    return false;

  m_entries.clear();
  m_cursor = -1;
  return true;
}

//--------------------------------------------------------------------------------------------------
// Resolution walks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Index of the nearest entry behind the cursor that @p resolvable accepts, or -1. The walk
 *        is bounded by the entry count so a resolver with side effects can never spin.
 */
int DataModel::ProjectNavHistory::previousResolvable(const Resolver& resolvable) const
{
  SS_ASSERT(static_cast<bool>(resolvable), return -1);

  int index = m_cursor - 1;
  for (int guard = static_cast<int>(m_entries.size()); index >= 0 && guard > 0; --guard, --index)
    if (resolvable(m_entries[static_cast<size_t>(index)]))
      return index;

  return -1;
}

/**
 * @brief Index of the nearest entry ahead of the cursor that @p resolvable accepts, or -1.
 */
int DataModel::ProjectNavHistory::nextResolvable(const Resolver& resolvable) const
{
  SS_ASSERT(static_cast<bool>(resolvable), return -1);

  const int count = static_cast<int>(m_entries.size());
  int index       = m_cursor + 1;
  for (int guard = count; index < count && guard > 0; --guard, ++index)
    if (resolvable(m_entries[static_cast<size_t>(index)]))
      return index;

  return -1;
}

/**
 * @brief Parks the cursor on @p index; an out-of-range index is refused so a failed resolve can
 *        never strand the history outside its own ring.
 */
void DataModel::ProjectNavHistory::setCursor(int index)
{
  SS_ASSERT(index >= 0 && index < static_cast<int>(m_entries.size()), return);

  m_cursor = index;
}

/**
 * @brief Publishes the reveal direction the tree view reads during currentChanged.
 */
void DataModel::ProjectNavHistory::setDirection(int direction)
{
  m_direction = direction;
}

/**
 * @brief Raises or clears the replay guard around a back/forward selection change.
 */
void DataModel::ProjectNavHistory::setNavigating(bool navigating)
{
  m_navigating = navigating;
}
