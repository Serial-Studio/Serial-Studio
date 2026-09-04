/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Tools/ToolSupport.h"

#include <QUuid>

#include "AI/Tools/ToolBulkTools.h"
#include "AI/Tools/ToolCompact.h"
#include "AI/Tools/ToolResolve.h"
#include "AI/Tools/ToolSchemas.h"
#include "AI/Tools/ToolScriptTools.h"
#include "AI/Tools/ToolTileTools.h"
#include "API/CommandRegistry.h"

namespace AI::ToolDetail {

//--------------------------------------------------------------------------------------------------
// Command execution envelope
//--------------------------------------------------------------------------------------------------

/**
 * @brief Executes an API command and wraps its response into an {ok,result|error} envelope.
 */
QJsonObject runCommand(const QString& name, const QJsonObject& args)
{
  const auto callId        = QUuid::createUuid().toString(QUuid::WithoutBraces);
  static auto& apiRegistry = API::CommandRegistry::instance();
  const auto response      = apiRegistry.execute(name, callId, args);

  QJsonObject reply;
  reply[QStringLiteral("ok")] = response.success;
  if (response.success)
    reply[QStringLiteral("result")] = response.result;
  else {
    QJsonObject error;
    error[QStringLiteral("code")]    = response.errorCode;
    error[QStringLiteral("message")] = response.errorMessage;
    if (!response.errorData.isEmpty())
      error[QStringLiteral("data")] = response.errorData;

    reply[QStringLiteral("error")] = error;
  }
  return reply;
}

//--------------------------------------------------------------------------------------------------
// Repair hints
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds command-specific repair hints attached to failed tool replies.
 */
QJsonObject makeRepairHint(const QString& name, const QString& message)
{
  QJsonObject repair;
  if (name == QStringLiteral("project.workspace.addWidget")
      || name == QStringLiteral("assistant.workspace.addTile")) {
    repair[QStringLiteral("next")] =
      QStringLiteral("Call assistant.snapshot, then assistant.workspace.addTile with a valid "
                     "workspaceId, groupId, widgetType slug, and dataset identifier if the "
                     "group lacks that widget compatibility.");
    repair[QStringLiteral("identityReminder")] =
      QStringLiteral("workspaceId identifies a workspace; groupId identifies a group; "
                     "datasetId is only scoped within a group; uniqueId is opaque.");
  }

  if (message.contains(QStringLiteral("customizeWorkspaces"), Qt::CaseInsensitive))
    repair[QStringLiteral("customizeMode")] =
      QStringLiteral("Run project.workspace.setCustomizeMode{enabled:true} before workspace "
                     "mutations.");

  if (name.contains(QStringLiteral("dryRun")) || name.contains(QStringLiteral("script"))) {
    repair[QStringLiteral("scriptWorkflow")] =
      QStringLiteral("Fetch the matching scripting reference, fix the code, run "
                     "assistant.script.dryRun again, then apply only after dry-run succeeds.");
  }

  if (name == QStringLiteral("project.batch")
      || name == QStringLiteral("assistant.project.bulkApply")) {
    repair[QStringLiteral("batchShape")] =
      QStringLiteral("Use ops:[{command:'project.dataset.update', params:{...}}, ...]. "
                     "Do not put params at the op top level and do not nest project.batch.");
  }

  return repair;
}

/**
 * @brief Merges a repair hint into the error object of a failed reply.
 */
QJsonObject attachRepairHint(QJsonObject reply, const QString& commandName)
{
  if (reply.value(QStringLiteral("ok")).toBool())
    return reply;

  auto error = reply.value(QStringLiteral("error")).toObject();
  const auto repair =
    makeRepairHint(commandName, error.value(QStringLiteral("message")).toString());
  if (!repair.isEmpty()) {
    error[QStringLiteral("repair")] = repair;
    reply[QStringLiteral("error")]  = error;
  }

  return reply;
}

//--------------------------------------------------------------------------------------------------
// Assistant tool routing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the metadata block for an assistant.* tool, or empty if none matches.
 */
QJsonObject assistantToolDescription(const QString& name)
{
  for (const auto& def : assistantToolDefs()) {
    if (def.name != name)
      continue;

    QJsonObject desc;
    desc[QStringLiteral("name")]        = def.name;
    desc[QStringLiteral("description")] = def.description;
    desc[QStringLiteral("inputSchema")] = def.inputSchema;
    return desc;
  }

  return {};
}

/**
 * @brief Top-level dispatcher for every assistant.* virtual tool name.
 */
QJsonObject executeAssistantTool(const QString& name, const QJsonObject& args)
{
  if (name == QStringLiteral("assistant.snapshot")) {
    QJsonObject snapArgs;
    if (args.contains(QStringLiteral("verbose")))
      snapArgs[QStringLiteral("verbose")] = args.value(QStringLiteral("verbose")).toBool();

    const auto projectReply   = runCommand(QStringLiteral("project.snapshot"), snapArgs);
    const auto workspaceReply = runCommand(QStringLiteral("project.workspace.list"));
    if (!projectReply.value(QStringLiteral("ok")).toBool())
      return projectReply;

    QJsonObject out;
    out[QStringLiteral("ok")] = true;
    out[QStringLiteral("snapshot")] =
      compactProjectSnapshotResult(projectReply.value(QStringLiteral("result")).toObject(),
                                   workspaceReply.value(QStringLiteral("result")).toObject(),
                                   snapArgs.value(QStringLiteral("verbose")).toBool());
    out[QStringLiteral("identity")] =
      QStringLiteral("Dataset path/title/uniqueId are resolver inputs. Dataset mutations use "
                     "{groupId,datasetId}. Workspace tiles use {workspaceId,widgetType,groupId}. "
                     "Never derive uniqueId in chat.");
    return out;
  }

  if (name == QStringLiteral("assistant.dataset.resolve"))
    return resolveDataset(args);

  if (name == QStringLiteral("assistant.workspace.resolve"))
    return resolveWorkspace(args);

  if (name == QStringLiteral("assistant.workspace.plan")) {
    QJsonObject out;
    out[QStringLiteral("ok")] = true;
    out[QStringLiteral("snapshot")] =
      executeAssistantTool(QStringLiteral("assistant.snapshot"), {});
    out[QStringLiteral("hint")] =
      QStringLiteral("Choose widgetType slugs already present in each group's "
                     "compatibleWidgetTypes. For plot/fft/bar/gauge/compass/led/waterfall on a "
                     "specific dataset, use assistant.workspace.addTile so options are enabled "
                     "before the tile is pinned.");
    return out;
  }

  if (name == QStringLiteral("assistant.workspace.addTile"))
    return executeAddTile(args);

  if (name == QStringLiteral("assistant.script.dryRun"))
    return executeScriptDryRun(args);

  if (name == QStringLiteral("assistant.script.apply"))
    return executeScriptApply(args);

  if (name == QStringLiteral("assistant.project.bulkApply"))
    return executeBulkApply(args);

  if (name == QStringLiteral("assistant.checkpoint") || name == QStringLiteral("assistant.restore")
      || name == QStringLiteral("assistant.listCheckpoints")
      || name == QStringLiteral("assistant.memory.propose"))
    return runCommand(name, args);

  QJsonObject out;
  out[QStringLiteral("ok")]    = false;
  out[QStringLiteral("error")] = QStringLiteral("unknown_assistant_tool");
  return out;
}

}  // namespace AI::ToolDetail
