/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QDateTime>

class QSettings;

namespace Licensing {
class SimpleCrypt;

namespace MonotonicClock {

/**
 * @brief Returns now floored at the highest wall-clock ever observed (anti clock-rewind).
 *
 * Shared by the online grace period and offline certificate expiry so both read
 * and advance the same encrypted `licensing/lastSeen` floor; rewinding the system
 * clock cannot revive an expired license through either path.
 */
[[nodiscard]] QDateTime now();

/**
 * @brief Core flooring over an explicit settings store and cipher (for testing).
 */
[[nodiscard]] QDateTime nowFloored(QSettings& settings, SimpleCrypt& crypt);

}  // namespace MonotonicClock
}  // namespace Licensing
