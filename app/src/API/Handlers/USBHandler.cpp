/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is part of Serial Studio Pro. All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/USBHandler.h"

#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all USB driver commands with the registry.
 */
void API::Handlers::USBHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  // setDeviceIndex schema
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "integer");
    prop.insert("description", "USB device index (0 = placeholder)");
    props.insert("deviceIndex", prop);
    QJsonObject schema;
    schema.insert("type", "object");
    schema.insert("properties", props);
    QJsonArray req;
    req.append("deviceIndex");
    schema.insert("required", req);
    registry.registerCommand(QStringLiteral("io.driver.usb.setDeviceIndex"),
                             QStringLiteral("Select USB device by index (params: deviceIndex)"),
                             schema,
                             &setDeviceIndex);
  }

  // setTransferMode schema
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "integer");
    prop.insert("description", "Transfer mode (0=Bulk, 1=Advanced, 2=Isochronous)");
    props.insert("mode", prop);
    QJsonObject schema;
    schema.insert("type", "object");
    schema.insert("properties", props);
    QJsonArray req;
    req.append("mode");
    schema.insert("required", req);
    registry.registerCommand(
      QStringLiteral("io.driver.usb.setTransferMode"),
      QStringLiteral("Set transfer mode (params: mode - 0=Bulk, 1=Advanced, 2=Isochronous)"),
      schema,
      &setTransferMode);
  }

  // setInEndpointIndex schema
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "integer");
    prop.insert("description", "IN endpoint index");
    props.insert("endpointIndex", prop);
    QJsonObject schema;
    schema.insert("type", "object");
    schema.insert("properties", props);
    QJsonArray req;
    req.append("endpointIndex");
    schema.insert("required", req);
    registry.registerCommand(
      QStringLiteral("io.driver.usb.setInEndpointIndex"),
      QStringLiteral("Select IN endpoint after connection (params: endpointIndex)"),
      schema,
      &setInEndpointIndex);
  }

  // setOutEndpointIndex schema
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "integer");
    prop.insert("description", "OUT endpoint index");
    props.insert("endpointIndex", prop);
    QJsonObject schema;
    schema.insert("type", "object");
    schema.insert("properties", props);
    QJsonArray req;
    req.append("endpointIndex");
    schema.insert("required", req);
    registry.registerCommand(
      QStringLiteral("io.driver.usb.setOutEndpointIndex"),
      QStringLiteral("Select OUT endpoint after connection (params: endpointIndex)"),
      schema,
      &setOutEndpointIndex);
  }

  // setIsoPacketSize schema
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "integer");
    prop.insert("description", "ISO transfer packet size in bytes (1-49152)");
    props.insert("size", prop);
    QJsonObject schema;
    schema.insert("type", "object");
    schema.insert("properties", props);
    QJsonArray req;
    req.append("size");
    schema.insert("required", req);
    registry.registerCommand(QStringLiteral("io.driver.usb.setIsoPacketSize"),
                             QStringLiteral("Set ISO transfer packet size in bytes (params: size)"),
                             schema,
                             &setIsoPacketSize);
  }

  // getDeviceList schema (no params)
  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("io.driver.usb.getDeviceList"),
                             QStringLiteral("List available USB devices"),
                             emptySchema,
                             &getDeviceList);
  }

  // getConfiguration schema (no params)
  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("io.driver.usb.getConfiguration"),
                             QStringLiteral("Get complete USB driver configuration"),
                             emptySchema,
                             &getConfiguration);
  }
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Select USB device by list index.
 * @param params Requires "deviceIndex" (int, 0 = placeholder "Select Device")
 */
API::CommandResponse API::Handlers::USBHandler::setDeviceIndex(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("deviceIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: deviceIndex"));
  }

  const int device_index = params.value(QStringLiteral("deviceIndex")).toInt();
  const auto& devices    = IO::ConnectionManager::instance().usb()->deviceList();

  if (device_index < 0 || device_index >= devices.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid deviceIndex: %1. Valid range: 0-%2"))
        .arg(device_index)
        .arg(devices.count() - 1));
  }

  IO::ConnectionManager::instance().usb()->setDeviceIndex(device_index);

  QJsonObject result;
  result[QStringLiteral("deviceIndex")] = device_index;
  result[QStringLiteral("deviceName")]  = devices.at(device_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set USB transfer mode.
 * @param params Requires "mode" (int: 0=BulkStream, 1=AdvancedControl, 2=Isochronous)
 */
API::CommandResponse API::Handlers::USBHandler::setTransferMode(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("mode"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: mode"));
  }

  const int mode = params.value(QStringLiteral("mode")).toInt();
  if (mode < 0 || mode > 2) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid mode: %1. Valid values: 0=Bulk, 1=Advanced, 2=Isochronous")
        .arg(mode));
  }

  IO::ConnectionManager::instance().usb()->setTransferMode(mode);

  QJsonObject result;
  result[QStringLiteral("mode")] = mode;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select the active IN endpoint.
 * @param params Requires "endpointIndex" (int)
 */
API::CommandResponse API::Handlers::USBHandler::setInEndpointIndex(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("endpointIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: endpointIndex"));
  }

  const int ep_index    = params.value(QStringLiteral("endpointIndex")).toInt();
  const auto& endpoints = IO::ConnectionManager::instance().usb()->inEndpointList();

  if (ep_index < 0 || ep_index >= endpoints.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid endpointIndex: %1. Valid range: 0-%2"))
        .arg(ep_index)
        .arg(endpoints.count() - 1));
  }

  IO::ConnectionManager::instance().usb()->setInEndpointIndex(ep_index);

  QJsonObject result;
  result[QStringLiteral("endpointIndex")] = ep_index;
  result[QStringLiteral("endpointName")]  = endpoints.at(ep_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select the active OUT endpoint.
 * @param params Requires "endpointIndex" (int)
 */
API::CommandResponse API::Handlers::USBHandler::setOutEndpointIndex(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("endpointIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: endpointIndex"));
  }

  const int ep_index    = params.value(QStringLiteral("endpointIndex")).toInt();
  const auto& endpoints = IO::ConnectionManager::instance().usb()->outEndpointList();

  if (ep_index < 0 || ep_index >= endpoints.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid endpointIndex: %1. Valid range: 0-%2"))
        .arg(ep_index)
        .arg(endpoints.count() - 1));
  }

  IO::ConnectionManager::instance().usb()->setOutEndpointIndex(ep_index);

  QJsonObject result;
  result[QStringLiteral("endpointIndex")] = ep_index;
  result[QStringLiteral("endpointName")]  = endpoints.at(ep_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the ISO transfer packet size.
 * @param params Requires "size" (int, 1-49152)
 */
API::CommandResponse API::Handlers::USBHandler::setIsoPacketSize(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("size"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: size"));
  }

  const int size = params.value(QStringLiteral("size")).toInt();
  if (size < 1 || size > 49152) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid size: %1. Valid range: 1-49152").arg(size));
  }

  IO::ConnectionManager::instance().usb()->setIsoPacketSize(size);

  QJsonObject result;
  result[QStringLiteral("size")] = size;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief List all available USB devices.
 */
API::CommandResponse API::Handlers::USBHandler::getDeviceList(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& devices = IO::ConnectionManager::instance().usb()->deviceList();

  QJsonArray devices_array;
  for (const auto& device : devices)
    devices_array.append(device);

  QJsonObject result;
  result[QStringLiteral("devices")]       = devices_array;
  result[QStringLiteral("selectedIndex")] = IO::ConnectionManager::instance().usb()->deviceIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the complete USB driver configuration.
 */
API::CommandResponse API::Handlers::USBHandler::getConfiguration(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* usb = IO::ConnectionManager::instance().usb();

  QJsonArray in_endpoints;
  for (const auto& ep : usb->inEndpointList())
    in_endpoints.append(ep);

  QJsonArray out_endpoints;
  for (const auto& ep : usb->outEndpointList())
    out_endpoints.append(ep);

  QJsonObject result;
  result[QStringLiteral("deviceIndex")]      = usb->deviceIndex();
  result[QStringLiteral("transferMode")]     = usb->transferMode();
  result[QStringLiteral("inEndpointIndex")]  = usb->inEndpointIndex();
  result[QStringLiteral("outEndpointIndex")] = usb->outEndpointIndex();
  result[QStringLiteral("isoPacketSize")]    = usb->isoPacketSize();
  result[QStringLiteral("inEndpoints")]      = in_endpoints;
  result[QStringLiteral("outEndpoints")]     = out_endpoints;
  return CommandResponse::makeSuccess(id, result);
}
