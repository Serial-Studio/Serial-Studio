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

#include "ApiHandlers/UiHandlers.h"

#include "ApiHandlers/AlarmsHandler.h"
#include "ApiHandlers/AssistantHandler.h"
#include "ApiHandlers/ConsoleHandler.h"
#include "ApiHandlers/DashboardHandler.h"
#include "ApiHandlers/DiagnosticsHandler.h"
#include "ApiHandlers/ExtensionHandler.h"
#include "ApiHandlers/ProblemsHandler.h"
#include "ApiHandlers/WindowHandler.h"
#include "ApiHandlers/WorkspacesHandler.h"

#ifdef BUILD_COMMERCIAL
#  include "ApiHandlers/NotificationsHandler.h"
#endif

/**
 * @brief Registers the API commands whose handlers reach the user interface (spec 0077 T62): the
 *        composition root calls this right after the core set, so the command names, schemas and
 *        replies are exactly what the one registry held before the handlers moved up a layer.
 */
void UI::ApiHandlers::registerAll(const std::unique_ptr<UI::Alarms::AlarmAnnunciator>* annunciator)
{
  API::Handlers::AlarmsHandler::registerCommands(annunciator);
  API::Handlers::ConsoleHandler::registerCommands();
  API::Handlers::DashboardHandler::registerCommands();
  API::Handlers::WindowHandler::registerCommands();
  API::Handlers::ExtensionHandler::registerCommands();
  API::Handlers::WorkspacesHandler::registerCommands();
  API::Handlers::AssistantHandler::registerCommands();
  API::Handlers::ProblemsHandler::registerCommands();
  API::Handlers::DiagnosticsHandler::registerCommands();
#ifdef BUILD_COMMERCIAL
  API::Handlers::NotificationsHandler::registerCommands();
#endif
}
