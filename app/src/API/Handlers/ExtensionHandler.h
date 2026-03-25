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

#pragma once

#include "API/CommandProtocol.h"

namespace API::Handlers {
/**
 * @brief Handler for extension management commands
 *
 * Provides API commands to manage extension repositories and installed extensions:
 * - List available and installed extensions
 * - Install/uninstall addons by ID
 * - Manage repository URLs
 * - Refresh repository catalogs
 */
class ExtensionHandler {
public:
  static void registerCommands();

private:
  static CommandResponse listAddons(const QString& id, const QJsonObject& params);
  static CommandResponse getAddonInfo(const QString& id, const QJsonObject& params);
  static CommandResponse installExtension(const QString& id, const QJsonObject& params);
  static CommandResponse uninstallExtension(const QString& id, const QJsonObject& params);
  static CommandResponse listRepositories(const QString& id, const QJsonObject& params);
  static CommandResponse addRepository(const QString& id, const QJsonObject& params);
  static CommandResponse removeRepository(const QString& id, const QJsonObject& params);
  static CommandResponse refreshRepositories(const QString& id, const QJsonObject& params);
  static CommandResponse savePluginState(const QString& id, const QJsonObject& params);
  static CommandResponse loadPluginState(const QString& id, const QJsonObject& params);
};

}  // namespace API::Handlers
