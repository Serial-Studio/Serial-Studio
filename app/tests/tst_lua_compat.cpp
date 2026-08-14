/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
// clang-format on

#include <QByteArray>
#include <QTest>

#include "DataModel/Scripting/LuaCompat.h"
#include "DataModel/Scripting/LuaCompatJIT.h"
#include "SSAssert.h"

// Every test function here is self-contained: each opens its own lua_State (via LuaState below)
// and installs only the shim pieces it exercises, so Qt Test's declaration-order execution is
// never load-bearing.

//--------------------------------------------------------------------------------------------------
// Test harness
//--------------------------------------------------------------------------------------------------

namespace {

/**
 * @brief RAII lua_State that opens the same restricted "_G/table/string/math" subset every
 *        production caller of LuaCompat uses (LuaScriptEngine, PublisherScript, FrameBuilder's
 *        transform engine, ...) -- never the full luaL_openlibs(), which no caller in this repo
 *        uses. Closes the state on scope exit so each slot below runs an independent interpreter.
 */
class LuaState {
public:
  LuaState() : m_state(luaL_newstate())
  {
    SS_ASSERT(m_state != nullptr, return);

    static const luaL_Reg kSafeLibs[] = {
      {    "_G",   luaopen_base},
      { "table",  luaopen_table},
      {"string", luaopen_string},
      {  "math",   luaopen_math},
      { nullptr,        nullptr}
    };

    for (const luaL_Reg* lib = kSafeLibs; lib->func; ++lib) {
      luaL_requiref(m_state, lib->name, lib->func, 1);
      lua_pop(m_state, 1);
    }
  }

  ~LuaState()
  {
    if (m_state != nullptr)
      lua_close(m_state);
  }

  LuaState(const LuaState&)            = delete;
  LuaState& operator=(const LuaState&) = delete;

  [[nodiscard]] lua_State* raw() const { return m_state; }

private:
  lua_State* m_state;
};

/**
 * @brief Runs a Lua chunk expected to succeed; returns an empty QByteArray on LUA_OK, otherwise
 *        the interpreter's own error message, so a failing QVERIFY2 shows the Lua-side reason
 *        instead of a bare boolean.
 */
QByteArray runOk(lua_State* L, const char* chunk)
{
  SS_ASSERT_LOG(L != nullptr);

  if (luaL_dostring(L, chunk) == LUA_OK)
    return QByteArray();

  const char* message = lua_tostring(L, -1);
  const QByteArray result(message != nullptr ? message : "unknown Lua error");
  lua_pop(L, 1);
  return result;
}

}  // namespace

/**
 * @brief Byte-level contract of the Lua 5.1/5.2 compatibility shim and console/os sandbox
 *        installed by DataModel::installLuaCompat/installLuaConsole/installLuaRestrictedOs.
 */
class TstLuaCompat : public QObject {
  Q_OBJECT

private slots:
  void mathAliasesMatchLua51Names();
  void hyperbolicFunctionsAndModAlias();
  void frexpLdexpRoundTrip();
  void tableMaxnGetnAndUnpack();
  void stringGfindAliasesGmatch();
  void trimStartswithEndswith();
  void bit32BitwiseOperations();
  void nativeStringSplitHandlesEmptyFieldsAndSeparator();
  void installLuaCompatIsIdempotent();
  void consoleAndPrintAcceptVariadicArgs();
  void restrictedOsExposesOnlyTimeFunctions();
  void runtimeErrorSurfacesWithoutAborting();
};

//--------------------------------------------------------------------------------------------------
// Math compatibility
//--------------------------------------------------------------------------------------------------

/**
 * @brief math.log10/pow/atan2 are re-added under their 5.1 names on top of Lua 5.4's log(x,base),
 *        exponentiation operator and two-argument atan.
 */
void TstLuaCompat::mathAliasesMatchLua51Names()
{
  LuaState lua;
  DataModel::installLuaCompat(lua.raw());

  const auto error = runOk(lua.raw(), R"LUA(
    assert(math.log10(100) == 2)
    assert(math.pow(2, 10) == 1024)
    assert(math.abs(math.atan2(1, 1) - math.pi / 4) < 1e-9)
  )LUA");
  QVERIFY2(error.isEmpty(), error.constData());
}

/**
 * @brief math.cosh/sinh/tanh match the known exp-based identities, and math.mod is the literal
 *        same function object as math.fmod, not merely an equivalent re-implementation.
 */
void TstLuaCompat::hyperbolicFunctionsAndModAlias()
{
  LuaState lua;
  DataModel::installLuaCompat(lua.raw());

  const auto error = runOk(lua.raw(), R"LUA(
    assert(math.cosh(0) == 1)
    assert(math.sinh(0) == 0)
    assert(math.tanh(0) == 0)
    assert(math.abs(math.cosh(1) - 1.5430806348152437) < 1e-9)
    assert(math.abs(math.sinh(1) - 1.1752011936438014) < 1e-9)
    assert(math.abs(math.tanh(1) - 0.7615941559557649) < 1e-9)

    assert(math.mod == math.fmod)
    assert(math.mod(7, 3) == 1)
  )LUA");
  QVERIFY2(error.isEmpty(), error.constData());
}

/**
 * @brief math.frexp/ldexp round-trip an arbitrary double, including the x == 0 special case the
 *        shim short-circuits before the log2-based exponent search.
 */
void TstLuaCompat::frexpLdexpRoundTrip()
{
  LuaState lua;
  DataModel::installLuaCompat(lua.raw());

  const auto error = runOk(lua.raw(), R"LUA(
    local m, e = math.frexp(100)
    assert(m >= 0.5 and m < 1)
    assert(math.ldexp(m, e) == 100)

    local nm, ne = math.frexp(-100)
    assert(nm <= -0.5 and nm > -1)
    assert(math.ldexp(nm, ne) == -100)

    local zm, ze = math.frexp(0)
    assert(zm == 0.0 and ze == 0)
  )LUA");
  QVERIFY2(error.isEmpty(), error.constData());
}

//--------------------------------------------------------------------------------------------------
// Table compatibility
//--------------------------------------------------------------------------------------------------

/**
 * @brief table.maxn walks past a hole to the highest integer key; table.getn matches the length
 *        operator on a contiguous table; the global unpack is table.unpack itself.
 */
void TstLuaCompat::tableMaxnGetnAndUnpack()
{
  LuaState lua;
  DataModel::installLuaCompat(lua.raw());

  const auto error = runOk(lua.raw(), R"LUA(
    local t = {}
    t[1] = "a"
    t[5] = "e"
    assert(table.maxn(t) == 5)

    local u = {10, 20, 30}
    assert(#u == 3)
    assert(table.getn(u) == #u)

    assert(unpack == table.unpack)
    local a, b, c = unpack(u)
    assert(a == 10 and b == 20 and c == 30)
    assert(select("#", unpack(u)) == 3)
  )LUA");
  QVERIFY2(error.isEmpty(), error.constData());
}

//--------------------------------------------------------------------------------------------------
// String compatibility
//--------------------------------------------------------------------------------------------------

/**
 * @brief string.gfind is the exact 5.4 string.gmatch, still usable as a stateless pattern
 *        iterator.
 */
void TstLuaCompat::stringGfindAliasesGmatch()
{
  LuaState lua;
  DataModel::installLuaCompat(lua.raw());

  const auto error = runOk(lua.raw(), R"LUA(
    assert(string.gfind == string.gmatch)

    local count = 0
    for _ in string.gfind("one two three", "%a+") do
      count = count + 1
    end
    assert(count == 3)
  )LUA");
  QVERIFY2(error.isEmpty(), error.constData());
}

/**
 * @brief string.trim strips only leading/trailing whitespace, leaving interior runs intact;
 *        startswith/endswith cover the true/false cases plus the empty-suffix edge that
 *        endswith treats as always matching.
 */
void TstLuaCompat::trimStartswithEndswith()
{
  LuaState lua;
  DataModel::installLuaCompat(lua.raw());

  const auto error = runOk(lua.raw(), R"LUA(
    assert(string.trim("  a b  ") == "a b")
    assert(string.trim("\t\nabc\t") == "abc")

    assert(string.startswith("hello", "he") == true)
    assert(string.startswith("hello", "lo") == false)

    assert(string.endswith("hello", "lo") == true)
    assert(string.endswith("hello", "he") == false)
    assert(string.endswith("hello", "") == true)
  )LUA");
  QVERIFY2(error.isEmpty(), error.constData());
}

//--------------------------------------------------------------------------------------------------
// bit32 compatibility
//--------------------------------------------------------------------------------------------------

/**
 * @brief The pure-Lua bit32 replacement matches known 32-bit patterns for the logical ops, clamps
 *        shifts of >= 32 bits to zero, sign-extends arshift, and round-trips extract/replace and
 *        the two rotate directions.
 */
void TstLuaCompat::bit32BitwiseOperations()
{
  LuaState lua;
  DataModel::installLuaCompat(lua.raw());

  const auto error = runOk(lua.raw(), R"LUA(
    assert(bit32.band(0xF0F0F0F0, 0x0F0F0F0F) == 0)
    assert(bit32.bor(0xF0F0F0F0, 0x0F0F0F0F) == 0xFFFFFFFF)
    assert(bit32.bxor(0xFF00FF00, 0x00FF00FF) == 0xFFFFFFFF)
    assert(bit32.bnot(0) == 0xFFFFFFFF)

    assert(bit32.lshift(1, 32) == 0)
    assert(bit32.rshift(1, 32) == 0)
    assert(bit32.lshift(1, 40) == 0)

    assert(bit32.arshift(0x80000000, 4) == 0xF8000000)
    assert(bit32.arshift(0x00000010, 4) == 0x00000001)

    local a = 0x12345678
    local v = bit32.extract(a, 8, 8)
    assert(v == 0x56)
    assert(bit32.replace(a, v, 8, 8) == a)

    local b = 0xA5A5A5A5
    assert(bit32.lrotate(bit32.rrotate(b, 5), 5) == b)
    assert(bit32.rrotate(bit32.lrotate(b, 7), 7) == b)
  )LUA");
  QVERIFY2(error.isEmpty(), error.constData());
}

//--------------------------------------------------------------------------------------------------
// Native string.split
//--------------------------------------------------------------------------------------------------

/**
 * @brief The native string.split keeps empty fields between consecutive separators, returns the
 *        whole string as a single element when the separator is empty or absent from the input,
 *        and handles an empty input string the same way.
 */
void TstLuaCompat::nativeStringSplitHandlesEmptyFieldsAndSeparator()
{
  LuaState lua;
  DataModel::installLuaCompat(lua.raw());

  const auto error = runOk(lua.raw(), R"LUA(
    local parts = string.split("a,,b", ",")
    assert(#parts == 3)
    assert(parts[1] == "a")
    assert(parts[2] == "")
    assert(parts[3] == "b")

    local whole = string.split("abc", "")
    assert(#whole == 1)
    assert(whole[1] == "abc")

    local none = string.split("nosep", ",")
    assert(#none == 1)
    assert(none[1] == "nosep")

    local empty = string.split("", ",")
    assert(#empty == 1)
    assert(empty[1] == "")
  )LUA");
  QVERIFY2(error.isEmpty(), error.constData());
}

//--------------------------------------------------------------------------------------------------
// Idempotent installation
//--------------------------------------------------------------------------------------------------

/**
 * @brief installLuaCompat guards every definition behind "if x == nil", so a second call on the
 *        same state (e.g. a script reload) is a no-op rather than an error.
 */
void TstLuaCompat::installLuaCompatIsIdempotent()
{
  LuaState lua;
  DataModel::installLuaCompat(lua.raw());
  DataModel::installLuaCompat(lua.raw());

  const auto error = runOk(lua.raw(), R"LUA(
    assert(math.log10(100) == 2)
    assert(unpack({1, 2, 3}) == 1)

    local parts = string.split("a,b", ",")
    assert(#parts == 2 and parts[1] == "a" and parts[2] == "b")
  )LUA");
  QVERIFY2(error.isEmpty(), error.constData());
}

//--------------------------------------------------------------------------------------------------
// Console logging
//--------------------------------------------------------------------------------------------------

/**
 * @brief print() and every console.* level accept zero or a mixed-type argument list without
 *        raising a Lua error, whatever the Qt message handler does with the resulting text.
 */
void TstLuaCompat::consoleAndPrintAcceptVariadicArgs()
{
  LuaState lua;
  DataModel::installLuaConsole(lua.raw());

  const auto error = runOk(lua.raw(), R"LUA(
    print()
    print("one", 2, 3.5)

    console.log()
    console.log("x", 1)
    console.debug("d")
    console.info("i", 2)
    console.warn("w")
    console.error("e", true)
  )LUA");
  QVERIFY2(error.isEmpty(), error.constData());
}

//--------------------------------------------------------------------------------------------------
// Restricted os sandbox
//--------------------------------------------------------------------------------------------------

/**
 * @brief installLuaRestrictedOs publishes only the side-effect-free time functions; the
 *        process/filesystem entries the full os library carries never reach the global os table.
 */
void TstLuaCompat::restrictedOsExposesOnlyTimeFunctions()
{
  LuaState lua;
  DataModel::installLuaRestrictedOs(lua.raw());

  const auto error = runOk(lua.raw(), R"LUA(
    assert(type(os) == "table")
    assert(type(os.time) == "function")
    assert(type(os.date) == "function")
    assert(type(os.clock) == "function")
    assert(type(os.difftime) == "function")

    assert(os.execute == nil)
    assert(os.remove == nil)
    assert(os.getenv == nil)
  )LUA");
  QVERIFY2(error.isEmpty(), error.constData());
}

//--------------------------------------------------------------------------------------------------
// Runtime error handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief A Lua runtime error surfaces as a non-LUA_OK luaL_dostring status carrying the error
 *        text; the host process never aborts and the same state keeps working afterward.
 */
void TstLuaCompat::runtimeErrorSurfacesWithoutAborting()
{
  LuaState lua;
  DataModel::installLuaCompat(lua.raw());

  const int status = luaL_dostring(lua.raw(), "error('boom')");
  QVERIFY(status != LUA_OK);

  const char* message = lua_tostring(lua.raw(), -1);
  QVERIFY(message != nullptr);
  QVERIFY(QByteArray(message).contains("boom"));
  lua_pop(lua.raw(), 1);

  const auto error = runOk(lua.raw(), "assert(math.log10(100) == 2)");
  QVERIFY2(error.isEmpty(), error.constData());
}

QTEST_APPLESS_MAIN(TstLuaCompat)

#include "tst_lua_compat.moc"
