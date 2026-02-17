/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include "API/CommandProtocol.h"

namespace API {
namespace Handlers {
/**
 * @class CANBusHandler
 * @brief Registers API commands for IO::Drivers::CANBus operations
 *
 * Provides commands for:
 * - io.driver.canbus.setPluginIndex - Select CAN plugin
 * - io.driver.canbus.setInterfaceIndex - Select CAN interface
 * - io.driver.canbus.setBitrate - Set CAN bitrate
 * - io.driver.canbus.setCanFD - Enable/disable CAN FD
 * - io.driver.canbus.getConfiguration - Query configuration
 * - io.driver.canbus.getPluginList - Query available CAN plugins
 * - io.driver.canbus.getInterfaceList - Query available interfaces
 * - io.driver.canbus.getBitrateList - Query supported bitrates
 * - io.driver.canbus.getInterfaceError - Query interface error message
 */
class CANBusHandler {
public:
  /**
   * @brief Register all CANBus commands with the CommandRegistry
   */
  static void registerCommands();

private:
  // Mutation commands
  static CommandResponse setPluginIndex(const QString& id, const QJsonObject& params);
  static CommandResponse setInterfaceIndex(const QString& id, const QJsonObject& params);
  static CommandResponse setBitrate(const QString& id, const QJsonObject& params);
  static CommandResponse setCanFD(const QString& id, const QJsonObject& params);

  // Query commands
  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
  static CommandResponse getPluginList(const QString& id, const QJsonObject& params);
  static CommandResponse getInterfaceList(const QString& id, const QJsonObject& params);
  static CommandResponse getBitrateList(const QString& id, const QJsonObject& params);
  static CommandResponse getInterfaceError(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API

#endif  // BUILD_COMMERCIAL
