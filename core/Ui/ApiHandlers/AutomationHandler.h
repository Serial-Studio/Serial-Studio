/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#pragma once

#include "Core/Api/CommandProtocol.h"

namespace API::Handlers {
/**
 * @brief Handler for the ui.automation.* commands (spec 0101): window inventory, element
 *        catalog, click, key, screenshot and the reduce-motion switch over the live Qt Quick
 *        windows. A headless root loads no QML, so every command but listWindows answers "no
 *        window". Click and key carry DrivesUi, the capability behind the third consent.
 */
class AutomationHandler {
public:
  static void registerCommands();

private:
  static void registerReadCommands();
  static void registerInputCommands();

  static CommandResponse listWindows(const QString& id, const QJsonObject& params);
  static CommandResponse listElements(const QString& id, const QJsonObject& params);
  static CommandResponse click(const QString& id, const QJsonObject& params);
  static CommandResponse key(const QString& id, const QJsonObject& params);
  static CommandResponse screenshot(const QString& id, const QJsonObject& params);
  static CommandResponse setReduceMotion(const QString& id, const QJsonObject& params);
};

}  // namespace API::Handlers
