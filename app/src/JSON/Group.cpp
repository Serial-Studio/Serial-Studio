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

#include <QJsonArray>
#include "JSON/Group.h"

/**
 * @brief Constructor function
 */
JSON::Group::Group(const int groupId)
  : m_groupId(groupId)
  , m_title("")
  , m_widget("")
{
}

/**
 * Destructor function
 */
JSON::Group::~Group()
{
  m_datasets.clear();
}

/**
 * @brief Serializes the group information and its associated datasets into a
 * QJsonObject.
 *
 * This function encodes the group's properties (title and widget) and each
 * dataset within the group into a JSON object. Calls the `encode()` method for
 * each dataset to ensure that all dataset details are properly serialized.
 *
 * @return A QJsonObject containing the group's properties and an array of
 * encoded datasets.
 */
QJsonObject JSON::Group::serialize() const
{
  QJsonArray datasetArray;
  for (const auto &dataset : m_datasets)
    datasetArray.append(dataset.serialize());

  QJsonObject object;
  object.insert(QStringLiteral("title"), m_title.trimmed());
  object.insert(QStringLiteral("widget"), m_widget.trimmed());
  object.insert(QStringLiteral("datasets"), datasetArray);
  return object;
}

/**
 * Reads the group information and all its asociated datasets from the given
 * JSON @c object.
 *
 * @return @c true on success, @c false on failure
 */
bool JSON::Group::read(const QJsonObject &object)
{
  if (!object.isEmpty())
  {
    // clang-format off
    const auto array = object.value(QStringLiteral("datasets")).toArray();
    const auto title = object.value(QStringLiteral("title")).toString().trimmed();
    const auto widget = object.value(QStringLiteral("widget")).toString().trimmed();
    // clang-format on

    if (!title.isEmpty() && !array.isEmpty())
    {
      m_title = title;
      m_widget = widget;
      m_datasets.clear();

      for (auto i = 0; i < array.count(); ++i)
      {
        const auto object = array.at(i).toObject();
        if (!object.isEmpty())
        {
          Dataset dataset(m_groupId, i);
          if (dataset.read(object))
            m_datasets.append(dataset);
        }
      }

      return datasetCount() > 0;
    }
  }

  return false;
}

/**
 * @return The title/description of this group
 */
const QString &JSON::Group::title() const
{
  return m_title;
}

/**
 * @return The widget type of this group (if any)
 */
const QString &JSON::Group::widget() const
{
  return m_widget;
}

/**
 * @return The group groupId in the project array, only used for interacting
 *         with the project model (which is used to build the Project Editor
 *         GUI).
 */
int JSON::Group::groupId() const
{
  return m_groupId;
}

/**
 * @return The number of datasets inside this group
 */
int JSON::Group::datasetCount() const
{
  return m_datasets.count();
}

/**
 * @return A list with all the dataset objects contained in this group
 */
const QVector<JSON::Dataset> &JSON::Group::datasets() const
{
  return m_datasets;
}

/**
 * @return The dataset at the given @a index,vreturns @c nullptr on invalid
 * index
 */
const JSON::Dataset &JSON::Group::getDataset(const int index) const
{
  return m_datasets.at(index);
}
