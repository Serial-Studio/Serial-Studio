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

#include "API/Handlers/DashboardHandler.h"

#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "Misc/TimerEvents.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all dashboard configuration commands with the registry
 */
void API::Handlers::DashboardHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  // Empty schema for parameter-less commands
  QJsonObject emptySchema;
  emptySchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  emptySchema.insert(QStringLiteral("properties"), QJsonObject());

  // Schema for setOperationMode
  QJsonObject modeProps;
  QJsonObject modeProp;
  modeProp.insert(QStringLiteral("type"), QStringLiteral("integer"));
  modeProp.insert(
    QStringLiteral("description"),
    QStringLiteral("Operation mode: 0=ProjectFile, 1=DeviceSendsJSON, 2=QuickPlot"));
  modeProps.insert(QStringLiteral("mode"), modeProp);

  QJsonObject setModeSchema;
  setModeSchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  setModeSchema.insert(QStringLiteral("properties"), modeProps);
  setModeSchema.insert(QStringLiteral("required"), QJsonArray{QStringLiteral("mode")});

  registry.registerCommand(
    QStringLiteral("dashboard.setOperationMode"),
    QStringLiteral(
      "Set the operation mode (params: mode - 0=ProjectFile, 1=DeviceSendsJSON, 2=QuickPlot)"),
    setModeSchema,
    &setOperationMode);

  registry.registerCommand(QStringLiteral("dashboard.getOperationMode"),
                           QStringLiteral("Get the current operation mode"),
                           emptySchema,
                           &getOperationMode);

  // Schema for setFPS
  QJsonObject fpsProps;
  QJsonObject fpsProp;
  fpsProp.insert(QStringLiteral("type"), QStringLiteral("integer"));
  fpsProp.insert(QStringLiteral("description"),
                 QStringLiteral("Visualization refresh rate in Hz (1-240)"));
  fpsProps.insert(QStringLiteral("fps"), fpsProp);

  QJsonObject setFpsSchema;
  setFpsSchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  setFpsSchema.insert(QStringLiteral("properties"), fpsProps);
  setFpsSchema.insert(QStringLiteral("required"), QJsonArray{QStringLiteral("fps")});

  registry.registerCommand(
    QStringLiteral("dashboard.setFPS"),
    QStringLiteral("Set the visualization refresh rate (params: fps - 1-240 Hz)"),
    setFpsSchema,
    &setFPS);

  registry.registerCommand(QStringLiteral("dashboard.getFPS"),
                           QStringLiteral("Get the current visualization refresh rate"),
                           emptySchema,
                           &getFPS);

  // Schema for setPoints
  QJsonObject pointsProps;
  QJsonObject pointsProp;
  pointsProp.insert(QStringLiteral("type"), QStringLiteral("integer"));
  pointsProp.insert(QStringLiteral("description"),
                    QStringLiteral("Number of data points per plot (1-100000)"));
  pointsProps.insert(QStringLiteral("points"), pointsProp);

  QJsonObject setPointsSchema;
  setPointsSchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  setPointsSchema.insert(QStringLiteral("properties"), pointsProps);
  setPointsSchema.insert(QStringLiteral("required"), QJsonArray{QStringLiteral("points")});

  registry.registerCommand(
    QStringLiteral("dashboard.setPoints"),
    QStringLiteral("Set the number of data points per plot (params: points)"),
    setPointsSchema,
    &setPoints);

  registry.registerCommand(QStringLiteral("dashboard.getPoints"),
                           QStringLiteral("Get the current number of data points per plot"),
                           emptySchema,
                           &getPoints);

  registry.registerCommand(QStringLiteral("dashboard.getStatus"),
                           QStringLiteral("Get all dashboard configuration settings"),
                           emptySchema,
                           &getStatus);

  registry.registerCommand(QStringLiteral("dashboard.getData"),
                           QStringLiteral("Get dashboard widget counts and latest frame data"),
                           emptySchema,
                           &getData);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Set the operation mode
 * @param params Requires "mode" (int: 0=ProjectFile, 1=DeviceSendsJSON,
 * 2=QuickPlot)
 */
API::CommandResponse API::Handlers::DashboardHandler::setOperationMode(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("mode"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: mode"));
  }

  const int mode = params.value(QStringLiteral("mode")).toInt();

  if (mode < 0 || mode > 2) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral(
        "Invalid mode: %1. Valid range: 0-2 (0=ProjectFile, 1=DeviceSendsJSON, 2=QuickPlot)")
        .arg(mode));
  }

  const auto operationMode = static_cast<SerialStudio::OperationMode>(mode);
  AppState::instance().setOperationMode(operationMode);

  QJsonObject result;
  result[QStringLiteral("mode")] = mode;

  const QStringList modeNames = {
    QStringLiteral("ProjectFile"), QStringLiteral("DeviceSendsJSON"), QStringLiteral("QuickPlot")};
  result[QStringLiteral("modeName")] = modeNames[mode];

  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get the current operation mode
 */
API::CommandResponse API::Handlers::DashboardHandler::getOperationMode(const QString& id,
                                                                       const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto mode     = AppState::instance().operationMode();
  const int modeIndex = static_cast<int>(mode);

  QJsonObject result;
  result[QStringLiteral("mode")] = modeIndex;

  const QStringList modeNames = {
    QStringLiteral("ProjectFile"), QStringLiteral("DeviceSendsJSON"), QStringLiteral("QuickPlot")};
  result[QStringLiteral("modeName")] = modeNames[modeIndex];

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the visualization refresh rate (FPS)
 * @param params Requires "fps" (int: 1-240 Hz)
 */
API::CommandResponse API::Handlers::DashboardHandler::setFPS(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("fps"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: fps"));
  }

  const int fps = params.value(QStringLiteral("fps")).toInt();

  if (fps < 1 || fps > 240) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid fps: %1. Valid range: 1-240").arg(fps));
  }

  Misc::TimerEvents::instance().setFPS(fps);

  QJsonObject result;
  result[QStringLiteral("fps")] = fps;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the current visualization refresh rate
 */
API::CommandResponse API::Handlers::DashboardHandler::getFPS(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  const int fps = Misc::TimerEvents::instance().fps();

  QJsonObject result;
  result[QStringLiteral("fps")] = fps;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the number of data points per plot
 * @param params Requires "points" (int)
 */
API::CommandResponse API::Handlers::DashboardHandler::setPoints(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("points"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: points"));
  }

  const int points = params.value(QStringLiteral("points")).toInt();

  if (points < 1 || points > 100000) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid points: %1. Valid range: 1-100000").arg(points));
  }

  UI::Dashboard::instance().setPoints(points);

  QJsonObject result;
  result[QStringLiteral("points")] = points;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the current number of data points per plot
 */
API::CommandResponse API::Handlers::DashboardHandler::getPoints(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  const int points = UI::Dashboard::instance().points();

  QJsonObject result;
  result[QStringLiteral("points")] = points;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get all dashboard configuration settings
 */
API::CommandResponse API::Handlers::DashboardHandler::getStatus(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto mode     = AppState::instance().operationMode();
  const int modeIndex = static_cast<int>(mode);
  const int fps       = Misc::TimerEvents::instance().fps();
  const int points    = UI::Dashboard::instance().points();

  QJsonObject result;
  result[QStringLiteral("operationMode")] = modeIndex;

  const QStringList modeNames = {
    QStringLiteral("ProjectFile"), QStringLiteral("DeviceSendsJSON"), QStringLiteral("QuickPlot")};
  result[QStringLiteral("operationModeName")] = modeNames[modeIndex];
  result[QStringLiteral("fps")]               = fps;
  result[QStringLiteral("points")]            = points;
  result[QStringLiteral("widgetCount")]       = UI::Dashboard::instance().totalWidgetCount();
  result[QStringLiteral("datasetCount")]      = UI::Dashboard::instance().datasets().size();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get dashboard widget counts and latest processed frame data
 */
API::CommandResponse API::Handlers::DashboardHandler::getData(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& dashboard = UI::Dashboard::instance();

  QJsonObject result;
  result[QStringLiteral("widgetCount")]  = dashboard.totalWidgetCount();
  result[QStringLiteral("datasetCount")] = dashboard.datasets().size();
  result[QStringLiteral("frame")]        = DataModel::serialize(dashboard.processedFrame());

  return CommandResponse::makeSuccess(id, result);
}
