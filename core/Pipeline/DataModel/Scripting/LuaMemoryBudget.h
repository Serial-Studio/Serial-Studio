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

#include <cstddef>
#include <QString>

namespace DataModel {

/**
 * @brief Bytes a Safe-mode Lua state may hold before the count hook aborts the running chunk.
 */
inline constexpr std::size_t kLuaSafeModeMemoryBudgetBytes = std::size_t(256) * 1024 * 1024;

/**
 * @brief Byte budget for a Safe-mode Lua state. install() wraps the state's own allocator so
 *        every allocation is counted; the wrapper never refuses an ordinary request (a refusal
 *        outside a protected call would reach the panic handler), it only rejects a single request
 *        at or above the whole limit. check() runs from the engine's LUA_MASKCOUNT hook and raises
 *        a plain Lua error once the live total is over budget, so only the running chunk fails and
 *        the state stays usable. The object must outlive the state and must not move while
 *        installed: the state keeps its address as allocator userdata. closeState() restores the
 *        original allocator first, which is what lets LuaJIT tear its own arena down.
 */
class LuaMemoryBudget {
public:
  explicit LuaMemoryBudget(std::size_t limitBytes = kLuaSafeModeMemoryBudgetBytes) noexcept;

  LuaMemoryBudget(const LuaMemoryBudget&)            = delete;
  LuaMemoryBudget& operator=(const LuaMemoryBudget&) = delete;

  void install(lua_State* L);

  [[nodiscard]] bool installed() const noexcept;
  [[nodiscard]] bool exceeded() const noexcept;
  [[nodiscard]] std::size_t usedBytes() const noexcept;
  [[nodiscard]] std::size_t limitBytes() const noexcept;

  static void check(lua_State* L);
  static void closeState(lua_State* L);
  [[nodiscard]] static QString errorText(lua_State* L);

private:
  [[nodiscard]] static LuaMemoryBudget* installedOn(lua_State* L) noexcept;
  [[nodiscard]] static void* budgetedAlloc(void* ud,
                                           void* ptr,
                                           std::size_t osize,
                                           std::size_t nsize);

private:
  lua_Alloc m_inner;
  void* m_innerUd;
  std::size_t m_used;
  std::size_t m_limit;
  bool m_refused;
};

}  // namespace DataModel
