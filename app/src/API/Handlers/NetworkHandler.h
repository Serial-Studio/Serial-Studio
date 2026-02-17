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
 * @class NetworkHandler
 * @brief Registers API commands for IO::Drivers::Network operations
 *
 * Provides commands for:
 * - io.driver.network.setRemoteAddress - Set remote host address
 * - io.driver.network.setTcpPort - Set TCP port
 * - io.driver.network.setUdpLocalPort - Set UDP local port
 * - io.driver.network.setUdpRemotePort - Set UDP remote port
 * - io.driver.network.setSocketType - Set socket type (TCP/UDP)
 * - io.driver.network.setUdpMulticast - Enable/disable UDP multicast
 * - io.driver.network.lookup - Perform DNS lookup
 * - io.driver.network.getConfiguration - Query current configuration
 * - io.driver.network.getSocketTypes - Query available socket types
 */
class NetworkHandler {
public:
  /**
   * @brief Register all Network commands with the CommandRegistry
   */
  static void registerCommands();

private:
  // Mutation commands
  static CommandResponse setRemoteAddress(const QString& id, const QJsonObject& params);
  static CommandResponse setTcpPort(const QString& id, const QJsonObject& params);
  static CommandResponse setUdpLocalPort(const QString& id, const QJsonObject& params);
  static CommandResponse setUdpRemotePort(const QString& id, const QJsonObject& params);
  static CommandResponse setSocketType(const QString& id, const QJsonObject& params);
  static CommandResponse setUdpMulticast(const QString& id, const QJsonObject& params);
  static CommandResponse lookup(const QString& id, const QJsonObject& params);

  // Query commands
  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
  static CommandResponse getSocketTypes(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
