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

#include "API/Handlers/ProjectGroupCommands.h"

#include <QJsonArray>
#include <QJsonObject>

#include "API/Handlers/ProjectApiSupport.h"
#include "API/SchemaBuilder.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "Misc/BackupManager.h"
#include "SerialStudio.h"

using namespace API::Handlers::ProjectApiSupport;

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the command class to the registry it publishes into.
 */
API::Handlers::ProjectGroupCommands::ProjectGroupCommands(CommandRegistry& registry)
  : m_registry(registry)
{}

/**
 * @brief Register group CRUD commands.
 */
void API::Handlers::ProjectGroupCommands::registerCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.group.add"),
    QStringLiteral("Create a new visualization group. Pick widgetType by data shape:\n"
                   "  - 5 (NoGroupWidget): just hold related datasets together; per-"
                   "dataset widgets render individually. Default for arbitrary "
                   "scalar data.\n"
                   "  - 4 (MultiPlot): N values plotted on a shared time axis. The "
                   "right choice for correlated signals (sensor-array, multi-channel "
                   "ADC).\n"
                   "  - 0 (DataGrid): tabular numeric readout. Good for long lists "
                   "of scalars where graphing isn't useful.\n"
                   "  - 1/2/3 (Accelerometer / Gyroscope / GPS): typed 3-axis IMU or "
                   "GPS group. Datasets must follow conventional widget tags "
                   "(\"x\", \"y\", \"z\" for IMUs; \"lat\", \"lon\", \"alt\" for GPS).\n"
                   "  - 6 (Plot3D, Pro): 3D point trail from three datasets.\n"
                   "  - 7 (ImageView, Pro): displays an embedded JPEG/PNG stream.\n"
                   "  - 8 (Painter, Pro): user-scripted JS canvas. Group can be "
                   "EMPTY (no datasets) and read peer datasets via "
                   "datasetGetFinal(uniqueId). See meta.howTo('add_painter').\n"
                   "Don't pick 0 / DataGrid as a default -- it makes a forgettable "
                   "table. Match the user's data."),
    makeSchema({
      {     QStringLiteral("title"),
       QStringLiteral("string"),
       QStringLiteral("Group title shown in dashboard headers and the Project Editor tree")},
      {QStringLiteral("widgetType"),
       QStringLiteral("integer"),
       QStringLiteral("GroupWidget enum -- see command description for decision "
       "guidance. 0=DataGrid, 1=Accelerometer, 2=Gyroscope, 3=GPS, "
       "4=MultiPlot, 5=NoGroupWidget, 6=Plot3D, 7=ImageView, 8=Painter, "
       "9=WebView, 10=BarPanel")                                                           }
  }),
    &groupAdd);

  registry.registerCommand(
    QStringLiteral("project.group.delete"),
    QStringLiteral("Delete a group by id. Pass dryRun:true to preview what would change "
                   "without committing -- the response contains the same {deleted, "
                   "renumbered, warnings} fields as a real call, plus a top-level "
                   "dryRun:true flag. Always preview before committing when the user "
                   "doesn't have a backup workflow."),
    makeSchema(
      {
        {QStringLiteral("groupId"),
         QStringLiteral("integer"),
         QStringLiteral("Group id to delete")}
  },
      {{QStringLiteral("dryRun"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, return the affected entities without committing. Auto-runs "
                       "without an approval card.")}}),
    &groupDelete);

  registry.registerCommand(QStringLiteral("project.group.duplicate"),
                           QStringLiteral("Duplicate a group by id (params: groupId)"),
                           makeSchema({
                             {QStringLiteral("groupId"),
                              QStringLiteral("integer"),
                              QStringLiteral("Group id to duplicate")}
  }),
                           &groupDuplicate);
}

/**
 * @brief Add group
 */
API::CommandResponse API::Handlers::ProjectGroupCommands::groupAdd(const QString& id,
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
  if (widget_type < 0 || widget_type > static_cast<int>(SerialStudio::BarPanel)) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid widgetType: must be 0..%1 "
                     "(see GroupWidget enum: 0=DataGrid, 1=Accelerometer, "
                     "2=Gyroscope, 3=GPS, 4=MultiPlot, 5=NoGroupWidget, "
                     "6=Plot3D, 7=ImageView, 8=Painter, 9=WebView, 10=BarPanel)")
        .arg(static_cast<int>(SerialStudio::BarPanel)));
  }

  const auto widget         = static_cast<SerialStudio::GroupWidget>(widget_type);
  static auto& projectModel = DataModel::ProjectModel::instance();
  projectModel.addGroup(title, widget);

  QJsonObject result;
  result[QStringLiteral("title")]      = title;
  result[QStringLiteral("widgetType")] = widget_type;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete a group by id
 */
API::CommandResponse API::Handlers::ProjectGroupCommands::groupDelete(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& targetGroup = groups[groupId];
  QJsonArray childDatasets;
  for (const auto& d : targetGroup.datasets) {
    QJsonObject row;
    row[Keys::DatasetId]         = d.datasetId;
    row[Keys::UniqueId]          = d.uniqueId;
    row[QStringLiteral("title")] = d.title;
    childDatasets.append(row);
  }

  QJsonObject deleted;
  deleted[QStringLiteral("groupId")]      = groupId;
  deleted[QStringLiteral("title")]        = targetGroup.title;
  deleted[QStringLiteral("widget")]       = targetGroup.widget;
  deleted[QStringLiteral("datasetCount")] = childDatasets.size();
  deleted[QStringLiteral("datasets")]     = childDatasets;

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);

  QJsonArray renumbered;
  for (const auto& g : groups) {
    if (g.groupId <= groupId)
      continue;

    QJsonObject row;
    row[QStringLiteral("oldGroupId")]   = g.groupId;
    row[QStringLiteral("newGroupId")]   = g.groupId - 1;
    row[QStringLiteral("title")]        = g.title;
    row[QStringLiteral("datasetCount")] = static_cast<int>(g.datasets.size());
    renumbered.append(row);
  }

  QString backupPath;
  qint64 preEpoch = 0;
  if (!isDryRun) {
    static auto& backupManager = Misc::BackupManager::instance();
    backupPath                 = backupManager.snapshot(QStringLiteral("pre-groupDelete"));
    preEpoch                   = captureProjectEpoch();
    project.deleteGroup(groupId);
  }

  QJsonObject result;
  if (isDryRun)
    result[QStringLiteral("dryRun")] = true;

  result[QStringLiteral("deleted")]    = deleted;
  result[QStringLiteral("renumbered")] = renumbered;
  if (!backupPath.isEmpty())
    result[QStringLiteral("backupPath")] = backupPath;

  QJsonArray warnings;
  if (isDryRun)
    warnings.append(QStringLiteral(
      "DRY RUN: no changes were written. Re-call without dryRun:true to commit. The "
      "renumbered[] array shows groupId values that WOULD shift; every dataset in those "
      "groups would have its uniqueId invalidated."));

  else if (!renumbered.isEmpty())
    warnings.append(QStringLiteral(
      "groupId values shifted after deletion; uniqueIds of every dataset in renumbered "
      "groups are now stale -- re-read project state before further mutations."));

  if (!isDryRun && !backupPath.isEmpty())
    warnings.append(QStringLiteral(
      "Pre-mutation snapshot saved at backupPath; pass it to assistant.restore to undo."));

  if (!warnings.isEmpty())
    result[QStringLiteral("warnings")] = warnings;

  if (!isDryRun)
    appendStaleProjectWarning(result, params, preEpoch);

  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate a group by id
 */
API::CommandResponse API::Handlers::ProjectGroupCommands::groupDuplicate(const QString& id,
                                                                         const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  project.duplicateGroup(groupId);

  QJsonObject result;
  result[QStringLiteral("groupId")]    = groupId;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}
