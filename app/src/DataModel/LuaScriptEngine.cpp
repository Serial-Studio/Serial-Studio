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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/LuaScriptEngine.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <QMessageBox>

#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Sandboxed library loader
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sandboxed subset of Lua standard libraries.
 */
static const luaL_Reg kSafeLibs[] = {
  {    "_G",   luaopen_base},
  { "table",  luaopen_table},
  {"string", luaopen_string},
  {  "math",   luaopen_math},
  {  "utf8",   luaopen_utf8},
  { nullptr,        nullptr}
};

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

  // Strip string.dump — bytecode exfiltration vector pairs with unsafe loaders.
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

DataModel::LuaScriptEngine::LuaScriptEngine()
  : m_state(nullptr), m_loaded(false), m_deadline(QDeadlineTimer::Forever)
{
  createState();
}

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

  openSafeLibs(m_state);

  // Expose notify* + level constants (gated on the active license tier)
  DataModel::NotificationCenter::installScriptApi(m_state);

  // Expose tableGet / tableSet / datasetGetRaw / datasetGetFinal
  DataModel::FrameBuilder::instance().injectTableApiLua(m_state);

  // Store 'this' pointer in the Lua registry for the watchdog hook
  lua_pushlightuserdata(m_state, this);
  lua_setfield(m_state, LUA_REGISTRYINDEX, "__ss_engine__");

  // Install instruction-count watchdog hook
  lua_sethook(m_state, watchdogHook, LUA_MASKCOUNT, kHookInstructionCount);

  // Deadline starts "never" — set per-call before entering Lua
  m_deadline = QDeadlineTimer(QDeadlineTimer::Forever);
  m_loaded   = false;
}

/**
 * @brief Closes the Lua state and resets internal flags.
 */
void DataModel::LuaScriptEngine::destroyState()
{
  if (m_state) {
    lua_close(m_state);
    m_state = nullptr;
  }

  m_loaded   = false;
  m_deadline = QDeadlineTimer(QDeadlineTimer::Forever);
}

bool DataModel::LuaScriptEngine::isLoaded() const noexcept
{
  return m_loaded;
}

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

  // No engine pointer stored — ignore (defensive, shouldn't happen)
  if (!self) [[unlikely]]
    return;

  // Deadline reached → abort the current Lua call
  if (self->m_deadline.hasExpired()) [[unlikely]]
    luaL_error(L, "execution timed out after %d ms", kRuntimeWatchdogMs);
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

  // Recreate state with clean environment
  destroyState();
  createState();

  // Evaluate the script
  const QByteArray utf8 = script.toUtf8();
  const auto fileName   = QStringLiteral("parser_%1.lua").arg(sourceId).toUtf8();
  int status = luaL_loadbuffer(m_state, utf8.constData(), utf8.size(), fileName.constData());
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

  // Execute the chunk to define the parse function, under the watchdog
  // (a malicious or buggy script could put `while true do end` at file scope)
  m_deadline.setRemainingTime(kRuntimeWatchdogMs);
  status     = lua_pcall(m_state, 0, 0, 0);
  m_deadline = QDeadlineTimer(QDeadlineTimer::Forever);
  if (status != LUA_OK) {
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

  // Verify that the global 'parse' function exists
  lua_getglobal(m_state, "parse");
  if (!lua_isfunction(m_state, -1)) {
    lua_pop(m_state, 1);
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

  lua_pop(m_state, 1);

  // Probe with test inputs for source 0
  if (sourceId == 0) {
    bool probeOk         = false;
    const char* probes[] = {"0", ""};
    QString lastError;

    for (const char* probe : probes) {
      lua_getglobal(m_state, "parse");
      lua_pushstring(m_state, probe);

      m_deadline.setRemainingTime(kRuntimeWatchdogMs);
      const int probeStatus = lua_pcall(m_state, 1, 1, 0);
      m_deadline            = QDeadlineTimer(QDeadlineTimer::Forever);

      if (probeStatus == LUA_OK) {
        lua_pop(m_state, 1);
        probeOk = true;
        break;
      }

      lastError = QString::fromUtf8(lua_tostring(m_state, -1));
      lua_pop(m_state, 1);
    }

    // Try with a byte table probe
    if (!probeOk) {
      lua_getglobal(m_state, "parse");
      lua_newtable(m_state);
      lua_pushinteger(m_state, 0);
      lua_rawseti(m_state, -2, 1);

      m_deadline.setRemainingTime(kRuntimeWatchdogMs);
      const int probeStatus = lua_pcall(m_state, 1, 1, 0);
      m_deadline            = QDeadlineTimer(QDeadlineTimer::Forever);

      if (probeStatus == LUA_OK) {
        lua_pop(m_state, 1);
        probeOk = true;
      } else {
        lastError = QString::fromUtf8(lua_tostring(m_state, -1));
        lua_pop(m_state, 1);
      }
    }

    if (!probeOk) {
      if (showMessageBoxes) {
        Misc::Utilities::showMessageBox(
          QObject::tr("Parse Function Runtime Error"),
          QObject::tr("The parse function contains an error:\n\n%1\n\n"
                      "Please fix the error in the function body.")
            .arg(lastError),
          QMessageBox::Critical);
      } else {
        qWarning() << "[LuaScriptEngine] Probe failed:" << lastError;
      }

      // Reset state but keep it usable for re-editing
      destroyState();
      createState();
      return false;
    }
  }

  m_loaded = true;
  return true;
}

//--------------------------------------------------------------------------------------------------
// Parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs the Lua parse function over a text frame.
 */
QList<QStringList> DataModel::LuaScriptEngine::parseString(const QString& frame)
{
  Q_ASSERT(!frame.isEmpty());
  Q_ASSERT(m_state != nullptr);

  if (!m_loaded)
    return {};

  // Push parse function and string argument
  lua_getglobal(m_state, "parse");
  const QByteArray utf8 = frame.toUtf8();
  lua_pushlstring(m_state, utf8.constData(), utf8.size());

  // Arm the watchdog deadline for the duration of this call
  m_deadline.setRemainingTime(kRuntimeWatchdogMs);
  const int status = lua_pcall(m_state, 1, 1, 0);
  m_deadline       = QDeadlineTimer(QDeadlineTimer::Forever);

  if (status != LUA_OK) [[unlikely]] {
    const QString err = QString::fromUtf8(lua_tostring(m_state, -1));
    lua_pop(m_state, 1);
    qWarning() << "[LuaScriptEngine] Parse error:" << err;
    return {};
  }

  return convertResult();
}

/**
 * @brief Runs the Lua parse function over a binary frame (1-indexed byte table).
 */
QList<QStringList> DataModel::LuaScriptEngine::parseBinary(const QByteArray& frame)
{
  Q_ASSERT(!frame.isEmpty());
  Q_ASSERT(m_state != nullptr);

  if (!m_loaded)
    return {};

  // Push parse function and byte-table argument
  lua_getglobal(m_state, "parse");

  lua_createtable(m_state, frame.size(), 0);
  const auto* data = reinterpret_cast<const quint8*>(frame.constData());
  for (int i = 0; i < frame.size(); ++i) {
    lua_pushinteger(m_state, data[i]);
    lua_rawseti(m_state, -2, i + 1);
  }

  // Arm the watchdog deadline for the duration of this call
  m_deadline.setRemainingTime(kRuntimeWatchdogMs);
  const int status = lua_pcall(m_state, 1, 1, 0);
  m_deadline       = QDeadlineTimer(QDeadlineTimer::Forever);

  if (status != LUA_OK) [[unlikely]] {
    const QString err = QString::fromUtf8(lua_tostring(m_state, -1));
    lua_pop(m_state, 1);
    qWarning() << "[LuaScriptEngine] Parse error:" << err;
    return {};
  }

  return convertResult();
}

//--------------------------------------------------------------------------------------------------
// Result conversion
//--------------------------------------------------------------------------------------------------

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

    if (lua_isinteger(m_state, -1))
      result.append(QString::number(lua_tointeger(m_state, -1)));
    else if (lua_isnumber(m_state, -1))
      result.append(QString::number(lua_tonumber(m_state, -1), 'g', 15));
    else
      result.append(QString::fromUtf8(lua_tostring(m_state, -1)));

    lua_pop(m_state, 1);
  }

  return result;
}

/**
 * @brief Converts the Lua return value at stack top to a frame list.
 */
QList<QStringList> DataModel::LuaScriptEngine::convertResult()
{
  QList<QStringList> results;

  // Scalar return (number or string)
  if (!lua_istable(m_state, -1)) {
    QStringList frame;
    if (lua_isinteger(m_state, -1))
      frame.append(QString::number(lua_tointeger(m_state, -1)));
    else if (lua_isnumber(m_state, -1))
      frame.append(QString::number(lua_tonumber(m_state, -1), 'g', 15));
    else if (lua_isstring(m_state, -1))
      frame.append(QString::fromUtf8(lua_tostring(m_state, -1)));

    lua_pop(m_state, 1);
    if (!frame.isEmpty())
      results.append(frame);

    return results;
  }

  // Classify table contents
  const int len = static_cast<int>(lua_rawlen(m_state, -1));
  if (len == 0) {
    lua_pop(m_state, 1);
    return results;
  }

  // Scan for sub-tables vs scalars
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

  // Pure 1D array — all scalars
  if (!hasTable) {
    results.append(tableToStringList(-1));
    lua_pop(m_state, 1);
    return results;
  }

  // Pure 2D array — all sub-tables
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

  // Mixed array — scalars + sub-tables (same unzip logic as JS engine)
  QStringList scalars;
  QList<QStringList> vectors;
  qsizetype maxVectorLength = 0;

  for (int i = 1; i <= qMin(len, kMaxElements); ++i) {
    lua_rawgeti(m_state, -1, i);

    if (lua_istable(m_state, -1)) {
      const auto vec = tableToStringList(-1);
      if (!vec.isEmpty()) {
        vectors.append(vec);
        maxVectorLength = std::max(maxVectorLength, vec.size());
      }
    } else if (lua_isinteger(m_state, -1)) {
      scalars.append(QString::number(lua_tointeger(m_state, -1)));
    } else if (lua_isnumber(m_state, -1)) {
      scalars.append(QString::number(lua_tonumber(m_state, -1), 'g', 15));
    } else {
      scalars.append(QString::fromUtf8(lua_tostring(m_state, -1)));
    }

    lua_pop(m_state, 1);
  }

  lua_pop(m_state, 1);

  // No vectors found — treat as flat frame
  if (vectors.isEmpty()) {
    results.append(scalars);
    return results;
  }

  // Cap vector length to prevent OOM
  maxVectorLength = qMin(maxVectorLength, static_cast<qsizetype>(kMaxVecLen));

  // Extend shorter vectors by repeating their last value
  for (auto& vec : vectors) {
    if (!vec.isEmpty() && vec.size() < maxVectorLength) {
      const QString lastValue = vec.last();
      while (vec.size() < maxVectorLength)
        vec.append(lastValue);
    }
  }

  // Build output frames: scalars repeated, vectors unzipped
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
