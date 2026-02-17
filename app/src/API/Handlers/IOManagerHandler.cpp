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

#include "API/Handlers/IOManagerHandler.h"

#include "API/CommandRegistry.h"
#include "IO/Manager.h"

/**
 * @brief Register all IO::Manager commands with the registry
 */
void API::Handlers::IOManagerHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("io.manager.connect"),
                           QStringLiteral("Connect to the currently configured device"),
                           &connect);

  registry.registerCommand(QStringLiteral("io.manager.disconnect"),
                           QStringLiteral("Disconnect from the current device"),
                           &disconnect);

  registry.registerCommand(QStringLiteral("io.manager.setPaused"),
                           QStringLiteral("Pause or resume data streaming (params: paused)"),
                           &setPaused);

  registry.registerCommand(
    QStringLiteral("io.manager.setBusType"),
    QStringLiteral("Set the bus type (params: busType - 0=UART, 1=Network, 2=BLE)"),
    &setBusType);

  registry.registerCommand(
    QStringLiteral("io.manager.writeData"),
    QStringLiteral("Write raw data to device (params: data - base64 encoded)"),
    &writeData);

  registry.registerCommand(QStringLiteral("io.manager.getStatus"),
                           QStringLiteral("Get current connection status and configuration"),
                           &getStatus);

  registry.registerCommand(QStringLiteral("io.manager.getAvailableBuses"),
                           QStringLiteral("Get list of available bus types"),
                           &getAvailableBuses);
}

/**
 * @brief Connect to the configured device
 */
API::CommandResponse API::Handlers::IOManagerHandler::connect(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& manager = IO::Manager::instance();

  // Check if already connected
  if (manager.isConnected()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Already connected"));
  }

  // Check configuration
  if (!manager.configurationOk()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Device configuration is invalid"));
  }

  // Attempt connection
  manager.connectDevice();

  // Return status (connection may be async for some drivers)
  QJsonObject result;
  result[QStringLiteral("connected")] = manager.isConnected();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Disconnect from the current device
 */
API::CommandResponse API::Handlers::IOManagerHandler::disconnect(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& manager = IO::Manager::instance();

  if (!manager.isConnected()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Not connected"));
  }

  manager.disconnectDevice();

  QJsonObject result;
  result[QStringLiteral("connected")] = false;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Pause or resume data streaming
 * @param params Requires "paused" (bool)
 */
API::CommandResponse API::Handlers::IOManagerHandler::setPaused(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("paused"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: paused"));
  }

  const bool paused = params.value(QStringLiteral("paused")).toBool();
  IO::Manager::instance().setPaused(paused);

  QJsonObject result;
  result[QStringLiteral("paused")] = paused;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the bus type
 * @param params Requires "busType" (int: 0=UART, 1=Network, 2=BLE)
 */
API::CommandResponse API::Handlers::IOManagerHandler::setBusType(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("busType"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: busType"));
  }

  const int busType          = params.value(QStringLiteral("busType")).toInt();
  const auto& availableBuses = IO::Manager::instance().availableBuses();

  if (busType < 0 || busType >= availableBuses.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid busType: %1. Valid range: 0-%2")
                                        .arg(busType)
                                        .arg(availableBuses.count() - 1));
  }

  IO::Manager::instance().setBusType(static_cast<SerialStudio::BusType>(busType));

  QJsonObject result;
  result[QStringLiteral("busType")]     = busType;
  result[QStringLiteral("busTypeName")] = availableBuses.at(busType);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Write raw data to the connected device
 * @param params Requires "data" (string - base64 encoded)
 */
API::CommandResponse API::Handlers::IOManagerHandler::writeData(const QString& id,
                                                                const QJsonObject& params)
{
  // Validate parameters first before checking execution preconditions
  if (!params.contains(QStringLiteral("data"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: data"));
  }

  const QString dataStr = params.value(QStringLiteral("data")).toString();
  const QByteArray data = QByteArray::fromBase64(dataStr.toUtf8());

  if (data.isEmpty() && !dataStr.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid base64 data"));
  }

  // Now check execution preconditions
  auto& manager = IO::Manager::instance();
  if (!manager.isConnected()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Not connected"));
  }

  const qint64 bytesWritten = manager.writeData(data);

  QJsonObject result;
  result[QStringLiteral("bytesWritten")] = bytesWritten;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get current connection status and configuration
 */
API::CommandResponse API::Handlers::IOManagerHandler::getStatus(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& manager = IO::Manager::instance();

  QJsonObject result;
  result[QStringLiteral("isConnected")]     = manager.isConnected();
  result[QStringLiteral("paused")]          = manager.paused();
  result[QStringLiteral("busType")]         = static_cast<int>(manager.busType());
  result[QStringLiteral("configurationOk")] = manager.configurationOk();
  result[QStringLiteral("readOnly")]        = manager.readOnly();
  result[QStringLiteral("readWrite")]       = manager.readWrite();

  // Include bus type name
  const auto& availableBuses = manager.availableBuses();
  const int busTypeIndex     = static_cast<int>(manager.busType());
  if (busTypeIndex >= 0 && busTypeIndex < availableBuses.count())
    result[QStringLiteral("busTypeName")] = availableBuses.at(busTypeIndex);

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of available bus types
 */
API::CommandResponse API::Handlers::IOManagerHandler::getAvailableBuses(const QString& id,
                                                                        const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& buses = IO::Manager::instance().availableBuses();

  QJsonArray busArray;
  for (int i = 0; i < buses.count(); ++i) {
    QJsonObject bus;
    bus[QStringLiteral("index")] = i;
    bus[QStringLiteral("name")]  = buses.at(i);
    busArray.append(bus);
  }

  QJsonObject result;
  result[QStringLiteral("buses")] = busArray;
  return CommandResponse::makeSuccess(id, result);
}
