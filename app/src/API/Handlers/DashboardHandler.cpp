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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/DashboardHandler.h"

#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "API/EnumLabels.h"
#include "API/Handlers/ProjectApiSupport.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
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
  registerWidgetDisplayCommands();
  registerQueryCommands();
}

/**
 * @brief Register operation-mode and refresh-rate (FPS) commands.
 */
void API::Handlers::DashboardHandler::registerModeAndFpsCommands()
{
  static auto& registry = CommandRegistry::instance();

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
  static auto& registry = CommandRegistry::instance();

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
 * @brief Register widget display-title override and freeze-title mode commands.
 */
void API::Handlers::DashboardHandler::registerWidgetDisplayCommands()
{
  static auto& registry = CommandRegistry::instance();

  QJsonObject emptySchema;
  emptySchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  emptySchema.insert(QStringLiteral("properties"), QJsonObject());

  QJsonObject uidProp;
  uidProp.insert(QStringLiteral("type"), QStringLiteral("integer"));
  uidProp.insert(QStringLiteral("description"),
                 QStringLiteral("Stable uniqueId of the dataset or group the widget displays"));

  QJsonObject typeProp;
  typeProp.insert(QStringLiteral("type"), QStringLiteral("integer"));
  typeProp.insert(QStringLiteral("description"),
                  QStringLiteral("SerialStudio::DashboardWidget enum value of the widget"));

  QJsonObject titleProp;
  titleProp.insert(QStringLiteral("type"), QStringLiteral("string"));
  titleProp.insert(QStringLiteral("description"),
                   QStringLiteral("Display title override; empty or absent clears the override"));

  QJsonObject titleProps;
  titleProps.insert(Keys::UniqueId, uidProp);
  titleProps.insert(QStringLiteral("title"), titleProp);
  titleProps.insert(QStringLiteral("widgetType"), typeProp);

  QJsonObject setTitleSchema;
  setTitleSchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  setTitleSchema.insert(QStringLiteral("properties"), titleProps);
  setTitleSchema.insert(QStringLiteral("required"), QJsonArray{QString(Keys::UniqueId)});

  registry.registerCommand(
    QStringLiteral("project.dashboard.setWidgetTitle"),
    QStringLiteral("Set a display-title override for the dataset/group with the given uniqueId. "
                   "Without widgetType the override is entity-level (every widget of that "
                   "dataset/group); with widgetType it targets only that widget kind (e.g. only "
                   "the FFT of a dataset) and wins over the entity-level entry. Display-only: "
                   "widget captions, freeze headers, painted instrument titles and taskbar "
                   "entries show it, while exports, dashboard.getData and project.* responses "
                   "keep the canonical title. Empty/absent title clears the override."),
    setTitleSchema,
    &setWidgetTitle);

  registry.registerCommand(
    QStringLiteral("project.dashboard.getWidgetTitles"),
    QStringLiteral("Get every display-title override (entity-level rows carry only uniqueId; "
                   "widget-level rows also carry widgetType), each annotated with the canonical "
                   "title it shadows (canonical:null when the target no longer exists)."),
    emptySchema,
    &getWidgetTitles);

  QJsonObject modeProp;
  modeProp.insert(QStringLiteral("type"), QStringLiteral("string"));
  modeProp.insert(
    QStringLiteral("enum"),
    QJsonArray{QStringLiteral("bar"), QStringLiteral("painted"), QStringLiteral("hidden")});
  modeProp.insert(QStringLiteral("description"),
                  QStringLiteral("Freeze-title mode: bar (panel header), painted (title on the "
                                 "instrument face; Bar/Gauge/Meter only), hidden (no title "
                                 "while frozen)"));

  QJsonObject modeProps;
  modeProps.insert(QStringLiteral("widgetType"), typeProp);
  modeProps.insert(Keys::UniqueId, uidProp);
  modeProps.insert(QStringLiteral("mode"), modeProp);

  QJsonObject setModeSchema;
  setModeSchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  setModeSchema.insert(QStringLiteral("properties"), modeProps);
  setModeSchema.insert(
    QStringLiteral("required"),
    QJsonArray{QStringLiteral("widgetType"), QString(Keys::UniqueId), QStringLiteral("mode")});

  registry.registerCommand(
    QStringLiteral("project.dashboard.setWidgetFreezeTitle"),
    QStringLiteral("Set the freeze-mode title presentation for one widget (widgetType + "
                   "uniqueId): bar shows the panel header, painted shows the title on the "
                   "instrument face (Bar/Gauge/Meter only, their default), hidden shows no "
                   "title while the dashboard is frozen. Every other widget type defaults to "
                   "bar; writing a widget's default clears its stored entry."),
    setModeSchema,
    &setWidgetFreezeTitle);
}

/**
 * @brief Register status, getData, and tailFrames query commands.
 */
void API::Handlers::DashboardHandler::registerQueryCommands()
{
  static auto& registry = CommandRegistry::instance();

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
    uidsItem.insert(QStringLiteral("type"),
                    QJsonArray{QStringLiteral("integer"), QStringLiteral("string")});

    QJsonObject uidsProp;
    uidsProp.insert(QStringLiteral("type"), QStringLiteral("array"));
    uidsProp.insert(QStringLiteral("items"), uidsItem);
    uidsProp.insert(QStringLiteral("description"),
                    QStringLiteral("Optional filter: only datasets whose uniqueId (integer) or "
                                   "alias (string) is in this list; unresolved aliases are "
                                   "skipped. Empty/missing = every plot-enabled dataset."));

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
                   "render table-driven computed datasets from the very first loop(), and it also "
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
  static auto& appState    = AppState::instance();
  appState.setOperationMode(operationMode);

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

  static auto& appState = AppState::instance();
  const auto mode       = appState.operationMode();
  const int modeIndex   = static_cast<int>(mode);

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

  static auto& timerEvents = Misc::TimerEvents::instance();
  timerEvents.setFPS(fps);

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

  static auto& timerEvents = Misc::TimerEvents::instance();
  const int fps            = timerEvents.fps();

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

  static auto& dashboard = UI::Dashboard::instance();
  dashboard.setPlotTimeRange(seconds);

  QJsonObject result;
  result[QStringLiteral("seconds")] = dashboard.plotTimeRange();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the current visible plot time window in seconds
 */
API::CommandResponse API::Handlers::DashboardHandler::getTimeRange(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& dashboard = UI::Dashboard::instance();
  const double seconds   = dashboard.plotTimeRange();

  QJsonObject result;
  result[QStringLiteral("seconds")] = seconds;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Resolves the canonical dataset/group title for a uniqueId; found reports success.
 */
static QString canonicalTitleForUniqueId(int uniqueId, bool& found)
{
  static auto& projectModel = DataModel::ProjectModel::instance();

  found = false;
  for (const auto& group : projectModel.groups()) {
    if (group.uniqueId == uniqueId) {
      found = true;
      return group.title;
    }

    for (const auto& dataset : group.datasets) {
      if (dataset.uniqueId == uniqueId) {
        found = true;
        return dataset.title;
      }
    }
  }

  return QString();
}

/**
 * @brief Set (or clear) the display-title override for a dataset/group uniqueId
 */
API::CommandResponse API::Handlers::DashboardHandler::setWidgetTitle(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(Keys::UniqueId)) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: uniqueId"));
  }

  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile) {
    return CommandResponse::makeError(
      id,
      ErrorCode::OperationFailed,
      QStringLiteral("Display-title overrides require ProjectFile mode with a loaded project."));
  }

  const int uniqueId = params.value(Keys::UniqueId).toInt(-1);
  bool found         = false;
  const auto canon   = canonicalTitleForUniqueId(uniqueId, found);
  if (!found) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("No dataset or group with uniqueId %1 exists in this project.").arg(uniqueId));
  }

  static auto& projectModel = DataModel::ProjectModel::instance();
  const bool widgetScoped   = params.contains(QStringLiteral("widgetType"));
  const int widgetType      = params.value(QStringLiteral("widgetType")).toInt(-1);
  const auto title          = params.value(QStringLiteral("title")).toString().trimmed();

  QString previous;
  if (widgetScoped) {
    previous = projectModel.widgetDisplayTitle(widgetType, uniqueId);
    projectModel.setWidgetDisplayTitle(widgetType, uniqueId, title);
  } else {
    previous = projectModel.displayTitle(uniqueId);
    projectModel.setDisplayTitle(uniqueId, title);
  }

  QJsonObject result;
  result[Keys::UniqueId]              = uniqueId;
  result[QStringLiteral("title")]     = title;
  result[QStringLiteral("previous")]  = previous;
  result[QStringLiteral("canonical")] = canon;
  result[QStringLiteral("cleared")]   = title.isEmpty();
  result[QStringLiteral("scope")] =
    widgetScoped ? QStringLiteral("widget") : QStringLiteral("entity");
  if (widgetScoped)
    result[QStringLiteral("widgetType")] = widgetType;

  result[QStringLiteral("_summary")] =
    title.isEmpty()
      ? QStringLiteral("Cleared the %1-level display title for uniqueId %2.")
          .arg(widgetScoped ? QStringLiteral("widget") : QStringLiteral("entity"))
          .arg(uniqueId)
      : QStringLiteral("%1 for uniqueId %2 now displays \"%3\" (canonical title \"%4\" is "
                       "unchanged in exports and API data).")
          .arg(widgetScoped ? QStringLiteral("The selected widget kind")
                            : QStringLiteral("Every widget"))
          .arg(uniqueId)
          .arg(title)
          .arg(canon);

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get every display-title override, annotated with the canonical title it shadows
 */
API::CommandResponse API::Handlers::DashboardHandler::getWidgetTitles(const QString& id,
                                                                      const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto overrides      = projectModel.displayTitles();

  QJsonArray titles;
  for (auto it = overrides.constBegin(); it != overrides.constEnd(); ++it) {
    const auto key     = it.key();
    const int sep      = key.indexOf(QLatin1Char(':'));
    const bool scoped  = sep > 0;
    const int uniqueId = scoped ? key.mid(sep + 1).toInt() : key.toInt();
    bool found         = false;
    const auto canon   = canonicalTitleForUniqueId(uniqueId, found);

    QJsonObject row;
    row[Keys::UniqueId]              = uniqueId;
    row[QStringLiteral("title")]     = it.value().toString();
    row[QStringLiteral("scope")]     = scoped ? QStringLiteral("widget") : QStringLiteral("entity");
    row[QStringLiteral("canonical")] = found ? QJsonValue(canon) : QJsonValue();
    if (scoped)
      row[QStringLiteral("widgetType")] = key.left(sep).toInt();

    titles.append(row);
  }

  QJsonObject result;
  result[QStringLiteral("titles")] = titles;
  result[QStringLiteral("count")]  = titles.size();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the freeze-title mode for one widget (widgetType + uniqueId)
 */
API::CommandResponse API::Handlers::DashboardHandler::setWidgetFreezeTitle(
  const QString& id, const QJsonObject& params)
{
  const QStringList required = {
    QStringLiteral("widgetType"), QString(Keys::UniqueId), QStringLiteral("mode")};
  for (const auto& key : required) {
    if (!params.contains(key)) {
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));
    }
  }

  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile) {
    return CommandResponse::makeError(
      id,
      ErrorCode::OperationFailed,
      QStringLiteral("Freeze-title modes require ProjectFile mode with a loaded project."));
  }

  const int widgetType = params.value(QStringLiteral("widgetType")).toInt(-1);
  const bool paints    = SerialStudio::dashboardWidgetPaintsTitle(
    static_cast<SerialStudio::DashboardWidget>(widgetType));

  const auto mode = params.value(QStringLiteral("mode")).toString();
  if (mode != QLatin1String("bar") && mode != QLatin1String("painted")
      && mode != QLatin1String("hidden")) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid mode \"%1\": must be bar, painted or hidden.").arg(mode));
  }

  if (mode == QLatin1String("painted") && !paints) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Mode \"painted\" is only valid for Bar, Gauge and Meter widgets."));
  }

  const int uniqueId = params.value(Keys::UniqueId).toInt(-1);
  bool found         = false;
  (void)canonicalTitleForUniqueId(uniqueId, found);
  if (!found) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("No dataset or group with uniqueId %1 exists in this project.").arg(uniqueId));
  }

  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto previous       = projectModel.freezeTitleMode(widgetType, uniqueId);
  projectModel.setFreezeTitleMode(widgetType, uniqueId, mode);

  QJsonObject result;
  result[QStringLiteral("widgetType")] = widgetType;
  result[Keys::UniqueId]               = uniqueId;
  result[QStringLiteral("mode")]       = mode;
  result[QStringLiteral("previous")]   = previous;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get all dashboard configuration settings
 */
API::CommandResponse API::Handlers::DashboardHandler::getStatus(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& appState    = AppState::instance();
  static auto& timerEvents = Misc::TimerEvents::instance();
  static auto& dashboard   = UI::Dashboard::instance();

  const auto mode        = appState.operationMode();
  const int modeIndex    = static_cast<int>(mode);
  const int fps          = timerEvents.fps();
  const double timeRange = dashboard.plotTimeRange();
  const int widgetCnt    = dashboard.totalWidgetCount();
  const int datasetCnt   = static_cast<int>(dashboard.datasets().size());
  const bool running     = dashboard.streamAvailable();

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

  static auto& dashboard = UI::Dashboard::instance();

  QJsonObject result;
  result[QStringLiteral("widgetCount")]  = dashboard.totalWidgetCount();
  result[QStringLiteral("datasetCount")] = dashboard.datasets().size();
  result[QStringLiteral("frame")]        = DataModel::serialize(dashboard.processedFrame());

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Adds one tailFrames "uniqueIds" filter element -- a numeric uniqueId or a string alias --
 *        to @p out; an unresolved alias is silently skipped, matching the filter's skip semantics.
 */
static void insertTailFrameFilterUid(const QJsonValue& selector, QSet<int>& out)
{
  if (!selector.isString()) {
    out.insert(selector.toInt());
    return;
  }

  QString error;
  const auto match = API::Handlers::resolveDatasetSelector(selector, error);
  if (match.dataset)
    out.insert(match.dataset->uniqueId);
}

/**
 * @brief Returns the last N samples of every plot-enabled dataset. Time-axis plots (the
 *        default X axis) sample from the decimating time ring; plotData()'s Y for them
 *        aliases the shared always-empty null buffer, so reading it alone yields nothing.
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
      insertTailFrameFilterUid(v, filterUids);

    filterActive = !filterUids.isEmpty();
  }

  static auto& dashboard = UI::Dashboard::instance();
  const int plotCount    = dashboard.widgetCount(SerialStudio::DashboardPlot);
  QJsonArray seriesArr;

  for (int i = 0; i < plotCount; ++i) {
    const auto& ds = dashboard.getDatasetWidget(SerialStudio::DashboardPlot, i);
    if (filterActive && !filterUids.contains(ds.uniqueId))
      continue;

    const auto& series  = dashboard.plotData(i);
    const auto& ring    = dashboard.plotTimeRing(i).level0;
    const bool timeAxis = ring.value.size() > 0;
    if (!timeAxis && (!series.x || !series.y))
      continue;

    const auto& xq = timeAxis ? ring.time : *series.x;
    const auto& yq = timeAxis ? ring.value : *series.y;
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
 *        to the dashboard; published:false is a state (no frame structure yet, or no dataset
 *        value changed), not an error.
 */
API::CommandResponse API::Handlers::DashboardHandler::reprocess(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  static auto& dashboard    = UI::Dashboard::instance();
  const bool produced       = frameBuilder.reprocessFrames();
  const bool published      = produced && dashboard.streamAvailable();

  QJsonObject result;
  result[QStringLiteral("published")] = published;
  if (produced && !published)
    result[QStringLiteral("_summary")] = QStringLiteral(
      "Frame produced but not rendered: the dashboard drops frames while no stream is open "
      "(device connection or CSV/MDF4/session player).");
  else if (!published)
    result[QStringLiteral("_summary")] = QStringLiteral(
      "Nothing to republish: requires ProjectFile mode and a loaded project, and at least one "
      "dataset value must have changed since the last publish.");

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Forces a dashboard render from the current state, seeding the frame structure if no
 *        device frame has arrived; published:false means ProjectFile mode / a project is
 *        missing, or no dataset value changed since the last tick.
 */
API::CommandResponse API::Handlers::DashboardHandler::tick(const QString& id,
                                                           const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  static auto& dashboard    = UI::Dashboard::instance();
  const bool produced       = frameBuilder.dashboardTick();
  const bool published      = produced && dashboard.streamAvailable();

  QJsonObject result;
  result[QStringLiteral("published")] = published;
  if (produced && !published)
    result[QStringLiteral("_summary")] = QStringLiteral(
      "Frame produced but not rendered: the dashboard drops frames while no stream is open "
      "(device connection or CSV/MDF4/session player). Export sinks, if enabled, still "
      "received the frame.");
  else if (!published)
    result[QStringLiteral("_summary")] = QStringLiteral(
      "Nothing to render: requires ProjectFile mode and a loaded project, and at least one "
      "dataset value must have changed since the last tick.");

  return CommandResponse::makeSuccess(id, result);
}
