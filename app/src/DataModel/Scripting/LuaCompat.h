/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <lua.h>

namespace DataModel {

/**
 * @brief Restores Lua 5.1 / 5.2 names that 5.4 dropped, so user code pasted
 * from external sources (math.log10, math.pow, bit32.*, unpack, ...) keeps
 * working in frame parsers and per-dataset transforms.
 */
void installLuaCompat(lua_State* L);

/**
 * @brief Replaces print() and installs a JS-style console table (log, debug,
 * info, warn, error) that route script output through the Qt message handler,
 * making it visible in the application console instead of stdout.
 */
void installLuaConsole(lua_State* L);

/**
 * @brief Installs an os table restricted to the side-effect-free time functions
 * (time, date, clock, difftime), so timestamping scripts work while process and
 * filesystem entries (execute, remove, getenv, ...) never exist in the sandbox.
 */
void installLuaRestrictedOs(lua_State* L);

}  // namespace DataModel
