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

#include "Core/Api/UserWaitScope.h"

static thread_local bool s_userWaitAllowed = false;

/**
 * @brief Sets this thread's permission to @p allowed, remembering the state it replaces.
 */
API::UserWaitScope::UserWaitScope(bool allowed) noexcept : m_previous(s_userWaitAllowed)
{
  s_userWaitAllowed = allowed;
}

/**
 * @brief Restores the permission that was in force when the scope opened.
 */
API::UserWaitScope::~UserWaitScope() noexcept
{
  s_userWaitAllowed = m_previous;
}

/**
 * @brief Whether the command now dispatching on this thread may block on the user.
 */
bool API::UserWaitScope::allowed() noexcept
{
  return s_userWaitAllowed;
}
