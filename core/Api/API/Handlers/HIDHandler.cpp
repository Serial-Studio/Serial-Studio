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
#include "API/HandlerContext.h"
#include "API/SchemaBuilder.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all HID driver commands with the registry.
 */
void API::Handlers::HIDHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("io.hid.setDeviceIndex"),
                           QStringLiteral("Select HID device by index (params: deviceIndex)"),
                           makeSchema({
                             {QStringLiteral("deviceIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("HID device index (0 = placeholder)")}
  }),
                           &setDeviceIndex);

  registry.registerCommand(QStringLiteral("io.hid.listDevices"),
                           QStringLiteral("List available HID devices"),
                           emptySchema(),
                           &getDeviceList);

  registry.registerCommand(QStringLiteral("io.hid.getConfig"),
                           QStringLiteral("Get complete HID driver configuration"),
                           emptySchema(),
                           &getConfiguration);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Select HID device by list index.
 */
API::CommandResponse API::Handlers::HIDHandler::setDeviceIndex(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("deviceIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: deviceIndex"));
  }

  const int device_index  = params.value(QStringLiteral("deviceIndex")).toInt();
  auto& connectionManager = API::handlerContext().connectionManager;
  const auto& devices     = connectionManager.hid()->deviceList();

  if (device_index < 0 || device_index >= devices.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid deviceIndex: %1. Valid range: 0-%2"))
        .arg(device_index)
        .arg(devices.count() - 1));
  }

  connectionManager.hid()->setDeviceIndex(device_index);

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
  Q_UNUSED(params)

  auto& connectionManager = API::handlerContext().connectionManager;
  const auto& devices     = connectionManager.hid()->deviceList();

  QJsonArray devices_array;
  for (const auto& device : devices)
    devices_array.append(device);

  QJsonObject result;
  result[QStringLiteral("devices")]       = devices_array;
  result[QStringLiteral("selectedIndex")] = connectionManager.hid()->deviceIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get complete HID driver configuration and connected-device info.
 */
API::CommandResponse API::Handlers::HIDHandler::getConfiguration(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& connectionManager = API::handlerContext().connectionManager;
  auto* hid               = connectionManager.hid();

  QJsonObject result;
  result[QStringLiteral("deviceIndex")] = hid->deviceIndex();
  result[QStringLiteral("usagePage")]   = hid->usagePage();
  result[QStringLiteral("usage")]       = hid->usage();
  return CommandResponse::makeSuccess(id, result);
}
