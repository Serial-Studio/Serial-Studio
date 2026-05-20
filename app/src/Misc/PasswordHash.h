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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QString>

namespace Misc::PasswordHash {

/**
 * @brief Default PBKDF2-SHA256 iteration count. Tuned against OWASP 2023 guidance.
 */
inline constexpr int kPbkdf2Iterations = 600000;

/**
 * @brief Derives a versioned, salted PBKDF2-SHA256 hash from a plaintext password.
 *
 * Output format: pbkdf2-sha256$<iterations>$<saltB64>$<hashB64>
 *
 * Salt is 16 random bytes from the system CSPRNG; key is 32 bytes (SHA-256 wide).
 * The format leads with an algorithm tag so future migrations (argon2id) can
 * be detected and slotted in alongside existing hashes.
 */
[[nodiscard]] QString hashPassword(const QString& plain);

/**
 * @brief Constant-time verification against a stored hash string.
 *
 * Accepts both the new PBKDF2 format and the legacy 32-hex-char MD5 format
 * written by Serial Studio 3.2.x. Returns true on match for either format.
 * Callers can detect upgrade candidates via isLegacyHash().
 */
[[nodiscard]] bool verifyPassword(const QString& plain, const QString& storedHash);

/**
 * @brief True when storedHash matches the legacy 32-hex-char MD5 shape.
 */
[[nodiscard]] bool isLegacyHash(const QString& storedHash);

}  // namespace Misc::PasswordHash
