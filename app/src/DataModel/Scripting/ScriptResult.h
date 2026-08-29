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

#pragma once

#include <QString>
#include <QVariantMap>

struct lua_State;

/**
 * @brief The {ok, error?} envelope every scripting host bridge answers with, in both languages.
 *        Kept in one place because a Lua table and a JS object that disagree on the shape turn
 *        one host call into two user-visible contracts.
 */
namespace DataModel::ScriptResult {

/**
 * @brief Pushes a {ok, error?} table onto the Lua stack; the error field exists only on failure.
 */
void pushLuaResult(lua_State* L, bool ok, const QString& error);

/**
 * @brief Pushes a failed {ok = false, error, errorCode?} table onto the Lua stack.
 */
void pushLuaError(lua_State* L, const QString& message, const QString& errorCode = QString());

/**
 * @brief Builds the JS-side {ok, error?} map; the error key exists only on failure.
 */
[[nodiscard]] QVariantMap makeResult(bool ok, const QString& error);

/**
 * @brief Builds a failed {ok = false, error} map.
 */
[[nodiscard]] QVariantMap makeError(const QString& message);

}  // namespace DataModel::ScriptResult
