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
#include <lauxlib.h>
#include <lua.h>
}
// clang-format on

#include <cmath>
#include <cstdint>

/*
 * C-API drift shims for the LuaJIT runtime swap (spec 0051 M2, R19). LuaJIT implements the
 * Lua 5.1 C API plus selected 5.2 additions (luaL_loadbufferx, luaL_setfuncs, LUA_OK,
 * lua_Debug.nparams); the functions below restore, under their canonical names, the newer-API
 * subset Serial Studio's call sites rely on, with 5.4-equivalent semantics. Include this
 * header AFTER the Lua headers in every TU that talks to the VM. All shims are cheap inline
 * wrappers; none allocate beyond what the wrapped operation itself allocates.
 */

/**
 * @brief 5.2 lua_absindex: converts a possibly-negative stack index to an absolute one.
 */
[[nodiscard]] inline int lua_absindex(lua_State* L, int idx)
{
  return (idx > 0 || idx <= LUA_REGISTRYINDEX) ? idx : lua_gettop(L) + idx + 1;
}

/**
 * @brief 5.2 lua_rawlen: raw length of a table/string/userdata (LuaJIT's lua_objlen has the
 *        same raw semantics for these types).
 */
[[nodiscard]] inline size_t lua_rawlen(lua_State* L, int idx)
{
  return lua_objlen(L, idx);
}

/**
 * @brief 5.3 lua_isinteger: LuaJIT numbers are doubles (single number type), so this reports
 *        whether the value is a number with an exact integral value in the lua_Integer range,
 *        preserving the int-vs-float display formatting the 5.4 call sites derive from it.
 */
[[nodiscard]] inline int lua_isinteger(lua_State* L, int idx)
{
  if (lua_type(L, idx) != LUA_TNUMBER)
    return 0;

  const double v = lua_tonumber(L, idx);
  return std::isfinite(v) && v == std::floor(v) && v >= -9007199254740992.0
      && v <= 9007199254740992.0;
}

/**
 * @brief 5.3 lua_geti: pushes t[n] (honoring metamethods) and returns the value's type.
 */
inline int lua_geti(lua_State* L, int idx, lua_Integer n)
{
  const int abs = lua_absindex(L, idx);
  lua_pushinteger(L, n);
  lua_gettable(L, abs);
  return lua_type(L, -1);
}

/**
 * @brief 5.3 lua_seti: performs t[n] = v (honoring metamethods), popping the value.
 */
inline void lua_seti(lua_State* L, int idx, lua_Integer n)
{
  const int abs = lua_absindex(L, idx);
  lua_pushinteger(L, n);
  lua_insert(L, -2);
  lua_settable(L, abs);
}

/**
 * @brief 5.2 luaL_len: length of the value at @p idx honoring __len; plain-table and string
 *        callers (the only ones in this codebase) get the raw length.
 */
[[nodiscard]] inline lua_Integer luaL_len(lua_State* L, int idx)
{
  return static_cast<lua_Integer>(lua_objlen(L, idx));
}

/**
 * @brief 5.2 lua_pushglobaltable: pushes the globals table (LuaJIT keeps the 5.1
 *        pseudo-index).
 */
inline void lua_pushglobaltable(lua_State* L)
{
  lua_pushvalue(L, LUA_GLOBALSINDEX);
}

/**
 * @brief 5.2 luaL_requiref: opens module @p openf under @p modname, optionally publishing it
 *        as a global, and leaves one copy of the module on the stack (5.2 contract).
 */
inline void luaL_requiref(lua_State* L, const char* modname, lua_CFunction openf, int glb)
{
  lua_pushcfunction(L, openf);
  lua_pushstring(L, modname);
  lua_call(L, 1, 1);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    lua_getglobal(L, modname);
  }

  if (glb) {
    lua_pushvalue(L, -1);
    lua_setglobal(L, modname);
  }
}

/**
 * @brief 5.2 luaL_tolstring: converts the value at @p idx to a string honoring __tostring,
 *        pushes the result, and returns its pointer (valid while it stays on the stack).
 */
inline const char* luaL_tolstring(lua_State* L, int idx, size_t* len)
{
  const int abs = lua_absindex(L, idx);
  if (luaL_callmeta(L, abs, "__tostring")) {
    if (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  } else {
    switch (lua_type(L, abs)) {
      case LUA_TNUMBER:
      case LUA_TSTRING:
        lua_pushvalue(L, abs);
        break;
      case LUA_TBOOLEAN:
        lua_pushstring(L, lua_toboolean(L, abs) ? "true" : "false");
        break;
      case LUA_TNIL:
        lua_pushliteral(L, "nil");
        break;
      default:
        lua_pushfstring(L, "%s: %p", luaL_typename(L, abs), lua_topointer(L, abs));
        break;
    }
  }

  return lua_tolstring(L, -1, len);
}

/**
 * @brief Binds the sandbox environment table at stack top-1 to the loaded chunk at top,
 *        replacing 5.4's set-first-upvalue _ENV idiom: LuaJIT uses 5.1 function
 *        environments, so this pops the env table and applies lua_setfenv to the chunk.
 *        Stack on entry: [... chunk env]; on exit: [... chunk].
 */
inline void luacompatSetChunkEnv(lua_State* L)
{
  lua_setfenv(L, -2);
}
