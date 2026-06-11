/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#pragma once

#include <cmath>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <vector>

#include "DataModel/Frame.h"
#include "SerialStudio.h"

namespace DataModel {

/**
 * @brief Appends an explicit "active" alarm band to a boolean LED dataset so the LED lights
 *        through the alarm-band system instead of the legacy runtime ledHigh synthesis.
 */
inline void applyBooleanLedBand(Dataset& dataset, const QString& label)
{
  AlarmBand band;
  band.min      = 0.5;
  band.max      = 1.0;
  band.severity = AlarmSeverity::Ok;
  band.label    = label;
  dataset.alarmBands.push_back(band);
}

/**
 * @brief Returns the string as a double-quoted Lua literal with escapes applied.
 */
[[nodiscard]] inline QString luaQuote(const QString& text)
{
  QString escaped = text;
  escaped.replace(QLatin1Char('\\'), QLatin1String("\\\\"))
    .replace(QLatin1Char('"'), QLatin1String("\\\""))
    .replace(QLatin1Char('\n'), QLatin1String("\\n"));

  return QLatin1Char('"') + escaped + QLatin1Char('"');
}

/**
 * @brief Returns the value as a Lua number literal with a lossless double round-trip.
 */
[[nodiscard]] inline QString luaNumber(double value)
{
  const bool whole = std::isfinite(value) && value == std::floor(value) && std::fabs(value) < 1e15;
  if (whole)
    return QString::number(static_cast<qint64>(value));

  return QString::number(value, 'g', 15);
}

/**
 * @brief Returns how many decimal places the value's fractional part needs (capped at 4),
 *        used to derive a dataset displayFormat from a raw->physical scale factor.
 */
[[nodiscard]] inline int fractionalDecimals(double value)
{
  int decimals = 0;
  double f     = std::fabs(value);
  while (decimals < 4 && std::isfinite(f) && std::fabs(f - std::round(f)) > 1e-9) {
    f *= 10.0;
    ++decimals;
  }

  return decimals;
}

/**
 * @brief Picks a tick count whose labels land on whole numbers: the smallest step from the
 *        1/2/2.5/5 decade that divides both the range and the lower bound into at most 12
 *        intervals (e.g. 0-10 -> 11 ticks, 0-100 -> 11 ticks, 0-150 -> 7 ticks). Sub-unity or
 *        non-aligned ranges keep @p fallback.
 */
[[nodiscard]] inline int integerTickCount(double min, double max, int fallback)
{
  const double span = max - min;
  if (!std::isfinite(span) || span < 1.0)
    return fallback;

  const auto divides = [](double value, double step) {
    return std::fabs(std::remainder(value, step)) < step * 1e-9;
  };

  static constexpr double kBases[] = {1.0, 2.0, 2.5, 5.0};
  for (double mag = 1.0; mag <= span; mag *= 10.0) {
    for (const double base : kBases) {
      const double step = base * mag;
      if (!divides(step, 1.0))
        continue;

      const double intervals = span / step;
      if (intervals <= 12.0 && divides(span, step) && divides(min, step))
        return static_cast<int>(std::lround(intervals)) + 1;
    }
  }

  return fallback;
}

/**
 * @brief Applies the analog-widget display policy to a generated bar/gauge/meter dataset:
 *        integer-aligned tick labels, and integer value formatting once the range spans more
 *        than one unit (sub-unity ranges keep their scale-derived decimals).
 */
inline void applyAnalogDisplayPolicy(Dataset& dataset)
{
  dataset.displayTickCount =
    integerTickCount(dataset.wgtMin, dataset.wgtMax, dataset.displayTickCount);

  if (dataset.wgtMax - dataset.wgtMin > 1.0)
    dataset.displayFormat = QStringLiteral("0d");
}

/**
 * @brief Marks the dataset as virtual and installs a Lua transform that reads its value back
 *        from a data-table register, decoupling it from positional parser channels.
 */
inline void applyTableTransform(Dataset& dataset, const QString& table, const QString& reg)
{
  dataset.virtual_          = true;
  dataset.transformLanguage = static_cast<int>(SerialStudio::Lua);
  dataset.transformCode =
    QStringLiteral("function transform(value)\n  return tableGet(%1, %2) or 0\nend\n")
      .arg(luaQuote(table), luaQuote(reg));
}

/**
 * @brief Builds the summary-first workspace list for an imported project: one workspace per
 *        group holding only the group-level widget (plus the LED panel when the group has LED
 *        datasets), so per-dataset plots/gauges stay disclosed-on-demand via the data grid.
 *        Multi-group projects get a leading Overview workspace aggregating those refs.
 */
[[nodiscard]] inline QJsonArray buildImportedWorkspaces(const std::vector<Group>& groups,
                                                        const QString& overviewTitle)
{
  std::vector<Workspace> perGroup;
  QMap<int, int> typeCounters;

  const auto appendRef =
    [&typeCounters](Workspace& ws, SerialStudio::DashboardWidget key, int groupUniqueId) {
      WidgetRef ref;
      ref.widgetType               = static_cast<int>(key);
      ref.groupUniqueId            = groupUniqueId;
      ref.relativeIndex            = typeCounters.value(ref.widgetType, 0);
      typeCounters[ref.widgetType] = ref.relativeIndex + 1;
      ws.widgetRefs.push_back(ref);
    };

  int workspaceId = WorkspaceIds::UserStart + 1;
  for (const auto& group : groups) {
    Workspace ws;
    ws.workspaceId = workspaceId;
    ws.title       = group.title;

    const auto groupKey = SerialStudio::getDashboardWidget(group);
    if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey))
      appendRef(ws, groupKey, group.uniqueId);

    bool hasLed = false;
    for (const auto& dataset : group.datasets)
      hasLed |= dataset.led;

    if (hasLed)
      appendRef(ws, SerialStudio::DashboardLED, group.uniqueId);

    if (ws.widgetRefs.empty())
      continue;

    perGroup.push_back(std::move(ws));
    ++workspaceId;
  }

  QJsonArray workspaces;
  if (perGroup.size() >= 2) {
    Workspace overview;
    overview.workspaceId = WorkspaceIds::UserStart;
    overview.title       = overviewTitle;
    overview.icon        = QStringLiteral("qrc:/icons/panes/overview.svg");
    for (const auto& ws : perGroup)
      overview.widgetRefs.insert(
        overview.widgetRefs.end(), ws.widgetRefs.begin(), ws.widgetRefs.end());

    workspaces.append(serialize(overview));
  }

  for (const auto& ws : perGroup)
    workspaces.append(serialize(ws));

  return workspaces;
}

/**
 * @brief Assigns stable group uniqueIds, serializes the groups and the seeded workspaces into
 *        @p project, and stamps schemaVersion/nextUniqueId: without those stamps the loader
 *        treats the import as an older-schema project and deliberately drops the workspaces.
 */
inline void finalizeImportedProject(QJsonObject& project,
                                    std::vector<Group>& groups,
                                    const std::vector<TableDef>& tables,
                                    const QString& overviewTitle)
{
  const int groupCount = static_cast<int>(groups.size());
  for (int g = 0; g < groupCount; ++g)
    groups[static_cast<size_t>(g)].uniqueId = kGroupIdStride * groupCount + g;

  QJsonArray groupArray;
  for (const auto& group : groups)
    groupArray.append(serialize(group));

  project.insert(Keys::Groups, groupArray);
  project.insert(Keys::SchemaVersion, kSchemaVersion);
  project.insert(Keys::NextUniqueId, qMax(1, kGroupIdStride * groupCount + groupCount));

  if (!tables.empty()) {
    QJsonArray tableArray;
    for (const auto& table : tables)
      tableArray.append(serialize(table));

    project.insert(Keys::Tables, tableArray);
  }

  const auto workspaces = buildImportedWorkspaces(groups, overviewTitle);
  if (!workspaces.isEmpty()) {
    project.insert(Keys::CustomizeWorkspaces, true);
    project.insert(Keys::Workspaces, workspaces);
  }
}

}  // namespace DataModel
