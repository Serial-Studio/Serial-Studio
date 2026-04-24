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
#include <QSettings>

#include "DSP.h"
#include "SerialStudio.h"
#include "UI/WidgetRegistry.h"

namespace UI {
/**
 * @brief Real-time dashboard manager for displaying data-driven widgets.
 */
class Dashboard : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool available
             READ available
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int  points
             READ  points
             WRITE setPoints
             NOTIFY pointsChanged)
  Q_PROPERTY(int  actionCount
             READ actionCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int  totalWidgetCount
             READ totalWidgetCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(bool pointsWidgetVisible
             READ pointsWidgetVisible
             NOTIFY widgetCountChanged)
  Q_PROPERTY(bool showActionPanel
             READ  showActionPanel
             WRITE setShowActionPanel
             NOTIFY showActionPanelChanged)
  Q_PROPERTY(bool terminalEnabled
             READ  terminalEnabled
             WRITE setTerminalEnabled
             NOTIFY terminalEnabledChanged)
  Q_PROPERTY(bool notificationLogEnabled
             READ  notificationLogEnabled
             WRITE setNotificationLogEnabled
             NOTIFY notificationLogEnabledChanged)
  Q_PROPERTY(bool autoHideToolbar
             READ  autoHideToolbar
             WRITE setAutoHideToolbar
             NOTIFY autoHideToolbarChanged)
  Q_PROPERTY(bool showTaskbarButtons
             READ  showTaskbarButtons
             WRITE setShowTaskbarButtons
             NOTIFY showTaskbarButtonsChanged)
  Q_PROPERTY(bool containsCommercialFeatures
             READ containsCommercialFeatures
             NOTIFY containsCommercialFeaturesChanged)
  Q_PROPERTY(QString title
             READ title
             NOTIFY widgetCountChanged)
  Q_PROPERTY(QVariantList actions
             READ actions
             NOTIFY actionStatusChanged)
  // clang-format on

signals:
  void updated();
  void dataReset();
  void pointsChanged();
  void widgetCountChanged();
  void actionStatusChanged();
  void showActionPanelChanged();
  void terminalEnabledChanged();
  void notificationLogEnabledChanged();
  void showTaskbarButtonsChanged();
  void autoHideToolbarChanged();
  void containsCommercialFeaturesChanged();

private:
  explicit Dashboard();
  Dashboard(Dashboard&&)                 = delete;
  Dashboard(const Dashboard&)            = delete;
  Dashboard& operator=(Dashboard&&)      = delete;
  Dashboard& operator=(const Dashboard&) = delete;

public:
  [[nodiscard]] static Dashboard& instance();

  [[nodiscard]] bool available() const;
  [[nodiscard]] bool showActionPanel() const noexcept;
  [[nodiscard]] bool streamAvailable() const;
  [[nodiscard]] bool terminalEnabled() const noexcept;
  [[nodiscard]] bool notificationLogEnabled() const noexcept;
  [[nodiscard]] bool autoHideToolbar() const noexcept;
  [[nodiscard]] bool showTaskbarButtons() const noexcept;
  [[nodiscard]] bool pointsWidgetVisible() const;
  [[nodiscard]] bool containsCommercialFeatures() const noexcept;

  [[nodiscard]] int points() const noexcept;
  [[nodiscard]] int actionCount() const;
  [[nodiscard]] int totalWidgetCount() const noexcept;

  [[nodiscard]] Q_INVOKABLE bool frameValid() const;
  [[nodiscard]] Q_INVOKABLE int relativeIndex(const int widgetIndex) const;
  [[nodiscard]] Q_INVOKABLE QString formatValue(double val, double min, double max) const;
  [[nodiscard]] Q_INVOKABLE SerialStudio::DashboardWidget widgetType(const int widgetIndex) const;
  [[nodiscard]] Q_INVOKABLE int widgetCount(const SerialStudio::DashboardWidget widget) const;

  [[nodiscard]] const QString& title() const;
  [[nodiscard]] QVariantList actions() const;
  [[nodiscard]] const SerialStudio::WidgetMap& widgetMap() const;

  // clang-format off
  [[nodiscard]] const QMap<int, DataModel::Dataset> &datasets() const;
  [[nodiscard]] const DataModel::Group &getGroupWidget(const SerialStudio::DashboardWidget widget, const int index) const;
  [[nodiscard]] const DataModel::Dataset &getDatasetWidget(const SerialStudio::DashboardWidget widget, const int index) const;
  // clang-format on

  [[nodiscard]] const DataModel::Frame& rawFrame();
  [[nodiscard]] const DataModel::Frame& processedFrame();
  [[nodiscard]] const DSP::AxisData& fftData(const int index) const;
  [[nodiscard]] const DSP::GpsSeries& gpsSeries(const int index) const;
  [[nodiscard]] const DSP::LineSeries& plotData(const int index) const;
  [[nodiscard]] const DSP::MultiLineSeries& multiplotData(const int index) const;

#ifdef BUILD_COMMERCIAL
  [[nodiscard]] const DSP::LineSeries3D& plotData3D(const int index) const;
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
  void setNotificationLogEnabled(const bool enabled);
  void setAutoHideToolbar(const bool enabled);
  void setShowTaskbarButtons(const bool enabled);
  void activateAction(const int index, const bool guiTrigger = false);

  void setPlotRunning(const int index, const bool enabled);
  void setFFTPlotRunning(const int index, const bool enabled);
  void setMultiplotRunning(const int index, const bool enabled);

  void hotpathRxFrame(const DataModel::TimestampedFramePtr& frame);

private:
  void updateDashboardData(const DataModel::Frame& frame);
  void reconfigureDashboard(const DataModel::Frame& frame);
  void processDatasetIntoWidgetMaps(const DataModel::Dataset& dataset, DataModel::Group& ledPanel);
  void removeTerminalWidget();
  void removeNotificationLogWidget();
  void handleMissingDataset(const DataModel::Frame& frame);
  void registerXAxisIfNeeded(const DataModel::Dataset& dataset);

  void updateDataSeries(int sourceId = -1);
  void updateFftSeries(int sourceId);
  void updateGpsSeries(int sourceId);
  void updatePlot3DSeries(int sourceId);
  void updateLineSeries(int sourceId);

  void configureGpsSeries();
  void configureFftSeries();
  void configureLineSeries();
  void configurePlot3DSeries();
  void configureMultiLineSeries();
  void configureActions(const DataModel::Frame& frame);

  void buildWidgetGroups(const DataModel::Frame& frame, bool pro);
  void registerWidgets();
  void buildDatasetReferences();

private:
  QSettings m_settings;
  int m_points;
  int m_widgetCount;
  bool m_updateRequired;
  bool m_showActionPanel;
  bool m_terminalEnabled;
  WidgetID m_terminalWidgetId;
  bool m_notificationLogEnabled;
  WidgetID m_notificationLogWidgetId;
  bool m_autoHideToolbar;
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

  QMap<int, QTimer*> m_timers;
  QMap<int, int> m_repeatCounters;
  QVector<DataModel::Action> m_actions;
  SerialStudio::WidgetMap m_widgetMap;
  QMap<int, DataModel::Dataset> m_datasets;

  QMap<int, QVector<DataModel::Dataset*>> m_datasetReferences;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Group>> m_widgetGroups;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Dataset>> m_widgetDatasets;

  DataModel::Frame m_lastFrame;
  QMap<int, DataModel::Frame> m_sourceRawFrames;
};
}  // namespace UI

inline QString FMT_VAL(double val, double min, double max)
{
  auto decPoints = [](double v) {
    double abs = std::abs(v);
    if (DSP::isZero(abs))
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

    return 10;
  };

  if (DSP::isZero(min) && DSP::isZero(max))
    return QString::number(val, 'f', decPoints(val));

  else {
    const int p = std::max(decPoints(min), decPoints(max));
    return QString::number(val, 'f', p);
  }
}

inline QString FMT_VAL(double val, const DataModel::Dataset& dataset)
{
  return FMT_VAL(val, dataset.pltMin, dataset.pltMax);
}

inline const DataModel::Group& GET_GROUP(const SerialStudio::DashboardWidget type, int index)
{
  return UI::Dashboard::instance().getGroupWidget(type, index);
}

inline const DataModel::Dataset& GET_DATASET(const SerialStudio::DashboardWidget type, int index)
{
  return UI::Dashboard::instance().getDatasetWidget(type, index);
}

inline bool VALIDATE_WIDGET(const SerialStudio::DashboardWidget type, int index)
{
  return index >= 0 && index < UI::Dashboard::instance().widgetCount(type);
}
