/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "API/Handlers/AssistantHandler.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QStringList>

#include "API/CommandRegistry.h"
#include "API/EnumLabels.h"
#include "API/SchemaBuilder.h"
#include "DataModel/Frame.h"
#include "Misc/BackupManager.h"

//--------------------------------------------------------------------------------------------------
// Shared dispatch helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Forward a call to an inner registered command, preserving the caller's id.
 */
static API::CommandResponse forward(const QString& innerCommand,
                                    const QString& id,
                                    const QJsonObject& params)
{
  return API::CommandRegistry::instance().execute(innerCommand, id, params);
}

/**
 * @brief Re-wrap an inner CommandResponse under a new outer id (chained calls).
 */
static API::CommandResponse rewrap(const QString& outerId, API::CommandResponse inner)
{
  inner.id = outerId;
  return inner;
}

/**
 * @brief Pack an inner CommandResponse into a step record for steps[] arrays.
 */
static QJsonObject toStep(const QString& label, const API::CommandResponse& r)
{
  QJsonObject step;
  step[QStringLiteral("step")]    = label;
  step[QStringLiteral("success")] = r.success;
  if (r.success) {
    if (!r.result.isEmpty())
      step[QStringLiteral("result")] = r.result;
  } else {
    QJsonObject err;
    err[QStringLiteral("code")]    = r.errorCode;
    err[QStringLiteral("message")] = r.errorMessage;
    if (!r.errorData.isEmpty())
      err[QStringLiteral("data")] = r.errorData;

    step[QStringLiteral("error")] = err;
  }
  return step;
}

//--------------------------------------------------------------------------------------------------
// assistant.snapshot -- pass-through to project.snapshot
//--------------------------------------------------------------------------------------------------

/**
 * @brief Forward assistant.snapshot to project.snapshot verbatim.
 */
API::CommandResponse API::Handlers::AssistantHandler::snapshot(const QString& id,
                                                               const QJsonObject& params)
{
  return forward(QStringLiteral("project.snapshot"), id, params);
}

//--------------------------------------------------------------------------------------------------
// assistant.dataset.resolve -- dispatch by which key is present
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolve a dataset by path, title, or uniqueId; exactly one key must be present.
 */
API::CommandResponse API::Handlers::AssistantHandler::datasetResolve(const QString& id,
                                                                     const QJsonObject& params)
{
  const bool hasPath     = params.contains(QStringLiteral("path"));
  const bool hasTitle    = params.contains(QStringLiteral("title"));
  const bool hasUniqueId = params.contains(Keys::UniqueId);

  const int presentCount = (hasPath ? 1 : 0) + (hasTitle ? 1 : 0) + (hasUniqueId ? 1 : 0);
  if (presentCount == 0)
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Provide exactly one of: path (e.g. 'Group/Dataset'), title, or uniqueId."));

  if (presentCount > 1)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Provide exactly one of {path, title, uniqueId} -- got %1.")
        .arg(presentCount));

  if (hasPath)
    return forward(QStringLiteral("project.dataset.getByPath"), id, params);

  if (hasTitle)
    return forward(QStringLiteral("project.dataset.getByTitle"), id, params);

  return forward(QStringLiteral("project.dataset.getByUniqueId"), id, params);
}

//--------------------------------------------------------------------------------------------------
// assistant.workspace.resolve -- match by id or case-insensitive title
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the workspaces array from a workspace.list response.
 */
static QJsonArray workspacesFromListResponse(const API::CommandResponse& listResp)
{
  return listResp.result.value(QStringLiteral("workspaces")).toArray();
}

/**
 * @brief Resolve a workspace by id (workspaceId) or case-insensitive title (workspace).
 */
API::CommandResponse API::Handlers::AssistantHandler::workspaceResolve(const QString& id,
                                                                       const QJsonObject& params)
{
  const bool hasId    = params.contains(QStringLiteral("workspaceId"));
  const bool hasTitle = params.contains(QStringLiteral("workspace"));
  if (!hasId && !hasTitle)
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Provide one of: workspaceId (integer) or workspace (title)."));

  const auto listResp =
    forward(QStringLiteral("project.workspace.list"), QStringLiteral("inner"), QJsonObject());
  if (!listResp.success)
    return rewrap(id, listResp);

  const auto workspaces = workspacesFromListResponse(listResp);

  QJsonArray matches;
  if (hasId) {
    const int wid = params.value(QStringLiteral("workspaceId")).toInt();
    for (const auto& v : workspaces) {
      const auto ws = v.toObject();
      if (ws.value(QStringLiteral("id")).toInt() == wid)
        matches.append(ws);
    }
  } else {
    const auto needle = params.value(QStringLiteral("workspace")).toString();
    for (const auto& v : workspaces) {
      const auto ws = v.toObject();
      if (ws.value(QStringLiteral("title")).toString().compare(needle, Qt::CaseInsensitive) == 0)
        matches.append(ws);
    }
  }

  if (matches.isEmpty()) {
    QJsonObject data;
    data[QStringLiteral("candidates")] = workspaces;
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("No workspace matched. Pick one from data.candidates by id or title."),
      data);
  }

  if (matches.size() > 1) {
    QJsonObject data;
    data[QStringLiteral("matches")] = matches;
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Ambiguous match (%1 workspaces share that title). "
                     "Re-call with workspaceId.")
        .arg(matches.size()),
      data);
  }

  QJsonObject result;
  result[QStringLiteral("workspace")] = matches.first().toObject();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// assistant.workspace.plan -- enumerate (group, widgetType) tuples valid for a workspace
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the groups array from a project.group.list response.
 */
static QJsonArray groupsFromListResponse(const API::CommandResponse& listResp)
{
  return listResp.result.value(QStringLiteral("groups")).toArray();
}

/**
 * @brief Build planning rows for compatible (groupId, widgetType slug) pairs.
 */
static QJsonArray buildPlanRows(const QJsonArray& groups, const QString& widgetFilterSlug)
{
  QJsonArray rows;
  for (const auto& gv : groups) {
    const auto group   = gv.toObject();
    const int groupId  = group.value(QStringLiteral("groupId")).toInt();
    const auto title   = group.value(QStringLiteral("title")).toString();
    const auto slugs   = group.value(QStringLiteral("compatibleWidgetTypeSlugs")).toArray();
    const auto dataset = group.value(QStringLiteral("datasetSummary")).toArray();

    for (const auto& sv : slugs) {
      const auto slug = sv.toString();
      if (!widgetFilterSlug.isEmpty() && slug != widgetFilterSlug)
        continue;

      QJsonObject row;
      row[QStringLiteral("groupId")]      = groupId;
      row[QStringLiteral("groupTitle")]   = title;
      row[QStringLiteral("widgetType")]   = slug;
      row[QStringLiteral("datasetCount")] = dataset.size();
      rows.append(row);
    }
  }
  return rows;
}

/**
 * @brief Plan a workspace: list compatible (group, widgetType) options for tile placement.
 */
API::CommandResponse API::Handlers::AssistantHandler::workspacePlan(const QString& id,
                                                                    const QJsonObject& params)
{
  const auto wsResp = workspaceResolve(QStringLiteral("inner"), params);
  if (!wsResp.success)
    return rewrap(id, wsResp);

  const auto groupListResp =
    forward(QStringLiteral("project.group.list"), QStringLiteral("inner"), QJsonObject());
  if (!groupListResp.success)
    return rewrap(id, groupListResp);

  QString widgetFilterSlug;
  if (params.contains(QStringLiteral("widgetType"))) {
    const auto v = params.value(QStringLiteral("widgetType"));
    if (v.isString())
      widgetFilterSlug = v.toString();
    else
      widgetFilterSlug = API::EnumLabels::dashboardWidgetSlug(v.toInt());
  }

  const auto groups = groupsFromListResponse(groupListResp);
  const auto rows   = buildPlanRows(groups, widgetFilterSlug);

  QJsonObject result;
  result[QStringLiteral("workspace")] = wsResp.result.value(QStringLiteral("workspace"));
  result[QStringLiteral("options")]   = rows;
  result[QStringLiteral("count")]     = rows.size();
  if (!widgetFilterSlug.isEmpty())
    result[QStringLiteral("widgetType")] = widgetFilterSlug;

  if (rows.isEmpty() && !widgetFilterSlug.isEmpty())
    result[QStringLiteral("hint")] =
      QStringLiteral(
        "No group currently exposes widgetType '%1'. Flip the matching dataset option "
        "(project.dataset.setOptions / .setOption / .update {graph|fft|led|waterfall}) "
        "on a dataset in the target group, then re-plan.")
        .arg(widgetFilterSlug);

  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// assistant.script.dryRun / assistant.script.apply -- kind-routed
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the project.* dryRun command for a given script kind, or empty on unknown.
 */
static QString dryRunCommandForKind(const QString& kind)
{
  if (kind == QStringLiteral("frame_parser"))
    return QStringLiteral("project.frameParser.dryRun");

  if (kind == QStringLiteral("transform"))
    return QStringLiteral("project.dataset.transform.dryRun");

  if (kind == QStringLiteral("painter"))
    return QStringLiteral("project.painter.dryRun");

  return QString();
}

/**
 * @brief Returns the project.* setCode command for a given script kind, or empty on unknown.
 */
static QString setCodeCommandForKind(const QString& kind)
{
  if (kind == QStringLiteral("frame_parser"))
    return QStringLiteral("project.frameParser.setCode");

  if (kind == QStringLiteral("transform"))
    return QStringLiteral("project.dataset.setTransformCode");

  if (kind == QStringLiteral("painter"))
    return QStringLiteral("project.painter.setCode");

  return QString();
}

/**
 * @brief Error response for an unsupported assistant.script.* kind.
 */
static API::CommandResponse unknownKindError(const QString& id, const QString& kind)
{
  return API::CommandResponse::makeError(
    id,
    API::ErrorCode::InvalidParam,
    QStringLiteral("Unknown kind: '%1'. Use one of: frame_parser, transform, painter.").arg(kind));
}

/**
 * @brief Dry-run a script by kind (frame_parser / transform / painter).
 */
API::CommandResponse API::Handlers::AssistantHandler::scriptDryRun(const QString& id,
                                                                   const QJsonObject& params)
{
  const auto kind  = params.value(QStringLiteral("kind")).toString();
  const auto inner = dryRunCommandForKind(kind);
  if (inner.isEmpty())
    return unknownKindError(id, kind);

  QJsonObject innerParams = params;
  innerParams.remove(QStringLiteral("kind"));
  return forward(inner, id, innerParams);
}

/**
 * @brief Dry-run a script then write it via the matching setCode endpoint.
 */
API::CommandResponse API::Handlers::AssistantHandler::scriptApply(const QString& id,
                                                                  const QJsonObject& params)
{
  const auto kind     = params.value(QStringLiteral("kind")).toString();
  const auto dryInner = dryRunCommandForKind(kind);
  const auto setInner = setCodeCommandForKind(kind);
  if (dryInner.isEmpty() || setInner.isEmpty())
    return unknownKindError(id, kind);

  QJsonObject inner = params;
  inner.remove(QStringLiteral("kind"));

  // Default an absent `values` to [0.0] so transform apply() doubles as a compile-only path.
  if (kind == QStringLiteral("transform") && !inner.contains(QStringLiteral("values"))) {
    QJsonArray defaults;
    defaults.append(0.0);
    inner.insert(QStringLiteral("values"), defaults);
  }

  // frame_parser without samples: fall back to dryCompile (syntax + parse() presence only).
  QString effectiveDry = dryInner;
  if (kind == QStringLiteral("frame_parser") && !inner.contains(QStringLiteral("sampleFrame"))
      && !inner.contains(QStringLiteral("sampleFrames")))
    effectiveDry = QStringLiteral("project.frameParser.dryCompile");

  QJsonArray steps;
  const auto dryResp = forward(effectiveDry, QStringLiteral("inner"), inner);
  steps.append(toStep(QStringLiteral("dryRun"), dryResp));

  // Painter dryRun and frame-parser dryCompile both signal compile failure via result.ok=false.
  const bool reportedOk = !dryResp.result.contains(QStringLiteral("ok"))
                       || dryResp.result.value(QStringLiteral("ok")).toBool(true);
  if (!dryResp.success || !reportedOk) {
    QJsonObject data;
    data[QStringLiteral("steps")] = steps;
    return CommandResponse::makeError(id,
                                      dryResp.success ? ErrorCode::ExecutionError
                                                      : dryResp.errorCode,
                                      QStringLiteral("Dry-run failed; setCode skipped."),
                                      data);
  }

  // setCode endpoints don't accept dry-run-only fields; strip them before forwarding.
  QJsonObject setParams = inner;
  setParams.remove(QStringLiteral("sampleFrame"));
  setParams.remove(QStringLiteral("sampleFrames"));
  setParams.remove(QStringLiteral("values"));

  const auto setResp = forward(setInner, QStringLiteral("inner"), setParams);
  steps.append(toStep(QStringLiteral("setCode"), setResp));
  if (!setResp.success) {
    QJsonObject data;
    data[QStringLiteral("steps")] = steps;
    return CommandResponse::makeError(
      id,
      setResp.errorCode,
      setResp.errorMessage.isEmpty() ? QStringLiteral("setCode failed") : setResp.errorMessage,
      data);
  }

  QJsonObject result;
  result[QStringLiteral("applied")] = true;
  result[QStringLiteral("kind")]    = kind;
  result[QStringLiteral("steps")]   = steps;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// assistant.workspace.addTile -- multi-step chained workspace edit
//--------------------------------------------------------------------------------------------------

/**
 * @brief DatasetOption bit that unlocks a given DashboardWidget slug; 0 = no flip required.
 */
static int datasetOptionBitForSlug(const QString& slug)
{
  if (slug == QStringLiteral("plot"))
    return 1;

  if (slug == QStringLiteral("fft"))
    return 2;

  if (slug == QStringLiteral("bar"))
    return 4;

  if (slug == QStringLiteral("gauge"))
    return 8;

  if (slug == QStringLiteral("compass"))
    return 16;

  if (slug == QStringLiteral("led"))
    return 32;

  if (slug == QStringLiteral("waterfall"))
    return 64;

  if (slug == QStringLiteral("meter"))
    return 128;

  return 0;
}

/**
 * @brief Subset of fields project.dataset.update accepts as range writes.
 */
static QJsonObject extractRangeParams(const QJsonObject& ranges)
{
  static const QStringList kKeys = {QStringLiteral("pltMin"),
                                    QStringLiteral("pltMax"),
                                    QStringLiteral("wgtMin"),
                                    QStringLiteral("wgtMax"),
                                    QStringLiteral("fftMin"),
                                    QStringLiteral("fftMax"),
                                    QStringLiteral("ledHigh")};
  QJsonObject out;
  for (const auto& k : kKeys)
    if (ranges.contains(k))
      out.insert(k, ranges.value(k));

  return out;
}

/**
 * @brief Resolve the target dataset for addTile; sets gid/did/uid in result on success.
 */
static API::CommandResponse resolveAddTileDataset(const QJsonObject& params,
                                                  int& outGroupId,
                                                  int& outDatasetId,
                                                  int& outUniqueId)
{
  if (params.contains(QStringLiteral("dataset"))) {
    QJsonObject p;
    p.insert(QStringLiteral("path"), params.value(QStringLiteral("dataset")));
    const auto resp =
      forward(QStringLiteral("project.dataset.getByPath"), QStringLiteral("inner"), p);
    if (!resp.success)
      return resp;

    outGroupId   = resp.result.value(QStringLiteral("groupId")).toInt();
    outDatasetId = resp.result.value(Keys::DatasetId).toInt();
    outUniqueId  = resp.result.value(Keys::UniqueId).toInt();
    return resp;
  }

  if (params.contains(Keys::UniqueId)) {
    QJsonObject p;
    p.insert(Keys::UniqueId, params.value(Keys::UniqueId));
    const auto resp =
      forward(QStringLiteral("project.dataset.getByUniqueId"), QStringLiteral("inner"), p);
    if (!resp.success)
      return resp;

    outGroupId   = resp.result.value(QStringLiteral("groupId")).toInt();
    outDatasetId = resp.result.value(Keys::DatasetId).toInt();
    outUniqueId  = resp.result.value(Keys::UniqueId).toInt();
    return resp;
  }

  return API::CommandResponse::makeSuccess(QStringLiteral("inner"));
}

/**
 * @brief Ensure customizeWorkspaces is enabled; returns the step entry.
 */
static API::CommandResponse ensureCustomizeMode()
{
  QJsonObject p;
  p.insert(QStringLiteral("enabled"), true);
  return forward(QStringLiteral("project.workspace.setCustomizeMode"), QStringLiteral("inner"), p);
}

/**
 * @brief Build the params for the inner workspace.addWidget call.
 */
static QJsonObject buildAddWidgetParams(
  int workspaceId, const QString& slug, int groupId, int datasetId, bool hasDataset)
{
  QJsonObject p;
  p.insert(QStringLiteral("workspaceId"), workspaceId);
  p.insert(QStringLiteral("widgetType"), slug);
  p.insert(QStringLiteral("groupId"), groupId);
  if (hasDataset)
    p.insert(Keys::DatasetId, datasetId);

  return p;
}

/**
 * @brief Locate a workspace in the list by id; returns an empty object when absent.
 */
static QJsonObject findWorkspaceById(const QJsonArray& workspaces, int wid)
{
  for (const auto& v : workspaces) {
    const auto ws = v.toObject();
    if (ws.value(QStringLiteral("id")).toInt() == wid)
      return ws;
  }
  return {};
}

/**
 * @brief Locate a workspace in the list by case-insensitive title; returns an empty object when
 * absent.
 */
static QJsonObject findWorkspaceByTitle(const QJsonArray& workspaces, const QString& needle)
{
  for (const auto& v : workspaces) {
    const auto ws = v.toObject();
    if (ws.value(QStringLiteral("title")).toString().compare(needle, Qt::CaseInsensitive) == 0)
      return ws;
  }
  return {};
}

/**
 * @brief Resolve or create the workspace; populates outWorkspaceId on success.
 */
static API::CommandResponse resolveOrCreateWorkspace(const QJsonObject& params,
                                                     int& outWorkspaceId,
                                                     QJsonArray& steps)
{
  const bool createIfMissing = params.value(QStringLiteral("createWorkspace")).toBool(false);
  const bool hasWid          = params.contains(QStringLiteral("workspaceId"));
  const bool hasTitle        = params.contains(QStringLiteral("workspace"));
  if (!hasWid && !hasTitle)
    return API::CommandResponse::makeError(
      QStringLiteral("inner"),
      API::ErrorCode::MissingParam,
      QStringLiteral("Provide workspaceId or workspace (title)."));

  const auto listResp =
    forward(QStringLiteral("project.workspace.list"), QStringLiteral("inner"), QJsonObject());
  if (!listResp.success) {
    steps.append(toStep(QStringLiteral("workspace.list"), listResp));
    return listResp;
  }

  const auto workspaces     = listResp.result.value(QStringLiteral("workspaces")).toArray();
  const QString titleNeedle = params.value(QStringLiteral("workspace")).toString();
  const QJsonObject match =
    hasWid ? findWorkspaceById(workspaces, params.value(QStringLiteral("workspaceId")).toInt())
           : findWorkspaceByTitle(workspaces, titleNeedle);
  const bool found = !match.isEmpty();

  if (found) {
    outWorkspaceId = match.value(QStringLiteral("id")).toInt();
    return API::CommandResponse::makeSuccess(QStringLiteral("inner"));
  }

  if (!createIfMissing) {
    QJsonObject data;
    data[QStringLiteral("candidates")] = workspaces;
    return API::CommandResponse::makeError(
      QStringLiteral("inner"),
      API::ErrorCode::InvalidParam,
      QStringLiteral("Workspace not found. Pass createWorkspace:true to create it, or pick "
                     "one from data.candidates."),
      data);
  }

  QJsonObject addParams;
  addParams.insert(QStringLiteral("title"), titleNeedle);
  const auto addResp =
    forward(QStringLiteral("project.workspace.add"), QStringLiteral("inner"), addParams);
  steps.append(toStep(QStringLiteral("workspace.add"), addResp));
  if (!addResp.success)
    return addResp;

  outWorkspaceId = addResp.result.value(QStringLiteral("id")).toInt();
  return addResp;
}

/**
 * @brief Re-emit an inner-step failure as an outer error response that carries steps[].
 */
static API::CommandResponse stepFailure(const QString& outerId,
                                        const API::CommandResponse& inner,
                                        const QJsonArray& steps)
{
  QJsonObject data;
  data[QStringLiteral("steps")] = steps;
  return API::CommandResponse::makeError(outerId, inner.errorCode, inner.errorMessage, data);
}

/**
 * @brief Resolve widgetType (slug string or DashboardWidget integer) into a slug.
 */
static QString resolveWidgetSlug(const QJsonValue& wtv)
{
  if (wtv.isString())
    return wtv.toString();

  return API::EnumLabels::dashboardWidgetSlug(wtv.toInt());
}

/**
 * @brief Step 4 of workspaceAddTile: enable the slug-matched DatasetOption bit.
 */
static API::CommandResponse applyAddTileDatasetOption(int groupId,
                                                      int datasetId,
                                                      const QString& slug,
                                                      QJsonArray& steps)
{
  const int optionBit = datasetOptionBitForSlug(slug);
  if (optionBit == 0)
    return API::CommandResponse::makeSuccess(QStringLiteral("inner"));

  QJsonObject p;
  p.insert(QStringLiteral("groupId"), groupId);
  p.insert(Keys::DatasetId, datasetId);
  QJsonArray opts;
  opts.append(slug);
  p.insert(QStringLiteral("options"), opts);
  const auto optResp =
    forward(QStringLiteral("project.dataset.setOptions"), QStringLiteral("inner"), p);
  steps.append(toStep(QStringLiteral("dataset.setOptions"), optResp));
  return optResp;
}

/**
 * @brief Step 5 of workspaceAddTile: patch dataset axis/range fields when supplied.
 */
static API::CommandResponse applyAddTileRanges(const QJsonObject& params,
                                               int groupId,
                                               int datasetId,
                                               QJsonArray& steps)
{
  if (!params.contains(QStringLiteral("ranges")))
    return API::CommandResponse::makeSuccess(QStringLiteral("inner"));

  const auto ranges = params.value(QStringLiteral("ranges")).toObject();
  QJsonObject p     = extractRangeParams(ranges);
  if (p.isEmpty())
    return API::CommandResponse::makeSuccess(QStringLiteral("inner"));

  p.insert(QStringLiteral("groupId"), groupId);
  p.insert(Keys::DatasetId, datasetId);
  const auto rngResp =
    forward(QStringLiteral("project.dataset.update"), QStringLiteral("inner"), p);
  steps.append(toStep(QStringLiteral("dataset.update.ranges"), rngResp));
  return rngResp;
}

/**
 * @brief Chain: resolve dataset + workspace, enable option, patch ranges, addWidget, validate.
 */
API::CommandResponse API::Handlers::AssistantHandler::workspaceAddTile(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("widgetType")))
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Missing required parameter: widgetType (slug or integer)."));

  const QString slug = resolveWidgetSlug(params.value(QStringLiteral("widgetType")));
  if (slug.isEmpty())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Unknown widgetType. Use a slug (plot, fft, gauge, ...) or DashboardWidget "
                     "integer."));

  QJsonArray steps;

  // Step 1: resolve dataset (optional)
  int gid = -1, did = -1, uid = -1;
  const bool hasDataset =
    params.contains(QStringLiteral("dataset")) || params.contains(Keys::UniqueId);
  if (hasDataset) {
    const auto dsResp = resolveAddTileDataset(params, gid, did, uid);
    steps.append(toStep(QStringLiteral("dataset.resolve"), dsResp));
    if (!dsResp.success)
      return stepFailure(id, dsResp, steps);
  }

  // Step 2: resolve or create the workspace
  int workspaceId   = -1;
  const auto wsResp = resolveOrCreateWorkspace(params, workspaceId, steps);
  if (!wsResp.success)
    return stepFailure(id, wsResp, steps);

  // Step 3: ensure customize mode
  const auto custResp = ensureCustomizeMode();
  steps.append(toStep(QStringLiteral("workspace.setCustomizeMode"), custResp));
  if (!custResp.success)
    return stepFailure(id, custResp, steps);

  if (hasDataset) {
    const auto optResp = applyAddTileDatasetOption(gid, did, slug, steps);
    if (!optResp.success)
      return stepFailure(id, optResp, steps);

    const auto rngResp = applyAddTileRanges(params, gid, did, steps);
    if (!rngResp.success)
      return stepFailure(id, rngResp, steps);
  }

  // Step 6: addWidget. If no dataset was provided, gid must come from params.
  if (!hasDataset) {
    if (!params.contains(QStringLiteral("groupId")))
      return CommandResponse::makeError(
        id,
        ErrorCode::MissingParam,
        QStringLiteral("Provide a dataset (path or uniqueId) OR a groupId for group-level tiles."));

    gid = params.value(QStringLiteral("groupId")).toInt();
  }

  const auto addParams = buildAddWidgetParams(workspaceId, slug, gid, did, hasDataset);
  const auto addResp =
    forward(QStringLiteral("project.workspace.addWidget"), QStringLiteral("inner"), addParams);
  steps.append(toStep(QStringLiteral("workspace.addWidget"), addResp));
  if (!addResp.success)
    return stepFailure(id, addResp, steps);

  QJsonObject result;
  result[QStringLiteral("workspaceId")] = workspaceId;
  result[QStringLiteral("widgetType")]  = slug;
  result[QStringLiteral("groupId")]     = gid;
  if (hasDataset) {
    result[Keys::DatasetId] = did;
    result[Keys::UniqueId]  = uid;
  }

  if (!addResp.result.isEmpty()) {
    if (addResp.result.contains(QStringLiteral("widgetId")))
      result[QStringLiteral("widgetId")] = addResp.result.value(QStringLiteral("widgetId"));

    if (addResp.result.contains(QStringLiteral("relativeIndex")))
      result[QStringLiteral("relativeIndex")] =
        addResp.result.value(QStringLiteral("relativeIndex"));
  }

  result[QStringLiteral("steps")] = steps;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// assistant.project.bulkApply -- nested-batch-rejecting wrapper around project.batch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reject nested batches; forward to project.batch on success.
 */
API::CommandResponse API::Handlers::AssistantHandler::projectBulkApply(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("ops")) || !params.value(QStringLiteral("ops")).isArray())
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Missing required parameter: ops (array of {command, params})."));

  const auto ops = params.value(QStringLiteral("ops")).toArray();
  for (int i = 0; i < ops.size(); ++i) {
    const auto cmd = ops.at(i).toObject().value(QStringLiteral("command")).toString();
    if (cmd == QStringLiteral("project.batch")
        || cmd == QStringLiteral("assistant.project.bulkApply"))
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Nested batch rejected: ops[%1].command = '%2'.")
          .arg(QString::number(i), cmd));
  }

  return forward(QStringLiteral("project.batch"), id, params);
}

//--------------------------------------------------------------------------------------------------
// assistant.checkpoint / restore / listCheckpoints
//--------------------------------------------------------------------------------------------------

/**
 * @brief Force an immediate project snapshot with an optional caller-supplied label.
 */
API::CommandResponse API::Handlers::AssistantHandler::checkpoint(const QString& id,
                                                                 const QJsonObject& params)
{
  const auto label = params.value(QStringLiteral("label")).toString();
  const auto path  = Misc::BackupManager::instance().snapshot(label);
  if (path.isEmpty())
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Failed to write checkpoint (BackupManager disabled, no project loaded, "
                     "or the backup directory is unwritable)."));

  QJsonObject result;
  result[QStringLiteral("path")]  = path;
  result[QStringLiteral("label")] = label;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Resolve a checkpoint reference (path / timestamp / label) to an absolute path.
 */
static QString resolveCheckpoint(const QJsonObject& params)
{
  if (params.contains(QStringLiteral("path")))
    return params.value(QStringLiteral("path")).toString();

  const auto rows = Misc::BackupManager::instance().list();
  if (params.contains(QStringLiteral("timestamp"))) {
    const auto needle = params.value(QStringLiteral("timestamp")).toString();
    for (const auto& v : rows) {
      const auto row = v.toMap();
      if (row.value(QStringLiteral("timestamp")).toString() == needle)
        return row.value(QStringLiteral("path")).toString();
    }
  }
  if (params.contains(QStringLiteral("label"))) {
    const auto needle = params.value(QStringLiteral("label")).toString();
    for (const auto& v : rows) {
      const auto row = v.toMap();
      if (row.value(QStringLiteral("label")).toString() == needle)
        return row.value(QStringLiteral("path")).toString();
    }
  }
  return QString();
}

/**
 * @brief Restore a previously taken snapshot, replacing the current ProjectModel state.
 */
API::CommandResponse API::Handlers::AssistantHandler::restore(const QString& id,
                                                              const QJsonObject& params)
{
  const auto path = resolveCheckpoint(params);
  if (path.isEmpty())
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Provide one of: path, timestamp, or label. Use "
                     "assistant.listCheckpoints to discover available references."));

  // Snapshot the current state first so this restore is itself reversible.
  const auto reverse = Misc::BackupManager::instance().snapshot(QStringLiteral("pre-restore"));

  if (!Misc::BackupManager::instance().restore(path))
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Failed to restore checkpoint at %1 (file missing, unreadable, or "
                     "contained invalid project JSON).")
        .arg(path));

  QJsonObject result;
  result[QStringLiteral("restored")] = true;
  result[QStringLiteral("path")]     = path;
  if (!reverse.isEmpty())
    result[QStringLiteral("reverseSnapshotPath")] = reverse;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Return the most recent N checkpoints for the current project.
 */
API::CommandResponse API::Handlers::AssistantHandler::listCheckpoints(const QString& id,
                                                                      const QJsonObject& params)
{
  const int limit = params.value(QStringLiteral("limit")).toInt(20);
  const auto rows = Misc::BackupManager::instance().list(limit);

  QJsonArray arr;
  for (const auto& v : rows) {
    const auto row = v.toMap();
    QJsonObject entry;
    entry[QStringLiteral("path")]      = row.value(QStringLiteral("path")).toString();
    entry[QStringLiteral("timestamp")] = row.value(QStringLiteral("timestamp")).toString();
    entry[QStringLiteral("sizeBytes")] =
      static_cast<qint64>(row.value(QStringLiteral("sizeBytes")).toLongLong());
    entry[QStringLiteral("label")] = row.value(QStringLiteral("label")).toString();
    arr.append(entry);
  }

  QJsonObject result;
  result[QStringLiteral("checkpoints")] = arr;
  result[QStringLiteral("count")]       = arr.size();
  result[QStringLiteral("directory")]   = Misc::BackupManager::instance().backupDirectory();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register the snapshot + resolver assistant commands.
 */
void API::Handlers::AssistantHandler::registerResolverCommands()
{
  auto& registry = CommandRegistry::instance();
  registry.registerCommand(
    QStringLiteral("assistant.snapshot"),
    QStringLiteral("Token-efficient project snapshot for LLMs. Identical payload to "
                   "project.snapshot -- prefer this name in skill docs so the assistant "
                   "rails stay grouped. Optional verbose=true includes parser code and "
                   "per-source frame settings."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("verbose"),
        QStringLiteral("boolean"),
        QStringLiteral("When true, include parser code and per-source frame settings.")}}),
    &API::Handlers::AssistantHandler::snapshot);

  registry.registerCommand(
    QStringLiteral("assistant.dataset.resolve"),
    QStringLiteral("Resolve a dataset by path, title, or uniqueId in one call -- "
                   "dispatches to project.dataset.getByPath / .getByTitle / .getByUniqueId "
                   "depending on which key is set. Exactly one of {path, title, uniqueId} "
                   "must be provided."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("path"),
        QStringLiteral("string"),
        QStringLiteral("'Group/Dataset' or 'Source/Group/Dataset' (preferred -- stable "
                       "across reorders).")},
       {QStringLiteral("title"),
        QStringLiteral("string"),
        QStringLiteral("Dataset title (may be ambiguous if duplicate titles exist).")},
       {Keys::UniqueId,
        QStringLiteral("integer"),
        QStringLiteral("Opaque uniqueId from a prior snapshot/list call.")}}),
    &API::Handlers::AssistantHandler::datasetResolve);

  registry.registerCommand(
    QStringLiteral("assistant.workspace.resolve"),
    QStringLiteral("Resolve a workspace by id or case-insensitive title. Returns the "
                   "matching workspace summary or an error with candidates[] for ambiguous "
                   "/ missing lookups."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("workspaceId"),
        QStringLiteral("integer"),
        QStringLiteral("Workspace id (1000-4999 auto, >=5000 user-created).")},
       {QStringLiteral("workspace"),
        QStringLiteral("string"),
        QStringLiteral("Workspace title (case-insensitive match).")}}),
    &API::Handlers::AssistantHandler::workspaceResolve);

  registry.registerCommand(
    QStringLiteral("assistant.workspace.plan"),
    QStringLiteral("Plan a workspace edit: returns the (groupId, widgetType) tuples whose "
                   "compatibleWidgetTypeSlugs match the optional widgetType filter. Use "
                   "this before assistant.workspace.addTile to discover which groups can "
                   "host a given visualization. Falls back to listing every compatible "
                   "tuple when no widgetType filter is set."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("workspaceId"),
        QStringLiteral("integer"),
        QStringLiteral("Workspace id (use this OR workspace).")},
       {QStringLiteral("workspace"),
        QStringLiteral("string"),
        QStringLiteral("Workspace title (use this OR workspaceId).")},
       {QStringLiteral("widgetType"),
        QStringLiteral("string|integer"),
        QStringLiteral("Optional: only return tuples compatible with this slug "
                       "(plot, fft, gauge, ...) or DashboardWidget integer.")}}),
    &API::Handlers::AssistantHandler::workspacePlan);
}

/**
 * @brief Register the assistant script + workspace-edit commands.
 */
void API::Handlers::AssistantHandler::registerEditCommands()
{
  auto& registry = CommandRegistry::instance();
  registry.registerCommand(
    QStringLiteral("assistant.script.dryRun"),
    QStringLiteral("Validate a script by kind without writing. Routes to "
                   "project.frameParser.dryRun, project.dataset.transform.dryRun, or "
                   "project.painter.dryRun based on `kind`. All other params are forwarded "
                   "verbatim (code, language, sampleFrame / sampleFrames / values, "
                   "groupId, etc.)."),
    API::makeSchema(
      {
        {QStringLiteral("kind"),
         QStringLiteral("string"),
         QStringLiteral("One of: frame_parser, transform, painter.")                        },
        {QStringLiteral("code"), QStringLiteral("string"),  QStringLiteral("Script source.")}
  },
      {{QStringLiteral("language"),
        QStringLiteral("integer"),
        QStringLiteral("0 = JavaScript, 1 = Lua (frame_parser / transform).")}}),
    &API::Handlers::AssistantHandler::scriptDryRun);

  registry.registerCommand(
    QStringLiteral("assistant.script.apply"),
    QStringLiteral("Dry-run a script then write it via the matching setCode endpoint. "
                   "Skips the dry run for frame_parser when no sampleFrame(s) are provided, "
                   "and defaults transform values to [0.0] so callers can apply without "
                   "test inputs. Returns steps[] so the LLM can see exactly which stage "
                   "ran. If the dry run fails, setCode is NOT called."),
    API::makeSchema(
      {
        {QStringLiteral("kind"),
         QStringLiteral("string"),
         QStringLiteral("One of: frame_parser, transform, painter.")                        },
        {QStringLiteral("code"), QStringLiteral("string"),  QStringLiteral("Script source.")}
  },
      {{QStringLiteral("language"),
        QStringLiteral("integer"),
        QStringLiteral("0 = JavaScript, 1 = Lua (frame_parser / transform).")},
       {QStringLiteral("groupId"),
        QStringLiteral("integer"),
        QStringLiteral("Target group id (transform / painter).")},
       {Keys::DatasetId,
        QStringLiteral("integer"),
        QStringLiteral("Target dataset id (transform).")},
       {Keys::SourceId,
        QStringLiteral("integer"),
        QStringLiteral("Target source id (frame_parser, default 0).")}}),
    &API::Handlers::AssistantHandler::scriptApply);

  registry.registerCommand(
    QStringLiteral("assistant.workspace.addTile"),
    QStringLiteral("Add a tile to a workspace in one call. Chains: resolve dataset "
                   "(by path or uniqueId) -> resolve or create workspace -> "
                   "setCustomizeMode(true) -> setOptions to enable the matching "
                   "DatasetOption bit -> patch ranges (pltMin/pltMax/wgtMin/wgtMax/"
                   "fftMin/fftMax/ledHigh) -> addWidget. Returns steps[] so the LLM can "
                   "see which sub-call failed. For group-level widgets "
                   "(datagrid/multiplot/led/accelerometer/gyroscope/gps/plot3d/painter/"
                   "imageview/output-panel), omit dataset/uniqueId and pass groupId "
                   "directly."),
    API::makeSchema(
      {
        {QStringLiteral("widgetType"),
         QStringLiteral("string|integer"),
         QStringLiteral("Widget slug (plot, fft, gauge, ...) or DashboardWidget integer.")}
  },
      {{QStringLiteral("workspaceId"),
        QStringLiteral("integer"),
        QStringLiteral("Workspace id (use this OR workspace).")},
       {QStringLiteral("workspace"),
        QStringLiteral("string"),
        QStringLiteral("Workspace title (use this OR workspaceId).")},
       {QStringLiteral("createWorkspace"),
        QStringLiteral("boolean"),
        QStringLiteral("When true and the workspace title doesn't exist, create it.")},
       {QStringLiteral("dataset"),
        QStringLiteral("string"),
        QStringLiteral("Dataset path 'Group/Dataset' (per-dataset widgets).")},
       {Keys::UniqueId,
        QStringLiteral("integer"),
        QStringLiteral("Dataset uniqueId (per-dataset widgets, alternative to dataset).")},
       {QStringLiteral("groupId"),
        QStringLiteral("integer"),
        QStringLiteral("Group id (required for group-level widgets when no dataset is "
                       "given).")},
       {QStringLiteral("ranges"),
        QStringLiteral("object"),
        QStringLiteral("Optional {pltMin, pltMax, wgtMin, wgtMax, fftMin, fftMax, "
                       "ledHigh} -- only keys present are written.")}}),
    &API::Handlers::AssistantHandler::workspaceAddTile);
}

/**
 * @brief Register the assistant checkpoint / restore / batch commands.
 */
void API::Handlers::AssistantHandler::registerCheckpointCommands()
{
  auto& registry = CommandRegistry::instance();
  registry.registerCommand(
    QStringLiteral("assistant.checkpoint"),
    QStringLiteral("Force an immediate project snapshot to disk and return its absolute path. "
                   "Call BEFORE any multi-step risky edit so you can roll back atomically with "
                   "assistant.restore if any subsequent step fails. Optional `label` lets you "
                   "tag the snapshot for retrieval by name. Snapshots are kept rolling (last 50 "
                   "per project) under the OS app-data backup directory."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("label"),
        QStringLiteral("string"),
        QStringLiteral("Optional human-readable tag for the checkpoint (alphanumerics + "
                       "dash/underscore, up to 32 chars after sanitisation).")}}),
    &API::Handlers::AssistantHandler::checkpoint);

  registry.registerCommand(
    QStringLiteral("assistant.restore"),
    QStringLiteral("Restore a previously taken checkpoint, replacing the current project state. "
                   "Provide one of: `path` (absolute path returned by assistant.checkpoint or "
                   "from a delete response's `backupPath`), `timestamp` (ISO string from "
                   "assistant.listCheckpoints), or `label` (most recent checkpoint matching "
                   "this label). DESTRUCTIVE: any unsaved current state is overwritten -- "
                   "a defensive pre-restore snapshot is taken and returned as "
                   "`reverseSnapshotPath` so you can undo the restore itself."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("path"),
        QStringLiteral("string"),
        QStringLiteral("Absolute path to the snapshot file.")},
       {QStringLiteral("timestamp"),
        QStringLiteral("string"),
        QStringLiteral("Snapshot timestamp from assistant.listCheckpoints.")},
       {QStringLiteral("label"),
        QStringLiteral("string"),
        QStringLiteral("Label assigned at checkpoint time.")}}),
    &API::Handlers::AssistantHandler::restore);

  registry.registerCommand(
    QStringLiteral("assistant.listCheckpoints"),
    QStringLiteral("List the rolling backup snapshots for the currently loaded project, "
                   "newest first. Returns `{checkpoints: [{path, timestamp, sizeBytes, "
                   "label}], count, directory}`. The `directory` field is the absolute "
                   "backup folder path -- useful for surfacing in UI escape hatches."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("limit"),
        QStringLiteral("integer"),
        QStringLiteral("Cap the number of entries returned (default 20, max 50).")}}),
    &API::Handlers::AssistantHandler::listCheckpoints);

  registry.registerCommand(
    QStringLiteral("assistant.project.bulkApply"),
    QStringLiteral("Run several project mutations atomically with respect to autosave. "
                   "Identical contract to project.batch (max 1024 ops, stopOnError "
                   "default false, NOT a database transaction) but pre-validates that "
                   "no op tries to nest another batch. Pass dryRun:true at the top "
                   "level to preview every op without committing -- forwarded to "
                   "project.batch which rejects the dryRun when any op is not in the "
                   "dryRun-aware command set."),
    API::makeSchema({
      API::arrayProp(
        QStringLiteral("ops"),
        QStringLiteral("Array of {command, params} ops. Max 1024. Nested "
                       "project.batch / assistant.project.bulkApply rejected."),
        QJsonObject{
                    {QStringLiteral("type"), QStringLiteral("object")},
                    {QStringLiteral("properties"),
           QJsonObject{{QStringLiteral("command"),
                        QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
                                    {QStringLiteral("description"),
                                     QStringLiteral("Registered command name (e.g. "
                                                    "'project.dataset.update').")}}},
                       {QStringLiteral("params"),
                        QJsonObject{{QStringLiteral("type"), QStringLiteral("object")},
                                    {QStringLiteral("description"),
                                     QStringLiteral("Arguments object for the command.")}}}}},
                    {QStringLiteral("required"),
           QJsonArray{QStringLiteral("command"), QStringLiteral("params")}}}
        )
  }),
    &API::Handlers::AssistantHandler::projectBulkApply);
}

/**
 * @brief Wire every assistant.* command into CommandRegistry.
 */
void API::Handlers::AssistantHandler::registerCommands()
{
  registerResolverCommands();
  registerEditCommands();
  registerCheckpointCommands();
}
