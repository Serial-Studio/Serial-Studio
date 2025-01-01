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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
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
 * @class JSON::Action
 * @brief A class that represents an action which can send data to connected
 *        devices.
 *
 * The JSON::Action class allows users to create an action with an ID, and
 * manage associated metadata such as an icon, title, transmission data
 * (txData), and an end-of-line (eol) sequence. It also provides functionality
 * to serialize and deserialize the action to and from a QJsonObject, making it
 * suitable for JSON-based communication or storage.
 */
class Action
{
public:
  Action(const int actionId = -1);

  [[nodiscard]] int actionId() const;
  [[nodiscard]] const QString &icon() const;
  [[nodiscard]] const QString &title() const;
  [[nodiscard]] const QString &txData() const;
  [[nodiscard]] const QString &eolSequence() const;

  [[nodiscard]] QJsonObject serialize() const;
  [[nodiscard]] bool read(const QJsonObject &object);

private:
  int m_actionId;
  QString m_icon;
  QString m_title;
  QString m_txData;
  QString m_eolSequence;

  friend class JSON::ProjectModel;
};
} // namespace JSON
