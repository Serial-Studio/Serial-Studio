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
}
// clang-format on

#include <QDeadlineTimer>
#include <QElapsedTimer>
#include <QString>
#include <QTest>

#include "DataModel/Scripting/LuaDeadlineHook.h"

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

// Wide enough that "it tripped" is about the budget and not about scheduler jitter; every assertion
// on wall time is a one-sided lower bound.
static constexpr int kTripBudgetMs = 250;

static constexpr int kGenerousBudgetMs = 30000;

static const char* const kInfiniteLoop = "local i = 0 while true do i = (i + 1) % 1000003 end";

static const char* const kBoundedLoop =
  "local s = 0 for i = 1, 200000 do s = (s + i) % 65521 end return s";

/**
 * @brief Builds a bare interpreter state with the JIT off, the shape every Safe-mode consumer of
 *        the hook creates. No library is opened: the chunks below are pure VM code, and hooks do
 *        not fire inside compiled traces, so the mode switch is what makes the test meaningful.
 *        The caller closes it.
 */
[[nodiscard]] static lua_State* newInterpreterState()
{
  lua_State* L = luaL_newstate();
  if (!L)
    return nullptr;

  luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);
  return L;
}

/**
 * @brief Loads and runs one chunk, returning the pcall status.
 */
[[nodiscard]] static int runChunk(lua_State* L, const char* code)
{
  if (luaL_loadstring(L, code) != LUA_OK)
    return LUA_ERRSYNTAX;

  return lua_pcall(L, 0, LUA_MULTRET, 0);
}

/**
 * @brief Contract of DataModel::LuaDeadlineHook: a runaway chunk becomes a Lua error inside the
 *        budget, a chunk that finishes is untouched, the verdict latches on disarm, and a state
 *        that was never bound or never enabled runs without a deadline.
 */
class LuaDeadlineHookTests : public QObject {
  Q_OBJECT

private slots:
  void boundedChunkRunsUntouched();
  void infiniteLoopErrorsWithinBudget();
  void timeoutMessageNamesLabelAndBudget();
  void stateStaysUsableAfterATimeout();
  void disarmedStateIsNeverInterrupted();
  void externalDeadlineIsHonoured();
};

//--------------------------------------------------------------------------------------------------
// Pass-through
//--------------------------------------------------------------------------------------------------

/**
 * @brief A chunk that completes inside its budget returns normally and reports no timeout.
 */
void LuaDeadlineHookTests::boundedChunkRunsUntouched()
{
  lua_State* L = newInterpreterState();
  QVERIFY(L != nullptr);

  DataModel::LuaDeadlineHook hook(kGenerousBudgetMs, "test");
  hook.install(L);
  QCOMPARE(hook.budgetMs(), kGenerousBudgetMs);

  hook.arm();
  const int status = runChunk(L, kBoundedLoop);
  hook.disarm();

  QCOMPARE(status, int(LUA_OK));
  QVERIFY(!hook.timedOut());
  QVERIFY(lua_isnumber(L, -1));

  lua_close(L);
}

//--------------------------------------------------------------------------------------------------
// Interruption
//--------------------------------------------------------------------------------------------------

/**
 * @brief The whole point: `while true do end` comes back as a Lua error once the budget elapses,
 *        instead of pinning the calling thread forever.
 */
void LuaDeadlineHookTests::infiniteLoopErrorsWithinBudget()
{
  lua_State* L = newInterpreterState();
  QVERIFY(L != nullptr);

  DataModel::LuaDeadlineHook hook(kTripBudgetMs, "test");
  hook.install(L);

  QElapsedTimer timer;
  timer.start();
  hook.arm();
  const int status       = runChunk(L, kInfiniteLoop);
  const qint64 elapsedMs = timer.elapsed();
  hook.disarm();

  QCOMPARE(status, int(LUA_ERRRUN));
  QVERIFY(hook.timedOut());
  QVERIFY(elapsedMs >= kTripBudgetMs);

  lua_close(L);
}

/**
 * @brief The raised error names the label and the budget, which is what every consumer shows the
 *        user and what the transform lane's message has always said.
 */
void LuaDeadlineHookTests::timeoutMessageNamesLabelAndBudget()
{
  lua_State* L = newInterpreterState();
  QVERIFY(L != nullptr);

  DataModel::LuaDeadlineHook hook(kTripBudgetMs, "transform");
  hook.install(L);

  hook.arm();
  const int status = runChunk(L, kInfiniteLoop);
  hook.disarm();

  QCOMPARE(status, int(LUA_ERRRUN));
  const QString message = QString::fromUtf8(lua_tostring(L, -1));
  QVERIFY(message.contains(QStringLiteral("transform timed out after 250 ms")));

  lua_close(L);
}

/**
 * @brief A timeout is not terminal for the state: the next armed chunk runs to completion and the
 *        latched verdict resets.
 */
void LuaDeadlineHookTests::stateStaysUsableAfterATimeout()
{
  lua_State* L = newInterpreterState();
  QVERIFY(L != nullptr);

  DataModel::LuaDeadlineHook hook(kTripBudgetMs, "test");
  hook.install(L);

  hook.arm();
  const int stuck = runChunk(L, kInfiniteLoop);
  hook.disarm();
  QCOMPARE(stuck, int(LUA_ERRRUN));
  QVERIFY(hook.timedOut());

  lua_settop(L, 0);

  hook.arm();
  const int status = runChunk(L, "return 42");
  hook.disarm();

  QCOMPARE(status, int(LUA_OK));
  QVERIFY(!hook.timedOut());
  QCOMPARE(int(lua_tointeger(L, -1)), 42);

  lua_close(L);
}

//--------------------------------------------------------------------------------------------------
// Arming
//--------------------------------------------------------------------------------------------------

/**
 * @brief With the hook installed but never armed the deadline is Forever, so an ordinary chunk is
 *        not cut off however long the state has been alive.
 */
void LuaDeadlineHookTests::disarmedStateIsNeverInterrupted()
{
  lua_State* L = newInterpreterState();
  QVERIFY(L != nullptr);

  DataModel::LuaDeadlineHook hook(kTripBudgetMs, "test");
  hook.install(L);

  QTest::qWait(kTripBudgetMs * 2);
  const int status = runChunk(L, kBoundedLoop);

  QCOMPARE(status, int(LUA_OK));
  QVERIFY(!hook.timedOut());

  lua_close(L);
}

/**
 * @brief bind() accepts a deadline the caller owns -- the transform lane's shape, where the timer
 *        lives in the engine record and the compiler only installs the hook.
 */
void LuaDeadlineHookTests::externalDeadlineIsHonoured()
{
  lua_State* L = newInterpreterState();
  QVERIFY(L != nullptr);

  QDeadlineTimer deadline(QDeadlineTimer::Forever);
  DataModel::LuaDeadlineHook::bind(L, &deadline, kTripBudgetMs, "external");
  DataModel::LuaDeadlineHook::enable(L);

  deadline.setRemainingTime(kTripBudgetMs);
  const int status = runChunk(L, kInfiniteLoop);
  deadline         = QDeadlineTimer(QDeadlineTimer::Forever);

  QCOMPARE(status, int(LUA_ERRRUN));
  const QString message = QString::fromUtf8(lua_tostring(L, -1));
  QVERIFY(message.contains(QStringLiteral("external timed out after 250 ms")));

  lua_close(L);
}

QTEST_GUILESS_MAIN(LuaDeadlineHookTests)

#include "tst_lua_deadline_hook.moc"
