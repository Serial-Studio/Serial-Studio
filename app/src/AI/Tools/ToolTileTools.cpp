/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Tools/ToolTileTools.h"

#include <QJsonArray>
#include <QStringList>

#include "AI/Tools/ToolBulkTools.h"
#include "AI/Tools/ToolCompact.h"
#include "AI/Tools/ToolResolve.h"
#include "AI/Tools/ToolSupport.h"
#include "DataModel/Frame.h"

namespace AI::ToolDetail {

//--------------------------------------------------------------------------------------------------
// Tile prerequisites
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a new workspace by title, recording the step.
 */
static QJsonObject createTileWorkspace(const QString& title, QJsonArray& steps)
{
  QJsonObject addWsArgs;
  addWsArgs[QStringLiteral("title")] = title;
  const auto addWsReply = runCommand(QStringLiteral("project.workspace.add"), addWsArgs);
  steps.append(QJsonObject{
    {  QStringLiteral("command"),         QStringLiteral("project.workspace.add")},
    {       QStringLiteral("ok"), addWsReply.value(QStringLiteral("ok")).toBool()},
    {QStringLiteral("arguments"),                                       addWsArgs}
  });
  return addWsReply;
}

/**
 * @brief Resolves a workspace from args, optionally creating one when allowed.
 */
static QJsonObject resolveOrCreateTileWorkspace(const QJsonObject& args, QJsonArray& steps)
{
  QJsonObject wsArgs;
  if (args.contains(QStringLiteral("workspaceId")))
    wsArgs[QStringLiteral("workspaceId")] = args.value(QStringLiteral("workspaceId")).toInt();

  if (!args.value(QStringLiteral("workspace")).toString().isEmpty())
    wsArgs[QStringLiteral("title")] = args.value(QStringLiteral("workspace")).toString();

  auto wsReply = resolveWorkspace(wsArgs);
  if (wsReply.value(QStringLiteral("ok")).toBool())
    return wsReply;

  const auto workspaceTitle = args.value(QStringLiteral("workspace")).toString();
  if (workspaceTitle.isEmpty() || !args.value(QStringLiteral("createWorkspace")).toBool(false))
    return wsReply;

  const auto addWsReply = createTileWorkspace(workspaceTitle, steps);
  if (!addWsReply.value(QStringLiteral("ok")).toBool())
    return attachRepairHint(addWsReply, QStringLiteral("project.workspace.add"));

  QJsonObject created                    = addWsReply.value(QStringLiteral("result")).toObject();
  created[QStringLiteral("widgetCount")] = 0;
  wsReply[QStringLiteral("ok")]          = true;
  wsReply[QStringLiteral("workspace")]   = created;
  return wsReply;
}

/**
 * @brief Resolves an optional dataset reference for the tile, updating group args in place.
 */
static QJsonObject resolveTileDataset(const QJsonObject& args, QJsonObject& groupArgs)
{
  const bool hasDatasetRef =
    !args.value(QStringLiteral("dataset")).toString().isEmpty() || args.contains(Keys::UniqueId);
  if (!hasDatasetRef)
    return {};

  QJsonObject dsArgs;
  if (args.contains(Keys::UniqueId))
    dsArgs[Keys::UniqueId] = args.value(Keys::UniqueId);

  const auto datasetName = args.value(QStringLiteral("dataset")).toString();
  if (datasetName.contains(QLatin1Char('/')))
    dsArgs[QStringLiteral("path")] = datasetName;
  else if (!datasetName.isEmpty())
    dsArgs[QStringLiteral("title")] = datasetName;

  if (args.contains(QStringLiteral("groupId")))
    dsArgs[QStringLiteral("groupId")] = args.value(QStringLiteral("groupId")).toInt();

  const auto dsReply = resolveDataset(dsArgs);
  if (!dsReply.value(QStringLiteral("ok")).toBool())
    return dsReply;

  const auto dataset = dsReply.value(QStringLiteral("dataset")).toObject();
  if (!groupArgs.contains(QStringLiteral("groupId")))
    groupArgs[QStringLiteral("groupId")] = dataset.value(QStringLiteral("groupId")).toInt();

  return dsReply;
}

/**
 * @brief Enables the dataset option a widget type requires, recording the API step.
 */
static QJsonObject enableTileWidgetOption(const QString& optionSlug,
                                          const QJsonObject& dataset,
                                          int groupId,
                                          QJsonArray& steps)
{
  QStringList slugs;
  for (const auto& value : dataset.value(QStringLiteral("enabledOptionsSlugs")).toArray())
    slugs.append(value.toString());

  if (!slugs.contains(optionSlug))
    slugs.append(optionSlug);

  QJsonObject optArgs;
  optArgs[QStringLiteral("groupId")] = dataset.value(QStringLiteral("groupId")).toInt(groupId);
  optArgs[Keys::DatasetId]           = dataset.value(Keys::DatasetId).toInt();
  optArgs[QStringLiteral("options")] = QJsonArray::fromStringList(slugs);

  const auto optReply = runCommand(QStringLiteral("project.dataset.setOptions"), optArgs);
  steps.append(QJsonObject{
    {  QStringLiteral("command"),  QStringLiteral("project.dataset.setOptions")},
    {       QStringLiteral("ok"), optReply.value(QStringLiteral("ok")).toBool()},
    {QStringLiteral("arguments"),                                       optArgs}
  });
  return optReply;
}

/**
 * @brief Patches dataset min/max ranges from the optional `ranges` request payload.
 */
static QJsonObject applyTileRangeUpdates(const QJsonObject& args,
                                         const QJsonObject& dataset,
                                         int groupId,
                                         QJsonArray& steps)
{
  const auto ranges = args.value(QStringLiteral("ranges")).toObject();
  if (ranges.isEmpty() || dataset.isEmpty())
    return {};

  static const QStringList kRangeFields = {QStringLiteral("pltMin"),
                                           QStringLiteral("pltMax"),
                                           QStringLiteral("wgtMin"),
                                           QStringLiteral("wgtMax"),
                                           QStringLiteral("fftMin"),
                                           QStringLiteral("fftMax")};
  QJsonObject updateArgs;
  updateArgs[QStringLiteral("groupId")] = dataset.value(QStringLiteral("groupId")).toInt(groupId);
  updateArgs[Keys::DatasetId]           = dataset.value(Keys::DatasetId).toInt();
  for (const auto& field : kRangeFields)
    if (ranges.contains(field))
      updateArgs[field] = ranges.value(field);

  if (updateArgs.size() <= 2)
    return {};

  const auto updateReply = runCommand(QStringLiteral("project.dataset.update"), updateArgs);
  steps.append(QJsonObject{
    {  QStringLiteral("command"),         QStringLiteral("project.dataset.update")},
    {       QStringLiteral("ok"), updateReply.value(QStringLiteral("ok")).toBool()},
    {QStringLiteral("arguments"),                                       updateArgs}
  });
  return updateReply;
}

/**
 * @brief Enables customize mode so subsequent workspace mutations are accepted.
 */
static QJsonObject enableCustomizeMode(QJsonArray& steps)
{
  const auto customize = runCommand(QStringLiteral("project.workspace.setCustomizeMode"),
                                    QJsonObject{
                                      {QStringLiteral("enabled"), true}
  });
  steps.append(QJsonObject{
    {QStringLiteral("command"), QStringLiteral("project.workspace.setCustomizeMode")},
    {     QStringLiteral("ok"),       customize.value(QStringLiteral("ok")).toBool()}
  });
  return customize;
}

//--------------------------------------------------------------------------------------------------
// Tile orchestration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Orchestrates the workspace tile addition flow end-to-end.
 */
QJsonObject executeAddTile(const QJsonObject& args)
{
  QJsonArray steps;
  QJsonArray warnings;

  const QString widgetType = args.value(QStringLiteral("widgetType")).toString();
  if (widgetType.isEmpty()) {
    QJsonObject out;
    out[QStringLiteral("ok")]    = false;
    out[QStringLiteral("error")] = QStringLiteral("missing_widgetType");
    return out;
  }

  static const QStringList kFanOutCommands = {
    QStringLiteral("project.workspace.add"),
    QStringLiteral("project.dataset.setOptions"),
    QStringLiteral("project.dataset.update"),
    QStringLiteral("project.workspace.setCustomizeMode"),
    QStringLiteral("project.workspace.addWidget"),
  };
  for (const auto& command : kFanOutCommands)
    if (!innerOpAllowed(command))
      return makeInnerOpRejection(command);

  auto wsReply = resolveOrCreateTileWorkspace(args, steps);
  if (!wsReply.value(QStringLiteral("ok")).toBool())
    return wsReply;

  const auto workspace  = wsReply.value(QStringLiteral("workspace")).toObject();
  const int workspaceId = workspace.value(QStringLiteral("id")).toInt();

  QJsonObject groupArgs = args;
  QJsonObject dataset;
  const auto dsReply = resolveTileDataset(args, groupArgs);
  if (!dsReply.isEmpty() && !dsReply.value(QStringLiteral("ok")).toBool())
    return dsReply;

  if (!dsReply.isEmpty())
    dataset = dsReply.value(QStringLiteral("dataset")).toObject();

  auto groupReply = resolveGroup(groupArgs);
  if (!groupReply.value(QStringLiteral("ok")).toBool())
    return groupReply;

  const auto group = groupReply.value(QStringLiteral("group")).toObject();
  const int groupId =
    group.value(QStringLiteral("groupId")).toInt(group.value(QStringLiteral("id")).toInt());

  const auto customize = enableCustomizeMode(steps);
  if (!customize.value(QStringLiteral("ok")).toBool())
    return attachRepairHint(customize, QStringLiteral("assistant.workspace.addTile"));

  const QString optionSlug = optionSlugForWidget(widgetType);
  if (!optionSlug.isEmpty() && dataset.isEmpty())
    warnings.append(QStringLiteral("Widget type needs a dataset option, but no dataset was "
                                   "provided; addWidget may fail if the group is not already "
                                   "compatible."));
  else if (!optionSlug.isEmpty()) {
    const auto optReply = enableTileWidgetOption(optionSlug, dataset, groupId, steps);
    if (!optReply.value(QStringLiteral("ok")).toBool())
      return attachRepairHint(optReply, QStringLiteral("project.dataset.setOptions"));
  }

  const auto updateReply = applyTileRangeUpdates(args, dataset, groupId, steps);
  if (!updateReply.isEmpty() && !updateReply.value(QStringLiteral("ok")).toBool())
    return attachRepairHint(updateReply, QStringLiteral("project.dataset.update"));

  QJsonObject addArgs;
  addArgs[QStringLiteral("workspaceId")] = workspaceId;
  addArgs[QStringLiteral("widgetType")]  = widgetType;
  addArgs[QStringLiteral("groupId")]     = groupId;
  const auto addReply = runCommand(QStringLiteral("project.workspace.addWidget"), addArgs);
  steps.append(QJsonObject{
    {  QStringLiteral("command"), QStringLiteral("project.workspace.addWidget")},
    {       QStringLiteral("ok"), addReply.value(QStringLiteral("ok")).toBool()},
    {QStringLiteral("arguments"),                                       addArgs}
  });
  if (!addReply.value(QStringLiteral("ok")).toBool())
    return attachRepairHint(addReply, QStringLiteral("assistant.workspace.addTile"));

  const auto verify = runCommand(QStringLiteral("project.workspace.get"),
                                 QJsonObject{
                                   {QStringLiteral("id"), workspaceId}
  });
  QJsonObject out;
  out[QStringLiteral("ok")]        = true;
  out[QStringLiteral("workspace")] = workspace;
  out[QStringLiteral("group")]     = group;
  if (!dataset.isEmpty())
    out[QStringLiteral("dataset")] = dataset;

  out[QStringLiteral("added")]    = addReply.value(QStringLiteral("result")).toObject();
  out[QStringLiteral("verify")]   = verify.value(QStringLiteral("result")).toObject();
  out[QStringLiteral("steps")]    = steps;
  out[QStringLiteral("warnings")] = warnings;
  return out;
}

}  // namespace AI::ToolDetail
