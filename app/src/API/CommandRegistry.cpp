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

#include "API/CommandRegistry.h"

/**
 * @brief Gets the singleton instance of the CommandRegistry
 * @return Reference to the singleton instance
 */
API::CommandRegistry &API::CommandRegistry::instance()
{
  static CommandRegistry singleton;
  return singleton;
}

/**
 * @brief Register a new command handler
 * @param name Command name (dot-notation, e.g., "io.manager.connect")
 * @param description Human-readable description of what the command does
 * @param handler Function to call when the command is executed
 */
void API::CommandRegistry::registerCommand(const QString &name,
                                           const QString &description,
                                           CommandFunction handler)
{
  CommandDefinition def;
  def.name = name;
  def.description = description;
  def.handler = std::move(handler);
  m_commands.insert(name, def);
}

/**
 * @brief Check if a command is registered
 * @param name Command name to look up
 * @return true if the command exists in the registry
 */
bool API::CommandRegistry::hasCommand(const QString &name) const
{
  return m_commands.contains(name);
}

/**
 * @brief Execute a registered command
 * @param name Command name to execute
 * @param id Request ID for response correlation
 * @param params JSON parameters to pass to the command handler
 * @return CommandResponse containing success/failure and any result data
 */
API::CommandResponse API::CommandRegistry::execute(const QString &name,
                                                   const QString &id,
                                                   const QJsonObject &params)
{
  if (!hasCommand(name))
  {
    return CommandResponse::makeError(
        id, ErrorCode::UnknownCommand,
        QStringLiteral("Unknown command: %1").arg(name));
  }

  try
  {
    return m_commands[name].handler(id, params);
  }
  catch (const std::exception &e)
  {
    return CommandResponse::makeError(
        id, ErrorCode::ExecutionError,
        QStringLiteral("Command execution failed: %1").arg(e.what()));
  }
}

/**
 * @brief Get a sorted list of all available command names
 * @return QStringList of command names in alphabetical order
 */
QStringList API::CommandRegistry::availableCommands() const
{
  QStringList names = m_commands.keys();
  names.sort();
  return names;
}

/**
 * @brief Get direct access to all command definitions
 * @return Const reference to the internal command map
 */
const QMap<QString, API::CommandDefinition> &
API::CommandRegistry::commands() const
{
  return m_commands;
}

