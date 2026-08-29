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

#include "DataModel/Project/ProjectEntities.h"

#include <algorithm>
#include <QInputDialog>
#include <QMap>
#include <QMessageBox>
#include <QSet>

#include "AppInfo.h"
#include "DataModel/Project/ProjectFixedLayouts.h"
#include "DataModel/Project/ProjectFolders.h"
#include "DataModel/Project/ProjectHistory.h"
#include "DataModel/Project/ProjectNaming.h"
#include "DataModel/Project/ProjectPersistence.h"
#include "DataModel/Project/ProjectWorkspaces.h"
#include "DataModel/ProjectModel.h"
#include "Misc/Utilities.h"
#include "SSAssert.h"

namespace DataModel {

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
 * @brief Returns @p title, or the lowest "title (N)" form free among @p taken.
 */
static QString uniqueEntityTitle(const QString& title, const QStringList& taken)
{
  int count        = 1;
  QString newTitle = title;
  for (const auto& t : taken) {
    if (t == newTitle) {
      count++;
      newTitle = QString("%1 (%2)").arg(title, QString::number(count));
    }
  }

  while (count > 1) {
    bool titleExists = false;
    for (const auto& t : taken) {
      if (t != newTitle)
        continue;

      count++;
      newTitle    = QString("%1 (%2)").arg(title, QString::number(count));
      titleExists = true;
      break;
    }

    if (!titleExists)
      break;
  }

  return newTitle;
}

}  // namespace DataModel

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the entity editor to @p model.
 */
DataModel::ProjectEntities::ProjectEntities(ProjectModel& model) : m_model(model) {}

/**
 * @brief Renumbers Group::groupId (and child Dataset::groupId) to match the vector order, after an
 *        erase or a reorder has left the positional ids stale.
 */
void DataModel::ProjectEntities::renumberGroupIds()
{
  int id = 0;
  for (auto g = m_model.m_groups.begin(); g != m_model.m_groups.end(); ++g, ++id) {
    g->groupId = id;
    for (auto d = g->datasets.begin(); d != g->datasets.end(); ++d)
      d->groupId = id;
  }
}

//--------------------------------------------------------------------------------------------------
// In-place updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Replaces the group at groupId and emits groupsChanged.
 */
void DataModel::ProjectEntities::updateGroup(const int groupId,
                                             const DataModel::Group& group,
                                             const bool rebuildTree)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Edit Group")};
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_model.m_groups.size())
    return;

  m_model.m_groups[groupId] = group;

  if (rebuildTree)
    Q_EMIT m_model.groupsChanged();
  else
    Q_EMIT m_model.groupDataChanged();

  m_model.setModified(true);
}

/**
 * @brief Replaces the dataset at @p groupId/@p datasetId.
 */
void DataModel::ProjectEntities::updateDataset(const int groupId,
                                               const int datasetId,
                                               const DataModel::Dataset& dataset,
                                               const bool rebuildTree)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Edit Dataset")};
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return;

  DataModel::Dataset resolved = dataset;
  if (resolved.transformLanguage < 0 && !resolved.transformCode.isEmpty()) {
    for (const auto& src : m_model.m_sources)
      if (src.sourceId == resolved.sourceId) {
        resolved.transformLanguage = src.frameParserLanguage == SerialStudio::Native
                                     ? static_cast<int>(SerialStudio::Lua)
                                     : src.frameParserLanguage;
        break;
      }

    if (resolved.transformLanguage < 0)
      resolved.transformLanguage = 0;
  }

  groups[groupId].datasets[datasetId] = resolved;
  m_model.m_selectedDataset           = resolved;

  if (rebuildTree)
    Q_EMIT m_model.groupsChanged();

  m_model.m_persistence.setRuntimeDirty(true);
  m_model.setModified(true);
  m_model.m_persistence.scheduleAutoSave();
}

/**
 * @brief Replaces the action at actionId and emits actionsChanged.
 */
void DataModel::ProjectEntities::updateAction(const int actionId,
                                              const DataModel::Action& action,
                                              const bool rebuildTree)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Edit Action")};
  if (actionId < 0 || static_cast<size_t>(actionId) >= m_model.m_actions.size())
    return;

  m_model.m_actions[actionId] = action;

  if (rebuildTree)
    Q_EMIT m_model.actionsChanged();

  m_model.setModified(true);
}

/**
 * @brief Fills every empty dataset alias from its title (deduplicated), leaving existing aliases
 *        untouched; returns the number seeded. One modified state + autosave for the whole batch.
 */
int DataModel::ProjectEntities::seedDatasetAliases()
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Seed Dataset Aliases")};
  QSet<QString> used;
  for (const auto& group : m_model.m_groups)
    for (const auto& ds : group.datasets)
      if (!ds.alias.isEmpty())
        used.insert(ds.alias);

  int seeded = 0;
  for (auto& group : m_model.m_groups) {
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
    m_model.m_persistence.setRuntimeDirty(true);
    m_model.setModified(true);
    Q_EMIT m_model.groupsChanged();
    m_model.m_persistence.scheduleAutoSave();
  }

  return seeded;
}

/**
 * @brief Enables or disables a group; a disabled group is excluded from frame building while the
 *        editor still shows it greyed. Refreshes the runtime frame so the dashboard updates at
 * once.
 */
void DataModel::ProjectEntities::setGroupEnabled(const int groupId, const bool enabled)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Toggle Group")};
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  if (groups[groupId].enabled == enabled)
    return;

  groups[groupId].enabled = enabled;

  Q_EMIT m_model.groupsChanged();
  m_model.setModified(true);
  m_model.m_persistence.syncRuntime();
}

/**
 * @brief Enables or disables a single dataset; a disabled dataset is excluded from frame building
 *        while its siblings keep their explicit frame indices. Refreshes the runtime frame.
 */
void DataModel::ProjectEntities::setDatasetEnabled(const int groupId,
                                                   const int datasetId,
                                                   const bool enabled)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Toggle Dataset")};
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return;

  if (groups[groupId].datasets[datasetId].enabled == enabled)
    return;

  groups[groupId].datasets[datasetId].enabled = enabled;

  Q_EMIT m_model.groupsChanged();
  m_model.setModified(true);
  m_model.m_persistence.syncRuntime();
}

//--------------------------------------------------------------------------------------------------
// Selection-based deletion and duplication
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deletes the currently selected group after user confirmation.
 */
void DataModel::ProjectEntities::deleteCurrentGroup()
{
  if (!m_model.m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      ProjectModel::tr("Do you want to delete group \"%1\"?").arg(m_model.m_selectedGroup.title),
      ProjectModel::tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto gid = m_model.m_selectedGroup.groupId;
  if (gid < 0 || static_cast<size_t>(gid) >= m_model.m_groups.size())
    return;

  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Delete Group")};

  auto& workspaces = m_model.m_workspaces;
  QMap<int, int> deletedTypeCounts;
  if (workspaces.customizeWorkspaces())
    deletedTypeCounts = workspaces.widgetTypeCountsForGroup(m_model.m_groups[gid]);

  m_model.m_groups.erase(m_model.m_groups.begin() + gid);
  renumberGroupIds();

  if (workspaces.customizeWorkspaces())
    workspaces.shiftWorkspaceRefsAfterGroupDelete(gid, deletedTypeCounts);

  workspaces.shiftHiddenGroupIdsAfterGroupDelete(gid);
  workspaces.shiftLayoutKeysAfterGroupDelete(gid);

  Q_EMIT m_model.groupsChanged();
  Q_EMIT m_model.groupDeleted();
  m_model.setModified(true);
}

/**
 * @brief Deletes the currently selected action after user confirmation.
 */
void DataModel::ProjectEntities::deleteCurrentAction()
{
  if (!m_model.m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      ProjectModel::tr("Do you want to delete action \"%1\"?").arg(m_model.m_selectedAction.title),
      ProjectModel::tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto aid = m_model.m_selectedAction.actionId;
  if (aid < 0 || static_cast<size_t>(aid) >= m_model.m_actions.size())
    return;

  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Delete Action")};

  m_model.m_actions.erase(m_model.m_actions.begin() + aid);

  int id = 0;
  for (auto a = m_model.m_actions.begin(); a != m_model.m_actions.end(); ++a, ++id)
    a->actionId = id;

  Q_EMIT m_model.actionsChanged();
  Q_EMIT m_model.actionDeleted();
  m_model.setModified(true);
}

/**
 * @brief Deletes the selected dataset, removing the group if it becomes empty.
 */
void DataModel::ProjectEntities::deleteCurrentDataset()
{
  if (!m_model.m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      ProjectModel::tr("Do you want to delete dataset \"%1\"?")
        .arg(m_model.m_selectedDataset.title),
      ProjectModel::tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto groupId   = m_model.m_selectedDataset.groupId;
  const auto datasetId = m_model.m_selectedDataset.datasetId;
  auto& groups         = m_model.m_groups;

  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return;

  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Delete Dataset")};

  auto& workspaces = m_model.m_workspaces;
  QMap<int, int> deletedTypeCounts;
  if (workspaces.customizeWorkspaces())
    deletedTypeCounts = workspaces.widgetTypeCountsForGroup(groups[groupId]);

  QMap<int, int> datasetTypeCounts;
  if (workspaces.customizeWorkspaces()) {
    const auto& ds  = groups[groupId].datasets[datasetId];
    const auto keys = SerialStudio::getDashboardWidgets(ds);
    for (const auto& k : keys)
      if (SerialStudio::datasetWidgetEligibleForWorkspace(k))
        datasetTypeCounts[static_cast<int>(k)] += 1;
  }

  groups[groupId].datasets.erase(groups[groupId].datasets.begin() + datasetId);

  const auto& widgetId        = groups[groupId].widget;
  const bool widgetCanBeEmpty = (widgetId == QLatin1String("painter")
                                 || widgetId == QLatin1String("image") || widgetId.isEmpty());

  if (groups[groupId].datasets.empty() && !widgetCanBeEmpty) {
    groups.erase(groups.begin() + groupId);
    renumberGroupIds();

    if (workspaces.customizeWorkspaces())
      workspaces.shiftWorkspaceRefsAfterGroupDelete(groupId, deletedTypeCounts);

    workspaces.shiftHiddenGroupIdsAfterGroupDelete(groupId);
    workspaces.shiftLayoutKeysAfterGroupDelete(groupId);

    Q_EMIT m_model.groupsChanged();
    Q_EMIT m_model.datasetDeleted(-1);
    m_model.setModified(true);
    return;
  }

  int id     = 0;
  auto begin = groups[groupId].datasets.begin();
  auto end   = groups[groupId].datasets.end();
  for (auto dataset = begin; dataset != end; ++dataset, ++id)
    dataset->datasetId = id;

  if (workspaces.customizeWorkspaces())
    workspaces.shiftWorkspaceRefsAfterDatasetDelete(groupId, datasetTypeCounts);

  Q_EMIT m_model.groupsChanged();
  Q_EMIT m_model.datasetDeleted(groupId);
  m_model.setModified(true);
}

/**
 * @brief Appends a copy of the currently selected group to the project.
 */
void DataModel::ProjectEntities::duplicateCurrentGroup()
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Duplicate Group")};
  auto& groups           = m_model.m_groups;
  const auto& selected   = m_model.m_selectedGroup;
  DataModel::Group group = selected;
  group.groupId          = groups.size();
  group.uniqueId         = m_model.allocateUniqueId();
  group.datasets.clear();
  group.outputWidgets.clear();

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(groups.size()));
  for (const auto& g : groups)
    existingTitles.append(g.title);

  group.title = nextDuplicateTitle(selected.title, existingTitles);

  for (size_t i = 0; i < selected.datasets.size(); ++i) {
    auto dataset     = selected.datasets[i];
    dataset.groupId  = group.groupId;
    dataset.index    = m_model.nextDatasetIndex() + static_cast<int>(i);
    dataset.uniqueId = m_model.allocateUniqueId();
    group.datasets.push_back(dataset);
  }

  for (const auto& ow : selected.outputWidgets) {
    auto copy    = ow;
    copy.groupId = group.groupId;
    group.outputWidgets.push_back(copy);
  }

  groups.push_back(group);
  m_model.m_selectedGroup = groups.back();

  Q_EMIT m_model.groupsChanged();
  Q_EMIT m_model.groupAdded(static_cast<int>(groups.size()) - 1);
  m_model.setModified(true);
}

/**
 * @brief Appends a copy of the currently selected action to the project.
 */
void DataModel::ProjectEntities::duplicateCurrentAction()
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Duplicate Action")};
  auto& actions        = m_model.m_actions;
  const auto& selected = m_model.m_selectedAction;

  DataModel::Action action;
  action.actionId             = actions.size();
  action.icon                 = selected.icon;
  action.txData               = selected.txData;
  action.timerMode            = selected.timerMode;
  action.repeatCount          = selected.repeatCount;
  action.eolSequence          = selected.eolSequence;
  action.timerIntervalMs      = selected.timerIntervalMs;
  action.autoExecuteOnConnect = selected.autoExecuteOnConnect;

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(actions.size()));
  for (const auto& a : actions)
    existingTitles.append(a.title);

  action.title = nextDuplicateTitle(selected.title, existingTitles);

  actions.push_back(action);
  m_model.m_selectedAction = action;

  Q_EMIT m_model.actionsChanged();
  Q_EMIT m_model.actionAdded(static_cast<int>(actions.size()) - 1);
  m_model.setModified(true);
}

/**
 * @brief Appends a copy of the currently selected dataset to its parent group.
 */
void DataModel::ProjectEntities::duplicateCurrentDataset()
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Duplicate Dataset")};
  auto& groups = m_model.m_groups;
  auto dataset = m_model.m_selectedDataset;

  if (dataset.groupId < 0 || static_cast<size_t>(dataset.groupId) >= groups.size())
    return;

  dataset.index     = m_model.nextDatasetIndex();
  dataset.datasetId = groups[dataset.groupId].datasets.size();
  dataset.uniqueId  = m_model.allocateUniqueId();

  const auto& siblings = groups[dataset.groupId].datasets;
  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(siblings.size()));
  for (const auto& d : siblings)
    existingTitles.append(d.title);

  dataset.title = nextDuplicateTitle(m_model.m_selectedDataset.title, existingTitles);

  groups[dataset.groupId].datasets.push_back(dataset);
  m_model.m_selectedDataset = dataset;

  Q_EMIT m_model.groupsChanged();
  Q_EMIT m_model.datasetAdded(dataset.groupId,
                              static_cast<int>(groups[dataset.groupId].datasets.size()) - 1);
  m_model.setModified(true);
}

//--------------------------------------------------------------------------------------------------
// Creation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Ensures a compatible group is selected before adding a dataset; honors sourceId scoping.
 */
void DataModel::ProjectEntities::ensureValidGroup(int sourceId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Add Group")};
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

  auto& groups          = m_model.m_groups;
  const auto selId      = m_model.m_selectedGroup.groupId;
  const bool selInRange = selId >= 0 && static_cast<size_t>(selId) < groups.size();

  if (selInRange && isValidGroup(groups[selId])) {
    m_model.m_selectedGroup = groups[selId];
    return;
  }

  for (const auto& group : std::as_const(groups)) {
    if (!isValidGroup(group))
      continue;

    m_model.m_selectedGroup = group;
    return;
  }

  addGroup(ProjectModel::tr("Group"), SerialStudio::NoGroupWidget, sourceId, -1);
  m_model.m_selectedGroup = groups.back();
}

/**
 * @brief Adds a new dataset of the given type to the selected group.
 */
void DataModel::ProjectEntities::addDataset(const SerialStudio::DatasetOption option, int sourceId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Add Dataset")};
  ensureValidGroup(sourceId);

  auto& groups       = m_model.m_groups;
  const auto groupId = m_model.m_selectedGroup.groupId;
  DataModel::Dataset dataset;
  dataset.groupId  = groupId;
  dataset.sourceId = groups[groupId].sourceId;

  QString title;
  switch (option) {
    case SerialStudio::DatasetGeneric:
      title = ProjectModel::tr("New Dataset");
      break;
    case SerialStudio::DatasetPlot:
      title       = ProjectModel::tr("New Plot");
      dataset.plt = true;
      break;
    case SerialStudio::DatasetFFT:
      title       = ProjectModel::tr("New FFT Plot");
      dataset.fft = true;
      break;
    case SerialStudio::DatasetBar:
      title          = ProjectModel::tr("New Level Indicator");
      dataset.widget = QStringLiteral("bar");
      break;
    case SerialStudio::DatasetGauge:
      title          = ProjectModel::tr("New Gauge");
      dataset.widget = QStringLiteral("gauge");
      break;
    case SerialStudio::DatasetCompass:
      title          = ProjectModel::tr("New Compass");
      dataset.wgtMin = 0;
      dataset.wgtMax = 360;
      dataset.widget = QStringLiteral("compass");
      break;
    case SerialStudio::DatasetMeter:
      title          = ProjectModel::tr("New Meter");
      dataset.widget = QStringLiteral("meter");
      break;
    case SerialStudio::DatasetLED:
      title       = ProjectModel::tr("New LED Indicator");
      dataset.led = true;
      break;
    case SerialStudio::DatasetWaterfall:
      title             = ProjectModel::tr("New Waterfall");
      dataset.waterfall = true;
      break;
    default:
      break;
  }

  QStringList siblingTitles;
  siblingTitles.reserve(static_cast<int>(groups[groupId].datasets.size()));
  for (const auto& d : std::as_const(groups[groupId].datasets))
    siblingTitles.append(d.title);

  dataset.title     = uniqueEntityTitle(title, siblingTitles);
  dataset.index     = m_model.nextDatasetIndex();
  dataset.datasetId = groups[groupId].datasets.size();
  dataset.uniqueId  = m_model.allocateUniqueId();

  groups[groupId].datasets.push_back(dataset);
  m_model.m_selectedDataset = dataset;

  Q_EMIT m_model.groupsChanged();
  Q_EMIT m_model.datasetAdded(groupId, static_cast<int>(groups[groupId].datasets.size()) - 1);
  m_model.setModified(true);
}

/**
 * @brief Appends template-defined datasets to a painter group when the group has fewer than the
 * spec demands. Existing datasets are preserved.
 */
void DataModel::ProjectEntities::ensurePainterDatasets(int groupId, const QVariantList& specs)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Edit Canvas Datasets")};
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  if (specs.isEmpty())
    return;

  auto& grp          = groups[groupId];
  const int existing = static_cast<int>(grp.datasets.size());
  bool changed       = false;

  for (int i = existing; i < specs.size(); ++i) {
    const auto map = specs.at(i).toMap();
    DataModel::Dataset ds;
    ds.groupId   = groupId;
    ds.sourceId  = grp.sourceId;
    ds.datasetId = static_cast<int>(grp.datasets.size());
    ds.index     = m_model.nextDatasetIndex();
    ds.uniqueId  = m_model.allocateUniqueId();
    ds.title =
      map.value(QStringLiteral("title"), ProjectModel::tr("Channel %1").arg(i + 1)).toString();
    ds.units  = map.value(QStringLiteral("units")).toString();
    ds.wgtMin = SerialStudio::toDouble(map.value(QStringLiteral("min"), 0.0));
    ds.wgtMax = SerialStudio::toDouble(map.value(QStringLiteral("max"), 100.0));
    grp.datasets.push_back(std::move(ds));
    changed = true;
  }

  if (changed) {
    m_model.m_selectedGroup = grp;
    Q_EMIT m_model.groupsChanged();
    m_model.setModified(true);
  }
}

/**
 * @brief Toggles a dataset option flag on the currently selected dataset.
 */
void DataModel::ProjectEntities::changeDatasetOption(const SerialStudio::DatasetOption option,
                                                     const bool checked)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Dataset Option")};
  auto& selected = m_model.m_selectedDataset;
  switch (option) {
    case SerialStudio::DatasetPlot:
      selected.plt = checked;
      break;
    case SerialStudio::DatasetFFT:
      selected.fft = checked;
      break;
    case SerialStudio::DatasetBar:
      selected.widget = checked ? QStringLiteral("bar") : "";
      break;
    case SerialStudio::DatasetGauge:
      selected.widget = checked ? QStringLiteral("gauge") : "";
      break;
    case SerialStudio::DatasetCompass:
      selected.widget = checked ? QStringLiteral("compass") : "";
      break;
    case SerialStudio::DatasetMeter:
      selected.widget = checked ? QStringLiteral("meter") : "";
      break;
    case SerialStudio::DatasetLED:
      selected.led = checked;
      break;
    case SerialStudio::DatasetWaterfall:
      selected.waterfall = checked;
      break;
    default:
      break;
  }

  auto& groups         = m_model.m_groups;
  const auto groupId   = selected.groupId;
  const auto datasetId = selected.datasetId;

  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return;

  groups[groupId].datasets[datasetId] = selected;

  Q_EMIT m_model.groupsChanged();
  m_model.setModified(true);
}

/**
 * @brief Adds a new action with a unique title to the project.
 */
void DataModel::ProjectEntities::addAction(int sourceId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Add Action")};
  auto& actions = m_model.m_actions;

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(actions.size()));
  for (const auto& a : std::as_const(actions))
    existingTitles.append(a.title);

  DataModel::Action action;
  action.title    = uniqueEntityTitle(ProjectModel::tr("New Action"), existingTitles);
  action.actionId = actions.size();
  if (sourceId >= 0)
    action.sourceId = sourceId;

  actions.push_back(action);
  m_model.m_selectedAction = action;

  Q_EMIT m_model.actionsChanged();
  Q_EMIT m_model.actionAdded(static_cast<int>(actions.size()) - 1);
  m_model.setModified(true);
}

/**
 * @brief Adds a new group with a unique title and the given widget type.
 */
void DataModel::ProjectEntities::addGroup(const QString& title,
                                          const SerialStudio::GroupWidget widget,
                                          int sourceId,
                                          int parentFolderId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Add Group")};
  auto& groups = m_model.m_groups;

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(groups.size()));
  for (const auto& g : std::as_const(groups))
    existingTitles.append(g.title);

  DataModel::Group group;
  group.title    = uniqueEntityTitle(title, existingTitles);
  group.groupId  = groups.size();
  group.uniqueId = m_model.allocateUniqueId();
  group.parentFolderId =
    (parentFolderId != -1 && folderExists(m_model.m_folders.groupFolders(), parentFolderId))
      ? parentFolderId
      : -1;

  if (sourceId >= 0)
    group.sourceId = sourceId;

  groups.push_back(group);
  (void)setGroupWidget(static_cast<int>(groups.size()) - 1, widget);
  m_model.m_selectedGroup = groups.back();

  Q_EMIT m_model.groupAdded(static_cast<int>(groups.size()) - 1);
  m_model.setModified(true);
}

//--------------------------------------------------------------------------------------------------
// Group widget assignment
//--------------------------------------------------------------------------------------------------

/**
 * @brief Assigns a widget type to the group, replacing fixed-layout datasets.
 */
bool DataModel::ProjectEntities::setGroupWidget(const int group,
                                                const SerialStudio::GroupWidget widget)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Group Widget")};
  auto& groups = m_model.m_groups;
  if (group < 0 || group >= static_cast<int>(groups.size())) [[unlikely]]
    return false;

  auto& grp = groups[group];

  if (!confirmGroupWidgetChange(grp, widget))
    return false;

  if (!applyGroupWidget(grp, widget))
    return false;

  for (auto& d : grp.datasets)
    if (d.uniqueId < 0)
      d.uniqueId = m_model.allocateUniqueId();

  groups[group] = grp;

  Q_EMIT m_model.groupsChanged();
  m_model.setModified(true);
  return true;
}

/**
 * @brief Confirms a destructive group widget change and clears existing datasets if needed.
 */
bool DataModel::ProjectEntities::confirmGroupWidgetChange(DataModel::Group& grp,
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

  auto ret = Misc::Utilities::showMessageBox(
    ProjectModel::tr("Are you sure you want to change the group-level widget?"),
    ProjectModel::tr("Existing datasets for this group are deleted"),
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
bool DataModel::ProjectEntities::applyGroupWidget(DataModel::Group& grp,
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
      return FixedLayouts::populateFixedLayoutGroup(grp, widget, m_model.nextDatasetIndex());

    return true;
  }

  return FixedLayouts::populateFixedLayoutGroup(grp, widget, m_model.nextDatasetIndex());
}

//--------------------------------------------------------------------------------------------------
// Reordering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Moves a group from one position to another, preserving widget settings,
 *        workspace refs, hidden state, and auto-workspace IDs across the shift. Refs are
 *        re-resolved before the auto-workspace IDs are renumbered, because the anchor
 *        snapshot is keyed by the workspace IDs that renumbering replaces.
 */
void DataModel::ProjectEntities::moveGroup(int fromGroupId, int toGroupId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Move Group")};
  auto& groups = m_model.m_groups;
  const int n  = static_cast<int>(groups.size());
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

  auto& workspaces = m_model.m_workspaces;
  ProjectWorkspaces::RefAnchors anchors;
  if (workspaces.customizeWorkspaces())
    anchors = workspaces.snapshotRefAnchors();

  auto group = groups[fromGroupId];
  groups.erase(groups.begin() + fromGroupId);
  groups.insert(groups.begin() + target, group);

  remapGroupIdsAfterReorder(oldToNewGid);

  workspaces.remapLayoutKeysAfterReorder(oldToNewGid);
  workspaces.remapHiddenGroupIdsAfterReorder(oldToNewGid);

  if (workspaces.customizeWorkspaces())
    workspaces.resolveRefAnchors(anchors);

  workspaces.remapAutoWorkspaceIdsAfterReorder(oldToNewGid);

  if (m_model.m_selectedGroup.groupId == fromGroupId)
    m_model.m_selectedGroup = groups[target];

  Q_EMIT m_model.groupsChanged();
  Q_EMIT m_model.widgetSettingsChanged();
  m_model.setModified(true);
}

/**
 * @brief Moves a dataset within its group, renumbering datasetIds and re-resolving
 *        workspace refs in customise mode.
 */
void DataModel::ProjectEntities::moveDataset(int groupId, int fromDatasetId, int toDatasetId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Move Dataset")};
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  auto& datasets = groups[groupId].datasets;
  const int n    = static_cast<int>(datasets.size());
  if (fromDatasetId < 0 || fromDatasetId >= n)
    return;

  const int target = std::clamp(toDatasetId, 0, n - 1);
  if (target == fromDatasetId)
    return;

  auto& workspaces = m_model.m_workspaces;
  ProjectWorkspaces::RefAnchors anchors;
  if (workspaces.customizeWorkspaces())
    anchors = workspaces.snapshotRefAnchors();

  auto dataset = datasets[fromDatasetId];
  datasets.erase(datasets.begin() + fromDatasetId);
  datasets.insert(datasets.begin() + target, dataset);

  for (size_t i = 0; i < datasets.size(); ++i)
    datasets[i].datasetId = static_cast<int>(i);

  if (workspaces.customizeWorkspaces())
    workspaces.resolveRefAnchors(anchors);

  auto& selected = m_model.m_selectedDataset;
  if (selected.groupId == groupId && selected.datasetId == fromDatasetId)
    selected = datasets[target];

  Q_EMIT m_model.groupsChanged();
  m_model.setModified(true);
}

/**
 * @brief Moves an action within the project actions list, renumbering actionIds.
 */
void DataModel::ProjectEntities::moveAction(int fromActionId, int toActionId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Move Action")};
  auto& actions = m_model.m_actions;
  const int n   = static_cast<int>(actions.size());
  if (fromActionId < 0 || fromActionId >= n)
    return;

  const int target = std::clamp(toActionId, 0, n - 1);
  if (target == fromActionId)
    return;

  auto action = actions[fromActionId];
  actions.erase(actions.begin() + fromActionId);
  actions.insert(actions.begin() + target, action);

  for (size_t i = 0; i < actions.size(); ++i)
    actions[i].actionId = static_cast<int>(i);

  if (m_model.m_selectedAction.actionId == fromActionId)
    m_model.m_selectedAction = actions[target];

  Q_EMIT m_model.actionsChanged();
  m_model.setModified(true);
}

/**
 * @brief Renumbers Group::groupId (and child Dataset::groupId) to match the new vector order.
 */
void DataModel::ProjectEntities::remapGroupIdsAfterReorder(const std::vector<int>& oldToNewGid)
{
  SS_ASSERT_LOG(oldToNewGid.size() == m_model.m_groups.size());

  renumberGroupIds();
  Q_UNUSED(oldToNewGid);
}

//--------------------------------------------------------------------------------------------------
// Stateless id-based mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deletes the group at @p groupId; opt-in confirmation reuses deleteCurrentGroup's dialog.
 */
void DataModel::ProjectEntities::deleteGroup(int groupId, bool confirm)
{
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  const auto previousSelection = m_model.m_selectedGroup;
  const auto groupCountBefore  = groups.size();
  m_model.setSelectedGroup(groups[groupId]);

  const bool previousSuppress    = m_model.m_suppressMessageBoxes;
  m_model.m_suppressMessageBoxes = !confirm;
  deleteCurrentGroup();
  m_model.m_suppressMessageBoxes = previousSuppress;

  const bool deleted = groups.size() < groupCountBefore;

  if (previousSelection.groupId < 0 || previousSelection.groupId == groupId)
    return;

  int restoreGid = previousSelection.groupId;
  if (deleted && restoreGid > groupId)
    restoreGid -= 1;

  if (static_cast<size_t>(restoreGid) < groups.size())
    m_model.setSelectedGroup(groups[restoreGid]);
}

/**
 * @brief Duplicates the group at @p groupId via the existing selection-based path.
 */
void DataModel::ProjectEntities::duplicateGroup(int groupId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Duplicate Group")};
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  const auto previousSelection = m_model.m_selectedGroup;
  m_model.setSelectedGroup(groups[groupId]);
  duplicateCurrentGroup();

  if (previousSelection.groupId >= 0
      && static_cast<size_t>(previousSelection.groupId) < groups.size())
    m_model.setSelectedGroup(groups[previousSelection.groupId]);
}

/**
 * @brief Deletes the dataset at @p groupId/@p datasetId.
 */
void DataModel::ProjectEntities::deleteDataset(int groupId, int datasetId, bool confirm)
{
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return;

  const auto previousSelection  = m_model.m_selectedDataset;
  const auto groupCountBefore   = groups.size();
  const auto datasetCountBefore = groups[groupId].datasets.size();
  m_model.setSelectedDataset(groups[groupId].datasets[datasetId]);

  const bool previousSuppress    = m_model.m_suppressMessageBoxes;
  m_model.m_suppressMessageBoxes = !confirm;
  deleteCurrentDataset();
  m_model.m_suppressMessageBoxes = previousSuppress;

  const bool groupDeleted = groups.size() < groupCountBefore;
  const bool sameGroupShrunk =
    !groupDeleted && groups[groupId].datasets.size() < datasetCountBefore;

  if (previousSelection.groupId < 0
      || (previousSelection.groupId == groupId && previousSelection.datasetId == datasetId))
    return;

  int restoreGid = previousSelection.groupId;
  int restoreDid = previousSelection.datasetId;
  if (groupDeleted && restoreGid > groupId)
    restoreGid -= 1;
  else if (sameGroupShrunk && restoreGid == groupId && restoreDid > datasetId)
    restoreDid -= 1;

  if (restoreGid >= 0 && static_cast<size_t>(restoreGid) < groups.size() && restoreDid >= 0
      && static_cast<size_t>(restoreDid) < groups[restoreGid].datasets.size())
    m_model.setSelectedDataset(groups[restoreGid].datasets[restoreDid]);
}

/**
 * @brief Duplicates the dataset at @p groupId/@p datasetId.
 */
void DataModel::ProjectEntities::duplicateDataset(int groupId, int datasetId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Duplicate Dataset")};
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return;

  const auto previousSelection = m_model.m_selectedDataset;
  m_model.setSelectedDataset(groups[groupId].datasets[datasetId]);
  duplicateCurrentDataset();

  if (previousSelection.groupId >= 0
      && static_cast<size_t>(previousSelection.groupId) < groups.size()
      && previousSelection.datasetId >= 0
      && static_cast<size_t>(previousSelection.datasetId)
           < groups[previousSelection.groupId].datasets.size())
    m_model.setSelectedDataset(
      groups[previousSelection.groupId].datasets[previousSelection.datasetId]);
}

/**
 * @brief Deletes the action at @p actionId via the existing selection-based path.
 */
void DataModel::ProjectEntities::deleteAction(int actionId, bool confirm)
{
  auto& actions = m_model.m_actions;
  if (actionId < 0 || static_cast<size_t>(actionId) >= actions.size())
    return;

  const auto previousSelection = m_model.m_selectedAction;
  const auto actionCountBefore = actions.size();
  m_model.setSelectedAction(actions[actionId]);

  const bool previousSuppress    = m_model.m_suppressMessageBoxes;
  m_model.m_suppressMessageBoxes = !confirm;
  deleteCurrentAction();
  m_model.m_suppressMessageBoxes = previousSuppress;

  const bool deleted = actions.size() < actionCountBefore;

  if (previousSelection.actionId < 0 || previousSelection.actionId == actionId)
    return;

  int restoreAid = previousSelection.actionId;
  if (deleted && restoreAid > actionId)
    restoreAid -= 1;

  if (static_cast<size_t>(restoreAid) < actions.size())
    m_model.setSelectedAction(actions[restoreAid]);
}

/**
 * @brief Duplicates the action at @p actionId.
 */
void DataModel::ProjectEntities::duplicateAction(int actionId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Duplicate Action")};
  auto& actions = m_model.m_actions;
  if (actionId < 0 || static_cast<size_t>(actionId) >= actions.size())
    return;

  const auto previousSelection = m_model.m_selectedAction;
  m_model.setSelectedAction(actions[actionId]);
  duplicateCurrentAction();

  if (previousSelection.actionId >= 0
      && static_cast<size_t>(previousSelection.actionId) < actions.size())
    m_model.setSelectedAction(actions[previousSelection.actionId]);
}

//--------------------------------------------------------------------------------------------------
// QInputDialog wrappers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts for a new title and applies it to the group at @p groupId.
 */
void DataModel::ProjectEntities::promptRenameGroup(int groupId)
{
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  bool ok          = false;
  const auto old   = groups[groupId].title;
  const auto fresh = QInputDialog::getText(nullptr,
                                           ProjectModel::tr("Rename Group"),
                                           ProjectModel::tr("Name:"),
                                           QLineEdit::Normal,
                                           old,
                                           &ok)
                       .trimmed();
  if (!ok || fresh.isEmpty() || fresh == old)
    return;

  auto group  = groups[groupId];
  group.title = fresh;
  updateGroup(groupId, group, true);
}

/**
 * @brief Prompts for a new title and applies it to the dataset at (groupId, datasetId).
 */
void DataModel::ProjectEntities::promptRenameDataset(int groupId, int datasetId)
{
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return;

  bool ok          = false;
  const auto old   = groups[groupId].datasets[datasetId].title;
  const auto fresh = QInputDialog::getText(nullptr,
                                           ProjectModel::tr("Rename Dataset"),
                                           ProjectModel::tr("Name:"),
                                           QLineEdit::Normal,
                                           old,
                                           &ok)
                       .trimmed();
  if (!ok || fresh.isEmpty() || fresh == old)
    return;

  auto dataset  = groups[groupId].datasets[datasetId];
  dataset.title = fresh;
  updateDataset(groupId, datasetId, dataset, true);
}

/**
 * @brief Prompts for a new title and applies it to the action at @p actionId.
 */
void DataModel::ProjectEntities::promptRenameAction(int actionId)
{
  auto& actions = m_model.m_actions;
  if (actionId < 0 || static_cast<size_t>(actionId) >= actions.size())
    return;

  bool ok          = false;
  const auto old   = actions[actionId].title;
  const auto fresh = QInputDialog::getText(nullptr,
                                           ProjectModel::tr("Rename Action"),
                                           ProjectModel::tr("Name:"),
                                           QLineEdit::Normal,
                                           old,
                                           &ok)
                       .trimmed();
  if (!ok || fresh.isEmpty() || fresh == old)
    return;

  auto action  = actions[actionId];
  action.title = fresh;
  updateAction(actionId, action, true);
}
