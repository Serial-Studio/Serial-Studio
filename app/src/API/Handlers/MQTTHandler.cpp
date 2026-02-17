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

#ifdef BUILD_COMMERCIAL

#  include "API/Handlers/MQTTHandler.h"

#  include "API/CommandRegistry.h"
#  include "MQTT/Client.h"

/**
 * @brief Register all MQTT commands with the registry
 */
void API::Handlers::MQTTHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  // Mutation commands
  registry.registerCommand(
    QStringLiteral("mqtt.setMode"),
    QStringLiteral("Set MQTT mode (params: modeIndex - 0=Publisher, 1=Subscriber)"),
    &setMode);

  registry.registerCommand(QStringLiteral("mqtt.setHostname"),
                           QStringLiteral("Set MQTT broker hostname (params: hostname)"),
                           &setHostname);

  registry.registerCommand(QStringLiteral("mqtt.setPort"),
                           QStringLiteral("Set MQTT broker port (params: port)"),
                           &setPort);

  registry.registerCommand(QStringLiteral("mqtt.setClientId"),
                           QStringLiteral("Set MQTT client ID (params: clientId)"),
                           &setClientId);

  registry.registerCommand(QStringLiteral("mqtt.setUsername"),
                           QStringLiteral("Set MQTT username (params: username)"),
                           &setUsername);

  registry.registerCommand(QStringLiteral("mqtt.setPassword"),
                           QStringLiteral("Set MQTT password (params: password)"),
                           &setPassword);

  registry.registerCommand(QStringLiteral("mqtt.setTopic"),
                           QStringLiteral("Set MQTT topic filter (params: topic)"),
                           &setTopic);

  registry.registerCommand(QStringLiteral("mqtt.setCleanSession"),
                           QStringLiteral("Set clean session flag (params: enabled)"),
                           &setCleanSession);

  registry.registerCommand(QStringLiteral("mqtt.setMqttVersion"),
                           QStringLiteral("Set MQTT protocol version (params: versionIndex)"),
                           &setMqttVersion);

  registry.registerCommand(QStringLiteral("mqtt.setKeepAlive"),
                           QStringLiteral("Set keep-alive interval in seconds (params: seconds)"),
                           &setKeepAlive);

  registry.registerCommand(QStringLiteral("mqtt.setAutoKeepAlive"),
                           QStringLiteral("Set auto keep-alive (params: enabled)"),
                           &setAutoKeepAlive);

  registry.registerCommand(QStringLiteral("mqtt.setWillQoS"),
                           QStringLiteral("Set will message QoS (params: qos - 0, 1, or 2)"),
                           &setWillQoS);

  registry.registerCommand(QStringLiteral("mqtt.setWillRetain"),
                           QStringLiteral("Set will message retain flag (params: enabled)"),
                           &setWillRetain);

  registry.registerCommand(QStringLiteral("mqtt.setWillTopic"),
                           QStringLiteral("Set will message topic (params: topic)"),
                           &setWillTopic);

  registry.registerCommand(QStringLiteral("mqtt.setWillMessage"),
                           QStringLiteral("Set will message payload (params: message)"),
                           &setWillMessage);

  registry.registerCommand(QStringLiteral("mqtt.setSslEnabled"),
                           QStringLiteral("Enable/disable SSL (params: enabled)"),
                           &setSslEnabled);

  registry.registerCommand(QStringLiteral("mqtt.setSslProtocol"),
                           QStringLiteral("Set SSL protocol (params: protocolIndex)"),
                           &setSslProtocol);

  registry.registerCommand(QStringLiteral("mqtt.setPeerVerifyMode"),
                           QStringLiteral("Set peer verification mode (params: modeIndex)"),
                           &setPeerVerifyMode);

  registry.registerCommand(QStringLiteral("mqtt.setPeerVerifyDepth"),
                           QStringLiteral("Set peer verification depth (params: depth)"),
                           &setPeerVerifyDepth);

  registry.registerCommand(
    QStringLiteral("mqtt.connect"), QStringLiteral("Open MQTT connection"), &connect);

  registry.registerCommand(
    QStringLiteral("mqtt.disconnect"), QStringLiteral("Close MQTT connection"), &disconnect);

  registry.registerCommand(QStringLiteral("mqtt.toggleConnection"),
                           QStringLiteral("Toggle MQTT connection state"),
                           &toggleConnection);

  registry.registerCommand(QStringLiteral("mqtt.regenerateClientId"),
                           QStringLiteral("Generate new random client ID"),
                           &regenerateClientId);

  // Query commands
  registry.registerCommand(QStringLiteral("mqtt.getConfiguration"),
                           QStringLiteral("Get current MQTT configuration"),
                           &getConfiguration);

  registry.registerCommand(QStringLiteral("mqtt.getConnectionStatus"),
                           QStringLiteral("Get MQTT connection status"),
                           &getConnectionStatus);

  registry.registerCommand(
    QStringLiteral("mqtt.getModes"), QStringLiteral("Get available MQTT modes"), &getModes);

  registry.registerCommand(QStringLiteral("mqtt.getMqttVersions"),
                           QStringLiteral("Get available MQTT versions"),
                           &getMqttVersions);

  registry.registerCommand(QStringLiteral("mqtt.getSslProtocols"),
                           QStringLiteral("Get available SSL protocols"),
                           &getSslProtocols);

  registry.registerCommand(QStringLiteral("mqtt.getPeerVerifyModes"),
                           QStringLiteral("Get available peer verify modes"),
                           &getPeerVerifyModes);
}

/**
 * @brief Set MQTT mode (Publisher or Subscriber)
 * @param params Requires "modeIndex" (int: 0=Publisher, 1=Subscriber)
 */
API::CommandResponse API::Handlers::MQTTHandler::setMode(const QString& id,
                                                         const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("modeIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: modeIndex"));
  }

  const int modeIndex = params.value(QStringLiteral("modeIndex")).toInt();
  auto& mqtt          = MQTT::Client::instance();
  const auto& modes   = mqtt.modes();

  if (modeIndex < 0 || modeIndex >= modes.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid modeIndex: %1. Valid range: 0-%2")
                                        .arg(modeIndex)
                                        .arg(modes.count() - 1));
  }

  mqtt.setMode(static_cast<quint8>(modeIndex));

  QJsonObject result;
  result[QStringLiteral("modeIndex")] = modeIndex;
  result[QStringLiteral("modeName")]  = modes.at(modeIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set MQTT broker hostname
 * @param params Requires "hostname" (string)
 */
API::CommandResponse API::Handlers::MQTTHandler::setHostname(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("hostname"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: hostname"));
  }

  const QString hostname = params.value(QStringLiteral("hostname")).toString();

  if (hostname.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Hostname cannot be empty"));
  }

  MQTT::Client::instance().setHostname(hostname);

  QJsonObject result;
  result[QStringLiteral("hostname")] = hostname;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set MQTT broker port
 * @param params Requires "port" (int)
 */
API::CommandResponse API::Handlers::MQTTHandler::setPort(const QString& id,
                                                         const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("port"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: port"));
  }

  const int port = params.value(QStringLiteral("port")).toInt();

  if (port < 1 || port > 65535) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid port: %1. Valid range: 1-65535").arg(port));
  }

  MQTT::Client::instance().setPort(static_cast<quint16>(port));

  QJsonObject result;
  result[QStringLiteral("port")] = port;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set MQTT client ID
 * @param params Requires "clientId" (string)
 */
API::CommandResponse API::Handlers::MQTTHandler::setClientId(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("clientId"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: clientId"));
  }

  const QString clientId = params.value(QStringLiteral("clientId")).toString();

  MQTT::Client::instance().setClientId(clientId);

  QJsonObject result;
  result[QStringLiteral("clientId")] = clientId;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set MQTT username
 * @param params Requires "username" (string)
 */
API::CommandResponse API::Handlers::MQTTHandler::setUsername(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("username"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: username"));
  }

  const QString username = params.value(QStringLiteral("username")).toString();

  MQTT::Client::instance().setUsername(username);

  QJsonObject result;
  result[QStringLiteral("username")] = username;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set MQTT password
 * @param params Requires "password" (string)
 */
API::CommandResponse API::Handlers::MQTTHandler::setPassword(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("password"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: password"));
  }

  const QString password = params.value(QStringLiteral("password")).toString();

  MQTT::Client::instance().setPassword(password);

  QJsonObject result;
  result[QStringLiteral("passwordSet")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set MQTT topic filter
 * @param params Requires "topic" (string)
 */
API::CommandResponse API::Handlers::MQTTHandler::setTopic(const QString& id,
                                                          const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("topic"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: topic"));
  }

  const QString topic = params.value(QStringLiteral("topic")).toString();

  MQTT::Client::instance().setTopic(topic);

  QJsonObject result;
  result[QStringLiteral("topic")] = topic;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set clean session flag
 * @param params Requires "enabled" (bool)
 */
API::CommandResponse API::Handlers::MQTTHandler::setCleanSession(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();

  MQTT::Client::instance().setCleanSession(enabled);

  QJsonObject result;
  result[QStringLiteral("cleanSession")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set MQTT protocol version
 * @param params Requires "versionIndex" (int)
 */
API::CommandResponse API::Handlers::MQTTHandler::setMqttVersion(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("versionIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: versionIndex"));
  }

  const int versionIndex = params.value(QStringLiteral("versionIndex")).toInt();
  auto& mqtt             = MQTT::Client::instance();
  const auto& versions   = mqtt.mqttVersions();

  if (versionIndex < 0 || versionIndex >= versions.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid versionIndex: %1. Valid range: 0-%2")
                                        .arg(versionIndex)
                                        .arg(versions.count() - 1));
  }

  mqtt.setMqttVersion(static_cast<quint8>(versionIndex));

  QJsonObject result;
  result[QStringLiteral("versionIndex")] = versionIndex;
  result[QStringLiteral("versionName")]  = versions.at(versionIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set keep-alive interval
 * @param params Requires "seconds" (int)
 */
API::CommandResponse API::Handlers::MQTTHandler::setKeepAlive(const QString& id,
                                                              const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("seconds"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: seconds"));
  }

  const int seconds = params.value(QStringLiteral("seconds")).toInt();

  if (seconds < 0) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Keep-alive seconds must be non-negative"));
  }

  MQTT::Client::instance().setKeepAlive(static_cast<quint16>(seconds));

  QJsonObject result;
  result[QStringLiteral("keepAlive")] = seconds;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set auto keep-alive
 * @param params Requires "enabled" (bool)
 */
API::CommandResponse API::Handlers::MQTTHandler::setAutoKeepAlive(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();

  MQTT::Client::instance().setAutoKeepAlive(enabled);

  QJsonObject result;
  result[QStringLiteral("autoKeepAlive")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set will message QoS
 * @param params Requires "qos" (int: 0, 1, or 2)
 */
API::CommandResponse API::Handlers::MQTTHandler::setWillQoS(const QString& id,
                                                            const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("qos"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: qos"));
  }

  const int qos = params.value(QStringLiteral("qos")).toInt();

  if (qos < 0 || qos > 2) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid QoS: %1. Valid values: 0, 1, or 2").arg(qos));
  }

  MQTT::Client::instance().setWillQoS(static_cast<quint8>(qos));

  QJsonObject result;
  result[QStringLiteral("willQoS")] = qos;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set will message retain flag
 * @param params Requires "enabled" (bool)
 */
API::CommandResponse API::Handlers::MQTTHandler::setWillRetain(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();

  MQTT::Client::instance().setWillRetain(enabled);

  QJsonObject result;
  result[QStringLiteral("willRetain")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set will message topic
 * @param params Requires "topic" (string)
 */
API::CommandResponse API::Handlers::MQTTHandler::setWillTopic(const QString& id,
                                                              const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("topic"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: topic"));
  }

  const QString topic = params.value(QStringLiteral("topic")).toString();

  MQTT::Client::instance().setWillTopic(topic);

  QJsonObject result;
  result[QStringLiteral("willTopic")] = topic;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set will message payload
 * @param params Requires "message" (string)
 */
API::CommandResponse API::Handlers::MQTTHandler::setWillMessage(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("message"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: message"));
  }

  const QString message = params.value(QStringLiteral("message")).toString();

  MQTT::Client::instance().setWillMessage(message);

  QJsonObject result;
  result[QStringLiteral("willMessage")] = message;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Enable or disable SSL
 * @param params Requires "enabled" (bool)
 */
API::CommandResponse API::Handlers::MQTTHandler::setSslEnabled(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();

  MQTT::Client::instance().setSslEnabled(enabled);

  QJsonObject result;
  result[QStringLiteral("sslEnabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set SSL protocol
 * @param params Requires "protocolIndex" (int)
 */
API::CommandResponse API::Handlers::MQTTHandler::setSslProtocol(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("protocolIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: protocolIndex"));
  }

  const int protocolIndex = params.value(QStringLiteral("protocolIndex")).toInt();
  auto& mqtt              = MQTT::Client::instance();
  const auto& protocols   = mqtt.sslProtocols();

  if (protocolIndex < 0 || protocolIndex >= protocols.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid protocolIndex: %1. Valid range: 0-%2")
                                        .arg(protocolIndex)
                                        .arg(protocols.count() - 1));
  }

  mqtt.setSslProtocol(static_cast<quint8>(protocolIndex));

  QJsonObject result;
  result[QStringLiteral("protocolIndex")] = protocolIndex;
  result[QStringLiteral("protocolName")]  = protocols.at(protocolIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set peer verification mode
 * @param params Requires "modeIndex" (int)
 */
API::CommandResponse API::Handlers::MQTTHandler::setPeerVerifyMode(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("modeIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: modeIndex"));
  }

  const int modeIndex = params.value(QStringLiteral("modeIndex")).toInt();
  auto& mqtt          = MQTT::Client::instance();
  const auto& modes   = mqtt.peerVerifyModes();

  if (modeIndex < 0 || modeIndex >= modes.count()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Invalid modeIndex: %1. Valid range: 0-%2")
                                        .arg(modeIndex)
                                        .arg(modes.count() - 1));
  }

  mqtt.setPeerVerifyMode(static_cast<quint8>(modeIndex));

  QJsonObject result;
  result[QStringLiteral("modeIndex")] = modeIndex;
  result[QStringLiteral("modeName")]  = modes.at(modeIndex);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set peer verification depth
 * @param params Requires "depth" (int)
 */
API::CommandResponse API::Handlers::MQTTHandler::setPeerVerifyDepth(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("depth"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: depth"));
  }

  const int depth = params.value(QStringLiteral("depth")).toInt();

  if (depth < 0) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Peer verify depth must be non-negative"));
  }

  MQTT::Client::instance().setPeerVerifyDepth(depth);

  QJsonObject result;
  result[QStringLiteral("peerVerifyDepth")] = depth;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Open MQTT connection
 */
API::CommandResponse API::Handlers::MQTTHandler::connect(const QString& id,
                                                         const QJsonObject& params)
{
  Q_UNUSED(params)

  MQTT::Client::instance().openConnection();

  QJsonObject result;
  result[QStringLiteral("connecting")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Close MQTT connection
 */
API::CommandResponse API::Handlers::MQTTHandler::disconnect(const QString& id,
                                                            const QJsonObject& params)
{
  Q_UNUSED(params)

  MQTT::Client::instance().closeConnection();

  QJsonObject result;
  result[QStringLiteral("disconnecting")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Toggle MQTT connection state
 */
API::CommandResponse API::Handlers::MQTTHandler::toggleConnection(const QString& id,
                                                                  const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& mqtt              = MQTT::Client::instance();
  const bool wasConnected = mqtt.isConnected();

  mqtt.toggleConnection();

  QJsonObject result;
  result[QStringLiteral("wasConnected")] = wasConnected;
  result[QStringLiteral("nowConnected")] = !wasConnected;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Generate new random client ID
 */
API::CommandResponse API::Handlers::MQTTHandler::regenerateClientId(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& mqtt = MQTT::Client::instance();
  mqtt.regenerateClientId();

  QJsonObject result;
  result[QStringLiteral("clientId")] = mqtt.clientId();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get current MQTT configuration
 */
API::CommandResponse API::Handlers::MQTTHandler::getConfiguration(const QString& id,
                                                                  const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& mqtt = MQTT::Client::instance();

  QJsonObject result;

  // Connection settings
  result[QStringLiteral("hostname")]     = mqtt.hostname();
  result[QStringLiteral("port")]         = mqtt.port();
  result[QStringLiteral("clientId")]     = mqtt.clientId();
  result[QStringLiteral("username")]     = mqtt.username();
  result[QStringLiteral("cleanSession")] = mqtt.cleanSession();

  // Mode
  result[QStringLiteral("modeIndex")] = mqtt.mode();
  const auto& modes                   = mqtt.modes();
  if (mqtt.mode() < modes.count())
    result[QStringLiteral("modeName")] = modes.at(mqtt.mode());

  // Topic
  result[QStringLiteral("topic")] = mqtt.topicFilter();

  // MQTT version
  result[QStringLiteral("mqttVersionIndex")] = mqtt.mqttVersion();
  const auto& versions                       = mqtt.mqttVersions();
  if (mqtt.mqttVersion() < versions.count())
    result[QStringLiteral("mqttVersionName")] = versions.at(mqtt.mqttVersion());

  // Keep-alive
  result[QStringLiteral("keepAlive")]     = mqtt.keepAlive();
  result[QStringLiteral("autoKeepAlive")] = mqtt.autoKeepAlive();

  // Will message
  result[QStringLiteral("willQoS")]     = mqtt.willQoS();
  result[QStringLiteral("willRetain")]  = mqtt.willRetain();
  result[QStringLiteral("willTopic")]   = mqtt.willTopic();
  result[QStringLiteral("willMessage")] = mqtt.willMessage();

  // SSL
  result[QStringLiteral("sslEnabled")]       = mqtt.sslEnabled();
  result[QStringLiteral("sslProtocolIndex")] = mqtt.sslProtocol();
  const auto& sslProtocols                   = mqtt.sslProtocols();
  if (mqtt.sslProtocol() < sslProtocols.count())
    result[QStringLiteral("sslProtocolName")] = sslProtocols.at(mqtt.sslProtocol());

  result[QStringLiteral("peerVerifyModeIndex")] = mqtt.peerVerifyMode();
  const auto& verifyModes                       = mqtt.peerVerifyModes();
  if (mqtt.peerVerifyMode() < verifyModes.count())
    result[QStringLiteral("peerVerifyModeName")] = verifyModes.at(mqtt.peerVerifyMode());

  result[QStringLiteral("peerVerifyDepth")] = mqtt.peerVerifyDepth();

  // Connection status
  result[QStringLiteral("isConnected")] = mqtt.isConnected();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get MQTT connection status
 */
API::CommandResponse API::Handlers::MQTTHandler::getConnectionStatus(const QString& id,
                                                                     const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& mqtt = MQTT::Client::instance();

  QJsonObject result;
  result[QStringLiteral("isConnected")]  = mqtt.isConnected();
  result[QStringLiteral("isPublisher")]  = mqtt.isPublisher();
  result[QStringLiteral("isSubscriber")] = mqtt.isSubscriber();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get available MQTT modes
 */
API::CommandResponse API::Handlers::MQTTHandler::getModes(const QString& id,
                                                          const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& modes = MQTT::Client::instance().modes();

  QJsonArray modesArray;
  for (int i = 0; i < modes.count(); ++i) {
    QJsonObject mode;
    mode[QStringLiteral("index")] = i;
    mode[QStringLiteral("name")]  = modes.at(i);
    modesArray.append(mode);
  }

  QJsonObject result;
  result[QStringLiteral("modes")]            = modesArray;
  result[QStringLiteral("currentModeIndex")] = MQTT::Client::instance().mode();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get available MQTT versions
 */
API::CommandResponse API::Handlers::MQTTHandler::getMqttVersions(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& versions = MQTT::Client::instance().mqttVersions();

  QJsonArray versionsArray;
  for (int i = 0; i < versions.count(); ++i) {
    QJsonObject version;
    version[QStringLiteral("index")] = i;
    version[QStringLiteral("name")]  = versions.at(i);
    versionsArray.append(version);
  }

  QJsonObject result;
  result[QStringLiteral("mqttVersions")]        = versionsArray;
  result[QStringLiteral("currentVersionIndex")] = MQTT::Client::instance().mqttVersion();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get available SSL protocols
 */
API::CommandResponse API::Handlers::MQTTHandler::getSslProtocols(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& protocols = MQTT::Client::instance().sslProtocols();

  QJsonArray protocolsArray;
  for (int i = 0; i < protocols.count(); ++i) {
    QJsonObject protocol;
    protocol[QStringLiteral("index")] = i;
    protocol[QStringLiteral("name")]  = protocols.at(i);
    protocolsArray.append(protocol);
  }

  QJsonObject result;
  result[QStringLiteral("sslProtocols")]         = protocolsArray;
  result[QStringLiteral("currentProtocolIndex")] = MQTT::Client::instance().sslProtocol();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get available peer verify modes
 */
API::CommandResponse API::Handlers::MQTTHandler::getPeerVerifyModes(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& modes = MQTT::Client::instance().peerVerifyModes();

  QJsonArray modesArray;
  for (int i = 0; i < modes.count(); ++i) {
    QJsonObject mode;
    mode[QStringLiteral("index")] = i;
    mode[QStringLiteral("name")]  = modes.at(i);
    modesArray.append(mode);
  }

  QJsonObject result;
  result[QStringLiteral("peerVerifyModes")]  = modesArray;
  result[QStringLiteral("currentModeIndex")] = MQTT::Client::instance().peerVerifyMode();
  return CommandResponse::makeSuccess(id, result);
}

#endif  // BUILD_COMMERCIAL
