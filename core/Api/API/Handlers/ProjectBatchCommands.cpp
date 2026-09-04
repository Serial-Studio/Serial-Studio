/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/ProjectBatchCommands.h"

#include <optional>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>

#include "API/SchemaBuilder.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the command class to the registry it publishes into.
 */
API::Handlers::ProjectBatchCommands::ProjectBatchCommands(CommandRegistry& registry)
  : m_registry(registry)
{}

/**
 * @brief Registers the project.batch multi-op endpoint.
 */
void API::Handlers::ProjectBatchCommands::registerCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.batch"),
    QStringLiteral(
      "Run several project mutations atomically WITH RESPECT TO AUTOSAVE -- "
      "all ops execute sequentially under one suspended-autosave window, "
      "and a single save is flushed at the end. Use this whenever you would "
      "otherwise issue more than ~5 sequential mutations (renames, retypes, "
      "reindexes), since N round-trips cost N times the latency and N times "
      "the autosave/tree-rebuild churn.\n"
      "Each op is {command: '<registered command name>', params: {...}}; "
      "results are returned in the same order with per-op success/error "
      "fields. Set stopOnError:true to abort the batch on the first failure "
      "(default false: best-effort, all ops attempted).\n"
      "Pass dryRun:true at the top level to preview every op without "
      "committing. Each op's per-result still carries the affected-entity "
      "payload as if it had run. Rejected when any op is not in the "
      "dryRun-aware command set -- mixing previewable and non-previewable "
      "ops would leave a partial mutation, which is worse than no preview "
      "at all.\n"
      "Note: NOT transactional. Already-applied ops are NOT rolled back on "
      "later failures -- this is a save-suspend wrapper, not a database "
      "transaction. Nested project.batch calls are rejected. Hard cap of "
      "1024 ops per call.\n"
      "Example: rename 40 datasets in one round-trip:\n"
      "  ops: [\n"
      "    {command:'project.dataset.update', params:{groupId:0, datasetId:0, title:'LED 1', index:1}},\n"
      "    {command:'project.dataset.update', params:{groupId:0, datasetId:1, title:'LED 2', index:2}},\n"
      "    ...\n"
      "  ]"),
    makeSchema({
      arrayProp(
        QStringLiteral("ops"),
        QStringLiteral("Array of {command, params} ops to execute sequentially. Max 1024."),
        QJsonObject{
                    {QStringLiteral("type"), QStringLiteral("object")},
                    {QStringLiteral("properties"),
           QJsonObject{
             {QStringLiteral("command"),
              QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
                          {QStringLiteral("description"),
                           QStringLiteral("Registered command name (e.g. "
                                          "'project.dataset.update'). Not "
                                          "'project.batch' -- nested batches are rejected.")}}},
             {QStringLiteral("params"),
              QJsonObject{{QStringLiteral("type"), QStringLiteral("object")},
                          {QStringLiteral("description"),
                           QStringLiteral(
                             "Arguments object for the command, exactly "
                             "what you would pass at the top level if calling it directly.")}}}}},
                    {QStringLiteral("required"),
           QJsonArray{QStringLiteral("command"), QStringLiteral("params")}}}
        )
  }),
    &projectBatch);
}

namespace API::Handlers {

//--------------------------------------------------------------------------------------------------
// Project batch: run a sequence of commands under one autosave window
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs an array of project mutations sequentially under a suspended autosave.
 */
/**
 * @brief Returns the schema-hint object attached to every project.batch validation error.
 */
static QJsonObject buildBatchSchemaHint()
{
  QJsonObject example;
  QJsonArray exampleOps;
  QJsonObject op0;
  op0[QStringLiteral("command")] = QStringLiteral("project.dataset.update");
  QJsonObject p0;
  p0[QStringLiteral("groupId")] = 0;
  p0[Keys::DatasetId]           = 0;
  p0[QStringLiteral("title")]   = QStringLiteral("LED 1");
  p0[QStringLiteral("index")]   = 1;
  op0[QStringLiteral("params")] = p0;
  exampleOps.append(op0);
  example[QStringLiteral("ops")]         = exampleOps;
  example[QStringLiteral("stopOnError")] = false;

  QJsonObject hint;
  hint[QStringLiteral("expected")] =
    QStringLiteral("{ ops: Array<{command: string, params: object}>, stopOnError?: boolean }");
  hint[QStringLiteral("opShape")] =
    QStringLiteral("Each op MUST be {command: '<registered name>', params: {...}}. Per-call "
                   "args go INSIDE params, not at the top of the op object.");
  hint[QStringLiteral("limits")] =
    QStringLiteral("1 <= ops.length <= 1024. Nested project.batch is rejected.");
  hint[QStringLiteral("example")] = example;
  return hint;
}

/**
 * @brief Builds a per-op result entry for a validation failure inside project.batch.
 */
static QJsonObject buildBatchErrorEntry(int index,
                                        const QString& command,
                                        const QString& code,
                                        const QString& message,
                                        const QJsonObject& data)
{
  QJsonObject entry;
  entry[QStringLiteral("index")] = index;
  if (!command.isEmpty())
    entry[QStringLiteral("command")] = command;

  entry[QStringLiteral("success")] = false;
  QJsonObject err;
  err[QStringLiteral("code")]    = code;
  err[QStringLiteral("message")] = message;
  if (!data.isEmpty())
    err[QStringLiteral("data")] = data;

  entry[QStringLiteral("error")] = err;
  return entry;
}

/**
 * @brief Commands that honour `dryRun:true` -- used to validate batch previews.
 */
static const QSet<QString>& dryRunAwareCommands()
{
  static const QSet<QString> kSet = {
    QStringLiteral("project.dataset.delete"),
    QStringLiteral("project.group.delete"),
    QStringLiteral("project.dataset.move"),
    QStringLiteral("project.group.move"),
    QStringLiteral("project.workspace.delete"),
    QStringLiteral("project.workspace.clearAll"),
    QStringLiteral("project.new"),
    QStringLiteral("project.open"),
    QStringLiteral("project.loadJson"),
    QStringLiteral("project.template.apply"),
    QStringLiteral("project.batch"),
    QStringLiteral("assistant.project.bulkApply"),
  };
  return kSet;
}

/**
 * @brief Validates and executes a single project.batch op; returns the per-op result entry.
 */
static QJsonObject executeBatchOp(int index, const QJsonObject& op, bool dryRun, bool& success)
{
  success = false;

  if (op.isEmpty()) {
    return buildBatchErrorEntry(
      index,
      QString(),
      API::ErrorCode::InvalidParam,
      QStringLiteral("ops[%1] must be an object of shape {command: string, params: object}")
        .arg(index),
      buildBatchSchemaHint());
  }

  const auto command = op.value(QStringLiteral("command")).toString();
  auto opParams      = op.value(QStringLiteral("params")).toObject();

  if (command.isEmpty()) {
    return buildBatchErrorEntry(index,
                                QString(),
                                API::ErrorCode::MissingParam,
                                QStringLiteral("ops[%1].command is required (each op is "
                                               "{command: '<registered name>', params: {...}})")
                                  .arg(index),
                                buildBatchSchemaHint());
  }

  if (command == QStringLiteral("project.batch")) {
    return buildBatchErrorEntry(index,
                                command,
                                API::ErrorCode::InvalidParam,
                                QStringLiteral("project.batch cannot be nested"),
                                QJsonObject());
  }

  if (dryRun)
    opParams.insert(QStringLiteral("dryRun"), true);

  static auto& commandRegistry = API::CommandRegistry::instance();
  const auto response          = commandRegistry.execute(command, QString::number(index), opParams);

  QJsonObject entry;
  entry[QStringLiteral("index")]   = index;
  entry[QStringLiteral("command")] = command;
  entry[QStringLiteral("success")] = response.success;
  if (response.success) {
    if (!response.result.isEmpty())
      entry[QStringLiteral("result")] = response.result;

    success = true;
  } else {
    QJsonObject err;
    err[QStringLiteral("code")]    = response.errorCode;
    err[QStringLiteral("message")] = response.errorMessage;
    if (!response.errorData.isEmpty())
      err[QStringLiteral("data")] = response.errorData;

    entry[QStringLiteral("error")] = err;
  }
  return entry;
}

/**
 * @brief Validate the `ops` array against the batch handler's preconditions; returns an error
 * response when invalid.
 */
static std::optional<API::CommandResponse> validateBatchOps(const QString& id,
                                                            const QJsonObject& params,
                                                            QJsonArray& outOps)
{
  constexpr int kMaxBatchOps = 1024;

  if (!params.contains(QStringLiteral("ops")))
    return API::CommandResponse::makeError(id,
                                           API::ErrorCode::MissingParam,
                                           QStringLiteral("Missing required parameter: ops"),
                                           buildBatchSchemaHint());

  if (!params.value(QStringLiteral("ops")).isArray())
    return API::CommandResponse::makeError(id,
                                           API::ErrorCode::InvalidParam,
                                           QStringLiteral("ops must be an array"),
                                           buildBatchSchemaHint());

  outOps = params.value(QStringLiteral("ops")).toArray();
  if (outOps.isEmpty())
    return API::CommandResponse::makeError(id,
                                           API::ErrorCode::InvalidParam,
                                           QStringLiteral("ops array must not be empty"),
                                           buildBatchSchemaHint());

  if (outOps.size() > kMaxBatchOps)
    return API::CommandResponse::makeError(
      id,
      API::ErrorCode::InvalidParam,
      QStringLiteral("ops array exceeds limit of %1 (got %2)")
        .arg(QString::number(kMaxBatchOps), QString::number(outOps.size())),
      buildBatchSchemaHint());

  return std::nullopt;
}

/**
 * @brief When dryRun is set, ensure every op supports dryRun; returns an error response if any does
 * not.
 */
static std::optional<API::CommandResponse> ensureBatchDryRunCompatible(const QString& id,
                                                                       const QJsonArray& ops)
{
  QJsonArray unsupported;
  for (int i = 0; i < ops.size(); ++i) {
    const auto cmd = ops.at(i).toObject().value(QStringLiteral("command")).toString();
    if (!dryRunAwareCommands().contains(cmd)) {
      QJsonObject row;
      row[QStringLiteral("index")]   = i;
      row[QStringLiteral("command")] = cmd;
      unsupported.append(row);
    }
  }
  if (unsupported.isEmpty())
    return std::nullopt;

  QJsonObject data;
  data[QStringLiteral("unsupportedOps")] = unsupported;
  data[QStringLiteral("supportedSet")] = QJsonArray::fromStringList(dryRunAwareCommands().values());
  return API::CommandResponse::makeError(
    id,
    API::ErrorCode::InvalidParam,
    QStringLiteral("dryRun rejected: %1 op(s) do not support dryRun. Either drop dryRun "
                   "for the whole batch or split the un-previewable ops out into a "
                   "separate batch.")
      .arg(unsupported.size()),
    data);
}

}  // namespace API::Handlers

/**
 * @brief Runs an array of project mutations under a single suspended-autosave window.
 */
API::CommandResponse API::Handlers::ProjectBatchCommands::projectBatch(const QString& id,
                                                                       const QJsonObject& params)
{
  QJsonArray ops;
  if (const auto invalid = validateBatchOps(id, params, ops); invalid)
    return *invalid;

  const bool stopOnError = params.value(QStringLiteral("stopOnError")).toBool(false);
  const bool isDryRun    = params.value(QStringLiteral("dryRun")).toBool(false);

  if (isDryRun) {
    if (const auto invalid = ensureBatchDryRunCompatible(id, ops); invalid)
      return *invalid;
  }

  static auto& project = DataModel::ProjectModel::instance();
  if (!isDryRun)
    project.setAutoSaveSuspended(true);

  QJsonArray results;
  int successCount = 0;
  int failureCount = 0;
  bool aborted     = false;

  for (int i = 0; i < ops.size(); ++i) {
    bool opSucceeded  = false;
    QJsonObject entry = executeBatchOp(i, ops.at(i).toObject(), isDryRun, opSucceeded);
    results.append(entry);
    if (opSucceeded)
      ++successCount;
    else
      ++failureCount;

    if (!opSucceeded && stopOnError) {
      aborted = true;
      break;
    }
  }

  if (!isDryRun) {
    project.setAutoSaveSuspended(false);
    project.flushAutoSave();
  }

  QJsonObject result;
  if (isDryRun)
    result[QStringLiteral("dryRun")] = true;

  result[QStringLiteral("results")]   = results;
  result[QStringLiteral("total")]     = ops.size();
  result[QStringLiteral("succeeded")] = successCount;
  result[QStringLiteral("failed")]    = failureCount;
  result[QStringLiteral("aborted")]   = aborted;
  result[QStringLiteral("autoSaveMode")] =
    isDryRun ? QStringLiteral("none") : QStringLiteral("flushed");
  if (isDryRun)
    result[QStringLiteral("warning")] =
      QStringLiteral("DRY RUN: no ops were committed. Each op's per-result still carries the "
                     "affected-entity payload as if it had run.");

  return CommandResponse::makeSuccess(id, result);
}
