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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/NetworkHandler.h"

#include <QJsonArray>
#include <QUrl>

#include "API/CommandRegistry.h"
#include "API/HandlerContext.h"
#include "API/SchemaBuilder.h"
#include "IO/ConnectionManager.h"

/**
 * @brief The UI-config Network driver the URL-transport commands operate on.
 */
[[nodiscard]] static IO::Drivers::Network* networkDriver()
{
  auto& connectionManager = API::handlerContext().connectionManager;
  return connectionManager.network();
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all Network commands with the registry
 */
void API::Handlers::NetworkHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();
  const auto empty      = emptySchema();

  SchemaProp address_prop;
  address_prop.name         = QStringLiteral("address");
  address_prop.type         = QStringLiteral("string");
  address_prop.description  = QStringLiteral("Remote host address (IP or hostname)");
  address_prop.defaultValue = QStringLiteral("localhost");
  registry.registerCommand(QStringLiteral("io.network.setRemoteAddress"),
                           QStringLiteral("Set remote host address (params: address)"),
                           makeSchema({address_prop}),
                           &setRemoteAddress);

  const SchemaProp port_prop =
    rangeProp(QStringLiteral("port"), QStringLiteral("Port number (1-65535)"), 1, 65535);

  registry.registerCommand(QStringLiteral("io.network.setTcpPort"),
                           QStringLiteral("Set TCP port (params: port)"),
                           makeSchema({port_prop}),
                           &setTcpPort);

  registry.registerCommand(QStringLiteral("io.network.setUdpLocalPort"),
                           QStringLiteral("Set UDP local port (params: port)"),
                           makeSchema({port_prop}),
                           &setUdpLocalPort);

  registry.registerCommand(QStringLiteral("io.network.setUdpRemotePort"),
                           QStringLiteral("Set UDP remote port (params: port)"),
                           makeSchema({port_prop}),
                           &setUdpRemotePort);

  registry.registerCommand(
    QStringLiteral("io.network.setSocketType"),
    QStringLiteral("Set socket type (params: socketTypeIndex - 0=TCP, 1=UDP, 2=WebSocket, 3=HTTP)"),
    makeSchema({enumProp(QStringLiteral("socketTypeIndex"),
                         QStringLiteral("Socket type index (0=TCP, 1=UDP, 2=WebSocket, 3=HTTP)"),
                         QJsonArray{0, 1, 2, 3},
                         0)}),
    &setSocketType);

  registry.registerCommand(QStringLiteral("io.network.setUdpMulticast"),
                           QStringLiteral("Enable/disable UDP multicast (params: enabled)"),
                           makeSchema({
                             {QStringLiteral("enabled"),
                              QStringLiteral("boolean"),
                              QStringLiteral("Enable UDP multicast mode")}
  }),
                           &setUdpMulticast);

  registry.registerCommand(QStringLiteral("io.network.lookup"),
                           QStringLiteral("Perform DNS lookup (params: host)"),
                           makeSchema({
                             {QStringLiteral("host"),
                              QStringLiteral("string"),
                              QStringLiteral("Hostname or IP address to look up")}
  }),
                           &lookup);

  registry.registerCommand(QStringLiteral("io.network.getConfig"),
                           QStringLiteral("Get current network configuration"),
                           empty,
                           &getConfiguration);

  registry.registerCommand(QStringLiteral("io.network.listSocketTypes"),
                           QStringLiteral("Get available socket types"),
                           empty,
                           &getSocketTypes);

  registry.registerCommand(QStringLiteral("io.network.getStatus"),
                           QStringLiteral("Get link state and HTTP poll counters"),
                           empty,
                           &getStatus);

  registerUrlTransportCommands(registry);
}

/**
 * @brief Registers the WebSocket and HTTP commands into @p registry. Split out of
 *        registerCommands() only to keep both functions inside the length cap.
 */
void API::Handlers::NetworkHandler::registerUrlTransportCommands(CommandRegistry& registry)
{
  SchemaProp ws_url_prop;
  ws_url_prop.name         = QStringLiteral("url");
  ws_url_prop.type         = QStringLiteral("string");
  ws_url_prop.description  = QStringLiteral("WebSocket endpoint (ws:// or wss://)");
  ws_url_prop.defaultValue = QStringLiteral("ws://127.0.0.1:8080");
  registry.registerCommand(QStringLiteral("io.network.setWebSocketUrl"),
                           QStringLiteral("Set the WebSocket endpoint URL (params: url)"),
                           makeSchema({ws_url_prop}),
                           &setWebSocketUrl);

  SchemaProp http_url_prop;
  http_url_prop.name         = QStringLiteral("url");
  http_url_prop.type         = QStringLiteral("string");
  http_url_prop.description  = QStringLiteral("REST endpoint (http:// or https://)");
  http_url_prop.defaultValue = QStringLiteral("http://127.0.0.1:8080/");
  registry.registerCommand(QStringLiteral("io.network.setHttpUrl"),
                           QStringLiteral("Set the HTTP endpoint URL (params: url)"),
                           makeSchema({http_url_prop}),
                           &setHttpUrl);

  SchemaProp method_prop;
  method_prop.name         = QStringLiteral("method");
  method_prop.type         = QStringLiteral("string");
  method_prop.description  = QStringLiteral("HTTP method");
  method_prop.enumValues   = QJsonArray{QStringLiteral("GET"),
                                      QStringLiteral("POST"),
                                      QStringLiteral("PUT"),
                                      QStringLiteral("PATCH"),
                                      QStringLiteral("DELETE")};
  method_prop.defaultValue = QStringLiteral("GET");
  registry.registerCommand(
    QStringLiteral("io.network.setHttpMethod"),
    QStringLiteral("Set the HTTP method (params: method - GET, POST, PUT, PATCH or DELETE)"),
    makeSchema({method_prop}),
    &setHttpMethod);

  registry.registerCommand(QStringLiteral("io.network.setHttpBody"),
                           QStringLiteral("Set the HTTP request body (params: body)"),
                           makeSchema({
                             {QStringLiteral("body"),
                              QStringLiteral("string"),
                              QStringLiteral("Body sent with every HTTP request")}
  }),
                           &setHttpBody);

  registry.registerCommand(
    QStringLiteral("io.network.setHttpHeaders"),
    QStringLiteral("Set custom HTTP request headers (params: headers, one Name: Value per line)"),
    makeSchema({
      {QStringLiteral("headers"),
       QStringLiteral("string"),
       QStringLiteral("Custom request headers, one Name: Value pair per line")}
  }),
    &setHttpHeaders);

  registry.registerCommand(
    QStringLiteral("io.network.setHttpInterval"),
    QStringLiteral("Set the HTTP poll interval in ms (params: interval; 0 = only on write)"),
    makeSchema({rangeProp(QStringLiteral("interval"),
                          QStringLiteral("Poll interval in milliseconds (0, or 10-3600000)"),
                          0,
                          3600000)}),
    &setHttpInterval);

  registry.registerCommand(
    QStringLiteral("io.network.setIgnoreTlsErrors"),
    QStringLiteral("Accept untrusted certificates on wss:// and https:// (params: enabled)"),
    makeSchema({
      {QStringLiteral("enabled"),
       QStringLiteral("boolean"),
       QStringLiteral("Bypass certificate verification")}
  }),
    &setIgnoreTlsErrors);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Set remote host address
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

  auto& conn = API::handlerContext().connectionManager;
  conn.network()->setRemoteAddress(address);

  QJsonObject result;
  result[QStringLiteral("address")] = address;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set TCP port
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

  auto& conn = API::handlerContext().connectionManager;
  conn.network()->setTcpPort(static_cast<quint16>(port));

  QJsonObject result;
  result[QStringLiteral("tcpPort")] = port;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set UDP local port
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

  auto& conn = API::handlerContext().connectionManager;
  conn.network()->setUdpLocalPort(static_cast<quint16>(port));

  QJsonObject result;
  result[QStringLiteral("udpLocalPort")] = port;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set UDP remote port
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

  auto& conn = API::handlerContext().connectionManager;
  conn.network()->setUdpRemotePort(static_cast<quint16>(port));

  QJsonObject result;
  result[QStringLiteral("udpRemotePort")] = port;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set socket type
 */
API::CommandResponse API::Handlers::NetworkHandler::setSocketType(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("socketTypeIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: socketTypeIndex"));
  }

  const int socketTypeIndex = params.value(QStringLiteral("socketTypeIndex")).toInt();
  auto& conn                = API::handlerContext().connectionManager;
  const auto& socketTypes   = conn.network()->socketTypes();

  if (socketTypeIndex < 0 || socketTypeIndex >= socketTypes.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid socketTypeIndex: %1. Valid range: 0-%2")
        .arg(socketTypeIndex)
        .arg(socketTypes.count() - 1));
  }

  conn.network()->setSocketTypeIndex(socketTypeIndex);

  QJsonObject result;
  result[QStringLiteral("socketTypeIndex")] = socketTypeIndex;
  result[QStringLiteral("socketTypeName")]  = socketTypes.at(socketTypeIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Enable or disable UDP multicast
 */
API::CommandResponse API::Handlers::NetworkHandler::setUdpMulticast(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();
  auto& conn         = API::handlerContext().connectionManager;
  conn.network()->setUdpMulticast(enabled);

  QJsonObject result;
  result[QStringLiteral("udpMulticast")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Perform DNS lookup for a hostname
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

  auto& conn = API::handlerContext().connectionManager;
  conn.network()->lookup(host);

  QJsonObject result;
  result[QStringLiteral("host")]          = host;
  result[QStringLiteral("lookupStarted")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the WebSocket endpoint URL
 */
API::CommandResponse API::Handlers::NetworkHandler::setWebSocketUrl(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("url"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: url"));
  }

  const QString url    = params.value(QStringLiteral("url")).toString().trimmed();
  const QString scheme = QUrl(url, QUrl::StrictMode).scheme().toLower();
  if (scheme != QLatin1String("ws") && scheme != QLatin1String("wss")) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("URL must start with ws:// or wss://"));
  }

  networkDriver()->setWebSocketUrl(url);

  QJsonObject result;
  result[QStringLiteral("webSocketUrl")] = url;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the HTTP endpoint URL
 */
API::CommandResponse API::Handlers::NetworkHandler::setHttpUrl(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("url"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: url"));
  }

  const QString url    = params.value(QStringLiteral("url")).toString().trimmed();
  const QString scheme = QUrl(url, QUrl::StrictMode).scheme().toLower();
  if (scheme != QLatin1String("http") && scheme != QLatin1String("https")) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("URL must start with http:// or https://"));
  }

  networkDriver()->setHttpUrl(url);

  QJsonObject result;
  result[QStringLiteral("httpUrl")] = url;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the HTTP method
 */
API::CommandResponse API::Handlers::NetworkHandler::setHttpMethod(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("method"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: method"));
  }

  const QString method = params.value(QStringLiteral("method")).toString().toUpper();
  const int index      = networkDriver()->httpMethods().indexOf(method);
  if (index < 0) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid method: %1. Valid values: %2")
        .arg(method, networkDriver()->httpMethods().join(QStringLiteral(", "))));
  }

  networkDriver()->setHttpMethodIndex(index);

  QJsonObject result;
  result[QStringLiteral("httpMethod")] = method;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the HTTP request body
 */
API::CommandResponse API::Handlers::NetworkHandler::setHttpBody(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("body"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: body"));
  }

  const QString body = params.value(QStringLiteral("body")).toString();
  networkDriver()->setHttpBody(body);

  QJsonObject result;
  result[QStringLiteral("httpBody")] = body;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the custom HTTP request headers
 */
API::CommandResponse API::Handlers::NetworkHandler::setHttpHeaders(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("headers"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: headers"));
  }

  const QString headers = params.value(QStringLiteral("headers")).toString();
  networkDriver()->setHttpHeaders(headers);

  QJsonObject result;
  result[QStringLiteral("httpHeaders")] = headers;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the HTTP poll interval
 */
API::CommandResponse API::Handlers::NetworkHandler::setHttpInterval(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("interval"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: interval"));
  }

  const int interval = params.value(QStringLiteral("interval")).toInt();
  if (interval < 0 || interval > 3600000) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Interval must be 0, or between 10 and 3600000"));
  }

  networkDriver()->setHttpInterval(interval);

  QJsonObject result;
  result[QStringLiteral("httpInterval")] = networkDriver()->httpInterval();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Enable or disable the TLS certificate-verification bypass
 */
API::CommandResponse API::Handlers::NetworkHandler::setIgnoreTlsErrors(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();
  networkDriver()->setIgnoreTlsErrors(enabled);

  QJsonObject result;
  result[QStringLiteral("ignoreTlsErrors")] = enabled;
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

  auto& conn    = API::handlerContext().connectionManager;
  auto* network = conn.network();

  QJsonObject result;

  result[QStringLiteral("remoteAddress")] = network->remoteAddress();

  result[QStringLiteral("tcpPort")]       = network->tcpPort();
  result[QStringLiteral("udpLocalPort")]  = network->udpLocalPort();
  result[QStringLiteral("udpRemotePort")] = network->udpRemotePort();

  result[QStringLiteral("socketTypeIndex")] = network->socketTypeIndex();
  const auto& socketTypes                   = network->socketTypes();
  if (network->socketTypeIndex() < socketTypes.count())
    result[QStringLiteral("socketTypeName")] = socketTypes.at(network->socketTypeIndex());

  result[QStringLiteral("udpMulticast")]    = network->udpMulticast();
  result[QStringLiteral("lookupActive")]    = network->lookupActive();
  result[QStringLiteral("isOpen")]          = network->isOpen();
  result[QStringLiteral("configurationOk")] = network->configurationOk();

  result[QStringLiteral("webSocketUrl")]         = network->webSocketUrl();
  result[QStringLiteral("webSocketFormatIndex")] = network->webSocketFormatIndex();
  result[QStringLiteral("ignoreTlsErrors")]      = network->ignoreTlsErrors();

  result[QStringLiteral("httpUrl")]      = network->httpUrl();
  result[QStringLiteral("httpBody")]     = network->httpBody();
  result[QStringLiteral("httpHeaders")]  = network->httpHeaders();
  result[QStringLiteral("httpInterval")] = network->httpInterval();
  const auto& httpMethods                = network->httpMethods();
  if (network->httpMethodIndex() < httpMethods.count())
    result[QStringLiteral("httpMethod")] = httpMethods.at(network->httpMethodIndex());

  result[QStringLiteral("defaultAddress")]       = network->defaultAddress();
  result[QStringLiteral("defaultTcpPort")]       = network->defaultTcpPort();
  result[QStringLiteral("defaultUdpLocalPort")]  = network->defaultUdpLocalPort();
  result[QStringLiteral("defaultUdpRemotePort")] = network->defaultUdpRemotePort();
  result[QStringLiteral("defaultWebSocketUrl")]  = network->defaultWebSocketUrl();
  result[QStringLiteral("defaultHttpUrl")]       = network->defaultHttpUrl();
  result[QStringLiteral("defaultHttpInterval")]  = network->defaultHttpInterval();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the link state and the HTTP poll counters. The counters are plain values read on
 *        demand: nothing is pushed per poll.
 */
API::CommandResponse API::Handlers::NetworkHandler::getStatus(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* network = networkDriver();

  QJsonObject result;
  result[QStringLiteral("socketTypeIndex")] = network->socketTypeIndex();
  result[QStringLiteral("isOpen")]          = network->isOpen();
  result[QStringLiteral("isConnecting")]    = network->isConnecting();

  result[QStringLiteral("pollsOk")]      = static_cast<double>(network->pollsOk());
  result[QStringLiteral("pollsFailed")]  = static_cast<double>(network->pollsFailed());
  result[QStringLiteral("pollsSkipped")] = static_cast<double>(network->pollsSkipped());
  result[QStringLiteral("consecutiveFailures")] =
    static_cast<double>(network->consecutiveFailures());

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get available socket types
 */
API::CommandResponse API::Handlers::NetworkHandler::getSocketTypes(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& conn              = API::handlerContext().connectionManager;
  const auto& socketTypes = conn.network()->socketTypes();

  QJsonArray types;
  for (int i = 0; i < socketTypes.count(); ++i) {
    QJsonObject type;
    type[QStringLiteral("index")] = i;
    type[QStringLiteral("name")]  = socketTypes.at(i);
    types.append(type);
  }

  QJsonObject result;
  result[QStringLiteral("socketTypes")]            = types;
  result[QStringLiteral("currentSocketTypeIndex")] = conn.network()->socketTypeIndex();
  return CommandResponse::makeSuccess(id, result);
}
