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

#include "Group.h"
#include "Logger.h"
#include "Dataset.h"
#include "Widgets.h"
#include "QmlBridge.h"

#include <cfloat>
#include <climits>

/*
 * Pointer to the only instance of the class
 */
static Widgets *INSTANCE = Q_NULLPTR;

/**
 * Initialization code of the @c Widgets class
 */
Widgets::Widgets()
{
    auto bridge = QmlBridge::getInstance();
    connect(bridge, SIGNAL(updated()), this, SLOT(updateModels()));

    LOG_INFO() << "Initialized Widgets module";
}

/**
 * Returns a pointer to the only instance of the class
 */
Widgets *Widgets::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Widgets;

    return INSTANCE;
}

/**
 * Returns a list with all the JSON datasets that implement a bar widget
 */
QList<Dataset *> Widgets::barDatasets() const
{
    return m_barDatasets;
}

/**
 * Returns a list with all the JSON groups that implement a map widget
 */
QList<Group *> Widgets::mapGroup() const
{
    return m_mapGroups;
}

/**
 * Returns a list with all the JSON groups that implement a gyro widget
 */
QList<Group *> Widgets::gyroGroup() const
{
    return m_gyroGroups;
}

/**
 * Returns a list with all the JSON groups that implement an accelerometer
 * widget
 */
QList<Group *> Widgets::accelerometerGroup() const
{
    return m_accelerometerGroups;
}

/**
 * Returns the total number of widgets that should be generated
 */
int Widgets::totalWidgetCount() const
{
    return barDatasetCount() + mapGroupCount() + gyroGroupCount()
        + accelerometerGroupCount();
}

/**
 * Returns the number of JSON groups that implement a bar widget
 */
int Widgets::barDatasetCount() const
{
    return barDatasets().count();
}

/**
 * Returns the number of JSON groups that implement a map widget
 */
int Widgets::mapGroupCount() const
{
    return mapGroup().count();
}

/**
 * Returns the number of JSON groups that implement a gyro widget
 */
int Widgets::gyroGroupCount() const
{
    return gyroGroup().count();
}

/**
 * Returns the number of JSON groups that implement an accelerometer widget
 */
int Widgets::accelerometerGroupCount() const
{
    return accelerometerGroup().count();
}

/**
 * Returns a pointer to the JSON dataset that implements a bar widget
 * with the given @a index
 */
Dataset *Widgets::barDatasetAt(const int index)
{
    if (barDatasets().count() > index)
        return barDatasets().at(index);

    return Q_NULLPTR;
}

/**
 * Returns a pointer to the JSON group that implements a map widget
 * with the given @a index
 */
Group *Widgets::mapGroupAt(const int index)
{
    if (mapGroup().count() > index)
        return mapGroup().at(index);

    return Q_NULLPTR;
}

/**
 * Returns a pointer to the JSON group that implements a gyro widget
 * with the given @a index
 */
Group *Widgets::gyroGroupAt(const int index)
{
    if (gyroGroup().count() > index)
        return gyroGroup().at(index);

    return Q_NULLPTR;
}

/**
 * Returns a pointer to the JSON group that implements an accelerometer
 * widget with the given @a index
 */
Group *Widgets::accelerometerGroupAt(const int index)
{
    if (accelerometerGroup().count() > index)
        return accelerometerGroup().at(index);

    return Q_NULLPTR;
}

/**
 * Returns the yaw angle for the gyro widget at the given @a index
 */
double Widgets::gyroYaw(const int index)
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
double Widgets::gyroRoll(const int index)
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
double Widgets::gyroPitch(const int index)
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
double Widgets::accelerometerX(const int index)
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
double Widgets::accelerometerY(const int index)
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
double Widgets::accelerometerZ(const int index)
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
double Widgets::bar(const int index)
{
    auto bar = barDatasetAt(index);
    if (bar)
        return bar->value().toDouble();

    return DBL_MAX;
}

/**
 * Returns the minimum value for the bar widget at the given @a index
 */
double Widgets::barMin(const int index)
{
    auto bar = barDatasetAt(index);
    if (bar)
        return bar->jsonData().value("min").toDouble();

    return DBL_MAX;
}

/**
 * Returns the maximum value for the bar widget at the given @a index
 */
double Widgets::barMax(const int index)
{
    auto bar = barDatasetAt(index);
    if (bar)
        return bar->jsonData().value("max").toDouble();

    return DBL_MAX;
}

/**
 * Returns the latitude value for the map widget at the given @a index
 */
double Widgets::mapLatitude(const int index)
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
double Widgets::mapLongitude(const int index)
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
 * Regenerates the widget groups with the latest data given by the @c QmlBridge
 * class.
 */
void Widgets::updateModels()
{
    // Clear current groups
    m_barDatasets.clear();
    m_mapGroups.clear();
    m_gyroGroups.clear();
    m_accelerometerGroups.clear();

    // Update groups
    m_mapGroups = getWidgetGroup("map");
    m_gyroGroups = getWidgetGroup("gyro");
    m_barDatasets = getWidgetDatasets("bar");
    m_accelerometerGroups = getWidgetGroup("accelerometer");

    // Update UI
    emit dataChanged();
}

/**
 * Obtains all the JSON groups that implement the given widget @a handle ID.
 * The JSON groups are provided by the @c QmlBridge class.
 */
QList<Group *> Widgets::getWidgetGroup(const QString &handle)
{
    QList<Group *> widgetGroup;

    foreach (auto group, QmlBridge::getInstance()->groups())
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
QList<Dataset *> Widgets::getWidgetDatasets(const QString &handle)
{
    QList<Dataset *> widgetDatasets;

    foreach (auto group, QmlBridge::getInstance()->groups())
    {
        foreach (auto dataset, group->datasets())
        {
            if (dataset->widget() == handle)
                widgetDatasets.append(dataset);
        }
    }

    return widgetDatasets;
}
