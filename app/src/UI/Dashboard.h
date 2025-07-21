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

#include "JSON/Frame.h"
#include "SerialStudio.h"

// clang-format off
#define GET_GROUP(type, index) UI::Dashboard::instance().getGroupWidget(type, index)
#define GET_DATASET(type, index) UI::Dashboard::instance().getDatasetWidget(type, index)
#define VALIDATE_WIDGET(type, index) (index >= 0 && index < UI::Dashboard::instance().widgetCount(type))
// clang-format on

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
  Q_PROPERTY(int precision READ precision WRITE setPrecision NOTIFY precisionChanged)
  Q_PROPERTY(bool pointsWidgetVisible READ pointsWidgetVisible NOTIFY widgetCountChanged)
  Q_PROPERTY(bool precisionWidgetVisible READ precisionWidgetVisible NOTIFY widgetCountChanged)
  Q_PROPERTY(bool showActionPanel READ showActionPanel WRITE setShowActionPanel NOTIFY showActionPanelChanged)
  Q_PROPERTY(bool terminalEnabled READ terminalEnabled WRITE setTerminalEnabled NOTIFY terminalEnabledChanged)
  Q_PROPERTY(bool containsCommercialFeatures READ containsCommercialFeatures NOTIFY containsCommercialFeaturesChanged)
  // clang-format on

signals:
  void updated();
  void dataReset();
  void pointsChanged();
  void precisionChanged();
  void widgetCountChanged();
  void actionStatusChanged();
  void showActionPanelChanged();
  void terminalEnabledChanged();
  void containsCommercialFeaturesChanged();

private:
  explicit Dashboard();
  Dashboard(Dashboard &&) = delete;
  Dashboard(const Dashboard &) = delete;
  Dashboard &operator=(Dashboard &&) = delete;
  Dashboard &operator=(const Dashboard &) = delete;

public:
  static Dashboard &instance();
  static double smartInterval(const double min, const double max,
                              const double multiplier = 0.2);

  [[nodiscard]] bool available() const;
  [[nodiscard]] bool showActionPanel() const;
  [[nodiscard]] bool streamAvailable() const;
  [[nodiscard]] bool terminalEnabled() const;
  [[nodiscard]] bool pointsWidgetVisible() const;
  [[nodiscard]] bool precisionWidgetVisible() const;
  [[nodiscard]] bool containsCommercialFeatures() const;

  [[nodiscard]] int points() const;
  [[nodiscard]] int precision() const;
  [[nodiscard]] int actionCount() const;
  [[nodiscard]] int totalWidgetCount() const;

  Q_INVOKABLE bool frameValid() const;
  Q_INVOKABLE int relativeIndex(const int widgetIndex);
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
  [[nodiscard]] const PlotDataY &fftData(const int index) const;
  [[nodiscard]] const GpsSeries &gpsSeries(const int index) const;
  [[nodiscard]] const LineSeries &plotData(const int index) const;
  [[nodiscard]] const MultiLineSeries &multiplotData(const int index) const;

#ifdef BUILD_COMMERCIAL
  [[nodiscard]] const PlotData3D &plotData3D(const int index) const;
#endif

public slots:
  void setPoints(const int points);
  void setPrecision(const int precision);
  void resetData(const bool notify = true);
  void setShowActionPanel(const bool enabled);
  void setTerminalEnabled(const bool enabled);
  void activateAction(const int index, const bool guiTrigger = false);

  void processFrame(const JSON::Frame &frame);

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
  int m_points;           // Number of plot points to retain
  int m_precision;        // Decimal display precision
  int m_widgetCount;      // Total number of active widgets
  bool m_updateRequired;  // Flag to trigger plot/UI update
  bool m_showActionPanel; // Whenever the UI shall display an action panel
  bool m_terminalEnabled; // Whether terminal group is enabled

  PlotDataX m_pltXAxis;      // Default X-axis data for line plots
  PlotDataX m_multipltXAxis; // Default X-axis data for multi-line plots

  QMap<int, PlotDataX> m_xAxisData; // X-axis data per dataset index
  QMap<int, PlotDataY> m_yAxisData; // Y-axis data per dataset index

  QVector<GpsSeries> m_gpsValues;            // GPS data per GPS widget
  QVector<PlotDataY> m_fftValues;            // FFT data per dataset
  QVector<LineSeries> m_pltValues;           // Line plot data
  QVector<MultiLineSeries> m_multipltValues; // Multi-line plot data
#ifdef BUILD_COMMERCIAL
  QVector<PlotData3D> m_plotData3D; // 3D plot data (commercial only)
#endif

  QMap<int, QTimer *> m_timers;        // Timers for dashboard actions
  QVector<JSON::Action> m_actions;     // User-defined dashboard actions
  SerialStudio::WidgetMap m_widgetMap; // Maps window ID index to widget type
  QMap<int, JSON::Dataset> m_datasets; // Raw input datasets (by dataset index)

  // Maps unique dataset ID to all dataset refs for value updates
  QMap<quint32, QVector<JSON::Dataset *>> m_datasetReferences;

  // Groups by widgets type
  QMap<SerialStudio::DashboardWidget, QVector<JSON::Group>> m_widgetGroups;

  // Datasets by widget type
  QMap<SerialStudio::DashboardWidget, QVector<JSON::Dataset>> m_widgetDatasets;

  JSON::Frame m_rawFrame;  // Unmodified incoming frame
  JSON::Frame m_lastFrame; // Processed frame used in UI
};
} // namespace UI
