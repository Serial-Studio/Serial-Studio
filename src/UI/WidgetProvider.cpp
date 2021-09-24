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

#include "DataProvider.h"
#include "WidgetProvider.h"

#include <cfloat>
#include <climits>
#include <CSV/Player.h>
#include <IO/Manager.h>
#include <IO/Console.h>
#include <JSON/Generator.h>
#include <Misc/TimerEvents.h>

using namespace UI;

/*
 * Pointer to the only instance of the class
 */
static WidgetProvider *INSTANCE = Q_NULLPTR;

/**
 * Initialization code of the @c Widgets class
 */
WidgetProvider::WidgetProvider()
    : m_widgetCount(0)
{
    // Module signals/slots
    auto cp = CSV::Player::getInstance();
    auto io = IO::Manager::getInstance();
    auto dp = DataProvider::getInstance();
    auto jg = JSON::Generator::getInstance();
    connect(dp, SIGNAL(updated()), this, SLOT(updateModels()));
    connect(cp, SIGNAL(openChanged()), this, SLOT(resetData()));
    connect(io, SIGNAL(connectedChanged()), this, SLOT(resetData()));
    connect(jg, SIGNAL(jsonFileMapChanged()), this, SLOT(resetData()));
}

/**
 * Returns a pointer to the only instance of the class
 */
WidgetProvider *WidgetProvider::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new WidgetProvider;

    return INSTANCE;
}

/**
 * Returns a list with all the JSON datasets that implement a bar widget
 */
QList<JSON::Dataset *> WidgetProvider::barDatasets() const
{
    return m_barDatasets;
}

/**
 * Returns a list with all the JSON datasets that implement a gauge widget
 */
QList<JSON::Dataset *> WidgetProvider::gaugeDatasets() const
{
    return m_gaugeDatasets;
}

/**
 * Returns a list with all the JSON datasets that implement a compass widget
 */
QList<JSON::Dataset *> WidgetProvider::compassDatasets() const
{
    return m_compassDatasets;
}

/**
 * Returns a list with all the JSON groups that implement a map widget
 */
QList<JSON::Group *> WidgetProvider::mapGroup() const
{
    return m_mapGroups;
}

/**
 * Returns a list with all the JSON groups that implement a gyro widget
 */
QList<JSON::Group *> WidgetProvider::gyroGroup() const
{
    return m_gyroGroups;
}

/**
 * Returns a list with all the JSON groups that implement an accelerometer
 * widget
 */
QList<JSON::Group *> WidgetProvider::accelerometerGroup() const
{
    return m_accelerometerGroups;
}

/**
 * Returns the total number of widgets that should be generated
 */
int WidgetProvider::totalWidgetCount() const
{
    // clang-format off
    return mapGroupCount() +
            gyroGroupCount() +
            barDatasetCount() +
            gaugeDatasetCount() +
            compassDatasetCount() +
            accelerometerGroupCount();
    // clang-format on
}

/**
 * Returns a list with all the available widgets
 */
QStringList WidgetProvider::widgetNames() const
{
    QStringList names;

    foreach (auto widget, m_mapGroups)
        names.append(widget->title());

    foreach (auto widget, m_gyroGroups)
        names.append(widget->title());

    foreach (auto widget, m_accelerometerGroups)
        names.append(widget->title());

    foreach (auto widget, m_barDatasets)
        names.append(widget->title());

    foreach (auto widget, m_gaugeDatasets)
        names.append(widget->title());

    foreach (auto widget, m_compassDatasets)
        names.append(widget->title());

    return names;
}

/**
 * Returns the visibility status for the widget at the given @a index
 */
bool WidgetProvider::widgetVisible(const int index) const
{
    if (index < totalWidgetCount())
        return m_widgetVisibility.at(index);

    return false;
}

/**
 * Returns the number of JSON groups that implement a bar widget
 */
int WidgetProvider::barDatasetCount() const
{
    return barDatasets().count();
}

/**
 * Returns the number of JSON groups that implement a gauge widget
 */
int WidgetProvider::gaugeDatasetCount() const
{
    return gaugeDatasets().count();
}

/**
 * Returns the number of JSON groups that implement a map widget
 */
int WidgetProvider::mapGroupCount() const
{
    return mapGroup().count();
}

/**
 * Returns the number of JSON groups that implement a gyro widget
 */
int WidgetProvider::gyroGroupCount() const
{
    return gyroGroup().count();
}

/**
 * Returns the number of JSON datasets that implement a compass widget
 */
int WidgetProvider::compassDatasetCount() const
{
    return compassDatasets().count();
}

/**
 * Returns the number of JSON groups that implement an accelerometer widget
 */
int WidgetProvider::accelerometerGroupCount() const
{
    return accelerometerGroup().count();
}

/**
 * Returns a pointer to the JSON dataset that implements a bar widget
 * with the given @a index
 */
JSON::Dataset *WidgetProvider::barDatasetAt(const int index)
{
    if (barDatasets().count() > index)
        return barDatasets().at(index);

    return Q_NULLPTR;
}

/**
 * Returns a pointer to the JSON dataset that implements a gauge widget
 * with the given @a index
 */
JSON::Dataset *WidgetProvider::gaugeDatasetAt(const int index)
{
    if (gaugeDatasets().count() > index)
        return gaugeDatasets().at(index);

    return Q_NULLPTR;
}

/**
 * Returns a pointer to the JSON dataset that implements a compass widget
 * with the given @a index.
 */
JSON::Dataset *WidgetProvider::compassDatasetAt(const int index)
{
    if (compassDatasets().count() > index)
        return compassDatasets().at(index);

    return Q_NULLPTR;
}

/**
 * Returns a pointer to the JSON group that implements a map widget
 * with the given @a index
 */
JSON::Group *WidgetProvider::mapGroupAt(const int index)
{
    if (mapGroup().count() > index)
        return mapGroup().at(index);

    return Q_NULLPTR;
}

/**
 * Returns a pointer to the JSON group that implements a gyro widget
 * with the given @a index
 */
JSON::Group *WidgetProvider::gyroGroupAt(const int index)
{
    if (gyroGroup().count() > index)
        return gyroGroup().at(index);

    return Q_NULLPTR;
}

/**
 * Returns a pointer to the JSON group that implements an accelerometer
 * widget with the given @a index
 */
JSON::Group *WidgetProvider::accelerometerGroupAt(const int index)
{
    if (accelerometerGroup().count() > index)
        return accelerometerGroup().at(index);

    return Q_NULLPTR;
}

/**
 * Returns the direction in degrees for the compass widget at the given @a index
 */
int WidgetProvider::compass(const int index)
{
    auto compass = compassDatasetAt(index);
    if (compass)
        return qMax(0, qMin(360, compass->value().toInt()));

    return INT_MAX;
}

/**
 * Returns the yaw angle for the gyro widget at the given @a index
 */
double WidgetProvider::gyroYaw(const int index)
{
    auto gyro = gyroGroupAt(index);

    if (gyro)
    {
        foreach (auto dataset, gyro->datasets())
        {
            auto widget = dataset->widget();
            if (widget.toLower() == "yaw")
                return dataset->value().toDouble();
        }
    }

    return DBL_MAX;
}

/**
 * Returns the roll angle for the gyro widget at the given @a index
 */
double WidgetProvider::gyroRoll(const int index)
{
    auto gyro = gyroGroupAt(index);

    if (gyro)
    {
        foreach (auto dataset, gyro->datasets())
        {
            auto widget = dataset->widget();
            if (widget.toLower() == "roll")
                return dataset->value().toDouble();
        }
    }

    return DBL_MAX;
}

/**
 * Returns the pitch angle for the gyro widget at the given @a index
 */
double WidgetProvider::gyroPitch(const int index)
{
    auto gyro = gyroGroupAt(index);

    if (gyro)
    {
        foreach (auto dataset, gyro->datasets())
        {
            auto widget = dataset->widget();
            if (widget.toLower() == "pitch")
                return dataset->value().toDouble();
        }
    }

    return DBL_MAX;
}

/**
 * Returns the value of the X axis for the accelerometer widget at the given @a
 * index
 */
double WidgetProvider::accelerometerX(const int index)
{
    auto accelerometer = accelerometerGroupAt(index);

    if (accelerometer)
    {
        foreach (auto dataset, accelerometer->datasets())
        {
            auto widget = dataset->widget();
            if (widget.toLower() == "x")
                return dataset->value().toDouble();
        }
    }

    return DBL_MAX;
}

/**
 * Returns the value of the Y axis for the accelerometer widget at the given @a
 * index
 */
double WidgetProvider::accelerometerY(const int index)
{
    auto accelerometer = accelerometerGroupAt(index);

    if (accelerometer)
    {
        foreach (auto dataset, accelerometer->datasets())
        {
            auto widget = dataset->widget();
            if (widget.toLower() == "y")
                return dataset->value().toDouble();
        }
    }

    return DBL_MAX;
}

/**
 * Returns the value of the Z axis for the accelerometer widget at the given @a
 * index
 */
double WidgetProvider::accelerometerZ(const int index)
{
    auto accelerometer = accelerometerGroupAt(index);

    if (accelerometer)
    {
        foreach (auto dataset, accelerometer->datasets())
        {
            auto widget = dataset->widget();
            if (widget.toLower() == "z")
                return dataset->value().toDouble();
        }
    }

    return DBL_MAX;
}

/**
 * Returns the value for the bar widget at the given @a index
 */
double WidgetProvider::bar(const int index)
{
    auto bar = barDatasetAt(index);
    if (bar)
        return bar->value().toDouble();

    return DBL_MAX;
}

/**
 * Returns the minimum value for the bar widget at the given @a index
 */
double WidgetProvider::barMin(const int index)
{
    auto bar = barDatasetAt(index);
    if (bar)
        return bar->jsonData().value("min").toDouble();

    return DBL_MAX;
}

/**
 * Returns the maximum value for the bar widget at the given @a index
 */
double WidgetProvider::barMax(const int index)
{
    auto bar = barDatasetAt(index);
    if (bar)
        return bar->jsonData().value("max").toDouble();

    return DBL_MAX;
}

/**
 * Returns the value for the gauge widget at the given @a index
 */
double WidgetProvider::gauge(const int index)
{
    auto gauge = gaugeDatasetAt(index);
    if (gauge)
        return gauge->value().toDouble();

    return DBL_MAX;
}

/**
 * Returns the minimum value for the gauge widget at the given @a index
 */
double WidgetProvider::gaugeMin(const int index)
{
    auto gauge = gaugeDatasetAt(index);
    if (gauge)
        return gauge->jsonData().value("min").toDouble();

    return DBL_MAX;
}

/**
 * Returns the maximum value for the gauge widget at the given @a index
 */
double WidgetProvider::gaugeMax(const int index)
{
    auto gauge = gaugeDatasetAt(index);
    if (gauge)
        return gauge->jsonData().value("max").toDouble();

    return DBL_MAX;
}

/**
 * Returns the latitude value for the map widget at the given @a index
 */
double WidgetProvider::mapLatitude(const int index)
{
    auto map = mapGroupAt(index);

    if (map)
    {
        foreach (auto dataset, map->datasets())
        {
            auto widget = dataset->widget();
            if (widget.toLower() == "lat")
                return dataset->value().toDouble();
        }
    }

    return DBL_MAX;
}

/**
 * Returns the longitude value for the map widget at the given @a index
 */
double WidgetProvider::mapLongitude(const int index)
{
    auto map = mapGroupAt(index);

    if (map)
    {
        foreach (auto dataset, map->datasets())
        {
            auto widget = dataset->widget();
            if (widget.toLower() == "lon")
                return dataset->value().toDouble();
        }
    }

    return DBL_MAX;
}

/**
 * Updates the visibility status for the given @a index
 */
void WidgetProvider::updateWidgetVisibility(const int index, const bool visible)
{
    if (index < totalWidgetCount())
        m_widgetVisibility.replace(index, visible);

    emit widgetVisiblityChanged();
}

/**
 * Deletes all stored widget information
 */
void WidgetProvider::resetData()
{
    m_widgetCount = 0;
    m_mapGroups.clear();
    m_gyroGroups.clear();
    m_barDatasets.clear();
    m_gaugeDatasets.clear();
    m_compassDatasets.clear();
    m_accelerometerGroups.clear();

    emit dataChanged();
    emit widgetCountChanged();
}

/**
 * Regenerates the widget groups with the latest JSON-generated frame
 */
void WidgetProvider::updateModels()
{
    // Clear current groups
    m_mapGroups.clear();
    m_gyroGroups.clear();
    m_barDatasets.clear();
    m_gaugeDatasets.clear();
    m_compassDatasets.clear();
    m_accelerometerGroups.clear();

    // Check if frame is valid
    if (!DataProvider::getInstance()->latestFrame()->isValid())
        return;

    // Update groups
    m_mapGroups = getWidgetGroup("map");
    m_gyroGroups = getWidgetGroup("gyro");
    m_barDatasets = getWidgetDatasets("bar");
    m_gaugeDatasets = getWidgetDatasets("gauge");
    m_compassDatasets = getWidgetDatasets("compass");
    m_accelerometerGroups = getWidgetGroup("accelerometer");

    // Tell UI to regenerate widget models if widget count has changed from last frame
    if (totalWidgetCount() != m_widgetCount)
    {
        m_widgetVisibility.clear();
        m_widgetCount = totalWidgetCount();
        for (int i = 0; i < m_widgetCount; ++i)
            m_widgetVisibility.append(true);

        emit widgetCountChanged();
        emit widgetVisiblityChanged();
    }

    // Update UI
    emit dataChanged();
}

/**
 * Obtains all the JSON groups that implement the given widget @a handle ID.
 * The JSON groups are provided by the @c QmlBridge class.
 */
QList<JSON::Group *> WidgetProvider::getWidgetGroup(const QString &handle)
{
    QList<JSON::Group *> widgetGroup;

    auto frame = DataProvider::getInstance()->latestFrame();
    foreach (auto group, frame->groups())
    {
        if (group->widget().toLower() == handle)
            widgetGroup.append(group);
    }

    return widgetGroup;
}

/**
 * Obtains all the JSON datasets that implement the given widget @a handle ID.
 * The JSON groups & datasets are provided by the @c QmlBridge class.
 */
QList<JSON::Dataset *> WidgetProvider::getWidgetDatasets(const QString &handle)
{
    QList<JSON::Dataset *> widgetDatasets;

    auto frame = DataProvider::getInstance()->latestFrame();
    foreach (auto group, frame->groups())
    {
        foreach (auto dataset, group->datasets())
        {
            if (dataset->widget() == handle)
                widgetDatasets.append(dataset);
        }
    }

    return widgetDatasets;
}
