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

#include "Dataset.h"
#include "Generator.h"

using namespace JSON;

Dataset::Dataset(QObject *parent)
    : QObject(parent)
    , m_graph(false)
    , m_title("")
    , m_value("")
    , m_units("")
    , m_widget("")
    , m_index(0)
    , m_max("100")
    , m_min("0")
{
}

/**
 * @return @c true if the UI should graph this dataset
 */
bool Dataset::graph() const
{
    return m_graph;
}

/**
 * @return The title/description of this dataset
 */
QString Dataset::title() const
{
    return m_title;
}

/**
 * @return The value/reading of this dataset
 */
QString Dataset::value() const
{
    return m_value;
}

/**
 * @return The units of this dataset
 */
QString Dataset::units() const
{
    return m_units;
}

/**
 * @return The widget value of this dataset
 */
QString Dataset::widget() const
{
    return m_widget;
}

/**
 * Returns the JSON data that represents this widget
 */
QJsonObject Dataset::jsonData() const
{
    return m_jsonData;
}

/**
 * Reads dataset information from the given @a object and evaluates any JS code in the
 * "value" field of the dataset.
 *
 * @return @c true on read success, @c false on failure
 */
bool Dataset::read(const QJsonObject &object)
{
    static QJSEngine JAVASCRIPT_ENGINE;

    if (!object.isEmpty())
    {
        auto graph = object.value("g").toVariant().toBool();
        auto title = object.value("t").toVariant().toString();
        auto value = object.value("v").toVariant().toString();
        auto units = object.value("u").toVariant().toString();
        auto widget = object.value("w").toVariant().toString();

        title = title.replace("\n", "");
        title = title.replace("\r", "");
        value = value.replace("\n", "");
        value = value.replace("\r", "");
        units = units.replace("\n", "");
        units = units.replace("\r", "");
        widget = widget.replace("\n", "");
        widget = widget.replace("\r", "");

        if (!value.isEmpty())
        {
            m_graph = graph;
            m_title = title;
            m_units = units;
            m_value = value;
            m_widget = widget;
            m_jsonData = object;

            return true;
        }
    }

    return false;
}
