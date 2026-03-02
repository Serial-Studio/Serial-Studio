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

#include "API/Handlers/NetworkHandler.h"

#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "IO/Drivers/Network.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all Network commands with the registry
 */
void API::Handlers::NetworkHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  // Mutation commands
  {
    QJsonObject props;
    props[QStringLiteral("address")] = QJsonObject{
      {       QStringLiteral("type"),                               QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Remote host address (IP or hostname)")},
      {    QStringLiteral("default"),                            QStringLiteral("localhost")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("address")};
    registry.registerCommand(QStringLiteral("io.driver.network.setRemoteAddress"),
                             QStringLiteral("Set remote host address (params: address)"),
                             schema,
                             &setRemoteAddress);
  }

  {
    QJsonObject portProp = QJsonObject{
      {       QStringLiteral("type"),               QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Port number (1-65535)")},
      {    QStringLiteral("minimum"),                                       1},
      {    QStringLiteral("maximum"),                                   65535}
    };

    {
      QJsonObject props;
      props[QStringLiteral("port")] = portProp;
      QJsonObject schema;
      schema[QStringLiteral("type")]       = QStringLiteral("object");
      schema[QStringLiteral("properties")] = props;
      schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("port")};
      registry.registerCommand(QStringLiteral("io.driver.network.setTcpPort"),
                               QStringLiteral("Set TCP port (params: port)"),
                               schema,
                               &setTcpPort);
    }

    {
      QJsonObject props;
      props[QStringLiteral("port")] = portProp;
      QJsonObject schema;
      schema[QStringLiteral("type")]       = QStringLiteral("object");
      schema[QStringLiteral("properties")] = props;
      schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("port")};
      registry.registerCommand(QStringLiteral("io.driver.network.setUdpLocalPort"),
                               QStringLiteral("Set UDP local port (params: port)"),
                               schema,
                               &setUdpLocalPort);
    }

    {
      QJsonObject props;
      props[QStringLiteral("port")] = portProp;
      QJsonObject schema;
      schema[QStringLiteral("type")]       = QStringLiteral("object");
      schema[QStringLiteral("properties")] = props;
      schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("port")};
      registry.registerCommand(QStringLiteral("io.driver.network.setUdpRemotePort"),
                               QStringLiteral("Set UDP remote port (params: port)"),
                               schema,
                               &setUdpRemotePort);
    }
  }

  {
    QJsonObject props;
    props[QStringLiteral("socketTypeIndex")] = QJsonObject{
      {       QStringLiteral("type"),                          QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Socket type index (0=TCP, 1=UDP)")},
      {       QStringLiteral("enum"),                                   QJsonArray{0, 1}},
      {    QStringLiteral("default"),                                                  0}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("socketTypeIndex")};
    registry.registerCommand(
      QStringLiteral("io.driver.network.setSocketType"),
      QStringLiteral("Set socket type (params: socketTypeIndex - 0=TCP, 1=UDP)"),
      schema,
      &setSocketType);
  }

  {
    QJsonObject props;
    props[QStringLiteral("enabled")] = QJsonObject{
      {       QStringLiteral("type"),                   QStringLiteral("boolean")},
      {QStringLiteral("description"), QStringLiteral("Enable UDP multicast mode")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("enabled")};
    registry.registerCommand(QStringLiteral("io.driver.network.setUdpMulticast"),
                             QStringLiteral("Enable/disable UDP multicast (params: enabled)"),
                             schema,
                             &setUdpMulticast);
  }

  registry.registerCommand(QStringLiteral("io.driver.network.lookup"),
                           QStringLiteral("Perform DNS lookup (params: host)"),
                           &lookup);

  // Query commands
  registry.registerCommand(QStringLiteral("io.driver.network.getConfiguration"),
                           QStringLiteral("Get current network configuration"),
                           &getConfiguration);

  registry.registerCommand(QStringLiteral("io.driver.network.getSocketTypes"),
                           QStringLiteral("Get available socket types"),
                           &getSocketTypes);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Set remote host address
 * @param params Requires "address" (string, e.g., "192.168.1.100" or
 * "localhost")
 */
API::CommandResponse API::Handlers::NetworkHandler::setRemoteAddress(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("address"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: address"));
  }

  const QString address = params.value(QStringLiteral("address")).toString();
  if (address.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Address cannot be empty"));
  }

  IO::Drivers::Network::instance().setRemoteAddress(address);

  QJsonObject result;
  result[QStringLiteral("address")] = address;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set TCP port
 * @param params Requires "port" (int, 1-65535)
 */
API::CommandResponse API::Handlers::NetworkHandler::setTcpPort(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("port"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: port"));
  }

  const int port = params.value(QStringLiteral("port")).toInt();
  if (port < 1 || port > 65535) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Port must be between 1 and 65535"));
  }

  IO::Drivers::Network::instance().setTcpPort(static_cast<quint16>(port));

  QJsonObject result;
  result[QStringLiteral("tcpPort")] = port;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set UDP local port
 * @param params Requires "port" (int, 0-65535, 0 = any available port)
 */
API::CommandResponse API::Handlers::NetworkHandler::setUdpLocalPort(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("port"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: port"));
  }

  const int port = params.value(QStringLiteral("port")).toInt();
  if (port < 0 || port > 65535) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Port must be between 0 and 65535"));
  }

  IO::Drivers::Network::instance().setUdpLocalPort(static_cast<quint16>(port));

  QJsonObject result;
  result[QStringLiteral("udpLocalPort")] = port;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set UDP remote port
 * @param params Requires "port" (int, 1-65535)
 */
API::CommandResponse API::Handlers::NetworkHandler::setUdpRemotePort(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("port"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: port"));
  }

  const int port = params.value(QStringLiteral("port")).toInt();
  if (port < 1 || port > 65535) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Port must be between 1 and 65535"));
  }

  IO::Drivers::Network::instance().setUdpRemotePort(static_cast<quint16>(port));

  QJsonObject result;
  result[QStringLiteral("udpRemotePort")] = port;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set socket type
 * @param params Requires "socketTypeIndex" (int: 0=TCP, 1=UDP)
 */
API::CommandResponse API::Handlers::NetworkHandler::setSocketType(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("socketTypeIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: socketTypeIndex"));
  }

  const int socketTypeIndex = params.value(QStringLiteral("socketTypeIndex")).toInt();
  const auto& socketTypes   = IO::Drivers::Network::instance().socketTypes();

  if (socketTypeIndex < 0 || socketTypeIndex >= socketTypes.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid socketTypeIndex: %1. Valid range: 0-%2")
        .arg(socketTypeIndex)
        .arg(socketTypes.count() - 1));
  }

  IO::Drivers::Network::instance().setSocketTypeIndex(socketTypeIndex);

  QJsonObject result;
  result[QStringLiteral("socketTypeIndex")] = socketTypeIndex;
  result[QStringLiteral("socketTypeName")]  = socketTypes.at(socketTypeIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Enable or disable UDP multicast
 * @param params Requires "enabled" (bool)
 */
API::CommandResponse API::Handlers::NetworkHandler::setUdpMulticast(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();
  IO::Drivers::Network::instance().setUdpMulticast(enabled);

  QJsonObject result;
  result[QStringLiteral("udpMulticast")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Perform DNS lookup for a hostname
 * @param params Requires "host" (string)
 *
 * Note: This is an asynchronous operation. The result indicates that the
 * lookup was initiated, not that it completed successfully.
 */
API::CommandResponse API::Handlers::NetworkHandler::lookup(const QString& id,
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

  IO::Drivers::Network::instance().lookup(host);

  QJsonObject result;
  result[QStringLiteral("host")]          = host;
  result[QStringLiteral("lookupStarted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get current network configuration
 */
API::CommandResponse API::Handlers::NetworkHandler::getConfiguration(const QString& id,
                                                                     const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& network = IO::Drivers::Network::instance();

  QJsonObject result;

  // Address
  result[QStringLiteral("remoteAddress")] = network.remoteAddress();

  // Ports
  result[QStringLiteral("tcpPort")]       = network.tcpPort();
  result[QStringLiteral("udpLocalPort")]  = network.udpLocalPort();
  result[QStringLiteral("udpRemotePort")] = network.udpRemotePort();

  // Socket type
  result[QStringLiteral("socketTypeIndex")] = network.socketTypeIndex();
  const auto& socketTypes                   = network.socketTypes();
  if (network.socketTypeIndex() < socketTypes.count())
    result[QStringLiteral("socketTypeName")] = socketTypes.at(network.socketTypeIndex());

  // Other settings
  result[QStringLiteral("udpMulticast")]    = network.udpMulticast();
  result[QStringLiteral("lookupActive")]    = network.lookupActive();
  result[QStringLiteral("isOpen")]          = network.isOpen();
  result[QStringLiteral("configurationOk")] = network.configurationOk();

  // Default values for reference
  result[QStringLiteral("defaultAddress")]       = network.defaultAddress();
  result[QStringLiteral("defaultTcpPort")]       = network.defaultTcpPort();
  result[QStringLiteral("defaultUdpLocalPort")]  = network.defaultUdpLocalPort();
  result[QStringLiteral("defaultUdpRemotePort")] = network.defaultUdpRemotePort();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get available socket types
 */
API::CommandResponse API::Handlers::NetworkHandler::getSocketTypes(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& socketTypes = IO::Drivers::Network::instance().socketTypes();

  QJsonArray types;
  for (int i = 0; i < socketTypes.count(); ++i) {
    QJsonObject type;
    type[QStringLiteral("index")] = i;
    type[QStringLiteral("name")]  = socketTypes.at(i);
    types.append(type);
  }

  QJsonObject result;
  result[QStringLiteral("socketTypes")] = types;
  result[QStringLiteral("currentSocketTypeIndex")] =
    IO::Drivers::Network::instance().socketTypeIndex();
  return CommandResponse::makeSuccess(id, result);
}
