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

#include "DataModel/CachedFlag.h"

#include <QByteArray>

#include "Core/SSAssert.h"

/**
 * @brief Binds the owner every spec callback receives; entries arrive as the owner's members
 *        construct.
 */
DataModel::CachedFlagChecker::CachedFlagChecker(void* owner) noexcept
  : m_owner(owner), m_count(0), m_entries{}
{}

/**
 * @brief Registers one flag: its spec plus, for a Cached<T>, the value and its reader.
 */
void DataModel::CachedFlagChecker::add(const CachedFlagSpec& spec,
                                       const void* value,
                                       int (*read)(const void*)) noexcept
{
  SS_ASSERT(m_count < kMaxEntries, return);
  SS_ASSERT(spec.fresh != nullptr && spec.repair != nullptr, return);

  m_entries[m_count++] = Entry{&spec, value, read, 0, false};
}

/**
 * @brief Number of registered flags.
 */
std::size_t DataModel::CachedFlagChecker::size() const noexcept
{
  return m_count;
}

/**
 * @brief The entry's current cached value: the registered member, or the spec's mirror reader.
 */
int DataModel::CachedFlagChecker::cachedValue(const Entry& entry) const
{
  SS_ASSERT(entry.spec != nullptr, return 0);
  if (entry.read != nullptr && entry.value != nullptr)
    return entry.read(entry.value);

  SS_ASSERT(entry.spec->cached != nullptr, return entry.spec->fresh(m_owner));
  return entry.spec->cached(m_owner);
}

/**
 * @brief One audit pass. Owner thread only, at a low fixed rate, never per frame.
 */
void DataModel::CachedFlagChecker::check()
{
  SS_ASSERT(m_owner != nullptr, return);
  SS_ASSERT(m_count <= kMaxEntries, return);

  for (std::size_t i = 0; i < m_count; ++i) {
    auto& entry = m_entries[i];
    if (entry.spec->settled != nullptr && !entry.spec->settled(m_owner)) {
      entry.suspect = 0;
      continue;
    }

    const int cached = cachedValue(entry);
    const int fresh  = entry.spec->fresh(m_owner);
    if (cached == fresh) {
      entry.suspect = 0;
      continue;
    }

    if (++entry.suspect < kMismatchChecks)
      continue;

    entry.suspect = 0;
    reportStale(entry, cached, fresh);
    entry.spec->repair(m_owner);
  }
}

/**
 * @brief Names the stale flag with both values, once per flag per session, then applies the
 *        assertion policy: a debug build aborts, a release build continues into the repair.
 */
void DataModel::CachedFlagChecker::reportStale(Entry& entry, const int cached, const int fresh)
{
  SS_ASSERT(entry.spec != nullptr, return);
  if (entry.reported)
    return;

  entry.reported  = true;
  const auto text = QByteArray("cached hotpath flag ") + entry.spec->name + " is stale: cached "
                  + QByteArray::number(cached) + ", derived " + QByteArray::number(fresh);
  SSAssertDetail::reportSoftAssert(text.constData(), __FILE__, __LINE__, Q_FUNC_INFO);
  if (SS_ASSERT_IS_FATAL())
    qt_assert(text.constData(), __FILE__, __LINE__);
}
