/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Tools/ToolDispatch.h"

#include <QHash>
#include <QUuid>

#include "AI/CommandRegistry.h"
#include "AI/Logging.h"
#include "AI/Tools/ToolCatalog.h"
#include "AI/Tools/ToolFilesystemTools.h"
#include "AI/Tools/ToolSchemas.h"
#include "AI/Tools/ToolSupport.h"
#include "API/CommandRegistry.h"
#include "Misc/JsonValidator.h"

namespace AI::ToolDetail {

/**
 * @brief Builds the Blocked-tool rejection reply, with a device-control repair hint.
 */
static QJsonObject makeBlockedReply(const QString& name)
{
  qCWarning(AI::serialStudioAI) << "Tool execution blocked:" << name;
  QJsonObject error;
  error[QStringLiteral("code")]    = QStringLiteral("blocked");
  error[QStringLiteral("message")] = QStringLiteral("This command is blocked for AI safety.");
  static auto& aiReg               = AI::CommandRegistry::instance();
  if (aiReg.isDeviceGated(name)) {
    error[QStringLiteral("repair")] =
      QStringLiteral("Hardware writes and connection changes must be performed by the user "
                     "unless they enable the 'Allow device control' checkbox in the AI panel. "
                     "Offer to build an Output Control tile instead; mention the checkbox only "
                     "if the user explicitly wants you to drive the device.");
  }

  QJsonObject reply;
  reply[QStringLiteral("ok")]    = false;
  reply[QStringLiteral("error")] = error;
  return reply;
}

/**
 * @brief Injects a bounded default `limit` for a whole-project list command called without
 *        one (never overwrites a caller's limit; raw TCP/SDK clients bypass this). Limits are
 *        calibrated per row shape -- a group.list row embeds full nested datasets, so its cap
 *        bounds count not bytes; project.search / project.group.get are the compact paths.
 */
static QJsonObject withDefaultListLimit(const QString& name, const QJsonObject& args)
{
  static const QHash<QString, int> kDefaultLimits = {
    {             QStringLiteral("project.dataset.list"),  4},
    {               QStringLiteral("project.group.list"),  5},
    {QStringLiteral("project.dataset.getExecutionOrder"), 20},
  };

  const int defaultLimit = kDefaultLimits.value(name, 0);
  if (defaultLimit == 0 || args.contains(QStringLiteral("limit")))
    return args;

  QJsonObject bounded              = args;
  bounded[QStringLiteral("limit")] = defaultLimit;
  return bounded;
}

/**
 * @brief Validates args and forwards to API::CommandRegistry honoring AI safety tags.
 */
QJsonObject executeCommand(const QString& requestedName, const QJsonObject& args)
{
  const QString name = canonicalToolName(requestedName);
  Misc::JsonValidator::Limits limits;
  limits.maxFileSize  = 1 * 1024 * 1024;
  limits.maxDepth     = 32;
  limits.maxArraySize = 1024;

  if (!Misc::JsonValidator::validateStructure(QJsonValue(args), limits)) {
    qCWarning(AI::serialStudioAI) << "Tool args validation failed for" << name;
    QJsonObject reply;
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("args_validation_failed");
    reply[QStringLiteral("repair")] =
      QStringLiteral("The arguments object failed structural validation (too deeply nested, "
                     "too many array items, or too large). Call meta.describeCommand{name: "
                     "\"%1\"} and rebuild a minimal arguments object that matches the schema.")
        .arg(name);
    return reply;
  }

  static auto& aiReg = AI::CommandRegistry::instance();
  if (aiReg.safetyOf(name) == Safety::Blocked)
    return makeBlockedReply(name);

  if (isAssistantTool(name))
    return executeAssistantTool(name, args);

  if (isFsTool(name))
    return executeFsTool(name, args);

  const auto callId        = QUuid::createUuid().toString(QUuid::WithoutBraces);
  static auto& apiRegistry = API::CommandRegistry::instance();
  const auto response      = apiRegistry.execute(name, callId, withDefaultListLimit(name, args));

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

}  // namespace AI::ToolDetail
