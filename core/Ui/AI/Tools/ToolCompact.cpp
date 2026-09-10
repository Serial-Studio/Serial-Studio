/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Tools/ToolCompact.h"

#include <QHash>
#include <QJsonArray>

#include "Core/DataModel/Frame.h"

namespace AI::ToolDetail {

/**
 * @brief Maps a widget slug to the dataset option slug that enables that widget.
 */
QString optionSlugForWidget(const QString& widgetType)
{
  static const QHash<QString, QString> kMap = {
    {     QStringLiteral("plot"),      QStringLiteral("plot")},
    {      QStringLiteral("fft"),       QStringLiteral("fft")},
    {      QStringLiteral("bar"),       QStringLiteral("bar")},
    {    QStringLiteral("gauge"),     QStringLiteral("gauge")},
    {  QStringLiteral("compass"),   QStringLiteral("compass")},
    {      QStringLiteral("led"),       QStringLiteral("led")},
    {QStringLiteral("waterfall"), QStringLiteral("waterfall")},
  };
  return kMap.value(widgetType);
}

/**
 * @brief Returns a trimmed dataset array carrying only fields the assistant needs.
 */
static QJsonArray compactDatasets(const QJsonArray& datasets)
{
  QJsonArray rows;
  for (const auto& value : datasets) {
    const auto ds = value.toObject();
    QJsonObject row;
    row[Keys::DatasetId]         = ds.value(Keys::DatasetId).toInt();
    row[Keys::UniqueId]          = ds.value(Keys::UniqueId).toInt();
    row[QStringLiteral("index")] = ds.value(QStringLiteral("index")).toInt();
    row[QStringLiteral("title")] = ds.value(QStringLiteral("title")).toString();
    if (ds.contains(QStringLiteral("units")))
      row[QStringLiteral("units")] = ds.value(QStringLiteral("units")).toString();

    if (ds.contains(QStringLiteral("enabledOptionsSlugs")))
      row[QStringLiteral("enabledOptionsSlugs")] =
        ds.value(QStringLiteral("enabledOptionsSlugs")).toArray();

    if (ds.value(QStringLiteral("hasTransform")).toBool())
      row[QStringLiteral("hasTransform")] = true;

    if (ds.value(QStringLiteral("isVirtual")).toBool())
      row[QStringLiteral("isVirtual")] = true;

    rows.append(row);
  }
  return rows;
}

/**
 * @brief Compacts a project.snapshot result into the assistant.snapshot shape.
 */
QJsonObject compactProjectSnapshotResult(const QJsonObject& projectResult,
                                         const QJsonObject& workspaceResult,
                                         bool includeRaw)
{
  const auto snapshot = projectResult.value(QStringLiteral("snapshot")).toObject();

  QJsonObject compact;
  compact[QStringLiteral("title")]    = snapshot.value(QStringLiteral("title")).toString();
  compact[QStringLiteral("filePath")] = snapshot.value(QStringLiteral("filePath")).toString();
  compact[QStringLiteral("operationMode")] =
    snapshot.value(QStringLiteral("operationMode")).toInt();
  compact[QStringLiteral("groupCount")]   = snapshot.value(QStringLiteral("groupCount")).toInt();
  compact[QStringLiteral("datasetCount")] = snapshot.value(QStringLiteral("datasetCount")).toInt();
  compact[QStringLiteral("projectEpoch")] = projectResult.value(QStringLiteral("projectEpoch"));
  compact[QStringLiteral("summary")]      = snapshot.value(QStringLiteral("_explanations"))
                                         .toObject()
                                         .value(QStringLiteral("summary"))
                                         .toString();

  QJsonArray groups;
  for (const auto& value : snapshot.value(QStringLiteral("groups")).toArray()) {
    const auto group = value.toObject();
    QJsonObject row;
    row[QStringLiteral("groupId")]      = group.value(QStringLiteral("groupId")).toInt();
    row[QStringLiteral("title")]        = group.value(QStringLiteral("title")).toString();
    row[QStringLiteral("widget")]       = group.value(QStringLiteral("widget")).toString();
    row[QStringLiteral("datasetCount")] = group.value(QStringLiteral("datasetCount")).toInt();
    if (group.contains(QStringLiteral("compatibleWidgetTypeSlugs")))
      row[QStringLiteral("compatibleWidgetTypeSlugs")] =
        group.value(QStringLiteral("compatibleWidgetTypeSlugs")).toArray();

    row[QStringLiteral("datasets")] =
      compactDatasets(group.value(QStringLiteral("datasets")).toArray());
    groups.append(row);
  }
  compact[QStringLiteral("groups")] = groups;

  const auto workspaceRows = workspaceResult.value(QStringLiteral("workspaces")).toArray();
  compact[QStringLiteral("workspaces")] = workspaceRows.isEmpty()
                                          ? snapshot.value(QStringLiteral("workspaces")).toArray()
                                          : workspaceRows;
  compact[QStringLiteral("dataTables")] = snapshot.value(QStringLiteral("dataTables")).toArray();
  compact[QStringLiteral("hint")]       = projectResult.value(QStringLiteral("hint")).toString();

  if (includeRaw)
    compact[QStringLiteral("rawProjectSnapshot")] = projectResult;

  return compact;
}

}  // namespace AI::ToolDetail
