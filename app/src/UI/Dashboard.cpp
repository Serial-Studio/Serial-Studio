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
#  include "Sessions/Player.h"
#endif

#include <QTimer>

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kDefaultPlotPoints  = 1000;
constexpr int kDefaultPlotBuckets = 1024;

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decrements a RepeatNTimes counter and stops the timer when it hits zero.
 */
static void tickRepeatTimer(int index, QMap<int, QTimer*>& timers, QMap<int, int>& counters)
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
/**
 * @brief Routes a 3D-plot dataset value into the matching X/Y/Z component of point.
 */
static inline void readPlot3DAxis(const DataModel::Dataset& dataset, QVector3D& point)
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

/**
 * @brief Routes a numeric GPS dataset value into the lat/lon/alt accumulator.
 */
static inline void readGpsField(const DataModel::Dataset& dataset,
                                double& lat,
                                double& lon,
                                double& alt)
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

/**
 * @brief Applies a non-RepeatNTimes timer mode to an action's QTimer.
 */
static void applyTimerMode(QTimer* timer,
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
  , m_clockEnabled(false)
  , m_clockWidgetId(kInvalidWidgetId)
  , m_stopwatchEnabled(false)
  , m_stopwatchWidgetId(kInvalidWidgetId)
  , m_autoHideToolbar(false)
  , m_persistSettings(true)
  , m_updateRetryInProgress(false)
  , m_plotTimeOriginSet(false)
  , m_plotGroupCount(0)
  , m_plotTimeRange(10.0)
  , m_relativeFrameTimeSec(0)
  , m_plotDisplayTimeSec(0)
  , m_plotGroupStartSec(0)
  , m_plotSamplePeriodSec(0)
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
  connect(&DataModel::FrameBuilder::instance(), &DataModel::FrameBuilder::jsonFileMapChanged, this, [this] {
    m_sourceRawFrames.clear();
    m_datasetReferences.clear();
  }, Qt::QueuedConnection);
  connect(&AppState::instance(), &AppState::operationModeChanged, this, [=, this] {
    const auto mode = AppState::instance().operationMode();
    if (mode == SerialStudio::ProjectFile) {
      const int project_pts = DataModel::ProjectModel::instance().pointCount();
      if (project_pts > 0 && m_points != project_pts) {
        m_points = project_pts;
        Q_EMIT pointsChanged();
      }

      const double project_range = DataModel::ProjectModel::instance().plotTimeRange();
      if (project_range > 0 && !qFuzzyCompare(m_plotTimeRange, project_range)) {
        m_plotTimeRange = project_range;
        Q_EMIT plotTimeRangeChanged();
      }
    } else {
      if (m_points != kDefaultPlotPoints) {
        m_points = kDefaultPlotPoints;
        Q_EMIT pointsChanged();
      }

      const double saved = qMax(0.001, m_settings.value("Dashboard/PlotTimeRange", 10.0).toDouble());
      if (!qFuzzyCompare(m_plotTimeRange, saved)) {
        m_plotTimeRange = saved;
        Q_EMIT plotTimeRangeChanged();
      }
    }

    // Leaving ProjectFile mode invalidates project-derived caches
    if (mode != SerialStudio::ProjectFile)
      resetData(true);
  }, Qt::QueuedConnection);
  // clang-format on

#ifdef BUILD_COMMERCIAL
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
  m_autoHideToolbar        = m_settings.value("Dashboard/AutoHideToolbar", false).toBool();
  m_showActionPanel        = m_settings.value("Dashboard/ShowActionPanel", true).toBool();
  m_terminalEnabled        = m_settings.value("Dashboard/TerminalEnabled", false).toBool();
  m_notificationLogEnabled = m_settings.value("Dashboard/NotificationLogEnabled", false).toBool();
  m_clockEnabled           = m_settings.value("Dashboard/ClockEnabled", false).toBool();
  m_stopwatchEnabled       = m_settings.value("Dashboard/StopwatchEnabled", false).toBool();
  m_plotTimeRange = qMax(0.001, m_settings.value("Dashboard/PlotTimeRange", 10.0).toDouble());
}

/**
 * @brief Retrieves the singleton instance of the Dashboard.
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

  // Clock works without datasets, same promotion as NotificationLog
  if (m_clockEnabled)
    return true;

  // Stopwatch works without datasets too
  if (m_stopwatchEnabled)
    return true;

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
 * @brief Returns the visible plot time window in seconds (newest sample at 0).
 */
double UI::Dashboard::plotTimeRange() const noexcept
{
  return m_plotTimeRange;
}

/**
 * @brief Returns true when a plot dataset should render against time.
 */
bool UI::Dashboard::useTimeXAxis(const DataModel::Dataset& dataset) const
{
  return dataset.xAxisId == DataModel::kXAxisTime;
}

/**
 * @brief Returns true when a multiplot group should render against time.
 */
bool UI::Dashboard::useTimeXAxisGroup(const DataModel::Group& group) const
{
  return !group.datasets.empty() && group.datasets.front().xAxisId == DataModel::kXAxisTime;
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
  static auto& sessPlayer = Sessions::Player::instance();
  const bool sessOpen     = sessPlayer.isOpen();
  return devOpen || csvOpen || mf4Open || sessOpen;
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
 * @brief Returns true if the clock widget should be displayed within the dashboard.
 */
bool UI::Dashboard::clockEnabled() const noexcept
{
  return m_clockEnabled;
}

/**
 * @brief Returns true if the stopwatch widget should be displayed within the dashboard.
 */
bool UI::Dashboard::stopwatchEnabled() const noexcept
{
  return m_stopwatchEnabled;
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
 */
int UI::Dashboard::relativeIndex(const int widgetIndex) const
{
  const auto it = m_widgetMap.constFind(widgetIndex);
  return it != m_widgetMap.cend() ? it->second : -1;
}

/**
 * @brief Formats a numerical value according to its context range.
 */
QString UI::Dashboard::formatValue(double val, double min, double max) const
{
  return FMT_VAL(val, min, max);
}

/**
 * @brief Retrieves the type of widget associated with a given widget index.
 */
SerialStudio::DashboardWidget UI::Dashboard::widgetType(const int widgetIndex) const
{
  const auto it = m_widgetMap.constFind(widgetIndex);
  return it != m_widgetMap.cend() ? it->first : SerialStudio::DashboardNoWidget;
}

/**
 * @brief Counts the number of instances of a specified widget type.
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
 * @brief Returns the runtime index of the action with the given public @p actionId, or -1.
 */
int UI::Dashboard::actionIndexForId(int actionId) const noexcept
{
  for (int i = 0; i < m_actions.count(); ++i)
    if (m_actions.at(i).actionId == actionId)
      return i;

  return -1;
}

/**
 * @brief Retrieves a map of all widgets/windows in the dashboard.
 */
const SerialStudio::WidgetMap& UI::Dashboard::widgetMap() const
{
  return m_widgetMap;
}

/**
 * @brief Resolves a Group.uniqueId to its positional groupId in the live frame.
 */
int UI::Dashboard::groupIdForUniqueId(int uniqueId) const
{
  if (uniqueId < 0)
    return -1;

  for (const auto& group : m_lastFrame.groups)
    if (group.uniqueId == uniqueId)
      return group.groupId;

  return -1;
}

/**
 * @brief Resolves a positional groupId to its Group.uniqueId; returns -1 if absent.
 */
int UI::Dashboard::groupUniqueIdForGroupId(int groupId) const
{
  if (groupId < 0)
    return -1;

  for (const auto& group : m_lastFrame.groups)
    if (group.groupId == groupId)
      return group.uniqueId;

  return -1;
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
 */
const DSP::MultiLineSeries& UI::Dashboard::multiplotData(const int index) const
{
  if (index < 0 || index >= m_multipltValues.size()) [[unlikely]] {
    static const DSP::MultiLineSeries kEmpty{};
    return kEmpty;
  }

  return m_multipltValues[index];
}

/**
 * @brief Returns the time-bucket envelope for a time-axis plot widget.
 */
const DSP::TimeBucketSeries& UI::Dashboard::plotBuckets(const int index) const
{
  const auto it = m_plotBuckets.find(index);
  if (it == m_plotBuckets.end()) [[unlikely]] {
    static const DSP::TimeBucketSeries kEmpty{};
    return kEmpty;
  }

  return it.value();
}

/**
 * @brief Returns the per-curve time-bucket envelopes for a time-axis multiplot widget.
 */
const std::vector<DSP::TimeBucketSeries>& UI::Dashboard::multiplotBuckets(const int index) const
{
  const auto it = m_multiplotBuckets.find(index);
  if (it == m_multiplotBuckets.end()) [[unlikely]] {
    static const std::vector<DSP::TimeBucketSeries> kEmpty{};
    return kEmpty;
  }

  return it.value();
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Returns the 3D trajectory data for a 3D plot widget.
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
 */
bool UI::Dashboard::plotRunning(const int index)
{
  if (m_activePlots.contains(index))
    return m_activePlots[index];

  return false;
}

/**
 * @brief Checks whether an FFT plot is currently active.
 */
bool UI::Dashboard::fftPlotRunning(const int index)
{
  if (m_activeFFTPlots.contains(index))
    return m_activeFFTPlots[index];

  return false;
}

/**
 * @brief Checks whether a multiplot is currently active.
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
  // Restore default point count when not in ProjectFile mode
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile
      && m_points != kDefaultPlotPoints) {
    m_points = kDefaultPlotPoints;
    Q_EMIT pointsChanged();
  }

  // Clear the widget registry (emits widgetDestroyed for each widget)
  WidgetRegistry::instance().clear();

  // Reset terminal widget ID
  m_terminalWidgetId = kInvalidWidgetId;
#ifdef BUILD_COMMERCIAL
  m_notificationLogWidgetId = kInvalidWidgetId;
#endif
  m_clockWidgetId     = kInvalidWidgetId;
  m_stopwatchWidgetId = kInvalidWidgetId;

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
  m_bucketPushes.clear();
  m_yLinePushes.shrink_to_fit();
  m_xLinePushes.shrink_to_fit();
  m_bucketPushes.shrink_to_fit();

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
  m_plotBuckets.clear();
  m_multiplotBuckets.clear();
  m_plotTimeOriginSet = false;

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

  // Clear time-axis envelopes and restart the relative-time origin
  for (auto it = m_plotBuckets.begin(); it != m_plotBuckets.end(); ++it)
    it.value().configure(m_plotTimeRange, kDefaultPlotBuckets);

  for (auto it = m_multiplotBuckets.begin(); it != m_multiplotBuckets.end(); ++it)
    for (auto& curve : it.value())
      curve.configure(m_plotTimeRange, kDefaultPlotBuckets);

  m_plotTimeOriginSet = false;

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
 * @brief Sets the visible plot time window in seconds and notifies time-axis plots.
 */
void UI::Dashboard::setPlotTimeRange(const double seconds)
{
  const double clamped = qMax(0.001, seconds);
  if (qFuzzyCompare(m_plotTimeRange, clamped))
    return;

  m_plotTimeRange = clamped;

  // Global default persists outside ProjectFile; inside a project the value lives in the .ssproj
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    m_settings.setValue("Dashboard/PlotTimeRange", m_plotTimeRange);

  // Re-bin the time-bucket envelopes to the new window width
  configureLineSeries();
  configureMultiLineSeries();

  m_updateRequired = true;
  Q_EMIT plotTimeRangeChanged();
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
      terminal.widget   = "terminal";
      terminal.title    = tr("Console");
      terminal.groupId  = static_cast<int>(m_lastFrame.groups.size());
      terminal.uniqueId = DataModel::runtime_group_unique_id(terminal.groupId);

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
      notif.widget   = "notification-log";
      notif.title    = tr("Notifications");
      notif.groupId  = static_cast<int>(m_lastFrame.groups.size());
      notif.uniqueId = DataModel::runtime_group_unique_id(notif.groupId);

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
// Clock widget
//--------------------------------------------------------------------------------------------------

/**
 * @brief Removes the clock widget from the registry and internal structures.
 */
void UI::Dashboard::removeClockWidget()
{
  auto& registry = WidgetRegistry::instance();

  if (m_clockWidgetId != kInvalidWidgetId) {
    registry.destroyWidget(m_clockWidgetId);
    m_clockWidgetId = kInvalidWidgetId;
  }

  m_widgetGroups.remove(SerialStudio::DashboardClock);

  auto& groups = m_lastFrame.groups;
  groups.erase(std::remove_if(groups.begin(),
                              groups.end(),
                              [](const DataModel::Group& g) { return g.widget == "clock"; }),
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
 * @brief Enables or disables the clock widget.
 */
void UI::Dashboard::setClockEnabled(const bool enabled)
{
  if (m_clockEnabled == enabled)
    return;

  m_clockEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/ClockEnabled", m_clockEnabled);

  // Same gating as NotificationLog: needs a live source frame to register
  if (!m_sourceRawFrames.isEmpty()) {
    auto& registry = WidgetRegistry::instance();
    if (enabled) {
      DataModel::Group clock;
      clock.widget   = "clock";
      clock.title    = tr("Clock");
      clock.groupId  = static_cast<int>(m_lastFrame.groups.size());
      clock.uniqueId = DataModel::runtime_group_unique_id(clock.groupId);

      m_lastFrame.groups.push_back(clock);
      m_widgetGroups[SerialStudio::DashboardClock].append(clock);

      m_clockWidgetId =
        registry.createWidget(SerialStudio::DashboardClock, clock.title, clock.groupId, -1, true);
      m_widgetMap.insert(m_widgetCount++, qMakePair(SerialStudio::DashboardClock, 0));
    } else {
      removeClockWidget();
    }
  }

  Q_EMIT widgetCountChanged();
  Q_EMIT clockEnabledChanged();

  // Re-evaluate Setup -> Dashboard transition in MainWindow
  Q_EMIT updated();
}

//--------------------------------------------------------------------------------------------------
// Stopwatch widget
//--------------------------------------------------------------------------------------------------

/**
 * @brief Removes the stopwatch widget from the registry and internal structures.
 */
void UI::Dashboard::removeStopwatchWidget()
{
  auto& registry = WidgetRegistry::instance();

  if (m_stopwatchWidgetId != kInvalidWidgetId) {
    registry.destroyWidget(m_stopwatchWidgetId);
    m_stopwatchWidgetId = kInvalidWidgetId;
  }

  m_widgetGroups.remove(SerialStudio::DashboardStopwatch);

  auto& groups = m_lastFrame.groups;
  groups.erase(std::remove_if(groups.begin(),
                              groups.end(),
                              [](const DataModel::Group& g) { return g.widget == "stopwatch"; }),
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
 * @brief Enables or disables the stopwatch widget.
 */
void UI::Dashboard::setStopwatchEnabled(const bool enabled)
{
  if (m_stopwatchEnabled == enabled)
    return;

  m_stopwatchEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/StopwatchEnabled", m_stopwatchEnabled);

  // Same gating as Clock: needs a live source frame to register
  if (!m_sourceRawFrames.isEmpty()) {
    auto& registry = WidgetRegistry::instance();
    if (enabled) {
      DataModel::Group stopwatch;
      stopwatch.widget   = "stopwatch";
      stopwatch.title    = tr("Stopwatch");
      stopwatch.groupId  = static_cast<int>(m_lastFrame.groups.size());
      stopwatch.uniqueId = DataModel::runtime_group_unique_id(stopwatch.groupId);

      m_lastFrame.groups.push_back(stopwatch);
      m_widgetGroups[SerialStudio::DashboardStopwatch].append(stopwatch);

      m_stopwatchWidgetId = registry.createWidget(
        SerialStudio::DashboardStopwatch, stopwatch.title, stopwatch.groupId, -1, true);
      m_widgetMap.insert(m_widgetCount++, qMakePair(SerialStudio::DashboardStopwatch, 0));
    } else {
      removeStopwatchWidget();
    }
  }

  Q_EMIT widgetCountChanged();
  Q_EMIT stopwatchEnabledChanged();

  // Re-evaluate Setup -> Dashboard transition in MainWindow
  Q_EMIT updated();
}

//--------------------------------------------------------------------------------------------------
// Action handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Activates a dashboard action by transmitting its associated data and handling timer logic.
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
 */
void UI::Dashboard::setPlotRunning(const int index, const bool enabled)
{
  if (m_activePlots.contains(index))
    m_activePlots[index] = enabled;
}

/**
 * @brief Sets the active state of an FFT plot.
 */
void UI::Dashboard::setFFTPlotRunning(const int index, const bool enabled)
{
  if (m_activeFFTPlots.contains(index))
    m_activeFFTPlots[index] = enabled;
}

/**
 * @brief Sets the active state of a multiplot.
 */
void UI::Dashboard::setMultiplotRunning(const int index, const bool enabled)
{
  if (m_activeMultiplots.contains(index))
    m_activeMultiplots[index] = enabled;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Sets the active state of a waterfall plot.
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

  // Track elapsed seconds since the first frame for time-axis plots
  if (!m_plotTimeOriginSet) [[unlikely]] {
    m_plotTimeOrigin      = frame->timestamp;
    m_plotTimeOriginSet   = true;
    m_plotGroupCount      = 0;
    m_plotGroupStartSec   = 0;
    m_plotDisplayTimeSec  = 0;
    m_plotSamplePeriodSec = 0;
  }
  m_relativeFrameTimeSec =
    std::chrono::duration<double>(frame->timestamp - m_plotTimeOrigin).count();

  // Spread same-timestamp frames by a smoothed per-sample period (display only, raw stamps kept)
  ++m_plotGroupCount;
  if (m_relativeFrameTimeSec > m_plotGroupStartSec) {
    const int n      = (m_plotGroupCount > 1) ? (m_plotGroupCount - 1) : 1;
    const double gap = m_relativeFrameTimeSec - m_plotGroupStartSec;

    // Divide only for rare multi-sample coarse-clock groups; fine-timestamp sources hit n == 1
    // code-verify off
    const double period = (n > 1) ? (gap / n) : gap;
    // code-verify on

    m_plotSamplePeriodSec =
      (m_plotSamplePeriodSec > 0) ? (0.8 * m_plotSamplePeriodSec + 0.2 * period) : period;
    m_plotGroupStartSec = m_relativeFrameTimeSec;
    m_plotGroupCount    = 1;
  }
  m_plotDisplayTimeSec = qMax(m_plotDisplayTimeSec + m_plotSamplePeriodSec, m_relativeFrameTimeSec);

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
        // reconfigure path; not per-frame
        // code-verify off
        combined.groups.push_back(g);
      // code-verify on
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
 */
void UI::Dashboard::processDatasetIntoWidgetMaps(const DataModel::Dataset& datasetIn,
                                                 DataModel::Group& ledPanel)
{
  Q_ASSERT(datasetIn.index >= 0);
  Q_ASSERT(datasetIn.uniqueId >= 0);

  // Widget/FFT display ranges fall back to the plot value range when left unset (min == max)
  DataModel::Dataset dataset = datasetIn;
  if (DSP::almostEqual(dataset.wgtMin, dataset.wgtMax)) {
    dataset.wgtMin = dataset.pltMin;
    dataset.wgtMax = dataset.pltMax;
  }

  if (DSP::almostEqual(dataset.fftMin, dataset.fftMax)) {
    dataset.fftMin = dataset.pltMin;
    dataset.fftMax = dataset.pltMax;
  }

  // Keyed by uniqueId; sibling datasets sharing a frame index stay distinct.
  if (!m_datasets.contains(dataset.uniqueId)) {
    m_datasets.insert(dataset.uniqueId, dataset);
  } else {
    auto prev     = m_datasets.value(dataset.uniqueId);
    double newMin = qMin(prev.pltMin, dataset.pltMin);
    double newMax = qMax(prev.pltMax, dataset.pltMax);

    auto d   = dataset;
    d.pltMin = newMin;
    d.pltMax = newMax;
    m_datasets.insert(dataset.uniqueId, d);
  }

  // hideOnDashboard suppresses dataset-level tiles; painter still sees the dataset
  if (dataset.hideOnDashboard)
    return;

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
    terminal.widget   = "terminal";
    terminal.title    = tr("Console");
    terminal.groupId  = m_lastFrame.groups.size();
    terminal.uniqueId = DataModel::runtime_group_unique_id(terminal.groupId);

    m_lastFrame.groups.push_back(terminal);
  }

#ifdef BUILD_COMMERCIAL
  // Add notification log group
  if (m_notificationLogEnabled) {
    DataModel::Group notif;
    notif.widget   = "notification-log";
    notif.title    = tr("Notifications");
    notif.groupId  = m_lastFrame.groups.size();
    notif.uniqueId = DataModel::runtime_group_unique_id(notif.groupId);

    m_lastFrame.groups.push_back(notif);
  }
#endif

  // Add clock group
  if (m_clockEnabled) {
    DataModel::Group clock;
    clock.widget   = "clock";
    clock.title    = tr("Clock");
    clock.groupId  = m_lastFrame.groups.size();
    clock.uniqueId = DataModel::runtime_group_unique_id(clock.groupId);

    m_lastFrame.groups.push_back(clock);
  }

  // Add stopwatch group
  if (m_stopwatchEnabled) {
    DataModel::Group stopwatch;
    stopwatch.widget   = "stopwatch";
    stopwatch.title    = tr("Stopwatch");
    stopwatch.groupId  = m_lastFrame.groups.size();
    stopwatch.uniqueId = DataModel::runtime_group_unique_id(stopwatch.groupId);

    m_lastFrame.groups.push_back(stopwatch);
  }

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
      auto& bucket = m_widgetGroups[key];
      if (!bucket.isEmpty() && bucket.last().groupId == group.groupId)
        bucket.removeLast();

      if (bucket.isEmpty())
        m_widgetGroups.remove(key);

      auto copy  = group;
      copy.title = tr("%1 (Fallback)").arg(group.title);
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(copy);
      relabelGroupAsMultiplotFallback(group.groupId, copy.title);
    }

#ifdef BUILD_COMMERCIAL
    // Painter (Pro): GPL builds fall back to a data grid so the group still renders
    if (key == SerialStudio::DashboardPainter && !pro) {
      auto& bucket = m_widgetGroups[key];
      if (!bucket.isEmpty() && bucket.last().groupId == group.groupId)
        bucket.removeLast();

      if (bucket.isEmpty())
        m_widgetGroups.remove(key);

      auto copy  = group;
      copy.title = tr("%1 (Fallback)").arg(group.title);
      m_widgetGroups[SerialStudio::DashboardDataGrid].append(copy);
    }
#endif

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

    // Group-level LED panel inherits parent uniqueId so workspace refs resolve.
    if (ledPanel.datasets.size() > 0) {
      ledPanel.widget   = "led-panel";
      ledPanel.groupId  = group.groupId;
      ledPanel.uniqueId = group.uniqueId;
      ledPanel.title    = tr("LED Panel (%1)").arg(group.title);
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
  m_clockWidgetId     = kInvalidWidgetId;
  m_stopwatchWidgetId = kInvalidWidgetId;

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

      // Store clock widget ID for incremental updates
      if (key == SerialStudio::DashboardClock)
        m_clockWidgetId = widgetId;

      // Store stopwatch widget ID for incremental updates
      if (key == SerialStudio::DashboardStopwatch)
        m_stopwatchWidgetId = widgetId;

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

    // Time multiplots fold each curve into its bucket envelope (value + real timestamp)
    auto bIt = m_multiplotBuckets.find(i);
    if (bIt != m_multiplotBuckets.end()) {
      auto& curves = bIt.value();
      for (size_t j = 0; j < group.datasets.size() && j < curves.size(); ++j)
        curves[j].fold(group.datasets[j].numericValue, m_plotDisplayTimeSec);

      continue;
    }

    // Sample/dataset multiplots push into the Y rings
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

  // Time plots fold value + real timestamp into their bucket envelope
  for (const auto& p : m_bucketPushes) {
    for (const auto& c : p.consumers) {
      if (sourceId >= 0 && c.sourceId != sourceId)
        continue;

      if (*c.activeFlag) {
        p.series->fold(*p.value, m_plotDisplayTimeSec);
        break;
      }
    }
  }
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

  // Unfilled so the XY ring grows from empty -- no seeded (0,0) point draws a false first line.
  DSP::AxisData xAxis(points() + 1);
  m_xAxisData.insert(xSource, xAxis);
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
  m_plotBuckets.clear();
  m_pltValues.clear();
  m_pltValues.squeeze();
  m_activePlots.clear();
  m_yLinePushes.clear();
  m_xLinePushes.clear();
  m_bucketPushes.clear();

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

      // Y-axis keyed by uniqueId for sibling separation; unfilled to skip the (0,0) phantom.
      DSP::AxisData yAxis(points() + 1);
      m_yAxisData.insert(d->uniqueId, yAxis);

      // Register X-axis
      registerXAxisIfNeeded(*d);
    }
  }

  // Construct plot values structure
  for (int i = 0; i < widgetCount(SerialStudio::DashboardPlot); ++i) {
    const auto& yDataset = getDatasetWidget(SerialStudio::DashboardPlot, i);

    // Time X-axis: fixed time-bucket envelope; appended series is a placeholder
    if (useTimeXAxis(yDataset)) {
      DSP::TimeBucketSeries buckets;
      buckets.configure(m_plotTimeRange, kDefaultPlotBuckets);
      m_plotBuckets.insert(i, buckets);

      DSP::LineSeries series;
      series.x = &m_pltXAxis;
      series.y = &m_yAxisData[yDataset.uniqueId];
      m_pltValues.append(series);
    }
    // Use custom X-axis source if available (Pro feature)
#ifdef BUILD_COMMERCIAL
    else if (const auto& tk2 = Licensing::CommercialToken::current();
             m_datasets.contains(yDataset.xAxisId) && tk2.isValid() && SS_LICENSE_GUARD()
             && tk2.featureTier() >= Licensing::FeatureTier::Trial) {
#else
    else if (false) {
#endif
      DSP::LineSeries series;
      series.x = &m_xAxisData[yDataset.xAxisId];
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

    // Time plots fold into a bucket envelope instead of the raw Y/X rings
    if (useTimeXAxis(yDataset)) {
      auto bIt = m_plotBuckets.find(i);
      if (bIt != m_plotBuckets.end()) {
        BucketPush bp;
        bp.consumers.push_back(consumer);
        bp.series = &bIt.value();
        bp.value  = &yDataset.numericValue;
        m_bucketPushes.push_back(std::move(bp));
      }

      continue;
    }

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
        : -1;
#else
    const int xAxisId = -1;
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
  m_multiplotBuckets.clear();

  // Reset default X-axis data
  m_multipltXAxis = DSP::AxisData(points() + 1);
  m_multipltXAxis.fillRange(0, 1);

  // Construct multi-plot values structure
  for (int i = 0; i < widgetCount(SerialStudio::DashboardMultiPlot); ++i) {
    const auto& group = getGroupWidget(SerialStudio::DashboardMultiPlot, i);

    // Shared X is a placeholder for time multiplots (the widget reads multiplotBuckets(i))
    DSP::MultiLineSeries series;
    series.x = &m_multipltXAxis;
    for (size_t j = 0; j < group.datasets.size(); ++j)
      series.y.push_back(DSP::AxisData(points() + 1));

    m_multipltValues.append(series);
    m_activeMultiplots.insert(i, true);

    // Time multiplots fold each curve into its own bucket envelope (shared time grid)
    if (useTimeXAxisGroup(group)) {
      std::vector<DSP::TimeBucketSeries> curves(group.datasets.size());
      for (auto& curve : curves)
        curve.configure(m_plotTimeRange, kDefaultPlotBuckets);

      m_multiplotBuckets.insert(i, std::move(curves));
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Action configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Configures dashboard actions and associated timers from the given DataModel frame.
 */
void UI::Dashboard::configureActions(const DataModel::Frame& frame)
{
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

  // Clear all timers
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

  // Execute actions & start timers (if needed)
  for (int i = 0; i < m_actions.count(); ++i) {
    // Action does not have a timer, skip
    const auto& action = m_actions[i];
    if (action.timerMode == DataModel::TimerMode::Off)
      continue;

    // Obtain the interval
    const auto interval = action.timerIntervalMs;
    if (interval <= 0) {
      qWarning() << "Interval for action" << action.title << "must be greater than 0!";
      continue;
    }

    // Configure the timer
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

    // Register the timer
    m_timers.insert(i, timer);
  }

  // Notify UI about the new action set
  Q_EMIT actionStatusChanged();
}
