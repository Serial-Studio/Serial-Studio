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

#include "DataModel/Scripting/LuaMemoryBudget.h"

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
}
// clang-format on

#include <algorithm>
#include <QLatin1String>

#include "Core/SSAssert.h"

static constexpr std::size_t kMebibyte = std::size_t(1024) * 1024;

//--------------------------------------------------------------------------------------------------
// Constructor & accessors
//--------------------------------------------------------------------------------------------------

DataModel::LuaMemoryBudget::LuaMemoryBudget(std::size_t limitBytes) noexcept
  : m_inner(nullptr), m_innerUd(nullptr), m_used(0), m_limit(limitBytes), m_refused(false)
{
  SS_ASSERT(limitBytes > 0, m_limit = kLuaSafeModeMemoryBudgetBytes);
}

/**
 * @brief Whether a state currently routes its allocations through this budget.
 */
bool DataModel::LuaMemoryBudget::installed() const noexcept
{
  return m_inner != nullptr;
}

/**
 * @brief Whether the live total is over the limit right now, or a single request was refused.
 */
bool DataModel::LuaMemoryBudget::exceeded() const noexcept
{
  return m_refused || m_used > m_limit;
}

std::size_t DataModel::LuaMemoryBudget::usedBytes() const noexcept
{
  return m_used;
}

std::size_t DataModel::LuaMemoryBudget::limitBytes() const noexcept
{
  return m_limit;
}

//--------------------------------------------------------------------------------------------------
// Allocator wrapping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wraps the state's allocator, seeding the count from what the state already holds so the
 *        limit is an absolute footprint rather than growth since install.
 */
void DataModel::LuaMemoryBudget::install(lua_State* L)
{
  SS_ASSERT(L != nullptr, return);
  SS_ASSERT(m_inner == nullptr, return);

  m_inner   = lua_getallocf(L, &m_innerUd);
  m_used    = static_cast<std::size_t>(lua_gc(L, LUA_GCCOUNT, 0)) * 1024
            + static_cast<std::size_t>(lua_gc(L, LUA_GCCOUNTB, 0));
  m_refused = false;
  lua_setallocf(L, &budgetedAlloc, this);
}

/**
 * @brief Returns the budget installed on @p L, or nullptr when the state runs its own allocator.
 */
DataModel::LuaMemoryBudget* DataModel::LuaMemoryBudget::installedOn(lua_State* L) noexcept
{
  void* ud          = nullptr;
  const lua_Alloc f = lua_getallocf(L, &ud);
  return f == &budgetedAlloc ? static_cast<LuaMemoryBudget*>(ud) : nullptr;
}

/**
 * @brief lua_Alloc that forwards to the wrapped allocator and keeps the live byte count; a request
 *        for the whole limit in one piece is the only thing it ever refuses.
 */
void* DataModel::LuaMemoryBudget::budgetedAlloc(void* ud,
                                                void* ptr,
                                                std::size_t osize,
                                                std::size_t nsize)
{
  auto* budget = static_cast<LuaMemoryBudget*>(ud);
  SS_ASSERT(budget != nullptr, return nullptr);
  SS_ASSERT(budget->m_inner != nullptr, return nullptr);

  if (nsize >= budget->m_limit) [[unlikely]] {
    budget->m_refused = true;
    return nullptr;
  }

  void* out = budget->m_inner(budget->m_innerUd, ptr, osize, nsize);
  if (nsize == 0) {
    budget->m_used -= std::min(osize, budget->m_used);
    return out;
  }

  if (out)
    budget->m_used = budget->m_used + nsize - std::min(osize, budget->m_used);

  return out;
}

//--------------------------------------------------------------------------------------------------
// Hook check, teardown and error text
//--------------------------------------------------------------------------------------------------

/**
 * @brief Count-hook entry: raises a Lua error once the state is over budget. Runs on the thread
 *        that owns the state, inside the chunk's own protected call.
 */
void DataModel::LuaMemoryBudget::check(lua_State* L)
{
  SS_ASSERT(L != nullptr, return);

  const auto* budget = installedOn(L);
  if (!budget) [[likely]]
    return;

  if (budget->m_used > budget->m_limit) [[unlikely]]
    luaL_error(L,
               "Lua memory budget exceeded (%d MiB in use, limit %d MiB)",
               static_cast<int>(budget->m_used / kMebibyte),
               static_cast<int>(budget->m_limit / kMebibyte));
}

/**
 * @brief Restores the wrapped allocator and closes the state; LuaJIT only destroys its own arena
 *        when the allocator it finds at close time is its own.
 */
void DataModel::LuaMemoryBudget::closeState(lua_State* L)
{
  if (!L)
    return;

  if (auto* budget = installedOn(L)) {
    lua_setallocf(L, budget->m_inner, budget->m_innerUd);
    budget->m_inner   = nullptr;
    budget->m_innerUd = nullptr;
  }

  lua_close(L);
}

/**
 * @brief The error message on top of the stack, with the bare "not enough memory" LuaJIT raises on
 *        a refused request named as the budget refusal it was.
 */
QString DataModel::LuaMemoryBudget::errorText(lua_State* L)
{
  SS_ASSERT(L != nullptr, return QString());

  const char* message = lua_tostring(L, -1);
  const QString text  = QString::fromUtf8(message ? message : "");
  const auto* budget  = installedOn(L);
  if (!budget || !budget->m_refused || !text.contains(QLatin1String("not enough memory")))
    return text;

  return QStringLiteral("Lua memory budget exceeded (single allocation of %1 MiB or more)")
    .arg(budget->m_limit / kMebibyte);
}
