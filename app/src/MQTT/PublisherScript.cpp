/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "MQTT/PublisherScript.h"

// clang-format off
extern "C" {
#  include <lauxlib.h>
#  include <lua.h>
#  include <luajit.h>
#  include <lualib.h>
}
// clang-format on

#  include <QDebug>
#  include <stdexcept>

#  include "DataModel/Scripting/LuaCompat.h"
#  include "DataModel/Scripting/LuaCompatJIT.h"

/**
 * @brief Calls lua_pcall under a C++ try/catch -- escaped exceptions become LUA_ERRRUN.
 */
[[nodiscard]] static int publisherGuardedPcall(lua_State* L,
                                               int nargs,
                                               int nresults,
                                               int msgh) noexcept
{
  try {
    return lua_pcall(L, nargs, nresults, msgh);
  } catch (...) {
    qWarning() << "[PublisherScript] Uncaught C++ exception escaped lua_pcall -- "
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
static int publisherLuaPanicHandler(lua_State* L)
{
  const char* msg = lua_tostring(L, -1);
  qWarning() << "[PublisherScript] Lua panic:" << (msg ? msg : "<unknown>");
  throw std::runtime_error(msg ? msg : "lua panic");
}

/**
 * @brief Loads a sandboxed library set into a fresh Lua state.
 */
static void openPublisherSafeLibs(lua_State* L)
{
  static const luaL_Reg kSafeLibs[] = {
    {    "_G",   luaopen_base},
    { "table",  luaopen_table},
    {"string", luaopen_string},
    {  "math",   luaopen_math},
    {   "bit",    luaopen_bit},
    { nullptr,        nullptr}
  };

  for (const luaL_Reg* lib = kSafeLibs; lib->func; ++lib) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);
  }

  static const char* const kUnsafe[] = {"dofile", "loadfile", "load"};
  for (const char* name : kUnsafe) {
    lua_pushnil(L);
    lua_setglobal(L, name);
  }

  lua_getglobal(L, "string");
  if (lua_istable(L, -1)) {
    lua_pushnil(L);
    lua_setfield(L, -2, "dump");
  }
  lua_pop(L, 1);

  DataModel::installLuaRestrictedOs(L);
}

/**
 * @brief Constructs the script with no language loaded.
 */
MQTT::PublisherScript::PublisherScript()
  : m_language(JavaScript)
  , m_loaded(false)
  , m_luaState(nullptr)
  , m_deadline(QDeadlineTimer::Forever)
{}

/**
 * @brief Destroys both the JS engine and the Lua state.
 */
MQTT::PublisherScript::~PublisherScript()
{
  destroyLua();
}

/**
 * @brief Compiles the user script in the chosen language.
 */
bool MQTT::PublisherScript::compile(const QString& source, int language, QString& errorOut)
{
  m_loaded   = false;
  m_language = language;

  if (source.trimmed().isEmpty()) {
    errorOut = QStringLiteral("Script is empty.");
    return false;
  }

  if (language == Lua) {
    destroyLua();
    resetJs();

    m_luaState = luaL_newstate();
    if (!m_luaState) {
      errorOut = QStringLiteral("Failed to create Lua state.");
      return false;
    }

    lua_atpanic(m_luaState, publisherLuaPanicHandler);
    openPublisherSafeLibs(m_luaState);
    DataModel::installLuaCompat(m_luaState);

    lua_pushlightuserdata(m_luaState, this);
    lua_setfield(m_luaState, LUA_REGISTRYINDEX, "__ss_publisher__");
    luaJIT_setmode(m_luaState, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);
    lua_sethook(m_luaState, watchdogHook, LUA_MASKCOUNT, kHookInstructionCount);

    const QByteArray utf8 = source.toUtf8();
    m_deadline            = QDeadlineTimer(kRuntimeWatchdogMs);
    int status            = luaL_loadstring(m_luaState, utf8.constData());
    if (status == LUA_OK)
      status = publisherGuardedPcall(m_luaState, 0, LUA_MULTRET, 0);

    m_deadline = QDeadlineTimer(QDeadlineTimer::Forever);
    if (status != LUA_OK) {
      errorOut = QString::fromUtf8(lua_tostring(m_luaState, -1));
      destroyLua();
      return false;
    }

    lua_getglobal(m_luaState, "mqtt");
    const bool hasFn = lua_isfunction(m_luaState, -1);
    lua_pop(m_luaState, 1);
    if (!hasFn) {
      errorOut = QStringLiteral("Script must define a function named 'mqtt(frame)'.");
      destroyLua();
      return false;
    }

    m_loaded = true;
    return true;
  }

  destroyLua();
  if (!m_jsEngine) {
    m_jsEngine = std::make_unique<QJSEngine>();
    m_jsEngine->installExtensions(QJSEngine::ConsoleExtension
                                  | QJSEngine::GarbageCollectionExtension);
    m_jsWatchdog = std::make_unique<DataModel::JsWatchdog>(
      m_jsEngine.get(), kRuntimeWatchdogMs, QStringLiteral("MQTT script"));
  }

  m_jsFunction = QJSValue();

  const QString wrapped = QStringLiteral("(function(){\n%1\n})();").arg(source);
  const auto result     = m_jsEngine->evaluate(wrapped, QStringLiteral("mqtt-script.js"));
  if (result.isError()) {
    errorOut = QStringLiteral("Line %1: %2")
                 .arg(result.property(QStringLiteral("lineNumber")).toInt())
                 .arg(result.toString());
    return false;
  }

  auto fn = m_jsEngine->globalObject().property(QStringLiteral("mqtt"));
  if (!fn.isCallable()) {
    errorOut = QStringLiteral("Script must define a function named 'mqtt(frame)'.");
    return false;
  }

  m_jsFunction = fn;
  m_loaded     = true;
  return true;
}

/**
 * @brief Returns whether a callable mqtt(frame) function is currently loaded.
 */
bool MQTT::PublisherScript::isLoaded() const noexcept
{
  return m_loaded;
}

/**
 * @brief Returns the language enum of the currently loaded script.
 */
int MQTT::PublisherScript::currentLanguage() const noexcept
{
  return m_language;
}

/**
 * @brief Runs mqtt(frame) and writes the byte payload returned by the user.
 */
bool MQTT::PublisherScript::run(const QByteArray& frame, QByteArray& payloadOut, QString& errorOut)
{
  payloadOut.clear();
  if (!m_loaded)
    return false;

  if (m_language == Lua) {
    if (!m_luaState)
      return false;

    lua_getglobal(m_luaState, "mqtt");
    if (!lua_isfunction(m_luaState, -1)) {
      lua_pop(m_luaState, 1);
      errorOut = QStringLiteral("mqtt() is no longer defined");
      return false;
    }

    lua_pushlstring(m_luaState, frame.constData(), static_cast<size_t>(frame.size()));

    m_deadline       = QDeadlineTimer(kRuntimeWatchdogMs);
    const int status = publisherGuardedPcall(m_luaState, 1, 1, 0);
    m_deadline       = QDeadlineTimer(QDeadlineTimer::Forever);
    if (status != LUA_OK) {
      errorOut = QString::fromUtf8(lua_tostring(m_luaState, -1));
      lua_pop(m_luaState, 1);
      return false;
    }

    if (lua_isnoneornil(m_luaState, -1)) {
      lua_pop(m_luaState, 1);
      return true;
    }

    if (lua_isstring(m_luaState, -1)) {
      size_t len       = 0;
      const char* data = lua_tolstring(m_luaState, -1, &len);
      payloadOut       = QByteArray(data, static_cast<int>(len));
      lua_pop(m_luaState, 1);
      return true;
    }

    lua_pop(m_luaState, 1);
    errorOut = QStringLiteral("mqtt() must return a string or nil");
    return false;
  }

  if (!m_jsEngine || !m_jsWatchdog || !m_jsFunction.isCallable())
    return false;

  QJSValueList args;
  args << QJSValue(QString::fromUtf8(frame));

  QJSValue result = m_jsWatchdog->call(m_jsFunction, args);
  if (m_jsWatchdog->lastCallTimedOut()) {
    errorOut = QStringLiteral("Script exceeded %1 ms budget; killed.").arg(kRuntimeWatchdogMs);
    return false;
  }

  if (result.isError()) {
    errorOut = QStringLiteral("Line %1: %2")
                 .arg(result.property(QStringLiteral("lineNumber")).toInt())
                 .arg(result.toString());
    return false;
  }

  if (result.isNull() || result.isUndefined())
    return true;

  if (result.isString()) {
    payloadOut = result.toString().toUtf8();
    return true;
  }

  const QVariant v = result.toVariant();
  if (v.canConvert<QByteArray>()) {
    payloadOut = v.toByteArray();
    return true;
  }

  payloadOut = result.toString().toUtf8();
  return true;
}

/**
 * @brief Drops the compiled function and forces the next compile() to re-evaluate.
 */
void MQTT::PublisherScript::reset()
{
  resetJs();
  destroyLua();
  m_loaded = false;
}

/**
 * @brief Closes and releases the Lua state if one is allocated.
 */
void MQTT::PublisherScript::destroyLua()
{
  if (m_luaState) {
    lua_close(m_luaState);
    m_luaState = nullptr;
  }
}

/**
 * @brief Drops the cached JS function reference and runs GC on the engine.
 */
void MQTT::PublisherScript::resetJs()
{
  m_jsFunction = QJSValue();
  if (m_jsEngine)
    m_jsEngine->collectGarbage();
}

/**
 * @brief Lua debug hook that aborts the running script once its deadline expires.
 */
void MQTT::PublisherScript::watchdogHook(lua_State* L, lua_Debug* ar)
{
  Q_UNUSED(ar)

  lua_getfield(L, LUA_REGISTRYINDEX, "__ss_publisher__");
  auto* self = static_cast<PublisherScript*>(lua_touserdata(L, -1));
  lua_pop(L, 1);

  if (!self)
    return;

  if (self->m_deadline.hasExpired())
    luaL_error(L, "execution timed out after %d ms", kRuntimeWatchdogMs);
}

#endif  // BUILD_COMMERCIAL
