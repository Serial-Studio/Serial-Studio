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
#include <memory>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <QTimer>

#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"
#include "SSAssert.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Publisher.h"
#  include "MQTT/PublisherScriptEditor.h"
#endif
#include "ProjectEditorItemIds.h"

/**
 * @brief Returns the first tree item in @p map whose mapped value satisfies @p pred.
 */
template<typename Map, typename Pred>
static QStandardItem* navFind(const Map& map, Pred&& pred)
{
  for (auto it = map.begin(); it != map.end(); ++it)
    if (pred(it.value()))
      return it.key();

  return nullptr;
}

//--------------------------------------------------------------------------------------------------
// Private slot: view management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Transitions the editor to the given view.
 */
void DataModel::ProjectEditor::setCurrentView(const DataModel::ProjectEditor::CurrentView view)
{
  if (m_suppressViewChange) [[unlikely]]
    return;

  if (m_currentView == view)
    return;

  m_currentView = view;
  Q_EMIT currentViewChanged();
  Q_EMIT selectedTextChanged();
}

/**
 * @brief Toggles the suppression latch used by the diagram's context-menu actions.
 */
void DataModel::ProjectEditor::setSuppressViewChange(bool suppress) noexcept
{
  m_suppressViewChange = suppress;
}

//--------------------------------------------------------------------------------------------------
// Private slot: tree selection changed
//--------------------------------------------------------------------------------------------------

/**
 * @brief Refreshes the selected-source snapshot and switches to the parser view.
 */
bool DataModel::ProjectEditor::selectSourceParserItem(QStandardItem* item)
{
  if (!m_sourceParserItems.contains(item))
    return false;

  const auto cached      = m_sourceParserItems.value(item);
  const auto& srcs       = m_projectModelRef.sources();
  DataModel::Source live = cached;
  for (const auto& s : srcs) {
    if (s.sourceId == cached.sourceId) {
      live = s;
      break;
    }
  }
  m_selectedSource = live;
  setCurrentView(SourceFrameParserView);
  Q_EMIT selectedSourceFrameParserCodeChanged();
  Q_EMIT sourceModelChanged();
  return true;
}

/**
 * @brief Switches the form to the SourceView for the clicked source item.
 */
bool DataModel::ProjectEditor::selectSourceItem(QStandardItem* item)
{
  if (!m_sourceItems.contains(item))
    return false;

  const auto cached      = m_sourceItems.value(item);
  const auto& srcs       = m_projectModelRef.sources();
  DataModel::Source live = cached;
  for (const auto& s : srcs) {
    if (s.sourceId == cached.sourceId) {
      live = s;
      break;
    }
  }

  if (m_currentView == SourceView && live.sourceId == m_selectedSource.sourceId)
    return true;

  setCurrentView(SourceView);
  buildSourceModel(live);
  return true;
}

/**
 * @brief Switches the form to the GroupView for the clicked group item.
 */
bool DataModel::ProjectEditor::selectGroupItem(QStandardItem* item)
{
  if (!m_groupItems.contains(item))
    return false;

  auto& pm              = m_projectModelRef;
  const auto cached     = m_groupItems.value(item);
  const auto& groups    = pm.groups();
  DataModel::Group live = cached;
  if (cached.groupId >= 0 && static_cast<size_t>(cached.groupId) < groups.size())
    live = groups[cached.groupId];

  pm.setSelectedGroup(live);
  setCurrentView(GroupView);
  buildGroupModel(live);
  return true;
}

/**
 * @brief Routes the Groups root and group folder items to their navigable views.
 */
bool DataModel::ProjectEditor::selectGroupFolderItem(QStandardItem* item)
{
  if (item == m_groupsRootItem) {
    setCurrentView(GroupsView);
    return true;
  }

  if (m_groupFolderItems.contains(item)) {
    const int fid = m_groupFolderItems.value(item);
    if (m_selectedGroupFolderId != fid) {
      m_selectedGroupFolderId = fid;
      Q_EMIT selectedGroupFolderIdChanged();
    }
    setCurrentView(GroupFolderView);
    return true;
  }

  return false;
}

/**
 * @brief Switches the form to the DatasetView for the clicked dataset item.
 */
bool DataModel::ProjectEditor::selectDatasetItem(QStandardItem* item)
{
  if (!m_datasetItems.contains(item))
    return false;

  auto& pm                = m_projectModelRef;
  const auto cached       = m_datasetItems.value(item);
  const auto& groups      = pm.groups();
  DataModel::Dataset live = cached;
  if (cached.groupId >= 0 && static_cast<size_t>(cached.groupId) < groups.size()) {
    for (const auto& d : groups[cached.groupId].datasets) {
      if (d.datasetId == cached.datasetId) {
        live = d;
        break;
      }
    }
  }

  pm.setSelectedDataset(live);
  setCurrentView(DatasetView);
  buildDatasetModel(live);
  return true;
}

/**
 * @brief Switches the form to the ActionView for the clicked action item.
 */
bool DataModel::ProjectEditor::selectActionItem(QStandardItem* item)
{
  if (!m_actionItems.contains(item))
    return false;

  auto& pm               = m_projectModelRef;
  const auto cached      = m_actionItems.value(item);
  const auto& actions    = pm.actions();
  DataModel::Action live = cached;
  for (const auto& a : actions) {
    if (a.actionId == cached.actionId) {
      live = a;
      break;
    }
  }

  pm.setSelectedAction(live);
  setCurrentView(ActionView);
  buildActionModel(live);
  return true;
}

/**
 * @brief Switches the form to the OutputWidgetView for the clicked widget item.
 */
bool DataModel::ProjectEditor::selectOutputWidgetItem(QStandardItem* item)
{
  if (!m_outputWidgetItems.contains(item))
    return false;

  auto& pm                     = m_projectModelRef;
  const auto cached            = m_outputWidgetItems.value(item);
  const auto& groups           = pm.groups();
  DataModel::OutputWidget live = cached;
  if (cached.groupId >= 0 && static_cast<size_t>(cached.groupId) < groups.size()) {
    for (const auto& w : groups[cached.groupId].outputWidgets) {
      if (w.widgetId == cached.widgetId) {
        live = w;
        break;
      }
    }
  }

  pm.setSelectedOutputWidget(live);
  setCurrentView(OutputWidgetView);
  buildOutputWidgetModel(live);
  return true;
}

/**
 * @brief Routes selections under the data-tables tree branch.
 */
bool DataModel::ProjectEditor::selectDataTableItem(QStandardItem* item)
{
  if (item == m_tablesRootItem) {
    setCurrentView(DataTablesView);
    return true;
  }

  if (item == m_systemDatasetsItem) {
    setCurrentView(SystemDatasetsView);
    return true;
  }

  if (m_userTableItems.contains(item)) {
    const auto name = m_userTableItems.value(item);
    if (m_selectedUserTable != name) {
      m_selectedUserTable = name;
      Q_EMIT selectedUserTableChanged();
    }
    setCurrentView(UserTableView);
    return true;
  }

  if (m_tableFolderItems.contains(item)) {
    const int fid = m_tableFolderItems.value(item);
    if (m_selectedTableFolderId != fid) {
      m_selectedTableFolderId = fid;
      Q_EMIT selectedTableFolderIdChanged();
    }
    setCurrentView(TableFolderView);
    return true;
  }

  return false;
}

/**
 * @brief Routes selections under the workspaces tree branch.
 */
bool DataModel::ProjectEditor::selectWorkspaceTreeItem(QStandardItem* item)
{
  if (item == m_workspacesRootItem) {
    setCurrentView(WorkspacesView);
    return true;
  }

  if (m_workspaceItems.contains(item)) {
    const int wid = m_workspaceItems.value(item);
    if (m_selectedWorkspaceId != wid) {
      m_selectedWorkspaceId = wid;
      Q_EMIT selectedWorkspaceIdChanged();
    }
    setCurrentView(WorkspaceView);
    return true;
  }

  if (m_workspaceFolderItems.contains(item)) {
    const int fid = m_workspaceFolderItems.value(item);
    if (m_selectedFolderId != fid) {
      m_selectedFolderId = fid;
      Q_EMIT selectedFolderIdChanged();
    }
    setCurrentView(WorkspaceFolderView);
    return true;
  }

  return false;
}

/**
 * @brief Routes selection of the single MQTT Publisher tree node.
 */
bool DataModel::ProjectEditor::selectMqttPublisherItem(QStandardItem* item)
{
  if (item != m_mqttPublisherItem || item == nullptr)
    return false;

  buildMqttPublisherModel();
  setCurrentView(MqttPublisherView);
  return true;
}

/**
 * @brief Routes selection of the single InfluxDB sink tree node.
 */
bool DataModel::ProjectEditor::selectInfluxSinkItem(QStandardItem* item)
{
  if (item != m_influxSinkItem || item == nullptr)
    return false;

  setCurrentView(InfluxSinkView);
  return true;
}

/**
 * @brief Switches to the control-script view when its tree node is selected.
 */
bool DataModel::ProjectEditor::selectControlScriptItem(QStandardItem* item)
{
  if (item != m_controlScriptItem || item == nullptr)
    return false;

  setCurrentView(ControlScriptView);
  return true;
}

/**
 * @brief Switches the active editor view based on the newly selected tree item.
 */
void DataModel::ProjectEditor::onCurrentSelectionChanged(const QModelIndex& current,
                                                         const QModelIndex& previous)
{
  (void)previous;

  if (!m_treeModel)
    return;

  if (tryMultiSelection())
    return;

  m_batchKind = KindNone;
  m_batchItems.clear();

  if (!current.isValid())
    return;

  auto* item = m_treeModel->itemFromIndex(current);
  if (!item)
    return;

  if (!m_nav.navigating() && m_nav.push(captureNavEntry(item)))
    Q_EMIT navHistoryChanged();

  const bool handled =
    selectSourceParserItem(item) || selectSourceItem(item) || selectGroupItem(item)
    || selectGroupFolderItem(item) || selectDatasetItem(item) || selectActionItem(item)
    || selectOutputWidgetItem(item) || selectDataTableItem(item) || selectWorkspaceTreeItem(item)
    || selectMqttPublisherItem(item) || selectInfluxSinkItem(item) || selectControlScriptItem(item);

  if (!handled && m_rootItems.contains(item)) {
    setCurrentView(ProjectView);
    buildProjectModel();
  }

  Q_EMIT editableOptionsChanged();
}

//--------------------------------------------------------------------------------------------------
// Back / forward navigation history
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when a previously visited tree node is available behind the cursor.
 */
bool DataModel::ProjectEditor::canGoBack() const noexcept
{
  return m_nav.canGoBack();
}

/**
 * @brief True when a visited tree node is available ahead of the cursor.
 */
bool DataModel::ProjectEditor::canGoForward() const noexcept
{
  return m_nav.canGoForward();
}

/**
 * @brief Snapshots a visited tree node as a rebuild-stable logical identity (roles for entity
 *        nodes; destination view for container/root nodes, which carry no TreeItemKind role).
 */
auto DataModel::ProjectEditor::captureNavEntry(QStandardItem* item) const -> NavEntry
{
  NavEntry entry;
  SS_ASSERT(item != nullptr, return entry);

  const int kindRole = item->data(TreeItemKind).toInt();
  if (kindRole != KindNone) {
    entry.valid    = true;
    entry.kind     = kindRole;
    entry.id       = item->data(TreeItemId).toInt();
    entry.parentId = item->data(TreeItemParentId).toInt();
    if (kindRole == KindUserTable)
      entry.key = m_userTableItems.value(item);

    return entry;
  }

  entry.container = true;
  entry.valid     = true;
  if (m_rootItems.contains(item)) {
    entry.view = static_cast<int>(ProjectView);
    return entry;
  }
  if (item == m_groupsRootItem) {
    entry.view = static_cast<int>(GroupsView);
    return entry;
  }
  if (item == m_tablesRootItem) {
    entry.view = static_cast<int>(DataTablesView);
    return entry;
  }
  if (item == m_systemDatasetsItem) {
    entry.view = static_cast<int>(SystemDatasetsView);
    return entry;
  }
  if (item == m_workspacesRootItem) {
    entry.view = static_cast<int>(WorkspacesView);
    return entry;
  }

  entry.valid = false;
  return entry;
}

/**
 * @brief Resolves a history entry to a live tree item in the current model, or nullptr if the
 *        node no longer exists (deleted item / different project).
 */
QStandardItem* DataModel::ProjectEditor::resolveNavEntry(const NavEntry& entry) const
{
  if (!entry.valid)
    return nullptr;

  if (entry.container) {
    switch (static_cast<CurrentView>(entry.view)) {
      case ProjectView:
        return m_rootItems.isEmpty() ? nullptr : m_rootItems.firstKey();
      case GroupsView:
        return m_groupsRootItem;
      case DataTablesView:
        return m_tablesRootItem;
      case SystemDatasetsView:
        return m_systemDatasetsItem;
      case WorkspacesView:
        return m_workspacesRootItem;
      default:
        return nullptr;
    }
  }

  const int id  = entry.id;
  const int pid = entry.parentId;
  switch (static_cast<ItemKind>(entry.kind)) {
    case KindGroup:
      return navFind(m_groupItems, [id](const auto& v) { return v.groupId == id; });
    case KindDataset:
      return navFind(m_datasetItems,
                     [id, pid](const auto& v) { return v.groupId == pid && v.datasetId == id; });
    case KindAction:
      return navFind(m_actionItems, [id](const auto& v) { return v.actionId == id; });
    case KindOutputWidget:
      return navFind(m_outputWidgetItems,
                     [id, pid](const auto& v) { return v.groupId == pid && v.widgetId == id; });
    case KindSource:
      return navFind(m_sourceItems, [id](const auto& v) { return v.sourceId == id; });
    case KindGroupFolder:
      return navFind(m_groupFolderItems, [id](const auto& v) { return v == id; });
    case KindTableFolder:
      return navFind(m_tableFolderItems, [id](const auto& v) { return v == id; });
    case KindWorkspace:
      return navFind(m_workspaceItems, [id](const auto& v) { return v == id; });
    case KindWorkspaceFolder:
      return navFind(m_workspaceFolderItems, [id](const auto& v) { return v == id; });
    case KindUserTable: {
      const QString& key = entry.key;
      return navFind(m_userTableItems, [&key](const auto& v) { return v == key; });
    }
    case KindMqttPublisher:
      return m_mqttPublisherItem;
    case KindInfluxSink:
      return m_influxSinkItem;
    case KindControlScript:
      return m_controlScriptItem;
    default:
      return nullptr;
  }
}

/**
 * @brief Drops the whole history (used when a different project is loaded).
 */
void DataModel::ProjectEditor::clearNavHistory()
{
  if (m_nav.clear())
    Q_EMIT navHistoryChanged();
}

/**
 * @brief Steps back to the nearest still-resolvable earlier node; the resolver skips deleted
 *        entries and the replay guard keeps the resulting selection out of the history.
 */
void DataModel::ProjectEditor::navigateBack()
{
  if (!canGoBack() || !m_selectionModel)
    return;

  QStandardItem* target = nullptr;
  const int index       = m_nav.previousResolvable([this, &target](const NavEntry& entry) {
    target = resolveNavEntry(entry);
    return target != nullptr;
  });

  if (index < 0 || !target)
    return;

  m_nav.setCursor(index);
  m_nav.setNavigating(true);
  m_nav.setDirection(-1);
  m_selectionModel->setCurrentIndex(target->index(), QItemSelectionModel::ClearAndSelect);
  m_nav.setDirection(0);
  m_nav.setNavigating(false);
  Q_EMIT navHistoryChanged();
}

/**
 * @brief Reveal direction of the in-flight selection change: -1 back, +1 forward, 0 normal.
 *        Read by the tree view during currentChanged to expand forward / collapse on back.
 */
int DataModel::ProjectEditor::navDirection() const noexcept
{
  return m_nav.direction();
}

/**
 * @brief Steps forward to the nearest still-resolvable later node; the resolver skips deleted
 *        entries and the replay guard keeps the resulting selection out of the history.
 */
void DataModel::ProjectEditor::navigateForward()
{
  if (!canGoForward() || !m_selectionModel)
    return;

  QStandardItem* target = nullptr;
  const int index       = m_nav.nextResolvable([this, &target](const NavEntry& entry) {
    target = resolveNavEntry(entry);
    return target != nullptr;
  });

  if (index < 0 || !target)
    return;

  m_nav.setCursor(index);
  m_nav.setNavigating(true);
  m_nav.setDirection(1);
  m_selectionModel->setCurrentIndex(target->index(), QItemSelectionModel::ClearAndSelect);
  m_nav.setDirection(0);
  m_nav.setNavigating(false);
  Q_EMIT navHistoryChanged();
}

/**
 * @brief Selects the source item with the given sourceId in the tree.
 */
void DataModel::ProjectEditor::selectSource(int sourceId)
{
  if (!m_selectionModel)
    return;

  for (auto it = m_sourceItems.begin(); it != m_sourceItems.end(); ++it) {
    if (it.value().sourceId == sourceId) {
      m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the group item with the given groupId in the tree.
 */
void DataModel::ProjectEditor::selectGroup(int groupId)
{
  if (!m_selectionModel)
    return;

  for (auto it = m_groupItems.begin(); it != m_groupItems.end(); ++it) {
    if (it.value().groupId == groupId) {
      m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the dataset item (groupId, datasetId) in the tree.
 */
void DataModel::ProjectEditor::selectDataset(int groupId, int datasetId)
{
  if (!m_selectionModel)
    return;

  for (auto it = m_datasetItems.begin(); it != m_datasetItems.end(); ++it) {
    if (it.value().groupId == groupId && it.value().datasetId == datasetId) {
      m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the action item with the given actionId in the tree.
 */
void DataModel::ProjectEditor::selectAction(int actionId)
{
  if (!m_selectionModel)
    return;

  for (auto it = m_actionItems.begin(); it != m_actionItems.end(); ++it) {
    if (it.value().actionId == actionId) {
      m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the Frame Parser tree node for the given source.
 */
void DataModel::ProjectEditor::selectFrameParser(int sourceId)
{
  displayFrameParserView(sourceId);
}

/**
 * @brief Selects the output widget (groupId, widgetId) in the tree.
 */
void DataModel::ProjectEditor::selectOutputWidget(int groupId, int widgetId)
{
  if (!m_selectionModel)
    return;

  for (auto it = m_outputWidgetItems.begin(); it != m_outputWidgetItems.end(); ++it) {
    if (it.value().groupId == groupId && it.value().widgetId == widgetId) {
      m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}
