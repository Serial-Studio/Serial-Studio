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

#include "DataModel/FrameBuilder/TableScriptBridge.h"

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
}
// clang-format on

#include <algorithm>
#include <QString>
#include <QStringList>
#include <QThread>
#include <utility>
#include <vector>

#include "Core/SSAssert.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/LuaCompatJIT.h"
#include "IO/PipelineHost.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Publisher.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Reserve hint only: luaL_len honours __len, so an untrusted length must never size an allocation
static constexpr lua_Integer kLuaHandleBatchHint = 1024;

//--------------------------------------------------------------------------------------------------
// Lua C closures
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pushes a register value (nil when absent) onto the Lua stack. Lua-thread only: every
 *        closure below resolves its value first and calls this after, because a lua_State must
 *        never be touched from inside a cross-thread marshal.
 */
static void luaPushRegister(lua_State* L, const DataModel::RegisterValue* val)
{
  if (!val) {
    lua_pushnil(L);
    return;
  }

  if (val->isNumeric) {
    lua_pushnumber(L, val->numericValue);
    return;
  }

  const auto utf8 = val->stringValue.toUtf8();
  lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
}

/**
 * @brief Returns the table-API context a closure carries as its upvalue.
 */
[[nodiscard]] static DataModel::TableApiContext* luaTableContext(lua_State* L)
{
  return static_cast<DataModel::TableApiContext*>(lua_touserdata(L, lua_upvalueindex(1)));
}

/**
 * @brief True when this Lua state runs on the thread that owns the store, i.e. the parser and
 *        dataset-transform engines. The interned-pointer caches are valid only here: they key on
 *        raw lua_State string pointers, which are meaningless across states on other threads.
 */
[[nodiscard]] static bool luaOnStoreThread(const DataModel::TableApiContext* ctx)
{
  return QThread::currentThread() == ctx->owner->thread();
}

/**
 * @brief Lua C closure for tableGet(table, reg).
 */
static int luaTableGet(lua_State* L)
{
  auto* ctx = luaTableContext(L);
  SS_ASSERT(ctx && ctx->store, {
    lua_pushnil(L);
    return 1;
  });

  const char* table = luaL_checkstring(L, 1);
  const char* reg   = luaL_checkstring(L, 2);

  if (luaOnStoreThread(ctx)) [[likely]] {
    luaPushRegister(L, ctx->store->getByInternedKey(table, reg));
    return 1;
  }

  const QString t = QString::fromUtf8(table);
  const QString r = QString::fromUtf8(reg);

  bool found = false;
  DataModel::RegisterValue value;
  DataModel::readTableView(*ctx, [&](const auto& view) {
    if (const auto* val = view.get(t, r)) {
      value = *val;
      found = true;
    }
  });

  luaPushRegister(L, found ? &value : nullptr);
  return 1;
}

/**
 * @brief Lua C closure for tableSet(table, reg, value). Cache-aware like tableGet. A nil value
 *        (e.g. a failed tonumber()) is a safe no-op for parity with JS, which never raises here.
 */
static int luaTableSet(lua_State* L)
{
  auto* ctx = luaTableContext(L);
  SS_ASSERT(ctx && ctx->store, return 0);

  const char* table = luaL_checkstring(L, 1);
  const char* reg   = luaL_checkstring(L, 2);

  if (lua_isnoneornil(L, 3))
    return 0;

  DataModel::RegisterValue rv;
  if (lua_isnumber(L, 3)) {
    rv.numericValue = lua_tonumber(L, 3);
    rv.isNumeric    = true;
  } else {
    rv.stringValue = QString::fromUtf8(luaL_checkstring(L, 3));
    rv.isNumeric   = false;
  }

  if (luaOnStoreThread(ctx)) [[likely]] {
    (void)ctx->store->setByInternedKey(table, reg, rv);
    return 0;
  }

  const QString t = QString::fromUtf8(table);
  const QString r = QString::fromUtf8(reg);
  DataModel::writeTableStore(*ctx,
                             [ctx, t, r, rv = std::move(rv)] { (void)ctx->store->set(t, r, rv); });
  return 0;
}

/**
 * @brief Lua C closure for tableHandle(table, reg) -> handle; resolve once, off the hot path.
 */
static int luaTableHandle(lua_State* L)
{
  auto* ctx = luaTableContext(L);
  SS_ASSERT(ctx && ctx->store, {
    lua_pushnil(L);
    return 1;
  });

  const QString table = QString::fromUtf8(luaL_checkstring(L, 1));
  const QString reg   = QString::fromUtf8(luaL_checkstring(L, 2));

  qint64 handle = -1;
  DataModel::readTableView(*ctx, [&](const auto& view) { handle = view.handleOf(table, reg); });

  lua_pushnumber(L, static_cast<lua_Number>(handle));
  return 1;
}

/**
 * @brief Lua C closure for tableHandleMany(table, regs) -> handles; one handle per name, -1 if
 *        unknown. Every name is collected before the store is reached so the whole batch costs
 *        one thread crossing instead of one per name.
 */
static int luaTableHandleMany(lua_State* L)
{
  auto* ctx = luaTableContext(L);
  SS_ASSERT(ctx && ctx->store, {
    lua_pushnil(L);
    return 1;
  });

  const QString table = QString::fromUtf8(luaL_checkstring(L, 1));
  luaL_checktype(L, 2, LUA_TTABLE);

  const lua_Integer n = luaL_len(L, 2);
  QStringList names;
  names.reserve(static_cast<qsizetype>(std::min<lua_Integer>(n, kLuaHandleBatchHint)));
  for (lua_Integer i = 1; i <= n; ++i) {
    lua_geti(L, 2, i);
    names.append(QString::fromUtf8(luaL_checkstring(L, -1)));
    lua_pop(L, 1);
  }

  std::vector<qint64> handles;
  handles.reserve(names.size());
  DataModel::readTableView(*ctx, [&](const auto& view) {
    for (const auto& reg : names)
      handles.push_back(view.handleOf(table, reg));
  });

  lua_newtable(L);
  for (std::size_t i = 0; i < handles.size(); ++i) {
    lua_pushnumber(L, static_cast<lua_Number>(handles[i]));
    lua_seti(L, -2, static_cast<lua_Integer>(i + 1));
  }

  return 1;
}

/**
 * @brief Lua C closure for tableGetH(handle); nil for a stale or invalid handle.
 */
static int luaTableGetH(lua_State* L)
{
  auto* ctx = luaTableContext(L);
  SS_ASSERT(ctx && ctx->store, {
    lua_pushnil(L);
    return 1;
  });

  const qint64 handle = static_cast<qint64>(luaL_checknumber(L, 1));

  bool found = false;
  DataModel::RegisterValue value;
  DataModel::readTableView(*ctx, [&](const auto& view) {
    if (const auto* val = view.getByHandle(handle)) {
      value = *val;
      found = true;
    }
  });

  luaPushRegister(L, found ? &value : nullptr);
  return 1;
}

/**
 * @brief Lua C closure for tableSetH(handle, value); ignores non-computed/stale/invalid handles.
 *        A nil value (e.g. a failed tonumber()) is a safe no-op for parity with JS, which never
 *        raises here; a raise would fail the load-time parse() probe and reject the script.
 */
static int luaTableSetH(lua_State* L)
{
  auto* ctx = luaTableContext(L);
  SS_ASSERT(ctx && ctx->store, return 0);

  const qint64 handle = static_cast<qint64>(luaL_checknumber(L, 1));

  if (lua_isnoneornil(L, 2))
    return 0;

  DataModel::RegisterValue rv;
  if (lua_isnumber(L, 2)) {
    rv.numericValue = lua_tonumber(L, 2);
    rv.isNumeric    = true;
  } else {
    rv.stringValue = QString::fromUtf8(luaL_checkstring(L, 2));
    rv.isNumeric   = false;
  }

  DataModel::writeTableStore(
    *ctx, [ctx, handle, rv = std::move(rv)] { (void)ctx->store->setByHandle(handle, rv); });
  return 0;
}

/**
 * @brief Resolves a datasetGet* argument on the Lua thread: a string arg is always an alias, a
 *        number always a uniqueId -- never coerce one to the other (lua_type, not lua_isnumber).
 *        Reading it here also keeps luaL_checkinteger's error longjmp on the Lua state's thread.
 */
[[nodiscard]] static const char* luaDatasetSelector(lua_State* L, int* uniqueId)
{
  if (lua_type(L, 1) == LUA_TSTRING)
    return lua_tostring(L, 1);

  *uniqueId = static_cast<int>(luaL_checkinteger(L, 1));
  return nullptr;
}

/**
 * @brief Lua C closure for datasetGetRaw(uniqueIdOrAlias).
 */
static int luaDatasetGetRaw(lua_State* L)
{
  auto* ctx = luaTableContext(L);
  SS_ASSERT(ctx && ctx->store, {
    lua_pushnil(L);
    return 1;
  });

  int uniqueId      = -1;
  const char* alias = luaDatasetSelector(L, &uniqueId);

  if (luaOnStoreThread(ctx)) [[likely]] {
    luaPushRegister(L,
                    alias ? ctx->store->getDatasetRawByAliasInterned(alias)
                          : ctx->store->getDatasetRaw(uniqueId));
    return 1;
  }

  const QString aliasStr = alias ? QString::fromUtf8(alias) : QString();

  bool found = false;
  DataModel::RegisterValue value;
  DataModel::readTableView(*ctx, [&](const auto& view) {
    const auto* val = alias ? view.getDatasetRawByAlias(aliasStr) : view.getDatasetRaw(uniqueId);
    if (val) {
      value = *val;
      found = true;
    }
  });

  luaPushRegister(L, found ? &value : nullptr);
  return 1;
}

/**
 * @brief Lua C closure for datasetGetFinal(uniqueIdOrAlias).
 */
static int luaDatasetGetFinal(lua_State* L)
{
  auto* ctx = luaTableContext(L);
  SS_ASSERT(ctx && ctx->store, {
    lua_pushnil(L);
    return 1;
  });

  int uniqueId      = -1;
  const char* alias = luaDatasetSelector(L, &uniqueId);

  if (luaOnStoreThread(ctx)) [[likely]] {
    luaPushRegister(L,
                    alias ? ctx->store->getDatasetFinalByAliasInterned(alias)
                          : ctx->store->getDatasetFinal(uniqueId));
    return 1;
  }

  const QString aliasStr = alias ? QString::fromUtf8(alias) : QString();

  bool found = false;
  DataModel::RegisterValue value;
  DataModel::readTableView(*ctx, [&](const auto& view) {
    const auto* val =
      alias ? view.getDatasetFinalByAlias(aliasStr) : view.getDatasetFinal(uniqueId);
    if (val) {
      value = *val;
      found = true;
    }
  });

  luaPushRegister(L, found ? &value : nullptr);
  return 1;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Lua C function for mqttPublish(topic, payload, qos?, retain?).
 */
static int luaMqttPublish(lua_State* L)
{
  const char* topic = luaL_checkstring(L, 1);

  size_t len            = 0;
  const char* payload_d = luaL_checklstring(L, 2, &len);

  int qos = 0;
  if (lua_gettop(L) >= 3 && !lua_isnil(L, 3))
    qos = static_cast<int>(luaL_checkinteger(L, 3));

  bool retain = false;
  if (lua_gettop(L) >= 4 && !lua_isnil(L, 4))
    retain = lua_toboolean(L, 4) != 0;

  static auto& publisher = MQTT::Publisher::instance();

  const auto id = publisher.mqttPublish(
    QString::fromUtf8(topic), QByteArray(payload_d, static_cast<qsizetype>(len)), qos, retain);

  lua_pushinteger(L, static_cast<lua_Integer>(id));
  return 1;
}
#endif

//--------------------------------------------------------------------------------------------------
// Construction & injection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the bridge to the store, the QObject whose thread owns it, and the GUI-side mirror
 *        slot the routing rule serves GUI readers from. All three are frame-builder members, so
 *        their addresses outlive every closure that captures the context.
 */
DataModel::TableScriptBridge::TableScriptBridge(QObject& owner,
                                                DataModel::DataTableStore& store,
                                                const DataModel::DataTableSnapshotPtr& guiMirror)
  : m_store(store)
{
  m_context.store  = &m_store;
  m_context.owner  = &owner;
  m_context.mirror = &guiMirror;
}

/**
 * @brief Injects tableGet / tableSet / datasetGetRaw / datasetGetFinal into the Lua state as C
 * closures.
 */
void DataModel::TableScriptBridge::installLua(lua_State* L)
{
  SS_ASSERT(L, return);
  SS_ASSERT_LOG(m_context.owner != nullptr);

  lua_pushlightuserdata(L, &m_context);
  lua_pushcclosure(L, luaTableGet, 1);
  lua_setglobal(L, "tableGet");

  lua_pushlightuserdata(L, &m_context);
  lua_pushcclosure(L, luaTableSet, 1);
  lua_setglobal(L, "tableSet");

  lua_pushlightuserdata(L, &m_context);
  lua_pushcclosure(L, luaTableHandle, 1);
  lua_setglobal(L, "tableHandle");

  lua_pushlightuserdata(L, &m_context);
  lua_pushcclosure(L, luaTableHandleMany, 1);
  lua_setglobal(L, "tableHandleMany");

  lua_pushlightuserdata(L, &m_context);
  lua_pushcclosure(L, luaTableGetH, 1);
  lua_setglobal(L, "tableGetH");

  lua_pushlightuserdata(L, &m_context);
  lua_pushcclosure(L, luaTableSetH, 1);
  lua_setglobal(L, "tableSetH");

  lua_pushlightuserdata(L, &m_context);
  lua_pushcclosure(L, luaDatasetGetRaw, 1);
  lua_setglobal(L, "datasetGetRaw");

  lua_pushlightuserdata(L, &m_context);
  lua_pushcclosure(L, luaDatasetGetFinal, 1);
  lua_setglobal(L, "datasetGetFinal");

#ifdef BUILD_COMMERCIAL
  lua_pushcfunction(L, luaMqttPublish);
  lua_setglobal(L, "mqttPublish");
#endif
}

/**
 * @brief Installs the __ss table-API bridge; the SDK prelude exposes the friendly globals.
 */
void DataModel::TableScriptBridge::installJs(QJSEngine* js)
{
  SS_ASSERT(js, return);
  SS_ASSERT_LOG(m_context.owner != nullptr);

  auto* bridge    = new DataModel::TableApiBridge(js);
  bridge->context = m_context;

  auto global    = js->globalObject();
  auto bridgeVal = js->newQObject(bridge);
  global.setProperty(QStringLiteral("__ss"), bridgeVal);
}

//--------------------------------------------------------------------------------------------------
// Store initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the DataTableStore from the project model and @p frame. Must run BEFORE
 *        scripts (re)load: evaluation resolves table handles (top level or the load-time parse()
 *        probe), and a later rebuild would bump the generation and stale them all.
 */
void DataModel::TableScriptBridge::initializeStore(const DataModel::Frame& frame)
{
  IO::PipelineHost::runOnGuiThreadBlocking([this, &frame] {
    static auto& pm = DataModel::ProjectModel::instance();
    m_store.initialize(pm.tables(), pm.editorTableFolders(), frame);
  });
}

/**
 * @brief Re-initializes the DataTableStore from the project model's in-flight edits, against a
 *        scratch frame built from the editor's current groups rather than the runtime frame.
 */
void DataModel::TableScriptBridge::refreshStoreFromProject()
{
  IO::PipelineHost::runOnGuiThreadBlocking([this] {
    static auto& pm = DataModel::ProjectModel::instance();
    DataModel::Frame scratch;
    scratch.title  = pm.title();
    scratch.groups = pm.groups();
    m_store.initialize(pm.tables(), pm.editorTableFolders(), scratch);
  });
}
