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

#include <QLatin1StringView>
#include <QStringView>
#include <span>

/**
 * @brief Which API commands a sealed (deployment package) session runs (spec 0096): live data,
 *        device control, console, exports, alarms, notifications, layout and the machine's own
 *        license are allowed; anything that returns or writes project content, or edits what the
 *        package configured, is denied. Every registered command sits in exactly one table
 *        (tst_sealed_policy_registry), and a name outside the allowed table is refused.
 */
namespace API::SealedPolicy {
[[nodiscard]] bool allowedWhenSealed(QStringView command) noexcept;
[[nodiscard]] std::span<const QLatin1StringView> allowedCommands() noexcept;
[[nodiscard]] std::span<const QLatin1StringView> deniedCommands() noexcept;
}  // namespace API::SealedPolicy
