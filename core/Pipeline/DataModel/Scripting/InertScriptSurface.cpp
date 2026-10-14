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

#include "DataModel/Scripting/InertScriptSurface.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJSEngine>
#include <QThread>

// clang-format off
extern "C" {
#include <lauxlib.h>
}
// clang-format on

#include "Core/SSAssert.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ChecksumApi.h"
#include "DataModel/Scripting/ScriptApiCall.h"

//--------------------------------------------------------------------------------------------------
// Inert globals
//--------------------------------------------------------------------------------------------------

/**
 * @brief The JS stand-ins, installed under the names the SDK prelude resolves at call time. The
 *        table object forwards to the scratch bridge, which is reachable only through this
 *        closure, so mqttPublish cannot be reached past it. Every stand-in carries an inert
 *        marker so the install is verified rather than assumed.
 */
static const auto kInertJs = QStringLiteral(
  "(function(g, tables) {"
  "  var dryRun = function() { return { ok: false, error: 'dryRun' }; };"
  "  var none = function() {};"
  "  g.__ss_dw = { inert: true, write: dryRun };"
  "  g.__ss_af = { inert: true, fire: dryRun };"
  "  g.__ss_bridge = { inert: true, call: dryRun, writeAndWait: dryRun, delay: none,"
  "                    listCommands: function() { return []; } };"
  "  g.__ss_db = { inert: true, clearPlots: dryRun, setPlotPoints: dryRun,"
  "                setTerminalVisible: dryRun, setNotificationLogVisible: dryRun,"
  "                setClockVisible: dryRun, setStopwatchVisible: dryRun,"
  "                setActiveWorkspace: dryRun };"
  "  g.__nc = { inert: true, post: none, postInfo: none, postWarning: none,"
  "             postCritical: none, resolve: none };"
  "  g.console = { inert: true, log: none, info: none, warn: none, error: none, debug: none };"
  "  g.print = none;"
  "  var inert = { inert: true,"
  "    tableGet: function(t, r) { return tables.tableGet(t, r); },"
  "    tableGetH: function(h) { return tables.tableGetH(h); },"
  "    tableHandle: function(t, r) { return tables.tableHandle(t, r); },"
  "    tableHandleMany: function(t, rs) { return tables.tableHandleMany(t, rs); },"
  "    datasetGetRaw: function(s) { return tables.datasetGetRaw(s); },"
  "    datasetGetFinal: function(s) { return tables.datasetGetFinal(s); },"
  "    tableSet: function(t, r, v) { tables.tableSet(t, r, v); },"
  "    tableSetH: function(h, v) { tables.tableSetH(h, v); } };"
  "  if (tables.mqttPublish)"
  "    inert.mqttPublish = function() { return -1; };"
  "  g.__ss = inert;"
  "})");

/**
 * @brief Confirms every bridge name a live engine carries is the inert stand-in here.
 */
static const auto kInertJsProbe = QStringLiteral(
  "(function(g) {"
  "  var names = ['__ss_dw', '__ss_af', '__ss_bridge', '__ss_db', '__nc', '__ss', 'console'];"
  "  for (var i = 0; i < names.length; ++i) {"
  "    var v = g[names[i]];"
  "    if (!v || v.inert !== true) return false;"
  "  }"
  "  return typeof g.apiCall === 'function';"
  "})(this)");

/**
 * @brief The Lua stand-ins, which replace the console, notify, device, action, dashboard,
 *        apiCall and mqttPublish globals. The chunk returns the table of what it installed, kept
 *        in the registry so verify() can compare every global by identity.
 */
static const char kInertLua[] =
  "local dryRun = function() return { ok = false, error = 'dryRun' } end\n"
  "local none = function() end\n"
  "local installed = {}\n"
  "local function put(name, value) _G[name] = value; installed[name] = value end\n"
  "put('print', none)\n"
  "put('console', { log = none, info = none, warn = none, error = none, debug = none })\n"
  "Info, Warning, Critical = 0, 1, 2\n"
  "for _, n in ipairs({ 'notify', 'notifyInfo', 'notifyWarning', 'notifyCritical',\n"
  "                     'notifyClear' }) do put(n, none) end\n"
  "for _, n in ipairs({ 'deviceWrite', 'actionFire', 'clearPlots', 'setPlotPoints',\n"
  "                     'setTerminalVisible', 'setNotificationLogVisible', 'setClockVisible',\n"
  "                     'setStopwatchVisible', 'setActiveWorkspace', 'apiCall' }) do\n"
  "  put(n, dryRun)\n"
  "end\n"
  "put('apiCallList', function() return {} end)\n"
  "if mqttPublish ~= nil then put('mqttPublish', function() return -1 end) end\n"
  "return installed\n";

/**
 * @brief Registry key of the table of Lua stand-ins.
 */
static constexpr char kInertLuaKey[] = "__ss_inert__";

/**
 * @brief Upper bound on the stand-in table verify() walks; the chunk installs about twenty.
 */
static constexpr int kMaxInertGlobals = 64;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Seeds the scratch store on the GUI thread, which owns it, so every table read and write
 *        a dry run makes is direct: no marshal into the pipeline thread, no nested event loop.
 */
DataModel::InertScriptSurface::InertScriptSurface()
  : m_owner(), m_store(), m_mirror(), m_tableBridge(m_owner, m_store, m_mirror)
{
  SS_ASSERT_LOG(qApp == nullptr || QThread::currentThread() == qApp->thread());
  seedStore();
}

/**
 * @brief Lays the scratch store out from the project, at its starting values. The live values are
 *        deliberately not copied: reading the GUI mirror registers a GUI table user for good, which
 *        would make every display tick publish a table snapshot nothing reads (spec 0098 review).
 */
void DataModel::InertScriptSurface::seedStore()
{
  auto& project = DataModel::pipelineModules().projectModel;

  DataModel::Frame layout;
  layout.title  = project.title();
  layout.groups = project.groups();
  m_store.initialize(project.tables(), project.editorTableFolders(), layout);
}

//--------------------------------------------------------------------------------------------------
// Installation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Installs the scratch table bridge, the real checksums, the inert stand-ins and the SDK
 *        into @p js, then verifies the result. False means the engine must not run user code.
 */
bool DataModel::InertScriptSurface::installJs(QJSEngine* js)
{
  SS_ASSERT(js != nullptr, return false);

  m_tableBridge.installJs(js);
  DataModel::ChecksumApi::installJS(js);

  auto global       = js->globalObject();
  const auto tables = global.property(QStringLiteral("__ss"));
  auto install      = js->evaluate(kInertJs);
  const auto result = install.call({global, tables});
  if (result.isError()) {
    qWarning() << "[InertScriptSurface] could not install the JS surface:" << result.toString();
    return false;
  }

  DataModel::ScriptApiCall::evaluateSdk(js);
  const bool verified = verify(js);
  SS_ASSERT_LOG(verified);
  return verified;
}

/**
 * @brief Installs the scratch table closures, the real checksums, the inert stand-ins and the
 *        SDK into @p L, then verifies the result. Runs inside the engine's protected bootstrap;
 *        the lookup cache is dropped first because it keys on the previous state's strings.
 */
bool DataModel::InertScriptSurface::installLua(lua_State* L)
{
  SS_ASSERT(L != nullptr, return false);

  m_store.clearLookupCache();
  m_tableBridge.installLua(L);
  DataModel::ChecksumApi::installLua(L);

  if (luaL_dostring(L, kInertLua) != LUA_OK || !lua_istable(L, -1)) {
    qWarning() << "[InertScriptSurface] could not install the Lua surface:"
               << QString::fromUtf8(lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }

  lua_setfield(L, LUA_REGISTRYINDEX, kInertLuaKey);
  DataModel::ScriptApiCall::evaluateSdk(L);
  const bool verified = verify(L);
  SS_ASSERT_LOG(verified);
  return verified;
}

/**
 * @brief Drops the scratch store's interned-key cache, which keys on a Lua state's strings: the
 *        owning engine calls it when it closes a state, instead of reaching the live store.
 */
void DataModel::InertScriptSurface::forgetLookupCache() const
{
  m_store.clearLookupCache();
}

//--------------------------------------------------------------------------------------------------
// Verification
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when every JS bridge name is the inert stand-in and the SDK's apiCall exists. A
 *        check, not an assertion: a reload after user code reassigned a global answers false.
 */
bool DataModel::InertScriptSurface::verify(QJSEngine* js)
{
  SS_ASSERT(js != nullptr, return false);

  return js->evaluate(kInertJsProbe).toBool();
}

/**
 * @brief True when every Lua global the surface installed is still the stand-in it installed.
 */
bool DataModel::InertScriptSurface::verify(lua_State* L)
{
  SS_ASSERT(L != nullptr, return false);

  lua_getfield(L, LUA_REGISTRYINDEX, kInertLuaKey);
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return false;
  }

  bool ok       = true;
  bool finished = false;
  int entries   = 0;
  lua_pushnil(L);
  for (int i = 0; i < kMaxInertGlobals && ok && !finished; ++i) {
    finished = lua_next(L, -2) == 0;
    if (finished)
      break;

    lua_pushvalue(L, -2);
    lua_gettable(L, LUA_GLOBALSINDEX);
    ok = lua_rawequal(L, -1, -2) != 0;
    lua_pop(L, 2);
    ++entries;
  }

  if (!finished)
    lua_pop(L, 1);

  lua_pop(L, 1);
  return ok && finished && entries > 0;
}
