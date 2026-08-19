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

#include "IO/StreamWorker.h"

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
}
// clang-format on

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <QDebug>
#include <QScopedValueRollback>

#include "DataModel/FrameBuilder.h"
#include "DataModel/HotpathOptimization.h"
#include "DataModel/Scripting/LuaCompat.h"
#include "DataModel/Scripting/LuaCompatJIT.h"
#include "DSPSimd.h"
#include "SerialStudio.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Lua sandbox helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the safe Lua libraries for stream transforms and strips the load/dump escape
 *        hatches; ffi and jit are never opened in either mode (sandbox escape, spec 0051).
 */
static void openSafeStreamLibs(lua_State* L)
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

//--------------------------------------------------------------------------------------------------
// StreamProcessor: construction & engine lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the worker-affine processor: per-channel state, reused float64 scratch and
 *        FFT rings sized from the configuration. Script engines compile later on the worker
 *        thread (compileEngines), never here.
 */
IO::StreamProcessor::StreamProcessor(const StreamConfig& config,
                                     std::atomic<bool>* paused,
                                     DataModel::FrameBuilder* frameBuilder)
  : m_config(config)
  , m_paused(paused)
  , m_frameBuilder(frameBuilder)
  , m_lua(nullptr)
  , m_js(nullptr)
  , m_luaDeadline(QDeadlineTimer::Forever)
  , m_inBlock(false)
  , m_observedChannels(0)
  , m_samplesProcessed(0)
  , m_blocksProcessed(0)
  , m_transformErrors(0)
  , m_displayDrops(0)
  , m_exprSlots()
  , m_hasExpressions(false)
  , m_updatePoolHint(0)
{
  SS_ASSERT_LOG(!m_config.datasets.empty());

  m_channels.reserve(m_config.datasets.size());
  for (const auto& dataset : m_config.datasets) {
    ChannelState state;
    state.config = dataset;
    if (dataset.fft && dataset.fftSamples > 0)
      state.fftRing.resize(static_cast<std::size_t>(dataset.fftSamples), 0.0);

    m_channels.push_back(std::move(state));
  }

  m_updatePool.reserve(kBlockPoolSlots);
  for (std::size_t i = 0; i < kBlockPoolSlots; ++i)
    m_updatePool.push_back(std::make_shared<DataModel::DataBlock>());
}

/**
 * @brief Destroys the processor; engines are expected to be gone already (teardownEngines runs
 *        queued before the thread quits), but a defensive teardown keeps leak-free shutdown on
 *        paths that never compiled.
 */
IO::StreamProcessor::~StreamProcessor()
{
  teardownEngines();
}

/**
 * @brief Lua LUA_MASKCOUNT hook aborting a runaway Safe-mode transform when the deadline expires.
 */
void IO::StreamProcessor::luaWatchdogHook(lua_State* L, lua_Debug* ar)
{
  Q_UNUSED(ar)

  lua_getfield(L, LUA_REGISTRYINDEX, "__ss_stream__");
  auto* self = static_cast<StreamProcessor*>(lua_touserdata(L, -1));
  lua_pop(L, 1);

  if (!self) [[unlikely]]
    return;

  if (self->m_luaDeadline.hasExpired()) [[unlikely]]
    luaL_error(L, "stream transform timed out after %d ms", kWatchdogMs);
}

/**
 * @brief Creates the sandboxed Lua state on the worker thread with the project's Safe/Fast mode
 *        applied: Safe = interpreter + count hook, Fast = JIT + no hook (one mode, spec 0051 R20).
 *        The shared data-table closures are injected for parity with frame-lane transforms; from
 *        this thread they route through the readTableView/writeTableStore marshal (spec 0051 M5).
 */
void IO::StreamProcessor::setupLuaState()
{
  lua_State* L = luaL_newstate();
  if (!L) [[unlikely]]
    return;

  lua_atpanic(L, [](lua_State* state) -> int {
    const char* msg = lua_tostring(state, -1);
    qWarning() << "[StreamWorker] Lua panic:" << (msg ? msg : "<unknown>");
    throw std::runtime_error(msg ? msg : "stream lua panic");
  });

  const auto bootstrap = [](lua_State* state) -> int {
    auto* self = static_cast<StreamProcessor*>(lua_touserdata(state, 1));
    openSafeStreamLibs(state);
    DataModel::installLuaCompat(state);
    lua_pushlightuserdata(state, self);
    lua_setfield(state, LUA_REGISTRYINDEX, "__ss_stream__");
    return 0;
  };

  lua_pushcfunction(L, bootstrap);
  lua_pushlightuserdata(L, this);
  if (lua_pcall(L, 1, 0, 0) != LUA_OK) [[unlikely]] {
    qWarning() << "[StreamWorker] Lua bootstrap failed:" << lua_tostring(L, -1);
    lua_close(L);
    return;
  }

  if (m_frameBuilder)
    m_frameBuilder->injectTableApiLua(L);

  if (m_config.luaFastMode) {
    luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);
  } else {
    luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);
    lua_sethook(L, &StreamProcessor::luaWatchdogHook, LUA_MASKCOUNT, kHookInstrCount);
  }

  m_lua = L;
}

/**
 * @brief Creates the worker-owned QJSEngine for JavaScript stream transforms. The __ss table-API
 *        bridge is installed for parity with frame-lane transforms, and the friendly globals are
 *        defined over it exactly as the SDK prelude (app/rcc/api/prelude.js) does.
 */
void IO::StreamProcessor::setupJsEngine()
{
  m_js = new QJSEngine();
  if (!m_frameBuilder)
    return;

  m_frameBuilder->injectTableApiJS(m_js);

  static const QString kTablePrelude =
    QStringLiteral("if (typeof __ss !== 'undefined') {"
                   " tableGet = function(t, r) { return __ss.tableGet(t, r); };"
                   " tableSet = function(t, r, v) { __ss.tableSet(t, r, v); };"
                   " tableHandle = function(t, r) { return __ss.tableHandle(t, r); };"
                   " tableHandleMany = function(t, regs) { return __ss.tableHandleMany(t, regs); };"
                   " tableGetH = function(h) { return __ss.tableGetH(h); };"
                   " tableSetH = function(h, v) { __ss.tableSetH(h, v); };"
                   " datasetGetRaw = function(u) { return __ss.datasetGetRaw(u); };"
                   " datasetGetFinal = function(u) { return __ss.datasetGetFinal(u); };"
                   " if (__ss.mqttPublish)"
                   "  mqttPublish = function(t, p, q, r) { return __ss.mqttPublish(t, p, q, r); };"
                   "}");
  (void)m_js->evaluate(kTablePrelude);
}

/**
 * @brief Compiles every configured transform on the worker thread, resolving both forms per
 *        dataset: transform_block(samples, info) preferred, transform(value) fallback (R9).
 */
void IO::StreamProcessor::compileEngines()
{
  m_hasExpressions = false;
  for (auto& state : m_channels) {
    if (state.config.transformCode.isEmpty())
      continue;

    const int language = state.config.transformLanguage;
    if (language == SerialStudio::Lua)
      compileLuaEntry(state);

    if (language == SerialStudio::JavaScript)
      compileJsEntry(state);

    if (language == SerialStudio::Expression)
      compileExprEntry(state);
  }
}

/**
 * @brief Compiles one dataset's expression transform (spec 0060) against this source's channel
 *        names; a compile error counts once and leaves the channel raw.
 */
void IO::StreamProcessor::compileExprEntry(ChannelState& state)
{
  const DataModel::Expression::NameResolver resolver = [this](QStringView name) -> int {
    for (const auto& channel : m_config.datasets)
      if (!channel.alias.isEmpty() && QStringView(channel.alias) == name)
        return m_exprSlots.slotFor(channel.uniqueId);

    bool ok               = false;
    const int resolved_id = name.toInt(&ok);
    if (ok)
      for (const auto& channel : m_config.datasets)
        if (channel.uniqueId == resolved_id)
          return m_exprSlots.slotFor(channel.uniqueId);

    return -1;
  };

  QString error;
  state.expr.reset();
  state.exprValid =
    DataModel::Expression::compile(state.config.transformCode, resolver, state.expr.program, error);
  if (!state.exprValid) {
    ++m_transformErrors;
    qWarning() << "[StreamProcessor] expression transform rejected for dataset"
               << state.config.uniqueId << ":" << error;
    return;
  }

  m_hasExpressions = true;
}

/**
 * @brief Compiles one dataset's Lua transform chunk into an isolated environment and refs both
 *        forms (transform_block preferred, transform fallback).
 */
void IO::StreamProcessor::compileLuaEntry(ChannelState& state)
{
  if (!m_lua)
    setupLuaState();

  if (!m_lua) [[unlikely]]
    return;

  lua_State* L          = m_lua;
  const QByteArray utf8 = state.config.transformCode.toUtf8();
  const QByteArray name = QByteArray("=stream[") + QByteArray::number(state.config.uniqueId) + "]";
  lua_newtable(L);
  lua_createtable(L, 0, 1);
  lua_pushglobaltable(L);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);

  if (luaL_loadbufferx(L, utf8.constData(), utf8.size(), name.constData(), "t") != LUA_OK) {
    qWarning() << "[StreamWorker] transform compile error for dataset" << state.config.uniqueId
               << ":" << lua_tostring(L, -1);
    lua_pop(L, 2);
    return;
  }

  lua_pushvalue(L, -2);
  luacompatSetChunkEnv(L);
  if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
    qWarning() << "[StreamWorker] transform runtime error for dataset" << state.config.uniqueId
               << ":" << lua_tostring(L, -1);
    lua_pop(L, 2);
    return;
  }

  lua_getfield(L, -1, "transform_block");
  if (lua_isfunction(L, -1))
    state.luaBlockRef = luaL_ref(L, LUA_REGISTRYINDEX);
  else
    lua_pop(L, 1);

  lua_getfield(L, -1, "transform");
  if (lua_isfunction(L, -1))
    state.luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
  else
    lua_pop(L, 1);

  lua_pop(L, 1);
}

/**
 * @brief Compiles one dataset's JS transform via an IIFE wrapper resolving both forms.
 */
void IO::StreamProcessor::compileJsEntry(ChannelState& state)
{
  if (!m_js)
    setupJsEngine();

  const QString wrapped =
    QStringLiteral("(function() {%1\n"
                   ";return { block: (typeof transform_block === 'function') ? "
                   "transform_block : null, sample: (typeof transform === 'function') ? "
                   "transform : null };\n"
                   "})();")
      .arg(state.config.transformCode);

  auto evalResult = m_js->evaluate(wrapped);
  if (evalResult.isError()) {
    qWarning() << "[StreamWorker] JS transform compile error for dataset" << state.config.uniqueId
               << ":" << evalResult.property(QStringLiteral("message")).toString();
    return;
  }

  state.jsBlockFn    = evalResult.property(QStringLiteral("block"));
  state.jsFn         = evalResult.property(QStringLiteral("sample"));
  state.jsBlockValid = state.jsBlockFn.isCallable();
  state.jsValid      = state.jsFn.isCallable();
}

/**
 * @brief Destroys the script engines on the worker thread (queued ahead of thread quit) so Lua
 *        states and QJSEngines die on the thread that owns them.
 */
void IO::StreamProcessor::teardownEngines()
{
  for (auto& state : m_channels) {
    if (m_lua) {
      if (state.luaRef >= 0)
        luaL_unref(m_lua, LUA_REGISTRYINDEX, state.luaRef);

      if (state.luaBlockRef >= 0)
        luaL_unref(m_lua, LUA_REGISTRYINDEX, state.luaBlockRef);
    }

    state.luaRef       = -1;
    state.luaBlockRef  = -1;
    state.jsValid      = false;
    state.jsBlockValid = false;
    state.jsFn         = QJSValue();
    state.jsBlockFn    = QJSValue();
  }

  if (m_lua) {
    lua_close(m_lua);
    m_lua = nullptr;
  }

  delete m_js;
  m_js = nullptr;
}

//--------------------------------------------------------------------------------------------------
// StreamProcessor: block processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Consumes one typed sample block: extract each channel into the reused scratch, transform,
 *        then publish samples + FFT + latest as one update. A re-entrant block is dropped and
 *        counted, and samples is clamped to the shortest column so a channel the source does not
 *        carry cannot be read past its end.
 */
void IO::StreamProcessor::onSampleBlock(const IO::SampleBlockPtr& block)
{
  SS_ASSERT(block != nullptr, return);
  SS_ASSERT(block->channels > 0, return);

  if (block->frames <= 0 || block->samples.empty()) [[unlikely]]
    return;

  if (m_paused && m_paused->load(std::memory_order_relaxed)) [[unlikely]]
    return;

  if (m_inBlock) [[unlikely]] {
    ++m_displayDrops;
    return;
  }

  const QScopedValueRollback<bool> reentry_guard(m_inBlock, true);
  const quint64 blockNumber = ++m_blocksProcessed;
  m_observedChannels        = block->channels;

  const auto slot = claimBlockSlot();
  if (!slot) [[unlikely]] {
    ++m_displayDrops;
    return;
  }

  auto& out               = *slot;
  out.sourceId            = m_config.sourceId;
  out.blockNumber         = blockNumber;
  out.structureGeneration = 0;
  out.t0                  = block->t0;
  out.dt                  = block->dt;
  out.samples             = block->frames;
  out.times.clear();

  if (out.columns.size() != m_channels.size()) [[unlikely]]
    bindBlockColumns(out);

  for (std::size_t i = 0; i < m_channels.size(); ++i)
    if (!m_channels[i].exprValid)
      processChannel(m_channels[i], *block, blockNumber, &out.columns[i]);

  if (m_hasExpressions) [[unlikely]]
    processExpressionChannels(*block, out);

  for (const auto& column : out.columns)
    out.samples = std::min(out.samples, static_cast<qsizetype>(column.values.size()));

  m_samplesProcessed += static_cast<quint64>(out.samples);

  Q_EMIT blockReady(DataModel::DataBlockPtr(slot, slot.get()));

  QList<QPair<int, double>> latest;
  latest.reserve(static_cast<qsizetype>(m_channels.size()));
  for (const auto& state : m_channels)
    latest.append({state.config.uniqueId, state.latest});

  Q_EMIT latestValuesReady(m_config.sourceId, latest);
}

/**
 * @brief Lays out one pooled block's columns from the configured datasets. Dense columns are
 *        numeric-only (spec 0055 D2), so they allocate no text or per-sample numeric storage.
 */
void IO::StreamProcessor::bindBlockColumns(DataModel::DataBlock& block) const
{
  block.columns.clear();
  block.columns.reserve(m_channels.size());
  for (const auto& state : m_channels) {
    DataModel::BlockColumn column;
    column.uniqueId = state.config.uniqueId;
    column.hasText  = false;
    column.values.resize(static_cast<std::size_t>(kStreamBlockSampleCap));
    block.columns.push_back(std::move(column));
  }
}

/**
 * @brief Runs one dataset's transform for the block, then fills its display payload and export
 *        column. A null @p channel means the display slot pool was exhausted: the transform and
 *        the export still run, only this block's trace points are lost.
 */
void IO::StreamProcessor::processChannel(ChannelState& state,
                                         const IO::SampleBlock& block,
                                         quint64 blockNumber,
                                         DataModel::BlockColumn* column)
{
  SS_ASSERT(column != nullptr, return);

  column->uniqueId = state.config.uniqueId;
  column->fftWindow.clear();

  const int index    = state.config.channel;
  const int channels = std::max(1, block.channels);
  if (index < 0 || index >= channels) [[unlikely]]
    return;

  const qsizetype frames = block.frames;
  m_scratch.resize(static_cast<std::size_t>(frames));

  DSP::simdDeinterleaveToF64(
    block.samples.data(), static_cast<std::size_t>(frames), channels, index, m_scratch.data());

  const double t0Ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
                        block.t0.time_since_epoch())
                        .count();

  const bool hasBlockForm = (state.luaBlockRef >= 0) || state.jsBlockValid;
  if (hasBlockForm) {
    if (!runBlockTransform(state, blockNumber, t0Ms))
      ++m_transformErrors;
  } else if (state.luaRef >= 0 || state.jsValid) {
    runSampleTransform(state);
  }

  state.firstSampleIndex += static_cast<quint64>(frames);

  if (!m_scratch.empty()) [[likely]] {
    state.latest = m_scratch.back();
    if (!state.fftRing.empty())
      appendFftRing(state, m_scratch.data(), m_scratch.size());
  }

  publishChannel(state, *column);
}

/**
 * @brief Calls transform_block(samples, info) once with the scratch samples; the returned array
 *        replaces the samples, a failed or hung call falls the block back to raw (R10). The
 *        info payload carries the frozen R9 field set.
 */
bool IO::StreamProcessor::runBlockTransform(ChannelState& state, quint64 blockNumber, double t0Ms)
{
  if (state.luaBlockRef >= 0 && m_lua)
    return runLuaBlockTransform(state, blockNumber, t0Ms);

  if (state.jsBlockValid && m_js)
    return runJsBlockTransform(state, blockNumber, t0Ms);

  return false;
}

/**
 * @brief Copies the numeric entries of the Lua array at stack top back into the scratch.
 */
static void readBackLuaSamples(lua_State* L, qsizetype count, std::vector<double>& scratch)
{
  for (qsizetype i = 0; i < count; ++i) {
    lua_rawgeti(L, -1, static_cast<int>(i + 1));
    if (lua_isnumber(L, -1))
      scratch[static_cast<std::size_t>(i)] = lua_tonumber(L, -1);

    lua_pop(L, 1);
  }
}

/**
 * @brief Lua half of the block dispatch: samples table + frozen info payload, one pcall under
 *        the Safe-mode deadline; the returned array overwrites the scratch in place.
 */
bool IO::StreamProcessor::runLuaBlockTransform(ChannelState& state,
                                               quint64 blockNumber,
                                               double t0Ms)
{
  const qsizetype count = static_cast<qsizetype>(m_scratch.size());
  lua_State* L          = m_lua;
  m_luaDeadline.setRemainingTime(kWatchdogMs);

  try {
    lua_rawgeti(L, LUA_REGISTRYINDEX, state.luaBlockRef);
    lua_createtable(L, static_cast<int>(count), 0);
    for (qsizetype i = 0; i < count; ++i) {
      lua_pushnumber(L, m_scratch[static_cast<std::size_t>(i)]);
      lua_rawseti(L, -2, static_cast<int>(i + 1));
    }

    lua_createtable(L, 0, 7);
    lua_pushinteger(L, m_config.sourceId);
    lua_setfield(L, -2, "sourceId");
    lua_pushinteger(L, state.config.uniqueId);
    lua_setfield(L, -2, "uniqueId");
    lua_pushinteger(L, static_cast<lua_Integer>(blockNumber));
    lua_setfield(L, -2, "blockNumber");
    lua_pushnumber(L, t0Ms);
    lua_setfield(L, -2, "timestampMs");
    lua_pushnumber(L, m_config.sampleRate);
    lua_setfield(L, -2, "sampleRate");
    lua_pushinteger(L, static_cast<lua_Integer>(count));
    lua_setfield(L, -2, "count");
    lua_pushinteger(L, static_cast<lua_Integer>(state.firstSampleIndex));
    lua_setfield(L, -2, "firstSampleIndex");

    if (lua_pcall(L, 2, 1, 0) != LUA_OK) [[unlikely]] {
      qWarning() << "[StreamWorker] transform_block failed for dataset" << state.config.uniqueId
                 << ":" << lua_tostring(L, -1);
      lua_pop(L, 1);
      m_luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);
      return false;
    }

    if (lua_istable(L, -1))
      readBackLuaSamples(L, count, m_scratch);

    lua_pop(L, 1);
    m_luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);
    return true;
  } catch (const std::exception& e) {
    qWarning() << "[StreamWorker] transform_block exception for dataset" << state.config.uniqueId
               << ":" << e.what();
    lua_settop(L, 0);
    m_luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);
    return false;
  }
}

/**
 * @brief JS half of the block dispatch: array + info object through the worker-owned QJSEngine.
 */
bool IO::StreamProcessor::runJsBlockTransform(ChannelState& state, quint64 blockNumber, double t0Ms)
{
  const qsizetype count = static_cast<qsizetype>(m_scratch.size());
  QJSValue samples      = m_js->newArray(static_cast<quint32>(count));
  for (qsizetype i = 0; i < count; ++i)
    samples.setProperty(static_cast<quint32>(i), m_scratch[static_cast<std::size_t>(i)]);

  QJSValue info = m_js->newObject();
  info.setProperty(QStringLiteral("sourceId"), m_config.sourceId);
  info.setProperty(QStringLiteral("uniqueId"), state.config.uniqueId);
  info.setProperty(QStringLiteral("blockNumber"), static_cast<double>(blockNumber));
  info.setProperty(QStringLiteral("timestampMs"), t0Ms);
  info.setProperty(QStringLiteral("sampleRate"), m_config.sampleRate);
  info.setProperty(QStringLiteral("count"), static_cast<double>(count));
  info.setProperty(QStringLiteral("firstSampleIndex"), static_cast<double>(state.firstSampleIndex));

  const QJSValue result = state.jsBlockFn.call({samples, info});
  if (result.isError()) [[unlikely]] {
    qWarning() << "[StreamWorker] JS transform_block failed for dataset" << state.config.uniqueId
               << ":" << result.toString();
    return false;
  }

  if (result.isArray()) {
    const quint32 n =
      std::min(static_cast<quint32>(count), result.property(QStringLiteral("length")).toUInt());
    for (quint32 i = 0; i < n; ++i)
      m_scratch[i] = result.property(i).toNumber();
  }

  return true;
}

/**
 * @brief Per-sample fallback: transform(value) applied to every scratch sample at full rate on
 *        this thread (the existing per-sample contract, R9).
 */
void IO::StreamProcessor::runSampleTransform(ChannelState& state)
{
  const std::size_t count = m_scratch.size();

  if (state.luaRef >= 0 && m_lua) {
    lua_State* L = m_lua;
    m_luaDeadline.setRemainingTime(kWatchdogMs);
    try {
      SS_NO_UNROLL
      for (std::size_t i = 0; i < count; ++i) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, state.luaRef);
        lua_pushnumber(L, m_scratch[i]);
        if (lua_pcall(L, 1, 1, 0) != LUA_OK) [[unlikely]] {
          ++m_transformErrors;
          lua_pop(L, 1);
          break;
        }

        if (lua_isnumber(L, -1))
          m_scratch[i] = lua_tonumber(L, -1);

        lua_pop(L, 1);
      }
    } catch (const std::exception&) {
      ++m_transformErrors;
      lua_settop(L, 0);
    }

    m_luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);
    return;
  }

  if (state.jsValid) {
    SS_NO_UNROLL
    for (std::size_t i = 0; i < count; ++i) {
      const QJSValue result = state.jsFn.call({QJSValue(m_scratch[i])});
      if (result.isError()) [[unlikely]] {
        ++m_transformErrors;
        break;
      }

      if (result.isNumber())
        m_scratch[i] = result.toNumber();
    }
  }
}

/**
 * @brief Copies the transformed block into the channel's column: the samples exactly as the source
 *        produced them, on the block's own uniform grid. Nothing is reduced here -- the widget
 *        decimates the visible window at screen resolution when it draws, and a GUI that cannot
 *        keep up drops whole blocks through the pool, which is counted.
 */
void IO::StreamProcessor::publishChannel(ChannelState& state, DataModel::BlockColumn& column)
{
  const std::size_t count = m_scratch.size();
  if (count == 0) [[unlikely]]
    return;

  if (column.values.size() < count) [[unlikely]]
    column.values.resize(count);

  std::copy_n(m_scratch.data(), count, column.values.data());
  publishFftWindow(state, column);
}

/**
 * @brief Copies a filled FFT ring into the column's window (two contiguous runs); nothing until
 *        the ring has filled once.
 */
void IO::StreamProcessor::publishFftWindow(const ChannelState& state,
                                           DataModel::BlockColumn& column) const
{
  if (state.fftRing.empty() || state.fftFill < state.fftRing.size())
    return;

  column.fftWindow.resize(state.fftRing.size());

  const std::size_t cap      = state.fftRing.size();
  const std::size_t firstRun = cap - state.fftHead;
  std::copy_n(state.fftRing.data() + state.fftHead, firstRun, column.fftWindow.data());
  std::copy_n(state.fftRing.data(), state.fftHead, column.fftWindow.data() + firstRun);
}

/**
 * @brief Evaluates every expression channel sample-major (spec 0060): per sample the processed
 *        channels publish their value into the SlotTable, then each expression channel
 *        evaluates its raw sample and publishes its result, so a sibling reads "the latest
 *        published value" exactly as on the frame lane. Allocation-free per sample.
 */
void IO::StreamProcessor::processExpressionChannels(const IO::SampleBlock& block,
                                                    DataModel::DataBlock& out)
{
  SS_ASSERT(block.channels > 0, return);
  SS_ASSERT(out.columns.size() == m_channels.size(), return);

  const auto frames  = static_cast<std::size_t>(block.frames);
  const int channels = std::max(1, block.channels);
  const double t0Sec = std::chrono::duration<double>(block.t0.time_since_epoch()).count();
  const double dtSec = std::chrono::duration<double>(block.dt).count();
  const float* raw   = block.samples.data();

  for (std::size_t c = 0; c < m_channels.size(); ++c)
    if (m_channels[c].exprValid && out.columns[c].values.size() < frames)
      out.columns[c].values.resize(frames);

  for (std::size_t i = 0; i < frames; ++i) {
    const double t = t0Sec + static_cast<double>(i) * dtSec;

    for (std::size_t c = 0; c < m_channels.size(); ++c) {
      const ChannelState& state = m_channels[c];
      if (state.exprValid || i >= out.columns[c].values.size())
        continue;

      m_exprSlots.publish(state.config.uniqueId, out.columns[c].values[i]);
    }

    for (std::size_t c = 0; c < m_channels.size(); ++c) {
      ChannelState& state = m_channels[c];
      if (!state.exprValid)
        continue;

      const int index = state.config.channel;
      const double v =
        (index >= 0 && index < channels)
          ? static_cast<double>(
              raw[i * static_cast<std::size_t>(channels) + static_cast<std::size_t>(index)])
          : std::numeric_limits<double>::quiet_NaN();
      const double y           = state.expr.run(v, t, m_exprSlots);
      out.columns[c].values[i] = y;
      m_exprSlots.publish(state.config.uniqueId, y);
    }
  }

  for (std::size_t c = 0; c < m_channels.size(); ++c) {
    ChannelState& state = m_channels[c];
    if (!state.exprValid)
      continue;

    auto& column            = out.columns[c];
    column.uniqueId         = state.config.uniqueId;
    state.firstSampleIndex += static_cast<quint64>(frames);
    if (frames == 0) [[unlikely]]
      continue;

    state.latest = column.values[frames - 1];
    column.fftWindow.clear();
    if (!state.fftRing.empty())
      appendFftRing(state, column.values.data(), frames);

    publishFftWindow(state, column);
  }
}

/**
 * @brief Appends a block to a sample ring as at most two contiguous runs. A block longer than the
 *        ring keeps only its tail, which is what the per-sample overwrite it replaces left behind.
 */
static void pushSamples(std::vector<double>& ring,
                        std::size_t& head,
                        std::size_t& fill,
                        const double* samples,
                        std::size_t count)
{
  const std::size_t cap = ring.size();
  SS_ASSERT(cap > 0, return);
  SS_ASSERT(head < cap, head = 0);

  const double* src = samples;
  std::size_t start = head;
  std::size_t n     = count;
  if (n > cap) [[unlikely]] {
    src   += (n - cap);
    start  = (start + (n - cap)) % cap;
    n      = cap;
  }

  const std::size_t firstRun = std::min(n, cap - start);
  std::copy_n(src, firstRun, ring.data() + start);
  if (n > firstRun)
    std::copy_n(src + firstRun, n - firstRun, ring.data());

  head = (start + n) % cap;
  fill = std::min(cap, fill + count);
}

/**
 * @brief Appends a block to a channel's FFT ring.
 */
void IO::StreamProcessor::appendFftRing(ChannelState& state,
                                        const double* samples,
                                        std::size_t count)
{
  pushSamples(state.fftRing, state.fftHead, state.fftFill, samples, count);
}

/**
 * @brief Claims a free pooled display update, or null when every slot is in flight (the caller
 *        drops and counts, the same coalescing contract as a full ring). The use_count probe is
 *        an atomic read and the acquire fence pairs with the GUI's release of its last alias, so
 *        slot reuse happens-after every consumer read of the slot's buffers.
 */
std::shared_ptr<DataModel::DataBlock> IO::StreamProcessor::claimBlockSlot()
{
  SS_ASSERT(!m_updatePool.empty(), return nullptr);

  const std::size_t n = m_updatePool.size();
  for (std::size_t k = 0; k < n; ++k) {
    const std::size_t idx = (m_updatePoolHint + k) % n;
    if (m_updatePool[idx].use_count() != 1)
      continue;

    std::atomic_thread_fence(std::memory_order_acquire);
    m_updatePoolHint = (idx + 1) % n;
    return m_updatePool[idx];
  }

  return nullptr;
}

//--------------------------------------------------------------------------------------------------
// StreamWorker facade
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates the worker thread, moves the processor onto it, wires the driver's typed block
 *        signal (queued, block rate) and schedules engine compilation on the worker thread. A
 *        non-null @p frameBuilder grants transforms the shared data-table API; unit tests and
 *        the benchmark pass null and keep a table-free sandbox.
 */
IO::StreamWorker::StreamWorker(HAL_Driver* driver,
                               const StreamConfig& config,
                               DataModel::FrameBuilder* frameBuilder,
                               QObject* parent)
  : QObject(parent)
  , m_config(config)
  , m_abandoned(false)
  , m_thread(std::make_unique<QThread>())
  , m_processor(nullptr)
  , m_paused(false)
{
  SS_ASSERT(driver != nullptr, return);
  SS_ASSERT_LOG(!config.datasets.empty());

  m_thread->setObjectName(QStringLiteral("StreamWorker-%1").arg(config.sourceId));

  m_processor = new StreamProcessor(config, &m_paused, frameBuilder);
  m_processor->moveToThread(m_thread.get());
  m_thread->start();

  m_feed = connect(driver,
                   &IO::HAL_Driver::sampleBlockReceived,
                   m_processor,
                   &IO::StreamProcessor::onSampleBlock,
                   Qt::QueuedConnection);

  QMetaObject::invokeMethod(
    m_processor, &IO::StreamProcessor::compileEngines, Qt::QueuedConnection);
}

/**
 * @brief Stops the worker (idempotent) before destruction.
 */
IO::StreamWorker::~StreamWorker()
{
  stop();

  if (!m_abandoned)
    delete m_processor;

  m_processor = nullptr;
}

/**
 * @brief Returns the project source this worker serves.
 */
int IO::StreamWorker::sourceId() const noexcept
{
  return m_config.sourceId;
}

/**
 * @brief True when teardown abandoned a hung Fast-mode script (the thread leaks by design, R21).
 */
bool IO::StreamWorker::abandoned() const noexcept
{
  return m_abandoned;
}

/**
 * @brief Returns the immutable stream configuration.
 */
const IO::StreamConfig& IO::StreamWorker::config() const noexcept
{
  return m_config;
}

/**
 * @brief Returns the worker-affine processor (counters are plain quint64 pulled at 1 Hz).
 */
IO::StreamProcessor* IO::StreamWorker::processor() const noexcept
{
  return m_processor;
}

/**
 * @brief Mirrors the session pause into the worker (GUI-written): the processor drops incoming
 *        blocks while set, the stream-lane counterpart of PipelineHost::routeFrames' pause gate.
 */
void IO::StreamWorker::setPaused(bool paused) noexcept
{
  m_paused.store(paused, std::memory_order_relaxed);
}

/**
 * @brief Joins the worker: engine teardown queued ahead of quit, bounded wait, then
 *        warn-and-abandon on a hung Fast-mode script (R21). The abandon latch makes the
 *        destructor's repeat call free and releases the QThread, whose OS thread still runs.
 */
void IO::StreamWorker::stop()
{
  if (m_abandoned || !m_thread || !m_thread->isRunning())
    return;

  if (m_feed)
    disconnect(m_feed);

  if (m_processor)
    QMetaObject::invokeMethod(
      m_processor, &IO::StreamProcessor::teardownEngines, Qt::QueuedConnection);

  m_thread->quit();
  if (!m_thread->wait(kJoinTimeoutMs)) [[unlikely]] {
    m_abandoned = true;
    (void)m_thread.release();
    qWarning() << "[StreamWorker] source" << m_config.sourceId << "worker did not stop within"
               << kJoinTimeoutMs << "ms (hung script?) -- abandoning it (R21)";
  }
}
