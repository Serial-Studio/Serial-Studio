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
 * SPDX-License-Identifier: GPL-3.0-only
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
 * (txData), and an end-of-line (eol) sequence. It also supports flags for
 * auto-execution on device connection and timer-based behavior.
 */
class Action
{
public:
  enum class TimerMode
  {
    Off,            ///< No timer
    AutoStart,      ///< Starts timer automatically (e.g. on connection)
    StartOnTrigger, ///< Starts timer when the action is triggered
    ToggleOnTrigger ///< Toggles timer state with each trigger
  };

  Action(const int actionId = -1);

  [[nodiscard]] int actionId() const;
  [[nodiscard]] bool binaryData() const;

  [[nodiscard]] const QString &icon() const;
  [[nodiscard]] const QString &title() const;
  [[nodiscard]] const QString &txData() const;
  [[nodiscard]] const QString &eolSequence() const;

  [[nodiscard]] TimerMode timerMode() const;
  [[nodiscard]] int timerIntervalMs() const;
  [[nodiscard]] bool autoExecuteOnConnect() const;

  [[nodiscard]] QJsonObject serialize() const;
  [[nodiscard]] bool read(const QJsonObject &object);

private:
  int m_actionId;
  bool m_binaryData;

  QString m_icon;
  QString m_title;
  QString m_txData;
  QString m_eolSequence;

  int m_timerIntervalMs;
  TimerMode m_timerMode;
  bool m_autoExecuteOnConnect;

  friend class JSON::ProjectModel;
};
} // namespace JSON
