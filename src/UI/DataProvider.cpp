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

using namespace UI;

/*
 * Only instance of the class
 */
static DataProvider *INSTANCE = nullptr;

/**
 * Constructor of the class
 */
DataProvider::DataProvider()
{
    // React to open/close of devices & files
    auto cp = CSV::Player::getInstance();
    auto io = IO::Manager::getInstance();
    auto ge = JSON::Generator::getInstance();
    connect(cp, SIGNAL(openChanged()), this, SLOT(resetData()));
    connect(ge, SIGNAL(jsonChanged()), this, SIGNAL(updated()));
    connect(io, SIGNAL(connectedChanged()), this, SLOT(resetData()));

    // Try to look like a pro
    LOG_INFO() << "Class initialized";
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
    return JSON::Generator::getInstance()->frame()->title();
}

/**
 * @return The number of groups contained in the current frame.
 */
int DataProvider::groupCount()
{
    return JSON::Generator::getInstance()->frame()->groupCount();
}

/**
 * Returns a reference to the group object registered with the given @a index.
 */
JSON::Group *DataProvider::getGroup(const int index)
{
    if (index < groupCount())
        return JSON::Generator::getInstance()->frame()->groups().at(index);

    return Q_NULLPTR;
}

/**
 * Removes all available data from the UI when the device is disconnected or the CSV
 * file is closed.
 */
void DataProvider::resetData()
{
    // Stop if dev. man is not disconnected or if CSV file is open
    if (IO::Manager::getInstance()->connected() || CSV::Player::getInstance()->isOpen())
        return;

    // Clear frame object
    JSON::Generator::getInstance()->frame()->clear();

    // Update UI
    emit updated();
    emit dataReset();
}
