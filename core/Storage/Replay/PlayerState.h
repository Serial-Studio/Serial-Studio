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

/**
 * @file PlayerState.h
 * @brief The "is any replay player open" predicates, owned by the layer that owns the players.
 *        Spec 0077 phase 2 replaces every reader with the ReplayPlayerStateChanged topic.
 */

namespace SerialStudio {
[[nodiscard]] bool isAnyPlayerOpen();

/**
 * @brief Returns true when a player that stores post-transform values is open; transforms must
 *        not run again on replayed final values (they read live inputs that do not exist then).
 */
[[nodiscard]] inline bool isFinalValuePlayerOpen()
{
  return isAnyPlayerOpen();
}
}  // namespace SerialStudio
