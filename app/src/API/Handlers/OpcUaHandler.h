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
class CommandRegistry;

namespace Handlers {
/**
 * @brief Registers API commands for IO::Drivers::OpcUa operations (spec 0066 R14).
 */
class OpcUaHandler {
public:
  static void registerCommands();

private:
  static void registerConfigCommands(CommandRegistry& registry);
  static void registerDiscoveryCommands(CommandRegistry& registry);
  static void registerTagCommands(CommandRegistry& registry);
  static void registerQueryCommands(CommandRegistry& registry);
  static void registerSecurityCommands(CommandRegistry& registry);

  static CommandResponse setEndpointUrl(const QString& id, const QJsonObject& params);
  static CommandResponse setEndpointIndex(const QString& id, const QJsonObject& params);
  static CommandResponse setAuthMode(const QString& id, const QJsonObject& params);
  static CommandResponse setUsername(const QString& id, const QJsonObject& params);
  static CommandResponse setPassword(const QString& id, const QJsonObject& params);
  static CommandResponse setPublishingInterval(const QString& id, const QJsonObject& params);

  static CommandResponse setSecurityPolicy(const QString& id, const QJsonObject& params);
  static CommandResponse setSecurityMode(const QString& id, const QJsonObject& params);
  static CommandResponse setIdentityType(const QString& id, const QJsonObject& params);
  static CommandResponse setUserCertificate(const QString& id, const QJsonObject& params);
  static CommandResponse getCertificate(const QString& id, const QJsonObject& params);
  static CommandResponse regenerateCertificate(const QString& id, const QJsonObject& params);
  static CommandResponse exportCertificate(const QString& id, const QJsonObject& params);
  static CommandResponse listTrusted(const QString& id, const QJsonObject& params);
  static CommandResponse trustServer(const QString& id, const QJsonObject& params);
  static CommandResponse revokeTrust(const QString& id, const QJsonObject& params);

  static CommandResponse discoverEndpoints(const QString& id, const QJsonObject& params);
  static CommandResponse listEndpoints(const QString& id, const QJsonObject& params);
  static CommandResponse startBrowse(const QString& id, const QJsonObject& params);
  static CommandResponse browse(const QString& id, const QJsonObject& params);
  static CommandResponse stopBrowse(const QString& id, const QJsonObject& params);

  static CommandResponse listTags(const QString& id, const QJsonObject& params);
  static CommandResponse setTags(const QString& id, const QJsonObject& params);
  static CommandResponse addTag(const QString& id, const QJsonObject& params);
  static CommandResponse removeTag(const QString& id, const QJsonObject& params);
  static CommandResponse clearTags(const QString& id, const QJsonObject& params);
  static CommandResponse generateProject(const QString& id, const QJsonObject& params);

  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
  static CommandResponse getStatus(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
