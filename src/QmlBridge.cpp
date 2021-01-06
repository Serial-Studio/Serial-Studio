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
#include "QmlBridge.h"
#include "JsonParser.h"
#include "SerialManager.h"

#include <QJsonArray>
#include <QJsonObject>

/*
 * Only instance of the class
 */
static QmlBridge *INSTANCE = nullptr;

QmlBridge::QmlBridge()
{
    auto jp = JsonParser::getInstance();
    auto sm = SerialManager::getInstance();
    connect(jp, SIGNAL(packetReceived()), this, SLOT(update()));
    connect(sm, SIGNAL(connectedChanged()), this, SLOT(resetData()));

    LOG_INFO() << "Initialized QML Bridge module";
}

/**
 * Returns the only instance of the class
 */
QmlBridge *QmlBridge::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new QmlBridge();

    return INSTANCE;
}

/**
 * @return The title of the project/frame sent by the serial device
 */
QString QmlBridge::projectTitle() const
{
    return m_title;
}

/**
 * @return The number of groups contained in the last frame.
 */
int QmlBridge::groupCount() const
{
    return groups().count();
}

/**
 * @return A list with all the @c Group objects generated
 *         after reading the last received frame.
 */
QList<Group *> QmlBridge::groups() const
{
    return m_groups;
}

/**
 * Returns a pointer to the group object registered with the given @a index.
 * If @a index is invalid, then @c Q_NULLPTR is returned.
 */
Group *QmlBridge::getGroup(const int index)
{
    if (index < groupCount() && index >= 0)
        return groups().at(index);

    return Q_NULLPTR;
}

/**
 * Extracts project title & groups from the latest JSON frame interpretted
 * by the @a JsonParser class.
 *
 * Afterwards, the function shall generate a list with @c Group objects from
 * the data. Each @c Group object shall generate a list with the @c Dataset
 * object asociated with the group.
 *
 * @note The function shall only re-generate @c Group & @c Dataset objects if
 *       the current JSON data frame is valid.
 */
void QmlBridge::update()
{
    // Get JSON document
    auto document = JsonParser::getInstance()->document();

    // Validate JSON document
    if (!document.isEmpty())
    {
        // Get JSON object information
        auto object = document.object();
        auto title = object.value("t").toString();
        auto groups = object.value("g").toArray();

        // We need to have a project title and at least one group
        if (!title.isEmpty() && !groups.isEmpty())
        {
            // Update title
            m_title = title;

            // Delete existing groups
            for (int i = 0; i < groupCount(); ++i)
                m_groups.at(i)->deleteLater();

            // Clear group list
            m_groups.clear();

            // Generate groups & datasets from data frame
            for (auto i = 0; i < groups.count(); ++i)
            {
                auto group = new Group(this);
                if (group->read(groups.at(i).toObject()))
                    m_groups.append(group);
                else
                    group->deleteLater();
            }

            // Update UI
            emit updated();
        }
    }
}

/**
 * Clears the project title, deletes all groups & notifies the UI. This function
 * is used when the serial device connection state is changed to avoid errors.
 */
void QmlBridge::resetData()
{
    // Delete existing groups
    for (int i = 0; i < groupCount(); ++i)
        m_groups.at(i)->deleteLater();

    // Clear group list
    m_title.clear();
    m_groups.clear();

    // Update UI
    emit updated();
}
