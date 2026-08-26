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

#include <algorithm>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <QColor>
#include <QFile>
#include <QHash>
#include <QJSEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "API/CommandRegistry.h"
#include "API/EnumLabels.h"
#include "API/Handlers/ProjectHandler.h"
#include "API/PathPolicy.h"
#include "API/SchemaBuilder.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/CFrameParser.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/FrameParserPipeline.h"
#include "DataModel/Scripting/JsScriptEngine.h"
#include "DataModel/Scripting/LuaScriptEngine.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "IO/ConnectionManager.h"
#include "Misc/BackupManager.h"
#include "SerialStudio.h"
#ifdef BUILD_COMMERCIAL
#  include "UI/Widgets/Output/Base.h"
#endif
#include "ProjectApiSupport.h"

//--------------------------------------------------------------------------------------------------
// Output widget commands
//--------------------------------------------------------------------------------------------------

/**
 * @brief Add an output widget to the specified group.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetAdd(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const int type       = params.value(QStringLiteral("type")).toInt(0);
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  project.setSelectedGroup(groups[groupId]);
  project.addOutputControl(static_cast<SerialStudio::OutputWidgetType>(
    qBound(0, type, static_cast<int>(SerialStudio::OutputKnob))));

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[QStringLiteral("type")]    = type;
  result[QStringLiteral("added")]   = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete an output widget by id.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetDelete(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("widgetId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const int widgetId   = params.value(QStringLiteral("widgetId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (widgetId < 0 || static_cast<size_t>(widgetId) >= groups[groupId].outputWidgets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Widget id not found: %1 in group %2")
                                        .arg(QString::number(widgetId), QString::number(groupId)));

  project.deleteOutputWidget(groupId, widgetId);

  QJsonObject result;
  result[QStringLiteral("groupId")]  = groupId;
  result[QStringLiteral("widgetId")] = widgetId;
  result[QStringLiteral("deleted")]  = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate an output widget by id.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetDuplicate(const QString& id,
                                                                          const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("widgetId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const int widgetId   = params.value(QStringLiteral("widgetId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (widgetId < 0 || static_cast<size_t>(widgetId) >= groups[groupId].outputWidgets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Widget id not found: %1 in group %2")
                                        .arg(QString::number(widgetId), QString::number(groupId)));

  project.duplicateOutputWidget(groupId, widgetId);

  QJsonObject result;
  result[QStringLiteral("groupId")]    = groupId;
  result[QStringLiteral("widgetId")]   = widgetId;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Read the project configuration of one output widget.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetGet(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("widgetId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetId"));

  const int groupId         = params.value(QStringLiteral("groupId")).toInt();
  const int widgetId        = params.value(QStringLiteral("widgetId")).toInt();
  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto& groups        = projectModel.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& widgets = groups[groupId].outputWidgets;
  if (widgetId < 0 || static_cast<size_t>(widgetId) >= widgets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Widget id not found: %1 in group %2")
                                        .arg(QString::number(widgetId), QString::number(groupId)));

  const auto& w = widgets[widgetId];
  QJsonObject result;
  result[QStringLiteral("groupId")]          = groupId;
  result[QStringLiteral("widgetId")]         = w.widgetId;
  result[QStringLiteral("type")]             = static_cast<int>(w.type);
  result[QStringLiteral("title")]            = w.title;
  result[QStringLiteral("icon")]             = w.icon;
  result[QStringLiteral("monoIcon")]         = w.monoIcon;
  result[QStringLiteral("minValue")]         = w.minValue;
  result[QStringLiteral("maxValue")]         = w.maxValue;
  result[QStringLiteral("stepSize")]         = w.stepSize;
  result[QStringLiteral("initialValue")]     = w.initialValue;
  result[Keys::SourceId]                     = w.sourceId;
  result[QStringLiteral("txEncoding")]       = w.txEncoding;
  result[QStringLiteral("transmitFunction")] = w.transmitFunction;
  return CommandResponse::makeSuccess(id, result);
}
