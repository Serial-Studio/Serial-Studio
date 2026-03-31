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
 * @class USBHandler
 * @brief Registers API commands for IO::Drivers::USB operations (Pro feature)
 *
 * Provides commands for raw USB driver configuration:
 *
 * Configuration:
 * - io.driver.usb.setDeviceIndex     - Select USB device by index
 * - io.driver.usb.setTransferMode   - Set transfer mode (0=Bulk, 1=Advanced, 2=ISO)
 * - io.driver.usb.setInEndpointIndex  - Select IN endpoint (post-connect)
 * - io.driver.usb.setOutEndpointIndex - Select OUT endpoint (post-connect)
 * - io.driver.usb.setIsoPacketSize   - Set ISO packet size (post-connect)
 *
 * Queries:
 * - io.driver.usb.getDeviceList     - List available USB devices
 * - io.driver.usb.getConfiguration  - Get complete USB config
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
