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
#include <optional>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>

#include "API/CommandRegistry.h"
#include "API/EnumLabels.h"
#include "API/SchemaBuilder.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Locates a workspace by id in ProjectModel's list.
 */
[[nodiscard]] static auto findWorkspace(const std::vector<DataModel::Workspace>& ws, int wid)
{
  return std::find_if(ws.begin(), ws.end(), [wid](const auto& w) { return w.workspaceId == wid; });
}

/**
 * @brief Returns true when the project model is in ProjectFile mode and ready
 *        to accept workspace mutations.
 */
[[nodiscard]] static bool inProjectFileMode()
{
  return AppState::instance().operationMode() == SerialStudio::ProjectFile;
}

/**
 * @brief Builds the derived widgetId string for a workspace widget reference.
 */
[[nodiscard]] static QString widgetIdFor(int workspaceId, int widgetType, int groupId, int relIdx)
{
  const QString slug = API::EnumLabels::dashboardWidgetSlug(widgetType);
  return QStringLiteral("ws%1:%2:g%3:%4")
    .arg(QString::number(workspaceId), slug, QString::number(groupId), QString::number(relIdx));
}

/**
 * @brief Outcome of parsing a widgetId string into its component fields.
 */
struct ParsedWidgetId {
  int workspaceId   = -1;
  int widgetType    = -1;
  int groupId       = -1;
  int relativeIndex = -1;
  bool valid        = false;
};

/**
 * @brief Parses a "ws<id>:<slug>:g<gid>:<rel>" string back into its fields.
 */
[[nodiscard]] static ParsedWidgetId parseWidgetId(const QString& s)
{
  ParsedWidgetId out;
  const auto parts = s.split(QChar(':'));
  if (parts.size() != 4)
    return out;

  const auto& wsTok = parts.at(0);
  const auto& slug  = parts.at(1);
  const auto& gTok  = parts.at(2);
  const auto& rTok  = parts.at(3);

  if (!wsTok.startsWith(QStringLiteral("ws")) || !gTok.startsWith(QChar('g')))
    return out;

  bool wsOk    = false;
  bool gOk     = false;
  bool rOk     = false;
  const int ws = wsTok.mid(2).toInt(&wsOk);
  const int g  = gTok.mid(1).toInt(&gOk);
  const int r  = rTok.toInt(&rOk);
  if (!wsOk || !gOk || !rOk)
    return out;

  const int wt = API::EnumLabels::dashboardWidgetFromSlug(slug);
  if (wt < 0)
    return out;

  out.workspaceId   = ws;
  out.widgetType    = wt;
  out.groupId       = g;
  out.relativeIndex = r;
  out.valid         = true;
  return out;
}

/**
 * @brief Serialises a workspace's widget refs as a QJsonArray of objects.
 */
[[nodiscard]] static QJsonArray refsToJson(int workspaceId,
                                           const std::vector<DataModel::WidgetRef>& refs)
{
  QJsonArray arr;
  for (const auto& r : refs) {
    QJsonObject entry;
    entry[QStringLiteral("widgetType")]     = r.widgetType;
    entry[QStringLiteral("widgetTypeSlug")] = API::EnumLabels::dashboardWidgetSlug(r.widgetType);
    entry[QStringLiteral("groupId")]        = r.groupId;
    entry[QStringLiteral("relativeIndex")]  = r.relativeIndex;
    entry[QStringLiteral("widgetId")] =
      widgetIdFor(workspaceId, r.widgetType, r.groupId, r.relativeIndex);
    arr.append(entry);
  }

  return arr;
}

/**
 * @brief Returns a hint on how to unlock @a wtype on @a group, or empty when none applies.
 */
[[nodiscard]] static QString unlockHint(int wtype, int gid)
{
  switch (wtype) {
    case SerialStudio::DashboardPlot:
      return QStringLiteral(" To enable widgetType=9 (Plot), set DatasetOption "
                            "bit 1 via project.dataset.setOptions{groupId:%1, "
                            "datasetId:<n>, options:<currentBitfield|1>} or "
                            "project.dataset.update{groupId:%1, datasetId:<n>, "
                            "graph:true}.")
        .arg(gid);
    case SerialStudio::DashboardFFT:
      return QStringLiteral(" To enable widgetType=7 (FFT), set DatasetOption "
                            "bit 2 via project.dataset.setOptions{groupId:%1, "
                            "datasetId:<n>, options:<currentBitfield|2>} or "
                            "project.dataset.update{groupId:%1, datasetId:<n>, "
                            "fft:true}.")
        .arg(gid);
    case SerialStudio::DashboardBar:
      return QStringLiteral(" To enable widgetType=10 (Bar), set DatasetOption "
                            "bit 4 via project.dataset.setOptions{groupId:%1, "
                            "datasetId:<n>, options:<bitfield with bit 4>} "
                            "(clears Gauge/Compass).")
        .arg(gid);
    case SerialStudio::DashboardGauge:
      return QStringLiteral(" To enable widgetType=11 (Gauge), set DatasetOption "
                            "bit 8 via project.dataset.setOptions{groupId:%1, "
                            "datasetId:<n>, options:<bitfield with bit 8>} "
                            "(clears Bar/Compass).")
        .arg(gid);
    case SerialStudio::DashboardCompass:
      return QStringLiteral(" To enable widgetType=12 (Compass), set DatasetOption "
                            "bit 16 via project.dataset.setOptions{groupId:%1, "
                            "datasetId:<n>, options:<bitfield with bit 16>} "
                            "(clears Bar/Gauge).")
        .arg(gid);
    case SerialStudio::DashboardMeter:
      return QStringLiteral(" To enable widgetType=13 (Meter), set DatasetOption "
                            "bit 128 via project.dataset.setOptions{groupId:%1, "
                            "datasetId:<n>, options:<bitfield with bit 128>} "
                            "(clears Bar/Gauge/Compass/Thermometer).")
        .arg(gid);
    case SerialStudio::DashboardThermometer:
      return QStringLiteral(" To enable widgetType=14 (Thermometer), set DatasetOption "
                            "bit 256 via project.dataset.setOptions{groupId:%1, "
                            "datasetId:<n>, options:<bitfield with bit 256>} "
                            "(clears Bar/Gauge/Compass/Meter).")
        .arg(gid);
    case SerialStudio::DashboardLED:
      return QStringLiteral(" To enable widgetType=8 (LED), set DatasetOption "
                            "bit 32 via project.dataset.setOptions{groupId:%1, "
                            "datasetId:<n>, options:<currentBitfield|32>} or "
                            "project.dataset.update{groupId:%1, datasetId:<n>, "
                            "led:true}.")
        .arg(gid);
#ifdef BUILD_COMMERCIAL
    case SerialStudio::DashboardWaterfall:
      return QStringLiteral(" To enable widgetType=17 (Waterfall), set DatasetOption "
                            "bit 64 via project.dataset.setOptions{groupId:%1, "
                            "datasetId:<n>, options:<currentBitfield|64>} or "
                            "project.dataset.update{groupId:%1, datasetId:<n>, "
                            "waterfall:true}.")
        .arg(gid);
#endif
    default:
      return QStringLiteral(" widgetType=%1 is a group-shape widget -- change "
                            "the group's widget string via project.group.update "
                            "instead of a dataset option.")
        .arg(wtype);
  }
}

/**
 * @brief Returns the deduped DashboardWidget enums a group can render.
 */
[[nodiscard]] static QList<int> compatibleWidgetTypes(const DataModel::Group& group)
{
  QList<int> out;
  const auto group_widget = static_cast<int>(SerialStudio::getDashboardWidget(group));
  if (group_widget != SerialStudio::DashboardNoWidget)
    out.append(group_widget);

  for (const auto& ds : group.datasets) {
    for (const auto w : SerialStudio::getDashboardWidgets(ds)) {
      const auto v = static_cast<int>(w);
      if (!out.contains(v))
        out.append(v);
    }
  }

  return out;
}

/**
 * @brief Resolves widgetType from int or slug; returns -1 when slug is unknown.
 */
[[nodiscard]] static int resolveWidgetType(const QJsonValue& wtypeJson)
{
  if (wtypeJson.isString())
    return API::EnumLabels::dashboardWidgetFromSlug(wtypeJson.toString());

  return wtypeJson.toInt();
}

/**
 * @brief Validates wtype is compatible with the group; returns empty optional on success.
 */
[[nodiscard]] static std::optional<QString> validateGroupCompatibility(
  const DataModel::Group& group, int wtype, int gid)
{
  const auto compatible = compatibleWidgetTypes(group);
  if (compatible.contains(wtype))
    return std::nullopt;

  QStringList compat_strs;
  for (int v : compatible)
    compat_strs.append(QString::number(v));

  return QStringLiteral("widgetType=%1 is not compatible with group %2 ('%3'). "
                        "Compatible widgetTypes for this group: [%4].%5")
    .arg(wtype)
    .arg(gid)
    .arg(group.title)
    .arg(compat_strs.join(QStringLiteral(", ")))
    .arg(unlockHint(wtype, gid));
}

/**
 * @brief Picks the next free relativeIndex for (widgetType, groupId) on the workspace.
 */
[[nodiscard]] static int nextFreeRelativeIndex(const std::vector<DataModel::Workspace>& wsList,
                                               int wid,
                                               int wtype,
                                               int gid)
{
  const auto wsIt = std::find_if(
    wsList.begin(), wsList.end(), [wid](const auto& ws) { return ws.workspaceId == wid; });
  if (wsIt == wsList.end())
    return 0;

  QSet<int> taken;
  for (const auto& ref : wsIt->widgetRefs)
    if (ref.widgetType == wtype && ref.groupId == gid)
      taken.insert(ref.relativeIndex);

  int relIndex = 0;
  while (taken.contains(relIndex))
    ++relIndex;

  return relIndex;
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all workspace-related commands with the registry.
 */
void API::Handlers::WorkspacesHandler::registerCommands()
{
  registerWorkspaceCrudCommands();
  registerCustomizeCommands();
  registerWidgetRefCommands();
}

/**
 * @brief Register list / get / add / delete / rename / autoGenerate commands.
 */
void API::Handlers::WorkspacesHandler::registerWorkspaceCrudCommands()
{
  auto& registry   = CommandRegistry::instance();
  const auto empty = API::emptySchema();

  registry.registerCommand(QStringLiteral("project.workspace.list"),
                           QStringLiteral("List all workspaces with widget counts"),
                           empty,
                           &list);
  registry.registerCommand(
    QStringLiteral("project.workspace.get"),
    QStringLiteral("Return widget refs for a workspace (params: id)"),
    API::makeSchema({
      {QStringLiteral("id"), QStringLiteral("integer"), QStringLiteral("Workspace id")}
  }),
    &get);
  registry.registerCommand(
    QStringLiteral("project.workspace.add"),
    QStringLiteral("Create a new workspace (params: title=\"Workspace\"). Returns new id."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("title"), QStringLiteral("string"), QStringLiteral("Workspace title")}}),
    &add);
  registry.registerCommand(
    QStringLiteral("project.workspace.delete"),
    QStringLiteral("Delete a workspace (params: id)"),
    API::makeSchema({
      {QStringLiteral("id"), QStringLiteral("integer"), QStringLiteral("Workspace id")}
  }),
    &remove);
  registry.registerCommand(
    QStringLiteral("project.workspace.rename"),
    QStringLiteral("Rename a workspace (params: id, title)"),
    API::makeSchema({
      {   QStringLiteral("id"), QStringLiteral("integer"),        QStringLiteral("Workspace id")},
      {QStringLiteral("title"),  QStringLiteral("string"), QStringLiteral("New workspace title")}
  }),
    &rename);
  registry.registerCommand(
    QStringLiteral("project.workspace.update"),
    QStringLiteral("Patch workspace fields (params: id; optional title, icon, description)"),
    API::makeSchema(
      {
        {QStringLiteral("id"), QStringLiteral("integer"), QStringLiteral("Workspace id")}
  },
      {{QStringLiteral("title"),
        QStringLiteral("string"),
        QStringLiteral("New workspace title (optional)")},
       {QStringLiteral("icon"),
        QStringLiteral("string"),
        QStringLiteral("New workspace icon, e.g. 'qrc:/icons/panes/overview.svg' (optional)")},
       {QStringLiteral("description"),
        QStringLiteral("string"),
        QStringLiteral("Free-text intent / audience note for the workspace (optional)")}}),
    &update);
  registry.registerCommand(
    QStringLiteral("project.workspace.reorder"),
    QStringLiteral("Reorder user-defined workspaces (workspaceId >= 5000) in the taskbar. Pass "
                   "every user workspace id in the desired order -- partial reorders are rejected "
                   "to avoid silent corruption. System workspaces (auto-generated, ids 1000-4999) "
                   "keep their original slots."),
    API::makeSchema({
      {QStringLiteral("workspaceIds"),
       QStringLiteral("array"),
       QStringLiteral("Every user-workspace id (>= 5000), in the desired order. Must "
                      "exactly match the set of current user workspaces.")}
  }),
    &reorder);
  registry.registerCommand(
    QStringLiteral("project.workspace.autoGenerate"),
    QStringLiteral(
      "Materialise synthetic workspaces into the customised set. No-op if already customised."),
    empty,
    &autoGenerate);
}

/**
 * @brief Register customizeWorkspaces flag get/set commands.
 */
void API::Handlers::WorkspacesHandler::registerCustomizeCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("project.workspace.getCustomizeMode"),
                           QStringLiteral("Return the customizeWorkspaces flag"),
                           API::emptySchema(),
                           &customizeGet);
  registry.registerCommand(QStringLiteral("project.workspace.setCustomizeMode"),
                           QStringLiteral("Flip the customizeWorkspaces flag (params: enabled)"),
                           API::makeSchema({
                             {QStringLiteral("enabled"),
                              QStringLiteral("boolean"),
                              QStringLiteral("Enable (true) or disable (false)")}
  }),
                           &customizeSet);
}

/**
 * @brief Register widget-ref add/remove commands.
 */
void API::Handlers::WorkspacesHandler::registerWidgetRefCommands()
{
  registerWidgetAddCommand();
  registerWidgetRemoveCommand();
  registerWidgetValidationCommands();
}

/**
 * @brief Register the project.workspace.addWidget command.
 */
void API::Handlers::WorkspacesHandler::registerWidgetAddCommand()
{
  auto& registry = CommandRegistry::instance();

  const auto addSchema = API::makeSchema({
    {  QStringLiteral("workspaceId"),
     QStringLiteral("integer"),
     QStringLiteral("Workspace id from project.workspace.list. IDs 1000-4999 are "
     "auto-generated (Overview=1000, AllData=1001, per-group >= 1002); "
     "user-created workspaces start at 5000.")                                             },
    {   QStringLiteral("widgetType"),
     QStringLiteral("string|integer"),
     QStringLiteral("Widget kind. PREFERRED form: a string slug -- 'plot', 'fft', "
     "'bar', 'gauge', 'compass', 'led', 'datagrid', 'multiplot', "
     "'accelerometer', 'gyroscope', 'gps', 'plot3d', 'imageview' (Pro), "
     "'output-panel' (Pro), 'notification-log' (Pro), 'waterfall' (Pro), "
     "'painter' (Pro). Integer DashboardWidget enum still accepted for "
     "back-compat (1=DataGrid, 2=MultiPlot, ... 9=Plot, 10=Bar, 11=Gauge, "
     "12=Compass) but the strings are unambiguous and forwards-compatible.")               },
    {      QStringLiteral("groupId"),
     QStringLiteral("integer"),
     QStringLiteral("groupId from project.groups.list. Use group.id, NOT the array index.")},
    {QStringLiteral("relativeIndex"),
     QStringLiteral("integer"),
     QStringLiteral("OPTIONAL. Per-(widgetType,groupId) deduplication counter; NOT a "
     "dataset index. Omit to auto-assign the next free slot. Pass an "
     "explicit value only when you are reproducing a prior layout.")                       }
  });
  registry.registerCommand(
    QStringLiteral("project.workspace.addWidget"),
    QStringLiteral("Pin a visualization tile onto a workspace. The tile renders the "
                   "dashboard widget of the given widgetType fed by the given groupId. "
                   "MANDATORY pre-flight: call project.group.list (for groupId + "
                   "compatibleWidgetTypes) and project.workspace.list (for workspaceId) "
                   "BEFORE calling this command. Calling it without that information "
                   "produces validation errors. If your widgetType isn't in the group's "
                   "compatibleWidgetTypes, first flip the matching dataset option via "
                   "project.dataset.setOption / project.dataset.setOptions / "
                   "project.dataset.update {graph|fft|led|waterfall}, then re-list and "
                   "call addWidget.\n"
                   "Batching: for 5+ widget mutations, wrap calls in project.batch{ops:"
                   "[{command:'project.workspace.addWidget', params:{...}}, ...]} -- "
                   "workspace ops execute correctly inside batch and the whole burst "
                   "lands as a single autosave (see meta.describeCommand{name:"
                   "'project.batch'}). Returns widgetId for use with removeWidget."),
    addSchema,
    &widgetAdd);
}

/**
 * @brief Register the project.workspace.removeWidget command.
 */
void API::Handlers::WorkspacesHandler::registerWidgetRemoveCommand()
{
  auto& registry = CommandRegistry::instance();

  const auto removeSchema = API::makeSchema(
    {
  },
    {{QStringLiteral("widgetId"),
      QStringLiteral("string"),
      QStringLiteral("Opaque widget identifier in the form "
                     "'ws<workspaceId>:<slug>:g<groupId>:<relativeIndex>' "
                     "(returned by project.workspace.addWidget and "
                     "project.workspace.get). Provide EITHER widgetId OR the four "
                     "tuple fields below.")},
     {QStringLiteral("workspaceId"), QStringLiteral("integer"), QStringLiteral("Workspace id")},
     {QStringLiteral("widgetType"),
      QStringLiteral("string|integer"),
      QStringLiteral("SerialStudio.DashboardWidget slug or enum (tuple form)")},
     {QStringLiteral("groupId"), QStringLiteral("integer"), QStringLiteral("Source group id")},
     {QStringLiteral("relativeIndex"),
      QStringLiteral("integer"),
      QStringLiteral("Relative widget index (tuple form)")}});
  registry.registerCommand(
    QStringLiteral("project.workspace.removeWidget"),
    QStringLiteral("Remove a widget ref. Pass either {widgetId} from a prior "
                   "addWidget/get response, or the legacy tuple "
                   "{workspaceId, widgetType, groupId, relativeIndex}.\n"
                   "Batching: for 5+ widget removals, wrap calls in "
                   "project.batch{ops:[...]} (see "
                   "meta.describeCommand{name:'project.batch'})."),
    removeSchema,
    &widgetRemove);
}

/**
 * @brief Register the project.workspace.validate and .cleanup commands.
 */
void API::Handlers::WorkspacesHandler::registerWidgetValidationCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.workspace.validate"),
    QStringLiteral("Walk workspace widget refs and report orphaned references (the "
                   "group/dataset/widgetType combo no longer exists). Read-only -- "
                   "does not modify project state or toggle customizeWorkspaces. "
                   "Pair with project.workspace.cleanup to repair. Pass an optional "
                   "workspaceId to limit scope; omit to validate all workspaces."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("workspaceId"),
        QStringLiteral("integer"),
        QStringLiteral("Limit validation to one workspace (optional)")}}),
    &validate);
  registry.registerCommand(
    QStringLiteral("project.workspace.cleanup"),
    QStringLiteral("Drop workspace widget refs that no longer resolve to a live "
                   "group/dataset/widgetType. Scope: orphans only -- does NOT touch "
                   "refs where the group still exists but compatibleWidgetTypes has "
                   "changed (that's reversible state). Set dryRun:true to enumerate "
                   "what would be removed without mutating."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("workspaceId"),
        QStringLiteral("integer"),
        QStringLiteral("Limit cleanup to one workspace (optional)")},
       {QStringLiteral("dryRun"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, report what would be removed without mutating "
                       "(default false)")}}),
    &cleanup);
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
    entry[QStringLiteral("icon")]        = SerialStudio::normalizeIconPath(ws.icon);
    entry[QStringLiteral("description")] = ws.description;
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
  result[QStringLiteral("id")]          = it->workspaceId;
  result[QStringLiteral("title")]       = it->title;
  result[QStringLiteral("icon")]        = it->icon;
  result[QStringLiteral("description")] = it->description;
  result[QStringLiteral("widgets")]     = refsToJson(it->workspaceId, it->widgetRefs);
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
  if (!inProjectFileMode())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace mutations require ProjectFile mode"));

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
  if (!inProjectFileMode())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace mutations require ProjectFile mode"));

  if (!params.contains(QStringLiteral("id")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: id"));

  auto& pm = DataModel::ProjectModel::instance();
  if (!pm.customizeWorkspaces())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("customizeWorkspaces is off; call project.workspaces.customize.set first"));

  const int wid = params.value(QStringLiteral("id")).toInt();
  pm.deleteWorkspace(wid);

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
  if (!inProjectFileMode())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace mutations require ProjectFile mode"));

  if (!params.contains(QStringLiteral("id")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: id"));

  if (!params.contains(QStringLiteral("title")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: title"));

  auto& pm = DataModel::ProjectModel::instance();
  if (!pm.customizeWorkspaces())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("customizeWorkspaces is off; call project.workspaces.customize.set first"));

  const int wid          = params.value(QStringLiteral("id")).toInt();
  const QString newTitle = params.value(QStringLiteral("title")).toString();
  if (newTitle.trimmed().isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace title cannot be empty"));

  pm.renameWorkspace(wid, newTitle);

  QJsonObject result;
  result[QStringLiteral("id")]      = wid;
  result[QStringLiteral("title")]   = newTitle;
  result[QStringLiteral("renamed")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Patches workspace title and/or icon by id.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::update(const QString& id,
                                                              const QJsonObject& params)
{
  if (!inProjectFileMode())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace mutations require ProjectFile mode"));

  if (!params.contains(QStringLiteral("id")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: id"));

  auto& pm = DataModel::ProjectModel::instance();
  if (!pm.customizeWorkspaces())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("customizeWorkspaces is off; call project.workspace.setCustomizeMode first"));

  const int wid             = params.value(QStringLiteral("id")).toInt();
  const bool setTitle       = params.contains(QStringLiteral("title"));
  const bool setIcon        = params.contains(QStringLiteral("icon"));
  const bool setDescription = params.contains(QStringLiteral("description"));
  if (!setTitle && !setIcon && !setDescription)
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Provide at least one of: title, icon, "
                                                     "description"));

  QString title;
  QString icon;
  QString description;
  if (setTitle) {
    title = params.value(QStringLiteral("title")).toString();
    if (title.trimmed().isEmpty())
      return CommandResponse::makeError(
        id, ErrorCode::InvalidParam, QStringLiteral("Workspace title cannot be empty"));
  }
  if (setIcon)
    icon = params.value(QStringLiteral("icon")).toString();

  if (setDescription)
    description = params.value(QStringLiteral("description")).toString();

  pm.updateWorkspace(wid, title, icon, description, setTitle, setIcon, setDescription);

  QJsonObject result;
  result[QStringLiteral("id")] = wid;
  if (setTitle)
    result[QStringLiteral("title")] = title;

  if (setIcon)
    result[QStringLiteral("icon")] = icon;

  if (setDescription)
    result[QStringLiteral("description")] = description;

  result[QStringLiteral("updated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Materialises the synthetic auto-workspaces into customized state.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::autoGenerate(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  if (!inProjectFileMode())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace mutations require ProjectFile mode"));

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
 */
API::CommandResponse API::Handlers::WorkspacesHandler::customizeSet(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!inProjectFileMode())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace mutations require ProjectFile mode"));

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
 */
API::CommandResponse API::Handlers::WorkspacesHandler::widgetAdd(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!inProjectFileMode())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace mutations require ProjectFile mode"));

  const QStringList required{
    QStringLiteral("workspaceId"),
    QStringLiteral("widgetType"),
    QStringLiteral("groupId"),
  };

  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  auto& pm = DataModel::ProjectModel::instance();
  if (!pm.customizeWorkspaces())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("customizeWorkspaces is off; call project.workspaces.customize.set first"));

  const int wid = params.value(QStringLiteral("workspaceId")).toInt();
  const int gid = params.value(QStringLiteral("groupId")).toInt();

  // widgetType accepts either an integer (DashboardWidget enum) or a string slug
  const QJsonValue wtypeJson = params.value(QStringLiteral("widgetType"));
  const int wtype            = resolveWidgetType(wtypeJson);
  if (wtype < 0)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Unknown widgetType slug '%1'. Use one of: plot, fft, bar, gauge, "
                     "compass, led, datagrid, multiplot, accelerometer, gyroscope, gps, "
                     "plot3d, terminal, imageview, output-panel, notification-log, "
                     "waterfall, painter.")
        .arg(wtypeJson.toString()));

  const bool hasRelIndex = params.contains(QStringLiteral("relativeIndex"));
  int relIndex           = hasRelIndex ? params.value(QStringLiteral("relativeIndex")).toInt()
                                       : nextFreeRelativeIndex(pm.editorWorkspaces(), wid, wtype, gid);
  const bool relIndexAutoAssigned = !hasRelIndex && relIndex != 0;

  // Reject DashboardTerminal / DashboardNoWidget with actionable errors
  if (wtype == SerialStudio::DashboardTerminal)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("widgetType=0 is DashboardTerminal/Console, not a workspace tile. "
                     "Common cause: confusing the DatasetOption bitflag (used by "
                     "project.dataset.setOptions: 1=Plot, 2=FFT, 4=Bar, 8=Gauge, "
                     "16=Compass, 32=LED, 64=Waterfall) with the DashboardWidget enum "
                     "(used here: 9=Plot, 7=FFT, 10=Bar, 11=Gauge, 12=Compass, 8=LED, "
                     "17=Waterfall, 1=DataGrid, 2=MultiPlot, 5=GPS). The numbers do not "
                     "line up. Pick a value from the group's compatibleWidgetTypes "
                     "(see project.group.list)."));

  if (wtype == SerialStudio::DashboardNoWidget)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("widgetType=13 is DashboardNoWidget (a sentinel value, "
                     "not a tile). Pick a real visualization type from the "
                     "group's compatibleWidgetTypes "
                     "(see project.group.list)."));

  const auto& wsList = pm.editorWorkspaces();
  const auto exists  = std::any_of(
    wsList.begin(), wsList.end(), [wid](const auto& ws) { return ws.workspaceId == wid; });
  if (!exists)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace not found: %1").arg(wid));

  const auto& groups  = DataModel::ProjectModel::instance().groups();
  const auto group_it = std::find_if(
    groups.begin(), groups.end(), [gid](const DataModel::Group& g) { return g.groupId == gid; });
  if (group_it == groups.end())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Group not found: %1. Use group.id from "
                                                     "project.group.list, not the array index.")
                                        .arg(gid));

  if (const auto err = validateGroupCompatibility(*group_it, wtype, gid))
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, *err);

  pm.addWidgetToWorkspace(wid, wtype, gid, relIndex);

  QJsonObject result;
  result[QStringLiteral("workspaceId")]               = wid;
  result[QStringLiteral("widgetType")]                = wtype;
  result[QStringLiteral("widgetTypeSlug")]            = API::EnumLabels::dashboardWidgetSlug(wtype);
  result[QStringLiteral("groupId")]                   = gid;
  result[QStringLiteral("relativeIndex")]             = relIndex;
  result[QStringLiteral("relativeIndexAutoAssigned")] = relIndexAutoAssigned;
  result[QStringLiteral("widgetId")]                  = widgetIdFor(wid, wtype, gid, relIndex);
  result[QStringLiteral("added")]                     = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Detaches a widget ref matching either {widgetId} or the legacy tuple.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::widgetRemove(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!inProjectFileMode())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace mutations require ProjectFile mode"));

  auto& pm = DataModel::ProjectModel::instance();
  if (!pm.customizeWorkspaces())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("customizeWorkspaces is off; call project.workspace.setCustomizeMode first"));

  // Two input forms: {widgetId:"..."} or the legacy 4-tuple
  int wid      = -1;
  int wtype    = -1;
  int gid      = -1;
  int relIndex = -1;

  if (params.contains(QStringLiteral("widgetId"))) {
    const auto parsed = parseWidgetId(params.value(QStringLiteral("widgetId")).toString());
    if (!parsed.valid)
      return CommandResponse::makeError(id,
                                        ErrorCode::InvalidParam,
                                        QStringLiteral("Malformed widgetId. Expected "
                                                       "'ws<workspaceId>:<slug>:g<groupId>:"
                                                       "<relativeIndex>'."));

    wid      = parsed.workspaceId;
    wtype    = parsed.widgetType;
    gid      = parsed.groupId;
    relIndex = parsed.relativeIndex;
  } else {
    const QStringList required{
      QStringLiteral("workspaceId"),
      QStringLiteral("widgetType"),
      QStringLiteral("groupId"),
      QStringLiteral("relativeIndex"),
    };

    for (const auto& key : required)
      if (!params.contains(key))
        return CommandResponse::makeError(
          id,
          ErrorCode::MissingParam,
          QStringLiteral("Missing parameter: %1 (or provide widgetId)").arg(key));

    wid      = params.value(QStringLiteral("workspaceId")).toInt();
    gid      = params.value(QStringLiteral("groupId")).toInt();
    relIndex = params.value(QStringLiteral("relativeIndex")).toInt();

    // widgetType accepts integer or string slug
    const QJsonValue wtypeJson = params.value(QStringLiteral("widgetType"));
    if (wtypeJson.isString()) {
      wtype = API::EnumLabels::dashboardWidgetFromSlug(wtypeJson.toString());
      if (wtype < 0)
        return CommandResponse::makeError(
          id,
          ErrorCode::InvalidParam,
          QStringLiteral("Unknown widgetType slug '%1'.").arg(wtypeJson.toString()));
    } else {
      wtype = wtypeJson.toInt();
    }
  }

  pm.removeWidgetFromWorkspace(wid, wtype, gid, relIndex);

  QJsonObject result;
  result[QStringLiteral("workspaceId")]    = wid;
  result[QStringLiteral("widgetType")]     = wtype;
  result[QStringLiteral("widgetTypeSlug")] = API::EnumLabels::dashboardWidgetSlug(wtype);
  result[QStringLiteral("groupId")]        = gid;
  result[QStringLiteral("relativeIndex")]  = relIndex;
  result[QStringLiteral("widgetId")]       = widgetIdFor(wid, wtype, gid, relIndex);
  result[QStringLiteral("removed")]        = true;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Validation, cleanup, reorder
//--------------------------------------------------------------------------------------------------

/**
 * @brief Walks workspace widget refs and reports orphans + unreferenced groups.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::validate(const QString& id,
                                                                const QJsonObject& params)
{
  const auto& pm     = DataModel::ProjectModel::instance();
  const auto& wsList = pm.editorWorkspaces();
  const auto lookup  = DataModel::ProjectEditor::buildResolvedWidgetLookup(pm);

  // Restrict to one workspace when caller asks
  const bool filtered = params.contains(QStringLiteral("workspaceId"));
  const int onlyWid   = filtered ? params.value(QStringLiteral("workspaceId")).toInt() : -1;

  QJsonArray issues;
  int orphanedCount  = 0;
  int workspaceCount = 0;
  QSet<int> referencedGroups;
  bool ok = true;

  for (const auto& ws : wsList) {
    if (filtered && ws.workspaceId != onlyWid)
      continue;

    ++workspaceCount;
    int refIndex = 0;
    for (const auto& ref : ws.widgetRefs) {
      referencedGroups.insert(ref.groupId);
      const auto key = DataModel::ProjectEditor::workspaceWidgetKey(
        ref.widgetType, ref.groupId, ref.relativeIndex);
      if (!lookup.contains(key)) {
        ++orphanedCount;
        ok = false;
        QJsonObject issue;
        issue[QStringLiteral("level")] = QStringLiteral("error");
        issue[QStringLiteral("location")] =
          QStringLiteral("workspace[%1].widgetRef[%2]")
            .arg(QString::number(ws.workspaceId), QString::number(refIndex));
        issue[QStringLiteral("workspaceId")] = ws.workspaceId;
        issue[QStringLiteral("widgetType")]  = ref.widgetType;
        issue[QStringLiteral("widgetTypeSlug")] =
          API::EnumLabels::dashboardWidgetSlug(ref.widgetType);
        issue[QStringLiteral("groupId")]       = ref.groupId;
        issue[QStringLiteral("relativeIndex")] = ref.relativeIndex;
        issue[QStringLiteral("widgetId")] =
          widgetIdFor(ws.workspaceId, ref.widgetType, ref.groupId, ref.relativeIndex);
        issue[QStringLiteral("message")] =
          QStringLiteral("Widget ref does not resolve to any live group/dataset/widgetType");
        issues.append(issue);
      }
      ++refIndex;
    }
  }

  // Unreferenced groups -- warning, only meaningful for full-project scope
  QJsonArray unreferenced;
  if (!filtered) {
    for (const auto& g : pm.groups()) {
      if (!referencedGroups.contains(g.groupId)) {
        unreferenced.append(g.groupId);
        QJsonObject issue;
        issue[QStringLiteral("level")]    = QStringLiteral("warning");
        issue[QStringLiteral("location")] = QStringLiteral("group[%1]").arg(g.groupId);
        issue[QStringLiteral("groupId")]  = g.groupId;
        issue[QStringLiteral("message")] =
          QStringLiteral("Group '%1' is not pinned to any workspace").arg(g.title);
        issues.append(issue);
      }
    }
  }

  QJsonObject result;
  result[QStringLiteral("ok")]                 = ok;
  result[QStringLiteral("issues")]             = issues;
  result[QStringLiteral("workspaceCount")]     = workspaceCount;
  result[QStringLiteral("orphanedCount")]      = orphanedCount;
  result[QStringLiteral("unreferencedGroups")] = unreferenced;
  result[QStringLiteral("issueCount")]         = issues.size();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Reports or removes orphaned workspace widget refs.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::cleanup(const QString& id,
                                                               const QJsonObject& params)
{
  const bool dryRun = params.value(QStringLiteral("dryRun")).toBool(false);
  if (!dryRun && !inProjectFileMode())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace mutations require ProjectFile mode"));

  auto& pm           = DataModel::ProjectModel::instance();
  const auto& wsList = pm.editorWorkspaces();
  const auto lookup  = DataModel::ProjectEditor::buildResolvedWidgetLookup(pm);

  const bool filtered = params.contains(QStringLiteral("workspaceId"));
  const int onlyWid   = filtered ? params.value(QStringLiteral("workspaceId")).toInt() : -1;

  // Enumerate orphans up-front; non-dryRun callers still see the removed identities
  QJsonArray removedRefs;
  for (const auto& ws : wsList) {
    if (filtered && ws.workspaceId != onlyWid)
      continue;

    for (const auto& ref : ws.widgetRefs) {
      const auto key = DataModel::ProjectEditor::workspaceWidgetKey(
        ref.widgetType, ref.groupId, ref.relativeIndex);
      if (lookup.contains(key))
        continue;

      QJsonObject entry;
      entry[QStringLiteral("workspaceId")] = ws.workspaceId;
      entry[QStringLiteral("widgetType")]  = ref.widgetType;
      entry[QStringLiteral("widgetTypeSlug")] =
        API::EnumLabels::dashboardWidgetSlug(ref.widgetType);
      entry[QStringLiteral("groupId")]       = ref.groupId;
      entry[QStringLiteral("relativeIndex")] = ref.relativeIndex;
      entry[QStringLiteral("widgetId")] =
        widgetIdFor(ws.workspaceId, ref.widgetType, ref.groupId, ref.relativeIndex);
      removedRefs.append(entry);
    }
  }

  int removed = 0;
  if (!dryRun && !removedRefs.isEmpty()) {
    QSet<qint64> validKeys;
    validKeys.reserve(lookup.size());
    for (auto it = lookup.constBegin(); it != lookup.constEnd(); ++it)
      validKeys.insert(it.key());

    removed = pm.cleanupWorkspaceWidgetRefs(validKeys);
  }

  QJsonObject result;
  result[QStringLiteral("ok")]          = true;
  result[QStringLiteral("dryRun")]      = dryRun;
  result[QStringLiteral("removed")]     = dryRun ? removedRefs.size() : removed;
  result[QStringLiteral("removedRefs")] = removedRefs;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Reorders user-defined workspaces (id >= 5000) by the given id sequence.
 */
API::CommandResponse API::Handlers::WorkspacesHandler::reorder(const QString& id,
                                                               const QJsonObject& params)
{
  if (!inProjectFileMode())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Workspace mutations require ProjectFile mode"));

  if (!params.contains(QStringLiteral("workspaceIds")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: workspaceIds"));

  if (!params.value(QStringLiteral("workspaceIds")).isArray())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("workspaceIds must be an array of integers"));

  const auto arr = params.value(QStringLiteral("workspaceIds")).toArray();

  // Reject any id below the user threshold up-front -- system workspaces don't reorder
  QList<int> orderedIds;
  QStringList rejected;
  for (const auto& v : arr) {
    const int wid = v.toInt(-1);
    if (wid < ::WorkspaceIds::UserStart) {
      rejected.append(QString::number(wid));
      continue;
    }
    orderedIds.append(wid);
  }

  if (!rejected.isEmpty())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Only user-defined workspaces (id >= 5000) can be reordered. "
                     "Rejected ids: [%1]")
        .arg(rejected.join(QStringLiteral(", "))));

  // Set-equality check against current user workspaces (avoid partial reorder)
  auto& pm           = DataModel::ProjectModel::instance();
  const auto& wsList = pm.editorWorkspaces();
  QSet<int> currentUserIds;
  for (const auto& ws : wsList)
    if (ws.workspaceId >= ::WorkspaceIds::UserStart)
      currentUserIds.insert(ws.workspaceId);

  const auto orderedSet = QSet<int>(orderedIds.begin(), orderedIds.end());
  if (orderedSet != currentUserIds)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("workspaceIds must include every user workspace exactly once. "
                     "Current user workspaces: %1; got %2.")
        .arg(currentUserIds.size())
        .arg(orderedIds.size()));

  pm.reorderWorkspaces(orderedIds);

  // Echo back the new order
  QJsonArray newOrder;
  for (const auto& ws : pm.editorWorkspaces())
    if (ws.workspaceId >= ::WorkspaceIds::UserStart)
      newOrder.append(ws.workspaceId);

  QJsonObject result;
  result[QStringLiteral("ok")]       = true;
  result[QStringLiteral("newOrder")] = newOrder;
  return CommandResponse::makeSuccess(id, result);
}
