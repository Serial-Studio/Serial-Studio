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
#include <UI/Dashboard.h>
#include <JSON/Generator.h>
#include <Misc/TimerEvents.h>

//----------------------------------------------------------------------------------------
// Constructor/deconstructor & singleton
//----------------------------------------------------------------------------------------

/**
 * Constructor of the class.
 */
UI::Dashboard::Dashboard()
    : m_points(100)
    , m_precision(2)
{
    // clang-format off
    connect(&CSV::Player::instance(), &CSV::Player::openChanged,
            this, &UI::Dashboard::resetData);
    connect(&IO::Manager::instance(), &IO::Manager::connectedChanged,
            this, &UI::Dashboard::resetData);
    connect(&JSON::Generator::instance(), &JSON::Generator::jsonChanged,
            this, &UI::Dashboard::processLatestJSON);
    connect(&JSON::Generator::instance(), &JSON::Generator::jsonFileMapChanged,
            this, &UI::Dashboard::resetData);
    // clang-format on
}

/**
 * Returns a pointer to the only instance of the class.
 */
UI::Dashboard &UI::Dashboard::instance()
{
    static Dashboard singleton;
    return singleton;
}

//----------------------------------------------------------------------------------------
// Group/Dataset access functions
//----------------------------------------------------------------------------------------

QFont UI::Dashboard::monoFont() const
{
    return QFont("Roboto Mono");
}

// clang-format off
const JSON::Group &UI::Dashboard::getLED(const int index) const           { return m_ledWidgets.at(index);           }
const JSON::Group &UI::Dashboard::getGPS(const int index) const           { return m_gpsWidgets.at(index);           }
const JSON::Dataset &UI::Dashboard::getBar(const int index) const         { return m_barWidgets.at(index);           }
const JSON::Dataset &UI::Dashboard::getFFT(const int index) const         { return m_fftWidgets.at(index);           }
const JSON::Dataset &UI::Dashboard::getPlot(const int index) const        { return m_plotWidgets.at(index);          }
const JSON::Group &UI::Dashboard::getGroups(const int index) const        { return m_groupWidgets.at(index);         }
const JSON::Dataset &UI::Dashboard::getGauge(const int index) const       { return m_gaugeWidgets.at(index);         }
const JSON::Group &UI::Dashboard::getGyroscope(const int index) const     { return m_gyroscopeWidgets.at(index);     }
const JSON::Dataset &UI::Dashboard::getCompass(const int index) const     { return m_compassWidgets.at(index);       }
const JSON::Group &UI::Dashboard::getMultiplot(const int index) const     { return m_multiPlotWidgets.at(index);     }
const JSON::Group &UI::Dashboard::getAccelerometer(const int index) const { return m_accelerometerWidgets.at(index); }
// clang-format on

//----------------------------------------------------------------------------------------
// Misc member access functions
//----------------------------------------------------------------------------------------

/**
 * Returns the title of the current JSON project/frame.
 */
QString UI::Dashboard::title()
{
    return m_latestFrame.title();
}

/**
 * Returns @c true if there is any data available to generate the QML dashboard.
 */
bool UI::Dashboard::available()
{
    return totalWidgetCount() > 0;
}

/**
 * Returns the number of points displayed by the graphs
 */
int UI::Dashboard::points() const
{
    return m_points;
}

/**
 * Returns the number of decimal digits displayed by the widgets
 */
int UI::Dashboard::precision() const
{
    return m_precision;
}

/**
 * Returns @c true if the current JSON frame is valid and ready-to-use by the QML
 * interface.
 */
bool UI::Dashboard::frameValid() const
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
int UI::Dashboard::totalWidgetCount() const
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
int UI::Dashboard::gpsCount() const           { return m_gpsWidgets.count();           }
int UI::Dashboard::ledCount() const           { return m_ledWidgets.count();           }
int UI::Dashboard::barCount() const           { return m_barWidgets.count();           }
int UI::Dashboard::fftCount() const           { return m_fftWidgets.count();           }
int UI::Dashboard::plotCount() const          { return m_plotWidgets.count();          }
int UI::Dashboard::gaugeCount() const         { return m_gaugeWidgets.count();         }
int UI::Dashboard::groupCount() const         { return m_groupWidgets.count();         }
int UI::Dashboard::compassCount() const       { return m_compassWidgets.count();       }
int UI::Dashboard::gyroscopeCount() const     { return m_gyroscopeWidgets.count();     }
int UI::Dashboard::multiPlotCount() const     { return m_multiPlotWidgets.count();     }
int UI::Dashboard::accelerometerCount() const { return m_accelerometerWidgets.count(); }
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
StringList UI::Dashboard::widgetTitles() const
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
int UI::Dashboard::relativeIndex(const int globalIndex) const
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
bool UI::Dashboard::widgetVisible(const int globalIndex) const
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
QString UI::Dashboard::widgetIcon(const int globalIndex) const
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
UI::Dashboard::WidgetType UI::Dashboard::widgetType(const int globalIndex) const
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
bool UI::Dashboard::barVisible(const int index) const           { return getVisibility(m_barVisibility, index);           }
bool UI::Dashboard::fftVisible(const int index) const           { return getVisibility(m_fftVisibility, index);           }
bool UI::Dashboard::gpsVisible(const int index) const           { return getVisibility(m_gpsVisibility, index);           }
bool UI::Dashboard::ledVisible(const int index) const           { return getVisibility(m_ledVisibility, index);           }
bool UI::Dashboard::plotVisible(const int index) const          { return getVisibility(m_plotVisibility, index);          }
bool UI::Dashboard::groupVisible(const int index) const         { return getVisibility(m_groupVisibility, index);         }
bool UI::Dashboard::gaugeVisible(const int index) const         { return getVisibility(m_gaugeVisibility, index);         }
bool UI::Dashboard::compassVisible(const int index) const       { return getVisibility(m_compassVisibility, index);       }
bool UI::Dashboard::gyroscopeVisible(const int index) const     { return getVisibility(m_gyroscopeVisibility, index);     }
bool UI::Dashboard::multiPlotVisible(const int index) const     { return getVisibility(m_multiPlotVisibility, index);     }
bool UI::Dashboard::accelerometerVisible(const int index) const { return getVisibility(m_accelerometerVisibility, index); }
// clang-format on

//----------------------------------------------------------------------------------------
// Widget title functions
//----------------------------------------------------------------------------------------

// clang-format off
StringList UI::Dashboard::gpsTitles() const           { return groupTitles(m_gpsWidgets);           }
StringList UI::Dashboard::ledTitles() const           { return groupTitles(m_ledWidgets);           }
StringList UI::Dashboard::groupTitles() const         { return groupTitles(m_groupWidgets);         }
StringList UI::Dashboard::barTitles() const           { return datasetTitles(m_barWidgets);         }
StringList UI::Dashboard::fftTitles() const           { return datasetTitles(m_fftWidgets);         }
StringList UI::Dashboard::plotTitles() const          { return datasetTitles(m_plotWidgets);        }
StringList UI::Dashboard::gaugeTitles() const         { return datasetTitles(m_gaugeWidgets);       }
StringList UI::Dashboard::compassTitles() const       { return datasetTitles(m_compassWidgets);     }
StringList UI::Dashboard::gyroscopeTitles() const     { return groupTitles(m_gyroscopeWidgets);     }
StringList UI::Dashboard::multiPlotTitles() const     { return groupTitles(m_multiPlotWidgets);     }
StringList UI::Dashboard::accelerometerTitles() const { return groupTitles(m_accelerometerWidgets); }
// clang-format on

//----------------------------------------------------------------------------------------
// Plot & widget options
//----------------------------------------------------------------------------------------

void UI::Dashboard::setPoints(const int points)
{
    if (m_points != points)
    {
        // Update number of points
        m_points = points;

        // Clear values
        m_fftPlotValues.clear();
        m_linearPlotValues.clear();

        // Regenerate x-axis values
        m_xData.resize(points);
        for (int i = 0; i < points; ++i)
            m_xData[i] = i;

        // Update plots
        Q_EMIT pointsChanged();
    }
}

void UI::Dashboard::setPrecision(const int precision)
{
    if (m_precision != precision)
    {
        m_precision = precision;
        Q_EMIT precisionChanged();
    }
}

//----------------------------------------------------------------------------------------
// Visibility-related slots
//----------------------------------------------------------------------------------------

// clang-format off
void UI::Dashboard::setBarVisible(const int i, const bool v)           { setVisibility(m_barVisibility, i, v);           }
void UI::Dashboard::setFFTVisible(const int i, const bool v)           { setVisibility(m_fftVisibility, i, v);           }
void UI::Dashboard::setGpsVisible(const int i, const bool v)           { setVisibility(m_gpsVisibility, i, v);           }
void UI::Dashboard::setLedVisible(const int i, const bool v)           { setVisibility(m_ledVisibility, i, v);           }
void UI::Dashboard::setPlotVisible(const int i, const bool v)          { setVisibility(m_plotVisibility, i, v);          }
void UI::Dashboard::setGroupVisible(const int i, const bool v)         { setVisibility(m_groupVisibility, i, v);         }
void UI::Dashboard::setGaugeVisible(const int i, const bool v)         { setVisibility(m_gaugeVisibility, i, v);         }
void UI::Dashboard::setCompassVisible(const int i, const bool v)       { setVisibility(m_compassVisibility, i, v);       }
void UI::Dashboard::setGyroscopeVisible(const int i, const bool v)     { setVisibility(m_gyroscopeVisibility, i, v);     }
void UI::Dashboard::setMultiplotVisible(const int i, const bool v)     { setVisibility(m_multiPlotVisibility, i, v);     }
void UI::Dashboard::setAccelerometerVisible(const int i, const bool v) { setVisibility(m_accelerometerVisibility, i, v); }
// clang-format on

//----------------------------------------------------------------------------------------
// Frame data handling slots
//----------------------------------------------------------------------------------------

/**
 * Removes all available data from the UI when the device is disconnected or the CSV
 * file is closed.
 */
void UI::Dashboard::resetData()
{
    // Make latest frame invalid
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
    Q_EMIT updated();
    Q_EMIT dataReset();
    Q_EMIT titleChanged();
    Q_EMIT widgetCountChanged();
    Q_EMIT widgetVisibilityChanged();
}

/**
 * Regenerates the data displayed on the dashboard plots
 */
void UI::Dashboard::updatePlots()
{
    // Initialize arrays that contain pointers to the
    // datasets that need to be plotted.
    QVector<JSON::Dataset> fftDatasets;
    QVector<JSON::Dataset> linearDatasets;

    // Create list with datasets that need to be graphed
    for (int i = 0; i < m_latestFrame.groupCount(); ++i)
    {
        auto group = m_latestFrame.groups().at(i);
        for (int j = 0; j < group.datasetCount(); ++j)
        {
            auto dataset = group.getDataset(j);
            if (dataset.fft())
                fftDatasets.append(dataset);
            if (dataset.graph())
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
            m_linearPlotValues.last().resize(points());

            // clang-format off
            std::fill(m_linearPlotValues.last().begin(),
                      m_linearPlotValues.last().end(),
                      0.0001);
            // clang-format on
        }
    }

    // Check if we need to update FFT dataset points
    if (m_fftPlotValues.count() != fftDatasets.count())
    {
        m_fftPlotValues.clear();

        for (int i = 0; i < fftDatasets.count(); ++i)
        {
            m_fftPlotValues.append(PlotData());
            m_fftPlotValues.last().resize(fftDatasets[i].fftSamples());

            // clang-format off
            std::fill(m_fftPlotValues.last().begin(),
                      m_fftPlotValues.last().end(),
                      0);
            // clang-format on
        }
    }

    // Append latest values to linear plot data
    for (int i = 0; i < linearDatasets.count(); ++i)
    {
        auto data = m_linearPlotValues[i].data();
        auto count = m_linearPlotValues[i].count();
        memmove(data, data + 1, count * sizeof(double));
        m_linearPlotValues[i][count - 1] = linearDatasets[i].value().toDouble();
    }

    // Append latest values to FFT plot data
    for (int i = 0; i < fftDatasets.count(); ++i)
    {
        auto data = m_fftPlotValues[i].data();
        auto count = m_fftPlotValues[i].count();
        memmove(data, data + 1, count * sizeof(double));
        m_fftPlotValues[i][count - 1] = fftDatasets[i].value().toDouble();
    }
}

/**
 * Regenerates the data displayed on the dashboard widgets
 */
void UI::Dashboard::processLatestJSON(const QJsonObject &json)
{
    // Save widget count
    const int barC = barCount();
    const int fftC = fftCount();
    const int gpsC = gpsCount();
    const int ledC = ledCount();
    const int plotC = plotCount();
    const int groupC = groupCount();
    const int gaugeC = gaugeCount();
    const int compassC = compassCount();
    const int gyroscopeC = gyroscopeCount();
    const int multiPlotC = multiPlotCount();
    const int accelerometerC = accelerometerCount();

    // Save previous title
    auto pTitle = title();

    // Try to read latest frame for widget updating
    if (!m_latestFrame.read(json))
        return;

    // Regenerate plot data
    updatePlots();

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
        Q_EMIT titleChanged();

    // Check if we need to regenerate widgets
    bool regenerateWidgets = false;
    regenerateWidgets |= (barC != barCount());
    regenerateWidgets |= (fftC != fftCount());
    regenerateWidgets |= (gpsC != gpsCount());
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
        m_barVisibility.resize(barCount());
        m_fftVisibility.resize(fftCount());
        m_gpsVisibility.resize(gpsCount());
        m_ledVisibility.resize(ledCount());
        m_plotVisibility.resize(plotCount());
        m_gaugeVisibility.resize(gaugeCount());
        m_groupVisibility.resize(groupCount());
        m_compassVisibility.resize(compassCount());
        m_gyroscopeVisibility.resize(gyroscopeCount());
        m_multiPlotVisibility.resize(multiPlotCount());
        m_accelerometerVisibility.resize(accelerometerCount());
        std::fill(m_barVisibility.begin(), m_barVisibility.end(), 1);
        std::fill(m_fftVisibility.begin(), m_fftVisibility.end(), 1);
        std::fill(m_gpsVisibility.begin(), m_gpsVisibility.end(), 1);
        std::fill(m_ledVisibility.begin(), m_ledVisibility.end(), 1);
        std::fill(m_plotVisibility.begin(), m_plotVisibility.end(), 1);
        std::fill(m_gaugeVisibility.begin(), m_gaugeVisibility.end(), 1);
        std::fill(m_groupVisibility.begin(), m_groupVisibility.end(), 1);
        std::fill(m_compassVisibility.begin(), m_compassVisibility.end(), 1);
        std::fill(m_gyroscopeVisibility.begin(), m_gyroscopeVisibility.end(), 1);
        std::fill(m_multiPlotVisibility.begin(), m_multiPlotVisibility.end(), 1);
        std::fill(m_accelerometerVisibility.begin(), m_accelerometerVisibility.end(), 1);

        Q_EMIT widgetCountChanged();
        Q_EMIT widgetVisibilityChanged();
    }

    // Update UI;
    Q_EMIT updated();
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
QVector<JSON::Group> UI::Dashboard::getLEDWidgets() const
{
    QVector<JSON::Dataset> widgets;
    Q_FOREACH (auto group, m_latestFrame.groups())
    {
        Q_FOREACH (auto dataset, group.datasets())
        {
            if (dataset.led())
                widgets.append(dataset);
        }
    }

    QVector<JSON::Group> groups;
    if (widgets.count() > 0)
    {
        JSON::Group group;
        group.m_title = tr("Status Panel");
        group.m_datasets = widgets;
        groups.append(group);
    }

    return groups;
}

/**
 * Returns a vector with all the datasets that need to be shown in the FFT widgets.
 */
QVector<JSON::Dataset> UI::Dashboard::getFFTWidgets() const
{
    QVector<JSON::Dataset> widgets;
    Q_FOREACH (auto group, m_latestFrame.groups())
    {
        Q_FOREACH (auto dataset, group.datasets())
        {
            if (dataset.fft())
                widgets.append(dataset);
        }
    }

    return widgets;
}

/**
 * Returns a vector with all the datasets that need to be plotted.
 */
QVector<JSON::Dataset> UI::Dashboard::getPlotWidgets() const
{
    QVector<JSON::Dataset> widgets;
    Q_FOREACH (auto group, m_latestFrame.groups())
    {
        Q_FOREACH (auto dataset, group.datasets())
        {
            if (dataset.graph())
                widgets.append(dataset);
        }
    }

    return widgets;
}

/**
 * Returns a vector with all the groups that implement the widget with the specied
 * @a handle.
 */
QVector<JSON::Group> UI::Dashboard::getWidgetGroups(const QString &handle) const
{
    QVector<JSON::Group> widgets;
    Q_FOREACH (auto group, m_latestFrame.groups())
    {
        if (group.widget() == handle)
            widgets.append(group);
    }

    return widgets;
}

/**
 * Returns a vector with all the datasets that implement a widget with the specified
 * @a handle.
 */
QVector<JSON::Dataset> UI::Dashboard::getWidgetDatasets(const QString &handle) const
{
    QVector<JSON::Dataset> widgets;
    Q_FOREACH (auto group, m_latestFrame.groups())
    {
        Q_FOREACH (auto dataset, group.datasets())
        {
            if (dataset.widget() == handle)
                widgets.append(dataset);
        }
    }

    return widgets;
}

/**
 * Returns the titles of the datasets contained in the specified @a vector.
 */
StringList UI::Dashboard::datasetTitles(const QVector<JSON::Dataset> &vector) const
{
    StringList list;
    Q_FOREACH (auto set, vector)
        list.append(set.title());

    return list;
}

/**
 * Returns the titles of the groups contained in the specified @a vector.
 */
StringList UI::Dashboard::groupTitles(const QVector<JSON::Group> &vector) const
{
    StringList list;
    Q_FOREACH (auto group, vector)
        list.append(group.title());

    return list;
}

/**
 * Returns @c true if the widget at the specifed @a index of the @a vector should be
 * displayed in the QML user interface.
 */
bool UI::Dashboard::getVisibility(const QVector<bool> &vector, const int index) const
{
    return vector[index];
}

/**
 * Changes the @a visible flag of the widget at the specified @a index of the given @a
 * vector. Calling this function with @a visible set to @c false will hide the widget in
 * the QML user interface.
 */
void UI::Dashboard::setVisibility(QVector<bool> &vector, const int index,
                                  const bool visible)
{
    vector[index] = visible;
    Q_EMIT widgetVisibilityChanged();
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_Dashboard.cpp"
#endif
