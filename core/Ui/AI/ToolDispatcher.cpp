/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/ToolDispatcher.h"

#include <QStringList>
#include <QUuid>

#include "AI/Tools/ToolCatalog.h"
#include "AI/Tools/ToolDispatch.h"
#include "API/CommandRegistry.h"

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
  return ToolDetail::availableTools(category);
}

/**
 * @brief Returns a compact name+description list of every command, optionally filtered by prefix
 *        and windowed by offset/limit.
 */
QJsonObject AI::ToolDispatcher::listCommands(const QString& prefix,
                                             int offset,
                                             int limit,
                                             bool namesOnly) const
{
  return ToolDetail::listCommands(prefix, offset, limit, namesOnly);
}

/**
 * @brief Case-insensitive name+description search over the merged tool catalog.
 */
QJsonObject AI::ToolDispatcher::searchCommands(const QString& query, int offset, int limit) const
{
  return ToolDetail::searchCommands(query, offset, limit);
}

/**
 * @brief Returns the top-level scope namespaces with descriptions.
 */
QJsonObject AI::ToolDispatcher::listCategories() const
{
  return ToolDetail::listCategories();
}

/**
 * @brief Folds a provider-sanitized tool name (dots/colons as '_') back to its canonical form.
 */
QString AI::ToolDispatcher::canonicalToolName(const QString& name) const
{
  return ToolDetail::canonicalToolName(name);
}

/**
 * @brief Returns the metadata block for a single command, or an empty object.
 */
QJsonObject AI::ToolDispatcher::describeCommand(const QString& name) const
{
  return ToolDetail::describeCommand(name);
}

//--------------------------------------------------------------------------------------------------
// Dispatch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates args and forwards to API::CommandRegistry honoring AI safety tags.
 */
QJsonObject AI::ToolDispatcher::executeCommand(const QString& name, const QJsonObject& args)
{
  return ToolDetail::executeCommand(name, args);
}

//--------------------------------------------------------------------------------------------------
// Context (placeholders for the next slice)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the result.result field of a safe command, or an empty object.
 */
static QJsonObject runSafeCommand(const QString& name)
{
  const auto callId        = QUuid::createUuid().toString(QUuid::WithoutBraces);
  static auto& apiRegistry = API::CommandRegistry::instance();
  const auto response      = apiRegistry.execute(name, callId, {});
  if (!response.success)
    return {};

  return response.result;
}

/**
 * @brief Returns a one-shot composite of every readable status endpoint.
 */
QJsonObject AI::ToolDispatcher::getSnapshot() const
{
  static const QStringList kStatusCommands = {
    QStringLiteral("project.getStatus"),
    QStringLiteral("io.getStatus"),
    QStringLiteral("dashboard.getStatus"),
    QStringLiteral("console.getConfig"),
    QStringLiteral("consoleExport.getStatus"),
    QStringLiteral("csvExport.getStatus"),
    QStringLiteral("csvPlayer.getStatus"),
    QStringLiteral("project.mqtt.publisher.getStatus"),
    QStringLiteral("project.mqtt.subscriber.getStatus"),
    QStringLiteral("sessions.getStatus"),
    QStringLiteral("mdf4Export.getStatus"),
    QStringLiteral("mdf4Player.getStatus"),
    QStringLiteral("licensing.getStatus"),
    QStringLiteral("notifications.getUnreadCount"),
  };

  QJsonObject snapshot;
  QJsonArray skipped;
  static auto& apiRegistry = API::CommandRegistry::instance();
  for (const auto& name : kStatusCommands) {
    if (!apiRegistry.hasCommand(name)) {
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
  static auto& apiRegistry = API::CommandRegistry::instance();
  for (const auto& name : kSafeListCommands) {
    if (!apiRegistry.hasCommand(name))
      continue;

    state.insert(name, runSafeCommand(name));
  }

  return state;
}
