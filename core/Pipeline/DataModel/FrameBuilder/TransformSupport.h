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

#pragma once

// clang-format off
extern "C" {
#include <lua.h>
}
// clang-format on

#include <QString>
#include <QVariantMap>

namespace DataModel {

/**
 * @brief Runs the project's shared Lua library chunk into @p L's global table. Returns false with
 *        @p error filled when the chunk fails to load or run; the stack is left as it was found.
 *        Shared by the frame lane, the stream lane and the editor so "the library" means one thing.
 */
[[nodiscard]] bool loadLuaLibraryChunk(lua_State* L, const QString& code, QString& error);

/**
 * @brief Pushes a dataset's parameter map as a Lua table: booleans, strings and numbers.
 */
void pushTransformParams(lua_State* L, const QVariantMap& params);

/**
 * @brief Returns a dataset's parameter map as compact JSON, the JavaScript IIFE's `params`
 * argument.
 */
[[nodiscard]] QString transformParamsJson(const QVariantMap& params);

}  // namespace DataModel
