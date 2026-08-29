/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Conversation/MetaToolRunner.h"

#include <QJsonArray>

#include "AI/ContextBuilder.h"
#include "AI/Conversation/HelpFetcher.h"
#include "AI/ToolDispatcher.h"

//--------------------------------------------------------------------------------------------------
// Construction / wiring
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the runner to its facade sink and to the help fetcher it drives; the
 *        dispatcher arrives later because the conversation is built before it exists.
 */
AI::MetaToolRunner::MetaToolRunner(MetaToolSink& sink, HelpFetcher& helpFetcher)
  : m_sink(sink), m_helpFetcher(helpFetcher), m_dispatcher(nullptr)
{}

/**
 * @brief Sets the tool dispatcher. The runner does not take ownership.
 */
void AI::MetaToolRunner::setDispatcher(ToolDispatcher* dispatcher)
{
  m_dispatcher = dispatcher;
}

//--------------------------------------------------------------------------------------------------
// Dispatch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Auto-handles meta-tool calls; returns true when consumed.
 */
bool AI::MetaToolRunner::dispatch(const QString& callId,
                                  const QString& name,
                                  const QJsonObject& arguments)
{
  if (name == QStringLiteral("meta.listCategories")) {
    runListCategories(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.snapshot")) {
    runSnapshot(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.listCommands")) {
    runListCommands(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.describeCommand")) {
    runDescribe(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.executeCommand")) {
    runExecuteCommand(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.fetchHelp")) {
    runFetchHelp(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.fetchScriptingDocs")) {
    runScriptingDocs(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.howTo")) {
    runHowTo(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.loadSkill")) {
    runLoadSkill(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.searchDocs")) {
    runSearchDocs(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.search")) {
    runSearch(callId, name, arguments);
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Catalog handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief meta.listCategories: returns the dispatcher's category list.
 */
void AI::MetaToolRunner::runListCategories(const QString& callId,
                                           const QString& name,
                                           const QJsonObject& arguments)
{
  m_sink.appendToolCallCard(callId, name, arguments, ToolCallStatus::Running);
  const auto reply = m_dispatcher->listCategories();
  m_sink.recordToolResult(callId, name, reply);
  m_sink.updateToolCallCard(callId, ToolCallStatus::Done, reply);
  m_sink.releaseOutstandingToolResult();
}

/**
 * @brief meta.snapshot: returns the dispatcher's current state snapshot.
 */
void AI::MetaToolRunner::runSnapshot(const QString& callId,
                                     const QString& name,
                                     const QJsonObject& arguments)
{
  m_sink.appendToolCallCard(callId, name, arguments, ToolCallStatus::Running);
  QJsonObject reply;
  reply[QStringLiteral("ok")]       = true;
  reply[QStringLiteral("snapshot")] = m_dispatcher->getSnapshot();
  m_sink.recordToolResult(callId, name, reply);
  m_sink.updateToolCallCard(callId, ToolCallStatus::Done, reply);
  m_sink.releaseOutstandingToolResult();
}

/**
 * @brief meta.listCommands: lists available commands filtered by prefix.
 */
void AI::MetaToolRunner::runListCommands(const QString& callId,
                                         const QString& name,
                                         const QJsonObject& arguments)
{
  m_sink.appendToolCallCard(callId, name, arguments, ToolCallStatus::Running);
  const auto prefix    = arguments.value(QStringLiteral("prefix")).toString();
  const int offset     = arguments.value(QStringLiteral("offset")).toInt(0);
  const int limit      = arguments.value(QStringLiteral("limit")).toInt(0);
  const bool namesOnly = arguments.value(QStringLiteral("namesOnly")).toBool(false);
  const auto reply     = m_dispatcher->listCommands(prefix, offset, limit, namesOnly);
  m_sink.recordToolResult(callId, name, reply);
  m_sink.updateToolCallCard(callId, ToolCallStatus::Done, reply);
  m_sink.releaseOutstandingToolResult();
}

/**
 * @brief meta.search: fuzzy command search, refusing an empty query with a hint that points
 *        at the enumerable catalog.
 */
void AI::MetaToolRunner::runSearch(const QString& callId,
                                   const QString& name,
                                   const QJsonObject& arguments)
{
  m_sink.appendToolCallCard(callId, name, arguments, ToolCallStatus::Running);
  const auto query = arguments.value(QStringLiteral("query")).toString().trimmed();
  QJsonObject reply;
  if (query.isEmpty()) {
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("missing_query");
    reply[QStringLiteral("hint")] =
      QStringLiteral("query cannot be empty; use meta.listCommands to enumerate the catalog.");
  } else {
    reply = m_dispatcher->searchCommands(query,
                                         arguments.value(QStringLiteral("offset")).toInt(0),
                                         arguments.value(QStringLiteral("limit")).toInt(0));
  }
  const bool ok = reply.value(QStringLiteral("ok")).toBool();
  m_sink.recordToolResult(callId, name, reply);
  m_sink.updateToolCallCard(callId, ok ? ToolCallStatus::Done : ToolCallStatus::Error, reply);
  m_sink.releaseOutstandingToolResult();
}

/**
 * @brief meta.executeCommand: dispatches the inner tool with the same safety policy.
 */
void AI::MetaToolRunner::runExecuteCommand(const QString& callId,
                                           const QString& name,
                                           const QJsonObject& arguments)
{
  const auto target    = arguments.value(QStringLiteral("name")).toString();
  const auto innerArgs = arguments.value(QStringLiteral("arguments")).toObject();

  if (target.isEmpty()) {
    QJsonObject err;
    err[QStringLiteral("ok")]    = false;
    err[QStringLiteral("error")] = QStringLiteral("missing_name");
    m_sink.appendToolCallCard(callId, name, arguments, ToolCallStatus::Error);
    m_sink.recordToolResult(callId, name, err);
    m_sink.updateToolCallCard(callId, ToolCallStatus::Error, err);
    m_sink.releaseOutstandingToolResult();
    return;
  }

  m_sink.dispatchByCallSafety(callId, target, innerArgs);
}

//--------------------------------------------------------------------------------------------------
// Documentation handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the skill id whose body documents @a commandName, or empty.
 */
static QString skillForCommand(const QString& commandName)
{
  if (commandName.startsWith(QStringLiteral("project.workspace."))
      || commandName == QStringLiteral("project.dataset.setOption")
      || commandName == QStringLiteral("project.dataset.setOptions"))
    return QStringLiteral("dashboard_layout");

  if (commandName.startsWith(QStringLiteral("project.frameParser.")))
    return QStringLiteral("frame_parsers");

  if (commandName.startsWith(QStringLiteral("project.painter.")))
    return QStringLiteral("painter");

  if (commandName.startsWith(QStringLiteral("project.outputWidget.")))
    return QStringLiteral("output_widgets");

  if (commandName == QStringLiteral("project.dataset.setTransformCode")
      || commandName == QStringLiteral("project.dataset.transform.dryRun")
      || commandName.startsWith(QStringLiteral("project.dataTable.")))
    return QStringLiteral("transforms");

  if (commandName.startsWith(QStringLiteral("project.mqtt.")))
    return QStringLiteral("mqtt");

  if (commandName.startsWith(QStringLiteral("io.canbus."))
      || commandName.startsWith(QStringLiteral("io.modbus.")))
    return QStringLiteral("can_modbus");

  if (commandName.startsWith(QStringLiteral("project.")))
    return QStringLiteral("project_basics");

  return QString();
}

/**
 * @brief meta.describeCommand handler: returns command schema or not_found.
 */
void AI::MetaToolRunner::runDescribe(const QString& callId,
                                     const QString& name,
                                     const QJsonObject& arguments)
{
  m_sink.appendToolCallCard(callId, name, arguments, ToolCallStatus::Running);
  const auto target = arguments.value(QStringLiteral("name")).toString();
  QJsonObject reply;
  if (target.isEmpty()) {
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("missing_name");
  } else {
    const auto desc = m_dispatcher->describeCommand(target);
    if (desc.isEmpty()) {
      reply[QStringLiteral("ok")]    = false;
      reply[QStringLiteral("error")] = QStringLiteral("not_found");
      reply[QStringLiteral("name")]  = target;
    } else {
      reply[QStringLiteral("ok")]      = true;
      reply[QStringLiteral("command")] = desc;
      const auto skill                 = skillForCommand(target);
      if (!skill.isEmpty())
        reply[QStringLiteral("loadSkillFirst")] = skill;
    }
  }
  m_sink.recordToolResult(callId, name, reply);
  m_sink.updateToolCallCard(callId,
                            reply.value(QStringLiteral("ok")).toBool() ? ToolCallStatus::Done
                                                                       : ToolCallStatus::Error,
                            reply);
  m_sink.releaseOutstandingToolResult();
}

/**
 * @brief meta.fetchHelp handler: hands the URL-checked fetch to the help fetcher, which
 *        reports back asynchronously and releases the outstanding result there.
 */
void AI::MetaToolRunner::runFetchHelp(const QString& callId,
                                      const QString& name,
                                      const QJsonObject& arguments)
{
  const auto path = arguments.value(QStringLiteral("path")).toString();
  m_sink.appendToolCallCard(callId, name, arguments, ToolCallStatus::Running);
  m_helpFetcher.fetchPage(callId, path);
}

/**
 * @brief meta.fetchScriptingDocs handler: returns the canonical doc body for a kind.
 */
void AI::MetaToolRunner::runScriptingDocs(const QString& callId,
                                          const QString& name,
                                          const QJsonObject& arguments)
{
  m_sink.appendToolCallCard(callId, name, arguments, ToolCallStatus::Running);
  const auto kind = arguments.value(QStringLiteral("kind")).toString();
  const auto body = ContextBuilder::scriptingDocFor(kind);

  QJsonObject result;
  if (body.isEmpty()) {
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("error")] =
      QStringLiteral("Unknown kind '%1'. Valid: frame_parser_js, "
                     "frame_parser_lua, transform_js, transform_lua, "
                     "output_widget_js, painter_js, control_script_js, "
                     "sdk_js, sdk_lua.")
        .arg(kind);
    m_sink.updateToolCallCard(callId, ToolCallStatus::Error, result);
  } else {
    result[QStringLiteral("ok")]      = true;
    result[QStringLiteral("kind")]    = kind;
    result[QStringLiteral("content")] = body;
    m_sink.updateToolCallCard(callId, ToolCallStatus::Done, result);
  }

  m_sink.recordToolResult(callId, name, result);
  m_sink.releaseOutstandingToolResult();
}

/**
 * @brief meta.howTo handler: returns a canned step-by-step recipe by task id.
 */
void AI::MetaToolRunner::runHowTo(const QString& callId,
                                  const QString& name,
                                  const QJsonObject& arguments)
{
  m_sink.appendToolCallCard(callId, name, arguments, ToolCallStatus::Running);
  const auto task   = arguments.value(QStringLiteral("task")).toString();
  const auto recipe = ContextBuilder::howToRecipe(task);

  QJsonObject result;
  if (recipe.isEmpty()) {
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("error")] =
      QStringLiteral("Unknown task '%1'. Valid tasks: %2")
        .arg(task, ContextBuilder::howToTasks().join(QStringLiteral(", ")));
    m_sink.updateToolCallCard(callId, ToolCallStatus::Error, result);
  } else {
    result[QStringLiteral("ok")]    = true;
    result[QStringLiteral("task")]  = task;
    result[QStringLiteral("steps")] = recipe;
    m_sink.updateToolCallCard(callId, ToolCallStatus::Done, result);
  }

  m_sink.recordToolResult(callId, name, result);
  m_sink.releaseOutstandingToolResult();
}

/**
 * @brief meta.loadSkill: returns the markdown body of a registered skill.
 */
void AI::MetaToolRunner::runLoadSkill(const QString& callId,
                                      const QString& name,
                                      const QJsonObject& arguments)
{
  m_sink.appendToolCallCard(callId, name, arguments, ToolCallStatus::Running);
  const auto skillId = arguments.value(QStringLiteral("name")).toString();
  const auto body    = AI::ContextBuilder::skillBody(skillId);

  QJsonObject reply;
  if (body.isEmpty()) {
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("unknown_skill");
    QJsonArray known;
    for (const auto& s : AI::ContextBuilder::skillIds())
      known.append(s);

    reply[QStringLiteral("availableSkills")] = known;
    m_sink.recordToolResult(callId, name, reply);
    m_sink.updateToolCallCard(callId, ToolCallStatus::Error, reply);
  } else {
    reply[QStringLiteral("ok")]    = true;
    reply[QStringLiteral("skill")] = skillId;
    reply[QStringLiteral("body")]  = body;
    m_sink.noteSkillLoaded(skillId);
    m_sink.recordToolResult(callId, name, reply);
    m_sink.updateToolCallCard(callId, ToolCallStatus::Done, reply);
  }
  m_sink.releaseOutstandingToolResult();
}

/**
 * @brief meta.searchDocs: BM25-style doc search; the corpus is resolved by the facade so
 *        the runner stays free of singletons and the index still loads lazily.
 */
void AI::MetaToolRunner::runSearchDocs(const QString& callId,
                                       const QString& name,
                                       const QJsonObject& arguments)
{
  m_sink.appendToolCallCard(callId, name, arguments, ToolCallStatus::Running);
  const auto query = arguments.value(QStringLiteral("query")).toString();
  const int k      = qBound(1, arguments.value(QStringLiteral("k")).toInt(5), 10);
  const auto hits  = m_sink.searchDocs(query, k);

  QJsonArray rows;
  for (const auto& h : hits) {
    QJsonObject row;
    row[QStringLiteral("id")]     = h.id;
    row[QStringLiteral("source")] = h.source;
    row[QStringLiteral("title")]  = h.title;
    row[QStringLiteral("body")]   = h.body;
    row[QStringLiteral("score")]  = h.score;
    rows.append(row);
  }

  QJsonObject reply;
  reply[QStringLiteral("ok")]    = true;
  reply[QStringLiteral("query")] = query;
  reply[QStringLiteral("hits")]  = rows;
  reply[QStringLiteral("count")] = rows.size();
  if (rows.isEmpty()) {
    reply[QStringLiteral("hint")] =
      QStringLiteral("No matches. Try rephrasing with command-shaped terms (e.g. "
                     "'project.dataset.add' instead of 'add a channel'), or fall back to "
                     "meta.listCommands{prefix} / meta.fetchHelp{path: 'help.json'}.");
  }

  m_sink.recordToolResult(callId, name, reply);
  m_sink.updateToolCallCard(callId, ToolCallStatus::Done, reply);
  m_sink.releaseOutstandingToolResult();
}
