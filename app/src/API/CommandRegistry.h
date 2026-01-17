/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QMap>
#include <QString>
#include <functional>

#include "API/CommandProtocol.h"

namespace API
{
/**
 * @brief Function signature for command handlers
 *
 * Takes params as input, returns a CommandResponse
 */
using CommandFunction = std::function<CommandResponse(const QString &id,
                                                      const QJsonObject &params)>;

/**
 * @struct CommandDefinition
 * @brief Describes a registered command
 */
struct CommandDefinition
{
  QString name;
  QString description;
  CommandFunction handler;
};

/**
 * @class CommandRegistry
 * @brief Central registry for all available API commands
 *
 * The CommandRegistry maintains a mapping of command names to their handler
 * functions. Command handlers are registered during application startup by
 * the various handler classes (IOManagerHandler, UARTHandler, etc.).
 *
 * Commands use dot-notation matching the C++ namespace hierarchy:
 * - io.manager.connect -> IO::Manager operations
 * - io.driver.uart.setBaudRate -> IO::Drivers::UART operations
 * - io.driver.network.setTcpPort -> IO::Drivers::Network operations
 */
class CommandRegistry
{
private:
  CommandRegistry() = default;
  CommandRegistry(CommandRegistry &&) = delete;
  CommandRegistry(const CommandRegistry &) = delete;
  CommandRegistry &operator=(CommandRegistry &&) = delete;
  CommandRegistry &operator=(const CommandRegistry &) = delete;

public:
  /**
   * @brief Gets the singleton instance
   */
  static CommandRegistry &instance();

  /**
   * @brief Register a new command
   * @param name Command name (e.g., "io.manager.connect")
   * @param description Human-readable description
   * @param handler Function to execute when command is invoked
   */
  void registerCommand(const QString &name, const QString &description,
                       CommandFunction handler);

  /**
   * @brief Check if a command is registered
   * @param name Command name to check
   * @return true if command exists
   */
  [[nodiscard]] bool hasCommand(const QString &name) const;

  /**
   * @brief Execute a command by name
   * @param name Command name
   * @param id Request ID for response correlation
   * @param params Command parameters
   * @return CommandResponse with result or error
   */
  [[nodiscard]] CommandResponse execute(const QString &name, const QString &id,
                                        const QJsonObject &params);

  /**
   * @brief Get list of all registered command names
   * @return List of command names sorted alphabetically
   */
  [[nodiscard]] QStringList availableCommands() const;

  /**
   * @brief Get all command definitions
   * @return Map of command name to definition
   */
  [[nodiscard]] const QMap<QString, CommandDefinition> &commands() const;

private:
  QMap<QString, CommandDefinition> m_commands;
};

} // namespace API

