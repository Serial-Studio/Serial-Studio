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
#include "DataModel/Scripting/IScriptEngine.h"
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

namespace API::Handlers {

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Summarise a serialised project JSON object for dryRun replies (no model mutation).
 */
static QJsonObject summarizeProjectJson(const QJsonObject& project)
{
  QJsonObject out;
  out[QStringLiteral("title")] = project.value(QStringLiteral("title")).toString();

  const auto groups                 = project.value(QStringLiteral("groups")).toArray();
  out[QStringLiteral("groupCount")] = groups.size();

  int datasetCount = 0;
  QJsonArray groupTitles;
  for (const auto& gv : groups) {
    const auto g  = gv.toObject();
    datasetCount += g.value(QStringLiteral("datasets")).toArray().size();
    if (groupTitles.size() < 32)
      groupTitles.append(g.value(QStringLiteral("title")).toString());
  }
  out[QStringLiteral("datasetCount")] = datasetCount;
  out[QStringLiteral("groupTitles")]  = groupTitles;

  const auto sources                 = project.value(QStringLiteral("sources")).toArray();
  out[QStringLiteral("sourceCount")] = sources.isEmpty() ? 1 : sources.size();
  return out;
}

/**
 * @brief Summarise the current ProjectModel for the `wouldDiscard` half of a dryRun reply.
 */
static QJsonObject summarizeCurrentProject()
{
  static const auto& pm = DataModel::ProjectModel::instance();
  QJsonObject out;
  out[QStringLiteral("title")]        = pm.title();
  out[QStringLiteral("groupCount")]   = pm.groupCount();
  out[QStringLiteral("datasetCount")] = pm.datasetCount();
  out[QStringLiteral("sourceCount")]  = pm.sourceCount();
  out[QStringLiteral("filePath")]     = pm.jsonFilePath();

  QJsonArray groupTitles;
  for (const auto& g : pm.groups())
    if (groupTitles.size() < 32)
      groupTitles.append(g.title);

  out[QStringLiteral("groupTitles")] = groupTitles;
  return out;
}

}  // namespace API::Handlers

/**
 * @brief Reset the active project to a blank slate.
 */
API::CommandResponse API::Handlers::ProjectHandler::fileNew(const QString& id,
                                                            const QJsonObject& params)
{
  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);

  if (isDryRun) {
    QJsonObject wouldCreate;
    wouldCreate[QStringLiteral("title")]        = QStringLiteral("Untitled Project");
    wouldCreate[QStringLiteral("groupCount")]   = 0;
    wouldCreate[QStringLiteral("datasetCount")] = 0;
    wouldCreate[QStringLiteral("sourceCount")]  = 1;

    QJsonObject result;
    result[QStringLiteral("dryRun")]       = true;
    result[QStringLiteral("wouldDiscard")] = summarizeCurrentProject();
    result[QStringLiteral("wouldCreate")]  = wouldCreate;
    result[QStringLiteral("warning")] =
      QStringLiteral("DRY RUN: no project was reset. The current project (wouldDiscard) "
                     "would be replaced by a blank one. Confirm before re-issuing without "
                     "dryRun:true.");
    return CommandResponse::makeSuccess(id, result);
  }

  static auto& projectModel = DataModel::ProjectModel::instance();
  projectModel.setSuppressMessageBoxes(true);
  projectModel.newJsonFile();
  projectModel.setSuppressMessageBoxes(false);

  static auto& appState = AppState::instance();
  appState.setOperationMode(SerialStudio::ProjectFile);

  QJsonObject result;
  result[QStringLiteral("created")] = true;
  result[QStringLiteral("title")]   = projectModel.title();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Undo the most recent project-document step (shared UI/API history).
 */
API::CommandResponse API::Handlers::ProjectHandler::projectUndo(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params);
  static auto& projectModel = DataModel::ProjectModel::instance();

  QJsonObject result;
  if (!projectModel.canUndo()) {
    result[QStringLiteral("performed")] = false;
    result[QStringLiteral("reason")]    = QStringLiteral("nothing to undo");
    attachProjectEpoch(result);
    return CommandResponse::makeSuccess(id, result);
  }

  const QString label = projectModel.undoText();
  projectModel.setSuppressMessageBoxes(true);
  const bool performed = projectModel.undo();
  projectModel.setSuppressMessageBoxes(false);

  if (!performed) {
    result[QStringLiteral("performed")] = false;
    result[QStringLiteral("reason")]    = QStringLiteral("snapshot apply failed");
    attachProjectEpoch(result);
    return CommandResponse::makeSuccess(id, result);
  }

  result[QStringLiteral("performed")] = true;
  result[QStringLiteral("undone")]    = label;
  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Redo the most recently undone project-document step.
 */
API::CommandResponse API::Handlers::ProjectHandler::projectRedo(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params);
  static auto& projectModel = DataModel::ProjectModel::instance();

  QJsonObject result;
  if (!projectModel.canRedo()) {
    result[QStringLiteral("performed")] = false;
    result[QStringLiteral("reason")]    = QStringLiteral("nothing to redo");
    attachProjectEpoch(result);
    return CommandResponse::makeSuccess(id, result);
  }

  const QString label = projectModel.redoText();
  projectModel.setSuppressMessageBoxes(true);
  const bool performed = projectModel.redo();
  projectModel.setSuppressMessageBoxes(false);

  if (!performed) {
    result[QStringLiteral("performed")] = false;
    result[QStringLiteral("reason")]    = QStringLiteral("snapshot apply failed");
    attachProjectEpoch(result);
    return CommandResponse::makeSuccess(id, result);
  }

  result[QStringLiteral("performed")] = true;
  result[QStringLiteral("redone")]    = label;
  attachProjectEpoch(result);
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

  static auto& projectModel = DataModel::ProjectModel::instance();
  projectModel.setTitle(title);

  QJsonObject result;
  result[QStringLiteral("title")] = projectModel.title();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Open project file
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

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);

  if (isDryRun) {
    QFile f(file_path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
      return CommandResponse::makeError(
        id,
        ErrorCode::OperationFailed,
        QStringLiteral("DRY RUN: cannot read filePath '%1'").arg(file_path));

    QJsonParseError err{};
    const auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    f.close();
    if (err.error != QJsonParseError::NoError || !doc.isObject())
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("DRY RUN: filePath does not contain a valid project JSON object."));

    QJsonObject result;
    result[QStringLiteral("dryRun")]       = true;
    result[QStringLiteral("filePath")]     = file_path;
    result[QStringLiteral("wouldDiscard")] = summarizeCurrentProject();
    result[QStringLiteral("wouldApply")]   = summarizeProjectJson(doc.object());
    result[QStringLiteral("warning")] =
      QStringLiteral("DRY RUN: no project was loaded. wouldDiscard shows what would be lost; "
                     "wouldApply shows what would replace it. Confirm before re-issuing "
                     "without dryRun:true.");
    return CommandResponse::makeSuccess(id, result);
  }

  static auto& projectModel = DataModel::ProjectModel::instance();
  projectModel.setSuppressMessageBoxes(true);
  const bool ok = projectModel.openJsonFile(file_path);
  projectModel.setSuppressMessageBoxes(false);

  if (!ok) {
    return CommandResponse::makeError(
      id,
      ErrorCode::OperationFailed,
      QStringLiteral("Failed to open project file (validation or I/O error)"));
  }

  static auto& appState = AppState::instance();
  appState.setOperationMode(SerialStudio::ProjectFile);

  QJsonObject result;
  result[QStringLiteral("filePath")] = projectModel.jsonFilePath();
  result[QStringLiteral("title")]    = projectModel.title();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Save project
 */
API::CommandResponse API::Handlers::ProjectHandler::fileSave(const QString& id,
                                                             const QJsonObject& params)
{
  const QString explicit_path = params.value(QStringLiteral("filePath")).toString();

  static auto& projectModel = DataModel::ProjectModel::instance();
  projectModel.setSuppressMessageBoxes(true);
  bool success = false;
  if (!explicit_path.isEmpty()) {
    if (!API::isPathAllowed(explicit_path, true)) {
      projectModel.setSuppressMessageBoxes(false);
      return CommandResponse::makeError(
        id, ErrorCode::InvalidParam, QStringLiteral("filePath is not allowed"));
    }

    success = projectModel.apiSaveJsonFile(explicit_path);
  } else {
    const bool ask_path = params.value(QStringLiteral("askPath")).toBool(false);
    success             = projectModel.saveJsonFile(ask_path);
  }

  projectModel.setSuppressMessageBoxes(false);

  if (!success) {
    return CommandResponse::makeError(
      id, ErrorCode::OperationFailed, QStringLiteral("Failed to save project"));
  }

  QJsonObject result;
  result[QStringLiteral("saved")]    = true;
  result[QStringLiteral("filePath")] = projectModel.jsonFilePath();
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

  static auto& project = DataModel::ProjectModel::instance();

  QJsonObject result;
  result[QStringLiteral("title")]                      = project.title();
  result[QStringLiteral("filePath")]                   = project.jsonFilePath();
  result[QStringLiteral("modified")]                   = project.modified();
  result[QStringLiteral("groupCount")]                 = project.groupCount();
  result[QStringLiteral("datasetCount")]               = project.datasetCount();
  result[QStringLiteral("actionCount")]                = static_cast<int>(project.actions().size());
  result[QStringLiteral("containsCommercialFeatures")] = project.containsCommercialFeatures();

  return CommandResponse::makeSuccess(id, result);
}

namespace API::Handlers {

/**
 * @brief Returns a composite read of the entire project state.
 */
/**
 * @brief Builds the prose _explanations block for a single Source.
 */
static QJsonObject buildSourceExplanations(const DataModel::Source& src, bool verbose)
{
  QJsonObject ex;
  ex[Keys::BusType]             = API::EnumLabels::busTypeLabel(src.busType);
  ex[Keys::FrameParserLanguage] = API::EnumLabels::scriptLanguageLabel(src.frameParserLanguage);

  if (verbose) {
    ex[Keys::FrameDetection] = API::EnumLabels::frameDetectionLabel(src.frameDetection);
    ex[Keys::DecoderMethod]  = API::EnumLabels::decoderMethodLabel(src.decoderMethod);
  }

  const QString summary =
    QStringLiteral("%1 source running a %2 parser; frames split by %3.")
      .arg(API::EnumLabels::busTypeLabel(src.busType),
           API::EnumLabels::scriptLanguageLabel(src.frameParserLanguage),
           API::EnumLabels::frameDetectionLabel(src.frameDetection).toLower());
  ex[QStringLiteral("summary")] = summary;
  return ex;
}

/**
 * @brief Builds the per-source array used by project.snapshot.
 */
static QJsonArray buildSnapshotSources(const DataModel::ProjectModel& pm, bool verbose)
{
  QJsonArray sources;
  for (const auto& src : pm.sources()) {
    QJsonObject s;
    s[Keys::SourceId]                    = src.sourceId;
    s[Keys::Title]                       = src.title;
    s[Keys::BusType]                     = src.busType;
    s[Keys::FrameParserLanguage]         = src.frameParserLanguage;
    s[QStringLiteral("frameParserSize")] = src.frameParserCode.size();

    if (verbose) {
      s[Keys::FrameStart]        = src.frameStart;
      s[Keys::FrameEnd]          = src.frameEnd;
      s[Keys::ChecksumAlgorithm] = src.checksumAlgorithm;
      s[Keys::FrameDetection]    = src.frameDetection;
      s[Keys::FrameParserCode]   = src.frameParserCode;
    }

    s[QStringLiteral("_explanations")] = buildSourceExplanations(src, verbose);
    sources.append(s);
  }
  return sources;
}

/**
 * @brief Builds the prose _explanations block for a single Group.
 */
static QJsonObject buildGroupExplanations(const DataModel::Group& group)
{
  QJsonObject ex;
  const QString widgetSlug     = group.widget.simplified().toLower();
  ex[QStringLiteral("widget")] = widgetSlug.isEmpty()
                                 ? QStringLiteral("No group widget (datasets render independently)")
                                 : QStringLiteral("Group widget: %1").arg(widgetSlug);

  if (!group.painterCode.isEmpty())
    ex[QStringLiteral("painterCode")] =
      QStringLiteral("Group has %1 bytes of painter JS (paint(ctx, w, h) entry point).")
        .arg(group.painterCode.size());

  return ex;
}

/**
 * @brief Builds the groups+datasets array windowed by offset/limit (limit 0 = all); writes the
 *        project-wide dataset count (all groups, not just the window) to @a totalDatasets.
 */
static QJsonArray buildSnapshotGroups(const DataModel::ProjectModel& pm,
                                      int offset,
                                      int limit,
                                      int& totalDatasets)
{
  QJsonArray groups;
  totalDatasets   = 0;
  const auto& all = pm.groups();
  const int total = static_cast<int>(all.size());
  const int start = qBound(0, offset, total);
  const int count = limit > 0 ? qMin(limit, total - start) : total - start;

  for (int i = 0; i < total; ++i) {
    const auto& group  = all[static_cast<size_t>(i)];
    totalDatasets     += static_cast<int>(group.datasets.size());
    if (i < start || i >= start + count)
      continue;

    QJsonObject g;
    g[Keys::GroupId]                  = group.groupId;
    g[Keys::Title]                    = group.title;
    g[QStringLiteral("widget")]       = group.widget;
    g[QStringLiteral("datasetCount")] = static_cast<int>(group.datasets.size());

    QJsonArray ds;
    for (const auto& dataset : group.datasets)
      ds.append(buildDatasetObject(dataset, group));

    g[QStringLiteral("datasets")]      = ds;
    g[QStringLiteral("_explanations")] = buildGroupExplanations(group);
    groups.append(g);
  }
  return groups;
}

/**
 * @brief Builds the workspace summary array used by project.snapshot.
 */
static QJsonArray buildSnapshotWorkspaces(const DataModel::ProjectModel& pm)
{
  QJsonArray workspaces;
  for (const auto& ws : pm.editorWorkspaces()) {
    QJsonObject w;
    w[Keys::WorkspaceId]             = ws.workspaceId;
    w[Keys::Title]                   = ws.title;
    w[QStringLiteral("widgetCount")] = static_cast<int>(ws.widgetRefs.size());
    workspaces.append(w);
  }
  return workspaces;
}

/**
 * @brief Builds the data-tables summary array used by project.snapshot.
 */
static QJsonArray buildSnapshotTables(const DataModel::ProjectModel& pm)
{
  QJsonArray tables;
  const auto& folders = pm.editorTableFolders();
  for (const auto& t : pm.tables()) {
    QJsonObject tbl;
    tbl[Keys::Title]            = t.name;
    tbl[QStringLiteral("path")] = DataModel::tableFullPath(folders, t.parentFolderId, t.name);
    tbl[QStringLiteral("registerCount")] = static_cast<int>(t.registers.size());

    const auto constants = std::count_if(t.registers.begin(), t.registers.end(), [](const auto& r) {
      return r.type == DataModel::RegisterType::Constant;
    });
    const auto computed  = std::count_if(t.registers.begin(), t.registers.end(), [](const auto& r) {
      return r.type == DataModel::RegisterType::Computed;
    });

    tbl[QStringLiteral("constantCount")] = static_cast<int>(constants);
    tbl[QStringLiteral("computedCount")] = static_cast<int>(computed);
    tables.append(tbl);
  }
  return tables;
}

/**
 * @brief Returns project.snapshot's caller hint, with a bulk-edit nudge above 10 datasets.
 */
static QString buildSnapshotHint(int totalDatasets)
{
  QString hint =
    QStringLiteral("Pass verbose=true to include frame parser source and source-level frame "
                   "settings. For per-table register details, call project.dataTable.get with "
                   "the table name.");
  if (totalDatasets >= 10)
    hint += QStringLiteral(" %1 datasets present -- bulk edits should go through project.batch "
                           "(ops: [{command, params}, ...]) instead of looping individual updates; "
                           "project.dataset.addMany handles 'create N similar' patterns. If this "
                           "snapshot is too large, pass sections:[\"groups\"] with offset/limit "
                           "to page the groups array (reply carries nextOffset).")
              .arg(totalDatasets);

  return hint;
}

/**
 * @brief Builds the top-level _explanations object summarizing the project's operating shape.
 */
static QJsonObject buildSnapshotExplanations(const DataModel::ProjectModel& pm,
                                             int operationMode,
                                             int totalDatasets)
{
  QJsonObject ex;
  ex[QStringLiteral("operationMode")] = API::EnumLabels::operationModeLabel(operationMode);

  const int sourceCount    = static_cast<int>(pm.sources().size());
  const int groupCount     = static_cast<int>(pm.groups().size());
  const int workspaceCount = static_cast<int>(pm.editorWorkspaces().size());
  const int tableCount     = static_cast<int>(pm.tables().size());

  QString summary;
  if (operationMode == SerialStudio::ConsoleOnly) {
    summary = QStringLiteral("Console-only mode: raw bytes go to the terminal, no parsing, "
                             "no dashboard. Dataset/group config is inert until you switch to "
                             "ProjectFile or QuickPlot via project.frameParser.update.");
  } else if (operationMode == SerialStudio::QuickPlot) {
    summary = QStringLiteral("Quick Plot mode: auto CSV plotting on CR/LF/CRLF delimiters. "
                             "Frame parser code and most project config are bypassed -- "
                             "Quick Plot generates datasets from comma-separated fields on "
                             "each line.");
  } else {
    summary =
      QStringLiteral("Project File mode: %1 source(s), %2 group(s), %3 dataset(s), "
                     "%4 workspace(s), %5 data table(s). Frame parser code is authoritative.")
        .arg(sourceCount)
        .arg(groupCount)
        .arg(totalDatasets)
        .arg(workspaceCount)
        .arg(tableCount);
  }

  if (totalDatasets >= 3)
    summary += QStringLiteral(" Loops touching multiple datasets should use project.batch.");

  ex[QStringLiteral("summary")] = summary;
  return ex;
}

}  // namespace API::Handlers

/**
 * @brief Returns a structured snapshot of the active project (sources, groups, tables, hint),
 *        filtered by the optional sections list and with the groups array paged by offset/limit.
 */
API::CommandResponse API::Handlers::ProjectHandler::projectSnapshot(const QString& id,
                                                                    const QJsonObject& params)
{
  const bool verbose    = params.value(QStringLiteral("verbose")).toBool(false);
  const int offset      = params.value(QStringLiteral("offset")).toInt(0);
  const int limit       = params.value(QStringLiteral("limit")).toInt(0);
  const auto sections   = params.value(QStringLiteral("sections")).toArray();
  static const auto& pm = DataModel::ProjectModel::instance();

  const auto wants = [&sections](QLatin1String section) {
    return sections.isEmpty() || sections.contains(QJsonValue(section));
  };

  QJsonObject snapshot;
  snapshot[Keys::Title]                = pm.title();
  snapshot[Keys::PointCount]           = pm.pointCount();
  snapshot[QStringLiteral("filePath")] = pm.jsonFilePath();
  snapshot[QStringLiteral("modified")] = pm.modified();
  if (wants(QLatin1String("sources")))
    snapshot[QStringLiteral("sources")] = buildSnapshotSources(pm, verbose);

  const int group_total = static_cast<int>(pm.groups().size());
  int totalDatasets     = 0;
  if (wants(QLatin1String("groups"))) {
    const QJsonArray groups            = buildSnapshotGroups(pm, offset, limit, totalDatasets);
    snapshot[QStringLiteral("groups")] = groups;
    const int start                    = qBound(0, offset, group_total);
    if (start + groups.size() < group_total)
      snapshot[QStringLiteral("nextOffset")] = start + groups.size();
  } else {
    for (const auto& group : pm.groups())
      totalDatasets += static_cast<int>(group.datasets.size());
  }

  snapshot[QStringLiteral("groupCount")]   = group_total;
  snapshot[QStringLiteral("datasetCount")] = totalDatasets;
  if (wants(QLatin1String("workspaces")))
    snapshot[QStringLiteral("workspaces")] = buildSnapshotWorkspaces(pm);

  if (wants(QLatin1String("dataTables")))
    snapshot[QStringLiteral("dataTables")] = buildSnapshotTables(pm);

  static auto& appState                     = AppState::instance();
  const int operationMode                   = static_cast<int>(appState.operationMode());
  snapshot[QStringLiteral("operationMode")] = operationMode;
  snapshot[QStringLiteral("_explanations")] =
    buildSnapshotExplanations(pm, operationMode, totalDatasets);

  QJsonObject result;
  result[QStringLiteral("snapshot")] = snapshot;
  result[QStringLiteral("hint")]     = buildSnapshotHint(totalDatasets);
  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Load project configuration from JSON object
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

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);
  if (isDryRun) {
    QJsonObject result;
    result[QStringLiteral("dryRun")]       = true;
    result[QStringLiteral("wouldDiscard")] = summarizeCurrentProject();
    result[QStringLiteral("wouldApply")]   = summarizeProjectJson(config);
    result[QStringLiteral("warning")] =
      QStringLiteral("DRY RUN: no project was loaded. wouldDiscard shows what would be lost; "
                     "wouldApply shows what would replace it. Confirm before re-issuing "
                     "without dryRun:true.");
    return CommandResponse::makeSuccess(id, result);
  }

  static auto& project = DataModel::ProjectModel::instance();
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
 * @brief Export current project configuration as JSON
 */
API::CommandResponse API::Handlers::ProjectHandler::exportJson(const QString& id,
                                                               const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& project   = DataModel::ProjectModel::instance();
  const QJsonObject json = project.serializeToJson();

  QJsonObject result;
  result[QStringLiteral("config")] = json;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Load current project JSON into FrameBuilder
 */
API::CommandResponse API::Handlers::ProjectHandler::loadIntoFrameBuilder(const QString& id,
                                                                         const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& project = DataModel::ProjectModel::instance();
  static auto& builder = DataModel::FrameBuilder::instance();

  const bool hasDatasetlessGroup =
    std::any_of(project.groups().begin(), project.groups().end(), [](const DataModel::Group& g) {
      return g.widget == QLatin1String("image") || g.widget == QLatin1String("painter");
    });

  if (project.groupCount() == 0 || (project.datasetCount() == 0 && !hasDatasetlessGroup)) {
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

namespace API::Handlers {

//--------------------------------------------------------------------------------------------------
// Project templates: starter projects loaded from rcc:/ai/templates/*
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the templates manifest as a QJsonDocument, or empty on error.
 */
static QJsonDocument loadTemplateManifest()
{
  QFile f(QStringLiteral(":/ai/templates/manifest.json"));
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    return {};

  return QJsonDocument::fromJson(f.readAll());
}

/**
 * @brief Loads a template body file by id. Returns empty doc when not found.
 */
static QJsonDocument loadTemplateBodyById(const QString& templateId)
{
  const auto manifest = loadTemplateManifest().object();
  const auto entries  = manifest.value(QStringLiteral("templates")).toArray();
  for (const auto& v : entries) {
    const auto entry = v.toObject();
    if (entry.value(QStringLiteral("id")).toString() != templateId)
      continue;

    const auto file = entry.value(QStringLiteral("file")).toString();
    QFile body(QStringLiteral(":/ai/templates/") + file);
    if (!body.open(QIODevice::ReadOnly | QIODevice::Text))
      return {};

    return QJsonDocument::fromJson(body.readAll());
  }
  return {};
}

}  // namespace API::Handlers

/**
 * @brief Returns the templates manifest.
 */
API::CommandResponse API::Handlers::ProjectHandler::templateList(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto manifest = loadTemplateManifest();
  if (manifest.isNull())
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Failed to load templates manifest"));

  QJsonObject result;
  result[QStringLiteral("templates")] = manifest.object().value(QStringLiteral("templates"));
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Loads a template into the project model.
 */
API::CommandResponse API::Handlers::ProjectHandler::templateApply(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("templateId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: templateId"));

  const auto templateId = params.value(QStringLiteral("templateId")).toString();
  const auto body       = loadTemplateBodyById(templateId);
  if (body.isNull())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Template not found: %1. Call project.template.list for the "
                     "available ids.")
        .arg(templateId));

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);
  if (isDryRun) {
    QJsonObject result;
    result[QStringLiteral("dryRun")]       = true;
    result[QStringLiteral("templateId")]   = templateId;
    result[QStringLiteral("wouldDiscard")] = summarizeCurrentProject();
    result[QStringLiteral("wouldApply")]   = summarizeProjectJson(body.object());
    result[QStringLiteral("warning")] =
      QStringLiteral("DRY RUN: template not applied. wouldDiscard shows what would be lost; "
                     "wouldApply shows what would replace it. Confirm before re-issuing "
                     "without dryRun:true.");
    return CommandResponse::makeSuccess(id, result);
  }

  static auto& project = DataModel::ProjectModel::instance();
  project.setSuppressMessageBoxes(true);
  const bool ok = project.loadFromJsonDocument(body);
  project.setSuppressMessageBoxes(false);

  if (!ok)
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Template applied but project failed to validate"));

  QJsonObject result;
  result[QStringLiteral("templateId")]   = templateId;
  result[QStringLiteral("title")]        = project.title();
  result[QStringLiteral("groupCount")]   = project.groupCount();
  result[QStringLiteral("datasetCount")] = project.datasetCount();
  return CommandResponse::makeSuccess(id, result);
}

namespace API::Handlers {

//--------------------------------------------------------------------------------------------------
// Project consistency validation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adds a structured issue row to the validate() result array.
 */
static void addIssue(QJsonArray& issues,
                     const QString& level,
                     const QString& location,
                     const QString& message)
{
  QJsonObject row;
  row[QStringLiteral("level")]    = level;
  row[QStringLiteral("location")] = location;
  row[QStringLiteral("message")]  = message;
  issues.append(row);
}

/**
 * @brief Builds a throwaway script engine for compile-only checks (never touches live m_engines).
 */
static std::unique_ptr<DataModel::IScriptEngine> makeScratchEngine(int language)
{
  if (language == SerialStudio::Native)
    return std::make_unique<DataModel::CFrameParser>();

  if (language == SerialStudio::Lua)
    return std::make_unique<DataModel::LuaScriptEngine>();

  return std::make_unique<DataModel::JsScriptEngine>();
}

/**
 * @brief Validates per-source frame parsers (compiles each, flags failures).
 */
static void validateSources(const std::vector<DataModel::Source>& sources,
                            QJsonArray& issues,
                            bool& ok)
{
  for (const auto& src : sources) {
    if (src.frameParserCode.isEmpty()) {
      addIssue(issues,
               QStringLiteral("info"),
               QStringLiteral("source[%1]").arg(src.sourceId),
               QStringLiteral("Source has no frame parser; raw frames will be dropped"));
      continue;
    }

    auto engine         = makeScratchEngine(src.frameParserLanguage);
    const bool compiled = engine->loadScript(src.frameParserCode, src.sourceId, false);
    if (!compiled) {
      ok = false;
      addIssue(issues,
               QStringLiteral("error"),
               QStringLiteral("source[%1].frameParser").arg(src.sourceId),
               QStringLiteral("Frame parser failed to compile or define parse()"));
    }
  }
}

/**
 * @brief Validates a single dataset's references and per-field invariants.
 */
static void validateDataset(const DataModel::Dataset& d,
                            const DataModel::Group& g,
                            const QSet<int>& sourceIds,
                            QSet<int>& datasetIndexes,
                            QJsonArray& issues,
                            bool& ok)
{
  const QString dloc = QStringLiteral("group[%1].dataset[%2]")
                         .arg(QString::number(g.groupId), QString::number(d.datasetId));

  if (!sourceIds.contains(d.sourceId)) {
    ok = false;
    addIssue(issues,
             QStringLiteral("error"),
             dloc,
             QStringLiteral("Dataset references missing source id %1").arg(d.sourceId));
  }

  if (d.title.trimmed().isEmpty())
    addIssue(issues, QStringLiteral("warning"), dloc, QStringLiteral("Dataset has no title"));

  if (!d.virtual_ && d.index <= 0)
    addIssue(issues,
             QStringLiteral("warning"),
             dloc,
             QStringLiteral("Non-computed dataset has index 0; nothing maps to it"));

  if (!d.virtual_ && datasetIndexes.contains(d.index)) {
    ok = false;
    addIssue(issues,
             QStringLiteral("error"),
             dloc,
             QStringLiteral("Dataset index %1 is already used on this source").arg(d.index));
  }
  datasetIndexes.insert(d.index);

  if (d.fft && d.fftSamples <= 0) {
    ok = false;
    addIssue(
      issues, QStringLiteral("error"), dloc, QStringLiteral("FFT enabled but fftSamples is 0"));
  }
}

/**
 * @brief Validates each group and its datasets against the source-id set.
 */
static void validateGroups(const std::vector<DataModel::Group>& groups,
                           const QSet<int>& sourceIds,
                           QJsonArray& issues,
                           bool& ok)
{
  QHash<int, QSet<int>> indexesBySource;
  for (const auto& g : groups) {
    const QString gloc = QStringLiteral("group[%1]").arg(g.groupId);

    if (g.title.trimmed().isEmpty())
      addIssue(issues, QStringLiteral("warning"), gloc, QStringLiteral("Group has no title"));

    if (g.datasets.empty())
      addIssue(issues,
               QStringLiteral("warning"),
               gloc,
               QStringLiteral("Group has no datasets; nothing to display"));

    if (!sourceIds.contains(g.sourceId)) {
      ok = false;
      addIssue(issues,
               QStringLiteral("error"),
               gloc,
               QStringLiteral("Group references missing source id %1").arg(g.sourceId));
    }

    QSet<int>& datasetIndexes = indexesBySource[g.sourceId];
    for (const auto& d : g.datasets)
      validateDataset(d, g, sourceIds, datasetIndexes, issues, ok);
  }
}

/**
 * @brief Validates actions (non-blocking sanity checks: titles, payloads).
 */
static void validateActions(const std::vector<DataModel::Action>& actions, QJsonArray& issues)
{
  for (const auto& a : actions) {
    const QString aloc = QStringLiteral("action[%1]").arg(a.actionId);
    if (a.title.trimmed().isEmpty())
      addIssue(issues, QStringLiteral("warning"), aloc, QStringLiteral("Action has no title"));

    if (a.txData.isEmpty())
      addIssue(
        issues, QStringLiteral("warning"), aloc, QStringLiteral("Action has no payload (txData)"));
  }
}

}  // namespace API::Handlers

/**
 * @brief Walks the project model, reports inconsistencies.
 */
API::CommandResponse API::Handlers::ProjectHandler::validate(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  static const auto& project = DataModel::ProjectModel::instance();
  const auto& groups         = project.groups();
  const auto& sources        = project.sources();
  const auto& actions        = project.actions();

  QJsonArray issues;
  bool ok = true;

  QSet<int> sourceIds;
  for (const auto& src : sources)
    sourceIds.insert(src.sourceId);

  if (groups.empty()) {
    addIssue(issues,
             QStringLiteral("warning"),
             QStringLiteral("project"),
             QStringLiteral("Project has no groups; no data will be visualized"));
  }

  validateSources(sources, issues, ok);
  validateGroups(groups, sourceIds, issues, ok);
  validateActions(actions, issues);

  QJsonObject result;
  result[QStringLiteral("ok")]          = ok;
  result[QStringLiteral("issues")]      = issues;
  result[QStringLiteral("groupCount")]  = static_cast<int>(groups.size());
  result[QStringLiteral("sourceCount")] = static_cast<int>(sources.size());
  result[QStringLiteral("actionCount")] = static_cast<int>(actions.size());
  result[QStringLiteral("issueCount")]  = issues.size();
  return CommandResponse::makeSuccess(id, result);
}
