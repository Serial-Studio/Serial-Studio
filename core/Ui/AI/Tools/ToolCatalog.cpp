/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Tools/ToolCatalog.h"

#include <algorithm>
#include <QHash>
#include <QSet>
#include <QVector>
#include <vector>

#include "AI/CommandRegistry.h"
#include "AI/Tools/ToolFilesystemTools.h"
#include "AI/Tools/ToolSchemas.h"
#include "AI/Tools/ToolSupport.h"
#include "API/CommandRegistry.h"

namespace AI::ToolDetail {

//--------------------------------------------------------------------------------------------------
// Catalog
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns AI-tool catalog derived from API::CommandRegistry, minus Blocked entries.
 */
QJsonArray availableTools(const QString& category)
{
  QJsonArray tools;
  auto appendVirtual = [&tools, &category](const QVector<AssistantToolDef>& defs) {
    for (const auto& def : defs) {
      if (!category.isEmpty() && !def.name.startsWith(category))
        continue;

      QJsonObject tool;
      tool[QStringLiteral("name")]        = def.name;
      tool[QStringLiteral("description")] = def.description;
      tool[QStringLiteral("inputSchema")] = def.inputSchema;
      tools.append(tool);
    }
  };
  appendVirtual(assistantToolDefs());
  appendVirtual(fsToolDefs());

  static auto& apiRegistry = API::CommandRegistry::instance();
  const auto& commands     = apiRegistry.commands();
  static auto& aiReg       = AI::CommandRegistry::instance();

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

/**
 * @brief Name + one-line description for every meta.* tool advertised by Conversation.cpp; must
 *        stay in sync with the makeMetaTool calls there so meta.listCommands can enumerate them.
 */
static const QVector<AssistantToolDef>& metaToolRoster()
{
  static const QVector<AssistantToolDef> kRoster = {
    {    QStringLiteral("meta.listCategories"),
     QStringLiteral("List the top-level command scopes with descriptions and command counts."),
     {}},
    {          QStringLiteral("meta.snapshot"),
     QStringLiteral("One-shot composite of every readable status endpoint."),
     {}},
    {      QStringLiteral("meta.listCommands"),
     QStringLiteral(
     "List every available command (name + 1-line description), optionally "
     "filtered by dotted prefix, paged with offset/limit; namesOnly:true returns bare names "
     "so large scopes fit in one reply."),
     {}},
    {   QStringLiteral("meta.describeCommand"),
     QStringLiteral("Fetch the full input schema and description for one command."),
     {}},
    {    QStringLiteral("meta.executeCommand"),
     QStringLiteral("Execute any command by name with an arguments object."),
     {}},
    {         QStringLiteral("meta.fetchHelp"),
     QStringLiteral("Fetch a Serial Studio documentation page from the canonical help source."),
     {}},
    {QStringLiteral("meta.fetchScriptingDocs"),
     QStringLiteral("Fetch the scripting reference for one scripting context (frame parser, "
     "transform, painter, output widget, SDK source)."),
     {}},
    {             QStringLiteral("meta.howTo"),
     QStringLiteral("Fetch a step-by-step recipe for a common Serial Studio workflow."),
     {}},
    {         QStringLiteral("meta.loadSkill"),
     QStringLiteral("Load a focused skill reference for one area of Serial Studio."),
     {}},
    {        QStringLiteral("meta.searchDocs"),
     QStringLiteral("Semantic search over bundled docs, skills, templates, and example scripts."),
     {}},
    {            QStringLiteral("meta.search"),
     QStringLiteral(
     "Substring-search the command catalog itself (names + descriptions, every "
     "namespace); rows feed meta.describeCommand. For documentation pages use meta.searchDocs "
     "instead."),
     {}},
  };
  return kRoster;
}

/** @brief Returns a compact name+description list of every command (virtual tool defs win over
 *         API-registry twins), optionally filtered by prefix, windowed by offset/limit (limit
 *         0 = all), and reduced to bare name strings when namesOnly is set. */
QJsonObject listCommands(const QString& prefix, int offset, int limit, bool namesOnly)
{
  static auto& apiRegistry = API::CommandRegistry::instance();
  const auto& commands     = apiRegistry.commands();
  static auto& aiReg       = AI::CommandRegistry::instance();

  QJsonArray entries;
  QSet<QString> seen;
  auto appendRow = [&entries, &seen, &prefix, namesOnly](const QString& name,
                                                         const QString& description) {
    if (!prefix.isEmpty() && !name.startsWith(prefix))
      return;

    if (seen.contains(name))
      return;

    seen.insert(name);
    if (namesOnly) {
      entries.append(name);
      return;
    }

    QJsonObject row;
    row[QStringLiteral("name")]        = name;
    row[QStringLiteral("description")] = description;
    entries.append(row);
  };
  auto appendVirtual = [&appendRow](const QVector<AssistantToolDef>& defs) {
    for (const auto& def : defs)
      appendRow(def.name, def.description);
  };
  appendVirtual(assistantToolDefs());
  appendVirtual(fsToolDefs());
  appendVirtual(metaToolRoster());

  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
    const auto& def = it.value();
    if (aiReg.safetyOf(def.name) == Safety::Blocked)
      continue;

    appendRow(def.name, def.description);
  }

  const int total = entries.size();
  const int start = qBound(0, offset, total);
  const int count = limit > 0 ? qMin(limit, total - start) : total - start;

  QJsonArray window;
  for (int i = start; i < start + count; ++i)
    window.append(entries.at(i));

  QJsonObject reply;
  reply[QStringLiteral("ok")]       = true;
  reply[QStringLiteral("total")]    = total;
  reply[QStringLiteral("count")]    = window.size();
  reply[QStringLiteral("commands")] = window;
  if (start + count < total)
    reply[QStringLiteral("nextOffset")] = start + count;

  return reply;
}

/**
 * @brief Case-insensitive name+description search over the merged tool catalog, sorted by
 *        name so paging stays coherent across the rosters; dual-registered names dedupe on
 *        first match (curated description wins, registry-only text can still surface it).
 */
QJsonObject searchCommands(const QString& query, int offset, int limit)
{
  const QString needle = query.trimmed();
  if (needle.isEmpty()) {
    QJsonObject reply;
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("missing_query");
    reply[QStringLiteral("hint")] =
      QStringLiteral("query cannot be empty; use meta.listCommands to enumerate the catalog.");
    return reply;
  }

  struct Match {
    QString name;
    QString description;
  };

  QVector<Match> matches;
  QSet<QString> seen;
  auto consider = [&matches, &seen, &needle](const QString& name, const QString& description) {
    if (seen.contains(name))
      return;

    if (!name.contains(needle, Qt::CaseInsensitive)
        && !description.contains(needle, Qt::CaseInsensitive))
      return;

    seen.insert(name);
    matches.append({name, description});
  };
  for (const auto& def : assistantToolDefs())
    consider(def.name, def.description);

  for (const auto& def : fsToolDefs())
    consider(def.name, def.description);

  for (const auto& def : metaToolRoster())
    consider(def.name, def.description);

  static auto& apiRegistry = API::CommandRegistry::instance();
  static auto& aiReg       = AI::CommandRegistry::instance();
  const auto& commands     = apiRegistry.commands();
  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
    if (aiReg.safetyOf(it.value().name) == Safety::Blocked)
      continue;

    consider(it.value().name, it.value().description);
  }

  std::sort(
    matches.begin(), matches.end(), [](const Match& a, const Match& b) { return a.name < b.name; });

  constexpr int kDefaultLimit = 25;
  constexpr int kMaxLimit     = 100;
  const int total             = matches.size();
  const int effective         = qBound(1, limit > 0 ? limit : kDefaultLimit, kMaxLimit);
  const int start             = qBound(0, offset, total);
  const int count             = qMin(effective, total - start);

  QJsonArray rows;
  for (int i = start; i < start + count; ++i) {
    const auto& match = matches.at(i);
    QJsonObject row;
    row[QStringLiteral("name")]    = match.name;
    row[QStringLiteral("family")]  = match.name.section(QLatin1Char('.'), 0, 0);
    row[QStringLiteral("snippet")] = match.description.left(120);
    rows.append(row);
  }

  QJsonObject reply;
  reply[QStringLiteral("ok")]         = true;
  reply[QStringLiteral("query")]      = query;
  reply[QStringLiteral("matchCount")] = total;
  reply[QStringLiteral("count")]      = rows.size();
  reply[QStringLiteral("rows")]       = rows;
  if (start + count < total)
    reply[QStringLiteral("nextOffset")] = start + count;

  reply[QStringLiteral("hint")] =
    QStringLiteral("Call meta.describeCommand{name} on a row to get its full input schema.");

  return reply;
}

/**
 * @brief Returns the curated one-line description for each top-level command scope.
 */
static const QHash<QString, QString>& scopeDescriptions()
{
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
    {QStringLiteral("csvExport"),
     QStringLiteral("CSV export to file.")},
    {QStringLiteral("csvPlayer"),
     QStringLiteral("CSV file replay: open/close, paused state, step, seek.")},
    {QStringLiteral("mdf4Export"),
     QStringLiteral("MDF4 export to file (Pro).")},
    {QStringLiteral("mdf4Player"),
     QStringLiteral("MDF4 file replay (Pro).")},
    {QStringLiteral("controlScript"),
     QStringLiteral("Project control script (setup/loop automation): get/set code, "
                    "dry-run, runtime status.")},
    {QStringLiteral("scripts"),
     QStringLiteral("Bundled reference scripts: list and fetch parser/transform/"
                    "painter/output examples.")},
    {QStringLiteral("dashboard"),
     QStringLiteral("Live dashboard data, FPS, point limits, operation mode.")},
    {QStringLiteral("ui"),
     QStringLiteral("Window state, layouts, widget settings, active group.")},
    {QStringLiteral("sessions"),
     QStringLiteral("Historian export (Pro).")},
    {QStringLiteral("licensing"),
     QStringLiteral("License activation, validation, trial mode.")},
    {QStringLiteral("notifications"),
     QStringLiteral("In-app notification center: post, list, channels, mark read.")},
    {QStringLiteral("problems"),
     QStringLiteral("Standing diagnostics: project-schema mistakes, link problems and script "
                    "failures, with the cause, a remedy and the entity to fix.")},
    {QStringLiteral("diagnostics"),
     QStringLiteral("Connection self-checks: serial-port permissions and the exact command that "
                    "fixes them, Bluetooth adapter and permission state, audio inputs, and host "
                    "or broker reachability. Findings are read through problems.list.")},
    {QStringLiteral("extensions"),
     QStringLiteral("Plugin/extension lifecycle.")},
    {QStringLiteral("meta"),
     QStringLiteral("Discovery: list categories, list commands by prefix, describe one "
                    "command, fetch help/scripting docs/recipes, execute by name.")},
    {QStringLiteral("assistant"),
     QStringLiteral("High-level assistant rails: compact project snapshots, dataset/workspace "
                    "resolvers, workspace planning, and safe workspace tile orchestration.")},
    {QStringLiteral("fs"),
     QStringLiteral("Sandboxed filesystem: read/list/search anything in the workspace folder "
                    "and dragged-in paths; write/append/delete only inside the 'AI/' "
                    "subfolder.")},
  };
  // clang-format on
  return kDescriptions;
}

/**
 * @brief Returns the top-level scope namespaces with descriptions.
 */
QJsonObject listCategories()
{
  const auto& kDescriptions = scopeDescriptions();
  static auto& apiRegistry  = API::CommandRegistry::instance();
  const auto& commands      = apiRegistry.commands();
  static auto& aiReg        = AI::CommandRegistry::instance();

  QHash<QString, int> counts;
  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
    if (aiReg.safetyOf(it.value().name) == Safety::Blocked)
      continue;

    const QString scope  = it.value().name.section(QLatin1Char('.'), 0, 0);
    counts[scope]       += 1;
  }

  counts[QStringLiteral("meta")]      = metaToolRoster().size();
  counts[QStringLiteral("assistant")] = assistantToolDefs().size();
  counts[QStringLiteral("fs")]        = fsToolDefs().size();

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
 * @brief Folds a provider-sanitized tool name (dots/colons as '_') back to its canonical form.
 */
QString canonicalToolName(const QString& name)
{
  if (isAssistantTool(name) || isFsTool(name))
    return name;

  static auto& apiRegistry = API::CommandRegistry::instance();
  if (apiRegistry.hasCommand(name))
    return name;

  static const auto kReverse = []() {
    QHash<QString, QString> map;
    const auto add = [&map](const QString& canonical) {
      QString key = canonical;
      key.replace(QChar('.'), QChar('_'));
      key.replace(QChar(':'), QChar('_'));
      map.insert(key, canonical);
    };
    for (const auto& def : assistantToolDefs())
      add(def.name);

    for (const auto& def : fsToolDefs())
      add(def.name);

    static auto& registry = API::CommandRegistry::instance();
    const auto& commands  = registry.commands();
    for (auto it = commands.constBegin(); it != commands.constEnd(); ++it)
      add(it.value().name);

    return map;
  }();

  const auto it = kReverse.constFind(name);
  if (it != kReverse.constEnd())
    return it.value();

  if (name.startsWith(QStringLiteral("meta_")))
    return QStringLiteral("meta.") + name.mid(5);

  return name;
}

/**
 * @brief Returns the metadata block for a single command, or an empty object.
 */
QJsonObject describeCommand(const QString& requestedName)
{
  const QString name = canonicalToolName(requestedName);
  if (isAssistantTool(name))
    return assistantToolDescription(name);

  if (isFsTool(name))
    return fsToolDescription(name);

  static auto& apiRegistry = API::CommandRegistry::instance();
  const auto& commands     = apiRegistry.commands();
  const auto it            = commands.constFind(name);
  if (it == commands.constEnd())
    return {};

  static auto& aiReg = AI::CommandRegistry::instance();
  if (aiReg.safetyOf(name) == Safety::Blocked)
    return {};

  QJsonObject desc;
  desc[QStringLiteral("name")]        = it.value().name;
  desc[QStringLiteral("description")] = it.value().description;
  desc[QStringLiteral("inputSchema")] = it.value().inputSchema;
  return desc;
}

}  // namespace AI::ToolDetail
