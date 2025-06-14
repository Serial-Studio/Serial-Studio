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

#pragma once

#include <QFont>
#include <QObject>
#include <QReadWriteLock>

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
  Q_PROPERTY(int actionCount READ actionCount NOTIFY actionCountChanged)
  Q_PROPERTY(QVariantList actions READ actions NOTIFY actionCountChanged)
  Q_PROPERTY(int points READ points WRITE setPoints NOTIFY pointsChanged)
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
  void actionCountChanged();
  void widgetCountChanged();
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
  static qreal smartInterval(const qreal min, const qreal max,
                             const qreal multiplier = 0.2);

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
  [[nodiscard]] const LineSeries &plotData(const int index) const;
  [[nodiscard]] const MultiLineSeries &multiplotData(const int index) const;

#ifdef USE_QT_COMMERCIAL
  [[nodiscard]] const PlotData3D &plotData3D(const int index) const;
#endif

public slots:
  void setPoints(const int points);
  void activateAction(const int index);
  void setPrecision(const int precision);
  void resetData(const bool notify = true);
  void setShowActionPanel(const bool enabled);
  void setTerminalEnabled(const bool enabled);

private slots:
  void processFrame(const JSON::Frame &frame);

private:
  void updateDashboardData(const JSON::Frame &frame);
  void reconfigureDashboard(const JSON::Frame &frame);

  void updatePlots();
  void configureFftSeries();
  void configureLineSeries();
  void configureMultiLineSeries();

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

  QVector<PlotDataY> m_fftValues;            // FFT data per dataset
  QVector<LineSeries> m_pltValues;           // Line plot data
  QVector<MultiLineSeries> m_multipltValues; // Multi-line plot data
#ifdef USE_QT_COMMERCIAL
  QVector<PlotData3D> m_plotData3D; // 3D plot data (commercial only)
#endif

  QVector<JSON::Action> m_actions;     // User-defined dashboard actions
  SerialStudio::WidgetMap m_widgetMap; // Maps window ID index to widget type
  QMap<int, JSON::Dataset> m_datasets; // Raw input datasets (by dataset index)

  // Maps unique dataset ID to all dataset refs for value updates
  QMap<quint32, QVector<JSON::Dataset *>> m_datasetReferences;

  // Groups by widgets type
  QMap<SerialStudio::DashboardWidget, QVector<JSON::Group>> m_widgetGroups;

  // Datasets by widget type
  QMap<SerialStudio::DashboardWidget, QVector<JSON::Dataset>> m_widgetDatasets;

  JSON::Frame m_rawFrame;            // Unmodified incoming frame
  JSON::Frame m_lastFrame;           // Processed frame used in UI
  mutable QReadWriteLock m_dataLock; // Thread-safe data access
};
} // namespace UI
