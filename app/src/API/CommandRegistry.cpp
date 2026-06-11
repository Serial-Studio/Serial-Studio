/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include "API/CommandRegistry.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QSet>

#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "Misc/BackupManager.h"

//--------------------------------------------------------------------------------------------------
// Closest-match suggestion helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Levenshtein distance between two strings, capped for speed.
 */
static int editDistance(const QString& a, const QString& b)
{
  const int la = a.size();
  const int lb = b.size();
  if (la == 0)
    return lb;

  if (lb == 0)
    return la;

  QVector<int> prev(lb + 1);
  QVector<int> curr(lb + 1);
  for (int j = 0; j <= lb; ++j)
    prev[j] = j;

  for (int i = 1; i <= la; ++i) {
    curr[0] = i;
    for (int j = 1; j <= lb; ++j) {
      const int cost = (a.at(i - 1).toLower() == b.at(j - 1).toLower()) ? 0 : 1;
      curr[j]        = std::min({curr[j - 1] + 1, prev[j] + 1, prev[j - 1] + cost});
    }
    std::swap(prev, curr);
  }
  return prev[lb];
}

/**
 * @brief Lower-is-better score: prefers names sharing the dotted prefix.
 */
static int similarityScore(const QString& want, const QString& have)
{
  const auto wantParts = want.split(QLatin1Char('.'));
  const auto haveParts = have.split(QLatin1Char('.'));

  int sharedSegments = 0;
  const int minSeg   = std::min(wantParts.size(), haveParts.size());
  for (int i = 0; i < minSeg; ++i)
    if (wantParts[i].compare(haveParts[i], Qt::CaseInsensitive) == 0)
      sharedSegments += 1;
    else
      break;

  const int dist = editDistance(want, have);

  return dist - (sharedSegments * 6);
}

//--------------------------------------------------------------------------------------------------
// Singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Gets the singleton instance of the CommandRegistry
 */
API::CommandRegistry& API::CommandRegistry::instance()
{
  static CommandRegistry singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register a new command handler
 */
void API::CommandRegistry::registerCommand(const QString& name,
                                           const QString& description,
                                           CommandFunction handler)
{
  CommandDefinition def;
  def.name        = name;
  def.description = description;
  def.handler     = std::move(handler);
  m_commands.insert(name, def);
}

/**
 * @brief Register a new command handler with a JSON Schema for MCP tool metadata
 */
void API::CommandRegistry::registerCommand(const QString& name,
                                           const QString& description,
                                           const QJsonObject& inputSchema,
                                           CommandFunction handler)
{
  CommandDefinition def;
  def.name        = name;
  def.description = description;
  def.inputSchema = inputSchema;
  def.handler     = std::move(handler);
  m_commands.insert(name, def);
}

//--------------------------------------------------------------------------------------------------
// Command lookup & execution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Check if a command is registered
 */
bool API::CommandRegistry::hasCommand(const QString& name) const
{
  return m_commands.contains(name);
}

/**
 * @brief Closed set of mutating commands that warrant a pre-mutation snapshot.
 */
static const QSet<QString>& destructiveCommandSet()
{
  static const QSet<QString> kSet = {
    QStringLiteral("project.new"),
    QStringLiteral("project.open"),
    QStringLiteral("project.loadJson"),
    QStringLiteral("project.template.apply"),
    QStringLiteral("project.dataset.delete"),
    QStringLiteral("project.group.delete"),
    QStringLiteral("project.action.delete"),
    QStringLiteral("project.outputWidget.delete"),
    QStringLiteral("project.dataset.move"),
    QStringLiteral("project.group.move"),
    QStringLiteral("project.workspace.delete"),
    QStringLiteral("project.workspace.clearAll"),
    QStringLiteral("project.workspace.removeWidget"),
    QStringLiteral("project.workspace.reorder"),
    QStringLiteral("project.batch"),
    QStringLiteral("assistant.project.bulkApply"),
    QStringLiteral("assistant.workspace.addTile"),
    QStringLiteral("assistant.restore"),
  };
  return kSet;
}

/**
 * @brief Queues a single FrameBuilder re-sync on the event loop so the live dashboard
 *        picks up programmatic project edits; a burst of mutating commands (project.batch,
 *        rapid tool calls) collapses into one apply.
 */
static void scheduleProjectApply()
{
  static bool pending = false;
  if (pending)
    return;

  pending = true;
  QMetaObject::invokeMethod(
    QCoreApplication::instance(),
    [] {
      pending = false;
      DataModel::FrameBuilder::instance().syncFromProjectModel();
    },
    Qt::QueuedConnection);
}

/**
 * @brief Execute a registered command
 */
API::CommandResponse API::CommandRegistry::execute(const QString& name,
                                                   const QString& id,
                                                   const QJsonObject& params)
{
  if (!hasCommand(name))
    return buildUnknownCommandResponse(name, id);

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);
  QString preMutationBackup;
  if (!isDryRun && destructiveCommandSet().contains(name))
    preMutationBackup = Misc::BackupManager::instance().snapshot(QStringLiteral("pre-") + name);

  const qint64 epochBefore = DataModel::ProjectModel::instance().mutationEpoch();

  try {
    auto response = m_commands[name].handler(id, params);
    attachErrorMetadata(name, response);

    if (!isDryRun && response.success
        && DataModel::ProjectModel::instance().mutationEpoch() != epochBefore) {
      DataModel::ProjectModel::instance().scheduleAutoSave();
      scheduleProjectApply();
    }

    if (!preMutationBackup.isEmpty() && response.success
        && !response.result.contains(QStringLiteral("backupPath"))) {
      auto result                          = response.result;
      result[QStringLiteral("backupPath")] = preMutationBackup;
      response.result                      = result;
    }

    return response;
  } catch (const std::exception& e) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Command execution failed: %1").arg(e.what()));
  } catch (...) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Command execution failed: unknown exception"));
  }
}

/**
 * @brief Builds a "Unknown command" error response with did_you_mean suggestions.
 */
API::CommandResponse API::CommandRegistry::buildUnknownCommandResponse(const QString& name,
                                                                       const QString& id) const
{
  QVector<QPair<int, QString>> ranked;
  ranked.reserve(m_commands.size());
  for (auto it = m_commands.cbegin(); it != m_commands.cend(); ++it)
    ranked.append({similarityScore(name, it.key()), it.key()});

  std::sort(
    ranked.begin(), ranked.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

  QJsonArray suggestions;
  const int kMax = std::min<int>(5, ranked.size());
  for (int i = 0; i < kMax; ++i) {
    QJsonObject hint;
    hint[QStringLiteral("name")]        = ranked[i].second;
    hint[QStringLiteral("description")] = m_commands[ranked[i].second].description;
    suggestions.append(hint);
  }

  QJsonObject data;
  data[QStringLiteral("did_you_mean")] = suggestions;
  data[QStringLiteral("hint")] =
    QStringLiteral("Use meta.listCommands to browse the full surface, then meta.describeCommand "
                   "for the exact schema before invoking.");

  return CommandResponse::makeError(
    id, ErrorCode::UnknownCommand, QStringLiteral("Unknown command: %1").arg(name), data);
}

/**
 * @brief Attaches inputSchema + structured category to a failed response.
 */
void API::CommandRegistry::attachErrorMetadata(const QString& name, CommandResponse& response) const
{
  if (response.success)
    return;

  const bool isValidation =
    response.errorCode == ErrorCode::MissingParam || response.errorCode == ErrorCode::InvalidParam;
  if (isValidation && !m_commands[name].inputSchema.isEmpty()) {
    auto data = response.errorData;
    if (!data.contains(QStringLiteral("inputSchema")))
      data[QStringLiteral("inputSchema")] = m_commands[name].inputSchema;

    if (!data.contains(QStringLiteral("commandName")))
      data[QStringLiteral("commandName")] = name;

    response.errorData = data;
  }

  if (response.errorData.contains(QStringLiteral("category")))
    return;

  auto data                        = response.errorData;
  const auto category              = classifyErrorCategory(name, response);
  data[QStringLiteral("category")] = category;

  if (category == QStringLiteral("script_compile_failed")
      && !data.contains(QStringLiteral("dryRunHint"))) {
    const auto hint = dryRunHintForScriptCommand(name);
    if (!hint.isEmpty())
      data[QStringLiteral("dryRunHint")] = hint;
  }

  response.errorData = data;
}

/**
 * @brief Returns a structured error category for a failed CommandResponse.
 */
QString API::CommandRegistry::classifyErrorCategory(const QString& commandName,
                                                    const CommandResponse& response)
{
  static const QStringList kHardwareWriteCommands = {
    QStringLiteral("io.writeData"),
    QStringLiteral("console.send"),
    QStringLiteral("io.connect"),
    QStringLiteral("io.disconnect"),
  };

  const auto msgLower = response.errorMessage.toLower();
  if (kHardwareWriteCommands.contains(commandName)
      && (msgLower.contains(QStringLiteral("denied"))
          || msgLower.contains(QStringLiteral("blocked"))
          || msgLower.contains(QStringLiteral("not allowed"))
          || msgLower.contains(QStringLiteral("requires user"))))
    return QStringLiteral("hardware_write_blocked");

  if (response.errorCode == ErrorCode::MissingParam || response.errorCode == ErrorCode::InvalidParam
      || response.errorCode == ErrorCode::InvalidJson)
    return QStringLiteral("validation_failed");

  if (response.errorCode == ErrorCode::UnknownCommand)
    return QStringLiteral("unknown_command");

  if (msgLower.contains(QStringLiteral("commercial"))
      || msgLower.contains(QStringLiteral("pro license"))
      || msgLower.contains(QStringLiteral("requires a pro")))
    return QStringLiteral("license_required");

  if (msgLower.contains(QStringLiteral("not connected"))
      || msgLower.contains(QStringLiteral("connection lost"))
      || msgLower.contains(QStringLiteral("device not")))
    return QStringLiteral("connection_lost");

  if (msgLower.contains(QStringLiteral("compile")) || msgLower.contains(QStringLiteral("syntax"))
      || msgLower.contains(QStringLiteral("script error"))
      || msgLower.contains(QStringLiteral("define parse"))
      || msgLower.contains(QStringLiteral("define paint"))
      || msgLower.contains(QStringLiteral("define transform")))
    return QStringLiteral("script_compile_failed");

  if (msgLower.contains(QStringLiteral("busy")) || msgLower.contains(QStringLiteral("in progress"))
      || msgLower.contains(QStringLiteral("already running")))
    return QStringLiteral("bus_busy");

  if (msgLower.contains(QStringLiteral("no such file"))
      || msgLower.contains(QStringLiteral("not found"))
      || msgLower.contains(QStringLiteral("does not exist")))
    return QStringLiteral("file_not_found");

  if (msgLower.contains(QStringLiteral("permission")) || msgLower.contains(QStringLiteral("denied"))
      || msgLower.contains(QStringLiteral("not allowed"))
      || msgLower.contains(QStringLiteral("blocked")))
    return QStringLiteral("permission_denied");

  return QStringLiteral("execution_error");
}

/**
 * @brief Maps a failing script-setter command name to the matching dryRun hint.
 */
QString API::CommandRegistry::dryRunHintForScriptCommand(const QString& commandName)
{
  if (commandName == QStringLiteral("project.painter.setCode"))
    return QStringLiteral("Re-validate with project.painter.dryRun{code} before setCode. "
                          "Painter scripts are JavaScript-only. Fetch the API surface "
                          "with meta.fetchScriptingDocs{kind:'painter_js'}.");

  if (commandName == QStringLiteral("project.frameParser.setCode"))
    return QStringLiteral("Re-validate with project.frameParser.dryRun{code, language, "
                          "inputBytesHex, decoderMethod, frameDetection, frameStart, "
                          "frameEnd} -- or project.frameParser.dryCompile for a syntax-only "
                          "check -- before setCode. dryRun drives the full pipeline "
                          "(extraction + decoder + parser) so you see what the live "
                          "FrameBuilder would actually emit. Mismatched language is the most "
                          "common silent compile failure. Fetch the API surface with "
                          "meta.fetchScriptingDocs{kind:'frame_parser_lua'} or "
                          "{kind:'frame_parser_js'}.");

  if (commandName == QStringLiteral("project.dataset.setTransformCode")
      || commandName == QStringLiteral("project.dataset.update"))
    return QStringLiteral("Re-validate with project.dataset.transform.dryRun{code, "
                          "language, values:[...]} before setting. Common mismatch: code "
                          "is Lua but transformLanguage is 0 (JavaScript), or vice versa. "
                          "Fetch the API surface with "
                          "meta.fetchScriptingDocs{kind:'transform_lua'} or "
                          "{kind:'transform_js'}.");

  if (commandName == QStringLiteral("project.outputWidget.update"))
    return QStringLiteral("Output widget transmitFunction is JavaScript-only. Fetch the "
                          "API surface and protocol helpers with "
                          "meta.fetchScriptingDocs{kind:'output_widget_js'} -- there is no "
                          "dedicated dryRun for transmitFunction yet; iterate the code in "
                          "the Transmit Test dialog or temporarily project.outputWidget."
                          "duplicate the widget to test in isolation.");

  return QString();
}

//--------------------------------------------------------------------------------------------------
// Command metadata
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get a sorted list of all available command names
 */
QStringList API::CommandRegistry::availableCommands() const
{
  QStringList names = m_commands.keys();
  names.sort();
  return names;
}

/**
 * @brief Get direct access to all command definitions
 */
const QMap<QString, API::CommandDefinition>& API::CommandRegistry::commands() const
{
  return m_commands;
}
