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
 * @brief Registers API commands for MDF4::Player operations (Pro feature).
 */
class MDF4PlayerHandler {
public:
  static void registerCommands();

private:
  static CommandResponse open(const QString& id, const QJsonObject& params);
  static CommandResponse close(const QString& id, const QJsonObject& params);

  static CommandResponse play(const QString& id, const QJsonObject& params);
  static CommandResponse pause(const QString& id, const QJsonObject& params);
  static CommandResponse toggle(const QString& id, const QJsonObject& params);
  static CommandResponse nextFrame(const QString& id, const QJsonObject& params);
  static CommandResponse previousFrame(const QString& id, const QJsonObject& params);
  static CommandResponse setProgress(const QString& id, const QJsonObject& params);

  static CommandResponse getStatus(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
