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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/ControlScriptHandler.h"

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all control-script commands with the registry.
 */
void API::Handlers::ControlScriptHandler::registerCommands()
{
  auto& registry   = CommandRegistry::instance();
  const auto empty = emptySchema();

  registry.registerCommand(
    QStringLiteral("controlscript.get"),
    QStringLiteral("Get the project's setup()/loop() control script source code."),
    empty,
    &getScript);

  registry.registerCommand(
    QStringLiteral("controlscript.set"),
    QStringLiteral("Replace the project's control script source (params: code). The script is "
                   "persisted in the project and applied to the live runtime; if a device is "
                   "connected it is recompiled and restarted immediately."),
    makeSchema({
      {QStringLiteral("code"), QStringLiteral("string"), QStringLiteral("Control script source")}
  }),
    &setScript);

  registry.registerCommand(
    QStringLiteral("controlscript.getStatus"),
    QStringLiteral("Returns whether the control script is currently running."),
    empty,
    &getStatus);
}

//--------------------------------------------------------------------------------------------------
// Handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current control-script source.
 */
API::CommandResponse API::Handlers::ControlScriptHandler::getScript(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  QJsonObject result;
  result[QStringLiteral("code")] = DataModel::ProjectModel::instance().controlScriptCode();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Replaces the control-script source through the project model.
 */
API::CommandResponse API::Handlers::ControlScriptHandler::setScript(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));
  }

  const QString code = params.value(QStringLiteral("code")).toString();
  DataModel::ProjectModel::instance().setControlScriptCode(code);

  QJsonObject result;
  result[QStringLiteral("running")] = DataModel::ControlScript::instance().running();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns the running state of the control script.
 */
API::CommandResponse API::Handlers::ControlScriptHandler::getStatus(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  QJsonObject result;
  result[QStringLiteral("running")] = DataModel::ControlScript::instance().running();
  return CommandResponse::makeSuccess(id, result);
}
