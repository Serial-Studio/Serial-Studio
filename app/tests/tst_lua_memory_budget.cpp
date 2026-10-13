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

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
}
// clang-format on

#include <cstddef>
#include <QString>
#include <QTest>

#include "DataModel/Scripting/LuaMemoryBudget.h"

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

static constexpr std::size_t kMebibyte    = std::size_t(1024) * 1024;
static constexpr std::size_t kSmallBudget = 4 * kMebibyte;
static constexpr int kHookInstructions    = 1000;

static const char* const kBoundedChunk = "local t = {} for i = 1, 1000 do t[i] = i end return #t";

static const char* const kRunawayChunk = "local t = {} for i = 1, 1e8 do t[i] = {i} end return #t";

static const char* const kOversizedChunk = "return string.rep('x', 5 * 1024 * 1024)";

static void budgetHook(lua_State* L, lua_Debug* ar)
{
  Q_UNUSED(ar)
  DataModel::LuaMemoryBudget::check(L);
}

[[nodiscard]] static lua_State* newInterpreterState()
{
  lua_State* L = luaL_newstate();
  if (!L)
    return nullptr;

  luaL_openlibs(L);
  luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);
  lua_sethook(L, &budgetHook, LUA_MASKCOUNT, kHookInstructions);
  return L;
}

[[nodiscard]] static int runChunk(lua_State* L, const char* code)
{
  if (luaL_loadstring(L, code) != LUA_OK)
    return LUA_ERRSYNTAX;

  return lua_pcall(L, 0, LUA_MULTRET, 0);
}

[[nodiscard]] static std::size_t collectorBytes(lua_State* L)
{
  return static_cast<std::size_t>(lua_gc(L, LUA_GCCOUNT, 0)) * 1024
       + static_cast<std::size_t>(lua_gc(L, LUA_GCCOUNTB, 0));
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class LuaMemoryBudgetTests : public QObject {
  Q_OBJECT

private slots:
  void boundedChunkRunsUntouched();
  void usedBytesTrackTheCollector();
  void runawayChunkErrorsFromTheHook();
  void oversizedRequestIsRefused();
  void stateStaysUsableAfterTheError();
  void closeRestoresTheAllocator();
  void plainStateIsIgnored();
};

//--------------------------------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------------------------------

void LuaMemoryBudgetTests::boundedChunkRunsUntouched()
{
  lua_State* L = newInterpreterState();
  QVERIFY(L != nullptr);

  DataModel::LuaMemoryBudget budget(kSmallBudget);
  budget.install(L);
  QVERIFY(budget.installed());
  QCOMPARE(budget.limitBytes(), kSmallBudget);

  QCOMPARE(runChunk(L, kBoundedChunk), int(LUA_OK));
  QVERIFY(lua_isnumber(L, -1));
  QCOMPARE(static_cast<int>(lua_tointeger(L, -1)), 1000);
  QVERIFY(!budget.exceeded());
  QVERIFY(budget.usedBytes() > 0);
  QVERIFY(budget.usedBytes() < kSmallBudget);

  DataModel::LuaMemoryBudget::closeState(L);
}

void LuaMemoryBudgetTests::usedBytesTrackTheCollector()
{
  lua_State* L = newInterpreterState();
  QVERIFY(L != nullptr);

  DataModel::LuaMemoryBudget budget(kSmallBudget);
  budget.install(L);
  QCOMPARE(budget.usedBytes(), collectorBytes(L));

  QCOMPARE(runChunk(L, "keep = {} for i = 1, 20000 do keep[i] = i end"), int(LUA_OK));
  QCOMPARE(budget.usedBytes(), collectorBytes(L));
  QVERIFY(budget.usedBytes() > 20000 * sizeof(double));

  QCOMPARE(runChunk(L, "keep = nil collectgarbage()"), int(LUA_OK));
  QCOMPARE(budget.usedBytes(), collectorBytes(L));

  DataModel::LuaMemoryBudget::closeState(L);
}

void LuaMemoryBudgetTests::runawayChunkErrorsFromTheHook()
{
  lua_State* L = newInterpreterState();
  QVERIFY(L != nullptr);

  DataModel::LuaMemoryBudget budget(kSmallBudget);
  budget.install(L);

  QCOMPARE(runChunk(L, kRunawayChunk), int(LUA_ERRRUN));
  QVERIFY(budget.exceeded());

  const QString message = DataModel::LuaMemoryBudget::errorText(L);
  QVERIFY2(message.contains(QStringLiteral("Lua memory budget exceeded")), qPrintable(message));
  QVERIFY2(message.contains(QStringLiteral("limit 4 MiB")), qPrintable(message));

  DataModel::LuaMemoryBudget::closeState(L);
}

void LuaMemoryBudgetTests::oversizedRequestIsRefused()
{
  lua_State* L = newInterpreterState();
  QVERIFY(L != nullptr);

  DataModel::LuaMemoryBudget budget(kSmallBudget);
  budget.install(L);

  QCOMPARE(runChunk(L, kOversizedChunk), int(LUA_ERRMEM));
  QVERIFY(budget.exceeded());

  const QString message = DataModel::LuaMemoryBudget::errorText(L);
  QVERIFY2(message.contains(QStringLiteral("single allocation of 4 MiB or more")),
           qPrintable(message));

  DataModel::LuaMemoryBudget::closeState(L);
}

void LuaMemoryBudgetTests::stateStaysUsableAfterTheError()
{
  lua_State* L = newInterpreterState();
  QVERIFY(L != nullptr);

  DataModel::LuaMemoryBudget budget(kSmallBudget);
  budget.install(L);

  QCOMPARE(runChunk(L, kRunawayChunk), int(LUA_ERRRUN));
  QVERIFY(budget.exceeded());
  lua_settop(L, 0);

  QCOMPARE(runChunk(L, "collectgarbage() return 42"), int(LUA_OK));
  QVERIFY(lua_isnumber(L, -1));
  QCOMPARE(static_cast<int>(lua_tointeger(L, -1)), 42);
  QVERIFY(budget.usedBytes() < kSmallBudget);
  lua_settop(L, 0);

  QCOMPARE(runChunk(L, kBoundedChunk), int(LUA_OK));
  QCOMPARE(static_cast<int>(lua_tointeger(L, -1)), 1000);

  DataModel::LuaMemoryBudget::closeState(L);
}

void LuaMemoryBudgetTests::closeRestoresTheAllocator()
{
  lua_State* L = newInterpreterState();
  QVERIFY(L != nullptr);

  void* originalUd           = nullptr;
  const lua_Alloc originalFn = lua_getallocf(L, &originalUd);

  DataModel::LuaMemoryBudget budget(kSmallBudget);
  budget.install(L);

  void* wrappedUd           = nullptr;
  const lua_Alloc wrappedFn = lua_getallocf(L, &wrappedUd);
  QVERIFY(wrappedFn != originalFn);
  QCOMPARE(wrappedUd, static_cast<void*>(&budget));

  DataModel::LuaMemoryBudget::closeState(L);
  QVERIFY(!budget.installed());

  DataModel::LuaMemoryBudget::closeState(nullptr);
}

void LuaMemoryBudgetTests::plainStateIsIgnored()
{
  lua_State* L = newInterpreterState();
  QVERIFY(L != nullptr);

  QCOMPARE(runChunk(L, kBoundedChunk), int(LUA_OK));
  lua_settop(L, 0);

  QCOMPARE(runChunk(L, "error('plain failure')"), int(LUA_ERRRUN));
  const QString message = DataModel::LuaMemoryBudget::errorText(L);
  QVERIFY2(message.contains(QStringLiteral("plain failure")), qPrintable(message));

  DataModel::LuaMemoryBudget::closeState(L);
}

QTEST_APPLESS_MAIN(LuaMemoryBudgetTests)
#include "tst_lua_memory_budget.moc"
