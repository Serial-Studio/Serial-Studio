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

#include "Replay/PlayerState.h"

#include "CSV/Player.h"
#include "MDF4/Player.h"

#ifdef BUILD_COMMERCIAL
#  include "Sessions/Player.h"
#endif

/**
 * @brief Checks if any playback player (CSV, MDF4 or Historian) is currently open.
 */
bool SerialStudio::isAnyPlayerOpen()
{
  static auto& csvPlayer  = CSV::Player::instance();
  static auto& mdf4Player = MDF4::Player::instance();

#ifdef BUILD_COMMERCIAL
  static auto& sqlPlayer = Sessions::Player::instance();
  return csvPlayer.isOpen() || mdf4Player.isOpen() || sqlPlayer.isOpen();
#else
  return csvPlayer.isOpen() || mdf4Player.isOpen();
#endif
}
