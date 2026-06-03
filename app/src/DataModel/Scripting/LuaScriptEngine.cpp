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

#include "DataModel/Scripting/LuaScriptEngine.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <QDebug>
#include <QMessageBox>
#include <stdexcept>

#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/Scripting/DashboardApi.h"
#include "DataModel/Scripting/DeviceWriteApi.h"
#include "DataModel/Scripting/LuaCompat.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Sandboxed library loader
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sandboxed subset of Lua standard libraries.
 */
static const luaL_Reg kSafeLibs[] = {
  {       "_G",      luaopen_base},
  {    "table",     luaopen_table},
  {   "string",    luaopen_string},
  {     "math",      luaopen_math},
  {     "utf8",      luaopen_utf8},
  {"coroutine", luaopen_coroutine},
  {    nullptr,           nullptr}
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
  Q_ASSERT(L != nullptr);

  for (const luaL_Reg* lib = kSafeLibs; lib->func; ++lib) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);
  }

  // Remove dangerous functions from base library
  static const char* const kUnsafe[] = {"dofile", "loadfile", "load"};
  for (const char* name : kUnsafe) {
    lua_pushnil(L);
    lua_setglobal(L, name);
  }

  // Strip string.dump: bytecode exfiltration vector pairs with unsafe loaders.
  lua_getglobal(L, "string");
  if (lua_istable(L, -1)) {
    lua_pushnil(L);
    lua_setfield(L, -2, "dump");
  }
  lua_pop(L, 1);
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
  Q_ASSERT(m_state == nullptr);

  m_state = luaL_newstate();
  Q_ASSERT(m_state != nullptr);

  // Replace default panic (which aborts) with one that throws -> guardedPcall catches
  lua_atpanic(m_state, luaPanicHandler);

  openSafeLibs(m_state);

  // Re-add Lua 5.1 / 5.2 names that 5.4 dropped (math.log10, math.pow, bit32, ...)
  DataModel::installLuaCompat(m_state);

  // Expose notify* + level constants (gated on the active license tier)
  DataModel::NotificationCenter::installScriptApi(m_state);

  // Expose tableGet / tableSet / datasetGetRaw / datasetGetFinal
  DataModel::FrameBuilder::instance().injectTableApiLua(m_state);

  // Expose deviceWrite(data, sourceId?) bound to this engine's source by default
  DataModel::DeviceWriteApi::installLua(m_state, m_sourceId);

  // Expose actionFire(actionId) for firing dashboard actions from the parser
  DataModel::ActionFireApi::installLua(m_state);

  // Expose dashboard.* helpers (clearPlots, setPlotPoints, UI toggles, setActiveWorkspace)
  DataModel::DashboardApi::installLua(m_state);

  // Expose apiCall(method, params?): gated to a read-only allow-list by default
  DataModel::ScriptApiCall::installLua(m_state, m_sourceId);

  // Store 'this' pointer in the Lua registry for the watchdog hook
  lua_pushlightuserdata(m_state, this);
  lua_setfield(m_state, LUA_REGISTRYINDEX, "__ss_engine__");

  // Install instruction-count watchdog hook
  lua_sethook(m_state, watchdogHook, LUA_MASKCOUNT, kHookInstructionCount);

  // Deadline armed per-call; fresh state re-arms the circuit breaker
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
  if (m_state) {
    DataModel::FrameBuilder::instance().tableStore().clearLookupCache();
    lua_close(m_state);
    m_state = nullptr;
  }

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
 * @brief Runs one full Lua garbage-collection cycle.
 */
void DataModel::LuaScriptEngine::collectGarbage()
{
  if (m_state)
    lua_gc(m_state, LUA_GCCOLLECT);
}

/**
 * @brief Resets the engine by recreating the Lua state.
 */
void DataModel::LuaScriptEngine::reset()
{
  destroyState();
  createState();
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

  // Retrieve the engine pointer from the registry
  lua_getfield(L, LUA_REGISTRYINDEX, "__ss_engine__");
  auto* self = static_cast<LuaScriptEngine*>(lua_touserdata(L, -1));
  lua_pop(L, 1);

  // No engine pointer stored, so ignore (defensive, shouldn't happen)
  if (!self) [[unlikely]]
    return;

  // Deadline reached -> abort the current Lua call
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

//--------------------------------------------------------------------------------------------------
// Script loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates and loads a Lua frame parser script.
 */
bool DataModel::LuaScriptEngine::loadScript(const QString& script,
                                            int sourceId,
                                            bool showMessageBoxes)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!script.isEmpty());

  destroyState();
  m_sourceId = sourceId;
  createState();

  // C++-boundary exception guard: a thrown Lua error during load must not crash the host
  try {
    const QByteArray utf8 = script.toUtf8();
    const auto fileName   = QStringLiteral("parser_%1.lua").arg(sourceId).toUtf8();
    const int status =
      luaL_loadbuffer(m_state, utf8.constData(), utf8.size(), fileName.constData());
    if (status != LUA_OK) {
      const QString errorMsg = QString::fromUtf8(lua_tostring(m_state, -1));
      lua_pop(m_state, 1);
      if (showMessageBoxes) {
        Misc::Utilities::showMessageBox(
          QObject::tr("Lua Syntax Error"),
          QObject::tr("The parser code contains an error:\n\n%1").arg(errorMsg),
          QMessageBox::Critical);
      } else {
        qWarning() << "[LuaScriptEngine] Source" << sourceId << "syntax error:" << errorMsg;
      }
      return false;
    }

    if (!runLoadedChunk(sourceId, showMessageBoxes))
      return false;

    if (!ensureParseFunction(sourceId, showMessageBoxes))
      return false;

    if (sourceId == 0 && !probeParseFunction(sourceId, showMessageBoxes))
      return false;

    // Cache parse() in the registry so the hotpath skips a global lookup on every frame.
    lua_getglobal(m_state, "parse");
    m_parseRef = luaL_ref(m_state, LUA_REGISTRYINDEX);

    m_loaded = true;
    return true;
  } catch (const std::exception& e) {
    qWarning() << "[LuaScriptEngine] Source" << sourceId << "load uncaught exception:" << e.what();
  } catch (...) {
    qWarning() << "[LuaScriptEngine] Source" << sourceId << "load uncaught non-std exception";
  }

  // Escaped Lua/C++ error: drop the half-built state and report a clean load failure
  destroyState();
  createState();
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

  destroyState();
  createState();
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
  Q_ASSERT(m_state != nullptr);
  Q_ASSERT(data != nullptr);

  if (!m_loaded || m_disabled)
    return {};

  // C++-boundary exception guard: a thrown Lua error must not crash the host
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
      if (err.contains(QLatin1String("timed out")))
        (void)noteTimeoutAndCheckDisabled(0);

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
  Q_ASSERT(!frame.isEmpty());
  Q_ASSERT(m_state != nullptr);

  const QByteArray utf8 = frame.toUtf8();
  return parseLuaText(utf8.constData(), utf8.size());
}

/**
 * @brief Runs the Lua parse function over a raw UTF-8 byte frame (no QString round-trip).
 */
QList<QStringList> DataModel::LuaScriptEngine::parseUtf8(const QByteArray& frame)
{
  Q_ASSERT(!frame.isEmpty());
  Q_ASSERT(m_state != nullptr);

  return parseLuaText(frame.constData(), frame.size());
}

/**
 * @brief Runs the Lua parse function over a binary frame (1-indexed byte table).
 */
QList<QStringList> DataModel::LuaScriptEngine::parseBinary(const QByteArray& frame)
{
  Q_ASSERT(!frame.isEmpty());
  Q_ASSERT(m_state != nullptr);

  if (!m_loaded || m_disabled)
    return {};

  // C++-boundary exception guard: a thrown Lua error must not crash the host
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
      if (err.contains(QLatin1String("timed out")))
        (void)noteTimeoutAndCheckDisabled(0);

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
 * @brief Converts the Lua value at stack top to a QString without coercing numeric strings.
 */
QString DataModel::LuaScriptEngine::luaValueToString()
{
  Q_ASSERT(m_state != nullptr);

  // Strings pass through verbatim (no strtod/reformat), matching JS; only real numbers format.
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
  Q_ASSERT(lua_istable(m_state, tableIndex));

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
  Q_ASSERT(m_state != nullptr);

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

  return classifyTable(len);
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
  QList<QStringList> results;

  QStringList scalars;
  QList<QStringList> vectors;
  qsizetype maxVectorLength = 0;

  for (int i = 1; i <= qMin(len, kMaxElements); ++i) {
    lua_rawgeti(m_state, -1, i);
    appendMixedElement(scalars, vectors, maxVectorLength);
    lua_pop(m_state, 1);
  }

  lua_pop(m_state, 1);

  if (vectors.isEmpty()) {
    results.append(scalars);
    return results;
  }

  maxVectorLength = qMin(maxVectorLength, static_cast<qsizetype>(kMaxVecLen));

  for (auto& vec : vectors) {
    if (!vec.isEmpty() && vec.size() < maxVectorLength) {
      const QString lastValue = vec.last();
      while (vec.size() < maxVectorLength)
        vec.append(lastValue);
    }
  }

  results.reserve(maxVectorLength);
  for (int i = 0; i < maxVectorLength; ++i) {
    QStringList frame;
    frame.reserve(scalars.size() + vectors.size());
    frame.append(scalars);

    for (const auto& vec : std::as_const(vectors))
      if (i < vec.size())
        frame.append(vec[i]);

    results.append(frame);
  }

  return results;
}
