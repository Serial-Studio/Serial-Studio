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

#pragma once

#ifdef BUILD_COMMERCIAL

#  include "API/CommandProtocol.h"

namespace API {
namespace Handlers {
/**
 * @class MQTTHandler
 * @brief Registers API commands for MQTT::Client operations
 *
 * Provides commands for:
 * - mqtt.setMode - Set publisher/subscriber mode
 * - mqtt.setHostname - Set broker hostname
 * - mqtt.setPort - Set broker port
 * - mqtt.setClientId - Set client ID
 * - mqtt.setUsername - Set username
 * - mqtt.setPassword - Set password
 * - mqtt.setTopic - Set topic filter
 * - mqtt.setCleanSession - Set clean session flag
 * - mqtt.setMqttVersion - Set MQTT protocol version
 * - mqtt.setKeepAlive - Set keep-alive interval
 * - mqtt.setAutoKeepAlive - Set auto keep-alive
 * - mqtt.setWillQoS - Set will message QoS
 * - mqtt.setWillRetain - Set will retain flag
 * - mqtt.setWillTopic - Set will topic
 * - mqtt.setWillMessage - Set will message
 * - mqtt.setSslEnabled - Enable/disable SSL
 * - mqtt.setSslProtocol - Set SSL protocol
 * - mqtt.setPeerVerifyMode - Set peer verification mode
 * - mqtt.setPeerVerifyDepth - Set peer verification depth
 * - mqtt.connect - Open MQTT connection
 * - mqtt.disconnect - Close MQTT connection
 * - mqtt.toggleConnection - Toggle connection state
 * - mqtt.regenerateClientId - Generate new client ID
 * - mqtt.getConfiguration - Query MQTT configuration
 * - mqtt.getConnectionStatus - Query connection state
 * - mqtt.getModes - Query mode list
 * - mqtt.getMqttVersions - Query MQTT version list
 * - mqtt.getSslProtocols - Query SSL protocol list
 * - mqtt.getPeerVerifyModes - Query peer verify mode list
 */
class MQTTHandler {
public:
  /**
   * @brief Register all MQTT commands with the CommandRegistry
   */
  static void registerCommands();

private:
  // Mutation commands
  static CommandResponse setMode(const QString& id, const QJsonObject& params);
  static CommandResponse setHostname(const QString& id, const QJsonObject& params);
  static CommandResponse setPort(const QString& id, const QJsonObject& params);
  static CommandResponse setClientId(const QString& id, const QJsonObject& params);
  static CommandResponse setUsername(const QString& id, const QJsonObject& params);
  static CommandResponse setPassword(const QString& id, const QJsonObject& params);
  static CommandResponse setTopic(const QString& id, const QJsonObject& params);
  static CommandResponse setCleanSession(const QString& id, const QJsonObject& params);
  static CommandResponse setMqttVersion(const QString& id, const QJsonObject& params);
  static CommandResponse setKeepAlive(const QString& id, const QJsonObject& params);
  static CommandResponse setAutoKeepAlive(const QString& id, const QJsonObject& params);
  static CommandResponse setWillQoS(const QString& id, const QJsonObject& params);
  static CommandResponse setWillRetain(const QString& id, const QJsonObject& params);
  static CommandResponse setWillTopic(const QString& id, const QJsonObject& params);
  static CommandResponse setWillMessage(const QString& id, const QJsonObject& params);
  static CommandResponse setSslEnabled(const QString& id, const QJsonObject& params);
  static CommandResponse setSslProtocol(const QString& id, const QJsonObject& params);
  static CommandResponse setPeerVerifyMode(const QString& id, const QJsonObject& params);
  static CommandResponse setPeerVerifyDepth(const QString& id, const QJsonObject& params);
  static CommandResponse connect(const QString& id, const QJsonObject& params);
  static CommandResponse disconnect(const QString& id, const QJsonObject& params);
  static CommandResponse toggleConnection(const QString& id, const QJsonObject& params);
  static CommandResponse regenerateClientId(const QString& id, const QJsonObject& params);

  // Query commands
  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
  static CommandResponse getConnectionStatus(const QString& id, const QJsonObject& params);
  static CommandResponse getModes(const QString& id, const QJsonObject& params);
  static CommandResponse getMqttVersions(const QString& id, const QJsonObject& params);
  static CommandResponse getSslProtocols(const QString& id, const QJsonObject& params);
  static CommandResponse getPeerVerifyModes(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API

#endif  // BUILD_COMMERCIAL
