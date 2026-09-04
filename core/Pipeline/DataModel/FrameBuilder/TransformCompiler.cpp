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

#include "DataModel/FrameBuilder/TransformCompiler.h"

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
}
// clang-format on

#include <limits>
#include <QDebug>
#include <QHash>
#include <QSet>
#include <stdexcept>
#include <utility>

#include "Core/SSAssert.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/DashboardApi.h"
#include "DataModel/Scripting/DeviceWriteApi.h"
#include "DataModel/Scripting/LuaCompat.h"
#include "DataModel/Scripting/LuaCompatJIT.h"
#include "DataModel/Scripting/LuaDeadlineHook.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constants & file-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the safe Lua libraries needed by transforms and strips dangerous globals, including
 *        string.dump whose bytecode serialization paired with a loader is a sandbox-escape vector.
 *        LuaJIT ships coroutine inside base and has no utf8 module; bit is its native bitwise
 *        library. ffi and jit are never opened: sandbox escape.
 */
static void openSafeLibsForTransform(lua_State* L)
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

  for (const char* name : {"dofile", "loadfile", "load"}) {
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
 * @brief Compile-time arity probe for the transform at stack top: LuaJIT's public lua_Debug
 *        carries no nparams field, so the count comes from debug.getinfo (whose library-side
 *        path fills the extended record). The debug module is loaded unpublished and the
 *        global luaopen_debug registers is nilled out, so the sandbox never gains it.
 */
[[nodiscard]] static bool luaTransformAcceptsInfo(lua_State* L)
{
  bool accepts = false;

  luaL_requiref(L, LUA_DBLIBNAME, luaopen_debug, 0);
  lua_getfield(L, -1, "getinfo");
  lua_pushvalue(L, -3);
  lua_pushliteral(L, "u");
  if (lua_pcall(L, 2, 1, 0) == LUA_OK && lua_istable(L, -1)) {
    lua_getfield(L, -1, "nparams");
    accepts = lua_tointeger(L, -1) >= 2;
    lua_pop(L, 1);
  }

  lua_pop(L, 2);
  lua_pushnil(L);
  lua_setglobal(L, "debug");
  return accepts;
}

/**
 * @brief Reads one data-table register for a compiled expression. The handle was resolved at
 *        compile time, so this is an index lookup; a missing or non-numeric register reads as
 *        NaN, which the transform pipeline already treats as "keep the raw value".
 */
static double expressionTableValue(const void* owner, qint64 handle)
{
  const auto* store = static_cast<const DataModel::DataTableStore*>(owner);
  if (!store) [[unlikely]]
    return std::numeric_limits<double>::quiet_NaN();

  const auto* value = store->getByHandle(handle);
  if (!value || !value->isNumeric) [[unlikely]]
    return std::numeric_limits<double>::quiet_NaN();

  return value->numericValue;
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the compiler to the frame it reads dataset transforms from, the store expression
 *        handles resolve against, and the installer that publishes the table API into a new Lua
 *        sandbox (the frame builder's own injector, so the cached capture flags stay armed).
 */
DataModel::TransformCompiler::TransformCompiler(const DataModel::Frame& frame,
                                                DataModel::DataTableStore& store,
                                                LuaApiInstaller installLuaTableApi)
  : m_frame(frame)
  , m_store(store)
  , m_installLuaTableApi(std::move(installLuaTableApi))
  , m_transformErrors(0)
  , m_lastTransformDatasetUniqueId(-1)
  , m_lastTransformError()
{
  SS_ASSERT_LOG(m_installLuaTableApi != nullptr);
}

/**
 * @brief Releases any engine still alive. A no-op on every ordinary path: the frame builder tears
 *        the engines down on their own thread in prepareShutdown(), long before this runs.
 */
DataModel::TransformCompiler::~TransformCompiler()
{
  destroy();
}

//--------------------------------------------------------------------------------------------------
// Engine lookup & diagnostics
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when any engine is a script interpreter, i.e. can read dataset values back out of
 *        the store. Compiled-expression engines read their own slot table and never the store.
 */
bool DataModel::TransformCompiler::hasScriptEngines() const noexcept
{
  bool script_engines = false;
  for (const auto& [key, engine] : m_engines)
    script_engines = script_engines || engine.luaState != nullptr || engine.jsEngine != nullptr;

  return script_engines;
}

/**
 * @brief Returns the engine compiled for (@p sourceId, @p language), or nullptr. Called only when
 *        the dataset pass changes source, never per dataset.
 */
DataModel::TransformEngine* DataModel::TransformCompiler::engineFor(int sourceId,
                                                                    int language) noexcept
{
  SS_ASSERT(sourceId >= 0, return nullptr);

  const auto it = m_engines.find({sourceId, language});
  return (it != m_engines.end()) ? &it->second : nullptr;
}

/**
 * @brief Counts a transform failure and retains its message only when the failing dataset differs
 *        from the one already recorded, so a dataset that throws on every frame stores the string
 *        once instead of allocating per frame.
 */
SS_COLD void DataModel::TransformCompiler::noteTransformError(int uniqueId, const char* message)
{
  ++m_transformErrors;
  if (m_lastTransformDatasetUniqueId == uniqueId)
    return;

  m_lastTransformDatasetUniqueId = uniqueId;
  m_lastTransformError           = QString::fromUtf8(message ? message : "");
}

/**
 * @brief Overload for the JavaScript branch, whose message string is already materialized.
 */
SS_COLD void DataModel::TransformCompiler::noteTransformError(int uniqueId, const QString& message)
{
  ++m_transformErrors;
  if (m_lastTransformDatasetUniqueId == uniqueId)
    return;

  m_lastTransformDatasetUniqueId = uniqueId;
  m_lastTransformError           = message;
}

//--------------------------------------------------------------------------------------------------
// Compilation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Compiles per-dataset transforms into one shared engine per (source, language): Lua, JS or
 *        the compiled-expression evaluator. The caller owns the deferral rules -- the frame builder
 *        never calls this under a dataset pass, while a player is open, or after aboutToQuit --
 *        because mutating the engine map under a pass dangles the pointers it cached.
 */
void DataModel::TransformCompiler::compile()
{
  destroy();
  SS_ASSERT_LOG(m_engines.empty());

  std::map<EngineKey, std::vector<TransformEntry>> byKey;
  for (const auto& group : m_frame.groups) {
    for (const auto& ds : group.datasets) {
      if (ds.transformCode.isEmpty())
        continue;

      byKey[{ds.sourceId, ds.transformLanguage}].push_back({ds.uniqueId, ds.transformCode});
    }
  }

  if (byKey.empty())
    return;

  for (auto& [key, entries] : byKey) {
    auto [it, inserted] = m_engines.emplace(key, TransformEngine{});
    SS_ASSERT_LOG(inserted);
    if (!inserted) [[unlikely]]
      continue;

    TransformEngine& engine = it->second;

    if (key.language == SerialStudio::Lua)
      compileLua(engine, key.sourceId, entries);
    else if (key.language == SerialStudio::Expression)
      compileExpr(engine, key.sourceId, entries);
    else
      compileJs(engine, key.sourceId, entries);

    if (!engine.luaState && !engine.jsEngine && !engine.exprSlots)
      m_engines.erase(it);
  }
}

/**
 * @brief Compiles per-dataset Lua transforms into a shared lua_State, caching refs for O(1) hotpath
 * lookup.
 */
void DataModel::TransformCompiler::compileLua(TransformEngine& engine,
                                              int sourceId,
                                              const std::vector<TransformEntry>& entries)
{
  lua_State* L = luaL_newstate();
  if (!L) [[unlikely]]
    return;

  lua_atpanic(L, [](lua_State* state) -> int {
    const char* msg = lua_tostring(state, -1);
    qWarning() << "[FrameBuilder] Lua transform panic:" << (msg ? msg : "<unknown>");
    throw std::runtime_error(msg ? msg : "lua transform panic");
  });

  struct BootstrapCtx {
    TransformCompiler* self;
    TransformEngine* engine;
    int sourceId;
  };

  const auto bootstrap = [](lua_State* state) -> int {
    auto* ctx = static_cast<BootstrapCtx*>(lua_touserdata(state, 1));

    openSafeLibsForTransform(state);
    DataModel::installLuaConsole(state);
    DataModel::installLuaCompat(state);
    ctx->self->m_installLuaTableApi(state);
    DataModel::DeviceWriteApi::installLua(state, ctx->sourceId);
    DataModel::ActionFireApi::installLua(state);
    DataModel::DashboardApi::installLua(state);
    DataModel::ScriptApiCall::installLua(state, ctx->sourceId);
    DataModel::NotificationCenter::installScriptApi(state);

    DataModel::LuaDeadlineHook::bind(
      state, &ctx->engine->luaDeadline, DataModel::kTransformWatchdogMs, "transform");
    return 0;
  };

  BootstrapCtx ctx{this, &engine, sourceId};
  lua_pushcfunction(L, bootstrap);
  lua_pushlightuserdata(L, &ctx);
  if (lua_pcall(L, 1, 0, 0) != LUA_OK) [[unlikely]] {
    qWarning() << "[FrameBuilder] Transform engine bootstrap failed for source" << sourceId << ":"
               << lua_tostring(L, -1);
    lua_close(L);
    return;
  }

  static auto& projectModel = DataModel::ProjectModel::instance();
  if (projectModel.luaFastMode()) {
    luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);
  } else {
    luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);
    LuaDeadlineHook::enable(L);
  }

  engine.luaDeadline.setRemainingTime(kTransformWatchdogMs);

  for (const auto& entry : entries)
    compileLuaEntry(L, engine, entry);

  engine.luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);
  engine.luaState    = L;
}

/**
 * @brief Compiles a single Lua dataset transform; logs and skips on any error.
 */
void DataModel::TransformCompiler::compileLuaEntry(lua_State* L,
                                                   TransformEngine& engine,
                                                   const TransformEntry& entry)
{
  const int baseTop = lua_gettop(L);

  try {
    lua_newtable(L);
    lua_createtable(L, 0, 1);
    lua_pushglobaltable(L);
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);

    const QByteArray utf8 = entry.code.toUtf8();
    const QByteArray chunkName =
      QByteArray("=transform[") + QByteArray::number(entry.uniqueId) + "]";
    if (luaL_loadbufferx(L, utf8.constData(), utf8.size(), chunkName.constData(), "t") != LUA_OK) {
      qWarning() << "[FrameBuilder] Transform compile error for dataset" << entry.uniqueId << ":"
                 << lua_tostring(L, -1);
      lua_settop(L, baseTop);
      return;
    }

    lua_pushvalue(L, -2);
    luacompatSetChunkEnv(L);

    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
      qWarning() << "[FrameBuilder] Transform runtime error for dataset" << entry.uniqueId << ":"
                 << lua_tostring(L, -1);
      lua_settop(L, baseTop);
      return;
    }

    lua_getfield(L, -1, "transform");
    if (!lua_isfunction(L, -1)) {
      qWarning() << "[FrameBuilder] Dataset" << entry.uniqueId
                 << "transform code does not define transform()";
      lua_settop(L, baseTop);
      return;
    }

    const bool acceptsInfo = luaTransformAcceptsInfo(L);

    auto existingIt = engine.luaRefs.find(entry.uniqueId);
    if (existingIt != engine.luaRefs.end()) [[unlikely]]
      luaL_unref(L, LUA_REGISTRYINDEX, existingIt->second.ref);

    engine.luaRefs[entry.uniqueId] = LuaTransformRef{luaL_ref(L, LUA_REGISTRYINDEX), acceptsInfo};

    lua_pop(L, 1);
    SS_ASSERT(lua_gettop(L) == baseTop, lua_settop(L, baseTop));
  } catch (const std::exception& e) {
    qWarning() << "[FrameBuilder] Transform compile uncaught exception for dataset"
               << entry.uniqueId << ":" << e.what();
    lua_settop(L, baseTop);
  } catch (...) {
    qWarning() << "[FrameBuilder] Transform compile uncaught non-std exception for dataset"
               << entry.uniqueId;
    lua_settop(L, baseTop);
  }
}

/**
 * @brief Compiles per-dataset expression transforms (spec 0060) against the sibling aliases and
 *        dataset ids of @p sourceId, into one SlotTable shared by every program of that source,
 *        plus read-only table handles through the store this thread owns.
 */
void DataModel::TransformCompiler::compileExpr(TransformEngine& engine,
                                               int sourceId,
                                               const std::vector<TransformEntry>& entries)
{
  auto table = std::make_unique<DataModel::Expression::SlotTable>();

  QHash<QString, int> aliases;
  QSet<int> uniqueIds;
  for (const auto& group : m_frame.groups) {
    if (group.sourceId != sourceId)
      continue;

    for (const auto& dataset : group.datasets) {
      uniqueIds.insert(dataset.uniqueId);
      if (!dataset.alias.isEmpty() && !aliases.contains(dataset.alias))
        aliases.insert(dataset.alias, dataset.uniqueId);
    }
  }

  const DataModel::Expression::NameResolver resolver =
    [&aliases, &uniqueIds, &table](QStringView name) -> int {
    const auto it = aliases.constFind(name.toString());
    if (it != aliases.cend())
      return table->slotFor(it.value());

    bool ok               = false;
    const int resolved_id = name.toInt(&ok);
    if (ok && uniqueIds.contains(resolved_id))
      return table->slotFor(resolved_id);

    return -1;
  };

  const DataModel::Expression::TableResolver tables = [this](QStringView table_name,
                                                             QStringView reg) -> qint64 {
    return m_store.handleOf(table_name.toString(), reg.toString());
  };

  for (const auto& entry : entries) {
    QString error;
    DataModel::Expression::Runtime runtime;
    runtime.tableOwner = &m_store;
    runtime.tableValue = &expressionTableValue;
    if (!DataModel::Expression::compile(entry.code, resolver, tables, runtime.program, error)) {
      ++m_transformErrors;
      qWarning() << "[FrameBuilder] Expression transform rejected for dataset" << entry.uniqueId
                 << ":" << error;
      noteTransformError(entry.uniqueId, error);
      continue;
    }

    engine.exprRefs.emplace(entry.uniqueId, std::move(runtime));
  }

  if (!engine.exprRefs.empty())
    engine.exprSlots = std::move(table);
}

/**
 * @brief Compiles per-dataset JavaScript transforms into a shared QJSEngine; code is IIFE-wrapped
 * for isolation.
 */
void DataModel::TransformCompiler::compileJs(TransformEngine& engine,
                                             int sourceId,
                                             const std::vector<TransformEntry>& entries)
{
  auto* js = new QJSEngine();

  DataModel::ScriptApiCall::installAll(js, sourceId);

  for (const auto& entry : entries) {
    const QString wrapped =
      QStringLiteral("(function() {%1\n"
                     ";return (typeof transform === 'function') ? transform : null;\n"
                     "})();")
        .arg(entry.code);

    auto evalResult = js->evaluate(wrapped);
    if (evalResult.isError()) {
      qWarning() << "[FrameBuilder] Transform compile error for"
                 << "dataset" << entry.uniqueId << "at line"
                 << evalResult.property("lineNumber").toInt() << ":"
                 << evalResult.property("message").toString();
      continue;
    }

    if (!evalResult.isCallable()) {
      qWarning() << "[FrameBuilder] Dataset" << entry.uniqueId
                 << "transform code does not define transform()";
      continue;
    }

    const bool acceptsInfo        = (evalResult.property(QStringLiteral("length")).toInt() >= 2);
    engine.jsRefs[entry.uniqueId] = JsTransformRef{evalResult, acceptsInfo};
  }

  engine.jsEngine = js;
  engine.jsWatchdog =
    std::make_unique<JsWatchdog>(js, kTransformWatchdogMs, QStringLiteral("transform"));
}

//--------------------------------------------------------------------------------------------------
// Teardown & maintenance
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs one GC pass over every per-source transform engine.
 */
void DataModel::TransformCompiler::collectGarbage()
{
  if (m_engines.empty())
    return;

  for (auto& [id, engine] : m_engines) {
    if (engine.luaState)
      lua_gc(engine.luaState, LUA_GCCOLLECT, 0);

    if (engine.jsEngine)
      engine.jsEngine->collectGarbage();
  }
}

/**
 * @brief Destroys all per-source transform engines and releases resources. The transform-error
 *        statistics reset with them, so a repaired transform stops being reported once the
 *        engines recompile. Callers run this on the thread that built the engines: a lua_State or
 *        a QJSEngine freed from another thread corrupts its own heap.
 */
void DataModel::TransformCompiler::destroy()
{
  m_transformErrors              = 0;
  m_lastTransformDatasetUniqueId = -1;
  m_lastTransformError.clear();

  for (auto& [id, engine] : m_engines) {
    engine.jsRefs.clear();
    engine.exprRefs.clear();
    engine.exprSlots.reset();

    if (engine.luaState)
      for (const auto& [uid, ref] : engine.luaRefs)
        luaL_unref(engine.luaState, LUA_REGISTRYINDEX, ref.ref);

    engine.luaRefs.clear();

    if (engine.luaState) {
      lua_close(engine.luaState);
      engine.luaState = nullptr;
    }

    engine.jsWatchdog.reset();

    delete engine.jsEngine;
    engine.jsEngine = nullptr;
  }

  m_engines.clear();
  SS_ASSERT_LOG(m_engines.empty());
}
