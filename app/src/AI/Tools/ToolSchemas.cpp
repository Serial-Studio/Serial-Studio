/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Tools/ToolSchemas.h"

#include <QJsonArray>
#include <QStringList>

#include "DataModel/Frame.h"

namespace AI::ToolDetail {

//--------------------------------------------------------------------------------------------------
// JSON Schema primitives
//--------------------------------------------------------------------------------------------------

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
 * @brief Builds a `{type, description}` fragment that accepts either of two JSON Schema types; a
 *        JSON number keeps its numeric meaning and a JSON string keeps its string meaning.
 */
static QJsonObject makeMultiTypeProperty(const QString& typeA,
                                         const QString& typeB,
                                         const QString& description)
{
  QJsonObject prop;
  prop[QStringLiteral("type")]        = QJsonArray{typeA, typeB};
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

//--------------------------------------------------------------------------------------------------
// Assistant-native virtual tool schemas
//--------------------------------------------------------------------------------------------------

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
                                "'Source/Group/Dataset'. A bare name without '/' falls back "
                                "to an exact title match."));
  props[QStringLiteral("title")] =
    makeProperty(QStringLiteral("string"), QStringLiteral("Exact dataset title."));
  props[Keys::UniqueId] = makeMultiTypeProperty(
    QStringLiteral("integer"),
    QStringLiteral("string"),
    QStringLiteral("Dataset selector: integer uniqueId (opaque, from snapshot/list) or string "
                   "alias set in the editor. A JSON number is a uniqueId; a JSON string is always "
                   "an alias, never a uniqueId."));
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
                                   QStringLiteral("output_widget"),
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
  props[QStringLiteral("inputValue")] =
    makeProperty(QStringLiteral("string"),
                 QStringLiteral("[output_widget] Optional sample value to run transmit() "
                                "against so runtime errors and the produced bytes surface."));
  props[QStringLiteral("hex")] =
    makeProperty(QStringLiteral("boolean"),
                 QStringLiteral("[output_widget] Treat inputValue as space-separated hex bytes."));
  props[QStringLiteral("groupId")] = makeProperty(
    QStringLiteral("integer"), QStringLiteral("Target group id for transform/painter."));
  props[Keys::DatasetId] =
    makeProperty(QStringLiteral("integer"), QStringLiteral("Target dataset id for transforms."));
  props[QStringLiteral("dataset")] =
    makeProperty(QStringLiteral("string"), QStringLiteral("Dataset path/title for transforms."));
  props[Keys::UniqueId] = makeMultiTypeProperty(
    QStringLiteral("integer"),
    QStringLiteral("string"),
    QStringLiteral("Dataset selector for transforms: integer uniqueId or string alias."));
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
 * @brief Input schema for assistant.memory.propose.
 */
static QJsonObject memoryProposeInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("category")] = makeProperty(
    QStringLiteral("string"), QStringLiteral("One of: user, feedback, project, reference."));
  props[QStringLiteral("text")] =
    makeProperty(QStringLiteral("string"),
                 QStringLiteral("The fact to remember, stated plainly, under 400 characters."));
  return makeObjectSchema(props, QJsonArray{QStringLiteral("category"), QStringLiteral("text")});
}

/**
 * @brief Returns the snapshot/dataset/workspace tool schemas exposed under the assistant.* prefix;
 *        built once (nothing runtime-variable feeds the defs) and served from a static cache.
 */
const QVector<AssistantToolDef>& assistantToolDefs()
{
  static const QVector<AssistantToolDef> kDefs = []() -> QVector<AssistantToolDef> {
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
                      "Use for frame parsers, dataset transforms, and painter widgets. For "
                      "frame parsers, any frameDetection / frameStart / frameEnd / "
                      "decoderMethod / hexadecimalDelimiters / checksumAlgorithm args are also "
                      "persisted to the source configuration on success (reply carries the "
                      "outcome under frameConfig)."),
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
      {QStringLiteral("assistant.memory.propose"),
       QStringLiteral("Propose remembering a small durable fact for future chats (a stated "
                      "preference, a correction you were given, a project convention). The user "
                      "sees a confirmation chip and decides; this call NEVER stores anything by "
                      "itself."),
       memoryProposeInputSchema()},
    };
  }();
  return kDefs;
}

/**
 * @brief Returns true when `name` targets an assistant.* virtual tool.
 */
bool isAssistantTool(const QString& name)
{
  return name.startsWith(QStringLiteral("assistant."));
}

//--------------------------------------------------------------------------------------------------
// Sandboxed filesystem virtual tool schemas
//--------------------------------------------------------------------------------------------------

/**
 * @brief Input schema for fs.list.
 */
static QJsonObject fsListInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("path")] = makeProperty(
    QStringLiteral("string"),
    QStringLiteral("Directory path relative to the workspace folder (default '.'), or an "
                   "absolute path the user dragged into the chat."));
  props[QStringLiteral("recursive")] = makeProperty(
    QStringLiteral("boolean"), QStringLiteral("Recurse into subdirectories (bounded depth)."));
  return makeObjectSchema(props);
}

/**
 * @brief Input schema for fs.read.
 */
static QJsonObject fsReadInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("path")] =
    makeProperty(QStringLiteral("string"),
                 QStringLiteral("File path relative to the workspace folder, or an absolute "
                                "dragged-in path. Text files only."));
  props[QStringLiteral("offset")] =
    makeProperty(QStringLiteral("integer"),
                 QStringLiteral("Byte offset to start at; follow nextOffset to "
                                "page large files. Default 0."));
  props[QStringLiteral("limit")] = makeProperty(
    QStringLiteral("integer"),
    QStringLiteral("Max bytes to return this call (capped at 32 KB). Default is the cap."));
  return makeObjectSchema(props, QJsonArray{QStringLiteral("path")});
}

/**
 * @brief Input schema for fs.search.
 */
static QJsonObject fsSearchInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("query")] =
    makeProperty(QStringLiteral("string"),
                 QStringLiteral("Text to find (case-insensitive) across the "
                                "workspace and dragged-in paths."));
  props[QStringLiteral("isRegex")] = makeProperty(
    QStringLiteral("boolean"),
    QStringLiteral("Treat query as a regular expression instead of a literal string."));
  return makeObjectSchema(props, QJsonArray{QStringLiteral("query")});
}

/**
 * @brief Input schema for fs.write and fs.append.
 */
static QJsonObject fsWriteInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("path")] = makeProperty(
    QStringLiteral("string"),
    QStringLiteral("Destination path under the workspace 'AI/' subfolder, e.g. 'notes.md' "
                   "or 'exports/data.csv'. Paths outside AI/ are rejected."));
  props[QStringLiteral("content")] =
    makeProperty(QStringLiteral("string"), QStringLiteral("UTF-8 text to write."));
  return makeObjectSchema(props, QJsonArray{QStringLiteral("path"), QStringLiteral("content")});
}

/**
 * @brief Input schema for fs.delete.
 */
static QJsonObject fsDeleteInputSchema()
{
  QJsonObject props;
  props[QStringLiteral("path")] =
    makeProperty(QStringLiteral("string"),
                 QStringLiteral("File or empty directory under the 'AI/' subfolder to delete."));
  return makeObjectSchema(props, QJsonArray{QStringLiteral("path")});
}

/**
 * @brief Returns the fs.* virtual-tool catalog exposed to assistant providers; built once
 *        (nothing runtime-variable feeds the defs) and served from a static cache.
 */
const QVector<AssistantToolDef>& fsToolDefs()
{
  static const QVector<AssistantToolDef> kDefs = {
    {  QStringLiteral("fs.list"),
     QStringLiteral("List files and folders in the Serial Studio workspace folder (or a "
     "dragged-in directory). Read-only."),
     fsListInputSchema()  },
    {  QStringLiteral("fs.read"),
     QStringLiteral("Read a text file from the workspace folder (or a dragged-in path). "
     "Paged: pass offset/limit and follow nextOffset for large files. Refuses "
     "binary files."),
     fsReadInputSchema()  },
    {QStringLiteral("fs.search"),
     QStringLiteral("Search file contents across the workspace folder and dragged-in paths "
     "(grep-like, literal or regex). Read-only."),
     fsSearchInputSchema()},
    { QStringLiteral("fs.write"),
     QStringLiteral("Write a UTF-8 text file inside the workspace 'AI/' subfolder, replacing "
     "it. Use for notes, summaries, generated configs/exports. Cannot write "
     "outside AI/."),
     fsWriteInputSchema() },
    {QStringLiteral("fs.append"),
     QStringLiteral("Append UTF-8 text to a file inside the workspace 'AI/' subfolder, "
     "creating it if needed. Cannot write outside AI/."),
     fsWriteInputSchema() },
    {QStringLiteral("fs.delete"),
     QStringLiteral("Delete a file or empty directory inside the workspace 'AI/' subfolder. "
     "Always asks the user first."),
     fsDeleteInputSchema()},
  };
  return kDefs;
}

/**
 * @brief Returns true when `name` targets an fs.* sandboxed filesystem tool.
 */
bool isFsTool(const QString& name)
{
  return name.startsWith(QStringLiteral("fs."));
}

}  // namespace AI::ToolDetail
