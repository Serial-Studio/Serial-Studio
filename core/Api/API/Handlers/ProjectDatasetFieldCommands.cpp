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

#include "API/Handlers/ProjectDatasetFieldCommands.h"

#include <algorithm>
#include <cmath>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

#include "API/Handlers/ProjectApiSupport.h"
#include "API/SchemaBuilder.h"
#include "Core/DataModel/Frame.h"
#include "Core/DataModel/FrameSupport.h"
#include "Core/License.h"
#include "Core/SerialStudio.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"

using namespace API::Handlers::ProjectApiSupport;

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the command class to the registry it publishes into.
 */
API::Handlers::ProjectDatasetFieldCommands::ProjectDatasetFieldCommands(CommandRegistry& registry)
  : m_registry(registry)
{}

/**
 * @brief Register dataset field setters, alarm bands, FFT markers and value labels.
 */
void API::Handlers::ProjectDatasetFieldCommands::registerCommands()
{
  registerFieldCommands();
  registerAlarmCommands();
  registerMarkerCommands();
  registerLabelCommands();
}

/**
 * @brief Register the dataset value-label getter / setter commands (spec 0108).
 */
void API::Handlers::ProjectDatasetFieldCommands::registerLabelCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.dataset.getLabels"),
    QStringLiteral("Returns the dataset's value labels: labels is an array of {value (integer), "
                   "text} sorted by value, labelsDefault the catch-all text shown for a value no "
                   "row lists (empty = show the number). Labels change only how the value reads "
                   "as text; plots, alarms, exports and the API's numeric fields keep the "
                   "number."),
    makeSchema({
      {  QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                  },
      {QString(Keys::DatasetId),
       QStringLiteral("integer"),
       QStringLiteral("Dataset id within the group")}
  }),
    &datasetGetLabels);

  registry.registerCommand(
    QStringLiteral("project.dataset.setLabels"),
    QStringLiteral("Atomically replaces the dataset's value labels. Each entry is {value, text} "
                   "where value must be an integer (a non-integer value is refused with an "
                   "error); rows with empty text or a repeated value are dropped and counted in "
                   "result.droppedInvalid (the first row of a value wins). The dataset's value "
                   "is rounded to the nearest integer before the lookup, and wherever it is "
                   "shown as text the matching row's text replaces it; plots, alarms, exports "
                   "and numeric API fields keep the number. labelsDefault (optional) sets the "
                   "catch-all text for unlisted values. Labels are not Pro content. Pass an "
                   "empty array to clear all labels."),
    makeSchema(
      {
        {                   QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                               },
        {                          QString(Keys::DatasetId),
         QStringLiteral("integer"),
         QStringLiteral("Dataset id within the group")              },
        arrayProp(
          QString(Keys::Labels),
          QStringLiteral("Full replacement label list (empty = no labels)."),
          QJsonObject{
         {QStringLiteral("type"), QStringLiteral("object")},
         {QStringLiteral("properties"),
         QJsonObject{
         {QString(Keys::Value),
         QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")},
         {QStringLiteral("description"),
         QStringLiteral("Integer the rounded value is matched against")}}},
         {QString(Keys::LabelText),
         QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
         {QStringLiteral("description"),
         QStringLiteral("Text shown in place of the number")}}}}},
         {QStringLiteral("required"),
         QJsonArray{QString(Keys::Value), QString(Keys::LabelText)}}}
        )
  },
      {{QString(Keys::LabelsDefault),
        QStringLiteral("string"),
        QStringLiteral("Optional catch-all text for unlisted values (empty = show the "
                       "number)")}}),
    &datasetSetLabels);
}

/**
 * @brief Register dataset field setters (virtual flag, transform code).
 */
void API::Handlers::ProjectDatasetFieldCommands::registerFieldCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.dataset.setVirtual"),
    QStringLiteral("Toggle the virtual flag on a dataset (params: groupId, datasetId, virtual)"),
    makeSchema({
      {  QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                           },
      {QString(Keys::DatasetId),
       QStringLiteral("integer"),
       QStringLiteral("Dataset id within the group")                    },
      {  QString(Keys::Virtual),
       QStringLiteral("boolean"),
       QStringLiteral("Mark dataset as virtual (computed by transform)")}
  }),
    &datasetSetVirtual);

  registry.registerCommand(
    QStringLiteral("project.dataset.setTransformCode"),
    QStringLiteral(
      "Set dataset transformCode. Empty clears. Pass `language` whenever "
      "you author code so the dataset's transformLanguage matches the "
      "syntax you wrote -- mismatches compile-fail silently. Lua (1) is "
      "the recommended default; it's measurably faster than JavaScript "
      "on hot transforms. If this dataset is compute-only (no slot in "
      "the parser output array), also set virtual=true via "
      "project.dataset.setVirtual or project.dataset.update. Validate with "
      "project.dataset.transform.dryRun before setting. **Call "
      "meta.fetchScriptingDocs{kind: 'transform_lua' | 'transform_js'} "
      "first** for the transform(value) signature, table API "
      "(tableGet/tableSet/datasetGetRaw/datasetGetFinal), and "
      "execution-order rules -- a transform may read RAW values from any "
      "dataset but only FINAL values of datasets earlier in "
      "project.dataset.getExecutionOrder. Remote clients must clear the one-time script-install consent prompt (the user's answer is remembered for the rest of the session); in-process callers are not prompted."),
    makeSchema(
      {
        {  QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                           },
        {QString(Keys::DatasetId),
         QStringLiteral("integer"),
         QStringLiteral("Dataset id within the group")                        },
        {  QStringLiteral("code"),
         QStringLiteral("string"),
         QStringLiteral("Transform source (Lua or JS, must match `language`)")}
  },
      {{QStringLiteral("language"),
        QStringLiteral("integer"),
        QStringLiteral("Optional: 0=JavaScript, 1=Lua (recommended). If omitted, "
                       "the dataset inherits the source's frameParserLanguage.")}}),
    &datasetSetTransformCode);
}

/**
 * @brief Register dataset alarm-band getter / setter commands.
 */
void API::Handlers::ProjectDatasetFieldCommands::registerAlarmCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.dataset.getAlarmBands"),
    QStringLiteral("Returns the dataset's coloured alarm bands as a JSON array. Each entry: "
                   "{min, max, severity (0=Info, 1=OK, 2=Warning, 3=Critical), color "
                   "(\"#rrggbb\" override or empty for severity default), label, blink}. Also "
                   "returns rangeMin/rangeMax (the dataset's wgtMin/wgtMax) so callers can "
                   "validate band ranges before writing back. An empty array means no alarms "
                   "configured. Bands are drawn, notified and alarmed only when the dataset has "
                   "a bar, gauge or meter widget or is an LED-panel member (and is not hidden "
                   "from the dashboard), or belongs to a bar-panel group; on any other dataset "
                   "they are stored and ignored, and this call still returns them. A dataset "
                   "with suppressAlarms set still draws its bands but never notifies or alarms; "
                   "one with alarmsWhileVisible set alarms only while a widget showing it is on "
                   "screen (suppressAlarms wins)."),
    makeSchema({
      {  QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                  },
      {QString(Keys::DatasetId),
       QStringLiteral("integer"),
       QStringLiteral("Dataset id within the group")}
  }),
    &datasetGetAlarmBands);

  registry.registerCommand(
    QStringLiteral("project.dataset.setAlarmBands"),
    QStringLiteral("Atomically replaces the dataset's alarmBands array. Each band entry must "
                   "provide numeric min and max (max>min; bands with max<=min are silently "
                   "dropped and counted in result.droppedInvalid), and an integer severity "
                   "(0=Info, 1=OK, 2=Warning, 3=Critical). color is optional (\"#rrggbb\" "
                   "override; empty/missing = severity's theme colour). label is optional "
                   "(surfaces in band-edge notifications). blink is optional (boolean; LED "
                   "panels flash the LED while the band is active). Bands may have gaps and "
                   "may overlap; rendering paints them in array order behind the value "
                   "indicator. Severity >= Warning triggers a notification and an alarm point "
                   "when the value enters the band, whether or not the widget is on screen "
                   "(3-second per-dataset cooldown suppresses oscillation spam). That happens "
                   "only when the dataset has a bar, gauge or meter widget or is an LED-panel "
                   "member (and is not hidden from the dashboard), or belongs to a bar-panel "
                   "group, and suppressAlarms is false; bands on any other dataset are stored "
                   "but never notify or alarm, and a suppressed dataset still draws them. A "
                   "dataset with alarmsWhileVisible set alarms only while a widget showing it "
                   "is on screen, and a reveal into a band counts as entering it.\n"
                   "Pass an empty array to clear all alarms."),
    makeSchema({
      {                     QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                             },
      {                          QString(Keys::DatasetId),
       QStringLiteral("integer"),
       QStringLiteral("Dataset id within the group")                             },
      arrayProp(
        QString(Keys::AlarmBands),
        QStringLiteral("Full replacement band list (empty = no alarms)."),
        QJsonObject{
       {QStringLiteral("type"), QStringLiteral("object")},
       {QStringLiteral("properties"),
       QJsonObject{{QStringLiteral("min"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("number")},
       {QStringLiteral("description"),
       QStringLiteral("Lower bound (inclusive)")}}},
       {QStringLiteral("max"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("number")},
       {QStringLiteral("description"),
       QStringLiteral("Upper bound (exclusive at top of range)")}}},
       {QStringLiteral("severity"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")},
       {QStringLiteral("description"),
       QStringLiteral("0=Info, 1=OK, 2=Warning, 3=Critical")}}},
       {QStringLiteral("color"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
       {QStringLiteral("description"),
       QStringLiteral("Optional \"#rrggbb\" override; empty = "
       "use severity's theme colour")}}},
       {QStringLiteral("label"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
       {QStringLiteral("description"),
       QStringLiteral("Optional band name (shown in "
       "notifications)")}}},
       {QStringLiteral("blink"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("boolean")},
       {QStringLiteral("description"),
       QStringLiteral("Optional; LED panels flash the LED "
       "while this band is active")}}}}},
       {QStringLiteral("required"),
       QJsonArray{
       QStringLiteral("min"), QStringLiteral("max"), QStringLiteral("severity")}}}
      )
  }),
    &datasetSetAlarmBands);
}

/**
 * @brief Register dataset FFT frequency-marker getter / setter commands.
 */
void API::Handlers::ProjectDatasetFieldCommands::registerMarkerCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.dataset.getFFTMarkers"),
    QStringLiteral("Returns the dataset's FFT frequency markers as a JSON array. Each entry: "
                   "{freq (Hz), endFreq (optional; present only for band markers), label, "
                   "color (\"#rrggbb\" override or empty for the theme accent), warningDb, "
                   "alarmDb (optional display-dB thresholds; omitted when unset)}. Also "
                   "returns nyquist (fftSamplingRate / 2) so callers can validate frequencies "
                   "before writing back. An empty array means no markers configured."),
    makeSchema({
      {  QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                  },
      {QString(Keys::DatasetId),
       QStringLiteral("integer"),
       QStringLiteral("Dataset id within the group")}
  }),
    &datasetGetFFTMarkers);

  registry.registerCommand(
    QStringLiteral("project.dataset.setFFTMarkers"),
    QStringLiteral("Atomically replaces the dataset's fftMarkers array (FFT plot + waterfall "
                   "frequency markers). Each entry must provide a finite freq > 0 in Hz; "
                   "invalid entries are silently dropped and counted in result.droppedInvalid. "
                   "endFreq is optional (> freq turns the marker into a band; otherwise it is "
                   "a point marker). label and color (\"#rrggbb\") are optional. warningDb and "
                   "alarmDb are optional display-dB thresholds evaluated against the rendered "
                   "spectrum; when both are present and reversed they are swapped. Pass an "
                   "empty array to clear all markers."),
    makeSchema({
      {                     QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                                     },
      {                          QString(Keys::DatasetId),
       QStringLiteral("integer"),
       QStringLiteral("Dataset id within the group")                   },
      arrayProp(
        QString(Keys::FFTMarkers),
        QStringLiteral("Full replacement marker list (empty = no markers)."),
        QJsonObject{
       {QStringLiteral("type"), QStringLiteral("object")},
       {QStringLiteral("properties"),
       QJsonObject{{QStringLiteral("freq"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("number")},
       {QStringLiteral("description"),
       QStringLiteral("Marker frequency in Hz (> 0)")}}},
       {QStringLiteral("endFreq"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("number")},
       {QStringLiteral("description"),
       QStringLiteral("Optional band end in Hz (> freq)")}}},
       {QStringLiteral("label"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
       {QStringLiteral("description"),
       QStringLiteral("Optional marker name shown on the chip")}}},
       {QStringLiteral("color"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
       {QStringLiteral("description"),
       QStringLiteral("Optional \"#rrggbb\" override; empty = "
       "theme accent")}}},
       {QStringLiteral("warningDb"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("number")},
       {QStringLiteral("description"),
       QStringLiteral("Optional warning level in display dB")}}},
       {QStringLiteral("alarmDb"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("number")},
       {QStringLiteral("description"),
       QStringLiteral("Optional alarm level in display dB")}}}}},
       {QStringLiteral("required"), QJsonArray{QStringLiteral("freq")}}}
      )
  }),
    &datasetSetFFTMarkers);
}

//--------------------------------------------------------------------------------------------------
// Dataset field setters (v3.3)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Toggle the @c virtual_ flag on the dataset identified by (groupId, datasetId).
 */
API::CommandResponse API::Handlers::ProjectDatasetFieldCommands::datasetSetVirtual(
  const QString& id, const QJsonObject& params)
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

  auto& pm           = DataModel::pipelineModules().projectModel;
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
API::CommandResponse API::Handlers::ProjectDatasetFieldCommands::datasetSetTransformCode(
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

  auto& pm           = DataModel::pipelineModules().projectModel;
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
    if (lang != SerialStudio::JavaScript && lang != SerialStudio::Lua
        && lang != SerialStudio::Expression)
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Invalid language: must be 0 (JavaScript), 1 (Lua) or 3 (Expression)"));

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

  if (SerialStudio::authorsTransform(*dit, updated) && !Core::License::activated())
    return CommandResponse::makeError(id, ErrorCode::OperationFailed, transformsRequirePro());

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

  if (!code.isEmpty() && updated.transformLanguage != -1
      && updated.transformLanguage != SerialStudio::Expression) {
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
API::CommandResponse API::Handlers::ProjectDatasetFieldCommands::datasetGetAlarmBands(
  const QString& id, const QJsonObject& params)
{
  const QStringList required{QString(Keys::GroupId), QString(Keys::DatasetId)};
  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();

  auto& pm           = DataModel::pipelineModules().projectModel;
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
API::CommandResponse API::Handlers::ProjectDatasetFieldCommands::datasetSetAlarmBands(
  const QString& id, const QJsonObject& params)
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

  auto& pm           = DataModel::pipelineModules().projectModel;
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
API::CommandResponse API::Handlers::ProjectDatasetFieldCommands::datasetGetFFTMarkers(
  const QString& id, const QJsonObject& params)
{
  const QStringList required{QString(Keys::GroupId), QString(Keys::DatasetId)};
  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();

  auto& pm           = DataModel::pipelineModules().projectModel;
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
API::CommandResponse API::Handlers::ProjectDatasetFieldCommands::datasetSetFFTMarkers(
  const QString& id, const QJsonObject& params)
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

  auto& pm           = DataModel::pipelineModules().projectModel;
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

//--------------------------------------------------------------------------------------------------
// Value labels (spec 0108)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Finds the project dataset @p params names; null with @p error set when none matches.
 */
[[nodiscard]] static const DataModel::Dataset* labelTarget(const QJsonObject& params,
                                                           QString& error)
{
  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  for (const auto& group : DataModel::pipelineModules().projectModel.groups()) {
    if (group.groupId != groupId)
      continue;

    for (const auto& dataset : group.datasets)
      if (dataset.datasetId == datasetId)
        return &dataset;

    error = QStringLiteral("Dataset id not found in group: %1/%2")
              .arg(QString::number(groupId), QString::number(datasetId));
    return nullptr;
  }

  error = QStringLiteral("Group id not found: %1").arg(groupId);
  return nullptr;
}

/**
 * @brief Returns the index of the first label entry whose value is not an integer a double holds
 *        exactly, or -1 when every entry passes.
 */
[[nodiscard]] static int firstNonIntegerLabel(const QJsonArray& labels)
{
  constexpr double kExactIntegerLimit = 9007199254740992.0;
  for (qsizetype i = 0; i < labels.size(); ++i) {
    bool ok            = false;
    const double value = SerialStudio::toDouble(labels.at(i).toObject().value(Keys::Value), &ok);
    if (!ok || std::abs(value) > kExactIntegerLimit || value != std::trunc(value))
      return static_cast<int>(i);
  }

  return -1;
}

/**
 * @brief Returns the dataset's value labels as {value, text} rows plus the catch-all text.
 */
API::CommandResponse API::Handlers::ProjectDatasetFieldCommands::datasetGetLabels(
  const QString& id, const QJsonObject& params)
{
  const QStringList required{QString(Keys::GroupId), QString(Keys::DatasetId)};
  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  QString error;
  const auto* dataset = labelTarget(params, error);
  if (!dataset)
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, error);

  QJsonArray labels;
  for (const auto& label : dataset->labels) {
    QJsonObject row;
    row.insert(Keys::Value, label.value);
    row.insert(Keys::LabelText, label.text);
    labels.append(row);
  }

  QJsonObject result;
  result[Keys::GroupId]           = dataset->groupId;
  result[Keys::DatasetId]         = dataset->datasetId;
  result[Keys::Labels]            = labels;
  result[Keys::LabelsDefault]     = dataset->labelsDefault;
  result[QStringLiteral("count")] = labels.size();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Atomic write of the dataset's value labels; a non-integer value refuses the whole call,
 *        and the rows then pass through the project reader's own rule.
 */
API::CommandResponse API::Handlers::ProjectDatasetFieldCommands::datasetSetLabels(
  const QString& id, const QJsonObject& params)
{
  const QStringList required{
    QString(Keys::GroupId), QString(Keys::DatasetId), QString(Keys::Labels)};
  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  QString error;
  const auto* dataset = labelTarget(params, error);
  if (!dataset)
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, error);

  const auto rows = params.value(Keys::Labels).toArray();
  if (const int bad = firstNonIntegerLabel(rows); bad >= 0)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid labels[%1].value: label values must be integers").arg(bad));

  QJsonObject payload;
  payload.insert(Keys::Labels, rows);

  DataModel::Dataset updated = *dataset;
  DataModel::readDatasetLabels(updated, payload);
  if (params.contains(Keys::LabelsDefault))
    updated.labelsDefault = params.value(Keys::LabelsDefault).toString().simplified();

  auto& pm = DataModel::pipelineModules().projectModel;
  pm.updateDataset(updated.groupId, updated.datasetId, updated, true);

  const int dropped = static_cast<int>(rows.size()) - static_cast<int>(updated.labels.size());

  QJsonObject result;
  result[Keys::GroupId]             = updated.groupId;
  result[Keys::DatasetId]           = updated.datasetId;
  result[QStringLiteral("count")]   = static_cast<int>(updated.labels.size());
  result[QStringLiteral("updated")] = true;
  if (dropped > 0)
    result[QStringLiteral("droppedInvalid")] = dropped;

  return CommandResponse::makeSuccess(id, result);
}
