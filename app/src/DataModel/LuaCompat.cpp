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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/LuaCompat.h"

#include <lauxlib.h>
#include <lua.h>

#include <QDebug>

//--------------------------------------------------------------------------------------------------
// Compatibility shim for Lua 5.1 / 5.2 names removed in 5.3 / 5.4
//--------------------------------------------------------------------------------------------------

// Re-adds 5.1/5.2 math/table/unpack/bit32 names -- pure Lua, watchdog applies.
static const char* const kLuaCompat = R"LUA(
-- math: removed/renamed in 5.3+
if math.log10 == nil then math.log10 = function(x) return math.log(x, 10) end end
if math.pow   == nil then math.pow   = function(x, y) return x ^ y end end
if math.atan2 == nil then math.atan2 = function(y, x) return math.atan(y, x) end end
if math.cosh  == nil then math.cosh  = function(x) return (math.exp(x) + math.exp(-x)) * 0.5 end end
if math.sinh  == nil then math.sinh  = function(x) return (math.exp(x) - math.exp(-x)) * 0.5 end end
if math.tanh  == nil then math.tanh  = function(x)
  local ex, en = math.exp(x), math.exp(-x); return (ex - en) / (ex + en)
end end
if math.frexp == nil then math.frexp = function(x)
  if x == 0 then return 0.0, 0 end
  local s = (x < 0) and -1 or 1
  local a = math.abs(x)
  local e = math.floor(math.log(a, 2)) + 1
  local m = a / (2 ^ e)
  return s * m, e
end end
if math.ldexp == nil then math.ldexp = function(m, e) return m * (2 ^ e) end end
if math.mod   == nil then math.mod   = math.fmod end

-- table: legacy aliases
if table.maxn == nil then table.maxn = function(t)
  local n = 0
  for k, _ in pairs(t) do
    if type(k) == "number" and k > n then n = k end
  end
  return n
end end
if table.getn == nil then table.getn = function(t) return #t end end

-- string: legacy + commonly-expected helpers
if string.gfind == nil then string.gfind = string.gmatch end
if string.split == nil then string.split = function(s, sep)
  if sep == nil or sep == "" then sep = "%s" end
  local out, i = {}, 1
  for piece in string.gmatch(s, "([^" .. sep .. "]+)") do
    out[i] = piece; i = i + 1
  end
  return out
end end
if string.trim == nil then string.trim = function(s)
  return (s:gsub("^%s+", ""):gsub("%s+$", ""))
end end
if string.startswith == nil then string.startswith = function(s, p)
  return string.sub(s, 1, #p) == p
end end
if string.endswith == nil then string.endswith = function(s, p)
  return p == "" or string.sub(s, -#p) == p
end end

-- globals: 5.1 had global unpack, moved to table.unpack in 5.2
if unpack == nil and table.unpack ~= nil then unpack = table.unpack end

-- bit32 was removed in 5.3; provide a portable replacement (32-bit unsigned)
if bit32 == nil then
  local function tou32(x)
    x = math.floor(x)
    x = x % 4294967296
    if x < 0 then x = x + 4294967296 end
    return x
  end
  local function band2(a, b)
    a, b = tou32(a), tou32(b)
    local r, p = 0, 1
    for _ = 1, 32 do
      if (a % 2 == 1) and (b % 2 == 1) then r = r + p end
      a = math.floor(a / 2); b = math.floor(b / 2); p = p * 2
    end
    return r
  end
  local function bor2(a, b)
    a, b = tou32(a), tou32(b)
    local r, p = 0, 1
    for _ = 1, 32 do
      if (a % 2 == 1) or (b % 2 == 1) then r = r + p end
      a = math.floor(a / 2); b = math.floor(b / 2); p = p * 2
    end
    return r
  end
  local function bxor2(a, b)
    a, b = tou32(a), tou32(b)
    local r, p = 0, 1
    for _ = 1, 32 do
      if (a % 2) ~= (b % 2) then r = r + p end
      a = math.floor(a / 2); b = math.floor(b / 2); p = p * 2
    end
    return r
  end
  bit32 = {}
  bit32.band = function(...)
    local args = {...}
    if #args == 0 then return 0xFFFFFFFF end
    local r = tou32(args[1])
    for i = 2, #args do r = band2(r, args[i]) end
    return r
  end
  bit32.bor = function(...)
    local args = {...}
    if #args == 0 then return 0 end
    local r = tou32(args[1])
    for i = 2, #args do r = bor2(r, args[i]) end
    return r
  end
  bit32.bxor = function(...)
    local args = {...}
    if #args == 0 then return 0 end
    local r = tou32(args[1])
    for i = 2, #args do r = bxor2(r, args[i]) end
    return r
  end
  bit32.bnot    = function(a) return tou32(-1 - tou32(a)) end
  bit32.lshift  = function(a, n)
    n = math.floor(n)
    if n >= 32 or n <= -32 then return 0 end
    if n < 0 then return bit32.rshift(a, -n) end
    return tou32(tou32(a) * (2 ^ n))
  end
  bit32.rshift  = function(a, n)
    n = math.floor(n)
    if n >= 32 or n <= -32 then return 0 end
    if n < 0 then return bit32.lshift(a, -n) end
    return math.floor(tou32(a) / (2 ^ n))
  end
  bit32.arshift = function(a, n)
    n = math.floor(n)
    a = tou32(a)
    if n <= 0 then return bit32.lshift(a, -n) end
    if n >= 32 then return (a >= 0x80000000) and 0xFFFFFFFF or 0 end
    local r = math.floor(a / (2 ^ n))
    if a >= 0x80000000 then
      local mask = 0xFFFFFFFF - math.floor(0xFFFFFFFF / (2 ^ n))
      r = bit32.bor(r, mask)
    end
    return r
  end
  bit32.extract = function(a, field, width)
    width = width or 1
    return bit32.band(bit32.rshift(a, field), bit32.lshift(1, width) - 1)
  end
  bit32.replace = function(a, v, field, width)
    width = width or 1
    local mask = bit32.lshift(bit32.lshift(1, width) - 1, field)
    return bit32.bor(bit32.band(a, bit32.bnot(mask)),
                     bit32.band(bit32.lshift(v, field), mask))
  end
  bit32.lrotate = function(a, n)
    n = n % 32
    return bit32.bor(bit32.lshift(a, n), bit32.rshift(a, 32 - n))
  end
  bit32.rrotate = function(a, n)
    n = n % 32
    return bit32.bor(bit32.rshift(a, n), bit32.lshift(a, 32 - n))
  end
  bit32.btest = function(...) return bit32.band(...) ~= 0 end
end
)LUA";

/**
 * @brief Loads the Lua compatibility shim into the given state.
 */
void DataModel::installLuaCompat(lua_State* L)
{
  Q_ASSERT(L != nullptr);

  if (luaL_dostring(L, kLuaCompat) != LUA_OK) [[unlikely]] {
    qWarning() << "[LuaCompat] Failed to install compatibility shim:" << lua_tostring(L, -1);
    lua_pop(L, 1);
  }
}
