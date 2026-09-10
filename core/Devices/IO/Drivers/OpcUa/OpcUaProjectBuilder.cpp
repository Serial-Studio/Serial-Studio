/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/OpcUa/OpcUaProjectBuilder.h"

#include <QCoreApplication>
#include <QHash>
#include <QStringList>

#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "Protocols/OpcUa/OpcUaWire.h"

//--------------------------------------------------------------------------------------------------
// Shared helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Translates a generated-project string in the OPC UA driver's context, which is where the
 *        catalogs already carry these entries.
 */
[[nodiscard]] static QString trOpcUaProject(const char* text)
{
  return QCoreApplication::translate("IO::Drivers::OpcUa", text);
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the builder to the tag list it translates; the list is copied so a generation can
 *        never observe an edit made while it runs.
 */
IO::Drivers::OpcUaProjectBuilder::OpcUaProjectBuilder(const QList<OpcUaTag>& tags) : m_tags(tags) {}

//--------------------------------------------------------------------------------------------------
// Project translation
//--------------------------------------------------------------------------------------------------

/**
 * @brief The `opcua` native template schema: one {i, t} entry per wire index.
 */
QJsonArray IO::Drivers::OpcUaProjectBuilder::wireSchema() const
{
  QJsonArray schema;
  int index = 0;
  for (const auto& tag : m_tags) {
    const auto code = OpcUaWire::codeFromType(wireTypeFor(tag));
    for (int i = 0; i < qMax(1, tag.arrayLen) && index < OpcUaWire::kMaxTags; ++i, ++index)
      schema.append(QJsonObject{
        { QStringLiteral("i"),      index},
        { QStringLiteral("t"),       code},
        {QStringLiteral("id"), tag.nodeId}
      });
  }

  return schema;
}

/**
 * @brief One dataset for a tag (or one array element): LED for booleans, plot for numerics.
 */
DataModel::Dataset IO::Drivers::OpcUaProjectBuilder::datasetFor(const OpcUaTag& tag,
                                                                int element,
                                                                int index)
{
  SS_ASSERT_LOG(index >= 1);
  const auto type = wireTypeFor(tag);

  DataModel::Dataset dataset;
  dataset.index = index;
  dataset.log   = true;
  dataset.units = tag.unit;
  dataset.title = qMax(1, tag.arrayLen) > 1
                  ? QStringLiteral("%1[%2]").arg(tag.name, QString::number(element))
                  : tag.name;

  if (type == OpcUaWire::Type::Bool) {
    dataset.led     = true;
    dataset.ledHigh = 1;
    dataset.wgtMax  = 1;
  } else if (type != OpcUaWire::Type::Str) {
    dataset.plt = true;
    if (tag.max > tag.min) {
      dataset.wgtMin = tag.min;
      dataset.wgtMax = tag.max;
      dataset.pltMin = tag.min;
      dataset.pltMax = tag.max;
    }
  }

  return dataset;
}

/**
 * @brief One group per parent folder, one dataset per wire index, the opcua native template. The
 *        connection settings are handed in because they are the driver's property model, not the
 *        tag list's.
 */
QJsonObject IO::Drivers::OpcUaProjectBuilder::buildProject(
  const QJsonObject& connectionSettings) const
{
  QJsonObject project;
  project[Keys::Title]   = trOpcUaProject("OPC UA Project");
  project[Keys::Actions] = QJsonArray();

  QJsonObject source;
  source[Keys::SourceId]              = 0;
  source[Keys::Title]                 = trOpcUaProject("OPC UA");
  source[Keys::BusType]               = static_cast<int>(SerialStudio::BusType::OpcUa);
  source[Keys::FrameStart]            = QString();
  source[Keys::FrameEnd]              = QString();
  source[Keys::Checksum]              = QString();
  source[Keys::FrameDetection]        = static_cast<int>(SerialStudio::NoDelimiters);
  source[Keys::Decoder]               = static_cast<int>(SerialStudio::Binary);
  source[Keys::HexadecimalDelimiters] = false;
  source[Keys::FrameParserCode]       = QString();
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Native);
  source[Keys::FrameParserTemplate]   = QStringLiteral("opcua");
  source[Keys::FrameParserParams]     = QJsonObject{
        {QStringLiteral("schema"), wireSchema()}
  };

  source[Keys::SourceConn] = connectionSettings;
  project[Keys::Sources]   = QJsonArray{source};

  QStringList order;
  QHash<QString, DataModel::Group> groups;
  int index = 0;
  for (const auto& tag : m_tags) {
    const QString key = tag.path.isEmpty() ? trOpcUaProject("Tags") : tag.path;
    if (!groups.contains(key)) {
      DataModel::Group group;
      group.groupId = order.size();
      group.widget  = QStringLiteral("datagrid");
      group.title   = key.section(QLatin1Char('/'), -1);
      groups.insert(key, group);
      order.append(key);
    }

    auto& group = groups[key];
    for (int i = 0; i < qMax(1, tag.arrayLen); ++i, ++index)
      group.datasets.push_back(datasetFor(tag, i, index + 1));
  }

  QJsonArray groupArray;
  for (const auto& key : order)
    groupArray.append(DataModel::serialize(groups.value(key)));

  project[Keys::Groups] = groupArray;
  return project;
}
