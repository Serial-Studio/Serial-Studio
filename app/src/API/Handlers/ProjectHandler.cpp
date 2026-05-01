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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/ProjectHandler.h"

#include <algorithm>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "API/CommandRegistry.h"
#include "API/PathPolicy.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/FrameParser.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all Project commands with the registry
 */
void API::Handlers::ProjectHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  // Empty schema for parameterless commands
  QJsonObject emptySchema;
  emptySchema[QStringLiteral("type")]       = QStringLiteral("object");
  emptySchema[QStringLiteral("properties")] = QJsonObject();

  registry.registerCommand(QStringLiteral("project.file.new"),
                           QStringLiteral("Create new project"),
                           emptySchema,
                           &fileNew);

  {
    QJsonObject props;
    props[QStringLiteral("title")] = QJsonObject{
      {       QStringLiteral("type"),        QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Project title")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("title")};
    registry.registerCommand(QStringLiteral("project.setTitle"),
                             QStringLiteral("Set project title (params: title)"),
                             schema,
                             &setTitle);
  }

  {
    QJsonObject props;
    props[QStringLiteral("filePath")] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("string")                                     },
      {QStringLiteral("description"),
       QStringLiteral("Absolute path to project file (.json or .ssproj)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("filePath")};
    registry.registerCommand(QStringLiteral("project.file.open"),
                             QStringLiteral("Open project file (params: filePath)"),
                             schema,
                             &fileOpen);
  }

  {
    QJsonObject props;
    props[QStringLiteral("filePath")] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("string")                                     },
      {QStringLiteral("description"),
       QStringLiteral("Absolute path to save to (headless save-as)")}
    };
    props[QStringLiteral("askPath")] = QJsonObject{
      {       QStringLiteral("type"),                                 QStringLiteral("boolean")},
      {QStringLiteral("description"), QStringLiteral("Show native save dialog (default false)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    registry.registerCommand(QStringLiteral("project.file.save"),
                             QStringLiteral("Save project (params: askPath=false)"),
                             schema,
                             &fileSave);
  }

  {
    QJsonObject props;
    props[QStringLiteral("config")] = QJsonObject{
      {       QStringLiteral("type"),                            QStringLiteral("object")},
      {QStringLiteral("description"), QStringLiteral("Project configuration JSON object")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("config")};
    registry.registerCommand(QStringLiteral("project.loadFromJSON"),
                             QStringLiteral("Load project from JSON object (params: config)"),
                             schema,
                             &loadFromJSON);
  }

  registry.registerCommand(QStringLiteral("project.getStatus"),
                           QStringLiteral("Get project status"),
                           emptySchema,
                           &getStatus);

  {
    QJsonObject props;
    props[QStringLiteral("title")] = QJsonObject{
      {       QStringLiteral("type"),      QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Group title")}
    };
    props[QStringLiteral("widgetType")] = QJsonObject{
      {       QStringLiteral("type"),                 QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Widget type index (0-6)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")] =
      QJsonArray{QStringLiteral("title"), QStringLiteral("widgetType")};
    registry.registerCommand(QStringLiteral("project.group.add"),
                             QStringLiteral("Add group (params: title, widgetType)"),
                             schema,
                             &groupAdd);
  }

  registry.registerCommand(QStringLiteral("project.group.delete"),
                           QStringLiteral("Delete current group"),
                           emptySchema,
                           &groupDelete);

  registry.registerCommand(QStringLiteral("project.group.duplicate"),
                           QStringLiteral("Duplicate current group"),
                           emptySchema,
                           &groupDuplicate);

  // project.group.select -- set the current group for delete/duplicate.
  {
    QJsonObject props;
    props[QStringLiteral("groupId")] = QJsonObject{
      {       QStringLiteral("type"),            QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Group id to select")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("groupId")};
    registry.registerCommand(
      QStringLiteral("project.group.select"),
      QStringLiteral("Select a group by id so next delete/duplicate targets it"),
      schema,
      &groupSelect);
  }

  {
    QJsonObject props;
    props[QStringLiteral("options")] = QJsonObject{
      {       QStringLiteral("type"),                         QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Dataset option bit flags (0-63)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("options")};
    registry.registerCommand(QStringLiteral("project.dataset.add"),
                             QStringLiteral("Add dataset (params: options)"),
                             schema,
                             &datasetAdd);
  }

  registry.registerCommand(QStringLiteral("project.dataset.delete"),
                           QStringLiteral("Delete current dataset"),
                           emptySchema,
                           &datasetDelete);

  registry.registerCommand(QStringLiteral("project.dataset.duplicate"),
                           QStringLiteral("Duplicate current dataset"),
                           emptySchema,
                           &datasetDuplicate);

  {
    QJsonObject props;
    props[QStringLiteral("option")] = QJsonObject{
      {       QStringLiteral("type"),                       QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Dataset option flag to toggle")}
    };
    props[QStringLiteral("enabled")] = QJsonObject{
      {       QStringLiteral("type"),                                 QStringLiteral("boolean")},
      {QStringLiteral("description"), QStringLiteral("Whether to enable or disable the option")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")] =
      QJsonArray{QStringLiteral("option"), QStringLiteral("enabled")};
    registry.registerCommand(QStringLiteral("project.dataset.setOption"),
                             QStringLiteral("Toggle dataset option (params: option, enabled)"),
                             schema,
                             &datasetSetOption);
  }

  // project.dataset.setVirtual -- gate the frame-index field
  {
    QJsonObject props;
    props[Keys::GroupId] = QJsonObject{
      {       QStringLiteral("type"),         QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Owning group id")}
    };
    props[Keys::DatasetId] = QJsonObject{
      {       QStringLiteral("type"),                     QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Dataset id within the group")}
    };
    props[Keys::Virtual] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("boolean")                                     },
      {QStringLiteral("description"),
       QStringLiteral("Mark dataset as virtual (computed by transform)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{
      QString(Keys::GroupId),
      QString(Keys::DatasetId),
      QString(Keys::Virtual),
    };
    registry.registerCommand(
      QStringLiteral("project.dataset.setVirtual"),
      QStringLiteral("Toggle the virtual flag on a dataset (params: groupId, datasetId, virtual)"),
      schema,
      &datasetSetVirtual);
  }

  // project.dataset.setTransformCode -- set Lua/JS transform source
  {
    QJsonObject props;
    props[Keys::GroupId] = QJsonObject{
      {       QStringLiteral("type"),         QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Owning group id")}
    };
    props[Keys::DatasetId] = QJsonObject{
      {       QStringLiteral("type"),                     QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Dataset id within the group")}
    };
    props[QStringLiteral("code")] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("string")                                     },
      {QStringLiteral("description"),
       QStringLiteral("Transform source (Lua or JS matching source language)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{
      QString(Keys::GroupId),
      QString(Keys::DatasetId),
      QStringLiteral("code"),
    };
    registry.registerCommand(
      QStringLiteral("project.dataset.setTransformCode"),
      QStringLiteral("Set dataset transformCode (params: groupId, datasetId, code). Empty clears."),
      schema,
      &datasetSetTransformCode);
  }

  registry.registerCommand(
    QStringLiteral("project.action.add"), QStringLiteral("Add action"), emptySchema, &actionAdd);

  registry.registerCommand(QStringLiteral("project.action.delete"),
                           QStringLiteral("Delete current action"),
                           emptySchema,
                           &actionDelete);

  registry.registerCommand(QStringLiteral("project.action.duplicate"),
                           QStringLiteral("Duplicate current action"),
                           emptySchema,
                           &actionDuplicate);

  // Output widget management
  {
    QJsonObject props;
    props[QStringLiteral("type")] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("integer")   },
      {QStringLiteral("description"),
       QStringLiteral("OutputWidgetType enum: 0=Button, 1=Slider, 2=Toggle, "
       "3=TextField, 4=Knob")}
    };
    QJsonObject owSchema;
    owSchema[QStringLiteral("type")]       = QStringLiteral("object");
    owSchema[QStringLiteral("properties")] = props;
    owSchema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("type")};
    registry.registerCommand(QStringLiteral("project.outputWidget.add"),
                             QStringLiteral("Add output widget of the given type to the "
                                            "currently selected group"),
                             owSchema,
                             &outputWidgetAdd);
  }

  registry.registerCommand(QStringLiteral("project.outputWidget.delete"),
                           QStringLiteral("Delete current output widget"),
                           emptySchema,
                           &outputWidgetDelete);

  registry.registerCommand(QStringLiteral("project.outputWidget.duplicate"),
                           QStringLiteral("Duplicate current output widget"),
                           emptySchema,
                           &outputWidgetDuplicate);

  {
    QJsonObject props;
    props[QStringLiteral("code")] = QJsonObject{
      {       QStringLiteral("type"),                               QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Frame parser script code (JS or Lua)")}
    };
    props[Keys::SourceId] = QJsonObject{
      {       QStringLiteral("type"),                  QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source index (default 0)")}
    };
    props[QStringLiteral("language")] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("integer")                      },
      {QStringLiteral("description"),
       QStringLiteral("Optional: 0 = JavaScript, 1 = Lua. When supplied, the "
       "source language is flipped before the code is validated "
       "and script errors are returned as API errors.")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("code")};
    registry.registerCommand(QStringLiteral("project.parser.setCode"),
                             QStringLiteral("Set frame parser code (params: code, "
                                            "optional sourceId, optional language)"),
                             schema,
                             &parserSetCode);

    registry.registerCommand(QStringLiteral("project.frameParser.setCode"),
                             QStringLiteral("Set frame parser code (params: code, "
                                            "optional sourceId, optional language)"),
                             schema,
                             &parserSetCode);
  }

  {
    QJsonObject props;
    props[Keys::SourceId] = QJsonObject{
      {       QStringLiteral("type"),                  QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source index (default 0)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    registry.registerCommand(QStringLiteral("project.parser.getCode"),
                             QStringLiteral("Get frame parser code"),
                             schema,
                             &parserGetCode);
  }

  {
    QJsonObject props;
    props[QStringLiteral("language")] = QJsonObject{
      {       QStringLiteral("type"),                                  QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Script language: 0 = JavaScript, 1 = Lua")}
    };
    props[Keys::SourceId] = QJsonObject{
      {       QStringLiteral("type"),                       QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source identifier (default 0)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("language")};
    registry.registerCommand(QStringLiteral("project.frameParser.setLanguage"),
                             QStringLiteral("Set the script language used by the frame parser "
                                            "for a given source (params: language, sourceId)"),
                             schema,
                             &parserSetLanguage);
  }

  {
    QJsonObject props;
    props[Keys::SourceId] = QJsonObject{
      {       QStringLiteral("type"),                       QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source identifier (default 0)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    registry.registerCommand(QStringLiteral("project.frameParser.getLanguage"),
                             QStringLiteral("Get the script language used by the frame parser "
                                            "for a given source"),
                             schema,
                             &parserGetLanguage);
  }

  {
    QJsonObject props;
    props[Keys::SourceId] = QJsonObject{
      {       QStringLiteral("type"),                  QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source index (default 0)")}
    };
    props[QStringLiteral("startSequence")] = QJsonObject{
      {       QStringLiteral("type"),                QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Frame start delimiter")}
    };
    props[QStringLiteral("endSequence")] = QJsonObject{
      {       QStringLiteral("type"),              QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Frame end delimiter")}
    };
    props[Keys::ChecksumAlgorithm] = QJsonObject{
      {       QStringLiteral("type"),                  QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Checksum algorithm name")}
    };
    props[Keys::FrameDetection] = QJsonObject{
      {       QStringLiteral("type"),                    QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Frame detection mode (0-3)")}
    };
    props[QStringLiteral("operationMode")] = QJsonObject{
      {       QStringLiteral("type"),              QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Operation mode (0-2)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    registry.registerCommand(
      QStringLiteral("project.frameParser.configure"),
      QStringLiteral("Configure frame parser settings (params: startSequence, endSequence, "
                     "checksumAlgorithm, frameDetection, operationMode)"),
      schema,
      &frameParserConfigure);
  }

  registry.registerCommand(QStringLiteral("project.frameParser.getConfig"),
                           QStringLiteral("Get frame parser configuration"),
                           emptySchema,
                           &frameParserGetConfig);

  registry.registerCommand(QStringLiteral("project.exportJson"),
                           QStringLiteral("Export project as JSON"),
                           emptySchema,
                           &exportJson);

  registry.registerCommand(QStringLiteral("project.loadIntoFrameBuilder"),
                           QStringLiteral("Load current project into FrameBuilder"),
                           emptySchema,
                           &loadIntoFrameBuilder);

  registry.registerCommand(QStringLiteral("project.groups.list"),
                           QStringLiteral("List all groups with dataset counts"),
                           emptySchema,
                           &groupsList);

  registry.registerCommand(QStringLiteral("project.datasets.list"),
                           QStringLiteral("List all datasets across all groups"),
                           emptySchema,
                           &datasetsList);

  registry.registerCommand(QStringLiteral("project.actions.list"),
                           QStringLiteral("List all actions"),
                           emptySchema,
                           &actionsList);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Create new project
 */
API::CommandResponse API::Handlers::ProjectHandler::fileNew(const QString& id,
                                                            const QJsonObject& params)
{
  Q_UNUSED(params)

  DataModel::ProjectModel::instance().newJsonFile();

  QJsonObject result;
  result[QStringLiteral("created")] = true;
  result[QStringLiteral("title")]   = DataModel::ProjectModel::instance().title();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set project title
 */
API::CommandResponse API::Handlers::ProjectHandler::setTitle(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("title"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: title"));
  }

  const QString title = params.value(QStringLiteral("title")).toString();
  if (title.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("title cannot be empty"));
  }

  DataModel::ProjectModel::instance().setTitle(title);

  QJsonObject result;
  result[QStringLiteral("title")] = DataModel::ProjectModel::instance().title();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Open project file
 * @param params Requires "filePath" (string)
 */
API::CommandResponse API::Handlers::ProjectHandler::fileOpen(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("filePath"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: filePath"));
  }

  const QString file_path = params.value(QStringLiteral("filePath")).toString();
  if (file_path.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("filePath cannot be empty"));
  }

  if (!API::isPathAllowed(file_path)) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("filePath is not allowed"));
  }

  DataModel::ProjectModel::instance().setSuppressMessageBoxes(true);
  const bool ok = DataModel::ProjectModel::instance().openJsonFile(file_path);
  DataModel::ProjectModel::instance().setSuppressMessageBoxes(false);

  if (!ok) {
    return CommandResponse::makeError(
      id,
      ErrorCode::OperationFailed,
      QStringLiteral("Failed to open project file (validation or I/O error)"));
  }

  AppState::instance().setOperationMode(SerialStudio::ProjectFile);

  QJsonObject result;
  result[QStringLiteral("filePath")] = DataModel::ProjectModel::instance().jsonFilePath();
  result[QStringLiteral("title")]    = DataModel::ProjectModel::instance().title();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Save project
 * @param params Optional "askPath" (bool, default false)
 */
API::CommandResponse API::Handlers::ProjectHandler::fileSave(const QString& id,
                                                             const QJsonObject& params)
{
  // When filePath is provided, save directly to that path (headless save-as)
  const QString explicit_path = params.value(QStringLiteral("filePath")).toString();

  DataModel::ProjectModel::instance().setSuppressMessageBoxes(true);
  bool success = false;
  if (!explicit_path.isEmpty()) {
    if (!API::isPathAllowed(explicit_path)) {
      DataModel::ProjectModel::instance().setSuppressMessageBoxes(false);
      return CommandResponse::makeError(
        id, ErrorCode::InvalidParam, QStringLiteral("filePath is not allowed"));
    }

    success = DataModel::ProjectModel::instance().apiSaveJsonFile(explicit_path);
  } else {
    const bool ask_path = params.value(QStringLiteral("askPath")).toBool(false);
    success             = DataModel::ProjectModel::instance().saveJsonFile(ask_path);
  }

  DataModel::ProjectModel::instance().setSuppressMessageBoxes(false);

  if (!success) {
    return CommandResponse::makeError(
      id, ErrorCode::OperationFailed, QStringLiteral("Failed to save project"));
  }

  QJsonObject result;
  result[QStringLiteral("saved")]    = true;
  result[QStringLiteral("filePath")] = DataModel::ProjectModel::instance().jsonFilePath();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get project status
 */
API::CommandResponse API::Handlers::ProjectHandler::getStatus(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& project = DataModel::ProjectModel::instance();

  QJsonObject result;
  result[QStringLiteral("title")]        = project.title();
  result[QStringLiteral("filePath")]     = project.jsonFilePath();
  result[QStringLiteral("modified")]     = project.modified();
  result[QStringLiteral("groupCount")]   = project.groupCount();
  result[QStringLiteral("datasetCount")] = project.datasetCount();
  result[QStringLiteral("actionCount")]  = static_cast<int>(project.actions().size());

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Add group
 * @param params Requires "title" (string) and "widgetType" (int)
 */
API::CommandResponse API::Handlers::ProjectHandler::groupAdd(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("title"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: title"));
  }

  if (!params.contains(QStringLiteral("widgetType"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetType"));
  }

  const QString title = params.value(QStringLiteral("title")).toString();
  if (title.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("title cannot be empty"));
  }

  const int widget_type = params.value(QStringLiteral("widgetType")).toInt();
  if (widget_type < 0 || widget_type > 6) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid widgetType: must be 0-6"));
  }

  const auto widget = static_cast<SerialStudio::GroupWidget>(widget_type);
  DataModel::ProjectModel::instance().addGroup(title, widget);

  QJsonObject result;
  result[QStringLiteral("title")]      = title;
  result[QStringLiteral("widgetType")] = widget_type;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete current group
 */
API::CommandResponse API::Handlers::ProjectHandler::groupDelete(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& project = DataModel::ProjectModel::instance();
  project.setSuppressMessageBoxes(true);
  project.deleteCurrentGroup();
  project.setSuppressMessageBoxes(false);

  QJsonObject result;
  result[QStringLiteral("deleted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate current group
 */
API::CommandResponse API::Handlers::ProjectHandler::groupDuplicate(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  DataModel::ProjectModel::instance().duplicateCurrentGroup();

  QJsonObject result;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select a group by id so subsequent mutations target it.
 *
 * ProjectModel's delete/duplicate operations act on @c m_selectedGroup. The
 * editor UI keeps this in sync via tree selection; over the API we need to
 * set it explicitly before calling group.delete / group.duplicate.
 */
API::CommandResponse API::Handlers::ProjectHandler::groupSelect(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId  = params.value(QStringLiteral("groupId")).toInt();
  const auto& groups = DataModel::ProjectModel::instance().groups();

  // Look up by logical groupId rather than vector index.
  const auto it = std::find_if(
    groups.begin(), groups.end(), [groupId](const auto& g) { return g.groupId == groupId; });

  if (it == groups.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  DataModel::ProjectModel::instance().setSelectedGroup(*it);

  QJsonObject result;
  result[QStringLiteral("groupId")]  = groupId;
  result[QStringLiteral("selected")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Add dataset
 * @param params Requires "options" (int, bit flags)
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetAdd(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("options"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: options"));
  }

  const int options = params.value(QStringLiteral("options")).toInt();
  if (options < 0 || options > 0b00111111) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid options: must be 0-63 (bit flags)"));
  }

  const auto dataset_options = static_cast<SerialStudio::DatasetOption>(options);
  DataModel::ProjectModel::instance().addDataset(dataset_options);

  QJsonObject result;
  result[QStringLiteral("options")] = options;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete current dataset
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetDelete(const QString& id,
                                                                  const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& project = DataModel::ProjectModel::instance();
  project.setSuppressMessageBoxes(true);
  project.deleteCurrentDataset();
  project.setSuppressMessageBoxes(false);

  QJsonObject result;
  result[QStringLiteral("deleted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate current dataset
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetDuplicate(const QString& id,
                                                                     const QJsonObject& params)
{
  Q_UNUSED(params)

  DataModel::ProjectModel::instance().duplicateCurrentDataset();

  QJsonObject result;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Toggle dataset option
 * @param params Requires "option" (int) and "enabled" (bool)
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetOption(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("option"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: option"));
  }

  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const int option   = params.value(QStringLiteral("option")).toInt();
  const bool enabled = params.value(QStringLiteral("enabled")).toBool();

  const auto dataset_option = static_cast<SerialStudio::DatasetOption>(option);
  DataModel::ProjectModel::instance().changeDatasetOption(dataset_option, enabled);

  QJsonObject result;
  result[QStringLiteral("option")]  = option;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Dataset field setters (v3.3)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Toggle the @c virtual_ flag on the dataset identified by
 *        (groupId, datasetId).
 *
 * Looks up the dataset in ProjectModel, applies the flag, and calls
 * updateDataset so signal listeners and the tree rebuild. The rebuildTree
 * flag is true so the ProjectEditor tree reflects the change immediately.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetVirtual(const QString& id,
                                                                      const QJsonObject& params)
{
  // Validate required parameters
  const QStringList required{
    QString(Keys::GroupId),
    QString(Keys::DatasetId),
    QString(Keys::Virtual),
  };

  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  const bool isVirt   = params.value(Keys::Virtual).toBool();

  // Locate the group by logical id, then the dataset within it
  auto& pm           = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  const auto git     = std::find_if(
    groups.begin(), groups.end(), [groupId](const auto& g) { return g.groupId == groupId; });

  if (git == groups.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = git->datasets;
  const auto dit       = std::find_if(datasets.begin(), datasets.end(), [datasetId](const auto& d) {
    return d.datasetId == datasetId;
  });

  if (dit == datasets.end())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Dataset id not found in group: %1/%2").arg(groupId).arg(datasetId));

  // Apply the flag -- updateDataset validates, emits signals, and rebuilds the tree
  DataModel::Dataset updated = *dit;
  updated.virtual_           = isVirt;
  pm.updateDataset(groupId, datasetId, updated, true);

  QJsonObject result;
  result[Keys::GroupId]             = groupId;
  result[Keys::DatasetId]           = datasetId;
  result[Keys::Virtual]             = isVirt;
  result[QStringLiteral("updated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the per-dataset transform code (Lua or JS; language matches the
 *        dataset's owning source).
 *
 * An empty code string clears the transform. FrameBuilder recompiles transforms
 * on the next connect / syncFromProjectModel call.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetTransformCode(
  const QString& id, const QJsonObject& params)
{
  const QStringList required{
    QString(Keys::GroupId),
    QString(Keys::DatasetId),
    QStringLiteral("code"),
  };

  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  const QString code  = params.value(QStringLiteral("code")).toString();

  // Locate the group by logical id, then the dataset within it
  auto& pm           = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  const auto git     = std::find_if(
    groups.begin(), groups.end(), [groupId](const auto& g) { return g.groupId == groupId; });

  if (git == groups.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = git->datasets;
  const auto dit       = std::find_if(datasets.begin(), datasets.end(), [datasetId](const auto& d) {
    return d.datasetId == datasetId;
  });

  if (dit == datasets.end())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Dataset id not found in group: %1/%2").arg(groupId).arg(datasetId));

  DataModel::Dataset updated = *dit;
  updated.transformCode      = code;
  pm.updateDataset(groupId, datasetId, updated, false);

  QJsonObject result;
  result[Keys::GroupId]                = groupId;
  result[Keys::DatasetId]              = datasetId;
  result[QStringLiteral("codeLength")] = code.size();
  result[QStringLiteral("updated")]    = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Add action
 */
API::CommandResponse API::Handlers::ProjectHandler::actionAdd(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  DataModel::ProjectModel::instance().addAction();

  QJsonObject result;
  result[QStringLiteral("added")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete current action
 */
API::CommandResponse API::Handlers::ProjectHandler::actionDelete(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& project = DataModel::ProjectModel::instance();
  project.setSuppressMessageBoxes(true);
  project.deleteCurrentAction();
  project.setSuppressMessageBoxes(false);

  QJsonObject result;
  result[QStringLiteral("deleted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate current action
 */
API::CommandResponse API::Handlers::ProjectHandler::actionDuplicate(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  DataModel::ProjectModel::instance().duplicateCurrentAction();

  QJsonObject result;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Output widget commands
//--------------------------------------------------------------------------------------------------

/**
 * @brief Add an output control to the project.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetAdd(const QString& id,
                                                                    const QJsonObject& params)
{
  const int type = params.value(QStringLiteral("type")).toInt(0);

  DataModel::ProjectModel::instance().addOutputControl(static_cast<SerialStudio::OutputWidgetType>(
    qBound(0, type, static_cast<int>(SerialStudio::OutputKnob))));

  QJsonObject result;
  result[QStringLiteral("added")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete the currently selected output widget.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetDelete(const QString& id,
                                                                       const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& project = DataModel::ProjectModel::instance();
  project.setSuppressMessageBoxes(true);
  project.deleteCurrentOutputWidget();
  project.setSuppressMessageBoxes(false);

  QJsonObject result;
  result[QStringLiteral("deleted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate the currently selected output widget.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetDuplicate(const QString& id,
                                                                          const QJsonObject& params)
{
  Q_UNUSED(params)

  DataModel::ProjectModel::instance().duplicateCurrentOutputWidget();

  QJsonObject result;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set frame parser code for a source.
 *
 * When the optional "language" parameter is supplied, the project model
 * language for the target source is switched atomically before the code is
 * validated, and validation errors (syntax, missing parse(), probe runtime
 * errors) surface as API errors rather than silently succeeding. When
 * "language" is absent the legacy fire-and-forget behaviour is preserved
 * for backwards compatibility with existing clients.
 *
 * @param params Requires "code" (string). Optional Keys::SourceId (int, default 0),
 *               "language" (int: 0 = JavaScript, 1 = Lua).
 */
API::CommandResponse API::Handlers::ProjectHandler::parserSetCode(const QString& id,
                                                                  const QJsonObject& params)
{
  // Validate required "code" parameter
  if (!params.contains(QStringLiteral("code"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));
  }

  // Resolve code and sourceId, validate bounds
  const QString code = params.value(QStringLiteral("code")).toString();
  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;
  auto& model        = DataModel::ProjectModel::instance();
  const int srcCount = static_cast<int>(model.sources().size());

  if (sourceId < 0 || sourceId >= srcCount)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid sourceId"));

  // Optional language flip -- validate under the new engine and roll back if invalid.
  const bool hasLanguage = params.contains(QStringLiteral("language"));
  int savedLanguage      = 0;
  if (hasLanguage) {
    const int language = params.value(QStringLiteral("language")).toInt();
    if (language != SerialStudio::JavaScript && language != SerialStudio::Lua)
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Invalid language: must be 0 (JavaScript) or 1 (Lua)"));

    // Snapshot prior language for rollback on validation failure.
    savedLanguage = model.frameParserLanguage(sourceId);

    // Flip the language so the engine dispatch picks the right implementation.
    model.updateSourceFrameParserLanguage(sourceId, language);

    // Suppress modal dialogs during headless API-driven validation.
    auto& parser            = DataModel::FrameParser::instance();
    const bool prevSuppress = model.suppressMessageBoxes();
    model.setSuppressMessageBoxes(true);
    parser.setSuppressMessageBoxes(true);

    // Validate + load the script under the new engine.
    const bool ok = parser.loadScript(sourceId, code, false);

    parser.setSuppressMessageBoxes(prevSuppress);
    model.setSuppressMessageBoxes(prevSuppress);

    // Roll back the language flip on validation failure.
    if (!ok) {
      model.updateSourceFrameParserLanguage(sourceId, savedLanguage);
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Script engine rejected the parser code (check logs)"));
    }
  }

  // Persist: source 0 -> setFrameParserCode; source >0 -> updateSourceFrameParser.
  if (sourceId == 0)
    model.setFrameParserCode(code);
  else
    model.updateSourceFrameParser(sourceId, code);

  QJsonObject result;
  result[Keys::SourceId]               = sourceId;
  result[QStringLiteral("codeLength")] = code.length();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get frame parser code for a source.
 * @param params Optional Keys::SourceId (int, default 0).
 */
API::CommandResponse API::Handlers::ProjectHandler::parserGetCode(const QString& id,
                                                                  const QJsonObject& params)
{
  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;
  const auto& model  = DataModel::ProjectModel::instance();
  const int srcCount = static_cast<int>(model.sources().size());

  if (sourceId < 0 || sourceId >= srcCount)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid sourceId"));

  const QString code =
    sourceId == 0 ? model.frameParserCode() : model.sources()[sourceId].frameParserCode;

  QJsonObject result;
  result[Keys::SourceId]               = sourceId;
  result[QStringLiteral("code")]       = code;
  result[QStringLiteral("codeLength")] = code.length();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the scripting language for a frame parser source.
 *
 * @param params Requires "language" (int: 0 = JavaScript, 1 = Lua).
 *               Optional Keys::SourceId (int, default 0).
 */
API::CommandResponse API::Handlers::ProjectHandler::parserSetLanguage(const QString& id,
                                                                      const QJsonObject& params)
{
  // Validate required "language" parameter
  if (!params.contains(QStringLiteral("language")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: language"));

  // Resolve sourceId (logical, default 0)
  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;

  // Validate language value against the SerialStudio::ScriptLanguage enum
  const int language = params.value(QStringLiteral("language")).toInt();
  if (language != SerialStudio::JavaScript && language != SerialStudio::Lua)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid language: must be 0 (JavaScript) or 1 (Lua)"));

  // Locate the source by logical ID (not by vector index)
  auto& model         = DataModel::ProjectModel::instance();
  const auto& sources = model.sources();
  const auto it =
    std::find_if(sources.begin(), sources.end(), [sourceId](const DataModel::Source& s) {
      return s.sourceId == sourceId;
    });

  // Reject unknown source IDs
  if (it == sources.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown sourceId"));

  model.updateSourceFrameParserLanguage(sourceId, language);

  // Reset parser code to the default template for the new language.
  DataModel::FrameParser::instance().loadDefaultTemplate(sourceId, true);

  QJsonObject result;
  result[Keys::SourceId]             = sourceId;
  result[QStringLiteral("language")] = language;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the scripting language for a frame parser source.
 *
 * @param params Optional Keys::SourceId (int, default 0).
 */
API::CommandResponse API::Handlers::ProjectHandler::parserGetLanguage(const QString& id,
                                                                      const QJsonObject& params)
{
  // Resolve sourceId (logical, default 0)
  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;

  // Verify the source exists before reporting a language
  const auto& model   = DataModel::ProjectModel::instance();
  const auto& sources = model.sources();
  const auto it =
    std::find_if(sources.begin(), sources.end(), [sourceId](const DataModel::Source& s) {
      return s.sourceId == sourceId;
    });

  // Reject unknown source IDs
  if (it == sources.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown sourceId"));

  QJsonObject result;
  result[Keys::SourceId]             = sourceId;
  result[QStringLiteral("language")] = it->frameParserLanguage;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief List all groups with basic info
 *
 * Uses existing serialize() function from Frame.h
 */
API::CommandResponse API::Handlers::ProjectHandler::groupsList(const QString& id,
                                                               const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& groups = DataModel::ProjectModel::instance().groups();

  QJsonArray groups_array;
  for (const auto& group : groups)
    groups_array.append(DataModel::serialize(group));

  QJsonObject result;
  result[QStringLiteral("groups")]     = groups_array;
  result[QStringLiteral("groupCount")] = static_cast<int>(groups.size());
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief List all datasets across all groups
 *
 * Uses existing serialize() function from Frame.h and adds groupId/groupTitle
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetsList(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& groups = DataModel::ProjectModel::instance().groups();

  QJsonArray datasets_array;
  int total_datasets = 0;

  for (const auto& group : groups) {
    for (const auto& dataset : group.datasets) {
      QJsonObject dataset_obj                   = DataModel::serialize(dataset);
      dataset_obj[QStringLiteral("groupId")]    = group.groupId;
      dataset_obj[QStringLiteral("groupTitle")] = group.title;
      datasets_array.append(dataset_obj);
      ++total_datasets;
    }
  }

  QJsonObject result;
  result[QStringLiteral("datasets")]     = datasets_array;
  result[QStringLiteral("datasetCount")] = total_datasets;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief List all actions
 */
API::CommandResponse API::Handlers::ProjectHandler::actionsList(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& actions = DataModel::ProjectModel::instance().actions();

  QJsonArray actions_array;
  for (const auto& action : actions) {
    QJsonObject obj;
    obj[QStringLiteral("actionId")] = action.actionId;
    obj[QStringLiteral("title")]    = action.title;
    obj[QStringLiteral("icon")]     = action.icon;
    obj[QStringLiteral("txData")]   = action.txData;
    actions_array.append(obj);
  }

  QJsonObject result;
  result[QStringLiteral("actions")]     = actions_array;
  result[QStringLiteral("actionCount")] = static_cast<int>(actions.size());

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Load project configuration from JSON object
 * @param params Requires "config" (JSON object with project configuration)
 *
 * Uses existing ProjectModel::openJsonFile() by writing to a temporary file.
 */
API::CommandResponse API::Handlers::ProjectHandler::loadFromJSON(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("config"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: config"));
  }

  const QJsonObject config = params.value(QStringLiteral("config")).toObject();
  if (config.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("config cannot be empty"));
  }

  // In-memory load -- no temp file, no file association
  auto& project = DataModel::ProjectModel::instance();
  project.setSuppressMessageBoxes(true);
  const bool ok = project.loadFromJsonDocument(QJsonDocument(config));
  project.setSuppressMessageBoxes(false);

  if (!ok) {
    return CommandResponse::makeError(
      id,
      ErrorCode::OperationFailed,
      QStringLiteral("Failed to load project from JSON (validation error)"));
  }

  QJsonObject result;
  result[QStringLiteral("loaded")]       = true;
  result[QStringLiteral("title")]        = project.title();
  result[QStringLiteral("groupCount")]   = project.groupCount();
  result[QStringLiteral("datasetCount")] = project.datasetCount();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Configure frame parser settings for a specific source.
 *
 * Optional Keys::SourceId (int, default 0) selects which source to configure.
 * When sourceId == 0 the top-level ProjectModel setters are used so that
 * the ConnectionManager's FrameReader is updated immediately. For sourceId > 0
 * the Source struct is updated directly via updateSource().
 *
 * @param params Optional: sourceId, startSequence, endSequence,
 *               checksumAlgorithm, frameDetection, operationMode.
 */
API::CommandResponse API::Handlers::ProjectHandler::frameParserConfigure(const QString& id,
                                                                         const QJsonObject& params)
{
  // Obtain instance and validate state
  auto& model   = DataModel::ProjectModel::instance();
  auto& manager = IO::ConnectionManager::instance();
  bool updated  = false;

  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;
  const int srcCount = static_cast<int>(model.sources().size());

  if (sourceId < 0 || (!model.sources().empty() && sourceId >= srcCount))
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid sourceId"));

  if (params.contains(QStringLiteral("operationMode"))) {
    const int modeIdx = params.value(QStringLiteral("operationMode")).toInt();
    if (modeIdx >= 0 && modeIdx <= 2) {
      AppState::instance().setOperationMode(static_cast<SerialStudio::OperationMode>(modeIdx));
      updated = true;
    }
  }

  if (sourceId == 0) {
    // Route through top-level setters -- they sync into source[0] and update the FrameReader.
    if (params.contains(QStringLiteral("startSequence"))) {
      const QString start = params.value(QStringLiteral("startSequence")).toString();
      manager.setStartSequence(start.toUtf8());
      model.setFrameStartSequence(start);
      updated = true;
    }

    if (params.contains(QStringLiteral("endSequence"))) {
      const QString end = params.value(QStringLiteral("endSequence")).toString();
      manager.setFinishSequence(end.toUtf8());
      model.setFrameEndSequence(end);
      updated = true;
    }

    if (params.contains(Keys::ChecksumAlgorithm)) {
      const QString checksumName = params.value(Keys::ChecksumAlgorithm).toString();
      manager.setChecksumAlgorithm(checksumName);
      model.setChecksumAlgorithm(checksumName);
      updated = true;
    }

    if (params.contains(Keys::FrameDetection)) {
      const int detectionIdx = params.value(Keys::FrameDetection).toInt();
      if (detectionIdx >= 0 && detectionIdx <= 3) {
        model.setFrameDetection(static_cast<SerialStudio::FrameDetection>(detectionIdx));
        updated = true;
      }
    }
  } else {
    DataModel::Source src = model.sources()[sourceId];

    if (params.contains(QStringLiteral("startSequence"))) {
      src.frameStart = params.value(QStringLiteral("startSequence")).toString();
      updated        = true;
    }

    if (params.contains(QStringLiteral("endSequence"))) {
      src.frameEnd = params.value(QStringLiteral("endSequence")).toString();
      updated      = true;
    }

    if (params.contains(Keys::ChecksumAlgorithm)) {
      src.checksumAlgorithm = params.value(Keys::ChecksumAlgorithm).toString();
      updated               = true;
    }

    if (params.contains(Keys::FrameDetection)) {
      const int detectionIdx = params.value(Keys::FrameDetection).toInt();
      if (detectionIdx >= 0 && detectionIdx <= 3) {
        src.frameDetection = detectionIdx;
        updated            = true;
      }
    }

    if (updated)
      model.updateSource(sourceId, src);
  }

  if (updated && sourceId == 0)
    manager.resetFrameReader();

  QJsonObject result;
  result[QStringLiteral("updated")] = updated;
  result[Keys::SourceId]            = sourceId;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get current frame parser configuration
 */
API::CommandResponse API::Handlers::ProjectHandler::frameParserGetConfig(const QString& id,
                                                                         const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& cfg = AppState::instance().frameConfig();

  // Build the multi-source arrays
  QJsonArray startArr, endArr;
  for (const auto& s : cfg.startSequences)
    startArr.append(QString::fromUtf8(s));

  for (const auto& f : cfg.finishSequences)
    endArr.append(QString::fromUtf8(f));

  // Primary (source[0]) delimiters as singular scalars -- single-source clients
  const QString primaryStart =
    cfg.startSequences.isEmpty() ? QString() : QString::fromUtf8(cfg.startSequences.first());
  const QString primaryEnd =
    cfg.finishSequences.isEmpty() ? QString() : QString::fromUtf8(cfg.finishSequences.first());

  // Assemble the response
  QJsonObject result;
  result[QStringLiteral("startSequence")]  = primaryStart;
  result[QStringLiteral("endSequence")]    = primaryEnd;
  result[QStringLiteral("startSequences")] = startArr;
  result[QStringLiteral("endSequences")]   = endArr;
  result[Keys::ChecksumAlgorithm]          = cfg.checksumAlgorithm;
  result[QStringLiteral("operationMode")]  = static_cast<int>(cfg.operationMode);
  result[Keys::FrameDetection]             = static_cast<int>(cfg.frameDetection);

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Export current project configuration as JSON
 */
API::CommandResponse API::Handlers::ProjectHandler::exportJson(const QString& id,
                                                               const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& project          = DataModel::ProjectModel::instance();
  const QJsonObject json = project.serializeToJson();

  QJsonObject result;
  result[QStringLiteral("config")] = json;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Load current project JSON into FrameBuilder
 *
 * This method exports the current ProjectModel configuration as JSON and loads
 * it into FrameBuilder, enabling API-modified projects to be used without
 * requiring a file on disk.
 */
API::CommandResponse API::Handlers::ProjectHandler::loadIntoFrameBuilder(const QString& id,
                                                                         const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& project = DataModel::ProjectModel::instance();
  auto& builder = DataModel::FrameBuilder::instance();

  const bool hasImageGroup =
    std::any_of(project.groups().begin(), project.groups().end(), [](const DataModel::Group& g) {
      return g.widget == QLatin1String("image");
    });

  if (project.groupCount() == 0 || (project.datasetCount() == 0 && !hasImageGroup)) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Project has no groups or datasets"));
  }

  builder.syncFromProjectModel();

  QJsonObject result;
  result[QStringLiteral("loaded")]       = true;
  result[QStringLiteral("source")]       = QStringLiteral("API");
  result[QStringLiteral("title")]        = project.title();
  result[QStringLiteral("groupCount")]   = project.groupCount();
  result[QStringLiteral("datasetCount")] = project.datasetCount();

  return CommandResponse::makeSuccess(id, result);
}
