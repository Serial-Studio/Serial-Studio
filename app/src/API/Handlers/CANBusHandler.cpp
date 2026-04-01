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

#include "API/Handlers/CANBusHandler.h"

#include "API/CommandRegistry.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all CANBus commands with the registry
 */
void API::Handlers::CANBusHandler::registerCommands()
{
  // Obtain registry and register all CAN bus commands
  auto& registry = CommandRegistry::instance();

  // Mutation commands
  QJsonObject setPluginIndexSchema;
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "integer");
    prop.insert("description", "Index of the CAN plugin to select");
    props.insert("pluginIndex", prop);
    setPluginIndexSchema.insert("type", "object");
    setPluginIndexSchema.insert("properties", props);
    QJsonArray req;
    req.append("pluginIndex");
    setPluginIndexSchema.insert("required", req);
  }
  registry.registerCommand(QStringLiteral("io.driver.canbus.setPluginIndex"),
                           QStringLiteral("Select CAN plugin by index (params: pluginIndex)"),
                           setPluginIndexSchema,
                           &setPluginIndex);

  QJsonObject setInterfaceIndexSchema;
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "integer");
    prop.insert("description", "Index of the CAN interface to select");
    props.insert("interfaceIndex", prop);
    setInterfaceIndexSchema.insert("type", "object");
    setInterfaceIndexSchema.insert("properties", props);
    QJsonArray req;
    req.append("interfaceIndex");
    setInterfaceIndexSchema.insert("required", req);
  }
  registry.registerCommand(QStringLiteral("io.driver.canbus.setInterfaceIndex"),
                           QStringLiteral("Select CAN interface by index (params: interfaceIndex)"),
                           setInterfaceIndexSchema,
                           &setInterfaceIndex);

  QJsonObject setBitrateSchema;
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "integer");
    prop.insert("description", "CAN bitrate in bits per second");
    props.insert("bitrate", prop);
    setBitrateSchema.insert("type", "object");
    setBitrateSchema.insert("properties", props);
    QJsonArray req;
    req.append("bitrate");
    setBitrateSchema.insert("required", req);
  }
  registry.registerCommand(QStringLiteral("io.driver.canbus.setBitrate"),
                           QStringLiteral("Set CAN bitrate (params: bitrate)"),
                           setBitrateSchema,
                           &setBitrate);

  QJsonObject setCanFDSchema;
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "boolean");
    prop.insert("description", "Enable or disable CAN FD");
    props.insert("enabled", prop);
    setCanFDSchema.insert("type", "object");
    setCanFDSchema.insert("properties", props);
    QJsonArray req;
    req.append("enabled");
    setCanFDSchema.insert("required", req);
  }
  registry.registerCommand(QStringLiteral("io.driver.canbus.setCanFD"),
                           QStringLiteral("Enable/disable CAN FD (params: enabled)"),
                           setCanFDSchema,
                           &setCanFD);

  // Query commands
  QJsonObject emptySchema;
  emptySchema.insert("type", "object");
  emptySchema.insert("properties", QJsonObject());

  registry.registerCommand(QStringLiteral("io.driver.canbus.getConfiguration"),
                           QStringLiteral("Get current CAN bus configuration"),
                           emptySchema,
                           &getConfiguration);

  registry.registerCommand(QStringLiteral("io.driver.canbus.getPluginList"),
                           QStringLiteral("Get list of available CAN plugins"),
                           emptySchema,
                           &getPluginList);

  registry.registerCommand(QStringLiteral("io.driver.canbus.getInterfaceList"),
                           QStringLiteral("Get list of available CAN interfaces"),
                           emptySchema,
                           &getInterfaceList);

  registry.registerCommand(QStringLiteral("io.driver.canbus.getBitrateList"),
                           QStringLiteral("Get list of supported bitrates"),
                           emptySchema,
                           &getBitrateList);

  registry.registerCommand(QStringLiteral("io.driver.canbus.getInterfaceError"),
                           QStringLiteral("Get interface error message if any"),
                           emptySchema,
                           &getInterfaceError);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Select CAN plugin by index
 * @param params Requires "pluginIndex" (int)
 */
API::CommandResponse API::Handlers::CANBusHandler::setPluginIndex(const QString& id,
                                                                  const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("pluginIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: pluginIndex"));
  }

  const int pluginIndex  = params.value(QStringLiteral("pluginIndex")).toInt();
  auto* canbus           = IO::ConnectionManager::instance().canBus();
  const auto& pluginList = canbus->pluginList();

  if (pluginIndex < 0 || pluginIndex >= pluginList.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid pluginIndex: %1. Valid range: 0-%2")
                                        .arg(pluginIndex)
                                        .arg(pluginList.count() - 1));
  }

  canbus->setPluginIndex(static_cast<quint8>(pluginIndex));

  QJsonObject result;
  result[QStringLiteral("pluginIndex")] = pluginIndex;
  result[QStringLiteral("pluginName")]  = pluginList.at(pluginIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select CAN interface by index
 * @param params Requires "interfaceIndex" (int)
 */
API::CommandResponse API::Handlers::CANBusHandler::setInterfaceIndex(const QString& id,
                                                                     const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("interfaceIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: interfaceIndex"));
  }

  const int interfaceIndex  = params.value(QStringLiteral("interfaceIndex")).toInt();
  auto* canbus              = IO::ConnectionManager::instance().canBus();
  const auto& interfaceList = canbus->interfaceList();

  if (interfaceIndex < 0 || interfaceIndex >= interfaceList.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid interfaceIndex: %1. Valid range: 0-%2")
        .arg(interfaceIndex)
        .arg(interfaceList.count() - 1));
  }

  canbus->setInterfaceIndex(static_cast<quint8>(interfaceIndex));

  QJsonObject result;
  result[QStringLiteral("interfaceIndex")] = interfaceIndex;
  result[QStringLiteral("interfaceName")]  = interfaceList.at(interfaceIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set CAN bitrate
 * @param params Requires "bitrate" (int)
 */
API::CommandResponse API::Handlers::CANBusHandler::setBitrate(const QString& id,
                                                              const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("bitrate"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: bitrate"));
  }

  const int bitrate = params.value(QStringLiteral("bitrate")).toInt();

  if (bitrate <= 0) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("bitrate must be positive"));
  }

  IO::ConnectionManager::instance().canBus()->setBitrate(static_cast<quint32>(bitrate));

  QJsonObject result;
  result[QStringLiteral("bitrate")] = bitrate;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Enable or disable CAN FD
 * @param params Requires "enabled" (bool)
 */
API::CommandResponse API::Handlers::CANBusHandler::setCanFD(const QString& id,
                                                            const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();
  IO::ConnectionManager::instance().canBus()->setCanFD(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get current CAN bus configuration
 */
API::CommandResponse API::Handlers::CANBusHandler::getConfiguration(const QString& id,
                                                                    const QJsonObject& params)
{
  // Retrieve current state
  Q_UNUSED(params)

  auto* canbus = IO::ConnectionManager::instance().canBus();

  QJsonObject result;

  // Plugin info
  result[QStringLiteral("pluginIndex")] = canbus->pluginIndex();
  const auto& pluginList                = canbus->pluginList();
  if (canbus->pluginIndex() < pluginList.count())
    result[QStringLiteral("pluginName")] = pluginList.at(canbus->pluginIndex());

  // Interface info
  result[QStringLiteral("interfaceIndex")] = canbus->interfaceIndex();
  const auto& interfaceList                = canbus->interfaceList();
  if (canbus->interfaceIndex() < interfaceList.count())
    result[QStringLiteral("interfaceName")] = interfaceList.at(canbus->interfaceIndex());

  // CAN settings
  result[QStringLiteral("bitrate")] = static_cast<qint64>(canbus->bitrate());
  result[QStringLiteral("canFD")]   = canbus->canFD();

  // Connection status
  result[QStringLiteral("isOpen")]          = canbus->isOpen();
  result[QStringLiteral("configurationOk")] = canbus->configurationOk();

  // Error info
  const QString interfaceError = canbus->interfaceError();
  if (!interfaceError.isEmpty())
    result[QStringLiteral("interfaceError")] = interfaceError;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of available CAN plugins
 */
API::CommandResponse API::Handlers::CANBusHandler::getPluginList(const QString& id,
                                                                 const QJsonObject& params)
{
  // Retrieve current state
  Q_UNUSED(params)

  auto* canbus           = IO::ConnectionManager::instance().canBus();
  const auto& pluginList = canbus->pluginList();

  QJsonArray plugins;
  for (int i = 0; i < pluginList.count(); ++i) {
    QJsonObject plugin;
    plugin[QStringLiteral("index")]       = i;
    plugin[QStringLiteral("name")]        = pluginList.at(i);
    plugin[QStringLiteral("displayName")] = canbus->pluginDisplayName(pluginList.at(i));
    plugins.append(plugin);
  }

  QJsonObject result;
  result[QStringLiteral("pluginList")]         = plugins;
  result[QStringLiteral("currentPluginIndex")] = canbus->pluginIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of available CAN interfaces
 */
API::CommandResponse API::Handlers::CANBusHandler::getInterfaceList(const QString& id,
                                                                    const QJsonObject& params)
{
  // Retrieve current state
  Q_UNUSED(params)

  auto* canbus              = IO::ConnectionManager::instance().canBus();
  const auto& interfaceList = canbus->interfaceList();

  QJsonArray interfaces;
  for (int i = 0; i < interfaceList.count(); ++i) {
    QJsonObject interface;
    interface[QStringLiteral("index")] = i;
    interface[QStringLiteral("name")]  = interfaceList.at(i);
    interfaces.append(interface);
  }

  QJsonObject result;
  result[QStringLiteral("interfaceList")]         = interfaces;
  result[QStringLiteral("currentInterfaceIndex")] = canbus->interfaceIndex();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get list of supported bitrates
 */
API::CommandResponse API::Handlers::CANBusHandler::getBitrateList(const QString& id,
                                                                  const QJsonObject& params)
{
  // Retrieve current state
  Q_UNUSED(params)

  const auto& bitrateList = IO::ConnectionManager::instance().canBus()->bitrateList();

  QJsonArray bitrates;
  for (const auto& rate : bitrateList)
    bitrates.append(rate);

  QJsonObject result;
  result[QStringLiteral("bitrateList")] = bitrates;
  result[QStringLiteral("currentBitrate")] =
    static_cast<qint64>(IO::ConnectionManager::instance().canBus()->bitrate());
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get interface error message if any
 */
API::CommandResponse API::Handlers::CANBusHandler::getInterfaceError(const QString& id,
                                                                     const QJsonObject& params)
{
  // Retrieve current state
  Q_UNUSED(params)

  const QString error = IO::ConnectionManager::instance().canBus()->interfaceError();

  QJsonObject result;
  result[QStringLiteral("hasError")] = !error.isEmpty();
  result[QStringLiteral("error")]    = error;

  return CommandResponse::makeSuccess(id, result);
}
