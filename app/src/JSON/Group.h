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

#include "JSON/Dataset.h"

namespace UI
{
class Taskbar;
class Dashboard;
} // namespace UI

namespace JSON
{
class Frame;
class ProjectModel;
class FrameBuilder;
} // namespace JSON

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

  friend class UI::Taskbar;
  friend class UI::Dashboard;

  friend class JSON::Frame;
  friend class JSON::ProjectModel;
  friend class JSON::FrameBuilder;
};
} // namespace JSON
