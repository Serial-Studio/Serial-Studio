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

#include "DataModel/Scripting/ScriptApiCall.h"

#include <lauxlib.h>
#include <lua.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <map>
#include <mutex>
#include <QByteArray>
#include <QFile>
#include <QJSEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSet>
#include <QString>
#include <QUuid>
#include <stdexcept>

#include "API/CommandHandler.h"
#include "API/CommandProtocol.h"
#include "API/CommandRegistry.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/Scripting/DashboardApi.h"
#include "DataModel/Scripting/DeviceWriteApi.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Sandbox policy: default-deny allow-list plus per-source rate limit and body cap
//--------------------------------------------------------------------------------------------------

namespace detail {
struct TokenBucket {
  double tokens;
  std::chrono::steady_clock::time_point lastRefill;
  int activeCount;
};
}  // namespace detail

static constexpr int kMaxBodyBytes    = 1 * 1024 * 1024;
static constexpr int kMaxConcurrent   = 8;
static constexpr double kTokensPerSec = 100.0;
static constexpr double kBucketSize   = 100.0;

static std::atomic<bool> s_allowFullSurface{false};

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Recursion guard: mirrors JsonValidator's 128 cap so a self-referencing table can't blow up
constexpr int kMaxJsonDepth = 64;

/**
 * @brief Command families callable from local user scripts without opt-in: read-only queries
 *        plus the system.* process launcher, which is still hard-denied to the network API by
 *        the server. Names must match CommandRegistry registrations exactly; a stale name
 *        silently denies the command because hasCommand() is checked before this list.
 */
static const QSet<QString>& safeMethods()
{
  static const QSet<QString> kSet = {
    QStringLiteral("api.getCommands"),
    QStringLiteral("project.getStatus"),
    QStringLiteral("project.snapshot"),
    QStringLiteral("project.dataset.getByPath"),
    QStringLiteral("project.dataset.getByTitle"),
    QStringLiteral("project.dataset.getByUniqueId"),
    QStringLiteral("project.dataset.list"),
    QStringLiteral("project.group.list"),
    QStringLiteral("project.source.getConfig"),
    QStringLiteral("project.source.list"),
    QStringLiteral("project.workspace.get"),
    QStringLiteral("project.workspace.list"),
    QStringLiteral("project.dataTable.list"),
    QStringLiteral("project.dataTable.get"),
    QStringLiteral("dashboard.getStatus"),
    QStringLiteral("dashboard.getData"),
    QStringLiteral("dashboard.tailFrames"),
    QStringLiteral("notifications.list"),
    QStringLiteral("notifications.post"),
    QStringLiteral("sessions.list"),
    QStringLiteral("sessions.get"),
    QStringLiteral("controlscript.get"),
    QStringLiteral("controlscript.getStatus"),
    QStringLiteral("system.projectDir"),
    QStringLiteral("system.exec"),
    QStringLiteral("system.kill"),
    QStringLiteral("system.runningProcesses"),
  };
  return kSet;
}

/**
 * @brief Returns true when the method is in the safe allow-list or the full surface is opted in.
 */
static bool isAllowed(const QString& method)
{
  if (s_allowFullSurface.load(std::memory_order_acquire))
    return true;

  return safeMethods().contains(method);
}

static std::mutex& bucketMutex()
{
  static std::mutex m;
  return m;
}

static std::map<int, detail::TokenBucket>& buckets()
{
  static std::map<int, detail::TokenBucket> b;
  return b;
}

/**
 * @brief Erases buckets that are idle and fully refilled, so stale source ids (deleted
 *        sources, one-shot editors) cannot grow the map forever. Caller holds bucketMutex().
 */
static void reclaimIdleBuckets(std::chrono::steady_clock::time_point now)
{
  for (auto it = buckets().begin(); it != buckets().end();) {
    const auto& b   = it->second;
    const double dt = std::chrono::duration<double>(now - b.lastRefill).count();
    if (b.activeCount == 0 && b.tokens + dt * kTokensPerSec >= kBucketSize)
      it = buckets().erase(it);
    else
      ++it;
  }
}

/**
 * @brief Returns true if a token was consumed; false if rate-limited or concurrency-capped.
 */
static bool consumeRateToken(int sourceId)
{
  using clock = std::chrono::steady_clock;
  std::lock_guard<std::mutex> lock(bucketMutex());
  const auto now = clock::now();
  reclaimIdleBuckets(now);
  auto& b = buckets()[sourceId];

  if (b.lastRefill.time_since_epoch().count() == 0) {
    b.tokens      = kBucketSize;
    b.lastRefill  = now;
    b.activeCount = 0;
  }

  const double dt = std::chrono::duration<double>(now - b.lastRefill).count();
  b.tokens        = std::min(kBucketSize, b.tokens + dt * kTokensPerSec);
  b.lastRefill    = now;

  if (b.activeCount >= kMaxConcurrent)
    return false;

  if (b.tokens < 1.0)
    return false;

  b.tokens      -= 1.0;
  b.activeCount += 1;
  return true;
}

/**
 * @brief Decrements the per-source in-flight counter after a dispatched apiCall returns.
 */
static void releaseConcurrency(int sourceId)
{
  std::lock_guard<std::mutex> lock(bucketMutex());
  auto it = buckets().find(sourceId);
  if (it != buckets().end() && it->second.activeCount > 0)
    --it->second.activeCount;
}

/**
 * @brief Returns the UTF-8 byte length of @p obj serialized as compact JSON.
 */
static int approxJsonSize(const QJsonObject& obj)
{
  return QJsonDocument(obj).toJson(QJsonDocument::Compact).size();
}

/**
 * @brief Opts the running project into the full MCP command surface for apiCall().
 */
void DataModel::ScriptApiCall::setAllowFullSurface(bool allow)
{
  s_allowFullSurface.store(allow, std::memory_order_release);
}

/**
 * @brief Returns the body-size cap (bytes) applied to apiCall request params and response payloads.
 */
int DataModel::ScriptApiCall::maxBodyBytes()
{
  return kMaxBodyBytes;
}

//--------------------------------------------------------------------------------------------------
// Lua <-> QJsonValue conversion
//--------------------------------------------------------------------------------------------------

static QJsonValue luaToJson(lua_State* L, int idx, int depth);

/**
 * @brief Detects pure-array Lua tables (1..N integer keys with no holes).
 */
static bool luaTableIsArray(lua_State* L, int idx)
{
  const int abs = (idx > 0) ? idx : lua_gettop(L) + idx + 1;
  lua_Integer n = 0;

  lua_pushnil(L);
  // code-verify off  -- canonical lua_next walk, bounded by Lua table size
  while (lua_next(L, abs) != 0) {
    if (lua_type(L, -2) != LUA_TNUMBER || !lua_isinteger(L, -2)) {
      lua_pop(L, 2);
      return false;
    }

    const lua_Integer key = lua_tointeger(L, -2);
    if (key < 1) {
      lua_pop(L, 2);
      return false;
    }

    if (key > n)
      n = key;

    lua_pop(L, 1);
  }
  // code-verify on

  return n == static_cast<lua_Integer>(lua_rawlen(L, abs));
}

/**
 * @brief Walks a Lua table at @p idx and returns its JSON object/array equivalent.
 */
static QJsonValue luaTableToJson(lua_State* L, int idx, int depth)
{
  if (depth >= kMaxJsonDepth)
    return QJsonValue();

  const int abs = (idx > 0) ? idx : lua_gettop(L) + idx + 1;

  if (luaTableIsArray(L, abs)) {
    QJsonArray arr;
    const lua_Integer n = static_cast<lua_Integer>(lua_rawlen(L, abs));
    for (lua_Integer i = 1; i <= n; ++i) {
      lua_rawgeti(L, abs, i);
      arr.append(luaToJson(L, -1, depth + 1));
      lua_pop(L, 1);
    }
    return arr;
  }

  QJsonObject obj;
  lua_pushnil(L);
  // code-verify off  -- canonical lua_next walk, bounded by Lua table size
  while (lua_next(L, abs) != 0) {
    if (lua_type(L, -2) == LUA_TSTRING) {
      size_t klen      = 0;
      const char* kstr = lua_tolstring(L, -2, &klen);
      const QString k  = QString::fromUtf8(kstr, static_cast<int>(klen));
      obj.insert(k, luaToJson(L, -1, depth + 1));
    } else if (lua_type(L, -2) == LUA_TNUMBER) {
      obj.insert(QString::number(lua_tonumber(L, -2)), luaToJson(L, -1, depth + 1));
    }
    lua_pop(L, 1);
  }
  // code-verify on
  return obj;
}

/**
 * @brief Converts the Lua value at @p idx into a QJsonValue; unsupported types become null.
 */
static QJsonValue luaToJson(lua_State* L, int idx, int depth)
{
  switch (lua_type(L, idx)) {
    case LUA_TBOOLEAN:
      return QJsonValue(lua_toboolean(L, idx) != 0);
    case LUA_TNUMBER: {
      if (lua_isinteger(L, idx))
        return QJsonValue(static_cast<qint64>(lua_tointeger(L, idx)));

      return QJsonValue(lua_tonumber(L, idx));
    }
    case LUA_TSTRING: {
      size_t len      = 0;
      const char* str = lua_tolstring(L, idx, &len);
      return QJsonValue(QString::fromUtf8(str, static_cast<int>(len)));
    }
    case LUA_TTABLE:
      return luaTableToJson(L, idx, depth);
    case LUA_TNIL:
    default:
      return QJsonValue();
  }
}

static void jsonValueToLua(lua_State* L, const QJsonValue& v, int depth);

/**
 * @brief Pushes a JSON double, narrowing to lua_Integer only when finite, in the qint64 range,
 *        and exact; the range check must precede the cast, which is UB otherwise.
 */
static void pushJsonNumber(lua_State* L, double d)
{
  constexpr double kInt64Min = -9223372036854775808.0;
  constexpr double kInt64Max = 9223372036854775808.0;

  if (std::isfinite(d) && d >= kInt64Min && d < kInt64Max) {
    const qint64 i = static_cast<qint64>(d);
    if (static_cast<double>(i) == d) {
      lua_pushinteger(L, static_cast<lua_Integer>(i));
      return;
    }
  }

  lua_pushnumber(L, d);
}

/**
 * @brief Pushes a QJsonObject onto the Lua stack as a string-keyed table.
 */
static void jsonObjectToLua(lua_State* L, const QJsonObject& obj, int depth)
{
  lua_createtable(L, 0, obj.size());
  if (depth >= kMaxJsonDepth)
    return;

  for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
    const QByteArray k = it.key().toUtf8();
    lua_pushlstring(L, k.constData(), static_cast<size_t>(k.size()));
    jsonValueToLua(L, it.value(), depth + 1);
    lua_settable(L, -3);
  }
}

/**
 * @brief Pushes a QJsonArray onto the Lua stack as a 1-indexed sequence.
 */
static void jsonArrayToLua(lua_State* L, const QJsonArray& arr, int depth)
{
  lua_createtable(L, arr.size(), 0);
  if (depth >= kMaxJsonDepth)
    return;

  int i = 1;
  for (const auto& v : arr) {
    jsonValueToLua(L, v, depth + 1);
    lua_rawseti(L, -2, i++);
  }
}

/**
 * @brief Pushes any QJsonValue onto the Lua stack; null becomes nil.
 */
static void jsonValueToLua(lua_State* L, const QJsonValue& v, int depth)
{
  switch (v.type()) {
    case QJsonValue::Bool:
      lua_pushboolean(L, v.toBool() ? 1 : 0);
      break;
    case QJsonValue::Double:
      pushJsonNumber(L, SerialStudio::toDouble(v));
      break;
    case QJsonValue::String: {
      const QByteArray s = v.toString().toUtf8();
      lua_pushlstring(L, s.constData(), static_cast<size_t>(s.size()));
      break;
    }
    case QJsonValue::Array:
      jsonArrayToLua(L, v.toArray(), depth);
      break;
    case QJsonValue::Object:
      jsonObjectToLua(L, v.toObject(), depth);
      break;
    case QJsonValue::Null:
    case QJsonValue::Undefined:
    default:
      lua_pushnil(L);
      break;
  }
}

//--------------------------------------------------------------------------------------------------
// Shared dispatcher
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs the API command synchronously and returns the wrapped CommandResponse.
 */
static API::CommandResponse dispatchApiCall(const QString& method, const QJsonObject& params)
{
  API::CommandRequest request;
  request.id      = QUuid::createUuid().toString(QUuid::WithoutBraces);
  request.command = method;
  request.params  = params;
  return API::CommandHandler::instance().processCommand(request);
}

//--------------------------------------------------------------------------------------------------
// JS bridge
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the JS-side apiCall bridge owned by @p parent.
 */
DataModel::ScriptApiCallBridge::ScriptApiCallBridge(int sourceId, QObject* parent)
  : QObject(parent), m_sourceId(sourceId)
{}

/**
 * @brief Implements apiCall(method, params?) for the JS scripting engine.
 */
QVariantMap DataModel::ScriptApiCallBridge::call(const QJSValue& methodVal,
                                                 const QJSValue& paramsVal)
{
  QVariantMap out;

  if (!methodVal.isString()) {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), QStringLiteral("apiCall: method must be a string"));
    return out;
  }

  const QString method = methodVal.toString();
  if (method.isEmpty()) {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), QStringLiteral("apiCall: method must not be empty"));
    return out;
  }

  if (!API::CommandRegistry::instance().hasCommand(method)) {
    const auto unknown = dispatchApiCall(method, QJsonObject());
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), unknown.errorMessage);
    out.insert(QStringLiteral("errorCode"), unknown.errorCode);
    if (!unknown.errorData.isEmpty())
      out.insert(QStringLiteral("errorData"), unknown.errorData.toVariantMap());

    return out;
  }

  if (!isAllowed(method)) {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("errorCode"), QStringLiteral("METHOD_NOT_ALLOWED"));
    out.insert(QStringLiteral("error"),
               QStringLiteral("apiCall: method '%1' not in default allow-list "
                              "(enable apiCall.allowFullSurface in project to opt in)")
                 .arg(method));
    return out;
  }

  QJsonObject params;
  if (!paramsVal.isUndefined() && !paramsVal.isNull()) {
    if (!paramsVal.isObject() || paramsVal.isArray() || paramsVal.isCallable()) {
      out.insert(QStringLiteral("ok"), false);
      out.insert(QStringLiteral("error"), QStringLiteral("apiCall: params must be an object"));
      return out;
    }

    const QVariant qv = paramsVal.toVariant();
    params            = QJsonObject::fromVariantMap(qv.toMap());

    if (approxJsonSize(params) > kMaxBodyBytes) {
      out.insert(QStringLiteral("ok"), false);
      out.insert(QStringLiteral("error"),
                 QStringLiteral("apiCall: params exceed %1 byte cap").arg(kMaxBodyBytes));
      return out;
    }
  }

  if (!consumeRateToken(m_sourceId)) {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"),
               QStringLiteral("apiCall: rate-limited or concurrency cap reached for source"));
    return out;
  }

  API::CommandResponse response;
  try {
    response = dispatchApiCall(method, params);
  } catch (const std::exception& e) {
    releaseConcurrency(m_sourceId);
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), QString::fromUtf8(e.what()));
    return out;
  } catch (...) {
    releaseConcurrency(m_sourceId);
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), QStringLiteral("apiCall: unknown exception"));
    return out;
  }
  releaseConcurrency(m_sourceId);

  if (response.success && approxJsonSize(response.result) > kMaxBodyBytes) {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"),
               QStringLiteral("apiCall: response exceeds %1 byte cap").arg(kMaxBodyBytes));
    return out;
  }

  out.insert(QStringLiteral("ok"), response.success);
  if (response.success) {
    if (!response.result.isEmpty())
      out.insert(QStringLiteral("result"), response.result.toVariantMap());
  } else {
    out.insert(QStringLiteral("error"), response.errorMessage);
    out.insert(QStringLiteral("errorCode"), response.errorCode);
    if (!response.errorData.isEmpty())
      out.insert(QStringLiteral("errorData"), response.errorData.toVariantMap());
  }
  return out;
}

/**
 * @brief Returns the full list of registered API command names for discovery.
 */
QVariantList DataModel::ScriptApiCallBridge::listCommands()
{
  QVariantList result;
  const auto& commands = API::CommandRegistry::instance().commands();
  result.reserve(commands.size());
  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it)
    result.append(it.key());

  return result;
}

//--------------------------------------------------------------------------------------------------
// Lua glue
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pushes the CommandResponse onto the Lua stack as a result table.
 */
static void pushApiResult(lua_State* L, const API::CommandResponse& r)
{
  lua_createtable(L, 0, r.success ? 2 : 4);

  lua_pushboolean(L, r.success ? 1 : 0);
  lua_setfield(L, -2, "ok");

  if (r.success) {
    if (!r.result.isEmpty()) {
      jsonObjectToLua(L, r.result, 0);
      lua_setfield(L, -2, "result");
    }
  } else {
    const QByteArray msg = r.errorMessage.toUtf8();
    lua_pushlstring(L, msg.constData(), static_cast<size_t>(msg.size()));
    lua_setfield(L, -2, "error");

    const QByteArray code = r.errorCode.toUtf8();
    lua_pushlstring(L, code.constData(), static_cast<size_t>(code.size()));
    lua_setfield(L, -2, "errorCode");

    if (!r.errorData.isEmpty()) {
      jsonObjectToLua(L, r.errorData, 0);
      lua_setfield(L, -2, "errorData");
    }
  }
}

/**
 * @brief Pushes an early-failure {ok = false, error = msg[, errorCode]} table onto the Lua stack.
 */
static void pushLuaErr(lua_State* L, const QString& msg, const QString& code = QString())
{
  lua_createtable(L, 0, code.isEmpty() ? 2 : 3);
  lua_pushboolean(L, 0);
  lua_setfield(L, -2, "ok");

  const QByteArray utf8 = msg.toUtf8();
  lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
  lua_setfield(L, -2, "error");

  if (!code.isEmpty()) {
    const QByteArray codeUtf8 = code.toUtf8();
    lua_pushlstring(L, codeUtf8.constData(), static_cast<size_t>(codeUtf8.size()));
    lua_setfield(L, -2, "errorCode");
  }
}

/**
 * @brief Lua C closure for apiCall(method, params?) returning a result table.
 */
static int luaApiCall(lua_State* L)
{
  const int sourceId = static_cast<int>(lua_tointeger(L, lua_upvalueindex(1)));

  if (!lua_isstring(L, 1)) {
    pushLuaErr(L, QStringLiteral("apiCall: method must be a string"));
    return 1;
  }

  size_t mlen      = 0;
  const char* mstr = lua_tolstring(L, 1, &mlen);
  const QString method(QString::fromUtf8(mstr, static_cast<int>(mlen)));
  if (method.isEmpty()) {
    pushLuaErr(L, QStringLiteral("apiCall: method must not be empty"));
    return 1;
  }

  if (!API::CommandRegistry::instance().hasCommand(method)) {
    pushApiResult(L, dispatchApiCall(method, QJsonObject()));
    return 1;
  }

  if (!isAllowed(method)) {
    pushLuaErr(L,
               QStringLiteral("apiCall: method '%1' not in default allow-list "
                              "(enable apiCall.allowFullSurface in project to opt in)")
                 .arg(method),
               QStringLiteral("METHOD_NOT_ALLOWED"));
    return 1;
  }

  QJsonObject params;
  if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
    if (!lua_istable(L, 2)) {
      pushLuaErr(L, QStringLiteral("apiCall: params must be a table"));
      return 1;
    }

    const QJsonValue v = luaTableToJson(L, 2, 0);
    if (v.isObject())
      params = v.toObject();
    else {
      pushLuaErr(L, QStringLiteral("apiCall: params must be an object-like table"));
      return 1;
    }

    if (approxJsonSize(params) > kMaxBodyBytes) {
      pushLuaErr(L, QStringLiteral("apiCall: params exceed %1 byte cap").arg(kMaxBodyBytes));
      return 1;
    }
  }

  if (!consumeRateToken(sourceId)) {
    pushLuaErr(L, QStringLiteral("apiCall: rate-limited or concurrency cap reached for source"));
    return 1;
  }

  try {
    const auto r = dispatchApiCall(method, params);
    releaseConcurrency(sourceId);
    if (r.success && approxJsonSize(r.result) > kMaxBodyBytes) {
      pushLuaErr(L, QStringLiteral("apiCall: response exceeds %1 byte cap").arg(kMaxBodyBytes));
      return 1;
    }
    pushApiResult(L, r);
  } catch (const std::exception& e) {
    releaseConcurrency(sourceId);
    pushLuaErr(L, QString::fromUtf8(e.what()));
  } catch (...) {
    releaseConcurrency(sourceId);
    pushLuaErr(L, QStringLiteral("apiCall: unknown exception"));
  }
  return 1;
}

/**
 * @brief Lua C closure that returns the array of registered API command names.
 */
static int luaApiCallList(lua_State* L)
{
  const auto& commands = API::CommandRegistry::instance().commands();
  lua_createtable(L, commands.size(), 0);

  int i = 1;
  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
    const QByteArray name = it.key().toUtf8();
    lua_pushlstring(L, name.constData(), static_cast<size_t>(name.size()));
    lua_rawseti(L, -2, i++);
  }
  return 1;
}

//--------------------------------------------------------------------------------------------------
// Public installation entry points
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads a cached copy of a generated SDK resource (SerialStudio.js / .lua).
 */
static QByteArray loadSdk(const QString& path)
{
  QFile file(path);
  if (file.open(QFile::ReadOnly))
    return file.readAll();

  return {};
}

/**
 * @brief Installs apiCall() / apiCallList() plus the SerialStudio SDK as Lua globals.
 */
void DataModel::ScriptApiCall::installLua(lua_State* L, int sourceId)
{
  Q_ASSERT(L);

  lua_pushinteger(L, sourceId);
  lua_pushcclosure(L, luaApiCall, 1);
  lua_setglobal(L, "apiCall");

  lua_pushcfunction(L, luaApiCallList);
  lua_setglobal(L, "apiCallList");

  static const QByteArray sdk = loadSdk(QStringLiteral(":/api/SerialStudio.lua"));
  if (!sdk.isEmpty())
    (void)luaL_dostring(L, sdk.constData());
}

/**
 * @brief Installs apiCall() / apiCallList() into a QJSEngine, reusing the bridge if attached.
 */
void DataModel::ScriptApiCall::installJS(QJSEngine* js, int sourceId)
{
  Q_ASSERT(js);

  auto global       = js->globalObject();
  auto existingProp = global.property(QStringLiteral("__ss_bridge"));
  if (existingProp.isQObject()
      && qobject_cast<DataModel::ScriptApiCallBridge*>(existingProp.toQObject()))
    return;

  auto* bridge   = new DataModel::ScriptApiCallBridge(sourceId, js);
  auto bridgeVal = js->newQObject(bridge);
  global.setProperty(QStringLiteral("__ss_bridge"), bridgeVal);

  static const QByteArray sdk = loadSdk(QStringLiteral(":/api/SerialStudio.js"));
  if (!sdk.isEmpty())
    js->evaluate(QString::fromUtf8(sdk));
}

/**
 * @brief Installs the convenience host bridges (no apiCall bridge, no SDK eval).
 */
void DataModel::ScriptApiCall::installHelperBridgesJS(QJSEngine* js, int sourceId)
{
  Q_ASSERT(js);

  DataModel::NotificationCenter::installScriptApi(js);
  DataModel::FrameBuilder::instance().injectTableApiJS(js);
  DataModel::DeviceWriteApi::installJS(js, sourceId);
  DataModel::ActionFireApi::installJS(js);
  DataModel::DashboardApi::installJS(js);
}

/**
 * @brief Installs every host bridge and the full SDK into a QJSEngine in one call.
 */
void DataModel::ScriptApiCall::installAll(QJSEngine* js, int sourceId)
{
  Q_ASSERT(js);

  installHelperBridgesJS(js, sourceId);
  installJS(js, sourceId);
}

/**
 * @brief Installs every host bridge and the full SDK into a Lua state in one call.
 */
void DataModel::ScriptApiCall::installAll(lua_State* L, int sourceId)
{
  Q_ASSERT(L);

  DataModel::NotificationCenter::installScriptApi(L);
  DataModel::FrameBuilder::instance().injectTableApiLua(L);
  DataModel::DeviceWriteApi::installLua(L, sourceId);
  DataModel::ActionFireApi::installLua(L);
  DataModel::DashboardApi::installLua(L);

  installLua(L, sourceId);
}
