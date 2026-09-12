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

#include "DataModel/FrameBuilder/TransformDispatch.h"

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
}
// clang-format on

#include <cmath>
#include <limits>
#include <QDebug>
#include <QJSValue>
#include <QJSValueList>
#include <stdexcept>

#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr double kMillisecondsToSeconds = 1.0 / 1000.0;

//--------------------------------------------------------------------------------------------------
// Engine cache
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts with no source selected; the first dataset pass resolves the engines.
 */
DataModel::TransformDispatch::TransformDispatch(TransformCompiler& transforms)
  : m_transforms(transforms)
  , m_cacheSourceId(-1)
  , m_lua(nullptr)
  , m_js(nullptr)
  , m_expr(nullptr)
  , m_jsTimedOut(false)
{}

/**
 * @brief Resolves the three engines for @p sourceId once; every dataset of the pass then reads the
 *        cached pointers instead of walking the compiler's engine map.
 */
void DataModel::TransformDispatch::select(int sourceId)
{
  SS_ASSERT(sourceId >= 0, return);

  m_cacheSourceId = sourceId;
  auto* expr      = m_transforms.engineFor(sourceId, SerialStudio::Expression);
  m_lua           = m_transforms.engineFor(sourceId, SerialStudio::Lua);
  m_js            = m_transforms.engineFor(sourceId, SerialStudio::JavaScript);
  m_expr          = (expr && expr->exprSlots) ? expr : nullptr;
}

/**
 * @brief Forgets the cached engines (the compiler is about to destroy them).
 */
void DataModel::TransformDispatch::reset() noexcept
{
  m_cacheSourceId = -1;
  m_lua           = nullptr;
  m_js            = nullptr;
  m_expr          = nullptr;
}

//--------------------------------------------------------------------------------------------------
// Dispatch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies the pre-compiled transform for a dataset; returns @p rawValue on error or missing
 * transform.
 */
QVariant DataModel::TransformDispatch::apply(int language,
                                             int uniqueId,
                                             const QVariant& rawValue,
                                             const TransformFrameInfo& info)
{
  SS_ASSERT_LOG(info.sourceId >= 0);
  SS_ASSERT_LOG(uniqueId >= 0);
  SS_ASSERT_LOG(info.sourceId == m_cacheSourceId);

  DataModel::TransformEngine* engine = nullptr;
  if (language == SerialStudio::Lua)
    engine = m_lua;
  else if (language == SerialStudio::Expression)
    engine = m_expr;
  else
    engine = m_js;

  if (!engine)
    return rawValue;

  if (engine->luaState)
    return applyLua(*engine, uniqueId, rawValue, info);

  if (engine->jsEngine)
    return applyJs(*engine, uniqueId, rawValue, info);

  if (engine->exprSlots)
    return applyExpr(*engine, uniqueId, rawValue, info);

  return rawValue;
}

/**
 * @brief Calls the cached Lua transform function for @p uniqueId under the per-call deadline.
 */
QVariant DataModel::TransformDispatch::applyLua(DataModel::TransformEngine& engine,
                                                int uniqueId,
                                                const QVariant& rawValue,
                                                const TransformFrameInfo& info)
{
  auto refIt = engine.luaRefs.find(uniqueId);
  if (refIt == engine.luaRefs.end())
    return rawValue;

  lua_State* L           = engine.luaState;
  const auto& transform  = refIt->second;
  const bool acceptsInfo = transform.acceptsInfo;
  engine.luaDeadline.setRemainingTime(kTransformWatchdogMs);

  try {
    lua_rawgeti(L, LUA_REGISTRYINDEX, transform.ref);
    if (rawValue.typeId() == QMetaType::Double) {
      lua_pushnumber(L, SerialStudio::toDouble(rawValue));
    } else {
      const auto utf8 = rawValue.toString().toUtf8();
      lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
    }

    int argCount = 1;
    if (acceptsInfo) {
      lua_createtable(L, 0, 3);
      lua_pushinteger(L, static_cast<lua_Integer>(info.frameNumber));
      lua_setfield(L, -2, "frameNumber");
      lua_pushinteger(L, info.sourceId);
      lua_setfield(L, -2, "sourceId");
      lua_pushinteger(L, static_cast<lua_Integer>(info.timestampMs));
      lua_setfield(L, -2, "timestampMs");
      argCount = 2;
    }

    int pcallStatus = LUA_ERRRUN;
    try {
      pcallStatus = lua_pcall(L, argCount, 1, 0);
    } catch (...) {
      qWarning() << "[TransformDispatch] Uncaught exception escaped lua_pcall in transform for"
                 << uniqueId;
      try {
        lua_settop(L, 0);
        lua_pushstring(L, "uncaught Lua exception (escaped lua_pcall)");
      } catch (...) {
      }
      pcallStatus = LUA_ERRRUN;
    }
    engine.luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);

    if (pcallStatus != LUA_OK) [[unlikely]] {
      qWarning() << "[TransformDispatch] Lua transform call failed for dataset" << uniqueId << ":"
                 << lua_tostring(L, -1);
      m_transforms.noteTransformError(uniqueId, lua_tostring(L, -1));
      lua_pop(L, 1);
      return rawValue;
    }

    if (lua_isnumber(L, -1)) {
      const double result = lua_tonumber(L, -1);
      lua_pop(L, 1);
      if (!std::isfinite(result)) [[unlikely]]
        return rawValue;

      return QVariant(result);
    }

    if (lua_isstring(L, -1)) {
      const QString result = QString::fromUtf8(lua_tostring(L, -1));
      lua_pop(L, 1);
      return QVariant(result);
    }

    lua_pop(L, 1);
    return rawValue;
  } catch (const std::exception& e) {
    qWarning() << "[TransformDispatch] applyTransformLua uncaught exception for" << uniqueId << ":"
               << e.what();
  } catch (...) {
    qWarning() << "[TransformDispatch] applyTransformLua uncaught non-std exception for"
               << uniqueId;
  }

  engine.luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);
  lua_settop(L, 0);
  return rawValue;
}

/**
 * @brief Evaluates the compiled expression for @p uniqueId (spec 0060): a text input enters as
 *        NaN so an arithmetic expression over a non-numeric channel degrades instead of parsing
 *        garbage, and `t` is the source's own frame timestamp, never a re-stamp here.
 */
QVariant DataModel::TransformDispatch::applyExpr(DataModel::TransformEngine& engine,
                                                 int uniqueId,
                                                 const QVariant& rawValue,
                                                 const TransformFrameInfo& info)
{
  auto refIt = engine.exprRefs.find(uniqueId);
  if (refIt == engine.exprRefs.end() || !engine.exprSlots)
    return rawValue;

  bool numeric   = false;
  const double v = SerialStudio::toDouble(rawValue, &numeric);
  const double t = static_cast<double>(info.timestampMs) * kMillisecondsToSeconds;
  return QVariant(refIt->second.run(
    numeric ? v : std::numeric_limits<double>::quiet_NaN(), t, *engine.exprSlots));
}

/**
 * @brief Calls the cached JS transform function for @p uniqueId under the watchdog timer, which is
 *        armed once per frame in beginDatasetPass rather than per call (unlike the Lua deadline).
 */
QVariant DataModel::TransformDispatch::applyJs(DataModel::TransformEngine& engine,
                                               int uniqueId,
                                               const QVariant& rawValue,
                                               const TransformFrameInfo& info)
{
  auto refIt = engine.jsRefs.find(uniqueId);
  if (refIt == engine.jsRefs.end())
    return rawValue;

  QJSValueList args;
  if (rawValue.typeId() == QMetaType::Double)
    args << QJSValue(SerialStudio::toDouble(rawValue));
  else
    args << QJSValue(rawValue.toString());

  if (refIt->second.acceptsInfo) {
    QJSValue jsInfo = engine.jsEngine->newObject();
    jsInfo.setProperty(QStringLiteral("frameNumber"),
                       QJSValue(static_cast<double>(info.frameNumber)));
    jsInfo.setProperty(QStringLiteral("sourceId"), QJSValue(info.sourceId));
    jsInfo.setProperty(QStringLiteral("timestampMs"),
                       QJSValue(static_cast<double>(info.timestampMs)));
    args << jsInfo;
  }

  auto result = refIt->second.fn.call(args);

  if (engine.jsEngine->isInterrupted()) [[unlikely]] {
    engine.jsEngine->setInterrupted(false);
    m_jsTimedOut = true;
    qWarning() << "[TransformDispatch] JS transform for dataset" << uniqueId << "timed out after"
               << kTransformWatchdogMs << "ms";
    m_transforms.noteTransformError(uniqueId, "transform timed out");
    return rawValue;
  }

  if (result.isNumber()) {
    const double val = result.toNumber();
    if (!std::isfinite(val)) [[unlikely]]
      return rawValue;

    return QVariant(val);
  }

  if (result.isString())
    return QVariant(result.toString());

  if (result.isError()) [[unlikely]] {
    const auto message = result.toString();
    qWarning() << "[TransformDispatch] JS transform call failed for dataset" << uniqueId << ":"
               << message;
    m_transforms.noteTransformError(uniqueId, message);
  }

  return rawValue;
}
