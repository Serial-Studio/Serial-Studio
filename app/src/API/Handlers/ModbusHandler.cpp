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

#ifdef BUILD_COMMERCIAL

#  include "API/Handlers/ModbusHandler.h"

#  include "API/CommandRegistry.h"
#  include "IO/Drivers/Modbus.h"

/**
 * @brief Register all Modbus commands with the registry
 */
void API::Handlers::ModbusHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  // Mutation commands
  registry.registerCommand(
    QStringLiteral("io.driver.modbus.setProtocolIndex"),
    QStringLiteral("Set Modbus protocol (params: protocolIndex - 0=RTU, 1=TCP)"),
    &setProtocolIndex);

  registry.registerCommand(QStringLiteral("io.driver.modbus.setSlaveAddress"),
                           QStringLiteral("Set Modbus slave address (params: address)"),
                           &setSlaveAddress);

  registry.registerCommand(
    QStringLiteral("io.driver.modbus.setPollInterval"),
    QStringLiteral("Set polling interval in milliseconds (params: intervalMs)"),
    &setPollInterval);

  registry.registerCommand(QStringLiteral("io.driver.modbus.setHost"),
                           QStringLiteral("Set TCP host (params: host)"),
                           &setHost);

  registry.registerCommand(QStringLiteral("io.driver.modbus.setPort"),
                           QStringLiteral("Set TCP port (params: port)"),
                           &setPort);

  registry.registerCommand(QStringLiteral("io.driver.modbus.setSerialPortIndex"),
                           QStringLiteral("Set RTU serial port by index (params: portIndex)"),
                           &setSerialPortIndex);

  registry.registerCommand(QStringLiteral("io.driver.modbus.setBaudRate"),
                           QStringLiteral("Set RTU baud rate (params: baudRate)"),
                           &setBaudRate);

  registry.registerCommand(QStringLiteral("io.driver.modbus.setParityIndex"),
                           QStringLiteral("Set RTU parity (params: parityIndex)"),
                           &setParityIndex);

  registry.registerCommand(QStringLiteral("io.driver.modbus.setDataBitsIndex"),
                           QStringLiteral("Set RTU data bits (params: dataBitsIndex)"),
                           &setDataBitsIndex);

  registry.registerCommand(QStringLiteral("io.driver.modbus.setStopBitsIndex"),
                           QStringLiteral("Set RTU stop bits (params: stopBitsIndex)"),
                           &setStopBitsIndex);

  registry.registerCommand(QStringLiteral("io.driver.modbus.addRegisterGroup"),
                           QStringLiteral("Add register group (params: type, startAddress, count)"),
                           &addRegisterGroup);

  registry.registerCommand(QStringLiteral("io.driver.modbus.removeRegisterGroup"),
                           QStringLiteral("Remove register group by index (params: groupIndex)"),
                           &removeRegisterGroup);

  registry.registerCommand(QStringLiteral("io.driver.modbus.clearRegisterGroups"),
                           QStringLiteral("Clear all register groups"),
                           &clearRegisterGroups);

  // Query commands
  registry.registerCommand(QStringLiteral("io.driver.modbus.getConfiguration"),
                           QStringLiteral("Get current Modbus configuration"),
                           &getConfiguration);

  registry.registerCommand(QStringLiteral("io.driver.modbus.getProtocolList"),
                           QStringLiteral("Get list of supported protocols"),
                           &getProtocolList);

  registry.registerCommand(QStringLiteral("io.driver.modbus.getSerialPortList"),
                           QStringLiteral("Get list of available serial ports"),
                           &getSerialPortList);

  registry.registerCommand(QStringLiteral("io.driver.modbus.getParityList"),
                           QStringLiteral("Get list of parity options"),
                           &getParityList);

  registry.registerCommand(QStringLiteral("io.driver.modbus.getDataBitsList"),
                           QStringLiteral("Get list of data bits options"),
                           &getDataBitsList);

  registry.registerCommand(QStringLiteral("io.driver.modbus.getStopBitsList"),
                           QStringLiteral("Get list of stop bits options"),
                           &getStopBitsList);

  registry.registerCommand(QStringLiteral("io.driver.modbus.getBaudRateList"),
                           QStringLiteral("Get list of baud rate options"),
                           &getBaudRateList);

  registry.registerCommand(QStringLiteral("io.driver.modbus.getRegisterTypeList"),
                           QStringLiteral("Get list of register type names"),
                           &getRegisterTypeList);

  registry.registerCommand(QStringLiteral("io.driver.modbus.getRegisterGroups"),
                           QStringLiteral("Get all configured register groups"),
                           &getRegisterGroups);
}

/**
 * @brief Set Modbus protocol (RTU or TCP)
 * @param params Requires "protocolIndex" (int: 0=RTU, 1=TCP)
 */
API::CommandResponse API::Handlers::ModbusHandler::setProtocolIndex(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("protocolIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: protocolIndex"));
  }

  const int protocolIndex  = params.value(QStringLiteral("protocolIndex")).toInt();
  auto& modbus             = IO::Drivers::Modbus::instance();
  const auto& protocolList = modbus.protocolList();

  if (protocolIndex < 0 || protocolIndex >= protocolList.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid protocolIndex: %1. Valid range: 0-%2")
                                        .arg(protocolIndex)
                                        .arg(protocolList.count() - 1));
  }

  modbus.setProtocolIndex(static_cast<quint8>(protocolIndex));

  QJsonObject result;
  result[QStringLiteral("protocolIndex")] = protocolIndex;
  result[QStringLiteral("protocolName")]  = protocolList.at(protocolIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set Modbus slave address
 * @param params Requires "address" (int: 1-247)
 */
API::CommandResponse API::Handlers::ModbusHandler::setSlaveAddress(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("address"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: address"));
  }

  const int address = params.value(QStringLiteral("address")).toInt();

  if (address < 1 || address > 247) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid address: %1. Valid range: 1-247").arg(address));
  }

  IO::Drivers::Modbus::instance().setSlaveAddress(static_cast<quint8>(address));

  QJsonObject result;
  result[QStringLiteral("address")] = address;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set polling interval in milliseconds
 * @param params Requires "intervalMs" (int)
 */
API::CommandResponse API::Handlers::ModbusHandler::setPollInterval(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("intervalMs"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: intervalMs"));
  }

  const int intervalMs = params.value(QStringLiteral("intervalMs")).toInt();

  if (intervalMs < 10) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("intervalMs must be at least 10 milliseconds"));
  }

  IO::Drivers::Modbus::instance().setPollInterval(static_cast<quint16>(intervalMs));

  QJsonObject result;
  result[QStringLiteral("intervalMs")] = intervalMs;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set TCP host address
 * @param params Requires "host" (string)
 */
API::CommandResponse API::Handlers::ModbusHandler::setHost(const QString& id,
                                                           const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("host"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: host"));
  }

  const QString host = params.value(QStringLiteral("host")).toString();

  if (host.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Host cannot be empty"));
  }

  IO::Drivers::Modbus::instance().setHost(host);

  QJsonObject result;
  result[QStringLiteral("host")] = host;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set TCP port
 * @param params Requires "port" (int)
 */
API::CommandResponse API::Handlers::ModbusHandler::setPort(const QString& id,
                                                           const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("port"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: port"));
  }

  const int port = params.value(QStringLiteral("port")).toInt();

  if (port < 1 || port > 65535) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid port: %1. Valid range: 1-65535").arg(port));
  }

  IO::Drivers::Modbus::instance().setPort(static_cast<quint16>(port));

  QJsonObject result;
  result[QStringLiteral("port")] = port;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set RTU serial port by index
 * @param params Requires "portIndex" (int)
 */
API::CommandResponse API::Handlers::ModbusHandler::setSerialPortIndex(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("portIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: portIndex"));
  }

  const int portIndex  = params.value(QStringLiteral("portIndex")).toInt();
  auto& modbus         = IO::Drivers::Modbus::instance();
  const auto& portList = modbus.serialPortList();

  if (portIndex < 0 || portIndex >= portList.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid portIndex: %1. Valid range: 0-%2")
                                        .arg(portIndex)
                                        .arg(portList.count() - 1));
  }

  modbus.setSerialPortIndex(static_cast<quint8>(portIndex));

  QJsonObject result;
  result[QStringLiteral("portIndex")] = portIndex;
  result[QStringLiteral("portName")]  = portList.at(portIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set RTU baud rate
 * @param params Requires "baudRate" (int)
 */
API::CommandResponse API::Handlers::ModbusHandler::setBaudRate(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("baudRate"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: baudRate"));
  }

  const int baudRate = params.value(QStringLiteral("baudRate")).toInt();

  if (baudRate <= 0) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("baudRate must be positive"));
  }

  IO::Drivers::Modbus::instance().setBaudRate(baudRate);

  QJsonObject result;
  result[QStringLiteral("baudRate")] = baudRate;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set RTU parity
 * @param params Requires "parityIndex" (int)
 */
API::CommandResponse API::Handlers::ModbusHandler::setParityIndex(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("parityIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: parityIndex"));
  }

  const int parityIndex  = params.value(QStringLiteral("parityIndex")).toInt();
  auto& modbus           = IO::Drivers::Modbus::instance();
  const auto& parityList = modbus.parityList();

  if (parityIndex < 0 || parityIndex >= parityList.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid parityIndex: %1. Valid range: 0-%2")
                                        .arg(parityIndex)
                                        .arg(parityList.count() - 1));
  }

  modbus.setParityIndex(static_cast<quint8>(parityIndex));

  QJsonObject result;
  result[QStringLiteral("parityIndex")] = parityIndex;
  result[QStringLiteral("parityName")]  = parityList.at(parityIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set RTU data bits
 * @param params Requires "dataBitsIndex" (int)
 */
API::CommandResponse API::Handlers::ModbusHandler::setDataBitsIndex(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("dataBitsIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: dataBitsIndex"));
  }

  const int dataBitsIndex  = params.value(QStringLiteral("dataBitsIndex")).toInt();
  auto& modbus             = IO::Drivers::Modbus::instance();
  const auto& dataBitsList = modbus.dataBitsList();

  if (dataBitsIndex < 0 || dataBitsIndex >= dataBitsList.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid dataBitsIndex: %1. Valid range: 0-%2")
                                        .arg(dataBitsIndex)
                                        .arg(dataBitsList.count() - 1));
  }

  modbus.setDataBitsIndex(static_cast<quint8>(dataBitsIndex));

  QJsonObject result;
  result[QStringLiteral("dataBitsIndex")] = dataBitsIndex;
  result[QStringLiteral("dataBitsValue")] = dataBitsList.at(dataBitsIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set RTU stop bits
 * @param params Requires "stopBitsIndex" (int)
 */
API::CommandResponse API::Handlers::ModbusHandler::setStopBitsIndex(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("stopBitsIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: stopBitsIndex"));
  }

  const int stopBitsIndex  = params.value(QStringLiteral("stopBitsIndex")).toInt();
  auto& modbus             = IO::Drivers::Modbus::instance();
  const auto& stopBitsList = modbus.stopBitsList();

  if (stopBitsIndex < 0 || stopBitsIndex >= stopBitsList.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid stopBitsIndex: %1. Valid range: 0-%2")
                                        .arg(stopBitsIndex)
                                        .arg(stopBitsList.count() - 1));
  }

  modbus.setStopBitsIndex(static_cast<quint8>(stopBitsIndex));

  QJsonObject result;
  result[QStringLiteral("stopBitsIndex")] = stopBitsIndex;
  result[QStringLiteral("stopBitsValue")] = stopBitsList.at(stopBitsIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Add a register group
 * @param params Requires "type" (int), "startAddress" (int), "count" (int)
 */
API::CommandResponse API::Handlers::ModbusHandler::addRegisterGroup(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("type")) || !params.contains(QStringLiteral("startAddress"))
      || !params.contains(QStringLiteral("count"))) {
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Missing required parameters: type, startAddress, count"));
  }

  const int type         = params.value(QStringLiteral("type")).toInt();
  const int startAddress = params.value(QStringLiteral("startAddress")).toInt();
  const int count        = params.value(QStringLiteral("count")).toInt();

  auto& modbus         = IO::Drivers::Modbus::instance();
  const auto& typeList = modbus.registerTypeList();

  if (type < 0 || type >= typeList.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid type: %1. Valid range: 0-%2").arg(type).arg(typeList.count() - 1));
  }

  if (startAddress < 0 || startAddress > 65535) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid startAddress: %1. Valid range: 0-65535").arg(startAddress));
  }

  if (count < 1 || count > 125) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid count: %1. Valid range: 1-125").arg(count));
  }

  modbus.addRegisterGroup(
    static_cast<quint8>(type), static_cast<quint16>(startAddress), static_cast<quint16>(count));

  QJsonObject result;
  result[QStringLiteral("type")]         = type;
  result[QStringLiteral("typeName")]     = typeList.at(type);
  result[QStringLiteral("startAddress")] = startAddress;
  result[QStringLiteral("count")]        = count;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Remove a register group by index
 * @param params Requires "groupIndex" (int)
 */
API::CommandResponse API::Handlers::ModbusHandler::removeRegisterGroup(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupIndex"));
  }

  const int groupIndex = params.value(QStringLiteral("groupIndex")).toInt();
  auto& modbus         = IO::Drivers::Modbus::instance();

  if (groupIndex < 0 || groupIndex >= modbus.registerGroupCount()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid groupIndex: %1. Valid range: 0-%2")
                                        .arg(groupIndex)
                                        .arg(modbus.registerGroupCount() - 1));
  }

  modbus.removeRegisterGroup(groupIndex);

  QJsonObject result;
  result[QStringLiteral("groupIndex")] = groupIndex;
  result[QStringLiteral("removed")]    = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Clear all register groups
 */
API::CommandResponse API::Handlers::ModbusHandler::clearRegisterGroups(const QString& id,
                                                                       const QJsonObject& params)
{
  Q_UNUSED(params)

  IO::Drivers::Modbus::instance().clearRegisterGroups();

  QJsonObject result;
  result[QStringLiteral("cleared")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get current Modbus configuration
 */
API::CommandResponse API::Handlers::ModbusHandler::getConfiguration(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& modbus = IO::Drivers::Modbus::instance();

  QJsonObject result;

  // Protocol info
  result[QStringLiteral("protocolIndex")] = modbus.protocolIndex();
  const auto& protocolList                = modbus.protocolList();
  if (modbus.protocolIndex() < protocolList.count())
    result[QStringLiteral("protocolName")] = protocolList.at(modbus.protocolIndex());

  // Common settings
  result[QStringLiteral("slaveAddress")] = modbus.slaveAddress();
  result[QStringLiteral("pollInterval")] = modbus.pollInterval();

  // TCP settings
  result[QStringLiteral("host")] = modbus.host();
  result[QStringLiteral("port")] = modbus.port();

  // RTU settings
  result[QStringLiteral("serialPortIndex")] = modbus.serialPortIndex();
  const auto& portList                      = modbus.serialPortList();
  if (modbus.serialPortIndex() < portList.count())
    result[QStringLiteral("serialPortName")] = portList.at(modbus.serialPortIndex());

  result[QStringLiteral("baudRate")] = modbus.baudRate();

  result[QStringLiteral("parityIndex")] = modbus.parityIndex();
  const auto& parityList                = modbus.parityList();
  if (modbus.parityIndex() < parityList.count())
    result[QStringLiteral("parityName")] = parityList.at(modbus.parityIndex());

  result[QStringLiteral("dataBitsIndex")] = modbus.dataBitsIndex();
  const auto& dataBitsList                = modbus.dataBitsList();
  if (modbus.dataBitsIndex() < dataBitsList.count())
    result[QStringLiteral("dataBitsValue")] = dataBitsList.at(modbus.dataBitsIndex());

  result[QStringLiteral("stopBitsIndex")] = modbus.stopBitsIndex();
  const auto& stopBitsList                = modbus.stopBitsList();
  if (modbus.stopBitsIndex() < stopBitsList.count())
    result[QStringLiteral("stopBitsValue")] = stopBitsList.at(modbus.stopBitsIndex());

  // Connection status
  result[QStringLiteral("isOpen")]          = modbus.isOpen();
  result[QStringLiteral("configurationOk")] = modbus.configurationOk();

  // Register groups
  result[QStringLiteral("registerGroupCount")] = modbus.registerGroupCount();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of supported protocols
 */
API::CommandResponse API::Handlers::ModbusHandler::getProtocolList(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& protocolList = IO::Drivers::Modbus::instance().protocolList();

  QJsonArray protocols;
  for (int i = 0; i < protocolList.count(); ++i) {
    QJsonObject protocol;
    protocol[QStringLiteral("index")] = i;
    protocol[QStringLiteral("name")]  = protocolList.at(i);
    protocols.append(protocol);
  }

  QJsonObject result;
  result[QStringLiteral("protocolList")]         = protocols;
  result[QStringLiteral("currentProtocolIndex")] = IO::Drivers::Modbus::instance().protocolIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of available serial ports
 */
API::CommandResponse API::Handlers::ModbusHandler::getSerialPortList(const QString& id,
                                                                     const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& portList = IO::Drivers::Modbus::instance().serialPortList();

  QJsonArray ports;
  for (int i = 0; i < portList.count(); ++i) {
    QJsonObject port;
    port[QStringLiteral("index")] = i;
    port[QStringLiteral("name")]  = portList.at(i);
    ports.append(port);
  }

  QJsonObject result;
  result[QStringLiteral("serialPortList")] = ports;
  result[QStringLiteral("currentSerialPortIndex")] =
    IO::Drivers::Modbus::instance().serialPortIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of parity options
 */
API::CommandResponse API::Handlers::ModbusHandler::getParityList(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& parityList = IO::Drivers::Modbus::instance().parityList();

  QJsonArray parities;
  for (int i = 0; i < parityList.count(); ++i) {
    QJsonObject parity;
    parity[QStringLiteral("index")] = i;
    parity[QStringLiteral("name")]  = parityList.at(i);
    parities.append(parity);
  }

  QJsonObject result;
  result[QStringLiteral("parityList")]         = parities;
  result[QStringLiteral("currentParityIndex")] = IO::Drivers::Modbus::instance().parityIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of data bits options
 */
API::CommandResponse API::Handlers::ModbusHandler::getDataBitsList(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& dataBitsList = IO::Drivers::Modbus::instance().dataBitsList();

  QJsonArray dataBitsArray;
  for (int i = 0; i < dataBitsList.count(); ++i) {
    QJsonObject dataBits;
    dataBits[QStringLiteral("index")] = i;
    dataBits[QStringLiteral("value")] = dataBitsList.at(i);
    dataBitsArray.append(dataBits);
  }

  QJsonObject result;
  result[QStringLiteral("dataBitsList")]         = dataBitsArray;
  result[QStringLiteral("currentDataBitsIndex")] = IO::Drivers::Modbus::instance().dataBitsIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of stop bits options
 */
API::CommandResponse API::Handlers::ModbusHandler::getStopBitsList(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& stopBitsList = IO::Drivers::Modbus::instance().stopBitsList();

  QJsonArray stopBitsArray;
  for (int i = 0; i < stopBitsList.count(); ++i) {
    QJsonObject stopBits;
    stopBits[QStringLiteral("index")] = i;
    stopBits[QStringLiteral("value")] = stopBitsList.at(i);
    stopBitsArray.append(stopBits);
  }

  QJsonObject result;
  result[QStringLiteral("stopBitsList")]         = stopBitsArray;
  result[QStringLiteral("currentStopBitsIndex")] = IO::Drivers::Modbus::instance().stopBitsIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of baud rate options
 */
API::CommandResponse API::Handlers::ModbusHandler::getBaudRateList(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& baudRateList = IO::Drivers::Modbus::instance().baudRateList();

  QJsonArray baudRates;
  for (const auto& rate : baudRateList)
    baudRates.append(rate.toInt());

  QJsonObject result;
  result[QStringLiteral("baudRateList")]    = baudRates;
  result[QStringLiteral("currentBaudRate")] = IO::Drivers::Modbus::instance().baudRate();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of register type names
 */
API::CommandResponse API::Handlers::ModbusHandler::getRegisterTypeList(const QString& id,
                                                                       const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& typeList = IO::Drivers::Modbus::instance().registerTypeList();

  QJsonArray types;
  for (int i = 0; i < typeList.count(); ++i) {
    QJsonObject type;
    type[QStringLiteral("index")] = i;
    type[QStringLiteral("name")]  = typeList.at(i);
    types.append(type);
  }

  QJsonObject result;
  result[QStringLiteral("registerTypeList")] = types;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get all configured register groups
 */
API::CommandResponse API::Handlers::ModbusHandler::getRegisterGroups(const QString& id,
                                                                     const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& modbus         = IO::Drivers::Modbus::instance();
  const int groupCount = modbus.registerGroupCount();

  QJsonArray groups;
  for (int i = 0; i < groupCount; ++i) {
    QJsonObject group;
    group[QStringLiteral("index")] = i;
    group[QStringLiteral("info")]  = modbus.registerGroupInfo(i);
    groups.append(group);
  }

  QJsonObject result;
  result[QStringLiteral("registerGroups")] = groups;
  result[QStringLiteral("groupCount")]     = groupCount;
  return CommandResponse::makeSuccess(id, result);
}

#endif  // BUILD_COMMERCIAL
