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

#include "DataModel/Project/WorkspaceKeys.h"

#include <algorithm>

#include "Core/DataModel/Frame.h"
#include "Core/License.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/WidgetResolution.h"

// Key layout: 20 bits of ordinal, 24 bits of group uniqueId (legacy ids pass a million), type above
static constexpr int kIndexBits    = 20;
static constexpr int kGroupBits    = 24;
static constexpr qint64 kIndexMask = (qint64(1) << kIndexBits) - 1;
static constexpr qint64 kGroupMask = (qint64(1) << kGroupBits) - 1;
static constexpr int kGroupShift   = kIndexBits;
static constexpr int kTypeShift    = kIndexBits + kGroupBits;

/**
 * @brief Encodes (widgetType, groupId, relativeIndex) into a single 64-bit key.
 */
qint64 DataModel::WorkspaceKeys::workspaceWidgetKey(int widgetType, int groupId, int relIdx)
{
  SS_ASSERT_LOG((static_cast<qint64>(groupId) & ~kGroupMask) == 0);
  SS_ASSERT_LOG((static_cast<qint64>(relIdx) & ~kIndexMask) == 0);
  return (static_cast<qint64>(widgetType) << kTypeShift)
       | ((static_cast<qint64>(groupId) & kGroupMask) << kGroupShift)
       | (static_cast<qint64>(relIdx) & kIndexMask);
}

/**
 * @brief The widget type a workspaceWidgetKey encodes.
 */
int DataModel::WorkspaceKeys::widgetTypeOfKey(qint64 key)
{
  return static_cast<int>(key >> kTypeShift);
}

/**
 * @brief The per-type ordinal a workspaceWidgetKey encodes.
 */
int DataModel::WorkspaceKeys::relativeIndexOfKey(qint64 key)
{
  return static_cast<int>(key & kIndexMask);
}

/**
 * @brief Identity key of one ref: type, group and dataset uniqueIds, ordinal-free. The dataset
 *        id takes the low 32 bits (+1 so -1 packs as 0), the group the 24 bits above it.
 */
[[nodiscard]] static qint64 identityKey(int widgetType, int groupUniqueId, int datasetUniqueId)
{
  return (static_cast<qint64>(widgetType) << (32 + kGroupBits))
       | ((static_cast<qint64>(groupUniqueId) & kGroupMask) << 32)
       | static_cast<qint64>(static_cast<quint32>(datasetUniqueId + 1));
}

/**
 * @brief Rebinds one ref: identity first, then the legacy ordinal with a back-fill.
 */
[[nodiscard]] static bool rebindOneRef(
  DataModel::WidgetRef& ref,
  const QHash<qint64, DataModel::WorkspaceKeys::ResolvedWidget>& lookup,
  const QHash<qint64, int>& identityToRel)
{
  const auto byIdentity =
    identityToRel.constFind(identityKey(ref.widgetType, ref.groupUniqueId, ref.datasetUniqueId));
  if (byIdentity != identityToRel.constEnd()) {
    ref.relativeIndex = byIdentity.value();
    return true;
  }

  const auto byOrdinal = lookup.constFind(DataModel::WorkspaceKeys::workspaceWidgetKey(
    ref.widgetType, ref.groupUniqueId, ref.relativeIndex));
  if (byOrdinal == lookup.constEnd())
    return false;

  if (!byOrdinal->isGroupWidget && ref.datasetUniqueId < 0)
    ref.datasetUniqueId = byOrdinal->uniqueId;

  return true;
}

/**
 * @brief Rebinds every ref of every workspace; see the header.
 */
int DataModel::WorkspaceKeys::rebindWidgetRefs(std::vector<Workspace>& workspaces,
                                               const QHash<qint64, ResolvedWidget>& lookup)
{
  QHash<qint64, int> identityToRel;
  identityToRel.reserve(lookup.size());
  for (auto it = lookup.constBegin(); it != lookup.constEnd(); ++it) {
    const auto& entry     = it.value();
    const int datasetUid  = entry.isGroupWidget ? -1 : entry.uniqueId;
    const qint64 identity = identityKey(widgetTypeOfKey(it.key()), entry.groupUniqueId, datasetUid);
    identityToRel.insert(identity, relativeIndexOfKey(it.key()));
  }

  int unresolved = 0;
  for (auto& workspace : workspaces)
    for (auto& ref : workspace.widgetRefs)
      if (!rebindOneRef(ref, lookup, identityToRel))
        ++unresolved;

  SS_ASSERT_LOG(unresolved >= 0);
  return unresolved;
}

namespace DataModel::WorkspaceKeys {

/**
 * @brief Builds the lookup of every widget reference the project currently exposes.
 */
QHash<qint64, ResolvedWidget> buildResolvedWidgetLookup(const DataModel::ProjectModel& pm)
{
  return buildResolvedWidgetLookup(pm.groups(), Core::License::activated());
}

/**
 * @brief The pure form of the lookup: every eligible widget of @p groups with its per-type
 *        ordinal, computed the way the dashboard numbers its slots.
 */
QHash<qint64, ResolvedWidget> buildResolvedWidgetLookup(const std::vector<Group>& groups, bool pro)
{
  QHash<qint64, ResolvedWidget> lookup;
  QHash<int, int> groupRunning;
  QHash<int, int> datasetRunning;

  datasetRunning.insert(static_cast<int>(SerialStudio::DashboardExtension),
                        SerialStudio::extensionGroupWidgetCount(groups));

  for (const auto& g : groups) {
    if (!SerialStudio::groupEligibleForWorkspace(g))
      continue;

    auto groupKey = SerialStudio::getDashboardWidget(g);
    if (groupKey == SerialStudio::DashboardPlot3D && !pro)
      groupKey = SerialStudio::DashboardMultiPlot;

    const bool isEmptyOutputPanel =
      g.groupType == DataModel::GroupType::Output && g.outputWidgets.empty();

    if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey) && !isEmptyOutputPanel) {
      const int typeKey = static_cast<int>(groupKey);
      const int relIdx  = groupRunning.value(typeKey, 0);
      groupRunning.insert(typeKey, relIdx + 1);

      ResolvedWidget entry;
      entry.groupTitle    = g.title;
      entry.datasetTitle  = QString();
      entry.uniqueId      = g.uniqueId;
      entry.groupUniqueId = g.uniqueId;
      entry.isGroupWidget = true;
      lookup.insert(workspaceWidgetKey(typeKey, g.uniqueId, relIdx), entry);
    }

    const auto recordDatasetWidget = [&](const DataModel::Dataset& ds,
                                         SerialStudio::DashboardWidget k) {
      const int typeKey = static_cast<int>(k);
      const int relIdx  = datasetRunning.value(typeKey, 0);
      datasetRunning.insert(typeKey, relIdx + 1);

      ResolvedWidget entry;
      entry.groupTitle    = g.title;
      entry.datasetTitle  = ds.title;
      entry.uniqueId      = ds.uniqueId;
      entry.groupUniqueId = g.uniqueId;
      lookup.insert(workspaceWidgetKey(typeKey, g.uniqueId, relIdx), entry);
    };

    const auto walkDatasetWidgets = [&](const DataModel::Dataset& ds) {
      const auto keys = SerialStudio::getDashboardWidgets(ds);
      for (const auto& k : keys)
        if (SerialStudio::datasetWidgetEligibleForWorkspace(k))
          recordDatasetWidget(ds, k);
    };

    for (const auto& ds : g.datasets)
      walkDatasetWidgets(ds);

    const bool groupHasLed =
      std::any_of(g.datasets.begin(), g.datasets.end(), [](const DataModel::Dataset& ds) {
        return !ds.hideOnDashboard && ds.led;
      });
    if (groupHasLed) {
      const int typeKey = static_cast<int>(SerialStudio::DashboardLED);
      const int relIdx  = groupRunning.value(typeKey, 0);
      groupRunning.insert(typeKey, relIdx + 1);

      ResolvedWidget entry;
      entry.groupTitle    = g.title;
      entry.datasetTitle  = QString();
      entry.uniqueId      = g.uniqueId;
      entry.groupUniqueId = g.uniqueId;
      entry.isGroupWidget = true;
      entry.isLedPanel    = true;
      lookup.insert(workspaceWidgetKey(typeKey, g.uniqueId, relIdx), entry);
    }
  }

  return lookup;
}

}  // namespace DataModel::WorkspaceKeys
