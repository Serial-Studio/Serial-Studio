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

#include <array>
#include <cstdint>

namespace Licensing {

/**
 * @brief Ed25519 PUBLIC verification key for offline license certificates.
 */
inline constexpr std::array<std::uint8_t, 32> kOfflinePublicKey = {
  0x13, 0x27, 0xc8, 0x96, 0x28, 0xab, 0xc4, 0x42, 0x53, 0x27, 0x7f, 0xf7, 0xf7, 0x04, 0x84, 0x48,
  0x75, 0xec, 0xc9, 0x0e, 0x57, 0x3c, 0x13, 0xe0, 0xcb, 0x5c, 0xce, 0x6b, 0x97, 0x26, 0x6e, 0xb4};
}  // namespace Licensing
