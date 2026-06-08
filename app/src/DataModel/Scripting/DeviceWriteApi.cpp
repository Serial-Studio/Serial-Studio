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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Scripting/DeviceWriteApi.h"

#include <lauxlib.h>
#include <lua.h>

#include <QByteArray>
#include <QDebug>
#include <QJSEngine>
#include <QString>
#include <stdexcept>

#include "IO/ConnectionManager.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Shared write + logging core
//--------------------------------------------------------------------------------------------------

/**
 * @brief Performs the synchronous write to @p sourceId and logs the attempt.
 */
static qint64 performDeviceWrite(int sourceId, const QByteArray& data, QString& errorMsgOut)
{
  if (sourceId < 0) {
    errorMsgOut = QStringLiteral("deviceWrite: sourceId must be >= 0");
    qInfo().noquote() << QStringLiteral("[deviceWrite] rejected source=%1 bytes=%2 (negative id)")
                           .arg(sourceId)
                           .arg(data.size());
    return -1;
  }

  qint64 written = -1;
  try {
    written = IO::ConnectionManager::instance().writeDataToDevice(sourceId, data);
  } catch (const std::exception& e) {
    errorMsgOut = QString::fromUtf8(e.what());
  } catch (...) {
    errorMsgOut = QStringLiteral("unknown exception");
  }

  if (written <= 0 && errorMsgOut.isEmpty())
    errorMsgOut = QStringLiteral("device not connected or write failed");

  qInfo().noquote() << QStringLiteral("[deviceWrite] source=%1 bytes=%2 written=%3")
                         .arg(sourceId)
                         .arg(data.size())
                         .arg(written);
  return written;
}

//--------------------------------------------------------------------------------------------------
// Lua glue
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pushes a {ok, error?} table onto the Lua stack.
 */
static void pushLuaResult(lua_State* L, bool ok, const QString& errorMsg)
{
  lua_createtable(L, 0, ok ? 1 : 2);

  lua_pushboolean(L, ok ? 1 : 0);
  lua_setfield(L, -2, "ok");

  if (!ok) {
    const QByteArray utf8 = errorMsg.toUtf8();
    lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
    lua_setfield(L, -2, "error");
  }
}

/**
 * @brief Lua C closure for deviceWrite(data, sourceId?) returning a result table.
 */
static int luaDeviceWrite(lua_State* L)
{
  const int defaultSourceId = static_cast<int>(lua_tointeger(L, lua_upvalueindex(1)));

  if (!lua_isstring(L, 1)) {
    pushLuaResult(L, false, QStringLiteral("deviceWrite: data must be a string"));
    return 1;
  }

  size_t len      = 0;
  const char* str = lua_tolstring(L, 1, &len);

  int sourceId = defaultSourceId;
  if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
    if (!lua_isnumber(L, 2)) {
      pushLuaResult(L, false, QStringLiteral("deviceWrite: sourceId must be a number"));
      return 1;
    }
    sourceId = static_cast<int>(lua_tointeger(L, 2));
  }

  const QByteArray bytes(str, static_cast<int>(len));
  if (bytes.isEmpty()) {
    pushLuaResult(L, false, QStringLiteral("deviceWrite: data is empty"));
    return 1;
  }

  QString errorMsg;
  const qint64 written = performDeviceWrite(sourceId, bytes, errorMsg);
  pushLuaResult(L, written > 0 && errorMsg.isEmpty(), errorMsg);
  return 1;
}

//--------------------------------------------------------------------------------------------------
// JS bridge
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a JS-side deviceWrite bridge owned by @p parent.
 */
DataModel::DeviceWriteBridge::DeviceWriteBridge(QObject* parent) : QObject(parent) {}

/**
 * @brief Implements deviceWrite(data, sourceId?) for the JS scripting engine.
 */
QVariantMap DataModel::DeviceWriteBridge::write(const QJSValue& data, const QJSValue& sourceIdVal)
{
  auto makeError = [](const QString& msg) -> QVariantMap {
    QVariantMap m;
    m.insert(QStringLiteral("ok"), false);
    m.insert(QStringLiteral("error"), msg);
    return m;
  };

  int sourceId = defaultSourceId;
  if (!sourceIdVal.isUndefined() && !sourceIdVal.isNull()) {
    if (!sourceIdVal.isNumber())
      return makeError(QStringLiteral("deviceWrite: sourceId must be a number"));

    sourceId = static_cast<int>(sourceIdVal.toInt());
  }

  QByteArray bytes;
  if (data.isString()) {
    bytes = data.toString().toUtf8();
  } else if (data.isArray()) {
    const int len = data.property(QStringLiteral("length")).toInt();
    bytes.reserve(len);
    for (int i = 0; i < len; ++i)
      bytes.append(static_cast<char>(data.property(static_cast<quint32>(i)).toInt() & 0xff));
  } else {
    return makeError(QStringLiteral("deviceWrite: data must be a string or byte array"));
  }

  if (bytes.isEmpty())
    return makeError(QStringLiteral("deviceWrite: data is empty"));

  QString errorMsg;
  const qint64 written = performDeviceWrite(sourceId, bytes, errorMsg);

  QVariantMap result;
  if (written > 0 && errorMsg.isEmpty()) {
    result.insert(QStringLiteral("ok"), true);
  } else {
    result.insert(QStringLiteral("ok"), false);
    result.insert(QStringLiteral("error"), errorMsg);
  }
  return result;
}

//--------------------------------------------------------------------------------------------------
// Public installation entry points
//--------------------------------------------------------------------------------------------------

/**
 * @brief Installs deviceWrite() as a Lua global with @p defaultSourceId in its upvalue.
 */
void DataModel::DeviceWriteApi::installLua(lua_State* L, int defaultSourceId)
{
  Q_ASSERT(L);

  lua_pushinteger(L, defaultSourceId);
  lua_pushcclosure(L, luaDeviceWrite, 1);
  lua_setglobal(L, "deviceWrite");
}

/**
 * @brief Installs deviceWrite() into a QJSEngine, reusing the bridge if already attached.
 */
void DataModel::DeviceWriteApi::installJS(QJSEngine* js, int defaultSourceId)
{
  Q_ASSERT(js);

  auto global       = js->globalObject();
  auto existingProp = global.property(QStringLiteral("__ss_dw"));

  DataModel::DeviceWriteBridge* bridge = nullptr;
  if (existingProp.isQObject())
    bridge = qobject_cast<DataModel::DeviceWriteBridge*>(existingProp.toQObject());

  if (!bridge) {
    bridge         = new DataModel::DeviceWriteBridge(js);
    auto bridgeVal = js->newQObject(bridge);
    global.setProperty(QStringLiteral("__ss_dw"), bridgeVal);

    js->evaluate(QStringLiteral("function deviceWrite(data, sourceId) {\n"
                                "  return __ss_dw.write(data, sourceId);\n"
                                "}\n"));
  }

  bridge->defaultSourceId = defaultSourceId;
}

/**
 * @brief Updates the Lua deviceWrite() default sourceId by re-installing the closure.
 */
void DataModel::DeviceWriteApi::setLuaDefaultSourceId(lua_State* L, int defaultSourceId)
{
  installLua(L, defaultSourceId);
}

/**
 * @brief Updates the JS deviceWrite() default sourceId in place; no-op if not installed.
 */
void DataModel::DeviceWriteApi::setJSDefaultSourceId(QJSEngine* js, int defaultSourceId)
{
  Q_ASSERT(js);

  auto existingProp = js->globalObject().property(QStringLiteral("__ss_dw"));
  if (!existingProp.isQObject())
    return;

  if (auto* bridge = qobject_cast<DataModel::DeviceWriteBridge*>(existingProp.toQObject()))
    bridge->defaultSourceId = defaultSourceId;
}

//--------------------------------------------------------------------------------------------------
// actionFire glue
//--------------------------------------------------------------------------------------------------

/**
 * @brief Dispatches an action by its public actionId; returns ok + diagnostic on failure.
 */
static bool fireActionByPublicId(int actionId, QString& errorMsgOut)
{
  const int idx = UI::Dashboard::instance().actionIndexForId(actionId);
  if (idx < 0) {
    errorMsgOut = QStringLiteral("actionFire: no action with actionId=%1").arg(actionId);
    qInfo().noquote() << QStringLiteral("[actionFire] rejected id=%1 (not found)").arg(actionId);
    return false;
  }

  try {
    UI::Dashboard::instance().activateAction(idx, false);
  } catch (const std::exception& e) {
    errorMsgOut = QString::fromUtf8(e.what());
  } catch (...) {
    errorMsgOut = QStringLiteral("unknown exception");
  }

  qInfo().noquote() << QStringLiteral("[actionFire] id=%1 index=%2 %3")
                         .arg(actionId)
                         .arg(idx)
                         .arg(errorMsgOut.isEmpty() ? QStringLiteral("ok") : errorMsgOut);
  return errorMsgOut.isEmpty();
}

/**
 * @brief Lua C closure for actionFire(actionId) returning {ok, error?}.
 */
static int luaActionFire(lua_State* L)
{
  if (!lua_isnumber(L, 1)) {
    pushLuaResult(L, false, QStringLiteral("actionFire: actionId must be a number"));
    return 1;
  }

  const int actionId = static_cast<int>(lua_tointeger(L, 1));
  QString errorMsg;
  const bool ok = fireActionByPublicId(actionId, errorMsg);
  pushLuaResult(L, ok, errorMsg);
  return 1;
}

/**
 * @brief Installs actionFire() as a Lua global.
 */
void DataModel::ActionFireApi::installLua(lua_State* L)
{
  Q_ASSERT(L);

  lua_pushcfunction(L, luaActionFire);
  lua_setglobal(L, "actionFire");
}

/**
 * @brief Constructs a JS-side actionFire bridge owned by @p parent.
 */
DataModel::ActionFireBridge::ActionFireBridge(QObject* parent) : QObject(parent) {}

/**
 * @brief Implements actionFire(actionId) for the JS scripting engine.
 */
QVariantMap DataModel::ActionFireBridge::fire(const QJSValue& actionIdVal)
{
  auto makeError = [](const QString& msg) -> QVariantMap {
    QVariantMap m;
    m.insert(QStringLiteral("ok"), false);
    m.insert(QStringLiteral("error"), msg);
    return m;
  };

  if (!actionIdVal.isNumber())
    return makeError(QStringLiteral("actionFire: actionId must be a number"));

  const int actionId = static_cast<int>(actionIdVal.toInt());
  QString errorMsg;
  const bool ok = fireActionByPublicId(actionId, errorMsg);

  QVariantMap result;
  result.insert(QStringLiteral("ok"), ok);
  if (!ok)
    result.insert(QStringLiteral("error"), errorMsg);

  return result;
}

/**
 * @brief Installs actionFire() into a QJSEngine, reusing the bridge if already attached.
 */
void DataModel::ActionFireApi::installJS(QJSEngine* js)
{
  Q_ASSERT(js);

  auto global       = js->globalObject();
  auto existingProp = global.property(QStringLiteral("__ss_af"));
  if (existingProp.isQObject()
      && qobject_cast<DataModel::ActionFireBridge*>(existingProp.toQObject()))
    return;

  auto* bridge   = new DataModel::ActionFireBridge(js);
  auto bridgeVal = js->newQObject(bridge);
  global.setProperty(QStringLiteral("__ss_af"), bridgeVal);

  js->evaluate(QStringLiteral("function actionFire(actionId) {\n"
                              "  return __ss_af.fire(actionId);\n"
                              "}\n"));
}
