/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Project/QuickPlotProject.h"

#include <QJsonArray>
#include <QObject>

#include "Core/DataModel/FrameKeys.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"

/**
 * @brief Upper bound on the groups a Quick Plot structure carries (data grid plus multi-plot).
 */
static constexpr size_t kMaxQuickPlotGroups = 8;

/**
 * @brief The device Quick Plot reads from, expressed as a project source: newline-terminated
 *        plain text split on commas by the Built-In "delimited" template. Trimming fields absorbs
 *        the CR of CRLF lines, since a project source carries a single end delimiter.
 */
[[nodiscard]] static DataModel::Source quickPlotSource(const int busType,
                                                       const QJsonObject& connectionSettings)
{
  DataModel::Source source;
  source.sourceId            = 0;
  source.busType             = busType;
  source.title               = QObject::tr("Device A");
  source.frameEnd            = QStringLiteral("\\n");
  source.frameDetection      = static_cast<int>(SerialStudio::EndDelimiterOnly);
  source.decoderMethod       = static_cast<int>(SerialStudio::PlainText);
  source.frameParserLanguage = static_cast<int>(SerialStudio::Native);
  source.frameParserTemplate = QStringLiteral("delimited");
  source.connectionSettings  = connectionSettings;
  source.frameParserParams   = QJsonObject{
    { QStringLiteral("separator"), QStringLiteral(",")},
    { QStringLiteral("quoteChar"),           QString()},
    {QStringLiteral("trimFields"),                true},
    { QStringLiteral("skipEmpty"),               false}
  };
  return source;
}

/**
 * @brief Rebuilds one Quick Plot group as a project group: every dataset setting carries over
 *        (titles, indices, plots, and the FFT rate, size and ranges Quick Plot derives for audio),
 *        while live values and runtime ids are dropped so the loader assigns fresh project ids.
 */
[[nodiscard]] static DataModel::Group projectGroup(const DataModel::Group& live, const int groupId)
{
  DataModel::Group group;
  group.groupId  = groupId;
  group.title    = live.title;
  group.widget   = live.widget;
  group.sourceId = 0;

  int datasetId = 0;
  for (const auto& liveDataset : live.datasets) {
    DataModel::Dataset dataset = liveDataset;
    dataset.groupId            = groupId;
    dataset.sourceId           = 0;
    dataset.datasetId          = datasetId++;
    dataset.uniqueId           = -1;
    dataset.xAxisId            = DataModel::kXAxisTime;
    dataset.waterfallYAxis     = 0;
    dataset.log                = true;
    dataset.isNumeric          = false;
    dataset.numericValue       = 0;
    dataset.rawNumericValue    = 0;
    dataset.displaySampleMs    = 0;
    dataset.value.clear();
    dataset.rawValue.clear();
    group.datasets.push_back(dataset);
  }

  return group;
}

/**
 * @brief Assembles the project document for @p quickPlotGroups on a @p busType device that
 *        reconnects with @p connectionSettings.
 */
QJsonObject DataModel::QuickPlotProject::build(const std::vector<Group>& quickPlotGroups,
                                               const int busType,
                                               const QJsonObject& connectionSettings)
{
  SS_ASSERT(!quickPlotGroups.empty(), return QJsonObject());
  SS_ASSERT(quickPlotGroups.size() <= kMaxQuickPlotGroups, return QJsonObject());

  QJsonArray groups;
  int groupId = 0;
  for (const auto& live : quickPlotGroups)
    groups.append(DataModel::serialize(projectGroup(live, groupId++)));

  QJsonObject project;
  project[Keys::Title]   = QObject::tr("Quick Plot Project");
  project[Keys::Actions] = QJsonArray();
  project[Keys::Sources] =
    QJsonArray{DataModel::serialize(quickPlotSource(busType, connectionSettings))};
  project[Keys::Groups]              = groups;
  project[Keys::CustomizeWorkspaces] = false;
  return project;
}
