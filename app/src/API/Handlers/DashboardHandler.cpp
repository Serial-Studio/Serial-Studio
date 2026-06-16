/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
#include "API/EnumLabels.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DSP.h"
#include "Misc/TimerEvents.h"
#include "SerialStudio.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all dashboard configuration commands with the registry
 */
void API::Handlers::DashboardHandler::registerCommands()
{
  registerModeAndFpsCommands();
  registerTimeRangeCommands();
  registerQueryCommands();
}

/**
 * @brief Register operation-mode and refresh-rate (FPS) commands.
 */
void API::Handlers::DashboardHandler::registerModeAndFpsCommands()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject emptySchema;
  emptySchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  emptySchema.insert(QStringLiteral("properties"), QJsonObject());

  QJsonObject modeProps;
  QJsonObject modeProp;
  modeProp.insert(QStringLiteral("type"), QStringLiteral("integer"));
  modeProp.insert(QStringLiteral("description"),
                  QStringLiteral("Operation mode: 0=ProjectFile, 1=ConsoleOnly, 2=QuickPlot"));
  modeProps.insert(QStringLiteral("mode"), modeProp);

  QJsonObject setModeSchema;
  setModeSchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  setModeSchema.insert(QStringLiteral("properties"), modeProps);
  setModeSchema.insert(QStringLiteral("required"), QJsonArray{QStringLiteral("mode")});

  registry.registerCommand(
    QStringLiteral("dashboard.setOperationMode"),
    QStringLiteral(
      "Set the operation mode (params: mode - 0=ProjectFile, 1=ConsoleOnly, 2=QuickPlot)"),
    setModeSchema,
    &setOperationMode);

  registry.registerCommand(QStringLiteral("dashboard.getOperationMode"),
                           QStringLiteral("Get the current operation mode"),
                           emptySchema,
                           &getOperationMode);

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
    QStringLiteral("dashboard.setFps"),
    QStringLiteral("Set the visualization refresh rate (params: fps - 1-240 Hz)"),
    setFpsSchema,
    &setFPS);

  registry.registerCommand(QStringLiteral("dashboard.getFps"),
                           QStringLiteral("Get the current visualization refresh rate"),
                           emptySchema,
                           &getFPS);
}

/**
 * @brief Register plot time-range get/set commands.
 */
void API::Handlers::DashboardHandler::registerTimeRangeCommands()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject emptySchema;
  emptySchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  emptySchema.insert(QStringLiteral("properties"), QJsonObject());

  QJsonObject rangeProps;
  QJsonObject rangeProp;
  rangeProp.insert(QStringLiteral("type"), QStringLiteral("number"));
  rangeProp.insert(QStringLiteral("description"),
                   QStringLiteral("Visible plot time window in seconds (0.001-300)"));
  rangeProps.insert(QStringLiteral("seconds"), rangeProp);

  QJsonObject setRangeSchema;
  setRangeSchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  setRangeSchema.insert(QStringLiteral("properties"), rangeProps);
  setRangeSchema.insert(QStringLiteral("required"), QJsonArray{QStringLiteral("seconds")});

  registry.registerCommand(
    QStringLiteral("dashboard.setTimeRange"),
    QStringLiteral("Set the visible plot time window (params: seconds - 0.001-300)"),
    setRangeSchema,
    &setTimeRange);

  registry.registerCommand(QStringLiteral("dashboard.getTimeRange"),
                           QStringLiteral("Get the current visible plot time window in seconds"),
                           emptySchema,
                           &getTimeRange);

  registry.registerCommand(
    QStringLiteral("project.dashboard.setTimeRange"),
    QStringLiteral("Set the visible plot time window (alias of dashboard.setTimeRange; "
                   "the value is per-project and survives project reload)."),
    setRangeSchema,
    &setTimeRange);

  registry.registerCommand(
    QStringLiteral("project.dashboard.getTimeRange"),
    QStringLiteral("Get the visible plot time window (alias of dashboard.getTimeRange)."),
    emptySchema,
    &getTimeRange);
}

/**
 * @brief Register status, getData, and tailFrames query commands.
 */
void API::Handlers::DashboardHandler::registerQueryCommands()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject emptySchema;
  emptySchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  emptySchema.insert(QStringLiteral("properties"), QJsonObject());

  registry.registerCommand(QStringLiteral("dashboard.getStatus"),
                           QStringLiteral("Get all dashboard configuration settings"),
                           emptySchema,
                           &getStatus);

  registry.registerCommand(QStringLiteral("dashboard.getData"),
                           QStringLiteral("Get dashboard widget counts and latest frame data"),
                           emptySchema,
                           &getData);

  {
    QJsonObject countProp;
    countProp.insert(QStringLiteral("type"), QStringLiteral("integer"));
    countProp.insert(QStringLiteral("description"),
                     QStringLiteral("Max samples per dataset (1-256, default 32)"));
    countProp.insert(QStringLiteral("minimum"), 1);
    countProp.insert(QStringLiteral("maximum"), 256);

    QJsonObject uidsItem;
    uidsItem.insert(QStringLiteral("type"), QStringLiteral("integer"));

    QJsonObject uidsProp;
    uidsProp.insert(QStringLiteral("type"), QStringLiteral("array"));
    uidsProp.insert(QStringLiteral("items"), uidsItem);
    uidsProp.insert(QStringLiteral("description"),
                    QStringLiteral("Optional filter: only datasets whose uniqueId is in "
                                   "this list. Empty/missing = every plot-enabled dataset."));

    QJsonObject props;
    props.insert(QStringLiteral("count"), countProp);
    props.insert(QStringLiteral("uniqueIds"), uidsProp);

    QJsonObject schema;
    schema.insert(QStringLiteral("type"), QStringLiteral("object"));
    schema.insert(QStringLiteral("properties"), props);

    registry.registerCommand(
      QStringLiteral("dashboard.tailFrames"),
      QStringLiteral("Return the last N samples (timestamp + value pairs) for each "
                     "plot-enabled dataset. Use to debug \"the values look wrong\" "
                     "complaints without polling dashboard.getData repeatedly. "
                     "Only returns datasets whose plt flag is on; toggle it via "
                     "project.dataset.setOption first if you need history for a "
                     "different dataset."),
      schema,
      &tailFrames);
  }

  registry.registerCommand(
    QStringLiteral("dashboard.reprocess"),
    QStringLiteral("Re-run every dataset transform from the last received values and republish "
                   "the frames to the dashboard (no export side effects). Dataset transforms "
                   "normally run only when a device frame arrives; call this after "
                   "project.dataTable.setValue writes (control scripts: tableSet()) so the "
                   "dashboard reflects them while the device is silent."),
    emptySchema,
    &reprocess);

  registry.registerCommand(
    QStringLiteral("dashboard.tick"),
    QStringLiteral("Force a dashboard render from the current table/dataset state, synthesizing "
                   "the project frame structure if no device frame has arrived yet. Unlike "
                   "dashboard.reprocess (which no-ops until a real frame exists and never feeds "
                   "exporters), this lets a control script that only writes tables (tableSet()) "
                   "render table-driven virtual datasets from the very first loop(), and it also "
                   "fans the synthesized frame out to the enabled export sinks "
                   "(CSV/MDF4/session/MQTT/API). SDK: dashboardTick()."),
    emptySchema,
    &tick);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Set the operation mode
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
        "Invalid mode: %1. Valid range: 0-2 (0=ProjectFile, 1=ConsoleOnly, 2=QuickPlot)")
        .arg(mode));
  }

  const auto operationMode = static_cast<SerialStudio::OperationMode>(mode);
  AppState::instance().setOperationMode(operationMode);

  QJsonObject result;
  result[QStringLiteral("mode")] = mode;

  const QStringList modeNames = {
    QStringLiteral("ProjectFile"), QStringLiteral("ConsoleOnly"), QStringLiteral("QuickPlot")};
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

  static const QStringList kModeNames = {
    QStringLiteral("ProjectFile"), QStringLiteral("ConsoleOnly"), QStringLiteral("QuickPlot")};
  result[QStringLiteral("modeName")] = (modeIndex >= 0 && modeIndex < kModeNames.size())
                                       ? kModeNames[modeIndex]
                                       : QStringLiteral("Unknown");

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the visualization refresh rate (FPS)
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
 * @brief Set the visible plot time window in seconds
 */
API::CommandResponse API::Handlers::DashboardHandler::setTimeRange(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("seconds"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: seconds"));
  }

  const double seconds = SerialStudio::toDouble(params.value(QStringLiteral("seconds")));

  if (seconds < 0.001 || seconds > 300.0) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid seconds: %1. Valid range: 0.001-300").arg(seconds));
  }

  UI::Dashboard::instance().setPlotTimeRange(seconds);

  QJsonObject result;
  result[QStringLiteral("seconds")] = UI::Dashboard::instance().plotTimeRange();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the current visible plot time window in seconds
 */
API::CommandResponse API::Handlers::DashboardHandler::getTimeRange(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  const double seconds = UI::Dashboard::instance().plotTimeRange();

  QJsonObject result;
  result[QStringLiteral("seconds")] = seconds;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get all dashboard configuration settings
 */
API::CommandResponse API::Handlers::DashboardHandler::getStatus(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto mode        = AppState::instance().operationMode();
  const int modeIndex    = static_cast<int>(mode);
  const int fps          = Misc::TimerEvents::instance().fps();
  const double timeRange = UI::Dashboard::instance().plotTimeRange();
  const int widgetCnt    = UI::Dashboard::instance().totalWidgetCount();
  const int datasetCnt   = static_cast<int>(UI::Dashboard::instance().datasets().size());
  const bool running     = UI::Dashboard::instance().streamAvailable();

  QJsonObject result;
  result[QStringLiteral("operationMode")]      = modeIndex;
  result[QStringLiteral("operationModeLabel")] = API::EnumLabels::operationModeLabel(modeIndex);
  result[QStringLiteral("operationModeSlug")]  = API::EnumLabels::operationModeSlug(modeIndex);

  static const QStringList kModeNames = {
    QStringLiteral("ProjectFile"), QStringLiteral("ConsoleOnly"), QStringLiteral("QuickPlot")};
  result[QStringLiteral("operationModeName")] = (modeIndex >= 0 && modeIndex < kModeNames.size())
                                                ? kModeNames[modeIndex]
                                                : QStringLiteral("Unknown");
  result[QStringLiteral("fps")]               = fps;
  result[QStringLiteral("timeRange")]         = timeRange;
  result[QStringLiteral("widgetCount")]       = widgetCnt;
  result[QStringLiteral("datasetCount")]      = datasetCnt;
  result[QStringLiteral("running")]           = running;

  result[QStringLiteral("_summary")] =
    QStringLiteral("Dashboard mode: %1. %2 widget%3 visible across %4 dataset%5, "
                   "rendering at %6 fps over a %7 s plot window.")
      .arg(API::EnumLabels::operationModeLabel(modeIndex))
      .arg(widgetCnt)
      .arg(widgetCnt == 1 ? QString() : QStringLiteral("s"))
      .arg(datasetCnt)
      .arg(datasetCnt == 1 ? QString() : QStringLiteral("s"))
      .arg(fps)
      .arg(timeRange);

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

/**
 * @brief Returns the last N samples of every plot-enabled dataset.
 */
API::CommandResponse API::Handlers::DashboardHandler::tailFrames(const QString& id,
                                                                 const QJsonObject& params)
{
  int count = 32;
  if (params.contains(QStringLiteral("count")))
    count = qBound(1, params.value(QStringLiteral("count")).toInt(32), 256);

  QSet<int> filterUids;
  bool filterActive = false;
  if (params.contains(QStringLiteral("uniqueIds"))) {
    const auto arr = params.value(QStringLiteral("uniqueIds")).toArray();
    for (const auto& v : arr)
      filterUids.insert(v.toInt());

    filterActive = !filterUids.isEmpty();
  }

  auto& dashboard     = UI::Dashboard::instance();
  const int plotCount = dashboard.widgetCount(SerialStudio::DashboardPlot);
  QJsonArray seriesArr;

  for (int i = 0; i < plotCount; ++i) {
    const auto& ds = dashboard.getDatasetWidget(SerialStudio::DashboardPlot, i);
    if (filterActive && !filterUids.contains(ds.uniqueId))
      continue;

    const auto& series = dashboard.plotData(i);
    if (!series.x || !series.y)
      continue;

    const auto& xq = *series.x;
    const auto& yq = *series.y;
    const auto n   = std::min<std::size_t>(xq.size(), yq.size());
    if (n == 0)
      continue;

    const std::size_t take  = std::min<std::size_t>(static_cast<std::size_t>(count), n);
    const std::size_t start = n - take;

    QJsonArray xs;
    QJsonArray ys;
    for (std::size_t k = start; k < n; ++k) {
      xs.append(xq[k]);
      ys.append(yq[k]);
    }

    QJsonObject row;
    row[Keys::UniqueId]            = ds.uniqueId;
    row[QStringLiteral("title")]   = ds.title;
    row[QStringLiteral("units")]   = ds.units;
    row[QStringLiteral("groupId")] = ds.groupId;
    row[Keys::DatasetId]           = ds.datasetId;
    row[QStringLiteral("count")]   = static_cast<int>(take);
    row[QStringLiteral("x")]       = xs;
    row[QStringLiteral("y")]       = ys;
    seriesArr.append(row);
  }

  QJsonObject result;
  result[QStringLiteral("series")]           = seriesArr;
  result[QStringLiteral("seriesCount")]      = seriesArr.size();
  result[QStringLiteral("samplesPerSeries")] = count;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Re-runs all dataset transforms from the last raw values and republishes the frames
 *        to the dashboard; published:false is a state (no frame structure yet), not an error.
 */
API::CommandResponse API::Handlers::DashboardHandler::reprocess(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  const bool published = DataModel::FrameBuilder::instance().reprocessFrames();

  QJsonObject result;
  result[QStringLiteral("published")] = published;
  if (!published)
    result[QStringLiteral("_summary")] =
      QStringLiteral("Nothing to republish: requires ProjectFile mode and a loaded project.");

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Forces a dashboard render from the current state, seeding the frame structure if no
 *        device frame has arrived; published:false means ProjectFile mode / a project is missing.
 */
API::CommandResponse API::Handlers::DashboardHandler::tick(const QString& id,
                                                           const QJsonObject& params)
{
  Q_UNUSED(params)

  const bool published = DataModel::FrameBuilder::instance().dashboardTick();

  QJsonObject result;
  result[QStringLiteral("published")] = published;
  if (!published)
    result[QStringLiteral("_summary")] =
      QStringLiteral("Nothing to render: requires ProjectFile mode and a loaded project.");

  return CommandResponse::makeSuccess(id, result);
}
