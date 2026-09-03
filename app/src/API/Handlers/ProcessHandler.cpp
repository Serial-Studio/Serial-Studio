/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is part of Serial Studio Pro. All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/ProcessHandler.h"

#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all Process driver commands with the registry.
 */
void API::Handlers::ProcessHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();
  const auto empty      = emptySchema();

  registry.registerCommand(QStringLiteral("io.process.setMode"),
                           QStringLiteral("Set driver mode (params: mode - 0=Launch, 1=NamedPipe)"),
                           makeSchema({
                             {QStringLiteral("mode"),
                              QStringLiteral("integer"),
                              QStringLiteral("Driver mode (0=Launch, 1=NamedPipe)")}
  }),
                           &setMode);

  registry.registerCommand(
    QStringLiteral("io.process.setExecutable"),
    QStringLiteral("Set executable path for Launch mode (params: executable)"),
    makeSchema({
      {QStringLiteral("executable"),
       QStringLiteral("string"),
       QStringLiteral("Absolute path to the executable")}
  }),
    &setExecutable);

  registry.registerCommand(
    QStringLiteral("io.process.setArguments"),
    QStringLiteral("Set command-line arguments for Launch mode (params: arguments)"),
    makeSchema({
      {QStringLiteral("arguments"),
       QStringLiteral("string"),
       QStringLiteral("Shell-style argument string")}
  }),
    &setArguments);

  registry.registerCommand(
    QStringLiteral("io.process.setWorkingDir"),
    QStringLiteral("Set working directory for Launch mode (params: workingDir)"),
    makeSchema({
      {QStringLiteral("workingDir"),
       QStringLiteral("string"),
       QStringLiteral("Absolute path to the working directory")}
  }),
    &setWorkingDir);

  registry.registerCommand(
    QStringLiteral("io.process.setPipePath"),
    QStringLiteral("Set named pipe / FIFO path for NamedPipe mode (params: pipePath)"),
    makeSchema({
      {QStringLiteral("pipePath"),
       QStringLiteral("string"),
       QStringLiteral("Named pipe or FIFO path")}
  }),
    &setPipePath);

  registry.registerCommand(QStringLiteral("io.process.listRunning"),
                           QStringLiteral("Refresh and return the list of running processes"),
                           empty,
                           &getRunningProcesses);

  registry.registerCommand(QStringLiteral("io.process.getConfig"),
                           QStringLiteral("Get complete Process driver configuration"),
                           empty,
                           &getConfiguration);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Set the driver mode.
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

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.process()->setMode(mode);

  QJsonObject result;
  result[QStringLiteral("mode")] = mode;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the executable path for Launch mode.
 */
API::CommandResponse API::Handlers::ProcessHandler::setExecutable(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("executable"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: executable"));
  }

  const QString path = params.value(QStringLiteral("executable")).toString();

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.process()->setExecutable(path);

  QJsonObject result;
  result[QStringLiteral("executable")] = path;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the command-line arguments for Launch mode.
 */
API::CommandResponse API::Handlers::ProcessHandler::setArguments(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("arguments"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: arguments"));
  }

  const QString args             = params.value(QStringLiteral("arguments")).toString();
  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.process()->setArguments(args);

  QJsonObject result;
  result[QStringLiteral("arguments")] = args;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the working directory for Launch mode.
 */
API::CommandResponse API::Handlers::ProcessHandler::setWorkingDir(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("workingDir"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: workingDir"));
  }

  const QString dir = params.value(QStringLiteral("workingDir")).toString();

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.process()->setWorkingDir(dir);

  QJsonObject result;
  result[QStringLiteral("workingDir")] = dir;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the named pipe / FIFO path for NamedPipe mode.
 */
API::CommandResponse API::Handlers::ProcessHandler::setPipePath(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("pipePath"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: pipePath"));
  }

  const QString path = params.value(QStringLiteral("pipePath")).toString();

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.process()->setPipePath(path);

  QJsonObject result;
  result[QStringLiteral("pipePath")] = path;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Refresh and return the list of running processes.
 */
API::CommandResponse API::Handlers::ProcessHandler::getRunningProcesses(const QString& id,
                                                                        const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.process()->refreshProcessList();

  const auto& processes = connectionManager.process()->runningProcesses();

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

  static auto& connectionManager = IO::ConnectionManager::instance();
  auto* proc                     = connectionManager.process();

  QJsonObject result;
  result[QStringLiteral("mode")]       = proc->mode();
  result[QStringLiteral("executable")] = proc->executable();
  result[QStringLiteral("arguments")]  = proc->arguments();
  result[QStringLiteral("workingDir")] = proc->workingDir();
  result[QStringLiteral("pipePath")]   = proc->pipePath();
  return CommandResponse::makeSuccess(id, result);
}
