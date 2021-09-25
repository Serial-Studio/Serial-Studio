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

using namespace UI;

/*
 * Only instance of the class
 */
static Dashboard *INSTANCE = nullptr;

/**
 * Constructor of the class
 */
Dashboard::Dashboard()
    : m_latestJsonFrame(JFI_Empty())
{
    auto cp = CSV::Player::getInstance();
    auto io = IO::Manager::getInstance();
    auto ge = JSON::Generator::getInstance();
    auto te = Misc::TimerEvents::getInstance();
    connect(cp, SIGNAL(openChanged()), this, SLOT(resetData()));
    connect(te, SIGNAL(highFreqTimeout()), this, SLOT(updateData()));
    connect(io, SIGNAL(connectedChanged()), this, SLOT(resetData()));
    connect(ge, SIGNAL(jsonFileMapChanged()), this, SLOT(resetData()));
    connect(ge, &JSON::Generator::jsonChanged, this, &Dashboard::selectLatestJSON);
}

/**
 * Returns the only instance of the class
 */
Dashboard *Dashboard::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Dashboard();

    return INSTANCE;
}

/**
 * @return The title of the current frame
 */
QString Dashboard::title()
{
    return m_latestFrame.title();
}

bool Dashboard::available()
{
    return groupCount() > 0;
}

int Dashboard::mapCount()
{
    return m_mapWidgets.count();
}

int Dashboard::barCount()
{
    return m_barWidgets.count();
}

int Dashboard::plotCount()
{
    return m_plotWidgets.count();
}

int Dashboard::gaugeCount()
{
    return m_gaugeWidgets.count();
}

/**
 * @return The number of groups contained in the current frame.
 */
int Dashboard::groupCount()
{
    return m_latestFrame.groupCount();
}

int Dashboard::compassCount()
{
    return m_compassWidgets.count();
}

int Dashboard::gyroscopeCount()
{
    return m_gyroscopeWidgets.count();
}

int Dashboard::thermometerCount()
{
    return m_thermometerWidgets.count();
}

int Dashboard::accelerometerCount()
{
    return m_accelerometerWidgets.count();
}

QStringList Dashboard::barTitles()
{
    QStringList list;
    foreach (auto set, m_barWidgets)
        list.append(set->title());

    return list;
}

QStringList Dashboard::mapTitles()
{
    QStringList list;
    foreach (auto group, m_mapWidgets)
        list.append(group->title());

    return list;
}

QStringList Dashboard::plotTitles()
{
    QStringList list;
    foreach (auto set, m_plotWidgets)
        list.append(set->title());

    return list;
}

QStringList Dashboard::groupTitles()
{
    QStringList list;
    foreach (auto group, m_latestFrame.groups())
        list.append(group->title());

    return list;
}

QStringList Dashboard::gaugeTitles()
{
    QStringList list;
    foreach (auto set, m_gaugeWidgets)
        list.append(set->title());

    return list;
}

QStringList Dashboard::compassTitles()
{
    QStringList list;
    foreach (auto set, m_compassWidgets)
        list.append(set->title());

    return list;
}

QStringList Dashboard::gyroscopeTitles()
{
    QStringList list;
    foreach (auto group, m_gyroscopeWidgets)
        list.append(group->title());

    return list;
}

QStringList Dashboard::thermometerTitles()
{
    QStringList list;
    foreach (auto set, m_thermometerWidgets)
        list.append(set->title());

    return list;
}

QStringList Dashboard::accelerometerTitles()
{
    QStringList list;
    foreach (auto group, m_accelerometerWidgets)
        list.append(group->title());

    return list;
}

bool Dashboard::barVisible(const int index)
{
    if (index < m_barVisibility.count())
        return m_barVisibility.at(index);

    return false;
}

bool Dashboard::mapVisible(const int index)
{
    if (index < m_mapVisibility.count())
        return m_mapVisibility.at(index);

    return false;
}

bool Dashboard::plotVisible(const int index)
{
    if (index < m_plotVisibility.count())
        return m_plotVisibility.at(index);

    return false;
}

bool Dashboard::groupVisible(const int index)
{
    if (index < m_groupVisiblity.count())
        return m_groupVisiblity.at(index);

    return false;
}

bool Dashboard::gaugeVisible(const int index)
{
    if (index < m_gaugeVisibility.count())
        return m_gaugeVisibility.at(index);

    return false;
}

bool Dashboard::compassVisible(const int index)
{
    if (index < m_compassVisibility.count())
        return m_compassVisibility.at(index);

    return false;
}

bool Dashboard::gyroscopeVisible(const int index)
{
    if (index < m_gyroscopeVisibility.count())
        return m_gyroscopeVisibility.at(index);

    return false;
}

bool Dashboard::thermometerVisible(const int index)
{
    if (index < m_thermometerVisibility.count())
        return m_thermometerVisibility.at(index);

    return false;
}

bool Dashboard::accelerometerVisible(const int index)
{
    if (index < m_accelerometerVisibility.count())
        return m_accelerometerVisibility.at(index);

    return false;
}

/**
 * Returns @c true if the latest frame contains data
 */
bool Dashboard::frameValid() const
{
    return m_latestFrame.isValid();
}

void Dashboard::setBarVisible(const int index, const bool visible)
{
    if (index < m_barVisibility.count())
    {
        if (barVisible(index) != visible)
        {
            m_barVisibility.replace(index, visible);
            emit widgetVisibilityChanged();
        }
    }
}

void Dashboard::setMapVisible(const int index, const bool visible)
{
    if (index < m_mapVisibility.count())
    {
        if (mapVisible(index) != visible)
        {
            m_mapVisibility.replace(index, visible);
            emit widgetVisibilityChanged();
        }
    }
}

void Dashboard::setPlotVisible(const int index, const bool visible)
{
    if (index < m_plotVisibility.count())
    {
        if (plotVisible(index) != visible)
        {
            m_plotVisibility.replace(index, visible);
            emit widgetVisibilityChanged();
        }
    }
}

void Dashboard::setGroupVisible(const int index, const bool visible)
{
    if (index < m_groupVisiblity.count())
    {
        if (groupVisible(index) != visible)
        {
            m_groupVisiblity.replace(index, visible);
            emit widgetVisibilityChanged();
        }
    }
}

void Dashboard::setGaugeVisible(const int index, const bool visible)
{
    if (index < m_gaugeVisibility.count())
    {
        if (gaugeVisible(index) != visible)
        {
            m_gaugeVisibility.replace(index, visible);
            emit widgetVisibilityChanged();
        }
    }
}

void Dashboard::setCompassVisible(const int index, const bool visible)
{
    if (index < m_compassVisibility.count())
    {
        if (compassVisible(index) != visible)
        {
            m_compassVisibility.replace(index, visible);
            emit widgetVisibilityChanged();
        }
    }
}

void Dashboard::setGyroscopeVisible(const int index, const bool visible)
{
    if (index < m_gyroscopeVisibility.count())
    {
        if (gyroscopeVisible(index) != visible)
        {
            m_gyroscopeVisibility.replace(index, visible);
            emit widgetVisibilityChanged();
        }
    }
}

void Dashboard::setThermometerVisible(const int index, const bool visible)
{
    if (index < m_thermometerVisibility.count())
    {
        if (thermometerVisible(index) != visible)
        {
            m_thermometerVisibility.replace(index, visible);
            emit widgetVisibilityChanged();
        }
    }
}

void Dashboard::setAccelerometerVisible(const int index, const bool visible)
{
    if (index < m_accelerometerVisibility.count())
    {
        if (accelerometerVisible(index) != visible)
        {
            m_accelerometerVisibility.replace(index, visible);
            emit widgetVisibilityChanged();
        }
    }
}

/**
 * Removes all available data from the UI when the device is disconnected or the CSV
 * file is closed.
 */
void Dashboard::resetData()
{
    // Make latest frame invalid
    m_latestJsonFrame = JFI_Empty();
    m_latestFrame.read(m_latestJsonFrame.jsonDocument.object());

    // Update UI
    emit updated();
    emit dataReset();
}

/**
 * Interprets the most recent JSON frame & signals the UI to regenerate itself.
 */
void Dashboard::updateData()
{
    if (m_latestFrame.read(m_latestJsonFrame.jsonDocument.object()))
        emit updated();
}

/**
 * Ensures that only the most recent JSON document will be displayed on the user
 * interface.
 */
void Dashboard::selectLatestJSON(const JFI_Object &frameInfo)
{
    auto frameCount = frameInfo.frameNumber;
    auto currFrameCount = m_latestJsonFrame.frameNumber;

    if (currFrameCount < frameCount)
        m_latestJsonFrame = frameInfo;
}

QVector<JSON::Dataset *> Dashboard::getPlotWidgets()
{
    QVector<JSON::Dataset *> widgets;
    return widgets;
}

QVector<JSON::Group *> Dashboard::getWidgetGroup(const QString &handle)
{
    QVector<JSON::Group *> widgets;
    return widgets;
}

QVector<JSON::Dataset *> Dashboard::getWidgetDatasets(const QString &handle)
{
    QVector<JSON::Dataset *> widgets;
    return widgets;
}
