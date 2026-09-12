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

#include "ProjectEditor/EditorTree.h"

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
#include "Core/IconRegistry.h"
#include "Core/SerialStudio.h"
#include "Core/Services.h"
#include "Core/SSAssert.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/Project/ProjectFolders.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/WidgetResolution.h"
#include "Misc/IconEngine.h"
#include "ProjectEditor/ProjectEditor.h"
#include "ProjectEditor/ProjectEditorIcons.h"
#include "ProjectEditorItemIds.h"

namespace DataModel {

using enum ProjectEditor::CustomRoles;
using enum ProjectEditor::CurrentView;

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the facade and the model and arms the zero-interval single-shot timer that
 *        coalesces a burst of model mutations into one tree rebuild.
 */
EditorTree::EditorTree(ProjectEditor& editor, ProjectModel& model)
  : m_editor(editor), m_model(model)
{
  m_rebuildTimer.setSingleShot(true);
  m_rebuildTimer.setInterval(0);
  QObject::connect(&m_rebuildTimer, &QTimer::timeout, &m_editor, [this] { buildTreeModel(); });
}

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

//--------------------------------------------------------------------------------------------------
// Model builders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Coalesces rapid ProjectModel mutation bursts into a single rebuild.
 */
void EditorTree::scheduleTreeRebuild()
{
  if (!m_rebuildTimer.isActive())
    m_rebuildTimer.start();
}

/**
 * @brief Forgets every item the previous build handed out; the tree is about to be rebuilt.
 */
void EditorTree::clearItemMaps()
{
  m_editor.m_rootItems.clear();
  m_editor.m_groupItems.clear();
  m_editor.m_sourceItems.clear();
  m_editor.m_actionItems.clear();
  m_editor.m_datasetItems.clear();
  m_editor.m_outputWidgetItems.clear();
  m_editor.m_sourceParserItems.clear();
  m_editor.m_userTableItems.clear();
  m_editor.m_groupFolderItems.clear();
  m_editor.m_tableFolderItems.clear();
  m_editor.m_workspaceItems.clear();
  m_editor.m_workspaceFolderItems.clear();
  m_editor.m_groupsRootItem       = nullptr;
  m_editor.m_tablesRootItem       = nullptr;
  m_editor.m_systemDatasetsItem   = nullptr;
  m_editor.m_workspacesRootItem   = nullptr;
  m_editor.m_mqttPublisherItem    = nullptr;
  m_editor.m_influxSinkItem       = nullptr;
  m_editor.m_controlScriptItem    = nullptr;
  m_editor.m_transformLibraryItem = nullptr;
  m_editor.m_jsLibraryItem        = nullptr;
  m_editor.m_scriptsRootItem      = nullptr;
  m_editor.m_exportRootItem       = nullptr;
}

/**
 * @brief Rebuilds the project-structure tree, restoring expansion and selection.
 */
void EditorTree::buildTreeModel()
{
  clearItemMaps();

  const bool seeding      = m_editor.m_seedExpansionFromModel;
  const bool filterActive = !m_editor.m_treeSearchQuery.trimmed().isEmpty();
  QHash<QString, bool> expandedStates;

  // code-verify off
  // Seed expansion only from the persisted map (the single source of truth, kept current by every
  // manual toggle via persistTreeExpansion), never from the live tree: &m_editor stops transient
  // force-expansion (search) and reveal (expandToIndex) from corrupting the saved state. A filtered
  // build is likewise never persisted back below, since its rows are force-expanded.
  // code-verify on
  const auto& persisted = m_model.treeExpansion();
  for (auto it = persisted.constBegin(); it != persisted.constEnd(); ++it)
    expandedStates.insert(it.key(), it.value().toBool());

  m_editor.m_seedExpansionFromModel = false;

  if (m_currentSelectionConnection) {
    QObject::disconnect(m_currentSelectionConnection);
    m_currentSelectionConnection = QMetaObject::Connection();
  }

  if (m_editor.m_selectionModel) {
    m_editor.m_selectionModel->deleteLater();
    m_editor.m_selectionModel = nullptr;
  }

  if (m_editor.m_treeModel) {
    m_editor.m_treeModel->disconnect(&m_editor);
    m_editor.m_treeModel->deleteLater();
    m_editor.m_treeModel = nullptr;
  }

  m_editor.m_treeModel = new CustomModel(&m_editor);

  auto& registry = Core::services().iconRegistry;
  const auto& pm = m_model;
  auto* root     = new QStandardItem(pm.title());
  root->setData(root->text(), TreeViewText);
  root->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("project-setup"), 16),
                TreeViewIcon);
  root->setData(true, TreeViewExpanded);
  root->setData(KindProjectRoot, TreeItemKind);
  root->setData(-1, TreeItemId);
  root->setData(-1, TreeItemParentId);

  m_editor.m_treeModel->appendRow(root);
  m_editor.m_rootItems.insert(root, kRootItem);

  buildTreeItems(root, expandedStates);

  m_editor.m_selectionModel = new QItemSelectionModel(m_editor.m_treeModel);
  m_currentSelectionConnection =
    QObject::connect(m_editor.m_selectionModel,
                     &QItemSelectionModel::currentChanged,
                     &m_editor,
                     [this](const QModelIndex& current, const QModelIndex& previous) {
                       m_editor.m_selection.onCurrentSelectionChanged(current, previous);
                     });

  QObject::connect(
    m_editor.m_selectionModel, &QItemSelectionModel::selectionChanged, &m_editor, [this] {
      m_editor.m_selection.onCurrentSelectionChanged(m_editor.m_selectionModel->currentIndex(),
                                                     QModelIndex());
    });

  Q_EMIT m_editor.treeModelChanged();

  const auto revealIndex = consumePendingSelection();
  if (!revealIndex.isValid())
    restoreTreeSelection();

  if (!seeding && !filterActive)
    m_model.storeTreeExpansion(snapshotTreeExpansion());

  Q_EMIT m_editor.treeRebuildFinished(revealIndex);
}

/**
 * @brief Selects a node queued by a deferred add-* signal handler.
 */
QModelIndex EditorTree::consumePendingSelection()
{
  if (m_editor.m_pendingSelectionKind == ProjectEditor::PendingSelectionKind::None
      || !m_editor.m_selectionModel)
    return {};

  const auto kind = m_editor.m_pendingSelectionKind;
  const auto gid  = m_editor.m_pendingSelectionGroupId;
  const auto iid  = m_editor.m_pendingSelectionItemId;

  m_editor.m_pendingSelectionKind    = ProjectEditor::PendingSelectionKind::None;
  m_editor.m_pendingSelectionGroupId = -1;
  m_editor.m_pendingSelectionItemId  = -1;

  const auto pickFirst = [this](const auto& map, auto&& pred) -> QModelIndex {
    for (auto it = map.begin(); it != map.end(); ++it) {
      if (!pred(it.value()))
        continue;

      const auto idx = it.key()->index();
      m_editor.m_selectionModel->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
      return idx;
    }
    return {};
  };

  if (kind == ProjectEditor::PendingSelectionKind::Source)
    return pickFirst(m_editor.m_sourceItems, [iid](const auto& v) { return v.sourceId == iid; });

  if (kind == ProjectEditor::PendingSelectionKind::Group)
    return pickFirst(m_editor.m_groupItems, [gid](const auto& v) { return v.groupId == gid; });

  if (kind == ProjectEditor::PendingSelectionKind::Dataset)
    return pickFirst(m_editor.m_datasetItems,
                     [gid, iid](const auto& v) { return v.groupId == gid && v.datasetId == iid; });

  if (kind == ProjectEditor::PendingSelectionKind::OutputWidget)
    return pickFirst(m_editor.m_outputWidgetItems,
                     [gid, iid](const auto& v) { return v.groupId == gid && v.widgetId == iid; });

  return {};
}

/**
 * @brief The tree glyph of a dataset: its first dashboard widget, else the plain dataset icon. One
 *        resolver for the build and for the in-place refresh an edit performs.
 */
QString EditorTree::datasetTreeIcon(const DataModel::Dataset& dataset)
{
  const auto widgets = SerialStudio::getDashboardWidgets(dataset);
  if (!widgets.isEmpty())
    return SerialStudio::dashboardWidgetIcon(widgets.first(), false);

  return Core::services().iconRegistry.icon(
    QStringLiteral("editor"), QStringLiteral("dataset"), 16);
}

/**
 * @brief Appends source tree items with their frame-parser children. No-op when filtering.
 */
void EditorTree::appendSourceTreeItems(QStandardItem* root)
{
  SS_ASSERT(root != nullptr, return);
  if (!m_editor.m_treeSearchQuery.trimmed().isEmpty())
    return;

  const auto& sources    = m_model.sources();
  const bool multiSource = sources.size() > 1;

  auto& registry = Core::services().iconRegistry;
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
    m_editor.m_sourceParserItems.insert(parserItem, source);

    root->appendRow(sourceItem);
    m_editor.m_sourceItems.insert(sourceItem, source);
  }
}

/**
 * @brief Appends action tree items, filtered by the current search query.
 */
void EditorTree::appendActionTreeItems(QStandardItem* root)
{
  SS_ASSERT(root != nullptr, return);

  const QString q         = m_editor.m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  const auto& actions     = m_model.actions();

  auto& registry = Core::services().iconRegistry;
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
    m_editor.m_actionItems.insert(actionItem, action);
  }
}

/**
 * @brief Appends dataset children of a group, filtered by the current search query.
 */
void EditorTree::appendDatasetChildren(QStandardItem* groupItem, const DataModel::Group& group)
{
  SS_ASSERT(groupItem != nullptr, return);

  const QString q         = m_editor.m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  const bool groupMatches = !filterActive || SerialStudio::searchMatches(q, group.title);

  for (const auto& dataset : group.datasets) {
    if (filterActive && !groupMatches && !SerialStudio::searchMatches(q, dataset.title))
      continue;

    auto* datasetItem = new QStandardItem(dataset.title);
    datasetItem->setData(datasetTreeIcon(dataset), TreeViewIcon);
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
    m_editor.m_datasetItems.insert(datasetItem, dataset);
  }
}

/**
 * @brief Appends output-widget children of a group. Skipped entirely when filtering.
 */
void EditorTree::appendOutputWidgetChildren(QStandardItem* groupItem, const DataModel::Group& group)
{
  SS_ASSERT(groupItem != nullptr, return);
  if (!m_editor.m_treeSearchQuery.trimmed().isEmpty())
    return;

  auto& registry = Core::services().iconRegistry;
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
    m_editor.m_outputWidgetItems.insert(owItem, ow);
  }
}

/**
 * @brief Appends the Groups subtree (groups, datasets, output widgets) into root.
 */
void EditorTree::appendGroupTreeItems(QStandardItem* root, QHash<QString, bool>& expandedStates)
{
  SS_ASSERT(root != nullptr, return);

  const QString q         = m_editor.m_treeSearchQuery.trimmed();
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

  const auto& groups     = m_model.groups();
  const auto& folders    = m_model.editorGroupFolders();
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

  auto& registry   = Core::services().iconRegistry;
  auto* groupsRoot = new QStandardItem(tr("Dashboard Widgets"));
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
    m_editor.m_groupItems.insert(groupItem, group);
  }

  if (!filterActive)
    restoreExpandedStateMap(groupsRoot, expandedStates, root->text() + "/" + groupsRoot->text());

  root->appendRow(groupsRoot);
  m_editor.m_groupsRootItem = groupsRoot;
}

/**
 * @brief Materializes the group folder items and re-parents them; returns the id->item map.
 */
QHash<int, QStandardItem*> EditorTree::appendGroupFolderItems(QStandardItem* groupsRoot,
                                                              const QString& pathPrefix,
                                                              QHash<QString, bool>& expandedStates)
{
  const auto& folders = m_model.editorGroupFolders();
  const auto& groups  = m_model.groups();

  QHash<int, bool> folderHasGroup;
  QHash<int, bool> folderHasEnabled;
  accumulateFolderEnabled(folders, groups, folderHasGroup, folderHasEnabled);

  auto& registry = Core::services().iconRegistry;
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
    m_editor.m_groupFolderItems.insert(item, f.folderId);
  }

  return folderItems;
}

/**
 * @brief Appends the "Variables" subtree (Pro only) with system + user tables.
 */
void EditorTree::appendSharedMemoryTreeItems(QStandardItem* root,
                                             QHash<QString, bool>& expandedStates)
{
  SS_ASSERT(root != nullptr, return);

#ifdef BUILD_COMMERCIAL
  auto& registry          = Core::services().iconRegistry;
  const QString q         = m_editor.m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  const auto matches      = [&q](const QString& s) {
    return SerialStudio::searchMatches(q, s);
  };

  const auto& userTables = m_model.tables();
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

  const auto& tableFolders    = m_model.editorTableFolders();
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
    m_editor.m_userTableItems.insert(tableItem, path);
  }

  restoreExpandedStateMap(tablesRoot, expandedStates, root->text() + "/" + tablesRoot->text());
  root->appendRow(tablesRoot);
  m_editor.m_tablesRootItem     = tablesRoot;
  m_editor.m_systemDatasetsItem = sysDsItem;
#else
  Q_UNUSED(root);
  Q_UNUSED(expandedStates);
#endif
}

/**
 * @brief Materializes the table folder items and re-parents them; returns the id->item map.
 */
QHash<int, QStandardItem*> EditorTree::appendTableFolderItems(QStandardItem* tablesRoot,
                                                              const QString& pathPrefix,
                                                              QHash<QString, bool>& expandedStates)
{
  const auto& folders = m_model.editorTableFolders();

  auto& registry = Core::services().iconRegistry;
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
    m_editor.m_tableFolderItems.insert(item, f.folderId);
  }

  return folderItems;
}

/**
 * @brief Builds a workspace tree item from a workspace record.
 */
QStandardItem* EditorTree::createWorkspaceItem(const DataModel::Workspace& ws)
{
  auto& registry = Core::services().iconRegistry;
  auto* wsItem   = new QStandardItem(ws.title);
  wsItem->setData(ws.title, TreeViewText);
  wsItem->setData(ws.icon.isEmpty()
                    ? registry.icon(QStringLiteral("widgets"), QStringLiteral("workspace"), 16)
                    : Misc::IconEngine::resolveActionIconSource(ws.icon),
                  TreeViewIcon);
  wsItem->setData(-1, TreeViewFrameIndex);
  wsItem->setData(KindWorkspace, TreeItemKind);
  wsItem->setData(ws.workspaceId, TreeItemId);
  wsItem->setData(ws.parentFolderId, TreeItemParentId);
  wsItem->setData(m_editor.m_summaries.workspaceHasUnresolvedRefs(ws.workspaceId),
                  TreeViewWorkspaceStale);
  return wsItem;
}

/**
 * @brief Builds the folder hierarchy under the Workspaces root (folders first, then workspaces).
 */
void EditorTree::buildWorkspaceFolderTree(QStandardItem* wsRoot,
                                          const QString& pathPrefix,
                                          QHash<QString, bool>& expandedStates)
{
  SS_ASSERT(wsRoot != nullptr, return);

  const auto& folders    = m_model.editorWorkspaceFolders();
  const auto& workspaces = m_model.editorWorkspaces();

  auto& registry = Core::services().iconRegistry;
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
    m_editor.m_workspaceFolderItems.insert(item, f.folderId);
  }

  for (const auto& ws : workspaces) {
    auto* wsItem          = createWorkspaceItem(ws);
    QStandardItem* parent = (ws.parentFolderId != -1 && folderItems.contains(ws.parentFolderId))
                            ? folderItems.value(ws.parentFolderId)
                            : wsRoot;
    parent->appendRow(wsItem);
    m_editor.m_workspaceItems.insert(wsItem, ws.workspaceId);
  }
}

/**
 * @brief Appends the "Workspaces" subtree, filtered by the current search query.
 */
void EditorTree::appendWorkspaceTreeItems(QStandardItem* root, QHash<QString, bool>& expandedStates)
{
  SS_ASSERT(root != nullptr, return);

  const QString q         = m_editor.m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  const auto matches      = [&q](const QString& s) {
    return SerialStudio::searchMatches(q, s);
  };

  const auto& workspaces = m_model.editorWorkspaces();
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

  auto& registry = Core::services().iconRegistry;
  auto* wsRoot   = new QStandardItem(tr("Workspaces"));
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
      m_editor.m_workspaceItems.insert(wsItem, ws.workspaceId);
    }
  } else {
    buildWorkspaceFolderTree(wsRoot, root->text() + "/" + tr("Workspaces"), expandedStates);
  }

  restoreExpandedStateMap(wsRoot, expandedStates, root->text() + "/" + wsRoot->text());
  root->appendRow(wsRoot);
  m_editor.m_workspacesRootItem = wsRoot;
}

/**
 * @brief Creates one single-instance leaf (Project Scripts and Data Export children).
 */
QStandardItem* EditorTree::createSingleLeaf(const QString& title,
                                            const QString& iconName,
                                            int kind,
                                            const QString& iconSet) const
{
  auto& registry = Core::services().iconRegistry;
  auto* item     = new QStandardItem(title);
  item->setData(title, TreeViewText);
  item->setData(registry.icon(iconSet, iconName, 16), TreeViewIcon);
  item->setData(-1, TreeViewFrameIndex);
  item->setData(kind, TreeItemKind);
  item->setData(-1, TreeItemId);
  item->setData(-1, TreeItemParentId);
  return item;
}

/**
 * @brief Appends the Project Scripts node (spec 0083 addendum) holding the control loop and the
 *        two shared transform libraries; a search keeps it when the title or any child matches.
 */
void EditorTree::appendScriptsTree(QStandardItem* root, QHash<QString, bool>& expandedStates)
{
  SS_ASSERT(root != nullptr, return);

  const QString q         = m_editor.m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  const auto matches      = [&q, filterActive](const QString& s) {
    return !filterActive || SerialStudio::searchMatches(q, s);
  };

  const QString rootTitle = tr("Project Scripts");
  const QString loopTitle = tr("Control Loop");
  const QString luaTitle  = tr("Lua Library");
  const QString jsTitle   = tr("JavaScript Library");
  const bool rootMatches  = matches(rootTitle);
  if (filterActive && !rootMatches && !matches(loopTitle) && !matches(luaTitle)
      && !matches(jsTitle))
    return;

  auto* node = createSingleLeaf(
    rootTitle, QStringLiteral("macro"), KindScriptsRoot, QStringLiteral("commands"));
  if (rootMatches || matches(loopTitle)) {
    m_editor.m_controlScriptItem =
      createSingleLeaf(loopTitle, QStringLiteral("code"), KindControlScript);
    node->appendRow(m_editor.m_controlScriptItem);
  }

  if (rootMatches || matches(luaTitle)) {
    m_editor.m_transformLibraryItem =
      createSingleLeaf(luaTitle, QStringLiteral("code"), KindTransformLibrary);
    node->appendRow(m_editor.m_transformLibraryItem);
  }

  if (rootMatches || matches(jsTitle)) {
    m_editor.m_jsLibraryItem = createSingleLeaf(jsTitle, QStringLiteral("code"), KindJsLibrary);
    node->appendRow(m_editor.m_jsLibraryItem);
  }

  if (filterActive)
    node->setData(true, TreeViewExpanded);
  else
    restoreExpandedStateMap(node, expandedStates, root->text() + "/" + rootTitle);

  root->appendRow(node);
  m_editor.m_scriptsRootItem = node;
}

/**
 * @brief Appends the Data Export node (Pro) holding the MQTT publisher and the InfluxDB sink; a
 *        search keeps it when the title or either child matches.
 */
void EditorTree::appendExportTree(QStandardItem* root, QHash<QString, bool>& expandedStates)
{
  SS_ASSERT(root != nullptr, return);

  const QString q         = m_editor.m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  const auto matches      = [&q, filterActive](const QString& s) {
    return !filterActive || SerialStudio::searchMatches(q, s);
  };

  const QString rootTitle   = tr("Data Export");
  const QString mqttTitle   = tr("MQTT Publisher");
  const QString influxTitle = tr("InfluxDB Sink");
  const bool rootMatches    = matches(rootTitle);
  if (filterActive && !rootMatches && !matches(mqttTitle) && !matches(influxTitle))
    return;

  auto* node = createSingleLeaf(rootTitle, QStringLiteral("tx-data"), KindExportRoot);
  if (rootMatches || matches(mqttTitle)) {
    m_editor.m_mqttPublisherItem =
      createSingleLeaf(mqttTitle, QStringLiteral("mqtt-publisher"), KindMqttPublisher);
    node->appendRow(m_editor.m_mqttPublisherItem);
  }

  if (rootMatches || matches(influxTitle)) {
    m_editor.m_influxSinkItem =
      createSingleLeaf(influxTitle, QStringLiteral("influx"), KindInfluxSink);
    node->appendRow(m_editor.m_influxSinkItem);
  }

  if (filterActive)
    node->setData(true, TreeViewExpanded);
  else
    restoreExpandedStateMap(node, expandedStates, root->text() + "/" + rootTitle);

  root->appendRow(node);
  m_editor.m_exportRootItem = node;
}

/**
 * @brief Populates the tree under root with sources, actions, groups, datasets.
 */
void EditorTree::buildTreeItems(QStandardItem* root, QHash<QString, bool>& expandedStates)
{
  SS_ASSERT(root != nullptr, return);

  appendScriptsTree(root, expandedStates);
#ifdef BUILD_COMMERCIAL
  appendExportTree(root, expandedStates);
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
QStandardItem* EditorTree::entitySelectionItem() const
{
  if (m_editor.m_currentView == DatasetView) {
    const auto gid = m_editor.m_selectedDataset.groupId;
    const auto did = m_editor.m_selectedDataset.datasetId;
    return findMappedItem(m_editor.m_datasetItems, [gid, did](const auto& v) {
      return v.groupId == gid && v.datasetId == did;
    });
  }

  if (m_editor.m_currentView == GroupView) {
    const auto gid = m_editor.m_selectedGroup.groupId;
    return findMappedItem(m_editor.m_groupItems, [gid](const auto& v) { return v.groupId == gid; });
  }

  if (m_editor.m_currentView == ActionView) {
    const auto aid = m_editor.m_selectedAction.actionId;
    return findMappedItem(m_editor.m_actionItems,
                          [aid](const auto& v) { return v.actionId == aid; });
  }

  if (m_editor.m_currentView == SourceView) {
    const auto sid = m_editor.m_selectedSource.sourceId;
    return findMappedItem(m_editor.m_sourceItems,
                          [sid](const auto& v) { return v.sourceId == sid; });
  }

  if (m_editor.m_currentView == OutputWidgetView) {
    const auto gid = m_editor.m_selectedOutputWidget.groupId;
    const auto wid = m_editor.m_selectedOutputWidget.widgetId;
    return findMappedItem(m_editor.m_outputWidgetItems, [gid, wid](const auto& v) {
      return v.groupId == gid && v.widgetId == wid;
    });
  }

  if (m_editor.m_currentView == SourceFrameParserView) {
    const auto sid = m_editor.m_selectedSource.sourceId;
    return findMappedItem(m_editor.m_sourceParserItems,
                          [sid](const auto& v) { return v.sourceId == sid; });
  }

  return nullptr;
}

/**
 * @brief Resolves the tree item for container and singleton views (tables, workspaces, folders,
 *        MQTT publisher, control loop), falling back to the owning subtree root when the
 *        selected entry no longer exists; returns nullptr when another view is active.
 */
QStandardItem* EditorTree::containerSelectionItem() const
{
  if (m_editor.m_currentView == DataTablesView)
    return m_editor.m_tablesRootItem;

  if (m_editor.m_currentView == SystemDatasetsView)
    return m_editor.m_systemDatasetsItem;

  if (m_editor.m_currentView == UserTableView) {
    const auto name = m_editor.m_selectedUserTable;
    auto* item =
      findMappedItem(m_editor.m_userTableItems, [&name](const auto& v) { return v == name; });
    return item ? item : m_editor.m_tablesRootItem;
  }

  if (m_editor.m_currentView == WorkspacesView)
    return m_editor.m_workspacesRootItem;

  if (m_editor.m_currentView == WorkspaceView) {
    const int wid = m_editor.m_selectedWorkspaceId;
    auto* item =
      findMappedItem(m_editor.m_workspaceItems, [wid](const auto& v) { return v == wid; });
    return item ? item : m_editor.m_workspacesRootItem;
  }

  if (m_editor.m_currentView == GroupsView)
    return m_editor.m_groupsRootItem;

  if (m_editor.m_currentView == GroupFolderView) {
    const int fid = m_editor.m_selectedGroupFolderId;
    auto* item =
      findMappedItem(m_editor.m_groupFolderItems, [fid](const auto& v) { return v == fid; });
    return item ? item : m_editor.m_groupsRootItem;
  }

  if (m_editor.m_currentView == TableFolderView) {
    const int fid = m_editor.m_selectedTableFolderId;
    auto* item =
      findMappedItem(m_editor.m_tableFolderItems, [fid](const auto& v) { return v == fid; });
    return item ? item : m_editor.m_tablesRootItem;
  }

  if (m_editor.m_currentView == WorkspaceFolderView) {
    const int fid = m_editor.m_selectedFolderId;
    auto* item =
      findMappedItem(m_editor.m_workspaceFolderItems, [fid](const auto& v) { return v == fid; });
    return item ? item : m_editor.m_workspacesRootItem;
  }

  if (m_editor.m_currentView == MqttPublisherView)
    return m_editor.m_mqttPublisherItem;

  if (m_editor.m_currentView == InfluxSinkView)
    return m_editor.m_influxSinkItem;

  if (m_editor.m_currentView == ControlScriptView)
    return m_editor.m_controlScriptItem;

  if (m_editor.m_currentView == TransformLibraryView)
    return m_editor.m_transformLibraryItem;

  if (m_editor.m_currentView == JsLibraryView)
    return m_editor.m_jsLibraryItem;

  if (m_editor.m_currentView == ProjectScriptsView)
    return m_editor.m_scriptsRootItem;

  if (m_editor.m_currentView == DataExportView)
    return m_editor.m_exportRootItem;

  return nullptr;
}

/**
 * @brief Restores the tree selection by matching IDs against the current view.
 */
void EditorTree::restoreTreeSelection()
{
  QStandardItem* toSelect = entitySelectionItem();
  if (!toSelect)
    toSelect = containerSelectionItem();

  if (!toSelect)
    toSelect = findMappedItem(m_editor.m_rootItems, [](const auto& v) { return v == kRootItem; });

  if (toSelect)
    m_editor.m_selectionModel->setCurrentIndex(toSelect->index(),
                                               QItemSelectionModel::ClearAndSelect);
}

//--------------------------------------------------------------------------------------------------
// Expansion state maps
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records TreeViewExpanded state of the subtree into a path-keyed map.
 */
void EditorTree::saveExpandedStateMap(QStandardItem* item,
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
void EditorTree::restoreExpandedStateMap(QStandardItem* item,
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
QJsonObject EditorTree::snapshotTreeExpansion()
{
  QJsonObject obj;
  if (!m_editor.m_treeModel)
    return obj;

  QHash<QString, bool> states;
  saveExpandedStateMap(m_editor.m_treeModel->invisibleRootItem(), states, "");
  for (auto it = states.constBegin(); it != states.constEnd(); ++it)
    obj.insert(it.key(), it.value());

  return obj;
}

/**
 * @brief Pushes the current tree expansion into the project model (e.g. after a manual toggle).
 */
void EditorTree::persistTreeExpansion()
{
  m_model.setTreeExpansion(snapshotTreeExpansion());
}

//--------------------------------------------------------------------------------------------------
// Navigation-driven expansion: model-role authority, the view follows via syncExpandedState
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks every ancestor of @p index expanded so the item is revealed. Writes the model role
 *        (the view's source of truth) rather than the view directly, so the reveal survives reuse.
 */
void EditorTree::expandTreeToIndex(const QModelIndex& index)
{
  if (!m_editor.m_treeModel)
    return;

  const auto* item = m_editor.m_treeModel->itemFromIndex(index);
  if (!item)
    return;

  for (auto* p = item->parent(); p != nullptr; p = p->parent())
    p->setData(true, TreeViewExpanded);
}

/**
 * @brief Sets the expanded model role of @p index; the delegate reacts and expands/collapses the
 * row.
 */
void EditorTree::setTreeIndexExpanded(const QModelIndex& index, bool expanded)
{
  if (!m_editor.m_treeModel)
    return;

  if (auto* item = m_editor.m_treeModel->itemFromIndex(index))
    item->setData(expanded, TreeViewExpanded);
}

/**
 * @brief Opens every foldable node in the tree; the view follows the model role per delegate.
 */
void EditorTree::expandAllTreeItems()
{
  if (!m_editor.m_treeModel)
    return;

  auto* root = m_editor.m_treeModel->invisibleRootItem();
  SS_ASSERT(root != nullptr, return);
  for (int r = 0; r < root->rowCount(); ++r)
    applyTreeExpansion(root->child(r), 0, std::numeric_limits<int>::max());

  persistTreeExpansion();
}

/**
 * @brief Collapses the tree to its overview: the root, top-level categories, and each category's
 *        direct items stay visible (depth < 2 open); everything deeper folds away.
 */
void EditorTree::collapseTreeToOverview()
{
  if (!m_editor.m_treeModel)
    return;

  auto* root = m_editor.m_treeModel->invisibleRootItem();
  SS_ASSERT(root != nullptr, return);
  for (int r = 0; r < root->rowCount(); ++r)
    applyTreeExpansion(root->child(r), 0, 2);

  persistTreeExpansion();
}

/**
 * @brief True when the tree item at @p index has child rows (a foldable node).
 */
bool EditorTree::treeIndexHasChildren(const QModelIndex& index) const
{
  const auto* item = m_editor.m_treeModel ? m_editor.m_treeModel->itemFromIndex(index) : nullptr;
  return item && item->hasChildren();
}

/**
 * @brief True when the tree item at @p index is currently marked expanded in the model.
 */
bool EditorTree::treeIndexExpanded(const QModelIndex& index) const
{
  const auto* item = m_editor.m_treeModel ? m_editor.m_treeModel->itemFromIndex(index) : nullptr;
  return item && item->data(TreeViewExpanded).toBool();
}

}  // namespace DataModel
