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

namespace API
{
namespace Handlers
{
/**
 * @class BluetoothLEHandler
 * @brief Registers API commands for IO::Drivers::BluetoothLE operations
 *
 * Provides commands for:
 * - io.driver.ble.startDiscovery - Start scanning for BLE devices
 * - io.driver.ble.selectDevice - Select device by index
 * - io.driver.ble.selectService - Select service by index
 * - io.driver.ble.setCharacteristicIndex - Select characteristic
 * - io.driver.ble.getDeviceList - Query discovered devices
 * - io.driver.ble.getServiceList - Query services for selected device
 * - io.driver.ble.getCharacteristicList - Query characteristics
 * - io.driver.ble.getConfiguration - Query current BLE configuration
 * - io.driver.ble.getStatus - Query adapter and connection status
 */
class BluetoothLEHandler
{
public:
  /**
   * @brief Register all BluetoothLE commands with the CommandRegistry
   */
  static void registerCommands();

private:
  // Mutation commands
  static CommandResponse startDiscovery(const QString &id,
                                        const QJsonObject &params);
  static CommandResponse selectDevice(const QString &id,
                                      const QJsonObject &params);
  static CommandResponse selectService(const QString &id,
                                       const QJsonObject &params);
  static CommandResponse setCharacteristicIndex(const QString &id,
                                                const QJsonObject &params);

  // Query commands
  static CommandResponse getDeviceList(const QString &id,
                                       const QJsonObject &params);
  static CommandResponse getServiceList(const QString &id,
                                        const QJsonObject &params);
  static CommandResponse getCharacteristicList(const QString &id,
                                               const QJsonObject &params);
  static CommandResponse getConfiguration(const QString &id,
                                          const QJsonObject &params);
  static CommandResponse getStatus(const QString &id,
                                   const QJsonObject &params);
};

} // namespace Handlers
} // namespace API
