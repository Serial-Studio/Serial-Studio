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

#include <Logger.h>
#include <IO/Manager.h>
#include <IO/Console.h>
#include <CSV/Player.h>
#include <JSON/Generator.h>
#include <ConsoleAppender.h>
#include <Misc/TimerEvents.h>

using namespace UI;

/*
 * Only instance of the class
 */
static DataProvider *INSTANCE = nullptr;

/**
 * Constructor of the class
 */
DataProvider::DataProvider()
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
    connect(ge, &JSON::Generator::jsonChanged, this, &DataProvider::selectLatestJSON);
    LOG_TRACE() << "Class initialized";
}

/**
 * Returns the only instance of the class
 */
DataProvider *DataProvider::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new DataProvider();

    return INSTANCE;
}

/**
 * @return The title of the current frame
 */
QString DataProvider::title()
{
    return m_latestFrame.title();
}

/**
 * @return The number of groups contained in the current frame.
 */
int DataProvider::groupCount()
{
    return m_latestFrame.groupCount();
}

/**
 * Returns a pointer to the latest frame
 */
JSON::Frame *DataProvider::latestFrame()
{
    return &m_latestFrame;
}

/**
 * Returns @c true if the latest frame contains data
 */
bool DataProvider::frameValid() const
{
    return m_latestFrame.isValid();
}

/**
 * Returns a reference to the group object registered with the given @a index.
 */
JSON::Group *DataProvider::getGroup(const int index)
{
    if (index < groupCount())
        return m_latestFrame.groups().at(index);

    return Q_NULLPTR;
}

/**
 * Removes all available data from the UI when the device is disconnected or the CSV
 * file is closed.
 */
void DataProvider::resetData()
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
void DataProvider::updateData()
{
    if (m_latestFrame.read(m_latestJsonFrame.jsonDocument.object()))
        emit updated();
}

/**
 * Ensures that only the most recent JSON document will be displayed on the user
 * interface.
 */
void DataProvider::selectLatestJSON(const JFI_Object &frameInfo)
{
    auto frameCount = frameInfo.frameNumber;
    auto currFrameCount = m_latestJsonFrame.frameNumber;

    if (currFrameCount < frameCount)
    {
        if (JFI_Valid(frameInfo))
            m_latestJsonFrame = frameInfo;
    }
}
