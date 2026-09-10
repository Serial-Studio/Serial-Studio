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

#include "API/Handlers/ProjectDatasetCommands.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

#include "API/CommandRegistry.h"
#include "API/Handlers/ProjectApiSupport.h"
#include "API/SchemaBuilder.h"
#include "Core/DataModel/Frame.h"
#include "Core/EnumLabels.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"

using namespace API::Handlers::ProjectApiSupport;

namespace API::Handlers {

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

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the command class to the registry it publishes into.
 */
API::Handlers::ProjectDatasetCommands::ProjectDatasetCommands(CommandRegistry& registry)
  : m_registry(registry)
{}

/**
 * @brief Register dataset CRUD and option-bitfield commands.
 */
void API::Handlers::ProjectDatasetCommands::registerCommands()
{
  registerCrudCommands();
}

/**
 * @brief Register dataset add/delete/duplicate and option-bitfield commands.
 */
void API::Handlers::ProjectDatasetCommands::registerCrudCommands()
{
  registerCreateCommands();
  registerLifecycleCommands();
  registerOptionCommands();
}

/**
 * @brief Registers project.dataset.add and project.dataset.addMany.
 */
void API::Handlers::ProjectDatasetCommands::registerCreateCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.dataset.add"),
    QStringLiteral("Add a dataset to a group. A dataset is one channel of incoming "
                   "data: it gets one column in CSV/MDF4 export, one entry in the "
                   "frame parser's output array (its `index` field), and zero or "
                   "more dashboard widgets driven by the `options` bitflags.\n"
                   "Pass `options` as the bitwise OR of the visualizations you want:\n"
                   "  1 = Plot (time-series line)\n"
                   "  2 = FFT (frequency-domain plot)\n"
                   "  4 = Bar\n"
                   "  8 = Gauge (radial dial; needs widgetMin/widgetMax)\n"
                   "  16 = Compass (heading 0-360)\n"
                   "  32 = LED (binary indicator with ledHigh threshold)\n"
                   "  64 = Waterfall (Pro; spectrogram, FFT-driven)\n"
                   "  0 = no widget (raw column for export only)\n"
                   "Combine: 1|8 = 9 = plot AND gauge. After creation, set title, "
                   "units, ranges, and transformCode via project.dataset.update."),
    makeSchema({
      {QStringLiteral("groupId"),
       QStringLiteral("integer"),
       QStringLiteral("Group to attach the dataset to")                                 },
      {QStringLiteral("options"),
       QStringLiteral("integer"),
       QStringLiteral("Visualization bit flags. See description for decision guidance.")}
  }),
    &datasetAdd);

  registry.registerCommand(
    QStringLiteral("project.dataset.addMany"),
    QStringLiteral("Bulk-create N datasets in one call -- the right tool whenever you "
                   "would otherwise loop project.dataset.add. Avoids per-call overhead "
                   "and the autosave-debounce churn that comes with rapid mutation "
                   "bursts. After creation, individual datasets can still be patched "
                   "with project.dataset.update or another project.batch round-trip.\n"
                   "  count          -- how many datasets to create (1..1024).\n"
                   "  options        -- visualization bitfield (same as project.dataset.add).\n"
                   "  titlePattern   -- optional, e.g. 'LED {n}'. {n} is replaced with "
                   "the running counter (startNumber + i), {i} with the zero-based index. "
                   "Omit to keep the auto-generated title from project.dataset.add.\n"
                   "  startNumber    -- optional, default 1; first {n} value.\n"
                   "  startIndex     -- optional, default -1 (auto-assign next free "
                   "parser slot). Pass 0 to leave index unset, or 1+ to assign "
                   "consecutive parser slots starting from there.\n"
                   "Returns {count, created: [{datasetId, title, index, uniqueId}...]}."),
    makeSchema({
      {QStringLiteral("groupId"),
       QStringLiteral("integer"),
       QStringLiteral("Group to attach the datasets to")                                    },
      {  QStringLiteral("count"),
       QStringLiteral("integer"),
       QStringLiteral("How many datasets to create (1..1024)")                              },
      {QStringLiteral("options"),
       QStringLiteral("integer"),
       QStringLiteral("Visualization bit flags. See project.dataset.add for the bit table.")}
  }),
    &datasetAddMany);
}

/**
 * @brief Registers project.dataset.delete/duplicate (single-target lifecycle ops).
 */
void API::Handlers::ProjectDatasetCommands::registerLifecycleCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.dataset.delete"),
    QStringLiteral("Delete a dataset by id. Pass dryRun:true to preview the renumbering "
                   "without committing -- the response carries the same {deleted, "
                   "renumbered, warnings} shape as a real call, plus a top-level "
                   "dryRun:true flag. Always preview before committing when destructive "
                   "intent is even slightly uncertain."),
    makeSchema(
      {
        {QStringLiteral("groupId"),QStringLiteral("integer"),QStringLiteral("Owning group id")                    },
        {          Keys::DatasetId,
         QStringLiteral("integer"),
         QStringLiteral("Dataset id within the group")}
  },
      {{QStringLiteral("dryRun"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, return the affected entities without committing. Auto-runs "
                       "without an approval card.")}}),
    &datasetDelete);

  registry.registerCommand(
    QStringLiteral("project.dataset.duplicate"),
    QStringLiteral("Duplicate a dataset by id (params: groupId, datasetId)"),
    makeSchema({
      {QStringLiteral("groupId"), QStringLiteral("integer"),             QStringLiteral("Owning group id")},
      {          Keys::DatasetId, QStringLiteral("integer"), QStringLiteral("Dataset id within the group")}
  }),
    &datasetDuplicate);
}

/**
 * @brief Registers project.dataset.setOption/setOptions option-bitfield commands.
 */
void API::Handlers::ProjectDatasetCommands::registerOptionCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.dataset.setOption"),
    QStringLiteral("DEPRECATED -- prefer project.dataset.setOptions, which takes the "
                   "full bitfield in one call and removes the singular/plural ambiguity. "
                   "Kept for backward compatibility with existing scripts. Toggles one "
                   "DatasetOption flag on a dataset and updates the group's "
                   "compatibleWidgetTypes immediately. option values (DatasetOption "
                   "bitflag): 1=Plot, 2=FFT, 4=Bar, 8=Gauge, 16=Compass, 32=LED, "
                   "64=Waterfall (Pro). Bar/Gauge/Compass are mutually exclusive. "
                   "NOTE: this is a DatasetOption bitflag, NOT a DashboardWidget enum -- "
                   "the numbers do not line up with project.workspace.addWidget's "
                   "widgetType."),
    makeSchema({
      {QStringLiteral("groupId"),QStringLiteral("integer"),QStringLiteral("Owning group id")                                                            },
      {          Keys::DatasetId, QStringLiteral("integer"), QStringLiteral("Dataset id within the group")},
      { QStringLiteral("option"),
       QStringLiteral("string|integer"),
       QStringLiteral("PREFERRED: a slug -- 'plot', 'fft', 'bar', 'gauge', 'compass', "
       "'led', 'waterfall'. Integer DatasetOption bitflag still accepted "
       "(1, 2, 4, 8, 16, 32, 64).")                                                                       },
      {QStringLiteral("enabled"),
       QStringLiteral("boolean"),
       QStringLiteral("Whether to enable or disable the option")                                          }
  }),
    &datasetSetOption);

  registry.registerCommand(
    QStringLiteral("project.dataset.setOptions"),
    QStringLiteral("Apply several DatasetOption flags at once (plural form of "
                   "project.dataset.setOption). Pass `options` as the bitwise OR of the "
                   "flags you want enabled; any flag NOT set in the value is disabled. "
                   "Bits: 1=Plot, 2=FFT, 4=Bar, 8=Gauge, 16=Compass, 32=LED, "
                   "64=Waterfall (Pro). Bar/Gauge/Compass are mutually exclusive -- if "
                   "more than one is set, the highest bit wins. Updates the group's "
                   "compatibleWidgetTypes immediately. NOTE: these are DatasetOption "
                   "bitflags, NOT DashboardWidget enum values -- the numbers do not "
                   "line up with project.workspace.addWidget's widgetType."),
    makeSchema({
      {QStringLiteral("groupId"),QStringLiteral("integer"),QStringLiteral("Owning group id")                                                            },
      {          Keys::DatasetId, QStringLiteral("integer"), QStringLiteral("Dataset id within the group")},
      {QStringLiteral("options"),
       QStringLiteral("array|integer"),
       QStringLiteral("PREFERRED: an array of slugs (e.g. ['plot','fft','waterfall']). "
       "Integer bitflag still accepted (Plot=1, FFT=2, Bar=4, Gauge=8, "
       "Compass=16, LED=32, Waterfall=64).")                                                              }
  }),
    &datasetSetOptions);
}

namespace API::Handlers {

/**
 * @brief Returns the widget string matching a one-of DatasetOption bit.
 */
[[nodiscard]] static QString widgetForDatasetOptions(int options)
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
static void applyDatasetVisualizationFlags(DataModel::Dataset& d, int options)
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

}  // namespace API::Handlers

/**
 * @brief Add a dataset to a specific group by id
 */
API::CommandResponse API::Handlers::ProjectDatasetCommands::datasetAdd(const QString& id,
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

  auto& project      = DataModel::pipelineModules().projectModel;
  const auto& groups = project.groups();
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

/**
 * @brief Bulk-creates N datasets in one call with optional title/index patterns.
 */
API::CommandResponse API::Handlers::ProjectDatasetCommands::datasetAddMany(
  const QString& id, const QJsonObject& params)
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

  auto& project      = DataModel::pipelineModules().projectModel;
  const auto& groups = project.groups();
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

/**
 * @brief Delete a dataset by id, returning the deleted entity + any renumbered peers.
 */
API::CommandResponse API::Handlers::ProjectDatasetCommands::datasetDelete(const QString& id,
                                                                          const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(Keys::DatasetId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: datasetId"));

  const int groupId   = params.value(QStringLiteral("groupId")).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  auto& project       = DataModel::pipelineModules().projectModel;
  const auto& groups  = project.groups();
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
    auto* checkpoints = API::CommandRegistry::instance().checkpointStore();
    SS_ASSERT_LOG(checkpoints != nullptr);
    if (checkpoints != nullptr)
      backupPath = checkpoints->snapshot(QStringLiteral("pre-datasetDelete"));

    preEpoch = captureProjectEpoch();
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
API::CommandResponse API::Handlers::ProjectDatasetCommands::datasetDuplicate(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(Keys::DatasetId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: datasetId"));

  const int groupId   = params.value(QStringLiteral("groupId")).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  auto& project       = DataModel::pipelineModules().projectModel;
  const auto& groups  = project.groups();
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
API::CommandResponse API::Handlers::ProjectDatasetCommands::datasetSetOption(
  const QString& id, const QJsonObject& params)
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

  auto& project      = DataModel::pipelineModules().projectModel;
  const auto& groups = project.groups();
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
API::CommandResponse API::Handlers::ProjectDatasetCommands::datasetSetOptions(
  const QString& id, const QJsonObject& params)
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

  auto& project      = DataModel::pipelineModules().projectModel;
  const auto& groups = project.groups();
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
