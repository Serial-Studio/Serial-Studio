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

#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all BluetoothLE commands with the registry
 */
void API::Handlers::BluetoothLEHandler::registerCommands()
{
  registerSelectionCommands();
  registerQueryCommands();
}

/**
 * @brief Register discovery, device/service/characteristic selection, and write commands.
 */
void API::Handlers::BluetoothLEHandler::registerSelectionCommands()
{
  auto& registry   = CommandRegistry::instance();
  const auto empty = emptySchema();

  registry.registerCommand(QStringLiteral("io.ble.startDiscovery"),
                           QStringLiteral("Start scanning for Bluetooth LE devices"),
                           empty,
                           &startDiscovery);

  registry.registerCommand(QStringLiteral("io.ble.selectDevice"),
                           QStringLiteral("Select BLE device by index (params: deviceIndex)"),
                           makeSchema({
                             {QStringLiteral("deviceIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("Index of the BLE device to select")}
  }),
                           &selectDevice);

  registry.registerCommand(QStringLiteral("io.ble.selectService"),
                           QStringLiteral("Select BLE service by index (params: serviceIndex)"),
                           makeSchema({
                             {QStringLiteral("serviceIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("Index of the BLE service to select")}
  }),
                           &selectService);

  registry.registerCommand(
    QStringLiteral("io.ble.setCharacteristicIndex"),
    QStringLiteral("Select BLE characteristic by index (params: characteristicIndex)"),
    makeSchema({
      {QStringLiteral("characteristicIndex"),
       QStringLiteral("integer"),
       QStringLiteral("Index of the BLE characteristic to select")}
  }),
    &setCharacteristicIndex);

  registry.registerCommand(
    QStringLiteral("io.ble.selectServiceByUuid"),
    QStringLiteral("Select the BLE service whose UUID matches (params: serviceUuid - 16-bit "
                   "short form like 'fff0' or full 128-bit UUID). Robust alternative to "
                   "selectService by index. The device must be connected."),
    makeSchema({
      {QStringLiteral("serviceUuid"),
       QStringLiteral("string"),
       QStringLiteral("Service UUID (short or full form)")}
  }),
    &selectServiceByUuid);

  registry.registerCommand(
    QStringLiteral("io.ble.setNotifyCharacteristic"),
    QStringLiteral("Subscribe to the notify characteristic with the given UUID for incoming "
                   "data (params: characteristicUuid - e.g. 'fff2'). Pairs with "
                   "io.ble.writeCharacteristic for split read/write devices. A service must be "
                   "selected first."),
    makeSchema({
      {QStringLiteral("characteristicUuid"),
       QStringLiteral("string"),
       QStringLiteral("Notify characteristic UUID (short or full form)")}
  }),
    &setNotifyCharacteristic);

  registry.registerCommand(
    QStringLiteral("io.ble.writeCharacteristic"),
    QStringLiteral("Write raw bytes to a BLE characteristic resolved by UUID, independent of "
                   "the selected notify characteristic (params: characteristicUuid - 16-bit "
                   "short form like 'fff1' or full 128-bit UUID; data - base64-encoded bytes). "
                   "Lets a control script drive split read/write devices. The device must be "
                   "connected and the owning service selected."),
    makeSchema({
      SchemaProp{QStringLiteral("characteristicUuid"),
                 QStringLiteral("string"),
                 QStringLiteral("Target characteristic UUID (short or full form)")},
      byteProp(QStringLiteral("data"),
               QStringLiteral("Bytes to write, tagged with an encoding argument "
                              "(SerialStudio.Hex / .Text / .Base64) in the SDK wrapper"))
  }),
    &writeCharacteristic);
}

/**
 * @brief Register the list/get query commands.
 */
void API::Handlers::BluetoothLEHandler::registerQueryCommands()
{
  auto& registry   = CommandRegistry::instance();
  const auto empty = emptySchema();

  registry.registerCommand(QStringLiteral("io.ble.listDevices"),
                           QStringLiteral("Get list of discovered Bluetooth LE devices"),
                           empty,
                           &getDeviceList);

  registry.registerCommand(QStringLiteral("io.ble.listServices"),
                           QStringLiteral("Get list of services for the selected BLE device"),
                           empty,
                           &getServiceList);

  registry.registerCommand(
    QStringLiteral("io.ble.listCharacteristics"),
    QStringLiteral("Get list of characteristics for the selected BLE service"),
    empty,
    &getCharacteristicList);

  registry.registerCommand(QStringLiteral("io.ble.getConfig"),
                           QStringLiteral("Get current Bluetooth LE configuration"),
                           empty,
                           &getConfiguration);

  registry.registerCommand(
    QStringLiteral("io.ble.getStatus"),
    QStringLiteral("Get Bluetooth adapter availability and connection status"),
    empty,
    &getStatus);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Start scanning for Bluetooth LE devices
 */
API::CommandResponse API::Handlers::BluetoothLEHandler::startDiscovery(const QString& id,
                                                                       const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* ble = IO::ConnectionManager::instance().bluetoothLE();

  if (!ble->operatingSystemSupported()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::OperationFailed,
      QStringLiteral("Bluetooth LE is not supported on this operating system"));
  }

  if (!ble->adapterAvailable()) {
    return CommandResponse::makeError(
      id, ErrorCode::OperationFailed, QStringLiteral("No Bluetooth adapter available"));
  }

  ble->startDiscovery();

  QJsonObject result;
  result[QStringLiteral("discoveryStarted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select BLE device by index
 */
API::CommandResponse API::Handlers::BluetoothLEHandler::selectDevice(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("deviceIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: deviceIndex"));
  }

  const int deviceIndex = params.value(QStringLiteral("deviceIndex")).toInt();
  auto* ble             = IO::ConnectionManager::instance().bluetoothLE();

  if (deviceIndex < 0 || deviceIndex >= ble->deviceCount()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid deviceIndex: %1. Valid range: 0-%2")
                                        .arg(deviceIndex)
                                        .arg(ble->deviceCount() - 1));
  }

  ble->selectDevice(deviceIndex);

  QJsonObject result;
  result[QStringLiteral("deviceIndex")] = deviceIndex;
  const auto& deviceNames               = ble->deviceNames();
  if (deviceIndex < deviceNames.count())
    result[QStringLiteral("deviceName")] = deviceNames.at(deviceIndex);

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select BLE service by index
 */
API::CommandResponse API::Handlers::BluetoothLEHandler::selectService(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("serviceIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: serviceIndex"));
  }

  const int serviceIndex   = params.value(QStringLiteral("serviceIndex")).toInt();
  auto* ble                = IO::ConnectionManager::instance().connectedBluetoothLE();
  const auto& serviceNames = ble->serviceNames();

  if (serviceIndex < 0 || serviceIndex >= serviceNames.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid serviceIndex: %1. Valid range: 0-%2")
                                        .arg(serviceIndex)
                                        .arg(serviceNames.count() - 1));
  }

  ble->selectService(serviceIndex);

  QJsonObject result;
  result[QStringLiteral("serviceIndex")] = serviceIndex;
  result[QStringLiteral("serviceName")]  = serviceNames.at(serviceIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select a BLE service by UUID
 */
API::CommandResponse API::Handlers::BluetoothLEHandler::selectServiceByUuid(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("serviceUuid"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: serviceUuid"));
  }

  const QString uuid = params.value(QStringLiteral("serviceUuid")).toString();
  auto* ble          = IO::ConnectionManager::instance().connectedBluetoothLE();

  if (!ble->selectServiceByUuid(uuid)) {
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("No discovered service matches UUID '%1' (is the device connected?)")
        .arg(uuid));
  }

  QJsonObject result;
  result[QStringLiteral("serviceUuid")] = uuid;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select BLE characteristic by index
 */
API::CommandResponse API::Handlers::BluetoothLEHandler::setCharacteristicIndex(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("characteristicIndex"))) {
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Missing required parameter: characteristicIndex"));
  }

  const int characteristicIndex   = params.value(QStringLiteral("characteristicIndex")).toInt();
  auto* ble                       = IO::ConnectionManager::instance().connectedBluetoothLE();
  const auto& characteristicNames = ble->characteristicNames();

  if (characteristicIndex < 0 || characteristicIndex >= characteristicNames.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid characteristicIndex: %1. Valid range: 0-%2")
        .arg(characteristicIndex)
        .arg(characteristicNames.count() - 1));
  }

  ble->setCharacteristicIndex(characteristicIndex);

  QJsonObject result;
  result[QStringLiteral("characteristicIndex")] = characteristicIndex;
  result[QStringLiteral("characteristicName")]  = characteristicNames.at(characteristicIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Subscribe to a notify characteristic by UUID
 */
API::CommandResponse API::Handlers::BluetoothLEHandler::setNotifyCharacteristic(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("characteristicUuid"))) {
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Missing required parameter: characteristicUuid"));
  }

  const QString uuid = params.value(QStringLiteral("characteristicUuid")).toString();
  auto* ble          = IO::ConnectionManager::instance().connectedBluetoothLE();

  if (!ble->setNotifyCharacteristicByUuid(uuid)) {
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("No characteristic matches UUID '%1' (is a service selected?)").arg(uuid));
  }

  QJsonObject result;
  result[QStringLiteral("characteristicUuid")] = uuid;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Write raw bytes to a BLE characteristic resolved by UUID
 */
API::CommandResponse API::Handlers::BluetoothLEHandler::writeCharacteristic(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("characteristicUuid"))) {
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Missing required parameter: characteristicUuid"));
  }

  if (!params.contains(QStringLiteral("data"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: data"));
  }

  const QString uuid    = params.value(QStringLiteral("characteristicUuid")).toString();
  const QString dataStr = params.value(QStringLiteral("data")).toString();
  const QByteArray data = QByteArray::fromBase64(dataStr.toUtf8());

  if (data.isEmpty() && !dataStr.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid base64 data"));
  }

  auto* ble = IO::ConnectionManager::instance().connectedBluetoothLE();
  if (!ble->isOpen()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Not connected"));
  }

  const qint64 bytesWritten = ble->writeCharacteristic(uuid, data);
  if (bytesWritten <= 0 && !data.isEmpty()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Failed to write to characteristic '%1' (not found or no active service)")
        .arg(uuid));
  }

  QJsonObject result;
  result[QStringLiteral("bytesWritten")] = bytesWritten;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get list of discovered Bluetooth LE devices
 */
API::CommandResponse API::Handlers::BluetoothLEHandler::getDeviceList(const QString& id,
                                                                      const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* ble               = IO::ConnectionManager::instance().bluetoothLE();
  const auto& deviceNames = ble->deviceNames();

  QJsonArray devices;
  for (int i = 0; i < deviceNames.count(); ++i) {
    QJsonObject device;
    device[QStringLiteral("index")] = i;
    device[QStringLiteral("name")]  = deviceNames.at(i);
    devices.append(device);
  }

  QJsonObject result;
  result[QStringLiteral("deviceList")]         = devices;
  result[QStringLiteral("currentDeviceIndex")] = ble->deviceIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of services for the selected BLE device
 */
API::CommandResponse API::Handlers::BluetoothLEHandler::getServiceList(const QString& id,
                                                                       const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* ble                = IO::ConnectionManager::instance().connectedBluetoothLE();
  const auto& serviceNames = ble->serviceNames();

  QJsonArray services;
  for (int i = 0; i < serviceNames.count(); ++i) {
    QJsonObject service;
    service[QStringLiteral("index")] = i;
    service[QStringLiteral("name")]  = serviceNames.at(i);
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
  const QString& id, const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* ble                       = IO::ConnectionManager::instance().connectedBluetoothLE();
  const auto& characteristicNames = ble->characteristicNames();

  QJsonArray characteristics;
  for (int i = 0; i < characteristicNames.count(); ++i) {
    QJsonObject characteristic;
    characteristic[QStringLiteral("index")] = i;
    characteristic[QStringLiteral("name")]  = characteristicNames.at(i);
    characteristics.append(characteristic);
  }

  QJsonObject result;
  result[QStringLiteral("characteristicList")]         = characteristics;
  result[QStringLiteral("currentCharacteristicIndex")] = ble->characteristicIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get current Bluetooth LE configuration
 */
API::CommandResponse API::Handlers::BluetoothLEHandler::getConfiguration(const QString& id,
                                                                         const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* ble = IO::ConnectionManager::instance().connectedBluetoothLE();

  QJsonObject result;

  result[QStringLiteral("deviceIndex")] = ble->deviceIndex();
  const auto& deviceNames               = ble->deviceNames();
  if (ble->deviceIndex() >= 0 && ble->deviceIndex() < deviceNames.count())
    result[QStringLiteral("deviceName")] = deviceNames.at(ble->deviceIndex());

  result[QStringLiteral("characteristicIndex")] = ble->characteristicIndex();
  const auto& characteristicNames               = ble->characteristicNames();
  if (ble->characteristicIndex() >= 0 && ble->characteristicIndex() < characteristicNames.count())
    result[QStringLiteral("characteristicName")] =
      characteristicNames.at(ble->characteristicIndex());

  result[QStringLiteral("isOpen")]          = ble->isOpen();
  result[QStringLiteral("configurationOk")] = ble->configurationOk();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get Bluetooth adapter availability and connection status
 */
API::CommandResponse API::Handlers::BluetoothLEHandler::getStatus(const QString& id,
                                                                  const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* ble = IO::ConnectionManager::instance().connectedBluetoothLE();

  QJsonObject result;
  result[QStringLiteral("operatingSystemSupported")] = ble->operatingSystemSupported();
  result[QStringLiteral("adapterAvailable")]         = ble->adapterAvailable();
  result[QStringLiteral("isOpen")]                   = ble->isOpen();
  result[QStringLiteral("deviceCount")]              = ble->deviceCount();

  return CommandResponse::makeSuccess(id, result);
}
