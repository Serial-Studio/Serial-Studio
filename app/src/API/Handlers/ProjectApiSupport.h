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

#pragma once

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QStringList>

#include "API/EnumLabels.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "SerialStudio.h"

namespace API::Handlers {

/**
 * @brief Appends a dataset's compatible DashboardWidget enums to compat (deduped).
 */
inline void appendDatasetWidgetTypes(const DataModel::Dataset& ds, QJsonArray& compat)
{
  for (const auto w : SerialStudio::getDashboardWidgets(ds)) {
    const auto v = static_cast<int>(w);
    if (!compat.contains(v))
      compat.append(v);
  }
}

/**
 * @brief Returns the per-dataset bitflag summary (mirrors enabled visualization options).
 */
inline int datasetOptionsBitflag(const DataModel::Dataset& d);

/**
 * @brief Builds the canonical dataset response object used by list / get* / snapshot.
 */
inline QJsonObject buildDatasetObject(const DataModel::Dataset& dataset,
                                      const DataModel::Group& group)
{
  QJsonObject dataset_obj = DataModel::serialize(dataset);

  dataset_obj[QStringLiteral("groupId")]    = group.groupId;
  dataset_obj[QStringLiteral("groupTitle")] = group.title;
  dataset_obj[Keys::SourceId]               = dataset.sourceId;
  dataset_obj[Keys::UniqueId]               = dataset.uniqueId;

  QStringList enabled;
  if (dataset.plt)
    enabled.append(QStringLiteral("plot"));

  if (dataset.fft)
    enabled.append(QStringLiteral("FFT"));

  if (dataset.led)
    enabled.append(QStringLiteral("LED"));

  if (dataset.log)
    enabled.append(QStringLiteral("log"));

  if (dataset.waterfall)
    enabled.append(QStringLiteral("waterfall"));

  if (!dataset.alarmBands.empty())
    enabled.append(QStringLiteral("alarm"));

  if (!dataset.widget.isEmpty())
    enabled.append(dataset.widget);

  dataset_obj[QStringLiteral("enabledFeatures")] =
    enabled.isEmpty() ? QStringLiteral("plain numeric") : enabled.join(QStringLiteral(", "));

  const int optionsBits                         = datasetOptionsBitflag(dataset);
  dataset_obj[QStringLiteral("enabledOptions")] = optionsBits;

  QJsonArray optionSlugs;
  for (const auto& slug : API::EnumLabels::datasetOptionsBitsToSlugs(optionsBits))
    optionSlugs.append(slug);

  dataset_obj[QStringLiteral("enabledOptionsSlugs")] = optionSlugs;

  QJsonArray ds_widget_types;
  appendDatasetWidgetTypes(dataset, ds_widget_types);
  dataset_obj[QStringLiteral("enabledWidgetTypes")] = ds_widget_types;

  QJsonArray ds_widget_type_slugs;
  for (const auto w : SerialStudio::getDashboardWidgets(dataset)) {
    const auto slug = API::EnumLabels::dashboardWidgetSlug(static_cast<int>(w));
    if (!ds_widget_type_slugs.contains(slug))
      ds_widget_type_slugs.append(slug);
  }
  dataset_obj[QStringLiteral("enabledWidgetTypesSlugs")] = ds_widget_type_slugs;

  dataset_obj[QStringLiteral("hasTransform")] = !dataset.transformCode.isEmpty();
  dataset_obj[QStringLiteral("isVirtual")]    = dataset.virtual_;

  QJsonObject explanations;
  explanations[QStringLiteral("enabledOptions")] =
    QStringLiteral("%1 (bitflag %2)")
      .arg(API::EnumLabels::datasetOptionsLabel(optionsBits))
      .arg(optionsBits);

  if (!dataset.transformCode.isEmpty())
    explanations[QStringLiteral("transformLanguage")] =
      QStringLiteral("Transform script in %1 (%2 bytes)")
        .arg(API::EnumLabels::scriptLanguageLabel(dataset.transformLanguage))
        .arg(dataset.transformCode.size());

  if (dataset.virtual_)
    explanations[QStringLiteral("virtual")] =
      QStringLiteral("Computed dataset: value comes from transform(), not a parser-output slot");

  if (dataset.index == 0 && !dataset.virtual_)
    explanations[QStringLiteral("index")] =
      QStringLiteral("index=0 means the dataset is unassigned -- it will read nothing unless "
                     "you set index>=1 (parser-output slot) or virtual=true");

  dataset_obj[QStringLiteral("_explanations")] = explanations;
  return dataset_obj;
}

/**
 * @brief A resolved dataset and its owning group; both null when resolution failed. The pointers
 *        alias ProjectModel::groups() and are valid only until the next project mutation.
 */
struct DatasetMatch {
  const DataModel::Group* group     = nullptr;
  const DataModel::Dataset* dataset = nullptr;
};

/**
 * @brief Returns the first dataset whose alias equals @p alias (assumed non-empty), or a null
 *        match. Alias comparison is exact (whitespace already trimmed by the caller).
 */
[[nodiscard]] inline DatasetMatch findDatasetByAlias(const QString& alias)
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  for (const auto& group : projectModel.groups()) {
    for (const auto& dataset : group.datasets)
      if (dataset.alias == alias)
        return {&group, &dataset};
  }

  return {};
}

/**
 * @brief Returns the dataset with the given stable uniqueId, or a null match.
 */
[[nodiscard]] inline DatasetMatch findDatasetByUniqueId(int uniqueId)
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  for (const auto& group : projectModel.groups()) {
    for (const auto& dataset : group.datasets)
      if (dataset.uniqueId == uniqueId)
        return {&group, &dataset};
  }

  return {};
}

/**
 * @brief Resolves a dataset selector -- a numeric uniqueId or a string alias -- against the live
 *        project. A string is always an alias and a number always a uniqueId. On failure @p error
 *        names the unresolved selector; on success both match pointers are set.
 */
[[nodiscard]] inline DatasetMatch resolveDatasetSelector(const QJsonValue& selector, QString& error)
{
  if (selector.isString()) {
    const QString alias = selector.toString().trimmed();
    if (alias.isEmpty()) {
      error = QStringLiteral("dataset alias cannot be empty");
      return {};
    }

    const auto match = findDatasetByAlias(alias);
    if (!match.dataset)
      error = QStringLiteral("Dataset not found for alias '%1'").arg(alias);

    return match;
  }

  const int uniqueId = selector.toInt();
  const auto match   = findDatasetByUniqueId(uniqueId);
  if (!match.dataset)
    error = QStringLiteral("Dataset not found for uniqueId %1").arg(uniqueId);

  return match;
}

/**
 * @brief A resolved offset/limit window over a list of @c total items; nextOffset is -1 when
 *        the window reaches the end.
 */
struct ListWindow {
  int start      = 0;
  int count      = 0;
  int nextOffset = -1;
};

/**
 * @brief Resolves offset/limit paging over @p total items; limit <= 0 means "everything after
 *        offset" (the wire back-compat default shared with project.snapshot).
 */
[[nodiscard]] inline ListWindow applyWindow(int total, int offset, int limit)
{
  ListWindow window;
  window.start = qBound(0, offset, total);
  window.count = limit > 0 ? qMin(limit, total - window.start) : total - window.start;
  if (window.start + window.count < total)
    window.nextOffset = window.start + window.count;

  return window;
}

/**
 * @brief Adds a window self-identification block so a paged reply cannot be mistaken for the
 *        full set (offset/count/total; matches the applyWindow result that produced it).
 */
inline void attachWindowInfo(QJsonObject& result, const ListWindow& window, int total)
{
  QJsonObject info;
  info[QStringLiteral("offset")]   = window.start;
  info[QStringLiteral("count")]    = window.count;
  info[QStringLiteral("total")]    = total;
  result[QStringLiteral("window")] = info;
  if (window.nextOffset >= 0)
    result[QStringLiteral("nextOffset")] = window.nextOffset;
}

/**
 * @brief Adds projectEpoch (monotonic mutation counter) to @p result.
 */
inline void attachProjectEpoch(QJsonObject& result)
{
  static auto& projectModel              = DataModel::ProjectModel::instance();
  result[QStringLiteral("projectEpoch")] = static_cast<qint64>(projectModel.mutationEpoch());
}

/**
 * @brief Snapshot of the project epoch before a mutating handler runs.
 */
[[nodiscard]] inline qint64 captureProjectEpoch()
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  return projectModel.mutationEpoch();
}

/**
 * @brief Append a stale_project warning when the caller's expectedProjectEpoch is stale.
 */
inline void appendStaleProjectWarning(QJsonObject& result,
                                      const QJsonObject& params,
                                      qint64 preMutationEpoch)
{
  if (!params.contains(QStringLiteral("expectedProjectEpoch")))
    return;

  const qint64 expected =
    params.value(QStringLiteral("expectedProjectEpoch")).toVariant().toLongLong();
  if (preMutationEpoch == expected)
    return;

  QJsonArray warnings;
  if (result.contains(QStringLiteral("warnings")))
    warnings = result.value(QStringLiteral("warnings")).toArray();

  QJsonObject w;
  w[QStringLiteral("code")]                 = QStringLiteral("stale_project");
  w[QStringLiteral("expectedProjectEpoch")] = expected;
  w[QStringLiteral("currentProjectEpoch")]  = preMutationEpoch;
  w[QStringLiteral("message")] =
    QStringLiteral("Project was mutated %1 time(s) between the epoch you supplied and "
                   "this call. uniqueIds derived from the older snapshot may now point "
                   "at a different group/dataset -- refetch via project.snapshot or "
                   "resolve by path with project.dataset.getByPath before mutating "
                   "further. The current call still executed as requested.")
      .arg(preMutationEpoch - expected);
  warnings.append(w);
  result[QStringLiteral("warnings")] = warnings;
}

/**
 * @brief Flags obvious language/syntax mismatches; returns a short warning or empty.
 */
[[nodiscard]] inline QString detectLanguageMismatch(const QString& code, int language)
{
  static const QString kJsHallmarks[] = {
    QStringLiteral("\nvar "),
    QStringLiteral("\nlet "),
    QStringLiteral("\nconst "),
    QStringLiteral(" => "),
    QStringLiteral("=== "),
    QStringLiteral("!== "),
    QStringLiteral(") {"),
    QStringLiteral("} else"),
  };
  static const QString kLuaHallmarks[] = {
    QStringLiteral("\nlocal "),
    QStringLiteral("\nfunction "),
    QStringLiteral(" then\n"),
    QStringLiteral(" do\n"),
    QStringLiteral("\nend\n"),
    QStringLiteral("\nelseif "),
    QStringLiteral("--[["),
  };

  bool looksJs  = false;
  bool looksLua = false;
  for (const auto& m : kJsHallmarks)
    if (code.contains(m)) {
      looksJs = true;
      break;
    }
  for (const auto& m : kLuaHallmarks)
    if (code.contains(m)) {
      looksLua = true;
      break;
    }

  if (language == SerialStudio::JavaScript && looksLua && !looksJs)
    return QStringLiteral("language=0 (JavaScript) but the code contains Lua-only "
                          "syntax (e.g. 'local', 'end', 'then'). The script will "
                          "fail to compile silently. Either pass language=1 (Lua) "
                          "or rewrite the code in JavaScript.");

  if (language == SerialStudio::Lua && looksJs && !looksLua)
    return QStringLiteral("language=1 (Lua) but the code contains JS-only syntax "
                          "(e.g. 'var', 'let', 'const', '=>'). The script will "
                          "fail to compile silently. Either pass language=0 "
                          "(JavaScript) or rewrite the code in Lua.");

  return QString();
}

/**
 * @brief Returns the DatasetOption bitflag value of @a ds (1=Plot, 2=FFT, ...).
 */
inline int datasetOptionsBitflag(const DataModel::Dataset& ds)
{
  int flags = SerialStudio::DatasetGeneric;
  if (ds.plt)
    flags |= SerialStudio::DatasetPlot;

  if (ds.fft)
    flags |= SerialStudio::DatasetFFT;

  if (ds.led)
    flags |= SerialStudio::DatasetLED;

  if (ds.waterfall)
    flags |= SerialStudio::DatasetWaterfall;

  static const QHash<QString, int> kWidgetFlags = {
    {    QStringLiteral("bar"),     SerialStudio::DatasetBar},
    {  QStringLiteral("gauge"),   SerialStudio::DatasetGauge},
    {QStringLiteral("compass"), SerialStudio::DatasetCompass},
    {  QStringLiteral("meter"),   SerialStudio::DatasetMeter},
  };
  flags |= kWidgetFlags.value(ds.widget, 0);

  return flags;
}

}  // namespace API::Handlers
