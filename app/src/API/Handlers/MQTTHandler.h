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

#include "API/CommandProtocol.h"

namespace API {
namespace Handlers {
/**
 * @brief Registers API commands for MQTT::Client operations.
 */
class MQTTHandler {
public:
  static void registerCommands();

private:
  static void registerConnectionCommands();
  static void registerAuthCommands();
  static void registerWillCommands();
  static void registerSslCommands();
  static void registerQueryCommands();

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

  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
  static CommandResponse getConnectionStatus(const QString& id, const QJsonObject& params);
  static CommandResponse getModes(const QString& id, const QJsonObject& params);
  static CommandResponse getMqttVersions(const QString& id, const QJsonObject& params);
  static CommandResponse getSslProtocols(const QString& id, const QJsonObject& params);
  static CommandResponse getPeerVerifyModes(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
