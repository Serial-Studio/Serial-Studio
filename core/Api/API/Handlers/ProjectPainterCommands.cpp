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

#include "API/Handlers/ProjectPainterCommands.h"

#include <QJsonObject>

#include "API/SchemaBuilder.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"

//--------------------------------------------------------------------------------------------------
// Painter (group widget JS) command surface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the command class to the registry it publishes into.
 */
API::Handlers::ProjectPainterCommands::ProjectPainterCommands(CommandRegistry& registry)
  : m_registry(registry)
{}

/**
 * @brief Register painter widget JS setCode/getCode commands.
 */
void API::Handlers::ProjectPainterCommands::registerCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.painter.setCode"),
    QStringLiteral("Set the canvas widget code for a group (params: groupId, code). "
                   "**JavaScript only** -- canvas scripts run in QJSEngine, not Lua. "
                   "Available globals: ctx (2D canvas context, QPainter-like), w, h "
                   "(canvas dimensions), datasetGetFinal(uid)/datasetGetRaw(uid). The "
                   "entry point is paint(ctx, w, h) and an optional zero-arg onFrame() "
                   "callback. "
                   "Validate with project.painter.dryRun before setCode. **Always call "
                   "meta.fetchScriptingDocs{kind:'painter_js'} first** for the full API "
                   "surface and worked examples -- don't invent canvas methods from JS "
                   "DOM Canvas, the surface is QPainter-shaped."),
    makeSchema({
      {QStringLiteral("groupId"),
       QStringLiteral("integer"),
       QStringLiteral("Target group id (from project.group.list)")              },
      {   QStringLiteral("code"),
       QStringLiteral("string"),
       QStringLiteral("Canvas widget JS source. Must define paint(ctx, w, h) and may "
       "define a zero-arg onFrame(). Replaces any existing code for the group.")}
  }),
    &painterSetCode);

  registry.registerCommand(
    QStringLiteral("project.painter.getCode"),
    QStringLiteral("Get the canvas widget JS for a group "
                   "(params: groupId)"),
    makeSchema({
      {QStringLiteral("groupId"), QStringLiteral("integer"), QStringLiteral("Target group id")}
  }),
    &painterGetCode);
}

/**
 * @brief Set painter code for a specific group by id.
 */
API::CommandResponse API::Handlers::ProjectPainterCommands::painterSetCode(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const QString code   = params.value(QStringLiteral("code")).toString();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();

  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  DataModel::Group g = groups[groupId];
  g.painterCode      = code;
  project.updateGroup(groupId, g, true);

  QJsonObject result;
  result[QStringLiteral("groupId")]    = groupId;
  result[QStringLiteral("codeLength")] = code.size();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Read the painter code for a group.
 */
API::CommandResponse API::Handlers::ProjectPainterCommands::painterGetCode(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId  = params.value(QStringLiteral("groupId")).toInt();
  static auto& model = DataModel::ProjectModel::instance();
  const auto& groups = model.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[QStringLiteral("code")]    = groups[groupId].painterCode;
  return CommandResponse::makeSuccess(id, result);
}
