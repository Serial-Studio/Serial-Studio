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

#include "DataModel/Scripting/DashboardApi.h"

#include <lauxlib.h>
#include <lua.h>

#include <QByteArray>
#include <QJSEngine>
#include <QString>
#include <stdexcept>

#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Result helpers
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
 * @brief Builds a JS-style {ok, error?} map.
 */
static QVariantMap makeJsResult(bool ok, const QString& errorMsg)
{
  QVariantMap result;
  result.insert(QStringLiteral("ok"), ok);
  if (!ok)
    result.insert(QStringLiteral("error"), errorMsg);

  return result;
}

//--------------------------------------------------------------------------------------------------
// Core operations (main-thread only: parser + transform engines both run on main)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clears time-series plot buffers without rebuilding widgets or actions.
 */
static bool coreClearPlots(QString& errorMsgOut)
{
  try {
    UI::Dashboard::instance().clearPlotData();
  } catch (const std::exception& e) {
    errorMsgOut = QString::fromUtf8(e.what());
  } catch (...) {
    errorMsgOut = QStringLiteral("unknown exception");
  }

  return errorMsgOut.isEmpty();
}

/**
 * @brief Sets the dashboard plot horizon (sample count).
 */
static bool coreSetPlotPoints(int points, QString& errorMsgOut)
{
  if (points < 1) {
    errorMsgOut = QStringLiteral("setPlotPoints: points must be >= 1");
    return false;
  }

  try {
    UI::Dashboard::instance().setPoints(points);
  } catch (const std::exception& e) {
    errorMsgOut = QString::fromUtf8(e.what());
  } catch (...) {
    errorMsgOut = QStringLiteral("unknown exception");
  }

  return errorMsgOut.isEmpty();
}

/**
 * @brief Toggles a boolean dashboard property via a Dashboard setter.
 */
static bool coreSetVisibility(void (UI::Dashboard::*setter)(const bool),
                              bool visible,
                              QString& errorMsgOut)
{
  try {
    (UI::Dashboard::instance().*setter)(visible);
  } catch (const std::exception& e) {
    errorMsgOut = QString::fromUtf8(e.what());
  } catch (...) {
    errorMsgOut = QStringLiteral("unknown exception");
  }

  return errorMsgOut.isEmpty();
}

/**
 * @brief Resolves a numeric workspace id, leaving validation to ProjectModel.
 */
static bool coreSetActiveWorkspaceById(int workspaceId, QString& errorMsgOut)
{
  try {
    DataModel::ProjectModel::instance().setActiveGroupId(workspaceId);
  } catch (const std::exception& e) {
    errorMsgOut = QString::fromUtf8(e.what());
  } catch (...) {
    errorMsgOut = QStringLiteral("unknown exception");
  }

  return errorMsgOut.isEmpty();
}

/**
 * @brief Resolves a workspace by case-insensitive title match against the active set.
 */
static bool coreSetActiveWorkspaceByName(const QString& name, QString& errorMsgOut)
{
  if (name.isEmpty()) {
    errorMsgOut = QStringLiteral("setActiveWorkspace: name must not be empty");
    return false;
  }

  int matchedId = -1;
  try {
    const auto& workspaces = DataModel::ProjectModel::instance().activeWorkspaces();
    for (const auto& w : workspaces) {
      if (w.title.compare(name, Qt::CaseInsensitive) == 0) {
        matchedId = w.workspaceId;
        break;
      }
    }
  } catch (const std::exception& e) {
    errorMsgOut = QString::fromUtf8(e.what());
    return false;
  } catch (...) {
    errorMsgOut = QStringLiteral("unknown exception");
    return false;
  }

  if (matchedId < 0) {
    errorMsgOut = QStringLiteral("setActiveWorkspace: no workspace named \"%1\"").arg(name);
    return false;
  }

  return coreSetActiveWorkspaceById(matchedId, errorMsgOut);
}

//--------------------------------------------------------------------------------------------------
// Lua glue: one C closure per public name
//--------------------------------------------------------------------------------------------------

/**
 * @brief Lua entry for clearPlots() returning {ok, error?}.
 */
static int luaClearPlots(lua_State* L)
{
  QString errorMsg;
  const bool ok = coreClearPlots(errorMsg);
  pushLuaResult(L, ok, errorMsg);
  return 1;
}

/**
 * @brief Lua entry for setPlotPoints(n) returning {ok, error?}.
 */
static int luaSetPlotPoints(lua_State* L)
{
  if (!lua_isnumber(L, 1)) {
    pushLuaResult(L, false, QStringLiteral("setPlotPoints: points must be a number"));
    return 1;
  }

  QString errorMsg;
  const bool ok = coreSetPlotPoints(static_cast<int>(lua_tointeger(L, 1)), errorMsg);
  pushLuaResult(L, ok, errorMsg);
  return 1;
}

/**
 * @brief Lua entry for a single boolean-arg dashboard setter.
 */
template<void (UI::Dashboard::*Setter)(const bool)>
static int luaSetVisibility(lua_State* L)
{
  if (!lua_isboolean(L, 1)) {
    pushLuaResult(L, false, QStringLiteral("visibility: argument must be a boolean"));
    return 1;
  }

  QString errorMsg;
  const bool ok = coreSetVisibility(Setter, lua_toboolean(L, 1) != 0, errorMsg);
  pushLuaResult(L, ok, errorMsg);
  return 1;
}

/**
 * @brief Lua entry for setActiveWorkspace(idOrName) returning {ok, error?}.
 */
static int luaSetActiveWorkspace(lua_State* L)
{
  QString errorMsg;
  bool ok = false;

  if (lua_isnumber(L, 1)) {
    ok = coreSetActiveWorkspaceById(static_cast<int>(lua_tointeger(L, 1)), errorMsg);
  } else if (lua_isstring(L, 1)) {
    size_t len      = 0;
    const char* str = lua_tolstring(L, 1, &len);
    ok = coreSetActiveWorkspaceByName(QString::fromUtf8(str, static_cast<int>(len)), errorMsg);
  } else {
    errorMsg = QStringLiteral("setActiveWorkspace: argument must be a workspace id or name");
  }

  pushLuaResult(L, ok, errorMsg);
  return 1;
}

//--------------------------------------------------------------------------------------------------
// JS bridge: one QJSEngine bridge instance shared by every dashboard.* call
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the JS-side dashboard bridge owned by @p parent.
 */
DataModel::DashboardBridge::DashboardBridge(QObject* parent) : QObject(parent) {}

/**
 * @brief Implements clearPlots() for JS scripts.
 */
QVariantMap DataModel::DashboardBridge::clearPlots()
{
  QString errorMsg;
  const bool ok = coreClearPlots(errorMsg);
  return makeJsResult(ok, errorMsg);
}

/**
 * @brief Implements setPlotPoints(n) for JS scripts.
 */
QVariantMap DataModel::DashboardBridge::setPlotPoints(const QJSValue& pointsVal)
{
  if (!pointsVal.isNumber())
    return makeJsResult(false, QStringLiteral("setPlotPoints: points must be a number"));

  QString errorMsg;
  const bool ok = coreSetPlotPoints(static_cast<int>(pointsVal.toInt()), errorMsg);
  return makeJsResult(ok, errorMsg);
}

/**
 * @brief Shared body for the four JS visibility setters.
 */
static QVariantMap setVisibilityJs(const QJSValue& visibleVal,
                                   const char* name,
                                   void (UI::Dashboard::*setter)(const bool))
{
  if (!visibleVal.isBool())
    return makeJsResult(false, QStringLiteral("%1: visible must be a boolean").arg(name));

  QString errorMsg;
  const bool ok = coreSetVisibility(setter, visibleVal.toBool(), errorMsg);
  return makeJsResult(ok, errorMsg);
}

/**
 * @brief Implements setTerminalVisible(bool) for JS scripts.
 */
QVariantMap DataModel::DashboardBridge::setTerminalVisible(const QJSValue& visibleVal)
{
  return setVisibilityJs(visibleVal, "setTerminalVisible", &UI::Dashboard::setTerminalEnabled);
}

/**
 * @brief Implements setNotificationLogVisible(bool) for JS scripts.
 */
QVariantMap DataModel::DashboardBridge::setNotificationLogVisible(const QJSValue& visibleVal)
{
  return setVisibilityJs(
    visibleVal, "setNotificationLogVisible", &UI::Dashboard::setNotificationLogEnabled);
}

/**
 * @brief Implements setClockVisible(bool) for JS scripts.
 */
QVariantMap DataModel::DashboardBridge::setClockVisible(const QJSValue& visibleVal)
{
  return setVisibilityJs(visibleVal, "setClockVisible", &UI::Dashboard::setClockEnabled);
}

/**
 * @brief Implements setStopwatchVisible(bool) for JS scripts.
 */
QVariantMap DataModel::DashboardBridge::setStopwatchVisible(const QJSValue& visibleVal)
{
  return setVisibilityJs(visibleVal, "setStopwatchVisible", &UI::Dashboard::setStopwatchEnabled);
}

/**
 * @brief Implements setActiveWorkspace(idOrName) for JS scripts.
 */
QVariantMap DataModel::DashboardBridge::setActiveWorkspace(const QJSValue& targetVal)
{
  QString errorMsg;
  bool ok = false;

  if (targetVal.isNumber())
    ok = coreSetActiveWorkspaceById(static_cast<int>(targetVal.toInt()), errorMsg);
  else if (targetVal.isString())
    ok = coreSetActiveWorkspaceByName(targetVal.toString(), errorMsg);
  else
    errorMsg = QStringLiteral("setActiveWorkspace: argument must be a workspace id or name");

  return makeJsResult(ok, errorMsg);
}

//--------------------------------------------------------------------------------------------------
// Installation entry points
//--------------------------------------------------------------------------------------------------

/**
 * @brief Installs the dashboard.* helpers as Lua globals.
 */
void DataModel::DashboardApi::installLua(lua_State* L)
{
  Q_ASSERT(L);

  lua_pushcfunction(L, luaClearPlots);
  lua_setglobal(L, "clearPlots");

  lua_pushcfunction(L, luaSetPlotPoints);
  lua_setglobal(L, "setPlotPoints");

  lua_pushcfunction(L, &luaSetVisibility<&UI::Dashboard::setTerminalEnabled>);
  lua_setglobal(L, "setTerminalVisible");

  lua_pushcfunction(L, &luaSetVisibility<&UI::Dashboard::setNotificationLogEnabled>);
  lua_setglobal(L, "setNotificationLogVisible");

  lua_pushcfunction(L, &luaSetVisibility<&UI::Dashboard::setClockEnabled>);
  lua_setglobal(L, "setClockVisible");

  lua_pushcfunction(L, &luaSetVisibility<&UI::Dashboard::setStopwatchEnabled>);
  lua_setglobal(L, "setStopwatchVisible");

  lua_pushcfunction(L, luaSetActiveWorkspace);
  lua_setglobal(L, "setActiveWorkspace");
}

/**
 * @brief Installs the dashboard.* helpers into a QJSEngine, reusing the bridge if attached.
 */
void DataModel::DashboardApi::installJS(QJSEngine* js)
{
  Q_ASSERT(js);

  auto global       = js->globalObject();
  auto existingProp = global.property(QStringLiteral("__ss_db"));
  if (existingProp.isQObject()
      && qobject_cast<DataModel::DashboardBridge*>(existingProp.toQObject()))
    return;

  auto* bridge   = new DataModel::DashboardBridge(js);
  auto bridgeVal = js->newQObject(bridge);
  global.setProperty(QStringLiteral("__ss_db"), bridgeVal);

  js->evaluate(QStringLiteral(
    "function clearPlots()                       { return __ss_db.clearPlots(); }\n"
    "function setPlotPoints(n)                   { return __ss_db.setPlotPoints(n); }\n"
    "function setTerminalVisible(v)              { return __ss_db.setTerminalVisible(v); }\n"
    "function setNotificationLogVisible(v)       { return __ss_db.setNotificationLogVisible(v); }\n"
    "function setClockVisible(v)                 { return __ss_db.setClockVisible(v); }\n"
    "function setStopwatchVisible(v)             { return __ss_db.setStopwatchVisible(v); }\n"
    "function setActiveWorkspace(idOrName)       { return __ss_db.setActiveWorkspace(idOrName); }\n"));
}
