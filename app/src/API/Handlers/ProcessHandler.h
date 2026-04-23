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

#include "API/CommandProtocol.h"

namespace API {
namespace Handlers {

/**
 * @brief Registers API commands for IO::Drivers::Process operations (Pro feature).
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
