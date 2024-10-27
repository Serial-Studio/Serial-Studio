/*
 * Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
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

#include "JSON/Action.h"

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
  , m_icon("Play Property")
  , m_title("")
  , m_txData("")
  , m_eolSequence("")
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
  object.insert(QStringLiteral("title"), m_title.trimmed());
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
    m_icon = object.value(QStringLiteral("icon")).toString();
    m_txData = object.value(QStringLiteral("txData")).toString();
    m_eolSequence = object.value(QStringLiteral("eol")).toString();
    m_title = object.value(QStringLiteral("title")).toString().trimmed();

    return true;
  }

  return false;
}
