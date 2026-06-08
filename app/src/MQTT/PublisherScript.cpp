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

#  include <lauxlib.h>
#  include <lua.h>
#  include <lualib.h>

#  include "DataModel/Scripting/LuaCompat.h"

/**
 * @brief Loads a sandboxed library set into a fresh Lua state.
 */
static void openSafeLibs(lua_State* L)
{
  static const luaL_Reg kSafeLibs[] = {
    {    "_G",   luaopen_base},
    { "table",  luaopen_table},
    {"string", luaopen_string},
    {  "math",   luaopen_math},
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
}

/**
 * @brief Constructs the script with no language loaded.
 */
MQTT::PublisherScript::PublisherScript()
  : m_language(JavaScript), m_loaded(false), m_luaState(nullptr)
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

    openSafeLibs(m_luaState);
    DataModel::installLuaCompat(m_luaState);

    const QByteArray utf8 = source.toUtf8();
    if (luaL_dostring(m_luaState, utf8.constData()) != LUA_OK) {
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
    if (lua_pcall(m_luaState, 1, 1, 0) != LUA_OK) {
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

#endif  // BUILD_COMMERCIAL
