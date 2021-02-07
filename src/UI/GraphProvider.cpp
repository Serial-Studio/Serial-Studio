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

#include "GraphProvider.h"

#include <QTimer>
#include <QXYSeries>
#include <QMetaType>

#include <Logger.h>
#include <CSV/Player.h>
#include <IO/Manager.h>
#include <IO/Console.h>
#include <JSON/Generator.h>
#include <ConsoleAppender.h>

using namespace UI;

/*
 * Only instance of the class
 */
static GraphProvider *INSTANCE = nullptr;

//
// Magic
//
QT_CHARTS_USE_NAMESPACE
Q_DECLARE_METATYPE(QAbstractAxis *)
Q_DECLARE_METATYPE(QAbstractSeries *)

/**
 * Sets the maximum displayed points to 10, connects SIGNALS/SLOTS & calls
 * QML/Qt magic functions to deal with QML charts from C++.
 */
GraphProvider::GraphProvider()
{
    // clang-format off

    // Start with 10 points
    m_prevFramePos = 0;
    m_displayedPoints = 10;

    // Register data types
    qRegisterMetaType<QAbstractAxis *>();
    qRegisterMetaType<QAbstractSeries *>();

    // Update graph values as soon as JSON manager reads data
    auto cp = CSV::Player::getInstance();
    auto io = IO::Manager::getInstance();
    connect(cp, SIGNAL(openChanged()), this, SLOT(resetData()));
    connect(io, SIGNAL(connectedChanged()), this, SLOT(resetData()));

    // Avoid issues when CSV player goes backwards
    connect(CSV::Player::getInstance(), SIGNAL(timestampChanged()),
            this, SLOT(csvPlayerFixes()));

    // Update user interface at a frequency of ~40 Hz
    m_timer.setInterval(1000 / 40);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &GraphProvider::updateValues);
    m_timer.start();

    // clang-format on
    LOG_TRACE() << "Class initialized";
}

/**
 * Returns the only instance of the class
 */
GraphProvider *GraphProvider::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new GraphProvider();

    return INSTANCE;
}

/**
 * Returns the number of graph data sources
 */
int GraphProvider::graphCount() const
{
    return datasets().count();
}

/**
 * Returns the number of points that are currently displayed on the graph
 */
int GraphProvider::displayedPoints() const
{
    return m_displayedPoints;
}

/**
 * Returns a list with the @a Dataset objects that act as data sources for the
 * graph views
 */
QVector<JSON::Dataset *> GraphProvider::datasets() const
{
    return m_datasets;
}

/**
 * Returns the latest value graphed by the dataset at the given @a index
 */
double GraphProvider::getValue(const int index) const
{
    if (index < graphCount() && index >= 0)
        return getDataset(index)->value().toDouble();

    return 0;
}

/**
 * Returns the smallest value registered with the dataset at the given @a index
 */
double GraphProvider::minimumValue(const int index) const
{
    double min = INT_MAX;

    if (index < m_minimumValues.count() && index >= 0)
        min = m_minimumValues.at(index);

    if (min != INT_MAX)
        return min;

    return -1;
}

/**
 * Returns the greatest value registered with the dataset at the given @a index
 */
double GraphProvider::maximumValue(const int index) const
{
    double max = INT_MIN;

    if (index < m_maximumValues.count() && index >= 0)
        max = m_maximumValues.at(index);

    if (max != INT_MIN)
        return max;

    return 1;
}

/**
 * Returns a pointer to the dataset object at the given @a index
 */
JSON::Dataset *GraphProvider::getDataset(const int index) const
{
    if (index < graphCount() && index >= 0)
        return datasets().at(index);

    return Q_NULLPTR;
}

/**
 * Changes the maximum number of points that should be displayed in the graph
 * views.
 */
void GraphProvider::setDisplayedPoints(const int points)
{
    if (points != displayedPoints() && points > 0)
    {
        m_displayedPoints = points;
        m_points.clear();

        emit displayedPointsUpdated();
        emit dataUpdated();
    }
}

/**
 * Deletes all stored information
 */
void GraphProvider::resetData()
{
    m_points.clear();
    m_datasets.clear();
    m_maximumValues.clear();
    m_minimumValues.clear();

    emit dataUpdated();
}

/**
 * Gets the latest values from the datasets that support/need to be graphed
 */
void GraphProvider::updateValues()
{
    // Abort if console is currently in use
    if (IO::Console::getInstance()->enabled())
        return;

    // Abort if we are not connected to a device
    if (!IO::Manager::getInstance()->connected())
        return;

    // Clear dataset & latest values list
    m_datasets.clear();

    // Create list with datasets that need to be graphed
    auto frame = JSON::Generator::getInstance()->frame();
    for (int i = 0; i < frame->groupCount(); ++i)
    {
        auto group = frame->groups().at(i);
        for (int j = 0; j < group->datasetCount(); ++j)
        {
            auto dataset = group->datasets().at(j);
            if (dataset->graph())
                m_datasets.append(dataset);
        }
    }

    // Create list with dataset values (converted to double)
    for (int i = 0; i < graphCount(); ++i)
    {
        // Register dataset for this graph
        if (m_points.count() < (i + 1))
        {
            auto vector = new QVector<double>;
            m_points.append(vector);
        }

        // Register min. values list
        if (m_minimumValues.count() < (i + 1))
            m_minimumValues.append(getValue(i));

        // Register max. values list
        if (m_maximumValues.count() < (i + 1))
            m_maximumValues.append(getValue(i));

        // Update minimum value
        if (minimumValue(i) > getValue(i))
            m_minimumValues.replace(i, getValue(i));

        // Update minimum value
        if (maximumValue(i) < getValue(i))
            m_maximumValues.replace(i, getValue(i));

        // Remove older items
        if (m_points.at(i)->count() >= displayedPoints())
            m_points.at(i)->remove(0, m_points.at(i)->count() - displayedPoints());

        // Add values
        m_points.at(i)->append(getValue(i));
    }

    // Update graphs
    QTimer::singleShot(25, this, SIGNAL(dataUpdated()));
}

/**
 * Removes graph points that are ahead of current data frame that is being
 * displayed/processed by the CSV Player.
 */
void GraphProvider::csvPlayerFixes()
{
    // If current frame comes before last-recorded frame, remove extra data
    auto currentFrame = CSV::Player::getInstance()->framePosition();
    if (m_prevFramePos > currentFrame)
    {
        auto diff = m_prevFramePos - currentFrame;
        for (int i = 0; i < graphCount(); ++i)
        {
            for (int j = 0; j < diff; ++j)
            {
                if (!m_points.at(i)->isEmpty())
                    m_points.at(i)->removeLast();
            }
        }

        emit dataUpdated();
    }

    // Update frame position
    m_prevFramePos = currentFrame;
}

/**
 * Updates the graph for the given data @a series prorivder, the @a index is
 * used to know which dataset object should be used to pull the latest data
 * point.
 */
void GraphProvider::updateGraph(QAbstractSeries *series, const int index)
{
    // Validation
    assert(series != Q_NULLPTR);

    // Update data
    if (series->isVisible())
    {
        if (m_points.count() > index && index >= 0)
        {
            QVector<QPointF> data;
            for (int i = 0; i < m_points.at(index)->count(); ++i)
                data.append(QPointF(i, m_points.at(index)->at(i)));

            static_cast<QXYSeries *>(series)->replace(data);
        }
    }
}
