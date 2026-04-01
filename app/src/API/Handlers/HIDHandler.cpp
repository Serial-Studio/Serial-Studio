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

#include "API/Handlers/HIDHandler.h"

#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all HID driver commands with the registry.
 */
void API::Handlers::HIDHandler::registerCommands()
{
  // Obtain registry and register HID commands
  auto& registry = CommandRegistry::instance();

  // setDeviceIndex schema
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "integer");
    prop.insert("description", "HID device index (0 = placeholder)");
    props.insert("deviceIndex", prop);
    QJsonObject schema;
    schema.insert("type", "object");
    schema.insert("properties", props);
    QJsonArray req;
    req.append("deviceIndex");
    schema.insert("required", req);
    registry.registerCommand(QStringLiteral("io.driver.hid.setDeviceIndex"),
                             QStringLiteral("Select HID device by index (params: deviceIndex)"),
                             schema,
                             &setDeviceIndex);
  }

  // getDeviceList schema (no params)
  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("io.driver.hid.getDeviceList"),
                             QStringLiteral("List available HID devices"),
                             emptySchema,
                             &getDeviceList);
  }

  // getConfiguration schema (no params)
  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("io.driver.hid.getConfiguration"),
                             QStringLiteral("Get complete HID driver configuration"),
                             emptySchema,
                             &getConfiguration);
  }
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Select HID device by list index.
 * @param params Requires "deviceIndex" (int, 0 = placeholder "Select Device")
 */
API::CommandResponse API::Handlers::HIDHandler::setDeviceIndex(const QString& id,
                                                               const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("deviceIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: deviceIndex"));
  }

  const int device_index = params.value(QStringLiteral("deviceIndex")).toInt();
  const auto& devices    = IO::ConnectionManager::instance().hid()->deviceList();

  if (device_index < 0 || device_index >= devices.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid deviceIndex: %1. Valid range: 0-%2"))
        .arg(device_index)
        .arg(devices.count() - 1));
  }

  IO::ConnectionManager::instance().hid()->setDeviceIndex(device_index);

  QJsonObject result;
  result[QStringLiteral("deviceIndex")] = device_index;
  result[QStringLiteral("deviceName")]  = devices.at(device_index);
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief List all available HID devices.
 */
API::CommandResponse API::Handlers::HIDHandler::getDeviceList(const QString& id,
                                                              const QJsonObject& params)
{
  // Retrieve current state
  Q_UNUSED(params)

  const auto& devices = IO::ConnectionManager::instance().hid()->deviceList();

  QJsonArray devices_array;
  for (const auto& device : devices)
    devices_array.append(device);

  QJsonObject result;
  result[QStringLiteral("devices")]       = devices_array;
  result[QStringLiteral("selectedIndex")] = IO::ConnectionManager::instance().hid()->deviceIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get complete HID driver configuration and connected-device info.
 */
API::CommandResponse API::Handlers::HIDHandler::getConfiguration(const QString& id,
                                                                 const QJsonObject& params)
{
  // Retrieve current state
  Q_UNUSED(params)

  auto* hid = IO::ConnectionManager::instance().hid();

  QJsonObject result;
  result[QStringLiteral("deviceIndex")] = hid->deviceIndex();
  result[QStringLiteral("usagePage")]   = hid->usagePage();
  result[QStringLiteral("usage")]       = hid->usage();
  return CommandResponse::makeSuccess(id, result);
}
