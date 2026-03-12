/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is part of Serial Studio Pro. All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "API/Handlers/ProcessHandler.h"

#  include <QJsonArray>

#  include "API/CommandRegistry.h"
#  include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all Process driver commands with the registry.
 */
void API::Handlers::ProcessHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("io.driver.process.setMode"),
                           QStringLiteral("Set driver mode (params: mode - 0=Launch, 1=NamedPipe)"),
                           &setMode);

  registry.registerCommand(
    QStringLiteral("io.driver.process.setExecutable"),
    QStringLiteral("Set executable path for Launch mode (params: executable)"),
    &setExecutable);

  registry.registerCommand(
    QStringLiteral("io.driver.process.setArguments"),
    QStringLiteral("Set command-line arguments for Launch mode (params: arguments)"),
    &setArguments);

  registry.registerCommand(
    QStringLiteral("io.driver.process.setWorkingDir"),
    QStringLiteral("Set working directory for Launch mode (params: workingDir)"),
    &setWorkingDir);

  registry.registerCommand(
    QStringLiteral("io.driver.process.setPipePath"),
    QStringLiteral("Set named pipe / FIFO path for NamedPipe mode (params: pipePath)"),
    &setPipePath);

  registry.registerCommand(
    QStringLiteral("io.driver.process.setOutputCapture"),
    QStringLiteral("Set output capture mode (params: capture - 0=StdOut, 1=StdErr, 2=Both)"),
    &setOutputCapture);

  registry.registerCommand(QStringLiteral("io.driver.process.getRunningProcesses"),
                           QStringLiteral("Refresh and return the list of running processes"),
                           &getRunningProcesses);

  registry.registerCommand(QStringLiteral("io.driver.process.getConfiguration"),
                           QStringLiteral("Get complete Process driver configuration"),
                           &getConfiguration);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Set the driver mode.
 * @param params Requires "mode" (int: 0=Launch, 1=NamedPipe)
 */
API::CommandResponse API::Handlers::ProcessHandler::setMode(const QString& id,
                                                            const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("mode"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: mode"));
  }

  const int mode = params.value(QStringLiteral("mode")).toInt();
  if (mode < 0 || mode > 1) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid mode: %1. Valid values: 0=Launch, 1=NamedPipe").arg(mode));
  }

  IO::ConnectionManager::instance().process()->setMode(mode);

  QJsonObject result;
  result[QStringLiteral("mode")] = mode;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the executable path for Launch mode.
 * @param params Requires "executable" (string, absolute path)
 */
API::CommandResponse API::Handlers::ProcessHandler::setExecutable(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("executable"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: executable"));
  }

  const QString path = params.value(QStringLiteral("executable")).toString();
  IO::ConnectionManager::instance().process()->setExecutable(path);

  QJsonObject result;
  result[QStringLiteral("executable")] = path;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the command-line arguments for Launch mode.
 * @param params Requires "arguments" (string, shell-style argument string)
 */
API::CommandResponse API::Handlers::ProcessHandler::setArguments(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("arguments"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: arguments"));
  }

  const QString args = params.value(QStringLiteral("arguments")).toString();
  IO::ConnectionManager::instance().process()->setArguments(args);

  QJsonObject result;
  result[QStringLiteral("arguments")] = args;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the working directory for Launch mode.
 * @param params Requires "workingDir" (string, absolute directory path)
 */
API::CommandResponse API::Handlers::ProcessHandler::setWorkingDir(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("workingDir"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: workingDir"));
  }

  const QString dir = params.value(QStringLiteral("workingDir")).toString();
  IO::ConnectionManager::instance().process()->setWorkingDir(dir);

  QJsonObject result;
  result[QStringLiteral("workingDir")] = dir;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the named pipe / FIFO path for NamedPipe mode.
 * @param params Requires "pipePath" (string)
 */
API::CommandResponse API::Handlers::ProcessHandler::setPipePath(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("pipePath"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: pipePath"));
  }

  const QString path = params.value(QStringLiteral("pipePath")).toString();
  IO::ConnectionManager::instance().process()->setPipePath(path);

  QJsonObject result;
  result[QStringLiteral("pipePath")] = path;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the output capture mode.
 * @param params Requires "capture" (int: 0=StdOut, 1=StdErr, 2=Both)
 */
API::CommandResponse API::Handlers::ProcessHandler::setOutputCapture(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("capture"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: capture"));
  }

  const int capture = params.value(QStringLiteral("capture")).toInt();
  if (capture < 0 || capture > 2) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid capture: %1. Valid values: 0=StdOut, 1=StdErr, 2=Both").arg(capture));
  }

  IO::ConnectionManager::instance().process()->setOutputCapture(capture);

  QJsonObject result;
  result[QStringLiteral("capture")] = capture;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Refresh and return the list of running processes.
 *
 * Calls refreshProcessList() and returns the updated list — useful for
 * picking a process from which to derive a named-pipe path.
 */
API::CommandResponse API::Handlers::ProcessHandler::getRunningProcesses(const QString& id,
                                                                        const QJsonObject& params)
{
  Q_UNUSED(params)

  IO::ConnectionManager::instance().process()->refreshProcessList();

  const auto& processes = IO::ConnectionManager::instance().process()->runningProcesses();

  QJsonArray processes_array;
  for (const auto& proc : processes)
    processes_array.append(proc);

  QJsonObject result;
  result[QStringLiteral("processes")] = processes_array;
  result[QStringLiteral("count")]     = processes_array.count();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the complete Process driver configuration.
 */
API::CommandResponse API::Handlers::ProcessHandler::getConfiguration(const QString& id,
                                                                     const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* proc = IO::ConnectionManager::instance().process();

  QJsonObject result;
  result[QStringLiteral("mode")]          = proc->mode();
  result[QStringLiteral("outputCapture")] = proc->outputCapture();
  result[QStringLiteral("executable")]    = proc->executable();
  result[QStringLiteral("arguments")]     = proc->arguments();
  result[QStringLiteral("workingDir")]    = proc->workingDir();
  result[QStringLiteral("pipePath")]      = proc->pipePath();
  return CommandResponse::makeSuccess(id, result);
}

#endif  // BUILD_COMMERCIAL
