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

#include "DataModel/FrameBuilder/TransformSupport.h"

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
}
// clang-format on

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"

/**
 * @brief Pushes a dataset's parameter map as a Lua table (spec 0083): booleans, strings and
 *        numbers, which is every value type the project reader lets through.
 */
void DataModel::pushTransformParams(lua_State* L, const QVariantMap& params)
{
  SS_ASSERT(L != nullptr, return);
  SS_ASSERT_LOG(params.size() < 4096);

  lua_createtable(L, 0, static_cast<int>(params.size()));
  for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
    const QVariant& value = it.value();
    if (value.typeId() == QMetaType::Bool) {
      lua_pushboolean(L, value.toBool() ? 1 : 0);
    } else if (value.typeId() == QMetaType::QString) {
      const QByteArray text = value.toString().toUtf8();
      lua_pushlstring(L, text.constData(), static_cast<size_t>(text.size()));
    } else {
      lua_pushnumber(L, SerialStudio::toDouble(value));
    }

    const QByteArray key = it.key().toUtf8();
    lua_setfield(L, -2, key.constData());
  }
}

/**
 * @brief Returns a dataset's parameter map as compact JSON, the argument the JavaScript IIFE
 *        wrapper receives as `params`.
 */
QString DataModel::transformParamsJson(const QVariantMap& params)
{
  return QString::fromUtf8(
    QJsonDocument(QJsonObject::fromVariantMap(params)).toJson(QJsonDocument::Compact));
}

/**
 * @brief Loads and runs the shared library chunk in the default (global) environment, so its
 *        top-level functions land in _G for every entry environment's __index fallthrough.
 */
bool DataModel::loadLuaLibraryChunk(lua_State* L, const QString& code, QString& error)
{
  SS_ASSERT(L != nullptr, return false);
  error.clear();
  if (code.trimmed().isEmpty())
    return true;

  const int baseTop     = lua_gettop(L);
  const QByteArray utf8 = code.toUtf8();
  const bool loaded = luaL_loadbufferx(L, utf8.constData(), utf8.size(), "=library", "t") == LUA_OK;
  if (!loaded || lua_pcall(L, 0, 0, 0) != LUA_OK) {
    const char* message = lua_tostring(L, -1);
    error               = QString::fromUtf8(message ? message : "library failed");
    lua_settop(L, baseTop);
    return false;
  }

  SS_ASSERT(lua_gettop(L) == baseTop, lua_settop(L, baseTop));
  return true;
}
