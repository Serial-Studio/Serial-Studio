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

#include "UI/Dashboard.h"

#include "IO/Manager.h"
#include "CSV/Player.h"
#include "Misc/TimerEvents.h"
#include "JSON/FrameBuilder.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Client.h"
#endif

#include <QTimer>

//------------------------------------------------------------------------------
// Constructor & singleton access
//------------------------------------------------------------------------------

/**
 * @brief Constructs the Dashboard object and establishes connections for
 *        various signal sources that may trigger data reset or frame
 *        processing.
 */
UI::Dashboard::Dashboard()
  : m_points(100)
  , m_widgetCount(0)
  , m_updateRequired(false)
  , m_showActionPanel(true)
  , m_terminalEnabled(false)
  , m_showTaskbarButtons(false)
  , m_pltXAxis(100)
  , m_multipltXAxis(100)
{
  // clang-format off
  connect(&CSV::Player::instance(), &CSV::Player::openChanged, this, [=, this] { resetData(true); });
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this, [=, this] { resetData(true); });
  connect(&JSON::FrameBuilder::instance(), &JSON::FrameBuilder::jsonFileMapChanged, this, [=, this] { resetData(); });
  // clang-format on

  // Reset dashboard data if MQTT client is subscribed
#ifdef BUILD_COMMERCIAL
  connect(&MQTT::Client::instance(), &MQTT::Client::connectedChanged, this,
          [=, this] {
            const bool subscribed = MQTT::Client::instance().isSubscriber();
            const bool wasSubscribed
                = !MQTT::Client::instance().isConnected()
                  && MQTT::Client::instance().isSubscriber();

            if (subscribed || wasSubscribed)
              resetData(true);
          });
#endif

  // Update the dashboard widgets at 24 Hz
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::uiTimeout, this,
          [=, this] {
            if (m_updateRequired)
            {
              m_updateRequired = false;
              Q_EMIT updated();
            }
          });

  // Update action items when frame format changes
  connect(this, &UI::Dashboard::widgetCountChanged, this,
          &UI::Dashboard::actionStatusChanged);
}

/**
 * @brief Retrieves the singleton instance of the Dashboard.
 * @return Reference to the singleton Dashboard instance.
 */
UI::Dashboard &UI::Dashboard::instance()
{
  static Dashboard instance;
  return instance;
}

//------------------------------------------------------------------------------
// Utility functions for widgets
//------------------------------------------------------------------------------

/**
 * @brief Calculates a suitable interval for dividing a range into "nice" steps.
 *
 * Determines an interval size that is visually pleasing and divides the
 * specified range evenly, adjusting for both small and large scales.
 *
 * @param min The minimum value of the range.
 * @param max The maximum value of the range.
 * @param multiplier Scaling factor for step size granularity (default = 0.2).
 * @return double The computed interval size for the given range.
 *
 * @note Common intervals (e.g., 0.1, 0.2, 0.5, 1, 2, 5, 10) are selected
 *       to enhance readability. Ensures the interval divides the range evenly.
 */
double UI::Dashboard::smartInterval(const double min, const double max,
                                    const double multiplier)
{
  // dDfault fallback for degenerate range
  const double range = qAbs(max - min);
  if (range == 0.0)
    return 1.0;

  // Estimate initial step size
  const int magnitude = static_cast<int>(std::ceil(std::log10(range)));
  const double scale = std::pow(10.0, -magnitude) * 10;
  const double normalizedRange = std::ceil(range * scale) / scale;
  double step = qMax(0.0001, normalizedRange * multiplier);

  // For smaller steps, use 0.1, 0.2, 0.5, etc.
  if (step < 1.0)
  {
    if (step <= 0.1)
      step = 0.1;
    else if (step <= 0.2)
      step = 0.2;
    else if (step <= 0.5)
      step = 0.5;
    else
      step = 1.0;
  }

  // For larger steps, round to 1, 2, 5, 10, etc.
  else
  {
    const double base = std::pow(10.0, std::floor(std::log10(step)));
    const double normalized = step / base;

    if (normalized <= 1.0)
      step = base;
    else if (normalized <= 2.0)
      step = 2 * base;
    else if (normalized <= 5.0)
      step = 5 * base;
    else
      step = 10 * base;
  }

  // Ensure the interval divides the range
  const double remainder = std::fmod(range, step);
  if (remainder != 0.0)
    step = range / std::ceil(range / step);

  // Return obtained step size
  return step;
}

//------------------------------------------------------------------------------
// Dashboard features getters
//------------------------------------------------------------------------------

/**
 * @brief Checks if the dashboard is currently available, determined by the
 *        total widget count.
 *
 * @return True if the dashboard has widgets; false otherwise.
 */
bool UI::Dashboard::available() const
{
  return totalWidgetCount() > 0 && streamAvailable();
}

/**
 * @brief Returns @c true if a rectangle with a list of actions should be
 *        displayed alongside the dashboard.
 */
bool UI::Dashboard::showActionPanel() const
{
  return m_showActionPanel;
}

/**
 * @brief Checks if the dashboard is currently available, determined by the
 *        data stream sources.
 *
 * @return True if at least one data source/stream is active.
 */
bool UI::Dashboard::streamAvailable() const
{
  static auto &player = CSV::Player::instance();
  static auto &manager = IO::Manager::instance();

  const bool csvOpen = player.isOpen();
  const bool serialConnected = manager.isConnected();

#ifdef BUILD_COMMERCIAL
  static auto &mqtt = MQTT::Client::instance();
  const bool mqttConnected = mqtt.isConnected() && mqtt.isSubscriber();
  return serialConnected || csvOpen || mqttConnected;
#else
  return serialConnected || csvOpen;
#endif
}

/**
 * @brief Returns @c true if a terminal widget should be displayed within
 *        the dashboard.
 */
bool UI::Dashboard::terminalEnabled() const
{
  return m_terminalEnabled;
}

/**
 * @brief Determines if the taskbar buttons should be always visible.
 *
 * By default, the taskbar buttons are only shown when either:
 * - At least a window is maximized
 * - Or the window in question is closed or minimized.
 *
 * If this function returns @c true, the taskbar will behave similar to
 * what someone would expect from GNOME 2 or Windows, where all taskbar
 * buttons are active regardless of window state.
 */
bool UI::Dashboard::showTaskbarButtons() const
{
  return m_showTaskbarButtons;
}

/**
 * @brief Determines if the point-selector widget should be visible based on the
 *        presence of relevant widget groups or datasets.
 *
 * @return True if points widget visibility is enabled; false otherwise.
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
 * @brief Determines if the decimal-point precision selector widget should be
 *        visible based on the presence of specific widget groups or datasets.
 *
 * @return True if precision widget visibility is enabled; false otherwise.
 */
bool UI::Dashboard::precisionWidgetVisible() const
{
  return m_widgetGroups.contains(SerialStudio::DashboardAccelerometer)
         || m_widgetGroups.contains(SerialStudio::DashboardGyroscope)
         || m_widgetGroups.contains(SerialStudio::DashboardDataGrid)
         || m_widgetDatasets.contains(SerialStudio::DashboardBar)
         || m_widgetDatasets.contains(SerialStudio::DashboardGauge)
         || m_widgetDatasets.contains(SerialStudio::DashboardCompass);
}

/**
 * Returns @c true if the frame contains features that should only be enabled
 * for commercial users with a valid license, such as the 3D plot widget.
 */
bool UI::Dashboard::containsCommercialFeatures() const
{
  return m_rawFrame.containsCommercialFeatures;
}

//------------------------------------------------------------------------------
// Dashboard getters
//------------------------------------------------------------------------------

/**
 * @brief Gets the current point/sample count setting for the dashboard plots.
 * @return Current point count.
 */
int UI::Dashboard::points() const
{
  return m_points;
}

/**
 * @brief Retrieves the count of actions available within the dashboard.
 * @return The count of dashboard actions.
 */
int UI::Dashboard::actionCount() const
{
  return m_actions.count();
}

/**
 * @brief Gets the total count of widgets currently available on the dashboard.
 * @return Total widget count.
 */
int UI::Dashboard::totalWidgetCount() const
{
  return m_widgetCount;
}

//------------------------------------------------------------------------------
// QML-callable status functions
//------------------------------------------------------------------------------

/**
 * @brief Checks if the current frame is valid for processing.
 * @return True if the current frame is valid; false otherwise.
 */
bool UI::Dashboard::frameValid() const
{
  return m_lastFrame.groups.size() > 0;
}

/**
 * @brief Retrieves the relative index of a widget within the list of widgets
 *        that share the similar type (e.g. plot, compass, bar, etc) based on
 *        the given widget index.
 *
 * @param widgetIndex The global index of the widget.
 * @return The relative index if found; -1 otherwise.
 */
int UI::Dashboard::relativeIndex(const int widgetIndex)
{
  const auto it = m_widgetMap.constFind(widgetIndex);
  return it != m_widgetMap.cend() ? it->second : -1;
}

/**
 * @brief Formats a numerical value according to its context range.
 *
 * This method delegates to `FMT_VAL` to determine the appropriate number of
 * decimal places for a given value, based on the provided `min` and `max`
 * range. It is typically used to provide human-readable display of values
 * within QML UIs.
 *
 * @param val The value to format.
 * @param min The minimum value of the expected range.
 * @param max The maximum value of the expected range.
 * @return A QString representing the formatted value.
 *
 * @see FMT_VAL
 */
QString UI::Dashboard::formatValue(double val, double min, double max) const
{
  return FMT_VAL(val, min, max);
}

/**
 * @brief Retrieves the type of widget associated with a given widget index.
 *
 * @param widgetIndex The global index of the widget.
 * @return The widget type, or SerialStudio::DashboardNoWidget if the index is
 * not found.
 */
SerialStudio::DashboardWidget UI::Dashboard::widgetType(const int widgetIndex)
{
  const auto it = m_widgetMap.constFind(widgetIndex);
  return it != m_widgetMap.cend() ? it->first : SerialStudio::DashboardNoWidget;
}

/**
 * @brief Counts the number of instances of a specified widget type within the
 *        dashboard.
 *
 * @param widget The type of widget to count.
 * @return The count of instances for the specified widget type.
 */
int UI::Dashboard::widgetCount(const SerialStudio::DashboardWidget widget) const
{
  if (SerialStudio::isGroupWidget(widget))
  {
    auto it = m_widgetGroups.constFind(widget);
    return it != m_widgetGroups.cend() ? it->count() : 0;
  }

  if (SerialStudio::isDatasetWidget(widget))
  {
    auto it = m_widgetDatasets.constFind(widget);
    return it != m_widgetDatasets.cend() ? it->count() : 0;
  }

  return 0;
}

//------------------------------------------------------------------------------
// Model access functions
//------------------------------------------------------------------------------

/**
 * @brief Retrieves the title of the current frame in the dashboard.
 * @return A reference to a QString containing the current frame title.
 */
const QString &UI::Dashboard::title() const
{
  return m_lastFrame.title;
}

/**
 * @brief Returns a list of available dashboard actions with their metadata.
 *
 * Each action is represented as a QVariantMap containing the following keys:
 * - `"id"`: The index of the action (used for identification).
 * - `"text"`: The display title of the action.
 * - `"icon"`: A path to the icon resource associated with the action.
 * - `"checked"`: A boolean indicating the toggle state of the action.
 *                This is only true if the action uses
 * TimerMode::ToggleOnTrigger and its corresponding timer is currently active.
 *
 * This method is used to populate the user interface with actionable items,
 * such as buttons. The "checked" state allows UI components to reflect
 * whether an action with toggle behavior is currently running.
 *
 * @return A QVariantList of QVariantMaps describing each action.
 */
QVariantList UI::Dashboard::actions() const
{
  QVariantList actions;
  for (int i = 0; i < m_actions.count(); ++i)
  {
    const auto &action = m_actions[i];

    QVariantMap m;
    m["id"] = i;
    m["checked"] = false;
    m["text"] = action.title;
    m["icon"] = QStringLiteral("qrc:/rcc/actions/%1.svg").arg(action.icon);
    if (action.timerMode == JSON::TimerMode::ToggleOnTrigger)
    {
      if (m_timers.contains(i) && m_timers[i] && m_timers[i]->isActive())
        m["checked"] = true;
    }

    actions.append(m);
  }

  return actions;
}

/**
 * @brief Retrieves a map containing information about all widgets/windows
 *        in the dashboard. The map key is the window index (UI order), and
 *        the value is a pair consisting of the widget type and the widget’s
 *        index relative to its type.
 *
 * For example, if there are two FFT plot widgets:
 * - The map will contain two entries with unique window indices.
 * - Each value will have the widget type (DashboardFFT) and an index
 *   indicating its order among FFT widgets (0 for the first, 1 for the second).
 *
 * This structure is used to track and categorize widgets for both rendering
 * and logical grouping (e.g., building a tree model of the dashboard layout).
 *
 * @return Reference to the widget map (QMap<int, QPair<DashboardWidget, int>>)
 *         where:
 *         - int: Window number in the UI
 *         - DashboardWidget: Enum/type identifying the widget type
 *         - int: Index relative to its widget type
 */
const SerialStudio::WidgetMap &UI::Dashboard::widgetMap() const
{
  return m_widgetMap;
}

//------------------------------------------------------------------------------
// Dataset & group access functions
//------------------------------------------------------------------------------

/**
 * @brief Provides access to the map of dataset objects.
 *
 * This function returns a constant reference to the map that associates dataset
 * indexes with their corresponding `JSON::Dataset` objects.
 *
 * @return A constant reference to the `QMap` mapping dataset indexes (`int`)
 *         to their respective `JSON::Dataset` objects.
 *
 * @note The map can be used to retrieve datasets by their index.
 */
const QMap<int, JSON::Dataset> &UI::Dashboard::datasets() const
{
  return m_datasets;
}

/**
 * @brief Retrieves a group widget by type and index.
 *
 * This function returns a constant reference to a @c JSON::Group object
 * corresponding to the specified widget type and index.
 *
 * If the widget type does not exist or the index is out of bounds, a
 * default-constructed (empty) group is returned and a warning is logged.
 *
 * @param widget The type of dashboard widget.
 * @param index  The index of the widget within its group type.
 * @return Reference to the requested @c JSON::Group, or an empty group if not
 *         found.
 *
 * @note This function is production-safe and will not crash if invalid
 *       arguments are provided. It logs warnings for missing widget types or
 *       out-of-bounds indices.
 */
const JSON::Group &
UI::Dashboard::getGroupWidget(const SerialStudio::DashboardWidget widget,
                              const int index) const
{
  static const JSON::Group emptyGroup;
  const auto it = m_widgetGroups.constFind(widget);

  if (it == m_widgetGroups.cend()) [[unlikely]]
  {
    qWarning() << "getGroupWidget: widget type not found:" << widget;
    return emptyGroup;
  }

  if (index < 0 || index >= it->size()) [[unlikely]]
  {
    qWarning() << "getGroupWidget: index out of bounds:" << index
               << "for widget" << widget;
    return emptyGroup;
  }

  return it->at(index);
}

/**
 * @brief Retrieves a dataset widget by type and index.
 *
 * This function returns a constant reference to a @c JSON::Dataset object
 * corresponding to the specified widget type and index.
 *
 * If the widget type does not exist or the index is out of bounds, a
 * default-constructed (empty) dataset is returned and a warning is logged.
 *
 * @param widget The type of dashboard widget.
 * @param index  The index of the dataset within its widget type.
 * @return Reference to the requested @c JSON::Dataset, or an empty dataset if
 *         not found.
 *
 * @note This function is production-safe and will not crash if invalid
 *       arguments are provided. It logs warnings for missing widget types or
 *       out-of-bounds indices.
 */
const JSON::Dataset &
UI::Dashboard::getDatasetWidget(const SerialStudio::DashboardWidget widget,
                                const int index) const
{
  static const JSON::Dataset emptyDataset;
  const auto it = m_widgetDatasets.constFind(widget);

  if (it == m_widgetDatasets.cend()) [[unlikely]]
  {
    qWarning() << "getDatasetWidget: widget type not found:" << widget;
    return emptyDataset;
  }

  if (index < 0 || index >= it->size()) [[unlikely]]
  {
    qWarning() << "getDatasetWidget: index out of bounds:" << index
               << "for widget" << widget;
    return emptyDataset;
  }

  return it->at(index);
}

//------------------------------------------------------------------------------
// Frame access
//------------------------------------------------------------------------------

/**
 * @brief Retrieves the last unmodified JSON frame for the dashboard.
 * @return A reference to the current JSON::Frame.
 */
const JSON::Frame &UI::Dashboard::rawFrame()
{
  return m_rawFrame;
}

/**
 * @brief Retrieves the processed JSON frame for the dashboard. Processing
 *        can add several group-level widgets, such as terminals, multiplots,
 *        etc...
 *
 * @return A reference to the current JSON::Frame.
 */
const JSON::Frame &UI::Dashboard::processedFrame()
{
  return m_lastFrame;
}

//------------------------------------------------------------------------------
// Time-series data access
//------------------------------------------------------------------------------

/**
 * @brief Returns the FFT plot data currently displayed on the dashboard.
 *
 * @param index The widget index for the FFT plot.
 * @return Reference to the corresponding AxisData buffer.
 */
const DSP::AxisData &UI::Dashboard::fftData(const int index) const
{
  return m_fftValues[index];
}

/**
 * @brief Returns the GPS trajectory data currently tracked by the dashboard.
 *
 * @param index The widget index for the GPS display.
 * @return Reference to the corresponding GpsSeries structure.
 */
const DSP::GpsSeries &UI::Dashboard::gpsSeries(const int index) const
{
  return m_gpsValues[index];
}

/**
 * @brief Returns the Y-axis values for a linear plot widget.
 *
 * @param index The widget index for the linear plot.
 * @return Reference to the corresponding LineSeries buffer.
 */
const DSP::LineSeries &UI::Dashboard::plotData(const int index) const
{
  return m_pltValues[index];
}

/**
 * @brief Returns the series data used by a multiplot widget.
 *
 * @param index The widget index for the multiplot.
 * @return Reference to the corresponding MultiLineSeries container.
 */
const DSP::MultiLineSeries &UI::Dashboard::multiplotData(const int index) const
{
  return m_multipltValues[index];
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Returns the 3D trajectory data for a 3D plot widget.
 *
 * @param index The widget index for the 3D plot.
 * @return Reference to the corresponding LineSeries3D buffer.
 */
const DSP::LineSeries3D &UI::Dashboard::plotData3D(const int index) const
{
  return m_plotData3D[index];
}
#endif

//------------------------------------------------------------------------------
// Plot active status getters
//------------------------------------------------------------------------------

/**
 * @brief Checks whether a plot is currently active.
 *
 * @param index Plot index to query.
 * @return @c true if the plot is running, otherwise @c false.
 *         Returns @c false if the index is not registered.
 */
bool UI::Dashboard::plotRunning(const int index)
{
  if (m_activePlots.contains(index))
    return m_activePlots[index];

  return false;
}

/**
 * @brief Checks whether an FFT plot is currently active.
 *
 * @param index FFT plot index to query.
 * @return @c true if the FFT plot is running, otherwise @c false.
 *         Returns @c false if the index is not registered.
 */
bool UI::Dashboard::fftPlotRunning(const int index)
{
  if (m_activeFFTPlots.contains(index))
    return m_activeFFTPlots[index];

  return false;
}

/**
 * @brief Checks whether a multiplot is currently active.
 *
 * @param index Multiplot index to query.
 * @return @c true if the multiplot is running, otherwise @c false.
 *         Returns @c false if the index is not registered.
 */
bool UI::Dashboard::multiplotRunning(const int index)
{
  if (m_activeMultiplots.contains(index))
    return m_activeMultiplots[index];

  return false;
}

//------------------------------------------------------------------------------
// Setter functions
//------------------------------------------------------------------------------

/**
 * @brief Sets the number of data points for the dashboard plots.
 *
 * This function updates the total number of points (samples) used in the plots
 * and reconfigures the data structures for linear and multi-line series to
 * reflect the new point count.
 *
 * @param points The new number of data points (samples).
 */
void UI::Dashboard::setPoints(const int points)
{
  if (m_points != points)
  {
    // Update number of points
    m_points = points;

    // Update plot data structures
    configureLineSeries();
    configureMultiLineSeries();

    // Update the UI
    Q_EMIT pointsChanged();
  }
}

/**
 * @brief Resets all data in the dashboard, including plot values,
 *        widget structures, and actions. Emits relevant signals to notify the
 *        UI about the reset state.
 */
void UI::Dashboard::resetData(const bool notify)
{
  // Clear plotting data
  m_fftValues.clear();
  m_pltValues.clear();
  m_multipltValues.clear();

  // Free memory associated with the containers of the plotting data
  m_fftValues.squeeze();
  m_pltValues.squeeze();
  m_multipltValues.squeeze();

  // Clear data for 3D plots
#ifdef BUILD_COMMERCIAL
  m_plotData3D.clear();
  m_plotData3D.squeeze();
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

  // Clear activity status flags for plot widgets
  m_activePlots.clear();
  m_activeFFTPlots.clear();
  m_activeMultiplots.clear();

  // Reset frame data
  m_rawFrame = JSON::Frame();
  m_lastFrame = JSON::Frame();
  m_updateRetryInProgress = false;

  // Configure actions
  auto *frameBuilder = &JSON::FrameBuilder::instance();
  if (frameBuilder->operationMode() == SerialStudio::ProjectFile)
    configureActions(frameBuilder->frame());

  // Notify user interface
  if (notify)
  {
    m_updateRequired = true;

    Q_EMIT updated();
    Q_EMIT dataReset();
    Q_EMIT widgetCountChanged();
    Q_EMIT containsCommercialFeaturesChanged();
  }
}

/**
 * @brief Enables/disables the action panel.
 */
void UI::Dashboard::setShowActionPanel(const bool enabled)
{
  if (m_showActionPanel != enabled)
  {
    m_showActionPanel = enabled;
    Q_EMIT showActionPanelChanged();
  }
}

/**
 * @brief Enables/disables adding a terminal widget.
 */
void UI::Dashboard::setTerminalEnabled(const bool enabled)
{
  if (m_terminalEnabled != enabled)
  {
    m_terminalEnabled = enabled;
    const auto frame = m_rawFrame;
    resetData(false);
    hotpathRxFrame(frame);
  }

  Q_EMIT terminalEnabledChanged();
}

/**
 * @brief Enables/disables displaying all taskbar buttons, regardless of
 *        window state.
 */
void UI::Dashboard::setShowTaskbarButtons(const bool enabled)
{
  if (m_showTaskbarButtons != enabled)
  {
    m_showTaskbarButtons = enabled;
    Q_EMIT showTaskbarButtonsChanged();
  }
}

//------------------------------------------------------------------------------
// Action activation, more complex that it seems...
//------------------------------------------------------------------------------

/**
 * @brief Activates a dashboard action by transmitting its associated data and
 * handling timer logic.
 *
 * This function is responsible for executing an action defined in the current
 * dashboard configuration. It sends the action's payload over the serial
 * interface and manages timer behavior based on the action's configured
 * TimerMode.
 *
 * @param index The index of the action to activate. Must be within bounds of
 *              the current action list.
 * @param guiTrigger Indicates whether the action was triggered by user
 *                   interaction (e.g. from the GUI). This affects behavior for
 *                   actions using TimerMode::ToggleOnTrigger—toggling only
 *                   occurs if the trigger originated from the GUI.
 *
 * Behavior:
 * - If the action is configured with TimerMode::StartOnTrigger, the timer will
 *   start on first activation.
 * - If the action is configured with TimerMode::ToggleOnTrigger, the timer
 *   toggles on GUI-triggered calls.
 * - All actions result in data being transmitted via IO::Manager, using either
 *   binary or text formatting.
 *
 * Emits:
 * - actionStatusChanged() signal to notify the UI that the action state may
 *   have changed (e.g. toggle state).
 */
void UI::Dashboard::activateAction(const int index, const bool guiTrigger)
{
  // Validate index
  if (index < 0 || index >= m_actions.count())
  {
    qWarning() << "Invalid action index:" << index;
    return;
  }

  // Obtain action data
  const auto &action = m_actions[index];

  // Handle timer behavior
  if (m_timers.contains(index))
  {
    auto *timer = m_timers[index];
    if (!timer)
      qWarning() << "Invalid timer pointer for action" << action.title;

    else
    {
      if (action.timerMode == JSON::TimerMode::StartOnTrigger)
      {
        if (!timer->isActive())
          timer->start();
      }

      else if (action.timerMode == JSON::TimerMode::ToggleOnTrigger)
      {
        if (guiTrigger)
        {
          if (timer->isActive())
            timer->stop();
          else
            timer->start();
        }
      }
    }
  }

  // Send data payload
  if (!IO::Manager::instance().paused())
    IO::Manager::instance().writeData(JSON::get_tx_bytes(action));

  // Update action model
  Q_EMIT actionStatusChanged();
}

//------------------------------------------------------------------------------
// Plot activation status setters
//------------------------------------------------------------------------------

/**
 * @brief Sets the active state of a plot.
 *
 * @param index Plot index to update.
 * @param enabled Set to @c true to mark running, or @c false to pause.
 *
 * Has no effect if @p index is not registered.
 */
void UI::Dashboard::setPlotRunning(const int index, const bool enabled)
{
  if (m_activePlots.contains(index))
    m_activePlots[index] = enabled;
}

/**
 * @brief Sets the active state of an FFT plot.
 *
 * @param index FFT plot index to update.
 * @param enabled Set to @c true to mark running, or @c false to pause.
 *
 * Has no effect if @p index is not registered.
 */
void UI::Dashboard::setFFTPlotRunning(const int index, const bool enabled)
{
  if (m_activeFFTPlots.contains(index))
    m_activeFFTPlots[index] = enabled;
}

/**
 * @brief Sets the active state of a multiplot.
 *
 * @param index Multiplot index to update.
 * @param enabled Set to @c true to mark running, or @c false to pause.
 *
 * Has no effect if @p index is not registered.
 */
void UI::Dashboard::setMultiplotRunning(const int index, const bool enabled)
{
  if (m_activeMultiplots.contains(index))
    m_activeMultiplots[index] = enabled;
}

//------------------------------------------------------------------------------
// Hotpath data processing
//------------------------------------------------------------------------------

/**
 * @brief Processes an incoming data frame and updates the dashboard
 *        accordingly.
 *
 * Validates the frame, checks for commercial feature flags, and compares the
 * frame structure with the current configuration. If the structure has changed,
 * the dashboard is reconfigured. Finally, updates dataset values and plots.
 *
 * @param frame The new JSON data frame to process.
 */
void UI::Dashboard::hotpathRxFrame(const JSON::Frame &frame)
{
  // Validate frame
  if (frame.groups.size() <= 0 || !streamAvailable()) [[unlikely]]
    return;

  // Regenerate dashboard model if frame structure changed
  if (!JSON::compare_frames(frame, m_rawFrame) || m_datasetReferences.isEmpty())
      [[unlikely]]
  {
    const bool hadProFeatures = m_rawFrame.containsCommercialFeatures;
    reconfigureDashboard(frame);
    if (hadProFeatures != frame.containsCommercialFeatures)
      Q_EMIT containsCommercialFeaturesChanged();
  }

  // Update dashboard data
  updateDashboardData(frame);

  // Set dashboard update flag
  m_updateRequired = true;
}

//------------------------------------------------------------------------------
// Frame processing & dashboard model generation
//------------------------------------------------------------------------------

/**
 * @brief Updates dataset values and plot data based on the given frame.
 *
 * Iterates through groups and datasets in the frame, updating internal
 * data structures with the latest values.
 *
 * @param frame The JSON frame containing new dataset values.
 */
void UI::Dashboard::updateDashboardData(const JSON::Frame &frame)
{
  // Update all datasets of the frame
  for (const auto &group : frame.groups)
  {
    for (const auto &dataset : group.datasets)
    {
      // Get the unique ID of the dataset
      const auto uid = dataset.uniqueId;
      const auto it = m_datasetReferences.find(uid);

      // Cannot find dataset UID
      if (it == m_datasetReferences.end()) [[unlikely]]
      {
        // Break recursion
        if (m_updateRetryInProgress)
        {
          qWarning() << "Failed to build dashboard widget model";

          // Disconnect from data source
          if (IO::Manager::instance().isConnected())
            IO::Manager::instance().disconnectDevice();
          else if (CSV::Player::instance().isOpen())
            CSV::Player::instance().closeFile();
#ifdef BUILD_COMMERCIAL
          else if (MQTT::Client::instance().isConnected())
            MQTT::Client::instance().closeConnection();
#endif

          return;
        }

        // Re-generate dashboard model
        reconfigureDashboard(frame);

        // Try running the update again
        m_updateRetryInProgress = true;
        updateDashboardData(frame);
        m_updateRetryInProgress = false;

        return;
      }

      // Update all datasets for the given UID
      const auto &datasets = it.value();
      for (auto *ptr : datasets)
      {
        ptr->value = dataset.value;
        ptr->isNumeric = dataset.isNumeric;
        ptr->numericValue = dataset.numericValue;
      }
    }
  }

  // Update plots & time-series widgets
  updateDataSeries();
}

/**
 * @brief Reconfigures the dashboard layout and widgets based on the new frame.
 *
 * Clears existing dashboard and plot data, detects appropriate widget mappings
 * for each group and dataset, handles commercial-only widgets (with fallback),
 * and regenerates widget model mappings. Emits signals if the widget or action
 * counts have changed.
 *
 * @param frame The JSON frame with the new structure to configure.
 * @param pro Indicates whether commercial (pro) features are enabled.
 */
void UI::Dashboard::reconfigureDashboard(const JSON::Frame &frame)
{
  // Check if we can use pro features
  const bool pro = SerialStudio::activated();

  // Reset dashboard data
  resetData(false);

  // Save frame structure
  m_rawFrame = frame;
  m_lastFrame = frame;

  // Add terminal group
  if (m_terminalEnabled)
  {
    JSON::Group terminal;
    terminal.widget = "terminal";
    terminal.title = tr("Console");
    terminal.groupId = m_lastFrame.groups.size();

    m_lastFrame.groups.push_back(terminal);
  }

  // Parse frame groups
  for (const auto &group : m_lastFrame.groups)
  {
    // Append group widgets
    const auto key = SerialStudio::getDashboardWidget(group);
    if (key != SerialStudio::DashboardNoWidget)
      m_widgetGroups[key].append(group);

    // Append fallback 3D plot widget
    if (key == SerialStudio::DashboardPlot3D && !pro)
    {
      m_widgetGroups.remove(key);
      auto copy = group;
      copy.title = tr("%1 (Fallback)").arg(group.title);
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(copy);
      for (size_t i = 0; i < m_lastFrame.groups.size(); ++i)
      {
        if (m_lastFrame.groups[i].groupId == group.groupId)
        {
          m_lastFrame.groups[i].title = copy.title;
          m_lastFrame.groups[i].widget = "multiplot";
          break;
        }
      }
    }

    // Append multiplot & 3D plot to accelerometer widget
    if (key == SerialStudio::DashboardAccelerometer)
    {
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(group);
      if (pro)
        m_widgetGroups[SerialStudio::DashboardPlot3D].append(group);
    }

    // Append multiplot to gyro widget
    if (key == SerialStudio::DashboardGyroscope)
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(group);

    // Parse group datasets
    JSON::Group ledPanel;
    for (const auto &dataset : group.datasets)
    {
      // Register a new dataset
      if (!m_datasets.contains(dataset.index))
        m_datasets.insert(dataset.index, dataset);

      // Dataset already registered, update min/max values
      else
      {
        auto prev = m_datasets.value(dataset.index);
        double newMin = qMin(prev.pltMin, dataset.pltMin);
        double newMax = qMax(prev.pltMax, dataset.pltMax);

        auto d = dataset;
        d.pltMin = newMin;
        d.pltMax = newMax;
        m_datasets.insert(dataset.index, d);
      }

      // Register dataset widgets
      auto keys = SerialStudio::getDashboardWidgets(dataset);
      for (const auto &widgetKeys : std::as_const(keys))
      {
        if (widgetKeys == SerialStudio::DashboardLED)
          ledPanel.datasets.push_back(dataset);

        else if (widgetKeys != SerialStudio::DashboardNoWidget)
          m_widgetDatasets[widgetKeys].append(dataset);
      }
    }

    // Add group-level LED panel
    if (ledPanel.datasets.size() > 0)
    {
      ledPanel.widget = "led-panel";
      ledPanel.groupId = group.groupId;
      ledPanel.title = tr("LED Panel (%1)").arg(group.title);
      m_widgetGroups[SerialStudio::DashboardLED].append(ledPanel);
    }
  }

  // Generate group model map
  for (auto i = m_widgetGroups.begin(); i != m_widgetGroups.end(); ++i)
  {
    const auto key = i.key();
    const auto count = widgetCount(key);
    for (int j = 0; j < count; ++j)
      m_widgetMap.insert(m_widgetCount++, qMakePair(key, j));
  }

  // Generate dataset model map
  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i)
  {
    const auto key = i.key();
    const auto count = widgetCount(key);
    for (int j = 0; j < count; ++j)
      m_widgetMap.insert(m_widgetCount++, qMakePair(key, j));
  }

  // Traverse all group-level datasets
  for (auto &groupList : m_widgetGroups)
  {
    for (auto &group : groupList)
    {
      for (auto &dataset : group.datasets)
        m_datasetReferences[dataset.uniqueId].append(&dataset);
    }
  }

  // Traverse all widget-level datasets
  for (auto &datasetList : m_widgetDatasets)
  {
    for (auto &dataset : datasetList)
      m_datasetReferences[dataset.uniqueId].append(&dataset);
  }

  // Transverse all datasets
  for (auto &dataset : m_datasets)
    m_datasetReferences[dataset.uniqueId].append(&dataset);

  // For edge cases, register any dataset that has not been added
  for (auto &group : m_lastFrame.groups)
  {
    for (auto &dataset : group.datasets)
    {
      bool registered = false;
      auto &list = m_datasetReferences[dataset.uniqueId];
      for (auto &d : list)
      {
        if (d == &dataset)
        {
          registered = true;
          break;
        }
      }

      if (!registered)
        m_datasetReferences[dataset.uniqueId].append(&dataset);
    }
  }

  // Initialize data series & update actions
  updateDataSeries();
  configureActions(frame);

  // Update user interface
  Q_EMIT widgetCountChanged();
}

//------------------------------------------------------------------------------
// Time series data processing
//------------------------------------------------------------------------------

/**
 * @brief Updates time-series data for all dashboard widgets that require
 * historical tracking.
 *
 * This method handles real-time updating of internal data buffers for all
 * widgets that visualize ordered or continuous data over time, including:
 * - FFT plots
 * - Linear (2D) plots
 * - Multi-series (grouped) plots
 * - GPS trajectory widgets (lat/lon/alt history)
 * - 3D trajectory plots (X/Y/Z vectors) [Pro only]
 *
 * For each type, the function:
 * - Checks if the internal buffer count matches the current number of widgets.
 *   If not, the corresponding `configure*Series()` method is called to
 *   allocate/initialize memory.
 * - Shifts in the latest sample from the dashboard dataset into the correct
 *   slot of the buffer.
 *
 * @warning GPS and 3D plots rely on structured dataset groups and expect the
 *          widgets to provide fields like [`lat`, `lon`, `alt`], or
 *          [`x`, `y`, `z`].
 */
void UI::Dashboard::updateDataSeries()
{
  // Cache widget counts
  const int gpsCount = widgetCount(SerialStudio::DashboardGPS);
  const int fftCount = widgetCount(SerialStudio::DashboardFFT);
  const int plotCount = widgetCount(SerialStudio::DashboardPlot);
  const int multiCount = widgetCount(SerialStudio::DashboardMultiPlot);
#ifdef BUILD_COMMERCIAL
  const int plot3DCount = widgetCount(SerialStudio::DashboardPlot3D);
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
#endif

  // Update GPS data
  for (int i = 0; i < gpsCount; ++i)
  {
    const auto &group = getGroupWidget(SerialStudio::DashboardGPS, i);
    auto &series = m_gpsValues[i];

    double lat = -1, lon = -1, alt = -1;
    for (const auto &dataset : group.datasets)
    {
      const QString &id = dataset.widget;
      if (id == "lat")
        lat = dataset.numericValue;
      else if (id == "lon")
        lon = dataset.numericValue;
      else if (id == "alt")
        alt = dataset.numericValue;
    }

    series.latitudes.push(lat);
    series.longitudes.push(lon);
    series.altitudes.push(alt);
  }

  // Update FFT plots
  for (int i = 0; i < fftCount; ++i)
  {
    if (!m_activeFFTPlots[i])
      continue;

    const auto &dataset = getDatasetWidget(SerialStudio::DashboardFFT, i);
    m_fftValues[i].push(dataset.numericValue);
  }

  // Append latest values to linear plots data
  QSet<int> xAxesMoved;
  QSet<int> yAxesMoved;
  for (int i = 0; i < plotCount; ++i)
  {
    // Stop if plot widget is not enabled
    if (!m_activePlots[i])
      continue;

    // Shift Y-axis points
    const auto &yDataset = getDatasetWidget(SerialStudio::DashboardPlot, i);
    if (!yAxesMoved.contains(yDataset.index))
    {
      yAxesMoved.insert(yDataset.index);
      m_yAxisData[yDataset.index].push(yDataset.numericValue);
    }

    // Shift X-axis points
    auto xAxisId = SerialStudio::activated() ? yDataset.xAxisId : 0;
    if (m_datasets.contains(xAxisId) && !xAxesMoved.contains(xAxisId))
    {
      xAxesMoved.insert(xAxisId);
      const auto &xDataset = m_datasets[xAxisId];
      m_xAxisData[xAxisId].push(xDataset.numericValue);
    }
  }

  // Update Multi-plots
  for (int i = 0; i < multiCount; ++i)
  {
    if (!m_activeMultiplots[i])
      continue;

    const auto &group = getGroupWidget(SerialStudio::DashboardMultiPlot, i);
    auto &multiSeries = m_multipltValues[i];
    for (size_t j = 0; j < group.datasets.size(); ++j)
      multiSeries.y[j].push(group.datasets[j].numericValue);
  }

  // Update 3D plots
#ifdef BUILD_COMMERCIAL
  for (int i = 0; i < plot3DCount; ++i)
  {
    auto &plotData = m_plotData3D[i];

    QVector3D point;
    const auto &group = getGroupWidget(SerialStudio::DashboardPlot3D, i);
    for (const auto &dataset : group.datasets)
    {
      const QString &id = dataset.widget;
      if (id == "x" || id == "X")
        point.setX(dataset.numericValue);
      else if (id == "y" || id == "Y")
        point.setY(dataset.numericValue);
      else if (id == "z" || id == "Z")
        point.setZ(dataset.numericValue);
    }

    plotData.push_back(point);
    const size_t maxPoints = static_cast<size_t>(points());
    if (plotData.size() > maxPoints)
      plotData.erase(plotData.begin(), plotData.end() - maxPoints);
  }
#endif
}

/**
 * @brief Initializes the GPS series structure for all GPS widgets.
 *
 * This method prepares internal storage for GPS trajectory data by:
 * - Clearing any existing series
 * - Allocating new `GpsSeries` entries for each GPS widget in the dashboard
 * - Pre-sizing the latitude, longitude, and altitude arrays with default values
 *
 * Each vector is resized to hold `points() + 1` samples, where `points()`
 * represents the maximum number of time-series points tracked. The default
 * value of -1 is used to represent invalid or uninitialized data.
 *
 * Called once during setup or reset of the dashboard's GPS data series.
 */
void UI::Dashboard::configureGpsSeries()
{
  // Clear memory
  m_gpsValues.clear();
  m_gpsValues.squeeze();

  // Construct GPS data structure
  for (int i = 0; i < widgetCount(SerialStudio::DashboardGPS); ++i)
  {
    DSP::GpsSeries series;
    const auto &group = getGroupWidget(SerialStudio::DashboardGPS, i);
    const QMap<QString, DSP::FixedQueue<double> *> fieldMap
        = {{"lat", &series.latitudes},
           {"lon", &series.longitudes},
           {"alt", &series.altitudes}};

    for (size_t j = 0; j < group.datasets.size(); ++j)
    {
      const auto &dataset = group.datasets[j];
      if (fieldMap.contains(dataset.widget))
      {
        auto *vector = fieldMap[dataset.widget];
        vector->resize(points() + 1);
        vector->fill(std::nan(""));
      }
    }

    m_gpsValues.append(series);
  }
}

/**
 * @brief Configures the FFT series data structure for the dashboard.
 *
 * This function clears existing FFT values and initializes the data structure
 * for each FFT plot widget with a predefined number of samples, filling it with
 * zeros.
 *
 * @note Typically called during dashboard setup or reset to prepare FFT plot
 *       widgets for rendering.
 */
void UI::Dashboard::configureFftSeries()
{
  // Clear memory
  m_fftValues.clear();
  m_fftValues.squeeze();
  m_activeFFTPlots.clear();

  // Construct FFT plot data structure
  for (int i = 0; i < widgetCount(SerialStudio::DashboardFFT); ++i)
  {
    const auto &dataset = getDatasetWidget(SerialStudio::DashboardFFT, i);
    m_fftValues.append(DSP::AxisData(dataset.fftSamples));
    m_activeFFTPlots.insert(i, true);
  }
}

/**
 * @brief Configures the line series data structure for the dashboard.
 *
 * This function clears and reinitializes the X-axis and Y-axis data arrays,
 * as well as the plot values structure (`m_pltValues`). It associates each
 * dataset with its respective X and Y data, creating `LineSeries` objects
 * for plotting.
 *
 * - If a dataset specifies an X-axis source, the corresponding data is used.
 * - Otherwise, the default X-axis (based on sample points) is used.
 *
 * @note Typically called during dashboard setup or reset to prepare plot
 *       widgets for rendering.
 */
void UI::Dashboard::configureLineSeries()
{
  // Clear memory
  m_xAxisData.clear();
  m_yAxisData.clear();
  m_pltValues.clear();
  m_pltValues.squeeze();
  m_activePlots.clear();

  // Reset default X-axis data
  m_pltXAxis = DSP::AxisData(points() + 1);
  m_pltXAxis.fillRange(0, 1);

  // Construct X/Y axis data arrays
  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i)
  {
    // Obtain list of datasets for a widget type
    const auto &datasets = i.value();

    // Iterate over all the datasets
    for (auto d = datasets.begin(); d != datasets.end(); ++d)
    {
      if (d->plt)
      {
        // Register Y-axis
        DSP::AxisData yAxis(points() + 1);
        m_yAxisData.insert(d->index, yAxis);
        m_yAxisData[d->index].fill(0);

        // Register X-axis
        if (SerialStudio::activated())
        {
          int xSource = d->xAxisId;
          if (!m_xAxisData.contains(xSource))
          {
            DSP::AxisData xAxis(points() + 1);
            if (m_datasets.contains(xSource))
            {
              m_xAxisData.insert(xSource, xAxis);
              m_xAxisData[xSource].fill(0);
            }
          }
        }
      }
    }
  }

  // Construct plot values structure
  for (int i = 0; i < widgetCount(SerialStudio::DashboardPlot); ++i)
  {
    // Obtain Y-axis data
    const auto &yDataset = getDatasetWidget(SerialStudio::DashboardPlot, i);

    // Add X-axis data & generate a line series with X/Y data
    if (m_datasets.contains(yDataset.xAxisId) && SerialStudio::activated())
    {
      const auto &xDataset = m_datasets[yDataset.xAxisId];
      DSP::LineSeries series;
      series.x = &m_xAxisData[xDataset.index];
      series.y = &m_yAxisData[yDataset.index];
      m_pltValues.append(series);
    }

    // Only use Y-axis data, use samples/points as X-axis
    else
    {
      DSP::LineSeries series;
      series.x = &m_pltXAxis;
      series.y = &m_yAxisData[yDataset.index];
      m_pltValues.append(series);
    }

    // Enable real-time updates for the plot
    m_activePlots.insert(i, true);
  }
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Initializes internal data structures for 3D trajectory plot widgets.
 *
 * This method ensures that the internal storage (`m_plotData3D`) is correctly
 * resized and cleared to match the current number of 3D plot widgets in the
 * dashboard. Each entry in the list corresponds to a widget and holds a
 * time-ordered list of 3D points (`QVector3D`) representing the X, Y, and
 * Z axes.
 *
 * @note This function is typically called when the number of widgets changes or
 *       during dashboard reinitialization to prevent buffer overflows or stale
 *       data.
 */
void UI::Dashboard::configurePlot3DSeries()
{
  m_plotData3D.clear();
  m_plotData3D.squeeze();
  m_plotData3D.resize(widgetCount(SerialStudio::DashboardPlot3D));
  for (int i = 0; i < m_plotData3D.count(); ++i)
  {
    m_plotData3D[i].clear();
    m_plotData3D[i].shrink_to_fit();
  }
}
#endif

/**
 * @brief Configures the multi-line series data structure for the dashboard.
 *
 * This function initializes the data structure used for multi-plot widgets.
 * It assigns the default X-axis to all multi-line series and creates a
 * `AxisData` vector for each dataset in the group, initializing it with zeros.
 *
 * @note Typically called during dashboard setup or reset to prepare multi-plot
 *       widgets for rendering.
 */
void UI::Dashboard::configureMultiLineSeries()
{
  // Clear data
  m_multipltValues.clear();
  m_multipltValues.squeeze();
  m_activeMultiplots.clear();

  // Reset default X-axis data
  m_multipltXAxis = DSP::AxisData(points() + 1);
  m_multipltXAxis.fillRange(0, 1);

  // Construct multi-plot values structure
  for (int i = 0; i < widgetCount(SerialStudio::DashboardMultiPlot); ++i)
  {
    const auto &group = getGroupWidget(SerialStudio::DashboardMultiPlot, i);

    DSP::MultiLineSeries series;
    series.x = &m_multipltXAxis;
    for (size_t j = 0; j < group.datasets.size(); ++j)
    {
      series.y.push_back(DSP::AxisData(points() + 1));
      series.y.back().fill(0);
    }

    m_multipltValues.append(series);
    m_activeMultiplots.insert(i, true);
  }
}

//------------------------------------------------------------------------------
// Action configuration
//------------------------------------------------------------------------------

/**
 * @brief Configures dashboard actions and associated timers from the
 *        given JSON frame.
 *
 * This method clears existing actions and timers, then loads a new set of
 * actions from the provided JSON frame. For each action, it sets up an optional
 * timer based on its configured TimerMode and interval.
 *
 * Timers are connected to trigger the corresponding action via
 * `activateAction()`, and are automatically started if the action is configured
 * with either:
 * - TimerMode::AutoStart
 * - autoExecuteOnConnect() flag
 *
 * @param frame The JSON frame containing the user-defined actions to configure.
 *
 * @note This method has no effect if:
 * - The frame is invalid (`!frame.isValid()`).
 * - There is no active device connection
 *
 * @warning If a timer-based action has an interval of 0 milliseconds, a warning
 *          is logged and the timer is not created.
 */
void UI::Dashboard::configureActions(const JSON::Frame &frame)
{
  // Stop if frame is not valid
  if (frame.groups.size() <= 0)
    return;

  // Delete actions
  m_actions.clear();
  m_actions.squeeze();

  // Stop and delete all timers
  for (auto it = m_timers.begin(); it != m_timers.end(); ++it)
  {
    if (it.value())
    {
      disconnect(it.value());
      it.value()->stop();
      it.value()->deleteLater();
    }
  }

  // Clear timer map
  m_timers.clear();

  // Update actions
  for (const auto &action : frame.actions)
    m_actions.append(action);

  // Configure timers
  if (IO::Manager::instance().isConnected())
  {
    for (int i = 0; i < m_actions.count(); ++i)
    {
      const auto &action = m_actions[i];
      if (action.timerMode != JSON::TimerMode::Off)
      {
        auto interval = action.timerIntervalMs;
        if (interval > 0)
        {
          auto *timer = new QTimer(this);
          timer->setInterval(interval);
          timer->setTimerType(Qt::PreciseTimer);
          connect(timer, &QTimer::timeout, this,
                  [this, i]() { activateAction(i, false); });

          if (action.timerMode == JSON::TimerMode::AutoStart
              || action.autoExecuteOnConnect)
            timer->start();

          m_timers.insert(i, timer);
        }

        else
        {
          qWarning() << "Interval for action" << action.title
                     << "must be greater than 0!";
        }
      }
    }
  }

  // Update actions
  Q_EMIT actionStatusChanged();
}
