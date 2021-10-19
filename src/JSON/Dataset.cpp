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
    , m_fft(false)
    , m_graph(false)
    , m_log(false)
    , m_title("")
    , m_value("")
    , m_units("")
    , m_widget("")
    , m_index(0)
    , m_max("0")
    , m_min("0")
    , m_alarm("0")
    , m_fftSamples(8)
{
}

/**
 * @return @c true if the UI should generate a FFT plot of this dataset
 */
bool Dataset::fft() const
{
    return m_fft;
}

/**
 * @return @c true if the UI should generate a logarithmic plot of this dataset
 */
bool Dataset::log() const
{
    return m_log;
}

/**
 * @return @c true if the UI should graph this dataset
 */
bool Dataset::graph() const
{
    return m_graph;
}

/**
 * Returns the minimum value of the dataset
 */
double Dataset::min() const
{
    return m_min.toDouble();
}

/**
 * Returns the maximum value of the dataset
 */
double Dataset::max() const
{
    return m_max.toDouble();
}

/**
 * Returns the alarm level of the dataset
 */
double Dataset::alarm() const
{
    return m_alarm.toDouble();
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
 * Returns the maximum freq. for the FFT transform
 */
int Dataset::fftSamples() const
{
    return qMax(1, m_fftSamples);
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
        auto fft = object.value("fft").toVariant().toBool();
        auto log = object.value("log").toVariant().toBool();
        auto graph = object.value("g").toVariant().toBool();
        auto title = object.value("t").toVariant().toString();
        auto value = object.value("v").toVariant().toString();
        auto units = object.value("u").toVariant().toString();
        auto widget = object.value("w").toVariant().toString();
        auto min = object.value("min").toVariant().toString();
        auto max = object.value("max").toVariant().toString();
        auto alarm = object.value("alarm").toVariant().toString();
        auto fftSamples = object.value("fftSamples").toVariant().toInt();

        min = min.replace("\n", "");
        min = min.replace("\r", "");
        max = max.replace("\n", "");
        max = max.replace("\r", "");
        title = title.replace("\n", "");
        title = title.replace("\r", "");
        value = value.replace("\n", "");
        value = value.replace("\r", "");
        units = units.replace("\n", "");
        units = units.replace("\r", "");
        alarm = alarm.replace("\n", "");
        alarm = alarm.replace("\r", "");
        widget = widget.replace("\n", "");
        widget = widget.replace("\r", "");

        if (!value.isEmpty() && !title.isEmpty())
        {
            m_min = min;
            m_max = max;
            m_fft = fft;
            m_log = log;
            m_graph = graph;
            m_title = title;
            m_units = units;
            m_value = value;
            m_alarm = alarm;
            m_widget = widget;
            m_jsonData = object;
            m_fftSamples = fftSamples;

            return true;
        }
    }

    return false;
}
