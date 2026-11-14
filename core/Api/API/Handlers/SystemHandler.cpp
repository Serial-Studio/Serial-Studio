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

#include "API/Handlers/SystemHandler.h"

#include <optional>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "API/ProcessLauncher.h"
#include "API/SchemaBuilder.h"
#include "Core/Runtime.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"

//--------------------------------------------------------------------------------------------------
// Package sessions (spec 0123)
//--------------------------------------------------------------------------------------------------

/**
 * @brief The refusal a package session gives a helper-program command: always, unless the
 *        package's author allowed helper programs, and to a remote caller when @p scriptOnly is
 *        set, so a network client never learns where the resources were written.
 */
[[nodiscard]] static std::optional<API::CommandResponse> sealedRefusal(const QString& id,
                                                                       const bool scriptOnly)
{
  if (!Core::Runtime::sealed())
    return std::nullopt;

  const auto* scope = API::RemoteDispatchScope::active();
  const bool remote = scriptOnly && scope != nullptr && scope->gated();
  if (Core::Runtime::sealedHelpers().enabled && !remote)
    return std::nullopt;

  return API::CommandResponse::makeError(
    id,
    QString::fromLatin1(API::ErrorCode::SealedSession),
    QStringLiteral("Helper programs are not available in this deployment package session"));
}

/**
 * @brief Where a package session's helpers live: the resources folder when the package carries
 *        resources, otherwise the folder that holds the package file.
 */
[[nodiscard]] static const QString& sealedHelperFolder()
{
  const auto& helpers = Core::Runtime::sealedHelpers();
  return helpers.resourceFolder.isEmpty() ? helpers.packageFolder : helpers.resourceFolder;
}

/**
 * @brief Answers system.projectDir in a package session: the helper folder as the directory, the
 *        package file as the file, and the package file's own folder under packageDirectory.
 */
[[nodiscard]] static API::CommandResponse sealedProjectDir(const QString& id)
{
  const auto& helpers = Core::Runtime::sealedHelpers();

  QJsonObject result;
  result[QStringLiteral("directory")]        = QDir::toNativeSeparators(sealedHelperFolder());
  result[QStringLiteral("filePath")]         = QDir::toNativeSeparators(helpers.packagePath);
  result[QStringLiteral("fileName")]         = QFileInfo(helpers.packagePath).fileName();
  result[QStringLiteral("packageDirectory")] = QDir::toNativeSeparators(helpers.packageFolder);
  return API::CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Commands
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the directory, file path, and file name of the loaded project.
 */
API::CommandResponse API::Handlers::SystemHandler::projectDir(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  if (auto refusal = sealedRefusal(id, true))
    return *refusal;

  if (Core::Runtime::sealed())
    return sealedProjectDir(id);

  auto& projectModel = DataModel::pipelineModules().projectModel;
  const auto& path   = projectModel.jsonFilePath();
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
  if (auto refusal = sealedRefusal(id, false))
    return *refusal;

  const auto program = params.value(QStringLiteral("program")).toString();
  if (program.isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing 'program'"));

  QStringList arguments;
  for (const auto& arg : params.value(QStringLiteral("args")).toArray())
    arguments.append(arg.toString());

  auto workingDir = params.value(QStringLiteral("workingDir")).toString();
  if (workingDir.isEmpty() && Core::Runtime::sealed())
    workingDir = sealedHelperFolder();

  if (workingDir.isEmpty()) {
    auto& projectModel      = DataModel::pipelineModules().projectModel;
    const auto& projectPath = projectModel.jsonFilePath();
    if (!projectPath.isEmpty())
      workingDir = QFileInfo(projectPath).absolutePath();
  }

  QString error;
  static auto& processLauncher = ProcessLauncher::instance();
  const int processId          = processLauncher.launch(program, arguments, workingDir, error);
  if (processId == ProcessLauncher::kLaunchConsentRequired)
    return CommandResponse::makeError(id, ErrorCode::ConsentRequired, error);

  if (processId == ProcessLauncher::kLaunchDenied)
    return CommandResponse::makeError(id, ErrorCode::ExecutionError, error);

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
  if (auto refusal = sealedRefusal(id, false))
    return *refusal;

  if (!params.contains(QStringLiteral("processId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing 'processId'"));

  const int processId          = params.value(QStringLiteral("processId")).toInt(-1);
  static auto& processLauncher = ProcessLauncher::instance();
  if (!processLauncher.kill(processId)) {
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

  if (auto refusal = sealedRefusal(id, false))
    return *refusal;

  static auto& processLauncher = ProcessLauncher::instance();

  QJsonObject result;
  result[QStringLiteral("processes")] =
    QJsonArray::fromVariantList(processLauncher.runningProcesses());
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Registers all system.* commands with the global registry.
 */
void API::Handlers::SystemHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();

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
                   "script only -- rejected over the network/SDK -- and gated by a one-time "
                   "per-project consent prompt: the first launch answers CONSENT_REQUIRED "
                   "while the user is asked, and the control script restarts once they "
                   "allow it. workingDir defaults to the project directory. Returns a "
                   "processId. The process is terminated automatically when the device "
                   "disconnects, the project changes, or the app quits."),
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
    QStringLiteral("List the helper processes currently managed by system.exec. Control "
                   "script only."),
    emptySchema(),
    &runningProcesses);
}
