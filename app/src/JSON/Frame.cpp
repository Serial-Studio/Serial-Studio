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

#include "JSON/Frame.h"
#include "SerialStudio.h"

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
 * @param defaultValue The value to return if the key is not found in the
 * QJsonObject.
 * @return The value associated with the key, or the defaultValue if the key is
 * not present.
 */
static QVariant SAFE_READ(const QJsonObject &object, const QString &key,
                          const QVariant &defaultValue)
{
  if (object.contains(key))
    return object.value(key);

  return defaultValue;
}

/**
 * Constructor function
 */
JSON::Frame::Frame()
  : m_containsCommercialFeatures(false)
{
}

/**
 * Destructor function, free memory used by the @c Group objects before
 * destroying an instance of this class.
 */
JSON::Frame::~Frame()
{
  m_groups.clear();
  m_actions.clear();
  m_groups.squeeze();
  m_actions.squeeze();
}

/**
 * Resets the frame title and frees the memory used by the @c Group objects
 * associated to the instance of the @c Frame object.
 */
void JSON::Frame::clear()
{
  m_title = "";
  m_frameEnd = "";
  m_frameStart = "";
  m_containsCommercialFeatures = false;

  m_groups.clear();
  m_actions.clear();
  m_groups.squeeze();
  m_actions.squeeze();
}

/**
 * @brief Returns @c true if the project has a defined title and it has at least
 *        one dataset group.
 */
bool JSON::Frame::isValid() const
{
  return !title().isEmpty() && groupCount() > 0;
}

/**
 * @brief Assigns globally unique identifiers to all datasets in the frame.
 *
 * This method iterates over all groups and their corresponding datasets,
 * assigning each dataset a unique ID starting from 1. These IDs are used
 * internally by the dashboard for unambiguous dataset tracking across groups,
 * especially when multiple datasets share the same logical index or title.
 *
 * Call this function after the frame structure has been fully initialized.
 */
void JSON::Frame::buildUniqueIds()
{
  quint32 id = 1;
  for (int i = 0; i < m_groups.count(); ++i)
  {
    for (int j = 0; j < m_groups[i].datasetCount(); ++j)
    {
      m_groups[i].m_datasets[j].setUniqueId(id);
      ++id;
    }
  }
}

/**
 * @brief Compares the structural equivalence of two JSON::Frame objects.
 *
 * This method checks whether the current frame and the given frame have the
 * same number of groups, and whether each corresponding group has the same
 * group ID and the same number of datasets in the same order, with matching
 * dataset indices.
 *
 * @param other The frame to compare against.
 * @return true if both frames have identical structural layout; false
 * otherwise.
 */
bool JSON::Frame::equalsStructure(const JSON::Frame &other) const
{
  if (groupCount() != other.groupCount())
    return false;

  for (int i = 0; i < groupCount(); ++i)
  {
    const auto &g1 = groups()[i];
    const auto &g2 = other.groups()[i];

    if (g1.groupId() != g2.groupId()
        || g1.datasets().size() != g2.datasets().size())
      return false;

    for (int j = 0; j < g1.datasets().size(); ++j)
    {
      if (g1.datasets()[j].index() != g2.datasets()[j].index())
        return false;
    }
  }

  return true;
}

/**
 * @brief Serializes the frame information and its data into a JSON object.
 *
 * @return A QJsonObject containing the group's properties and an array of
 * encoded datasets.
 */
QJsonObject JSON::Frame::serialize() const
{
  QJsonArray groupArray;
  for (const auto &group : m_groups)
    groupArray.append(group.serialize());

  QJsonArray actionArray;
  for (const auto &action : m_actions)
    actionArray.append(action.serialize());

  QJsonObject object;
  object.insert(QStringLiteral("title"), m_title.simplified());
  object.insert(QStringLiteral("groups"), groupArray);
  object.insert(QStringLiteral("actions"), actionArray);
  return object;
}

/**
 * Reads the frame information and all its asociated groups (and datatsets) from
 * the given JSON @c object.
 *
 * @return @c true on success, @c false on failure
 */
bool JSON::Frame::read(const QJsonObject &object)
{
  // Reset frame data
  clear();

  // Get title & groups array
  const auto groups = object.value(QStringLiteral("groups")).toArray();
  const auto actions = object.value(QStringLiteral("actions")).toArray();
  const auto title = SAFE_READ(object, "title", "").toString().simplified();

  // We need to have a project title and at least one group
  if (!title.isEmpty() && !groups.isEmpty())
  {
    // Update title
    m_title = title;

    // Obtain frame delimiters
    auto fEndStr = SAFE_READ(object, "frameEnd", "").toString();
    auto fStartStr = SAFE_READ(object, "frameStart", "").toString();
    auto isHex = SAFE_READ(object, "hexadecimalDelimiters", false).toBool();

    // Convert hex + escape strings (e.g. "0A 0D", or "\\n0D") to raw bytes
    if (isHex)
    {
      QString resolvedEnd = SerialStudio::resolveEscapeSequences(fEndStr);
      QString resolvedStart = SerialStudio::resolveEscapeSequences(fStartStr);
      m_frameEnd = QByteArray::fromHex(resolvedEnd.remove(' ').toUtf8());
      m_frameStart = QByteArray::fromHex(resolvedStart.remove(' ').toUtf8());
    }

    // Resolve escape sequences (e.g. "\\n") and encode to UTF-8 bytes
    else
    {
      m_frameEnd = SerialStudio::resolveEscapeSequences(fEndStr).toUtf8();
      m_frameStart = SerialStudio::resolveEscapeSequences(fStartStr).toUtf8();
    }

    // Generate groups & datasets from data frame
    for (auto i = 0; i < groups.count(); ++i)
    {
      Group group(i);
      if (group.read(groups.at(i).toObject()))
        m_groups.append(group);
    }

    // Generate actions from data frame
    for (auto i = 0; i < actions.count(); ++i)
    {
      Action action;
      if (action.read(actions.at(i).toObject()))
        m_actions.append(action);
    }

    // Check if any of the groups contains commercial widgets
    m_containsCommercialFeatures = SerialStudio::commercialCfg(m_groups);

    // Build unique dataset IDs
    buildUniqueIds();

    // Return status
    return groupCount() > 0;
  }

  // Error
  clear();
  return false;
}

/**
 * Returns the number of groups contained in the frame.
 */
int JSON::Frame::groupCount() const
{
  return static_cast<int>(m_groups.size());
}

/**
 * Returns @c true if the frame contains features that should only be enabled
 * for commercial users with a valid license, such as the 3D plot widget.
 */
bool JSON::Frame::containsCommercialFeatures() const
{
  return m_containsCommercialFeatures;
}

/**
 * Returns the title of the frame.
 */
const QString &JSON::Frame::title() const
{
  return m_title;
}

/**
 * Returns the frame end sequence.
 */
const QByteArray &JSON::Frame::frameEnd() const
{
  return m_frameEnd;
}

/**
 * Returns the frame start sequence.
 */
const QByteArray &JSON::Frame::frameStart() const
{
  return m_frameStart;
}

/**
 * Returns a vector of pointers to the @c Group objects associated to this
 * frame.
 */
const QVector<JSON::Group> &JSON::Frame::groups() const
{
  return m_groups;
}

/**
 * Returns a vector of pointers to the @c Action objects associated to this
 * frame.
 */
const QVector<JSON::Action> &JSON::Frame::actions() const
{
  return m_actions;
}
