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

/**
 * @brief Builds the prose summary for groupsList; names come from the windowed rows only.
 */
static QString groupsListSummary(const std::vector<DataModel::Group>& groups,
                                 const API::Handlers::ListWindow& window,
                                 int total)
{
  if (groups.empty())
    return QStringLiteral("No groups configured.");

  if (window.count == 0)
    return QStringLiteral("%1 groups; the requested offset is past the end.")
      .arg(QString::number(total));

  QStringList names;
  for (int g = window.start; g < window.start + window.count; ++g) {
    const auto& grp      = groups[static_cast<size_t>(g)];
    const auto widgetStr = grp.widget.simplified();
    names.append(
      QStringLiteral("\"%1\"%2")
        .arg(grp.title, widgetStr.isEmpty() ? QString() : QStringLiteral(" (%1)").arg(widgetStr)));
  }

  const QString shown =
    window.count < total
      ? QStringLiteral(" (showing %1-%2)")
          .arg(QString::number(window.start), QString::number(window.start + window.count - 1))
      : QString();
  return QStringLiteral("%1 group%2%3: %4.")
    .arg(QString::number(total),
         total == 1 ? QString() : QStringLiteral("s"),
         shown,
         names.join(QStringLiteral(", ")));
}

/**
 * @brief List all groups with basic info
 */
API::CommandResponse API::Handlers::ProjectHandler::groupsList(const QString& id,
                                                               const QJsonObject& params)
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto& groups        = projectModel.groups();

  const int total   = static_cast<int>(groups.size());
  const auto window = applyWindow(total,
                                  params.value(QStringLiteral("offset")).toInt(0),
                                  params.value(QStringLiteral("limit")).toInt(0));

  QJsonArray groups_array;
  for (int g = window.start; g < window.start + window.count; ++g) {
    const auto& group = groups[static_cast<size_t>(g)];
    QJsonObject obj   = DataModel::serialize(group);

    obj[Keys::GroupId]                  = group.groupId;
    obj[QStringLiteral("datasetCount")] = static_cast<int>(group.datasets.size());

    QJsonArray ds_summary;
    for (const auto& ds : group.datasets) {
      QJsonObject d;
      d[Keys::DatasetId]         = ds.datasetId;
      d[Keys::UniqueId]          = ds.uniqueId;
      d[QStringLiteral("index")] = ds.index;
      d[QStringLiteral("title")] = ds.title;
      if (!ds.units.isEmpty())
        d[QStringLiteral("units")] = ds.units;

      d[QStringLiteral("enabledOptions")] = datasetOptionsBitflag(ds);

      QJsonArray ds_compat;
      appendDatasetWidgetTypes(ds, ds_compat);
      d[QStringLiteral("enabledWidgetTypes")] = ds_compat;

      ds_summary.append(d);
    }
    obj[QStringLiteral("datasetSummary")] = ds_summary;

    QJsonArray compat;
    const auto group_w = static_cast<int>(SerialStudio::getDashboardWidget(group));
    if (group_w != SerialStudio::DashboardNoWidget)
      compat.append(group_w);

    for (const auto& ds : group.datasets)
      appendDatasetWidgetTypes(ds, compat);

    obj[QStringLiteral("compatibleWidgetTypes")] = compat;

    QJsonArray compatSlugs;
    for (const auto& v : compat) {
      const auto slug = API::EnumLabels::dashboardWidgetSlug(v.toInt());
      if (!compatSlugs.contains(slug))
        compatSlugs.append(slug);
    }
    obj[QStringLiteral("compatibleWidgetTypeSlugs")] = compatSlugs;

    groups_array.append(obj);
  }

  const QString summary = groupsListSummary(groups, window, total);

  int totalDatasets = 0;
  for (const auto& g : groups)
    totalDatasets += static_cast<int>(g.datasets.size());

  QJsonObject result;
  result[QStringLiteral("_summary")]   = summary;
  result[QStringLiteral("groups")]     = groups_array;
  result[QStringLiteral("groupCount")] = total;
  attachWindowInfo(result, window, total);
  attachProjectEpoch(result);
  if (groups.size() >= 5 || totalDatasets >= 10)
    result[QStringLiteral("_hint")] =
      QStringLiteral("%1 groups / %2 datasets present. For bulk edits across many "
                     "groups/datasets use project.batch instead of looping per-item updates "
                     "(see meta.describeCommand{name:\"project.batch\"}). For creating arrays "
                     "of similar datasets, use project.dataset.addMany.")
        .arg(groups.size())
        .arg(totalDatasets);

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief List all datasets across all groups
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetsList(const QString& id,
                                                                 const QJsonObject& params)
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto& groups        = projectModel.groups();

  int total_datasets = 0;
  for (const auto& group : groups)
    total_datasets += static_cast<int>(group.datasets.size());

  const auto window = applyWindow(total_datasets,
                                  params.value(QStringLiteral("offset")).toInt(0),
                                  params.value(QStringLiteral("limit")).toInt(0));

  QJsonArray datasets_array;
  int flat_index = 0;
  for (const auto& group : groups) {
    for (const auto& dataset : group.datasets) {
      if (flat_index >= window.start && flat_index < window.start + window.count)
        datasets_array.append(buildDatasetObject(dataset, group));

      ++flat_index;
    }
  }

  QString summary;
  if (total_datasets == 0) {
    summary = QStringLiteral("No datasets configured.");
  } else if (window.count == 0) {
    summary = QStringLiteral("%1 datasets; the requested offset is past the end.")
                .arg(QString::number(total_datasets));
  } else {
    const QString shown =
      window.count < total_datasets
        ? QStringLiteral(" (showing %1-%2)")
            .arg(QString::number(window.start), QString::number(window.start + window.count - 1))
        : QString();
    summary = QStringLiteral("%1 datasets across %2 group%3%4.")
                .arg(QString::number(total_datasets),
                     QString::number(groups.size()),
                     groups.size() == 1 ? QString() : QStringLiteral("s"),
                     shown);
  }

  QJsonObject result;
  result[QStringLiteral("_summary")]     = summary;
  result[QStringLiteral("datasets")]     = datasets_array;
  result[QStringLiteral("datasetCount")] = total_datasets;
  attachWindowInfo(result, window, total_datasets);
  attachProjectEpoch(result);
  if (total_datasets >= 10)
    result[QStringLiteral("_hint")] =
      QStringLiteral("Bulk edits across %1 datasets: use project.batch (rename/retitle/reindex/"
                     "update many at once) or project.dataset.addMany (create N similar). "
                     "Looping single project.dataset.update calls costs N round-trips and N "
                     "autosave-debounce restarts. See meta.describeCommand{name:"
                     "\"project.batch\"} for the {ops:[{command,params},...]} shape.")
        .arg(total_datasets);

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Find a dataset by its uniqueId (number) or alias (string) across all groups.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetGetByUniqueId(const QString& id,
                                                                         const QJsonObject& params)
{
  if (!params.contains(Keys::UniqueId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: uniqueId"));

  QString error;
  const auto match = resolveDatasetSelector(params.value(Keys::UniqueId), error);
  if (!match.dataset)
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, error);

  return CommandResponse::makeSuccess(id, buildDatasetObject(*match.dataset, *match.group));
}

/**
 * @brief Find a dataset by title (optionally narrowed by sourceId / groupId).
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetGetByTitle(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(Keys::Title))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: title"));

  const QString title        = params.value(Keys::Title).toString();
  const bool hasSourceFilter = params.contains(Keys::SourceId);
  const bool hasGroupFilter  = params.contains(Keys::GroupId);
  const int filterSourceId   = hasSourceFilter ? params.value(Keys::SourceId).toInt() : 0;
  const int filterGroupId    = hasGroupFilter ? params.value(Keys::GroupId).toInt() : 0;

  if (title.isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("title cannot be empty"));

  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto& groups        = projectModel.groups();

  QJsonArray matches;
  for (const auto& group : groups) {
    if (hasGroupFilter && group.groupId != filterGroupId)
      continue;

    for (const auto& dataset : group.datasets) {
      if (hasSourceFilter && dataset.sourceId != filterSourceId)
        continue;

      if (dataset.title == title)
        matches.append(buildDatasetObject(dataset, group));
    }
  }

  if (matches.isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("No dataset matched title '%1'").arg(title));

  if (matches.size() > 1) {
    QJsonObject extra;
    extra[QStringLiteral("matches")] = matches;
    extra[QStringLiteral("hint")] =
      QStringLiteral("Multiple datasets match this title. Pass sourceId or groupId to "
                     "disambiguate, or call project.dataset.getByPath.");
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Ambiguous title '%1' (%2 matches)")
                                        .arg(title, QString::number(matches.size())),
                                      extra);
  }

  return CommandResponse::makeSuccess(id, matches.first().toObject());
}

/**
 * @brief Find a dataset by 'Group/Dataset' or 'Source/Group/Dataset' path.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetGetByPath(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("path")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: path"));

  const QString path  = params.value(QStringLiteral("path")).toString();
  const auto segments = path.split(QChar('/'), Qt::SkipEmptyParts);

  if (segments.size() != 2 && segments.size() != 3)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("path must be 'Group/Dataset' or 'Source/Group/Dataset'"));

  const QString sourceTitle  = segments.size() == 3 ? segments.at(0) : QString();
  const QString groupTitle   = segments.size() == 3 ? segments.at(1) : segments.at(0);
  const QString datasetTitle = segments.last();

  static const auto& pm = DataModel::ProjectModel::instance();
  const auto& sources   = pm.sources();
  const auto& groups    = pm.groups();

  int sourceFilterId = -1;
  if (!sourceTitle.isEmpty()) {
    for (const auto& src : sources)
      if (src.title == sourceTitle) {
        sourceFilterId = src.sourceId;
        break;
      }

    if (sourceFilterId < 0)
      return CommandResponse::makeError(
        id, ErrorCode::InvalidParam, QStringLiteral("Source not found: '%1'").arg(sourceTitle));
  }

  for (const auto& group : groups) {
    if (group.title != groupTitle)
      continue;

    for (const auto& dataset : group.datasets) {
      if (sourceFilterId >= 0 && dataset.sourceId != sourceFilterId)
        continue;

      if (dataset.title == datasetTitle)
        return CommandResponse::makeSuccess(id, buildDatasetObject(dataset, group));
    }
  }

  return CommandResponse::makeError(
    id, ErrorCode::InvalidParam, QStringLiteral("No dataset matched path '%1'").arg(path));
}

namespace API::Handlers {

/**
 * @brief Compute the new ordinal for an item after a list move; mirrors std::vector reorder.
 */
static int projectedAfterMove(int oldIndex, int from, int to)
{
  if (oldIndex == from)
    return to;

  if (from < to && oldIndex > from && oldIndex <= to)
    return oldIndex - 1;

  if (from > to && oldIndex >= to && oldIndex < from)
    return oldIndex + 1;

  return oldIndex;
}

}  // namespace API::Handlers

/**
 * @brief Moves a dataset to a new position within its group.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetMove(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(Keys::UniqueId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: uniqueId"));

  if (!params.contains(QStringLiteral("newPosition")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: newPosition"));

  const int newPosition = params.value(QStringLiteral("newPosition")).toInt();
  const bool isDryRun   = params.value(QStringLiteral("dryRun")).toBool(false);

  QString error;
  const auto match = resolveDatasetSelector(params.value(Keys::UniqueId), error);
  if (!match.dataset)
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, error);

  const DataModel::Group* matchGroup     = match.group;
  const DataModel::Dataset* matchDataset = match.dataset;

  static auto& pm = DataModel::ProjectModel::instance();

  const int datasetUniqueId = matchDataset->uniqueId;
  const int oldPosition     = matchDataset->datasetId;
  const int clampedNewId =
    std::clamp(newPosition, 0, static_cast<int>(matchGroup->datasets.size()) - 1);

  QJsonArray renumbered;
  for (const auto& d : matchGroup->datasets) {
    const int newId = projectedAfterMove(d.datasetId, oldPosition, clampedNewId);
    if (newId == d.datasetId)
      continue;

    QJsonObject row;
    row[QStringLiteral("groupId")]      = matchGroup->groupId;
    row[QStringLiteral("oldDatasetId")] = d.datasetId;
    row[QStringLiteral("newDatasetId")] = newId;
    row[Keys::UniqueId]                 = d.uniqueId;
    row[QStringLiteral("title")]        = d.title;
    renumbered.append(row);
  }

  qint64 preEpoch = 0;
  if (!isDryRun) {
    preEpoch = captureProjectEpoch();
    pm.moveDataset(matchGroup->groupId, oldPosition, newPosition);
  }

  QJsonObject result;
  if (isDryRun)
    result[QStringLiteral("dryRun")] = true;

  result[Keys::UniqueId]                = datasetUniqueId;
  result[Keys::GroupId]                 = matchGroup->groupId;
  result[QStringLiteral("oldPosition")] = oldPosition;
  result[QStringLiteral("newPosition")] = clampedNewId;
  result[QStringLiteral("moved")]       = !isDryRun;
  result[QStringLiteral("renumbered")]  = renumbered;

  QString warning;
  if (isDryRun)
    warning = QStringLiteral(
      "DRY RUN: no changes were written. Re-call without dryRun:true to commit. The "
      "renumbered[] array shows the new datasetId for each affected dataset (uniqueId "
      "is stable across reorders).");

  else
    warning = QStringLiteral(
      "Dataset reorder renumbers datasetId within the group. uniqueId stays stable "
      "across reorders, so workspace refs and xAxisId references survive untouched.");

  result[QStringLiteral("warning")] = warning;

  if (!isDryRun)
    appendStaleProjectWarning(result, params, preEpoch);

  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Moves a group to a new position in the project.
 */
API::CommandResponse API::Handlers::ProjectHandler::groupMove(const QString& id,
                                                              const QJsonObject& params)
{
  if (!params.contains(Keys::GroupId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("newPosition")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: newPosition"));

  const int groupId     = params.value(Keys::GroupId).toInt();
  const int newPosition = params.value(QStringLiteral("newPosition")).toInt();
  const bool isDryRun   = params.value(QStringLiteral("dryRun")).toBool(false);

  static auto& pm    = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  if (groupId < 0 || groupId >= static_cast<int>(groups.size()))
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id out of range: %1").arg(groupId));

  const int clampedNew = std::clamp(newPosition, 0, static_cast<int>(groups.size()) - 1);

  QJsonArray renumbered;
  for (const auto& g : groups) {
    const int newId = projectedAfterMove(g.groupId, groupId, clampedNew);
    if (newId == g.groupId)
      continue;

    QJsonObject row;
    row[QStringLiteral("oldGroupId")]   = g.groupId;
    row[QStringLiteral("newGroupId")]   = newId;
    row[QStringLiteral("title")]        = g.title;
    row[QStringLiteral("datasetCount")] = static_cast<int>(g.datasets.size());
    renumbered.append(row);
  }

  qint64 preEpoch = 0;
  if (!isDryRun) {
    preEpoch = captureProjectEpoch();
    pm.moveGroup(groupId, newPosition);
  }

  QJsonObject result;
  if (isDryRun)
    result[QStringLiteral("dryRun")] = true;

  result[QStringLiteral("oldPosition")] = groupId;
  result[QStringLiteral("newPosition")] = clampedNew;
  result[QStringLiteral("moved")]       = !isDryRun;
  result[QStringLiteral("renumbered")]  = renumbered;

  QString warning;
  if (isDryRun)
    warning = QStringLiteral(
      "DRY RUN: no changes were written. Re-call without dryRun:true to commit. The "
      "renumbered[] array shows the new groupId for each affected group; uniqueIds stay "
      "stable across reorders.");

  else
    warning = QStringLiteral(
      "Group reorder renumbers groupId. Dataset uniqueIds and Group.uniqueId stay stable, "
      "so workspace refs and xAxisId references survive untouched.");

  result[QStringLiteral("warning")] = warning;

  if (!isDryRun)
    appendStaleProjectWarning(result, params, preEpoch);

  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns datasets in the order FrameBuilder traverses them.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetGetExecutionOrder(
  const QString& id, const QJsonObject& params)
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto& groups        = projectModel.groups();

  int total = 0;
  for (const auto& group : groups)
    total += static_cast<int>(group.datasets.size());

  const auto window = applyWindow(total,
                                  params.value(QStringLiteral("offset")).toInt(0),
                                  params.value(QStringLiteral("limit")).toInt(0));

  QJsonArray order;
  int position = 0;
  for (const auto& group : groups) {
    for (const auto& dataset : group.datasets) {
      if (position >= window.start && position < window.start + window.count) {
        QJsonObject entry;
        entry[QStringLiteral("position")]          = position;
        entry[Keys::UniqueId]                      = dataset.uniqueId;
        entry[Keys::Title]                         = dataset.title;
        entry[Keys::SourceId]                      = dataset.sourceId;
        entry[Keys::GroupId]                       = group.groupId;
        entry[Keys::DatasetId]                     = dataset.datasetId;
        entry[QStringLiteral("hasTransform")]      = !dataset.transformCode.isEmpty();
        entry[QStringLiteral("isVirtual")]         = dataset.virtual_;
        entry[QStringLiteral("transformLanguage")] = dataset.transformLanguage;
        order.append(entry);
      }

      ++position;
    }
  }

  QJsonObject result;
  result[QStringLiteral("order")] = order;
  result[QStringLiteral("count")] = total;
  attachWindowInfo(result, window, total);

  QJsonObject ex;
  ex[QStringLiteral("summary")] =
    QStringLiteral("Datasets execute in (group-array, dataset-array) order. A transform "
                   "may read raw values of ALL datasets via datasetGetRaw(uid), but only "
                   "final values of datasets EARLIER in this list via datasetGetFinal(uid).");
  result[QStringLiteral("_explanations")] = ex;
  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief List all actions
 */
API::CommandResponse API::Handlers::ProjectHandler::actionsList(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto& actions       = projectModel.actions();

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

namespace API::Handlers {

//--------------------------------------------------------------------------------------------------
// Project batch: run a sequence of commands under one autosave window
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs an array of project mutations sequentially under a suspended autosave.
 */
/**
 * @brief Returns the schema-hint object attached to every project.batch validation error.
 */
static QJsonObject buildBatchSchemaHint()
{
  QJsonObject example;
  QJsonArray exampleOps;
  QJsonObject op0;
  op0[QStringLiteral("command")] = QStringLiteral("project.dataset.update");
  QJsonObject p0;
  p0[QStringLiteral("groupId")] = 0;
  p0[Keys::DatasetId]           = 0;
  p0[QStringLiteral("title")]   = QStringLiteral("LED 1");
  p0[QStringLiteral("index")]   = 1;
  op0[QStringLiteral("params")] = p0;
  exampleOps.append(op0);
  example[QStringLiteral("ops")]         = exampleOps;
  example[QStringLiteral("stopOnError")] = false;

  QJsonObject hint;
  hint[QStringLiteral("expected")] =
    QStringLiteral("{ ops: Array<{command: string, params: object}>, stopOnError?: boolean }");
  hint[QStringLiteral("opShape")] =
    QStringLiteral("Each op MUST be {command: '<registered name>', params: {...}}. Per-call "
                   "args go INSIDE params, not at the top of the op object.");
  hint[QStringLiteral("limits")] =
    QStringLiteral("1 <= ops.length <= 1024. Nested project.batch is rejected.");
  hint[QStringLiteral("example")] = example;
  return hint;
}

/**
 * @brief Builds a per-op result entry for a validation failure inside project.batch.
 */
static QJsonObject buildBatchErrorEntry(int index,
                                        const QString& command,
                                        const QString& code,
                                        const QString& message,
                                        const QJsonObject& data)
{
  QJsonObject entry;
  entry[QStringLiteral("index")] = index;
  if (!command.isEmpty())
    entry[QStringLiteral("command")] = command;

  entry[QStringLiteral("success")] = false;
  QJsonObject err;
  err[QStringLiteral("code")]    = code;
  err[QStringLiteral("message")] = message;
  if (!data.isEmpty())
    err[QStringLiteral("data")] = data;

  entry[QStringLiteral("error")] = err;
  return entry;
}

/**
 * @brief Commands that honour `dryRun:true` -- used to validate batch previews.
 */
static const QSet<QString>& dryRunAwareCommands()
{
  static const QSet<QString> kSet = {
    QStringLiteral("project.dataset.delete"),
    QStringLiteral("project.group.delete"),
    QStringLiteral("project.dataset.move"),
    QStringLiteral("project.group.move"),
    QStringLiteral("project.workspace.delete"),
    QStringLiteral("project.workspace.clearAll"),
    QStringLiteral("project.new"),
    QStringLiteral("project.open"),
    QStringLiteral("project.loadJson"),
    QStringLiteral("project.template.apply"),
    QStringLiteral("project.batch"),
    QStringLiteral("assistant.project.bulkApply"),
  };
  return kSet;
}

/**
 * @brief Validates and executes a single project.batch op; returns the per-op result entry.
 */
static QJsonObject executeBatchOp(int index, const QJsonObject& op, bool dryRun, bool& success)
{
  success = false;

  if (op.isEmpty()) {
    return buildBatchErrorEntry(
      index,
      QString(),
      API::ErrorCode::InvalidParam,
      QStringLiteral("ops[%1] must be an object of shape {command: string, params: object}")
        .arg(index),
      buildBatchSchemaHint());
  }

  const auto command = op.value(QStringLiteral("command")).toString();
  auto opParams      = op.value(QStringLiteral("params")).toObject();

  if (command.isEmpty()) {
    return buildBatchErrorEntry(index,
                                QString(),
                                API::ErrorCode::MissingParam,
                                QStringLiteral("ops[%1].command is required (each op is "
                                               "{command: '<registered name>', params: {...}})")
                                  .arg(index),
                                buildBatchSchemaHint());
  }

  if (command == QStringLiteral("project.batch")) {
    return buildBatchErrorEntry(index,
                                command,
                                API::ErrorCode::InvalidParam,
                                QStringLiteral("project.batch cannot be nested"),
                                QJsonObject());
  }

  if (dryRun)
    opParams.insert(QStringLiteral("dryRun"), true);

  static auto& commandRegistry = API::CommandRegistry::instance();
  const auto response          = commandRegistry.execute(command, QString::number(index), opParams);

  QJsonObject entry;
  entry[QStringLiteral("index")]   = index;
  entry[QStringLiteral("command")] = command;
  entry[QStringLiteral("success")] = response.success;
  if (response.success) {
    if (!response.result.isEmpty())
      entry[QStringLiteral("result")] = response.result;

    success = true;
  } else {
    QJsonObject err;
    err[QStringLiteral("code")]    = response.errorCode;
    err[QStringLiteral("message")] = response.errorMessage;
    if (!response.errorData.isEmpty())
      err[QStringLiteral("data")] = response.errorData;

    entry[QStringLiteral("error")] = err;
  }
  return entry;
}

/**
 * @brief Validate the `ops` array against the batch handler's preconditions; returns an error
 * response when invalid.
 */
static std::optional<API::CommandResponse> validateBatchOps(const QString& id,
                                                            const QJsonObject& params,
                                                            QJsonArray& outOps)
{
  constexpr int kMaxBatchOps = 1024;

  if (!params.contains(QStringLiteral("ops")))
    return API::CommandResponse::makeError(id,
                                           API::ErrorCode::MissingParam,
                                           QStringLiteral("Missing required parameter: ops"),
                                           buildBatchSchemaHint());

  if (!params.value(QStringLiteral("ops")).isArray())
    return API::CommandResponse::makeError(id,
                                           API::ErrorCode::InvalidParam,
                                           QStringLiteral("ops must be an array"),
                                           buildBatchSchemaHint());

  outOps = params.value(QStringLiteral("ops")).toArray();
  if (outOps.isEmpty())
    return API::CommandResponse::makeError(id,
                                           API::ErrorCode::InvalidParam,
                                           QStringLiteral("ops array must not be empty"),
                                           buildBatchSchemaHint());

  if (outOps.size() > kMaxBatchOps)
    return API::CommandResponse::makeError(
      id,
      API::ErrorCode::InvalidParam,
      QStringLiteral("ops array exceeds limit of %1 (got %2)")
        .arg(QString::number(kMaxBatchOps), QString::number(outOps.size())),
      buildBatchSchemaHint());

  return std::nullopt;
}

/**
 * @brief When dryRun is set, ensure every op supports dryRun; returns an error response if any does
 * not.
 */
static std::optional<API::CommandResponse> ensureBatchDryRunCompatible(const QString& id,
                                                                       const QJsonArray& ops)
{
  QJsonArray unsupported;
  for (int i = 0; i < ops.size(); ++i) {
    const auto cmd = ops.at(i).toObject().value(QStringLiteral("command")).toString();
    if (!dryRunAwareCommands().contains(cmd)) {
      QJsonObject row;
      row[QStringLiteral("index")]   = i;
      row[QStringLiteral("command")] = cmd;
      unsupported.append(row);
    }
  }
  if (unsupported.isEmpty())
    return std::nullopt;

  QJsonObject data;
  data[QStringLiteral("unsupportedOps")] = unsupported;
  data[QStringLiteral("supportedSet")] = QJsonArray::fromStringList(dryRunAwareCommands().values());
  return API::CommandResponse::makeError(
    id,
    API::ErrorCode::InvalidParam,
    QStringLiteral("dryRun rejected: %1 op(s) do not support dryRun. Either drop dryRun "
                   "for the whole batch or split the un-previewable ops out into a "
                   "separate batch.")
      .arg(unsupported.size()),
    data);
}

}  // namespace API::Handlers

/**
 * @brief Runs an array of project mutations under a single suspended-autosave window.
 */
API::CommandResponse API::Handlers::ProjectHandler::projectBatch(const QString& id,
                                                                 const QJsonObject& params)
{
  QJsonArray ops;
  if (const auto invalid = validateBatchOps(id, params, ops); invalid)
    return *invalid;

  const bool stopOnError = params.value(QStringLiteral("stopOnError")).toBool(false);
  const bool isDryRun    = params.value(QStringLiteral("dryRun")).toBool(false);

  if (isDryRun) {
    if (const auto invalid = ensureBatchDryRunCompatible(id, ops); invalid)
      return *invalid;
  }

  static auto& project = DataModel::ProjectModel::instance();
  if (!isDryRun)
    project.setAutoSaveSuspended(true);

  QJsonArray results;
  int successCount = 0;
  int failureCount = 0;
  bool aborted     = false;

  for (int i = 0; i < ops.size(); ++i) {
    bool opSucceeded  = false;
    QJsonObject entry = executeBatchOp(i, ops.at(i).toObject(), isDryRun, opSucceeded);
    results.append(entry);
    if (opSucceeded)
      ++successCount;
    else
      ++failureCount;

    if (!opSucceeded && stopOnError) {
      aborted = true;
      break;
    }
  }

  if (!isDryRun) {
    project.setAutoSaveSuspended(false);
    project.flushAutoSave();
  }

  QJsonObject result;
  if (isDryRun)
    result[QStringLiteral("dryRun")] = true;

  result[QStringLiteral("results")]   = results;
  result[QStringLiteral("total")]     = ops.size();
  result[QStringLiteral("succeeded")] = successCount;
  result[QStringLiteral("failed")]    = failureCount;
  result[QStringLiteral("aborted")]   = aborted;
  result[QStringLiteral("autoSaveMode")] =
    isDryRun ? QStringLiteral("none") : QStringLiteral("flushed");
  if (isDryRun)
    result[QStringLiteral("warning")] =
      QStringLiteral("DRY RUN: no ops were committed. Each op's per-result still carries the "
                     "affected-entity payload as if it had run.");

  return CommandResponse::makeSuccess(id, result);
}
