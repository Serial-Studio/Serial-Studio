/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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
