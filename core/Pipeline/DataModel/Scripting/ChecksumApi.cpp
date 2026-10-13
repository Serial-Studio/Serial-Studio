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

#include "DataModel/Scripting/ChecksumApi.h"

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
}
// clang-format on

#include <QByteArray>
#include <QJSEngine>
#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QVariantList>

#include "Core/Checksum.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Shared core
//--------------------------------------------------------------------------------------------------

static constexpr qsizetype kMaxChecksumBytes = qsizetype(1) << 20;

static const char* const kDataError = "data must be a string or an array of byte values";

/**
 * @brief Whether @p name is a real algorithm; the registry's blank "no checksum" row is not one.
 */
[[nodiscard]] static bool algorithmKnown(const QString& name)
{
  return !name.isEmpty() && IO::checksumFunctionMap().contains(name);
}

/**
 * @brief The algorithm names a script may pass, in registry order and without the blank row.
 */
[[nodiscard]] static QStringList scriptAlgorithms()
{
  QStringList names;
  for (const auto& name : IO::availableChecksums())
    if (!name.isEmpty())
      names.append(name);

  return names;
}

//--------------------------------------------------------------------------------------------------
// Lua binding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the data argument at @p index as raw bytes: a Lua string as-is, a table as one
 *        byte per element.
 */
[[nodiscard]] static bool luaBytes(lua_State* L, int index, QByteArray& out)
{
  if (lua_type(L, index) == LUA_TSTRING) {
    size_t length    = 0;
    const char* data = lua_tolstring(L, index, &length);
    if (static_cast<qsizetype>(length) > kMaxChecksumBytes)
      return false;

    out = QByteArray(data, static_cast<qsizetype>(length));
    return true;
  }

  if (lua_type(L, index) != LUA_TTABLE)
    return false;

  const auto count = static_cast<qsizetype>(lua_objlen(L, index));
  if (count > kMaxChecksumBytes)
    return false;

  out.resize(count);
  for (qsizetype i = 0; i < count; ++i) {
    lua_rawgeti(L, index, static_cast<int>(i + 1));
    out[i] = static_cast<char>(lua_tointeger(L, -1) & 0xFF);
    lua_pop(L, 1);
  }

  return true;
}

/**
 * @brief Shared body of the two Lua entries: validates both arguments and pushes the digest. The
 *        argument errors are raised while no Qt object is alive on this frame, so the longjmp
 *        underneath luaL_error has nothing to leak.
 */
static int luaChecksumImpl(lua_State* L, bool hex)
{
  size_t nameLength = 0;
  const char* name  = luaL_checklstring(L, 1, &nameLength);
  if (!algorithmKnown(QString::fromUtf8(name, static_cast<qsizetype>(nameLength))))
    return luaL_error(L, "unknown checksum algorithm %s", name);

  const int dataType = lua_type(L, 2);
  if (dataType != LUA_TSTRING && dataType != LUA_TTABLE)
    return luaL_argerror(L, 2, kDataError);

  bool tooLong = false;
  QByteArray digest;
  {
    QByteArray data;
    tooLong = !luaBytes(L, 2, data);
    if (!tooLong) {
      const QString algorithm = QString::fromUtf8(name, static_cast<qsizetype>(nameLength));
      digest = hex ? IO::checksum(algorithm, data).toHex() : IO::checksum(algorithm, data);
    }
  }

  if (tooLong) {
    digest = QByteArray();
    return luaL_argerror(L, 2, "data longer than 1 MiB");
  }

  lua_pushlstring(L, digest.constData(), static_cast<size_t>(digest.size()));
  return 1;
}

/**
 * @brief Lua entry for checksum(algorithm, data) returning the digest bytes as a string.
 */
static int luaChecksum(lua_State* L)
{
  return luaChecksumImpl(L, false);
}

/**
 * @brief Lua entry for checksumHex(algorithm, data) returning lowercase hex.
 */
static int luaChecksumHex(lua_State* L)
{
  return luaChecksumImpl(L, true);
}

/**
 * @brief Lua entry for checksumAlgorithms() returning the algorithm names as an array.
 */
static int luaChecksumAlgorithms(lua_State* L)
{
  const QStringList names = scriptAlgorithms();
  lua_createtable(L, static_cast<int>(names.size()), 0);
  for (qsizetype i = 0; i < names.size(); ++i) {
    const QByteArray utf8 = names[i].toUtf8();
    lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
    lua_rawseti(L, -2, static_cast<int>(i + 1));
  }

  return 1;
}

/**
 * @brief Registers the three checksum globals in a Lua state.
 */
void DataModel::ChecksumApi::installLua(lua_State* L)
{
  SS_ASSERT(L != nullptr, return);

  lua_pushcfunction(L, luaChecksum);
  lua_setglobal(L, "checksum");

  lua_pushcfunction(L, luaChecksumHex);
  lua_setglobal(L, "checksumHex");

  lua_pushcfunction(L, luaChecksumAlgorithms);
  lua_setglobal(L, "checksumAlgorithms");
}

//--------------------------------------------------------------------------------------------------
// JavaScript binding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the data argument as raw bytes: a string by its Latin-1 code points (the SDK's
 *        binary-string convention), an array or typed array one byte per element, an ArrayBuffer
 *        as-is.
 */
[[nodiscard]] static bool jsBytes(const QJSValue& value, QByteArray& out)
{
  if (value.isString()) {
    const QString text = value.toString();
    if (text.size() > kMaxChecksumBytes)
      return false;

    out = text.toLatin1();
    return true;
  }

  if (value.isArray()) {
    const auto length = static_cast<qsizetype>(value.property(QStringLiteral("length")).toUInt());
    if (length > kMaxChecksumBytes)
      return false;

    out.reserve(length);
    for (qsizetype i = 0; i < length; ++i)
      out.append(static_cast<char>(value.property(static_cast<quint32>(i)).toInt() & 0xFF));

    return true;
  }

  const QVariant variant = value.toVariant();
  if (variant.typeId() == QMetaType::QByteArray) {
    out = variant.toByteArray();
    return out.size() <= kMaxChecksumBytes;
  }

  if (!variant.canConvert<QVariantList>())
    return false;

  const QVariantList list = variant.toList();
  if (list.size() > kMaxChecksumBytes)
    return false;

  out.reserve(list.size());
  for (const auto& element : list)
    out.append(static_cast<char>(element.toInt() & 0xFF));

  return true;
}

DataModel::ChecksumBridge::ChecksumBridge(QObject* parent) : QObject(parent) {}

/**
 * @brief JS entry for checksum(algorithm, data): the digest bytes as a binary string.
 */
QJSValue DataModel::ChecksumBridge::checksum(const QJSValue& algorithm, const QJSValue& data)
{
  return compute(algorithm, data, false);
}

/**
 * @brief JS entry for checksumHex(algorithm, data): the digest as lowercase hex.
 */
QJSValue DataModel::ChecksumBridge::checksumHex(const QJSValue& algorithm, const QJSValue& data)
{
  return compute(algorithm, data, true);
}

/**
 * @brief JS entry for checksumAlgorithms(): the algorithm names.
 */
QStringList DataModel::ChecksumBridge::checksumAlgorithms() const
{
  return scriptAlgorithms();
}

/**
 * @brief Validates both arguments, throwing a TypeError into the calling script on a bad one.
 */
QJSValue DataModel::ChecksumBridge::compute(const QJSValue& algorithm,
                                            const QJSValue& data,
                                            bool hex)
{
  auto* engine = qjsEngine(this);
  SS_ASSERT(engine != nullptr, return QJSValue());

  const QString name = algorithm.isString() ? algorithm.toString() : QString();
  if (!algorithmKnown(name)) {
    engine->throwError(QJSValue::TypeError,
                       QStringLiteral("unknown checksum algorithm %1").arg(name));
    return QJSValue();
  }

  QByteArray bytes;
  if (!jsBytes(data, bytes)) {
    engine->throwError(QJSValue::TypeError, QString::fromLatin1(kDataError));
    return QJSValue();
  }

  const QByteArray digest = IO::checksum(name, bytes);
  if (hex)
    return QJSValue(QString::fromLatin1(digest.toHex()));

  return QJSValue(QString::fromLatin1(digest));
}

/**
 * @brief Installs the __ss_ck bridge once per engine; the SDK prelude defines the globals on it.
 */
void DataModel::ChecksumApi::installJS(QJSEngine* js)
{
  SS_ASSERT(js != nullptr, return);

  auto global       = js->globalObject();
  auto existingProp = global.property(QStringLiteral("__ss_ck"));
  if (existingProp.isQObject()
      && qobject_cast<DataModel::ChecksumBridge*>(existingProp.toQObject()))
    return;

  auto* bridge   = new DataModel::ChecksumBridge(js);
  auto bridgeVal = js->newQObject(bridge);
  global.setProperty(QStringLiteral("__ss_ck"), bridgeVal);
}
