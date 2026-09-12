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

#include "DataModel/Scripting/LuaCellCollector.h"

// clang-format off
extern "C" {
#include <lua.h>
}
// clang-format on

#include <cstring>
#include <QByteArray>
#include <QDebug>

#include "Core/SSAssert.h"
#include "DataModel/Scripting/LuaCompatJIT.h"

namespace DataModel {

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr qsizetype kNumberTextCapacity = 32;
static constexpr qsizetype kBytesPerCellGuess  = 16;

//--------------------------------------------------------------------------------------------------
// Element conversion
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the value on top of the stack as one cell: numbers keep their value with Lua's
 *        list-path text, strings are copied up to their first NUL (the list path's
 *        QString::fromUtf8(const char*) stopped there), anything else coerces through lua_tostring
 *        as luaValueToString() did (nil and booleans become empty text).
 */
void LuaCellCollector::appendTop(lua_State* L, ScriptCellRows& rows)
{
  SS_ASSERT(L != nullptr, return);

  const int type = lua_type(L, -1);
  if (type == LUA_TNUMBER) {
    char text[kNumberTextCapacity];
    const double value  = lua_tonumber(L, -1);
    const bool integral = lua_isinteger(L, -1) != 0;
    const qsizetype len = formatLuaNumber(value, integral, text, kNumberTextCapacity);
    SS_ASSERT_LOG(len > 0);
    rows.appendNumber(value, text, len);
    return;
  }

  size_t length       = 0;
  const char* bytes   = (type == LUA_TSTRING) ? lua_tolstring(L, -1, &length) : nullptr;
  const char* coerced = bytes ? bytes : lua_tostring(L, -1);
  const qsizetype count =
    coerced ? static_cast<qsizetype>(bytes ? qstrnlen(coerced, length) : std::strlen(coerced)) : 0;
  rows.appendText(coerced, count);
}

/**
 * @brief Appends the first @p maxElements array entries of the table on top of the stack as one
 *        row. With @p rejectTables (a top-level flat row) a nested table means the
 * scalar-plus-table shape the unzip path owns: false, with the stack restored to the table.
 */
bool LuaCellCollector::appendTableRow(lua_State* L,
                                      ScriptCellRows& rows,
                                      qsizetype maxElements,
                                      bool rejectTables)
{
  SS_ASSERT(L != nullptr, return false);
  SS_ASSERT(lua_istable(L, -1), return false);

  const auto len   = static_cast<qsizetype>(lua_rawlen(L, -1));
  const auto count = qMin(len, maxElements);
  rows.beginRow();
  for (qsizetype i = 1; i <= count; ++i) {
    lua_rawgeti(L, -1, static_cast<int>(i));
    if (rejectTables && lua_istable(L, -1)) [[unlikely]] {
      lua_pop(L, 1);
      return false;
    }

    appendTop(L, rows);
    lua_pop(L, 1);
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Entry point
//--------------------------------------------------------------------------------------------------

/**
 * @brief Scalar -> one cell; flat table -> one row; table of tables -> one row each, decided by the
 *        first entry and checked on the same single walk that appends; a mixed shape -> false with
 *        the value left on the stack for the list path.
 */
bool LuaCellCollector::collect(lua_State* L, ScriptCellRows& rows, qsizetype maxElements)
{
  SS_ASSERT(L != nullptr, return false);
  SS_ASSERT(maxElements > 0, return false);

  rows.clear();
  if (!lua_istable(L, -1)) {
    const int type = lua_type(L, -1);
    if (type == LUA_TSTRING || type == LUA_TNUMBER) {
      rows.reserve(1, kNumberTextCapacity);
      rows.beginRow();
      appendTop(L, rows);
    }

    lua_pop(L, 1);
    return true;
  }

  const auto len = static_cast<qsizetype>(lua_rawlen(L, -1));
  if (len == 0) {
    lua_pop(L, 1);
    return true;
  }

  const auto count = qMin(len, maxElements);
  rows.reserve(count, count * kBytesPerCellGuess);
  lua_rawgeti(L, -1, 1);
  const bool nested = lua_istable(L, -1);
  lua_pop(L, 1);

  if (!nested) {
    if (!appendTableRow(L, rows, maxElements, true)) {
      rows.clear();
      return false;
    }

    lua_pop(L, 1);
    return true;
  }

  for (qsizetype i = 1; i <= count; ++i) {
    lua_rawgeti(L, -1, static_cast<int>(i));
    const bool is_table = lua_istable(L, -1);
    if (is_table)
      (void)appendTableRow(L, rows, maxElements, false);

    lua_pop(L, 1);
    if (!is_table) [[unlikely]] {
      rows.clear();
      return false;
    }
  }

  lua_pop(L, 1);
  return true;
}

}  // namespace DataModel
