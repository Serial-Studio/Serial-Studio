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

#include "Core/DataModel/FrameSupport.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
// Commercial feature detection, appreciate your respect for this project
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true if a transform script references the notify() API family.
 */
static bool transformUsesNotifications(const QString& code)
{
  return code.contains(QStringLiteral("notify(")) || code.contains(QStringLiteral("notifyInfo("))
      || code.contains(QStringLiteral("notifyWarning("))
      || code.contains(QStringLiteral("notifyCritical("))
      || code.contains(QStringLiteral("notifyClear("));
}

/**
 * @brief Returns true when one group needs a commercial feature.
 */
static bool groupNeedsCommercial(const DataModel::Group& group)
{
  if (group.groupType == DataModel::GroupType::Output)
    return true;

  if (group.widget == QStringLiteral("plot3d") || group.widget == QStringLiteral("image")
      || group.widget == QStringLiteral("painter"))
    return true;

  for (const auto& dataset : std::as_const(group.datasets)) {
    if (dataset.waterfall)
      return true;

    if (!dataset.transformCode.isEmpty() && transformUsesNotifications(dataset.transformCode))
      return true;
  }

  return false;
}

/**
 * @brief Checks if a project configuration (QVector form) requires commercial features.
 */
bool SerialStudio::commercialCfg(const QVector<DataModel::Group>& g)
{
  for (const auto& group : std::as_const(g))
    if (groupNeedsCommercial(group))
      return true;

  return false;
}

/**
 * @brief Checks if a project configuration requires commercial features.
 */
bool SerialStudio::commercialCfg(const std::vector<DataModel::Group>& g)
{
  for (const auto& group : std::as_const(g))
    if (groupNeedsCommercial(group))
      return true;

  return false;
}

//--------------------------------------------------------------------------------------------------
// Pro content detection
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when @p code holds anything but whitespace; scans in place, no trimmed copy.
 */
static bool hasCode(const QString& code)
{
  return std::any_of(code.cbegin(), code.cend(), [](const QChar c) { return !c.isSpace(); });
}

/**
 * @brief Summarizes the content that needs an entitlement to run: datasets with non-blank
 *        transform code, plus the user tables the caller counted.
 */
SerialStudio::ProContentSummary SerialStudio::proContentSummary(
  const std::vector<DataModel::Group>& g, const int tableCount)
{
  int transforms = 0;
  for (const auto& group : g)
    for (const auto& dataset : group.datasets)
      if (hasCode(dataset.transformCode))
        ++transforms;

  return {transforms, qMax(0, tableCount)};
}

/**
 * @brief Whether replacing @p before with @p after writes a transform: non-blank code that differs
 *        from what was there in code, language or parameters. Clearing never counts (spec 0094 R7).
 */
bool SerialStudio::authorsTransform(const DataModel::Dataset& before,
                                    const DataModel::Dataset& after)
{
  if (!hasCode(after.transformCode))
    return false;

  return after.transformCode != before.transformCode
      || after.transformLanguage != before.transformLanguage
      || after.transformParams != before.transformParams;
}

//--------------------------------------------------------------------------------------------------
// Workspace eligibility and X-axis policy
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether a group contributes any widgets to Dashboard's walker.
 */
bool SerialStudio::groupEligibleForWorkspace(const DataModel::Group& g)
{
  Q_UNUSED(g);
  return true;
}

/**
 * @brief Classifies a group's X-axis mode from its front dataset's encoding. Empty groups and the
 *        time sentinel map to Time, the samples sentinel to Samples, any dataset id to Dataset.
 *        Callers apply their own guards on the empty case (Dashboard's !empty guard sends it to
 *        the samples path, ProjectEditor shows "Time"); that asymmetry is intentional.
 */
SerialStudio::XAxisMode SerialStudio::groupXAxisMode(const DataModel::Group& g)
{
  if (g.datasets.empty())
    return XAxisMode::Time;

  const int frontXAxisId = g.datasets.front().xAxisId;
  if (frontXAxisId == DataModel::kXAxisSamples)
    return XAxisMode::Samples;

  if (frontXAxisId == DataModel::kXAxisTime)
    return XAxisMode::Time;

  return XAxisMode::Dataset;
}

/**
 * @brief Resolves a dataset's X-axis policy: Time for the time sentinel, Dataset when the X source
 *        resolves against the live map, else Samples. No license tier gates this mode; an xAxisId
 *        that no longer names a live dataset is the only path to the Samples degrade.
 */
SerialStudio::XAxisPolicy SerialStudio::resolveXAxisPolicy(
  const DataModel::Dataset& d, const QMap<int, DataModel::Dataset>& datasets)
{
  if (d.xAxisId == DataModel::kXAxisTime)
    return {XAxisMode::Time, -1};

  if (d.xAxisId >= 0 && datasets.contains(d.xAxisId))
    return {XAxisMode::Dataset, d.xAxisId};

  return {XAxisMode::Samples, -1};
}
