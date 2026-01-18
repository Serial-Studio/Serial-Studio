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
#include "API/CommandRegistry.h"
#include "DataModel/ProjectModel.h"
#include "SerialStudio.h"

#include <QJsonArray>

/**
 * @brief Register all Project commands with the registry
 */
void API::Handlers::ProjectHandler::registerCommands()
{
  auto &registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("project.file.new"),
                           QStringLiteral("Create new project"), &fileNew);

  registry.registerCommand(
      QStringLiteral("project.file.open"),
      QStringLiteral("Open project file (params: filePath)"), &fileOpen);

  registry.registerCommand(
      QStringLiteral("project.file.save"),
      QStringLiteral("Save project (params: askPath=false)"), &fileSave);

  registry.registerCommand(QStringLiteral("project.getStatus"),
                           QStringLiteral("Get project status"), &getStatus);

  registry.registerCommand(
      QStringLiteral("project.group.add"),
      QStringLiteral("Add group (params: title, widgetType)"), &groupAdd);

  registry.registerCommand(QStringLiteral("project.group.delete"),
                           QStringLiteral("Delete current group"),
                           &groupDelete);

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

  registry.registerCommand(
      QStringLiteral("project.dataset.setOption"),
      QStringLiteral("Toggle dataset option (params: option, enabled)"),
      &datasetSetOption);

  registry.registerCommand(QStringLiteral("project.action.add"),
                           QStringLiteral("Add action"), &actionAdd);

  registry.registerCommand(QStringLiteral("project.action.delete"),
                           QStringLiteral("Delete current action"),
                           &actionDelete);

  registry.registerCommand(QStringLiteral("project.action.duplicate"),
                           QStringLiteral("Duplicate current action"),
                           &actionDuplicate);

  registry.registerCommand(
      QStringLiteral("project.parser.setCode"),
      QStringLiteral("Set frame parser code (params: code)"), &parserSetCode);

  registry.registerCommand(QStringLiteral("project.parser.getCode"),
                           QStringLiteral("Get frame parser code"),
                           &parserGetCode);

  registry.registerCommand(
      QStringLiteral("project.groups.list"),
      QStringLiteral("List all groups with dataset counts"), &groupsList);

  registry.registerCommand(
      QStringLiteral("project.datasets.list"),
      QStringLiteral("List all datasets across all groups"), &datasetsList);

  registry.registerCommand(QStringLiteral("project.actions.list"),
                           QStringLiteral("List all actions"), &actionsList);
}

/**
 * @brief Create new project
 */
API::CommandResponse
API::Handlers::ProjectHandler::fileNew(const QString &id,
                                       const QJsonObject &params)
{
  Q_UNUSED(params)

  DataModel::ProjectModel::instance().newJsonFile();

  QJsonObject result;
  result[QStringLiteral("created")] = true;
  result[QStringLiteral("title")] = DataModel::ProjectModel::instance().title();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Open project file
 * @param params Requires "filePath" (string)
 */
API::CommandResponse
API::Handlers::ProjectHandler::fileOpen(const QString &id,
                                        const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("filePath")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: filePath"));
  }

  const QString file_path = params.value(QStringLiteral("filePath")).toString();
  if (file_path.isEmpty())
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("filePath cannot be empty"));
  }

  DataModel::ProjectModel::instance().openJsonFile(file_path);

  QJsonObject result;
  result[QStringLiteral("filePath")]
      = DataModel::ProjectModel::instance().jsonFilePath();
  result[QStringLiteral("title")] = DataModel::ProjectModel::instance().title();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Save project
 * @param params Optional "askPath" (bool, default false)
 */
API::CommandResponse
API::Handlers::ProjectHandler::fileSave(const QString &id,
                                        const QJsonObject &params)
{
  const bool ask_path = params.contains(QStringLiteral("askPath"))
                            ? params.value(QStringLiteral("askPath")).toBool()
                            : false;

  const bool success
      = DataModel::ProjectModel::instance().saveJsonFile(ask_path);

  if (!success)
  {
    return CommandResponse::makeError(id, ErrorCode::OperationFailed,
                                      QStringLiteral("Failed to save project"));
  }

  QJsonObject result;
  result[QStringLiteral("saved")] = true;
  result[QStringLiteral("filePath")]
      = DataModel::ProjectModel::instance().jsonFilePath();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get project status
 */
API::CommandResponse
API::Handlers::ProjectHandler::getStatus(const QString &id,
                                         const QJsonObject &params)
{
  Q_UNUSED(params)

  auto &project = DataModel::ProjectModel::instance();

  QJsonObject result;
  result[QStringLiteral("title")] = project.title();
  result[QStringLiteral("filePath")] = project.jsonFilePath();
  result[QStringLiteral("modified")] = project.modified();
  result[QStringLiteral("groupCount")] = project.groupCount();
  result[QStringLiteral("datasetCount")] = project.datasetCount();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Add group
 * @param params Requires "title" (string) and "widgetType" (int)
 */
API::CommandResponse
API::Handlers::ProjectHandler::groupAdd(const QString &id,
                                        const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("title")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: title"));
  }

  if (!params.contains(QStringLiteral("widgetType")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: widgetType"));
  }

  const QString title = params.value(QStringLiteral("title")).toString();
  if (title.isEmpty())
  {
    return CommandResponse::makeError(id, ErrorCode::InvalidParam,
                                      QStringLiteral("title cannot be empty"));
  }

  const int widget_type = params.value(QStringLiteral("widgetType")).toInt();
  if (widget_type < 0 || widget_type > 6)
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("Invalid widgetType: must be 0-6"));
  }

  const auto widget = static_cast<SerialStudio::GroupWidget>(widget_type);
  DataModel::ProjectModel::instance().addGroup(title, widget);

  QJsonObject result;
  result[QStringLiteral("title")] = title;
  result[QStringLiteral("widgetType")] = widget_type;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete current group
 */
API::CommandResponse
API::Handlers::ProjectHandler::groupDelete(const QString &id,
                                           const QJsonObject &params)
{
  Q_UNUSED(params)

  DataModel::ProjectModel::instance().deleteCurrentGroup();

  QJsonObject result;
  result[QStringLiteral("deleted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate current group
 */
API::CommandResponse
API::Handlers::ProjectHandler::groupDuplicate(const QString &id,
                                              const QJsonObject &params)
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
API::CommandResponse
API::Handlers::ProjectHandler::datasetAdd(const QString &id,
                                          const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("options")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: options"));
  }

  const int options = params.value(QStringLiteral("options")).toInt();
  if (options < 0 || options > 0b00111111)
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("Invalid options: must be 0-63 (bit flags)"));
  }

  const auto dataset_options
      = static_cast<SerialStudio::DatasetOption>(options);
  DataModel::ProjectModel::instance().addDataset(dataset_options);

  QJsonObject result;
  result[QStringLiteral("options")] = options;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete current dataset
 */
API::CommandResponse
API::Handlers::ProjectHandler::datasetDelete(const QString &id,
                                             const QJsonObject &params)
{
  Q_UNUSED(params)

  DataModel::ProjectModel::instance().deleteCurrentDataset();

  QJsonObject result;
  result[QStringLiteral("deleted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate current dataset
 */
API::CommandResponse
API::Handlers::ProjectHandler::datasetDuplicate(const QString &id,
                                                const QJsonObject &params)
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
API::CommandResponse
API::Handlers::ProjectHandler::datasetSetOption(const QString &id,
                                                const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("option")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: option"));
  }

  if (!params.contains(QStringLiteral("enabled")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: enabled"));
  }

  const int option = params.value(QStringLiteral("option")).toInt();
  const bool enabled = params.value(QStringLiteral("enabled")).toBool();

  const auto dataset_option = static_cast<SerialStudio::DatasetOption>(option);
  DataModel::ProjectModel::instance().changeDatasetOption(dataset_option,
                                                          enabled);

  QJsonObject result;
  result[QStringLiteral("option")] = option;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Add action
 */
API::CommandResponse
API::Handlers::ProjectHandler::actionAdd(const QString &id,
                                         const QJsonObject &params)
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
API::CommandResponse
API::Handlers::ProjectHandler::actionDelete(const QString &id,
                                            const QJsonObject &params)
{
  Q_UNUSED(params)

  DataModel::ProjectModel::instance().deleteCurrentAction();

  QJsonObject result;
  result[QStringLiteral("deleted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate current action
 */
API::CommandResponse
API::Handlers::ProjectHandler::actionDuplicate(const QString &id,
                                               const QJsonObject &params)
{
  Q_UNUSED(params)

  DataModel::ProjectModel::instance().duplicateCurrentAction();

  QJsonObject result;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set frame parser code
 * @param params Requires "code" (string)
 */
API::CommandResponse
API::Handlers::ProjectHandler::parserSetCode(const QString &id,
                                             const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("code")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: code"));
  }

  const QString code = params.value(QStringLiteral("code")).toString();
  DataModel::ProjectModel::instance().setFrameParserCode(code);

  QJsonObject result;
  result[QStringLiteral("codeLength")] = code.length();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get frame parser code
 */
API::CommandResponse
API::Handlers::ProjectHandler::parserGetCode(const QString &id,
                                             const QJsonObject &params)
{
  Q_UNUSED(params)

  const QString code = DataModel::ProjectModel::instance().frameParserCode();

  QJsonObject result;
  result[QStringLiteral("code")] = code;
  result[QStringLiteral("codeLength")] = code.length();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief List all groups with basic info
 */
API::CommandResponse
API::Handlers::ProjectHandler::groupsList(const QString &id,
                                          const QJsonObject &params)
{
  Q_UNUSED(params)

  const auto &groups = DataModel::ProjectModel::instance().groups();

  QJsonArray groups_array;
  for (const auto &group : groups)
  {
    QJsonObject group_obj;
    group_obj[QStringLiteral("groupId")] = group.groupId;
    group_obj[QStringLiteral("title")] = group.title;
    group_obj[QStringLiteral("widget")] = group.widget;
    group_obj[QStringLiteral("datasetCount")]
        = static_cast<int>(group.datasets.size());
    groups_array.append(group_obj);
  }

  QJsonObject result;
  result[QStringLiteral("groups")] = groups_array;
  result[QStringLiteral("groupCount")] = static_cast<int>(groups.size());
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief List all datasets across all groups
 */
API::CommandResponse
API::Handlers::ProjectHandler::datasetsList(const QString &id,
                                            const QJsonObject &params)
{
  Q_UNUSED(params)

  const auto &groups = DataModel::ProjectModel::instance().groups();

  QJsonArray datasets_array;
  int total_datasets = 0;

  for (const auto &group : groups)
  {
    for (const auto &dataset : group.datasets)
    {
      QJsonObject dataset_obj;
      dataset_obj[QStringLiteral("groupId")] = group.groupId;
      dataset_obj[QStringLiteral("groupTitle")] = group.title;
      dataset_obj[QStringLiteral("index")] = dataset.index;
      dataset_obj[QStringLiteral("title")] = dataset.title;
      dataset_obj[QStringLiteral("units")] = dataset.units;
      dataset_obj[QStringLiteral("widget")] = dataset.widget;
      dataset_obj[QStringLiteral("value")] = dataset.value;
      datasets_array.append(dataset_obj);
      ++total_datasets;
    }
  }

  QJsonObject result;
  result[QStringLiteral("datasets")] = datasets_array;
  result[QStringLiteral("datasetCount")] = total_datasets;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief List all actions
 */
API::CommandResponse
API::Handlers::ProjectHandler::actionsList(const QString &id,
                                           const QJsonObject &params)
{
  Q_UNUSED(params)

  QJsonObject result;
  result[QStringLiteral("actions")] = QJsonArray();
  result[QStringLiteral("actionCount")] = 0;

  return CommandResponse::makeSuccess(id, result);
}
