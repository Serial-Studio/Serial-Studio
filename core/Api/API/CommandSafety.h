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

#include <optional>
#include <QLatin1StringView>
#include <QString>
#include <QStringView>
#include <span>

/**
 * @brief The assistant's per-command safety tiers (app/rcc/ai/command_safety.json), projected
 *        by scripts/generate-command-safety.py into CommandSafetyTable.cpp so the MCP server
 *        can say what a tool does before a client calls it. A hint describes; only the remote
 *        gate (spec 0098) authorizes, and a name the table lacks reads as a plain mutation,
 *        never as read-only.
 */
namespace API::CommandSafety {
/**
 * @brief The assistant's tier. A device-gated name carries the tier it resolves to once device
 *        control is enabled (AlwaysConfirm) plus the flag that says so.
 */
enum class Tier : quint8 {
  Safe,
  Confirm,
  AlwaysConfirm,
  Blocked,
};

/**
 * @brief One generated row: the command, its tier, and whether the device-control toggle gates it.
 */
struct Entry {
  QLatin1StringView name;
  Tier tier;
  bool device_gated;
};

/**
 * @brief What an MCP client may assume about a tool before calling it.
 */
struct ToolHints {
  bool read_only;
  bool destructive;
};

[[nodiscard]] std::span<const Entry> entries() noexcept;
[[nodiscard]] std::span<const QLatin1StringView> assistantTools() noexcept;
[[nodiscard]] std::optional<Tier> tierOf(QStringView command) noexcept;
[[nodiscard]] bool isDeviceGated(QStringView command) noexcept;
[[nodiscard]] ToolHints hintsFor(QStringView command) noexcept;
[[nodiscard]] bool isListedTool(QStringView command);
[[nodiscard]] QString humanizeToolName(QStringView command);
}  // namespace API::CommandSafety
