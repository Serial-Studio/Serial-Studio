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

#pragma once

#include <QObject>
#include <QVariant>
#include <QJsonObject>

namespace JSON
{
class ProjectModel;
}

namespace JSON
{
/**
 * @brief The Dataset class
 *
 * The dataset class represents the properties and values of an
 * individual data unit.
 *
 * For example, supose that you are reading the values of a temperature
 * sensor. In this case, the dataset could have the following values:
 *
 * - Value: 21
 * - Units: °C
 * - Title: External temperature
 * - Widget: "bar"
 * - Graph: true
 * - Max: 100
 * - Min: -15
 * - Alarm: 45
 *
 * Description for each field of the dataset class:
 * - Value: represents the current sensor reading/value.
 * - Units: represents the measurement units of the reading.
 * - Title: description of the dataset.
 * - Widget: widget that shall be used to represents the value,
 *           for example, a level widget, a gauge, a compass, etc.
 * - Graph: if set to true, Serial Studio shall plot the value in
 *          realtime.
 * - Max: maximum value of the dataset, used for gauges & bars.
 * - Min: minimum value of the dataset, used for gauges & bars.
 * - Alarm: if the value exceeds the alarm level, bar widgets
 *          shall be rendered with a dark-red background.
 *
 * @note All of the dataset fields are optional, except the "value"
 *       field and the "title" field.
 */
class Group;
class FrameBuilder;
class Dataset
{
public:
  Dataset(const int groupId = -1, const int datasetId = -1);

  [[nodiscard]] bool fft() const;
  [[nodiscard]] bool led() const;
  [[nodiscard]] bool log() const;
  [[nodiscard]] int index() const;
  [[nodiscard]] bool graph() const;
  [[nodiscard]] double min() const;
  [[nodiscard]] double max() const;
  [[nodiscard]] double alarm() const;
  [[nodiscard]] double ledHigh() const;

  [[nodiscard]] int xAxisId() const;
  [[nodiscard]] int fftSamples() const;
  [[nodiscard]] int fftSamplingRate() const;

  [[nodiscard]] int groupId() const;
  [[nodiscard]] int datasetId() const;

  [[nodiscard]] const QString &title() const;
  [[nodiscard]] const QString &value() const;
  [[nodiscard]] const QString &units() const;
  [[nodiscard]] const QString &widget() const;
  [[nodiscard]] const QJsonObject &jsonData() const;

  [[nodiscard]] QJsonObject serialize() const;
  [[nodiscard]] bool read(const QJsonObject &object);

  void setTitle(const QString &title) { m_title = title; }

private:
  bool m_fft;
  bool m_led;
  bool m_log;
  bool m_graph;

  QString m_title;
  QString m_value;
  QString m_units;
  QString m_widget;
  QJsonObject m_jsonData;

  int m_index;
  double m_max;
  double m_min;
  double m_alarm;
  double m_ledHigh;
  int m_fftSamples;
  int m_fftSamplingRate;

  int m_groupId;
  int m_xAxisId;
  int m_datasetId;

  friend class JSON::ProjectModel;
  friend class JSON::FrameBuilder;
};
} // namespace JSON
