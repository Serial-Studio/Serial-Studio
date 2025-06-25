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
  [[nodiscard]] QByteArray txByteArray() const;

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
