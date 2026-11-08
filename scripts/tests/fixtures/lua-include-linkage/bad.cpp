// SPDX-FileCopyrightText: 2026 Alex Spataru <https://serial-studio.com/>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <lua.h>

/**
 * @brief Counts the stack through a header included with C++ linkage.
 */
int Sample::depth(lua_State* L)
{
  return lua_gettop(L);
}
