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
#include <QApplication>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHash>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSValue>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QTimer>

#include "AppInfo.h"
#include "AppState.h"
#include "DataModel/Editors/OutputCodeEditor.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/JsonValidator.h"
#include "Misc/PasswordHash.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "ProjectModelShared.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"

namespace DataModel {

namespace detail {

/**
 * @brief Layout config for fixed three-axis group widgets (Accel/Gyro/GPS/Plot3D).
 */
struct ThreeAxisLayout {
  const char* widgetTag;
  const char* axisWidgets[3];
  QString units[3];
  QString titles[3];
  double wgtMin[3];
  double wgtMax[3];
  bool plt;
};

}  // namespace detail

using detail::ThreeAxisLayout;

/**
 * @brief Populates a group with three canonical axis datasets per supplied layout.
 */
static void populateThreeAxisDatasets(DataModel::Group& grp,
                                      int baseIndex,
                                      const ThreeAxisLayout& layout)
{
  grp.widget = QString::fromUtf8(layout.widgetTag);

  DataModel::Dataset axes[3];
  for (int i = 0; i < 3; ++i) {
    axes[i].datasetId = i;
    axes[i].groupId   = grp.groupId;
    axes[i].sourceId  = grp.sourceId;
    axes[i].index     = baseIndex + i;
    axes[i].units     = layout.units[i];
    axes[i].widget    = QString::fromUtf8(layout.axisWidgets[i]);
    axes[i].title     = layout.titles[i];
    axes[i].wgtMin    = layout.wgtMin[i];
    axes[i].wgtMax    = layout.wgtMax[i];
    axes[i].plt       = layout.plt;

    grp.datasets.push_back(axes[i]);
  }
}

}  // namespace DataModel

/**
 * @brief Replaces the group at groupId and emits groupsChanged.
 */
void DataModel::ProjectModel::updateGroup(const int groupId,
                                          const DataModel::Group& group,
                                          const bool rebuildTree)
{
  const ProjectUndoScope undo_scope{*this, tr("Edit Group")};
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  m_groups[groupId] = group;

  if (rebuildTree)
    Q_EMIT groupsChanged();
  else
    Q_EMIT groupDataChanged();

  setModified(true);
}

/**
 * @brief Replaces the dataset at @p groupId/@p datasetId.
 */
void DataModel::ProjectModel::updateDataset(const int groupId,
                                            const int datasetId,
                                            const DataModel::Dataset& dataset,
                                            const bool rebuildTree)
{
  const ProjectUndoScope undo_scope{*this, tr("Edit Dataset")};
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  DataModel::Dataset resolved = dataset;
  if (resolved.transformLanguage < 0 && !resolved.transformCode.isEmpty()) {
    for (const auto& src : m_sources)
      if (src.sourceId == resolved.sourceId) {
        resolved.transformLanguage = src.frameParserLanguage == SerialStudio::Native
                                     ? static_cast<int>(SerialStudio::Lua)
                                     : src.frameParserLanguage;
        break;
      }

    if (resolved.transformLanguage < 0)
      resolved.transformLanguage = 0;
  }

  m_groups[groupId].datasets[datasetId] = resolved;
  m_selectedDataset                     = resolved;

  if (rebuildTree)
    Q_EMIT groupsChanged();

  m_runtimeDirty = true;
  setModified(true);
  scheduleAutoSave();
}

/**
 * @brief Returns @a base when unused, otherwise base with the lowest '-N' suffix (N>=2) not already
 *        in @a used; the try count is bounded by the used-set size so it always terminates.
 */
static QString uniqueAliasCandidate(const QString& base, const QSet<QString>& used)
{
  if (!used.contains(base))
    return base;

  const int maxTries = static_cast<int>(used.size()) + 2;
  for (int suffix = 2; suffix < maxTries + 2; ++suffix) {
    const QString candidate = QStringLiteral("%1-%2").arg(base, QString::number(suffix));
    if (!used.contains(candidate))
      return candidate;
  }

  return base;
}

/**
 * @brief Fills every empty dataset alias from its title (deduplicated), leaving existing aliases
 *        untouched; returns the number seeded. One modified state + autosave for the whole batch.
 */
int DataModel::ProjectModel::seedDatasetAliases()
{
  const ProjectUndoScope undo_scope{*this, tr("Seed Dataset Aliases")};
  QSet<QString> used;
  for (const auto& group : m_groups)
    for (const auto& ds : group.datasets)
      if (!ds.alias.isEmpty())
        used.insert(ds.alias);

  int seeded = 0;
  for (auto& group : m_groups) {
    for (auto& ds : group.datasets) {
      const QString base = ds.title.simplified();
      if (!ds.alias.isEmpty() || base.isEmpty())
        continue;

      ds.alias = uniqueAliasCandidate(base, used);
      used.insert(ds.alias);
      ++seeded;
    }
  }

  if (seeded > 0) {
    m_runtimeDirty = true;
    setModified(true);
    Q_EMIT groupsChanged();
    scheduleAutoSave();
  }

  return seeded;
}

/**
 * @brief Enables or disables a group; a disabled group is excluded from frame building while the
 *        editor still shows it greyed. Refreshes the runtime frame so the dashboard updates at
 * once.
 */
void DataModel::ProjectModel::setGroupEnabled(const int groupId, const bool enabled)
{
  const ProjectUndoScope undo_scope{*this, tr("Toggle Group")};
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (m_groups[groupId].enabled == enabled)
    return;

  m_groups[groupId].enabled = enabled;

  Q_EMIT groupsChanged();
  setModified(true);
  syncRuntime();
}

/**
 * @brief Enables or disables a single dataset; a disabled dataset is excluded from frame building
 *        while its siblings keep their explicit frame indices. Refreshes the runtime frame.
 */
void DataModel::ProjectModel::setDatasetEnabled(const int groupId,
                                                const int datasetId,
                                                const bool enabled)
{
  const ProjectUndoScope undo_scope{*this, tr("Toggle Dataset")};
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  if (m_groups[groupId].datasets[datasetId].enabled == enabled)
    return;

  m_groups[groupId].datasets[datasetId].enabled = enabled;

  Q_EMIT groupsChanged();
  setModified(true);
  syncRuntime();
}

/**
 * @brief Replaces the action at actionId and emits actionsChanged.
 */
void DataModel::ProjectModel::updateAction(const int actionId,
                                           const DataModel::Action& action,
                                           const bool rebuildTree)
{
  const ProjectUndoScope undo_scope{*this, tr("Edit Action")};
  if (actionId < 0 || static_cast<size_t>(actionId) >= m_actions.size())
    return;

  m_actions[actionId] = action;

  if (rebuildTree)
    Q_EMIT actionsChanged();

  setModified(true);
}

//--------------------------------------------------------------------------------------------------
// Group / dataset / action mutation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deletes the currently selected group after user confirmation.
 */
void DataModel::ProjectModel::deleteCurrentGroup()
{
  if (!m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete group \"%1\"?").arg(m_selectedGroup.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto gid = m_selectedGroup.groupId;
  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  const ProjectUndoScope undo_scope{*this, tr("Delete Group")};

  QMap<int, int> deletedTypeCounts;
  if (m_customizeWorkspaces)
    deletedTypeCounts = widgetTypeCountsForGroup(m_groups[gid]);

  m_groups.erase(m_groups.begin() + gid);

  int id = 0;
  for (auto g = m_groups.begin(); g != m_groups.end(); ++g, ++id) {
    g->groupId = id;
    for (auto d = g->datasets.begin(); d != g->datasets.end(); ++d)
      d->groupId = id;
  }

  if (m_customizeWorkspaces)
    shiftWorkspaceRefsAfterGroupDelete(gid, deletedTypeCounts);

  shiftHiddenGroupIdsAfterGroupDelete(gid);
  shiftLayoutKeysAfterGroupDelete(gid);

  Q_EMIT groupsChanged();
  Q_EMIT groupDeleted();
  setModified(true);
}

/**
 * @brief Deletes the currently selected action after user confirmation.
 */
void DataModel::ProjectModel::deleteCurrentAction()
{
  if (!m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete action \"%1\"?").arg(m_selectedAction.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto aid = m_selectedAction.actionId;
  if (aid < 0 || static_cast<size_t>(aid) >= m_actions.size())
    return;

  const ProjectUndoScope undo_scope{*this, tr("Delete Action")};

  m_actions.erase(m_actions.begin() + aid);

  int id = 0;
  for (auto a = m_actions.begin(); a != m_actions.end(); ++a, ++id)
    a->actionId = id;

  Q_EMIT actionsChanged();
  Q_EMIT actionDeleted();
  setModified(true);
}

/**
 * @brief Deletes the selected dataset, removing the group if it becomes empty.
 */
void DataModel::ProjectModel::deleteCurrentDataset()
{
  if (!m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete dataset \"%1\"?").arg(m_selectedDataset.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto groupId   = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;

  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  const ProjectUndoScope undo_scope{*this, tr("Delete Dataset")};

  QMap<int, int> deletedTypeCounts;
  if (m_customizeWorkspaces)
    deletedTypeCounts = widgetTypeCountsForGroup(m_groups[groupId]);

  QMap<int, int> datasetTypeCounts;
  if (m_customizeWorkspaces) {
    const auto& ds  = m_groups[groupId].datasets[datasetId];
    const auto keys = SerialStudio::getDashboardWidgets(ds);
    for (const auto& k : keys)
      if (SerialStudio::datasetWidgetEligibleForWorkspace(k))
        datasetTypeCounts[static_cast<int>(k)] += 1;
  }

  m_groups[groupId].datasets.erase(m_groups[groupId].datasets.begin() + datasetId);

  const auto& widgetId        = m_groups[groupId].widget;
  const bool widgetCanBeEmpty = (widgetId == QLatin1String("painter")
                                 || widgetId == QLatin1String("image") || widgetId.isEmpty());

  if (m_groups[groupId].datasets.empty() && !widgetCanBeEmpty) {
    m_groups.erase(m_groups.begin() + groupId);

    int id = 0;
    for (auto g = m_groups.begin(); g != m_groups.end(); ++g, ++id) {
      g->groupId = id;
      for (auto d = g->datasets.begin(); d != g->datasets.end(); ++d)
        d->groupId = id;
    }

    if (m_customizeWorkspaces)
      shiftWorkspaceRefsAfterGroupDelete(groupId, deletedTypeCounts);

    shiftHiddenGroupIdsAfterGroupDelete(groupId);
    shiftLayoutKeysAfterGroupDelete(groupId);

    Q_EMIT groupsChanged();
    Q_EMIT datasetDeleted(-1);
    setModified(true);
    return;
  }

  int id     = 0;
  auto begin = m_groups[groupId].datasets.begin();
  auto end   = m_groups[groupId].datasets.end();
  for (auto dataset = begin; dataset != end; ++dataset, ++id)
    dataset->datasetId = id;

  if (m_customizeWorkspaces)
    shiftWorkspaceRefsAfterDatasetDelete(groupId, datasetTypeCounts);

  Q_EMIT groupsChanged();
  Q_EMIT datasetDeleted(groupId);
  setModified(true);
}

/**
 * @brief Appends a copy of the currently selected group to the project.
 */
void DataModel::ProjectModel::duplicateCurrentGroup()
{
  const ProjectUndoScope undo_scope{*this, tr("Duplicate Group")};
  DataModel::Group group = m_selectedGroup;
  group.groupId          = m_groups.size();
  group.uniqueId         = allocateUniqueId();
  group.datasets.clear();
  group.outputWidgets.clear();

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(m_groups.size()));
  for (const auto& g : m_groups)
    existingTitles.append(g.title);

  group.title = nextDuplicateTitle(m_selectedGroup.title, existingTitles);

  for (size_t i = 0; i < m_selectedGroup.datasets.size(); ++i) {
    auto dataset     = m_selectedGroup.datasets[i];
    dataset.groupId  = group.groupId;
    dataset.index    = nextDatasetIndex() + static_cast<int>(i);
    dataset.uniqueId = allocateUniqueId();
    group.datasets.push_back(dataset);
  }

  for (const auto& ow : m_selectedGroup.outputWidgets) {
    auto copy    = ow;
    copy.groupId = group.groupId;
    group.outputWidgets.push_back(copy);
  }

  m_groups.push_back(group);
  m_selectedGroup = m_groups.back();

  Q_EMIT groupsChanged();
  Q_EMIT groupAdded(static_cast<int>(m_groups.size()) - 1);
  setModified(true);
}

/**
 * @brief Appends a copy of the currently selected action to the project.
 */
void DataModel::ProjectModel::duplicateCurrentAction()
{
  const ProjectUndoScope undo_scope{*this, tr("Duplicate Action")};
  DataModel::Action action;
  action.actionId             = m_actions.size();
  action.icon                 = m_selectedAction.icon;
  action.txData               = m_selectedAction.txData;
  action.timerMode            = m_selectedAction.timerMode;
  action.repeatCount          = m_selectedAction.repeatCount;
  action.eolSequence          = m_selectedAction.eolSequence;
  action.timerIntervalMs      = m_selectedAction.timerIntervalMs;
  action.autoExecuteOnConnect = m_selectedAction.autoExecuteOnConnect;

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(m_actions.size()));
  for (const auto& a : m_actions)
    existingTitles.append(a.title);

  action.title = nextDuplicateTitle(m_selectedAction.title, existingTitles);

  m_actions.push_back(action);
  m_selectedAction = action;

  Q_EMIT actionsChanged();
  Q_EMIT actionAdded(static_cast<int>(m_actions.size()) - 1);
  setModified(true);
}

namespace DataModel {

//--------------------------------------------------------------------------------------------------
// Group / dataset / workspace / action reorder
//--------------------------------------------------------------------------------------------------

namespace detail {
/**
 * @brief Stable anchor for a workspace ref across group/dataset reorders.
 */
struct RefAnchor {
  int widgetType;
  int sourceGid;
  int datasetFrameIndex;
  bool isGroupOrLed;
};
}  // namespace detail

/**
 * @brief Resolves a workspace ref into a stable RefAnchor before a reorder.
 */
static detail::RefAnchor anchorRef(const DataModel::WidgetRef& r,
                                   const std::vector<DataModel::Group>& groups)
{
  detail::RefAnchor a;
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
static int slotForAnchor(const detail::RefAnchor& a, const DataModel::Group& g)
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
 * @brief Snapshots one anchor per workspace ref before a reorder, keyed by workspaceId so
 *        the buckets survive any reordering of the workspace list between snapshot and resolve.
 */
static QHash<int, std::vector<detail::RefAnchor>> snapshotAllRefs(
  const std::vector<DataModel::Workspace>& workspaces, const std::vector<DataModel::Group>& groups)
{
  QHash<int, std::vector<detail::RefAnchor>> out;
  out.reserve(static_cast<qsizetype>(workspaces.size()));
  for (const auto& ws : workspaces) {
    std::vector<detail::RefAnchor> bucket;
    bucket.reserve(ws.widgetRefs.size());
    for (const auto& r : ws.widgetRefs)
      bucket.push_back(anchorRef(r, groups));

    out.insert(ws.workspaceId, std::move(bucket));
  }
  return out;
}

/**
 * @brief Walks one workspace's refs against the new group/dataset layout, refreshing
 *        the dataset slot. The group identity is uniqueId-based so it never needs remapping.
 */
static void resolveOneWorkspaceRefs(DataModel::Workspace& ws,
                                    const std::vector<detail::RefAnchor>& src,
                                    const std::vector<DataModel::Group>& groups)
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
 * @brief Re-resolves every workspace against its own snapshot bucket, paired by workspaceId
 *        rather than by list position.
 */
static void resolveAllWorkspaceRefs(std::vector<DataModel::Workspace>& workspaces,
                                    const QHash<int, std::vector<detail::RefAnchor>>& anchors,
                                    const std::vector<DataModel::Group>& groups)
{
  for (auto& ws : workspaces) {
    const auto it = anchors.constFind(ws.workspaceId);
    if (it == anchors.constEnd())
      continue;

    resolveOneWorkspaceRefs(ws, it.value(), groups);
  }
}

}  // namespace DataModel

/**
 * @brief Moves a group from one position to another, preserving widget settings,
 *        workspace refs, hidden state, and auto-workspace IDs across the shift. Refs are
 *        re-resolved before the auto-workspace IDs are renumbered, because the anchor
 *        snapshot is keyed by the workspace IDs that renumbering replaces.
 */
void DataModel::ProjectModel::moveGroup(int fromGroupId, int toGroupId)
{
  const ProjectUndoScope undo_scope{*this, tr("Move Group")};
  const int n = static_cast<int>(m_groups.size());
  if (fromGroupId < 0 || fromGroupId >= n)
    return;

  const int target = std::clamp(toGroupId, 0, n - 1);
  if (target == fromGroupId)
    return;

  std::vector<int> oldToNewGid(static_cast<size_t>(n));
  for (int i = 0; i < n; ++i)
    oldToNewGid[static_cast<size_t>(i)] = i;

  if (fromGroupId < target)
    for (int i = fromGroupId + 1; i <= target; ++i)
      oldToNewGid[static_cast<size_t>(i)] = i - 1;

  else
    for (int i = target; i < fromGroupId; ++i)
      oldToNewGid[static_cast<size_t>(i)] = i + 1;

  oldToNewGid[static_cast<size_t>(fromGroupId)] = target;

  QHash<int, std::vector<detail::RefAnchor>> anchors;
  if (m_customizeWorkspaces)
    anchors = snapshotAllRefs(m_workspaces, m_groups);

  auto group = m_groups[fromGroupId];
  m_groups.erase(m_groups.begin() + fromGroupId);
  m_groups.insert(m_groups.begin() + target, group);

  remapGroupIdsAfterReorder(oldToNewGid);

  remapLayoutKeysAfterReorder(oldToNewGid);
  remapHiddenGroupIdsAfterReorder(oldToNewGid);

  if (m_customizeWorkspaces)
    resolveAllWorkspaceRefs(m_workspaces, anchors, m_groups);

  remapAutoWorkspaceIdsAfterReorder(oldToNewGid);

  if (m_selectedGroup.groupId == fromGroupId)
    m_selectedGroup = m_groups[target];

  Q_EMIT groupsChanged();
  Q_EMIT widgetSettingsChanged();
  setModified(true);
}

/**
 * @brief Moves a dataset within its group, renumbering datasetIds and re-resolving
 *        workspace refs in customise mode.
 */
void DataModel::ProjectModel::moveDataset(int groupId, int fromDatasetId, int toDatasetId)
{
  const ProjectUndoScope undo_scope{*this, tr("Move Dataset")};
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  auto& datasets = m_groups[groupId].datasets;
  const int n    = static_cast<int>(datasets.size());
  if (fromDatasetId < 0 || fromDatasetId >= n)
    return;

  const int target = std::clamp(toDatasetId, 0, n - 1);
  if (target == fromDatasetId)
    return;

  QHash<int, std::vector<detail::RefAnchor>> anchors;
  if (m_customizeWorkspaces)
    anchors = snapshotAllRefs(m_workspaces, m_groups);

  auto dataset = datasets[fromDatasetId];
  datasets.erase(datasets.begin() + fromDatasetId);
  datasets.insert(datasets.begin() + target, dataset);

  for (size_t i = 0; i < datasets.size(); ++i)
    datasets[i].datasetId = static_cast<int>(i);

  if (m_customizeWorkspaces)
    resolveAllWorkspaceRefs(m_workspaces, anchors, m_groups);

  if (m_selectedDataset.groupId == groupId && m_selectedDataset.datasetId == fromDatasetId)
    m_selectedDataset = datasets[target];

  Q_EMIT groupsChanged();
  setModified(true);
}

/**
 * @brief Moves a workspace to the given index in the editor list.
 *        Auto workspaces (id < UserStart) are pinned to the top and ignored.
 */
void DataModel::ProjectModel::moveWorkspace(int workspaceId, int targetIndex)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (workspaceId < WorkspaceIds::UserStart)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  auto it = std::find_if(m_workspaces.begin(), m_workspaces.end(), [workspaceId](const auto& ws) {
    return ws.workspaceId == workspaceId;
  });

  if (it == m_workspaces.end())
    return;

  const int from = static_cast<int>(std::distance(m_workspaces.begin(), it));

  int firstUserSlot = 0;
  for (const auto& ws : m_workspaces) {
    if (ws.workspaceId >= WorkspaceIds::UserStart)
      break;

    firstUserSlot += 1;
  }

  const int last   = static_cast<int>(m_workspaces.size()) - 1;
  const int target = std::clamp(targetIndex, firstUserSlot, last);
  if (target == from)
    return;

  auto ws = *it;
  m_workspaces.erase(it);
  m_workspaces.insert(m_workspaces.begin() + target, ws);

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Moves an action within the project actions list, renumbering actionIds.
 */
void DataModel::ProjectModel::moveAction(int fromActionId, int toActionId)
{
  const ProjectUndoScope undo_scope{*this, tr("Move Action")};
  const int n = static_cast<int>(m_actions.size());
  if (fromActionId < 0 || fromActionId >= n)
    return;

  const int target = std::clamp(toActionId, 0, n - 1);
  if (target == fromActionId)
    return;

  auto action = m_actions[fromActionId];
  m_actions.erase(m_actions.begin() + fromActionId);
  m_actions.insert(m_actions.begin() + target, action);

  for (size_t i = 0; i < m_actions.size(); ++i)
    m_actions[i].actionId = static_cast<int>(i);

  if (m_selectedAction.actionId == fromActionId)
    m_selectedAction = m_actions[target];

  Q_EMIT actionsChanged();
  setModified(true);
}

/**
 * @brief Moves an output widget within its group's outputWidgets list.
 */
void DataModel::ProjectModel::moveOutputWidget(int groupId, int fromWidgetId, int toWidgetId)
{
  const ProjectUndoScope undo_scope{*this, tr("Move Output Widget")};
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  auto& widgets = m_groups[groupId].outputWidgets;
  const int n   = static_cast<int>(widgets.size());
  if (fromWidgetId < 0 || fromWidgetId >= n)
    return;

  const int target = std::clamp(toWidgetId, 0, n - 1);
  if (target == fromWidgetId)
    return;

  auto widget = widgets[fromWidgetId];
  widgets.erase(widgets.begin() + fromWidgetId);
  widgets.insert(widgets.begin() + target, widget);

  for (size_t i = 0; i < widgets.size(); ++i)
    widgets[i].widgetId = static_cast<int>(i);

  if (m_selectedOutputWidget.groupId == groupId && m_selectedOutputWidget.widgetId == fromWidgetId)
    m_selectedOutputWidget = widgets[target];

  Q_EMIT groupsChanged();
  setModified(true);
}

/**
 * @brief Renumbers Group::groupId (and child Dataset::groupId) to match the new vector order.
 */
void DataModel::ProjectModel::remapGroupIdsAfterReorder(const std::vector<int>& oldToNewGid)
{
  SS_ASSERT_LOG(oldToNewGid.size() == m_groups.size());

  for (size_t i = 0; i < m_groups.size(); ++i) {
    auto& g   = m_groups[i];
    g.groupId = static_cast<int>(i);
    for (auto& d : g.datasets)
      d.groupId = g.groupId;
  }

  Q_UNUSED(oldToNewGid);
}

/**
 * @brief Updates m_hiddenGroupIds so each hidden ID follows its renamed group.
 */
void DataModel::ProjectModel::remapHiddenGroupIdsAfterReorder(const std::vector<int>& oldToNewGid)
{
  if (m_hiddenGroupIds.isEmpty())
    return;

  QSet<int> updated;
  for (const int id : std::as_const(m_hiddenGroupIds)) {
    if (id < 0 || static_cast<size_t>(id) >= oldToNewGid.size())
      continue;

    updated.insert(oldToNewGid[static_cast<size_t>(id)]);
  }

  m_hiddenGroupIds = std::move(updated);
}

/**
 * @brief Rewrites every layout:N widgetSettings entry to use the new groupId.
 */
void DataModel::ProjectModel::remapLayoutKeysAfterReorder(const std::vector<int>& oldToNewGid)
{
  if (m_widgetSettings.isEmpty())
    return;

  const QString prefix = QStringLiteral("layout:");
  QMap<int, QJsonObject> snapshot;

  for (const auto& key : m_widgetSettings.keys()) {
    if (!key.startsWith(prefix))
      continue;

    bool ok      = false;
    const int id = key.mid(prefix.length()).toInt(&ok);
    if (!ok || id < 0 || static_cast<size_t>(id) >= oldToNewGid.size())
      continue;

    snapshot.insert(id, m_widgetSettings.value(key).toObject());
    m_widgetSettings.remove(key);
  }

  for (auto it = snapshot.constBegin(); it != snapshot.constEnd(); ++it) {
    const int newId = oldToNewGid[static_cast<size_t>(it.key())];
    m_widgetSettings.insert(Keys::layoutKey(newId), it.value());
  }
}

/**
 * @brief Renames per-group auto workspaces (PerGroupStart + groupId) so they
 *        track their group across a reorder.
 */
void DataModel::ProjectModel::remapAutoWorkspaceIdsAfterReorder(const std::vector<int>& oldToNewGid)
{
  for (auto& ws : m_workspaces) {
    if (ws.workspaceId < WorkspaceIds::PerGroupStart || ws.workspaceId >= WorkspaceIds::UserStart)
      continue;

    const int oldGid = ws.workspaceId - WorkspaceIds::PerGroupStart;
    if (oldGid < 0 || static_cast<size_t>(oldGid) >= oldToNewGid.size())
      continue;

    ws.workspaceId = WorkspaceIds::PerGroupStart + oldToNewGid[static_cast<size_t>(oldGid)];
  }

  std::stable_sort(
    m_workspaces.begin(), m_workspaces.end(), [](const Workspace& a, const Workspace& b) {
      const bool aUser = a.workspaceId >= WorkspaceIds::UserStart;
      const bool bUser = b.workspaceId >= WorkspaceIds::UserStart;
      if (aUser != bUser)
        return !aUser;

      if (!aUser && !bUser)
        return a.workspaceId < b.workspaceId;

      return false;
    });
}

//--------------------------------------------------------------------------------------------------
// Output widget CRUD
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the editor's currently selected output widget.
 */
void DataModel::ProjectModel::setSelectedOutputWidget(const DataModel::OutputWidget& widget)
{
  m_selectedOutputWidget = widget;
}

/**
 * @brief Changes the type of the currently selected output widget.
 */
void DataModel::ProjectModel::setOutputWidgetType(int type)
{
  const ProjectUndoScope undo_scope{*this, tr("Change Output Widget Type")};
  const auto gid = m_selectedOutputWidget.groupId;
  const auto wid = m_selectedOutputWidget.widgetId;

  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  auto& widgets = m_groups[gid].outputWidgets;
  if (wid < 0 || static_cast<size_t>(wid) >= widgets.size())
    return;

  const auto newType = static_cast<DataModel::OutputWidgetType>(
    qBound(0, type, static_cast<int>(DataModel::OutputWidgetType::Knob)));

  if (widgets[wid].type == newType)
    return;

  widgets[wid].type           = newType;
  m_selectedOutputWidget.type = newType;

  Q_EMIT groupsChanged();
  Q_EMIT outputWidgetAdded(gid, wid);
  setModified(true);
}

/**
 * @brief Sets the icon of the currently selected output widget.
 */
void DataModel::ProjectModel::setOutputWidgetIcon(const QString& icon)
{
  const ProjectUndoScope undo_scope{*this, tr("Change Output Widget Icon")};
  const auto gid = m_selectedOutputWidget.groupId;
  const auto wid = m_selectedOutputWidget.widgetId;

  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  auto& widgets = m_groups[gid].outputWidgets;
  if (wid < 0 || static_cast<size_t>(wid) >= widgets.size())
    return;

  widgets[wid].icon           = icon;
  m_selectedOutputWidget.icon = icon;

  Q_EMIT groupDataChanged();
  Q_EMIT outputWidgetAdded(gid, wid);
  setModified(true);
}

/**
 * @brief Creates a new output group with a default button control.
 */
void DataModel::ProjectModel::addOutputPanel(int sourceId)
{
  const ProjectUndoScope undo_scope{*this, tr("Add Output Panel")};
  addGroup(tr("Output Controls"), SerialStudio::NoGroupWidget, sourceId);
  auto& group     = m_groups.back();
  group.groupType = DataModel::GroupType::Output;
  m_selectedGroup = group;

  addOutputControl(SerialStudio::OutputButton, sourceId);
}

/**
 * @brief Adds an output control, creating a new output group if needed.
 */
void DataModel::ProjectModel::addOutputControl(const SerialStudio::OutputWidgetType type,
                                               int sourceId)
{
  const ProjectUndoScope undo_scope{*this, tr("Add Output Widget")};
  int groupId    = -1;
  const auto sel = m_selectedGroup.groupId;
  if (sel >= 0 && static_cast<size_t>(sel) < m_groups.size()
      && m_groups[sel].groupType == DataModel::GroupType::Output
      && (sourceId < 0 || m_groups[sel].sourceId == sourceId))
    groupId = sel;

  if (groupId < 0) {
    for (const auto& g : std::as_const(m_groups)) {
      if (g.groupType != DataModel::GroupType::Output)
        continue;

      if (sourceId >= 0 && g.sourceId != sourceId)
        continue;

      groupId         = g.groupId;
      m_selectedGroup = g;
      break;
    }
  }

  if (groupId < 0) {
    addGroup(tr("Output Controls"), SerialStudio::NoGroupWidget, sourceId);
    auto& group     = m_groups.back();
    group.groupType = DataModel::GroupType::Output;
    groupId         = group.groupId;
    m_selectedGroup = group;
  }

  auto& group = m_groups[groupId];

  QString title;
  switch (type) {
    case SerialStudio::OutputButton:
      title = tr("New Button");
      break;
    case SerialStudio::OutputSlider:
      title = tr("New Slider");
      break;
    case SerialStudio::OutputToggle:
      title = tr("New Toggle");
      break;
    case SerialStudio::OutputTextField:
      title = tr("New Text Field");
      break;
    case SerialStudio::OutputKnob:
      title = tr("New Knob");
      break;
  }

  DataModel::OutputWidget ow;
  ow.widgetId         = static_cast<int>(group.outputWidgets.size());
  ow.groupId          = groupId;
  ow.sourceId         = group.sourceId;
  ow.title            = title;
  ow.type             = static_cast<DataModel::OutputWidgetType>(type);
  ow.transmitFunction = DataModel::OutputCodeEditor::defaultTemplate();

  group.outputWidgets.push_back(ow);
  m_selectedOutputWidget = ow;

  Q_EMIT groupsChanged();
  Q_EMIT outputWidgetAdded(groupId, ow.widgetId);
  setModified(true);
}

/**
 * @brief Deletes the currently selected output widget after confirmation.
 */
void DataModel::ProjectModel::deleteCurrentOutputWidget()
{
  if (!m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete output widget \"%1\"?").arg(m_selectedOutputWidget.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto gid = m_selectedOutputWidget.groupId;
  const auto wid = m_selectedOutputWidget.widgetId;

  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  auto& widgets = m_groups[gid].outputWidgets;
  if (wid < 0 || static_cast<size_t>(wid) >= widgets.size())
    return;

  const ProjectUndoScope undo_scope{*this, tr("Delete Output Widget")};
  QMap<int, int> deletedTypeCounts;
  if (m_customizeWorkspaces)
    deletedTypeCounts = widgetTypeCountsForGroup(m_groups[gid]);

  widgets.erase(widgets.begin() + wid);

  if (widgets.empty()) {
    m_groups.erase(m_groups.begin() + gid);

    int id = 0;
    for (auto g = m_groups.begin(); g != m_groups.end(); ++g, ++id) {
      g->groupId = id;
      for (auto d = g->datasets.begin(); d != g->datasets.end(); ++d)
        d->groupId = id;
    }

    if (m_customizeWorkspaces)
      shiftWorkspaceRefsAfterGroupDelete(gid, deletedTypeCounts);

    shiftHiddenGroupIdsAfterGroupDelete(gid);
    shiftLayoutKeysAfterGroupDelete(gid);

    Q_EMIT groupsChanged();
    Q_EMIT groupDeleted();
    setModified(true);
    return;
  }

  for (int i = 0; i < static_cast<int>(widgets.size()); ++i)
    widgets[i].widgetId = i;

  Q_EMIT groupsChanged();
  Q_EMIT outputWidgetDeleted(gid);
  setModified(true);
}

/**
 * @brief Duplicates the currently selected output widget.
 */
void DataModel::ProjectModel::duplicateCurrentOutputWidget()
{
  const ProjectUndoScope undo_scope{*this, tr("Duplicate Output Widget")};
  const auto gid = m_selectedOutputWidget.groupId;
  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  auto& widgets = m_groups[gid].outputWidgets;

  DataModel::OutputWidget ow = m_selectedOutputWidget;
  ow.widgetId                = static_cast<int>(widgets.size());

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(widgets.size()));
  for (const auto& w : widgets)
    existingTitles.append(w.title);

  ow.title = nextDuplicateTitle(m_selectedOutputWidget.title, existingTitles);

  widgets.push_back(ow);
  m_selectedOutputWidget = ow;

  Q_EMIT groupsChanged();
  Q_EMIT outputWidgetAdded(gid, ow.widgetId);
  setModified(true);
}

/**
 * @brief Updates an output widget in place.
 */
void DataModel::ProjectModel::updateOutputWidget(int groupId,
                                                 int widgetId,
                                                 const DataModel::OutputWidget& widget,
                                                 bool rebuildTree)
{
  const ProjectUndoScope undo_scope{*this, tr("Edit Output Widget")};
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  auto& widgets = m_groups[groupId].outputWidgets;
  if (widgetId < 0 || static_cast<size_t>(widgetId) >= widgets.size())
    return;

  widgets[widgetId] = widget;

  if (rebuildTree)
    Q_EMIT groupsChanged();
  else
    Q_EMIT groupDataChanged();

  setModified(true);
}

/**
 * @brief Appends a copy of the currently selected dataset to its parent group.
 */
void DataModel::ProjectModel::duplicateCurrentDataset()
{
  const ProjectUndoScope undo_scope{*this, tr("Duplicate Dataset")};
  auto dataset = m_selectedDataset;

  if (dataset.groupId < 0 || static_cast<size_t>(dataset.groupId) >= m_groups.size())
    return;

  dataset.index     = nextDatasetIndex();
  dataset.datasetId = m_groups[dataset.groupId].datasets.size();
  dataset.uniqueId  = allocateUniqueId();

  const auto& siblings = m_groups[dataset.groupId].datasets;
  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(siblings.size()));
  for (const auto& d : siblings)
    existingTitles.append(d.title);

  dataset.title = nextDuplicateTitle(m_selectedDataset.title, existingTitles);

  m_groups[dataset.groupId].datasets.push_back(dataset);
  m_selectedDataset = dataset;

  Q_EMIT groupsChanged();
  Q_EMIT datasetAdded(dataset.groupId,
                      static_cast<int>(m_groups[dataset.groupId].datasets.size()) - 1);
  setModified(true);
}

/**
 * @brief Ensures a compatible group is selected before adding a dataset; honors sourceId scoping.
 */
void DataModel::ProjectModel::ensureValidGroup(int sourceId)
{
  const ProjectUndoScope undo_scope{*this, tr("Add Group")};
  const auto isValidGroup = [sourceId](const DataModel::Group& g) -> bool {
    if (g.groupType == DataModel::GroupType::Output)
      return false;

    if (sourceId >= 0 && g.sourceId != sourceId)
      return false;

    switch (SerialStudio::groupWidgetFromId(g.widget)) {
      case SerialStudio::MultiPlot:
      case SerialStudio::DataGrid:
      case SerialStudio::BarPanel:
      case SerialStudio::NoGroupWidget:
        return true;
      default:
        return false;
    }
  };

  const auto selId      = m_selectedGroup.groupId;
  const bool selInRange = selId >= 0 && static_cast<size_t>(selId) < m_groups.size();

  if (selInRange && isValidGroup(m_groups[selId])) {
    m_selectedGroup = m_groups[selId];
    return;
  }

  for (const auto& group : std::as_const(m_groups)) {
    if (!isValidGroup(group))
      continue;

    m_selectedGroup = group;
    return;
  }

  addGroup(tr("Group"), SerialStudio::NoGroupWidget, sourceId);
  m_selectedGroup = m_groups.back();
}

/**
 * @brief Adds a new dataset of the given type to the selected group.
 */
void DataModel::ProjectModel::addDataset(const SerialStudio::DatasetOption option, int sourceId)
{
  const ProjectUndoScope undo_scope{*this, tr("Add Dataset")};
  ensureValidGroup(sourceId);

  const auto groupId = m_selectedGroup.groupId;
  DataModel::Dataset dataset;
  dataset.groupId  = groupId;
  dataset.sourceId = m_groups[groupId].sourceId;

  QString title;
  switch (option) {
    case SerialStudio::DatasetGeneric:
      title = tr("New Dataset");
      break;
    case SerialStudio::DatasetPlot:
      title       = tr("New Plot");
      dataset.plt = true;
      break;
    case SerialStudio::DatasetFFT:
      title       = tr("New FFT Plot");
      dataset.fft = true;
      break;
    case SerialStudio::DatasetBar:
      title          = tr("New Level Indicator");
      dataset.widget = QStringLiteral("bar");
      break;
    case SerialStudio::DatasetGauge:
      title          = tr("New Gauge");
      dataset.widget = QStringLiteral("gauge");
      break;
    case SerialStudio::DatasetCompass:
      title          = tr("New Compass");
      dataset.wgtMin = 0;
      dataset.wgtMax = 360;
      dataset.widget = QStringLiteral("compass");
      break;
    case SerialStudio::DatasetMeter:
      title          = tr("New Meter");
      dataset.widget = QStringLiteral("meter");
      break;
    case SerialStudio::DatasetLED:
      title       = tr("New LED Indicator");
      dataset.led = true;
      break;
    case SerialStudio::DatasetWaterfall:
      title             = tr("New Waterfall");
      dataset.waterfall = true;
      break;
    default:
      break;
  }

  int count        = 1;
  QString newTitle = title;
  for (const auto& d : std::as_const(m_groups[groupId].datasets)) {
    if (d.title == newTitle) {
      count++;
      newTitle = QString("%1 (%2)").arg(title, QString::number(count));
    }
  }

  while (count > 1) {
    bool titleExists = false;
    for (const auto& d : std::as_const(m_groups[groupId].datasets)) {
      if (d.title != newTitle)
        continue;

      count++;
      newTitle    = QString("%1 (%2)").arg(title, QString::number(count));
      titleExists = true;
      break;
    }

    if (!titleExists)
      break;
  }

  dataset.title     = newTitle;
  dataset.index     = nextDatasetIndex();
  dataset.datasetId = m_groups[groupId].datasets.size();
  dataset.uniqueId  = allocateUniqueId();

  m_groups[groupId].datasets.push_back(dataset);
  m_selectedDataset = dataset;

  Q_EMIT groupsChanged();
  Q_EMIT datasetAdded(groupId, static_cast<int>(m_groups[groupId].datasets.size()) - 1);
  setModified(true);
}

/**
 * @brief Appends template-defined datasets to a painter group when the group has fewer than the
 * spec demands. Existing datasets are preserved.
 */
void DataModel::ProjectModel::ensurePainterDatasets(int groupId, const QVariantList& specs)
{
  const ProjectUndoScope undo_scope{*this, tr("Edit Canvas Datasets")};
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (specs.isEmpty())
    return;

  auto& grp          = m_groups[groupId];
  const int existing = static_cast<int>(grp.datasets.size());
  bool changed       = false;

  for (int i = existing; i < specs.size(); ++i) {
    const auto map = specs.at(i).toMap();
    DataModel::Dataset ds;
    ds.groupId   = groupId;
    ds.sourceId  = grp.sourceId;
    ds.datasetId = static_cast<int>(grp.datasets.size());
    ds.index     = nextDatasetIndex();
    ds.uniqueId  = allocateUniqueId();
    ds.title     = map.value(QStringLiteral("title"), tr("Channel %1").arg(i + 1)).toString();
    ds.units     = map.value(QStringLiteral("units")).toString();
    ds.wgtMin    = SerialStudio::toDouble(map.value(QStringLiteral("min"), 0.0));
    ds.wgtMax    = SerialStudio::toDouble(map.value(QStringLiteral("max"), 100.0));
    grp.datasets.push_back(std::move(ds));
    changed = true;
  }

  if (changed) {
    m_selectedGroup = grp;
    Q_EMIT groupsChanged();
    setModified(true);
  }
}

/**
 * @brief Toggles a dataset option flag on the currently selected dataset.
 */
void DataModel::ProjectModel::changeDatasetOption(const SerialStudio::DatasetOption option,
                                                  const bool checked)
{
  const ProjectUndoScope undo_scope{*this, tr("Change Dataset Option")};
  switch (option) {
    case SerialStudio::DatasetPlot:
      m_selectedDataset.plt = checked;
      break;
    case SerialStudio::DatasetFFT:
      m_selectedDataset.fft = checked;
      break;
    case SerialStudio::DatasetBar:
      m_selectedDataset.widget = checked ? QStringLiteral("bar") : "";
      break;
    case SerialStudio::DatasetGauge:
      m_selectedDataset.widget = checked ? QStringLiteral("gauge") : "";
      break;
    case SerialStudio::DatasetCompass:
      m_selectedDataset.widget = checked ? QStringLiteral("compass") : "";
      break;
    case SerialStudio::DatasetMeter:
      m_selectedDataset.widget = checked ? QStringLiteral("meter") : "";
      break;
    case SerialStudio::DatasetLED:
      m_selectedDataset.led = checked;
      break;
    case SerialStudio::DatasetWaterfall:
      m_selectedDataset.waterfall = checked;
      break;
    default:
      break;
  }

  const auto groupId   = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;

  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  m_groups[groupId].datasets[datasetId] = m_selectedDataset;

  Q_EMIT groupsChanged();
  setModified(true);
}

/**
 * @brief Adds a new action with a unique title to the project.
 */
void DataModel::ProjectModel::addAction(int sourceId)
{
  const ProjectUndoScope undo_scope{*this, tr("Add Action")};
  int count     = 1;
  QString title = tr("New Action");
  for (const auto& action : std::as_const(m_actions)) {
    if (action.title == title) {
      count++;
      title = QString("%1 (%2)").arg(title, QString::number(count));
    }
  }

  while (count > 1) {
    bool titleExists = false;
    for (const auto& action : std::as_const(m_actions)) {
      if (action.title != title)
        continue;

      count++;
      title       = QString("%1 (%2)").arg(title, QString::number(count));
      titleExists = true;
      break;
    }

    if (!titleExists)
      break;
  }

  DataModel::Action action;
  action.title    = title;
  action.actionId = m_actions.size();
  if (sourceId >= 0)
    action.sourceId = sourceId;

  m_actions.push_back(action);
  m_selectedAction = action;

  Q_EMIT actionsChanged();
  Q_EMIT actionAdded(static_cast<int>(m_actions.size()) - 1);
  setModified(true);
}

/**
 * @brief Adds a new group with a unique title and the given widget type.
 */
void DataModel::ProjectModel::addGroup(const QString& title,
                                       const SerialStudio::GroupWidget widget,
                                       int sourceId,
                                       int parentFolderId)
{
  const ProjectUndoScope undo_scope{*this, tr("Add Group")};
  int count        = 1;
  QString newTitle = title;
  for (const auto& group : std::as_const(m_groups)) {
    if (group.title == newTitle) {
      count++;
      newTitle = QString("%1 (%2)").arg(title, QString::number(count));
    }
  }

  while (count > 1) {
    bool titleExists = false;
    for (const auto& group : std::as_const(m_groups)) {
      if (group.title != newTitle)
        continue;

      count++;
      newTitle    = QString("%1 (%2)").arg(title, QString::number(count));
      titleExists = true;
      break;
    }

    if (!titleExists)
      break;
  }

  DataModel::Group group;
  group.title    = newTitle;
  group.groupId  = m_groups.size();
  group.uniqueId = allocateUniqueId();
  group.parentFolderId =
    (parentFolderId != -1 && folderExists(m_groupFolders, parentFolderId)) ? parentFolderId : -1;

  if (sourceId >= 0)
    group.sourceId = sourceId;

  m_groups.push_back(group);
  setGroupWidget(static_cast<int>(m_groups.size()) - 1, widget);
  m_selectedGroup = m_groups.back();

  Q_EMIT groupAdded(static_cast<int>(m_groups.size()) - 1);
  setModified(true);
}

/**
 * @brief Assigns a widget type to the group, replacing fixed-layout datasets.
 */
bool DataModel::ProjectModel::setGroupWidget(const int group,
                                             const SerialStudio::GroupWidget widget)
{
  const ProjectUndoScope undo_scope{*this, tr("Change Group Widget")};
  if (group < 0 || group >= static_cast<int>(m_groups.size())) [[unlikely]]
    return false;

  auto& grp = m_groups[group];

  if (!confirmGroupWidgetChange(grp, widget))
    return false;

  if (!applyGroupWidget(grp, widget))
    return false;

  for (auto& d : grp.datasets)
    if (d.uniqueId < 0)
      d.uniqueId = allocateUniqueId();

  m_groups[group] = grp;

  Q_EMIT groupsChanged();
  setModified(true);
  return true;
}

/**
 * @brief Confirms a destructive group widget change and clears existing datasets if needed.
 */
bool DataModel::ProjectModel::confirmGroupWidgetChange(DataModel::Group& grp,
                                                       SerialStudio::GroupWidget widget)
{
  if (grp.datasets.empty())
    return true;

  if (widget == SerialStudio::Painter) {
    grp.widget = "painter";
    return true;
  }

  const bool compatibleTarget =
    (widget == SerialStudio::DataGrid || widget == SerialStudio::MultiPlot
     || widget == SerialStudio::BarPanel || widget == SerialStudio::NoGroupWidget);
  const bool compatibleSource =
    (grp.widget == "multiplot" || grp.widget == "datagrid" || grp.widget == "barpanel"
     || grp.widget == "painter" || grp.widget == "");
  if (compatibleTarget && compatibleSource) {
    grp.widget = "";
    return true;
  }

  auto ret = Misc::Utilities::showMessageBox(tr("Are you sure you want to change the group-level "
                                                "widget?"),
                                             tr("Existing datasets for this group are deleted"),
                                             QMessageBox::Question,
                                             APP_NAME,
                                             QMessageBox::Yes | QMessageBox::No);

  if (ret == QMessageBox::No)
    return false;

  grp.datasets.clear();
  return true;
}

/**
 * @brief Assigns a group widget tag and any canonical datasets for fixed-layout types.
 */
bool DataModel::ProjectModel::applyGroupWidget(DataModel::Group& grp,
                                               SerialStudio::GroupWidget widget)
{
  if (widget == SerialStudio::NoGroupWidget) {
    grp.widget = "";
    return true;
  }

  if (widget == SerialStudio::DataGrid) {
    grp.widget = "datagrid";
    return true;
  }

  if (widget == SerialStudio::BarPanel) {
    grp.widget = "barpanel";
    return true;
  }

  if (widget == SerialStudio::MultiPlot) {
    grp.widget = "multiplot";
    return true;
  }

  if (widget == SerialStudio::ImageView) {
    grp.widget = "image";
    return true;
  }

  if (widget == SerialStudio::WebView) {
    grp.widget = "webview";
    return true;
  }

  if (widget == SerialStudio::Painter) {
    grp.widget = "painter";
    if (grp.datasets.empty())
      return populateFixedLayoutGroup(grp, widget);

    return true;
  }

  return populateFixedLayoutGroup(grp, widget);
}

/**
 * @brief Fills a group with the three-axis canonical datasets for sensor-style widgets.
 */
bool DataModel::ProjectModel::populateFixedLayoutGroup(DataModel::Group& grp,
                                                       SerialStudio::GroupWidget widget)
{
  const int baseIndex = nextDatasetIndex();

  if (widget == SerialStudio::Accelerometer) {
    // code-verify off
    ThreeAxisLayout layout{
      "accelerometer",
      {                            "x","y",                    "z"                                                },
      {                         "m/s²", "m/s²",                         "m/s²"},
      {tr("Accelerometer %1").arg("X"),
        tr("Accelerometer %1").arg("Y"),
        tr("Accelerometer %1").arg("Z")                                        },
      {                              0,      0,                              0},
      {                              0,      0,                              0},
      true
    };
    // code-verify on
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  if (widget == SerialStudio::Gyroscope) {
    ThreeAxisLayout layout{
      "gyro",
      {                   "x",                    "y",                    "z"},
      {               "deg/s",                "deg/s",                "deg/s"},
      {tr("Gyro %1").arg("X"), tr("Gyro %1").arg("Y"), tr("Gyro %1").arg("Z")},
      {                     0,                      0,                      0},
      {                     0,                      0,                      0},
      true
    };
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  if (widget == SerialStudio::GPS) {
    // code-verify off
    ThreeAxisLayout layout{
      "map",
      {         "lat",           "lon",          "alt"},
      {           "°",             "°",            "m"},
      {tr("Latitude"), tr("Longitude"), tr("Altitude")},
      {         -90.0,          -180.0,         -500.0},
      {          90.0,           180.0,      1000000.0},
      false
    };
    // code-verify on
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  if (widget == SerialStudio::Plot3D) {
    ThreeAxisLayout layout{
      "plot3d",
      {    "x",     "y",     "z"},
      {     "",      "",      ""},
      {tr("X"), tr("Y"), tr("Z")},
      {      0,       0,       0},
      {      0,       0,       0},
      false
    };
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  if (widget == SerialStudio::Painter) {
    ThreeAxisLayout layout{
      "painter",
      {     "",      "",      ""},
      {     "",      "",      ""},
      {tr("X"), tr("Y"), tr("Z")},
      {   -100,    -100,    -100},
      {    100,     100,     100},
      false
    };
    populateThreeAxisDatasets(grp, baseIndex, layout);
    return true;
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Stateless id-based mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deletes the group at @p groupId; opt-in confirmation reuses deleteCurrentGroup's dialog.
 */
void DataModel::ProjectModel::deleteGroup(int groupId, bool confirm)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  const auto previousSelection = m_selectedGroup;
  const auto groupCountBefore  = m_groups.size();
  setSelectedGroup(m_groups[groupId]);

  const bool previousSuppress = m_suppressMessageBoxes;
  m_suppressMessageBoxes      = !confirm;
  deleteCurrentGroup();
  m_suppressMessageBoxes = previousSuppress;

  const bool deleted = m_groups.size() < groupCountBefore;

  if (previousSelection.groupId < 0 || previousSelection.groupId == groupId)
    return;

  int restoreGid = previousSelection.groupId;
  if (deleted && restoreGid > groupId)
    restoreGid -= 1;

  if (static_cast<size_t>(restoreGid) < m_groups.size())
    setSelectedGroup(m_groups[restoreGid]);
}

/**
 * @brief Duplicates the group at @p groupId via the existing selection-based path.
 */
void DataModel::ProjectModel::duplicateGroup(int groupId)
{
  const ProjectUndoScope undo_scope{*this, tr("Duplicate Group")};
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  const auto previousSelection = m_selectedGroup;
  setSelectedGroup(m_groups[groupId]);
  duplicateCurrentGroup();

  if (previousSelection.groupId >= 0
      && static_cast<size_t>(previousSelection.groupId) < m_groups.size())
    setSelectedGroup(m_groups[previousSelection.groupId]);
}

/**
 * @brief Deletes the dataset at @p groupId/@p datasetId.
 */
void DataModel::ProjectModel::deleteDataset(int groupId, int datasetId, bool confirm)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  const auto previousSelection  = m_selectedDataset;
  const auto groupCountBefore   = m_groups.size();
  const auto datasetCountBefore = m_groups[groupId].datasets.size();
  setSelectedDataset(m_groups[groupId].datasets[datasetId]);

  const bool previousSuppress = m_suppressMessageBoxes;
  m_suppressMessageBoxes      = !confirm;
  deleteCurrentDataset();
  m_suppressMessageBoxes = previousSuppress;

  const bool groupDeleted = m_groups.size() < groupCountBefore;
  const bool sameGroupShrunk =
    !groupDeleted && m_groups[groupId].datasets.size() < datasetCountBefore;

  if (previousSelection.groupId < 0
      || (previousSelection.groupId == groupId && previousSelection.datasetId == datasetId))
    return;

  int restoreGid = previousSelection.groupId;
  int restoreDid = previousSelection.datasetId;
  if (groupDeleted && restoreGid > groupId)
    restoreGid -= 1;
  else if (sameGroupShrunk && restoreGid == groupId && restoreDid > datasetId)
    restoreDid -= 1;

  if (restoreGid >= 0 && static_cast<size_t>(restoreGid) < m_groups.size() && restoreDid >= 0
      && static_cast<size_t>(restoreDid) < m_groups[restoreGid].datasets.size())
    setSelectedDataset(m_groups[restoreGid].datasets[restoreDid]);
}

/**
 * @brief Duplicates the dataset at @p groupId/@p datasetId.
 */
void DataModel::ProjectModel::duplicateDataset(int groupId, int datasetId)
{
  const ProjectUndoScope undo_scope{*this, tr("Duplicate Dataset")};
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  const auto previousSelection = m_selectedDataset;
  setSelectedDataset(m_groups[groupId].datasets[datasetId]);
  duplicateCurrentDataset();

  if (previousSelection.groupId >= 0
      && static_cast<size_t>(previousSelection.groupId) < m_groups.size()
      && previousSelection.datasetId >= 0
      && static_cast<size_t>(previousSelection.datasetId)
           < m_groups[previousSelection.groupId].datasets.size())
    setSelectedDataset(m_groups[previousSelection.groupId].datasets[previousSelection.datasetId]);
}

/**
 * @brief Deletes the action at @p actionId via the existing selection-based path.
 */
void DataModel::ProjectModel::deleteAction(int actionId, bool confirm)
{
  if (actionId < 0 || static_cast<size_t>(actionId) >= m_actions.size())
    return;

  const auto previousSelection = m_selectedAction;
  const auto actionCountBefore = m_actions.size();
  setSelectedAction(m_actions[actionId]);

  const bool previousSuppress = m_suppressMessageBoxes;
  m_suppressMessageBoxes      = !confirm;
  deleteCurrentAction();
  m_suppressMessageBoxes = previousSuppress;

  const bool deleted = m_actions.size() < actionCountBefore;

  if (previousSelection.actionId < 0 || previousSelection.actionId == actionId)
    return;

  int restoreAid = previousSelection.actionId;
  if (deleted && restoreAid > actionId)
    restoreAid -= 1;

  if (static_cast<size_t>(restoreAid) < m_actions.size())
    setSelectedAction(m_actions[restoreAid]);
}

/**
 * @brief Duplicates the action at @p actionId.
 */
void DataModel::ProjectModel::duplicateAction(int actionId)
{
  const ProjectUndoScope undo_scope{*this, tr("Duplicate Action")};
  if (actionId < 0 || static_cast<size_t>(actionId) >= m_actions.size())
    return;

  const auto previousSelection = m_selectedAction;
  setSelectedAction(m_actions[actionId]);
  duplicateCurrentAction();

  if (previousSelection.actionId >= 0
      && static_cast<size_t>(previousSelection.actionId) < m_actions.size())
    setSelectedAction(m_actions[previousSelection.actionId]);
}

/**
 * @brief Deletes the output widget at @p groupId/@p widgetId.
 */
void DataModel::ProjectModel::deleteOutputWidget(int groupId, int widgetId, bool confirm)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (widgetId < 0 || static_cast<size_t>(widgetId) >= m_groups[groupId].outputWidgets.size())
    return;

  const auto previousSelection = m_selectedOutputWidget;
  const auto groupCountBefore  = m_groups.size();
  const auto widgetCountBefore = m_groups[groupId].outputWidgets.size();
  setSelectedOutputWidget(m_groups[groupId].outputWidgets[widgetId]);

  const bool previousSuppress = m_suppressMessageBoxes;
  m_suppressMessageBoxes      = !confirm;
  deleteCurrentOutputWidget();
  m_suppressMessageBoxes = previousSuppress;

  const bool groupDeleted = m_groups.size() < groupCountBefore;
  const bool sameGroupShrunk =
    !groupDeleted && m_groups[groupId].outputWidgets.size() < widgetCountBefore;

  if (previousSelection.groupId < 0
      || (previousSelection.groupId == groupId && previousSelection.widgetId == widgetId))
    return;

  int restoreGid = previousSelection.groupId;
  int restoreWid = previousSelection.widgetId;
  if (groupDeleted && restoreGid > groupId)
    restoreGid -= 1;
  else if (sameGroupShrunk && restoreGid == groupId && restoreWid > widgetId)
    restoreWid -= 1;

  if (restoreGid >= 0 && static_cast<size_t>(restoreGid) < m_groups.size() && restoreWid >= 0
      && static_cast<size_t>(restoreWid) < m_groups[restoreGid].outputWidgets.size())
    setSelectedOutputWidget(m_groups[restoreGid].outputWidgets[restoreWid]);
}

/**
 * @brief Duplicates the output widget at @p groupId/@p widgetId.
 */
void DataModel::ProjectModel::duplicateOutputWidget(int groupId, int widgetId)
{
  const ProjectUndoScope undo_scope{*this, tr("Duplicate Output Widget")};
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (widgetId < 0 || static_cast<size_t>(widgetId) >= m_groups[groupId].outputWidgets.size())
    return;

  const auto previousSelection = m_selectedOutputWidget;
  setSelectedOutputWidget(m_groups[groupId].outputWidgets[widgetId]);
  duplicateCurrentOutputWidget();

  if (previousSelection.groupId >= 0
      && static_cast<size_t>(previousSelection.groupId) < m_groups.size()
      && previousSelection.widgetId >= 0
      && static_cast<size_t>(previousSelection.widgetId)
           < m_groups[previousSelection.groupId].outputWidgets.size())
    setSelectedOutputWidget(
      m_groups[previousSelection.groupId].outputWidgets[previousSelection.widgetId]);
}

//--------------------------------------------------------------------------------------------------
// Bulk multi-selection mutators
//--------------------------------------------------------------------------------------------------

namespace DataModel {

/**
 * @brief Returns the parent folder id of @p folderId, or -1 when the folder is absent or top level.
 */
template<typename Folder>
static int folderParentId(const std::vector<Folder>& folders, int folderId)
{
  for (const auto& f : folders)
    if (f.folderId == folderId)
      return f.parentFolderId;

  return -1;
}

/**
 * @brief True when a folder strictly above @p folderId (an ancestor, never itself) is in @p chosen.
 */
template<typename Folder>
static bool folderHasSelectedAncestor(const std::vector<Folder>& folders,
                                      int folderId,
                                      const QSet<int>& chosen)
{
  int parent     = folderParentId(folders, folderId);
  const int kMax = static_cast<int>(folders.size());
  for (int i = 0; i < kMax && parent != -1; ++i) {
    if (chosen.contains(parent))
      return true;

    parent = folderParentId(folders, parent);
  }

  return false;
}

/**
 * @brief One pending batch-delete item, pinned to stable uniqueIds where they exist.
 */
struct BatchDeleteEntry {
  int kind;
  int id;
  int parentId;
  int groupUid;
  int datasetUid;
  QString path;
};

/**
 * @brief Returns the uniqueId of the group at positional @p gid, or -1 when out of range.
 */
static int batchGroupUidAt(const std::vector<Group>& groups, int gid)
{
  return (gid >= 0 && static_cast<size_t>(gid) < groups.size()) ? groups[gid].uniqueId : -1;
}

/**
 * @brief Returns the uniqueId of dataset @p did inside group @p gid, or -1 when out of range.
 */
static int batchDatasetUidAt(const std::vector<Group>& groups, int gid, int did)
{
  if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
    return -1;

  const auto& datasets = groups[gid].datasets;
  return (did >= 0 && static_cast<size_t>(did) < datasets.size()) ? datasets[did].uniqueId : -1;
}

/**
 * @brief Re-resolves a pinned group uniqueId to its current positional id; falls back to the
 *        original positional id for legacy unassigned uniqueIds, and -1 when the group is gone.
 */
static int batchResolveGroupId(const std::vector<Group>& groups, int groupUid, int staleGid)
{
  if (groupUid < 0)
    return staleGid;

  for (const auto& group : groups)
    if (group.uniqueId == groupUid)
      return group.groupId;

  return -1;
}

/**
 * @brief Re-resolves a pinned dataset uniqueId to its current index inside group @p gid; falls
 *        back to the original positional id when unpinned, and -1 when the dataset is gone.
 */
static int batchResolveDatasetId(const std::vector<Group>& groups,
                                 int gid,
                                 int datasetUid,
                                 int staleDid)
{
  if (datasetUid < 0)
    return staleDid;

  if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
    return -1;

  const auto& datasets = groups[gid].datasets;
  for (size_t d = 0; d < datasets.size(); ++d)
    if (datasets[d].uniqueId == datasetUid)
      return static_cast<int>(d);

  return -1;
}

}  // namespace DataModel

/**
 * @brief Duplicates every item in @p items, folder subtrees first: each clones its whole folder
 *        tree plus every group or table filed under it, skipping a nested folder whose ancestor is
 *        also selected. Remaining leaf items follow in declared order, skipping any group or table
 * a duplicated subtree already re-created.
 */
void DataModel::ProjectModel::duplicateSelectedItems(const QVariantList& items)
{
  const ProjectUndoScope undo_scope{*this, tr("Duplicate Selection")};
  QSet<int> selectedGroupFolders;
  QSet<int> selectedTableFolders;
  for (const auto& v : items) {
    const auto m   = v.toMap();
    const int kind = m.value(QStringLiteral("kind"), -1).toInt();
    const int id   = m.value(QStringLiteral("id"), -1).toInt();
    if (kind == ProjectEditor::KindGroupFolder)
      selectedGroupFolders.insert(id);
    else if (kind == ProjectEditor::KindTableFolder)
      selectedTableFolders.insert(id);
  }

  QSet<int> coveredGroupFolders;
  QSet<int> coveredTableFolders;
  for (const auto& v : items) {
    const auto m   = v.toMap();
    const int kind = m.value(QStringLiteral("kind"), -1).toInt();
    const int id   = m.value(QStringLiteral("id"), -1).toInt();

    if (kind == ProjectEditor::KindGroupFolder
        && !folderHasSelectedAncestor(m_groupFolders, id, selectedGroupFolders))
      coveredGroupFolders.unite(duplicateGroupFolderSubtree(id));

    else if (kind == ProjectEditor::KindTableFolder
             && !folderHasSelectedAncestor(m_tableFolders, id, selectedTableFolders))
      coveredTableFolders.unite(duplicateTableFolderSubtree(id));
  }

  for (const auto& v : items) {
    const auto entry   = v.toMap();
    const int kind     = entry.value(QStringLiteral("kind"), -1).toInt();
    const int id       = entry.value(QStringLiteral("id"), -1).toInt();
    const int parent   = entry.value(QStringLiteral("parentId"), -1).toInt();
    const QString path = entry.value(QStringLiteral("path")).toString();

    if (kind == ProjectEditor::KindGroup && coveredGroupFolders.contains(parent))
      continue;

    if (kind == ProjectEditor::KindUserTable && coveredTableFolders.contains(parent))
      continue;

    switch (kind) {
      case ProjectEditor::KindGroup:
        duplicateGroup(id);
        break;
      case ProjectEditor::KindDataset:
        duplicateDataset(parent, id);
        break;
      case ProjectEditor::KindAction:
        duplicateAction(id);
        break;
      case ProjectEditor::KindOutputWidget:
        duplicateOutputWidget(parent, id);
        break;
      case ProjectEditor::KindUserTable:
        duplicateTableByPath(path);
        break;
      default:
        break;
    }
  }
}

/**
 * @brief Deletes every item in @p items, children (higher kind) first, descending ids within a
 * kind. Group-referencing entries are pinned to uniqueIds and re-resolved per delete: deleting
 * a group's last dataset/output widget cascades into deleting the group and renumbers every
 * later group, so a stale positional id would delete an unrelated, unselected item.
 */
void DataModel::ProjectModel::deleteSelectedItems(const QVariantList& items)
{
  const ProjectUndoScope undo_scope{*this, tr("Delete Selection")};
  QList<BatchDeleteEntry> entries;
  entries.reserve(items.size());
  for (const auto& v : items) {
    const auto m = v.toMap();
    BatchDeleteEntry e;
    e.kind       = m.value(QStringLiteral("kind"), -1).toInt();
    e.id         = m.value(QStringLiteral("id"), -1).toInt();
    e.parentId   = m.value(QStringLiteral("parentId"), -1).toInt();
    e.path       = m.value(QStringLiteral("path")).toString();
    e.groupUid   = -1;
    e.datasetUid = -1;

    if (e.kind == ProjectEditor::KindGroup)
      e.groupUid = batchGroupUidAt(m_groups, e.id);

    if (e.kind == ProjectEditor::KindDataset || e.kind == ProjectEditor::KindOutputWidget)
      e.groupUid = batchGroupUidAt(m_groups, e.parentId);

    if (e.kind == ProjectEditor::KindDataset)
      e.datasetUid = batchDatasetUidAt(m_groups, e.parentId, e.id);

    entries.append(e);
  }

  std::sort(
    entries.begin(), entries.end(), [](const BatchDeleteEntry& a, const BatchDeleteEntry& b) {
      if (a.kind != b.kind)
        return a.kind > b.kind;

      if (a.parentId != b.parentId)
        return a.parentId > b.parentId;

      return a.id > b.id;
    });

  for (const auto& e : entries) {
    switch (e.kind) {
      case ProjectEditor::KindGroup: {
        const int gid = batchResolveGroupId(m_groups, e.groupUid, e.id);
        if (gid >= 0)
          deleteGroup(gid, false);

        break;
      }
      case ProjectEditor::KindDataset: {
        const int gid = batchResolveGroupId(m_groups, e.groupUid, e.parentId);
        const int did = batchResolveDatasetId(m_groups, gid, e.datasetUid, e.id);
        if (gid >= 0 && did >= 0)
          deleteDataset(gid, did, false);

        break;
      }
      case ProjectEditor::KindAction:
        deleteAction(e.id, false);
        break;
      case ProjectEditor::KindOutputWidget: {
        const int gid = batchResolveGroupId(m_groups, e.groupUid, e.parentId);
        if (gid >= 0)
          deleteOutputWidget(gid, e.id, false);

        break;
      }
      case ProjectEditor::KindWorkspace:
        deleteWorkspace(e.id);
        break;
      case ProjectEditor::KindWorkspaceFolder:
        deleteWorkspaceFolder(e.id);
        break;
      case ProjectEditor::KindGroupFolder:
        deleteGroupFolder(e.id);
        break;
      case ProjectEditor::KindUserTable:
        deleteTable(e.path);
        break;
      case ProjectEditor::KindTableFolder:
        deleteTableFolder(e.id);
        break;
      default:
        break;
    }
  }
}

/**
 * @brief Prompts before deleting a multi-selection, then deletes it. A single
 * item is removed without a prompt to preserve the existing delete behavior.
 */
void DataModel::ProjectModel::confirmDeleteSelectedItems(const QVariantList& items)
{
  if (items.isEmpty())
    return;

  const int count = static_cast<int>(items.size());
  if (count > 1) {
    const int choice = Misc::Utilities::showMessageBox(tr("Delete %1 selected items?").arg(count),
                                                       tr("This action cannot be undone."),
                                                       QMessageBox::Warning,
                                                       tr("Delete Items"),
                                                       QMessageBox::Yes | QMessageBox::Cancel,
                                                       QMessageBox::Cancel);

    if (choice != QMessageBox::Yes)
      return;
  }

  deleteSelectedItems(items);
}

/**
 * @brief Files every item in @p items into folder @p folderId via its per-kind move. The caller
 * filters the selection to a single section so @p folderId is interpreted correctly.
 */
void DataModel::ProjectModel::moveSelectedItemsToFolder(const QVariantList& items, int folderId)
{
  const ProjectUndoScope undo_scope{*this, tr("Move Selection")};
  for (const auto& v : items) {
    const auto m       = v.toMap();
    const int kind     = m.value(QStringLiteral("kind"), -1).toInt();
    const int id       = m.value(QStringLiteral("id"), -1).toInt();
    const QString path = m.value(QStringLiteral("path")).toString();

    switch (kind) {
      case ProjectEditor::KindWorkspace:
        moveWorkspaceToFolder(id, folderId);
        break;
      case ProjectEditor::KindWorkspaceFolder:
        moveFolderToFolder(id, folderId);
        break;
      case ProjectEditor::KindGroup:
        moveGroupToFolder(id, folderId);
        break;
      case ProjectEditor::KindGroupFolder:
        moveGroupFolderToFolder(id, folderId);
        break;
      case ProjectEditor::KindUserTable:
        moveTableToFolder(path, folderId);
        break;
      case ProjectEditor::KindTableFolder:
        moveTableFolderToFolder(id, folderId);
        break;
      default:
        break;
    }
  }
}

/**
 * @brief Applies @p enabled to every group whose folder is @p folderId or nested beneath it, so a
 *        folder toggle cascades to its whole subtree. Returns true when a group actually changed.
 */
bool DataModel::ProjectModel::setGroupsInFolderEnabled(const int folderId, const bool enabled)
{
  bool changed = false;
  for (auto& g : m_groups) {
    if (g.parentFolderId == -1)
      continue;

    if (!folderIsSelfOrDescendant(m_groupFolders, folderId, g.parentFolderId))
      continue;

    if (g.enabled == enabled)
      continue;

    g.enabled = enabled;
    changed   = true;
  }

  return changed;
}

/**
 * @brief Enables or disables every applicable item in a tree multi-selection in one pass; a group
 *        folder cascades to its whole subtree. Emits one change, refreshes the runtime frame once,
 *        and folds the batch into a single autosave.
 */
void DataModel::ProjectModel::setItemsEnabled(const QVariantList& items, const bool enabled)
{
  const ProjectUndoScope undo_scope{*this, tr("Toggle Selection")};
  if (items.isEmpty())
    return;

  const int groupCount = static_cast<int>(m_groups.size());

  setAutoSaveSuspended(true);
  bool changed = false;
  for (const auto& v : items) {
    const auto m     = v.toMap();
    const int kind   = m.value(QStringLiteral("kind"), -1).toInt();
    const int id     = m.value(QStringLiteral("id"), -1).toInt();
    const int parent = m.value(QStringLiteral("parentId"), -1).toInt();

    if (kind == ProjectEditor::KindGroupFolder) {
      changed |= setGroupsInFolderEnabled(id, enabled);
      continue;
    }

    if (kind == ProjectEditor::KindGroup && id >= 0 && id < groupCount
        && m_groups[id].enabled != enabled) {
      m_groups[id].enabled = enabled;
      changed              = true;
      continue;
    }

    if (kind == ProjectEditor::KindDataset && parent >= 0 && parent < groupCount) {
      auto& datasets = m_groups[parent].datasets;
      if (id < 0 || static_cast<size_t>(id) >= datasets.size() || datasets[id].enabled == enabled)
        continue;

      datasets[id].enabled = enabled;
      changed              = true;
    }
  }

  setAutoSaveSuspended(false);
  if (!changed)
    return;

  Q_EMIT groupsChanged();
  setModified(true);
  syncRuntime();
  flushAutoSave();
}
