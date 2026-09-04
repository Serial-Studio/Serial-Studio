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

#include "DataModel/Project/ProjectWorkspaceRefs.h"

#include <algorithm>
#include <QPair>

#include "Core/SSAssert.h"
#include "SerialStudio.h"

namespace DataModel::WorkspaceRefs {

/**
 * @brief Resolves a Group.uniqueId to its current positional groupId; returns -1 if absent.
 */
static int positionalGroupId(const std::vector<Group>& groups, int uniqueId)
{
  if (uniqueId < 0)
    return -1;

  for (const auto& group : groups)
    if (group.uniqueId == uniqueId)
      return group.groupId;

  return -1;
}

/**
 * @brief Increments the per-type counter for every eligible dataset widget.
 */
static void tallyDatasetWidgetTypes(const Dataset& ds, QMap<int, int>& counts)
{
  const auto keys = SerialStudio::getDashboardWidgets(ds);
  for (const auto& k : keys)
    if (SerialStudio::datasetWidgetEligibleForWorkspace(k))
      counts[static_cast<int>(k)] += 1;
}

/**
 * @brief Resolves a workspace ref into a stable RefAnchor before a reorder.
 */
static RefAnchor anchorRef(const WidgetRef& r, const std::vector<Group>& groups)
{
  RefAnchor a;
  a.widgetType        = r.widgetType;
  a.sourceGid         = r.groupUniqueId;
  a.datasetFrameIndex = -1;
  a.isGroupOrLed      = false;

  auto git = std::find_if(groups.begin(), groups.end(), [uid = r.groupUniqueId](const auto& g) {
    return g.uniqueId == uid;
  });
  if (git == groups.end())
    return a;

  const auto& g            = *git;
  const auto groupKey      = SerialStudio::getDashboardWidget(g);
  const bool emptyOutPanel = g.groupType == DataModel::GroupType::Output && g.outputWidgets.empty();
  const bool groupRef = SerialStudio::groupWidgetEligibleForWorkspace(groupKey) && !emptyOutPanel
                     && static_cast<int>(groupKey) == r.widgetType;
  const bool ledAggregate = (r.widgetType == static_cast<int>(SerialStudio::DashboardLED));
  if (groupRef || ledAggregate) {
    a.isGroupOrLed = true;
    return a;
  }

  int slot = 0;
  for (const auto& d : g.datasets) {
    const auto keys = SerialStudio::getDashboardWidgets(d);
    for (const auto& k : keys) {
      if (static_cast<int>(k) != r.widgetType)
        continue;

      if (!SerialStudio::datasetWidgetEligibleForWorkspace(k))
        continue;

      if (slot == r.relativeIndex) {
        a.datasetFrameIndex = d.index;
        return a;
      }

      slot += 1;
    }
  }

  return a;
}

/**
 * @brief Re-resolves a RefAnchor into a per-type slot index in the given group.
 *        Returns -1 if the anchor's dataset is not present.
 */
static int slotForAnchor(const RefAnchor& a, const Group& g)
{
  if (a.datasetFrameIndex < 0)
    return -1;

  int slot = 0;
  for (const auto& d : g.datasets) {
    const auto keys = SerialStudio::getDashboardWidgets(d);
    for (const auto& k : keys) {
      if (static_cast<int>(k) != a.widgetType)
        continue;

      if (!SerialStudio::datasetWidgetEligibleForWorkspace(k))
        continue;

      if (d.index == a.datasetFrameIndex)
        return slot;

      slot += 1;
    }
  }

  return -1;
}

/**
 * @brief Walks one workspace's refs against the new group/dataset layout, refreshing
 *        the dataset slot. The group identity is uniqueId-based so it never needs remapping.
 */
static void resolveOneWorkspaceRefs(Workspace& ws,
                                    const std::vector<RefAnchor>& src,
                                    const std::vector<Group>& groups)
{
  SS_ASSERT_LOG(src.size() == ws.widgetRefs.size());

  const size_t count = std::min(src.size(), ws.widgetRefs.size());
  for (size_t i = 0; i < count; ++i) {
    auto& r       = ws.widgetRefs[i];
    const auto& a = src[i];

    if (a.sourceGid < 0 || a.isGroupOrLed)
      continue;

    auto git = std::find_if(groups.begin(), groups.end(), [uid = r.groupUniqueId](const auto& g) {
      return g.uniqueId == uid;
    });
    if (git == groups.end())
      continue;

    const int newSlot = slotForAnchor(a, *git);
    if (newSlot >= 0)
      r.relativeIndex = newSlot;
  }
}

/**
 * @brief Sums the per-type widget counts every group ahead of @p groupId contributes, so a
 *        dataset-level ref index can be compared against the running counter Dashboard uses.
 */
static QMap<int, int> runningCountsBeforeGroup(const std::vector<Group>& groups, int groupId)
{
  QMap<int, int> running;
  for (const auto& g : groups) {
    if (!SerialStudio::groupEligibleForWorkspace(g))
      continue;

    if (g.groupId == groupId)
      break;

    const auto groupKey = SerialStudio::getDashboardWidget(g);

    const bool isEmptyOutputPanel =
      g.groupType == DataModel::GroupType::Output && g.outputWidgets.empty();

    if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey) && !isEmptyOutputPanel)
      running[static_cast<int>(groupKey)] += 1;

    for (const auto& ds : g.datasets)
      tallyDatasetWidgetTypes(ds, running);
  }

  return running;
}

/**
 * @brief Counts, per dashboard-widget type, how many widgets the given group
 *        contributes to Dashboard::buildWidgetGroups's running type counter.
 */
QMap<int, int> widgetTypeCountsForGroup(const Group& group)
{
  QMap<int, int> counts;

  if (!SerialStudio::groupEligibleForWorkspace(group))
    return counts;

  auto groupKey = SerialStudio::getDashboardWidget(group);
  if (groupKey == SerialStudio::DashboardPlot3D && !SerialStudio::activated())
    groupKey = SerialStudio::DashboardMultiPlot;

  const bool isEmptyOutputPanel =
    group.groupType == DataModel::GroupType::Output && group.outputWidgets.empty();

  if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey) && !isEmptyOutputPanel)
    counts[static_cast<int>(groupKey)] += 1;

  bool groupHasLed = false;
  for (const auto& ds : group.datasets) {
    if (ds.hideOnDashboard)
      continue;

    const auto keys = SerialStudio::getDashboardWidgets(ds);
    for (const auto& k : keys) {
      if (k == SerialStudio::DashboardLED) {
        groupHasLed = true;
        continue;
      }
      if (!SerialStudio::datasetWidgetEligibleForWorkspace(k))
        continue;

      counts[static_cast<int>(k)] += 1;
    }
  }

  if (groupHasLed)
    counts[static_cast<int>(SerialStudio::DashboardLED)] += 1;

  return counts;
}

/**
 * @brief Shifts or drops user-customised widget refs after a group delete.
 */
void shiftRefsAfterGroupDelete(std::vector<Workspace>& workspaces,
                               const std::vector<Group>& groups,
                               int deletedGid,
                               const QMap<int, int>& deletedTypeCounts)
{
  SS_ASSERT(deletedGid >= 0, return);

  const int deletedAutoId = WorkspaceIds::PerGroupStart + deletedGid;

  workspaces.erase(
    std::remove_if(workspaces.begin(),
                   workspaces.end(),
                   [deletedAutoId](const Workspace& w) { return w.workspaceId == deletedAutoId; }),
    workspaces.end());

  for (auto& ws : workspaces) {
    if (ws.workspaceId > deletedAutoId && ws.workspaceId < WorkspaceIds::PerFolderStart)
      ws.workspaceId -= 1;

    for (auto it = ws.widgetRefs.begin(); it != ws.widgetRefs.end();) {
      const int newPos = positionalGroupId(groups, it->groupUniqueId);
      if (newPos < 0) {
        it = ws.widgetRefs.erase(it);
        continue;
      }

      if (newPos >= deletedGid) {
        const int lost    = deletedTypeCounts.value(it->widgetType, 0);
        it->relativeIndex = std::max(0, it->relativeIndex - lost);
      }

      ++it;
    }
  }
}

/**
 * @brief Shifts user-customised widget refs after a single dataset is deleted from a surviving
 *        group; @p groupUniqueId is the persistent id of the group at @p groupId.
 */
void shiftRefsAfterDatasetDelete(std::vector<Workspace>& workspaces,
                                 const std::vector<Group>& groups,
                                 int groupId,
                                 int groupUniqueId,
                                 const QMap<int, int>& datasetTypeCounts)
{
  SS_ASSERT(groupId >= 0, return);

  if (datasetTypeCounts.isEmpty())
    return;

  const QMap<int, int> runningAtGroup = runningCountsBeforeGroup(groups, groupId);

  for (auto& ws : workspaces) {
    ws.widgetRefs.erase(std::remove_if(ws.widgetRefs.begin(),
                                       ws.widgetRefs.end(),
                                       [&](const WidgetRef& r) {
                                         const int lost = datasetTypeCounts.value(r.widgetType, 0);
                                         if (lost == 0 || r.groupUniqueId != groupUniqueId)
                                           return false;

                                         const int base = runningAtGroup.value(r.widgetType, 0);
                                         return r.relativeIndex >= base
                                             && r.relativeIndex < base + lost;
                                       }),
                        ws.widgetRefs.end());

    for (auto& r : ws.widgetRefs) {
      const int lost = datasetTypeCounts.value(r.widgetType, 0);
      if (lost == 0)
        continue;

      const int base = runningAtGroup.value(r.widgetType, 0);
      if (r.relativeIndex < base + lost)
        continue;

      r.relativeIndex -= lost;
      SS_ASSERT(r.relativeIndex >= 0, r.relativeIndex = 0);
    }
  }
}

/**
 * @brief Updates the hidden-group set after a group is removed and surviving groups are renumbered
 *        down by 1.
 */
void shiftHiddenGroupIdsAfterGroupDelete(QSet<int>& hiddenGroupIds, int deletedGid)
{
  if (hiddenGroupIds.isEmpty())
    return;

  QSet<int> updated;
  for (const int id : std::as_const(hiddenGroupIds)) {
    if (id == deletedGid)
      continue;

    updated.insert(id > deletedGid ? id - 1 : id);
  }

  hiddenGroupIds = std::move(updated);
}

/**
 * @brief Updates layout:N widgetSettings entries after a group renumber; returns whether the blob
 *        changed so the caller can emit widgetSettingsChanged exactly once.
 */
bool shiftLayoutKeysAfterGroupDelete(QJsonObject& widgetSettings, int deletedGid)
{
  if (widgetSettings.isEmpty())
    return false;

  const auto keys = widgetSettings.keys();
  bool changed    = false;

  if (widgetSettings.contains(Keys::layoutKey(deletedGid))) {
    widgetSettings.remove(Keys::layoutKey(deletedGid));
    changed = true;
  }

  const QString prefix = QStringLiteral("layout:");
  QList<QPair<int, QJsonObject>> moves;
  for (const auto& key : keys) {
    if (!key.startsWith(prefix))
      continue;

    bool ok      = false;
    const int id = key.mid(prefix.length()).toInt(&ok);
    if (!ok || id <= deletedGid)
      continue;

    moves.append({id, widgetSettings.value(key).toObject()});
  }

  std::sort(
    moves.begin(), moves.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

  for (const auto& move : moves) {
    widgetSettings.remove(Keys::layoutKey(move.first));
    widgetSettings.insert(Keys::layoutKey(move.first - 1), move.second);
    changed = true;
  }

  return changed;
}

/**
 * @brief Updates the hidden-group set so each hidden ID follows its renamed group.
 */
void remapHiddenGroupIdsAfterReorder(QSet<int>& hiddenGroupIds, const std::vector<int>& oldToNewGid)
{
  if (hiddenGroupIds.isEmpty())
    return;

  QSet<int> updated;
  for (const int id : std::as_const(hiddenGroupIds)) {
    if (id < 0 || static_cast<size_t>(id) >= oldToNewGid.size())
      continue;

    updated.insert(oldToNewGid[static_cast<size_t>(id)]);
  }

  hiddenGroupIds = std::move(updated);
}

/**
 * @brief Rewrites every layout:N widgetSettings entry to use the new groupId.
 */
void remapLayoutKeysAfterReorder(QJsonObject& widgetSettings, const std::vector<int>& oldToNewGid)
{
  if (widgetSettings.isEmpty())
    return;

  const QString prefix = QStringLiteral("layout:");
  QMap<int, QJsonObject> snapshot;

  for (const auto& key : widgetSettings.keys()) {
    if (!key.startsWith(prefix))
      continue;

    bool ok      = false;
    const int id = key.mid(prefix.length()).toInt(&ok);
    if (!ok || id < 0 || static_cast<size_t>(id) >= oldToNewGid.size())
      continue;

    snapshot.insert(id, widgetSettings.value(key).toObject());
    widgetSettings.remove(key);
  }

  for (auto it = snapshot.constBegin(); it != snapshot.constEnd(); ++it) {
    const int newId = oldToNewGid[static_cast<size_t>(it.key())];
    widgetSettings.insert(Keys::layoutKey(newId), it.value());
  }
}

/**
 * @brief Renames per-group auto workspaces (PerGroupStart + groupId) so they
 *        track their group across a reorder.
 */
void remapAutoWorkspaceIdsAfterReorder(std::vector<Workspace>& workspaces,
                                       const std::vector<int>& oldToNewGid)
{
  for (auto& ws : workspaces) {
    if (ws.workspaceId < WorkspaceIds::PerGroupStart || ws.workspaceId >= WorkspaceIds::UserStart)
      continue;

    const int oldGid = ws.workspaceId - WorkspaceIds::PerGroupStart;
    if (oldGid < 0 || static_cast<size_t>(oldGid) >= oldToNewGid.size())
      continue;

    ws.workspaceId = WorkspaceIds::PerGroupStart + oldToNewGid[static_cast<size_t>(oldGid)];
  }

  std::stable_sort(
    workspaces.begin(), workspaces.end(), [](const Workspace& a, const Workspace& b) {
      const bool aUser = a.workspaceId >= WorkspaceIds::UserStart;
      const bool bUser = b.workspaceId >= WorkspaceIds::UserStart;
      if (aUser != bUser)
        return !aUser;

      if (!aUser && !bUser)
        return a.workspaceId < b.workspaceId;

      return false;
    });
}

/**
 * @brief Snapshots one anchor per workspace ref before a reorder, keyed by workspaceId so
 *        the buckets survive any reordering of the workspace list between snapshot and resolve.
 */
RefAnchors snapshotRefAnchors(const std::vector<Workspace>& workspaces,
                              const std::vector<Group>& groups)
{
  RefAnchors out;
  out.reserve(static_cast<qsizetype>(workspaces.size()));
  for (const auto& ws : workspaces) {
    std::vector<RefAnchor> bucket;
    bucket.reserve(ws.widgetRefs.size());
    for (const auto& r : ws.widgetRefs)
      bucket.push_back(anchorRef(r, groups));

    out.insert(ws.workspaceId, std::move(bucket));
  }
  return out;
}

/**
 * @brief Re-resolves every workspace against its own snapshot bucket, paired by workspaceId
 *        rather than by list position.
 */
void resolveRefAnchors(std::vector<Workspace>& workspaces,
                       const RefAnchors& anchors,
                       const std::vector<Group>& groups)
{
  for (auto& ws : workspaces) {
    const auto it = anchors.constFind(ws.workspaceId);
    if (it == anchors.constEnd())
      continue;

    resolveOneWorkspaceRefs(ws, it.value(), groups);
  }
}

}  // namespace DataModel::WorkspaceRefs
