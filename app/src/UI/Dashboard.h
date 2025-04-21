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
  Q_PROPERTY(int points READ points WRITE setPoints NOTIFY pointsChanged)
  Q_PROPERTY(QStringList actionIcons READ actionIcons NOTIFY actionCountChanged)
  Q_PROPERTY(QStringList actionTitles READ actionTitles NOTIFY actionCountChanged)
  Q_PROPERTY(int totalWidgetCount READ totalWidgetCount NOTIFY widgetCountChanged)
  Q_PROPERTY(int precision READ precision WRITE setPrecision NOTIFY precisionChanged)
  Q_PROPERTY(bool pointsWidgetVisible READ pointsWidgetVisible NOTIFY widgetCountChanged)
  Q_PROPERTY(bool showLegends READ showLegends WRITE setShowLegends NOTIFY showLegendsChanged)
  Q_PROPERTY(bool precisionWidgetVisible READ precisionWidgetVisible NOTIFY widgetCountChanged)
  Q_PROPERTY(bool axisOptionsWidgetVisible READ axisOptionsWidgetVisible NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList availableWidgetIcons READ availableWidgetIcons NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList availableWidgetTitles READ availableWidgetTitles NOTIFY widgetCountChanged)
  Q_PROPERTY(bool showCrosshairs READ showCrosshairs WRITE setShowCrosshairs NOTIFY showCrosshairsChanged)
  Q_PROPERTY(QList<SerialStudio::DashboardWidget> availableWidgets READ availableWidgets NOTIFY widgetCountChanged)
  Q_PROPERTY(bool showAreaUnderPlots READ showAreaUnderPlots WRITE setShowAreaUnderPlots NOTIFY showAreaUnderPlotsChanged)
  Q_PROPERTY(SerialStudio::AxisVisibility axisVisibility READ axisVisibility WRITE setAxisVisibility NOTIFY axisVisibilityChanged)
  // clang-format on

signals:
  void updated();
  void dataReset();
  void pointsChanged();
  void precisionChanged();
  void showLegendsChanged();
  void actionCountChanged();
  void widgetCountChanged();
  void showCrosshairsChanged();
  void axisVisibilityChanged();
  void widgetVisibilityChanged();
  void showAreaUnderPlotsChanged();

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
  [[nodiscard]] bool showLegends() const;
  [[nodiscard]] bool showCrosshairs() const;
  [[nodiscard]] bool streamAvailable() const;
  [[nodiscard]] bool showAreaUnderPlots() const;
  [[nodiscard]] bool pointsWidgetVisible() const;
  [[nodiscard]] bool precisionWidgetVisible() const;
  [[nodiscard]] bool axisOptionsWidgetVisible() const;
  [[nodiscard]] SerialStudio::AxisVisibility axisVisibility() const;

  [[nodiscard]] int points() const;
  [[nodiscard]] int precision() const;
  [[nodiscard]] int actionCount() const;
  [[nodiscard]] int totalWidgetCount() const;

  Q_INVOKABLE bool frameValid() const;
  Q_INVOKABLE int relativeIndex(const int widgetIndex);
  Q_INVOKABLE SerialStudio::DashboardWidget widgetType(const int widgetIndex);
  Q_INVOKABLE int widgetCount(const SerialStudio::DashboardWidget widget) const;
  Q_INVOKABLE QStringList
  widgetTitles(const SerialStudio::DashboardWidget widget);
  Q_INVOKABLE QStringList
  widgetColors(const SerialStudio::DashboardWidget widget);
  Q_INVOKABLE bool widgetVisible(const SerialStudio::DashboardWidget widget,
                                 const int index) const;

  [[nodiscard]] const QStringList availableWidgetIcons() const;
  [[nodiscard]] const QStringList availableWidgetTitles() const;
  [[nodiscard]] const QList<SerialStudio::DashboardWidget>
  availableWidgets() const;

  [[nodiscard]] const QString &title() const;
  [[nodiscard]] QStringList actionIcons() const;
  [[nodiscard]] QStringList actionTitles() const;

  // clang-format off
  [[nodiscard]] const QMap<int, JSON::Dataset> &datasets() const;
  [[nodiscard]] const JSON::Group &getGroupWidget(const SerialStudio::DashboardWidget widget, const int index) const;
  [[nodiscard]] const JSON::Dataset &getDatasetWidget(const SerialStudio::DashboardWidget widget, const int index) const;
  // clang-format on

  [[nodiscard]] const JSON::Frame &currentFrame();

  [[nodiscard]] const PlotDataY &fftData(const int index) const;
  [[nodiscard]] const LineSeries &plotData(const int index) const;
  [[nodiscard]] const MultiLineSeries &multiplotData(const int index) const;

public slots:
  void setPoints(const int points);
  void activateAction(const int index);
  void setPrecision(const int precision);
  void setShowLegends(const bool enabled);
  void resetData(const bool notify = true);
  void setShowCrosshairs(const bool enabled);
  void setShowAreaUnderPlots(const bool enabled);
  void setAxisVisibility(const SerialStudio::AxisVisibility option);
  void setWidgetVisible(const SerialStudio::DashboardWidget widget,
                        const int index, const bool visible);

private slots:
  void updatePlots();
  void configureFftSeries();
  void configureLineSeries();
  void configureMultiLineSeries();
  void processFrame(const JSON::Frame &frame);

private:
  int m_points;
  int m_precision;
  int m_widgetCount;
  bool m_showLegends;
  bool m_updateRequired;
  bool m_showCrosshairs;
  bool m_showAreaUnderPlots;
  SerialStudio::AxisVisibility m_axisVisibility;

  PlotDataX m_pltXAxis;
  PlotDataX m_multipltXAxis;
  QMap<int, PlotDataX> m_xAxisData;
  QMap<int, PlotDataY> m_yAxisData;

  QVector<PlotDataY> m_fftValues;
  QVector<LineSeries> m_pltValues;
  QVector<MultiLineSeries> m_multipltValues;

  QVector<JSON::Action> m_actions;
  QMap<int, JSON::Dataset> m_datasets;
  QList<SerialStudio::DashboardWidget> m_availableWidgets;
  QMap<int, QPair<SerialStudio::DashboardWidget, int>> m_widgetMap;
  QMap<SerialStudio::DashboardWidget, QVector<bool>> m_widgetVisibility;
  QMap<SerialStudio::DashboardWidget, QVector<JSON::Group>> m_widgetGroups;
  QMap<SerialStudio::DashboardWidget, QVector<JSON::Dataset>> m_widgetDatasets;

  JSON::Frame m_currentFrame;
};
} // namespace UI
