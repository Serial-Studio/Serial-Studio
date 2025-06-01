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

#include <QVector>
#include <QObject>
#include <QVariant>
#include <QJsonArray>
#include <QJsonObject>

#include "JSON/Group.h"
#include "JSON/Action.h"

namespace JSON
{
/**
 * @brief The Frame class
 *
 * The frame class represents a complete frame, including the groups & datasets
 * that compose it. This class allows Serial Studio to build the user interface
 * dinamically from the received data.
 *
 * The process of building a frame and representing it in Serial Studio is:
 * 1) Physical device sends data
 * 2) I/O driver receives data
 * 3) I/O manager processes the data and separates the frames
 * 4) JSON generator creates a JSON file with the data contained in each frame.
 * 5) UI dashboard class receives the JSON file.
 * 6) TimerEvents class notifies the UI dashboard that the user interface should
 *    be re-generated.
 * 7) UI dashboard feeds JSON data to this class.
 * 8) This class creates a model of the JSON data with the values of the latest
 *    frame.
 * 9) UI dashboard updates the widgets with the C++ model provided by this
 * class.
 */
class FrameBuilder;
class Frame
{
public:
  Frame();
  ~Frame();

  void clear();
  [[nodiscard]] bool isValid() const;

  [[nodiscard]] QJsonObject serialize() const;
  [[nodiscard]] bool read(const QJsonObject &object);

  [[nodiscard]] int groupCount() const;
  [[nodiscard]] bool containsCommercialFeatures() const;

  [[nodiscard]] const QString &title() const;
  [[nodiscard]] const QByteArray &frameEnd() const;
  [[nodiscard]] const QByteArray &frameStart() const;

  [[nodiscard]] const QVector<Group> &groups() const;
  [[nodiscard]] const QVector<Action> &actions() const;

private:
  QString m_title;
  QByteArray m_frameEnd;
  QByteArray m_frameStart;

  QVector<Group> m_groups;
  QVector<Action> m_actions;

  bool m_containsCommercialFeatures;

  friend class UI::Dashboard;
  friend class JSON::FrameBuilder;
};
} // namespace JSON
