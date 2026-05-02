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

#include "UI/Dashboard.h"

#include "AppState.h"
#include "CSV/Player.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "MDF4/Player.h"
#include "Misc/IconEngine.h"
#include "Misc/TimerEvents.h"
#include "UI/WidgetRegistry.h"

#ifdef BUILD_COMMERCIAL
#  include "Licensing/CommercialToken.h"
#  include "MQTT/Client.h"
#  include "Sessions/Player.h"
#endif

#include <QTimer>

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kDefaultPlotPoints = 100;

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

namespace {

/** @brief Decrements a RepeatNTimes counter and stops the timer when it hits zero. */
void tickRepeatTimer(int index, QMap<int, QTimer*>& timers, QMap<int, int>& counters)
{
  const auto it = counters.find(index);
  if (it == counters.end())
    return;

  if (--it.value() > 0)
    return;

  const auto timerIt = timers.find(index);
  if (timerIt != timers.end() && timerIt.value())
    timerIt.value()->stop();

  counters.erase(it);
}

#ifdef BUILD_COMMERCIAL
/** @brief Routes a 3D-plot dataset value into the matching X/Y/Z component of point. */
inline void readPlot3DAxis(const DataModel::Dataset& dataset, QVector3D& point)
{
  const QString& id = dataset.widget;
  if (id == "x" || id == "X")
    point.setX(dataset.numericValue);
  else if (id == "y" || id == "Y")
    point.setY(dataset.numericValue);
  else if (id == "z" || id == "Z")
    point.setZ(dataset.numericValue);
}
#endif

/** @brief Routes a numeric GPS dataset value into the lat/lon/alt accumulator. */
inline void readGpsField(const DataModel::Dataset& dataset, double& lat, double& lon, double& alt)
{
  if (!dataset.isNumeric)
    return;

  const QString& id = dataset.widget;
  if (id == "lat")
    lat = dataset.numericValue;
  else if (id == "lon")
    lon = dataset.numericValue;
  else if (id == "alt")
    alt = dataset.numericValue;
}

/** @brief Applies a non-RepeatNTimes timer mode to an action's QTimer. */
void applyTimerMode(QTimer* timer,
                    DataModel::TimerMode mode,
                    bool guiTrigger,
                    const QString& actionTitle)
{
  if (!timer) {
    qWarning() << "Invalid timer pointer for action" << actionTitle;
    return;
  }

  if (mode == DataModel::TimerMode::StartOnTrigger && !timer->isActive())
    timer->start();

  else if (mode == DataModel::TimerMode::ToggleOnTrigger && guiTrigger) {
    if (timer->isActive())
      timer->stop();
    else
      timer->start();
  }
}

}  // namespace

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Dashboard, wires reset signals and loads persisted settings.
 */
UI::Dashboard::Dashboard()
  : m_points(kDefaultPlotPoints)
  , m_widgetCount(0)
  , m_updateRequired(false)
  , m_showActionPanel(true)
  , m_terminalEnabled(false)
  , m_terminalWidgetId(kInvalidWidgetId)
  , m_notificationLogEnabled(false)
  , m_notificationLogWidgetId(kInvalidWidgetId)
  , m_autoHideToolbar(false)
  , m_persistSettings(true)
  , m_updateRetryInProgress(false)
  , m_pltXAxis(kDefaultPlotPoints)
  , m_multipltXAxis(kDefaultPlotPoints)
{
  // Reset dashboard when data sources open/close or project changes
  // clang-format off
  connect(&CSV::Player::instance(), &CSV::Player::openChanged, this, [=, this] { resetData(true); }, Qt::QueuedConnection);
  connect(&MDF4::Player::instance(), &MDF4::Player::openChanged, this, [=, this] { resetData(true); }, Qt::QueuedConnection);
  connect(&IO::ConnectionManager::instance(), &IO::ConnectionManager::connectedChanged, this, [=, this] {
    if (!IO::ConnectionManager::instance().isConnected())
      resetData(true);
  }, Qt::QueuedConnection);
  connect(&AppState::instance(), &AppState::projectFileChanged, this, [=, this] { resetData(); }, Qt::QueuedConnection);
  connect(&AppState::instance(), &AppState::operationModeChanged, this, [=, this] {
    const auto mode = AppState::instance().operationMode();
    if (mode == SerialStudio::ProjectFile) {
      const int project_pts = DataModel::ProjectModel::instance().pointCount();
      if (project_pts > 0 && m_points != project_pts) {
        m_points = project_pts;
        Q_EMIT pointsChanged();
      }
    } else {
      const int saved = qMax(1, m_settings.value("Dashboard/Points", kDefaultPlotPoints).toInt());
      if (m_points != saved) {
        m_points = saved;
        Q_EMIT pointsChanged();
      }
    }

    // Leaving ProjectFile mode invalidates project-derived caches
    if (mode != SerialStudio::ProjectFile)
      resetData(true);
  }, Qt::QueuedConnection);
  // clang-format on

  // Reset dashboard data if MQTT client is subscribed
#ifdef BUILD_COMMERCIAL
  connect(
    &MQTT::Client::instance(),
    &MQTT::Client::connectedChanged,
    this,
    [=, this] {
      const bool subscribed = MQTT::Client::instance().isSubscriber();
      const bool wasSubscribed =
        !MQTT::Client::instance().isConnected() && MQTT::Client::instance().isSubscriber();

      if (subscribed || wasSubscribed)
        resetData(true);
    },
    Qt::QueuedConnection);
  connect(
    &Sessions::Player::instance(),
    &Sessions::Player::openChanged,
    this,
    [=, this] { resetData(true); },
    Qt::QueuedConnection);
#endif

  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::uiTimeout, this, [=, this] {
    if (m_updateRequired) {
      m_updateRequired = false;
      Q_EMIT updated();
    }
  });

  // Update action items when frame format changes
  connect(this, &UI::Dashboard::widgetCountChanged, this, &UI::Dashboard::actionStatusChanged);

  // Load persisted settings
  m_points          = qMax(1, m_settings.value("Dashboard/Points", kDefaultPlotPoints).toInt());
  m_autoHideToolbar = m_settings.value("Dashboard/AutoHideToolbar", false).toBool();
  m_showActionPanel = m_settings.value("Dashboard/ShowActionPanel", true).toBool();
  m_terminalEnabled = m_settings.value("Dashboard/TerminalEnabled", false).toBool();
  m_notificationLogEnabled = m_settings.value("Dashboard/NotificationLogEnabled", false).toBool();
}

/**
 * @brief Retrieves the singleton instance of the Dashboard.
 * @return Reference to the singleton Dashboard instance.
 */
UI::Dashboard& UI::Dashboard::instance()
{
  static Dashboard instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Availability & state queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks if the dashboard is currently available.
 */
bool UI::Dashboard::available() const
{
  if (!streamAvailable())
    return false;

  if (totalWidgetCount() > 0)
    return true;

#ifdef BUILD_COMMERCIAL
  // Promote an empty project to the dashboard when NotificationLog is the only widget
  if (m_notificationLogEnabled)
    return true;
#endif

  return false;
}

/**
 * @brief Returns true if a rectangle with a list of actions should be displayed alongside the
 * dashboard.
 */
bool UI::Dashboard::showActionPanel() const noexcept
{
  return m_showActionPanel;
}

/**
 * @brief Returns true if the toolbar should automatically hide when the dashboard is visible.
 */
bool UI::Dashboard::autoHideToolbar() const noexcept
{
  return m_autoHideToolbar;
}

/**
 * @brief Checks if at least one data source/stream is active.
 */
bool UI::Dashboard::streamAvailable() const
{
  static auto& manager   = IO::ConnectionManager::instance();
  static auto& csvPlayer = CSV::Player::instance();
  static auto& mf4Player = MDF4::Player::instance();

  const bool csvOpen = csvPlayer.isOpen();
  const bool mf4Open = mf4Player.isOpen();
  const bool devOpen = manager.isConnected();

#ifdef BUILD_COMMERCIAL
  static auto& mqtt        = MQTT::Client::instance();
  static auto& sessPlayer  = Sessions::Player::instance();
  const bool mqttConnected = mqtt.isConnected() && mqtt.isSubscriber();
  const bool sessOpen      = sessPlayer.isOpen();
  return devOpen || csvOpen || mqttConnected || mf4Open || sessOpen;
#else
  return devOpen || csvOpen || mf4Open;
#endif
}

/**
 * @brief Returns true if a terminal widget should be displayed within the dashboard.
 */
bool UI::Dashboard::terminalEnabled() const noexcept
{
  return m_terminalEnabled;
}

/**
 * @brief Returns true if the notification log widget should be displayed within the dashboard.
 */
bool UI::Dashboard::notificationLogEnabled() const noexcept
{
  return m_notificationLogEnabled;
}

/**
 * @brief Determines if the point-selector widget should be visible.
 */
bool UI::Dashboard::pointsWidgetVisible() const
{
#ifdef BUILD_COMMERCIAL
  return m_widgetGroups.contains(SerialStudio::DashboardMultiPlot)
      || m_widgetDatasets.contains(SerialStudio::DashboardPlot)
      || m_widgetGroups.contains(SerialStudio::DashboardPlot3D);
#else
  return m_widgetGroups.contains(SerialStudio::DashboardMultiPlot)
      || m_widgetDatasets.contains(SerialStudio::DashboardPlot);
#endif
}

/**
 * @brief Returns true if the frame contains Pro-only features.
 */
bool UI::Dashboard::containsCommercialFeatures() const noexcept
{
  for (const auto& f : m_sourceRawFrames)
    if (f.containsCommercialFeatures)
      return true;

  return false;
}

//--------------------------------------------------------------------------------------------------
// UI configuration queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Gets the current point/sample count setting for the dashboard plots.
 */
int UI::Dashboard::points() const noexcept
{
  return m_points;
}

/**
 * @brief Retrieves the count of actions available within the dashboard.
 */
int UI::Dashboard::actionCount() const
{
  return m_actions.count();
}

/**
 * @brief Gets the total count of widgets currently available on the dashboard.
 */
int UI::Dashboard::totalWidgetCount() const noexcept
{
  return m_widgetCount;
}

//--------------------------------------------------------------------------------------------------
// Data & widget queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks if the current frame is valid for processing.
 */
bool UI::Dashboard::frameValid() const
{
  return m_lastFrame.groups.size() > 0;
}

/**
 * @brief Retrieves the relative index of a widget within its type group.
 * @param widgetIndex The global index of the widget.
 * @return The relative index if found; -1 otherwise.
 */
int UI::Dashboard::relativeIndex(const int widgetIndex) const
{
  const auto it = m_widgetMap.constFind(widgetIndex);
  return it != m_widgetMap.cend() ? it->second : -1;
}

/**
 * @brief Formats a numerical value according to its context range.
 * @param val The value to format.
 * @param min The minimum value of the expected range.
 * @param max The maximum value of the expected range.
 * @return A QString representing the formatted value.
 */
QString UI::Dashboard::formatValue(double val, double min, double max) const
{
  return FMT_VAL(val, min, max);
}

/**
 * @brief Retrieves the type of widget associated with a given widget index.
 * @param widgetIndex The global index of the widget.
 * @return The widget type, or SerialStudio::DashboardNoWidget if not found.
 */
SerialStudio::DashboardWidget UI::Dashboard::widgetType(const int widgetIndex) const
{
  const auto it = m_widgetMap.constFind(widgetIndex);
  return it != m_widgetMap.cend() ? it->first : SerialStudio::DashboardNoWidget;
}

/**
 * @brief Counts the number of instances of a specified widget type.
 * @param widget The type of widget to count.
 * @return The count of instances for the specified widget type.
 */
int UI::Dashboard::widgetCount(const SerialStudio::DashboardWidget widget) const
{
  // Check group widgets first
  if (SerialStudio::isGroupWidget(widget)) {
    auto it = m_widgetGroups.constFind(widget);
    return it != m_widgetGroups.cend() ? it->count() : 0;
  }

  // Fall through to dataset widgets
  if (SerialStudio::isDatasetWidget(widget)) {
    auto it = m_widgetDatasets.constFind(widget);
    return it != m_widgetDatasets.cend() ? it->count() : 0;
  }

  return 0;
}

//--------------------------------------------------------------------------------------------------
// Specialized data access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Retrieves the title of the current frame in the dashboard.
 */
const QString& UI::Dashboard::title() const
{
  return m_lastFrame.title;
}

/**
 * @brief Returns a list of available dashboard actions with their metadata.
 */
QVariantList UI::Dashboard::actions() const
{
  // Populate metadata for each action
  QVariantList actions;
  for (int i = 0; i < m_actions.count(); ++i) {
    const auto& action = m_actions[i];

    QVariantMap m;
    m["id"]      = i;
    m["checked"] = false;
    m["text"]    = action.title;
    m["icon"]    = Misc::IconEngine::resolveActionIconSource(action.icon);
    if (action.timerMode == DataModel::TimerMode::ToggleOnTrigger) {
      if (m_timers.contains(i) && m_timers[i] && m_timers[i]->isActive())
        m["checked"] = true;
    }

    actions.append(m);
  }

  return actions;
}

/**
 * @brief Retrieves a map of all widgets/windows in the dashboard.
 */
const SerialStudio::WidgetMap& UI::Dashboard::widgetMap() const
{
  return m_widgetMap;
}

//--------------------------------------------------------------------------------------------------
// Dataset & group access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Provides access to the map of dataset objects.
 */
const QMap<int, DataModel::Dataset>& UI::Dashboard::datasets() const
{
  return m_datasets;
}

/**
 * @brief Retrieves a group widget by type and index.
 * @param widget The type of dashboard widget.
 * @param index  The index of the widget within its group type.
 * @return Reference to the requested DataModel::Group, or an empty group if not found.
 */
const DataModel::Group& UI::Dashboard::getGroupWidget(const SerialStudio::DashboardWidget widget,
                                                      const int index) const
{
  static const DataModel::Group emptyGroup;
  const auto it = m_widgetGroups.constFind(widget);

  if (it == m_widgetGroups.cend()) [[unlikely]] {
    qWarning() << "getGroupWidget: widget type not found:" << widget;
    return emptyGroup;
  }

  // Validate index bounds
  if (index < 0 || index >= it->size()) [[unlikely]] {
    qWarning() << "getGroupWidget: index out of bounds:" << index << "for widget" << widget;
    return emptyGroup;
  }

  return it->at(index);
}

/**
 * @brief Retrieves a dataset widget by type and index.
 * @param widget The type of dashboard widget.
 * @param index  The index of the dataset within its widget type.
 * @return Reference to the requested DataModel::Dataset, or an empty dataset if not found.
 */
const DataModel::Dataset& UI::Dashboard::getDatasetWidget(
  const SerialStudio::DashboardWidget widget, const int index) const
{
  static const DataModel::Dataset emptyDataset;
  const auto it = m_widgetDatasets.constFind(widget);

  if (it == m_widgetDatasets.cend()) [[unlikely]] {
    qWarning() << "getDatasetWidget: widget type not found:" << widget;
    return emptyDataset;
  }

  // Validate index bounds
  if (index < 0 || index >= it->size()) [[unlikely]] {
    qWarning() << "getDatasetWidget: index out of bounds:" << index << "for widget" << widget;
    return emptyDataset;
  }

  return it->at(index);
}

//--------------------------------------------------------------------------------------------------
// Frame access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Retrieves the last unmodified DataModel frame for the dashboard.
 */
const DataModel::Frame& UI::Dashboard::rawFrame()
{
  return m_lastFrame;
}

/**
 * @brief Retrieves the processed DataModel frame for the dashboard.
 */
const DataModel::Frame& UI::Dashboard::processedFrame()
{
  return m_lastFrame;
}

//--------------------------------------------------------------------------------------------------
// Time-series data access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the FFT plot data currently displayed on the dashboard.
 * @param index The widget index for the FFT plot.
 * @return Reference to the corresponding AxisData buffer.
 */
const DSP::AxisData& UI::Dashboard::fftData(const int index) const
{
  if (index < 0 || index >= m_fftValues.size()) [[unlikely]] {
    static const DSP::AxisData kEmpty;
    return kEmpty;
  }

  return m_fftValues[index];
}

/**
 * @brief Returns the GPS trajectory data currently tracked by the dashboard.
 * @param index The widget index for the GPS display.
 * @return Reference to the corresponding GpsSeries structure.
 */
const DSP::GpsSeries& UI::Dashboard::gpsSeries(const int index) const
{
  if (index < 0 || index >= m_gpsValues.size()) [[unlikely]] {
    static const DSP::GpsSeries kEmpty;
    return kEmpty;
  }

  return m_gpsValues[index];
}

/**
 * @brief Returns the Y-axis values for a linear plot widget.
 * @param index The widget index for the linear plot.
 * @return Reference to the corresponding LineSeries buffer.
 */
const DSP::LineSeries& UI::Dashboard::plotData(const int index) const
{
  if (index < 0 || index >= m_pltValues.size()) [[unlikely]] {
    static const DSP::LineSeries kEmpty{};
    return kEmpty;
  }

  return m_pltValues[index];
}

/**
 * @brief Returns the series data used by a multiplot widget.
 * @param index The widget index for the multiplot.
 * @return Reference to the corresponding MultiLineSeries container.
 */
const DSP::MultiLineSeries& UI::Dashboard::multiplotData(const int index) const
{
  if (index < 0 || index >= m_multipltValues.size()) [[unlikely]] {
    static const DSP::MultiLineSeries kEmpty{};
    return kEmpty;
  }

  return m_multipltValues[index];
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Returns the 3D trajectory data for a 3D plot widget.
 * @param index The widget index for the 3D plot.
 * @return Reference to the corresponding LineSeries3D buffer.
 */
const DSP::LineSeries3D& UI::Dashboard::plotData3D(const int index) const
{
  if (index < 0 || index >= m_plotData3D.size()) [[unlikely]] {
    static const DSP::LineSeries3D kEmpty;
    return kEmpty;
  }

  return m_plotData3D[index];
}

/**
 * @brief Returns the time-domain ring buffer feeding a waterfall widget.
 * @param index The widget index for the waterfall plot.
 * @return Reference to the corresponding AxisData buffer.
 */
const DSP::AxisData& UI::Dashboard::waterfallData(const int index) const
{
  if (index < 0 || index >= m_waterfallValues.size()) [[unlikely]] {
    static const DSP::AxisData kEmpty;
    return kEmpty;
  }

  return m_waterfallValues[index];
}
#endif

//--------------------------------------------------------------------------------------------------
// Plot active status getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks whether a plot is currently active.
 * @param index Plot index to query.
 * @return true if the plot is running.
 */
bool UI::Dashboard::plotRunning(const int index)
{
  if (m_activePlots.contains(index))
    return m_activePlots[index];

  return false;
}

/**
 * @brief Checks whether an FFT plot is currently active.
 * @param index FFT plot index to query.
 * @return true if the FFT plot is running.
 */
bool UI::Dashboard::fftPlotRunning(const int index)
{
  if (m_activeFFTPlots.contains(index))
    return m_activeFFTPlots[index];

  return false;
}

/**
 * @brief Checks whether a multiplot is currently active.
 * @param index Multiplot index to query.
 * @return true if the multiplot is running.
 */
bool UI::Dashboard::multiplotRunning(const int index)
{
  if (m_activeMultiplots.contains(index))
    return m_activeMultiplots[index];

  return false;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Checks whether a waterfall plot is currently active.
 * @param index Waterfall index to query.
 * @return true if the waterfall is running.
 */
bool UI::Dashboard::waterfallRunning(const int index)
{
  if (m_activeWaterfalls.contains(index))
    return m_activeWaterfalls[index];

  return false;
}
#endif

//--------------------------------------------------------------------------------------------------
// UI configuration setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the number of data points for the dashboard plots.
 * @param points The new number of data points (samples).
 */
void UI::Dashboard::setPoints(const int points)
{
  if (m_points != points) {
    m_points = points;

    if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
      m_settings.setValue("Dashboard/Points", m_points);

    configureLineSeries();
    configureMultiLineSeries();
    Q_EMIT pointsChanged();
  }
}

/**
 * @brief Resets all data in the dashboard, including plot values, widget structures, and actions.
 */
void UI::Dashboard::resetData(const bool notify)
{
  // Restore saved point count when not in ProjectFile mode
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile) {
    const int saved = qMax(1, m_settings.value("Dashboard/Points", kDefaultPlotPoints).toInt());
    if (m_points != saved) {
      m_points = saved;
      Q_EMIT pointsChanged();
    }
  }

  // Clear the widget registry (emits widgetDestroyed for each widget)
  WidgetRegistry::instance().clear();

  // Reset terminal widget ID
  m_terminalWidgetId = kInvalidWidgetId;
#ifdef BUILD_COMMERCIAL
  m_notificationLogWidgetId = kInvalidWidgetId;
#endif

  // Clear plotting data
  m_fftValues.clear();
  m_pltValues.clear();
  m_multipltValues.clear();

  // Free memory associated with the containers of the plotting data
  m_fftValues.squeeze();
  m_pltValues.squeeze();
  m_multipltValues.squeeze();

  // Drop pre-resolved hotpath push tables (point into maps cleared below)
  m_yLinePushes.clear();
  m_xLinePushes.clear();
  m_yLinePushes.shrink_to_fit();
  m_xLinePushes.shrink_to_fit();

  // Clear data for 3D plots
#ifdef BUILD_COMMERCIAL
  m_plotData3D.clear();
  m_plotData3D.squeeze();
  m_waterfallValues.clear();
  m_waterfallValues.squeeze();
#endif

  // Clear GPS data
  m_gpsValues.clear();
  m_gpsValues.squeeze();

  // Clear X/Y axis arrays
  m_xAxisData.clear();
  m_yAxisData.clear();

  // Clear widget & action structures
  m_widgetCount = 0;
  m_widgetMap.clear();
  m_widgetGroups.clear();
  m_widgetDatasets.clear();
  m_datasetReferences.clear();

  // Drop the flat dataset cache so callers don't see stale entries
  m_datasets.clear();

  // Clear activity status flags for plot widgets
  m_activePlots.clear();
  m_activeFFTPlots.clear();
  m_activeMultiplots.clear();
#ifdef BUILD_COMMERCIAL
  m_activeWaterfalls.clear();
#endif

  // Reset frame data
  m_lastFrame = DataModel::Frame();
  m_sourceRawFrames.clear();
  m_updateRetryInProgress = false;

  // Configure actions
  if (AppState::instance().operationMode() == SerialStudio::ProjectFile)
    configureActions(DataModel::FrameBuilder::instance().frame());

  // Notify user interface
  if (notify) {
    m_updateRequired = true;

    Q_EMIT updated();
    Q_EMIT dataReset();
    Q_EMIT widgetCountChanged();
    Q_EMIT containsCommercialFeaturesChanged();
  }
}

/**
 * @brief Clears only the time-series plot data without rebuilding the dashboard.
 */
void UI::Dashboard::clearPlotData()
{
  // Clear FFT plot data
  for (auto& fft : m_fftValues)
    fft.clear();

#ifdef BUILD_COMMERCIAL
  // Clear waterfall time-domain history
  for (auto& wf : m_waterfallValues)
    wf.clear();
#endif

  // Clear line plot Y-axis data
  for (auto it = m_yAxisData.begin(); it != m_yAxisData.end(); ++it)
    it.value().clear();

  // Clear custom X-axis data (preserve default X-axis)
  for (auto it = m_xAxisData.begin(); it != m_xAxisData.end(); ++it)
    it.value().clear();

  // Clear multiplot Y-axis data
  for (auto& multiSeries : m_multipltValues)
    for (auto& yAxis : multiSeries.y)
      yAxis.clear();

  // Clear GPS trajectory data
  for (auto& gps : m_gpsValues) {
    gps.latitudes.clear();
    gps.longitudes.clear();
    gps.altitudes.clear();
  }

#ifdef BUILD_COMMERCIAL
  // Clear 3D plot data
  for (auto& plot3d : m_plotData3D)
    plot3d.clear();
#endif
}

/**
 * @brief Enables/disables the action panel.
 */
void UI::Dashboard::setShowActionPanel(const bool enabled)
{
  if (m_showActionPanel != enabled) {
    m_showActionPanel = enabled;
    if (m_persistSettings)
      m_settings.setValue("Dashboard/ShowActionPanel", m_showActionPanel);

    Q_EMIT showActionPanelChanged();
  }
}

/**
 * @brief Enables or disables auto-hiding the toolbar when the dashboard is shown.
 */
void UI::Dashboard::setAutoHideToolbar(const bool enabled)
{
  if (m_autoHideToolbar != enabled) {
    m_autoHideToolbar = enabled;
    m_settings.setValue("Dashboard/AutoHideToolbar", m_autoHideToolbar);
    Q_EMIT autoHideToolbarChanged();
  }
}

/**
 * @brief Removes the terminal widget from the registry and internal structures.
 */
void UI::Dashboard::removeTerminalWidget()
{
  // Remove terminal from registry if it exists
  auto& registry = WidgetRegistry::instance();

  if (m_terminalWidgetId != kInvalidWidgetId) {
    registry.destroyWidget(m_terminalWidgetId);
    m_terminalWidgetId = kInvalidWidgetId;
  }

  // Purge terminal from widget groups and frame
  m_widgetGroups.remove(SerialStudio::DashboardTerminal);

  auto& groups = m_lastFrame.groups;
  groups.erase(std::remove_if(groups.begin(),
                              groups.end(),
                              [](const DataModel::Group& g) { return g.widget == "terminal"; }),
               groups.end());

  // Rebuild contiguous widget map from remaining widgets
  m_widgetMap.clear();
  m_widgetCount = 0;
  for (auto i = m_widgetGroups.begin(); i != m_widgetGroups.end(); ++i) {
    const auto count = widgetCount(i.key());
    for (int j = 0; j < count; ++j)
      m_widgetMap.insert(m_widgetCount++, qMakePair(i.key(), j));
  }
  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i) {
    const auto count = widgetCount(i.key());
    for (int j = 0; j < count; ++j)
      m_widgetMap.insert(m_widgetCount++, qMakePair(i.key(), j));
  }
}

/**
 * @brief Toggles whether dashboard preference changes are written to QSettings.
 */
void UI::Dashboard::setSettingsPersistent(const bool persistent)
{
  m_persistSettings = persistent;
}

/**
 * @brief Enables or disables the terminal widget.
 */
void UI::Dashboard::setTerminalEnabled(const bool enabled)
{
  if (m_terminalEnabled == enabled)
    return;

  m_terminalEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/TerminalEnabled", m_terminalEnabled);

  // Incremental update path when a dashboard is already live
  if (!m_sourceRawFrames.isEmpty() && m_widgetCount > 0) {
    auto& registry = WidgetRegistry::instance();
    if (enabled) {
      // Create terminal group and add to internal structures
      DataModel::Group terminal;
      terminal.widget  = "terminal";
      terminal.title   = tr("Console");
      terminal.groupId = static_cast<int>(m_lastFrame.groups.size());

      m_lastFrame.groups.push_back(terminal);
      m_widgetGroups[SerialStudio::DashboardTerminal].append(terminal);

      // Register in widget registry and widget map
      m_terminalWidgetId = registry.createWidget(
        SerialStudio::DashboardTerminal, terminal.title, terminal.groupId, -1, true);
      m_widgetMap.insert(m_widgetCount++, qMakePair(SerialStudio::DashboardTerminal, 0));
    } else {
      removeTerminalWidget();
    }
  }

  Q_EMIT widgetCountChanged();
  Q_EMIT terminalEnabledChanged();
}

//--------------------------------------------------------------------------------------------------
// Notification log widget (Pro-only)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Removes the notification log widget from the registry and internal structures.
 */
void UI::Dashboard::removeNotificationLogWidget()
{
#ifdef BUILD_COMMERCIAL
  auto& registry = WidgetRegistry::instance();

  if (m_notificationLogWidgetId != kInvalidWidgetId) {
    registry.destroyWidget(m_notificationLogWidgetId);
    m_notificationLogWidgetId = kInvalidWidgetId;
  }

  m_widgetGroups.remove(SerialStudio::DashboardNotificationLog);

  auto& groups = m_lastFrame.groups;
  groups.erase(
    std::remove_if(groups.begin(),
                   groups.end(),
                   [](const DataModel::Group& g) { return g.widget == "notification-log"; }),
    groups.end());

  // Rebuild contiguous widget map from remaining widgets
  m_widgetMap.clear();
  m_widgetCount = 0;
  for (auto i = m_widgetGroups.begin(); i != m_widgetGroups.end(); ++i) {
    const auto count = widgetCount(i.key());
    for (int j = 0; j < count; ++j)
      m_widgetMap.insert(m_widgetCount++, qMakePair(i.key(), j));
  }
  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i) {
    const auto count = widgetCount(i.key());
    for (int j = 0; j < count; ++j)
      m_widgetMap.insert(m_widgetCount++, qMakePair(i.key(), j));
  }
#endif
}

/**
 * @brief Enables or disables the notification log widget.
 */
void UI::Dashboard::setNotificationLogEnabled(const bool enabled)
{
  if (m_notificationLogEnabled == enabled)
    return;

  m_notificationLogEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/NotificationLogEnabled", m_notificationLogEnabled);

#ifdef BUILD_COMMERCIAL
  // Unlike Terminal, NotificationLog can be the only widget -- only a live source frame is required
  if (!m_sourceRawFrames.isEmpty()) {
    auto& registry = WidgetRegistry::instance();
    if (enabled) {
      DataModel::Group notif;
      notif.widget  = "notification-log";
      notif.title   = tr("Notifications");
      notif.groupId = static_cast<int>(m_lastFrame.groups.size());

      m_lastFrame.groups.push_back(notif);
      m_widgetGroups[SerialStudio::DashboardNotificationLog].append(notif);

      m_notificationLogWidgetId = registry.createWidget(
        SerialStudio::DashboardNotificationLog, notif.title, notif.groupId, -1, true);
      m_widgetMap.insert(m_widgetCount++, qMakePair(SerialStudio::DashboardNotificationLog, 0));
    } else {
      removeNotificationLogWidget();
    }
  }
#endif

  Q_EMIT widgetCountChanged();
  Q_EMIT notificationLogEnabledChanged();

  // Re-evaluate Setup -> Dashboard transition in MainWindow
  Q_EMIT updated();
}

//--------------------------------------------------------------------------------------------------
// Action handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Activates a dashboard action by transmitting its associated data and handling timer logic.
 * @param index The index of the action to activate.
 * @param guiTrigger True if triggered by user interaction; affects ToggleOnTrigger behavior.
 */
void UI::Dashboard::activateAction(const int index, const bool guiTrigger)
{
  Q_ASSERT(index >= 0);
  Q_ASSERT(index < m_actions.count());

  // Validate index
  if (index < 0 || index >= m_actions.count()) {
    qWarning() << "Invalid action index:" << index;
    return;
  }

  // Fetch the action configuration
  const auto& action = m_actions[index];

  // Handle RepeatNTimes mode: on GUI trigger, start the repeat sequence
  if (action.timerMode == DataModel::TimerMode::RepeatNTimes && guiTrigger) {
    if (m_timers.contains(index) && m_timers[index]) {
      m_repeatCounters[index] = qMax(1, action.repeatCount);
      m_timers[index]->start();
    }

    // Send first occurrence immediately
    if (!IO::ConnectionManager::instance().paused())
      (void)IO::ConnectionManager::instance().writeData(DataModel::get_tx_bytes(action));

    if (m_repeatCounters.contains(index))
      m_repeatCounters[index]--;

    Q_EMIT actionStatusChanged();
    return;
  }

  // Handle RepeatNTimes mode: timer tick (not GUI trigger)
  if (action.timerMode == DataModel::TimerMode::RepeatNTimes && !guiTrigger) {
    if (!IO::ConnectionManager::instance().paused())
      (void)IO::ConnectionManager::instance().writeData(DataModel::get_tx_bytes(action));

    tickRepeatTimer(index, m_timers, m_repeatCounters);

    Q_EMIT actionStatusChanged();
    return;
  }

  // Handle other timer behaviors
  const auto timerIt = m_timers.find(index);
  if (timerIt != m_timers.end())
    applyTimerMode(timerIt.value(), action.timerMode, guiTrigger, action.title);

  // Send data payload
  if (!IO::ConnectionManager::instance().paused())
    (void)IO::ConnectionManager::instance().writeData(DataModel::get_tx_bytes(action));

  // Notify UI of potential toggle-state change
  Q_EMIT actionStatusChanged();
}

//--------------------------------------------------------------------------------------------------
// Plot active state setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the active state of a plot.
 * @param index Plot index to update.
 * @param enabled true to mark running, false to pause.
 */
void UI::Dashboard::setPlotRunning(const int index, const bool enabled)
{
  if (m_activePlots.contains(index))
    m_activePlots[index] = enabled;
}

/**
 * @brief Sets the active state of an FFT plot.
 * @param index FFT plot index to update.
 * @param enabled true to mark running, false to pause.
 */
void UI::Dashboard::setFFTPlotRunning(const int index, const bool enabled)
{
  if (m_activeFFTPlots.contains(index))
    m_activeFFTPlots[index] = enabled;
}

/**
 * @brief Sets the active state of a multiplot.
 * @param index Multiplot index to update.
 * @param enabled true to mark running, false to pause.
 */
void UI::Dashboard::setMultiplotRunning(const int index, const bool enabled)
{
  if (m_activeMultiplots.contains(index))
    m_activeMultiplots[index] = enabled;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Sets the active state of a waterfall plot.
 * @param index Waterfall index to update.
 * @param enabled true to mark running, false to pause.
 */
void UI::Dashboard::setWaterfallRunning(const int index, const bool enabled)
{
  if (m_activeWaterfalls.contains(index))
    m_activeWaterfalls[index] = enabled;
}
#endif

//--------------------------------------------------------------------------------------------------
// Frame processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Processes an incoming data frame and updates the dashboard accordingly.
 * @param frame The new DataModel data frame to process.
 */
void UI::Dashboard::hotpathRxFrame(const DataModel::TimestampedFramePtr& frame)
{
  Q_ASSERT(frame);
  Q_ASSERT(!frame->data.groups.empty());
  Q_ASSERT(frame->data.sourceId >= 0);

  const auto& payload = frame->data;

  // Validate frame
  if (payload.groups.size() <= 0 || !streamAvailable()) [[unlikely]]
    return;

  const int sid             = payload.sourceId;
  const bool hadProFeatures = containsCommercialFeatures();

  const auto it               = m_sourceRawFrames.find(sid);
  const bool structureChanged = it == m_sourceRawFrames.end()
                             || !DataModel::compare_frames(payload, it.value())
                             || m_datasetReferences.isEmpty();

  if (structureChanged) [[unlikely]] {
    m_sourceRawFrames[sid] = payload;

    // Build a combined frame from all known sources for reconfigureDashboard
    DataModel::Frame combined;
    combined.title   = payload.title;
    combined.actions = payload.actions;
    for (const auto& sf : std::as_const(m_sourceRawFrames)) {
      combined.containsCommercialFeatures |= sf.containsCommercialFeatures;
      for (const auto& g : sf.groups)
        combined.groups.push_back(g);
    }

    reconfigureDashboard(combined);

    if (hadProFeatures != containsCommercialFeatures())
      Q_EMIT containsCommercialFeaturesChanged();
  }

  // Update dashboard data (only this source's datasets)
  updateDashboardData(payload);

  // Schedule a UI refresh on the next timer tick
  m_updateRequired = true;
}

//--------------------------------------------------------------------------------------------------
// Widget map population
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles the case where a dataset UID is not in m_datasetReferences.
 * @param frame Frame that triggered the missing-dataset condition.
 */
void UI::Dashboard::handleMissingDataset(const DataModel::Frame& frame)
{
  Q_ASSERT(!frame.groups.empty());
  Q_ASSERT(frame.sourceId >= 0);

  // Disconnect on repeated failure, or retry once after reconfiguring
  if (m_updateRetryInProgress) {
    qWarning() << "Failed to build dashboard widget model";

    auto& connMgr = IO::ConnectionManager::instance();
    if (connMgr.isConnected()) {
      connMgr.disconnectDevice();
      return;
    }

    if (CSV::Player::instance().isOpen()) {
      CSV::Player::instance().closeFile();
      return;
    }

    if (MDF4::Player::instance().isOpen()) {
      MDF4::Player::instance().closeFile();
      return;
    }

#ifdef BUILD_COMMERCIAL
    if (MQTT::Client::instance().isConnected()) {
      MQTT::Client::instance().closeConnection();
      return;
    }

    if (Sessions::Player::instance().isOpen()) {
      Sessions::Player::instance().closeFile();
      return;
    }
#endif
    return;
  }

  // Regenerate the dashboard model and retry the update once
  reconfigureDashboard(frame);

  m_updateRetryInProgress = true;
  updateDashboardData(frame);
  m_updateRetryInProgress = false;
}

/**
 * @brief Updates dataset values and plot data based on the given frame.
 * @param frame The JSON frame containing new dataset values.
 */
void UI::Dashboard::updateDashboardData(const DataModel::Frame& frame)
{
  Q_ASSERT(!frame.groups.empty());
  Q_ASSERT(!m_datasetReferences.isEmpty());

  // Propagate new values to all dataset references
  for (const auto& group : frame.groups) {
    for (const auto& dataset : group.datasets) {
      const auto uid = dataset.uniqueId;
      const auto it  = m_datasetReferences.find(uid);

      // Cannot find dataset UID; regenerate model and retry (once)
      if (it == m_datasetReferences.end()) [[unlikely]] {
        handleMissingDataset(frame);
        return;
      }

      // Update all datasets for the given UID
      const auto& datasets = it.value();
      for (auto* ptr : datasets) {
        ptr->value        = dataset.value;
        ptr->isNumeric    = dataset.isNumeric;
        ptr->numericValue = dataset.numericValue;
      }
    }
  }

  // Update plots & time-series widgets (only for this source)
  updateDataSeries(frame.sourceId);
}

/**
 * @brief Registers a dataset's index and per-widget-key mappings.
 * @param dataset  Dataset to process.
 * @param ledPanel Group accumulating LED-panel datasets for the current group.
 */
void UI::Dashboard::processDatasetIntoWidgetMaps(const DataModel::Dataset& dataset,
                                                 DataModel::Group& ledPanel)
{
  Q_ASSERT(dataset.index >= 0);
  Q_ASSERT(dataset.uniqueId >= 0);

  // Insert or merge dataset, preserving min/max across frames
  if (!m_datasets.contains(dataset.index)) {
    m_datasets.insert(dataset.index, dataset);
  } else {
    auto prev     = m_datasets.value(dataset.index);
    double newMin = qMin(prev.pltMin, dataset.pltMin);
    double newMax = qMax(prev.pltMax, dataset.pltMax);

    auto d   = dataset;
    d.pltMin = newMin;
    d.pltMax = newMax;
    m_datasets.insert(dataset.index, d);
  }

  // Route dataset into per-widget-type lists (LEDs go to the group panel)
  auto keys = SerialStudio::getDashboardWidgets(dataset);
  for (const auto& widgetKey : std::as_const(keys)) {
    if (widgetKey == SerialStudio::DashboardLED) {
      ledPanel.datasets.push_back(dataset);
      continue;
    }
    if (widgetKey != SerialStudio::DashboardNoWidget)
      m_widgetDatasets[widgetKey].append(dataset);
  }
}

/**
 * @brief Reconfigures the dashboard layout and widgets based on the new frame.
 * @param frame The combined frame (all sources merged) to configure from.
 */
void UI::Dashboard::reconfigureDashboard(const DataModel::Frame& frame)
{
  Q_ASSERT(!frame.groups.empty());
  Q_ASSERT(streamAvailable());

  // Check commercial license status for Pro-only widgets
  const bool pro = SerialStudio::proWidgetsEnabled();

  // Save per-source cache before reset so multi-source merging still works
  auto savedSourceFrames = m_sourceRawFrames;

  // Reset all dashboard state without notifying the UI
  resetData(false);

  // Restore per-source cache that resetData cleared
  m_sourceRawFrames = std::move(savedSourceFrames);

  // Save combined frame structure for terminal/widget tracking
  m_lastFrame = frame;

  // Add terminal group
  if (m_terminalEnabled) {
    DataModel::Group terminal;
    terminal.widget  = "terminal";
    terminal.title   = tr("Console");
    terminal.groupId = m_lastFrame.groups.size();

    m_lastFrame.groups.push_back(terminal);
  }

#ifdef BUILD_COMMERCIAL
  // Add notification log group
  if (m_notificationLogEnabled) {
    DataModel::Group notif;
    notif.widget  = "notification-log";
    notif.title   = tr("Notifications");
    notif.groupId = m_lastFrame.groups.size();

    m_lastFrame.groups.push_back(notif);
  }
#endif

  // Build widget type -> group lists from the frame
  buildWidgetGroups(frame, pro);

  // Register all widgets with the dashboard registry
  registerWidgets();

  // Build dataset reference maps for value propagation
  buildDatasetReferences();

  // Initialize data series & update actions
  updateDataSeries();
  configureActions(frame);

  // Update user interface
  Q_EMIT widgetCountChanged();
}

/**
 * @brief Populates m_widgetGroups and m_widgetDatasets from the current frame.
 * @param frame The source frame (used for context only).
 * @param pro   Whether commercial/Pro features are active.
 */
void UI::Dashboard::buildWidgetGroups(const DataModel::Frame& frame, bool pro)
{
  Q_ASSERT(!m_lastFrame.groups.empty());
  Q_ASSERT(!frame.groups.empty());
  (void)frame;

  for (const auto& group : m_lastFrame.groups) {
    // Append group widgets
    const auto key = SerialStudio::getDashboardWidget(group);
    if (key != SerialStudio::DashboardNoWidget)
      m_widgetGroups[key].append(group);

    // Append fallback 3D plot widget
    if (key == SerialStudio::DashboardPlot3D && !pro) {
      m_widgetGroups.remove(key);
      auto copy  = group;
      copy.title = tr("%1 (Fallback)").arg(group.title);
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(copy);
      relabelGroupAsMultiplotFallback(group.groupId, copy.title);
    }

    // Append multiplot & 3D plot to accelerometer widget
    if (key == SerialStudio::DashboardAccelerometer) {
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(group);
      if (pro)
        m_widgetGroups[SerialStudio::DashboardPlot3D].append(group);
    }

    // Append multiplot to gyro widget
    if (key == SerialStudio::DashboardGyroscope)
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(group);

    // Parse group datasets into widget maps and LED panel
    DataModel::Group ledPanel;
    for (const auto& dataset : group.datasets)
      processDatasetIntoWidgetMaps(dataset, ledPanel);

    // Add group-level LED panel
    if (ledPanel.datasets.size() > 0) {
      ledPanel.widget  = "led-panel";
      ledPanel.groupId = group.groupId;
      ledPanel.title   = tr("LED Panel (%1)").arg(group.title);
      m_widgetGroups[SerialStudio::DashboardLED].append(ledPanel);
    }
  }
}

/**
 * @brief Rewrites the matching group entry in m_lastFrame as a multiplot fallback.
 */
void UI::Dashboard::relabelGroupAsMultiplotFallback(int groupId, const QString& newTitle)
{
  for (size_t i = 0; i < m_lastFrame.groups.size(); ++i) {
    if (m_lastFrame.groups[i].groupId != groupId)
      continue;

    m_lastFrame.groups[i].title  = newTitle;
    m_lastFrame.groups[i].widget = "multiplot";
    return;
  }
}

/**
 * @brief Registers all group and dataset widgets with the WidgetRegistry.
 */
void UI::Dashboard::registerWidgets()
{
  Q_ASSERT(!m_widgetGroups.isEmpty() || !m_widgetDatasets.isEmpty());
  Q_ASSERT(m_widgetCount == 0);

  // Begin batch update on the registry (defers batchUpdateCompleted signal)
  auto& registry = WidgetRegistry::instance();
  registry.beginBatchUpdate();

  // Prepare for fresh widget registration
  m_terminalWidgetId = kInvalidWidgetId;
#ifdef BUILD_COMMERCIAL
  m_notificationLogWidgetId = kInvalidWidgetId;
#endif

  // Register group-level widgets
  for (auto i = m_widgetGroups.begin(); i != m_widgetGroups.end(); ++i) {
    const auto key   = i.key();
    const auto count = widgetCount(key);
    for (int j = 0; j < count; ++j) {
      const auto& group = i.value().at(j);
      auto widgetId     = registry.createWidget(key, group.title, group.groupId, -1, true);

      // Store terminal widget ID for incremental updates
      if (key == SerialStudio::DashboardTerminal)
        m_terminalWidgetId = widgetId;

#ifdef BUILD_COMMERCIAL
      // Store notification log widget ID for incremental updates
      if (key == SerialStudio::DashboardNotificationLog)
        m_notificationLogWidgetId = widgetId;
#endif

      m_widgetMap.insert(m_widgetCount++, qMakePair(key, j));
    }
  }

  // Register dataset-level widgets
  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i) {
    const auto key   = i.key();
    const auto count = widgetCount(key);
    for (int j = 0; j < count; ++j) {
      const auto& dataset = i.value().at(j);
      (void)registry.createWidget(key, dataset.title, dataset.groupId, dataset.index, false);
      m_widgetMap.insert(m_widgetCount++, qMakePair(key, j));
    }
  }

  // End batch update (emits batchUpdateCompleted if changes occurred)
  registry.endBatchUpdate();
}

/**
 * @brief Builds the m_datasetReferences map from all widget and frame sources.
 */
void UI::Dashboard::buildDatasetReferences()
{
  Q_ASSERT(!m_lastFrame.groups.empty());
  Q_ASSERT(!m_widgetGroups.isEmpty() || !m_widgetDatasets.isEmpty());

  // Traverse all group-level datasets
  for (auto& groupList : m_widgetGroups) {
    for (auto& group : groupList)
      for (auto& dataset : group.datasets)
        m_datasetReferences[dataset.uniqueId].append(&dataset);
  }

  // Traverse all widget-level datasets
  for (auto& datasetList : m_widgetDatasets)
    for (auto& dataset : datasetList)
      m_datasetReferences[dataset.uniqueId].append(&dataset);

  // Traverse all datasets
  for (auto& dataset : m_datasets)
    m_datasetReferences[dataset.uniqueId].append(&dataset);

  // For edge cases, register any dataset that has not been added
  for (auto& group : m_lastFrame.groups) {
    for (auto& dataset : group.datasets) {
      auto& list = m_datasetReferences[dataset.uniqueId];
      if (!list.contains(&dataset))
        list.append(&dataset);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Data series configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates time-series data for all dashboard widgets that require historical tracking.
 * @param sourceId Source to update, or -1 for all sources.
 */
void UI::Dashboard::updateDataSeries(int sourceId)
{
  Q_ASSERT(m_widgetCount > 0 || m_widgetMap.isEmpty());
  Q_ASSERT(!m_sourceRawFrames.isEmpty());

  // Cache widget counts
  const int gpsCount   = widgetCount(SerialStudio::DashboardGPS);
  const int fftCount   = widgetCount(SerialStudio::DashboardFFT);
  const int plotCount  = widgetCount(SerialStudio::DashboardPlot);
  const int multiCount = widgetCount(SerialStudio::DashboardMultiPlot);
#ifdef BUILD_COMMERCIAL
  const int plot3DCount    = widgetCount(SerialStudio::DashboardPlot3D);
  const int waterfallCount = widgetCount(SerialStudio::DashboardWaterfall);
#endif

  // Resize data points if needed
  if (m_gpsValues.size() != gpsCount) [[unlikely]]
    configureGpsSeries();
  if (m_fftValues.size() != fftCount) [[unlikely]]
    configureFftSeries();
  if (m_pltValues.size() != plotCount) [[unlikely]]
    configureLineSeries();
  if (m_multipltValues.size() != multiCount) [[unlikely]]
    configureMultiLineSeries();
#ifdef BUILD_COMMERCIAL
  if (m_plotData3D.size() != plot3DCount) [[unlikely]]
    configurePlot3DSeries();
  if (m_waterfallValues.size() != waterfallCount) [[unlikely]]
    configureWaterfallSeries();
#endif

  // Delegate to per-type update helpers
  updateGpsSeries(sourceId);
  updateFftSeries(sourceId);
  updateLineSeries(sourceId);
#ifdef BUILD_COMMERCIAL
  updateWaterfallSeries(sourceId);
#endif

  // Update multi-plots
  for (int i = 0; i < multiCount; ++i) {
    if (!m_activeMultiplots[i])
      continue;

    const auto& group = getGroupWidget(SerialStudio::DashboardMultiPlot, i);
    if (sourceId >= 0 && group.sourceId != sourceId)
      continue;

    auto& multiSeries   = m_multipltValues[i];
    const size_t yCount = multiSeries.y.size();
    for (size_t j = 0; j < group.datasets.size() && j < yCount; ++j)
      multiSeries.y[j].push(group.datasets[j].numericValue);
  }

  // Update 3D plots
#ifdef BUILD_COMMERCIAL
  updatePlot3DSeries(sourceId);
#endif
}

/**
 * @brief Updates FFT data series for all active FFT plot widgets.
 * @param sourceId Source to update, or -1 for all sources.
 */
void UI::Dashboard::updateFftSeries(int sourceId)
{
  const int fftCount = widgetCount(SerialStudio::DashboardFFT);
  Q_ASSERT(m_fftValues.size() == fftCount);
  Q_ASSERT(m_activeFFTPlots.size() == fftCount);

  for (int i = 0; i < fftCount; ++i) {
    if (!m_activeFFTPlots[i])
      continue;

    const auto& dataset = getDatasetWidget(SerialStudio::DashboardFFT, i);
    if (sourceId >= 0 && dataset.sourceId != sourceId)
      continue;

    m_fftValues[i].push(dataset.numericValue);
  }
}

/**
 * @brief Updates GPS trajectory series for all GPS widgets.
 * @param sourceId Source to update, or -1 for all sources.
 */
void UI::Dashboard::updateGpsSeries(int sourceId)
{
  const int gpsCount = widgetCount(SerialStudio::DashboardGPS);
  Q_ASSERT(m_gpsValues.size() == gpsCount);
  Q_ASSERT(m_widgetGroups.contains(SerialStudio::DashboardGPS) || gpsCount == 0);

  for (int i = 0; i < gpsCount; ++i) {
    const auto& group = getGroupWidget(SerialStudio::DashboardGPS, i);
    if (sourceId >= 0 && group.sourceId != sourceId)
      continue;

    auto& series = m_gpsValues[i];

    // Extract lat/lon/alt from the group's datasets
    double lat = std::nan(""), lon = std::nan(""), alt = std::nan("");
    for (const auto& dataset : group.datasets)
      readGpsField(dataset, lat, lon, alt);

    // Append coordinates to the trajectory ring buffers
    series.latitudes.push(lat);
    series.longitudes.push(lon);
    series.altitudes.push(alt);
  }
}

/**
 * @brief Updates 3D trajectory plot series for all 3D plot widgets.
 * @param sourceId Source to update, or -1 for all sources.
 */
void UI::Dashboard::updatePlot3DSeries(int sourceId)
{
#ifdef BUILD_COMMERCIAL
  const int plot3DCount = widgetCount(SerialStudio::DashboardPlot3D);
  Q_ASSERT(m_plotData3D.size() == plot3DCount);
  Q_ASSERT(m_points > 0);

  for (int i = 0; i < plot3DCount; ++i) {
    const auto& group = getGroupWidget(SerialStudio::DashboardPlot3D, i);
    if (sourceId >= 0 && group.sourceId != sourceId)
      continue;

    auto& plotData = m_plotData3D[i];

    // Extract X/Y/Z components
    QVector3D point;
    for (const auto& dataset : group.datasets)
      readPlot3DAxis(dataset, point);

    // Pre-reserve to keep the 10kHz+ hotpath alloc-free
    const size_t maxPoints = static_cast<size_t>(points());
    if (plotData.capacity() < maxPoints) [[unlikely]]
      plotData.reserve(maxPoints);

    if (plotData.size() >= maxPoints) [[likely]]
      plotData.erase(plotData.begin());

    plotData.push_back(point);
  }
#else
  (void)sourceId;
#endif
}

/**
 * @brief Updates linear plot data series for all active plot widgets.
 * @param sourceId Source to update, or -1 for all sources.
 */
void UI::Dashboard::updateLineSeries(int sourceId)
{
  Q_ASSERT(m_pltValues.size() == widgetCount(SerialStudio::DashboardPlot));
  Q_ASSERT(m_activePlots.size() == widgetCount(SerialStudio::DashboardPlot));

  // Hotpath: walk pre-resolved push tables (no allocations, no lookups)
  auto fire = [sourceId](const LinePush& p) {
    for (const auto& c : p.consumers) {
      if (sourceId >= 0 && c.sourceId != sourceId)
        continue;

      if (*c.activeFlag) {
        p.buf->push(*p.value);
        return;
      }
    }
  };

  for (const auto& p : m_yLinePushes)
    fire(p);

  for (const auto& p : m_xLinePushes)
    fire(p);
}

/**
 * @brief Initializes the GPS series structure for all GPS widgets.
 */
void UI::Dashboard::configureGpsSeries()
{
  // Release existing GPS buffers
  m_gpsValues.clear();
  m_gpsValues.squeeze();

  // Allocate and pre-fill series for each GPS widget
  for (int i = 0; i < widgetCount(SerialStudio::DashboardGPS); ++i) {
    DSP::GpsSeries series;
    const auto& group = getGroupWidget(SerialStudio::DashboardGPS, i);
    const QMap<QString, DSP::FixedQueue<double>*> fieldMap = {
      {"lat",  &series.latitudes},
      {"lon", &series.longitudes},
      {"alt",  &series.altitudes}
    };

    for (size_t j = 0; j < group.datasets.size(); ++j) {
      const auto& dataset = group.datasets[j];
      if (fieldMap.contains(dataset.widget)) {
        auto* vector = fieldMap[dataset.widget];
        vector->resize(points() + 1);
        vector->fill(std::nan(""));
      }
    }

    m_gpsValues.append(series);
  }
}

/**
 * @brief Configures the FFT series data structure for the dashboard.
 */
void UI::Dashboard::configureFftSeries()
{
  // Release existing FFT buffers
  m_fftValues.clear();
  m_fftValues.squeeze();
  m_activeFFTPlots.clear();

  // Allocate ring buffers sized to each dataset's FFT sample count
  for (int i = 0; i < widgetCount(SerialStudio::DashboardFFT); ++i) {
    const auto& dataset = getDatasetWidget(SerialStudio::DashboardFFT, i);
    m_fftValues.append(DSP::AxisData(dataset.fftSamples));
    m_activeFFTPlots.insert(i, true);
  }
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Updates time-domain ring buffers feeding all active waterfall widgets.
 * @param sourceId Source to update, or -1 for all sources.
 */
void UI::Dashboard::updateWaterfallSeries(int sourceId)
{
  const int waterfallCount = widgetCount(SerialStudio::DashboardWaterfall);
  Q_ASSERT(m_waterfallValues.size() == waterfallCount);
  Q_ASSERT(m_activeWaterfalls.size() == waterfallCount);

  for (int i = 0; i < waterfallCount; ++i) {
    if (!m_activeWaterfalls[i])
      continue;

    const auto& dataset = getDatasetWidget(SerialStudio::DashboardWaterfall, i);
    if (sourceId >= 0 && dataset.sourceId != sourceId)
      continue;

    m_waterfallValues[i].push(dataset.numericValue);
  }
}

/**
 * @brief Configures the waterfall series data structure for the dashboard.
 */
void UI::Dashboard::configureWaterfallSeries()
{
  // Release existing buffers
  m_waterfallValues.clear();
  m_waterfallValues.squeeze();
  m_activeWaterfalls.clear();

  // Allocate ring buffers sized to each dataset's FFT sample count
  for (int i = 0; i < widgetCount(SerialStudio::DashboardWaterfall); ++i) {
    const auto& dataset = getDatasetWidget(SerialStudio::DashboardWaterfall, i);
    m_waterfallValues.append(DSP::AxisData(dataset.fftSamples));
    m_activeWaterfalls.insert(i, true);
  }
}
#endif

/**
 * @brief Registers an X-axis data buffer for a dataset's custom X source.
 * @param dataset Dataset whose xAxisId specifies the X data source.
 */
void UI::Dashboard::registerXAxisIfNeeded(const DataModel::Dataset& dataset)
{
#ifdef BUILD_COMMERCIAL
  const auto& tk = Licensing::CommercialToken::current();
  if (!tk.isValid() || !SS_LICENSE_GUARD() || tk.featureTier() < Licensing::FeatureTier::Trial)
    return;
#else
  Q_UNUSED(dataset);
  return;
#endif

  const int xSource = dataset.xAxisId;
  if (m_xAxisData.contains(xSource))
    return;

  if (!m_datasets.contains(xSource))
    return;

  DSP::AxisData xAxis(points() + 1);
  m_xAxisData.insert(xSource, xAxis);
  m_xAxisData[xSource].fill(0);
}

/**
 * @brief Configures the line series data structures for all dashboard plots.
 */
void UI::Dashboard::configureLineSeries()
{
  Q_ASSERT(m_points > 0);

  // Release existing plot buffers
  m_xAxisData.clear();
  m_yAxisData.clear();
  m_pltValues.clear();
  m_pltValues.squeeze();
  m_activePlots.clear();
  m_yLinePushes.clear();
  m_xLinePushes.clear();

  // Reset default X-axis data
  m_pltXAxis = DSP::AxisData(points() + 1);
  m_pltXAxis.fillRange(0, 1);

  // Construct X/Y axis data arrays
  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i) {
    const auto& datasets = i.value();

    // Register axis data for each plottable dataset
    for (auto d = datasets.begin(); d != datasets.end(); ++d) {
      if (!d->plt)
        continue;

      // Register Y-axis; keyed by uniqueId so transformed siblings keep separate buffers
      DSP::AxisData yAxis(points() + 1);
      m_yAxisData.insert(d->uniqueId, yAxis);
      m_yAxisData[d->uniqueId].fill(0);

      // Register X-axis
      registerXAxisIfNeeded(*d);
    }
  }

  // Construct plot values structure
  for (int i = 0; i < widgetCount(SerialStudio::DashboardPlot); ++i) {
    const auto& yDataset = getDatasetWidget(SerialStudio::DashboardPlot, i);

    // Use custom X-axis source if available (Pro feature)
#ifdef BUILD_COMMERCIAL
    const auto& tk2 = Licensing::CommercialToken::current();
    if (m_datasets.contains(yDataset.xAxisId) && tk2.isValid() && SS_LICENSE_GUARD()
        && tk2.featureTier() >= Licensing::FeatureTier::Trial) {
#else
    if (false) {
#endif
      const auto& xDataset = m_datasets[yDataset.xAxisId];
      DSP::LineSeries series;
      series.x = &m_xAxisData[xDataset.index];
      series.y = &m_yAxisData[yDataset.uniqueId];
      m_pltValues.append(series);
    }

    // Only use Y-axis data, use samples/points as X-axis
    else {
      DSP::LineSeries series;
      series.x = &m_pltXAxis;
      series.y = &m_yAxisData[yDataset.uniqueId];
      m_pltValues.append(series);
    }

    // Enable real-time updates for the plot
    m_activePlots.insert(i, true);
  }

  // Build the pre-resolved push tables consumed on the hotpath
  QHash<int, std::size_t> yByUid;
  QHash<int, std::size_t> xByXAxisId;
  for (int i = 0; i < widgetCount(SerialStudio::DashboardPlot); ++i) {
    const auto& yDataset = getDatasetWidget(SerialStudio::DashboardPlot, i);
    const LinePush::Consumer consumer{yDataset.sourceId, &m_activePlots[i]};

    // Y push entry -- one per uniqueId
    auto yIt = m_yAxisData.find(yDataset.uniqueId);
    if (yIt != m_yAxisData.end()) {
      auto cacheIt = yByUid.find(yDataset.uniqueId);
      if (cacheIt == yByUid.end()) {
        LinePush push;
        push.consumers.push_back(consumer);
        push.buf   = &yIt.value();
        push.value = &yDataset.numericValue;
        yByUid.insert(yDataset.uniqueId, m_yLinePushes.size());
        m_yLinePushes.push_back(std::move(push));
      } else {
        m_yLinePushes[cacheIt.value()].consumers.push_back(consumer);
      }
    }

    // X push entry -- one per xAxisId
#ifdef BUILD_COMMERCIAL
    const auto& tk = Licensing::CommercialToken::current();
    const int xAxisId =
      (tk.isValid() && SS_LICENSE_GUARD() && tk.featureTier() >= Licensing::FeatureTier::Trial)
        ? yDataset.xAxisId
        : 0;
#else
    const int xAxisId = 0;
#endif

    auto xDsIt = m_datasets.find(xAxisId);
    if (xDsIt == m_datasets.end())
      continue;

    auto& xBuf   = m_xAxisData[xAxisId];
    auto cacheIt = xByXAxisId.find(xAxisId);
    if (cacheIt == xByXAxisId.end()) {
      LinePush push;
      push.consumers.push_back(consumer);
      push.buf   = &xBuf;
      push.value = &xDsIt.value().numericValue;
      xByXAxisId.insert(xAxisId, m_xLinePushes.size());
      m_xLinePushes.push_back(std::move(push));
    } else {
      m_xLinePushes[cacheIt.value()].consumers.push_back(consumer);
    }
  }
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Initializes internal data structures for 3D trajectory plot widgets.
 */
void UI::Dashboard::configurePlot3DSeries()
{
  m_plotData3D.clear();
  m_plotData3D.squeeze();
  m_plotData3D.resize(widgetCount(SerialStudio::DashboardPlot3D));
  for (int i = 0; i < m_plotData3D.count(); ++i) {
    m_plotData3D[i].clear();
    m_plotData3D[i].shrink_to_fit();
  }
}
#endif

/**
 * @brief Configures the multi-line series data structure for the dashboard.
 */
void UI::Dashboard::configureMultiLineSeries()
{
  Q_ASSERT(m_points > 0);

  // Release existing multiplot buffers
  m_multipltValues.clear();
  m_multipltValues.squeeze();
  m_activeMultiplots.clear();

  // Reset default X-axis data
  m_multipltXAxis = DSP::AxisData(points() + 1);
  m_multipltXAxis.fillRange(0, 1);

  // Construct multi-plot values structure
  for (int i = 0; i < widgetCount(SerialStudio::DashboardMultiPlot); ++i) {
    const auto& group = getGroupWidget(SerialStudio::DashboardMultiPlot, i);

    DSP::MultiLineSeries series;
    series.x = &m_multipltXAxis;
    for (size_t j = 0; j < group.datasets.size(); ++j) {
      series.y.push_back(DSP::AxisData(points() + 1));
      series.y.back().fill(0);
    }

    m_multipltValues.append(series);
    m_activeMultiplots.insert(i, true);
  }
}

//--------------------------------------------------------------------------------------------------
// Action configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Configures dashboard actions and associated timers from the given DataModel frame.
 * @param frame The DataModel frame containing the user-defined actions to configure.
 */
void UI::Dashboard::configureActions(const DataModel::Frame& frame)
{
  Q_ASSERT(!frame.groups.empty());

  // Stop if frame is not valid
  if (frame.groups.size() <= 0)
    return;

  // Tear down previous actions and timers
  m_actions.clear();
  m_actions.squeeze();

  // Stop and release all existing timers
  for (auto it = m_timers.begin(); it != m_timers.end(); ++it) {
    if (it.value()) {
      disconnect(it.value());
      it.value()->stop();
      delete it.value();
    }
  }

  m_timers.clear();
  m_repeatCounters.clear();

  // Load actions from the new frame
  for (const auto& action : frame.actions)
    m_actions.append(action);

  // Configure timers
  if (!IO::ConnectionManager::instance().isConnected()) {
    Q_EMIT actionStatusChanged();
    return;
  }

  for (int i = 0; i < m_actions.count(); ++i) {
    const auto& action = m_actions[i];
    if (action.timerMode == DataModel::TimerMode::Off)
      continue;

    const auto interval = action.timerIntervalMs;
    if (interval <= 0) {
      qWarning() << "Interval for action" << action.title << "must be greater than 0!";
      continue;
    }

    auto* timer = new QTimer(this);
    timer->setInterval(interval);
    timer->setTimerType(Qt::PreciseTimer);
    connect(timer, &QTimer::timeout, this, [this, i]() { activateAction(i, false); });

    // Auto-start for RepeatNTimes: init counter and start
    const bool isRepeat = action.timerMode == DataModel::TimerMode::RepeatNTimes;
    if (isRepeat && action.autoExecuteOnConnect) {
      m_repeatCounters[i] = qMax(1, action.repeatCount);
      timer->start();
    }

    // Auto-start for other timer modes
    else if (!isRepeat
             && (action.timerMode == DataModel::TimerMode::AutoStart
                 || action.autoExecuteOnConnect))
      timer->start();

    m_timers.insert(i, timer);
  }

  // Notify UI about the new action set
  Q_EMIT actionStatusChanged();
}
