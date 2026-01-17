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

namespace API
{
namespace Handlers
{
/**
 * @class ModbusHandler
 * @brief Registers API commands for IO::Drivers::Modbus operations
 *
 * Provides commands for:
 * - io.driver.modbus.setProtocolIndex - Set RTU/TCP protocol
 * - io.driver.modbus.setSlaveAddress - Set slave address
 * - io.driver.modbus.setPollInterval - Set polling interval
 * - io.driver.modbus.setHost - Set TCP host
 * - io.driver.modbus.setPort - Set TCP port
 * - io.driver.modbus.setSerialPortIndex - Set RTU serial port
 * - io.driver.modbus.setBaudRate - Set baud rate
 * - io.driver.modbus.setParityIndex - Set parity
 * - io.driver.modbus.setDataBitsIndex - Set data bits
 * - io.driver.modbus.setStopBitsIndex - Set stop bits
 * - io.driver.modbus.addRegisterGroup - Add register group
 * - io.driver.modbus.removeRegisterGroup - Remove register group
 * - io.driver.modbus.clearRegisterGroups - Clear all register groups
 * - io.driver.modbus.getConfiguration - Query configuration
 * - io.driver.modbus.getProtocolList - Query protocol list
 * - io.driver.modbus.getSerialPortList - Query serial ports
 * - io.driver.modbus.getParityList - Query parity options
 * - io.driver.modbus.getDataBitsList - Query data bits options
 * - io.driver.modbus.getStopBitsList - Query stop bits options
 * - io.driver.modbus.getBaudRateList - Query baud rate options
 * - io.driver.modbus.getRegisterTypeList - Query register type names
 * - io.driver.modbus.getRegisterGroups - Query register groups
 */
class ModbusHandler
{
public:
  /**
   * @brief Register all Modbus commands with the CommandRegistry
   */
  static void registerCommands();

private:
  // Mutation commands
  static CommandResponse setProtocolIndex(const QString &id,
                                          const QJsonObject &params);
  static CommandResponse setSlaveAddress(const QString &id,
                                         const QJsonObject &params);
  static CommandResponse setPollInterval(const QString &id,
                                         const QJsonObject &params);
  static CommandResponse setHost(const QString &id, const QJsonObject &params);
  static CommandResponse setPort(const QString &id, const QJsonObject &params);
  static CommandResponse setSerialPortIndex(const QString &id,
                                            const QJsonObject &params);
  static CommandResponse setBaudRate(const QString &id,
                                     const QJsonObject &params);
  static CommandResponse setParityIndex(const QString &id,
                                        const QJsonObject &params);
  static CommandResponse setDataBitsIndex(const QString &id,
                                          const QJsonObject &params);
  static CommandResponse setStopBitsIndex(const QString &id,
                                          const QJsonObject &params);
  static CommandResponse addRegisterGroup(const QString &id,
                                          const QJsonObject &params);
  static CommandResponse removeRegisterGroup(const QString &id,
                                             const QJsonObject &params);
  static CommandResponse clearRegisterGroups(const QString &id,
                                             const QJsonObject &params);

  // Query commands
  static CommandResponse getConfiguration(const QString &id,
                                          const QJsonObject &params);
  static CommandResponse getProtocolList(const QString &id,
                                         const QJsonObject &params);
  static CommandResponse getSerialPortList(const QString &id,
                                           const QJsonObject &params);
  static CommandResponse getParityList(const QString &id,
                                       const QJsonObject &params);
  static CommandResponse getDataBitsList(const QString &id,
                                         const QJsonObject &params);
  static CommandResponse getStopBitsList(const QString &id,
                                         const QJsonObject &params);
  static CommandResponse getBaudRateList(const QString &id,
                                         const QJsonObject &params);
  static CommandResponse getRegisterTypeList(const QString &id,
                                             const QJsonObject &params);
  static CommandResponse getRegisterGroups(const QString &id,
                                           const QJsonObject &params);
};

} // namespace Handlers
} // namespace API

#endif // BUILD_COMMERCIAL
