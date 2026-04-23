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

#include <functional>
#include <QJsonObject>
#include <QMap>
#include <QString>

#include "API/CommandProtocol.h"

namespace API {
using CommandFunction =
  std::function<CommandResponse(const QString& id, const QJsonObject& params)>;

/**
 * @brief Describes a registered command.
 */
struct CommandDefinition {
  QString name;
  QString description;
  QJsonObject inputSchema;
  CommandFunction handler;
};

/**
 * @brief Central registry for all available API commands.
 */
class CommandRegistry {
private:
  CommandRegistry()                                  = default;
  CommandRegistry(CommandRegistry&&)                 = delete;
  CommandRegistry(const CommandRegistry&)            = delete;
  CommandRegistry& operator=(CommandRegistry&&)      = delete;
  CommandRegistry& operator=(const CommandRegistry&) = delete;

public:
  [[nodiscard]] static CommandRegistry& instance();

  void registerCommand(const QString& name, const QString& description, CommandFunction handler);
  void registerCommand(const QString& name,
                       const QString& description,
                       const QJsonObject& inputSchema,
                       CommandFunction handler);

  [[nodiscard]] bool hasCommand(const QString& name) const;
  [[nodiscard]] CommandResponse execute(const QString& name,
                                        const QString& id,
                                        const QJsonObject& params);

  [[nodiscard]] QStringList availableCommands() const;
  [[nodiscard]] const QMap<QString, CommandDefinition>& commands() const;

private:
  QMap<QString, CommandDefinition> m_commands;
};

}  // namespace API
