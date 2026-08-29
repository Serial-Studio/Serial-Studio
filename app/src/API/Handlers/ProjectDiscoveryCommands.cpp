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

#include "API/Handlers/ProjectDiscoveryCommands.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSet>

#include "API/Handlers/ProjectApiSupport.h"
#include "API/SchemaBuilder.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"

using namespace API::Handlers::ProjectApiSupport;

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kSearchDefaultLimit   = 20;
static constexpr int kSearchMaxLimit       = 100;
static constexpr int kGroupGetDefaultLimit = 50;
static constexpr int kGroupGetMaxLimit     = 200;

static const QString kGroupIdSpacesHint = QStringLiteral(
  "Pass exactly one selector: groupId (positional, 0..N-1, shifts on reorder; what "
  "project.group.update and dataset CRUD take) OR uniqueId (stable persisted counter; "
  "can be far larger than the group count). Workspace widget refs carry the group's "
  "uniqueId under the key 'groupId', so a large id read from workspace data belongs in "
  "this command's uniqueId parameter. Find valid ids via project.search or "
  "project.group.list.");

/**
 * @brief Parsed and validated project.search filters shared by the per-type row collectors.
 */
struct SearchFilters {
  QString query;
  QSet<QString> types;
  bool hasGroupId  = false;
  bool hasSourceId = false;
  int groupId      = 0;
  int sourceId     = 0;

  /**
   * @brief Returns true when the type filter is empty or names this entity type.
   */
  [[nodiscard]] bool wantsType(const QString& type) const
  {
    return types.isEmpty() || types.contains(type);
  }
};

/**
 * @brief Case-insensitive substring match shared by every entity type.
 */
[[nodiscard]] static bool matchesQuery(const QString& text, const QString& query)
{
  return !text.isEmpty() && text.contains(query, Qt::CaseInsensitive);
}

/**
 * @brief Reads the `type` filter (string or array of strings) into filters; empty = all types.
 */
[[nodiscard]] static bool parseTypeFilter(const QJsonValue& value, QSet<QString>& out)
{
  static const QSet<QString> kValidTypes = {QStringLiteral("dataset"),
                                            QStringLiteral("group"),
                                            QStringLiteral("action"),
                                            QStringLiteral("source"),
                                            QStringLiteral("workspace"),
                                            QStringLiteral("table")};

  QStringList requested;
  if (value.isString())
    requested.append(value.toString());
  else if (value.isArray())
    for (const auto& v : value.toArray())
      requested.append(v.toString());
  else if (!value.isUndefined() && !value.isNull())
    return false;

  for (const auto& type : requested) {
    const auto clean = type.trimmed().toLower();
    if (!kValidTypes.contains(clean))
      return false;

    out.insert(clean);
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Per-type row collectors. The walk order is fixed (sources, groups, datasets, actions,
// workspaces, tables) so offset paging over an unchanged project is deterministic.
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends matching source rows in project order.
 */
static void appendSourceRows(const SearchFilters& filters, QJsonArray& out)
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  for (const auto& source : projectModel.sources()) {
    if (filters.hasSourceId && source.sourceId != filters.sourceId)
      continue;

    if (!matchesQuery(source.title, filters.query))
      continue;

    QJsonObject row;
    row[QStringLiteral("type")] = QStringLiteral("source");
    row[Keys::SourceId]         = source.sourceId;
    row[Keys::Title]            = source.title;
    out.append(row);
  }
}

/**
 * @brief Appends matching group rows (both id spaces included) in project order.
 */
static void appendGroupRows(const SearchFilters& filters, QJsonArray& out)
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  for (const auto& group : projectModel.groups()) {
    if (filters.hasSourceId && group.sourceId != filters.sourceId)
      continue;

    if (!matchesQuery(group.title, filters.query))
      continue;

    QJsonObject row;
    row[QStringLiteral("type")]         = QStringLiteral("group");
    row[Keys::GroupId]                  = group.groupId;
    row[Keys::UniqueId]                 = group.uniqueId;
    row[Keys::Title]                    = group.title;
    row[QStringLiteral("datasetCount")] = static_cast<int>(group.datasets.size());
    out.append(row);
  }
}

/**
 * @brief Appends one dataset row when the query hits its title, alias, or units; matchedField
 *        records a non-title hit so the caller knows why the row matched.
 */
static void appendDatasetRow(const DataModel::Group& group,
                             const DataModel::Dataset& dataset,
                             const SearchFilters& filters,
                             QJsonArray& out)
{
  const bool titleHit = matchesQuery(dataset.title, filters.query);
  const bool aliasHit = !titleHit && matchesQuery(dataset.alias, filters.query);
  const bool unitsHit = !titleHit && !aliasHit && matchesQuery(dataset.units, filters.query);
  if (!titleHit && !aliasHit && !unitsHit)
    return;

  QJsonObject row;
  row[QStringLiteral("type")]       = QStringLiteral("dataset");
  row[Keys::UniqueId]               = dataset.uniqueId;
  row[Keys::Title]                  = dataset.title;
  row[QStringLiteral("path")]       = QStringLiteral("%1/%2").arg(group.title, dataset.title);
  row[Keys::GroupId]                = group.groupId;
  row[QStringLiteral("groupTitle")] = group.title;
  row[QStringLiteral("index")]      = dataset.index;
  if (!dataset.units.isEmpty())
    row[QStringLiteral("units")] = dataset.units;

  if (aliasHit)
    row[QStringLiteral("matchedField")] = QStringLiteral("alias");
  else if (unitsHit)
    row[QStringLiteral("matchedField")] = QStringLiteral("units");

  out.append(row);
}

/**
 * @brief Appends matching dataset rows in project order, honoring group/source filters.
 */
static void appendDatasetRows(const SearchFilters& filters, QJsonArray& out)
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  for (const auto& group : projectModel.groups()) {
    if (filters.hasGroupId && group.groupId != filters.groupId)
      continue;

    for (const auto& dataset : group.datasets) {
      if (filters.hasSourceId && dataset.sourceId != filters.sourceId)
        continue;

      appendDatasetRow(group, dataset, filters, out);
    }
  }
}

/**
 * @brief Appends matching action rows in project order.
 */
static void appendActionRows(const SearchFilters& filters, QJsonArray& out)
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  for (const auto& action : projectModel.actions()) {
    if (filters.hasSourceId && action.sourceId != filters.sourceId)
      continue;

    if (!matchesQuery(action.title, filters.query))
      continue;

    QJsonObject row;
    row[QStringLiteral("type")] = QStringLiteral("action");
    row[Keys::ActionId]         = action.actionId;
    row[Keys::Title]            = action.title;
    out.append(row);
  }
}

/**
 * @brief Appends matching workspace rows in project order.
 */
static void appendWorkspaceRows(const SearchFilters& filters, QJsonArray& out)
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  for (const auto& workspace : projectModel.activeWorkspaces()) {
    if (!matchesQuery(workspace.title, filters.query))
      continue;

    QJsonObject row;
    row[QStringLiteral("type")]        = QStringLiteral("workspace");
    row[QStringLiteral("workspaceId")] = workspace.workspaceId;
    row[Keys::Title]                   = workspace.title;
    out.append(row);
  }
}

/**
 * @brief Appends matching data-table rows (tables are identified by name) in project order.
 */
static void appendTableRows(const SearchFilters& filters, QJsonArray& out)
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  for (const auto& table : projectModel.tables()) {
    if (!matchesQuery(table.name, filters.query))
      continue;

    QJsonObject row;
    row[QStringLiteral("type")] = QStringLiteral("table");
    row[QStringLiteral("name")] = table.name;
    row[Keys::Title]            = table.name;
    out.append(row);
  }
}

/**
 * @brief Collects every matching row across all requested types in the fixed walk order.
 */
[[nodiscard]] static QJsonArray collectSearchRows(const SearchFilters& filters)
{
  QJsonArray rows;
  if (filters.wantsType(QStringLiteral("source")))
    appendSourceRows(filters, rows);

  if (filters.wantsType(QStringLiteral("group")))
    appendGroupRows(filters, rows);

  if (filters.wantsType(QStringLiteral("dataset")))
    appendDatasetRows(filters, rows);

  if (filters.wantsType(QStringLiteral("action")))
    appendActionRows(filters, rows);

  if (filters.wantsType(QStringLiteral("workspace")))
    appendWorkspaceRows(filters, rows);

  if (filters.wantsType(QStringLiteral("table")))
    appendTableRows(filters, rows);

  return rows;
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the command class to the registry it publishes into.
 */
API::Handlers::ProjectDiscoveryCommands::ProjectDiscoveryCommands(CommandRegistry& registry)
  : m_registry(registry)
{}

/**
 * @brief Register the discovery commands (project.search, project.group.get).
 */
void API::Handlers::ProjectDiscoveryCommands::registerCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.search"),
    QStringLiteral("Case-insensitive substring search across every project entity type: "
                   "datasets (title, alias, units), groups, actions, sources, workspaces, and "
                   "data tables (title/name). Returns compact typed rows -- never full objects "
                   "-- with matchCount, window/nextOffset paging (default limit 20, max 100), "
                   "and projectEpoch. THE starting point for finding anything in a large "
                   "project; drill into hits with project.dataset.getByUniqueId / getByPath or "
                   "project.group.get."),
    makeSchema(
      {
        {QStringLiteral("query"),
         QStringLiteral("string"),
         QStringLiteral("Substring to find (case-insensitive); must be non-empty.")}
  },
      {{QStringLiteral("type"),
        QStringLiteral("string|array"),
        QStringLiteral("Restrict to one or more of: dataset, group, action, source, "
                       "workspace, table.")},
       {QString(Keys::GroupId),
        QStringLiteral("integer"),
        QStringLiteral("Only datasets in this positional groupId.")},
       {QString(Keys::SourceId),
        QStringLiteral("integer"),
        QStringLiteral("Only entities bound to this sourceId.")},
       {QStringLiteral("offset"),
        QStringLiteral("integer"),
        QStringLiteral("First match to return (default 0).")},
       {QStringLiteral("limit"),
        QStringLiteral("integer"),
        QStringLiteral("Max rows to return (default 20, max 100).")}}),
    &projectSearch);

  registry.registerCommand(
    QStringLiteral("project.group.get"),
    QStringLiteral("Read one group plus a compact dataset summary (datasetId, uniqueId, index, "
                   "title, units) without pulling the whole project. Address it by EITHER the "
                   "positional groupId (what group.update / dataset CRUD take) OR the stable "
                   "uniqueId (what workspace widget refs carry under the key 'groupId'); the "
                   "response returns both ids. The summary pages via offset/limit (default 50, "
                   "max 200)."),
    makeSchema(
      {
  },
      {{QString(Keys::GroupId),
        QStringLiteral("integer"),
        QStringLiteral("Positional group id (0..N-1, shifts on reorder). Mutually exclusive "
                       "with uniqueId.")},
       {QString(Keys::UniqueId),
        QStringLiteral("integer"),
        QStringLiteral("Stable persisted group id (sparse counter; what workspace refs "
                       "carry). Mutually exclusive with groupId.")},
       {QStringLiteral("offset"),
        QStringLiteral("integer"),
        QStringLiteral("First dataset-summary row to return (default 0).")},
       {QStringLiteral("limit"),
        QStringLiteral("integer"),
        QStringLiteral("Max dataset-summary rows (default 50, max 200).")}}),
    &groupGet);
}

//--------------------------------------------------------------------------------------------------
// project.search
//--------------------------------------------------------------------------------------------------

/**
 * @brief Substring search across every project entity type; compact typed rows, bounded paging.
 */
API::CommandResponse API::Handlers::ProjectDiscoveryCommands::projectSearch(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("query")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: query"));

  SearchFilters filters;
  filters.query = params.value(QStringLiteral("query")).toString().trimmed();
  if (filters.query.isEmpty())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("query cannot be empty. To enumerate everything, use project.group.list / "
                     "project.dataset.list (with offset/limit) or project.snapshot instead."));

  if (!parseTypeFilter(params.value(QStringLiteral("type")), filters.types))
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("type must be one (or an array) of: dataset, group, action, source, "
                     "workspace, table."));

  filters.hasGroupId  = params.contains(Keys::GroupId);
  filters.hasSourceId = params.contains(Keys::SourceId);
  if ((filters.hasGroupId && !params.value(Keys::GroupId).isDouble())
      || (filters.hasSourceId && !params.value(Keys::SourceId).isDouble()))
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("groupId / sourceId filters must be JSON numbers, not strings -- a "
                     "string-typed filter would silently match the wrong group/source."));

  filters.groupId  = params.value(Keys::GroupId).toInt();
  filters.sourceId = params.value(Keys::SourceId).toInt();

  const QJsonArray matches = collectSearchRows(filters);
  const int matchCount     = matches.size();

  const int wantedLimit = params.value(QStringLiteral("limit")).toInt(kSearchDefaultLimit);
  const int limit = qBound(1, wantedLimit > 0 ? wantedLimit : kSearchDefaultLimit, kSearchMaxLimit);
  const auto window =
    applyWindow(matchCount, params.value(QStringLiteral("offset")).toInt(0), limit);

  QJsonArray rows;
  for (int i = window.start; i < window.start + window.count; ++i)
    rows.append(matches.at(i));

  QString summary;
  if (matchCount > 0 && window.count == 0)
    summary = QStringLiteral("%1 matches for \"%2\"; the requested offset is past the end.")
                .arg(QString::number(matchCount), filters.query);
  else
    summary =
      QStringLiteral("%1 match%2 for \"%3\"%4.")
        .arg(QString::number(matchCount),
             matchCount == 1 ? QString() : QStringLiteral("es"),
             filters.query,
             window.count < matchCount ? QStringLiteral(" (showing %1-%2)")
                                           .arg(QString::number(window.start),
                                                QString::number(window.start + window.count - 1))
                                       : QString());

  QJsonObject result;
  result[QStringLiteral("_summary")]   = summary;
  result[QStringLiteral("query")]      = filters.query;
  result[QStringLiteral("matchCount")] = matchCount;
  result[QStringLiteral("rows")]       = rows;
  attachWindowInfo(result, window, matchCount);
  attachProjectEpoch(result);
  if (matchCount > 0)
    result[QStringLiteral("_hint")] =
      QStringLiteral("Drill into a match: project.dataset.getByUniqueId{uniqueId} or "
                     "project.dataset.getByPath{path} for datasets, project.group.get{groupId "
                     "or uniqueId} for groups.");

  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// project.group.get
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads one group by positional groupId or stable uniqueId; compact windowed summary.
 */
API::CommandResponse API::Handlers::ProjectDiscoveryCommands::groupGet(const QString& id,
                                                                       const QJsonObject& params)
{
  const bool byGroupId  = params.contains(Keys::GroupId);
  const bool byUniqueId = params.contains(Keys::UniqueId);
  if (!byGroupId && !byUniqueId)
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing selector. ") + kGroupIdSpacesHint);

  if (byGroupId && byUniqueId)
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Pass either groupId or uniqueId, not both. ")
                                        + kGroupIdSpacesHint);

  const auto selectorValue = params.value(byGroupId ? Keys::GroupId : Keys::UniqueId);
  if (!selectorValue.isDouble())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("The selector must be a JSON number, not a string -- a string-typed id "
                     "would silently resolve to group 0. ")
        + kGroupIdSpacesHint);

  static auto& projectModel     = DataModel::ProjectModel::instance();
  const auto& groups            = projectModel.groups();
  const int selector            = selectorValue.toInt();
  const DataModel::Group* match = nullptr;
  for (const auto& group : groups) {
    if ((byGroupId ? group.groupId : group.uniqueId) == selector) {
      match = &group;
      break;
    }
  }

  if (match == nullptr)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("No group matched %1=%2. ")
          .arg(QString(byGroupId ? Keys::GroupId : Keys::UniqueId), QString::number(selector))
        + kGroupIdSpacesHint);

  const int total       = static_cast<int>(match->datasets.size());
  const int wantedLimit = params.value(QStringLiteral("limit")).toInt(kGroupGetDefaultLimit);
  const int limit =
    qBound(1, wantedLimit > 0 ? wantedLimit : kGroupGetDefaultLimit, kGroupGetMaxLimit);
  const auto window = applyWindow(total, params.value(QStringLiteral("offset")).toInt(0), limit);

  QJsonArray summary;
  for (int i = window.start; i < window.start + window.count; ++i) {
    const auto& dataset = match->datasets[static_cast<size_t>(i)];
    QJsonObject row;
    row[Keys::DatasetId]         = dataset.datasetId;
    row[Keys::UniqueId]          = dataset.uniqueId;
    row[QStringLiteral("index")] = dataset.index;
    row[Keys::Title]             = dataset.title;
    if (!dataset.units.isEmpty())
      row[QStringLiteral("units")] = dataset.units;

    summary.append(row);
  }

  QJsonObject result;
  result[Keys::GroupId]                    = match->groupId;
  result[Keys::UniqueId]                   = match->uniqueId;
  result[Keys::Title]                      = match->title;
  result[QStringLiteral("widget")]         = match->widget;
  result[Keys::SourceId]                   = match->sourceId;
  result[QStringLiteral("datasetCount")]   = total;
  result[QStringLiteral("datasetSummary")] = summary;
  result[QStringLiteral("_summary")] =
    QStringLiteral("Group \"%1\" (groupId %2, uniqueId %3): %4 dataset%5.")
      .arg(match->title,
           QString::number(match->groupId),
           QString::number(match->uniqueId),
           QString::number(total),
           total == 1 ? QString() : QStringLiteral("s"));
  attachWindowInfo(result, window, total);
  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}
