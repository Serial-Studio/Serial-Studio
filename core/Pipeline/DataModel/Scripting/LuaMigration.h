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

#include <QString>

namespace DataModel::LuaMigration {

/**
 * @brief Reports whether the script uses a Lua 5.3-only operator (<< >> & | ~ //) in code
 *        position, which LuaJIT's 5.1 grammar rejects at compile time.
 */
[[nodiscard]] bool usesLua53Operators(const QString& script);

/**
 * @brief Rewrites the Lua 5.3-only operators into LuaJIT bit.* calls and math.floor division,
 *        returning an empty string when there is nothing to rewrite or an operand cannot be
 *        delimited with certainty -- a partial rewrite would silently change the arithmetic.
 */
[[nodiscard]] QString migrateToLuaJit(const QString& script);

}  // namespace DataModel::LuaMigration
