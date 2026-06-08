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
  registerLifecycleCommands();
  registerBrowsingCommands();
  registerTagCommands();
}

/**
 * @brief Register the recorder lifecycle commands (status, enable, close, dbPath).
 */
void API::Handlers::SessionsHandler::registerLifecycleCommands()
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
      QStringLiteral("sessions.getDbPath"),
      QStringLiteral("Return the canonical .db path for a given project title"),
      schema,
      &getCanonicalDbPath);
  }
}

/**
 * @brief Register database browsing/management commands (open, list, get, delete...).
 */
void API::Handlers::SessionsHandler::registerBrowsingCommands()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject emptySchema;
  emptySchema[QStringLiteral("type")]       = QStringLiteral("object");
  emptySchema[QStringLiteral("properties")] = QJsonObject();

  {
    QJsonObject props;
    props[QStringLiteral("filePath")] = QJsonObject{
      {       QStringLiteral("type"),                                QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Absolute path to the .db file to open")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("filePath")};
    registry.registerCommand(
      QStringLiteral("sessions.openDatabase"),
      QStringLiteral("Open a session database file for browsing (params: filePath)"),
      schema,
      &openDatabase);
  }

  registry.registerCommand(
    QStringLiteral("sessions.list"),
    QStringLiteral("List all sessions stored in the currently open database"),
    emptySchema,
    &list);

  {
    QJsonObject props;
    props[QStringLiteral("sessionId")] = QJsonObject{
      {       QStringLiteral("type"),                         QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Session id (from sessions.list)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("sessionId")};

    registry.registerCommand(
      QStringLiteral("sessions.get"),
      QStringLiteral("Return metadata + tags for one stored session (params: sessionId)"),
      schema,
      &get);
    registry.registerCommand(QStringLiteral("sessions.delete"),
                             QStringLiteral("Delete a stored session by id (params: sessionId)"),
                             schema,
                             &deleteSession);
    registry.registerCommand(
      QStringLiteral("sessions.replay"),
      QStringLiteral("Start the player on a stored session (params: sessionId)"),
      schema,
      &replay);
    registry.registerCommand(
      QStringLiteral("sessions.exportToCsv"),
      QStringLiteral("Export a stored session to CSV (async; params: sessionId)"),
      schema,
      &exportToCsv);
  }

  {
    QJsonObject props;
    props[QStringLiteral("sessionId")] = QJsonObject{
      {       QStringLiteral("type"),    QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Session id")}
    };
    props[QStringLiteral("notes")] = QJsonObject{
      {       QStringLiteral("type"),                          QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Free-form notes for the session")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")] =
      QJsonArray{QStringLiteral("sessionId"), QStringLiteral("notes")};
    registry.registerCommand(
      QStringLiteral("sessions.setNotes"),
      QStringLiteral("Set free-form notes on a stored session (params: sessionId, notes)"),
      schema,
      &setNotes);
  }
}

/**
 * @brief Register tag-related commands (list/add/delete/rename/assign/remove).
 */
void API::Handlers::SessionsHandler::registerTagCommands()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject emptySchema;
  emptySchema[QStringLiteral("type")]       = QStringLiteral("object");
  emptySchema[QStringLiteral("properties")] = QJsonObject();

  registry.registerCommand(QStringLiteral("sessions.listTags"),
                           QStringLiteral("List all tags defined in the open database"),
                           emptySchema,
                           &listTags);

  {
    QJsonObject props;
    props[QStringLiteral("label")] = QJsonObject{
      {       QStringLiteral("type"),    QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Tag label")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("label")};
    registry.registerCommand(QStringLiteral("sessions.addTag"),
                             QStringLiteral("Create a new tag (params: label)"),
                             schema,
                             &addTag);
  }

  {
    QJsonObject delProps;
    delProps[QStringLiteral("tagId")] = QJsonObject{
      {       QStringLiteral("type"), QStringLiteral("integer")},
      {QStringLiteral("description"),  QStringLiteral("Tag id")}
    };
    QJsonObject delSchema;
    delSchema[QStringLiteral("type")]       = QStringLiteral("object");
    delSchema[QStringLiteral("properties")] = delProps;
    delSchema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("tagId")};
    registry.registerCommand(QStringLiteral("sessions.deleteTag"),
                             QStringLiteral("Delete a tag by id (params: tagId)"),
                             delSchema,
                             &deleteTag);

    QJsonObject renProps                 = delProps;
    renProps[QStringLiteral("newLabel")] = QJsonObject{
      {       QStringLiteral("type"),    QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("New label")}
    };
    QJsonObject renSchema;
    renSchema[QStringLiteral("type")]       = QStringLiteral("object");
    renSchema[QStringLiteral("properties")] = renProps;
    renSchema[QStringLiteral("required")] =
      QJsonArray{QStringLiteral("tagId"), QStringLiteral("newLabel")};
    registry.registerCommand(QStringLiteral("sessions.renameTag"),
                             QStringLiteral("Rename a tag (params: tagId, newLabel)"),
                             renSchema,
                             &renameTag);
  }

  {
    QJsonObject props;
    props[QStringLiteral("sessionId")] = QJsonObject{
      {       QStringLiteral("type"),    QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Session id")}
    };
    props[QStringLiteral("tagId")] = QJsonObject{
      {       QStringLiteral("type"), QStringLiteral("integer")},
      {QStringLiteral("description"),  QStringLiteral("Tag id")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")] =
      QJsonArray{QStringLiteral("sessionId"), QStringLiteral("tagId")};
    registry.registerCommand(QStringLiteral("sessions.assignTag"),
                             QStringLiteral("Assign a tag to a session (params: sessionId, tagId)"),
                             schema,
                             &assignTag);
    registry.registerCommand(
      QStringLiteral("sessions.removeTag"),
      QStringLiteral("Remove a tag from a session (params: sessionId, tagId)"),
      schema,
      &removeTag);
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
 * @brief Returns the canonical .db path used by the session recorder for a given project title.
 * Useful for tests that need to verify the recorder wrote to the expected location.
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

//--------------------------------------------------------------------------------------------------
// Database browsing / management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Open a session database file by path.
 */
API::CommandResponse API::Handlers::SessionsHandler::openDatabase(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("filePath")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: filePath"));

  const auto filePath = params.value(QStringLiteral("filePath")).toString();
  Sessions::DatabaseManager::instance().openDatabase(filePath);

  QJsonObject result;
  result[QStringLiteral("filePath")] = filePath;
  result[QStringLiteral("opening")]  = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Return all sessions in the open database.
 */
API::CommandResponse API::Handlers::SessionsHandler::list(const QString& id,
                                                          const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& db = Sessions::DatabaseManager::instance();
  if (!db.isOpen())
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("No database open. Call sessions.openDatabase first."));

  QJsonObject result;
  result[QStringLiteral("isOpen")]   = db.isOpen();
  result[QStringLiteral("filePath")] = db.filePath();
  result[QStringLiteral("count")]    = db.sessionCount();
  result[QStringLiteral("sessions")] = QJsonArray::fromVariantList(db.sessionList());
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Return one session's metadata + tags.
 */
API::CommandResponse API::Handlers::SessionsHandler::get(const QString& id,
                                                         const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("sessionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: sessionId"));

  const int sessionId = params.value(QStringLiteral("sessionId")).toInt();
  auto& db            = Sessions::DatabaseManager::instance();
  if (!db.isOpen())
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("No database open. Call sessions.openDatabase first."));

  const auto meta = db.sessionMetadata(sessionId);
  if (meta.isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Session not found: %1").arg(sessionId));

  QJsonObject result;
  result[QStringLiteral("sessionId")] = sessionId;
  result[QStringLiteral("metadata")]  = QJsonObject::fromVariantMap(meta);
  result[QStringLiteral("tags")]      = QJsonArray::fromVariantList(db.tagsForSession(sessionId));
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete a stored session by id.
 */
API::CommandResponse API::Handlers::SessionsHandler::deleteSession(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("sessionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: sessionId"));

  const int sessionId = params.value(QStringLiteral("sessionId")).toInt();
  auto& db            = Sessions::DatabaseManager::instance();
  if (!db.isOpen())
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("No database open. Call sessions.openDatabase first."));

  db.confirmDeleteSession(sessionId);

  QJsonObject result;
  result[QStringLiteral("sessionId")] = sessionId;
  result[QStringLiteral("deleting")]  = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the free-form notes on a stored session.
 */
API::CommandResponse API::Handlers::SessionsHandler::setNotes(const QString& id,
                                                              const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("sessionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: sessionId"));

  if (!params.contains(QStringLiteral("notes")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: notes"));

  const int sessionId = params.value(QStringLiteral("sessionId")).toInt();
  const auto notes    = params.value(QStringLiteral("notes")).toString();
  auto& db            = Sessions::DatabaseManager::instance();
  db.setSelectedSessionId(sessionId);
  db.setSelectedSessionNotes(notes);

  QJsonObject result;
  result[QStringLiteral("sessionId")] = sessionId;
  result[QStringLiteral("updated")]   = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Replay a stored session through the player.
 */
API::CommandResponse API::Handlers::SessionsHandler::replay(const QString& id,
                                                            const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("sessionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: sessionId"));

  const int sessionId = params.value(QStringLiteral("sessionId")).toInt();
  auto& db            = Sessions::DatabaseManager::instance();
  db.setSelectedSessionId(sessionId);
  db.replaySelectedSession();

  QJsonObject result;
  result[QStringLiteral("sessionId")] = sessionId;
  result[QStringLiteral("replaying")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Export a session to CSV (async; check sessions.getStatus / busy).
 */
API::CommandResponse API::Handlers::SessionsHandler::exportToCsv(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("sessionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: sessionId"));

  const int sessionId = params.value(QStringLiteral("sessionId")).toInt();
  Sessions::DatabaseManager::instance().exportSessionToCsv(sessionId);

  QJsonObject result;
  result[QStringLiteral("sessionId")] = sessionId;
  result[QStringLiteral("exporting")] = true;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Tags
//--------------------------------------------------------------------------------------------------

/**
 * @brief List all tags defined in the open database.
 */
API::CommandResponse API::Handlers::SessionsHandler::listTags(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& db = Sessions::DatabaseManager::instance();
  if (!db.isOpen())
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("No database open. Call sessions.openDatabase first."));

  QJsonObject result;
  result[QStringLiteral("tags")] = QJsonArray::fromVariantList(db.tagList());
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Create a new tag.
 */
API::CommandResponse API::Handlers::SessionsHandler::addTag(const QString& id,
                                                            const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("label")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: label"));

  const auto label = params.value(QStringLiteral("label")).toString();
  Sessions::DatabaseManager::instance().addTag(label);

  QJsonObject result;
  result[QStringLiteral("label")] = label;
  result[QStringLiteral("added")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete a tag by id.
 */
API::CommandResponse API::Handlers::SessionsHandler::deleteTag(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("tagId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: tagId"));

  const int tagId = params.value(QStringLiteral("tagId")).toInt();
  Sessions::DatabaseManager::instance().deleteTag(tagId);

  QJsonObject result;
  result[QStringLiteral("tagId")]   = tagId;
  result[QStringLiteral("deleted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Rename a tag.
 */
API::CommandResponse API::Handlers::SessionsHandler::renameTag(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("tagId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: tagId"));

  if (!params.contains(QStringLiteral("newLabel")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: newLabel"));

  const int tagId     = params.value(QStringLiteral("tagId")).toInt();
  const auto newLabel = params.value(QStringLiteral("newLabel")).toString();
  Sessions::DatabaseManager::instance().renameTag(tagId, newLabel);

  QJsonObject result;
  result[QStringLiteral("tagId")]    = tagId;
  result[QStringLiteral("newLabel")] = newLabel;
  result[QStringLiteral("renamed")]  = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Attach a tag to a stored session.
 */
API::CommandResponse API::Handlers::SessionsHandler::assignTag(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("sessionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: sessionId"));

  if (!params.contains(QStringLiteral("tagId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: tagId"));

  const int sessionId = params.value(QStringLiteral("sessionId")).toInt();
  const int tagId     = params.value(QStringLiteral("tagId")).toInt();
  Sessions::DatabaseManager::instance().assignTagToSession(sessionId, tagId);

  QJsonObject result;
  result[QStringLiteral("sessionId")] = sessionId;
  result[QStringLiteral("tagId")]     = tagId;
  result[QStringLiteral("assigned")]  = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Detach a tag from a stored session.
 */
API::CommandResponse API::Handlers::SessionsHandler::removeTag(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("sessionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: sessionId"));

  if (!params.contains(QStringLiteral("tagId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: tagId"));

  const int sessionId = params.value(QStringLiteral("sessionId")).toInt();
  const int tagId     = params.value(QStringLiteral("tagId")).toInt();
  Sessions::DatabaseManager::instance().removeTagFromSession(sessionId, tagId);

  QJsonObject result;
  result[QStringLiteral("sessionId")] = sessionId;
  result[QStringLiteral("tagId")]     = tagId;
  result[QStringLiteral("removed")]   = true;
  return CommandResponse::makeSuccess(id, result);
}

#endif  // BUILD_COMMERCIAL
