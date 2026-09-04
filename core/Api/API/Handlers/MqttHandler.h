/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include "API/CommandProtocol.h"

namespace API {
namespace Handlers {

/**
 * @brief Registers API commands for the MQTT publisher and subscriber driver.
 */
class MqttHandler {
public:
  static void registerCommands();

private:
  static void registerPublisherCommands();
  static void registerSubscriberCommands();

  static CommandResponse publisherGetConfig(const QString& id, const QJsonObject& params);
  static CommandResponse publisherSetConfig(const QString& id, const QJsonObject& params);
  static CommandResponse publisherGetStatus(const QString& id, const QJsonObject& params);

  static CommandResponse subscriberGetConfig(const QString& id, const QJsonObject& params);
  static CommandResponse subscriberSetConfig(const QString& id, const QJsonObject& params);
  static CommandResponse subscriberGetStatus(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API

#endif  // BUILD_COMMERCIAL
