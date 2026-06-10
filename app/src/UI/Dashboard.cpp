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
#include "Benchmark/HotpathBenchmark.h"
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

#include <limits>
#include <QSet>
#include <QTimer>

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kDefaultPlotPoints      = 1000;
constexpr int kDefaultPlotBuckets     = 1024;
constexpr int kMaxTimeRingSamples     = 262144;
constexpr double kAssumedMaxRateHz    = 50000.0;
constexpr double kSmoothMaxPeriodSec  = 0.002;
constexpr double kSmoothMaxForwardSec = 0.050;

// Pre-resolved push fallbacks for GPS / 3D groups missing an axis dataset
static constexpr double kNoGpsFix   = std::numeric_limits<double>::quiet_NaN();
static constexpr bool kNeverNumeric = false;
#ifdef BUILD_COMMERCIAL
static constexpr double kZeroAxisSource = 0.0;
#endif

/**
 * @brief Time-ring capacity for a window: enough for the assumed max rate, capped.
 */
static int timeRingCapacity(const double plotTimeRangeSec)
{
  const double want = plotTimeRangeSec * kAssumedMaxRateHz;
  if (want >= static_cast<double>(kMaxTimeRingSamples))
    return kMaxTimeRingSamples;

  return std::max(kDefaultPlotBuckets, static_cast<int>(want));
}

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
  , m_notificationLogEnabled(false)
  , m_clockEnabled(false)
  , m_stopwatchEnabled(false)
  , m_autoHideToolbar(false)
  , m_persistSettings(true)
  , m_updateRetryInProgress(false)
  , m_layoutValid(false)
  , m_streamAvailable(false)
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
    m_valuePushes.clear();
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

      const double saved
        = qMax(0.001, SerialStudio::toDouble(m_settings.value("Dashboard/PlotTimeRange", 10.0)));
      if (!qFuzzyCompare(m_plotTimeRange, saved)) {
        m_plotTimeRange = saved;
        Q_EMIT plotTimeRangeChanged();
      }
    }

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

  connectStreamAvailableInputs();

  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::uiTimeout, this, [=, this] {
    if (m_updateRequired) {
      m_updateRequired = false;
      Q_EMIT updated();
    }
  });

  connect(this, &UI::Dashboard::widgetCountChanged, this, &UI::Dashboard::actionStatusChanged);

  updateStreamAvailable();

  m_autoHideToolbar        = m_settings.value("Dashboard/AutoHideToolbar", false).toBool();
  m_showActionPanel        = m_settings.value("Dashboard/ShowActionPanel", true).toBool();
  m_terminalEnabled        = m_settings.value("Dashboard/TerminalEnabled", false).toBool();
  m_notificationLogEnabled = m_settings.value("Dashboard/NotificationLogEnabled", false).toBool();
  m_clockEnabled           = m_settings.value("Dashboard/ClockEnabled", false).toBool();
  m_stopwatchEnabled       = m_settings.value("Dashboard/StopwatchEnabled", false).toBool();
  m_plotTimeRange =
    qMax(0.001, SerialStudio::toDouble(m_settings.value("Dashboard/PlotTimeRange", 10.0)));
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
  return streamAvailable() && totalWidgetCount() > 0;
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
  if (Benchmark::HotpathBenchmark::active()) [[unlikely]]
    return true;

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
 * @brief Refreshes the cached stream flag read by hotpathRxFrame; wired to every input that
 *        streamAvailable() derives from (connection, players, benchmark activation).
 */
void UI::Dashboard::updateStreamAvailable()
{
  m_streamAvailable = streamAvailable();
}

/**
 * @brief Wires every streamAvailable() input to the cache refresh. Direct connections keep the
 *        cached flag valid for frames arriving in the same event-loop turn.
 */
void UI::Dashboard::connectStreamAvailableInputs()
{
  connect(&IO::ConnectionManager::instance(),
          &IO::ConnectionManager::connectedChanged,
          this,
          &UI::Dashboard::updateStreamAvailable,
          Qt::DirectConnection);
  connect(&CSV::Player::instance(),
          &CSV::Player::openChanged,
          this,
          &UI::Dashboard::updateStreamAvailable,
          Qt::DirectConnection);
  connect(&MDF4::Player::instance(),
          &MDF4::Player::openChanged,
          this,
          &UI::Dashboard::updateStreamAvailable,
          Qt::DirectConnection);
#ifdef BUILD_COMMERCIAL
  connect(&Sessions::Player::instance(),
          &Sessions::Player::openChanged,
          this,
          &UI::Dashboard::updateStreamAvailable,
          Qt::DirectConnection);
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
  if (SerialStudio::isGroupWidget(widget)) {
    auto it = m_widgetGroups.constFind(widget);
    return it != m_widgetGroups.cend() ? it->count() : 0;
  }

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
 * @brief Returns the decimating time ring for a time-axis plot widget.
 */
const DSP::TimeRing& UI::Dashboard::plotTimeRing(const int index) const
{
  const auto it = m_plotTimeRings.find(index);
  if (it == m_plotTimeRings.end()) [[unlikely]] {
    static const DSP::TimeRing kEmpty{};
    return kEmpty;
  }

  return it.value();
}

/**
 * @brief Returns the per-curve decimating time rings for a time-axis multiplot widget.
 */
const std::vector<DSP::TimeRing>& UI::Dashboard::multiplotTimeRings(const int index) const
{
  const auto it = m_multiplotTimeRings.find(index);
  if (it == m_multiplotTimeRings.end()) [[unlikely]] {
    static const std::vector<DSP::TimeRing> kEmpty{};
    return kEmpty;
  }

  return it.value();
}

/**
 * @brief Returns the sweep/trigger engine for a time-axis plot widget.
 */
const DSP::SweepEngine& UI::Dashboard::plotSweep(const int index) const
{
  const auto it = m_plotSweep.find(index);
  if (it == m_plotSweep.end()) [[unlikely]] {
    static const DSP::SweepEngine kEmpty{};
    return kEmpty;
  }

  return it.value();
}

/**
 * @brief Returns the sweep/trigger engine for a time-axis multiplot widget.
 */
const DSP::SweepEngine& UI::Dashboard::multiplotSweep(const int index) const
{
  const auto it = m_multiplotSweep.find(index);
  if (it == m_multiplotSweep.end()) [[unlikely]] {
    static const DSP::SweepEngine kEmpty{};
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

  Q_ASSERT(index < m_plot3DRings.size());

  const auto& ring = m_plot3DRings[index];
  auto& snapshot   = m_plotData3D[index];

  const std::size_t count = ring.size();
  snapshot.resize(count);
  for (std::size_t k = 0; k < count; ++k)
    snapshot[k] = ring[k];

  return snapshot;
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

    auto savedPlotRings      = snapshotPlotTimeRings();
    auto savedMultiplotRings = snapshotMultiplotTimeRings();

    configureLineSeries();
    configureMultiLineSeries();

    restorePlotTimeRings(savedPlotRings);
    restoreMultiplotTimeRings(savedMultiplotRings);

    Q_EMIT pointsChanged();
  }
}

/**
 * @brief Drops every pre-resolved hotpath push table (their pointers follow the layout).
 */
void UI::Dashboard::clearPushTables()
{
  m_yLinePushes.clear();
  m_xLinePushes.clear();
  m_timePushes.clear();
  m_fftPushes.clear();
  m_gpsPushes.clear();
  m_valuePushes.clear();
  m_multiplotPushes.clear();
  m_yLinePushes.shrink_to_fit();
  m_xLinePushes.shrink_to_fit();
  m_timePushes.shrink_to_fit();
  m_fftPushes.shrink_to_fit();
  m_gpsPushes.shrink_to_fit();
  m_multiplotPushes.shrink_to_fit();
#ifdef BUILD_COMMERCIAL
  m_waterfallPushes.clear();
  m_plot3DPushes.clear();
  m_waterfallPushes.shrink_to_fit();
  m_plot3DPushes.shrink_to_fit();
#endif
}

/**
 * @brief Resets all data in the dashboard, including plot values, widget structures, and actions.
 */
void UI::Dashboard::resetData(const bool notify)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile
      && m_points != kDefaultPlotPoints) {
    m_points = kDefaultPlotPoints;
    Q_EMIT pointsChanged();
  }

  WidgetRegistry::instance().clear();

  m_fftValues.clear();
  m_pltValues.clear();
  m_multipltValues.clear();

  m_fftValues.squeeze();
  m_pltValues.squeeze();
  m_multipltValues.squeeze();

  m_layoutValid = false;

  clearPushTables();

#ifdef BUILD_COMMERCIAL
  m_plotData3D.clear();
  m_plotData3D.squeeze();
  m_plot3DRings.clear();
  m_plot3DRings.squeeze();
  m_waterfallValues.clear();
  m_waterfallValues.squeeze();
#endif

  m_gpsValues.clear();
  m_gpsValues.squeeze();

  m_xAxisData.clear();
  m_yAxisData.clear();
  m_plotTimeRings.clear();
  m_multiplotTimeRings.clear();
  m_plotSweep.clear();
  m_multiplotSweep.clear();
  m_plotTimeOriginSet = false;

  m_widgetCount = 0;
  m_widgetMap.clear();
  m_widgetGroups.clear();
  m_widgetDatasets.clear();
  m_datasetReferences.clear();

  m_datasets.clear();

  m_activePlots.clear();
  m_activeFFTPlots.clear();
  m_activeMultiplots.clear();
#ifdef BUILD_COMMERCIAL
  m_activeWaterfalls.clear();
#endif

  m_lastFrame = DataModel::Frame();
  m_sourceRawFrames.clear();
  m_updateRetryInProgress = false;

  if (AppState::instance().operationMode() == SerialStudio::ProjectFile)
    configureActions(DataModel::FrameBuilder::instance().frame());

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
  for (auto& fft : m_fftValues)
    fft.clear();

#ifdef BUILD_COMMERCIAL
  for (auto& wf : m_waterfallValues)
    wf.clear();
#endif

  for (auto it = m_yAxisData.begin(); it != m_yAxisData.end(); ++it)
    it.value().clear();

  for (auto it = m_xAxisData.begin(); it != m_xAxisData.end(); ++it)
    it.value().clear();

  for (auto it = m_plotTimeRings.begin(); it != m_plotTimeRings.end(); ++it)
    it.value().clear();

  for (auto it = m_multiplotTimeRings.begin(); it != m_multiplotTimeRings.end(); ++it)
    for (auto& ring : it.value())
      ring.clear();

  for (auto it = m_plotSweep.begin(); it != m_plotSweep.end(); ++it)
    it.value().resetState();

  for (auto it = m_multiplotSweep.begin(); it != m_multiplotSweep.end(); ++it)
    it.value().resetState();

  m_plotTimeOriginSet = false;

  for (auto& multiSeries : m_multipltValues)
    for (auto& yAxis : multiSeries.y)
      yAxis.clear();

  for (auto& gps : m_gpsValues) {
    gps.latitudes.clear();
    gps.longitudes.clear();
    gps.altitudes.clear();
  }

#ifdef BUILD_COMMERCIAL
  for (auto& ring : m_plot3DRings)
    ring.clear();

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

  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    m_settings.setValue("Dashboard/PlotTimeRange", m_plotTimeRange);

  auto savedPlotRings            = snapshotPlotTimeRings();
  auto savedMultiplotRings       = snapshotMultiplotTimeRings();
  const auto savedPlotSweep      = m_plotSweep;
  const auto savedMultiplotSweep = m_multiplotSweep;

  configureLineSeries();
  configureMultiLineSeries();

  restorePlotTimeRings(savedPlotRings);
  restoreMultiplotTimeRings(savedMultiplotRings);
  restorePlotSweepConfig(savedPlotSweep);
  restoreMultiplotSweepConfig(savedMultiplotSweep);

  m_updateRequired = true;
  Q_EMIT plotTimeRangeChanged();
}

/**
 * @brief Toggles whether dashboard preference changes are written to QSettings.
 */
void UI::Dashboard::setSettingsPersistent(const bool persistent)
{
  m_persistSettings = persistent;
}

//--------------------------------------------------------------------------------------------------
// Dashboard tools (terminal, notification log, clock, stopwatch)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Shows or hides the terminal tool window. The widget itself is always registered;
 *        the flag only drives external-window visibility, so no rebuild occurs.
 */
void UI::Dashboard::setTerminalEnabled(const bool enabled)
{
  if (m_terminalEnabled == enabled)
    return;

  m_terminalEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/TerminalEnabled", m_terminalEnabled);

  Q_EMIT terminalEnabledChanged();
}

/**
 * @brief Shows or hides the notification log tool window (Pro-only widget).
 */
void UI::Dashboard::setNotificationLogEnabled(const bool enabled)
{
  if (m_notificationLogEnabled == enabled)
    return;

  m_notificationLogEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/NotificationLogEnabled", m_notificationLogEnabled);

  Q_EMIT notificationLogEnabledChanged();
}

/**
 * @brief Shows or hides the clock tool window.
 */
void UI::Dashboard::setClockEnabled(const bool enabled)
{
  if (m_clockEnabled == enabled)
    return;

  m_clockEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/ClockEnabled", m_clockEnabled);

  Q_EMIT clockEnabledChanged();
}

/**
 * @brief Shows or hides the stopwatch tool window.
 */
void UI::Dashboard::setStopwatchEnabled(const bool enabled)
{
  if (m_stopwatchEnabled == enabled)
    return;

  m_stopwatchEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/StopwatchEnabled", m_stopwatchEnabled);

  Q_EMIT stopwatchEnabledChanged();
}

//--------------------------------------------------------------------------------------------------
// Action handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Activates a dashboard action by transmitting its associated data and handling timer
 *        logic. actionStatusChanged makes QML rebuild the whole actions list, so it only fires
 *        when a timer's activity flips; per-tick transmissions emit nothing.
 */
void UI::Dashboard::activateAction(const int index, const bool guiTrigger)
{
  Q_ASSERT(index >= 0);
  Q_ASSERT(index < m_actions.count());

  if (index < 0 || index >= m_actions.count()) {
    qWarning() << "Invalid action index:" << index;
    return;
  }

  const auto& action = m_actions[index];

  if (action.timerMode == DataModel::TimerMode::RepeatNTimes && guiTrigger) {
    if (m_timers.contains(index) && m_timers[index]) {
      m_repeatCounters[index] = qMax(1, action.repeatCount);
      m_timers[index]->start();
    }

    if (!IO::ConnectionManager::instance().paused())
      (void)IO::ConnectionManager::instance().writeData(DataModel::get_tx_bytes(action));

    if (m_repeatCounters.contains(index))
      m_repeatCounters[index]--;

    return;
  }

  if (action.timerMode == DataModel::TimerMode::RepeatNTimes && !guiTrigger) {
    if (!IO::ConnectionManager::instance().paused())
      (void)IO::ConnectionManager::instance().writeData(DataModel::get_tx_bytes(action));

    tickRepeatTimer(index, m_timers, m_repeatCounters);
    return;
  }

  bool timerFlipped  = false;
  const auto timerIt = m_timers.find(index);
  if (timerIt != m_timers.end()) {
    const bool wasActive = timerIt.value() && timerIt.value()->isActive();
    applyTimerMode(timerIt.value(), action.timerMode, guiTrigger, action.title);
    const bool isActive = timerIt.value() && timerIt.value()->isActive();
    timerFlipped        = (wasActive != isActive);
  }

  if (!IO::ConnectionManager::instance().paused())
    (void)IO::ConnectionManager::instance().writeData(DataModel::get_tx_bytes(action));

  if (timerFlipped)
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

/**
 * @brief Configures sweep/trigger mode for a plot; gated to commercial tiers.
 */
void UI::Dashboard::setPlotSweep(const int index,
                                 const bool enabled,
                                 const double level,
                                 const int edge,
                                 const int mode,
                                 const double holdoff,
                                 const double timebase)
{
  auto it = m_plotSweep.find(index);
  if (it == m_plotSweep.end())
    return;

#ifdef BUILD_COMMERCIAL
  const auto& tk = Licensing::CommercialToken::current();
  const bool ok  = enabled && tk.isValid() && SS_LICENSE_GUARD()
               && tk.featureTier() >= Licensing::FeatureTier::Trial;
#else
  const bool ok = false;
#endif

  auto& engine = it.value();
  engine.setTrigger(level, edge, mode, holdoff, 0);
  engine.setTimebase(timebase);
  if (engine.enabled != ok) {
    engine.enabled = ok;
    engine.resetState();
  }
}

/**
 * @brief Configures sweep/trigger mode for a multiplot; gated to commercial tiers.
 */
void UI::Dashboard::setMultiplotSweep(const int index,
                                      const bool enabled,
                                      const double level,
                                      const int edge,
                                      const int mode,
                                      const double holdoff,
                                      const int triggerCurve,
                                      const double timebase)
{
  auto it = m_multiplotSweep.find(index);
  if (it == m_multiplotSweep.end())
    return;

#ifdef BUILD_COMMERCIAL
  const auto& tk = Licensing::CommercialToken::current();
  const bool ok  = enabled && tk.isValid() && SS_LICENSE_GUARD()
               && tk.featureTier() >= Licensing::FeatureTier::Trial;
#else
  const bool ok = false;
#endif

  auto& engine = it.value();
  engine.setTrigger(level, edge, mode, holdoff, triggerCurve);
  engine.setTimebase(timebase);
  if (engine.enabled != ok) {
    engine.enabled = ok;
    engine.resetState();
  }
}

/**
 * @brief Re-arms a single-shot plot sweep capture.
 */
void UI::Dashboard::armPlotSweep(const int index)
{
  auto it = m_plotSweep.find(index);
  if (it != m_plotSweep.end())
    it.value().arm();
}

/**
 * @brief Re-arms a single-shot multiplot sweep capture.
 */
void UI::Dashboard::armMultiplotSweep(const int index)
{
  auto it = m_multiplotSweep.find(index);
  if (it != m_multiplotSweep.end())
    it.value().arm();
}

//--------------------------------------------------------------------------------------------------
// Frame processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Processes an incoming data frame and updates the dashboard. The display clock never runs
 *        backwards; it only smooths small forward jitter for sub-ms-cadence sources, snapping
 *        low-rate sources and large gaps to real arrival time. A matching cached publisher
 *        generation skips the per-frame compare_frames() walk (it changes only on pool rebuild).
 */
void UI::Dashboard::hotpathRxFrame(const DataModel::TimestampedFramePtr& frame)
{
  Q_ASSERT(frame);
  Q_ASSERT(!frame->data.groups.empty());
  Q_ASSERT(frame->data.sourceId >= 0);

  const auto& payload = frame->data;

  if (payload.groups.size() <= 0 || !m_streamAvailable) [[unlikely]]
    return;

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

  ++m_plotGroupCount;
  if (m_relativeFrameTimeSec > m_plotGroupStartSec) {
    const int n      = (m_plotGroupCount > 1) ? (m_plotGroupCount - 1) : 1;
    const double gap = m_relativeFrameTimeSec - m_plotGroupStartSec;

    // code-verify off
    // Divide only for rare multi-sample coarse-clock groups; fine-timestamp sources hit n == 1.
    const double period = (n > 1) ? (gap / n) : gap;
    // code-verify on

    m_plotSamplePeriodSec =
      (m_plotSamplePeriodSec > 0) ? (0.8 * m_plotSamplePeriodSec + 0.2 * period) : period;
    m_plotGroupStartSec = m_relativeFrameTimeSec;
    m_plotGroupCount    = 1;
  }
  const double expected = m_plotDisplayTimeSec + m_plotSamplePeriodSec;

  double displayNext = qMax(expected, m_relativeFrameTimeSec);

  const double forwardError = m_relativeFrameTimeSec - expected;
  if (m_plotSamplePeriodSec > 0 && m_plotSamplePeriodSec < kSmoothMaxPeriodSec && forwardError > 0
      && forwardError < kSmoothMaxForwardSec)
    displayNext = expected;

  m_plotDisplayTimeSec = displayNext;

  const int sid = payload.sourceId;

  const auto genIt = m_sourceStructureGen.constFind(sid);
  const bool genKnown =
    genIt != m_sourceStructureGen.cend() && genIt.value() == frame->structureGeneration;

  const bool structureChanged =
    !genKnown || !m_sourceRawFrames.contains(sid) || m_datasetReferences.isEmpty();

  if (structureChanged) [[unlikely]] {
    const bool hadProFeatures = containsCommercialFeatures();
    m_sourceStructureGen[sid] = frame->structureGeneration;
    m_sourceRawFrames[sid]    = payload;
    DataModel::Frame combined;
    combined.title   = payload.title;
    combined.actions = payload.actions;

    // code-verify off
    for (const auto& sf : std::as_const(m_sourceRawFrames)) {
      combined.containsCommercialFeatures |= sf.containsCommercialFeatures;
      for (const auto& g : sf.groups)
        combined.groups.push_back(g);
    }
    // code-verify on

    reconfigureDashboard(combined);

    if (hadProFeatures != containsCommercialFeatures())
      Q_EMIT containsCommercialFeaturesChanged();
  }

  updateDashboardData(payload);

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

  if (!m_layoutValid) [[unlikely]]
    return;

  const auto pit = m_valuePushes.constFind(frame.sourceId);
  if (pit == m_valuePushes.cend()) [[unlikely]] {
    handleMissingDataset(frame);
    return;
  }

  const auto& table         = pit.value();
  const std::size_t entries = table.size();

  std::size_t i = 0;
  for (const auto& group : frame.groups) {
    for (const auto& dataset : group.datasets) {
      if (i >= entries || table[i].uniqueId != dataset.uniqueId) [[unlikely]] {
        handleMissingDataset(frame);
        return;
      }

      const auto& push = table[i++];
      for (auto* ptr : push.targets) {
        ptr->isNumeric    = dataset.isNumeric;
        ptr->numericValue = dataset.numericValue;
      }

      const auto& string_targets = dataset.isNumeric ? push.stringTargets : push.targets;
      for (auto* ptr : string_targets)
        ptr->value = dataset.value;
    }
  }

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

  DataModel::Dataset dataset = datasetIn;
  if (DSP::almostEqual(dataset.wgtMin, dataset.wgtMax)) {
    dataset.wgtMin = dataset.pltMin;
    dataset.wgtMax = dataset.pltMax;
  }

  if (DSP::almostEqual(dataset.fftMin, dataset.fftMax)) {
    dataset.fftMin = dataset.pltMin;
    dataset.fftMax = dataset.pltMax;
  }

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

  if (dataset.hideOnDashboard)
    return;

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
 * @brief Reconfigures the dashboard layout and widgets based on the new frame. Time rings
 *        and the per-source cache are snapshotted before resetData() and restored after,
 *        so a cosmetic project edit that rebuilds the layout does not erase in-flight plot
 *        history.
 */
void UI::Dashboard::reconfigureDashboard(const DataModel::Frame& frame)
{
  Q_ASSERT(!frame.groups.empty());
  Q_ASSERT(streamAvailable());

  const bool pro = SerialStudio::proWidgetsEnabled();

  auto savedSourceFrames = m_sourceRawFrames;

  auto savedPlotRings      = snapshotPlotTimeRings();
  auto savedMultiplotRings = snapshotMultiplotTimeRings();

  resetData(false);

  m_sourceRawFrames = std::move(savedSourceFrames);

  m_lastFrame = frame;

  DataModel::Group terminal;
  terminal.widget   = "terminal";
  terminal.title    = tr("Console");
  terminal.groupId  = m_lastFrame.groups.size();
  terminal.uniqueId = DataModel::runtime_group_unique_id(terminal.groupId);
  m_lastFrame.groups.push_back(terminal);

#ifdef BUILD_COMMERCIAL
  DataModel::Group notif;
  notif.widget   = "notification-log";
  notif.title    = tr("Notifications");
  notif.groupId  = m_lastFrame.groups.size();
  notif.uniqueId = DataModel::runtime_group_unique_id(notif.groupId);
  m_lastFrame.groups.push_back(notif);
#endif

  DataModel::Group clock;
  clock.widget   = "clock";
  clock.title    = tr("Clock");
  clock.groupId  = m_lastFrame.groups.size();
  clock.uniqueId = DataModel::runtime_group_unique_id(clock.groupId);
  m_lastFrame.groups.push_back(clock);

  DataModel::Group stopwatch;
  stopwatch.widget   = "stopwatch";
  stopwatch.title    = tr("Stopwatch");
  stopwatch.groupId  = m_lastFrame.groups.size();
  stopwatch.uniqueId = DataModel::runtime_group_unique_id(stopwatch.groupId);
  m_lastFrame.groups.push_back(stopwatch);

  buildWidgetGroups(frame, pro);

  registerWidgets();

  buildDatasetReferences();
  buildValuePushes();

  updateDataSeries();
  configureActions(frame);

  restorePlotTimeRings(savedPlotRings);
  restoreMultiplotTimeRings(savedMultiplotRings);

  m_layoutValid = true;

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
    const auto key = SerialStudio::getDashboardWidget(group);
    if (key != SerialStudio::DashboardNoWidget)
      m_widgetGroups[key].append(group);

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

    if (key == SerialStudio::DashboardAccelerometer) {
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(group);
      if (pro)
        m_widgetGroups[SerialStudio::DashboardPlot3D].append(group);
    }

    if (key == SerialStudio::DashboardGyroscope)
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(group);

    DataModel::Group ledPanel;
    for (const auto& dataset : group.datasets)
      processDatasetIntoWidgetMaps(dataset, ledPanel);

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

  auto& registry = WidgetRegistry::instance();
  registry.beginBatchUpdate();

  for (auto i = m_widgetGroups.begin(); i != m_widgetGroups.end(); ++i) {
    const auto key   = i.key();
    const auto count = widgetCount(key);
    for (int j = 0; j < count; ++j) {
      const auto& group = i.value().at(j);
      (void)registry.createWidget(key, group.title, group.groupId, -1, true);
      m_widgetMap.insert(m_widgetCount++, qMakePair(key, j));
    }
  }

  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i) {
    const auto key   = i.key();
    const auto count = widgetCount(key);
    for (int j = 0; j < count; ++j) {
      const auto& dataset = i.value().at(j);
      (void)registry.createWidget(key, dataset.title, dataset.groupId, dataset.index, false);
      m_widgetMap.insert(m_widgetCount++, qMakePair(key, j));
    }
  }

  registry.endBatchUpdate();
}

/**
 * @brief Builds the m_datasetReferences map from all widget and frame sources.
 */
void UI::Dashboard::buildDatasetReferences()
{
  Q_ASSERT(!m_lastFrame.groups.empty());
  Q_ASSERT(!m_widgetGroups.isEmpty() || !m_widgetDatasets.isEmpty());

  for (auto& groupList : m_widgetGroups) {
    for (auto& group : groupList)
      for (auto& dataset : group.datasets)
        m_datasetReferences[dataset.uniqueId].append(&dataset);
  }

  for (auto& datasetList : m_widgetDatasets)
    for (auto& dataset : datasetList)
      m_datasetReferences[dataset.uniqueId].append(&dataset);

  for (auto& dataset : m_datasets)
    m_datasetReferences[dataset.uniqueId].append(&dataset);

  for (auto& group : m_lastFrame.groups) {
    for (auto& dataset : group.datasets) {
      auto& list = m_datasetReferences[dataset.uniqueId];
      if (!list.contains(&dataset))
        list.append(&dataset);
    }
  }
}

/**
 * @brief Rebuilds the dataset reference map after the frame layout has changed.
 *        Any push_back/erase on m_lastFrame.groups shifts elements and dangles the
 *        &dataset pointers stored here, so every such mutation must call this; the
 *        early-out guards buildDatasetReferences(), which asserts on an empty frame.
 */
void UI::Dashboard::rebuildDatasetReferences()
{
  m_datasetReferences.clear();
  m_valuePushes.clear();

  if (m_lastFrame.groups.empty())
    return;

  buildDatasetReferences();
  buildValuePushes();
}

/**
 * @brief Resolves one dataset's propagation targets from m_datasetReferences.
 */
UI::Dashboard::ValuePush UI::Dashboard::makeValuePush(
  const DataModel::Dataset& dataset, const QSet<const DataModel::Dataset*>& stringTargets) const
{
  Q_ASSERT(!m_datasetReferences.isEmpty());

  ValuePush push;
  push.uniqueId = dataset.uniqueId;

  const auto ref_it = m_datasetReferences.constFind(dataset.uniqueId);
  if (ref_it == m_datasetReferences.cend()) {
    push.uniqueId = std::numeric_limits<int>::min();
    return push;
  }

  for (auto* target : ref_it.value()) {
    push.targets.push_back(target);
    if (stringTargets.contains(target))
      push.stringTargets.push_back(target);
  }

  return push;
}

/**
 * @brief Pre-resolves the per-source value-propagation tables from m_datasetReferences.
 */
void UI::Dashboard::buildValuePushes()
{
  Q_ASSERT(!m_datasetReferences.isEmpty());
  Q_ASSERT(!m_lastFrame.groups.empty());

  m_valuePushes.clear();

  QSet<const DataModel::Dataset*> string_targets;
  for (auto& group : m_lastFrame.groups)
    for (auto& dataset : group.datasets)
      string_targets.insert(&dataset);

  const auto grid_it = m_widgetGroups.constFind(SerialStudio::DashboardDataGrid);
  if (grid_it != m_widgetGroups.cend()) {
    for (const auto& group : grid_it.value())
      for (const auto& dataset : group.datasets)
        string_targets.insert(&dataset);
  }

  for (auto it = m_sourceRawFrames.cbegin(); it != m_sourceRawFrames.cend(); ++it) {
    auto& table = m_valuePushes[it.key()];
    for (const auto& group : it.value().groups)
      for (const auto& dataset : group.datasets)
        table.push_back(makeValuePush(dataset, string_targets));
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

  const int gpsCount   = widgetCount(SerialStudio::DashboardGPS);
  const int fftCount   = widgetCount(SerialStudio::DashboardFFT);
  const int plotCount  = widgetCount(SerialStudio::DashboardPlot);
  const int multiCount = widgetCount(SerialStudio::DashboardMultiPlot);
#ifdef BUILD_COMMERCIAL
  const int plot3DCount    = widgetCount(SerialStudio::DashboardPlot3D);
  const int waterfallCount = widgetCount(SerialStudio::DashboardWaterfall);
#endif

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

  updateGpsSeries(sourceId);
  updateFftSeries(sourceId);
  updateLineSeries(sourceId);
#ifdef BUILD_COMMERCIAL
  updateWaterfallSeries(sourceId);
#endif

  auto feedMultiSweep = [this](const MultiPush& p) {
    if (!p.sweep || !p.sweep->enabled || p.timeCurves.empty())
      return;

    const int last  = static_cast<int>(p.timeCurves.size()) - 1;
    const int tc    = qBound(0, p.sweep->triggerCurve, last);
    const double st = p.sweep->advance(m_plotDisplayTimeSec, *p.timeCurves[tc].value);
    if (st < 0)
      return;

    const std::size_t n = std::min(p.sweep->back.size(), p.timeCurves.size());
    for (std::size_t j = 0; j < n; ++j)
      p.sweep->back[j].appendDecimated(st, *p.timeCurves[j].value);
  };

  Q_ASSERT(static_cast<int>(m_multiplotPushes.size()) == multiCount);
  for (const auto& p : m_multiplotPushes) {
    if (!*p.activeFlag)
      continue;

    if (sourceId >= 0 && p.sourceId != sourceId)
      continue;

    for (const auto& tc : p.timeCurves)
      tc.ring->appendDecimated(m_plotDisplayTimeSec, *tc.value);

    feedMultiSweep(p);

    for (const auto& s : p.samples)
      s.first->push(*s.second);
  }

#ifdef BUILD_COMMERCIAL
  updatePlot3DSeries(sourceId);
#endif
}

/**
 * @brief Updates FFT data series for all active FFT plot widgets.
 */
void UI::Dashboard::updateFftSeries(int sourceId)
{
  Q_ASSERT(static_cast<int>(m_fftPushes.size()) == m_fftValues.size());
  Q_ASSERT(m_activeFFTPlots.size() == m_fftValues.size());

  for (const auto& p : m_fftPushes) {
    if (!*p.activeFlag)
      continue;

    if (sourceId >= 0 && p.sourceId != sourceId)
      continue;

    p.buf->push(*p.value);
  }
}

/**
 * @brief Updates GPS trajectory series for all GPS widgets.
 */
void UI::Dashboard::updateGpsSeries(int sourceId)
{
  Q_ASSERT(static_cast<int>(m_gpsPushes.size()) == m_gpsValues.size());
  Q_ASSERT(m_widgetGroups.contains(SerialStudio::DashboardGPS) || m_gpsPushes.empty());

  for (const auto& p : m_gpsPushes) {
    if (sourceId >= 0 && p.sourceId != sourceId)
      continue;

    const double lat = *p.lat.numeric ? *p.lat.value : kNoGpsFix;
    const double lon = *p.lon.numeric ? *p.lon.value : kNoGpsFix;
    const double alt = *p.alt.numeric ? *p.alt.value : kNoGpsFix;

    p.series->latitudes.push(lat);
    p.series->longitudes.push(lon);
    p.series->altitudes.push(alt);
  }
}

/**
 * @brief Updates 3D trajectory plot series for all 3D plot widgets.
 */
void UI::Dashboard::updatePlot3DSeries(int sourceId)
{
#ifdef BUILD_COMMERCIAL
  Q_ASSERT(static_cast<int>(m_plot3DPushes.size()) == m_plot3DRings.size());
  Q_ASSERT(m_points > 0);

  const auto maxPoints = static_cast<std::size_t>(points());
  for (const auto& p : m_plot3DPushes) {
    if (sourceId >= 0 && p.sourceId != sourceId)
      continue;

    if (p.ring->capacity() != maxPoints) [[unlikely]]
      p.ring->resize(maxPoints);

    p.ring->push(
      QVector3D(static_cast<float>(*p.x), static_cast<float>(*p.y), static_cast<float>(*p.z)));
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

  auto feedSweep = [this](const TimePush& p) {
    if (!p.sweep || !p.sweep->enabled || p.sweep->back.empty())
      return;

    const double st = p.sweep->advance(m_plotDisplayTimeSec, *p.value);
    if (st >= 0)
      p.sweep->back[0].appendDecimated(st, *p.value);
  };

  for (const auto& p : m_timePushes) {
    for (const auto& c : p.consumers) {
      if (sourceId >= 0 && c.sourceId != sourceId)
        continue;

      if (*c.activeFlag) {
        p.ring->appendDecimated(m_plotDisplayTimeSec, *p.value);
        feedSweep(p);
        break;
      }
    }
  }
}

/**
 * @brief Initializes the GPS series structure for all GPS widgets.
 *        Two passes are deliberate: the push table stores raw pointers into m_gpsValues,
 *        and those addresses are stable only after that vector stops growing, so all
 *        series are allocated first and the pushes resolved in a second pass.
 */
void UI::Dashboard::configureGpsSeries()
{
  m_gpsValues.clear();
  m_gpsValues.squeeze();
  m_gpsPushes.clear();
  m_gpsPushes.shrink_to_fit();

  const int gpsCount = widgetCount(SerialStudio::DashboardGPS);
  for (int i = 0; i < gpsCount; ++i) {
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

  m_gpsPushes.reserve(static_cast<std::size_t>(gpsCount));
  for (int i = 0; i < gpsCount; ++i) {
    const auto& group = getGroupWidget(SerialStudio::DashboardGPS, i);

    GpsPush push;
    push.sourceId = group.sourceId;
    push.series   = &m_gpsValues[i];
    push.lat      = {&kNoGpsFix, &kNeverNumeric};
    push.lon      = {&kNoGpsFix, &kNeverNumeric};
    push.alt      = {&kNoGpsFix, &kNeverNumeric};

    for (const auto& dataset : group.datasets) {
      const GpsPush::Field field{&dataset.numericValue, &dataset.isNumeric};
      if (dataset.widget == QStringLiteral("lat"))
        push.lat = field;

      if (dataset.widget == QStringLiteral("lon"))
        push.lon = field;

      if (dataset.widget == QStringLiteral("alt"))
        push.alt = field;
    }

    m_gpsPushes.push_back(push);
  }
}

/**
 * @brief Configures the FFT series data structure for the dashboard.
 *        Two passes are deliberate: the push table stores raw pointers into m_fftValues,
 *        and those addresses are stable only after that vector stops growing, so all
 *        buffers are allocated first and the pushes resolved in a second pass.
 */
void UI::Dashboard::configureFftSeries()
{
  m_fftValues.clear();
  m_fftValues.squeeze();
  m_activeFFTPlots.clear();
  m_fftPushes.clear();
  m_fftPushes.shrink_to_fit();

  const int fftCount = widgetCount(SerialStudio::DashboardFFT);
  for (int i = 0; i < fftCount; ++i) {
    const auto& dataset = getDatasetWidget(SerialStudio::DashboardFFT, i);
    m_fftValues.append(DSP::AxisData(dataset.fftSamples));
    m_activeFFTPlots.insert(i, true);
  }

  m_fftPushes.reserve(static_cast<std::size_t>(fftCount));
  for (int i = 0; i < fftCount; ++i) {
    const auto& dataset = getDatasetWidget(SerialStudio::DashboardFFT, i);

    SeriesPush push;
    push.sourceId   = dataset.sourceId;
    push.activeFlag = &m_activeFFTPlots[i];
    push.buf        = &m_fftValues[i];
    push.value      = &dataset.numericValue;
    m_fftPushes.push_back(push);
  }
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Updates time-domain ring buffers feeding all active waterfall widgets.
 */
void UI::Dashboard::updateWaterfallSeries(int sourceId)
{
  Q_ASSERT(static_cast<int>(m_waterfallPushes.size()) == m_waterfallValues.size());
  Q_ASSERT(m_activeWaterfalls.size() == m_waterfallValues.size());

  for (const auto& p : m_waterfallPushes) {
    if (!*p.activeFlag)
      continue;

    if (sourceId >= 0 && p.sourceId != sourceId)
      continue;

    p.buf->push(*p.value);
  }
}

/**
 * @brief Configures the waterfall series data structure for the dashboard.
 *        Two passes are deliberate: the push table stores raw pointers into
 *        m_waterfallValues, and those addresses are stable only after that vector stops
 *        growing, so all buffers are allocated first and the pushes resolved in a second.
 */
void UI::Dashboard::configureWaterfallSeries()
{
  m_waterfallValues.clear();
  m_waterfallValues.squeeze();
  m_activeWaterfalls.clear();
  m_waterfallPushes.clear();
  m_waterfallPushes.shrink_to_fit();

  const int waterfallCount = widgetCount(SerialStudio::DashboardWaterfall);
  for (int i = 0; i < waterfallCount; ++i) {
    const auto& dataset = getDatasetWidget(SerialStudio::DashboardWaterfall, i);
    m_waterfallValues.append(DSP::AxisData(dataset.fftSamples));
    m_activeWaterfalls.insert(i, true);
  }

  m_waterfallPushes.reserve(static_cast<std::size_t>(waterfallCount));
  for (int i = 0; i < waterfallCount; ++i) {
    const auto& dataset = getDatasetWidget(SerialStudio::DashboardWaterfall, i);

    SeriesPush push;
    push.sourceId   = dataset.sourceId;
    push.activeFlag = &m_activeWaterfalls[i];
    push.buf        = &m_waterfallValues[i];
    push.value      = &dataset.numericValue;
    m_waterfallPushes.push_back(push);
  }
}
#endif

/**
 * @brief Registers an X-axis data buffer for a dataset's custom X source. The AxisData is
 *        left unfilled on purpose so the XY ring grows from empty; a seeded (0,0) point
 *        would draw a false first line segment.
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
}

//--------------------------------------------------------------------------------------------------
// Time-ring snapshot / restore
//--------------------------------------------------------------------------------------------------

/**
 * @brief Snapshots plot time-ring contents keyed by dataset uniqueId.
 */
QHash<int, DSP::TimeRing> UI::Dashboard::snapshotPlotTimeRings() const
{
  QHash<int, DSP::TimeRing> out;
  const int n = widgetCount(SerialStudio::DashboardPlot);
  for (int i = 0; i < n; ++i) {
    const auto it = m_plotTimeRings.find(i);
    if (it == m_plotTimeRings.end())
      continue;

    const auto& d = getDatasetWidget(SerialStudio::DashboardPlot, i);
    out.insert(d.uniqueId, it.value());
  }

  return out;
}

/**
 * @brief Snapshots multiplot time-ring contents keyed by group uniqueId.
 */
QHash<int, std::vector<DSP::TimeRing>> UI::Dashboard::snapshotMultiplotTimeRings() const
{
  QHash<int, std::vector<DSP::TimeRing>> out;
  const int n = widgetCount(SerialStudio::DashboardMultiPlot);
  for (int i = 0; i < n; ++i) {
    const auto it = m_multiplotTimeRings.find(i);
    if (it == m_multiplotTimeRings.end())
      continue;

    const auto& g = getGroupWidget(SerialStudio::DashboardMultiPlot, i);
    out.insert(g.uniqueId, it.value());
  }

  return out;
}

/**
 * @brief Replays a saved ring's samples into @p target via appendDecimated. Used when
 *        the new ring has a different capacity / interval than the saved one.
 */
static void replayTimeRing(const DSP::TimeRing& saved, DSP::TimeRing& target)
{
  const std::size_t n = saved.time.size();
  for (std::size_t k = 0; k < n; ++k)
    target.appendDecimated(saved.time[k], saved.value[k]);
}

/**
 * @brief Restores saved plot rings into the currently configured widget slots. Splices when
 *        the new ring shape matches the saved one, replays through appendDecimated otherwise.
 */
void UI::Dashboard::restorePlotTimeRings(QHash<int, DSP::TimeRing>& snapshot)
{
  if (snapshot.isEmpty())
    return;

  const int n = widgetCount(SerialStudio::DashboardPlot);
  for (int i = 0; i < n; ++i) {
    auto ringIt = m_plotTimeRings.find(i);
    if (ringIt == m_plotTimeRings.end())
      continue;

    const auto& d = getDatasetWidget(SerialStudio::DashboardPlot, i);
    auto savedIt  = snapshot.find(d.uniqueId);
    if (savedIt == snapshot.end())
      continue;

    auto& live = ringIt.value();
    auto& kept = savedIt.value();

    if (live.time.capacity() == kept.time.capacity() && qFuzzyCompare(live.interval, kept.interval))
      live = std::move(kept);
    else
      replayTimeRing(kept, live);
  }
}

/**
 * @brief Restores saved multiplot rings; matches by group uniqueId and per-curve shape.
 */
void UI::Dashboard::restoreMultiplotTimeRings(QHash<int, std::vector<DSP::TimeRing>>& snapshot)
{
  if (snapshot.isEmpty())
    return;

  const int n = widgetCount(SerialStudio::DashboardMultiPlot);
  for (int i = 0; i < n; ++i) {
    auto ringIt = m_multiplotTimeRings.find(i);
    if (ringIt == m_multiplotTimeRings.end())
      continue;

    const auto& g = getGroupWidget(SerialStudio::DashboardMultiPlot, i);
    auto savedIt  = snapshot.find(g.uniqueId);
    if (savedIt == snapshot.end())
      continue;

    auto& live = ringIt.value();
    auto& kept = savedIt.value();

    const std::size_t count = std::min(live.size(), kept.size());
    for (std::size_t j = 0; j < count; ++j)
      if (live[j].time.capacity() == kept[j].time.capacity()
          && qFuzzyCompare(live[j].interval, kept[j].interval))
        live[j] = std::move(kept[j]);
      else
        replayTimeRing(kept[j], live[j]);
  }
}

/**
 * @brief Re-applies saved sweep trigger settings onto freshly configured plot engines.
 */
void UI::Dashboard::restorePlotSweepConfig(const QMap<int, DSP::SweepEngine>& saved)
{
  for (auto it = saved.begin(); it != saved.end(); ++it) {
    auto live = m_plotSweep.find(it.key());
    if (live == m_plotSweep.end())
      continue;

    const auto& src = it.value();
    live->setTrigger(src.level, src.edge, src.mode, src.holdoffSec, src.triggerCurve);
    live->setTimebase(src.timebaseSec);
    live->enabled = src.enabled;
  }
}

/**
 * @brief Re-applies saved sweep trigger settings onto freshly configured multiplot engines.
 */
void UI::Dashboard::restoreMultiplotSweepConfig(const QMap<int, DSP::SweepEngine>& saved)
{
  for (auto it = saved.begin(); it != saved.end(); ++it) {
    auto live = m_multiplotSweep.find(it.key());
    if (live == m_multiplotSweep.end())
      continue;

    const auto& src = it.value();
    live->setTrigger(src.level, src.edge, src.mode, src.holdoffSec, src.triggerCurve);
    live->setTimebase(src.timebaseSec);
    live->enabled = src.enabled;
  }
}

/**
 * @brief Configures the line series data structures for all dashboard plots.
 */
void UI::Dashboard::configureLineSeries()
{
  Q_ASSERT(m_points > 0);

  m_xAxisData.clear();
  m_yAxisData.clear();
  m_plotTimeRings.clear();
  m_plotSweep.clear();
  m_pltValues.clear();
  m_pltValues.squeeze();
  m_activePlots.clear();
  m_yLinePushes.clear();
  m_xLinePushes.clear();
  m_timePushes.clear();

  m_pltXAxis = DSP::AxisData(points() + 1);
  m_pltXAxis.fillRange(0, 1);

  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i) {
    const auto& datasets = i.value();

    for (auto d = datasets.begin(); d != datasets.end(); ++d) {
      if (!d->plt)
        continue;

      DSP::AxisData yAxis(points() + 1);
      m_yAxisData.insert(d->uniqueId, yAxis);

      registerXAxisIfNeeded(*d);
    }
  }

  for (int i = 0; i < widgetCount(SerialStudio::DashboardPlot); ++i) {
    const auto& yDataset = getDatasetWidget(SerialStudio::DashboardPlot, i);

    if (useTimeXAxis(yDataset)) {
      const int cap = timeRingCapacity(m_plotTimeRange);
      m_plotTimeRings.insert(i, DSP::TimeRing(cap, m_plotTimeRange));

      DSP::SweepEngine sweep;
      sweep.configure(1, cap, m_plotTimeRange);
      m_plotSweep.insert(i, std::move(sweep));

      DSP::LineSeries series;
      series.x = &m_pltXAxis;
      series.y = &m_yAxisData[yDataset.uniqueId];
      m_pltValues.append(series);
    }
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

    else {
      DSP::LineSeries series;
      series.x = &m_pltXAxis;
      series.y = &m_yAxisData[yDataset.uniqueId];
      m_pltValues.append(series);
    }

    m_activePlots.insert(i, true);
  }

  QHash<int, std::size_t> yByUid;
  QHash<int, std::size_t> xByXAxisId;
  for (int i = 0; i < widgetCount(SerialStudio::DashboardPlot); ++i) {
    const auto& yDataset = getDatasetWidget(SerialStudio::DashboardPlot, i);
    const LinePush::Consumer consumer{yDataset.sourceId, &m_activePlots[i]};

    if (useTimeXAxis(yDataset)) {
      auto rIt = m_plotTimeRings.find(i);
      if (rIt != m_plotTimeRings.end()) {
        auto sIt = m_plotSweep.find(i);
        TimePush tp;
        tp.consumers.push_back(consumer);
        tp.ring  = &rIt.value();
        tp.value = &yDataset.numericValue;
        tp.sweep = (sIt != m_plotSweep.end()) ? &sIt.value() : nullptr;
        m_timePushes.push_back(std::move(tp));
      }

      continue;
    }

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
 *        Two passes are deliberate: the push table stores raw pointers into m_plot3DRings,
 *        and those addresses are stable only after that vector stops growing, so all rings
 *        are allocated first and the pushes resolved in a second pass.
 */
void UI::Dashboard::configurePlot3DSeries()
{
  Q_ASSERT(m_points > 0);

  const int plot3DCount = widgetCount(SerialStudio::DashboardPlot3D);

  m_plotData3D.clear();
  m_plotData3D.squeeze();
  m_plotData3D.resize(plot3DCount);

  m_plot3DRings.clear();
  m_plot3DRings.squeeze();
  m_plot3DPushes.clear();
  m_plot3DPushes.shrink_to_fit();

  m_plot3DRings.reserve(plot3DCount);
  for (int i = 0; i < plot3DCount; ++i) {
    m_plot3DRings.append(DSP::FixedQueue<QVector3D>(static_cast<std::size_t>(points())));
    m_plotData3D[i].reserve(static_cast<std::size_t>(points()));
  }

  m_plot3DPushes.reserve(static_cast<std::size_t>(plot3DCount));
  for (int i = 0; i < plot3DCount; ++i) {
    const auto& group = getGroupWidget(SerialStudio::DashboardPlot3D, i);

    Plot3DPush push;
    push.sourceId = group.sourceId;
    push.ring     = &m_plot3DRings[i];
    push.x        = &kZeroAxisSource;
    push.y        = &kZeroAxisSource;
    push.z        = &kZeroAxisSource;

    for (const auto& dataset : group.datasets) {
      const QString& id = dataset.widget;
      if (id == QStringLiteral("x") || id == QStringLiteral("X"))
        push.x = &dataset.numericValue;

      if (id == QStringLiteral("y") || id == QStringLiteral("Y"))
        push.y = &dataset.numericValue;

      if (id == QStringLiteral("z") || id == QStringLiteral("Z"))
        push.z = &dataset.numericValue;
    }

    m_plot3DPushes.push_back(push);
  }
}
#endif

/**
 * @brief Configures the multi-line series data structure for the dashboard.
 */
void UI::Dashboard::configureMultiLineSeries()
{
  Q_ASSERT(m_points > 0);

  m_multipltValues.clear();
  m_multipltValues.squeeze();
  m_activeMultiplots.clear();
  m_multiplotTimeRings.clear();
  m_multiplotSweep.clear();

  m_multipltXAxis = DSP::AxisData(points() + 1);
  m_multipltXAxis.fillRange(0, 1);

  for (int i = 0; i < widgetCount(SerialStudio::DashboardMultiPlot); ++i) {
    const auto& group = getGroupWidget(SerialStudio::DashboardMultiPlot, i);

    DSP::MultiLineSeries series;
    series.x = &m_multipltXAxis;
    for (size_t j = 0; j < group.datasets.size(); ++j)
      series.y.push_back(DSP::AxisData(points() + 1));

    m_multipltValues.append(series);
    m_activeMultiplots.insert(i, true);

    if (useTimeXAxisGroup(group)) {
      const int cap = timeRingCapacity(m_plotTimeRange);
      std::vector<DSP::TimeRing> rings;
      rings.reserve(group.datasets.size());
      for (size_t j = 0; j < group.datasets.size(); ++j)
        rings.emplace_back(cap, m_plotTimeRange);

      m_multiplotTimeRings.insert(i, std::move(rings));

      DSP::SweepEngine sweep;
      sweep.configure(static_cast<int>(group.datasets.size()), cap, m_plotTimeRange);
      m_multiplotSweep.insert(i, std::move(sweep));
    }
  }

  buildMultiplotPushes();
}

/**
 * @brief Resolves the per-tick multiplot push table from the configured buffers.
 */
void UI::Dashboard::buildMultiplotPushes()
{
  m_multiplotPushes.clear();
  m_multiplotPushes.shrink_to_fit();

  const int multiCount = widgetCount(SerialStudio::DashboardMultiPlot);
  m_multiplotPushes.reserve(static_cast<std::size_t>(multiCount));

  for (int i = 0; i < multiCount; ++i) {
    const auto& group = getGroupWidget(SerialStudio::DashboardMultiPlot, i);

    MultiPush push;
    push.sourceId   = group.sourceId;
    push.activeFlag = &m_activeMultiplots[i];

    auto swIt  = m_multiplotSweep.find(i);
    push.sweep = (swIt != m_multiplotSweep.end()) ? &swIt.value() : nullptr;

    auto rIt = m_multiplotTimeRings.find(i);
    if (rIt != m_multiplotTimeRings.end()) {
      auto& rings        = rIt.value();
      const size_t count = std::min(group.datasets.size(), rings.size());
      for (size_t j = 0; j < count; ++j)
        push.timeCurves.push_back({&rings[j], &group.datasets[j].numericValue});
    }

    else {
      auto& multiSeries   = m_multipltValues[i];
      const size_t yCount = multiSeries.y.size();
      const size_t count  = std::min(group.datasets.size(), yCount);
      for (size_t j = 0; j < count; ++j)
        push.samples.emplace_back(&multiSeries.y[j], &group.datasets[j].numericValue);
    }

    m_multiplotPushes.push_back(std::move(push));
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
  if (frame.groups.size() <= 0)
    return;

  m_actions.clear();
  m_actions.squeeze();

  for (auto it = m_timers.begin(); it != m_timers.end(); ++it) {
    if (it.value()) {
      disconnect(it.value());
      it.value()->stop();
      delete it.value();
    }
  }

  m_timers.clear();
  m_repeatCounters.clear();

  for (const auto& action : frame.actions)
    m_actions.append(action);

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

    const bool isRepeat = action.timerMode == DataModel::TimerMode::RepeatNTimes;
    if (isRepeat && action.autoExecuteOnConnect) {
      m_repeatCounters[i] = qMax(1, action.repeatCount);
      timer->start();
    }

    else if (!isRepeat
             && (action.timerMode == DataModel::TimerMode::AutoStart
                 || action.autoExecuteOnConnect))
      timer->start();

    m_timers.insert(i, timer);
  }

  Q_EMIT actionStatusChanged();
}
