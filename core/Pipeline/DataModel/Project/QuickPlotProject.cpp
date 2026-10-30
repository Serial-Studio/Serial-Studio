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

#include <algorithm>
#include <QJsonArray>
#include <QObject>
#include <QStringList>

#include "Core/DataModel/FrameKeys.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"

/**
 * @brief Upper bound on the groups a Quick Plot structure carries (data grid plus multi-plot).
 */
static constexpr size_t kMaxQuickPlotGroups = 8;

/**
 * @brief True when the "delimited" comma source reproduces the stream: unwrapped positional comma
 *        values with no decimal comma and no unit on any dataset. Every other shape needs
 *        line_values, which shares Quick Plot's tokenizer.
 */
[[nodiscard]] static bool plainCommaStream(const DataModel::LineFormat& format,
                                           const std::vector<DataModel::Group>& groups)
{
  SS_ASSERT(!groups.empty(), return false);
  SS_ASSERT(format.separator == ',' || format.separator == ';' || format.separator == '\t'
              || format.separator == ' ' || format.separator == '\0',
            return false);
  if (format.kind != DataModel::LineFormatKind::Delimited || format.separator != ','
      || format.decimalComma || format.named || format.wrapper)
    return false;

  for (const auto& group : groups)
    for (const auto& dataset : group.datasets)
      if (!dataset.units.isEmpty())
        return false;

  return true;
}

/**
 * @brief The line_values separator option for a tokenizer separator byte; a single-field stream
 *        splits on commas, which its lines never contain.
 */
[[nodiscard]] static QString separatorName(const char separator)
{
  switch (separator) {
    case ';':
      return QStringLiteral("semicolon");
    case '\t':
      return QStringLiteral("tab");
    case ' ':
      return QStringLiteral("space");
    default:
      return QStringLiteral("comma");
  }
}

/**
 * @brief The keyed parser's field order: the data grid's dataset titles by frame index, so a wire
 *        name or JSON path maps back to the channel it created and a positional slot keeps its
 * place.
 */
[[nodiscard]] static QString fieldList(const DataModel::Group& datagrid)
{
  SS_ASSERT(!datagrid.datasets.empty(), return QString());
  SS_ASSERT(datagrid.datasets.size() <= static_cast<size_t>(DataModel::kMaxLineFields),
            return QString());

  std::vector<const DataModel::Dataset*> ordered;
  ordered.reserve(datagrid.datasets.size());
  for (const auto& dataset : datagrid.datasets)
    ordered.push_back(&dataset);

  std::sort(ordered.begin(), ordered.end(), [](const auto* a, const auto* b) {
    return a->index < b->index;
  });

  QStringList names;
  names.reserve(static_cast<int>(ordered.size()));
  for (const auto* dataset : ordered)
    names.append(dataset->title);

  return names.join(QLatin1Char(','));
}

/**
 * @brief The line_values parameters that pin the detected format (spec 0106 R12).
 */
[[nodiscard]] static QJsonObject lineValuesParams(const DataModel::LineFormat& format,
                                                  const std::vector<DataModel::Group>& groups)
{
  SS_ASSERT(!groups.empty(), return QJsonObject());
  const bool json = format.kind == DataModel::LineFormatKind::Json;

  QJsonObject params;
  params[QStringLiteral("format")]    = json ? QStringLiteral("json") : QStringLiteral("delimited");
  params[QStringLiteral("separator")] = separatorName(format.separator);
  params[QStringLiteral("decimalComma")] = format.decimalComma;
  params[QStringLiteral("fields")]       = format.named ? fieldList(groups.front()) : QString();
  return params;
}

/**
 * @brief The device Quick Plot reads from, expressed as a project source: newline-terminated
 *        plain text parsed by the Built-In template that reproduces the detected format, the
 *        "delimited" comma split for a plain comma stream and "line_values" otherwise (spec 0106
 *        R12). Trimming absorbs the CR of CRLF lines; a project source has one end delimiter.
 */
[[nodiscard]] static DataModel::Source quickPlotSource(const int busType,
                                                       const QJsonObject& connectionSettings,
                                                       const DataModel::LineFormat& format,
                                                       const std::vector<DataModel::Group>& groups)
{
  DataModel::Source source;
  source.sourceId            = 0;
  source.busType             = busType;
  source.title               = QObject::tr("Device A");
  source.frameEnd            = QStringLiteral("\\n");
  source.frameDetection      = static_cast<int>(SerialStudio::EndDelimiterOnly);
  source.decoderMethod       = static_cast<int>(SerialStudio::PlainText);
  source.frameParserLanguage = static_cast<int>(SerialStudio::Native);
  source.connectionSettings  = connectionSettings;
  if (plainCommaStream(format, groups)) {
    source.frameParserTemplate = QStringLiteral("delimited");
    source.frameParserParams   = QJsonObject{
      { QStringLiteral("separator"), QStringLiteral(",")},
      { QStringLiteral("quoteChar"),           QString()},
      {QStringLiteral("trimFields"),                true},
      { QStringLiteral("skipEmpty"),               false}
    };
    return source;
  }

  source.frameParserTemplate = QStringLiteral("line_values");
  source.frameParserParams   = lineValuesParams(format, groups);
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
 *        reconnects with @p connectionSettings and parses with the @p format the live lane
 * detected.
 */
QJsonObject DataModel::QuickPlotProject::build(const std::vector<Group>& quickPlotGroups,
                                               const int busType,
                                               const QJsonObject& connectionSettings,
                                               const LineFormat& format)
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
  project[Keys::Sources] = QJsonArray{
    DataModel::serialize(quickPlotSource(busType, connectionSettings, format, quickPlotGroups))};
  project[Keys::Groups]              = groups;
  project[Keys::CustomizeWorkspaces] = false;
  return project;
}
