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
 * @brief Handler for window/taskbar UI commands.
 */
class WindowHandler {
public:
  static void registerCommands();

private:
  static void registerStatusCommands();
  static void registerStateCommands();
  static void registerLayoutCommands();
  static void registerWidgetSettingCommands();

  static CommandResponse getStatus(const QString& id, const QJsonObject& params);
  static CommandResponse getGroups(const QString& id, const QJsonObject& params);
  static CommandResponse setActiveGroup(const QString& id, const QJsonObject& params);
  static CommandResponse setActiveGroupIndex(const QString& id, const QJsonObject& params);
  static CommandResponse getWindowStates(const QString& id, const QJsonObject& params);
  static CommandResponse setWindowState(const QString& id, const QJsonObject& params);
  static CommandResponse setAutoLayout(const QString& id, const QJsonObject& params);
  static CommandResponse saveLayout(const QString& id, const QJsonObject& params);
  static CommandResponse loadLayout(const QString& id, const QJsonObject& params);
  static CommandResponse getLayout(const QString& id, const QJsonObject& params);
  static CommandResponse setLayout(const QString& id, const QJsonObject& params);
  static CommandResponse getWidgetSettings(const QString& id, const QJsonObject& params);
  static CommandResponse setWidgetSetting(const QString& id, const QJsonObject& params);
};

}  // namespace API::Handlers
