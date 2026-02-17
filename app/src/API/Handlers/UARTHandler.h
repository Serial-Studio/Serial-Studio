/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include "API/CommandProtocol.h"

namespace API {
namespace Handlers {
/**
 * @class UARTHandler
 * @brief Registers API commands for IO::Drivers::UART operations
 *
 * Provides commands for:
 * - io.driver.uart.setDevice - Set port by device name
 * - io.driver.uart.setPortIndex - Set port by index
 * - io.driver.uart.setBaudRate - Set baud rate
 * - io.driver.uart.setParity - Set parity
 * - io.driver.uart.setDataBits - Set data bits
 * - io.driver.uart.setStopBits - Set stop bits
 * - io.driver.uart.setFlowControl - Set flow control
 * - io.driver.uart.setDtrEnabled - Enable/disable DTR
 * - io.driver.uart.setAutoReconnect - Set auto-reconnect behavior
 * - io.driver.uart.getPortList - Query available ports
 * - io.driver.uart.getBaudRateList - Query available baud rates
 * - io.driver.uart.getConfiguration - Query current configuration
 */
class UARTHandler {
public:
  /**
   * @brief Register all UART commands with the CommandRegistry
   */
  static void registerCommands();

private:
  // Mutation commands
  static CommandResponse setDevice(const QString& id, const QJsonObject& params);
  static CommandResponse setPortIndex(const QString& id, const QJsonObject& params);
  static CommandResponse setBaudRate(const QString& id, const QJsonObject& params);
  static CommandResponse setParity(const QString& id, const QJsonObject& params);
  static CommandResponse setDataBits(const QString& id, const QJsonObject& params);
  static CommandResponse setStopBits(const QString& id, const QJsonObject& params);
  static CommandResponse setFlowControl(const QString& id, const QJsonObject& params);
  static CommandResponse setDtrEnabled(const QString& id, const QJsonObject& params);
  static CommandResponse setAutoReconnect(const QString& id, const QJsonObject& params);

  // Query commands
  static CommandResponse getPortList(const QString& id, const QJsonObject& params);
  static CommandResponse getBaudRateList(const QString& id, const QJsonObject& params);
  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
