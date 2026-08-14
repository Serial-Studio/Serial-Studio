/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <algorithm>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <QColor>
#include <QFile>
#include <QHash>
#include <QJSEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "API/CommandRegistry.h"
#include "API/EnumLabels.h"
#include "API/Handlers/ProjectHandler.h"
#include "API/PathPolicy.h"
#include "API/SchemaBuilder.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/CFrameParser.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/FrameParserPipeline.h"
#include "DataModel/Scripting/JsScriptEngine.h"
#include "DataModel/Scripting/LuaScriptEngine.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "IO/ConnectionManager.h"
#include "Misc/BackupManager.h"
#include "SerialStudio.h"
#ifdef BUILD_COMMERCIAL
#  include "UI/Widgets/Output/Base.h"
#endif
#include "ProjectApiSupport.h"

namespace API::Handlers {

//--------------------------------------------------------------------------------------------------
// Legacy alarm-field synthesis for MCP input compatibility
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies simple-mode alarmEnabled / alarmLow / alarmHigh fields to a dataset's alarmBands.
 *        Non-static: the generated dataset field appliers call it for the v3.3 legacy inputs.
 */
void applySimpleAlarmFields(DataModel::Dataset& d,
                            std::optional<bool> enabled,
                            std::optional<double> low,
                            std::optional<double> high)
{
  bool curEnabled       = !d.alarmBands.empty();
  double curLow         = std::numeric_limits<double>::quiet_NaN();
  double curHigh        = std::numeric_limits<double>::quiet_NaN();
  const double rangeMin = qMin(d.wgtMin, d.wgtMax);
  const double rangeMax = qMax(d.wgtMin, d.wgtMax);
  for (const auto& b : d.alarmBands) {
    if (static_cast<int>(b.severity) < 2)
      continue;

    const double bLo = qMin(b.min, b.max);
    const double bHi = qMax(b.min, b.max);
    if (qFuzzyCompare(1.0 + bLo, 1.0 + rangeMin) && bHi < rangeMax)
      curLow = bHi;
    else if (bLo > rangeMin && qFuzzyCompare(1.0 + bHi, 1.0 + rangeMax))
      curHigh = bLo;
  }

  const bool useEnabled = enabled.value_or(curEnabled);
  const double useLow   = low.value_or(curLow);
  const double useHigh  = high.value_or(curHigh);

  d.alarmBands.clear();
  if (!useEnabled)
    return;

  const double range = rangeMax - rangeMin;
  if (range <= 0)
    return;

  const double lo = std::isnan(useLow) ? rangeMin + range * 0.20 : useLow;
  const double hi = std::isnan(useHigh) ? rangeMin + range * 0.80 : useHigh;
  if (lo > rangeMin && lo < rangeMax) {
    DataModel::AlarmBand band;
    band.min      = rangeMin;
    band.max      = lo;
    band.severity = DataModel::AlarmSeverity::Warning;
    d.alarmBands.push_back(band);
  }

  if (hi > rangeMin && hi < rangeMax && hi > lo) {
    DataModel::AlarmBand band;
    band.min      = hi;
    band.max      = rangeMax;
    band.severity = DataModel::AlarmSeverity::Warning;
    d.alarmBands.push_back(band);
  }
}

/**
 * @brief Appends an unknown_field warning to @p result when @p params has unconsumed keys.
 */
static void appendUnknownFieldsWarning(QJsonObject& result,
                                       const QJsonObject& params,
                                       const QSet<QString>& consumed,
                                       const QString& command)
{
  QJsonArray unknownFields;
  for (const auto& key : params.keys())
    if (!consumed.contains(key))
      unknownFields.append(key);

  if (unknownFields.isEmpty())
    return;

  QJsonArray warnings;
  if (result.contains(QStringLiteral("warnings")))
    warnings = result.value(QStringLiteral("warnings")).toArray();

  QJsonObject w;
  w[QStringLiteral("code")]   = QStringLiteral("unknown_field");
  w[QStringLiteral("fields")] = unknownFields;
  w[QStringLiteral("message")] =
    QStringLiteral("These fields were ignored because they are not patchable via %1. "
                   "Call meta.describeCommand for the list of writable fields, "
                   "or check your spelling.")
      .arg(command);
  warnings.append(w);

  result[QStringLiteral("warnings")] = warnings;
}

}  // namespace API::Handlers

/**
 * @brief Add group
 */
API::CommandResponse API::Handlers::ProjectHandler::groupAdd(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("title"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: title"));
  }

  if (!params.contains(QStringLiteral("widgetType"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetType"));
  }

  const QString title = params.value(QStringLiteral("title")).toString();
  if (title.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("title cannot be empty"));
  }

  const int widget_type = params.value(QStringLiteral("widgetType")).toInt();
  if (widget_type < 0 || widget_type > static_cast<int>(SerialStudio::BarPanel)) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid widgetType: must be 0..%1 "
                     "(see GroupWidget enum: 0=DataGrid, 1=Accelerometer, "
                     "2=Gyroscope, 3=GPS, 4=MultiPlot, 5=NoGroupWidget, "
                     "6=Plot3D, 7=ImageView, 8=Painter, 9=WebView, 10=BarPanel)")
        .arg(static_cast<int>(SerialStudio::BarPanel)));
  }

  const auto widget         = static_cast<SerialStudio::GroupWidget>(widget_type);
  static auto& projectModel = DataModel::ProjectModel::instance();
  projectModel.addGroup(title, widget);

  QJsonObject result;
  result[QStringLiteral("title")]      = title;
  result[QStringLiteral("widgetType")] = widget_type;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete a group by id
 */
API::CommandResponse API::Handlers::ProjectHandler::groupDelete(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& targetGroup = groups[groupId];
  QJsonArray childDatasets;
  for (const auto& d : targetGroup.datasets) {
    QJsonObject row;
    row[Keys::DatasetId]         = d.datasetId;
    row[Keys::UniqueId]          = d.uniqueId;
    row[QStringLiteral("title")] = d.title;
    childDatasets.append(row);
  }

  QJsonObject deleted;
  deleted[QStringLiteral("groupId")]      = groupId;
  deleted[QStringLiteral("title")]        = targetGroup.title;
  deleted[QStringLiteral("widget")]       = targetGroup.widget;
  deleted[QStringLiteral("datasetCount")] = childDatasets.size();
  deleted[QStringLiteral("datasets")]     = childDatasets;

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);

  QJsonArray renumbered;
  for (const auto& g : groups) {
    if (g.groupId <= groupId)
      continue;

    QJsonObject row;
    row[QStringLiteral("oldGroupId")]   = g.groupId;
    row[QStringLiteral("newGroupId")]   = g.groupId - 1;
    row[QStringLiteral("title")]        = g.title;
    row[QStringLiteral("datasetCount")] = static_cast<int>(g.datasets.size());
    renumbered.append(row);
  }

  QString backupPath;
  qint64 preEpoch = 0;
  if (!isDryRun) {
    static auto& backupManager = Misc::BackupManager::instance();
    backupPath                 = backupManager.snapshot(QStringLiteral("pre-groupDelete"));
    preEpoch                   = captureProjectEpoch();
    project.deleteGroup(groupId);
  }

  QJsonObject result;
  if (isDryRun)
    result[QStringLiteral("dryRun")] = true;

  result[QStringLiteral("deleted")]    = deleted;
  result[QStringLiteral("renumbered")] = renumbered;
  if (!backupPath.isEmpty())
    result[QStringLiteral("backupPath")] = backupPath;

  QJsonArray warnings;
  if (isDryRun)
    warnings.append(QStringLiteral(
      "DRY RUN: no changes were written. Re-call without dryRun:true to commit. The "
      "renumbered[] array shows groupId values that WOULD shift; every dataset in those "
      "groups would have its uniqueId invalidated."));

  else if (!renumbered.isEmpty())
    warnings.append(QStringLiteral(
      "groupId values shifted after deletion; uniqueIds of every dataset in renumbered "
      "groups are now stale -- re-read project state before further mutations."));

  if (!isDryRun && !backupPath.isEmpty())
    warnings.append(QStringLiteral(
      "Pre-mutation snapshot saved at backupPath; pass it to assistant.restore to undo."));

  if (!warnings.isEmpty())
    result[QStringLiteral("warnings")] = warnings;

  if (!isDryRun)
    appendStaleProjectWarning(result, params, preEpoch);

  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate a group by id
 */
API::CommandResponse API::Handlers::ProjectHandler::groupDuplicate(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  project.duplicateGroup(groupId);

  QJsonObject result;
  result[QStringLiteral("groupId")]    = groupId;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns the widget string matching a one-of DatasetOption bit.
 */
QString API::Handlers::ProjectHandler::widgetForDatasetOptions(int options)
{
  if (options & SerialStudio::DatasetMeter)
    return QStringLiteral("meter");

  if (options & SerialStudio::DatasetCompass)
    return QStringLiteral("compass");

  if (options & SerialStudio::DatasetGauge)
    return QStringLiteral("gauge");

  if (options & SerialStudio::DatasetBar)
    return QStringLiteral("bar");

  return QString();
}

/**
 * @brief Sets plt/fft/led/waterfall/widget on @p d from a DatasetOption bitfield.
 */
void API::Handlers::ProjectHandler::applyDatasetVisualizationFlags(DataModel::Dataset& d,
                                                                   int options)
{
  if (options & SerialStudio::DatasetPlot)
    d.plt = true;

  if (options & SerialStudio::DatasetFFT)
    d.fft = true;

  if (options & SerialStudio::DatasetLED)
    d.led = true;

  if (options & SerialStudio::DatasetWaterfall)
    d.waterfall = true;

  const QString widget = widgetForDatasetOptions(options);
  if (!widget.isEmpty())
    d.widget = widget;
}

/**
 * @brief Add a dataset to a specific group by id
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetAdd(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("options")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: options"));

  const int groupId = params.value(QStringLiteral("groupId")).toInt();
  const int options = params.value(QStringLiteral("options")).toInt();
  if (options < 0 || options > 0b11111111)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid options: must be 0-255 (bit flags)"));

  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  SerialStudio::DatasetOption headline = SerialStudio::DatasetGeneric;
  for (const auto cand : {SerialStudio::DatasetPlot,
                          SerialStudio::DatasetFFT,
                          SerialStudio::DatasetBar,
                          SerialStudio::DatasetGauge,
                          SerialStudio::DatasetCompass,
                          SerialStudio::DatasetLED,
                          SerialStudio::DatasetWaterfall,
                          SerialStudio::DatasetMeter}) {
    if (options & cand) {
      headline = cand;
      break;
    }
  }

  project.setSelectedGroup(groups[groupId]);
  project.addDataset(headline);

  const int remaining = options & ~static_cast<int>(headline);
  const auto& post    = project.groups();
  const int newIndex  = static_cast<int>(post[groupId].datasets.size()) - 1;
  if (remaining != 0 && newIndex >= 0) {
    DataModel::Dataset d = post[groupId].datasets[newIndex];
    applyDatasetVisualizationFlags(d, remaining);
    project.updateDataset(groupId, newIndex, d, true);
  }

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[QStringLiteral("options")] = options;
  return CommandResponse::makeSuccess(id, result);
}

namespace API::Handlers {

/**
 * @brief Add several datasets to a group in one call (bulk creation).
 */
/**
 * @brief Picks the headline visualization flag from an options bitfield.
 */
static SerialStudio::DatasetOption pickHeadlineDatasetOption(int options)
{
  for (const auto cand : {SerialStudio::DatasetPlot,
                          SerialStudio::DatasetFFT,
                          SerialStudio::DatasetBar,
                          SerialStudio::DatasetGauge,
                          SerialStudio::DatasetCompass,
                          SerialStudio::DatasetLED,
                          SerialStudio::DatasetWaterfall,
                          SerialStudio::DatasetMeter}) {
    if (options & cand)
      return cand;
  }
  return SerialStudio::DatasetGeneric;
}

}  // namespace API::Handlers

/**
 * @brief Bulk-creates N datasets in one call with optional title/index patterns.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetAddMany(const QString& id,
                                                                   const QJsonObject& params)
{
  constexpr int kMaxAddManyCount = 1024;

  for (const auto& key :
       {QStringLiteral("groupId"), QStringLiteral("count"), QStringLiteral("options")}) {
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));
  }

  const int groupId = params.value(QStringLiteral("groupId")).toInt();
  const int count   = params.value(QStringLiteral("count")).toInt();
  const int options = params.value(QStringLiteral("options")).toInt();

  if (count <= 0 || count > kMaxAddManyCount)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid count: must be 1..%1 (got %2)")
        .arg(QString::number(kMaxAddManyCount), QString::number(count)));

  if (options < 0 || options > 0b11111111)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid options: must be 0-255 (bit flags)"));

  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const QString titlePattern = params.value(QStringLiteral("titlePattern")).toString();
  const int startNumber      = params.contains(QStringLiteral("startNumber"))
                               ? params.value(QStringLiteral("startNumber")).toInt()
                               : 1;
  const int startIndex       = params.contains(QStringLiteral("startIndex"))
                               ? params.value(QStringLiteral("startIndex")).toInt()
                               : -1;

  if (startIndex < -1)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid startIndex: must be -1 (auto), 0, or 1+"));

  const auto headline     = pickHeadlineDatasetOption(options);
  const int remainingBits = options & ~static_cast<int>(headline);

  project.setAutoSaveSuspended(true);
  project.setSelectedGroup(groups[groupId]);

  QJsonArray created;
  for (int i = 0; i < count; ++i) {
    project.addDataset(headline);

    const auto& post   = project.groups();
    const int newIndex = static_cast<int>(post[groupId].datasets.size()) - 1;
    if (newIndex < 0)
      continue;

    DataModel::Dataset d = post[groupId].datasets[newIndex];
    if (remainingBits != 0)
      applyDatasetVisualizationFlags(d, remainingBits);

    if (!titlePattern.isEmpty()) {
      QString title = titlePattern;
      title.replace(QStringLiteral("{n}"), QString::number(startNumber + i));
      title.replace(QStringLiteral("{i}"), QString::number(i));
      d.title = title;
    }

    if (startIndex >= 0)
      d.index = startIndex + i;

    project.updateDataset(groupId, newIndex, d, true);

    QJsonObject entry;
    entry[QStringLiteral("groupId")] = groupId;
    entry[Keys::DatasetId]           = d.datasetId;
    entry[Keys::Title]               = d.title;
    entry[QStringLiteral("index")]   = d.index;
    entry[Keys::UniqueId]            = d.uniqueId;
    created.append(entry);
  }

  project.setAutoSaveSuspended(false);
  project.flushAutoSave();

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[QStringLiteral("count")]   = created.size();
  result[QStringLiteral("created")] = created;
  return CommandResponse::makeSuccess(id, result);
}

namespace API::Handlers {

/**
 * @brief Build the warnings array for a dataset-delete response.
 */
static QJsonArray buildDatasetDeleteWarnings(int groupId,
                                             bool isDryRun,
                                             const QJsonArray& renumbered,
                                             const QString& backupPath)
{
  QJsonArray warnings;
  if (isDryRun)
    warnings.append(
      QStringLiteral(
        "DRY RUN: no changes were written. Re-call without dryRun:true to commit. The "
        "renumbered[] array shows datasetId/uniqueId values that WOULD shift in groupId=%1.")
        .arg(groupId));

  else if (!renumbered.isEmpty())
    warnings.append(
      QStringLiteral(
        "datasetId values in groupId=%1 were renumbered; cached uniqueIds for the affected "
        "datasets are now stale -- re-read project state before further mutations.")
        .arg(groupId));

  if (!isDryRun && !backupPath.isEmpty())
    warnings.append(QStringLiteral(
      "Pre-mutation snapshot saved at backupPath; pass it to assistant.restore to undo."));

  return warnings;
}

}  // namespace API::Handlers

/**
 * @brief Delete a dataset by id, returning the deleted entity + any renumbered peers.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetDelete(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(Keys::DatasetId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: datasetId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const int datasetId  = params.value(Keys::DatasetId).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found: %1 in group %2")
                                        .arg(QString::number(datasetId), QString::number(groupId)));

  const auto& targetGroup   = groups[groupId];
  const auto& targetDataset = targetGroup.datasets[datasetId];

  QJsonObject deleted;
  deleted[QStringLiteral("groupId")]    = groupId;
  deleted[QStringLiteral("groupTitle")] = targetGroup.title;
  deleted[Keys::DatasetId]              = datasetId;
  deleted[Keys::UniqueId]               = targetDataset.uniqueId;
  deleted[QStringLiteral("title")]      = targetDataset.title;
  if (!targetDataset.units.isEmpty())
    deleted[QStringLiteral("units")] = targetDataset.units;

  if (!targetDataset.transformCode.isEmpty()) {
    deleted[QStringLiteral("hadTransform")]       = true;
    deleted[QStringLiteral("transformByteCount")] = targetDataset.transformCode.size();
  }

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);

  QJsonArray renumbered;
  for (const auto& d : targetGroup.datasets) {
    if (d.datasetId <= datasetId)
      continue;

    QJsonObject row;
    row[QStringLiteral("groupId")]      = groupId;
    row[QStringLiteral("oldDatasetId")] = d.datasetId;
    row[QStringLiteral("newDatasetId")] = d.datasetId - 1;
    row[Keys::UniqueId]                 = d.uniqueId;
    row[QStringLiteral("title")]        = d.title;
    renumbered.append(row);
  }

  QString backupPath;
  qint64 preEpoch = 0;
  if (!isDryRun) {
    static auto& backupManager = Misc::BackupManager::instance();
    backupPath                 = backupManager.snapshot(QStringLiteral("pre-datasetDelete"));
    preEpoch                   = captureProjectEpoch();
    project.deleteDataset(groupId, datasetId);
  }

  QJsonObject result;
  if (isDryRun)
    result[QStringLiteral("dryRun")] = true;

  result[QStringLiteral("deleted")]    = deleted;
  result[QStringLiteral("renumbered")] = renumbered;
  if (!backupPath.isEmpty())
    result[QStringLiteral("backupPath")] = backupPath;

  const auto warnings = buildDatasetDeleteWarnings(groupId, isDryRun, renumbered, backupPath);
  if (!warnings.isEmpty())
    result[QStringLiteral("warnings")] = warnings;

  if (!isDryRun)
    appendStaleProjectWarning(result, params, preEpoch);

  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate a dataset by id
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetDuplicate(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(Keys::DatasetId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: datasetId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const int datasetId  = params.value(Keys::DatasetId).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found: %1 in group %2")
                                        .arg(QString::number(datasetId), QString::number(groupId)));

  project.duplicateDataset(groupId, datasetId);

  QJsonObject result;
  result[QStringLiteral("groupId")]    = groupId;
  result[Keys::DatasetId]              = datasetId;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Toggle a dataset option by id.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetOption(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(Keys::DatasetId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: datasetId"));

  if (!params.contains(QStringLiteral("option")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: option"));

  if (!params.contains(QStringLiteral("enabled")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));

  const int groupId   = params.value(QStringLiteral("groupId")).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  const bool enabled  = params.value(QStringLiteral("enabled")).toBool();

  int option                  = 0;
  const QJsonValue optionJson = params.value(QStringLiteral("option"));
  if (optionJson.isString()) {
    option = API::EnumLabels::datasetOptionFromSlug(optionJson.toString());
    if (option == 0)
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Unknown option slug '%1'. Use one of: plot, fft, bar, gauge, "
                       "compass, led, waterfall.")
          .arg(optionJson.toString()));
  } else {
    option = optionJson.toInt();
  }

  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found: %1 in group %2")
                                        .arg(QString::number(datasetId), QString::number(groupId)));

  project.setSelectedDataset(groups[groupId].datasets[datasetId]);
  project.changeDatasetOption(static_cast<SerialStudio::DatasetOption>(option), enabled);

  QJsonObject result;
  result[QStringLiteral("groupId")]    = groupId;
  result[Keys::DatasetId]              = datasetId;
  result[QStringLiteral("option")]     = option;
  result[QStringLiteral("optionSlug")] = API::EnumLabels::datasetOptionSlug(option);
  result[QStringLiteral("enabled")]    = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Apply a bitmask of DatasetOption flags in one call.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetOptions(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(Keys::DatasetId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: datasetId"));

  if (!params.contains(QStringLiteral("options")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: options"));

  const int groupId   = params.value(QStringLiteral("groupId")).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();

  int options                  = 0;
  const QJsonValue optionsJson = params.value(QStringLiteral("options"));
  if (optionsJson.isArray()) {
    QStringList slugs;
    for (const auto& v : optionsJson.toArray())
      slugs.append(v.toString());

    options = API::EnumLabels::datasetOptionsSlugsToBits(slugs);
  } else {
    options = optionsJson.toInt();
  }

  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found: %1 in group %2")
                                        .arg(QString::number(datasetId), QString::number(groupId)));

  DataModel::Dataset d = groups[groupId].datasets[datasetId];
  d.plt                = (options & SerialStudio::DatasetPlot) != 0;
  d.fft                = (options & SerialStudio::DatasetFFT) != 0;
  d.led                = (options & SerialStudio::DatasetLED) != 0;
  d.waterfall          = (options & SerialStudio::DatasetWaterfall) != 0;

  const QString chosen = widgetForDatasetOptions(options);
  const bool wasOneOf  = d.widget == QStringLiteral("bar") || d.widget == QStringLiteral("gauge")
                     || d.widget == QStringLiteral("compass")
                     || d.widget == QStringLiteral("meter");
  if (!chosen.isEmpty())
    d.widget = chosen;
  else if (wasOneOf)
    d.widget = QString();

  project.updateDataset(groupId, datasetId, d, true);

  QJsonArray slugs;
  for (const auto& s : API::EnumLabels::datasetOptionsBitsToSlugs(options))
    slugs.append(s);

  QJsonObject result;
  result[QStringLiteral("groupId")]      = groupId;
  result[Keys::DatasetId]                = datasetId;
  result[QStringLiteral("options")]      = options;
  result[QStringLiteral("optionsSlugs")] = slugs;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Dataset field setters (v3.3)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Toggle the @c virtual_ flag on the dataset identified by (groupId, datasetId).
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetVirtual(const QString& id,
                                                                      const QJsonObject& params)
{
  const QStringList required{
    QString(Keys::GroupId),
    QString(Keys::DatasetId),
    QString(Keys::Virtual),
  };

  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  const bool isVirt   = params.value(Keys::Virtual).toBool();

  static auto& pm    = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  const auto git     = std::find_if(
    groups.begin(), groups.end(), [groupId](const auto& g) { return g.groupId == groupId; });

  if (git == groups.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = git->datasets;
  const auto dit       = std::find_if(datasets.begin(), datasets.end(), [datasetId](const auto& d) {
    return d.datasetId == datasetId;
  });

  if (dit == datasets.end())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found in group: %1/%2")
                                        .arg(QString::number(groupId), QString::number(datasetId)));

  DataModel::Dataset updated = *dit;
  updated.virtual_           = isVirt;
  pm.updateDataset(groupId, datasetId, updated, true);

  QJsonObject result;
  result[Keys::GroupId]             = groupId;
  result[Keys::DatasetId]           = datasetId;
  result[Keys::Virtual]             = isVirt;
  result[QStringLiteral("updated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the per-dataset transform code (Lua or JS; language matches the dataset's owning
 * source).
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetTransformCode(
  const QString& id, const QJsonObject& params)
{
  const QStringList required{
    QString(Keys::GroupId),
    QString(Keys::DatasetId),
    QStringLiteral("code"),
  };

  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  const QString code  = params.value(QStringLiteral("code")).toString();

  static auto& pm    = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  const auto git     = std::find_if(
    groups.begin(), groups.end(), [groupId](const auto& g) { return g.groupId == groupId; });

  if (git == groups.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = git->datasets;
  const auto dit       = std::find_if(datasets.begin(), datasets.end(), [datasetId](const auto& d) {
    return d.datasetId == datasetId;
  });

  if (dit == datasets.end())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found in group: %1/%2")
                                        .arg(QString::number(groupId), QString::number(datasetId)));

  DataModel::Dataset updated = *dit;
  updated.transformCode      = code;

  bool languageInherited = false;
  if (params.contains(QStringLiteral("language"))) {
    const int lang = params.value(QStringLiteral("language")).toInt();
    if (lang != SerialStudio::JavaScript && lang != SerialStudio::Lua)
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Invalid language: must be 0 (JavaScript) or 1 (Lua)"));

    updated.transformLanguage = lang;
  } else if (!code.isEmpty() && updated.transformLanguage < 0) {
    const auto& srcs = pm.sources();
    const auto sit   = std::find_if(
      srcs.begin(), srcs.end(), [&](const auto& s) { return s.sourceId == updated.sourceId; });
    int inherited = (sit != srcs.end()) ? sit->frameParserLanguage : 0;
    if (inherited == SerialStudio::Native)
      inherited = SerialStudio::Lua;

    updated.transformLanguage = inherited;
    languageInherited         = true;
  }

  pm.updateDataset(groupId, datasetId, updated, true);

  QJsonObject result;
  result[Keys::GroupId]                = groupId;
  result[Keys::DatasetId]              = datasetId;
  result[QStringLiteral("codeLength")] = code.size();
  result[QStringLiteral("language")]   = updated.transformLanguage;
  result[QStringLiteral("updated")]    = true;

  if (languageInherited) {
    result[QStringLiteral("languageInherited")] = true;
    result[QStringLiteral("inheritNotice")] =
      QStringLiteral("language was not provided; inherited from source "
                     "frameParserLanguage (%1). Pass language explicitly "
                     "to silence this notice.")
        .arg(updated.transformLanguage == 1 ? QStringLiteral("Lua") : QStringLiteral("JavaScript"));
  }

  if (!code.isEmpty() && updated.transformLanguage != -1) {
    const auto warning = detectLanguageMismatch(code, updated.transformLanguage);
    if (!warning.isEmpty())
      result[QStringLiteral("warning")] = warning;
  }

  if (!code.isEmpty() && !updated.virtual_ && updated.index <= 0)
    result[QStringLiteral("hint")] =
      QStringLiteral("transformCode set but virtual=false and index<=0; if this "
                     "dataset has no slot in the parser output array, set "
                     "virtual=true via project.dataset.update{virtual:true} "
                     "or the dataset will read empty channel data.");

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns the dataset's alarm bands as a JSON array.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetGetAlarmBands(const QString& id,
                                                                         const QJsonObject& params)
{
  const QStringList required{QString(Keys::GroupId), QString(Keys::DatasetId)};
  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();

  static auto& pm    = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  const auto git     = std::find_if(
    groups.begin(), groups.end(), [groupId](const auto& g) { return g.groupId == groupId; });

  if (git == groups.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = git->datasets;
  const auto dit       = std::find_if(datasets.begin(), datasets.end(), [datasetId](const auto& d) {
    return d.datasetId == datasetId;
  });

  if (dit == datasets.end())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found in group: %1/%2")
                                        .arg(QString::number(groupId), QString::number(datasetId)));

  QJsonArray bands;
  for (const auto& b : dit->alarmBands)
    bands.append(DataModel::serialize(b));

  QJsonObject result;
  result[Keys::GroupId]              = groupId;
  result[Keys::DatasetId]            = datasetId;
  result[Keys::AlarmBands]           = bands;
  result[QStringLiteral("count")]    = bands.size();
  result[QStringLiteral("rangeMin")] = qMin(dit->wgtMin, dit->wgtMax);
  result[QStringLiteral("rangeMax")] = qMax(dit->wgtMin, dit->wgtMax);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Atomic write of the full alarmBands array onto a dataset.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetAlarmBands(const QString& id,
                                                                         const QJsonObject& params)
{
  const QStringList required{
    QString(Keys::GroupId), QString(Keys::DatasetId), QString(Keys::AlarmBands)};
  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  const auto arr      = params.value(Keys::AlarmBands).toArray();

  static auto& pm    = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  const auto git     = std::find_if(
    groups.begin(), groups.end(), [groupId](const auto& g) { return g.groupId == groupId; });

  if (git == groups.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = git->datasets;
  const auto dit       = std::find_if(datasets.begin(), datasets.end(), [datasetId](const auto& d) {
    return d.datasetId == datasetId;
  });

  if (dit == datasets.end())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found in group: %1/%2")
                                        .arg(QString::number(groupId), QString::number(datasetId)));

  DataModel::Dataset updated = *dit;
  updated.alarmBands.clear();
  updated.alarmBands.reserve(arr.size());

  int dropped = 0;
  for (const auto& v : arr) {
    DataModel::AlarmBand b;
    if (DataModel::read(b, v.toObject()))
      updated.alarmBands.push_back(std::move(b));
    else
      ++dropped;
  }

  pm.updateDataset(groupId, datasetId, updated, true);

  QJsonObject result;
  result[Keys::GroupId]             = groupId;
  result[Keys::DatasetId]           = datasetId;
  result[QStringLiteral("count")]   = static_cast<int>(updated.alarmBands.size());
  result[QStringLiteral("updated")] = true;
  if (dropped > 0)
    result[QStringLiteral("droppedInvalid")] = dropped;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns the dataset's FFT frequency markers as a JSON array.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetGetFFTMarkers(const QString& id,
                                                                         const QJsonObject& params)
{
  const QStringList required{QString(Keys::GroupId), QString(Keys::DatasetId)};
  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();

  static auto& pm    = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  const auto git     = std::find_if(
    groups.begin(), groups.end(), [groupId](const auto& g) { return g.groupId == groupId; });

  if (git == groups.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = git->datasets;
  const auto dit       = std::find_if(datasets.begin(), datasets.end(), [datasetId](const auto& d) {
    return d.datasetId == datasetId;
  });

  if (dit == datasets.end())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found in group: %1/%2")
                                        .arg(QString::number(groupId), QString::number(datasetId)));

  QJsonArray markers;
  for (const auto& m : dit->fftMarkers)
    markers.append(DataModel::serialize(m));

  QJsonObject result;
  result[Keys::GroupId]             = groupId;
  result[Keys::DatasetId]           = datasetId;
  result[Keys::FFTMarkers]          = markers;
  result[QStringLiteral("count")]   = markers.size();
  result[QStringLiteral("nyquist")] = qMax(1, dit->fftSamplingRate) * 0.5;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Atomic write of the full fftMarkers array onto a dataset.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetFFTMarkers(const QString& id,
                                                                         const QJsonObject& params)
{
  const QStringList required{
    QString(Keys::GroupId), QString(Keys::DatasetId), QString(Keys::FFTMarkers)};
  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  const auto arr      = params.value(Keys::FFTMarkers).toArray();

  static auto& pm    = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  const auto git     = std::find_if(
    groups.begin(), groups.end(), [groupId](const auto& g) { return g.groupId == groupId; });

  if (git == groups.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = git->datasets;
  const auto dit       = std::find_if(datasets.begin(), datasets.end(), [datasetId](const auto& d) {
    return d.datasetId == datasetId;
  });

  if (dit == datasets.end())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found in group: %1/%2")
                                        .arg(QString::number(groupId), QString::number(datasetId)));

  DataModel::Dataset updated = *dit;
  updated.fftMarkers.clear();
  updated.fftMarkers.reserve(arr.size());

  int dropped = 0;
  for (const auto& v : arr) {
    DataModel::FrequencyMarker m;
    if (DataModel::read(m, v.toObject()))
      updated.fftMarkers.push_back(std::move(m));
    else
      ++dropped;
  }

  pm.updateDataset(groupId, datasetId, updated, true);

  QJsonObject result;
  result[Keys::GroupId]             = groupId;
  result[Keys::DatasetId]           = datasetId;
  result[QStringLiteral("count")]   = static_cast<int>(updated.fftMarkers.size());
  result[QStringLiteral("updated")] = true;
  if (dropped > 0)
    result[QStringLiteral("droppedInvalid")] = dropped;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Add action
 */
API::CommandResponse API::Handlers::ProjectHandler::actionAdd(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& projectModel = DataModel::ProjectModel::instance();
  projectModel.addAction();

  QJsonObject result;
  result[QStringLiteral("added")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete an action by id
 */
API::CommandResponse API::Handlers::ProjectHandler::actionDelete(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("actionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: actionId"));

  const int actionId   = params.value(QStringLiteral("actionId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& actions  = project.actions();
  if (actionId < 0 || static_cast<size_t>(actionId) >= actions.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Action id not found: %1").arg(actionId));

  project.deleteAction(actionId);

  QJsonObject result;
  result[QStringLiteral("actionId")] = actionId;
  result[QStringLiteral("deleted")]  = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate an action by id
 */
API::CommandResponse API::Handlers::ProjectHandler::actionDuplicate(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("actionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: actionId"));

  const int actionId   = params.value(QStringLiteral("actionId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& actions  = project.actions();
  if (actionId < 0 || static_cast<size_t>(actionId) >= actions.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Action id not found: %1").arg(actionId));

  project.duplicateAction(actionId);

  QJsonObject result;
  result[QStringLiteral("actionId")]   = actionId;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Output widget commands
//--------------------------------------------------------------------------------------------------

/**
 * @brief Add an output widget to the specified group.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetAdd(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const int type       = params.value(QStringLiteral("type")).toInt(0);
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  project.setSelectedGroup(groups[groupId]);
  project.addOutputControl(static_cast<SerialStudio::OutputWidgetType>(
    qBound(0, type, static_cast<int>(SerialStudio::OutputKnob))));

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[QStringLiteral("type")]    = type;
  result[QStringLiteral("added")]   = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete an output widget by id.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetDelete(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("widgetId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const int widgetId   = params.value(QStringLiteral("widgetId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (widgetId < 0 || static_cast<size_t>(widgetId) >= groups[groupId].outputWidgets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Widget id not found: %1 in group %2")
                                        .arg(QString::number(widgetId), QString::number(groupId)));

  project.deleteOutputWidget(groupId, widgetId);

  QJsonObject result;
  result[QStringLiteral("groupId")]  = groupId;
  result[QStringLiteral("widgetId")] = widgetId;
  result[QStringLiteral("deleted")]  = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate an output widget by id.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetDuplicate(const QString& id,
                                                                          const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("widgetId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const int widgetId   = params.value(QStringLiteral("widgetId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (widgetId < 0 || static_cast<size_t>(widgetId) >= groups[groupId].outputWidgets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Widget id not found: %1 in group %2")
                                        .arg(QString::number(widgetId), QString::number(groupId)));

  project.duplicateOutputWidget(groupId, widgetId);

  QJsonObject result;
  result[QStringLiteral("groupId")]    = groupId;
  result[QStringLiteral("widgetId")]   = widgetId;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Read the project configuration of one output widget.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetGet(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("widgetId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetId"));

  const int groupId         = params.value(QStringLiteral("groupId")).toInt();
  const int widgetId        = params.value(QStringLiteral("widgetId")).toInt();
  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto& groups        = projectModel.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& widgets = groups[groupId].outputWidgets;
  if (widgetId < 0 || static_cast<size_t>(widgetId) >= widgets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Widget id not found: %1 in group %2")
                                        .arg(QString::number(widgetId), QString::number(groupId)));

  const auto& w = widgets[widgetId];
  QJsonObject result;
  result[QStringLiteral("groupId")]          = groupId;
  result[QStringLiteral("widgetId")]         = w.widgetId;
  result[QStringLiteral("type")]             = static_cast<int>(w.type);
  result[QStringLiteral("title")]            = w.title;
  result[QStringLiteral("icon")]             = w.icon;
  result[QStringLiteral("monoIcon")]         = w.monoIcon;
  result[QStringLiteral("minValue")]         = w.minValue;
  result[QStringLiteral("maxValue")]         = w.maxValue;
  result[QStringLiteral("stepSize")]         = w.stepSize;
  result[QStringLiteral("initialValue")]     = w.initialValue;
  result[Keys::SourceId]                     = w.sourceId;
  result[QStringLiteral("txEncoding")]       = w.txEncoding;
  result[QStringLiteral("transmitFunction")] = w.transmitFunction;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Bulk update mutators: stateless, id required, PATCH semantics
//--------------------------------------------------------------------------------------------------

/**
 * @brief Patch any subset of group fields by id.
 */
API::CommandResponse API::Handlers::ProjectHandler::groupUpdate(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  DataModel::Group g = groups[groupId];
  bool rebuildTree   = false;
  QSet<QString> consumed{QStringLiteral("groupId")};
  const auto identityKeyCount = consumed.size();

  const auto take = [&](const QString& key) -> bool {
    if (!params.contains(key))
      return false;

    consumed.insert(key);
    return true;
  };

  if (take(QStringLiteral("title"))) {
    g.title     = params.value(QStringLiteral("title")).toString();
    rebuildTree = true;
  }
  if (take(QStringLiteral("widget"))) {
    g.widget    = params.value(QStringLiteral("widget")).toString();
    rebuildTree = true;
  }
  if (take(QStringLiteral("columns")))
    g.columns = params.value(QStringLiteral("columns")).toInt();

  if (take(Keys::SourceId))
    g.sourceId = params.value(Keys::SourceId).toInt();

  if (take(QStringLiteral("painterCode")))
    g.painterCode = params.value(QStringLiteral("painterCode")).toString();

  if (consumed.size() > identityKeyCount)
    rebuildTree = true;

  project.updateGroup(groupId, g, rebuildTree);

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[QStringLiteral("updated")] = true;
  appendUnknownFieldsWarning(result, params, consumed, QStringLiteral("project.group.update"));
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Patch any subset of dataset fields by groupId + datasetId; any applied field forces
 *        the tree rebuild so the epoch-gated dashboard apply and editor reload fire.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetUpdate(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(Keys::DatasetId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: datasetId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const int datasetId  = params.value(Keys::DatasetId).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = groups[groupId].datasets;
  if (datasetId < 0 || static_cast<size_t>(datasetId) >= datasets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found: %1 in group %2")
                                        .arg(QString::number(datasetId), QString::number(groupId)));

  DataModel::Dataset d = datasets[datasetId];
  bool rebuildTree     = false;
  QSet<QString> consumed{
    QStringLiteral("groupId"), Keys::DatasetId, QStringLiteral("expectedProjectEpoch")};
  const auto identityKeyCount = consumed.size();
  const QString err           = applyDatasetUpdateParams(d, params, rebuildTree, consumed);
  if (!err.isEmpty())
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, err);

  if (consumed.size() > identityKeyCount)
    rebuildTree = true;

  const auto preEpoch = captureProjectEpoch();
  project.updateDataset(groupId, datasetId, d, rebuildTree);

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[Keys::DatasetId]           = datasetId;
  result[QStringLiteral("updated")] = true;

  appendUnknownFieldsWarning(result, params, consumed, QStringLiteral("project.dataset.update"));
  appendStaleProjectWarning(result, params, preEpoch);
  attachProjectEpoch(result);

  if (!d.transformCode.isEmpty() && d.transformLanguage != -1) {
    const auto warning = detectLanguageMismatch(d.transformCode, d.transformLanguage);
    if (!warning.isEmpty())
      result[QStringLiteral("warning")] = warning;
  }

  if (!d.transformCode.isEmpty() && !d.virtual_ && d.index <= 0)
    result[QStringLiteral("hint")] =
      QStringLiteral("transformCode set but virtual=false and index<=0; if this "
                     "dataset has no slot in the parser output array, set "
                     "virtual=true (next call: project.dataset.update{virtual:true}).");

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Patch any subset of action fields by id.
 */
API::CommandResponse API::Handlers::ProjectHandler::actionUpdate(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("actionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: actionId"));

  const int actionId   = params.value(QStringLiteral("actionId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& actions  = project.actions();
  if (actionId < 0 || static_cast<size_t>(actionId) >= actions.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Action id not found: %1").arg(actionId));

  DataModel::Action a = actions[actionId];
  bool rebuildTree    = false;
  QSet<QString> consumed{QStringLiteral("actionId")};
  const auto identityKeyCount = consumed.size();

  const auto take = [&](const QString& key) -> bool {
    if (!params.contains(key))
      return false;

    consumed.insert(key);
    return true;
  };

  if (take(QStringLiteral("title"))) {
    a.title     = params.value(QStringLiteral("title")).toString();
    rebuildTree = true;
  }
  if (take(QStringLiteral("icon"))) {
    a.icon      = params.value(QStringLiteral("icon")).toString();
    rebuildTree = true;
  }
  if (take(QStringLiteral("txData")))
    a.txData = params.value(QStringLiteral("txData")).toString();

  if (take(QStringLiteral("eolSequence")))
    a.eolSequence = params.value(QStringLiteral("eolSequence")).toString();

  if (take(QStringLiteral("timerMode")))
    a.timerMode =
      static_cast<DataModel::TimerMode>(params.value(QStringLiteral("timerMode")).toInt());

  if (take(QStringLiteral("timerIntervalMs")))
    a.timerIntervalMs = params.value(QStringLiteral("timerIntervalMs")).toInt();

  if (take(QStringLiteral("repeatCount")))
    a.repeatCount = params.value(QStringLiteral("repeatCount")).toInt();

  if (take(Keys::SourceId))
    a.sourceId = params.value(Keys::SourceId).toInt();

  if (take(QStringLiteral("txEncoding")))
    a.txEncoding = params.value(QStringLiteral("txEncoding")).toInt();

  if (take(QStringLiteral("binaryData")))
    a.binaryData = params.value(QStringLiteral("binaryData")).toBool();

  if (take(QStringLiteral("autoExecuteOnConnect")))
    a.autoExecuteOnConnect = params.value(QStringLiteral("autoExecuteOnConnect")).toBool();

  if (consumed.size() > identityKeyCount)
    rebuildTree = true;

  project.updateAction(actionId, a, rebuildTree);

  QJsonObject result;
  result[QStringLiteral("actionId")] = actionId;
  result[QStringLiteral("updated")]  = true;
  appendUnknownFieldsWarning(result, params, consumed, QStringLiteral("project.action.update"));
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Patch any subset of output-widget fields by groupId + widgetId.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetUpdate(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("widgetId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const int widgetId   = params.value(QStringLiteral("widgetId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  DataModel::Group g = groups[groupId];
  if (widgetId < 0 || static_cast<size_t>(widgetId) >= g.outputWidgets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Widget id not found: %1 in group %2")
                                        .arg(QString::number(widgetId), QString::number(groupId)));

  DataModel::OutputWidget& w = g.outputWidgets[widgetId];
  bool rebuildTree           = false;
  QSet<QString> consumed{QStringLiteral("groupId"), QStringLiteral("widgetId")};
  const auto identityKeyCount = consumed.size();

  const auto take = [&](const QString& key) -> bool {
    if (!params.contains(key))
      return false;

    consumed.insert(key);
    return true;
  };

  if (take(QStringLiteral("title"))) {
    w.title     = params.value(QStringLiteral("title")).toString();
    rebuildTree = true;
  }
  if (take(QStringLiteral("icon"))) {
    w.icon      = params.value(QStringLiteral("icon")).toString();
    rebuildTree = true;
  }
  if (take(QStringLiteral("transmitFunction")))
    w.transmitFunction = params.value(QStringLiteral("transmitFunction")).toString();

  if (take(Keys::SourceId))
    w.sourceId = params.value(Keys::SourceId).toInt();

  if (take(QStringLiteral("txEncoding")))
    w.txEncoding = params.value(QStringLiteral("txEncoding")).toInt();

  if (take(QStringLiteral("monoIcon"))) {
    w.monoIcon  = params.value(QStringLiteral("monoIcon")).toBool();
    rebuildTree = true;
  }

  if (take(QStringLiteral("minValue")))
    w.minValue = SerialStudio::toDouble(params.value(QStringLiteral("minValue")));

  if (take(QStringLiteral("maxValue")))
    w.maxValue = SerialStudio::toDouble(params.value(QStringLiteral("maxValue")));

  if (take(QStringLiteral("stepSize")))
    w.stepSize = SerialStudio::toDouble(params.value(QStringLiteral("stepSize")));

  if (take(QStringLiteral("initialValue")))
    w.initialValue = SerialStudio::toDouble(params.value(QStringLiteral("initialValue")));

  if (consumed.size() > identityKeyCount)
    rebuildTree = true;

  project.updateGroup(groupId, g, rebuildTree);

  QJsonObject result;
  result[QStringLiteral("groupId")]  = groupId;
  result[QStringLiteral("widgetId")] = widgetId;
  result[QStringLiteral("updated")]  = true;
  appendUnknownFieldsWarning(
    result, params, consumed, QStringLiteral("project.outputWidget.update"));
  return CommandResponse::makeSuccess(id, result);
}
