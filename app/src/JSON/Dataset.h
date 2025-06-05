/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QObject>
#include <QVariant>
#include <QJsonObject>

namespace JSON
{
class Group;
class Frame;
class FrameBuilder;
class ProjectModel;
} // namespace JSON

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
class Dataset
{
public:
  Dataset(const int groupId = -1, const int datasetId = -1);

  [[nodiscard]] quint32 uniqueId() const;

  [[nodiscard]] bool fft() const;
  [[nodiscard]] bool led() const;
  [[nodiscard]] bool log() const;
  [[nodiscard]] int index() const;
  [[nodiscard]] bool graph() const;
  [[nodiscard]] double min() const;
  [[nodiscard]] double max() const;
  [[nodiscard]] double alarm() const;
  [[nodiscard]] double ledHigh() const;
  [[nodiscard]] bool displayInOverview() const;

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

  void setMin(double min) { m_min = min; }
  void setMax(double max) { m_max = max; }
  void setUniqueId(const quint32 id) { m_uniqueId = id; }
  void setValue(const QString &value) { m_value = value; }
  void setTitle(const QString &title) { m_title = title; }

private:
  quint32 m_uniqueId;

  bool m_fft;
  bool m_led;
  bool m_log;
  bool m_graph;
  bool m_displayInOverview;

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

  friend class JSON::Group;
  friend class JSON::Frame;
  friend class JSON::ProjectModel;
  friend class JSON::FrameBuilder;
};
} // namespace JSON
