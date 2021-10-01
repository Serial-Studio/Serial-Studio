/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

#ifndef UI_DASHBOARD_H
#define UI_DASHBOARD_H

#include <QFont>
#include <QObject>
#include <JSON/Frame.h>
#include <JSON/FrameInfo.h>

namespace UI
{
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
    Q_PROPERTY(int totalWidgetCount
               READ totalWidgetCount
               NOTIFY widgetCountChanged)
    Q_PROPERTY(int mapCount
               READ mapCount
               NOTIFY widgetCountChanged)
    Q_PROPERTY(int barCount
               READ barCount
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
    Q_PROPERTY(int thermometerCount
               READ thermometerCount
               NOTIFY widgetCountChanged)
    Q_PROPERTY(int accelerometerCount
               READ accelerometerCount
               NOTIFY widgetCountChanged)
    Q_PROPERTY(QStringList mapTitles
               READ mapTitles
               NOTIFY widgetCountChanged)
    Q_PROPERTY(QStringList barTitles
               READ barTitles
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
    Q_PROPERTY(QStringList thermometerTitles
               READ thermometerTitles
               NOTIFY widgetCountChanged)
    Q_PROPERTY(QStringList accelerometerTitles
               READ accelerometerTitles
               NOTIFY widgetCountChanged)
    // clang-format on

signals:
    void updated();
    void dataReset();
    void titleChanged();
    void widgetCountChanged();
    void widgetVisibilityChanged();

public:
    enum class WidgetType
    {
        Group,
        MultiPlot,
        Plot,
        Bar,
        Gauge,
        Thermometer,
        Compass,
        Gyroscope,
        Accelerometer,
        Map,
        Unknown
    };
    Q_ENUM(WidgetType)

    static Dashboard *getInstance();

    QFont monoFont() const;
    JSON::Group *getMap(const int index);
    JSON::Dataset *getBar(const int index);
    JSON::Group *getGroups(const int index);
    JSON::Dataset *getPlot(const int index);
    JSON::Dataset *getGauge(const int index);
    JSON::Group *getGyroscope(const int index);
    JSON::Dataset *getCompass(const int index);
    JSON::Group *getMultiplot(const int index);
    JSON::Group *getAccelerometer(const int index);
    JSON::Dataset *getThermometer(const int index);

    QString title();
    bool available();

    int totalWidgetCount() const;
    int mapCount() const;
    int barCount() const;
    int plotCount() const;
    int groupCount() const;
    int gaugeCount() const;
    int compassCount() const;
    int gyroscopeCount() const;
    int multiPlotCount() const;
    int thermometerCount() const;
    int accelerometerCount() const;

    Q_INVOKABLE bool frameValid() const;
    Q_INVOKABLE QStringList widgetTitles() const;
    Q_INVOKABLE int relativeIndex(const int globalIndex) const;
    Q_INVOKABLE bool widgetVisible(const int globalIndex) const;
    Q_INVOKABLE QString widgetIcon(const int globalIndex) const;
    Q_INVOKABLE UI::Dashboard::WidgetType widgetType(const int globalIndex) const;

    Q_INVOKABLE bool barVisible(const int index) const;
    Q_INVOKABLE bool mapVisible(const int index) const;
    Q_INVOKABLE bool plotVisible(const int index) const;
    Q_INVOKABLE bool groupVisible(const int index) const;
    Q_INVOKABLE bool gaugeVisible(const int index) const;
    Q_INVOKABLE bool compassVisible(const int index) const;
    Q_INVOKABLE bool gyroscopeVisible(const int index) const;
    Q_INVOKABLE bool multiPlotVisible(const int index) const;
    Q_INVOKABLE bool thermometerVisible(const int index) const;
    Q_INVOKABLE bool accelerometerVisible(const int index) const;

    Q_INVOKABLE QStringList barTitles() const;
    Q_INVOKABLE QStringList mapTitles() const;
    Q_INVOKABLE QStringList plotTitles() const;
    Q_INVOKABLE QStringList groupTitles() const;
    Q_INVOKABLE QStringList gaugeTitles() const;
    Q_INVOKABLE QStringList compassTitles() const;
    Q_INVOKABLE QStringList gyroscopeTitles() const;
    Q_INVOKABLE QStringList multiPlotTitles() const;
    Q_INVOKABLE QStringList thermometerTitles() const;
    Q_INVOKABLE QStringList accelerometerTitles() const;

public slots:
    void setBarVisible(const int index, const bool visible);
    void setMapVisible(const int index, const bool visible);
    void setPlotVisible(const int index, const bool visible);
    void setGroupVisible(const int index, const bool visible);
    void setGaugeVisible(const int index, const bool visible);
    void setCompassVisible(const int index, const bool visible);
    void setGyroscopeVisible(const int index, const bool visible);
    void setMultiplotVisible(const int index, const bool visible);
    void setThermometerVisible(const int index, const bool visible);
    void setAccelerometerVisible(const int index, const bool visible);

private slots:
    void resetData();
    void updateData();
    void selectLatestJSON(const JFI_Object &frameInfo);

private:
    Dashboard();

    QVector<JSON::Dataset *> getPlotWidgets() const;
    QVector<JSON::Group *> getWidgetGroups(const QString &handle) const;
    QVector<JSON::Dataset *> getWidgetDatasets(const QString &handle) const;

    QStringList groupTitles(const QVector<JSON::Group *> &vector) const;
    QStringList datasetTitles(const QVector<JSON::Dataset *> &vector) const;

    bool getVisibility(const QVector<bool> &vector, const int index) const;
    void setVisibility(QVector<bool> &vector, const int index, const bool visible);

    JSON::Group *getGroupWidget(const QVector<JSON::Group *> vector, const int index);
    JSON::Dataset *getDatasetWidget(const QVector<JSON::Dataset *> vector,
                                    const int index);

private:
    QVector<bool> m_barVisibility;
    QVector<bool> m_mapVisibility;
    QVector<bool> m_plotVisibility;
    QVector<bool> m_groupVisibility;
    QVector<bool> m_gaugeVisibility;
    QVector<bool> m_compassVisibility;
    QVector<bool> m_gyroscopeVisibility;
    QVector<bool> m_multiPlotVisibility;
    QVector<bool> m_thermometerVisibility;
    QVector<bool> m_accelerometerVisibility;

    QVector<JSON::Dataset *> m_barWidgets;
    QVector<JSON::Dataset *> m_plotWidgets;
    QVector<JSON::Dataset *> m_gaugeWidgets;
    QVector<JSON::Dataset *> m_compassWidgets;
    QVector<JSON::Dataset *> m_thermometerWidgets;

    QVector<JSON::Group *> m_mapWidgets;
    QVector<JSON::Group *> m_multiPlotWidgets;
    QVector<JSON::Group *> m_gyroscopeWidgets;
    QVector<JSON::Group *> m_accelerometerWidgets;

    JSON::Frame m_latestFrame;
    JFI_Object m_latestJsonFrame;
};
}

#endif
