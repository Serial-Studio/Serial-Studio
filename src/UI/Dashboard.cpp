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

#include <IO/Manager.h>
#include <IO/Console.h>
#include <CSV/Player.h>
#include <JSON/Generator.h>
#include <Misc/TimerEvents.h>

#include "Dashboard.h"

namespace UI
{
static Dashboard *DASHBOARD = Q_NULLPTR;

//----------------------------------------------------------------------------------------
// Constructor/deconstructor & singleton
//----------------------------------------------------------------------------------------

/**
 * Constructor of the class.
 */
Dashboard::Dashboard()
    : m_points(100)
    , m_precision(2)
{
    const auto cp = CSV::Player::getInstance();
    const auto io = IO::Manager::getInstance();
    const auto ge = JSON::Generator::getInstance();
    const auto te = Misc::TimerEvents::getInstance();
    connect(cp, SIGNAL(openChanged()), this, SLOT(resetData()));
    connect(te, SIGNAL(highFreqTimeout()), this, SLOT(updateData()));
    connect(io, SIGNAL(connectedChanged()), this, SLOT(resetData()));
    connect(ge, SIGNAL(jsonFileMapChanged()), this, SLOT(resetData()));
    connect(ge, &JSON::Generator::jsonChanged, this, &Dashboard::processLatestJSON);
}

/**
 * Returns a pointer to the only instance of the class.
 */
Dashboard *Dashboard::getInstance()
{
    if (!DASHBOARD)
        DASHBOARD = new Dashboard();

    return DASHBOARD;
}

//----------------------------------------------------------------------------------------
// Group/Dataset access functions
//----------------------------------------------------------------------------------------

QFont Dashboard::monoFont() const
{
    return QFont("Roboto Mono");
}

// clang-format off
JSON::Group *Dashboard::getLED(const int index)           { return getGroupWidget(m_ledWidgets, index);           }
JSON::Group *Dashboard::getGPS(const int index)           { return getGroupWidget(m_gpsWidgets, index);           }
JSON::Dataset *Dashboard::getBar(const int index)         { return getDatasetWidget(m_barWidgets, index);         }
JSON::Dataset *Dashboard::getFFT(const int index)         { return getDatasetWidget(m_fftWidgets, index);         }
JSON::Dataset *Dashboard::getPlot(const int index)        { return getDatasetWidget(m_plotWidgets, index);        }
JSON::Group *Dashboard::getGroups(const int index)        { return getGroupWidget(m_groupWidgets, index);         }
JSON::Dataset *Dashboard::getGauge(const int index)       { return getDatasetWidget(m_gaugeWidgets, index);       }
JSON::Group *Dashboard::getGyroscope(const int index)     { return getGroupWidget(m_gyroscopeWidgets, index);     }
JSON::Dataset *Dashboard::getCompass(const int index)     { return getDatasetWidget(m_compassWidgets, index);     }
JSON::Group *Dashboard::getMultiplot(const int index)     { return getGroupWidget(m_multiPlotWidgets, index);     }
JSON::Group *Dashboard::getAccelerometer(const int index) { return getGroupWidget(m_accelerometerWidgets, index); }
// clang-format on

//----------------------------------------------------------------------------------------
// Misc member access functions
//----------------------------------------------------------------------------------------

/**
 * Returns the title of the current JSON project/frame.
 */
QString Dashboard::title()
{
    return m_latestFrame.title();
}

/**
 * Returns @c true if there is any data available to generate the QML dashboard.
 */
bool Dashboard::available()
{
    return totalWidgetCount() > 0;
}

/**
 * Returns the number of points displayed by the graphs
 */
int Dashboard::points() const
{
    return m_points;
}

/**
 * Returns the number of decimal digits displayed by the widgets
 */
int Dashboard::precision() const
{
    return m_precision;
}

/**
 * Returns @c true if the current JSON frame is valid and ready-to-use by the QML
 * interface.
 */
bool Dashboard::frameValid() const
{
    return m_latestFrame.isValid();
}

//----------------------------------------------------------------------------------------
// Widget count functions
//----------------------------------------------------------------------------------------

/**
 * Returns the total number of widgets that compose the current JSON frame.
 * This function is used as a helper to the functions that use the global-index widget
 * system.
 *
 * In the case of this function, we do not care about the order of the items that generate
 * the summatory count of all widgets. But in the other global-index functions we need to
 * be careful to sincronize the order of the widgets in order to allow the global-index
 * system to work correctly.
 */
int Dashboard::totalWidgetCount() const
{
    // clang-format off
    const int count =
            gpsCount() +
            ledCount() +
            barCount() +
            fftCount() +
            plotCount() +
            gaugeCount() +
            groupCount() +
            compassCount() +
            multiPlotCount() +
            gyroscopeCount() +
            accelerometerCount();
    // clang-format on

    return count;
}

// clang-format off
int Dashboard::gpsCount() const           { return m_gpsWidgets.count();           }
int Dashboard::ledCount() const           { return m_ledWidgets.count();           }
int Dashboard::barCount() const           { return m_barWidgets.count();           }
int Dashboard::fftCount() const           { return m_fftWidgets.count();           }
int Dashboard::plotCount() const          { return m_plotWidgets.count();          }
int Dashboard::gaugeCount() const         { return m_gaugeWidgets.count();         }
int Dashboard::groupCount() const         { return m_groupWidgets.count();         }
int Dashboard::compassCount() const       { return m_compassWidgets.count();       }
int Dashboard::gyroscopeCount() const     { return m_gyroscopeWidgets.count();     }
int Dashboard::multiPlotCount() const     { return m_multiPlotWidgets.count();     }
int Dashboard::accelerometerCount() const { return m_accelerometerWidgets.count(); }
// clang-format on

//----------------------------------------------------------------------------------------
// Relative-to-global widget index utility functions
//----------------------------------------------------------------------------------------

/**
 * Returns a @c list with the titles of all the widgets that compose the current JSON
 * frame.
 *
 * We need to be careful to sincronize the order of the widgets in order to allow
 * the global-index system to work correctly.
 */
StringList Dashboard::widgetTitles() const
{
    // Warning: maintain same order as the view option repeaters in ViewOptions.qml!

    // clang-format off
    return groupTitles() +
            multiPlotTitles() +
            ledTitles() +
            fftTitles() +
            plotTitles() +
            barTitles() +
            gaugeTitles() +
            compassTitles() +
            gyroscopeTitles() +
            accelerometerTitles() +
            gpsTitles();
    // clang-format on
}

/**
 * Returns the widget-specific index for the widget with the specified @a index.
 *
 * To simplify operations and user interface generation in QML, this class represents
 * the widgets in two manners:
 *
 * - A global list of all widgets
 * - A widget-specific list for each type of widget
 *
 * The global index allows us to use the @c WidgetLoader class to load any type of
 * widget, which reduces the need of implementing QML-specific code for each widget
 * that Serial Studio implements.
 *
 * The relative index is used by the visibility switches in the QML user interface.
 *
 * We need to be careful to sincronize the order of the widgets in order to allow
 * the global-index system to work correctly.
 */
int Dashboard::relativeIndex(const int globalIndex) const
{
    //
    // Warning: relative widget index should be calculated using the same order as defined
    // by the UI::Dashboard::widgetTitles() function.
    //

    // Check if we should return group widget
    int index = globalIndex;
    if (index < groupCount())
        return index;

    // Check if we should return multi-plot widget
    index -= groupCount();
    if (index < multiPlotCount())
        return index;

    // Check if we should return LED widget
    index -= multiPlotCount();
    if (index < ledCount())
        return index;

    // Check if we should return FFT widget
    index -= ledCount();
    if (index < fftCount())
        return index;

    // Check if we should return plot widget
    index -= fftCount();
    if (index < plotCount())
        return index;

    // Check if we should return bar widget
    index -= plotCount();
    if (index < barCount())
        return index;

    // Check if we should return gauge widget
    index -= barCount();
    if (index < gaugeCount())
        return index;

    // Check if we should return compass widget
    index -= gaugeCount();
    if (index < compassCount())
        return index;

    // Check if we should return gyro widget
    index -= compassCount();
    if (index < gyroscopeCount())
        return index;

    // Check if we should return accelerometer widget
    index -= gyroscopeCount();
    if (index < accelerometerCount())
        return index;

    // Check if we should return map widget
    index -= accelerometerCount();
    if (index < gpsCount())
        return index;

    // Return unknown widget
    return -1;
}

/**
 * Returns @c true if the widget with the specifed @a globalIndex should be
 * displayed in the QML user interface.
 *
 * To simplify operations and user interface generation in QML, this class represents
 * the widgets in two manners:
 *
 * - A global list of all widgets
 * - A widget-specific list for each type of widget
 *
 * The global index allows us to use the @c WidgetLoader class to load any type of
 * widget, which reduces the need of implementing QML-specific code for each widget
 * that Serial Studio implements.
 *
 * We need to be careful to sincronize the order of the widgets in order to allow
 * the global-index system to work correctly.
 */
bool Dashboard::widgetVisible(const int globalIndex) const
{
    bool visible = false;
    auto index = relativeIndex(globalIndex);

    switch (widgetType(globalIndex))
    {
        case WidgetType::Group:
            visible = groupVisible(index);
            break;
        case WidgetType::MultiPlot:
            visible = multiPlotVisible(index);
            break;
        case WidgetType::FFT:
            visible = fftVisible(index);
            break;
        case WidgetType::Plot:
            visible = plotVisible(index);
            break;
        case WidgetType::Bar:
            visible = barVisible(index);
            break;
        case WidgetType::Gauge:
            visible = gaugeVisible(index);
            break;
        case WidgetType::Compass:
            visible = compassVisible(index);
            break;
        case WidgetType::Gyroscope:
            visible = gyroscopeVisible(index);
            break;
        case WidgetType::Accelerometer:
            visible = accelerometerVisible(index);
            break;
        case WidgetType::GPS:
            visible = gpsVisible(index);
            break;
        case WidgetType::LED:
            visible = ledVisible(index);
            break;
        default:
            visible = false;
            break;
    }

    return visible;
}

/**
 * Returns the appropiate SVG-icon to load for the widget at the specified @a globalIndex.
 *
 * To simplify operations and user interface generation in QML, this class represents
 * the widgets in two manners:
 *
 * - A global list of all widgets
 * - A widget-specific list for each type of widget
 *
 * The global index allows us to use the @c WidgetLoader class to load any type of
 * widget, which reduces the need of implementing QML-specific code for each widget
 * that Serial Studio implements.
 *
 * We need to be careful to sincronize the order of the widgets in order to allow
 * the global-index system to work correctly.
 */
QString Dashboard::widgetIcon(const int globalIndex) const
{
    switch (widgetType(globalIndex))
    {
        case WidgetType::Group:
            return "qrc:/icons/group.svg";
            break;
        case WidgetType::MultiPlot:
            return "qrc:/icons/multiplot.svg";
            break;
        case WidgetType::FFT:
            return "qrc:/icons/fft.svg";
            break;
        case WidgetType::Plot:
            return "qrc:/icons/plot.svg";
            break;
        case WidgetType::Bar:
            return "qrc:/icons/bar.svg";
            break;
        case WidgetType::Gauge:
            return "qrc:/icons/gauge.svg";
            break;
        case WidgetType::Compass:
            return "qrc:/icons/compass.svg";
            break;
        case WidgetType::Gyroscope:
            return "qrc:/icons/gyro.svg";
            break;
        case WidgetType::Accelerometer:
            return "qrc:/icons/accelerometer.svg";
            break;
        case WidgetType::GPS:
            return "qrc:/icons/gps.svg";
            break;
        case WidgetType::LED:
            return "qrc:/icons/led.svg";
            break;
        default:
            return "qrc:/icons/close.svg";
            break;
    }
}

/**
 * Returns the type of widget that corresponds to the given @a globalIndex.
 *
 * Possible return values are:
 * - @c WidgetType::Unknown
 * - @c WidgetType::Group
 * - @c WidgetType::MultiPlot
 * - @c WidgetType::FFT
 * - @c WidgetType::Plot
 * - @c WidgetType::Bar
 * - @c WidgetType::Gauge
 * - @c WidgetType::Compass
 * - @c WidgetType::Gyroscope
 * - @c WidgetType::Accelerometer
 * - @c WidgetType::GPS
 * - @c WidgetType::LED
 *
 * To simplify operations and user interface generation in QML, this class represents
 * the widgets in two manners:
 *
 * - A global list of all widgets
 * - A widget-specific list for each type of widget
 *
 * The global index allows us to use the @c WidgetLoader class to load any type of
 * widget, which reduces the need of implementing QML-specific code for each widget
 * that Serial Studio implements.
 *
 * We need to be careful to sincronize the order of the widgets in order to allow
 * the global-index system to work correctly.
 */
UI::Dashboard::WidgetType Dashboard::widgetType(const int globalIndex) const
{
    //
    // Warning: relative widget index should be calculated using the same order as defined
    // by the UI::Dashboard::widgetTitles() function.
    //

    // Unitialized widget loader class
    if (globalIndex < 0)
        return WidgetType::Unknown;

    // Check if we should return group widget
    int index = globalIndex;
    if (index < groupCount())
        return WidgetType::Group;

    // Check if we should return multi-plot widget
    index -= groupCount();
    if (index < multiPlotCount())
        return WidgetType::MultiPlot;

    // Check if we should return LED widget
    index -= multiPlotCount();
    if (index < ledCount())
        return WidgetType::LED;

    // Check if we should return FFT widget
    index -= ledCount();
    if (index < fftCount())
        return WidgetType::FFT;

    // Check if we should return plot widget
    index -= fftCount();
    if (index < plotCount())
        return WidgetType::Plot;

    // Check if we should return bar widget
    index -= plotCount();
    if (index < barCount())
        return WidgetType::Bar;

    // Check if we should return gauge widget
    index -= barCount();
    if (index < gaugeCount())
        return WidgetType::Gauge;

    // Check if we should return compass widget
    index -= gaugeCount();
    if (index < compassCount())
        return WidgetType::Compass;

    // Check if we should return gyro widget
    index -= compassCount();
    if (index < gyroscopeCount())
        return WidgetType::Gyroscope;

    // Check if we should return accelerometer widget
    index -= gyroscopeCount();
    if (index < accelerometerCount())
        return WidgetType::Accelerometer;

    // Check if we should return map widget
    index -= accelerometerCount();
    if (index < gpsCount())
        return WidgetType::GPS;

    // Return unknown widget
    return WidgetType::Unknown;
}

//----------------------------------------------------------------------------------------
// Widget visibility access functions
//----------------------------------------------------------------------------------------

// clang-format off
bool Dashboard::barVisible(const int index) const           { return getVisibility(m_barVisibility, index);           }
bool Dashboard::fftVisible(const int index) const           { return getVisibility(m_fftVisibility, index);           }
bool Dashboard::gpsVisible(const int index) const           { return getVisibility(m_gpsVisibility, index);           }
bool Dashboard::ledVisible(const int index) const           { return getVisibility(m_ledVisibility, index);           }
bool Dashboard::plotVisible(const int index) const          { return getVisibility(m_plotVisibility, index);          }
bool Dashboard::groupVisible(const int index) const         { return getVisibility(m_groupVisibility, index);         }
bool Dashboard::gaugeVisible(const int index) const         { return getVisibility(m_gaugeVisibility, index);         }
bool Dashboard::compassVisible(const int index) const       { return getVisibility(m_compassVisibility, index);       }
bool Dashboard::gyroscopeVisible(const int index) const     { return getVisibility(m_gyroscopeVisibility, index);     }
bool Dashboard::multiPlotVisible(const int index) const     { return getVisibility(m_multiPlotVisibility, index);     }
bool Dashboard::accelerometerVisible(const int index) const { return getVisibility(m_accelerometerVisibility, index); }
// clang-format on

//----------------------------------------------------------------------------------------
// Widget title functions
//----------------------------------------------------------------------------------------

// clang-format off
StringList Dashboard::gpsTitles() const           { return groupTitles(m_gpsWidgets);           }
StringList Dashboard::ledTitles() const           { return groupTitles(m_ledWidgets);           }
StringList Dashboard::groupTitles() const         { return groupTitles(m_groupWidgets);         }
StringList Dashboard::barTitles() const           { return datasetTitles(m_barWidgets);         }
StringList Dashboard::fftTitles() const           { return datasetTitles(m_fftWidgets);         }
StringList Dashboard::plotTitles() const          { return datasetTitles(m_plotWidgets);        }
StringList Dashboard::gaugeTitles() const         { return datasetTitles(m_gaugeWidgets);       }
StringList Dashboard::compassTitles() const       { return datasetTitles(m_compassWidgets);     }
StringList Dashboard::gyroscopeTitles() const     { return groupTitles(m_gyroscopeWidgets);     }
StringList Dashboard::multiPlotTitles() const     { return groupTitles(m_multiPlotWidgets);     }
StringList Dashboard::accelerometerTitles() const { return groupTitles(m_accelerometerWidgets); }
// clang-format on

//----------------------------------------------------------------------------------------
// Plot & widget options
//----------------------------------------------------------------------------------------

void Dashboard::setPoints(const int points)
{
    if (m_points != points)
    {
        // Update number of points
        m_points = points;

        // Clear values
        m_fftPlotValues.clear();
        m_linearPlotValues.clear();

        // Regenerate x-axis values
        m_xData.clear();
        m_xData.reserve(points);
        for (int i = 0; i < points; ++i)
            m_xData.append(i);

        // Update plots
        emit pointsChanged();
    }
}

void Dashboard::setPrecision(const int precision)
{
    if (m_precision != precision)
    {
        m_precision = precision;
        emit precisionChanged();
    }
}

//----------------------------------------------------------------------------------------
// Visibility-related slots
//----------------------------------------------------------------------------------------

// clang-format off
void Dashboard::setBarVisible(const int i, const bool v)           { setVisibility(m_barVisibility, i, v);           }
void Dashboard::setFFTVisible(const int i, const bool v)           { setVisibility(m_fftVisibility, i, v);           }
void Dashboard::setGpsVisible(const int i, const bool v)           { setVisibility(m_gpsVisibility, i, v);           }
void Dashboard::setLedVisible(const int i, const bool v)           { setVisibility(m_ledVisibility, i, v);           }
void Dashboard::setPlotVisible(const int i, const bool v)          { setVisibility(m_plotVisibility, i, v);          }
void Dashboard::setGroupVisible(const int i, const bool v)         { setVisibility(m_groupVisibility, i, v);         }
void Dashboard::setGaugeVisible(const int i, const bool v)         { setVisibility(m_gaugeVisibility, i, v);         }
void Dashboard::setCompassVisible(const int i, const bool v)       { setVisibility(m_compassVisibility, i, v);       }
void Dashboard::setGyroscopeVisible(const int i, const bool v)     { setVisibility(m_gyroscopeVisibility, i, v);     }
void Dashboard::setMultiplotVisible(const int i, const bool v)     { setVisibility(m_multiPlotVisibility, i, v);     }
void Dashboard::setAccelerometerVisible(const int i, const bool v) { setVisibility(m_accelerometerVisibility, i, v); }
// clang-format on

//----------------------------------------------------------------------------------------
// Frame data handling slots
//----------------------------------------------------------------------------------------

/**
 * Removes all available data from the UI when the device is disconnected or the CSV
 * file is closed.
 */
void Dashboard::resetData()
{
    // Make latest frame invalid
    m_jsonList.clear();
    m_latestFrame.read(QJsonObject {});

    // Clear plot data
    m_fftPlotValues.clear();
    m_linearPlotValues.clear();

    // Clear widget data
    m_barWidgets.clear();
    m_fftWidgets.clear();
    m_gpsWidgets.clear();
    m_ledWidgets.clear();
    m_plotWidgets.clear();
    m_gaugeWidgets.clear();
    m_groupWidgets.clear();
    m_compassWidgets.clear();
    m_gyroscopeWidgets.clear();
    m_multiPlotWidgets.clear();
    m_accelerometerWidgets.clear();

    // Clear widget visibility data
    m_barVisibility.clear();
    m_fftVisibility.clear();
    m_gpsVisibility.clear();
    m_ledVisibility.clear();
    m_plotVisibility.clear();
    m_gaugeVisibility.clear();
    m_groupVisibility.clear();
    m_compassVisibility.clear();
    m_gyroscopeVisibility.clear();
    m_multiPlotVisibility.clear();
    m_accelerometerVisibility.clear();

    // Update UI
    emit updated();
    emit dataReset();
    emit titleChanged();
    emit widgetCountChanged();
    emit widgetVisibilityChanged();
}

/**
 * Interprets the most recent JSON frame & signals the UI to regenerate itself.
 */
void Dashboard::updateData()
{
    // Check if we have anything to read
    if (m_jsonList.isEmpty())
        return;

    // Sort JSON list
    JFI_SortList(&m_jsonList);

    // Save widget count
    const int barC = barCount();
    const int fftC = fftCount();
    const int mapC = gpsCount();
    const int ledC = ledCount();
    const int plotC = plotCount();
    const int groupC = groupCount();
    const int gaugeC = gaugeCount();
    const int compassC = compassCount();
    const int gyroscopeC = gyroscopeCount();
    const int multiPlotC = multiPlotCount();
    const int accelerometerC = accelerometerCount();

    // Save previous title
    const auto pTitle = title();

    // Try to read latest frame for widget updating
    auto lastJson = m_jsonList.last();
    if (!m_latestFrame.read(lastJson.jsonDocument.object()))
        return;

    // Clear widget data
    m_barWidgets.clear();
    m_fftWidgets.clear();
    m_gpsWidgets.clear();
    m_ledWidgets.clear();
    m_plotWidgets.clear();
    m_gaugeWidgets.clear();
    m_groupWidgets.clear();
    m_compassWidgets.clear();
    m_gyroscopeWidgets.clear();
    m_multiPlotWidgets.clear();
    m_accelerometerWidgets.clear();

    // Regenerate plot data
    updatePlots();

    // Remove previous values from JSON list
    m_jsonList.clear();

    // Update widget vectors
    m_fftWidgets = getFFTWidgets();
    m_ledWidgets = getLEDWidgets();
    m_plotWidgets = getPlotWidgets();
    m_groupWidgets = getWidgetGroups("");
    m_gpsWidgets = getWidgetGroups("map");
    m_barWidgets = getWidgetDatasets("bar");
    m_gaugeWidgets = getWidgetDatasets("gauge");
    m_gyroscopeWidgets = getWidgetGroups("gyro");
    m_compassWidgets = getWidgetDatasets("compass");
    m_multiPlotWidgets = getWidgetGroups("multiplot");
    m_accelerometerWidgets = getWidgetGroups("accelerometer");

    // Add accelerometer widgets to multiplot
    for (int i = 0; i < m_accelerometerWidgets.count(); ++i)
        m_multiPlotWidgets.append(m_accelerometerWidgets.at(i));

    // Add gyroscope widgets to multiplot
    for (int i = 0; i < m_gyroscopeWidgets.count(); ++i)
        m_multiPlotWidgets.append(m_gyroscopeWidgets.at(i));

    // Check if we need to update title
    if (pTitle != title())
        emit titleChanged();

    // Check if we need to regenerate widgets
    bool regenerateWidgets = false;
    regenerateWidgets |= (barC != barCount());
    regenerateWidgets |= (fftC != fftCount());
    regenerateWidgets |= (mapC != gpsCount());
    regenerateWidgets |= (ledC != ledCount());
    regenerateWidgets |= (plotC != plotCount());
    regenerateWidgets |= (gaugeC != gaugeCount());
    regenerateWidgets |= (groupC != groupCount());
    regenerateWidgets |= (compassC != compassCount());
    regenerateWidgets |= (gyroscopeC != gyroscopeCount());
    regenerateWidgets |= (multiPlotC != multiPlotCount());
    regenerateWidgets |= (accelerometerC != accelerometerCount());

    // Regenerate widget visiblity models
    if (regenerateWidgets)
    {
        m_barVisibility.clear();
        m_fftVisibility.clear();
        m_gpsVisibility.clear();
        m_ledVisibility.clear();
        m_plotVisibility.clear();
        m_gaugeVisibility.clear();
        m_groupVisibility.clear();
        m_compassVisibility.clear();
        m_gyroscopeVisibility.clear();
        m_multiPlotVisibility.clear();
        m_accelerometerVisibility.clear();

        int i;
        for (i = 0; i < barCount(); ++i)
            m_barVisibility.append(true);
        for (i = 0; i < fftCount(); ++i)
            m_fftVisibility.append(true);
        for (i = 0; i < gpsCount(); ++i)
            m_gpsVisibility.append(true);
        for (i = 0; i < ledCount(); ++i)
            m_ledVisibility.append(true);
        for (i = 0; i < plotCount(); ++i)
            m_plotVisibility.append(true);
        for (i = 0; i < gaugeCount(); ++i)
            m_gaugeVisibility.append(true);
        for (i = 0; i < groupCount(); ++i)
            m_groupVisibility.append(true);
        for (i = 0; i < compassCount(); ++i)
            m_compassVisibility.append(true);
        for (i = 0; i < gyroscopeCount(); ++i)
            m_gyroscopeVisibility.append(true);
        for (i = 0; i < multiPlotCount(); ++i)
            m_multiPlotVisibility.append(true);
        for (i = 0; i < accelerometerCount(); ++i)
            m_accelerometerVisibility.append(true);

        emit widgetCountChanged();
        emit widgetVisibilityChanged();
    }

    // Update UI;
    emit updated();
}

void Dashboard::updatePlots()
{
    // Initialize arrays that contain pointers to the
    // datasets that need to be plotted.
    QVector<JSON::Dataset *> fftDatasets;
    QVector<JSON::Dataset *> linearDatasets;

    // Register all plot values for each frame
    for (int f = 0; f < m_jsonList.count(); ++f)
    {
        // Clear dataset & latest values list
        fftDatasets.clear();
        linearDatasets.clear();

        // Get frame, abort if frame is invalid
        JSON::Frame frame;
        if (!frame.read(m_jsonList.at(f).jsonDocument.object()))
            continue;

        // Create list with datasets that need to be graphed
        for (int i = 0; i < frame.groupCount(); ++i)
        {
            const auto group = frame.groups().at(i);
            for (int j = 0; j < group->datasetCount(); ++j)
            {
                auto dataset = group->datasets().at(j);
                if (dataset->fft())
                    fftDatasets.append(dataset);
                if (dataset->graph())
                    linearDatasets.append(dataset);
            }
        }

        // Check if we need to update dataset points
        if (m_linearPlotValues.count() != linearDatasets.count())
        {
            m_linearPlotValues.clear();

            for (int i = 0; i < linearDatasets.count(); ++i)
            {
                m_linearPlotValues.append(PlotData());
                m_linearPlotValues.last().reserve(points());
                for (int j = 0; j < points(); ++j)
                    m_linearPlotValues[i].append(0.0001);
            }
        }

        // Check if we need to update FFT dataset points
        if (m_fftPlotValues.count() != fftDatasets.count())
        {
            m_fftPlotValues.clear();

            for (int i = 0; i < fftDatasets.count(); ++i)
            {
                m_fftPlotValues.append(PlotData());
                m_fftPlotValues.last().reserve(fftDatasets[i]->fftSamples());
                for (int j = 0; j < fftDatasets[i]->fftSamples(); ++j)
                    m_fftPlotValues[i].append(0);
            }
        }

        // Append latest values to linear plot data
        for (int i = 0; i < linearDatasets.count(); ++i)
        {
            auto data = m_linearPlotValues[i].data();
            auto count = m_linearPlotValues[i].count();
            memmove(data, data + 1, count * sizeof(double));
            m_linearPlotValues[i][count - 1] = linearDatasets[i]->value().toDouble();
        }

        // Append latest values to FFT plot data
        for (int i = 0; i < fftDatasets.count(); ++i)
        {
            auto data = m_fftPlotValues[i].data();
            auto count = m_fftPlotValues[i].count();
            memmove(data, data + 1, count * sizeof(double));
            m_fftPlotValues[i][count - 1] = fftDatasets[i]->value().toDouble();
        }
    }
}

/**
 * Registers the JSON frame to the list of JSON frame vectors, which is later used to
 * update widgets & graphs
 */
void Dashboard::processLatestJSON(const JFI_Object &frameInfo)
{
    m_jsonList.append(frameInfo);
}

//----------------------------------------------------------------------------------------
// Widget utility functions
//----------------------------------------------------------------------------------------

/**
 * Returns a group with all the datasets that need to be shown in the LED status panel.
 *
 * @note We return a vector with a single group item because we want to display a title on
 * the window without breaking the current software architecture.
 */
QVector<JSON::Group *> Dashboard::getLEDWidgets() const
{
    QVector<JSON::Dataset *> widgets;
    foreach (auto group, m_latestFrame.groups())
    {
        foreach (auto dataset, group->datasets())
        {
            if (dataset->led())
                widgets.append(dataset);
        }
    }

    QVector<JSON::Group *> groups;
    if (widgets.count() > 0)
    {
        JSON::Group *group = new JSON::Group();
        group->m_title = tr("Status Panel");
        group->m_datasets = widgets;
        groups.append(group);
    }

    return groups;
}

/**
 * Returns a vector with all the datasets that need to be shown in the FFT widgets.
 */
QVector<JSON::Dataset *> Dashboard::getFFTWidgets() const
{
    QVector<JSON::Dataset *> widgets;
    foreach (auto group, m_latestFrame.groups())
    {
        foreach (auto dataset, group->datasets())
        {
            if (dataset->fft())
                widgets.append(dataset);
        }
    }

    return widgets;
}

/**
 * Returns a vector with all the datasets that need to be plotted.
 */
QVector<JSON::Dataset *> Dashboard::getPlotWidgets() const
{
    QVector<JSON::Dataset *> widgets;
    foreach (auto group, m_latestFrame.groups())
    {
        foreach (auto dataset, group->datasets())
        {
            if (dataset->graph())
                widgets.append(dataset);
        }
    }

    return widgets;
}

/**
 * Returns a vector with all the groups that implement the widget with the specied
 * @a handle.
 */
QVector<JSON::Group *> Dashboard::getWidgetGroups(const QString &handle) const
{
    QVector<JSON::Group *> widgets;
    foreach (auto group, m_latestFrame.groups())
    {
        if (group->widget() == handle)
            widgets.append(group);
    }

    return widgets;
}

/**
 * Returns a vector with all the datasets that implement a widget with the specified
 * @a handle.
 */
QVector<JSON::Dataset *> Dashboard::getWidgetDatasets(const QString &handle) const
{
    QVector<JSON::Dataset *> widgets;
    foreach (auto group, m_latestFrame.groups())
    {
        foreach (auto dataset, group->datasets())
        {
            if (dataset->widget() == handle)
                widgets.append(dataset);
        }
    }

    return widgets;
}

/**
 * Returns the titles of the datasets contained in the specified @a vector.
 */
StringList Dashboard::datasetTitles(const QVector<JSON::Dataset *> &vector) const
{
    StringList list;
    foreach (auto set, vector)
        list.append(set->title());

    return list;
}

/**
 * Returns the titles of the groups contained in the specified @a vector.
 */
StringList Dashboard::groupTitles(const QVector<JSON::Group *> &vector) const
{
    StringList list;
    foreach (auto group, vector)
        list.append(group->title());

    return list;
}

/**
 * Returns @c true if the widget at the specifed @a index of the @a vector should be
 * displayed in the QML user interface.
 */
bool Dashboard::getVisibility(const QVector<bool> &vector, const int index) const
{
    if (index < vector.count())
        return vector[index];

    return false;
}

/**
 * Changes the @a visible flag of the widget at the specified @a index of the given @a
 * vector. Calling this function with @a visible set to @c false will hide the widget in
 * the QML user interface.
 */
void Dashboard::setVisibility(QVector<bool> &vector, const int index, const bool visible)
{
    if (index < vector.count())
    {
        vector[index] = visible;
        emit widgetVisibilityChanged();
    }
}

/**
 * Returns a pointer to the group at the specified @a index of the given @a vector.
 * If the @a index is invalid, then this function shall return a NULL pointer.
 */
JSON::Group *Dashboard::getGroupWidget(const QVector<JSON::Group *> vector,
                                       const int index)
{
    if (index < vector.count())
        return vector.at(index);

    return Q_NULLPTR;
}

/**
 * Returns a pointer to the dataset at the specified @a index of the given @a vector.
 * If the @a index is invalid, then this function shall return a NULL pointer.
 */
JSON::Dataset *Dashboard::getDatasetWidget(const QVector<JSON::Dataset *> vector,
                                           const int index)
{
    if (index < vector.count())
        return vector.at(index);

    return Q_NULLPTR;
}
}
