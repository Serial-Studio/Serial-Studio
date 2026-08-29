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

#include "DataModel/Scripting/ScriptResult.h"

// clang-format off
extern "C" {
#include <lua.h>
}
// clang-format on

#include <QByteArray>

#include "SSAssert.h"

/**
 * @brief Pushes a UTF-8 copy of @p value as the string field @p field of the table at stack top.
 */
static void setLuaStringField(lua_State* L, const char* field, const QString& value)
{
  SS_ASSERT(L != nullptr, return);
  SS_ASSERT(field != nullptr, return);

  const QByteArray utf8 = value.toUtf8();
  lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
  lua_setfield(L, -2, field);
}

/**
 * @brief Pushes a {ok, error?} table onto the Lua stack; the error field exists only on failure.
 */
void DataModel::ScriptResult::pushLuaResult(lua_State* L, bool ok, const QString& error)
{
  SS_ASSERT(L != nullptr, return);

  lua_createtable(L, 0, ok ? 1 : 2);

  lua_pushboolean(L, ok ? 1 : 0);
  lua_setfield(L, -2, "ok");

  if (!ok)
    setLuaStringField(L, "error", error);
}

/**
 * @brief Pushes a failed {ok = false, error, errorCode?} table onto the Lua stack.
 */
void DataModel::ScriptResult::pushLuaError(lua_State* L,
                                           const QString& message,
                                           const QString& errorCode)
{
  SS_ASSERT(L != nullptr, return);

  lua_createtable(L, 0, errorCode.isEmpty() ? 2 : 3);

  lua_pushboolean(L, 0);
  lua_setfield(L, -2, "ok");

  setLuaStringField(L, "error", message);

  if (!errorCode.isEmpty())
    setLuaStringField(L, "errorCode", errorCode);
}

/**
 * @brief Builds the JS-side {ok, error?} map; the error key exists only on failure.
 */
QVariantMap DataModel::ScriptResult::makeResult(bool ok, const QString& error)
{
  QVariantMap result;
  result.insert(QStringLiteral("ok"), ok);
  if (!ok)
    result.insert(QStringLiteral("error"), error);

  return result;
}

/**
 * @brief Builds a failed {ok = false, error} map.
 */
QVariantMap DataModel::ScriptResult::makeError(const QString& message)
{
  return makeResult(false, message);
}
