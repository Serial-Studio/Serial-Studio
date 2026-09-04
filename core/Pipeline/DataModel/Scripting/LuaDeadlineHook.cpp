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

#include "DataModel/Scripting/LuaDeadlineHook.h"

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
}
// clang-format on

#include <QtGlobal>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Registry binding
//--------------------------------------------------------------------------------------------------

static const char* const kDeadlineRegistryKey = "__ss_lua_deadline__";

/**
 * @brief What the hook reads out of the registry: the deadline the host arms and disarms, plus the
 *        budget and label the timeout error names. Allocated as Lua userdata, so the state frees it
 *        and a trivially destructible layout is a requirement, not a convenience.
 */
struct LuaDeadlineContext {
  QDeadlineTimer* deadline;
  int budgetMs;
  const char* label;
};

/**
 * @brief LUA_MASKCOUNT hook: raises a Lua error once the armed deadline elapses. luaL_error()
 *        longjmps out of the running chunk, which is why this only ever runs on the thread that
 *        owns the state -- the error has to unwind that thread's own protected call.
 */
static void luaDeadlineHookFn(lua_State* L, lua_Debug* ar)
{
  Q_UNUSED(ar)

  lua_getfield(L, LUA_REGISTRYINDEX, kDeadlineRegistryKey);
  auto* context = static_cast<LuaDeadlineContext*>(lua_touserdata(L, -1));
  lua_pop(L, 1);

  if (!context || !context->deadline) [[unlikely]]
    return;

  if (context->deadline->hasExpired()) [[unlikely]]
    luaL_error(L, "%s timed out after %d ms", context->label, context->budgetMs);
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a disarmed guard; @p label must outlive every state it is installed on, so it is a
 *        string literal at every call site.
 */
DataModel::LuaDeadlineHook::LuaDeadlineHook(int budgetMs, const char* label)
  : m_deadline(QDeadlineTimer::Forever)
  , m_budgetMs(budgetMs)
  , m_label(label ? label : "script")
  , m_timedOut(false)
{
  SS_ASSERT(budgetMs > 0, m_budgetMs = 1000);
  SS_ASSERT_LOG(label != nullptr);
}

//--------------------------------------------------------------------------------------------------
// Installation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Points @p L's count hook at this object's deadline. The object must outlive the state:
 *        the registry keeps a raw pointer to the timer armed below.
 */
void DataModel::LuaDeadlineHook::install(lua_State* L)
{
  SS_ASSERT(L != nullptr, return);

  bind(L, &m_deadline, m_budgetMs, m_label);
  enable(L);
}

/**
 * @brief Registers the deadline the hook watches. Allocates one userdata, so a caller that has a
 *        protected bootstrap (lua_pcall) runs this inside it and keeps lua_atpanic unreachable.
 */
void DataModel::LuaDeadlineHook::bind(lua_State* L,
                                      QDeadlineTimer* deadline,
                                      int budgetMs,
                                      const char* label)
{
  SS_ASSERT(L != nullptr, return);
  SS_ASSERT(deadline != nullptr, return);
  SS_ASSERT_LOG(budgetMs > 0);

  auto* context = static_cast<LuaDeadlineContext*>(lua_newuserdata(L, sizeof(LuaDeadlineContext)));
  context->deadline = deadline;
  context->budgetMs = budgetMs;
  context->label    = label ? label : "script";
  lua_setfield(L, LUA_REGISTRYINDEX, kDeadlineRegistryKey);
}

/**
 * @brief Turns the count hook on for a state already bound to a deadline. Allocation-free, and
 *        never called in Fast mode: hooks do not fire inside JIT-compiled traces.
 */
void DataModel::LuaDeadlineHook::enable(lua_State* L)
{
  SS_ASSERT(L != nullptr, return);
  SS_ASSERT_LOG(kLuaHookInstructionCount > 0);

  lua_sethook(L, &luaDeadlineHookFn, LUA_MASKCOUNT, kLuaHookInstructionCount);
}

//--------------------------------------------------------------------------------------------------
// Arm / disarm
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts the budget for one chunk or one call; clears the previous verdict.
 */
void DataModel::LuaDeadlineHook::arm() noexcept
{
  m_timedOut = false;
  m_deadline.setRemainingTime(m_budgetMs);
}

/**
 * @brief Retires the budget and latches whether it had elapsed, so the caller can tell a timeout
 *        apart from an ordinary script error after the call returned.
 */
void DataModel::LuaDeadlineHook::disarm() noexcept
{
  m_timedOut = m_deadline.hasExpired();
  m_deadline = QDeadlineTimer(QDeadlineTimer::Forever);
}

//--------------------------------------------------------------------------------------------------
// Accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the most recent armed run reached its deadline.
 */
bool DataModel::LuaDeadlineHook::timedOut() const noexcept
{
  return m_timedOut;
}

/**
 * @brief Returns the per-run budget in milliseconds.
 */
int DataModel::LuaDeadlineHook::budgetMs() const noexcept
{
  return m_budgetMs;
}
