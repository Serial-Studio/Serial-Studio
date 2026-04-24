/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include "API/CommandProtocol.h"

namespace API {
namespace Handlers {

/**
 * @brief Registers API commands for the dashboard NotificationCenter.
 */
class NotificationsHandler {
public:
  static void registerCommands();

private:
  static CommandResponse post(const QString& id, const QJsonObject& params);
  static CommandResponse postInfo(const QString& id, const QJsonObject& params);
  static CommandResponse postWarning(const QString& id, const QJsonObject& params);
  static CommandResponse postCritical(const QString& id, const QJsonObject& params);
  static CommandResponse resolve(const QString& id, const QJsonObject& params);

  static CommandResponse list(const QString& id, const QJsonObject& params);
  static CommandResponse channels(const QString& id, const QJsonObject& params);
  static CommandResponse unreadCount(const QString& id, const QJsonObject& params);

  static CommandResponse clearChannel(const QString& id, const QJsonObject& params);
  static CommandResponse clearAll(const QString& id, const QJsonObject& params);
  static CommandResponse markRead(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API

#endif  // BUILD_COMMERCIAL
