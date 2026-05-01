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

#include "API/Handlers/WorkspacesHandler.h"

#include <algorithm>
#include <QJsonArray>
#include <QJsonObject>

#include "API/CommandRegistry.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"

//--------------------------------------------------------------------------------------------------
// Local helpers
//--------------------------------------------------------------------------------------------------

namespace {

/**
 * @brief Locates a workspace by id in ProjectModel's list.
 * @return Iterator; end() if not found.
 */
[[nodiscard]] auto findWorkspace(const std::vector<DataModel::Workspace>& ws, int wid)
{
  return std::find_if(ws.begin(), ws.end(), [wid](const auto& w) { return w.workspaceId == wid; });
}

/**
 * @brief Serialises a workspace's widget refs as a QJsonArray of objects.
 */
[[nodiscard]] QJsonArray refsToJson(const std::vector<DataModel::WidgetRef>& refs)
{
  QJsonArray arr;
  for (const auto& r : refs) {
    QJsonObject entry;
    entry[QStringLiteral("widgetType")]    = r.widgetType;
    entry[QStringLiteral("groupId")]       = r.groupId;
    entry[QStringLiteral("relativeIndex")] = r.relativeIndex;
    arr.append(entry);
  }

  return arr;
}

}  // anonymous namespace

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all workspace-related commands with the registry.
 */
void API::Handlers::WorkspacesHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject emptySchema;
  emptySchema[QStringLiteral("type")]       = QStringLiteral("object");
  emptySchema[QStringLiteral("properties")] = QJsonObject();

  // project.workspaces.list
  registry.registerCommand(QStringLiteral("project.workspaces.list"),
                           QStringLiteral("List all workspaces with widget counts"),
                           emptySchema,
                           &list);

  // project.workspaces.get -- id required
  {
    QJsonObject props;
    props[QStringLiteral("id")] = QJsonObject{
      {       QStringLiteral("type"),      QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Workspace id")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("id")};
    registry.registerCommand(QStringLiteral("project.workspaces.get"),
                             QStringLiteral("Return widget refs for a workspace (params: id)"),
                             schema,
                             &get);
  }

  // project.workspaces.add
  {
    QJsonObject props;
    props[QStringLiteral("title")] = QJsonObject{
      {       QStringLiteral("type"),          QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Workspace title")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    registry.registerCommand(
      QStringLiteral("project.workspaces.add"),
      QStringLiteral("Create a new workspace (params: title=\"Workspace\"). Returns new id."),
      schema,
      &add);
  }

  // project.workspaces.delete
  {
    QJsonObject props;
    props[QStringLiteral("id")] = QJsonObject{
      {       QStringLiteral("type"),      QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Workspace id")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("id")};
    registry.registerCommand(QStringLiteral("project.workspaces.delete"),
                             QStringLiteral("Delete a workspace (params: id)"),
                             schema,
                             &remove);
  }

  // project.workspaces.rename
  {
    QJsonObject props;
    props[QStringLiteral("id")] = QJsonObject{
      {       QStringLiteral("type"),      QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Workspace id")}
    };
    props[QStringLiteral("title")] = QJsonObject{
      {       QStringLiteral("type"),              QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("New workspace title")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")] = QJsonArray{QStringLiteral("id"), QStringLiteral("title")};
    registry.registerCommand(QStringLiteral("project.workspaces.rename"),
                             QStringLiteral("Rename a workspace (params: id, title)"),
                             schema,
                             &rename);
  }

  // project.workspaces.autoGenerate
  registry.registerCommand(
    QStringLiteral("project.workspaces.autoGenerate"),
    QStringLiteral(
      "Materialise synthetic workspaces into the customised set. No-op if already customised."),
    emptySchema,
    &autoGenerate);

  // project.workspaces.customize.get
  registry.registerCommand(QStringLiteral("project.workspaces.customize.get"),
                           QStringLiteral("Return the customizeWorkspaces flag"),
                           emptySchema,
                           &customizeGet);

  // project.workspaces.customize.set
  {
    QJsonObject props;
    props[QStringLiteral("enabled")] = QJsonObject{
      {       QStringLiteral("type"),                          QStringLiteral("boolean")},
      {QStringLiteral("description"), QStringLiteral("Enable (true) or disable (false)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("enabled")};
    registry.registerCommand(QStringLiteral("project.workspaces.customize.set"),
                             QStringLiteral("Flip the customizeWorkspaces flag (params: enabled)"),
                             schema,
                             &customizeSet);
  }

  // project.workspaces.widgets.add
  {
    QJsonObject props;
    props[QStringLiteral("workspaceId")] = QJsonObject{
      {       QStringLiteral("type"),      QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Workspace id")}
    };
    props[QStringLiteral("widgetType")] = QJsonObject{
      {       QStringLiteral("type"),                           QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("SerialStudio.DashboardWidget enum")}
    };
    props[QStringLiteral("groupId")] = QJsonObject{
      {       QStringLiteral("type"),         QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source group id")}
    };
    props[QStringLiteral("relativeIndex")] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("integer")                                     },
      {QStringLiteral("description"),
       QStringLiteral("Per-type running widget counter in project order")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{
      QStringLiteral("workspaceId"),
      QStringLiteral("widgetType"),
      QStringLiteral("groupId"),
      QStringLiteral("relativeIndex"),
    };
    registry.registerCommand(
      QStringLiteral("project.workspaces.widgets.add"),
      QStringLiteral("Add a widget ref (params: workspaceId, widgetType, groupId, relativeIndex)"),
      schema,
      &widgetAdd);
  }

  // project.workspaces.widgets.remove
  {
    QJsonObject props;
    props[QStringLiteral("workspaceId")] = QJsonObject{
      {       QStringLiteral("type"),      QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Workspace id")}
    };
    props[QStringLiteral("widgetType")] = QJsonObject{
      {       QStringLiteral("type"),                           QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("SerialStudio.DashboardWidget enum")}
    };
    props[QStringLiteral("groupId")] = QJsonObject{
      {       QStringLiteral("type"),         QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source group id")}
    };
    props[QStringLiteral("relativeIndex")] = QJsonObject{
      {       QStringLiteral("type"),               QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Relative widget index")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{
      QStringLiteral("workspaceId"),
      QStringLiteral("widgetType"),
      QStringLiteral("groupId"),
      QStringLiteral("relativeIndex"),
    };
    registry.registerCommand(
      QStringLiteral("project.workspaces.widgets.remove"),
      QStringLiteral(
        "Remove a widget ref (params: workspaceId, widgetType, groupId, relativeIndex)"),
      schema,
      &widgetRemove);
  }
}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the workspace list with id, title, icon, and widget count.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::list(const QString& id,
                                                            const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& pm         = DataModel::ProjectModel::instance();
  const auto& workspaces = pm.activeWorkspaces();

  QJsonArray arr;
  for (const auto& ws : workspaces) {
    QJsonObject entry;
    entry[QStringLiteral("id")]          = ws.workspaceId;
    entry[QStringLiteral("title")]       = ws.title;
    entry[QStringLiteral("icon")]        = ws.icon;
    entry[QStringLiteral("widgetCount")] = static_cast<int>(ws.widgetRefs.size());
    arr.append(entry);
  }

  QJsonObject result;
  result[QStringLiteral("workspaces")]       = arr;
  result[QStringLiteral("count")]            = static_cast<int>(workspaces.size());
  result[QStringLiteral("customizeEnabled")] = pm.customizeWorkspaces();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns widget refs for a single workspace.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::get(const QString& id,
                                                           const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("id")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: id"));

  const int wid = params.value(QStringLiteral("id")).toInt();

  const auto& workspaces = DataModel::ProjectModel::instance().activeWorkspaces();
  const auto it          = findWorkspace(workspaces, wid);
  if (it == workspaces.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace not found: %1").arg(wid));

  QJsonObject result;
  result[QStringLiteral("id")]      = it->workspaceId;
  result[QStringLiteral("title")]   = it->title;
  result[QStringLiteral("icon")]    = it->icon;
  result[QStringLiteral("widgets")] = refsToJson(it->widgetRefs);
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a new workspace; returns the assigned id.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::add(const QString& id,
                                                           const QJsonObject& params)
{
  const QString title = params.value(QStringLiteral("title")).toString(QStringLiteral("Workspace"));

  const int newId = DataModel::ProjectModel::instance().addWorkspace(title);

  QJsonObject result;
  result[QStringLiteral("id")]    = newId;
  result[QStringLiteral("title")] = title;
  result[QStringLiteral("added")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Deletes a workspace. No-op if id not found.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::remove(const QString& id,
                                                              const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("id")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: id"));

  const int wid = params.value(QStringLiteral("id")).toInt();
  DataModel::ProjectModel::instance().deleteWorkspace(wid);

  QJsonObject result;
  result[QStringLiteral("id")]      = wid;
  result[QStringLiteral("deleted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Renames a workspace. No-op if id not found.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::rename(const QString& id,
                                                              const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("id")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: id"));

  if (!params.contains(QStringLiteral("title")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: title"));

  const int wid          = params.value(QStringLiteral("id")).toInt();
  const QString newTitle = params.value(QStringLiteral("title")).toString();
  if (newTitle.trimmed().isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace title cannot be empty"));

  DataModel::ProjectModel::instance().renameWorkspace(wid, newTitle);

  QJsonObject result;
  result[QStringLiteral("id")]      = wid;
  result[QStringLiteral("title")]   = newTitle;
  result[QStringLiteral("renamed")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Materialises the synthetic auto-workspaces into customized state.
 *
 * Guard-returns the id of an existing customised list so callers don't
 * accidentally overwrite hand-edited workspaces.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::autoGenerate(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  const int firstId = DataModel::ProjectModel::instance().autoGenerateWorkspaces();

  QJsonObject result;
  result[QStringLiteral("firstWorkspaceId")] = firstId;
  result[QStringLiteral("generated")]        = (firstId != -1);
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Customize flag
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current value of the customizeWorkspaces flag.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::customizeGet(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  QJsonObject result;
  result[QStringLiteral("enabled")] = DataModel::ProjectModel::instance().customizeWorkspaces();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Flips the customizeWorkspaces flag.
 *
 * On off->on, seeds m_workspaces from the synthetic auto list; on on->off,
 * drops the customised list and returns to automatic mode.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::customizeSet(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();
  DataModel::ProjectModel::instance().setCustomizeWorkspaces(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  result[QStringLiteral("updated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Widget ref mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Attaches a widget ref to a workspace.
 *
 * Implicitly flips customizeWorkspaces on if it wasn't already -- the project
 * must be in customize mode to persist a widget ref.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::widgetAdd(const QString& id,
                                                                 const QJsonObject& params)
{
  const QStringList required{
    QStringLiteral("workspaceId"),
    QStringLiteral("widgetType"),
    QStringLiteral("groupId"),
    QStringLiteral("relativeIndex"),
  };

  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int wid      = params.value(QStringLiteral("workspaceId")).toInt();
  const int wtype    = params.value(QStringLiteral("widgetType")).toInt();
  const int gid      = params.value(QStringLiteral("groupId")).toInt();
  const int relIndex = params.value(QStringLiteral("relativeIndex")).toInt();

  DataModel::ProjectModel::instance().addWidgetToWorkspace(wid, wtype, gid, relIndex);

  QJsonObject result;
  result[QStringLiteral("workspaceId")]   = wid;
  result[QStringLiteral("widgetType")]    = wtype;
  result[QStringLiteral("groupId")]       = gid;
  result[QStringLiteral("relativeIndex")] = relIndex;
  result[QStringLiteral("added")]         = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Detaches a widget ref matching (widgetType, groupId, relativeIndex).
 */
API::CommandResponse API::Handlers::WorkspacesHandler::widgetRemove(const QString& id,
                                                                    const QJsonObject& params)
{
  const QStringList required{
    QStringLiteral("workspaceId"),
    QStringLiteral("widgetType"),
    QStringLiteral("groupId"),
    QStringLiteral("relativeIndex"),
  };

  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int wid      = params.value(QStringLiteral("workspaceId")).toInt();
  const int wtype    = params.value(QStringLiteral("widgetType")).toInt();
  const int gid      = params.value(QStringLiteral("groupId")).toInt();
  const int relIndex = params.value(QStringLiteral("relativeIndex")).toInt();

  DataModel::ProjectModel::instance().removeWidgetFromWorkspace(wid, wtype, gid, relIndex);

  QJsonObject result;
  result[QStringLiteral("workspaceId")]   = wid;
  result[QStringLiteral("widgetType")]    = wtype;
  result[QStringLiteral("groupId")]       = gid;
  result[QStringLiteral("relativeIndex")] = relIndex;
  result[QStringLiteral("removed")]       = true;
  return CommandResponse::makeSuccess(id, result);
}
