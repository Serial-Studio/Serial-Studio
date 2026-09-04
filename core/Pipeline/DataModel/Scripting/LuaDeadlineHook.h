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

// clang-format off
extern "C" {
#include <lua.h>
}
// clang-format on

#include <QDeadlineTimer>

namespace DataModel {

// Instructions between count-hook runs, shared by every Safe-mode Lua watchdog in the tree
inline constexpr int kLuaHookInstructionCount = 10000;

/**
 * @brief The one Lua execution deadline: a LUA_MASKCOUNT hook raising a Lua error once the armed
 *        budget elapses, so a runaway chunk errors instead of pinning its thread. The error unwinds
 *        the state's own pcall (hence: state's thread only), Fast mode never installs it (hooks
 *        never fire inside JIT traces), and the deadline and label outlive the state.
 */
class LuaDeadlineHook {
public:
  LuaDeadlineHook(int budgetMs, const char* label);

  LuaDeadlineHook(const LuaDeadlineHook&)            = delete;
  LuaDeadlineHook& operator=(const LuaDeadlineHook&) = delete;

  void install(lua_State* L);
  void arm() noexcept;
  void disarm() noexcept;

  [[nodiscard]] bool timedOut() const noexcept;
  [[nodiscard]] int budgetMs() const noexcept;

  static void bind(lua_State* L, QDeadlineTimer* deadline, int budgetMs, const char* label);
  static void enable(lua_State* L);

private:
  QDeadlineTimer m_deadline;
  int m_budgetMs;
  const char* m_label;
  bool m_timedOut;
};

}  // namespace DataModel
