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
#include "API/Handlers/ProjectApiSupport.h"

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
