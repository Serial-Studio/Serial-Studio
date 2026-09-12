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

#include <QtGlobal>

#include "DataModel/Scripting/ScriptCells.h"

struct lua_State;

namespace DataModel {

/**
 * @brief Turns the Lua value a parser script returned into typed cell rows (spec 0086): a scalar
 *        or a flat table is one row, a table of tables is one row per inner table. Numbers keep
 *        their value and carry the text luaValueToString() would have produced. Free of engine
 *        state on purpose, so the unit tier drives it with a plain lua_State.
 */
class LuaCellCollector {
public:
  [[nodiscard]] static bool collect(lua_State* L, ScriptCellRows& rows, qsizetype maxElements);

private:
  static void appendTop(lua_State* L, ScriptCellRows& rows);
  [[nodiscard]] static bool appendTableRow(lua_State* L,
                                           ScriptCellRows& rows,
                                           qsizetype maxElements,
                                           bool rejectTables);
};

}  // namespace DataModel
