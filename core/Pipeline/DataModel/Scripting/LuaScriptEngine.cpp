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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Scripting/LuaScriptEngine.h"

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
}
// clang-format on

#include <QDebug>
#include <QHash>
#include <QMessageBox>
#include <QSet>
#include <stdexcept>

#include "Core/SSAssert.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/DashboardApi.h"
#include "DataModel/Scripting/DeviceWriteApi.h"
#include "DataModel/Scripting/LuaCompat.h"
#include "DataModel/Scripting/LuaCompatJIT.h"
#include "DataModel/Scripting/LuaMigration.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "DataModel/Scripting/ScriptFrameShaping.h"
#include "IO/PipelineHost.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

static constexpr int kMaxOfferedMigrations = 64;

//--------------------------------------------------------------------------------------------------
// Sandboxed library loader
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sandboxed subset of Lua standard libraries. On LuaJIT the coroutine functions ship
 *        inside the base library and there is no utf8 module (unused by the shipped script
 *        corpus); bit is LuaJIT's native bitwise library, the compat layer's foundation.
 *        ffi and jit are deliberately absent in every mode: sandbox escape.
 */
static const luaL_Reg kLuaSafeLibs[] = {
  {    "_G",   luaopen_base},
  { "table",  luaopen_table},
  {"string", luaopen_string},
  {  "math",   luaopen_math},
  {   "bit",    luaopen_bit},
  { nullptr,        nullptr}
};

/**
 * @brief Calls lua_pcall under a C++ try/catch -- escaped exceptions become LUA_ERRRUN.
 */
[[nodiscard]] static int guardedPcall(lua_State* L, int nargs, int nresults, int msgh) noexcept
{
  try {
    return lua_pcall(L, nargs, nresults, msgh);
  } catch (...) {
    qWarning() << "[LuaScriptEngine] Uncaught C++ exception escaped lua_pcall -- "
                  "treating as LUA_ERRRUN. Check Lua build unwind tables.";
    try {
      lua_settop(L, 0);
      lua_pushstring(L, "uncaught Lua exception (escaped lua_pcall)");
    } catch (...) {
    }
    return LUA_ERRRUN;
  }
}

/**
 * @brief Lua atpanic handler that throws so abort() is never reached.
 */
static int luaPanicHandler(lua_State* L)
{
  const char* msg = lua_tostring(L, -1);
  qWarning() << "[LuaScriptEngine] Lua panic:" << (msg ? msg : "<unknown>");
  throw std::runtime_error(msg ? msg : "lua panic");
}

/**
 * @brief Opens the safe standard libraries and strips dangerous globals.
 */
static void openSafeLibs(lua_State* L)
{
  SS_ASSERT(L != nullptr, return);

  for (const luaL_Reg* lib = kLuaSafeLibs; lib->func; ++lib) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);
  }

  static const char* const kUnsafe[] = {"dofile", "loadfile", "load"};
  for (const char* name : kUnsafe) {
    lua_pushnil(L);
    lua_setglobal(L, name);
  }

  // code-verify off
  // Strip string.dump: bytecode exfiltration vector that pairs with unsafe loaders.
  lua_getglobal(L, "string");
  if (lua_istable(L, -1)) {
    lua_pushnil(L);
    lua_setfield(L, -2, "dump");
  }
  lua_pop(L, 1);
  // code-verify on
}

/**
 * @brief Everything a fresh engine state needs installed, bundled for the protected bootstrap.
 */
struct EngineBootstrapCtx {
  DataModel::LuaScriptEngine* self;
  int sourceId;
};

/**
 * @brief Runs the whole sandbox/library/API installation under lua_pcall protection: on LuaJIT
 *        any raw API call outside a protected frame can reach the panic handler on allocation
 *        failure, and the never-aborts-host contract requires the panic to be unreachable.
 */
static int bootstrapEngineState(lua_State* L)
{
  auto* ctx = static_cast<EngineBootstrapCtx*>(lua_touserdata(L, 1));

  openSafeLibs(L);

  DataModel::installLuaConsole(L);
  DataModel::installLuaCompat(L);
  DataModel::NotificationCenter::installScriptApi(L);

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.injectTableApiLua(L);

  DataModel::DeviceWriteApi::installLua(L, ctx->sourceId);
  DataModel::ActionFireApi::installLua(L);
  DataModel::DashboardApi::installLua(L);
  DataModel::ScriptApiCall::installLua(L, ctx->sourceId);

  lua_pushlightuserdata(L, ctx->self);
  lua_setfield(L, LUA_REGISTRYINDEX, "__ss_engine__");
  return 0;
}

/**
 * @brief Appends an actionable migration hint when a compile error looks like Lua 5.3-only
 *        bitwise/floor-division syntax, which LuaJIT's 5.1 grammar rejects (spec 0051 R22):
 *        the raw "unexpected symbol" is useless to a user whose script worked on the old
 *        runtime, so the construct and its bit.* replacement are named explicitly.
 */
[[nodiscard]] static QString enrichSyntaxError(const QString& error, const QString& script)
{
  static const struct {
    const char* needle;
    const char* hint;
  } kConstructs[] = {
    {"<<",             "'<<' -> bit.lshift(a, b)"},
    {">>",             "'>>' -> bit.rshift(a, b)"},
    { "&",                "'&' -> bit.band(a, b)"},
    { "|",                 "'|' -> bit.bor(a, b)"},
    { "~", "'~' -> bit.bxor(a, b) or bit.bnot(a)"},
    {"//",            "'//' -> math.floor(a / b)"},
  };

  QStringList hints;
  for (const auto& construct : kConstructs)
    if (script.contains(QLatin1String(construct.needle)))
      hints.append(QLatin1String(construct.hint));

  if (hints.isEmpty())
    return error;

  return error
       + QObject::tr("\n\nThis script may use Lua 5.3 bitwise syntax, which this runtime "
                     "does not support. Replace: %1. The bit and bit32 libraries are "
                     "available in every script.")
           .arg(hints.join(QLatin1String("; ")));
}

/**
 * @brief Returns the LuaJIT rewrite of the script, but only when the rewrite itself compiles:
 *        trading one syntax error for another is worse than offering no fix at all.
 */
[[nodiscard]] static QString compilableMigration(lua_State* L, const QString& script)
{
  const QString migrated = DataModel::LuaMigration::migrateToLuaJit(script);
  if (migrated.isEmpty())
    return {};

  const QByteArray utf8 = migrated.toUtf8();
  const int status      = luaL_loadbuffer(L, utf8.constData(), utf8.size(), "migration_probe");
  lua_pop(L, 1);
  return status == LUA_OK ? migrated : QString();
}

/**
 * @brief Returns the project model, resolved once for every consumer in this translation unit.
 */
[[nodiscard]] static DataModel::ProjectModel& luaProjectModel()
{
  static auto& model = DataModel::ProjectModel::instance();
  return model;
}

/**
 * @brief Writes an accepted rewrite back into the project, whose own change signal reloads the
 *        parser and repopulates the code editor.
 */
static void applyLuaMigration(int sourceId, const QString& fixed)
{
  if (sourceId == 0)
    luaProjectModel().setFrameParserCode(fixed);
  else
    luaProjectModel().updateSourceFrameParser(sourceId, fixed);
}

/**
 * @brief Offers the rewrite from the GUI thread, once per broken script: the engine runs on the
 *        pipeline thread, where a modal dialog cannot be raised and its answer cannot be waited
 *        on, and one project load re-enters the parser several times.
 */
static void offerLuaMigration(int sourceId,
                              const QString& error,
                              const QString& script,
                              const QString& fixed)
{
  const QString key =
    QStringLiteral("%1:%2").arg(QString::number(sourceId), QString::number(qHash(script)));
  QMetaObject::invokeMethod(
    qApp,
    [sourceId, error, fixed, key] {
      static QSet<QString> offered;
      if (offered.contains(key))
        return;

      if (offered.size() >= kMaxOfferedMigrations)
        offered.clear();

      offered.insert(key);
      const auto answer = Misc::Utilities::showMessageBox(
        QObject::tr("Lua Syntax Error"),
        QObject::tr("The parser code contains an error:\n\n%1\n\n"
                    "Serial Studio can rewrite the unsupported operators as bit.* calls and "
                    "reload the parser. The bit library works on 32-bit integers, so a value "
                    "wider than 32 bits will wrap once rewritten.")
          .arg(error),
        QMessageBox::Critical,
        QString(),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes,
        {
          {QMessageBox::Yes, QObject::tr("Fix Automatically")},
          { QMessageBox::No,   QObject::tr("Leave Unchanged")}
      });

      if (answer == QMessageBox::Yes)
        applyLuaMigration(sourceId, fixed);
    },
    Qt::QueuedConnection);
}

/**
 * @brief Reports a compile failure: an offer to rewrite it when the script is Lua 5.3 syntax that
 *        migrates cleanly, a plain error dialog otherwise, and a log line when boxes are muted.
 */
static void reportSyntaxError(
  lua_State* L, int sourceId, const QString& script, const QString& errorMsg, bool showMessageBoxes)
{
  if (!showMessageBoxes) {
    qWarning() << "[LuaScriptEngine] Source" << sourceId << "syntax error:" << errorMsg;
    return;
  }

  const QString fixed = compilableMigration(L, script);
  if (!fixed.isEmpty()) {
    offerLuaMigration(sourceId, errorMsg, script, fixed);
    return;
  }

  Misc::Utilities::showMessageBox(
    QObject::tr("Lua Syntax Error"),
    QObject::tr("The parser code contains an error:\n\n%1").arg(errorMsg),
    QMessageBox::Critical);
}

/**
 * @brief Closes a Lua state after invalidating the interned-key cache it may have populated.
 *        The cache clear is skipped during teardown: the store dies moments later, and reaching
 *        it from a destructor would marshal across threads while no event loop is pumping.
 */
static void closeLuaState(lua_State* state)
{
  if (!state)
    return;

  if (!IO::PipelineHost::tearingDown()) {
    static auto& frameBuilder = DataModel::FrameBuilder::instance();
    frameBuilder.invokeOnBuilderThreadBlocking(
      [] { frameBuilder.tableStore().clearLookupCache(); });
  }

  lua_close(state);
}

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Lua script engine and creates its sandboxed state.
 */
DataModel::LuaScriptEngine::LuaScriptEngine()
  : m_state(nullptr)
  , m_loaded(false)
  , m_disabled(false)
  , m_sourceId(0)
  , m_parseRef(LUA_NOREF)
  , m_consecutiveTimeouts(0)
  , m_errorCount(0)
  , m_lastError()
  , m_deadline(QDeadlineTimer::Forever)
{
  createState();
}

/**
 * @brief Closes the Lua state on destruction.
 */
DataModel::LuaScriptEngine::~LuaScriptEngine()
{
  destroyState();
}

//--------------------------------------------------------------------------------------------------
// State management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a fresh Lua state with safe libraries and watchdog hook.
 */
void DataModel::LuaScriptEngine::createState()
{
  SS_ASSERT(m_state == nullptr, return);

  m_state = luaL_newstate();
  SS_ASSERT(m_state != nullptr, {
    m_loaded = false;
    return;
  });

  lua_atpanic(m_state, luaPanicHandler);

  EngineBootstrapCtx ctx{this, m_sourceId};
  lua_pushcfunction(m_state, bootstrapEngineState);
  lua_pushlightuserdata(m_state, &ctx);
  const int status = guardedPcall(m_state, 1, 0, 0);
  if (status != LUA_OK) [[unlikely]] {
    qWarning() << "[LuaScriptEngine] Engine bootstrap failed:"
               << QString::fromUtf8(lua_tostring(m_state, -1));
    closeLuaState(m_state);
    m_state    = nullptr;
    m_loaded   = false;
    m_disabled = true;
    return;
  }

  if (luaProjectModel().luaFastMode()) {
    luaJIT_setmode(m_state, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);
  } else {
    luaJIT_setmode(m_state, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);
    lua_sethook(m_state, watchdogHook, LUA_MASKCOUNT, kHookInstructionCount);
  }

  m_deadline            = QDeadlineTimer(QDeadlineTimer::Forever);
  m_loaded              = false;
  m_disabled            = false;
  m_parseRef            = LUA_NOREF;
  m_consecutiveTimeouts = 0;
}

/**
 * @brief Closes the Lua state and resets internal flags.
 */
void DataModel::LuaScriptEngine::destroyState()
{
  closeLuaState(m_state);
  m_state    = nullptr;
  m_loaded   = false;
  m_deadline = QDeadlineTimer(QDeadlineTimer::Forever);
}

/**
 * @brief Returns true once a parse() function has been loaded into the Lua state.
 */
bool DataModel::LuaScriptEngine::isLoaded() const noexcept
{
  return m_loaded;
}

/**
 * @brief Returns the ScriptLanguage implemented by this engine.
 */
int DataModel::LuaScriptEngine::language() const noexcept
{
  return SerialStudio::Lua;
}

/**
 * @brief Runs one full Lua garbage-collection cycle.
 */
void DataModel::LuaScriptEngine::collectGarbage()
{
  if (m_state)
    lua_gc(m_state, LUA_GCCOLLECT, 0);
}

/**
 * @brief Resets the engine by recreating the Lua state.
 */
void DataModel::LuaScriptEngine::reset()
{
  destroyState();
  createState();
  resetErrorStats();
}

//--------------------------------------------------------------------------------------------------
// Watchdog hook
//--------------------------------------------------------------------------------------------------

/**
 * @brief Lua debug hook that aborts the call when the deadline expires.
 */
void DataModel::LuaScriptEngine::watchdogHook(lua_State* L, lua_Debug* ar)
{
  Q_UNUSED(ar)

  lua_getfield(L, LUA_REGISTRYINDEX, "__ss_engine__");
  auto* self = static_cast<LuaScriptEngine*>(lua_touserdata(L, -1));
  lua_pop(L, 1);

  if (!self) [[unlikely]]
    return;

  if (self->m_deadline.hasExpired()) [[unlikely]]
    luaL_error(L, "execution timed out after %d ms", kRuntimeWatchdogMs);
}

/**
 * @brief Records a watchdog timeout and disables the parser after too many in a row.
 */
bool DataModel::LuaScriptEngine::noteTimeoutAndCheckDisabled(int sourceId)
{
  ++m_consecutiveTimeouts;
  if (m_consecutiveTimeouts < kMaxConsecutiveTimeouts)
    return false;

  m_disabled = true;
  qWarning() << "[LuaScriptEngine] Source" << sourceId << "disabled after"
             << kMaxConsecutiveTimeouts << "consecutive watchdog timeouts.";
  Misc::Utilities::showMessageBox(
    QObject::tr("Frame Parser Disabled"),
    QObject::tr("The Lua frame parser for source %1 timed out %2 frames in a row "
                "and has been disabled to keep Serial Studio responsive.\n\n"
                "Most likely cause: an infinite loop or extremely slow operation "
                "in the script body. Fix the script and reload the project to "
                "re-enable parsing.")
      .arg(sourceId)
      .arg(kMaxConsecutiveTimeouts),
    QMessageBox::Critical);
  return true;
}

/**
 * @brief Clears the consecutive-timeout counter after a successful parse.
 */
void DataModel::LuaScriptEngine::resetTimeoutCounter() noexcept
{
  m_consecutiveTimeouts = 0;
}

/**
 * @brief Records a runtime parse failure for the 1 Hz diagnostics sample.
 */
void DataModel::LuaScriptEngine::noteError(const QString& message)
{
  ++m_errorCount;
  m_lastError = message;
}

/**
 * @brief Returns whether the watchdog cut this parser off after repeated timeouts.
 */
bool DataModel::LuaScriptEngine::disabled() const noexcept
{
  return m_disabled;
}

/**
 * @brief Returns the most recent runtime parse error message.
 */
QString DataModel::LuaScriptEngine::lastError() const
{
  return m_lastError;
}

/**
 * @brief Returns how many runtime parse failures this engine has seen.
 */
quint64 DataModel::LuaScriptEngine::errorCount() const noexcept
{
  return m_errorCount;
}

/**
 * @brief Returns the current run of back-to-back watchdog timeouts.
 */
int DataModel::LuaScriptEngine::consecutiveTimeouts() const noexcept
{
  return m_consecutiveTimeouts;
}

/**
 * @brief Clears the runtime error statistics so a repaired script stops being reported.
 */
void DataModel::LuaScriptEngine::resetErrorStats()
{
  m_errorCount = 0;
  m_lastError.clear();
  m_consecutiveTimeouts = 0;
}

//--------------------------------------------------------------------------------------------------
// Script loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates and loads a Lua frame parser script, caching parse() in the
 *        registry so the hotpath skips a global lookup on every frame.
 */
bool DataModel::LuaScriptEngine::loadScript(const QString& script,
                                            int sourceId,
                                            bool showMessageBoxes)
{
  SS_ASSERT(sourceId >= 0, return false);
  SS_ASSERT(!script.isEmpty(), return false);

  lua_State* const prevState        = m_state;
  const bool prevLoaded             = m_loaded;
  const bool prevDisabled           = m_disabled;
  const int prevSourceId            = m_sourceId;
  const int prevParseRef            = m_parseRef;
  const int prevConsecutiveTimeouts = m_consecutiveTimeouts;

  auto restorePrevious = [&]() {
    closeLuaState(m_state);
    m_state               = prevState;
    m_sourceId            = prevSourceId;
    m_parseRef            = prevParseRef;
    m_loaded              = prevLoaded;
    m_disabled            = prevDisabled;
    m_consecutiveTimeouts = prevConsecutiveTimeouts;
    m_deadline            = QDeadlineTimer(QDeadlineTimer::Forever);
  };

  m_state    = nullptr;
  m_sourceId = sourceId;
  createState();

  try {
    const QByteArray utf8 = script.toUtf8();
    const auto fileName   = QStringLiteral("parser_%1.lua").arg(sourceId).toUtf8();
    const int status =
      luaL_loadbuffer(m_state, utf8.constData(), utf8.size(), fileName.constData());
    if (status != LUA_OK) {
      const QString errorMsg =
        enrichSyntaxError(QString::fromUtf8(lua_tostring(m_state, -1)), script);
      lua_pop(m_state, 1);
      reportSyntaxError(m_state, sourceId, script, errorMsg, showMessageBoxes);
      restorePrevious();
      return false;
    }

    if (!runLoadedChunk(sourceId, showMessageBoxes)) {
      restorePrevious();
      return false;
    }

    if (!ensureParseFunction(sourceId, showMessageBoxes)) {
      restorePrevious();
      return false;
    }

    if (sourceId == 0 && !probeParseFunction(sourceId, showMessageBoxes)) {
      restorePrevious();
      return false;
    }

    lua_getglobal(m_state, "parse");
    m_parseRef = luaL_ref(m_state, LUA_REGISTRYINDEX);
    m_loaded   = true;

    closeLuaState(prevState);
    return true;
  } catch (const std::exception& e) {
    qWarning() << "[LuaScriptEngine] Source" << sourceId << "load uncaught exception:" << e.what();
  } catch (...) {
    qWarning() << "[LuaScriptEngine] Source" << sourceId << "load uncaught non-std exception";
  }

  restorePrevious();
  return false;
}

/**
 * @brief Runs the just-loaded chunk under the watchdog and reports any runtime error.
 */
bool DataModel::LuaScriptEngine::runLoadedChunk(int sourceId, bool showMessageBoxes)
{
  m_deadline.setRemainingTime(kRuntimeWatchdogMs);
  const int status = guardedPcall(m_state, 0, 0, 0);
  m_deadline       = QDeadlineTimer(QDeadlineTimer::Forever);
  if (status == LUA_OK)
    return true;

  const QString errorMsg = QString::fromUtf8(lua_tostring(m_state, -1));
  lua_pop(m_state, 1);
  if (showMessageBoxes) {
    Misc::Utilities::showMessageBox(
      QObject::tr("Lua Runtime Error"),
      QObject::tr("The parser code triggered an error:\n\n%1").arg(errorMsg),
      QMessageBox::Critical);
  } else {
    qWarning() << "[LuaScriptEngine] Source" << sourceId << "runtime error:" << errorMsg;
  }
  return false;
}

/**
 * @brief Confirms that a global 'parse' function was declared by the chunk.
 */
bool DataModel::LuaScriptEngine::ensureParseFunction(int sourceId, bool showMessageBoxes)
{
  lua_getglobal(m_state, "parse");
  const bool isFn = lua_isfunction(m_state, -1);
  lua_pop(m_state, 1);
  if (isFn)
    return true;

  if (showMessageBoxes) {
    Misc::Utilities::showMessageBox(
      QObject::tr("Missing Parse Function"),
      QObject::tr("The 'parse' function is not defined in the script.\n\n"
                  "Please ensure your code includes:\n"
                  "function parse(frame) ... end"),
      QMessageBox::Critical);
  } else {
    qWarning() << "[LuaScriptEngine] Source" << sourceId << "missing parse() function";
  }
  return false;
}

/**
 * @brief Probes parse() with empty/zero/byte-table inputs to surface dead-on-arrival errors.
 */
bool DataModel::LuaScriptEngine::probeParseFunction(int sourceId, bool showMessageBoxes)
{
  Q_UNUSED(sourceId);

  bool probeOk         = false;
  const char* probes[] = {"0", ""};
  QString lastError;

  for (const char* probe : probes) {
    lua_getglobal(m_state, "parse");
    lua_pushstring(m_state, probe);

    m_deadline.setRemainingTime(kRuntimeWatchdogMs);
    const int probeStatus = guardedPcall(m_state, 1, 1, 0);
    m_deadline            = QDeadlineTimer(QDeadlineTimer::Forever);

    if (probeStatus == LUA_OK) {
      lua_pop(m_state, 1);
      probeOk = true;
      break;
    }

    lastError = QString::fromUtf8(lua_tostring(m_state, -1));
    lua_pop(m_state, 1);
  }

  if (!probeOk) {
    lua_getglobal(m_state, "parse");
    lua_newtable(m_state);
    lua_pushinteger(m_state, 0);
    lua_rawseti(m_state, -2, 1);

    m_deadline.setRemainingTime(kRuntimeWatchdogMs);
    const int probeStatus = guardedPcall(m_state, 1, 1, 0);
    m_deadline            = QDeadlineTimer(QDeadlineTimer::Forever);

    if (probeStatus == LUA_OK) {
      lua_pop(m_state, 1);
      probeOk = true;
    } else {
      lastError = QString::fromUtf8(lua_tostring(m_state, -1));
      lua_pop(m_state, 1);
    }
  }

  if (probeOk)
    return true;

  if (showMessageBoxes) {
    Misc::Utilities::showMessageBox(QObject::tr("Parse Function Runtime Error"),
                                    QObject::tr("The parse function contains an error:\n\n%1\n\n"
                                                "Please fix the error in the function body.")
                                      .arg(lastError),
                                    QMessageBox::Critical);
  } else {
    qWarning() << "[LuaScriptEngine] Probe failed:" << lastError;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Shared driver: invokes the cached parse() over a raw byte buffer and converts the result.
 */
QList<QStringList> DataModel::LuaScriptEngine::parseLuaText(const char* data, qsizetype len)
{
  SS_ASSERT(m_state != nullptr, return {});
  SS_ASSERT(data != nullptr, return {});

  if (!m_loaded || m_disabled)
    return {};

  try {
    lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_parseRef);
    lua_pushlstring(m_state, data, static_cast<size_t>(len));

    m_deadline.setRemainingTime(kRuntimeWatchdogMs);
    const int status = guardedPcall(m_state, 1, 1, 0);
    m_deadline       = QDeadlineTimer(QDeadlineTimer::Forever);

    if (status != LUA_OK) [[unlikely]] {
      const QString err = QString::fromUtf8(lua_tostring(m_state, -1));
      lua_pop(m_state, 1);
      qWarning() << "[LuaScriptEngine] Parse error:" << err;
      noteError(err);
      if (err.contains(QLatin1String("timed out")))
        (void)noteTimeoutAndCheckDisabled(m_sourceId);

      return {};
    }

    resetTimeoutCounter();
    return convertResult();
  } catch (const std::exception& e) {
    qWarning() << "[LuaScriptEngine] parse uncaught exception:" << e.what();
  } catch (...) {
    qWarning() << "[LuaScriptEngine] parse uncaught non-std exception";
  }

  m_deadline = QDeadlineTimer(QDeadlineTimer::Forever);
  lua_settop(m_state, 0);
  return {};
}

/**
 * @brief Runs the Lua parse function over a text frame (transcodes UTF-16 to UTF-8 bytes).
 */
QList<QStringList> DataModel::LuaScriptEngine::parseString(const QString& frame)
{
  SS_ASSERT(!frame.isEmpty(), return {});
  SS_ASSERT(m_state != nullptr, return {});

  const QByteArray utf8 = frame.toUtf8();
  return parseLuaText(utf8.constData(), utf8.size());
}

/**
 * @brief Runs the Lua parse function over a raw UTF-8 byte frame (no QString round-trip).
 */
QList<QStringList> DataModel::LuaScriptEngine::parseUtf8(const QByteArray& frame)
{
  SS_ASSERT(!frame.isEmpty(), return {});
  SS_ASSERT(m_state != nullptr, return {});

  return parseLuaText(frame.constData(), frame.size());
}

/**
 * @brief Runs the Lua parse function over a binary frame (1-indexed byte table).
 */
QList<QStringList> DataModel::LuaScriptEngine::parseBinary(const QByteArray& frame)
{
  SS_ASSERT(!frame.isEmpty(), return {});
  SS_ASSERT(m_state != nullptr, return {});

  if (!m_loaded || m_disabled)
    return {};

  try {
    lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_parseRef);

    lua_createtable(m_state, frame.size(), 0);
    const auto* data = reinterpret_cast<const quint8*>(frame.constData());
    for (int i = 0; i < frame.size(); ++i) {
      lua_pushinteger(m_state, data[i]);
      lua_rawseti(m_state, -2, i + 1);
    }

    m_deadline.setRemainingTime(kRuntimeWatchdogMs);
    const int status = guardedPcall(m_state, 1, 1, 0);
    m_deadline       = QDeadlineTimer(QDeadlineTimer::Forever);

    if (status != LUA_OK) [[unlikely]] {
      const QString err = QString::fromUtf8(lua_tostring(m_state, -1));
      lua_pop(m_state, 1);
      qWarning() << "[LuaScriptEngine] Parse error:" << err;
      noteError(err);
      if (err.contains(QLatin1String("timed out")))
        (void)noteTimeoutAndCheckDisabled(m_sourceId);

      return {};
    }

    resetTimeoutCounter();
    return convertResult();
  } catch (const std::exception& e) {
    qWarning() << "[LuaScriptEngine] parseBinary uncaught exception:" << e.what();
  } catch (...) {
    qWarning() << "[LuaScriptEngine] parseBinary uncaught non-std exception";
  }

  m_deadline = QDeadlineTimer(QDeadlineTimer::Forever);
  lua_settop(m_state, 0);
  return {};
}

//--------------------------------------------------------------------------------------------------
// Result conversion
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts the Lua value at stack top to a QString without coercing numeric
 *        strings: strings pass through verbatim (no strtod/reformat) to match the JS
 *        engine; only real numbers are formatted.
 */
QString DataModel::LuaScriptEngine::luaValueToString()
{
  SS_ASSERT(m_state != nullptr, return {});

  switch (lua_type(m_state, -1)) {
    case LUA_TSTRING:
      return QString::fromUtf8(lua_tostring(m_state, -1));
    case LUA_TNUMBER:
      if (lua_isinteger(m_state, -1))
        return QString::number(lua_tointeger(m_state, -1));

      return QString::number(lua_tonumber(m_state, -1), 'g', 15);
    default:
      break;
  }

  const char* coerced = lua_tostring(m_state, -1);
  return coerced ? QString::fromUtf8(coerced) : QString();
}

/**
 * @brief Converts the Lua table at tableIndex to a QStringList.
 */
QStringList DataModel::LuaScriptEngine::tableToStringList(int tableIndex)
{
  SS_ASSERT(m_state != nullptr && lua_istable(m_state, tableIndex), return {});

  QStringList result;
  const int len = static_cast<int>(lua_rawlen(m_state, tableIndex));
  result.reserve(qMin(len, kMaxElements));

  for (int i = 1; i <= qMin(len, kMaxElements); ++i) {
    lua_rawgeti(m_state, tableIndex, i);
    result.append(luaValueToString());
    lua_pop(m_state, 1);
  }

  return result;
}

/**
 * @brief Converts the scalar Lua value at stack top into a single-element list.
 */
QStringList DataModel::LuaScriptEngine::scalarToStringList()
{
  SS_ASSERT(m_state != nullptr, return {});

  QStringList frame;
  const int type = lua_type(m_state, -1);
  if (type == LUA_TSTRING || type == LUA_TNUMBER)
    frame.append(luaValueToString());

  return frame;
}

/**
 * @brief Routes the Lua value at stack top into the scalars or vectors bucket.
 */
void DataModel::LuaScriptEngine::appendMixedElement(QStringList& scalars,
                                                    QList<QStringList>& vectors,
                                                    qsizetype& maxVectorLength)
{
  if (lua_istable(m_state, -1)) {
    const auto vec = tableToStringList(-1);
    if (!vec.isEmpty()) {
      vectors.append(vec);
      maxVectorLength = std::max(maxVectorLength, vec.size());
    }
    return;
  }

  scalars.append(luaValueToString());
}

/**
 * @brief Converts the Lua return value at stack top to a frame list.
 */
QList<QStringList> DataModel::LuaScriptEngine::convertResult()
{
  QList<QStringList> results;

  if (!lua_istable(m_state, -1)) {
    QStringList frame = scalarToStringList();
    lua_pop(m_state, 1);
    if (!frame.isEmpty())
      results.append(frame);

    return results;
  }

  const int len = static_cast<int>(lua_rawlen(m_state, -1));
  if (len == 0) {
    lua_pop(m_state, 1);
    return results;
  }

  const int scanLen = qMin(len, kMaxElements);
  QStringList scalars;
  scalars.reserve(scanLen);
  for (int i = 1; i <= scanLen; ++i) {
    lua_rawgeti(m_state, -1, i);
    if (lua_istable(m_state, -1)) [[unlikely]] {
      lua_pop(m_state, 1);
      return classifyTable(len);
    }

    scalars.append(luaValueToString());
    lua_pop(m_state, 1);
  }

  lua_pop(m_state, 1);
  results.append(scalars);
  return results;
}

/**
 * @brief Dispatches the table at stack top to scalar/2D/mixed conversion paths.
 */
QList<QStringList> DataModel::LuaScriptEngine::classifyTable(int len)
{
  QList<QStringList> results;

  bool hasTable  = false;
  bool hasScalar = false;
  for (int i = 1; i <= qMin(len, kMaxElements); ++i) {
    lua_rawgeti(m_state, -1, i);
    if (lua_istable(m_state, -1))
      hasTable = true;
    else
      hasScalar = true;

    lua_pop(m_state, 1);

    if (hasTable && hasScalar)
      break;
  }

  if (!hasTable) {
    results.append(tableToStringList(-1));
    lua_pop(m_state, 1);
    return results;
  }

  if (hasTable && !hasScalar) {
    results.reserve(len);
    for (int i = 1; i <= qMin(len, kMaxElements); ++i) {
      lua_rawgeti(m_state, -1, i);
      if (lua_istable(m_state, -1))
        results.append(tableToStringList(-1));
      else [[unlikely]]
        qWarning() << "[LuaScriptEngine] Row" << i << "is not a table, skipping";

      lua_pop(m_state, 1);
    }

    lua_pop(m_state, 1);
    return results;
  }

  return unzipMixedTable(len);
}

/**
 * @brief Unzips a mixed scalar/vector table into a list of per-step frames.
 */
QList<QStringList> DataModel::LuaScriptEngine::unzipMixedTable(int len)
{
  QStringList scalars;
  QList<QStringList> vectors;
  qsizetype maxVectorLength = 0;

  for (int i = 1; i <= qMin(len, kMaxElements); ++i) {
    lua_rawgeti(m_state, -1, i);
    appendMixedElement(scalars, vectors, maxVectorLength);
    lua_pop(m_state, 1);
  }

  lua_pop(m_state, 1);

  return DataModel::ScriptFrames::unzipMixedFrames(scalars, vectors, maxVectorLength);
}
