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

#include "JSON/Action.h"

/**
 * @brief Reads a value from a QJsonObject based on a key, returning a default
 *        value if the key does not exist.
 *
 * This function checks if the given key exists in the provided QJsonObject.
 * If the key is found, it returns the associated value. Otherwise, it returns
 * the specified default value.
 *
 * @param object The QJsonObject to read the data from.
 * @param key The key to look for in the QJsonObject.
 * @param defaultValue The value to return if the key is not found in JSON.
 *
 * @return The value associated with the key, or the defaultValue if the key is
 *         not present.
 */
static QVariant SAFE_READ(const QJsonObject &object, const QString &key,
                          const QVariant &defaultValue)
{
  if (object.contains(key))
    return object.value(key);

  return defaultValue;
}

/**
 * @brief Constructs an Action object with a specified action ID.
 *
 * This constructor initializes the action with the provided ID, and sets
 * the icon, title, txData, and eolSequence to empty strings.
 *
 * @param actionId The unique ID for this action, set by the project editor.
 */
JSON::Action::Action(const int actionId)
  : m_actionId(actionId)
  , m_binaryData(false)
  , m_icon("Play Property")
  , m_title("")
  , m_txData("")
  , m_eolSequence("")
  , m_timerIntervalMs(100)
  , m_timerMode(TimerMode::Off)
  , m_autoExecuteOnConnect(false)
{
}

/**
 * @return The action ID in the project array, only used for interacting
 *         with the project model (which is used to build the Project Editor
 *         GUI).
 */
int JSON::Action::actionId() const
{
  return m_actionId;
}

/**
 * @brief Checks if the user wants to send binary data to the connected device.
 *
 * @return @c true if binary data encoding is enabled, @c false otherwise.
 */
bool JSON::Action::binaryData() const
{
  return m_binaryData;
}

/**
 * @brief Gets the icon associated with the action.
 *
 * @return A constant reference to the icon as a QString.
 */
const QString &JSON::Action::icon() const
{
  return m_icon;
}

/**
 * @brief Gets the title of the action.
 *
 * @return A constant reference to the title as a QString.
 */
const QString &JSON::Action::title() const
{
  return m_title;
}

/**
 * @brief Gets the transmission data (txData) to be sent to the device.
 *
 * @return A constant reference to the txData as a QString.
 */
const QString &JSON::Action::txData() const
{
  return m_txData;
}

/**
 * @brief Gets the end-of-line (eol) sequence associated with the action.
 *
 * @return A constant reference to the eolSequence as a QString.
 */
const QString &JSON::Action::eolSequence() const
{
  return m_eolSequence;
}

/**
 * @brief Returns the current timer mode for this action.
 *
 * The timer mode controls how and when the action is executed repeatedly.
 * Possible values are:
 *
 * - TimerMode::Off: No timer is used.
 * - TimerMode::AutoStart: The timer starts automatically.
 * - TimerMode::StartOnTrigger: The timer starts when the action is manually
 *                              triggered.
 * - TimerMode::ToggleOnTrigger: Each manual trigger toggles the timer on or
 *                               off.
 *
 * @return The timer behavior mode.
 */
JSON::Action::TimerMode JSON::Action::timerMode() const
{
  return m_timerMode;
}

/**
 * @brief Returns the timer interval in milliseconds.
 *
 * If the timer mode is active (i.e., not TimerMode::Off), this value defines
 * how frequently the action should be triggered.
 *
 * @return Interval in milliseconds between each timed execution.
 */
int JSON::Action::timerIntervalMs() const
{
  return m_timerIntervalMs;
}

/**
 * @brief Returns whether the action should automatically execute when a device
 * connects.
 *
 * If set to true, this action will be triggered immediately upon device
 * connection, without user interaction.
 *
 * @return true if the action auto-executes on connection, false otherwise.
 */
bool JSON::Action::autoExecuteOnConnect() const
{
  return m_autoExecuteOnConnect;
}

/**
 * @brief Serializes the action to a QJsonObject.
 *
 * This method converts the Action object to a QJsonObject, which can be used
 * for JSON-based transmission or storage.
 *
 * @return A QJsonObject representing the serialized Action.
 */
QJsonObject JSON::Action::serialize() const
{
  QJsonObject object;
  object.insert(QStringLiteral("icon"), m_icon);
  object.insert(QStringLiteral("txData"), m_txData);
  object.insert(QStringLiteral("eol"), m_eolSequence);
  object.insert(QStringLiteral("binary"), m_binaryData);
  object.insert(QStringLiteral("title"), m_title.simplified());
  object.insert(QStringLiteral("timerIntervalMs"), m_timerIntervalMs);
  object.insert(QStringLiteral("timerMode"), static_cast<int>(m_timerMode));
  object.insert(QStringLiteral("autoExecuteOnConnect"), m_autoExecuteOnConnect);
  return object;
}

/**
 * @brief Reads the action's data from a QJsonObject.
 *
 * This method populates the Action object by deserializing data from a given
 * QJsonObject.
 *
 * It expects the object to contain fields for "icon", "title", "txData", and
 * "eol".
 *
 * @param object The QJsonObject containing the action's data.
 * @return true if the object was successfully read, false if the object is
 * empty.
 */
bool JSON::Action::read(const QJsonObject &object)
{
  if (!object.isEmpty())
  {
    // clang-format off
    m_eolSequence          = SAFE_READ(object, "eol", "").toString();
    m_txData               = SAFE_READ(object, "txData", "").toString();
    m_binaryData           = SAFE_READ(object, "binary", false).toBool();
    m_timerIntervalMs      = SAFE_READ(object, "timerIntervalMs", 100).toInt();
    m_icon                 = SAFE_READ(object, "icon", "").toString().simplified();
    m_title                = SAFE_READ(object, "title", "").toString().simplified();
    m_autoExecuteOnConnect = SAFE_READ(object, "autoExecuteOnConnect", false).toBool();
    m_timerMode            = static_cast<TimerMode>(SAFE_READ(object, "timerMode", static_cast<int>(TimerMode::Off)).toInt());
    // clang-format on

    return true;
  }

  return false;
}
