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

#include <cstddef>
#include <cstdint>

/**
 * @brief Verifies a detached Ed25519 signature (RFC 8032) over a message.
 *
 * Thin, crypto-provider-independent wrapper over the vendored TweetNaCl
 * implementation so offline license verification does not depend on whichever
 * OpenSSL/BoringSSL variant a given build happens to link.
 *
 * @param signature  the 64-byte detached signature.
 * @param message    the signed payload (may be null only when length is 0).
 * @param length     number of bytes in @p message.
 * @param publicKey  the 32-byte Ed25519 public key.
 * @return true when the signature is valid for the message and public key.
 */
[[nodiscard]] bool ed25519_verify(const std::uint8_t signature[64],
                                  const std::uint8_t* message,
                                  std::size_t length,
                                  const std::uint8_t publicKey[32]);
