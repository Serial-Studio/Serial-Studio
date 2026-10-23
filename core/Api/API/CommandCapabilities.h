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
#include <QFlags>
#include <QJsonObject>
#include <QLatin1StringView>
#include <QStringView>
#include <span>

/**
 * @brief What each API command can do (spec 0098), the classes the remote gate decides by.
 *        Every registered command sits in exactly one table (tst_command_capability_registry),
 *        and a name in neither is unclassified, which the gate refuses: a new command cannot
 *        reach a remote client by being forgotten.
 */
namespace API::CommandCapabilities {
/**
 * @brief One thing a command can do that a remote caller must not get for free.
 */
enum Capability : quint16 {
  NoCapability      = 0x0000,
  ControlScriptOnly = 0x0001,
  InstallsScript    = 0x0002,
  LoadsProject      = 0x0004,
  WritesDevice      = 0x0008,
  ChoosesProgram    = 0x0010,
  StartsProgram     = 0x0020,
  RunsInertCode     = 0x0040,
  Dispatches        = 0x0080,
  DrivesUi          = 0x0100,
};
Q_DECLARE_FLAGS(Capabilities, Capability)
Q_DECLARE_OPERATORS_FOR_FLAGS(Capabilities)

/**
 * @brief A command with capabilities: those it always has, plus those its arguments can add.
 */
struct Entry {
  QLatin1StringView name;
  Capabilities always;
  Capabilities withArguments;
};

[[nodiscard]] std::optional<Capabilities> of(QStringView command, const QJsonObject& params);
[[nodiscard]] bool argumentsAddCapabilities(QStringView command, const QJsonObject& params);
[[nodiscard]] std::span<const Entry> capableCommands() noexcept;
[[nodiscard]] std::span<const QLatin1StringView> plainCommands() noexcept;
}  // namespace API::CommandCapabilities
