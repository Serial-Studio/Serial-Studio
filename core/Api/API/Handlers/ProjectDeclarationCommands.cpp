/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "API/Handlers/ProjectDeclarationCommands.h"

#include <QJsonObject>

#include "API/SchemaBuilder.h"
#include "AppState.h"
#include "Core/SerialStudio.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/Project/DeclarationWatch.h"
#include "DataModel/ProjectModel.h"

//--------------------------------------------------------------------------------------------------
// Device declaration command surface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the command class to the registry it publishes into.
 */
API::Handlers::ProjectDeclarationCommands::ProjectDeclarationCommands(CommandRegistry& registry)
  : m_registry(registry)
{}

/**
 * @brief Registers project.declaration.get, adopt and keep, in that order (append-only).
 */
void API::Handlers::ProjectDeclarationCommands::registerCommands()
{
  m_registry.registerCommand(
    QStringLiteral("project.declaration.get"),
    QStringLiteral(
      "Report device declarations (spec 0111). quickPlot: whether the live Quick Plot "
      "dashboard was built from a device declaration, with its title and group "
      "count. In Project File mode: the last declaration the device sent (status "
      "none, matches, differs, kept or rejected; reason; title; groups; datasets; notes), "
      "pendingDifference, and requestOnConnect (the Quick Plot request toggle)."),
    emptySchema(),
    &declarationGet);

  m_registry.registerCommand(
    QStringLiteral("project.declaration.adopt"),
    QStringLiteral("Open the pending device declaration as a new, unsaved project (Project File "
                   "mode, pendingDifference true). Disconnects the device first; the loaded "
                   "project's file is not modified, so reopening it reverses the choice. "
                   "Refused while the loaded project has unsaved changes, unless "
                   "discardChanges:true says to drop them (DESTRUCTIVE for those edits)."),
    makeSchema(
      {
  },
      {{QStringLiteral("discardChanges"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, discard the loaded project's unsaved changes instead "
                       "of refusing.")}}),
    &declarationAdopt);

  m_registry.registerCommand(
    QStringLiteral("project.declaration.keep"),
    QStringLiteral("Keep the loaded project instead of the pending device declaration (Project "
                   "File mode, pendingDifference true). The offer stays dismissed for this project "
                   "and declaration content; nothing is loaded or changed on disk."),
    emptySchema(),
    &declarationKeep);
}

/**
 * @brief Returns the watch's Project File state plus the live Quick Plot declaration, which the
 *        pipeline owns and is read in one command-rate blocking hop.
 */
API::CommandResponse API::Handlers::ProjectDeclarationCommands::declarationGet(
  const QString& id, const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& modules = DataModel::pipelineModules();
  auto result   = modules.projectModel.declarationWatch().status();
  if (modules.appState.operationMode() == SerialStudio::QuickPlot) {
    bool declared = false;
    QString title;
    int groups         = 0;
    auto& frameBuilder = modules.frameBuilder;
    frameBuilder.invokeOnBuilderThreadBlocking([&] {
      declared = frameBuilder.quickPlotDeclared();
      title    = declared ? frameBuilder.quickPlotFrame().title : QString();
      groups   = declared ? static_cast<int>(frameBuilder.quickPlotFrame().groups.size()) : 0;
    });

    result[QStringLiteral("quickPlot")] = QJsonObject{
      {QStringLiteral("declared"), declared},
      {   QStringLiteral("title"),    title},
      {  QStringLiteral("groups"),   groups}
    };
  }

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Opens the pending declaration as a new, unsaved project; refused when none is pending, and
 *        while the loaded project has unsaved edits unless the client says to discard them: a
 *        save prompt would block the client, and dropping the edits is never the default.
 */
API::CommandResponse API::Handlers::ProjectDeclarationCommands::declarationAdopt(
  const QString& id, const QJsonObject& params)
{
  const bool discard = params.value(QStringLiteral("discardChanges")).toBool(false);
  auto& model        = DataModel::pipelineModules().projectModel;
  auto& watch        = model.declarationWatch();
  if (!watch.pendingDifference())
    return CommandResponse::makeError(
      id,
      ErrorCode::OperationFailed,
      QStringLiteral("No differing device declaration is pending (Project File mode only)"));

  if (model.modified() && !discard)
    return CommandResponse::makeError(
      id,
      ErrorCode::OperationFailed,
      QStringLiteral("The loaded project has unsaved changes: save it, or pass "
                     "discardChanges:true to drop them"));

  model.setSuppressMessageBoxes(true);
  const bool adopted = watch.adopt();
  model.setSuppressMessageBoxes(false);
  if (!adopted)
    return CommandResponse::makeError(
      id, ErrorCode::OperationFailed, QStringLiteral("The declared project was not opened"));

  QJsonObject result;
  result[QStringLiteral("title")]    = model.title();
  result[QStringLiteral("groups")]   = static_cast<int>(model.groups().size());
  result[QStringLiteral("modified")] = model.modified();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Keeps the loaded project and dismisses the pending declaration (spec 0111 R3); refused
 *        when none is pending.
 */
API::CommandResponse API::Handlers::ProjectDeclarationCommands::declarationKeep(
  const QString& id, const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& watch = DataModel::pipelineModules().projectModel.declarationWatch();
  if (!watch.pendingDifference())
    return CommandResponse::makeError(
      id,
      ErrorCode::OperationFailed,
      QStringLiteral("No differing device declaration is pending (Project File mode only)"));

  watch.keep();
  QJsonObject result;
  result[QStringLiteral("kept")] = !watch.pendingDifference();
  return CommandResponse::makeSuccess(id, result);
}
