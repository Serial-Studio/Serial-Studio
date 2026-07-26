/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/DiagnosticsHandler.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "Misc/ConnectionDiagnostics.h"

//--------------------------------------------------------------------------------------------------
// Local aliases
//--------------------------------------------------------------------------------------------------

using Misc::ConnectionDiagnostics;
using Misc::Diagnostics::Bus;
using Misc::Diagnostics::busBit;
using Misc::Diagnostics::busFromSlug;
using Misc::Diagnostics::BusMask;
using Misc::Diagnostics::busSlug;
using Misc::Diagnostics::kBusCount;
using Misc::Diagnostics::Result;
using Misc::Diagnostics::scopeCovers;
using Misc::Diagnostics::Verdict;
using Misc::Diagnostics::verdictName;

//--------------------------------------------------------------------------------------------------
// Serialization helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Lists the bus slugs covered by @p scope, in bus order.
 */
[[nodiscard]] static QJsonArray busArray(BusMask scope)
{
  QJsonArray buses;
  for (int i = 0; i < kBusCount; ++i)
    if (scopeCovers(scope, static_cast<Bus>(i)))
      buses.append(busSlug(static_cast<Bus>(i)));

  return buses;
}

/**
 * @brief Returns the comma-separated slugs of every bus this build can check.
 */
[[nodiscard]] static QString supportedSlugs()
{
  QStringList slugs;
  for (int i = 0; i < kBusCount; ++i)
    if (scopeCovers(ConnectionDiagnostics::supportedBuses(), static_cast<Bus>(i)))
      slugs.append(busSlug(static_cast<Bus>(i)));

  return slugs.join(QStringLiteral(", "));
}

/**
 * @brief Lowers one result into the JSON row diagnostics.run returns.
 */
[[nodiscard]] static QJsonObject resultToJson(const Result& result)
{
  QJsonObject row;
  row[QStringLiteral("bus")]         = busSlug(result.bus);
  row[QStringLiteral("verdict")]     = verdictName(result.verdict);
  row[QStringLiteral("code")]        = result.code;
  row[QStringLiteral("title")]       = result.title;
  row[QStringLiteral("explanation")] = result.explanation;
  row[QStringLiteral("remedy")]      = result.remedy;
  return row;
}

/**
 * @brief Counts the standing results of each verdict across @p scope.
 */
[[nodiscard]] static QJsonObject countsFor(BusMask scope)
{
  int info    = 0;
  int warning = 0;
  int failure = 0;

  static auto& diagnostics = ConnectionDiagnostics::instance();

  const auto results = diagnostics.results(scope);
  for (const auto& result : results)
    if (result.verdict == Verdict::Failure)
      ++failure;
    else if (result.verdict == Verdict::Warning)
      ++warning;
    else
      ++info;

  QJsonObject counts;
  counts[QStringLiteral("info")]    = info;
  counts[QStringLiteral("warning")] = warning;
  counts[QStringLiteral("failure")] = failure;
  return counts;
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires diagnostics.run and diagnostics.status into CommandRegistry.
 */
void API::Handlers::DiagnosticsHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("diagnostics.run"),
    QStringLiteral("Check whether this machine is set up to connect: serial-port permissions "
                   "and the exact command that fixes them on Linux, Bluetooth adapter power and "
                   "permission, audio inputs, and whether the configured host or broker "
                   "resolves and accepts a connection. THE first call when a connection fails "
                   "or the Connect button does nothing. Returns immediately with the instant "
                   "results already complete; slower reachability probes finish in the "
                   "background. Optional param: bus."),
    makeSchema(
      {
  },
      {{QStringLiteral("bus"),
        QStringLiteral("string"),
        QStringLiteral("Only check this bus: serial, bluetooth, network, broker or audio. "
                       "Omitted means every bus this build supports.")}}),
    &run);

  registry.registerCommand(
    QStringLiteral("diagnostics.status"),
    QStringLiteral("Report whether a diagnostics run is still probing, when the last run "
                   "finished, and how many results of each verdict stand. Poll this after "
                   "diagnostics.run, then read the findings with problems.list filtered by "
                   "checkerId 'diagnostics.<bus>'."),
    emptySchema(),
    &status);
}

//--------------------------------------------------------------------------------------------------
// Command implementations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts a run and acknowledges immediately with the instant results, following the
 *        sessions.exportToCsv ack-and-poll precedent: the dispatch signature is synchronous, so
 *        blocking it would stall the API connection for the whole run.
 */
API::CommandResponse API::Handlers::DiagnosticsHandler::run(const QString& id,
                                                            const QJsonObject& params)
{
  static auto& diagnostics = ConnectionDiagnostics::instance();

  BusMask scope     = ConnectionDiagnostics::supportedBuses();
  const auto busArg = params.value(QStringLiteral("bus")).toString();
  if (!busArg.isEmpty()) {
    Bus bus = Bus::Serial;
    if (!busFromSlug(busArg, bus) || !scopeCovers(scope, bus))
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Unknown bus: %1. Valid values: %2.").arg(busArg, supportedSlugs()));

    scope = busBit(bus);
  }

  const auto probing = ConnectionDiagnostics::probingBuses(scope);
  const int estimate = ConnectionDiagnostics::estimatedMsec(scope);
  diagnostics.run(scope);

  QJsonArray instant;
  const auto results = diagnostics.results(scope);
  for (const auto& result : results)
    instant.append(resultToJson(result));

  QJsonObject result;
  result[QStringLiteral("started")]     = true;
  result[QStringLiteral("running")]     = diagnostics.running();
  result[QStringLiteral("buses")]       = busArray(scope);
  result[QStringLiteral("probing")]     = busArray(probing);
  result[QStringLiteral("instant")]     = instant;
  result[QStringLiteral("estimatedMs")] = estimate;
  result[QStringLiteral("hint")] =
    QStringLiteral("Each instant result has {bus, verdict, code, title, explanation, remedy}; "
                   "remedy holds any shell command verbatim. Poll diagnostics.status until "
                   "running is false, then read every finding with problems.list.");
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Reports whether a run is still in flight and what the cached results add up to.
 */
API::CommandResponse API::Handlers::DiagnosticsHandler::status(const QString& id,
                                                               const QJsonObject& params)
{
  Q_UNUSED(params)

  static const auto& diagnostics = ConnectionDiagnostics::instance();
  const auto scope               = ConnectionDiagnostics::supportedBuses();

  QJsonObject result;
  result[QStringLiteral("running")] = diagnostics.running();
  result[QStringLiteral("lastRun")] = diagnostics.lastRunTime();
  result[QStringLiteral("buses")]   = busArray(scope);
  result[QStringLiteral("counts")]  = countsFor(scope);
  result[QStringLiteral("hint")] =
    QStringLiteral("running false means every probe settled. Findings are read with "
                   "problems.list, filtered by checkerId 'diagnostics.serial', "
                   "'diagnostics.bluetooth', 'diagnostics.network', 'diagnostics.broker' or "
                   "'diagnostics.audio'.");
  return CommandResponse::makeSuccess(id, result);
}
