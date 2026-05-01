/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "API/Handlers/SessionsHandler.h"

#  include <QJsonArray>
#  include <QJsonObject>

#  include "API/CommandRegistry.h"
#  include "DataModel/ProjectModel.h"
#  include "Sessions/DatabaseManager.h"
#  include "Sessions/Export.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all session-recording commands.
 */
void API::Handlers::SessionsHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject emptySchema;
  emptySchema[QStringLiteral("type")]       = QStringLiteral("object");
  emptySchema[QStringLiteral("properties")] = QJsonObject();

  registry.registerCommand(
    QStringLiteral("sessions.getStatus"),
    QStringLiteral("Return the session-recording flags (exportEnabled, isOpen)"),
    emptySchema,
    &getStatus);

  // sessions.setExportEnabled -- {enabled: bool}
  {
    QJsonObject props;
    props[QStringLiteral("enabled")] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("boolean")                                     },
      {QStringLiteral("description"),
       QStringLiteral("Turn session recording on (true) or off (false)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("enabled")};
    registry.registerCommand(QStringLiteral("sessions.setExportEnabled"),
                             QStringLiteral("Turn session recording on/off (params: enabled)"),
                             schema,
                             &setExportEnabled);
  }

  registry.registerCommand(
    QStringLiteral("sessions.close"),
    QStringLiteral("Finalize and close the current session (no-op if none open)"),
    emptySchema,
    &close);

  // sessions.getCanonicalDbPath -- {projectTitle: string}
  {
    QJsonObject props;
    props[QStringLiteral("projectTitle")] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("string")                                     },
      {QStringLiteral("description"),
       QStringLiteral("Project title used for the session .db path")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("projectTitle")};
    registry.registerCommand(
      QStringLiteral("sessions.getCanonicalDbPath"),
      QStringLiteral("Return the canonical .db path for a given project title"),
      schema,
      &getCanonicalDbPath);
  }
}

//--------------------------------------------------------------------------------------------------
// Command implementations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports current session recording state.
 */
API::CommandResponse API::Handlers::SessionsHandler::getStatus(const QString& id,
                                                               const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& exp = Sessions::Export::instance();

  QJsonObject result;
  result[QStringLiteral("exportEnabled")] = exp.exportEnabled();
  result[QStringLiteral("isOpen")]        = exp.isOpen();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Turns session recording on or off.
 */
API::CommandResponse API::Handlers::SessionsHandler::setExportEnabled(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();
  Sessions::Export::instance().setExportEnabled(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  result[QStringLiteral("updated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Closes the current session file if one is open.
 */
API::CommandResponse API::Handlers::SessionsHandler::close(const QString& id,
                                                           const QJsonObject& params)
{
  Q_UNUSED(params)

  Sessions::Export::instance().closeFile();

  QJsonObject result;
  result[QStringLiteral("closed")] = true;
  result[QStringLiteral("isOpen")] = Sessions::Export::instance().isOpen();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns the canonical .db path used by the session recorder for a
 *        given project title. Useful for tests that need to verify the
 *        recorder wrote to the expected location.
 */
API::CommandResponse API::Handlers::SessionsHandler::getCanonicalDbPath(const QString& id,
                                                                        const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("projectTitle")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: projectTitle"));

  const auto title = params.value(QStringLiteral("projectTitle")).toString();
  const auto path  = Sessions::DatabaseManager::canonicalDbPath(title);

  QJsonObject result;
  result[QStringLiteral("projectTitle")] = title;
  result[QStringLiteral("path")]         = path;
  return CommandResponse::makeSuccess(id, result);
}

#endif  // BUILD_COMMERCIAL
