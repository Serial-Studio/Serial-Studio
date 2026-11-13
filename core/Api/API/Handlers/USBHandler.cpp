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
#include "API/HandlerContext.h"
#include "API/SchemaBuilder.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all USB driver commands with the registry.
 */
void API::Handlers::USBHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();
  const auto empty      = emptySchema();

  registry.registerCommand(QStringLiteral("io.usb.setDeviceIndex"),
                           QStringLiteral("Select USB device by index (params: deviceIndex)"),
                           makeSchema({
                             {QStringLiteral("deviceIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("USB device index (0 = placeholder)")}
  }),
                           &setDeviceIndex);

  registry.registerCommand(
    QStringLiteral("io.usb.setTransferMode"),
    QStringLiteral("Set transfer mode (params: mode - 0=Bulk, 1=Advanced, 2=Isochronous)"),
    makeSchema({
      {QStringLiteral("mode"),
       QStringLiteral("integer"),
       QStringLiteral("Transfer mode (0=Bulk, 1=Advanced, 2=Isochronous)")}
  }),
    &setTransferMode);

  registry.registerCommand(
    QStringLiteral("io.usb.setInEndpointIndex"),
    QStringLiteral("Select IN endpoint after connection (params: endpointIndex)"),
    makeSchema({
      {QStringLiteral("endpointIndex"),
       QStringLiteral("integer"),
       QStringLiteral("IN endpoint index")}
  }),
    &setInEndpointIndex);

  registry.registerCommand(
    QStringLiteral("io.usb.setOutEndpointIndex"),
    QStringLiteral("Select OUT endpoint after connection (params: endpointIndex)"),
    makeSchema({
      {QStringLiteral("endpointIndex"),
       QStringLiteral("integer"),
       QStringLiteral("OUT endpoint index")}
  }),
    &setOutEndpointIndex);

  registry.registerCommand(QStringLiteral("io.usb.setIsoPacketSize"),
                           QStringLiteral("Set ISO transfer packet size in bytes (params: size)"),
                           makeSchema({
                             {QStringLiteral("size"),
                              QStringLiteral("integer"),
                              QStringLiteral("ISO transfer packet size in bytes (1-49152)")}
  }),
                           &setIsoPacketSize);

  registry.registerCommand(QStringLiteral("io.usb.listDevices"),
                           QStringLiteral("List available USB devices"),
                           empty,
                           &getDeviceList);

  registry.registerCommand(QStringLiteral("io.usb.getConfig"),
                           QStringLiteral("Get complete USB driver configuration"),
                           empty,
                           &getConfiguration);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validate an index against a USB list, apply the setter and echo {index, name}.
 */
[[nodiscard]] static API::CommandResponse setListIndex(const QString& id,
                                                       const QJsonObject& params,
                                                       const char* index_key,
                                                       const char* name_key,
                                                       QStringList (IO::Drivers::USB::*list_fn)()
                                                         const,
                                                       void (IO::Drivers::USB::*setter)(int))
{
  if (!params.contains(QLatin1String(index_key)))
    return API::CommandResponse::makeMissingParam(id, index_key);

  const int index  = params.value(QLatin1String(index_key)).toInt();
  auto* usb        = API::handlerContext().connectionManager.usb();
  const auto names = (usb->*list_fn)();

  if (index < 0 || index >= names.count()) {
    return API::CommandResponse::makeError(id,
                                           API::ErrorCode::InvalidParam,
                                           QStringLiteral("Invalid %1: %2. Valid range: 0-%3")
                                             .arg(QLatin1String(index_key))
                                             .arg(index)
                                             .arg(names.count() - 1));
  }

  (usb->*setter)(index);

  QJsonObject result;
  result[QLatin1String(index_key)] = index;
  result[QLatin1String(name_key)]  = names.at(index);
  return API::CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select USB device by list index.
 */
API::CommandResponse API::Handlers::USBHandler::setDeviceIndex(const QString& id,
                                                               const QJsonObject& params)
{
  return setListIndex(id,
                      params,
                      "deviceIndex",
                      "deviceName",
                      &IO::Drivers::USB::deviceList,
                      &IO::Drivers::USB::setDeviceIndex);
}

/**
 * @brief Set USB transfer mode.
 */
API::CommandResponse API::Handlers::USBHandler::setTransferMode(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("mode")))
    return CommandResponse::makeMissingParam(id, "mode");

  const int mode = params.value(QStringLiteral("mode")).toInt();
  if (mode < 0 || mode > 2) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid mode: %1. Valid values: 0=Bulk, 1=Advanced, 2=Isochronous")
        .arg(mode));
  }

  auto& connectionManager = API::handlerContext().connectionManager;
  connectionManager.usb()->setTransferMode(mode);

  QJsonObject result;
  result[QStringLiteral("mode")] = mode;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select the active IN endpoint.
 */
API::CommandResponse API::Handlers::USBHandler::setInEndpointIndex(const QString& id,
                                                                   const QJsonObject& params)
{
  return setListIndex(id,
                      params,
                      "endpointIndex",
                      "endpointName",
                      &IO::Drivers::USB::inEndpointList,
                      &IO::Drivers::USB::setInEndpointIndex);
}

/**
 * @brief Select the active OUT endpoint.
 */
API::CommandResponse API::Handlers::USBHandler::setOutEndpointIndex(const QString& id,
                                                                    const QJsonObject& params)
{
  return setListIndex(id,
                      params,
                      "endpointIndex",
                      "endpointName",
                      &IO::Drivers::USB::outEndpointList,
                      &IO::Drivers::USB::setOutEndpointIndex);
}

/**
 * @brief Set the ISO transfer packet size.
 */
API::CommandResponse API::Handlers::USBHandler::setIsoPacketSize(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("size")))
    return CommandResponse::makeMissingParam(id, "size");

  const int size = params.value(QStringLiteral("size")).toInt();
  if (size < 1 || size > 49152) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid size: %1. Valid range: 1-49152").arg(size));
  }

  auto& connectionManager = API::handlerContext().connectionManager;
  connectionManager.usb()->setIsoPacketSize(size);

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

  auto& connectionManager = API::handlerContext().connectionManager;
  const auto& devices     = connectionManager.usb()->deviceList();

  QJsonObject result;
  result[QStringLiteral("devices")]       = QJsonArray::fromStringList(devices);
  result[QStringLiteral("selectedIndex")] = connectionManager.usb()->deviceIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the complete USB driver configuration.
 */
API::CommandResponse API::Handlers::USBHandler::getConfiguration(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& connectionManager = API::handlerContext().connectionManager;
  auto* usb               = connectionManager.usb();

  QJsonObject result;
  result[QStringLiteral("deviceIndex")]      = usb->deviceIndex();
  result[QStringLiteral("transferMode")]     = usb->transferMode();
  result[QStringLiteral("inEndpointIndex")]  = usb->inEndpointIndex();
  result[QStringLiteral("outEndpointIndex")] = usb->outEndpointIndex();
  result[QStringLiteral("isoPacketSize")]    = usb->isoPacketSize();
  result[QStringLiteral("inEndpoints")]      = QJsonArray::fromStringList(usb->inEndpointList());
  result[QStringLiteral("outEndpoints")]     = QJsonArray::fromStringList(usb->outEndpointList());
  return CommandResponse::makeSuccess(id, result);
}
