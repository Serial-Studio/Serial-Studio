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
}
// clang-format on

#include <QElapsedTimer>
#include <QJSValue>
#include <QString>
#include <QTest>

#include "DataModel/Scripting/ScriptDryRun.h"

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

// The JS watchdog thread polls every 20 ms, so a trip budget is several poll intervals wide and
// every timing assertion is a one-sided lower bound.
static constexpr int kTripBudgetMs = 250;

static constexpr int kGenerousBudgetMs = 30000;

static const QString kJsInfiniteLoop =
  QStringLiteral("var i = 0; while (true) { i = (i + 1) % 1000003; }");

static const QString kJsRunawayFunction =
  QStringLiteral("(function() { var i = 0; while (true) { i = (i + 1) % 1000003; } })");

static const QString kLuaInfiniteLoop =
  QStringLiteral("local i = 0 while true do i = (i + 1) % 1000003 end");

/**
 * @brief The guarantee every validate / test / preview / dryRun path now inherits: user code runs
 *        under a deadline in a throwaway engine, so a runaway script returns an error instead of
 *        freezing the thread that validated it.
 */
class ScriptDryRunTests : public QObject {
  Q_OBJECT

private slots:
  void jsSessionRunsBoundedCode();
  void jsRunawayEvaluationTimesOut();
  void jsRunawayCallTimesOut();
  void jsSyntaxErrorIsNotATimeout();

  void luaSessionRunsBoundedCode();
  void luaRunawayChunkTimesOut();
  void luaRunawayCallTimesOut();
  void luaSandboxHasNoFilesystemGlobals();

  void oneShotDryRunReportsOkErrorAndTimeout();
};

//--------------------------------------------------------------------------------------------------
// JavaScript sessions
//--------------------------------------------------------------------------------------------------

/**
 * @brief A session evaluates ordinary code, keeps the engine reachable for probes, and reports no
 *        timeout.
 */
void ScriptDryRunTests::jsSessionRunsBoundedCode()
{
  DataModel::ScriptDryRun session(
    DataModel::ScriptDryRun::Language::JavaScript, kGenerousBudgetMs, "test");
  QVERIFY(session.valid());
  QVERIFY(session.jsEngine() != nullptr);
  QVERIFY(session.luaState() == nullptr);

  const auto result = session.evaluate(
    QStringLiteral("function transform(v) { return v * 2; }; 21;"), QStringLiteral("test.js"));

  QVERIFY(!result.isError());
  QVERIFY(!session.timedOut());
  QVERIFY(session.jsEngine()->globalObject().property(QStringLiteral("transform")).isCallable());
}

/**
 * @brief Top-level `while(true){}` is interrupted inside the budget and reported as a timeout;
 *        the engine is usable afterwards, so a dialog can stay open with a message.
 */
void ScriptDryRunTests::jsRunawayEvaluationTimesOut()
{
  DataModel::ScriptDryRun session(
    DataModel::ScriptDryRun::Language::JavaScript, kTripBudgetMs, "test");
  QVERIFY(session.valid());

  QElapsedTimer timer;
  timer.start();
  const auto result      = session.evaluate(kJsInfiniteLoop, QStringLiteral("test.js"));
  const qint64 elapsedMs = timer.elapsed();

  QVERIFY(session.timedOut());
  QVERIFY(elapsedMs >= kTripBudgetMs);
  QVERIFY(result.isError());
  QVERIFY(!session.jsEngine()->isInterrupted());

  const auto after = session.evaluate(QStringLiteral("40 + 2"), QStringLiteral("after.js"));
  QCOMPARE(after.toInt(), 42);
  QVERIFY(!session.timedOut());
}

/**
 * @brief A compiled function whose body never returns is cut off by the same budget, which is the
 *        path the transmit / mqtt / painter previews take when they run the user's function.
 */
void ScriptDryRunTests::jsRunawayCallTimesOut()
{
  DataModel::ScriptDryRun session(
    DataModel::ScriptDryRun::Language::JavaScript, kTripBudgetMs, "test");
  QVERIFY(session.valid());

  auto fn = session.evaluate(kJsRunawayFunction, QStringLiteral("test.js"));
  QVERIFY(fn.isCallable());

  const QByteArray warning =
    QStringLiteral("[JsWatchdog] test timed out after %1 ms -- interrupted")
      .arg(kTripBudgetMs)
      .toUtf8();
  QTest::ignoreMessage(QtWarningMsg, warning.constData());
  const auto result = session.call(fn, QJSValueList());

  QVERIFY(session.timedOut());
  QVERIFY(result.isError());
}

/**
 * @brief A syntax error is an error with a line number, never a timeout.
 */
void ScriptDryRunTests::jsSyntaxErrorIsNotATimeout()
{
  DataModel::ScriptDryRun session(
    DataModel::ScriptDryRun::Language::JavaScript, kTripBudgetMs, "test");
  QVERIFY(session.valid());

  const auto result = session.evaluate(QStringLiteral("function ( {"), QStringLiteral("broken.js"));

  QVERIFY(result.isError());
  QVERIFY(!session.timedOut());
}

//--------------------------------------------------------------------------------------------------
// Lua sessions
//--------------------------------------------------------------------------------------------------

/**
 * @brief The Lua session opens the sandbox, runs a chunk, and leaves the state reachable so the
 *        caller can look its function up and call it.
 */
void ScriptDryRunTests::luaSessionRunsBoundedCode()
{
  DataModel::ScriptDryRun session(
    DataModel::ScriptDryRun::Language::Lua, kGenerousBudgetMs, "test");
  QVERIFY(session.valid());
  QVERIFY(session.luaState() != nullptr);
  QVERIFY(session.jsEngine() == nullptr);

  const int status =
    session.runLuaChunk(QStringLiteral("function transform(v) return v * 2 end"), "transform");
  QCOMPARE(status, int(LUA_OK));
  QVERIFY(!session.timedOut());

  lua_State* L = session.luaState();
  lua_getglobal(L, "transform");
  QVERIFY(lua_isfunction(L, -1));
  lua_pushnumber(L, 21);

  QCOMPARE(session.callLua(1, 1), int(LUA_OK));
  QCOMPARE(int(lua_tonumber(L, -1)), 42);
  QVERIFY(!session.timedOut());
}

/**
 * @brief `while true do end` at chunk level errors inside the budget instead of hanging the GUI
 *        thread that asked for the validation.
 */
void ScriptDryRunTests::luaRunawayChunkTimesOut()
{
  DataModel::ScriptDryRun session(
    DataModel::ScriptDryRun::Language::Lua, kTripBudgetMs, "transform");
  QVERIFY(session.valid());

  QElapsedTimer timer;
  timer.start();
  const int status       = session.runLuaChunk(kLuaInfiniteLoop, "runaway");
  const qint64 elapsedMs = timer.elapsed();

  QCOMPARE(status, int(LUA_ERRRUN));
  QVERIFY(session.timedOut());
  QVERIFY(elapsedMs >= kTripBudgetMs);
  QVERIFY(session.luaError().contains(QStringLiteral("transform timed out after")));
}

/**
 * @brief The same budget bounds a call into a compiled Lua function, the Test-button path.
 */
void ScriptDryRunTests::luaRunawayCallTimesOut()
{
  DataModel::ScriptDryRun session(
    DataModel::ScriptDryRun::Language::Lua, kTripBudgetMs, "transform");
  QVERIFY(session.valid());

  const int status = session.runLuaChunk(
    QStringLiteral("function transform(v) local i = 0 while true do i = i + 1 end end"),
    "transform");
  QCOMPARE(status, int(LUA_OK));

  lua_State* L = session.luaState();
  lua_getglobal(L, "transform");
  QVERIFY(lua_isfunction(L, -1));
  lua_pushnumber(L, 1);

  QCOMPARE(session.callLua(1, 1), int(LUA_ERRRUN));
  QVERIFY(session.timedOut());
}

/**
 * @brief The session's sandbox is the transform sandbox: the base library is there, the host
 *        libraries are not.
 */
void ScriptDryRunTests::luaSandboxHasNoFilesystemGlobals()
{
  DataModel::ScriptDryRun session(
    DataModel::ScriptDryRun::Language::Lua, kGenerousBudgetMs, "test");
  QVERIFY(session.valid());

  const int status = session.runLuaChunk(
    QStringLiteral("return (io == nil) and (os == nil) and (type(math.floor) == 'function')"),
    "probe");

  QCOMPARE(status, int(LUA_OK));
  QVERIFY(lua_toboolean(session.luaState(), -1) != 0);
}

//--------------------------------------------------------------------------------------------------
// One-shot compile check
//--------------------------------------------------------------------------------------------------

/**
 * @brief runJsDryRun reports the three outcomes callers branch on: valid, compile error with a
 *        line, and timeout.
 */
void ScriptDryRunTests::oneShotDryRunReportsOkErrorAndTimeout()
{
  const auto good = DataModel::ScriptDryRun::runJsDryRun(
    QStringLiteral("function mqtt(frame) { return frame; }"), QString(), kGenerousBudgetMs);
  QVERIFY(good.ok);
  QVERIFY(!good.timedOut);

  const auto broken = DataModel::ScriptDryRun::runJsDryRun(
    QStringLiteral("function ( {"), QString(), kGenerousBudgetMs);
  QVERIFY(!broken.ok);
  QVERIFY(!broken.timedOut);
  QVERIFY(!broken.error.isEmpty());

  const auto stuck =
    DataModel::ScriptDryRun::runJsDryRun(kJsInfiniteLoop, QString(), kTripBudgetMs);
  QVERIFY(!stuck.ok);
  QVERIFY(stuck.timedOut);
  QVERIFY(stuck.error.contains(QStringLiteral("did not finish")));
}

QTEST_GUILESS_MAIN(ScriptDryRunTests)

#include "tst_script_dryrun.moc"
