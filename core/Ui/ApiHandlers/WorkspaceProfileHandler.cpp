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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "ApiHandlers/WorkspaceProfileHandler.h"

#include <QJsonArray>
#include <QJsonObject>
#include <vector>

#include "API/CommandRegistry.h"
#include "API/HandlerContext.h"
#include "API/SchemaBuilder.h"
#include "AppState.h"
#include "Core/DataModel/WorkspaceProfile.h"
#include "Core/SerialStudio.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"

//--------------------------------------------------------------------------------------------------
// Local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the project model is in ProjectFile mode, the only mode that owns a
 *        document a profile can belong to.
 */
[[nodiscard]] static bool hasProjectDocument()
{
  auto& appState = DataModel::pipelineModules().appState;
  return appState.operationMode() == SerialStudio::ProjectFile;
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers the workspace-profile commands (spec 0083).
 */
void API::Handlers::WorkspaceProfileHandler::registerCommands()
{
  auto& registry = API::handlerContext().registry;

  const auto idSchema = API::makeSchema({
    {QStringLiteral("profileId"), QStringLiteral("integer"), QStringLiteral("Profile id")}
  });

  registry.registerCommand(
    QStringLiteral("project.workspace.profile.list"),
    QStringLiteral("List the project's workspace profiles (named subsets of workspace folders "
                   "and workspaces) and which one is active (-1 = all workspaces)."),
    API::emptySchema(),
    &profileList);
  registry.registerCommand(
    QStringLiteral("project.workspace.profile.add"),
    QStringLiteral("Add an empty workspace profile (params: title). Fill it with "
                   "project.workspace.profile.update."),
    API::makeSchema({
      {QStringLiteral("title"), QStringLiteral("string"), QStringLiteral("Profile title")}
  }),
    &profileAdd);
  registry.registerCommand(
    QStringLiteral("project.workspace.profile.update"),
    QStringLiteral("Edit a profile (params: profileId, [title], [folderIds], [workspaceIds]). "
                   "A listed folder shows its whole subtree; the id arrays replace the lists."),
    API::makeSchema(
      {
        {QStringLiteral("profileId"), QStringLiteral("integer"), QStringLiteral("Profile id")}
  },
      {{QStringLiteral("title"), QStringLiteral("string"), QStringLiteral("New title")},
       {QStringLiteral("folderIds"),
        QStringLiteral("array"),
        QStringLiteral("Workspace folder ids the profile shows")},
       {QStringLiteral("workspaceIds"),
        QStringLiteral("array"),
        QStringLiteral("Individual workspace ids the profile shows")}}),
    &profileUpdate);
  registry.registerCommand(QStringLiteral("project.workspace.profile.remove"),
                           QStringLiteral("Delete a workspace profile (params: profileId)."),
                           idSchema,
                           &profileRemove);
  registry.registerCommand(
    QStringLiteral("project.workspace.profile.select"),
    QStringLiteral("Show only the workspaces of a profile (params: profileId, or title; -1 or "
                   "an empty title shows all). Runtime state, not saved with the project."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("profileId"),
        QStringLiteral("integer"),
        QStringLiteral("Profile id, -1 for all workspaces")},
       {QStringLiteral("title"), QStringLiteral("string"), QStringLiteral("Profile title")}}),
    &profileSelect);
}

//--------------------------------------------------------------------------------------------------
// Workspace profiles (spec 0083)
//--------------------------------------------------------------------------------------------------

/**
 * @brief One profile as an API object.
 */
[[nodiscard]] static QJsonObject profileToJson(const DataModel::WorkspaceProfile& profile)
{
  QJsonArray folders;
  for (const int folderId : profile.folderIds)
    folders.append(folderId);

  QJsonArray workspaces;
  for (const int workspaceId : profile.workspaceIds)
    workspaces.append(workspaceId);

  QJsonObject entry;
  entry[QStringLiteral("profileId")]    = profile.profileId;
  entry[QStringLiteral("title")]        = profile.title;
  entry[QStringLiteral("folderIds")]    = folders;
  entry[QStringLiteral("workspaceIds")] = workspaces;
  return entry;
}

/**
 * @brief Reads an integer array parameter into a vector.
 */
[[nodiscard]] static std::vector<int> intArray(const QJsonValue& value)
{
  std::vector<int> ids;
  for (const auto& item : value.toArray())
    ids.push_back(item.toInt());

  return ids;
}

/**
 * @brief Lists the profiles and the active selection.
 */
API::CommandResponse API::Handlers::WorkspaceProfileHandler::profileList(const QString& id,
                                                                         const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& pm = DataModel::pipelineModules().projectModel;

  QJsonArray arr;
  for (const auto& profile : pm.workspaceProfileList())
    arr.append(profileToJson(profile));

  QJsonObject result;
  result[QStringLiteral("profiles")]      = arr;
  result[QStringLiteral("activeProfile")] = pm.activeWorkspaceProfile();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Adds an empty profile.
 */
API::CommandResponse API::Handlers::WorkspaceProfileHandler::profileAdd(const QString& id,
                                                                        const QJsonObject& params)
{
  if (!hasProjectDocument())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace mutations require ProjectFile mode"));

  const QString title = params.value(QStringLiteral("title")).toString().simplified();
  if (title.isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: title"));

  auto& pm            = DataModel::pipelineModules().projectModel;
  const int profileId = pm.addWorkspaceProfile(title);

  QJsonObject result;
  result[QStringLiteral("profileId")] = profileId;
  result[QStringLiteral("title")]     = title;
  result[QStringLiteral("added")]     = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Replaces a profile's title and/or id lists.
 */
API::CommandResponse API::Handlers::WorkspaceProfileHandler::profileUpdate(
  const QString& id, const QJsonObject& params)
{
  if (!hasProjectDocument())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace mutations require ProjectFile mode"));

  if (!params.contains(QStringLiteral("profileId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: profileId"));

  auto& pm            = DataModel::pipelineModules().projectModel;
  const int profileId = params.value(QStringLiteral("profileId")).toInt();
  const auto* before  = pm.workspaceProfile(profileId);
  if (!before)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Profile not found: %1").arg(profileId));

  if (params.contains(QStringLiteral("title")))
    pm.renameWorkspaceProfile(profileId, params.value(QStringLiteral("title")).toString());

  if (params.contains(QStringLiteral("folderIds")))
    pm.setWorkspaceProfileFolders(profileId, intArray(params.value(QStringLiteral("folderIds"))));

  if (params.contains(QStringLiteral("workspaceIds")))
    pm.setWorkspaceProfileWorkspaces(profileId,
                                     intArray(params.value(QStringLiteral("workspaceIds"))));

  const auto* after                 = pm.workspaceProfile(profileId);
  QJsonObject result                = after ? profileToJson(*after) : QJsonObject();
  result[QStringLiteral("updated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Deletes a profile.
 */
API::CommandResponse API::Handlers::WorkspaceProfileHandler::profileRemove(
  const QString& id, const QJsonObject& params)
{
  if (!hasProjectDocument())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace mutations require ProjectFile mode"));

  if (!params.contains(QStringLiteral("profileId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: profileId"));

  auto& pm            = DataModel::pipelineModules().projectModel;
  const int profileId = params.value(QStringLiteral("profileId")).toInt();
  if (!pm.workspaceProfile(profileId))
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Profile not found: %1").arg(profileId));

  pm.deleteWorkspaceProfile(profileId);

  QJsonObject result;
  result[QStringLiteral("profileId")] = profileId;
  result[QStringLiteral("removed")]   = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Selects the profile the dashboard shows, by id or title; -1 / empty shows every workspace.
 */
API::CommandResponse API::Handlers::WorkspaceProfileHandler::profileSelect(
  const QString& id, const QJsonObject& params)
{
  auto& pm      = DataModel::pipelineModules().projectModel;
  int profileId = params.value(QStringLiteral("profileId")).toInt(-1);
  if (params.contains(QStringLiteral("title"))) {
    const QString title = params.value(QStringLiteral("title")).toString().simplified();
    profileId           = title.isEmpty() ? -1 : pm.workspaceProfileIdForTitle(title);
    if (!title.isEmpty() && profileId < 0)
      return CommandResponse::makeError(
        id, ErrorCode::InvalidParam, QStringLiteral("Profile not found: %1").arg(title));
  }

  if (profileId >= 0 && !pm.workspaceProfile(profileId))
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Profile not found: %1").arg(profileId));

  pm.setActiveWorkspaceProfile(profileId);

  QJsonObject result;
  result[QStringLiteral("activeProfile")] = pm.activeWorkspaceProfile();
  result[QStringLiteral("selected")]      = true;
  return CommandResponse::makeSuccess(id, result);
}
