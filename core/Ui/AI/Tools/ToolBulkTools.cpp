/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Tools/ToolBulkTools.h"

#include <QJsonArray>
#include <QStringList>

#include "AI/CommandRegistry.h"
#include "AI/Tools/ToolSupport.h"
#include "DataModel/Frame.h"

namespace AI::ToolDetail {

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
 * @brief True when a fanned-out inner command (Safe/Confirm tier) may run without an extra
 *        AI confirmation; Blocked and AlwaysConfirm (incl. deviceGated) ops are rejected.
 */
bool innerOpAllowed(const QString& commandName)
{
  static auto& aiReg = AI::CommandRegistry::instance();
  const auto safety  = aiReg.safetyOf(commandName);
  return safety == AI::Safety::Safe || safety == AI::Safety::Confirm;
}

/**
 * @brief Builds the rejection reply for a fanned-out op whose safety tier blocks fan-out.
 */
QJsonObject makeInnerOpRejection(const QString& commandName)
{
  static auto& aiReg = AI::CommandRegistry::instance();
  const auto safety  = aiReg.safetyOf(commandName);

  QJsonObject out;
  out[QStringLiteral("ok")]      = false;
  out[QStringLiteral("error")]   = QStringLiteral("inner_op_blocked");
  out[QStringLiteral("command")] = commandName;
  out[QStringLiteral("safety")]  = static_cast<int>(safety);
  if (aiReg.isDeviceGated(commandName) && !aiReg.deviceControlAllowed())
    out[QStringLiteral("hint")] =
      QStringLiteral("This op drives the device; enable 'Allow device control' or run it as a "
                     "separate confirmed tool call. Batch fan-out cannot satisfy that gate.");
  else if (safety == AI::Safety::Blocked)
    out[QStringLiteral("hint")] =
      QStringLiteral("This op is blocked for AI safety and cannot run inside a batch.");
  else
    out[QStringLiteral("hint")] =
      QStringLiteral("This op needs explicit user approval and must be run as a separate tool "
                     "call, not inside a batch fan-out.");

  return out;
}

/**
 * @brief Validates and forwards a project.batch payload, summarizing any per-op failures.
 */
QJsonObject executeBulkApply(const QJsonObject& args)
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
    const auto op        = value.toObject();
    const auto opCommand = op.value(QStringLiteral("command")).toString();
    if (opCommand == QStringLiteral("project.batch")) {
      QJsonObject out;
      out[QStringLiteral("ok")]    = false;
      out[QStringLiteral("error")] = QStringLiteral("nested_batch_rejected");
      return out;
    }

    if (!innerOpAllowed(opCommand))
      return makeInnerOpRejection(opCommand);
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

}  // namespace AI::ToolDetail
