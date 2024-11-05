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

#pragma once

#include <QFont>
#include <QObject>
#include <JSON/Frame.h>
#include <WidgetsCommon.h>

namespace UI
{

/**
 * @class UI::Dashboard
 * @brief Real-time dashboard manager for displaying data-driven widgets and p
 *        lots.
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
  Q_PROPERTY(bool precisionWidgetVisible READ precisionWidgetVisible NOTIFY widgetCountChanged)
  Q_PROPERTY(bool axisOptionsWidgetVisible READ axisOptionsWidgetVisible NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList availableWidgetIcons READ availableWidgetIcons NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList availableWidgetTitles READ availableWidgetTitles NOTIFY widgetCountChanged)
  Q_PROPERTY(QList<WC::DashboardWidget> availableWidgets READ availableWidgets NOTIFY widgetCountChanged)
  Q_PROPERTY(AxisVisibility axisVisibility READ axisVisibility WRITE setAxisVisibility NOTIFY axisVisibilityChanged)
  // clang-format on

signals:
  void updated();
  void dataReset();
  void pointsChanged();
  void precisionChanged();
  void actionCountChanged();
  void widgetCountChanged();
  void axisVisibilityChanged();
  void widgetVisibilityChanged();
  void frameReceived(const JSON::Frame &frame);

private:
  explicit Dashboard();
  Dashboard(Dashboard &&) = delete;
  Dashboard(const Dashboard &) = delete;
  Dashboard &operator=(Dashboard &&) = delete;
  Dashboard &operator=(const Dashboard &) = delete;

public:
  enum AxisVisibility
  {
    AxisXY = 0b11,
    AxisX = 0b01,
    AxisY = 0b10,
    NoAxesVisible = 0b00,
  };
  Q_ENUM(AxisVisibility)

  static Dashboard &instance();
  static qreal smartInterval(const qreal min, const qreal max,
                             const qreal multiplier = 0.2);

  [[nodiscard]] bool available() const;
  [[nodiscard]] bool pointsWidgetVisible() const;
  [[nodiscard]] bool precisionWidgetVisible() const;
  [[nodiscard]] bool axisOptionsWidgetVisible() const;
  [[nodiscard]] AxisVisibility axisVisibility() const;

  [[nodiscard]] int points() const;
  [[nodiscard]] int precision() const;
  [[nodiscard]] int actionCount() const;
  [[nodiscard]] int totalWidgetCount() const;

  Q_INVOKABLE bool frameValid() const;
  Q_INVOKABLE int relativeIndex(const int widgetIndex);
  Q_INVOKABLE WC::DashboardWidget widgetType(const int widgetIndex);
  Q_INVOKABLE int widgetCount(const WC::DashboardWidget widget) const;
  Q_INVOKABLE QStringList widgetTitles(const WC::DashboardWidget widget);
  Q_INVOKABLE QStringList widgetColors(const WC::DashboardWidget widget);
  Q_INVOKABLE bool widgetVisible(const WC::DashboardWidget widget,
                                 const int index) const;

  [[nodiscard]] const QStringList availableWidgetIcons() const;
  [[nodiscard]] const QStringList availableWidgetTitles() const;
  [[nodiscard]] const QList<WC::DashboardWidget> availableWidgets() const;

  [[nodiscard]] const QString &title() const;
  [[nodiscard]] QStringList actionIcons() const;
  [[nodiscard]] QStringList actionTitles() const;

  // clang-format off
  [[nodiscard]] const JSON::Group &getGroupWidget(const WC::DashboardWidget widget, const int index) const;
  [[nodiscard]] const JSON::Dataset &getDatasetWidget(const WC::DashboardWidget widget, const int index) const;
  // clang-format on

  [[nodiscard]] const JSON::Frame &currentFrame();
  [[nodiscard]] const QVector<Curve> &fftPlotValues();
  [[nodiscard]] const QVector<Curve> &linearPlotValues();
  [[nodiscard]] const QVector<MultipleCurves> &multiplotValues();

public slots:
  void setPoints(const int points);
  void activateAction(const int index);
  void setPrecision(const int precision);
  void resetData(const bool notify = true);
  void setAxisVisibility(const AxisVisibility option);
  void setWidgetVisible(const WC::DashboardWidget widget, const int index,
                        const bool visible);

private slots:
  void updatePlots();
  void processFrame(const JSON::Frame &frame);

private:
  int m_points;
  int m_precision;
  int m_widgetCount;
  bool m_updateRequired;
  AxisVisibility m_axisVisibility;

  QVector<Curve> m_fftPlotValues;
  QVector<Curve> m_linearPlotValues;
  QVector<MultipleCurves> m_multiplotValues;

  QVector<JSON::Action> m_actions;
  QList<WC::DashboardWidget> m_availableWidgets;
  QMap<int, QPair<WC::DashboardWidget, int>> m_widgetMap;
  QMap<WC::DashboardWidget, QVector<bool>> m_widgetVisibility;
  QMap<WC::DashboardWidget, QVector<JSON::Group>> m_widgetGroups;
  QMap<WC::DashboardWidget, QVector<JSON::Dataset>> m_widgetDatasets;

  JSON::Frame m_currentFrame;
};
} // namespace UI
