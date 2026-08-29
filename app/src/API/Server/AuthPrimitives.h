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

#include <QByteArray>
#include <QString>

namespace API {
namespace Auth {

/**
 * @brief Generates a cryptographically random hex token for external API auth.
 */
[[nodiscard]] QString generateToken();

/**
 * @brief Normalizes a caller-supplied token, returning an empty string when it is not a hex
 *        credential of at least 32 characters. Refusing is deliberate: a short or non-hex token
 *        would quietly weaken the credential that guards every non-loopback connection.
 */
[[nodiscard]] QString normalizeToken(const QString& token);

/**
 * @brief Compares two byte arrays in constant time to avoid token timing side channels.
 */
[[nodiscard]] bool constantTimeEquals(const QByteArray& a, const QByteArray& b);

/**
 * @brief Whether a command may only be issued by an in-process control script, never by a remote
 *        API client.
 */
[[nodiscard]] bool commandIsControlScriptOnly(const QString& command);

/**
 * @brief Whether a command reaches the connected hardware, and therefore has to clear the
 *        device-write consent gate before a remote client may run it.
 */
[[nodiscard]] bool commandWritesToDevice(const QString& command);

}  // namespace Auth
}  // namespace API
