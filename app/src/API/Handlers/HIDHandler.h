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
 * @class HIDHandler
 * @brief Registers API commands for IO::Drivers::HID operations (Pro feature)
 *
 * Provides commands for HID device driver configuration:
 *
 * Configuration:
 * - io.driver.hid.setDeviceIndex - Select HID device by index
 *
 * Queries:
 * - io.driver.hid.getDeviceList  - List available HID devices
 * - io.driver.hid.getConfiguration - Get complete HID config and info
 */
class HIDHandler {
public:
  static void registerCommands();

private:
  static CommandResponse setDeviceIndex(const QString& id, const QJsonObject& params);
  static CommandResponse getDeviceList(const QString& id, const QJsonObject& params);
  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
