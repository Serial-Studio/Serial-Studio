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

#include "API/Handlers/BluetoothLEHandler.h"
#include "API/CommandRegistry.h"
#include "IO/Drivers/BluetoothLE.h"

/**
 * @brief Register all BluetoothLE commands with the registry
 */
void API::Handlers::BluetoothLEHandler::registerCommands()
{
  auto &registry = CommandRegistry::instance();

  // Mutation commands
  registry.registerCommand(
      QStringLiteral("io.driver.ble.startDiscovery"),
      QStringLiteral("Start scanning for Bluetooth LE devices"),
      &startDiscovery);

  registry.registerCommand(
      QStringLiteral("io.driver.ble.selectDevice"),
      QStringLiteral("Select BLE device by index (params: deviceIndex)"),
      &selectDevice);

  registry.registerCommand(
      QStringLiteral("io.driver.ble.selectService"),
      QStringLiteral("Select BLE service by index (params: serviceIndex)"),
      &selectService);

  registry.registerCommand(
      QStringLiteral("io.driver.ble.setCharacteristicIndex"),
      QStringLiteral(
          "Select BLE characteristic by index (params: characteristicIndex)"),
      &setCharacteristicIndex);

  // Query commands
  registry.registerCommand(
      QStringLiteral("io.driver.ble.getDeviceList"),
      QStringLiteral("Get list of discovered Bluetooth LE devices"),
      &getDeviceList);

  registry.registerCommand(
      QStringLiteral("io.driver.ble.getServiceList"),
      QStringLiteral("Get list of services for the selected BLE device"),
      &getServiceList);

  registry.registerCommand(
      QStringLiteral("io.driver.ble.getCharacteristicList"),
      QStringLiteral(
          "Get list of characteristics for the selected BLE service"),
      &getCharacteristicList);

  registry.registerCommand(
      QStringLiteral("io.driver.ble.getConfiguration"),
      QStringLiteral("Get current Bluetooth LE configuration"),
      &getConfiguration);

  registry.registerCommand(
      QStringLiteral("io.driver.ble.getStatus"),
      QStringLiteral(
          "Get Bluetooth adapter availability and connection status"),
      &getStatus);
}

/**
 * @brief Start scanning for Bluetooth LE devices
 */
API::CommandResponse
API::Handlers::BluetoothLEHandler::startDiscovery(const QString &id,
                                                  const QJsonObject &params)
{
  Q_UNUSED(params)

  auto &ble = IO::Drivers::BluetoothLE::instance();

  if (!ble.operatingSystemSupported())
  {
    return CommandResponse::makeError(
        id, ErrorCode::OperationFailed,
        QStringLiteral("Bluetooth LE is not supported on this operating "
                       "system"));
  }

  if (!ble.adapterAvailable())
  {
    return CommandResponse::makeError(
        id, ErrorCode::OperationFailed,
        QStringLiteral("No Bluetooth adapter available"));
  }

  ble.startDiscovery();

  QJsonObject result;
  result[QStringLiteral("discoveryStarted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select BLE device by index
 * @param params Requires "deviceIndex" (int)
 */
API::CommandResponse
API::Handlers::BluetoothLEHandler::selectDevice(const QString &id,
                                                const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("deviceIndex")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: deviceIndex"));
  }

  const int deviceIndex = params.value(QStringLiteral("deviceIndex")).toInt();
  auto &ble = IO::Drivers::BluetoothLE::instance();

  if (deviceIndex < 0 || deviceIndex >= ble.deviceCount())
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("Invalid deviceIndex: %1. Valid range: 0-%2")
            .arg(deviceIndex)
            .arg(ble.deviceCount() - 1));
  }

  ble.selectDevice(deviceIndex);

  QJsonObject result;
  result[QStringLiteral("deviceIndex")] = deviceIndex;
  const auto &deviceNames = ble.deviceNames();
  if (deviceIndex < deviceNames.count())
    result[QStringLiteral("deviceName")] = deviceNames.at(deviceIndex);

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select BLE service by index
 * @param params Requires "serviceIndex" (int)
 */
API::CommandResponse
API::Handlers::BluetoothLEHandler::selectService(const QString &id,
                                                 const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("serviceIndex")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: serviceIndex"));
  }

  const int serviceIndex = params.value(QStringLiteral("serviceIndex")).toInt();
  auto &ble = IO::Drivers::BluetoothLE::instance();
  const auto &serviceNames = ble.serviceNames();

  if (serviceIndex < 0 || serviceIndex >= serviceNames.count())
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("Invalid serviceIndex: %1. Valid range: 0-%2")
            .arg(serviceIndex)
            .arg(serviceNames.count() - 1));
  }

  ble.selectService(serviceIndex);

  QJsonObject result;
  result[QStringLiteral("serviceIndex")] = serviceIndex;
  result[QStringLiteral("serviceName")] = serviceNames.at(serviceIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select BLE characteristic by index
 * @param params Requires "characteristicIndex" (int)
 */
API::CommandResponse API::Handlers::BluetoothLEHandler::setCharacteristicIndex(
    const QString &id, const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("characteristicIndex")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: characteristicIndex"));
  }

  const int characteristicIndex
      = params.value(QStringLiteral("characteristicIndex")).toInt();
  auto &ble = IO::Drivers::BluetoothLE::instance();
  const auto &characteristicNames = ble.characteristicNames();

  if (characteristicIndex < 0
      || characteristicIndex >= characteristicNames.count())
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("Invalid characteristicIndex: %1. Valid range: 0-%2")
            .arg(characteristicIndex)
            .arg(characteristicNames.count() - 1));
  }

  ble.setCharacteristicIndex(characteristicIndex);

  QJsonObject result;
  result[QStringLiteral("characteristicIndex")] = characteristicIndex;
  result[QStringLiteral("characteristicName")]
      = characteristicNames.at(characteristicIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of discovered Bluetooth LE devices
 */
API::CommandResponse
API::Handlers::BluetoothLEHandler::getDeviceList(const QString &id,
                                                 const QJsonObject &params)
{
  Q_UNUSED(params)

  auto &ble = IO::Drivers::BluetoothLE::instance();
  const auto &deviceNames = ble.deviceNames();

  QJsonArray devices;
  for (int i = 0; i < deviceNames.count(); ++i)
  {
    QJsonObject device;
    device[QStringLiteral("index")] = i;
    device[QStringLiteral("name")] = deviceNames.at(i);
    devices.append(device);
  }

  QJsonObject result;
  result[QStringLiteral("deviceList")] = devices;
  result[QStringLiteral("currentDeviceIndex")] = ble.deviceIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of services for the selected BLE device
 */
API::CommandResponse
API::Handlers::BluetoothLEHandler::getServiceList(const QString &id,
                                                  const QJsonObject &params)
{
  Q_UNUSED(params)

  auto &ble = IO::Drivers::BluetoothLE::instance();
  const auto &serviceNames = ble.serviceNames();

  QJsonArray services;
  for (int i = 0; i < serviceNames.count(); ++i)
  {
    QJsonObject service;
    service[QStringLiteral("index")] = i;
    service[QStringLiteral("name")] = serviceNames.at(i);
    services.append(service);
  }

  QJsonObject result;
  result[QStringLiteral("serviceList")] = services;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of characteristics for the selected BLE service
 */
API::CommandResponse API::Handlers::BluetoothLEHandler::getCharacteristicList(
    const QString &id, const QJsonObject &params)
{
  Q_UNUSED(params)

  auto &ble = IO::Drivers::BluetoothLE::instance();
  const auto &characteristicNames = ble.characteristicNames();

  QJsonArray characteristics;
  for (int i = 0; i < characteristicNames.count(); ++i)
  {
    QJsonObject characteristic;
    characteristic[QStringLiteral("index")] = i;
    characteristic[QStringLiteral("name")] = characteristicNames.at(i);
    characteristics.append(characteristic);
  }

  QJsonObject result;
  result[QStringLiteral("characteristicList")] = characteristics;
  result[QStringLiteral("currentCharacteristicIndex")]
      = ble.characteristicIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get current Bluetooth LE configuration
 */
API::CommandResponse
API::Handlers::BluetoothLEHandler::getConfiguration(const QString &id,
                                                    const QJsonObject &params)
{
  Q_UNUSED(params)

  auto &ble = IO::Drivers::BluetoothLE::instance();

  QJsonObject result;

  // Device info
  result[QStringLiteral("deviceIndex")] = ble.deviceIndex();
  const auto &deviceNames = ble.deviceNames();
  if (ble.deviceIndex() >= 0 && ble.deviceIndex() < deviceNames.count())
    result[QStringLiteral("deviceName")] = deviceNames.at(ble.deviceIndex());

  // Characteristic info
  result[QStringLiteral("characteristicIndex")] = ble.characteristicIndex();
  const auto &characteristicNames = ble.characteristicNames();
  if (ble.characteristicIndex() >= 0
      && ble.characteristicIndex() < characteristicNames.count())
    result[QStringLiteral("characteristicName")]
        = characteristicNames.at(ble.characteristicIndex());

  // Connection status
  result[QStringLiteral("isOpen")] = ble.isOpen();
  result[QStringLiteral("configurationOk")] = ble.configurationOk();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get Bluetooth adapter availability and connection status
 */
API::CommandResponse
API::Handlers::BluetoothLEHandler::getStatus(const QString &id,
                                             const QJsonObject &params)
{
  Q_UNUSED(params)

  auto &ble = IO::Drivers::BluetoothLE::instance();

  QJsonObject result;
  result[QStringLiteral("operatingSystemSupported")]
      = ble.operatingSystemSupported();
  result[QStringLiteral("adapterAvailable")] = ble.adapterAvailable();
  result[QStringLiteral("isOpen")] = ble.isOpen();
  result[QStringLiteral("deviceCount")] = ble.deviceCount();

  return CommandResponse::makeSuccess(id, result);
}
