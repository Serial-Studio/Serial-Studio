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
#include "GraphProvider.h"

#include <QtMath>
#include <QXYSeries>
#include <QMetaType>

#include <Logger.h>
#include <CSV/Player.h>
#include <IO/Manager.h>
#include <IO/Console.h>
#include <JSON/Generator.h>
#include <ConsoleAppender.h>
#include <Misc/TimerEvents.h>

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

    // Module signals/slots
    auto cp = CSV::Player::getInstance();
    auto io = IO::Manager::getInstance();
    auto ge = JSON::Generator::getInstance();
    auto te = Misc::TimerEvents::getInstance();
    connect(cp, SIGNAL(openChanged()), this, SLOT(resetData()));
    connect(te, SIGNAL(timeout42Hz()), this, SLOT(drawGraphs()));
    connect(io, SIGNAL(connectedChanged()), this, SLOT(resetData()));
    connect(ge, &JSON::Generator::jsonChanged, this, &GraphProvider::registerFrame);

    // Avoid issues when CSV player goes backwards
    connect(CSV::Player::getInstance(), SIGNAL(timestampChanged()),
            this, SLOT(csvPlayerFixes()));

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
 * Returns a point object with the recommended min/max values for the graph at the
 * given @a index
 */
QPointF GraphProvider::graphRange(const int index) const
{
    // Update maximum value (if required)
    double maxV = maximumValue(index);
    double minV = minimumValue(index);

    // Get central value
    double medianValue = qMax<double>(1, (maxV + minV)) / 2;
    if (maxV == minV)
        medianValue = maxV;

    // Center graph verticaly
    double mostDiff = qMax<double>(qAbs<double>(minV), qAbs<double>(maxV));
    double min = medianValue * (1 - 0.5) - qAbs<double>(medianValue - mostDiff);
    double max = medianValue * (1 + 0.5) + qAbs<double>(medianValue - mostDiff);
    if (minV < 0)
        min = max * -1;

    // Fix issues when min & max are equal
    if (min == max)
    {
        max = qAbs<double>(max);
        min = max * -1;
    }

    // Fix issues on min = max = (0,0)
    if (min == 0 && max == 0)
    {
        max = 1;
        min = -1;
    }

    // Return point as (min, max)
    return QPointF(min, max);
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
void GraphProvider::drawGraphs()
{
    // Sort JSON frames so that they are ordered from least-recent to most-recent
    JFI_SortList(&m_jsonList);

    // Graph each frame
    for (int f = 0; f < m_jsonList.count(); ++f)
    {
        // Clear dataset & latest values list
        m_datasets.clear();

        // Get frame, abort if frame is invalid
        JSON::Frame frame;
        if (!frame.read(m_jsonList.at(f).jsonDocument.object()))
            continue;

        // Create list with datasets that need to be graphed
        for (int i = 0; i < frame.groupCount(); ++i)
        {
            auto group = frame.groups().at(i);
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
    }

    // Clear frame list
    m_jsonList.clear();

    // Update UI
    emit dataUpdated();
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

/**
 * Obtains the latest JSON dataframe & appends it to the JSON list, which is later read,
 * sorted & graphed by the @c drawGraph() function.
 */
void GraphProvider::registerFrame(const JFI_Object &frameInfo)
{
    if (JFI_Valid(frameInfo))
        m_jsonList.append(frameInfo);
}
