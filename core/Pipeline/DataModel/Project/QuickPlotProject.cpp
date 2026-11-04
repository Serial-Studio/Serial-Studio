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
#include <QStringList>

#include "Core/DataModel/FrameKeys.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "DataModel/Generated/DeclarationVocabulary.h"

/**
 * @brief Upper bound on the groups a Quick Plot structure carries: a device declaration's cap,
 *        which also covers the detected data grid plus multi-plot.
 */
static constexpr size_t kMaxQuickPlotGroups = DataModel::Declaration::kMaxGroups;

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
 * @brief The keyed parser's field order: one title per frame index, from the first dataset at
 *        that index, so a wire name maps back to its channel and a positional slot keeps its
 *        place. A comma in a declared title becomes ';' (never a wire-name byte): the title keeps
 *        its one slot and, as live, matches no wire name.
 */
[[nodiscard]] static QString fieldList(const std::vector<DataModel::Group>& groups)
{
  SS_ASSERT(!groups.empty(), return QString());

  QStringList names;
  for (const auto& group : groups) {
    for (const auto& dataset : group.datasets) {
      const bool inRange = dataset.index >= 1 && dataset.index <= DataModel::kMaxLineFields;
      SS_ASSERT_LOG(inRange);
      if (!inRange || (dataset.index <= names.size() && !names.at(dataset.index - 1).isEmpty()))
        continue;

      while (names.size() < dataset.index)
        names.append(QString());

      names[dataset.index - 1] = QString(dataset.title).replace(QLatin1Char(','), QLatin1Char(';'));
    }
  }

  SS_ASSERT(names.size() <= DataModel::kMaxLineFields, return QString());
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
  params[QStringLiteral("fields")]       = format.named ? fieldList(groups) : QString();
  return params;
}

/**
 * @brief The declared binary framing and packed_struct decoder (spec 0111 R14) on @p source.
 */
static void applyBinaryFraming(DataModel::Source& source, const DataModel::Source& declared)
{
  SS_ASSERT(declared.decoderMethod == SerialStudio::Binary, return);
  SS_ASSERT(!declared.frameStart.isEmpty(), return);
  source.frameStart            = declared.frameStart;
  source.frameEnd              = declared.frameEnd;
  source.frameDetection        = declared.frameDetection;
  source.checksumAlgorithm     = declared.checksumAlgorithm;
  source.hexadecimalDelimiters = declared.hexadecimalDelimiters;
  source.decoderMethod         = declared.decoderMethod;
  source.frameParserTemplate   = declared.frameParserTemplate;
  source.frameParserParams     = declared.frameParserParams;
}

/**
 * @brief The device Quick Plot reads from, as a project source: newline-terminated plain text
 *        parsed by "delimited" for a plain comma stream and "line_values" otherwise (spec 0106
 *        R12). A declared session takes line_values, which skips declaration lines as the live
 *        lane does (spec 0111 R9), unless it declared a packed_struct source, kept as is (R14).
 */
[[nodiscard]] static DataModel::Source quickPlotSource(const int busType,
                                                       const QJsonObject& connectionSettings,
                                                       const DataModel::LineFormat& format,
                                                       const std::vector<DataModel::Group>& groups,
                                                       const bool declared,
                                                       const DataModel::Source& declaredSource)
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
  if (declared && declaredSource.frameParserTemplate == DataModel::Declaration::kBinaryTemplate) {
    applyBinaryFraming(source, declaredSource);
    return source;
  }

  if (!declared && plainCommaStream(format, groups)) {
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
 * @brief Rebuilds one Quick Plot group as a project group: every group and dataset setting carries
 *        over (titles, indices, plots, output controls, and the FFT settings Quick Plot derives for
 *        audio), live values and runtime ids are dropped, and a detected session logs every dataset
 *        while a declared one keeps the device's choice.
 */
[[nodiscard]] static DataModel::Group projectGroup(const DataModel::Group& live,
                                                   const int groupId,
                                                   const bool declared)
{
  DataModel::Group group;
  group.groupId       = groupId;
  group.title         = live.title;
  group.widget        = live.widget;
  group.sourceId      = 0;
  group.groupType     = live.groupType;
  group.columns       = live.columns;
  group.outputWidgets = live.outputWidgets;
  for (auto& widget : group.outputWidgets)
    widget.groupId = groupId;

  int datasetId = 0;
  for (const auto& liveDataset : live.datasets) {
    DataModel::Dataset dataset = liveDataset;
    dataset.groupId            = groupId;
    dataset.sourceId           = 0;
    dataset.datasetId          = datasetId++;
    dataset.uniqueId           = -1;
    dataset.xAxisId            = DataModel::kXAxisTime;
    dataset.waterfallYAxis     = 0;
    dataset.log                = declared ? liveDataset.log : true;
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
 *        detected; a non-empty @p declaredTitle names a device-declared session's project.
 */
QJsonObject DataModel::QuickPlotProject::build(const std::vector<Group>& quickPlotGroups,
                                               const int busType,
                                               const QJsonObject& connectionSettings,
                                               const LineFormat& format,
                                               const QString& declaredTitle,
                                               const Source& declaredSource)
{
  SS_ASSERT(!quickPlotGroups.empty(), return QJsonObject());
  SS_ASSERT(quickPlotGroups.size() <= kMaxQuickPlotGroups, return QJsonObject());

  const bool declared = !declaredTitle.isEmpty();
  QJsonArray groups;
  int groupId = 0;
  for (const auto& live : quickPlotGroups)
    groups.append(DataModel::serialize(projectGroup(live, groupId++, declared)));

  const auto source =
    quickPlotSource(busType, connectionSettings, format, quickPlotGroups, declared, declaredSource);

  QJsonObject project;
  project[Keys::Title]               = declared ? declaredTitle : QObject::tr("Quick Plot Project");
  project[Keys::Actions]             = QJsonArray();
  project[Keys::Sources]             = QJsonArray{DataModel::serialize(source)};
  project[Keys::Groups]              = groups;
  project[Keys::CustomizeWorkspaces] = false;
  return project;
}
