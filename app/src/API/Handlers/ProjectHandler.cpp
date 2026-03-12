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

#include "API/Handlers/ProjectHandler.h"

#include <algorithm>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryFile>

#include "API/CommandRegistry.h"
#include "API/PathPolicy.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
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

  registry.registerCommand(
    QStringLiteral("project.file.new"), QStringLiteral("Create new project"), &fileNew);

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

  registry.registerCommand(QStringLiteral("project.file.save"),
                           QStringLiteral("Save project (params: askPath=false)"),
                           &fileSave);

  registry.registerCommand(QStringLiteral("project.loadFromJSON"),
                           QStringLiteral("Load project from JSON object (params: config)"),
                           &loadFromJSON);

  registry.registerCommand(
    QStringLiteral("project.getStatus"), QStringLiteral("Get project status"), &getStatus);

  registry.registerCommand(QStringLiteral("project.group.add"),
                           QStringLiteral("Add group (params: title, widgetType)"),
                           &groupAdd);

  registry.registerCommand(
    QStringLiteral("project.group.delete"), QStringLiteral("Delete current group"), &groupDelete);

  registry.registerCommand(QStringLiteral("project.group.duplicate"),
                           QStringLiteral("Duplicate current group"),
                           &groupDuplicate);

  registry.registerCommand(QStringLiteral("project.dataset.add"),
                           QStringLiteral("Add dataset (params: options)"),
                           &datasetAdd);

  registry.registerCommand(QStringLiteral("project.dataset.delete"),
                           QStringLiteral("Delete current dataset"),
                           &datasetDelete);

  registry.registerCommand(QStringLiteral("project.dataset.duplicate"),
                           QStringLiteral("Duplicate current dataset"),
                           &datasetDuplicate);

  registry.registerCommand(QStringLiteral("project.dataset.setOption"),
                           QStringLiteral("Toggle dataset option (params: option, enabled)"),
                           &datasetSetOption);

  registry.registerCommand(
    QStringLiteral("project.action.add"), QStringLiteral("Add action"), &actionAdd);

  registry.registerCommand(QStringLiteral("project.action.delete"),
                           QStringLiteral("Delete current action"),
                           &actionDelete);

  registry.registerCommand(QStringLiteral("project.action.duplicate"),
                           QStringLiteral("Duplicate current action"),
                           &actionDuplicate);

  registry.registerCommand(QStringLiteral("project.parser.setCode"),
                           QStringLiteral("Set frame parser code (params: code)"),
                           &parserSetCode);

  registry.registerCommand(QStringLiteral("project.frameParser.setCode"),
                           QStringLiteral("Set frame parser code (params: code)"),
                           &parserSetCode);

  registry.registerCommand(QStringLiteral("project.parser.getCode"),
                           QStringLiteral("Get frame parser code"),
                           &parserGetCode);

  registry.registerCommand(
    QStringLiteral("project.frameParser.configure"),
    QStringLiteral(
      "Configure frame parser settings (params: startSequence, endSequence, checksumAlgorithm, frameDetection, operationMode)"),
    &frameParserConfigure);

  registry.registerCommand(QStringLiteral("project.frameParser.getConfig"),
                           QStringLiteral("Get frame parser configuration"),
                           &frameParserGetConfig);

  registry.registerCommand(
    QStringLiteral("project.exportJson"), QStringLiteral("Export project as JSON"), &exportJson);

  registry.registerCommand(QStringLiteral("project.loadIntoFrameBuilder"),
                           QStringLiteral("Load current project into FrameBuilder"),
                           &loadIntoFrameBuilder);

  registry.registerCommand(QStringLiteral("project.groups.list"),
                           QStringLiteral("List all groups with dataset counts"),
                           &groupsList);

  registry.registerCommand(QStringLiteral("project.datasets.list"),
                           QStringLiteral("List all datasets across all groups"),
                           &datasetsList);

  registry.registerCommand(
    QStringLiteral("project.actions.list"), QStringLiteral("List all actions"), &actionsList);
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
  DataModel::ProjectModel::instance().openJsonFile(file_path);
  AppState::instance().setOperationMode(SerialStudio::ProjectFile);
  DataModel::ProjectModel::instance().setSuppressMessageBoxes(false);

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
  const bool ask_path = params.contains(QStringLiteral("askPath"))
                        ? params.value(QStringLiteral("askPath")).toBool()
                        : false;

  DataModel::ProjectModel::instance().setSuppressMessageBoxes(true);
  const bool success = DataModel::ProjectModel::instance().saveJsonFile(ask_path);
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

/**
 * @brief Set frame parser code for a source.
 * @param params Requires "code" (string). Optional "sourceId" (int, default 0).
 */
API::CommandResponse API::Handlers::ProjectHandler::parserSetCode(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));
  }

  const QString code = params.value(QStringLiteral("code")).toString();
  const int sourceId = params.contains(QStringLiteral("sourceId"))
                       ? params.value(QStringLiteral("sourceId")).toInt()
                       : 0;
  auto& model        = DataModel::ProjectModel::instance();
  const int srcCount = static_cast<int>(model.sources().size());

  if (sourceId < 0 || sourceId >= srcCount)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid sourceId"));

  if (sourceId == 0)
    model.setFrameParserCode(code);
  else
    model.updateSourceFrameParser(sourceId, code);

  QJsonObject result;
  result[QStringLiteral("sourceId")]   = sourceId;
  result[QStringLiteral("codeLength")] = code.length();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get frame parser code for a source.
 * @param params Optional "sourceId" (int, default 0).
 */
API::CommandResponse API::Handlers::ProjectHandler::parserGetCode(const QString& id,
                                                                  const QJsonObject& params)
{
  const int sourceId = params.contains(QStringLiteral("sourceId"))
                       ? params.value(QStringLiteral("sourceId")).toInt()
                       : 0;
  const auto& model  = DataModel::ProjectModel::instance();
  const int srcCount = static_cast<int>(model.sources().size());

  if (sourceId < 0 || sourceId >= srcCount)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid sourceId"));

  const QString code =
    sourceId == 0 ? model.frameParserCode() : model.sources()[sourceId].frameParserCode;

  QJsonObject result;
  result[QStringLiteral("sourceId")]   = sourceId;
  result[QStringLiteral("code")]       = code;
  result[QStringLiteral("codeLength")] = code.length();
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

  // Write to temporary file and use existing openJsonFile()
  QTemporaryFile tempFile;
  tempFile.setAutoRemove(false);

  if (!tempFile.open()) {
    return CommandResponse::makeError(
      id, ErrorCode::OperationFailed, QStringLiteral("Failed to create temporary file"));
  }

  const QJsonDocument doc(config);
  tempFile.write(doc.toJson(QJsonDocument::Indented));
  const QString tempPath = tempFile.fileName();
  tempFile.close();

  // Suppress messageboxes during API call
  DataModel::ProjectModel::instance().setSuppressMessageBoxes(true);
  DataModel::ProjectModel::instance().openJsonFile(tempPath);
  DataModel::ProjectModel::instance().setSuppressMessageBoxes(false);

  // Clean up
  QFile::remove(tempPath);
  DataModel::ProjectModel::instance().clearJsonFilePath();

  auto& project = DataModel::ProjectModel::instance();
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
 * Optional "sourceId" (int, default 0) selects which source to configure.
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
  auto& model   = DataModel::ProjectModel::instance();
  auto& manager = IO::ConnectionManager::instance();
  bool updated  = false;

  const int sourceId = params.contains(QStringLiteral("sourceId"))
                       ? params.value(QStringLiteral("sourceId")).toInt()
                       : 0;
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
    // Route through top-level setters — they sync into source[0] and update the FrameReader.
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

    if (params.contains(QStringLiteral("checksumAlgorithm"))) {
      const QString checksumName = params.value(QStringLiteral("checksumAlgorithm")).toString();
      manager.setChecksumAlgorithm(checksumName);
      model.setChecksumAlgorithm(checksumName);
      updated = true;
    }

    if (params.contains(QStringLiteral("frameDetection"))) {
      const int detectionIdx = params.value(QStringLiteral("frameDetection")).toInt();
      if (detectionIdx >= 0 && detectionIdx <= 3) {
        model.setFrameDetection(static_cast<SerialStudio::FrameDetection>(detectionIdx));
        updated = true;
      }
    }
  } else {
    // Update the source struct directly and trigger a device reconfigure.
    DataModel::Source src = model.sources()[sourceId];

    if (params.contains(QStringLiteral("startSequence"))) {
      src.frameStart = params.value(QStringLiteral("startSequence")).toString();
      updated        = true;
    }

    if (params.contains(QStringLiteral("endSequence"))) {
      src.frameEnd = params.value(QStringLiteral("endSequence")).toString();
      updated      = true;
    }

    if (params.contains(QStringLiteral("checksumAlgorithm"))) {
      src.checksumAlgorithm = params.value(QStringLiteral("checksumAlgorithm")).toString();
      updated               = true;
    }

    if (params.contains(QStringLiteral("frameDetection"))) {
      const int detectionIdx = params.value(QStringLiteral("frameDetection")).toInt();
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
  result[QStringLiteral("updated")]  = updated;
  result[QStringLiteral("sourceId")] = sourceId;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get current frame parser configuration
 */
API::CommandResponse API::Handlers::ProjectHandler::frameParserGetConfig(const QString& id,
                                                                         const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& manager = IO::ConnectionManager::instance();
  QJsonObject result;
  result[QStringLiteral("startSequence")]     = QString::fromUtf8(manager.startSequence());
  result[QStringLiteral("endSequence")]       = QString::fromUtf8(manager.finishSequence());
  result[QStringLiteral("checksumAlgorithm")] = manager.checksumAlgorithm();
  result[QStringLiteral("operationMode")] = static_cast<int>(AppState::instance().operationMode());
  result[QStringLiteral("frameDetection")] =
    static_cast<int>(DataModel::ProjectModel::instance().frameDetection());

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Export current project configuration as JSON
 */
API::CommandResponse API::Handlers::ProjectHandler::exportJson(const QString& id,
                                                               const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& project = DataModel::ProjectModel::instance();

  // Serialize project using the public method
  const QJsonObject json = project.serializeToJson();

  // Return the project JSON
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
