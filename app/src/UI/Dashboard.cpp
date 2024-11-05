/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "IO/Manager.h"
#include "CSV/Player.h"
#include "UI/Dashboard.h"
#include "MQTT/Client.h"
#include "Misc/TimerEvents.h"
#include "Misc/ThemeManager.h"
#include "JSON/FrameBuilder.h"

/**
 * @brief Constructs the Dashboard object and establishes connections for
 *        various signal sources that may trigger data reset or frame
 *        processing.
 */
UI::Dashboard::Dashboard()
{
  // clang-format off
  connect(&CSV::Player::instance(), &CSV::Player::openChanged, this, [=] { resetData(); });
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this, [=] { resetData(); });
  connect(&MQTT::Client::instance(), &MQTT::Client::connectedChanged, this, [=] { resetData(); });
  connect(&JSON::FrameBuilder::instance(), &JSON::FrameBuilder::jsonFileMapChanged, this, [=] { resetData(); });
  connect(&JSON::FrameBuilder::instance(), &JSON::FrameBuilder::frameChanged, this, &UI::Dashboard::processFrame);
  // clang-format on

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

/**
 * @brief Checks if the dashboard is currently available, determined by the
 *        total widget count.
 *
 * @return True if the dashboard has widgets; false otherwise.
 */
bool UI::Dashboard::available() const
{
  return totalWidgetCount() > 0;
}

/**
 * @brief Determines if the point-selector widget should be visible based on the
 *        presence of relevant widget groups or datasets.
 *
 * @return True if points widget visibility is enabled; false otherwise.
 */
bool UI::Dashboard::pointsWidgetVisible() const
{
  return m_widgetGroups.contains(WC::DashboardMultiPlot)
         || m_widgetDatasets.contains(WC::DashboardPlot);
}

/**
 * @brief Determines if the decimal-point precision selector widget should be
 *        visible based on the presence of specific widget groups or datasets.
 *
 * @return True if precision widget visibility is enabled; false otherwise.
 */
bool UI::Dashboard::precisionWidgetVisible() const
{
  return m_widgetGroups.contains(WC::DashboardAccelerometer)
         || m_widgetGroups.contains(WC::DashboardGyroscope)
         || m_widgetGroups.contains(WC::DashboardDataGrid)
         || m_widgetDatasets.contains(WC::DashboardBar)
         || m_widgetDatasets.contains(WC::DashboardGauge)
         || m_widgetDatasets.contains(WC::DashboardCompass);
}

/**
 * @brief Determines if axis options should be visible based on the presence of
 *        specific widget groups or datasets that support axis visibility.
 *
 * @return True if axis options widget visibility is enabled; false otherwise.
 */
bool UI::Dashboard::axisOptionsWidgetVisible() const
{
  return m_widgetGroups.contains(WC::DashboardMultiPlot)
         || m_widgetDatasets.contains(WC::DashboardPlot)
         || m_widgetDatasets.contains(WC::DashboardFFT);
}

/**
 * @brief Retrieves the current visibility setting for the dashboard's axes.
 * @return Current AxisVisibility setting.
 */
UI::Dashboard::AxisVisibility UI::Dashboard::axisVisibility() const
{
  return m_axisVisibility;
}

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

/**
 * @brief Checks if the current frame is valid for processing.
 * @return True if the current frame is valid; false otherwise.
 */
bool UI::Dashboard::frameValid() const
{
  return m_currentFrame.isValid();
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
 * @return The widget type, or WC::DashboardNoWidget if the index is not found.
 */
WC::DashboardWidget UI::Dashboard::widgetType(const int widgetIndex)
{
  if (m_widgetMap.contains(widgetIndex))
    return m_widgetMap.value(widgetIndex).first;

  return WC::DashboardNoWidget;
}

/**
 * @brief Counts the number of instances of a specified widget type within the
 *        dashboard.
 *
 * @param widget The type of widget to count.
 * @return The count of instances for the specified widget type.
 */
int UI::Dashboard::widgetCount(const WC::DashboardWidget widget) const
{
  if (WC::isGroupWidget(widget))
    return m_widgetGroups.value(widget).count();

  else if (WC::isDatasetWidget(widget))
    return m_widgetDatasets.value(widget).count();

  return 0;
}

/**
 * @brief Retrieves the titles of widgets of a specific type.
 * @param widget The type of widget whose titles are requested.
 * @return A list of titles associated with the specified widget type.
 */
QStringList UI::Dashboard::widgetTitles(const WC::DashboardWidget widget)
{
  QStringList titles;

  if (WC::isGroupWidget(widget))
    for (const auto &group : m_widgetGroups[widget])
      titles.append(group.title());

  else if (WC::isDatasetWidget(widget))
    for (const auto &dataset : m_widgetDatasets[widget])
      titles.append(dataset.title());

  return titles;
}

/**
 * @brief Retrieves the colors associated with a specific widget type.
 * @param widget The type of widget whose colors are requested.
 * @return A list of color values for the specified widget type.
 */
QStringList UI::Dashboard::widgetColors(const WC::DashboardWidget widget)
{
  QStringList list;

  if (WC::isDatasetWidget(widget) && m_widgetDatasets.contains(widget))
  {
    for (const auto &dataset : m_widgetDatasets[widget])
      list.append(WC::getDatasetColor(dataset.index()));
  }

  if (list.isEmpty())
  {
    const auto colors = Misc::ThemeManager::instance().colors();
    list.append(colors["switch_highlight"].toString());
  }

  return list;
}

/**
 * @brief Checks if a specific widget instance is currently visible.
 * @param widget The type of the widget to check.
 * @param index The index of the widget instance within its type.
 * @return True if the widget instance is visible; false otherwise.
 */
bool UI::Dashboard::widgetVisible(const WC::DashboardWidget widget,
                                  const int index) const
{
  Q_ASSERT(index >= 0 && index < m_widgetVisibility[widget].count());
  return m_widgetVisibility[widget].at(index);
}

/**
 * @brief Retrieves the icons for all available widgets on the dashboard.
 * @return A list of icon representations for each available widget.
 */
const QStringList UI::Dashboard::availableWidgetIcons() const
{
  QStringList list;
  for (auto widget : availableWidgets())
    list.append(WC::dashboardWidgetIcon(widget));

  return list;
}

/**
 * @brief Retrieves the titles of all available widgets on the dashboard.
 * @return A list of titles for each available widget.
 */
const QStringList UI::Dashboard::availableWidgetTitles() const
{
  QStringList list;
  for (auto widget : availableWidgets())
    list.append(WC::dashboardWidgetTitle(widget));

  return list;
}

/**
 * @brief Provides a list of available widget types on the dashboard.
 * @return A list of WC::DashboardWidget items representing each available
 *         widget type.
 */
const QList<WC::DashboardWidget> UI::Dashboard::availableWidgets() const
{
  return m_availableWidgets;
}

/**
 * @brief Retrieves the title of the current frame in the dashboard.
 * @return A reference to a QString containing the current frame title.
 */
const QString &UI::Dashboard::title() const
{
  return m_currentFrame.title();
}

/**
 * @brief Retrieves the icons for each action available on the dashboard.
 * @return A list of icons corresponding to each action.
 */
QStringList UI::Dashboard::actionIcons() const
{
  QStringList icons;
  for (const auto &action : m_actions)
    icons.append(QString("qrc:/rcc/actions/%1.svg").arg(action.icon()));

  return icons;
}

/**
 * @brief Retrieves the titles of each action available on the dashboard.
 * @return A list of titles corresponding to each action.
 */
QStringList UI::Dashboard::actionTitles() const
{
  QStringList titles;
  for (const auto &action : m_actions)
    titles.append(action.title());

  return titles;
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
UI::Dashboard::getGroupWidget(const WC::DashboardWidget widget,
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
UI::Dashboard::getDatasetWidget(const WC::DashboardWidget widget,
                                const int index) const
{
  Q_ASSERT(index >= 0 && index < m_widgetDatasets[widget].count());
  return m_widgetDatasets[widget].at(index);
}

/**
 * @brief Retrieves the current JSON frame for the dashboard.
 * @return A reference to the current JSON::Frame.
 */
const JSON::Frame &UI::Dashboard::currentFrame()
{
  return m_currentFrame;
}

/**
 * @brief Provides the FFT plot values currently displayed on the dashboard.
 * @return A reference to a QVector containing the FFT Curve data.
 */
const QVector<Curve> &UI::Dashboard::fftPlotValues()
{
  return m_fftPlotValues;
}

/**
 * @brief Provides the linear plot values currently displayed on the dashboard.
 * @return A reference to a QVector containing the linear Curve data.
 */
const QVector<Curve> &UI::Dashboard::linearPlotValues()
{
  return m_linearPlotValues;
}

/**
 * @brief Provides the values for multiplot visuals on the dashboard.
 * @return A reference to a QVector containing MultipleCurves data.
 */
const QVector<MultipleCurves> &UI::Dashboard::multiplotValues()
{
  return m_multiplotValues;
}

/**
 * @brief Sets the number of data points displayed in the dashboard plots.
 *        Clears existing multiplot and linear plot values and emits the
 *        @c pointsChanged signal.
 *
 * @param points The new point/sample count.
 */
void UI::Dashboard::setPoints(const int points)
{
  if (m_points != points)
  {
    m_points = points;
    m_multiplotValues.clear();
    m_linearPlotValues.clear();

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
  Q_ASSERT(index >= 0 && index < m_actions.count());

  const auto &action = m_actions[index];
  const auto &data = action.txData() + action.eolSequence();
  IO::Manager::instance().writeData(data.toUtf8());
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
  m_fftPlotValues.clear();
  m_multiplotValues.clear();
  m_linearPlotValues.clear();

  // Clear widget & action structures
  m_widgetCount = 0;
  m_actions.clear();
  m_widgetMap.clear();
  m_widgetGroups.clear();
  m_widgetDatasets.clear();
  m_widgetVisibility.clear();
  m_availableWidgets.clear();

  // Reset frame data
  m_currentFrame = JSON::Frame();

  // Notify user interface
  if (notify)
  {
    Q_EMIT updated();
    Q_EMIT dataReset();
    Q_EMIT actionCountChanged();
    Q_EMIT widgetCountChanged();
    Q_EMIT widgetVisibilityChanged();
  }
}

/**
 * @brief Sets the visibility option for the dashboard's axes and emits the
 *        @c axisVisibilityChanged signal if the option changes.
 *
 * @param option The new axis visibility setting.
 */
void UI::Dashboard::setAxisVisibility(const AxisVisibility option)
{
  if (m_axisVisibility != option)
  {
    m_axisVisibility = option;
    Q_EMIT axisVisibilityChanged();
  }
}

/**
 * @brief Sets the visibility of a specific widget at a given index.
 *        Emits the @c widgetVisibilityChanged signal if the visibility changes.
 *
 * @param widget The type of widget to modify.
 * @param index The index of the widget instance.
 * @param visible True to make the widget visible; false to hide it.
 * @throws An assertion failure if the index is out of bounds.
 */
void UI::Dashboard::setWidgetVisible(const WC::DashboardWidget widget,
                                     const int index, const bool visible)
{
  Q_ASSERT(index >= 0 && m_widgetVisibility[widget].count() > index);

  const auto currentValue = m_widgetVisibility[widget][index];
  if (currentValue != visible)
  {
    m_widgetVisibility[widget][index] = visible;
    Q_EMIT widgetVisibilityChanged();
  }
}

/**
 * @brief Updates plot data for linear, FFT, and multiplot widgets on the
 *        dashboard.
 *
 * This function checks and initializes the data structures for each plot type
 * (linear plots, FFT plots, and multiplots) if needed.
 *
 * It then appends the latest values from the data sources to these plots by
 * shifting older data back and adding new data to the end.
 */
void UI::Dashboard::updatePlots()
{
  // Check if we need to re-initialize linear plots data
  if (m_linearPlotValues.count() != widgetCount(WC::DashboardPlot))
  {
    m_linearPlotValues.clear();
    for (int i = 0; i < widgetCount(WC::DashboardPlot); ++i)
    {
      m_linearPlotValues.append(Curve());
      m_linearPlotValues.last().resize(points() + 1);
      std::fill(m_linearPlotValues.last().begin(),
                m_linearPlotValues.last().end(), 0.0001);
    }
  }

  // Check if we need to re-initialize FFT plots data
  if (m_fftPlotValues.count() != widgetCount(WC::DashboardFFT))
  {
    m_fftPlotValues.clear();
    for (int i = 0; i < widgetCount(WC::DashboardFFT); ++i)
    {
      const auto &dataset = getDatasetWidget(WC::DashboardFFT, i);
      m_fftPlotValues.append(Curve());
      m_fftPlotValues.last().resize(dataset.fftSamples());
      std::fill(m_fftPlotValues.last().begin(), m_fftPlotValues.last().end(),
                0);
    }
  }

  // Check if we need to re-initialize multiplot data
  if (m_multiplotValues.count() != widgetCount(WC::DashboardMultiPlot))
  {
    m_multiplotValues.clear();
    for (int i = 0; i < widgetCount(WC::DashboardMultiPlot); ++i)
    {
      const auto &group = getGroupWidget(WC::DashboardMultiPlot, i);
      m_multiplotValues.append(MultipleCurves());
      m_multiplotValues.last().resize(group.datasetCount());
      for (int j = 0; j < group.datasetCount(); ++j)
      {
        m_multiplotValues[i][j].resize(points() + 1);
        std::fill(m_multiplotValues[i][j].begin(),
                  m_multiplotValues[i][j].end(), 0.0001);
      }
    }
  }

  // Append latest values to linear plots data
  for (int i = 0; i < widgetCount(WC::DashboardPlot); ++i)
  {
    const auto &dataset = getDatasetWidget(WC::DashboardPlot, i);
    auto *data = m_linearPlotValues[i].data();
    auto count = m_linearPlotValues[i].count();
    memmove(data, data + 1, count * sizeof(qreal));
    m_linearPlotValues[i][count - 1] = dataset.value().toDouble();
  }

  // Append latest values to FFT plots data
  for (int i = 0; i < widgetCount(WC::DashboardFFT); ++i)
  {
    const auto &dataset = getDatasetWidget(WC::DashboardFFT, i);
    auto *data = m_fftPlotValues[i].data();
    auto count = m_fftPlotValues[i].count();
    memmove(data, data + 1, count * sizeof(qreal));
    m_fftPlotValues[i][count - 1] = dataset.value().toDouble();
  }

  // Append latest values to multiplots data
  for (int i = 0; i < widgetCount(WC::DashboardMultiPlot); ++i)
  {
    const auto &group = getGroupWidget(WC::DashboardMultiPlot, i);
    for (int j = 0; j < group.datasetCount(); ++j)
    {
      const auto &dataset = group.datasets()[j];
      auto *data = m_multiplotValues[i][j].data();
      auto count = m_multiplotValues[i][j].count();
      memmove(data, data + 1, count * sizeof(qreal));
      m_multiplotValues[i][j][count - 1] = dataset.value().toDouble();
    }
  }
}

/**
 * @brief Processes and updates the dashboard data based on a new frame.
 *
 * This function validates and processes the given `frame`, updating the
 * dashboard’s internal data structures, widgets, and visibility settings.
 *
 * It clears and reconfigures widget groups, datasets, and actions to reflect
 * the new frame's data. The function also emits necessary signals to notify
 * changes and triggers the update of plot data.
 *
 * The function:
 * - Validates the frame and updates widget data structures.
 * - Clears and reinitializes widget groups and datasets.
 * - Updates the list of actions.
 * - Checks for differences in widget counts or titles to determine if a
 *   dashboard regeneration is required.
 * - Configures widget visibility and mappings based on the new frame data.
 * - Emits signals to update the UI with the new widget count and visibility.
 * - Calls `updatePlots()` to ensure plotting data aligns with the new frame.
 * - Notifies the application about the updated frame.
 *
 * @param frame The new JSON::Frame to process for the dashboard.
 */
void UI::Dashboard::processFrame(const JSON::Frame &frame)
{
  // Validate frame
  if (!frame.isValid())
    return;

  // Get previous counts & title
  const auto previousTitle = title();
  const auto previousActionCount = actionCount();
  QMap<WC::DashboardWidget, int> previousCounts;
  for (auto i = m_widgetGroups.begin(); i != m_widgetGroups.end(); ++i)
    previousCounts[i.key()] = widgetCount(i.key());
  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i)
    previousCounts[i.key()] = widgetCount(i.key());

  // Copy frame data & set update required flag to true
  m_currentFrame = frame;
  m_updateRequired = true;

  // Reset widget structures
  m_widgetGroups.clear();
  m_widgetDatasets.clear();

  // Update actions
  m_actions = m_currentFrame.actions();
  if (actionCount() != previousActionCount)
    Q_EMIT actionCountChanged();

  // Update widget data structures
  JSON::Group ledPanel;
  for (const auto &group : frame.groups())
  {
    const auto key = WC::getDashboardWidget(group);
    if (key != WC::DashboardNoWidget)
      m_widgetGroups[key].append(group);

    if (key == WC::DashboardAccelerometer || key == WC::DashboardGyroscope)
      m_widgetGroups[WC::DashboardMultiPlot].append(group);

    for (const auto &dataset : group.datasets())
    {
      auto keys = WC::getDashboardWidgets(dataset);
      for (const auto &key : keys)
      {
        if (key == WC::DashboardLED)
          ledPanel.m_datasets.append(dataset);

        else if (key != WC::DashboardNoWidget)
          m_widgetDatasets[key].append(dataset);
      }
    }
  }

  // Add LED panel to group widgets (if required)
  if (ledPanel.datasetCount() > 0)
  {
    ledPanel.m_title = tr("Status Panel");
    m_widgetGroups[WC::DashboardLED].append(ledPanel);
  }

  // Get current counts & title
  const auto currentTitle = title();
  QMap<WC::DashboardWidget, int> currentCounts;
  for (auto i = m_widgetGroups.begin(); i != m_widgetGroups.end(); ++i)
    currentCounts[i.key()] = widgetCount(i.key());
  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i)
    currentCounts[i.key()] = widgetCount(i.key());

  // Check if we need to regenerate the dashboard
  if (previousCounts != currentCounts || previousTitle != currentTitle)
  {
    // Clear visibility flags & widget indexes
    m_widgetMap.clear();
    m_widgetVisibility.clear();

    // Initialize widget count parameter
    m_widgetCount = 0;
    m_availableWidgets.clear();

    // Register group widgets
    for (auto i = m_widgetGroups.begin(); i != m_widgetGroups.end(); ++i)
    {
      // Get number of widgets for current widget type
      const auto key = i.key();
      const auto count = widgetCount(key);

      // Make all widgets visible
      QVector<bool> visibilityFlags;
      visibilityFlags.resize(count, true);
      m_widgetVisibility[key] = visibilityFlags;

      // Register available widget types in the same order as the widgets
      // that are drawn on the dashboard grid
      if (!m_availableWidgets.contains(key))
        m_availableWidgets.append(key);

      // Map "global" widget index to index relative to widget type/key
      for (int j = 0; j < count; ++j)
      {
        m_widgetMap.insert(m_widgetCount, qMakePair(key, j));
        ++m_widgetCount;
      }
    }

    // Register dataset widgets
    for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i)
    {
      // Get number of widgets for current widget type
      const auto key = i.key();
      const auto count = widgetCount(key);

      // Make all widgets visible
      QVector<bool> visibilityFlags;
      visibilityFlags.resize(count, true);
      m_widgetVisibility[key] = visibilityFlags;

      // Register available widget types in the same order as the widgets
      // that are drawn on the dashboard grid
      if (!m_availableWidgets.contains(key))
        m_availableWidgets.append(key);

      // Map "global" widget index to index relative to widget type/key
      for (int j = 0; j < count; ++j)
      {
        m_widgetMap.insert(m_widgetCount, qMakePair(key, j));
        ++m_widgetCount;
      }
    }

    // Update user interface
    Q_EMIT widgetCountChanged();
    Q_EMIT widgetVisibilityChanged();
  }

  // Update plot data
  updatePlots();

  // Notify rest of application about the current frame
  Q_EMIT frameReceived(frame);
}
