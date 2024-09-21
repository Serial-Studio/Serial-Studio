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

namespace UI
{
/**
 * @brief The Dashboard class
 *
 * The @c Dashboard class receives data from the @c JSON::Generator class and
 * builds the vector modules used by the QML user interface and the C++ widgets
 * to display the current frame.
 *
 * This class is very large, but its really simple to understand. Most of the
 * code here is repeated in order to support all the widgets implemented by
 * Serial Studio.
 *
 * The important functions are:
 *
 * - @c Dashboard::updateData()
 * - @c Dashboard::groupTitles()
 * - @c Dashboard::datasetTitles()
 * - @c Dashboard::getVisibility()
 * - @c Dashboard::getVisibility()
 * - @c Dashboard::getGroupWidget()
 * - @c Dashboard::getWidgetGroups()
 * - @c Dashboard::getDatasetWidget()
 * - @c Dashboard::getWidgetDatasets()
 * - @c Dashboard::processLatestJSON()
 *
 * The rest of the functions of this class rely on the procedures above in order
 * to implement common functionality features for each widget type.
 */
class Dashboard : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString title
             READ title
             NOTIFY titleChanged)
  Q_PROPERTY(bool available
             READ available
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int points
             READ points
             WRITE setPoints
             NOTIFY pointsChanged)
  Q_PROPERTY(int precision
             READ precision
             WRITE setPrecision
             NOTIFY precisionChanged)
  Q_PROPERTY(int totalWidgetCount
             READ totalWidgetCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int gpsCount
             READ gpsCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int ledCount
             READ ledCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int barCount
             READ barCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int fftCount
             READ fftCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int plotCount
             READ plotCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int groupCount
             READ groupCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int gaugeCount
             READ gaugeCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int compassCount
             READ compassCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int gyroscopeCount
             READ gyroscopeCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int multiPlotCount
             READ multiPlotCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int accelerometerCount
             READ accelerometerCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList gpsTitles
             READ gpsTitles
             NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList ledTitles
             READ ledTitles
             NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList barTitles
             READ barTitles
             NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList fftTitles
             READ fftTitles
             NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList plotTitles
             READ plotTitles
             NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList groupTitles
             READ groupTitles
             NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList gaugeTitles
             READ gaugeTitles
             NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList compassTitles
             READ compassTitles
             NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList gyroscopeTitles
             READ gyroscopeTitles
             NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList multiPlotTitles
             READ multiPlotTitles
             NOTIFY widgetCountChanged)
  Q_PROPERTY(QStringList accelerometerTitles
             READ accelerometerTitles
             NOTIFY widgetCountChanged)
  // clang-format on

signals:
  void updated();
  void dataReset();
  void titleChanged();
  void pointsChanged();
  void precisionChanged();
  void widgetCountChanged();
  void widgetVisibilityChanged();
  void frameReceived(const JSON::Frame &frame);

private:
  explicit Dashboard();
  Dashboard(Dashboard &&) = delete;
  Dashboard(const Dashboard &) = delete;
  Dashboard &operator=(Dashboard &&) = delete;
  Dashboard &operator=(const Dashboard &) = delete;

public:
  enum class WidgetType
  {
    Group,
    MultiPlot,
    FFT,
    Plot,
    Bar,
    Gauge,
    Compass,
    Gyroscope,
    Accelerometer,
    GPS,
    LED,
    Unknown
  };
  Q_ENUM(WidgetType)

  static Dashboard &instance();

  [[nodiscard]] const QFont &monoFont() const;
  [[nodiscard]] const JSON::Group &getLED(const int index) const;
  [[nodiscard]] const JSON::Group &getGPS(const int index) const;
  [[nodiscard]] const JSON::Dataset &getFFT(const int index) const;
  [[nodiscard]] const JSON::Dataset &getBar(const int index) const;
  [[nodiscard]] const JSON::Group &getGroups(const int index) const;
  [[nodiscard]] const JSON::Dataset &getPlot(const int index) const;
  [[nodiscard]] const JSON::Dataset &getGauge(const int index) const;
  [[nodiscard]] const JSON::Group &getGyroscope(const int index) const;
  [[nodiscard]] const JSON::Dataset &getCompass(const int index) const;
  [[nodiscard]] const JSON::Group &getMultiplot(const int index) const;
  [[nodiscard]] const JSON::Group &getAccelerometer(const int index) const;

  [[nodiscard]] const QString &title() const;

  [[nodiscard]] bool available();
  [[nodiscard]] int points() const;
  [[nodiscard]] int precision() const;

  [[nodiscard]] int totalWidgetCount() const;
  [[nodiscard]] int gpsCount() const;
  [[nodiscard]] int ledCount() const;
  [[nodiscard]] int fftCount() const;
  [[nodiscard]] int barCount() const;
  [[nodiscard]] int plotCount() const;
  [[nodiscard]] int groupCount() const;
  [[nodiscard]] int gaugeCount() const;
  [[nodiscard]] int compassCount() const;
  [[nodiscard]] int gyroscopeCount() const;
  [[nodiscard]] int multiPlotCount() const;
  [[nodiscard]] int accelerometerCount() const;

  Q_INVOKABLE bool frameValid() const;
  Q_INVOKABLE QStringList widgetTitles();
  Q_INVOKABLE int relativeIndex(const int globalIndex) const;
  Q_INVOKABLE bool widgetVisible(const int globalIndex) const;
  Q_INVOKABLE QString widgetIcon(const int globalIndex) const;
  Q_INVOKABLE UI::Dashboard::WidgetType widgetType(const int globalIndex) const;

  Q_INVOKABLE bool barVisible(const int index) const;
  Q_INVOKABLE bool fftVisible(const int index) const;
  Q_INVOKABLE bool gpsVisible(const int index) const;
  Q_INVOKABLE bool ledVisible(const int index) const;
  Q_INVOKABLE bool plotVisible(const int index) const;
  Q_INVOKABLE bool groupVisible(const int index) const;
  Q_INVOKABLE bool gaugeVisible(const int index) const;
  Q_INVOKABLE bool compassVisible(const int index) const;
  Q_INVOKABLE bool gyroscopeVisible(const int index) const;
  Q_INVOKABLE bool multiPlotVisible(const int index) const;
  Q_INVOKABLE bool accelerometerVisible(const int index) const;

  QStringList barTitles();
  QStringList fftTitles();
  QStringList gpsTitles();
  QStringList ledTitles();
  QStringList plotTitles();
  QStringList groupTitles();
  QStringList gaugeTitles();
  QStringList compassTitles();
  QStringList gyroscopeTitles();
  QStringList multiPlotTitles();
  QStringList accelerometerTitles();

  const QVector<qreal> &xPlotValues() { return m_xData; }
  const JSON::Frame &currentFrame() { return m_currentFrame; }
  const QVector<QVector<qreal>> &fftPlotValues() { return m_fftPlotValues; }
  const QVector<QVector<qreal>> &linearPlotValues()
  {
    return m_linearPlotValues;
  }

public slots:
  void resetData();
  void setPoints(const int points);
  void setPrecision(const int precision);
  void setBarVisible(const int index, const bool visible);
  void setFFTVisible(const int index, const bool visible);
  void setGpsVisible(const int index, const bool visible);
  void setLedVisible(const int index, const bool visible);
  void setPlotVisible(const int index, const bool visible);
  void setGroupVisible(const int index, const bool visible);
  void setGaugeVisible(const int index, const bool visible);
  void setCompassVisible(const int index, const bool visible);
  void setGyroscopeVisible(const int index, const bool visible);
  void setMultiplotVisible(const int index, const bool visible);
  void setAccelerometerVisible(const int index, const bool visible);

private slots:
  void updatePlots();
  void processLatestJSON(const QJsonObject &json);

private:
  QVector<JSON::Group> getLEDWidgets();
  QVector<JSON::Dataset> getFFTWidgets();
  QVector<JSON::Dataset> getPlotWidgets();
  QVector<JSON::Group> getWidgetGroups(const QString &handle);
  QVector<JSON::Dataset> getWidgetDatasets(const QString &handle);

  QStringList groupTitles(const QVector<JSON::Group> &vector);
  QStringList datasetTitles(const QVector<JSON::Dataset> &vector);

  bool getVisibility(const QVector<bool> &vector, const int index) const;
  void setVisibility(QVector<bool> &vector, const int index,
                     const bool visible);

private:
  int m_points;
  int m_precision;
  QVector<qreal> m_xData;
  QVector<QVector<qreal>> m_fftPlotValues;
  QVector<QVector<qreal>> m_linearPlotValues;
  QVector<QVector<QVector<qreal>>> m_multiplotValues;

  QVector<bool> m_barVisibility;
  QVector<bool> m_fftVisibility;
  QVector<bool> m_gpsVisibility;
  QVector<bool> m_ledVisibility;
  QVector<bool> m_plotVisibility;
  QVector<bool> m_groupVisibility;
  QVector<bool> m_gaugeVisibility;
  QVector<bool> m_compassVisibility;
  QVector<bool> m_gyroscopeVisibility;
  QVector<bool> m_multiPlotVisibility;
  QVector<bool> m_accelerometerVisibility;

  QVector<JSON::Dataset> m_barWidgets;
  QVector<JSON::Dataset> m_fftWidgets;
  QVector<JSON::Dataset> m_plotWidgets;
  QVector<JSON::Dataset> m_gaugeWidgets;
  QVector<JSON::Dataset> m_compassWidgets;

  QVector<JSON::Group> m_ledWidgets;
  QVector<JSON::Group> m_gpsWidgets;
  QVector<JSON::Group> m_groupWidgets;
  QVector<JSON::Group> m_multiPlotWidgets;
  QVector<JSON::Group> m_gyroscopeWidgets;
  QVector<JSON::Group> m_accelerometerWidgets;

  JSON::Frame m_currentFrame;
};
} // namespace UI
