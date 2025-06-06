/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "UI/Dashboard.h"

#include "SIMD/SIMD.h"
#include "IO/Manager.h"
#include "IO/Console.h"
#include "CSV/Player.h"
#include "Misc/TimerEvents.h"
#include "JSON/FrameBuilder.h"

#ifdef USE_QT_COMMERCIAL
#  include "MQTT/Client.h"
#endif

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
  , m_precision(2)
  , m_widgetCount(0)
  , m_updateRequired(false)
  , m_terminalEnabled(false)
{
  // clang-format off
  connect(&CSV::Player::instance(), &CSV::Player::openChanged, this, [=] { resetData(true); });
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this, [=] { resetData(true); });
  connect(&JSON::FrameBuilder::instance(), &JSON::FrameBuilder::jsonFileMapChanged, this, [=] { resetData(); });
  connect(&JSON::FrameBuilder::instance(), &JSON::FrameBuilder::frameChanged, this, &UI::Dashboard::processFrame);
  // clang-format on

  // Reset dashboard data if MQTT client is subscribed
#ifdef USE_QT_COMMERCIAL
  connect(
      &MQTT::Client::instance(), &MQTT::Client::connectedChanged, this, [=] {
        const bool subscribed = MQTT::Client::instance().isSubscriber();
        const bool wasSubscribed = !MQTT::Client::instance().isConnected()
                                   && MQTT::Client::instance().isSubscriber();

        if (subscribed || wasSubscribed)
          resetData(true);
      });
#endif

  // Update the dashboard widgets at 24 Hz
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout24Hz, this,
          [=] {
            if (m_updateRequired)
            {
              m_updateRequired = false;
              Q_EMIT updated();
            }
          });
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
 * @return qreal The computed interval size for the given range.
 *
 * @note Common intervals (e.g., 0.1, 0.2, 0.5, 1, 2, 5, 10) are selected
 *       to enhance readability. Ensures the interval divides the range evenly.
 */
qreal UI::Dashboard::smartInterval(const qreal min, const qreal max,
                                   const qreal multiplier)
{
  // Calculate an initial step size
  const auto range = qAbs(max - min);
  const auto digits = static_cast<int>(std::ceil(std::log10(range)));
  const qreal r = std::pow(10.0, -digits) * 10;
  const qreal v = std::ceil(range * r) / r;
  qreal step = qMax(0.0001, v * multiplier);

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
    const qreal factor = std::pow(10.0, std::floor(std::log10(step)));
    const qreal normalizedStep = step / factor;

    if (normalizedStep <= 1.0)
      step = factor;
    else if (normalizedStep <= 2.0)
      step = 2 * factor;
    else if (normalizedStep <= 5.0)
      step = 5 * factor;
    else
      step = 10 * factor;
  }

  // Recompute step to ensure it divides the range cleanly
  if (std::fmod(range, step) != 0.0)
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
 * @brief Checks if the dashboard is currently available, determined by the
 *        data stream sources.
 *
 * @return True if at least one data source/stream is active.
 */
bool UI::Dashboard::streamAvailable() const
{
  bool available = IO::Manager::instance().isConnected();
  available |= CSV::Player::instance().isOpen();

#ifdef USE_QT_COMMERCIAL
  available |= MQTT::Client::instance().isConnected()
               && MQTT::Client::instance().isSubscriber();
#endif

  return available;
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
 * @brief Determines if the point-selector widget should be visible based on the
 *        presence of relevant widget groups or datasets.
 *
 * @return True if points widget visibility is enabled; false otherwise.
 */
bool UI::Dashboard::pointsWidgetVisible() const
{
#ifdef USE_QT_COMMERCIAL
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
  return m_rawFrame.containsCommercialFeatures();
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
 * @brief Gets the number of decimal points for the dashboard widgets.
 * @return Current precision level.
 */
int UI::Dashboard::precision() const
{
  return m_precision;
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
  return m_lastFrame.isValid();
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
  if (m_widgetMap.contains(widgetIndex))
    return m_widgetMap.value(widgetIndex).second;

  return -1;
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
  if (m_widgetMap.contains(widgetIndex))
    return m_widgetMap.value(widgetIndex).first;

  return SerialStudio::DashboardNoWidget;
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
    return m_widgetGroups.value(widget).count();

  else if (SerialStudio::isDatasetWidget(widget))
    return m_widgetDatasets.value(widget).count();

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
  return m_lastFrame.title();
}

/**
 * @brief Retrieves the icons for each action available on the dashboard.
 * @return A list of icons corresponding to each action.
 */
QVariantList UI::Dashboard::actions() const
{
  QVariantList actions;
  for (int i = 0; i < m_actions.count(); ++i)
  {
    const auto &action = m_actions[i];

    QVariantMap m;
    m["id"] = i;
    m["text"] = action.title();
    m["icon"] = QStringLiteral("qrc:/rcc/actions/%1.svg").arg(action.icon());
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
 * @brief Provides access to a specific group widget based on widget type and
 *        relative index.
 *
 * @param widget The type of widget requested.
 * @param index The index of the widget within its type.
 * @return A reference to the JSON::Group representing the specified widget.
 * @throws An assertion failure if the index is out of bounds.
 */
const JSON::Group &
UI::Dashboard::getGroupWidget(const SerialStudio::DashboardWidget widget,
                              const int index) const
{
  Q_ASSERT(index >= 0 && index < m_widgetGroups[widget].count());
  return m_widgetGroups[widget].at(index);
}

/**
 * @brief Provides access to a specific dataset widget based on widget type and
 *        relative index.
 *
 * @param widget The type of widget requested.
 * @param index The index of the widget within its type.
 * @return A reference to the JSON::Dataset representing the specified widget.
 * @throws An assertion failure if the index is out of bounds.
 */
const JSON::Dataset &
UI::Dashboard::getDatasetWidget(const SerialStudio::DashboardWidget widget,
                                const int index) const
{
  Q_ASSERT(index >= 0 && index < m_widgetDatasets[widget].count());
  return m_widgetDatasets[widget].at(index);
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
// Plot data access
//------------------------------------------------------------------------------

/**
 * @brief Provides the FFT plot values currently displayed on the dashboard.
 * @return A reference to a QVector containing the FFT PlotDataY data.
 */
const PlotDataY &UI::Dashboard::fftData(const int index) const
{
  return m_fftValues[index];
}

/**
 * @brief Provides the linear plot values currently displayed on the dashboard.
 * @return A reference to a QVector containing the linear PlotDataY data.
 */
const LineSeries &UI::Dashboard::plotData(const int index) const
{
  return m_pltValues[index];
}

/**
 * @brief Provides the values for multiplot visuals on the dashboard.
 * @return A reference to a QVector containing MultiPlotDataY data.
 */
const MultiLineSeries &UI::Dashboard::multiplotData(const int index) const
{
  return m_multipltValues[index];
}

#ifdef USE_QT_COMMERCIAL
/**
 * @brief Provides the values for 3D plot visuals on the dashboard.
 * @return A reference to a QVector containing ThreeDimensionalSeries data.
 */
const PlotData3D &UI::Dashboard::plotData3D(const int index) const
{
  return m_plotData3D[index];
}
#endif

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
 * @brief Activates an action by sending its associated data via the IO Manager.
 * @param index The index of the action to activate.
 * @throws An assertion failure if the index is out of bounds.
 */
void UI::Dashboard::activateAction(const int index)
{
  if (index >= 0 && index < m_actions.count())
  {
    const auto &action = m_actions[index];

    QByteArray bin;
    if (action.binaryData())
      bin = IO::Console::hexToBytes(action.txData());
    else
      bin = QString(action.txData() + action.eolSequence()).toUtf8();

    IO::Manager::instance().writeData(bin);
  }
}

/**
 * @brief Sets the precision level for the dashboard, if changed, and emits
 *        the @c precisionChanged signal to update the UI.
 *
 * @param precision The new precision level.
 */
void UI::Dashboard::setPrecision(const int precision)
{
  if (m_precision != precision)
  {
    m_precision = precision;
    Q_EMIT precisionChanged();
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
  m_plotData3D.clear();
  m_multipltValues.clear();

  // Free memory associated with the containers of the plotting data
  m_fftValues.squeeze();
  m_pltValues.squeeze();
  m_plotData3D.squeeze();
  m_multipltValues.squeeze();

  // Clear X/Y axis arrays
  m_xAxisData.clear();
  m_yAxisData.clear();

  // Clear widget & action structures
  m_widgetCount = 0;
  m_actions.clear();
  m_actions.squeeze();
  m_widgetMap.clear();
  m_widgetGroups.clear();
  m_widgetDatasets.clear();
  m_datasetReferences.clear();

  // Reset frame data
  m_rawFrame = JSON::Frame();
  m_lastFrame = JSON::Frame();

  // Notify user interface
  if (notify)
  {
    m_updateRequired = true;

    Q_EMIT updated();
    Q_EMIT dataReset();
    Q_EMIT actionCountChanged();
    Q_EMIT widgetCountChanged();
    Q_EMIT containsCommercialFeaturesChanged();
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
    resetData(false);
  }

  Q_EMIT terminalEnabledChanged();
}

//------------------------------------------------------------------------------
// Frame processing & dashboard model generation
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
void UI::Dashboard::processFrame(const JSON::Frame &frame)
{
  // Validate frame
  if (!frame.isValid() || !streamAvailable())
    return;

  // Lock access to frame data
  QWriteLocker locker(&m_dataLock);

  // Regenerate dashboard model if frame structure changed
  if (!frame.equalsStructure(m_rawFrame))
  {
    const bool hadProFeatures = m_rawFrame.containsCommercialFeatures();
    reconfigureDashboard(frame);
    if (hadProFeatures != frame.containsCommercialFeatures())
      Q_EMIT containsCommercialFeaturesChanged();
  }

  // Update dashboard data
  updateDashboardData(frame);

  // Set dashboard update flag
  m_updateRequired = true;
}

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
  // Iterate over all datasets in the incoming frame
  for (const auto &group : frame.groups())
  {
    for (const auto &dataset : group.datasets())
    {
      // Get the dataset UID
      auto uid = dataset.uniqueId();

      // UID not registered -> frame structure changed
      if (!m_datasetReferences.contains(uid))
      {
        resetData(false);
        processFrame(frame);
        return;
      }

      // Update internal references to the **exact** same dataset
      for (auto *ptr : std::as_const(m_datasetReferences[uid]))
        ptr->setValue(dataset.value());
    }
  }

  // Update plot memory structures
  updatePlots();
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
    terminal.m_groupId = m_lastFrame.groupCount();
    terminal.m_widget = "terminal";
    terminal.m_title = tr("Console");
    m_lastFrame.m_groups.append(terminal);
  }

  // Parse frame groups
  for (const auto &group : m_lastFrame.groups())
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
      copy.m_title = tr("%1 (Fallback)").arg(group.title());
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(copy);
      for (int i = 0; i < m_lastFrame.groupCount(); ++i)
      {
        if (m_lastFrame.groups()[i].groupId() == group.groupId())
        {
          m_lastFrame.m_groups[i].m_widget = "multiplot";
          m_lastFrame.m_groups[i].m_title = copy.m_title;
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
    for (const auto &dataset : group.datasets())
    {
      // Register a new dataset
      if (!m_datasets.contains(dataset.index()))
        m_datasets.insert(dataset.index(), dataset);

      // Dataset already registered, update min/max values
      else
      {
        auto prev = m_datasets.value(dataset.index());
        double newMin = qMin(prev.min(), dataset.min());
        double newMax = qMax(prev.max(), dataset.max());

        auto d = dataset;
        d.setMin(newMin);
        d.setMax(newMax);
        m_datasets.insert(dataset.index(), d);
      }

      // Register dataset widgets
      auto keys = SerialStudio::getDashboardWidgets(dataset);
      for (const auto &widgetKeys : std::as_const(keys))
      {
        if (widgetKeys == SerialStudio::DashboardLED)
          ledPanel.m_datasets.append(dataset);

        else if (widgetKeys != SerialStudio::DashboardNoWidget)
          m_widgetDatasets[widgetKeys].append(dataset);
      }
    }

    // Add group-level LED panel
    if (ledPanel.datasetCount() > 0)
    {
      ledPanel.m_groupId = group.groupId();
      ledPanel.m_title = tr("LED Panel (%1)").arg(group.title());
      ledPanel.m_widget = "led-panel";
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
      for (auto &dataset : group.m_datasets)
      {
        const quint32 uid = dataset.uniqueId();
        m_datasetReferences[uid].append(&dataset);
      }
    }
  }

  // Traverse all widget-level datasets
  for (auto &datasetList : m_widgetDatasets)
  {
    for (auto &dataset : datasetList)
    {
      const quint32 uid = dataset.uniqueId();
      m_datasetReferences[uid].append(&dataset);
    }
  }

  // Transverse all plot datasets
  for (auto &dataset : m_datasets)
  {
    const quint32 uid = dataset.uniqueId();
    m_datasetReferences[uid].append(&dataset);
  }

  // Update actions
  m_actions = frame.actions();
  Q_EMIT actionCountChanged();

  // Update user interface
  Q_EMIT widgetCountChanged();
}

//------------------------------------------------------------------------------
// Plot data processing
//------------------------------------------------------------------------------

/**
 * @brief Updates the plot data for all dashboard widgets.
 *
 * This function ensures that the data structures for FFT plots, linear plots,
 * and multiplots are correctly initialized and updated with the latest values
 * from the datasets. It handles reinitialization if the widget count changes
 * and shifts data to accommodate new samples.
 *
 * @note This function is typically called in real-time to keep plots
 *       synchronized with incoming data.
 */
void UI::Dashboard::updatePlots()
{
  // Check if we need to re-initialize FFT plots data
  if (m_fftValues.count() != widgetCount(SerialStudio::DashboardFFT))
    configureFftSeries();

  // Check if we need to re-initialize linear plots data
  if (m_pltValues.count() != widgetCount(SerialStudio::DashboardPlot))
    configureLineSeries();

  // Check if we need to re-initialize multiplot data
  if (m_multipltValues.count() != widgetCount(SerialStudio::DashboardMultiPlot))
    configureMultiLineSeries();

  // Check if we need to re-initialize 3D plot data
#ifdef USE_QT_COMMERCIAL
  if (m_plotData3D.count() != widgetCount(SerialStudio::DashboardPlot3D))
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

  // Append latest values to FFT plots data
  for (int i = 0; i < widgetCount(SerialStudio::DashboardFFT); ++i)
  {
    const auto &dataset = getDatasetWidget(SerialStudio::DashboardFFT, i);
    auto *data = m_fftValues[i].data();
    auto count = m_fftValues[i].size();
    SIMD::shift<qreal>(data, count, dataset.value().toDouble());
  }

  // Append latest values to linear plots data
  QSet<int> xAxesMoved;
  QSet<int> yAxesMoved;
  for (int i = 0; i < widgetCount(SerialStudio::DashboardPlot); ++i)
  {
    // Shift Y-axis points
    const auto &yDataset = getDatasetWidget(SerialStudio::DashboardPlot, i);
    if (!yAxesMoved.contains(yDataset.index()))
    {
      yAxesMoved.insert(yDataset.index());
      auto *yData = m_yAxisData[yDataset.index()].data();
      auto yCount = m_yAxisData[yDataset.index()].size();
      SIMD::shift<qreal>(yData, yCount, yDataset.value().toDouble());
    }

    // Shift X-axis points
    auto xAxisId = SerialStudio::activated() ? yDataset.xAxisId() : 0;
    if (m_datasets.contains(xAxisId) && !xAxesMoved.contains(xAxisId))
    {
      xAxesMoved.insert(xAxisId);
      const auto &xDataset = m_datasets[xAxisId];
      auto *xData = m_xAxisData[xAxisId].data();
      auto xCount = m_xAxisData[xAxisId].size();
      SIMD::shift<qreal>(xData, xCount, xDataset.value().toDouble());
    }
  }

  // Append latest values to multiplots data
  for (int i = 0; i < widgetCount(SerialStudio::DashboardMultiPlot); ++i)
  {
    const auto &group = getGroupWidget(SerialStudio::DashboardMultiPlot, i);
    for (int j = 0; j < group.datasetCount(); ++j)
    {
      const auto &dataset = group.datasets()[j];
      auto *data = m_multipltValues[i].y[j].data();
      auto count = m_multipltValues[i].y[j].size();
      SIMD::shift<qreal>(data, count, dataset.value().toDouble());
    }
  }

// Append latest values to 3D plots
#ifdef USE_QT_COMMERCIAL
  for (int i = 0; i < widgetCount(SerialStudio::DashboardPlot3D); ++i)
  {
    // Get pointer to vector with 3D points for current widget
    auto &plotData = m_plotData3D[i];

    // Initialize new point
    QVector3D point;
    const auto &group = getGroupWidget(SerialStudio::DashboardPlot3D, i);
    for (int j = 0; j < group.datasetCount(); ++j)
    {
      const auto &dataset = group.datasets()[j];
      if (dataset.widget().toLower() == "x")
        point.setX(dataset.value().toDouble());
      else if (dataset.widget().toLower() == "y")
        point.setY(dataset.value().toDouble());
      else if (dataset.widget().toLower() == "z")
        point.setZ(dataset.value().toDouble());
    }

    // Add point to data
    plotData.push_back(point);
    if (static_cast<int>(plotData.size()) > points())
      plotData.erase(plotData.begin(), plotData.end() - points());
  }
#endif
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

  // Construct FFT plot data structure
  for (int i = 0; i < widgetCount(SerialStudio::DashboardFFT); ++i)
  {
    const auto &dataset = getDatasetWidget(SerialStudio::DashboardFFT, i);
    m_fftValues.append(PlotDataY());
    m_fftValues.last().resize(dataset.fftSamples());
    SIMD::fill<qreal>(m_fftValues.last().data(), dataset.fftSamples(), 0);
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

  // Reset default X-axis data
  m_pltXAxis.clear();
  m_pltXAxis.shrink_to_fit();
  m_pltXAxis.resize(points() + 1);
  SIMD::fill_range<qreal>(m_pltXAxis.data(), m_pltXAxis.size(), 0);

  // Construct X/Y axis data arrays
  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i)
  {
    // Obtain list of datasets for a widget type
    const auto &datasets = i.value();

    // Iterate over all the datasets
    for (auto d = datasets.begin(); d != datasets.end(); ++d)
    {
      if (d->graph())
      {
        // Register X-axis
        PlotDataY yAxis;
        m_yAxisData.insert(d->index(), yAxis);

        // Register X-axis
        if (SerialStudio::activated())
        {
          int xSource = d->xAxisId();
          if (!m_xAxisData.contains(xSource))
          {
            PlotDataX xAxis;
            if (m_datasets.contains(xSource))
              m_xAxisData.insert(xSource, xAxis);
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
    if (m_datasets.contains(yDataset.xAxisId()) && SerialStudio::activated())
    {
      const auto &xDataset = m_datasets[yDataset.xAxisId()];
      m_xAxisData[xDataset.index()].resize(points() + 1);
      m_yAxisData[yDataset.index()].resize(points() + 1);
      SIMD::fill<qreal>(m_xAxisData[xDataset.index()].data(), points() + 1, 0);
      SIMD::fill<qreal>(m_yAxisData[yDataset.index()].data(), points() + 1, 0);

      LineSeries series;
      series.x = &m_xAxisData[xDataset.index()];
      series.y = &m_yAxisData[yDataset.index()];
      m_pltValues.append(series);
    }

    // Only use Y-axis data, use samples/points as X-axis
    else
    {
      m_yAxisData[yDataset.index()].resize(points() + 1);
      SIMD::fill<qreal>(m_yAxisData[yDataset.index()].data(), points() + 1, 0);

      LineSeries series;
      series.x = &m_pltXAxis;
      series.y = &m_yAxisData[yDataset.index()];
      m_pltValues.append(series);
    }
  }
}

/**
 * @brief Configures the multi-line series data structure for the dashboard.
 *
 * This function initializes the data structure used for multi-plot widgets.
 * It assigns the default X-axis to all multi-line series and creates a
 * `PlotDataY` vector for each dataset in the group, initializing it with zeros.
 *
 * @note Typically called during dashboard setup or reset to prepare multi-plot
 *       widgets for rendering.
 */
void UI::Dashboard::configureMultiLineSeries()
{
  // Clear data
  m_multipltValues.clear();
  m_multipltValues.squeeze();

  // Reset default X-axis data
  m_multipltXAxis.clear();
  m_multipltXAxis.shrink_to_fit();
  m_multipltXAxis.resize(points() + 1);
  SIMD::fill_range<qreal>(m_multipltXAxis.data(), m_multipltXAxis.size(), 0);

  // Construct multi-plot values structure
  for (int i = 0; i < widgetCount(SerialStudio::DashboardMultiPlot); ++i)
  {
    const auto &group = getGroupWidget(SerialStudio::DashboardMultiPlot, i);

    MultiLineSeries series;
    series.x = &m_multipltXAxis;
    for (int j = 0; j < group.datasetCount(); ++j)
    {
      series.y.push_back(PlotDataY());
      series.y.back().resize(points() + 1);
      SIMD::fill<qreal>(series.y.back().data(), points() + 1, 0);
    }

    m_multipltValues.append(series);
  }
}
