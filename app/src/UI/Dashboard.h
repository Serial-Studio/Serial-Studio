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

#include <QFont>
#include <QObject>

#include "DSP.h"
#include "SerialStudio.h"
#include "UI/WidgetRegistry.h"

namespace UI
{
/**
 * @class UI::Dashboard
 * @brief Real-time dashboard manager for displaying data-driven widgets.
 *
 * The `Dashboard` class creates and maintains the model used for generating a
 * dashboard user interface, updating various widgets such as plots, multiplots,
 * and status indicators based on JSON frame data.
 *
 * Updates occur at a maximum rate of 20 Hz for optimal performance. It manages
 * real-time data for different plot types (linear, FFT, multiplot) and supports
 * actions that can be triggered from the UI.
 *
 * Properties notify changes to dynamically adjust UI elements like widget
 * visibility and count.
 *
 * @note This class is implemented as a singleton and is non-copyable and
 *       non-movable.
 */
class Dashboard : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString title READ title NOTIFY widgetCountChanged)
  Q_PROPERTY(bool available READ available NOTIFY widgetCountChanged)
  Q_PROPERTY(int actionCount READ actionCount NOTIFY widgetCountChanged)
  Q_PROPERTY(int points READ points WRITE setPoints NOTIFY pointsChanged)
  Q_PROPERTY(QVariantList actions READ actions NOTIFY actionStatusChanged)
  Q_PROPERTY(int totalWidgetCount READ totalWidgetCount NOTIFY widgetCountChanged)
  Q_PROPERTY(bool pointsWidgetVisible READ pointsWidgetVisible NOTIFY widgetCountChanged)
  Q_PROPERTY(bool precisionWidgetVisible READ precisionWidgetVisible NOTIFY widgetCountChanged)
  Q_PROPERTY(bool showActionPanel READ showActionPanel WRITE setShowActionPanel NOTIFY showActionPanelChanged)
  Q_PROPERTY(bool terminalEnabled READ terminalEnabled WRITE setTerminalEnabled NOTIFY terminalEnabledChanged)
  Q_PROPERTY(bool containsCommercialFeatures READ containsCommercialFeatures NOTIFY containsCommercialFeaturesChanged)
  Q_PROPERTY(bool showTaskbarButtons READ showTaskbarButtons WRITE setShowTaskbarButtons NOTIFY showTaskbarButtonsChanged)
  // clang-format on

signals:
  void updated();
  void dataReset();
  void pointsChanged();
  void widgetCountChanged();
  void actionStatusChanged();
  void showActionPanelChanged();
  void terminalEnabledChanged();
  void showTaskbarButtonsChanged();
  void containsCommercialFeaturesChanged();

private:
  explicit Dashboard();
  Dashboard(Dashboard &&) = delete;
  Dashboard(const Dashboard &) = delete;
  Dashboard &operator=(Dashboard &&) = delete;
  Dashboard &operator=(const Dashboard &) = delete;

public:
  static Dashboard &instance();

  [[nodiscard]] bool available() const;
  [[nodiscard]] bool showActionPanel() const;
  [[nodiscard]] bool streamAvailable() const;
  [[nodiscard]] bool terminalEnabled() const;
  [[nodiscard]] bool showTaskbarButtons() const;
  [[nodiscard]] bool pointsWidgetVisible() const;
  [[nodiscard]] bool precisionWidgetVisible() const;
  [[nodiscard]] bool containsCommercialFeatures() const;

  [[nodiscard]] int points() const;
  [[nodiscard]] int actionCount() const;
  [[nodiscard]] int totalWidgetCount() const;

  Q_INVOKABLE bool frameValid() const;
  Q_INVOKABLE int relativeIndex(const int widgetIndex);
  Q_INVOKABLE QString formatValue(double val, double min, double max) const;
  Q_INVOKABLE SerialStudio::DashboardWidget widgetType(const int widgetIndex);
  Q_INVOKABLE int widgetCount(const SerialStudio::DashboardWidget widget) const;

  [[nodiscard]] const QString &title() const;
  [[nodiscard]] QVariantList actions() const;
  [[nodiscard]] const SerialStudio::WidgetMap &widgetMap() const;

  // clang-format off
  [[nodiscard]] const QMap<int, JSON::Dataset> &datasets() const;
  [[nodiscard]] const JSON::Group &getGroupWidget(const SerialStudio::DashboardWidget widget, const int index) const;
  [[nodiscard]] const JSON::Dataset &getDatasetWidget(const SerialStudio::DashboardWidget widget, const int index) const;
  // clang-format on

  [[nodiscard]] const JSON::Frame &rawFrame();
  [[nodiscard]] const JSON::Frame &processedFrame();
  [[nodiscard]] const DSP::AxisData &fftData(const int index) const;
  [[nodiscard]] const DSP::GpsSeries &gpsSeries(const int index) const;
  [[nodiscard]] const DSP::LineSeries &plotData(const int index) const;
  [[nodiscard]] const DSP::MultiLineSeries &
  multiplotData(const int index) const;

#ifdef BUILD_COMMERCIAL
  [[nodiscard]] const DSP::LineSeries3D &plotData3D(const int index) const;
#endif

  [[nodiscard]] bool plotRunning(const int index);
  [[nodiscard]] bool fftPlotRunning(const int index);
  [[nodiscard]] bool multiplotRunning(const int index);

public slots:
  void setPoints(const int points);
  void resetData(const bool notify = true);
  void clearPlotData();
  void setShowActionPanel(const bool enabled);
  void setTerminalEnabled(const bool enabled);
  void setShowTaskbarButtons(const bool enabled);
  void activateAction(const int index, const bool guiTrigger = false);

  void setPlotRunning(const int index, const bool enabled);
  void setFFTPlotRunning(const int index, const bool enabled);
  void setMultiplotRunning(const int index, const bool enabled);

  void hotpathRxFrame(const JSON::Frame &frame);

private:
  void updateDashboardData(const JSON::Frame &frame);
  void reconfigureDashboard(const JSON::Frame &frame);

  void updateDataSeries();
  void configureGpsSeries();
  void configureFftSeries();
  void configureLineSeries();
  void configurePlot3DSeries();
  void configureMultiLineSeries();
  void configureActions(const JSON::Frame &frame);

private:
  int m_points;
  int m_widgetCount;
  bool m_updateRequired;
  bool m_showActionPanel;
  bool m_terminalEnabled;
  WidgetID m_terminalWidgetId;
  bool m_showTaskbarButtons;

  bool m_updateRetryInProgress;

  DSP::AxisData m_pltXAxis;
  DSP::AxisData m_multipltXAxis;

  QMap<int, DSP::AxisData> m_xAxisData;
  QMap<int, DSP::AxisData> m_yAxisData;

  QMap<int, bool> m_activePlots;
  QMap<int, bool> m_activeFFTPlots;
  QMap<int, bool> m_activeMultiplots;

  QVector<DSP::GpsSeries> m_gpsValues;
  QVector<DSP::AxisData> m_fftValues;
  QVector<DSP::LineSeries> m_pltValues;
  QVector<DSP::MultiLineSeries> m_multipltValues;
#ifdef BUILD_COMMERCIAL
  QVector<DSP::LineSeries3D> m_plotData3D;
#endif

  QMap<int, QTimer *> m_timers;
  QVector<JSON::Action> m_actions;
  SerialStudio::WidgetMap m_widgetMap;
  QMap<int, JSON::Dataset> m_datasets;

  // Maps unique dataset ID to all dataset refs for value updates
  QMap<int, QVector<JSON::Dataset *>> m_datasetReferences;

  // Groups by widgets type
  QMap<SerialStudio::DashboardWidget, QVector<JSON::Group>> m_widgetGroups;

  // Datasets by widget type
  QMap<SerialStudio::DashboardWidget, QVector<JSON::Dataset>> m_widgetDatasets;

  JSON::Frame m_rawFrame;
  JSON::Frame m_lastFrame;
};
} // namespace UI

//------------------------------------------------------------------------------
// Inline functions for widgets
//------------------------------------------------------------------------------

/**
 * @brief Formats a floating-point value with dynamic decimal precision based on
 * context.
 *
 * This function determines a suitable number of decimal places for the input
 * value `val` based on the magnitudes of `min` and `max`, which define the
 * expected range of values. The formatting logic is intended to make numeric
 * output clean, readable, and free from scientific notation in most practical
 * engineering use cases.
 *
 * If `min` and `max` are both zero, it will determine the precision solely from
 * `val`.
 *
 * @param val The value to format.
 * @param min The minimum value of the range.
 * @param max The maximum value of the range.
 * @return A QString representing the formatted number.
 */
inline QString FMT_VAL(double val, double min, double max)
{
  auto decPoints = [](double v) {
    double abs = std::abs(v);
    if (qFuzzyIsNull(abs))
      return 2;
    if (abs >= 1e6)
      return 0;
    if (abs >= 1e5)
      return 0;
    if (abs >= 1e4)
      return 0;
    if (abs >= 1e3)
      return 1;
    if (abs >= 1e2)
      return 2;
    if (abs >= 1e1)
      return 2;
    if (abs >= 1.0)
      return 3;
    if (abs >= 1e-1)
      return 4;
    if (abs >= 1e-2)
      return 5;
    if (abs >= 1e-3)
      return 6;
    if (abs >= 1e-4)
      return 7;
    if (abs >= 1e-5)
      return 8;
    if (abs >= 1e-6)
      return 9;

    // Beyond this is just sad
    return 10;
  };

  if (qFuzzyIsNull(min) && qFuzzyIsNull(max))
    return QString::number(val, 'f', decPoints(val));

  else
  {
    const int p = std::max(decPoints(min), decPoints(max));
    return QString::number(val, 'f', p);
  }
}

/**
 * @brief Formats a floating-point value with adaptive decimal precision based
 * on dataset range.
 *
 * Determines the number of decimal places to display based on the magnitude of
 * the dataset’s pltMin and pltMax. Useful for presenting values in UIs where
 * consistent but meaningful precision is important.
 *
 * @param val The value to format.
 * @param dataset A dataset providing context (pltMin/pltMax) for determining
 *                required precision.
 *
 * @return QString Formatted number with appropriate decimal places.
 */
inline QString FMT_VAL(double val, const JSON::Dataset &dataset)
{
  return FMT_VAL(val, dataset.pltMin, dataset.pltMax);
}

/**
 * @brief Retrieves a reference to a dataset group widget by type and index.
 *
 * Provides direct access to a dataset group from the dashboard instance.
 * Use this in contexts where group-based widgets (e.g., GPS, 3D, multi-plot)
 * are expected.
 *
 * @param type The widget type (must be a group-based widget).
 * @param index Index of the widget in the corresponding group list.
 * @return Reference to the matching JSON::Group.
 *
 * @note Caller is responsible for ensuring the index is valid.
 */
inline const JSON::Group &GET_GROUP(const SerialStudio::DashboardWidget type,
                                    int index)
{
  return UI::Dashboard::instance().getGroupWidget(type, index);
}

/**
 * @brief Retrieves a reference to a dataset widget by type and index.
 *
 * Provides direct access to a single dataset widget from the dashboard
 * instance. Use for widget types tied to individual datasets (e.g., plots,
 * FFT, gauges).
 *
 * @param type The widget type (must be a dataset-based widget).
 * @param index Index of the widget in the corresponding dataset list.
 * @return Reference to the matching JSON::Dataset.
 *
 * @note Caller is responsible for ensuring the index is valid.
 */
inline const JSON::Dataset &
GET_DATASET(const SerialStudio::DashboardWidget type, int index)
{
  return UI::Dashboard::instance().getDatasetWidget(type, index);
}

/**
 * @brief Validates whether a widget index is in bounds for the given type.
 *
 * Checks if a widget of the given type exists at the specified index.
 * Prevents out-of-bounds access when working with dashboard widgets.
 *
 * @param type The widget type to check.
 * @param index Index to validate.
 * @return true if the widget index is valid, false otherwise.
 */
inline bool VALIDATE_WIDGET(const SerialStudio::DashboardWidget type, int index)
{
  return index >= 0 && index < UI::Dashboard::instance().widgetCount(type);
}
