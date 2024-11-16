/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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

#include "JSON/Dataset.h"

JSON::Dataset::Dataset(const int groupId, const int datasetId)
  : m_fft(false)
  , m_led(false)
  , m_log(false)
  , m_graph(false)
  , m_title("")
  , m_value("")
  , m_units("")
  , m_widget("")
  , m_index(0)
  , m_max(0)
  , m_min(0)
  , m_alarm(0)
  , m_ledHigh(1)
  , m_fftSamples(256)
  , m_fftSamplingRate(100)
  , m_groupId(groupId)
  , m_datasetId(datasetId)
{
}

/**
 * @return @c true if the UI should generate a FFT plot of this dataset
 */
bool JSON::Dataset::fft() const
{
  return m_fft;
}

/**
 * @return @c true if the UI should generate a LED of this dataset
 */
bool JSON::Dataset::led() const
{
  return m_led;
}

/**
 * @return @c true if the UI should generate a logarithmic plot of this dataset
 */
bool JSON::Dataset::log() const
{
  return m_log;
}

/**
 * @return the field index represented by the current dataset
 */
int JSON::Dataset::index() const
{
  return m_index;
}

/**
 * @return @c true if the UI should graph this dataset
 */
bool JSON::Dataset::graph() const
{
  return m_graph;
}

/**
 * Returns the minimum value of the dataset
 */
double JSON::Dataset::min() const
{
  return m_min;
}

/**
 * Returns the maximum value of the dataset
 */
double JSON::Dataset::max() const
{
  return m_max;
}

/**
 * Returns the alarm level of the dataset
 */
double JSON::Dataset::alarm() const
{
  return m_alarm;
}

/**
 * Returns the LED active threshold value.
 */
double JSON::Dataset::ledHigh() const
{
  return m_ledHigh;
}

/**
 * @return The title/description of this dataset
 */
const QString &JSON::Dataset::title() const
{
  return m_title;
}

/**
 * @return The value/reading of this dataset
 */
const QString &JSON::Dataset::value() const
{
  return m_value;
}

/**
 * @return The units of this dataset
 */
const QString &JSON::Dataset::units() const
{
  return m_units;
}

/**
 * @return The widget value of this dataset
 */
const QString &JSON::Dataset::widget() const
{
  return m_widget;
}

/**
 * Returns the maximum freq. for the FFT transform
 */
int JSON::Dataset::fftSamples() const
{
  return qMax(1, m_fftSamples);
}

/**
 * Returns the sampling rate for the FFT transform
 */
int JSON::Dataset::fftSamplingRate() const
{
  return m_fftSamplingRate;
}

/**
 * @return The index of the group to which the dataset belongs to, used by
 *         the project model to easily identify which group/dataset to update
 *         when the user modifies a parameter in the project model.
 */
int JSON::Dataset::groupId() const
{
  return m_groupId;
}

/**
 * @return The index of the dataset relative to the available datasets of the
 *         parent group.
 */
int JSON::Dataset::datasetId() const
{
  return m_datasetId;
}

/**
 * Returns the JSON data that represents this widget
 */
const QJsonObject &JSON::Dataset::jsonData() const
{
  return m_jsonData;
}

/**
 * @brief Encodes the dataset information into a QJsonObject.
 *
 * This function serializes the dataset's properties into a JSON object.
 * Note that the "m_value" field is deliberately excluded from the encoding
 * process.
 *
 * @return A QJsonObject containing the dataset's properties.
 */
QJsonObject JSON::Dataset::serialize() const
{
  QJsonObject object;
  object.insert(QStringLiteral("fft"), m_fft);
  object.insert(QStringLiteral("led"), m_led);
  object.insert(QStringLiteral("log"), m_log);
  object.insert(QStringLiteral("min"), m_min);
  object.insert(QStringLiteral("max"), m_max);
  object.insert(QStringLiteral("index"), m_index);
  object.insert(QStringLiteral("alarm"), m_alarm);
  object.insert(QStringLiteral("graph"), m_graph);
  object.insert(QStringLiteral("ledHigh"), m_ledHigh);
  object.insert(QStringLiteral("fftSamples"), m_fftSamples);
  object.insert(QStringLiteral("value"), m_value.simplified());
  object.insert(QStringLiteral("title"), m_title.simplified());
  object.insert(QStringLiteral("units"), m_units.simplified());
  object.insert(QStringLiteral("widget"), m_widget.simplified());
  object.insert(QStringLiteral("fftSamplingRate"), m_fftSamplingRate);
  return object;
}

/**
 * Reads dataset information from the given @a object.
 *
 * @return @c true on read success, @c false on failure
 */
bool JSON::Dataset::read(const QJsonObject &object)
{
  if (!object.isEmpty())
  {
    m_fft = object.value(QStringLiteral("fft")).toBool();
    m_led = object.value(QStringLiteral("led")).toBool();
    m_log = object.value(QStringLiteral("log")).toBool();
    m_min = object.value(QStringLiteral("min")).toDouble();
    m_max = object.value(QStringLiteral("max")).toDouble();
    m_index = object.value(QStringLiteral("index")).toInt();
    m_alarm = object.value(QStringLiteral("alarm")).toDouble();
    m_graph = object.value(QStringLiteral("graph")).toBool();
    m_ledHigh = object.value(QStringLiteral("ledHigh")).toDouble();
    m_fftSamples = object.value(QStringLiteral("fftSamples")).toInt();
    m_title = object.value(QStringLiteral("title")).toString().simplified();
    m_value = object.value(QStringLiteral("value")).toString().simplified();
    m_units = object.value(QStringLiteral("units")).toString().simplified();
    m_widget = object.value(QStringLiteral("widget")).toString().simplified();
    m_fftSamplingRate = object.value(QStringLiteral("fftSamplingRate")).toInt();
    if (m_value.isEmpty())
      m_value = QStringLiteral("--.--");

    return true;
  }

  return false;
}
