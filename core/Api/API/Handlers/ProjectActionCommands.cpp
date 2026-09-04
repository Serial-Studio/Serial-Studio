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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/ProjectActionCommands.h"

#include <QJsonObject>

#include "API/SchemaBuilder.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the command class to the registry it publishes into.
 */
API::Handlers::ProjectActionCommands::ProjectActionCommands(CommandRegistry& registry)
  : m_registry(registry)
{}

/**
 * @brief Register action CRUD commands.
 */
void API::Handlers::ProjectActionCommands::registerCommands()
{
  auto& registry   = m_registry;
  const auto empty = emptySchema();

  registry.registerCommand(
    QStringLiteral("project.action.add"),
    QStringLiteral("Create a new outgoing-action button shown on the toolbar. Actions "
                   "transmit a configurable payload (text or binary, with optional "
                   "EOL sequence) to the device on click, or repeat on a timer. After "
                   "creation, populate it with project.action.update {actionId, "
                   "title, txData, eolSequence, timerMode (0=Off, 1=AutoStart, "
                   "2=ToggleOnTrigger), timerIntervalMs, repeatCount, icon}. Common "
                   "uses: 'send AT command', 'request telemetry', 'reset device'."),
    empty,
    &actionAdd);
  registry.registerCommand(QStringLiteral("project.action.delete"),
                           QStringLiteral("Delete an action by id (params: actionId)"),
                           makeSchema({
                             {QStringLiteral("actionId"),
                              QStringLiteral("integer"),
                              QStringLiteral("Action id to delete")}
  }),
                           &actionDelete);
  registry.registerCommand(QStringLiteral("project.action.duplicate"),
                           QStringLiteral("Duplicate an action by id (params: actionId)"),
                           makeSchema({
                             {QStringLiteral("actionId"),
                              QStringLiteral("integer"),
                              QStringLiteral("Action id to duplicate")}
  }),
                           &actionDuplicate);
}

/**
 * @brief Add action
 */
API::CommandResponse API::Handlers::ProjectActionCommands::actionAdd(const QString& id,
                                                                     const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& projectModel = DataModel::ProjectModel::instance();
  projectModel.addAction();

  QJsonObject result;
  result[QStringLiteral("added")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete an action by id
 */
API::CommandResponse API::Handlers::ProjectActionCommands::actionDelete(const QString& id,
                                                                        const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("actionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: actionId"));

  const int actionId   = params.value(QStringLiteral("actionId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& actions  = project.actions();
  if (actionId < 0 || static_cast<size_t>(actionId) >= actions.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Action id not found: %1").arg(actionId));

  project.deleteAction(actionId);

  QJsonObject result;
  result[QStringLiteral("actionId")] = actionId;
  result[QStringLiteral("deleted")]  = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate an action by id
 */
API::CommandResponse API::Handlers::ProjectActionCommands::actionDuplicate(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("actionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: actionId"));

  const int actionId   = params.value(QStringLiteral("actionId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& actions  = project.actions();
  if (actionId < 0 || static_cast<size_t>(actionId) >= actions.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Action id not found: %1").arg(actionId));

  project.duplicateAction(actionId);

  QJsonObject result;
  result[QStringLiteral("actionId")]   = actionId;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}
