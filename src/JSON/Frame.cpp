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

#include "Frame.h"

using namespace JSON;

Frame::Frame(QObject *parent)
    : QObject(parent)
    , m_title("")
{
}

Frame::~Frame()
{
    clear();
}

void Frame::clear()
{
    for (int i = 0; i < groupCount(); ++i)
        m_groups.at(i)->deleteLater();

    m_title = "";
    m_groups.clear();
}

QString Frame::title() const
{
    return m_title;
}

int Frame::groupCount() const
{
    return groups().count();
}

QVector<Group *> Frame::groups() const
{
    return m_groups;
}

bool Frame::read(const QJsonObject &object)
{
    // Rest frame data
    clear();

    // Get title & groups array
    auto title = object.value("t").toString();
    auto groups = object.value("g").toArray();

    // We need to have a project title and at least one group
    if (!title.isEmpty() && !groups.isEmpty())
    {
        // Update title
        m_title = title;

        // Generate groups & datasets from data frame
        for (auto i = 0; i < groups.count(); ++i)
        {
            Group *group = new Group(this);
            if (group->read(groups.at(i).toObject()))
                m_groups.append(group);
            else
                group->deleteLater();
        }

        // Return status
        return groupCount() > 0;
    }

    // Error
    clear();
    return false;
}

/**
 * @return The group at the given @a index,vreturns @c Q_NULLPTR on invalid index
 */
Group *Frame::getGroup(const int index)
{
    if (index < groupCount() && index >= 0)
        return m_groups.at(index);

    return Q_NULLPTR;
}
