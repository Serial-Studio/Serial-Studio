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
 * @brief Registers API commands for IO::Drivers::USB operations (Pro feature).
 */
class USBHandler {
public:
  static void registerCommands();

private:
  static CommandResponse setDeviceIndex(const QString& id, const QJsonObject& params);
  static CommandResponse setTransferMode(const QString& id, const QJsonObject& params);
  static CommandResponse setInEndpointIndex(const QString& id, const QJsonObject& params);
  static CommandResponse setOutEndpointIndex(const QString& id, const QJsonObject& params);
  static CommandResponse setIsoPacketSize(const QString& id, const QJsonObject& params);
  static CommandResponse getDeviceList(const QString& id, const QJsonObject& params);
  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
