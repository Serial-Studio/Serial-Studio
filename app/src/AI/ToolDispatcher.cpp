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
#include <QUuid>

#include "AI/CommandRegistry.h"
#include "AI/Logging.h"
#include "API/CommandProtocol.h"
#include "API/CommandRegistry.h"
#include "Misc/JsonValidator.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a tool dispatcher with the given Qt parent.
 */
AI::ToolDispatcher::ToolDispatcher(QObject* parent) : QObject(parent) {}

//--------------------------------------------------------------------------------------------------
// Catalog
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns AI-tool catalog derived from API::CommandRegistry, minus Blocked entries.
 */
QJsonArray AI::ToolDispatcher::availableTools(const QString& category) const
{
  QJsonArray tools;
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

  // Honor safety tags
  const auto safety = AI::CommandRegistry::instance().safetyOf(name);
  if (safety == Safety::Blocked) {
    qCWarning(serialStudioAI) << "Tool execution blocked:" << name;
    QJsonObject reply;
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("blocked");
    return reply;
  }

  // AlwaysConfirm bypasses autoApproveEdits so destructive families never run silently
  if (safety == Safety::AlwaysConfirm) {
    Q_EMIT confirmationRequested(name, args);
    QJsonObject reply;
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("awaiting_confirmation");
    return reply;
  }

  if (safety == Safety::Confirm && !autoConfirmSafe) {
    Q_EMIT confirmationRequested(name, args);
    QJsonObject reply;
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("awaiting_confirmation");
    return reply;
  }

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
