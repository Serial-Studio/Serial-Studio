/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/ToolDispatcher.h"

#include <QFile>
#include <QHash>
#include <QStringList>
#include <QUuid>
#include <QVector>

#include "AI/CommandRegistry.h"
#include "AI/Logging.h"
#include "API/CommandProtocol.h"
#include "API/CommandRegistry.h"
#include "DataModel/Frame.h"
#include "Misc/JsonValidator.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a tool dispatcher with the given Qt parent.
 */
AI::ToolDispatcher::ToolDispatcher(QObject* parent) : QObject(parent) {}

//--------------------------------------------------------------------------------------------------
// Assistant-native virtual tools
//--------------------------------------------------------------------------------------------------

namespace detail {
/**
 * @brief Tool catalog entry advertised to assistant providers.
 */
struct AssistantToolDef {
  QString name;
  QString description;
  QJsonObject inputSchema;
};
}  // namespace detail

/**
 * @brief Builds a `{type, description}` JSON Schema property fragment.
 */
static QJsonObject makeProperty(const QString& type, const QString& description)
{
  QJsonObject prop;
  prop[QStringLiteral("type")]        = type;
  prop[QStringLiteral("description")] = description;
  return prop;
}

/**
 * @brief Builds an `array`-typed property with a full items sub-schema.
 */
static QJsonObject makeArrayProperty(const QString& description, const QJsonObject& items)
{
  QJsonObject prop;
  prop[QStringLiteral("type")]        = QStringLiteral("array");
  prop[QStringLiteral("description")] = description;
  prop[QStringLiteral("items")]       = items;
  return prop;
}

/**
 * @brief Wraps a property map in a JSON Schema `object` envelope with optional required keys.
 */
static QJsonObject makeObjectSchema(const QJsonObject& properties, const QJsonArray& required = {})
{
  QJsonObject schema;
  schema[QStringLiteral("type")]       = QStringLiteral("object");
  schema[QStringLiteral("properties")] = properties;
  if (!required.isEmpty())
    schema[QStringLiteral("required")] = required;

  return schema;
}

/**
 * @brief Converts a string list into a JSON array for use as a schema `enum`.
 */
static QJsonArray stringEnum(const QStringList& values)
{
  QJsonArray arr;
  for (const auto& value : values)
    arr.append(value);

  return arr;
}

/**
 * @brief Builds a JSON Schema string property restricted to the given enum values.
 */
static QJsonObject stringEnumProperty(const QString& description, const QStringList& values)
{
  auto prop                    = makeProperty(QStringLiteral("string"), description);
  prop[QStringLiteral("enum")] = stringEnum(values);
  return prop;
}

/**
 * @brief Input schema for assistant.snapshot.
 */
static QJsonObject snapshotInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("verbose")] =
    makeProperty(QStringLiteral("boolean"),
                 QStringLiteral("Forwarded to project.snapshot. Use false by default."));
  return makeObjectSchema(props);
}

/**
 * @brief Input schema for assistant.dataset.resolve.
 */
static QJsonObject datasetInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("path")] =
    makeProperty(QStringLiteral("string"),
                 QStringLiteral("Preferred human address: 'Group/Dataset' or "
                                "'Source/Group/Dataset'."));
  props[QStringLiteral("title")] =
    makeProperty(QStringLiteral("string"), QStringLiteral("Exact dataset title."));
  props[Keys::UniqueId] =
    makeProperty(QStringLiteral("integer"), QStringLiteral("Opaque dataset uniqueId."));
  props[QStringLiteral("groupId")] =
    makeProperty(QStringLiteral("integer"), QStringLiteral("Optional group filter."));
  props[Keys::SourceId] =
    makeProperty(QStringLiteral("integer"), QStringLiteral("Optional source filter."));
  return makeObjectSchema(props);
}

/**
 * @brief Input schema for assistant.workspace.resolve.
 */
static QJsonObject workspaceInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("workspaceId")] =
    makeProperty(QStringLiteral("integer"), QStringLiteral("Workspace id from workspace list."));
  props[QStringLiteral("title")] =
    makeProperty(QStringLiteral("string"), QStringLiteral("Exact or case-insensitive title."));
  return makeObjectSchema(props);
}

/**
 * @brief Input schema for assistant.workspace.addTile.
 */
static QJsonObject tileInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("workspaceId")] =
    makeProperty(QStringLiteral("integer"), QStringLiteral("Workspace id. Preferred if known."));
  props[QStringLiteral("workspace")] =
    makeProperty(QStringLiteral("string"), QStringLiteral("Workspace title when id is unknown."));
  props[QStringLiteral("createWorkspace")] = makeProperty(
    QStringLiteral("boolean"), QStringLiteral("Create the named workspace if it does not exist."));
  props[QStringLiteral("groupId")] =
    makeProperty(QStringLiteral("integer"), QStringLiteral("Group id that owns the tile."));
  props[QStringLiteral("group")] =
    makeProperty(QStringLiteral("string"), QStringLiteral("Group title when id is unknown."));
  props[QStringLiteral("dataset")] =
    makeProperty(QStringLiteral("string"),
                 QStringLiteral("Optional dataset path/title. Required when the widget must be "
                                "enabled on a specific dataset."));
  props[Keys::UniqueId] =
    makeProperty(QStringLiteral("integer"), QStringLiteral("Optional dataset uniqueId."));
  props[QStringLiteral("widgetType")] =
    stringEnumProperty(QStringLiteral("Dashboard widget slug to add."),
                       QStringList{QStringLiteral("plot"),
                                   QStringLiteral("fft"),
                                   QStringLiteral("bar"),
                                   QStringLiteral("gauge"),
                                   QStringLiteral("compass"),
                                   QStringLiteral("led"),
                                   QStringLiteral("waterfall"),
                                   QStringLiteral("datagrid"),
                                   QStringLiteral("multiplot"),
                                   QStringLiteral("accelerometer"),
                                   QStringLiteral("gyroscope"),
                                   QStringLiteral("gps"),
                                   QStringLiteral("plot3d"),
                                   QStringLiteral("imageview"),
                                   QStringLiteral("painter"),
                                   QStringLiteral("output-panel"),
                                   QStringLiteral("notification-log")});
  props[QStringLiteral("ranges")] =
    makeProperty(QStringLiteral("object"),
                 QStringLiteral("Optional dataset range patch: pltMin/pltMax/wgtMin/wgtMax/"
                                "fftMin/fftMax."));
  return makeObjectSchema(props, QJsonArray{QStringLiteral("widgetType")});
}

/**
 * @brief Input schema for assistant.workspace.plan.
 */
static QJsonObject planInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("workspaceId")] =
    makeProperty(QStringLiteral("integer"), QStringLiteral("Optional workspace id to plan for."));
  props[QStringLiteral("workspace")] =
    makeProperty(QStringLiteral("string"), QStringLiteral("Optional workspace title to plan for."));
  return makeObjectSchema(props);
}

/**
 * @brief Shared property bag for assistant.script.dryRun and assistant.script.apply.
 */
static QJsonObject scriptPropsBag()
{
  QJsonObject props;
  props[QStringLiteral("kind")] =
    stringEnumProperty(QStringLiteral("Script surface to validate or apply."),
                       QStringList{QStringLiteral("frame_parser"),
                                   QStringLiteral("transform"),
                                   QStringLiteral("painter"),
                                   QStringLiteral("end_to_end")});
  props[QStringLiteral("code")] =
    makeProperty(QStringLiteral("string"), QStringLiteral("Script source to validate/apply."));
  props[QStringLiteral("language")] =
    makeProperty(QStringLiteral("integer"),
                 QStringLiteral("0 = JavaScript, 1 = Lua, 2 = Built-In (frame_parser only; "
                                "pass the JSON template descriptor {\"template\": id, "
                                "\"params\": {...}} as `code`)."));
  props[Keys::SourceId] =
    makeProperty(QStringLiteral("integer"), QStringLiteral("Optional source id, default 0."));
  props[QStringLiteral("inputBytes")] =
    makeProperty(QStringLiteral("string"),
                 QStringLiteral("[frame_parser] Raw stream bytes as UTF-8 text. Lossy "
                                "for binary -- prefer inputBytesHex."));
  props[QStringLiteral("inputBytesHex")] =
    makeProperty(QStringLiteral("string"),
                 QStringLiteral("[frame_parser, recommended] Raw stream bytes as a hex "
                                "string. Binary-safe; required for COBS / Modbus / "
                                "any non-ASCII protocol."));
  props[Keys::DecoderMethod] =
    makeProperty(QStringLiteral("integer"),
                 QStringLiteral("[frame_parser] 0=PlainText, 1=Hexadecimal, 2=Base64, "
                                "3=Binary. Binary is the only decoder safe for non-text."));
  props[Keys::FrameDetection] =
    makeProperty(QStringLiteral("integer"),
                 QStringLiteral("[frame_parser] 0=EndDelimiterOnly, 1=StartAndEnd, "
                                "2=NoDelimiters, 3=StartDelimiterOnly."));
  props[Keys::FrameStart] =
    makeProperty(QStringLiteral("string"), QStringLiteral("[frame_parser] Start delimiter."));
  props[Keys::FrameEnd] =
    makeProperty(QStringLiteral("string"), QStringLiteral("[frame_parser] End delimiter."));
  props[Keys::HexadecimalDelimiters] =
    makeProperty(QStringLiteral("boolean"),
                 QStringLiteral("[frame_parser] Treat frameStart / frameEnd as hex bytes."));
  props[Keys::ChecksumAlgorithm] = makeProperty(
    QStringLiteral("string"), QStringLiteral("[frame_parser] Checksum name (or empty for none)."));
  props[QStringLiteral("operationMode")] = makeProperty(
    QStringLiteral("integer"), QStringLiteral("[frame_parser] 0=ProjectFile, 2=QuickPlot."));
  props[QStringLiteral("sampleFrame")] = makeProperty(
    QStringLiteral("string"), QStringLiteral("[end_to_end only] Single pre-extracted frame body."));
  props[QStringLiteral("sampleFrames")] =
    makeProperty(QStringLiteral("array"),
                 QStringLiteral("[end_to_end only] Multiple pre-extracted frame bodies."));
  props[QStringLiteral("values")] =
    makeProperty(QStringLiteral("array"), QStringLiteral("Sample values for transform tests."));
  props[QStringLiteral("groupId")] = makeProperty(
    QStringLiteral("integer"), QStringLiteral("Target group id for transform/painter."));
  props[Keys::DatasetId] =
    makeProperty(QStringLiteral("integer"), QStringLiteral("Target dataset id for transforms."));
  props[QStringLiteral("dataset")] =
    makeProperty(QStringLiteral("string"), QStringLiteral("Dataset path/title for transforms."));
  props[Keys::UniqueId] =
    makeProperty(QStringLiteral("integer"), QStringLiteral("Dataset uniqueId for transforms."));
  props[QStringLiteral("virtual")] =
    makeProperty(QStringLiteral("boolean"),
                 QStringLiteral("For transform apply: mark dataset virtual before setting code."));
  return props;
}

/**
 * @brief Input schema for assistant.project.bulkApply.
 */
static QJsonObject bulkInputSchema()
{
  QJsonObject opItem;
  opItem[QStringLiteral("type")] = QStringLiteral("object");
  QJsonObject opProps;
  opProps[QStringLiteral("command")] = makeProperty(
    QStringLiteral("string"),
    QStringLiteral("Registered command name, e.g. 'project.dataset.update'. Not 'project.batch' "
                   "or 'assistant.project.bulkApply' -- nested batches are rejected."));
  opProps[QStringLiteral("params")] = makeProperty(
    QStringLiteral("object"),
    QStringLiteral("Arguments object for the command, exactly what you would pass at the top "
                   "level if calling it directly."));
  opItem[QStringLiteral("properties")] = opProps;
  opItem[QStringLiteral("required")] =
    QJsonArray{QStringLiteral("command"), QStringLiteral("params")};

  QJsonObject props;
  props[QStringLiteral("ops")] =
    makeArrayProperty(QStringLiteral("project.batch ops: [{command:'project.dataset.update', "
                                     "params:{...}}, ...]. Nested batches rejected."),
                      opItem);
  props[QStringLiteral("stopOnError")] =
    makeProperty(QStringLiteral("boolean"), QStringLiteral("Forwarded to project.batch."));
  return makeObjectSchema(props, QJsonArray{QStringLiteral("ops")});
}

/**
 * @brief Input schema for assistant.checkpoint.
 */
static QJsonObject checkpointInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("label")] = makeProperty(
    QStringLiteral("string"), QStringLiteral("Optional human-readable tag for the checkpoint."));
  return makeObjectSchema(props);
}

/**
 * @brief Input schema for assistant.restore.
 */
static QJsonObject restoreInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("path")] =
    makeProperty(QStringLiteral("string"),
                 QStringLiteral("Absolute snapshot path from assistant.checkpoint or from a "
                                "destructive command's backupPath."));
  props[QStringLiteral("timestamp")] = makeProperty(
    QStringLiteral("string"), QStringLiteral("Snapshot timestamp from assistant.listCheckpoints."));
  props[QStringLiteral("label")] =
    makeProperty(QStringLiteral("string"), QStringLiteral("Label assigned at checkpoint time."));
  return makeObjectSchema(props);
}

/**
 * @brief Input schema for assistant.listCheckpoints.
 */
static QJsonObject listCheckpointsInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("limit")] =
    makeProperty(QStringLiteral("integer"),
                 QStringLiteral("Cap the number of entries returned (default 20, max 50)."));
  return makeObjectSchema(props);
}

/**
 * @brief Returns the snapshot/dataset/workspace tool schemas exposed under the assistant.* prefix.
 */
static QVector<detail::AssistantToolDef> assistantToolDefs()
{
  const auto scriptProps = scriptPropsBag();
  return {
    {QStringLiteral("assistant.snapshot"),
     QStringLiteral("Assistant-oriented project snapshot with resolved project structure, "
                    "workspaces, and a compact identity guide. Prefer this over chaining "
                    "project.snapshot + list calls."),
     snapshotInputSchema()},
    {QStringLiteral("assistant.dataset.resolve"),
     QStringLiteral("Resolve a dataset from human input. Accepts path, title, or uniqueId and "
                    "returns the canonical dataset object plus identity hints."),
     datasetInputSchema()},
    {QStringLiteral("assistant.workspace.resolve"),
     QStringLiteral("Resolve a workspace by title or id and return the canonical workspace row."),
     workspaceInputSchema()},
    {QStringLiteral("assistant.workspace.plan"),
     QStringLiteral("Read the current project and suggest workspace tile additions using "
                    "compatible widget slugs. This is read-only planning."),
     planInputSchema()},
    {QStringLiteral("assistant.workspace.addTile"),
     QStringLiteral("High-level workspace mutation: resolves workspace/group/dataset, enables "
                    "the matching dataset option when needed, patches optional ranges, turns "
                    "customize mode on, adds the widget, then verifies the workspace."),
     tileInputSchema()},
    {QStringLiteral("assistant.script.dryRun"),
     QStringLiteral("Validate script code on the right Serial Studio surface. Routes frame "
                    "parsers, transforms, painters, and end-to-end parser+transform checks to "
                    "the matching dry-run endpoint and returns actionable reference hints."),
     makeObjectSchema(scriptProps, QJsonArray{QStringLiteral("kind")})},
    {QStringLiteral("assistant.script.apply"),
     QStringLiteral("Dry-run script code first, then apply it to the correct project target. "
                    "Use for frame parsers, dataset transforms, and painter widgets."),
     makeObjectSchema(scriptProps, QJsonArray{QStringLiteral("kind"), QStringLiteral("code")})},
    {QStringLiteral("assistant.project.bulkApply"),
     QStringLiteral("Validate and execute a project.batch mutation, rejecting nested batches and "
                    "summarizing per-op failures so models do not loop individual edits."),
     bulkInputSchema()},
    {QStringLiteral("assistant.checkpoint"),
     QStringLiteral("Force an immediate project snapshot to disk and return its absolute path. "
                    "Call BEFORE any multi-step risky edit so you can roll back atomically with "
                    "assistant.restore if any subsequent step fails."),
     checkpointInputSchema()},
    {QStringLiteral("assistant.restore"),
     QStringLiteral("Restore a previously taken checkpoint, replacing the current project state. "
                    "Provide one of: path (absolute path), timestamp (ISO string from "
                    "assistant.listCheckpoints), or label. Returns reverseSnapshotPath so the "
                    "restore itself is reversible."),
     restoreInputSchema()},
    {QStringLiteral("assistant.listCheckpoints"),
     QStringLiteral("List the rolling backup snapshots for the currently loaded project, newest "
                    "first. Returns {checkpoints:[{path,timestamp,sizeBytes,label}],count,"
                    "directory}."),
     listCheckpointsInputSchema()},
  };
}

/**
 * @brief Returns true when `name` targets an assistant.* virtual tool.
 */
static bool isAssistantTool(const QString& name)
{
  return name.startsWith(QStringLiteral("assistant."));
}

/**
 * @brief Returns the metadata block for an assistant.* tool, or empty if none matches.
 */
static QJsonObject assistantToolDescription(const QString& name)
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
 * @brief Executes an API command and wraps its response into an {ok,result|error} envelope.
 */
static QJsonObject runCommand(const QString& name, const QJsonObject& args = {})
{
  const auto callId   = QUuid::createUuid().toString(QUuid::WithoutBraces);
  const auto response = API::CommandRegistry::instance().execute(name, callId, args);

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

/**
 * @brief Builds command-specific repair hints attached to failed tool replies.
 */
static QJsonObject makeRepairHint(const QString& name, const QString& message)
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
static QJsonObject attachRepairHint(QJsonObject reply, const QString& commandName)
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

/**
 * @brief Resolves a dataset from uniqueId, path, or title and returns the canonical row.
 */
static QJsonObject resolveDataset(const QJsonObject& args)
{
  QJsonArray attempts;

  auto tryResolve = [&attempts](const QString& command, const QJsonObject& params) {
    QJsonObject attempt;
    attempt[QStringLiteral("command")]   = command;
    attempt[QStringLiteral("arguments")] = params;
    const auto reply                     = runCommand(command, params);
    attempt[QStringLiteral("ok")]        = reply.value(QStringLiteral("ok")).toBool();
    attempts.append(attempt);
    return reply;
  };

  QJsonObject resolved;
  QString method;
  if (args.contains(Keys::UniqueId)) {
    QJsonObject params;
    params[Keys::UniqueId] = args.value(Keys::UniqueId).toInt();
    const auto reply       = tryResolve(QStringLiteral("project.dataset.getByUniqueId"), params);
    if (reply.value(QStringLiteral("ok")).toBool()) {
      resolved = reply.value(QStringLiteral("result")).toObject();
      method   = Keys::UniqueId;
    }
  }

  if (resolved.isEmpty() && !args.value(QStringLiteral("path")).toString().isEmpty()) {
    QJsonObject params;
    params[QStringLiteral("path")] = args.value(QStringLiteral("path")).toString();
    const auto reply = tryResolve(QStringLiteral("project.dataset.getByPath"), params);
    if (reply.value(QStringLiteral("ok")).toBool()) {
      resolved = reply.value(QStringLiteral("result")).toObject();
      method   = QStringLiteral("path");
    }
  }

  if (resolved.isEmpty() && !args.value(QStringLiteral("title")).toString().isEmpty()) {
    QJsonObject params;
    params[QStringLiteral("title")] = args.value(QStringLiteral("title")).toString();
    if (args.contains(QStringLiteral("groupId")))
      params[QStringLiteral("groupId")] = args.value(QStringLiteral("groupId")).toInt();

    if (args.contains(Keys::SourceId))
      params[Keys::SourceId] = args.value(Keys::SourceId).toInt();

    const auto reply = tryResolve(QStringLiteral("project.dataset.getByTitle"), params);
    if (reply.value(QStringLiteral("ok")).toBool()) {
      resolved = reply.value(QStringLiteral("result")).toObject();
      method   = QStringLiteral("title");
    }
  }

  QJsonObject out;
  if (resolved.isEmpty()) {
    out[QStringLiteral("ok")]       = false;
    out[QStringLiteral("error")]    = QStringLiteral("dataset_not_resolved");
    out[QStringLiteral("attempts")] = attempts;
    return out;
  }

  out[QStringLiteral("ok")]      = true;
  out[QStringLiteral("method")]  = method;
  out[QStringLiteral("dataset")] = resolved;
  out[QStringLiteral("identity")] =
    QStringLiteral("Use groupId + datasetId for dataset mutations; use uniqueId only as an opaque "
                   "stable resolver; use index only for parser output order.");
  return out;
}

/**
 * @brief Resolves a workspace by id or title against project.workspace.list.
 */
static QJsonObject resolveWorkspace(const QJsonObject& args)
{
  const auto listReply = runCommand(QStringLiteral("project.workspace.list"));
  if (!listReply.value(QStringLiteral("ok")).toBool())
    return listReply;

  const auto list      = listReply.value(QStringLiteral("result")).toObject();
  const auto rows      = list.value(QStringLiteral("workspaces")).toArray();
  const int wantedId   = args.value(QStringLiteral("workspaceId")).toInt(-1);
  const QString title  = args.value(QStringLiteral("title")).toString();
  const auto titleFold = title.toCaseFolded();

  if (wantedId < 0 && title.isEmpty() && !rows.isEmpty()) {
    QJsonObject out;
    out[QStringLiteral("ok")]        = true;
    out[QStringLiteral("workspace")] = rows.first().toObject();
    out[QStringLiteral("defaulted")] = true;
    out[QStringLiteral("hint")] =
      QStringLiteral("No workspace was specified, so the first active workspace was selected.");
    return out;
  }

  for (const auto& value : rows) {
    const auto row = value.toObject();
    if (wantedId >= 0 && row.value(QStringLiteral("id")).toInt() == wantedId) {
      QJsonObject out;
      out[QStringLiteral("ok")]        = true;
      out[QStringLiteral("workspace")] = row;
      return out;
    }

    if (!title.isEmpty()
        && row.value(QStringLiteral("title")).toString().toCaseFolded() == titleFold) {
      QJsonObject out;
      out[QStringLiteral("ok")]        = true;
      out[QStringLiteral("workspace")] = row;
      return out;
    }
  }

  QJsonObject out;
  out[QStringLiteral("ok")]         = false;
  out[QStringLiteral("error")]      = QStringLiteral("workspace_not_resolved");
  out[QStringLiteral("workspaces")] = rows;
  return out;
}

/**
 * @brief Resolves a group by id or title against project.group.list.
 */
static QJsonObject resolveGroup(const QJsonObject& args)
{
  const auto listReply = runCommand(QStringLiteral("project.group.list"));
  if (!listReply.value(QStringLiteral("ok")).toBool())
    return listReply;

  const auto rows =
    listReply.value(QStringLiteral("result")).toObject().value(QStringLiteral("groups")).toArray();
  const int wantedId = args.value(QStringLiteral("groupId")).toInt(-1);
  const auto title   = args.value(QStringLiteral("group")).toString().toCaseFolded();

  for (const auto& value : rows) {
    const auto row = value.toObject();
    if (wantedId >= 0
        && (row.value(QStringLiteral("groupId")).toInt(-1) == wantedId
            || row.value(QStringLiteral("id")).toInt(-1) == wantedId)) {
      QJsonObject out;
      out[QStringLiteral("ok")]    = true;
      out[QStringLiteral("group")] = row;
      return out;
    }

    if (!title.isEmpty() && row.value(QStringLiteral("title")).toString().toCaseFolded() == title) {
      QJsonObject out;
      out[QStringLiteral("ok")]    = true;
      out[QStringLiteral("group")] = row;
      return out;
    }
  }

  QJsonObject out;
  out[QStringLiteral("ok")]     = false;
  out[QStringLiteral("error")]  = QStringLiteral("group_not_resolved");
  out[QStringLiteral("groups")] = rows;
  return out;
}

/**
 * @brief Maps a widget slug to the dataset option slug that enables that widget.
 */
static QString optionSlugForWidget(const QString& widgetType)
{
  static const QHash<QString, QString> kMap = {
    {     QStringLiteral("plot"),      QStringLiteral("plot")},
    {      QStringLiteral("fft"),       QStringLiteral("fft")},
    {      QStringLiteral("bar"),       QStringLiteral("bar")},
    {    QStringLiteral("gauge"),     QStringLiteral("gauge")},
    {  QStringLiteral("compass"),   QStringLiteral("compass")},
    {      QStringLiteral("led"),       QStringLiteral("led")},
    {QStringLiteral("waterfall"), QStringLiteral("waterfall")},
  };
  return kMap.value(widgetType);
}

/**
 * @brief Returns a trimmed dataset array carrying only fields the assistant needs.
 */
static QJsonArray compactDatasets(const QJsonArray& datasets)
{
  QJsonArray rows;
  for (const auto& value : datasets) {
    const auto ds = value.toObject();
    QJsonObject row;
    row[Keys::DatasetId]         = ds.value(Keys::DatasetId).toInt();
    row[Keys::UniqueId]          = ds.value(Keys::UniqueId).toInt();
    row[QStringLiteral("index")] = ds.value(QStringLiteral("index")).toInt();
    row[QStringLiteral("title")] = ds.value(QStringLiteral("title")).toString();
    if (ds.contains(QStringLiteral("units")))
      row[QStringLiteral("units")] = ds.value(QStringLiteral("units")).toString();

    if (ds.contains(QStringLiteral("enabledOptionsSlugs")))
      row[QStringLiteral("enabledOptionsSlugs")] =
        ds.value(QStringLiteral("enabledOptionsSlugs")).toArray();

    if (ds.value(QStringLiteral("hasTransform")).toBool())
      row[QStringLiteral("hasTransform")] = true;

    if (ds.value(QStringLiteral("isVirtual")).toBool())
      row[QStringLiteral("isVirtual")] = true;

    rows.append(row);
  }
  return rows;
}

/**
 * @brief Compacts a project.snapshot result into the assistant.snapshot shape.
 */
static QJsonObject compactProjectSnapshotResult(const QJsonObject& projectResult,
                                                const QJsonObject& workspaceResult,
                                                bool includeRaw)
{
  const auto snapshot = projectResult.value(QStringLiteral("snapshot")).toObject();

  QJsonObject compact;
  compact[QStringLiteral("title")]    = snapshot.value(QStringLiteral("title")).toString();
  compact[QStringLiteral("filePath")] = snapshot.value(QStringLiteral("filePath")).toString();
  compact[QStringLiteral("operationMode")] =
    snapshot.value(QStringLiteral("operationMode")).toInt();
  compact[QStringLiteral("groupCount")]   = snapshot.value(QStringLiteral("groupCount")).toInt();
  compact[QStringLiteral("datasetCount")] = snapshot.value(QStringLiteral("datasetCount")).toInt();
  compact[QStringLiteral("projectEpoch")] = projectResult.value(QStringLiteral("projectEpoch"));
  compact[QStringLiteral("summary")]      = snapshot.value(QStringLiteral("_explanations"))
                                         .toObject()
                                         .value(QStringLiteral("summary"))
                                         .toString();

  QJsonArray groups;
  for (const auto& value : snapshot.value(QStringLiteral("groups")).toArray()) {
    const auto group = value.toObject();
    QJsonObject row;
    row[QStringLiteral("groupId")]      = group.value(QStringLiteral("groupId")).toInt();
    row[QStringLiteral("title")]        = group.value(QStringLiteral("title")).toString();
    row[QStringLiteral("widget")]       = group.value(QStringLiteral("widget")).toString();
    row[QStringLiteral("datasetCount")] = group.value(QStringLiteral("datasetCount")).toInt();
    if (group.contains(QStringLiteral("compatibleWidgetTypeSlugs")))
      row[QStringLiteral("compatibleWidgetTypeSlugs")] =
        group.value(QStringLiteral("compatibleWidgetTypeSlugs")).toArray();

    row[QStringLiteral("datasets")] =
      compactDatasets(group.value(QStringLiteral("datasets")).toArray());
    groups.append(row);
  }
  compact[QStringLiteral("groups")] = groups;

  const auto workspaceRows = workspaceResult.value(QStringLiteral("workspaces")).toArray();
  compact[QStringLiteral("workspaces")] = workspaceRows.isEmpty()
                                          ? snapshot.value(QStringLiteral("workspaces")).toArray()
                                          : workspaceRows;
  compact[QStringLiteral("dataTables")] = snapshot.value(QStringLiteral("dataTables")).toArray();
  compact[QStringLiteral("hint")]       = projectResult.value(QStringLiteral("hint")).toString();

  if (includeRaw)
    compact[QStringLiteral("rawProjectSnapshot")] = projectResult;

  return compact;
}

/**
 * @brief Maps a script kind + language pair to the matching scripting-docs key.
 */
static QString scriptingDocKindForScriptKind(const QString& kind, int language)
{
  if (kind == QStringLiteral("frame_parser"))
    return language == 0 ? QStringLiteral("frame_parser_js") : QStringLiteral("frame_parser_lua");

  if (kind == QStringLiteral("transform"))
    return language == 0 ? QStringLiteral("transform_js") : QStringLiteral("transform_lua");

  if (kind == QStringLiteral("painter"))
    return QStringLiteral("painter_js");

  return {};
}

/**
 * @brief Returns true when a dry-run reply succeeded at both envelope and inner-result level.
 */
static bool dryRunResultOk(const QJsonObject& reply)
{
  if (!reply.value(QStringLiteral("ok")).toBool())
    return false;

  const auto result = reply.value(QStringLiteral("result")).toObject();
  return result.isEmpty() || result.value(QStringLiteral("ok")).toBool(true);
}

/**
 * @brief Resolves the dataset that a transform-apply call should target.
 */
static QJsonObject scriptTargetDataset(const QJsonObject& args)
{
  if (args.contains(QStringLiteral("groupId")) && args.contains(Keys::DatasetId)) {
    QJsonObject dataset;
    dataset[QStringLiteral("groupId")] = args.value(QStringLiteral("groupId")).toInt();
    dataset[Keys::DatasetId]           = args.value(Keys::DatasetId).toInt();
    QJsonObject out;
    out[QStringLiteral("ok")]      = true;
    out[QStringLiteral("dataset")] = dataset;
    return out;
  }

  QJsonObject dsArgs;
  if (args.contains(Keys::UniqueId))
    dsArgs[Keys::UniqueId] = args.value(Keys::UniqueId).toInt();

  if (!args.value(QStringLiteral("dataset")).toString().isEmpty()) {
    const auto datasetName = args.value(QStringLiteral("dataset")).toString();
    if (datasetName.contains(QLatin1Char('/')))
      dsArgs[QStringLiteral("path")] = datasetName;
    else
      dsArgs[QStringLiteral("title")] = datasetName;
  }
  if (args.contains(QStringLiteral("groupId")))
    dsArgs[QStringLiteral("groupId")] = args.value(QStringLiteral("groupId")).toInt();

  if (dsArgs.isEmpty()) {
    QJsonObject out;
    out[QStringLiteral("ok")]    = false;
    out[QStringLiteral("error")] = QStringLiteral("missing_dataset_target");
    out[QStringLiteral("hint")] =
      QStringLiteral("Pass groupId+datasetId, dataset path/title, or uniqueId.");
    return out;
  }

  return resolveDataset(dsArgs);
}

/**
 * @brief Picks the frame-parser dry-run command and seeds its inputs.
 */
static QString frameParserDryRunCommand(const QJsonObject& args, QJsonObject& dryArgs, int language)
{
  dryArgs[QStringLiteral("language")] = language;

  if (args.contains(QStringLiteral("inputBytes"))
      || args.contains(QStringLiteral("inputBytesHex"))) {
    static const Keys::KeyView keys[] = {
      Keys::KeyView("inputBytes"),
      Keys::KeyView("inputBytesHex"),
      Keys::DecoderMethod,
      Keys::FrameDetection,
      Keys::FrameStart,
      Keys::FrameEnd,
      Keys::HexadecimalDelimiters,
      Keys::ChecksumAlgorithm,
      Keys::KeyView("operationMode"),
    };
    for (const auto& k : keys)
      if (args.contains(k))
        dryArgs[k] = args.value(k);

    return QStringLiteral("project.frameParser.dryRun");
  }

  return QStringLiteral("project.frameParser.dryCompile");
}

/**
 * @brief Populates transform dry-run inputs with language and sample values.
 */
static void seedTransformDryArgs(const QJsonObject& args, QJsonObject& dryArgs, int language)
{
  dryArgs[QStringLiteral("language")] = language;
  dryArgs[QStringLiteral("values")]   = args.contains(QStringLiteral("values"))
                                        ? args.value(QStringLiteral("values")).toArray()
                                        : QJsonArray{0};
}

/**
 * @brief Populates end-to-end dry-run inputs, copying optional source / sample / verbose keys.
 */
static void seedEndToEndDryArgs(const QJsonObject& args, QJsonObject& dryArgs, int language)
{
  if (args.contains(Keys::SourceId))
    dryArgs[Keys::SourceId] = args.value(Keys::SourceId).toInt();

  if (args.contains(QStringLiteral("language")))
    dryArgs[QStringLiteral("language")] = language;

  if (args.contains(QStringLiteral("sampleFrames")))
    dryArgs[QStringLiteral("sampleFrames")] = args.value(QStringLiteral("sampleFrames")).toArray();

  if (args.contains(QStringLiteral("sampleFrame")))
    dryArgs[QStringLiteral("sampleFrame")] = args.value(QStringLiteral("sampleFrame")).toString();

  if (args.contains(QStringLiteral("verbose")))
    dryArgs[QStringLiteral("verbose")] = args.value(QStringLiteral("verbose")).toBool();
}

/**
 * @brief Picks the dry-run command for a script kind and seeds its argument map.
 */
static QString dryRunCommandForKind(const QString& kind,
                                    const QJsonObject& args,
                                    QJsonObject& dryArgs,
                                    int language)
{
  if (kind == QStringLiteral("frame_parser"))
    return frameParserDryRunCommand(args, dryArgs, language);

  if (kind == QStringLiteral("transform")) {
    seedTransformDryArgs(args, dryArgs, language);
    return QStringLiteral("project.dataset.transform.dryRun");
  }

  if (kind == QStringLiteral("painter"))
    return QStringLiteral("project.painter.dryRun");

  if (kind == QStringLiteral("end_to_end")) {
    seedEndToEndDryArgs(args, dryArgs, language);
    return QStringLiteral("project.dryRun.endToEnd");
  }

  return {};
}

/**
 * @brief Dispatches a dry-run call to the matching scripting endpoint and attaches references.
 */
static QJsonObject executeScriptDryRun(const QJsonObject& args)
{
  const auto kind = args.value(QStringLiteral("kind")).toString();
  const auto code = args.value(QStringLiteral("code")).toString();
  const int language =
    args.contains(QStringLiteral("language")) ? args.value(QStringLiteral("language")).toInt() : 1;

  QJsonObject dryArgs;
  if (!code.isEmpty())
    dryArgs[QStringLiteral("code")] = code;

  const QString command = dryRunCommandForKind(kind, args, dryArgs, language);
  if (command.isEmpty()) {
    QJsonObject out;
    out[QStringLiteral("ok")]    = false;
    out[QStringLiteral("error")] = QStringLiteral("unknown_script_kind");
    return out;
  }

  auto reply                         = runCommand(command, dryArgs);
  reply[QStringLiteral("command")]   = command;
  reply[QStringLiteral("arguments")] = dryArgs;
  const auto docsKind                = scriptingDocKindForScriptKind(kind, language);
  if (!docsKind.isEmpty())
    reply[QStringLiteral("reference")] =
      QStringLiteral("meta.fetchScriptingDocs{kind:'%1'}").arg(docsKind);

  return attachRepairHint(reply, command);
}

/**
 * @brief Toggles the target dataset to virtual when the apply request asks for it.
 */
static QJsonObject maybeMarkDatasetVirtual(const QJsonObject& args,
                                           int groupId,
                                           int datasetId,
                                           QJsonArray& steps)
{
  if (!args.value(QStringLiteral("virtual")).toBool(false))
    return {};

  const QJsonObject virtualArgs{
    {QStringLiteral("groupId"),   groupId},
    {          Keys::DatasetId, datasetId},
    {QStringLiteral("virtual"),      true}
  };
  const auto virtualReply = runCommand(QStringLiteral("project.dataset.setVirtual"), virtualArgs);
  steps.append(QJsonObject{
    {QStringLiteral("command"),      QStringLiteral("project.dataset.setVirtual")},
    {     QStringLiteral("ok"), virtualReply.value(QStringLiteral("ok")).toBool()}
  });
  return virtualReply;
}

/**
 * @brief Applies a transform script to its target dataset, optionally promoting it to virtual.
 */
static QJsonObject applyTransformScript(const QJsonObject& args,
                                        const QJsonObject& dryRun,
                                        int language)
{
  const auto target = scriptTargetDataset(args);
  if (!target.value(QStringLiteral("ok")).toBool())
    return target;

  const auto dataset  = target.value(QStringLiteral("dataset")).toObject();
  const int groupId   = dataset.value(QStringLiteral("groupId")).toInt();
  const int datasetId = dataset.value(Keys::DatasetId).toInt();

  QJsonArray steps;
  const auto virtualReply = maybeMarkDatasetVirtual(args, groupId, datasetId, steps);
  if (!virtualReply.isEmpty() && !virtualReply.value(QStringLiteral("ok")).toBool())
    return attachRepairHint(virtualReply, QStringLiteral("project.dataset.setVirtual"));

  QJsonObject applyArgs;
  applyArgs[QStringLiteral("code")]     = args.value(QStringLiteral("code")).toString();
  applyArgs[QStringLiteral("groupId")]  = groupId;
  applyArgs[Keys::DatasetId]            = datasetId;
  applyArgs[QStringLiteral("language")] = language;

  const QString command                = QStringLiteral("project.dataset.setTransformCode");
  auto applyReply                      = runCommand(command, applyArgs);
  applyReply[QStringLiteral("dryRun")] = dryRun;
  if (!steps.isEmpty())
    applyReply[QStringLiteral("steps")] = steps;

  return attachRepairHint(applyReply, command);
}

/**
 * @brief Applies a frame-parser script, forwarding optional sourceId to the API.
 */
static QJsonObject applyFrameParserScript(const QJsonObject& args,
                                          const QJsonObject& dryRun,
                                          int language)
{
  QJsonObject applyArgs;
  applyArgs[QStringLiteral("code")]     = args.value(QStringLiteral("code")).toString();
  applyArgs[QStringLiteral("language")] = language;
  if (args.contains(Keys::SourceId))
    applyArgs[Keys::SourceId] = args.value(Keys::SourceId).toInt();

  const QString command                = QStringLiteral("project.frameParser.setCode");
  auto applyReply                      = runCommand(command, applyArgs);
  applyReply[QStringLiteral("dryRun")] = dryRun;
  return attachRepairHint(applyReply, command);
}

/**
 * @brief Applies a painter script, resolving the target group from the request.
 */
static QJsonObject applyPainterScript(const QJsonObject& args, const QJsonObject& dryRun)
{
  auto groupReply = resolveGroup(args);
  if (!groupReply.value(QStringLiteral("ok")).toBool())
    return groupReply;

  QJsonObject applyArgs;
  applyArgs[QStringLiteral("code")] = args.value(QStringLiteral("code")).toString();
  applyArgs[QStringLiteral("groupId")] =
    groupReply.value(QStringLiteral("group")).toObject().value(QStringLiteral("groupId")).toInt();

  const QString command                = QStringLiteral("project.painter.setCode");
  auto applyReply                      = runCommand(command, applyArgs);
  applyReply[QStringLiteral("dryRun")] = dryRun;
  return attachRepairHint(applyReply, command);
}

/**
 * @brief Validates a script with dryRun, then routes the apply to the right project mutation.
 */
static QJsonObject executeScriptApply(const QJsonObject& args)
{
  const auto kind = args.value(QStringLiteral("kind")).toString();
  const int language =
    args.contains(QStringLiteral("language")) ? args.value(QStringLiteral("language")).toInt() : 1;

  QJsonObject dryRun = executeScriptDryRun(args);
  if (!dryRunResultOk(dryRun)) {
    QJsonObject out;
    out[QStringLiteral("ok")]     = false;
    out[QStringLiteral("error")]  = QStringLiteral("dry_run_failed");
    out[QStringLiteral("dryRun")] = dryRun;
    out[QStringLiteral("repair")] =
      QStringLiteral("Fix the script using the returned dry-run error before applying.");
    return out;
  }

  if (kind == QStringLiteral("frame_parser"))
    return applyFrameParserScript(args, dryRun, language);

  if (kind == QStringLiteral("transform"))
    return applyTransformScript(args, dryRun, language);

  if (kind == QStringLiteral("painter"))
    return applyPainterScript(args, dryRun);

  QJsonObject out;
  out[QStringLiteral("ok")]    = false;
  out[QStringLiteral("error")] = QStringLiteral("unsupported_apply_kind");
  out[QStringLiteral("hint")] =
    QStringLiteral("assistant.script.apply supports frame_parser, transform, and painter.");
  return out;
}

/**
 * @brief Projects a per-op result blob onto just the identity/status fields the model needs.
 */
static QJsonObject compactBatchRowResult(const QJsonObject& opResult)
{
  static const QStringList kIdentityKeys = {
    QStringLiteral("widgetId"),
    QStringLiteral("widgetType"),
    QStringLiteral("widgetTypeSlug"),
    QStringLiteral("workspaceId"),
    QStringLiteral("groupId"),
    QString(Keys::DatasetId),
    QString(Keys::UniqueId),
    QStringLiteral("id"),
    QStringLiteral("title"),
    QStringLiteral("index"),
    QStringLiteral("relativeIndex"),
    QStringLiteral("added"),
    QStringLiteral("removed"),
    QStringLiteral("deleted"),
    QStringLiteral("updated"),
    QStringLiteral("renamed"),
    QStringLiteral("cleared"),
  };

  QJsonObject slim;
  for (const auto& key : kIdentityKeys) {
    const auto v = opResult.value(key);
    if (!v.isUndefined() && !v.isNull())
      slim.insert(key, v);
  }
  return slim;
}

/**
 * @brief Validates and forwards a project.batch payload, summarizing any per-op failures.
 */
static QJsonObject executeBulkApply(const QJsonObject& args)
{
  const auto ops = args.value(QStringLiteral("ops")).toArray();
  if (ops.isEmpty()) {
    QJsonObject out;
    out[QStringLiteral("ok")]    = false;
    out[QStringLiteral("error")] = QStringLiteral("missing_ops");
    return out;
  }

  if (ops.size() > 1024) {
    QJsonObject out;
    out[QStringLiteral("ok")]    = false;
    out[QStringLiteral("error")] = QStringLiteral("too_many_ops");
    out[QStringLiteral("hint")]  = QStringLiteral("Split project.batch calls at 1024 ops.");
    return out;
  }

  for (const auto& value : ops) {
    const auto op = value.toObject();
    if (op.value(QStringLiteral("command")).toString() == QStringLiteral("project.batch")) {
      QJsonObject out;
      out[QStringLiteral("ok")]    = false;
      out[QStringLiteral("error")] = QStringLiteral("nested_batch_rejected");
      return out;
    }
  }

  QJsonObject batchArgs;
  batchArgs[QStringLiteral("ops")] = ops;
  if (args.contains(QStringLiteral("stopOnError")))
    batchArgs[QStringLiteral("stopOnError")] = args.value(QStringLiteral("stopOnError")).toBool();

  auto raw = runCommand(QStringLiteral("project.batch"), batchArgs);
  if (!raw.value(QStringLiteral("ok")).toBool())
    return attachRepairHint(raw, QStringLiteral("project.batch"));

  const auto batchResult = raw.value(QStringLiteral("result")).toObject();
  const auto rows        = batchResult.value(QStringLiteral("results")).toArray();

  QJsonArray slimResults;
  QJsonArray failures;
  for (int i = 0; i < rows.size(); ++i) {
    const auto row = rows.at(i).toObject();
    const bool success =
      row.value(QStringLiteral("success")).toBool(row.value(QStringLiteral("ok")).toBool(true));

    QJsonObject slim;
    slim[QStringLiteral("index")]   = i;
    slim[QStringLiteral("command")] = row.value(QStringLiteral("command"));
    slim[QStringLiteral("success")] = success;
    if (success) {
      const auto opResult = row.value(QStringLiteral("result")).toObject();
      const auto identity = compactBatchRowResult(opResult);
      if (!identity.isEmpty())
        slim[QStringLiteral("result")] = identity;
    } else {
      slim[QStringLiteral("error")] = row.value(QStringLiteral("error"));
      failures.append(slim);
    }
    slimResults.append(slim);
  }

  QJsonObject summary;
  summary[QStringLiteral("total")]        = batchResult.value(QStringLiteral("total"));
  summary[QStringLiteral("succeeded")]    = batchResult.value(QStringLiteral("succeeded"));
  summary[QStringLiteral("failed")]       = batchResult.value(QStringLiteral("failed"));
  summary[QStringLiteral("aborted")]      = batchResult.value(QStringLiteral("aborted"));
  summary[QStringLiteral("autoSaveMode")] = batchResult.value(QStringLiteral("autoSaveMode"));

  QJsonObject reply;
  reply[QStringLiteral("ok")]           = true;
  reply[QStringLiteral("summary")]      = summary;
  reply[QStringLiteral("failureCount")] = failures.size();
  if (!failures.isEmpty())
    reply[QStringLiteral("failures")] = failures;

  reply[QStringLiteral("results")] = slimResults;
  return reply;
}

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
    dsArgs[Keys::UniqueId] = args.value(Keys::UniqueId).toInt();

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

/**
 * @brief Orchestrates the workspace tile addition flow end-to-end.
 */
static QJsonObject executeAddTile(const QJsonObject& args)
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

/**
 * @brief Top-level dispatcher for every assistant.* virtual tool name.
 */
static QJsonObject executeAssistantTool(const QString& name, const QJsonObject& args)
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
      || name == QStringLiteral("assistant.listCheckpoints"))
    return runCommand(name, args);

  QJsonObject out;
  out[QStringLiteral("ok")]    = false;
  out[QStringLiteral("error")] = QStringLiteral("unknown_assistant_tool");
  return out;
}

//--------------------------------------------------------------------------------------------------
// Catalog
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns AI-tool catalog derived from API::CommandRegistry, minus Blocked entries.
 */
QJsonArray AI::ToolDispatcher::availableTools(const QString& category) const
{
  QJsonArray tools;
  for (const auto& def : assistantToolDefs()) {
    if (!category.isEmpty() && !def.name.startsWith(category))
      continue;

    QJsonObject tool;
    tool[QStringLiteral("name")]        = def.name;
    tool[QStringLiteral("description")] = def.description;
    tool[QStringLiteral("inputSchema")] = def.inputSchema;
    tools.append(tool);
  }

  const auto& commands = API::CommandRegistry::instance().commands();
  const auto& aiReg    = AI::CommandRegistry::instance();

  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
    const auto& def = it.value();
    if (aiReg.safetyOf(def.name) == Safety::Blocked)
      continue;

    if (!category.isEmpty() && !def.name.startsWith(category))
      continue;

    QJsonObject tool;
    tool[QStringLiteral("name")]        = def.name;
    tool[QStringLiteral("description")] = def.description;
    tool[QStringLiteral("inputSchema")] = def.inputSchema;
    tools.append(tool);
  }

  return tools;
}

/** @brief Returns a compact name+description list of every command, optionally
 *         filtered by prefix. Useful as a discovery tool for the AI. */
QJsonObject AI::ToolDispatcher::listCommands(const QString& prefix) const
{
  const auto& commands = API::CommandRegistry::instance().commands();
  const auto& aiReg    = AI::CommandRegistry::instance();

  QJsonArray entries;
  for (const auto& def : assistantToolDefs()) {
    if (!prefix.isEmpty() && !def.name.startsWith(prefix))
      continue;

    QJsonObject row;
    row[QStringLiteral("name")]        = def.name;
    row[QStringLiteral("description")] = def.description;
    entries.append(row);
  }

  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
    const auto& def = it.value();
    if (aiReg.safetyOf(def.name) == Safety::Blocked)
      continue;

    if (!prefix.isEmpty() && !def.name.startsWith(prefix))
      continue;

    QJsonObject row;
    row[QStringLiteral("name")]        = def.name;
    row[QStringLiteral("description")] = def.description;
    entries.append(row);
  }

  QJsonObject reply;
  reply[QStringLiteral("ok")]       = true;
  reply[QStringLiteral("count")]    = entries.size();
  reply[QStringLiteral("commands")] = entries;
  return reply;
}

/**
 * @brief Returns the top-level scope namespaces with descriptions.
 */
QJsonObject AI::ToolDispatcher::listCategories() const
{
  // Curated scope descriptions (only emitted when scope has non-Blocked commands)
  // clang-format off
  static const QHash<QString, QString> kDescriptions = {
    {QStringLiteral("project"),
     QStringLiteral("Project document operations: open/save, groups, datasets, "
                    "actions, sources, parsers, painter scripts, output widgets, "
                    "data tables, workspaces.")},
    {QStringLiteral("io"),
     QStringLiteral("Hardware I/O: connect/disconnect, bus selection, plus "
                    "per-driver configuration (uart, network, ble, audio, canbus, "
                    "modbus, hid, usb, process).")},
    {QStringLiteral("console"),
     QStringLiteral("Terminal display, send raw frames, console export.")},
    {QStringLiteral("consoleExport"),
     QStringLiteral("Console capture export to file.")},
    {QStringLiteral("csv"),
     QStringLiteral("CSV export status and control.")},
    {QStringLiteral("csvExport"),
     QStringLiteral("CSV export to file.")},
    {QStringLiteral("csvPlayer"),
     QStringLiteral("CSV file replay: open/close, paused state, step, seek.")},
    {QStringLiteral("mdf4Export"),
     QStringLiteral("MDF4 export to file (Pro).")},
    {QStringLiteral("mdf4Player"),
     QStringLiteral("MDF4 file replay (Pro).")},
    {QStringLiteral("mqtt"),
     QStringLiteral("MQTT broker connectivity, authentication, SSL, topics (Pro).")},
    {QStringLiteral("dashboard"),
     QStringLiteral("Live dashboard data, FPS, point limits, operation mode.")},
    {QStringLiteral("ui"),
     QStringLiteral("Window state, layouts, widget settings, active group.")},
    {QStringLiteral("sessions"),
     QStringLiteral("Session database export (Pro).")},
    {QStringLiteral("licensing"),
     QStringLiteral("License activation, validation, trial mode.")},
    {QStringLiteral("notifications"),
     QStringLiteral("In-app notification center: post, list, channels, mark read.")},
    {QStringLiteral("extensions"),
     QStringLiteral("Plugin/extension lifecycle.")},
    {QStringLiteral("meta"),
     QStringLiteral("Discovery: list categories, list commands by prefix, describe one "
                    "command, fetch help/scripting docs/recipes, execute by name.")},
    {QStringLiteral("assistant"),
     QStringLiteral("High-level assistant rails: compact project snapshots, dataset/workspace "
                    "resolvers, workspace planning, and safe workspace tile orchestration.")},
  };
  // clang-format on

  const auto& commands = API::CommandRegistry::instance().commands();
  const auto& aiReg    = AI::CommandRegistry::instance();

  QHash<QString, int> counts;
  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
    if (aiReg.safetyOf(it.value().name) == Safety::Blocked)
      continue;

    const QString scope  = it.value().name.section(QLatin1Char('.'), 0, 0);
    counts[scope]       += 1;
  }

  // Always advertise the meta scope even if it isn't in the registry.
  if (!counts.contains(QStringLiteral("meta")))
    counts[QStringLiteral("meta")] = 6;

  counts[QStringLiteral("assistant")] = assistantToolDefs().size();

  // Sort scopes (build vector first since QJsonValueRef is non-swappable)
  std::vector<QJsonObject> rows;
  rows.reserve(counts.size());
  for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
    QJsonObject row;
    row[QStringLiteral("scope")]        = it.key();
    row[QStringLiteral("commandCount")] = it.value();
    row[QStringLiteral("description")]  = kDescriptions.value(it.key(), QStringLiteral(""));
    row[QStringLiteral("listPrefix")]   = it.key() + QLatin1Char('.');
    rows.push_back(row);
  }

  std::sort(rows.begin(), rows.end(), [](const QJsonObject& a, const QJsonObject& b) {
    return a.value(QStringLiteral("scope")).toString()
         < b.value(QStringLiteral("scope")).toString();
  });

  QJsonArray entries;
  for (const auto& row : rows)
    entries.append(row);

  QJsonObject reply;
  reply[QStringLiteral("ok")]         = true;
  reply[QStringLiteral("count")]      = entries.size();
  reply[QStringLiteral("categories")] = entries;
  reply[QStringLiteral("hint")] =
    QStringLiteral("Call meta.listCommands{prefix: \"<scope>.\"} to see the commands in a scope, "
                   "then meta.describeCommand{name} for the input schema before invoking.");
  return reply;
}

/**
 * @brief Returns the metadata block for a single command, or an empty object.
 */
QJsonObject AI::ToolDispatcher::describeCommand(const QString& name) const
{
  if (isAssistantTool(name))
    return assistantToolDescription(name);

  const auto& commands = API::CommandRegistry::instance().commands();
  const auto it        = commands.constFind(name);
  if (it == commands.constEnd())
    return {};

  if (AI::CommandRegistry::instance().safetyOf(name) == Safety::Blocked)
    return {};

  QJsonObject desc;
  desc[QStringLiteral("name")]        = it.value().name;
  desc[QStringLiteral("description")] = it.value().description;
  desc[QStringLiteral("inputSchema")] = it.value().inputSchema;
  return desc;
}

//--------------------------------------------------------------------------------------------------
// Dispatch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates args and forwards to API::CommandRegistry honoring AI safety tags.
 */
QJsonObject AI::ToolDispatcher::executeCommand(const QString& name,
                                               const QJsonObject& args,
                                               bool autoConfirmSafe)
{
  // Validate the args envelope
  Misc::JsonValidator::Limits limits;
  limits.maxFileSize  = 1 * 1024 * 1024;
  limits.maxDepth     = 32;
  limits.maxArraySize = 1000;

  if (!Misc::JsonValidator::validateStructure(QJsonValue(args), limits)) {
    qCWarning(serialStudioAI) << "Tool args validation failed for" << name;
    QJsonObject reply;
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("args_validation_failed");
    return reply;
  }

  // Read-only previews (dryRun:true) bypass the Confirm gate; Blocked still blocks.
  const auto safety        = AI::CommandRegistry::instance().safetyOf(name);
  const bool isDryRunQuery = args.value(QStringLiteral("dryRun")).toBool(false);
  if (safety == Safety::Blocked) {
    qCWarning(serialStudioAI) << "Tool execution blocked:" << name;
    QJsonObject error;
    error[QStringLiteral("code")]    = QStringLiteral("blocked");
    error[QStringLiteral("message")] = QStringLiteral("This command is blocked for AI safety.");
    if (name.startsWith(QStringLiteral("io.")) || name == QStringLiteral("console.send")) {
      error[QStringLiteral("repair")] =
        QStringLiteral("Hardware writes and connection changes must be performed by the user. "
                       "Offer to build an Output Control tile instead.");
    }

    QJsonObject reply;
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = error;
    return reply;
  }

  // Confirm gates require explicit user approval (autoConfirmSafe=true).
  if ((safety == Safety::Confirm || safety == Safety::AlwaysConfirm) && !autoConfirmSafe
      && !isDryRunQuery) {
    Q_EMIT confirmationRequested(name, args);
    QJsonObject reply;
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("awaiting_confirmation");
    reply[QStringLiteral("message")] =
      QStringLiteral("User approval pending. The chat UI is showing an approval card for this "
                     "tool call. Do not retry, do not invoke with confirm/force/token params -- "
                     "the result will arrive automatically once the user clicks Approve or Deny.");
    reply[QStringLiteral("hint")] =
      QStringLiteral("If you intend many similar destructive operations, queue them all and let "
                     "the user use the Approve-all group action in the chat. Retrying this call "
                     "will only queue another pending approval -- it cannot bypass the gate.");
    return reply;
  }

  if (isAssistantTool(name))
    return executeAssistantTool(name, args);

  // Forward to API::CommandRegistry
  const auto callId   = QUuid::createUuid().toString(QUuid::WithoutBraces);
  const auto response = API::CommandRegistry::instance().execute(name, callId, args);

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

    const auto repair = makeRepairHint(name, response.errorMessage);
    if (!repair.isEmpty())
      error[QStringLiteral("repair")] = repair;

    reply[QStringLiteral("error")] = error;
  }
  return reply;
}

//--------------------------------------------------------------------------------------------------
// Context (placeholders for the next slice)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the result.result field of a safe command, or an empty object.
 */
static QJsonObject runSafeCommand(const QString& name)
{
  const auto callId   = QUuid::createUuid().toString(QUuid::WithoutBraces);
  const auto response = API::CommandRegistry::instance().execute(name, callId, {});
  if (!response.success)
    return {};

  return response.result;
}

/**
 * @brief Returns a one-shot composite of every readable status endpoint.
 */
QJsonObject AI::ToolDispatcher::getSnapshot() const
{
  // Curated read-only commands; missing ones (e.g. commercial-only) are skipped.
  static const QStringList kStatusCommands = {
    QStringLiteral("project.getStatus"),
    QStringLiteral("io.getStatus"),
    QStringLiteral("dashboard.getStatus"),
    QStringLiteral("console.getConfig"),
    QStringLiteral("consoleExport.getStatus"),
    QStringLiteral("csvExport.getStatus"),
    QStringLiteral("csvPlayer.getStatus"),
    QStringLiteral("mqtt.getConnectionStatus"),
    QStringLiteral("sessions.getStatus"),
    QStringLiteral("mdf4Export.getStatus"),
    QStringLiteral("mdf4Player.getStatus"),
    QStringLiteral("licensing.getStatus"),
    QStringLiteral("notifications.getUnreadCount"),
  };

  QJsonObject snapshot;
  QJsonArray skipped;
  for (const auto& name : kStatusCommands) {
    if (!API::CommandRegistry::instance().hasCommand(name)) {
      skipped.append(name);
      continue;
    }

    snapshot.insert(name, runSafeCommand(name));
  }

  if (!skipped.isEmpty())
    snapshot.insert(QStringLiteral("skipped"), skipped);

  return snapshot;
}

/**
 * @brief Returns project structure assembled from a curated set of safe list commands.
 */
QJsonObject AI::ToolDispatcher::getProjectState() const
{
  static const QStringList kSafeListCommands = {
    QStringLiteral("project.group.list"),
    QStringLiteral("project.dataset.list"),
    QStringLiteral("project.action.list"),
    QStringLiteral("project.source.list"),
    QStringLiteral("project.dataTable.list"),
    QStringLiteral("project.workspace.list"),
    QStringLiteral("project.frameParser.getCode"),
    QStringLiteral("project.frameParser.getConfig"),
  };

  QJsonObject state;
  for (const auto& name : kSafeListCommands) {
    if (!API::CommandRegistry::instance().hasCommand(name))
      continue;

    state.insert(name, runSafeCommand(name));
  }

  return state;
}

/**
 * @brief Returns the markdown reference body for the given scripting kind.
 */
QJsonObject AI::ToolDispatcher::getScriptingDocs(const QString& kind) const
{
  static const QStringList kAllowed = {
    QStringLiteral("frame_parser_js"),
    QStringLiteral("frame_parser_lua"),
    QStringLiteral("transform_js"),
    QStringLiteral("transform_lua"),
    QStringLiteral("output_widget_js"),
    QStringLiteral("painter_js"),
  };

  QJsonObject reply;
  if (!kAllowed.contains(kind)) {
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("unknown_kind");
    reply[QStringLiteral("known")] = QJsonArray::fromStringList(kAllowed);
    return reply;
  }

  QFile file(QStringLiteral(":/ai/docs/%1.md").arg(kind));
  if (!file.open(QIODevice::ReadOnly)) {
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("doc_not_found");
    return reply;
  }

  const auto body = QString::fromUtf8(file.readAll());
  file.close();

  reply[QStringLiteral("ok")]   = true;
  reply[QStringLiteral("kind")] = kind;
  reply[QStringLiteral("body")] = body;
  return reply;
}
