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
 * @brief Handler for dashboard configuration commands
 *
 * Provides API commands to configure dashboard settings including:
 * - Operation mode (ProjectFile, DeviceSendsJSON, QuickPlot)
 * - Visualization refresh rate (FPS)
 * - Data points per plot
 */
class DashboardHandler {
public:
  static void registerCommands();

private:
  static CommandResponse setOperationMode(const QString& id, const QJsonObject& params);
  static CommandResponse getOperationMode(const QString& id, const QJsonObject& params);
  static CommandResponse setFPS(const QString& id, const QJsonObject& params);
  static CommandResponse getFPS(const QString& id, const QJsonObject& params);
  static CommandResponse setPoints(const QString& id, const QJsonObject& params);
  static CommandResponse getPoints(const QString& id, const QJsonObject& params);
  static CommandResponse getStatus(const QString& id, const QJsonObject& params);
  static CommandResponse getData(const QString& id, const QJsonObject& params);
};

}  // namespace API::Handlers
