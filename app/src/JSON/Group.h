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

#include "JSON/Dataset.h"

namespace UI
{
class Dashboard;
}

namespace JSON
{
class ProjectModel;
}

namespace JSON
{
/**
 * @brief The Group class
 *
 * The group class represents a set of datasets that are related
 * together (e.g. they correspond to the same category).
 *
 * Some special widgets, such as the accelerometer, the map or
 * the gyroscope are generated using groups instead of datasets.
 * For example, an accelerometer widget is constructed with a group
 * with the following datasets:
 *
 * - X-axis readings
 * - Y-axis readings
 * - Z-axis readings
 *
 * A group contains the following properties:
 * - Title
 * - Widget
 * - A vector of datasets
 */
class FrameBuilder;
class Group
{
public:
  Group(const int groupId = -1);
  ~Group();

  [[nodiscard]] QJsonObject serialize() const;
  [[nodiscard]] bool read(const QJsonObject &object);

  [[nodiscard]] int groupId() const;
  [[nodiscard]] int datasetCount() const;
  [[nodiscard]] const QString &title() const;
  [[nodiscard]] const QString &widget() const;
  [[nodiscard]] const QVector<JSON::Dataset> &datasets() const;
  [[nodiscard]] const JSON::Dataset &getDataset(const int index) const;

private:
  int m_groupId;
  QString m_title;
  QString m_widget;
  QVector<JSON::Dataset> m_datasets;

  friend class UI::Dashboard;
  friend class JSON::ProjectModel;
  friend class JSON::FrameBuilder;
};
} // namespace JSON
