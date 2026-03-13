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

#pragma once

#ifdef BUILD_COMMERCIAL

#  include "API/CommandProtocol.h"

namespace API {
namespace Handlers {

/**
 * @class ProcessHandler
 * @brief Registers API commands for IO::Drivers::Process operations (Pro
 *        feature).
 *
 * Provides commands for the Process I/O driver (child process or named pipe):
 *
 * Configuration:
 * - io.driver.process.setMode       - Set mode (0=Launch, 1=NamedPipe)
 * - io.driver.process.setExecutable - Set executable path (Launch mode)
 * - io.driver.process.setArguments  - Set command-line arguments (Launch mode)
 * - io.driver.process.setWorkingDir - Set working directory (Launch mode)
 * - io.driver.process.setPipePath   - Set named pipe / FIFO path (Pipe mode)
 *
 * Queries:
 * - io.driver.process.getRunningProcesses - List running processes
 * - io.driver.process.getConfiguration    - Get complete driver config
 */
class ProcessHandler {
public:
  static void registerCommands();

private:
  static CommandResponse setMode(const QString& id, const QJsonObject& params);
  static CommandResponse setExecutable(const QString& id, const QJsonObject& params);
  static CommandResponse setArguments(const QString& id, const QJsonObject& params);
  static CommandResponse setWorkingDir(const QString& id, const QJsonObject& params);
  static CommandResponse setPipePath(const QString& id, const QJsonObject& params);
  static CommandResponse getRunningProcesses(const QString& id, const QJsonObject& params);
  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API

#endif  // BUILD_COMMERCIAL
