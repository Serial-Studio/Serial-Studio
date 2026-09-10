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

#include "DataModel/Scripting/ScriptDryRun.h"

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
}
// clang-format on

#include <QDebug>
#include <stdexcept>

#include "Core/SSAssert.h"
#include "DataModel/Scripting/LuaCompatJIT.h"

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sandboxed library subset every throwaway Lua session opens: the same set the transform and
 *        parser engines run with, minus everything that reaches the filesystem or the host.
 */
static const luaL_Reg kDryRunSafeLibs[] = {
  {    "_G",   luaopen_base},
  { "table",  luaopen_table},
  {"string", luaopen_string},
  {  "math",   luaopen_math},
  {   "bit",    luaopen_bit},
  { nullptr,        nullptr}
};

/**
 * @brief Falls back to the shared budget when the caller passes a non-positive one.
 */
[[nodiscard]] static int sanitizedBudget(int budgetMs)
{
  return budgetMs > 0 ? budgetMs : DataModel::kScriptDryRunBudgetMs;
}

/**
 * @brief Falls back to a generic label when the caller passes none.
 */
[[nodiscard]] static const char* sanitizedLabel(const char* label)
{
  return label ? label : "script";
}

/**
 * @brief Lua atpanic handler that throws so a dry run can never abort the host.
 */
static int dryRunLuaPanic(lua_State* L)
{
  const char* message = lua_tostring(L, -1);
  qWarning() << "[ScriptDryRun] Lua panic:" << (message ? message : "<unknown>");
  throw std::runtime_error(message ? message : "lua panic");
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the requested throwaway engine already guarded by its deadline; the caller installs
 *        its own API surface on top before running anything.
 */
DataModel::ScriptDryRun::ScriptDryRun(Language language, int budgetMs, const char* label)
  : m_language(language)
  , m_budgetMs(sanitizedBudget(budgetMs))
  , m_label(sanitizedLabel(label))
  , m_timedOut(false)
  , m_luaState(nullptr)
  , m_luaHook(sanitizedBudget(budgetMs), sanitizedLabel(label))
  , m_jsEngine(nullptr)
  , m_jsWatchdog(nullptr)
{
  SS_ASSERT_LOG(budgetMs > 0);

  if (m_language == Language::Lua) {
    createLuaState();
    return;
  }

  m_jsEngine = std::make_unique<QJSEngine>();
  m_jsWatchdog =
    std::make_unique<JsWatchdog>(m_jsEngine.get(), m_budgetMs, QString::fromUtf8(m_label));
}

/**
 * @brief Closes the Lua state; the JavaScript engine and its watchdog go with the members.
 */
DataModel::ScriptDryRun::~ScriptDryRun()
{
  if (m_luaState)
    lua_close(m_luaState);

  m_luaState = nullptr;
}

/**
 * @brief Creates the sandboxed interpreter state. The JIT is switched off because a count hook
 *        never fires inside a compiled trace, and an uninterruptible validation is what this whole
 *        helper exists to prevent.
 */
void DataModel::ScriptDryRun::createLuaState()
{
  m_luaState = luaL_newstate();
  SS_ASSERT(m_luaState != nullptr, return);

  lua_atpanic(m_luaState, dryRunLuaPanic);

  for (const luaL_Reg* lib = kDryRunSafeLibs; lib->func; ++lib) {
    luaL_requiref(m_luaState, lib->name, lib->func, 1);
    lua_pop(m_luaState, 1);
  }

  luaJIT_setmode(m_luaState, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);
  m_luaHook.install(m_luaState);
}

//--------------------------------------------------------------------------------------------------
// Accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the underlying engine was created.
 */
bool DataModel::ScriptDryRun::valid() const noexcept
{
  return (m_language == Language::Lua) ? m_luaState != nullptr : m_jsEngine != nullptr;
}

/**
 * @brief Returns the JavaScript engine, or nullptr for a Lua session.
 */
QJSEngine* DataModel::ScriptDryRun::jsEngine() const noexcept
{
  return m_jsEngine.get();
}

/**
 * @brief Returns the Lua state, or nullptr for a JavaScript session.
 */
lua_State* DataModel::ScriptDryRun::luaState() const noexcept
{
  return m_luaState;
}

/**
 * @brief Returns whether the most recent guarded step hit the deadline.
 */
bool DataModel::ScriptDryRun::timedOut() const noexcept
{
  return m_timedOut;
}

/**
 * @brief Returns the per-step budget in milliseconds.
 */
int DataModel::ScriptDryRun::budgetMs() const noexcept
{
  return m_budgetMs;
}

//--------------------------------------------------------------------------------------------------
// JavaScript
//--------------------------------------------------------------------------------------------------

/**
 * @brief Evaluates top-level code under the watchdog. A runaway script is interrupted off-thread
 *        by JsWatchdogThread; the interrupt flag is cleared here so the engine stays usable and
 *        timedOut() carries the verdict.
 */
QJSValue DataModel::ScriptDryRun::evaluate(const QString& code, const QString& fileName)
{
  SS_ASSERT(m_jsEngine != nullptr, return QJSValue());
  SS_ASSERT(m_jsWatchdog != nullptr, return QJSValue());

  m_timedOut = false;
  m_jsWatchdog->arm();
  auto result = m_jsEngine->evaluate(code, fileName);
  m_jsWatchdog->disarm();

  if (m_jsEngine->isInterrupted()) [[unlikely]] {
    m_jsEngine->setInterrupted(false);
    m_timedOut = true;
  }

  return result;
}

/**
 * @brief Calls a compiled function under the watchdog, so a runaway body is an error and not a
 *        frozen dialog.
 */
QJSValue DataModel::ScriptDryRun::call(QJSValue& fn, const QJSValueList& args)
{
  SS_ASSERT(m_jsWatchdog != nullptr, return QJSValue());
  SS_ASSERT(fn.isCallable(), return QJSValue());

  const auto result = m_jsWatchdog->call(fn, args);
  m_timedOut        = m_jsWatchdog->lastCallTimedOut();
  return result;
}

//--------------------------------------------------------------------------------------------------
// Lua
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads and runs one chunk under the deadline, returning the Lua status; on failure the
 *        error object is left on the stack for luaError().
 */
int DataModel::ScriptDryRun::runLuaChunk(const QString& code, const char* chunkName)
{
  SS_ASSERT(m_luaState != nullptr, return LUA_ERRRUN);
  SS_ASSERT_LOG(chunkName != nullptr);

  m_timedOut            = false;
  const QByteArray utf8 = code.toUtf8();
  const int loadStatus  = luaL_loadbuffer(m_luaState,
                                         utf8.constData(),
                                         static_cast<size_t>(utf8.size()),
                                         chunkName ? chunkName : "chunk");
  if (loadStatus != LUA_OK)
    return loadStatus;

  return callLua(0, LUA_MULTRET);
}

/**
 * @brief Runs lua_pcall under the deadline and under a C++ catch, so neither a runaway loop nor an
 *        exception escaping the VM can take the host down. A call that finished is never reported
 *        as a timeout, however close to the budget it landed.
 */
int DataModel::ScriptDryRun::callLua(int nargs, int nresults)
{
  SS_ASSERT(m_luaState != nullptr, return LUA_ERRRUN);
  SS_ASSERT_LOG(nargs >= 0);

  m_timedOut = false;
  m_luaHook.arm();

  int status = LUA_ERRRUN;
  try {
    status = lua_pcall(m_luaState, nargs, nresults, 0);
  } catch (const std::exception& e) {
    qWarning() << "[ScriptDryRun] Uncaught exception escaped lua_pcall:" << e.what();
    lua_settop(m_luaState, 0);
    lua_pushstring(m_luaState, e.what());
  } catch (...) {
    qWarning() << "[ScriptDryRun] Uncaught non-std exception escaped lua_pcall";
    lua_settop(m_luaState, 0);
    lua_pushstring(m_luaState, "uncaught Lua exception (escaped lua_pcall)");
  }

  m_luaHook.disarm();
  m_timedOut = (status != LUA_OK) && m_luaHook.timedOut();
  return status;
}

/**
 * @brief Returns the Lua value at the top of the stack as text, the error message after a failed
 *        chunk or call. The value is left in place, matching the raw API the callers replaced.
 */
QString DataModel::ScriptDryRun::luaError()
{
  SS_ASSERT(m_luaState != nullptr, return QString());
  SS_ASSERT_LOG(lua_gettop(m_luaState) > 0);

  const char* message = lua_tostring(m_luaState, -1);
  return QString::fromUtf8(message ? message : "");
}

//--------------------------------------------------------------------------------------------------
// One-shot compile check
//--------------------------------------------------------------------------------------------------

/**
 * @brief Compiles @p code in a throwaway engine after an optional prelude and reports the outcome:
 *        the shape every "is this script valid" caller needs, with the deadline already applied.
 */
DataModel::ScriptDryRunResult DataModel::ScriptDryRun::runJsDryRun(const QString& code,
                                                                   const QString& prelude,
                                                                   int budgetMs)
{
  ScriptDryRunResult result;
  ScriptDryRun session(Language::JavaScript, budgetMs, "dryRun");
  SS_ASSERT(session.valid(), return result);

  session.jsEngine()->installExtensions(QJSEngine::ConsoleExtension);

  if (!prelude.isEmpty()) {
    const auto preludeResult = session.evaluate(prelude, QStringLiteral("prelude.js"));
    if (preludeResult.isError()) {
      result.error = preludeResult.toString();
      return result;
    }
  }

  const auto evaluated = session.evaluate(code, QStringLiteral("dryrun.js"));
  if (session.timedOut()) {
    result.timedOut = true;
    result.error =
      QStringLiteral("Script did not finish evaluating within %1 ms").arg(session.budgetMs());
    return result;
  }

  if (evaluated.isError()) {
    result.line  = evaluated.property(QStringLiteral("lineNumber")).toInt();
    result.error = evaluated.toString();
    return result;
  }

  result.ok = true;
  return result;
}
