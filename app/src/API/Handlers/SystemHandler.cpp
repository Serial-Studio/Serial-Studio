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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/SystemHandler.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "API/ProcessLauncher.h"
#include "API/SchemaBuilder.h"
#include "DataModel/ProjectModel.h"

/**
 * @brief Returns the directory, file path, and file name of the loaded project.
 */
API::CommandResponse API::Handlers::SystemHandler::projectDir(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& path = DataModel::ProjectModel::instance().jsonFilePath();
  if (path.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("No project file is loaded"));
  }

  const QFileInfo info(path);

  QJsonObject result;
  result[QStringLiteral("directory")] = QDir::toNativeSeparators(info.absolutePath());
  result[QStringLiteral("filePath")]  = QDir::toNativeSeparators(info.absoluteFilePath());
  result[QStringLiteral("fileName")]  = info.fileName();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Spawns a helper process (e.g. a Python script) tied to the connection lifecycle.
 */
API::CommandResponse API::Handlers::SystemHandler::exec(const QString& id,
                                                        const QJsonObject& params)
{
  const auto program = params.value(QStringLiteral("program")).toString();
  if (program.isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing 'program'"));

  QStringList arguments;
  for (const auto& arg : params.value(QStringLiteral("args")).toArray())
    arguments.append(arg.toString());

  auto workingDir = params.value(QStringLiteral("workingDir")).toString();
  if (workingDir.isEmpty()) {
    const auto& projectPath = DataModel::ProjectModel::instance().jsonFilePath();
    if (!projectPath.isEmpty())
      workingDir = QFileInfo(projectPath).absolutePath();
  }

  QString error;
  const int processId = ProcessLauncher::instance().launch(program, arguments, workingDir, error);
  if (processId < 0)
    return CommandResponse::makeError(id, ErrorCode::OperationFailed, error);

  QJsonObject result;
  result[QStringLiteral("processId")] = processId;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Terminates a single managed helper process by id.
 */
API::CommandResponse API::Handlers::SystemHandler::kill(const QString& id,
                                                        const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("processId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing 'processId'"));

  const int processId = params.value(QStringLiteral("processId")).toInt(-1);
  if (!ProcessLauncher::instance().kill(processId)) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("No running process with id %1").arg(processId));
  }

  return CommandResponse::makeSuccess(id, QJsonObject());
}

/**
 * @brief Lists every managed helper process currently running.
 */
API::CommandResponse API::Handlers::SystemHandler::runningProcesses(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  QJsonObject result;
  result[QStringLiteral("processes")] =
    QJsonArray::fromVariantList(ProcessLauncher::instance().runningProcesses());
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Registers all system.* commands with the global registry.
 */
void API::Handlers::SystemHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("system.projectDir"),
    QStringLiteral("Return the loaded project's directory, file path, and file name. Use this "
                   "to resolve paths relative to the .ssproj (e.g. a sibling Python script) "
                   "before calling system.exec."),
    emptySchema(),
    &projectDir);

  registry.registerCommand(
    QStringLiteral("system.exec"),
    QStringLiteral("Launch a helper process (e.g. \"python\" with a script path). Control "
                   "script only -- rejected over the network/SDK. workingDir defaults to the "
                   "project directory. Returns a processId. The process is terminated "
                   "automatically when the device disconnects, the project changes, or the "
                   "app quits."),
    makeSchema(
      {
        {QStringLiteral("program"),
         QStringLiteral("string"),
         QStringLiteral("Executable to run (resolved from PATH or an absolute path)")}
  },
      {{QStringLiteral("args"), QStringLiteral("array"), QStringLiteral("Command-line arguments")},
       {QStringLiteral("workingDir"),
        QStringLiteral("string"),
        QStringLiteral("Working directory (defaults to the project directory)")}}),
    &exec);

  registry.registerCommand(
    QStringLiteral("system.kill"),
    QStringLiteral("Terminate a managed helper process by id. Control script only."),
    makeSchema({
      {QStringLiteral("processId"), QStringLiteral("integer"), QStringLiteral("Process id")}
  }),
    &kill);

  registry.registerCommand(
    QStringLiteral("system.runningProcesses"),
    QStringLiteral("List the helper processes currently managed by system.exec."),
    emptySchema(),
    &runningProcesses);
}
