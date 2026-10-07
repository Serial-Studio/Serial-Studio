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

#include <array>
#include <cstddef>

#include "Core/HotpathOptimization.h"

namespace DataModel {

/**
 * @brief How the cross-check derives one cached flag from scratch (spec 0095 M1). Callbacks take
 *        the checker's owner; @c cached is only for a mirror the owner reads but does not hold as
 *        a Cached<T>, and @c settled (optional) says when a stale value is still legitimate.
 */
struct CachedFlagSpec {
  const char* name;
  int (*fresh)(const void* owner);
  void (*repair)(void* owner);
  bool (*settled)(const void* owner);
  int (*cached)(const void* owner);
};

/**
 * @brief Audits an owner's cached hotpath flags at a low fixed rate on the owner's thread: each is
 *        re-derived and compared, and a mismatch that survives two consecutive checks (one queued
 *        refresh may legitimately be in flight) reports once per flag, aborts a debug build, and
 *        repairs through the owner's own refresh path. Fixed capacity, no allocation.
 */
class CachedFlagChecker {
public:
  static constexpr std::size_t kMaxEntries = 16;
  static constexpr int kMismatchChecks     = 2;

  explicit CachedFlagChecker(void* owner) noexcept;
  CachedFlagChecker(CachedFlagChecker&&)                 = delete;
  CachedFlagChecker(const CachedFlagChecker&)            = delete;
  CachedFlagChecker& operator=(CachedFlagChecker&&)      = delete;
  CachedFlagChecker& operator=(const CachedFlagChecker&) = delete;

  void add(const CachedFlagSpec& spec, const void* value, int (*read)(const void*)) noexcept;
  void check();

  [[nodiscard]] std::size_t size() const noexcept;

private:
  struct Entry {
    const CachedFlagSpec* spec;
    const void* value;
    int (*read)(const void*);
    int suspect;
    bool reported;
  };

  [[nodiscard]] int cachedValue(const Entry& entry) const;
  void reportStale(Entry& entry, int cached, int fresh);

private:
  void* m_owner;
  std::size_t m_count;
  std::array<Entry, kMaxEntries> m_entries;
};

/**
 * @brief A cached hotpath flag: one T, read as a plain load, that cannot exist without registering
 *        its derivation with a checker. Converts to const T& so a sub-object binding a reference to
 *        the flag binds the member itself, never a temporary.
 */
template<typename T>
class Cached {
public:
  Cached(CachedFlagChecker& checker, const CachedFlagSpec& spec, T initial) noexcept
    : m_value(initial)
  {
    checker.add(spec, &m_value, &Cached::readAsInt);
  }

  Cached(Cached&&)                 = delete;
  Cached(const Cached&)            = delete;
  Cached& operator=(Cached&&)      = delete;
  Cached& operator=(const Cached&) = delete;

  SS_FORCE_INLINE operator const T&() const noexcept { return m_value; }

  SS_FORCE_INLINE Cached& operator=(T value) noexcept
  {
    m_value = value;
    return *this;
  }

private:
  [[nodiscard]] static int readAsInt(const void* value) noexcept
  {
    return static_cast<int>(*static_cast<const T*>(value));
  }

  T m_value;
};

}  // namespace DataModel
