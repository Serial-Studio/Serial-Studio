/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/ProblemsHandler.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>
#include <QVariantMap>

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "Misc/ProblemCenter.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kHandlerMaxFindings = 200;
constexpr int kDefaultFindings    = 50;

//--------------------------------------------------------------------------------------------------
// Serialization helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps a finding severity onto the lowercase token the API speaks.
 */
static QString severityName(int severity)
{
  if (severity == Misc::ProblemCenter::Error)
    return QStringLiteral("error");

  if (severity == Misc::ProblemCenter::Warning)
    return QStringLiteral("warning");

  return QStringLiteral("info");
}

/**
 * @brief Parses a severity token into its level, returning -1 when the token is not a severity.
 */
static int severityFromName(const QString& name)
{
  if (name.compare(QStringLiteral("info"), Qt::CaseInsensitive) == 0)
    return Misc::ProblemCenter::Info;

  if (name.compare(QStringLiteral("warning"), Qt::CaseInsensitive) == 0)
    return Misc::ProblemCenter::Warning;

  if (name.compare(QStringLiteral("error"), Qt::CaseInsensitive) == 0)
    return Misc::ProblemCenter::Error;

  return -1;
}

/**
 * @brief Lowers one finding into the JSON row returned by problems.list and problems.run.
 */
static QJsonObject findingToJson(const Misc::ProblemCenter::Finding& finding)
{
  QJsonObject row;
  row[QStringLiteral("severity")]       = severityName(finding.severity);
  row[QStringLiteral("checkerId")]      = finding.checkerId;
  row[QStringLiteral("code")]           = finding.code;
  row[QStringLiteral("title")]          = finding.title;
  row[QStringLiteral("explanation")]    = finding.explanation;
  row[QStringLiteral("remedy")]         = finding.remedy;
  row[QStringLiteral("entityUniqueId")] = finding.entityUniqueId;
  row[QStringLiteral("jump")]           = finding.jump;
  return row;
}

/**
 * @brief Builds the shared problems.list / problems.run payload, filtered and windowed.
 */
static QJsonObject buildFindingsResult(int severity, const QString& checker_id, int limit)
{
  static const auto& center = Misc::ProblemCenter::instance();

  const auto& findings = center.findings();

  int matched = 0;
  QJsonArray rows;
  for (const auto& finding : findings) {
    if (severity >= 0 && static_cast<int>(finding.severity) != severity)
      continue;

    if (!checker_id.isEmpty() && finding.checkerId != checker_id)
      continue;

    ++matched;
    if (rows.size() < limit)
      rows.append(findingToJson(finding));
  }

  QJsonObject counts;
  counts[QStringLiteral("info")]    = center.infoCount();
  counts[QStringLiteral("error")]   = center.errorCount();
  counts[QStringLiteral("warning")] = center.warningCount();

  QJsonObject result;
  result[QStringLiteral("findings")]   = rows;
  result[QStringLiteral("counts")]     = counts;
  result[QStringLiteral("total")]      = center.totalCount();
  result[QStringLiteral("matchCount")] = matched;
  result[QStringLiteral("lastRun")]    = center.lastRunTime();
  result[QStringLiteral("hint")] =
    QStringLiteral("Each finding has {severity, checkerId, code, title, explanation, remedy, "
                   "entityUniqueId, jump}. entityUniqueId >= 0 names the project entity to fix "
                   "-- read it with project.dataset.getByUniqueId or project.group.get. Call "
                   "problems.run to refresh after an edit.");
  return result;
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires problems.list, problems.run and problems.listCheckers into CommandRegistry.
 */
void API::Handlers::ProblemsHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();
  const auto empty      = emptySchema();

  registry.registerCommand(
    QStringLiteral("problems.list"),
    QStringLiteral("List the diagnostics Serial Studio currently reports about the loaded "
                   "project, the live link and the parser/transform scripts. THE first call "
                   "when a dashboard is blank, a widget stays empty or values look wrong: each "
                   "finding names the concrete cause, a remedy and the entity to fix. Optional "
                   "params: severity (info/warning/error), checkerId, limit."),
    makeSchema(
      {
  },
      {{QStringLiteral("severity"),
        QStringLiteral("string"),
        QStringLiteral("Only findings at this severity: info, warning or error.")},
       {QStringLiteral("checkerId"),
        QStringLiteral("string"),
        QStringLiteral("Only findings produced by this checker; see problems.listCheckers.")},
       {QStringLiteral("limit"),
        QStringLiteral("integer"),
        QStringLiteral("Max findings to return (default 50, max 200).")}}),
    &list);

  registry.registerCommand(
    QStringLiteral("problems.run"),
    QStringLiteral("Re-run every registered diagnostic checker and return the refreshed list "
                   "in the same shape as problems.list. Read-only: checkers inspect the "
                   "project, the link counters and the script engines, and never modify them. "
                   "Use it after an edit to confirm a finding actually cleared."),
    empty,
    &run);

  registry.registerCommand(
    QStringLiteral("problems.listCheckers"),
    QStringLiteral("List the registered diagnostic checkers and the triggers each one answers "
                   "to (projectChanged, linkSample, onDemand), so you can see what "
                   "problems.run covers and filter problems.list by checkerId."),
    empty,
    &listCheckers);
}

//--------------------------------------------------------------------------------------------------
// Command implementations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current findings, optionally narrowed by severity and checker.
 */
API::CommandResponse API::Handlers::ProblemsHandler::list(const QString& id,
                                                          const QJsonObject& params)
{
  int severity           = -1;
  const auto severityStr = params.value(QStringLiteral("severity")).toString();
  if (!severityStr.isEmpty()) {
    severity = severityFromName(severityStr);
    if (severity < 0)
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Unknown severity: %1. Valid values: info, warning, error.")
          .arg(severityStr));
  }

  int limit = params.value(QStringLiteral("limit")).toInt(kDefaultFindings);
  if (limit <= 0)
    limit = kDefaultFindings;

  limit = qMin(limit, kHandlerMaxFindings);

  const auto checkerId = params.value(QStringLiteral("checkerId")).toString();
  return CommandResponse::makeSuccess(id, buildFindingsResult(severity, checkerId, limit));
}

/**
 * @brief Re-runs every checker and returns the refreshed list.
 */
API::CommandResponse API::Handlers::ProblemsHandler::run(const QString& id,
                                                         const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& center = Misc::ProblemCenter::instance();

  center.runNow();
  return CommandResponse::makeSuccess(id, buildFindingsResult(-1, QString(), kDefaultFindings));
}

/**
 * @brief Returns the registered checkers and the triggers they answer to.
 */
API::CommandResponse API::Handlers::ProblemsHandler::listCheckers(const QString& id,
                                                                  const QJsonObject& params)
{
  Q_UNUSED(params)

  static const auto& center = Misc::ProblemCenter::instance();

  QJsonArray rows;
  const auto catalog = center.checkerCatalog();
  for (const auto& entry : catalog) {
    const auto map        = entry.toMap();
    const auto triggerIds = map.value(QStringLiteral("triggers")).toStringList();

    QJsonArray triggers;
    for (const auto& trigger : triggerIds)
      triggers.append(trigger);

    QJsonObject row;
    row[QStringLiteral("id")]       = map.value(QStringLiteral("id")).toString();
    row[QStringLiteral("triggers")] = triggers;
    rows.append(row);
  }

  QJsonObject result;
  result[QStringLiteral("checkers")] = rows;
  result[QStringLiteral("total")]    = rows.size();
  result[QStringLiteral("hint")] =
    QStringLiteral("projectChanged runs on project load/edit/save, linkSample on the shared "
                   "1 Hz tick, onDemand only via problems.run.");
  return CommandResponse::makeSuccess(id, result);
}
