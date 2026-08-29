/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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
 * @brief Registers API commands for IO::Drivers::EthernetIp operations (Pro feature).
 */
class EipHandler {
public:
  static void registerCommands();

private:
  static CommandResponse getStatus(const QString& id, const QJsonObject& params);
  static CommandResponse getConfig(const QString& id, const QJsonObject& params);
  static CommandResponse setProperty(const QString& id, const QJsonObject& params);
  static CommandResponse addTag(const QString& id, const QJsonObject& params);
  static CommandResponse removeTag(const QString& id, const QJsonObject& params);
  static CommandResponse clearTags(const QString& id, const QJsonObject& params);
  static CommandResponse generateProject(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
