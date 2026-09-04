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

#include <cmath>
#include <limits>
#include <memory>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <QTimer>

#include "Core/Checksum.h"
#include "Core/SSAssert.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/IconRegistry.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Publisher.h"
#  include "MQTT/PublisherScriptEditor.h"
#endif
#include "DataModel/Project/ProjectEditorIcons.h"
#include "DataModel/Project/ProjectFolders.h"
#include "ProjectEditorItemIds.h"

namespace DataModel {

/**
 * @brief Marks every group folder that owns at least one group, and every one that owns at least
 *        one ENABLED group, by walking each group's ancestor-folder chain. Feeds derived folder
 *        dimming in the tree (a folder shows disabled only when all its groups are disabled).
 */
static void accumulateFolderEnabled(const std::vector<DataModel::GroupFolder>& folders,
                                    const std::vector<DataModel::Group>& groups,
                                    QHash<int, bool>& hasGroup,
                                    QHash<int, bool>& hasEnabled)
{
  QHash<int, int> parentOf;
  for (const auto& f : folders)
    parentOf.insert(f.folderId, f.parentFolderId);

  const int kMax = static_cast<int>(folders.size());
  for (const auto& g : groups) {
    int cur = g.parentFolderId;
    for (int i = 0; i <= kMax && cur != -1; ++i) {
      hasGroup[cur] = true;
      if (g.enabled)
        hasEnabled[cur] = true;

      cur = parentOf.value(cur, -1);
    }
  }
}

/**
 * @brief Returns the first tree item in @p map whose mapped value satisfies @p pred.
 */
template<typename Map, typename Pred>
static QStandardItem* findMappedItem(const Map& map, Pred&& pred)
{
  for (auto it = map.begin(); it != map.end(); ++it)
    if (pred(it.value()))
      return it.key();

  return nullptr;
}

/**
 * @brief Writes TreeViewExpanded across a subtree: every foldable node shallower than
 *        @p maxExpandDepth is opened, deeper ones closed; recursion is depth-capped so a
 *        malformed model can never loop unbounded (NASA Power of Ten).
 */
static void applyTreeExpansion(QStandardItem* item, int depth, int maxExpandDepth)
{
  SS_ASSERT(item != nullptr, return);
  if (depth > 64)
    return;

  if (item->hasChildren())
    item->setData(depth < maxExpandDepth, ProjectEditor::TreeViewExpanded);

  const int rows = item->rowCount();
  for (int r = 0; r < rows; ++r)
    applyTreeExpansion(item->child(r), depth + 1, maxExpandDepth);
}

}  // namespace DataModel

//--------------------------------------------------------------------------------------------------
// Model builders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Coalesces rapid ProjectModel mutation bursts into a single rebuild.
 */
void DataModel::ProjectEditor::scheduleTreeRebuild()
{
  if (!m_rebuildTimer.isActive())
    m_rebuildTimer.start();
}

/**
 * @brief Rebuilds the project-structure tree, restoring expansion and selection.
 */
void DataModel::ProjectEditor::buildTreeModel()
{
  m_rootItems.clear();
  m_groupItems.clear();
  m_sourceItems.clear();
  m_actionItems.clear();
  m_datasetItems.clear();
  m_outputWidgetItems.clear();
  m_sourceParserItems.clear();
  m_userTableItems.clear();
  m_groupFolderItems.clear();
  m_tableFolderItems.clear();
  m_workspaceItems.clear();
  m_workspaceFolderItems.clear();
  m_groupsRootItem     = nullptr;
  m_tablesRootItem     = nullptr;
  m_systemDatasetsItem = nullptr;
  m_workspacesRootItem = nullptr;
  m_mqttPublisherItem  = nullptr;
  m_influxSinkItem     = nullptr;
  m_controlScriptItem  = nullptr;

  const bool seeding      = m_seedExpansionFromModel;
  const bool filterActive = !m_treeSearchQuery.trimmed().isEmpty();
  QHash<QString, bool> expandedStates;

  // code-verify off
  // Seed expansion only from the persisted map (the single source of truth, kept current by every
  // manual toggle via persistTreeExpansion), never from the live tree: this stops transient
  // force-expansion (search) and reveal (expandToIndex) from corrupting the saved state. A filtered
  // build is likewise never persisted back below, since its rows are force-expanded.
  // code-verify on
  const auto& persisted = m_projectModelRef.treeExpansion();
  for (auto it = persisted.constBegin(); it != persisted.constEnd(); ++it)
    expandedStates.insert(it.key(), it.value().toBool());

  m_seedExpansionFromModel = false;

  if (m_currentSelectionConnection) {
    QObject::disconnect(m_currentSelectionConnection);
    m_currentSelectionConnection = QMetaObject::Connection();
  }

  if (m_selectionModel) {
    m_selectionModel->deleteLater();
    m_selectionModel = nullptr;
  }

  if (m_treeModel) {
    m_treeModel->disconnect(this);
    m_treeModel->deleteLater();
    m_treeModel = nullptr;
  }

  m_treeModel = new CustomModel(this);

  static auto& registry = Misc::IconRegistry::instance();
  const auto& pm        = m_projectModelRef;
  auto* root            = new QStandardItem(pm.title());
  root->setData(root->text(), TreeViewText);
  root->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("project-setup"), 16),
                TreeViewIcon);
  root->setData(true, TreeViewExpanded);
  root->setData(KindProjectRoot, TreeItemKind);
  root->setData(-1, TreeItemId);
  root->setData(-1, TreeItemParentId);

  m_treeModel->appendRow(root);
  m_rootItems.insert(root, kRootItem);

  buildTreeItems(root, expandedStates);

  m_selectionModel             = new QItemSelectionModel(m_treeModel);
  m_currentSelectionConnection = connect(m_selectionModel,
                                         &QItemSelectionModel::currentChanged,
                                         this,
                                         &DataModel::ProjectEditor::onCurrentSelectionChanged);

  connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, [this] {
    onCurrentSelectionChanged(m_selectionModel->currentIndex(), QModelIndex());
  });

  Q_EMIT treeModelChanged();

  const auto revealIndex = consumePendingSelection();
  if (!revealIndex.isValid())
    restoreTreeSelection();

  if (!seeding && !filterActive)
    m_projectModelRef.setTreeExpansion(snapshotTreeExpansion());

  Q_EMIT treeRebuildFinished(revealIndex);
}

/**
 * @brief Selects a node queued by a deferred add-* signal handler.
 */
QModelIndex DataModel::ProjectEditor::consumePendingSelection()
{
  if (m_pendingSelectionKind == PendingSelectionKind::None || !m_selectionModel)
    return {};

  const auto kind = m_pendingSelectionKind;
  const auto gid  = m_pendingSelectionGroupId;
  const auto iid  = m_pendingSelectionItemId;

  m_pendingSelectionKind    = PendingSelectionKind::None;
  m_pendingSelectionGroupId = -1;
  m_pendingSelectionItemId  = -1;

  const auto pickFirst = [this](const auto& map, auto&& pred) -> QModelIndex {
    for (auto it = map.begin(); it != map.end(); ++it) {
      if (!pred(it.value()))
        continue;

      const auto idx = it.key()->index();
      m_selectionModel->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
      return idx;
    }
    return {};
  };

  if (kind == PendingSelectionKind::Source)
    return pickFirst(m_sourceItems, [iid](const auto& v) { return v.sourceId == iid; });

  if (kind == PendingSelectionKind::Group)
    return pickFirst(m_groupItems, [gid](const auto& v) { return v.groupId == gid; });

  if (kind == PendingSelectionKind::Dataset)
    return pickFirst(m_datasetItems,
                     [gid, iid](const auto& v) { return v.groupId == gid && v.datasetId == iid; });

  if (kind == PendingSelectionKind::OutputWidget)
    return pickFirst(m_outputWidgetItems,
                     [gid, iid](const auto& v) { return v.groupId == gid && v.widgetId == iid; });

  return {};
}

/**
 * @brief Appends source tree items with their frame-parser children. No-op when filtering.
 */
void DataModel::ProjectEditor::appendSourceTreeItems(QStandardItem* root)
{
  SS_ASSERT(root != nullptr, return);
  if (!m_treeSearchQuery.trimmed().isEmpty())
    return;

  const auto& sources    = m_projectModelRef.sources();
  const bool multiSource = sources.size() > 1;

  static auto& registry = Misc::IconRegistry::instance();
  for (const auto& source : sources) {
    auto* sourceItem = new QStandardItem(source.title);
    sourceItem->setData(-1, TreeViewFrameIndex);
    sourceItem->setData(busTypeIcon(source.busType), TreeViewIcon);
    sourceItem->setData(source.title, TreeViewText);
    sourceItem->setData(source.sourceId, TreeViewSourceId);
    sourceItem->setData(multiSource ? source.title : QString(), TreeViewSourceName);
    sourceItem->setData(true, TreeViewExpanded);
    sourceItem->setData(KindSource, TreeItemKind);
    sourceItem->setData(source.sourceId, TreeItemId);

    auto* parserItem = new QStandardItem(tr("Frame Parser"));
    parserItem->setData(-1, TreeViewFrameIndex);
    parserItem->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("code"), 16),
                        TreeViewIcon);
    parserItem->setData(tr("Frame Parser"), TreeViewText);
    parserItem->setData(KindFrameParser, TreeItemKind);
    parserItem->setData(source.sourceId, TreeItemId);
    parserItem->setData(source.sourceId, TreeItemParentId);
    sourceItem->appendRow(parserItem);
    m_sourceParserItems.insert(parserItem, source);

    root->appendRow(sourceItem);
    m_sourceItems.insert(sourceItem, source);
  }
}

/**
 * @brief Appends action tree items, filtered by the current search query.
 */
void DataModel::ProjectEditor::appendActionTreeItems(QStandardItem* root)
{
  SS_ASSERT(root != nullptr, return);

  const QString q         = m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  const auto& actions     = m_projectModelRef.actions();

  static auto& registry = Misc::IconRegistry::instance();
  for (const auto& action : actions) {
    if (filterActive && !SerialStudio::searchMatches(q, action.title))
      continue;

    auto* actionItem = new QStandardItem(action.title);
    actionItem->setData(-1, TreeViewFrameIndex);
    actionItem->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("action"), 16),
                        TreeViewIcon);
    actionItem->setData(action.title, TreeViewText);
    actionItem->setData(KindAction, TreeItemKind);
    actionItem->setData(action.actionId, TreeItemId);
    actionItem->setData(-1, TreeItemParentId);
    root->appendRow(actionItem);
    m_actionItems.insert(actionItem, action);
  }
}

/**
 * @brief Appends dataset children of a group, filtered by the current search query.
 */
void DataModel::ProjectEditor::appendDatasetChildren(QStandardItem* groupItem,
                                                     const DataModel::Group& group)
{
  SS_ASSERT(groupItem != nullptr, return);

  const QString q         = m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  const bool groupMatches = !filterActive || SerialStudio::searchMatches(q, group.title);

  static auto& registry = Misc::IconRegistry::instance();
  for (const auto& dataset : group.datasets) {
    if (filterActive && !groupMatches && !SerialStudio::searchMatches(q, dataset.title))
      continue;

    auto* datasetItem = new QStandardItem(dataset.title);
    auto widgets      = SerialStudio::getDashboardWidgets(dataset);
    QString dIcon     = registry.icon(QStringLiteral("editor"), QStringLiteral("dataset"), 16);
    if (widgets.count() > 0)
      dIcon = SerialStudio::dashboardWidgetIcon(widgets.first(), false);

    datasetItem->setData(dIcon, TreeViewIcon);
    datasetItem->setData(dataset.title, TreeViewText);
    datasetItem->setData(dataset.index, TreeViewFrameIndex);
    datasetItem->setData(dataset.sourceId, TreeViewSourceId);
    datasetItem->setData(QString(), TreeViewSourceName);
    datasetItem->setData(dataset.virtual_, TreeViewVirtual);
    datasetItem->setData(group.enabled && dataset.enabled, TreeViewEnabled);
    datasetItem->setData(dataset.enabled, TreeViewSelfEnabled);
    datasetItem->setData(KindDataset, TreeItemKind);
    datasetItem->setData(dataset.datasetId, TreeItemId);
    datasetItem->setData(group.groupId, TreeItemParentId);
    groupItem->appendRow(datasetItem);
    m_datasetItems.insert(datasetItem, dataset);
  }
}

/**
 * @brief Appends output-widget children of a group. Skipped entirely when filtering.
 */
void DataModel::ProjectEditor::appendOutputWidgetChildren(QStandardItem* groupItem,
                                                          const DataModel::Group& group)
{
  SS_ASSERT(groupItem != nullptr, return);
  if (!m_treeSearchQuery.trimmed().isEmpty())
    return;

  static auto& registry = Misc::IconRegistry::instance();
  for (const auto& ow : group.outputWidgets) {
    auto* owItem = new QStandardItem(ow.title);

    QString owIcon;
    switch (ow.type) {
      case DataModel::OutputWidgetType::Button:
        owIcon = registry.icon(QStringLiteral("editor"), QStringLiteral("output-button"), 16);
        break;
      case DataModel::OutputWidgetType::Slider:
        owIcon = registry.icon(QStringLiteral("editor"), QStringLiteral("output-slider"), 16);
        break;
      case DataModel::OutputWidgetType::Toggle:
        owIcon = registry.icon(QStringLiteral("editor"), QStringLiteral("output-toggle"), 16);
        break;
      case DataModel::OutputWidgetType::TextField:
        owIcon = registry.icon(QStringLiteral("editor"), QStringLiteral("output-textfield"), 16);
        break;
      case DataModel::OutputWidgetType::Knob:
        owIcon = registry.icon(QStringLiteral("editor"), QStringLiteral("output-knob"), 16);
        break;
      default:
        owIcon = registry.icon(QStringLiteral("editor"), QStringLiteral("widget"), 16);
        break;
    }

    owItem->setData(owIcon, TreeViewIcon);
    owItem->setData(ow.title, TreeViewText);
    owItem->setData(-2, TreeViewFrameIndex);
    owItem->setData(ow.sourceId, TreeViewSourceId);
    owItem->setData(QString(), TreeViewSourceName);
    owItem->setData(group.enabled, TreeViewEnabled);
    owItem->setData(group.enabled, TreeViewSelfEnabled);
    owItem->setData(KindOutputWidget, TreeItemKind);
    owItem->setData(ow.widgetId, TreeItemId);
    owItem->setData(group.groupId, TreeItemParentId);
    groupItem->appendRow(owItem);
    m_outputWidgetItems.insert(owItem, ow);
  }
}

/**
 * @brief Appends the Groups subtree (groups, datasets, output widgets) into root.
 */
void DataModel::ProjectEditor::appendGroupTreeItems(QStandardItem* root,
                                                    QHash<QString, bool>& expandedStates)
{
  SS_ASSERT(root != nullptr, return);

  const QString q         = m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  const auto matches      = [&q](const QString& s) {
    return SerialStudio::searchMatches(q, s);
  };

  const auto groupFilteredOut = [&](const DataModel::Group& g, bool groupTitleMatches) {
    if (!filterActive || groupTitleMatches)
      return false;

    for (const auto& ds : g.datasets)
      if (matches(ds.title))
        return false;

    return true;
  };

  const auto& groups     = m_projectModelRef.groups();
  const auto& folders    = m_projectModelRef.editorGroupFolders();
  const bool showFolders = !filterActive && !folders.empty();

  bool anyGroup = false;
  for (const auto& group : groups) {
    const bool groupMatches = !filterActive || matches(group.title);
    if (!groupFilteredOut(group, groupMatches)) {
      anyGroup = true;
      break;
    }
  }

  if (!anyGroup && !showFolders)
    return;

  static auto& registry = Misc::IconRegistry::instance();
  auto* groupsRoot      = new QStandardItem(tr("Dashboard Widgets"));
  groupsRoot->setData(tr("Dashboard Widgets"), TreeViewText);
  groupsRoot->setData(
    registry.icon(QStringLiteral("editor"), QStringLiteral("dashboard-widgets"), 16), TreeViewIcon);
  groupsRoot->setData(-1, TreeViewFrameIndex);
  groupsRoot->setData(true, TreeViewExpanded);
  groupsRoot->setData(KindGroupsRoot, TreeItemKind);
  groupsRoot->setData(-1, TreeItemId);
  groupsRoot->setData(-1, TreeItemParentId);

  QHash<int, QStandardItem*> folderItems;
  if (showFolders)
    folderItems =
      appendGroupFolderItems(groupsRoot, root->text() + "/" + groupsRoot->text(), expandedStates);

  for (const auto& group : groups) {
    const bool groupMatches = !filterActive || matches(group.title);
    if (groupFilteredOut(group, groupMatches))
      continue;

    auto* groupItem = new QStandardItem(group.title);
    auto icon = SerialStudio::dashboardWidgetIcon(SerialStudio::getDashboardWidget(group), false);

    groupItem->setData(icon, TreeViewIcon);
    groupItem->setData(-1, TreeViewFrameIndex);
    groupItem->setData(group.title, TreeViewText);
    groupItem->setData(QString(), TreeViewSourceName);
    groupItem->setData(group.sourceId, TreeViewSourceId);
    groupItem->setData(group.enabled, TreeViewEnabled);
    groupItem->setData(group.enabled, TreeViewSelfEnabled);
    groupItem->setData(KindGroup, TreeItemKind);
    groupItem->setData(group.groupId, TreeItemId);
    groupItem->setData(group.parentFolderId, TreeItemParentId);

    if (filterActive)
      groupItem->setData(true, TreeViewExpanded);

    appendDatasetChildren(groupItem, group);
    appendOutputWidgetChildren(groupItem, group);

    QString gPath = root->text() + "/" + groupsRoot->text();
    if (showFolders && group.parentFolderId != -1 && folderItems.contains(group.parentFolderId))
      gPath += "/" + folderDisplayPath(folders, group.parentFolderId);

    gPath += "/" + group.title;
    if (!filterActive)
      restoreExpandedStateMap(groupItem, expandedStates, gPath);

    QStandardItem* parent =
      (showFolders && group.parentFolderId != -1 && folderItems.contains(group.parentFolderId))
        ? folderItems.value(group.parentFolderId)
        : groupsRoot;
    parent->appendRow(groupItem);
    m_groupItems.insert(groupItem, group);
  }

  if (!filterActive)
    restoreExpandedStateMap(groupsRoot, expandedStates, root->text() + "/" + groupsRoot->text());

  root->appendRow(groupsRoot);
  m_groupsRootItem = groupsRoot;
}

/**
 * @brief Materializes the group folder items and re-parents them; returns the id->item map.
 */
QHash<int, QStandardItem*> DataModel::ProjectEditor::appendGroupFolderItems(
  QStandardItem* groupsRoot, const QString& pathPrefix, QHash<QString, bool>& expandedStates)
{
  const auto& folders = m_projectModelRef.editorGroupFolders();
  const auto& groups  = m_projectModelRef.groups();

  QHash<int, bool> folderHasGroup;
  QHash<int, bool> folderHasEnabled;
  accumulateFolderEnabled(folders, groups, folderHasGroup, folderHasEnabled);

  static auto& registry = Misc::IconRegistry::instance();
  QHash<int, QStandardItem*> folderItems;
  for (const auto& f : folders) {
    const bool folderEnabled =
      !folderHasGroup.value(f.folderId, false) || folderHasEnabled.value(f.folderId, false);

    auto* item = new QStandardItem(f.title);
    item->setData(f.title, TreeViewText);
    item->setData(registry.icon(QStringLiteral("widgets"), QStringLiteral("folder"), 16),
                  TreeViewIcon);
    item->setData(-1, TreeViewFrameIndex);
    item->setData(KindGroupFolder, TreeItemKind);
    item->setData(f.folderId, TreeItemId);
    item->setData(f.parentFolderId, TreeItemParentId);
    item->setData(folderEnabled, TreeViewEnabled);
    item->setData(folderEnabled, TreeViewSelfEnabled);

    const QString fPath = pathPrefix + QStringLiteral("/") + folderDisplayPath(folders, f.folderId);
    restoreExpandedStateMap(item, expandedStates, fPath);
    folderItems.insert(f.folderId, item);
  }

  for (const auto& f : folders) {
    auto* item            = folderItems.value(f.folderId);
    QStandardItem* parent = (f.parentFolderId != -1 && folderItems.contains(f.parentFolderId))
                            ? folderItems.value(f.parentFolderId)
                            : groupsRoot;
    parent->appendRow(item);
    m_groupFolderItems.insert(item, f.folderId);
  }

  return folderItems;
}

/**
 * @brief Appends the "Variables" subtree (Pro only) with system + user tables.
 */
void DataModel::ProjectEditor::appendSharedMemoryTreeItems(QStandardItem* root,
                                                           QHash<QString, bool>& expandedStates)
{
  SS_ASSERT(root != nullptr, return);

#ifdef BUILD_COMMERCIAL
  static auto& registry   = Misc::IconRegistry::instance();
  const QString q         = m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  const auto matches      = [&q](const QString& s) {
    return SerialStudio::searchMatches(q, s);
  };

  const auto& userTables = m_projectModelRef.tables();
  bool includeSharedRoot =
    !filterActive || matches(tr("Variables")) || matches(tr("Dataset Values"));
  if (!includeSharedRoot) {
    for (const auto& t : userTables) {
      if (matches(t.name)) {
        includeSharedRoot = true;
        break;
      }
    }
  }

  if (!includeSharedRoot)
    return;

  auto* tablesRoot = new QStandardItem(tr("Variables"));
  tablesRoot->setData(tr("Variables"), TreeViewText);
  tablesRoot->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("shared-memory"), 16),
                      TreeViewIcon);
  tablesRoot->setData(-1, TreeViewFrameIndex);
  tablesRoot->setData(true, TreeViewExpanded);
  tablesRoot->setData(KindTablesRoot, TreeItemKind);
  tablesRoot->setData(-1, TreeItemId);
  tablesRoot->setData(-1, TreeItemParentId);

  auto* sysDsItem = new QStandardItem(tr("Dataset Values"));
  sysDsItem->setData(tr("Dataset Values"), TreeViewText);
  sysDsItem->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("dataset-values"), 16),
                     TreeViewIcon);
  sysDsItem->setData(-1, TreeViewFrameIndex);
  sysDsItem->setData(KindSystemDatasets, TreeItemKind);
  sysDsItem->setData(-1, TreeItemId);
  sysDsItem->setData(-1, TreeItemParentId);
  tablesRoot->appendRow(sysDsItem);

  const auto& tableFolders    = m_projectModelRef.editorTableFolders();
  const bool showTableFolders = !filterActive && !tableFolders.empty();

  QHash<int, QStandardItem*> folderItems;
  if (showTableFolders)
    folderItems =
      appendTableFolderItems(tablesRoot, root->text() + "/" + tr("Variables"), expandedStates);

  for (const auto& table : userTables) {
    if (filterActive && !matches(table.name))
      continue;

    const QString path = DataModel::tableFullPath(tableFolders, table.parentFolderId, table.name);

    auto* tableItem = new QStandardItem(table.name);
    tableItem->setData(table.name, TreeViewText);
    tableItem->setData(
      registry.icon(QStringLiteral("editor"), QStringLiteral("shared-table-alt"), 16),
      TreeViewIcon);
    tableItem->setData(-1, TreeViewFrameIndex);
    tableItem->setData(KindUserTable, TreeItemKind);
    tableItem->setData(-1, TreeItemId);
    tableItem->setData(table.parentFolderId, TreeItemParentId);
    tableItem->setData(path, TreeItemPath);

    QStandardItem* parent =
      (showTableFolders && table.parentFolderId != -1 && folderItems.contains(table.parentFolderId))
        ? folderItems.value(table.parentFolderId)
        : tablesRoot;
    parent->appendRow(tableItem);
    m_userTableItems.insert(tableItem, path);
  }

  restoreExpandedStateMap(tablesRoot, expandedStates, root->text() + "/" + tablesRoot->text());
  root->appendRow(tablesRoot);
  m_tablesRootItem     = tablesRoot;
  m_systemDatasetsItem = sysDsItem;
#else
  Q_UNUSED(root);
  Q_UNUSED(expandedStates);
#endif
}

/**
 * @brief Materializes the table folder items and re-parents them; returns the id->item map.
 */
QHash<int, QStandardItem*> DataModel::ProjectEditor::appendTableFolderItems(
  QStandardItem* tablesRoot, const QString& pathPrefix, QHash<QString, bool>& expandedStates)
{
  const auto& folders = m_projectModelRef.editorTableFolders();

  static auto& registry = Misc::IconRegistry::instance();
  QHash<int, QStandardItem*> folderItems;
  for (const auto& f : folders) {
    auto* item = new QStandardItem(f.title);
    item->setData(f.title, TreeViewText);
    item->setData(registry.icon(QStringLiteral("widgets"), QStringLiteral("folder"), 16),
                  TreeViewIcon);
    item->setData(-1, TreeViewFrameIndex);
    item->setData(KindTableFolder, TreeItemKind);
    item->setData(f.folderId, TreeItemId);
    item->setData(f.parentFolderId, TreeItemParentId);

    const QString fPath = pathPrefix + QStringLiteral("/") + folderDisplayPath(folders, f.folderId);
    restoreExpandedStateMap(item, expandedStates, fPath);
    folderItems.insert(f.folderId, item);
  }

  for (const auto& f : folders) {
    auto* item            = folderItems.value(f.folderId);
    QStandardItem* parent = (f.parentFolderId != -1 && folderItems.contains(f.parentFolderId))
                            ? folderItems.value(f.parentFolderId)
                            : tablesRoot;
    parent->appendRow(item);
    m_tableFolderItems.insert(item, f.folderId);
  }

  return folderItems;
}

/**
 * @brief Builds a workspace tree item from a workspace record.
 */
QStandardItem* DataModel::ProjectEditor::createWorkspaceItem(const DataModel::Workspace& ws)
{
  static auto& registry = Misc::IconRegistry::instance();
  auto* wsItem          = new QStandardItem(ws.title);
  wsItem->setData(ws.title, TreeViewText);
  wsItem->setData(ws.icon.isEmpty()
                    ? registry.icon(QStringLiteral("widgets"), QStringLiteral("workspace"), 16)
                    : Misc::IconEngine::resolveActionIconSource(ws.icon),
                  TreeViewIcon);
  wsItem->setData(-1, TreeViewFrameIndex);
  wsItem->setData(KindWorkspace, TreeItemKind);
  wsItem->setData(ws.workspaceId, TreeItemId);
  wsItem->setData(ws.parentFolderId, TreeItemParentId);
  wsItem->setData(workspaceHasUnresolvedRefs(ws.workspaceId), TreeViewWorkspaceStale);
  return wsItem;
}

/**
 * @brief Builds the folder hierarchy under the Workspaces root (folders first, then workspaces).
 */
void DataModel::ProjectEditor::buildWorkspaceFolderTree(QStandardItem* wsRoot,
                                                        const QString& pathPrefix,
                                                        QHash<QString, bool>& expandedStates)
{
  SS_ASSERT(wsRoot != nullptr, return);

  const auto& folders    = m_projectModelRef.editorWorkspaceFolders();
  const auto& workspaces = m_projectModelRef.editorWorkspaces();

  static auto& registry = Misc::IconRegistry::instance();
  QHash<int, QStandardItem*> folderItems;
  for (const auto& f : folders) {
    auto* item = new QStandardItem(f.title);
    item->setData(f.title, TreeViewText);
    item->setData(registry.icon(QStringLiteral("widgets"), QStringLiteral("folder"), 16),
                  TreeViewIcon);
    item->setData(-1, TreeViewFrameIndex);
    item->setData(KindWorkspaceFolder, TreeItemKind);
    item->setData(f.folderId, TreeItemId);
    item->setData(f.parentFolderId, TreeItemParentId);

    const QString fPath = pathPrefix + QStringLiteral("/") + folderDisplayPath(folders, f.folderId);
    restoreExpandedStateMap(item, expandedStates, fPath);
    folderItems.insert(f.folderId, item);
  }

  for (const auto& f : folders) {
    auto* item            = folderItems.value(f.folderId);
    QStandardItem* parent = (f.parentFolderId != -1 && folderItems.contains(f.parentFolderId))
                            ? folderItems.value(f.parentFolderId)
                            : wsRoot;
    parent->appendRow(item);
    m_workspaceFolderItems.insert(item, f.folderId);
  }

  for (const auto& ws : workspaces) {
    auto* wsItem          = createWorkspaceItem(ws);
    QStandardItem* parent = (ws.parentFolderId != -1 && folderItems.contains(ws.parentFolderId))
                            ? folderItems.value(ws.parentFolderId)
                            : wsRoot;
    parent->appendRow(wsItem);
    m_workspaceItems.insert(wsItem, ws.workspaceId);
  }
}

/**
 * @brief Appends the "Workspaces" subtree, filtered by the current search query.
 */
void DataModel::ProjectEditor::appendWorkspaceTreeItems(QStandardItem* root,
                                                        QHash<QString, bool>& expandedStates)
{
  SS_ASSERT(root != nullptr, return);

  const QString q         = m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  const auto matches      = [&q](const QString& s) {
    return SerialStudio::searchMatches(q, s);
  };

  const auto& workspaces = m_projectModelRef.editorWorkspaces();
  bool includeWorkspaces = !filterActive || matches(tr("Workspaces"));
  if (!includeWorkspaces) {
    for (const auto& ws : workspaces) {
      if (matches(ws.title)) {
        includeWorkspaces = true;
        break;
      }
    }
  }

  if (!includeWorkspaces)
    return;

  static auto& registry = Misc::IconRegistry::instance();
  auto* wsRoot          = new QStandardItem(tr("Workspaces"));
  wsRoot->setData(tr("Workspaces"), TreeViewText);
  wsRoot->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("workspace"), 16),
                  TreeViewIcon);
  wsRoot->setData(-1, TreeViewFrameIndex);
  wsRoot->setData(true, TreeViewExpanded);
  wsRoot->setData(KindWorkspacesRoot, TreeItemKind);
  wsRoot->setData(-1, TreeItemId);
  wsRoot->setData(-1, TreeItemParentId);

  if (filterActive) {
    for (const auto& ws : workspaces) {
      if (!matches(ws.title))
        continue;

      auto* wsItem = createWorkspaceItem(ws);
      wsRoot->appendRow(wsItem);
      m_workspaceItems.insert(wsItem, ws.workspaceId);
    }
  } else {
    buildWorkspaceFolderTree(wsRoot, root->text() + "/" + tr("Workspaces"), expandedStates);
  }

  restoreExpandedStateMap(wsRoot, expandedStates, root->text() + "/" + wsRoot->text());
  root->appendRow(wsRoot);
  m_workspacesRootItem = wsRoot;
}

/**
 * @brief Appends the single-instance "MQTT Publisher" node (Pro) to the project tree.
 */
void DataModel::ProjectEditor::appendMqttPublisherTreeItem(QStandardItem* root)
{
  SS_ASSERT(root != nullptr, return);

  const QString q         = m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  if (filterActive && !SerialStudio::searchMatches(q, tr("MQTT Publisher")))
    return;

  static auto& registry = Misc::IconRegistry::instance();
  auto* item            = new QStandardItem(tr("MQTT Publisher"));
  item->setData(tr("MQTT Publisher"), TreeViewText);
  item->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("mqtt-publisher"), 16),
                TreeViewIcon);
  item->setData(-1, TreeViewFrameIndex);
  item->setData(KindMqttPublisher, TreeItemKind);
  item->setData(-1, TreeItemId);
  item->setData(-1, TreeItemParentId);

  root->appendRow(item);
  m_mqttPublisherItem = item;
}

/**
 * @brief Appends the single InfluxDB sink node to the tree.
 */
void DataModel::ProjectEditor::appendInfluxSinkTreeItem(QStandardItem* root)
{
  SS_ASSERT(root != nullptr, return);

  const QString q         = m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  if (filterActive && !SerialStudio::searchMatches(q, tr("InfluxDB Sink")))
    return;

  static auto& registry = Misc::IconRegistry::instance();
  auto* item            = new QStandardItem(tr("InfluxDB Sink"));
  item->setData(tr("InfluxDB Sink"), TreeViewText);
  item->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("influx"), 16),
                TreeViewIcon);
  item->setData(-1, TreeViewFrameIndex);
  item->setData(KindInfluxSink, TreeItemKind);
  item->setData(-1, TreeItemId);
  item->setData(-1, TreeItemParentId);

  root->appendRow(item);
  m_influxSinkItem = item;
}

/**
 * @brief Appends the project-global control-script node to the tree.
 */
void DataModel::ProjectEditor::appendControlScriptTreeItem(QStandardItem* root)
{
  SS_ASSERT(root != nullptr, return);

  const QString q         = m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  if (filterActive && !SerialStudio::searchMatches(q, tr("Control Loop")))
    return;

  static auto& registry = Misc::IconRegistry::instance();
  auto* item            = new QStandardItem(tr("Control Loop"));
  item->setData(tr("Control Loop"), TreeViewText);
  item->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("control-script"), 16),
                TreeViewIcon);
  item->setData(-1, TreeViewFrameIndex);
  item->setData(KindControlScript, TreeItemKind);
  item->setData(-1, TreeItemId);
  item->setData(-1, TreeItemParentId);

  root->appendRow(item);
  m_controlScriptItem = item;
}

/**
 * @brief Populates the tree under root with sources, actions, groups, datasets.
 */
void DataModel::ProjectEditor::buildTreeItems(QStandardItem* root,
                                              QHash<QString, bool>& expandedStates)
{
  SS_ASSERT(root != nullptr, return);

  appendControlScriptTreeItem(root);
#ifdef BUILD_COMMERCIAL
  appendMqttPublisherTreeItem(root);
  appendInfluxSinkTreeItem(root);
#endif

  appendActionTreeItems(root);
  appendSourceTreeItems(root);
  appendGroupTreeItems(root, expandedStates);
  appendSharedMemoryTreeItems(root, expandedStates);
  appendWorkspaceTreeItems(root, expandedStates);

  auto* spacer = new QStandardItem(" ");
  spacer->setData(" ", TreeViewText);
  spacer->setData("", TreeViewIcon);
  spacer->setData(-1, TreeViewFrameIndex);
  spacer->setEnabled(false);
  spacer->setSelectable(false);
  root->appendRow(spacer);
}

/**
 * @brief Resolves the tree item for ID-keyed entity views (dataset, group, action, source,
 *        output widget, frame parser); returns nullptr when another view is active.
 */
QStandardItem* DataModel::ProjectEditor::entitySelectionItem() const
{
  if (m_currentView == DatasetView) {
    const auto gid = m_selectedDataset.groupId;
    const auto did = m_selectedDataset.datasetId;
    return findMappedItem(
      m_datasetItems, [gid, did](const auto& v) { return v.groupId == gid && v.datasetId == did; });
  }

  if (m_currentView == GroupView) {
    const auto gid = m_selectedGroup.groupId;
    return findMappedItem(m_groupItems, [gid](const auto& v) { return v.groupId == gid; });
  }

  if (m_currentView == ActionView) {
    const auto aid = m_selectedAction.actionId;
    return findMappedItem(m_actionItems, [aid](const auto& v) { return v.actionId == aid; });
  }

  if (m_currentView == SourceView) {
    const auto sid = m_selectedSource.sourceId;
    return findMappedItem(m_sourceItems, [sid](const auto& v) { return v.sourceId == sid; });
  }

  if (m_currentView == OutputWidgetView) {
    const auto gid = m_selectedOutputWidget.groupId;
    const auto wid = m_selectedOutputWidget.widgetId;
    return findMappedItem(m_outputWidgetItems, [gid, wid](const auto& v) {
      return v.groupId == gid && v.widgetId == wid;
    });
  }

  if (m_currentView == SourceFrameParserView) {
    const auto sid = m_selectedSource.sourceId;
    return findMappedItem(m_sourceParserItems, [sid](const auto& v) { return v.sourceId == sid; });
  }

  return nullptr;
}

/**
 * @brief Resolves the tree item for container and singleton views (tables, workspaces, folders,
 *        MQTT publisher, control loop), falling back to the owning subtree root when the
 *        selected entry no longer exists; returns nullptr when another view is active.
 */
QStandardItem* DataModel::ProjectEditor::containerSelectionItem() const
{
  if (m_currentView == DataTablesView)
    return m_tablesRootItem;

  if (m_currentView == SystemDatasetsView)
    return m_systemDatasetsItem;

  if (m_currentView == UserTableView) {
    const auto name = m_selectedUserTable;
    auto* item = findMappedItem(m_userTableItems, [&name](const auto& v) { return v == name; });
    return item ? item : m_tablesRootItem;
  }

  if (m_currentView == WorkspacesView)
    return m_workspacesRootItem;

  if (m_currentView == WorkspaceView) {
    const int wid = m_selectedWorkspaceId;
    auto* item    = findMappedItem(m_workspaceItems, [wid](const auto& v) { return v == wid; });
    return item ? item : m_workspacesRootItem;
  }

  if (m_currentView == GroupsView)
    return m_groupsRootItem;

  if (m_currentView == GroupFolderView) {
    const int fid = m_selectedGroupFolderId;
    auto* item    = findMappedItem(m_groupFolderItems, [fid](const auto& v) { return v == fid; });
    return item ? item : m_groupsRootItem;
  }

  if (m_currentView == TableFolderView) {
    const int fid = m_selectedTableFolderId;
    auto* item    = findMappedItem(m_tableFolderItems, [fid](const auto& v) { return v == fid; });
    return item ? item : m_tablesRootItem;
  }

  if (m_currentView == WorkspaceFolderView) {
    const int fid = m_selectedFolderId;
    auto* item = findMappedItem(m_workspaceFolderItems, [fid](const auto& v) { return v == fid; });
    return item ? item : m_workspacesRootItem;
  }

  if (m_currentView == MqttPublisherView)
    return m_mqttPublisherItem;

  if (m_currentView == InfluxSinkView)
    return m_influxSinkItem;

  if (m_currentView == ControlScriptView)
    return m_controlScriptItem;

  return nullptr;
}

/**
 * @brief Restores the tree selection by matching IDs against the current view.
 */
void DataModel::ProjectEditor::restoreTreeSelection()
{
  QStandardItem* toSelect = entitySelectionItem();
  if (!toSelect)
    toSelect = containerSelectionItem();

  if (!toSelect)
    toSelect = findMappedItem(m_rootItems, [](const auto& v) { return v == kRootItem; });

  if (toSelect)
    m_selectionModel->setCurrentIndex(toSelect->index(), QItemSelectionModel::ClearAndSelect);
}

//--------------------------------------------------------------------------------------------------
// Expansion state maps
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records TreeViewExpanded state of the subtree into a path-keyed map.
 */
void DataModel::ProjectEditor::saveExpandedStateMap(QStandardItem* item,
                                                    QHash<QString, bool>& map,
                                                    const QString& title)
{
  if (!item)
    return;

  map[title] = item->data(TreeViewExpanded).toBool();

  for (auto i = 0; i < item->rowCount(); ++i) {
    QStandardItem* child = item->child(i);
    auto childT          = title.isEmpty() ? child->text() : title + "/" + child->text();
    saveExpandedStateMap(child, map, childT);
  }
}

/**
 * @brief Restores TreeViewExpanded state from the saved path-keyed map.
 */
void DataModel::ProjectEditor::restoreExpandedStateMap(QStandardItem* item,
                                                       QHash<QString, bool>& map,
                                                       const QString& title,
                                                       bool defaultExpanded)
{
  if (!item)
    return;

  if (map.contains(title))
    item->setData(map[title], TreeViewExpanded);
  else
    item->setData(defaultExpanded, TreeViewExpanded);
}

//--------------------------------------------------------------------------------------------------
// Expansion snapshot persistence
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes the live tree's expansion state into a path-keyed JSON object.
 */
QJsonObject DataModel::ProjectEditor::snapshotTreeExpansion()
{
  QJsonObject obj;
  if (!m_treeModel)
    return obj;

  QHash<QString, bool> states;
  saveExpandedStateMap(m_treeModel->invisibleRootItem(), states, "");
  for (auto it = states.constBegin(); it != states.constEnd(); ++it)
    obj.insert(it.key(), it.value());

  return obj;
}

/**
 * @brief Pushes the current tree expansion into the project model (e.g. after a manual toggle).
 */
void DataModel::ProjectEditor::persistTreeExpansion()
{
  m_projectModelRef.setTreeExpansion(snapshotTreeExpansion());
}

//--------------------------------------------------------------------------------------------------
// Navigation-driven expansion: model-role authority, the view follows via syncExpandedState
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks every ancestor of @p index expanded so the item is revealed. Writes the model role
 *        (the view's source of truth) rather than the view directly, so the reveal survives reuse.
 */
void DataModel::ProjectEditor::expandTreeToIndex(const QModelIndex& index)
{
  if (!m_treeModel)
    return;

  const auto* item = m_treeModel->itemFromIndex(index);
  if (!item)
    return;

  for (auto* p = item->parent(); p != nullptr; p = p->parent())
    p->setData(true, TreeViewExpanded);
}

/**
 * @brief Sets the expanded model role of @p index; the delegate reacts and expands/collapses the
 * row.
 */
void DataModel::ProjectEditor::setTreeIndexExpanded(const QModelIndex& index, bool expanded)
{
  if (!m_treeModel)
    return;

  if (auto* item = m_treeModel->itemFromIndex(index))
    item->setData(expanded, TreeViewExpanded);
}

/**
 * @brief Opens every foldable node in the tree; the view follows the model role per delegate.
 */
void DataModel::ProjectEditor::expandAllTreeItems()
{
  if (!m_treeModel)
    return;

  auto* root = m_treeModel->invisibleRootItem();
  SS_ASSERT(root != nullptr, return);
  for (int r = 0; r < root->rowCount(); ++r)
    applyTreeExpansion(root->child(r), 0, std::numeric_limits<int>::max());

  persistTreeExpansion();
}

/**
 * @brief Collapses the tree to its overview: the root, top-level categories, and each category's
 *        direct items stay visible (depth < 2 open); everything deeper folds away.
 */
void DataModel::ProjectEditor::collapseTreeToOverview()
{
  if (!m_treeModel)
    return;

  auto* root = m_treeModel->invisibleRootItem();
  SS_ASSERT(root != nullptr, return);
  for (int r = 0; r < root->rowCount(); ++r)
    applyTreeExpansion(root->child(r), 0, 2);

  persistTreeExpansion();
}

/**
 * @brief True when the tree item at @p index has child rows (a foldable node).
 */
bool DataModel::ProjectEditor::treeIndexHasChildren(const QModelIndex& index) const
{
  const auto* item = m_treeModel ? m_treeModel->itemFromIndex(index) : nullptr;
  return item && item->hasChildren();
}

/**
 * @brief True when the tree item at @p index is currently marked expanded in the model.
 */
bool DataModel::ProjectEditor::treeIndexExpanded(const QModelIndex& index) const
{
  const auto* item = m_treeModel ? m_treeModel->itemFromIndex(index) : nullptr;
  return item && item->data(TreeViewExpanded).toBool();
}
