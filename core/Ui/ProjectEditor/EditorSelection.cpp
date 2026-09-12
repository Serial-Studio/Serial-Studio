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

#include "ProjectEditor/EditorSelection.h"

#include <cmath>
#include <memory>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <QTimer>

#include "Core/Checksum.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "ProjectEditor/ProjectEditor.h"
#include "ProjectEditorItemIds.h"

namespace DataModel {

using enum ProjectEditor::CustomRoles;
using enum ProjectEditor::CurrentView;
using ItemKind    = ProjectEditor::ItemKind;
using CurrentView = ProjectEditor::CurrentView;

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the facade and the model; the navigation history starts empty.
 */
EditorSelection::EditorSelection(ProjectEditor& editor, ProjectModel& model)
  : m_editor(editor), m_model(model)
{}

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
// Private slot: tree selection changed
//--------------------------------------------------------------------------------------------------

/**
 * @brief Refreshes the selected-source snapshot and switches to the parser view.
 */
bool EditorSelection::selectSourceParserItem(QStandardItem* item)
{
  if (!m_editor.m_sourceParserItems.contains(item))
    return false;

  const auto cached      = m_editor.m_sourceParserItems.value(item);
  const auto& srcs       = m_model.sources();
  DataModel::Source live = cached;
  for (const auto& s : srcs) {
    if (s.sourceId == cached.sourceId) {
      live = s;
      break;
    }
  }
  m_editor.m_selectedSource = live;
  m_editor.setCurrentView(SourceFrameParserView);
  Q_EMIT m_editor.selectedSourceFrameParserCodeChanged();
  Q_EMIT m_editor.sourceModelChanged();
  return true;
}

/**
 * @brief Switches the form to the SourceView for the clicked source item.
 */
bool EditorSelection::selectSourceItem(QStandardItem* item)
{
  if (!m_editor.m_sourceItems.contains(item))
    return false;

  const auto cached      = m_editor.m_sourceItems.value(item);
  const auto& srcs       = m_model.sources();
  DataModel::Source live = cached;
  for (const auto& s : srcs) {
    if (s.sourceId == cached.sourceId) {
      live = s;
      break;
    }
  }

  if (m_editor.m_currentView == SourceView && live.sourceId == m_editor.m_selectedSource.sourceId)
    return true;

  m_editor.setCurrentView(SourceView);
  m_editor.m_forms.buildSourceModel(live);
  return true;
}

/**
 * @brief Switches the form to the GroupView for the clicked group item.
 */
bool EditorSelection::selectGroupItem(QStandardItem* item)
{
  if (!m_editor.m_groupItems.contains(item))
    return false;

  auto& pm              = m_model;
  const auto cached     = m_editor.m_groupItems.value(item);
  const auto& groups    = pm.groups();
  DataModel::Group live = cached;
  if (cached.groupId >= 0 && static_cast<size_t>(cached.groupId) < groups.size())
    live = groups[cached.groupId];

  pm.setSelectedGroup(live);
  m_editor.setCurrentView(GroupView);
  m_editor.m_forms.buildGroupModel(live);
  return true;
}

/**
 * @brief Routes the Groups root and group folder items to their navigable views.
 */
bool EditorSelection::selectGroupFolderItem(QStandardItem* item)
{
  if (item == m_editor.m_groupsRootItem) {
    m_editor.setCurrentView(GroupsView);
    return true;
  }

  if (m_editor.m_groupFolderItems.contains(item)) {
    const int fid = m_editor.m_groupFolderItems.value(item);
    if (m_editor.m_selectedGroupFolderId != fid) {
      m_editor.m_selectedGroupFolderId = fid;
      Q_EMIT m_editor.selectedGroupFolderIdChanged();
    }
    m_editor.setCurrentView(GroupFolderView);
    return true;
  }

  return false;
}

/**
 * @brief Switches the form to the DatasetView for the clicked dataset item.
 */
bool EditorSelection::selectDatasetItem(QStandardItem* item)
{
  if (!m_editor.m_datasetItems.contains(item))
    return false;

  auto& pm                = m_model;
  const auto cached       = m_editor.m_datasetItems.value(item);
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
  m_editor.setCurrentView(DatasetView);
  m_editor.m_forms.buildDatasetModel(live);
  return true;
}

/**
 * @brief Switches the form to the ActionView for the clicked action item.
 */
bool EditorSelection::selectActionItem(QStandardItem* item)
{
  if (!m_editor.m_actionItems.contains(item))
    return false;

  auto& pm               = m_model;
  const auto cached      = m_editor.m_actionItems.value(item);
  const auto& actions    = pm.actions();
  DataModel::Action live = cached;
  for (const auto& a : actions) {
    if (a.actionId == cached.actionId) {
      live = a;
      break;
    }
  }

  pm.setSelectedAction(live);
  m_editor.setCurrentView(ActionView);
  m_editor.m_forms.buildActionModel(live);
  return true;
}

/**
 * @brief Switches the form to the OutputWidgetView for the clicked widget item.
 */
bool EditorSelection::selectOutputWidgetItem(QStandardItem* item)
{
  if (!m_editor.m_outputWidgetItems.contains(item))
    return false;

  auto& pm                     = m_model;
  const auto cached            = m_editor.m_outputWidgetItems.value(item);
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
  m_editor.setCurrentView(OutputWidgetView);
  m_editor.m_forms.buildOutputWidgetModel(live);
  return true;
}

/**
 * @brief Routes selections under the data-tables tree branch.
 */
bool EditorSelection::selectDataTableItem(QStandardItem* item)
{
  if (item == m_editor.m_tablesRootItem) {
    m_editor.setCurrentView(DataTablesView);
    return true;
  }

  if (item == m_editor.m_systemDatasetsItem) {
    m_editor.setCurrentView(SystemDatasetsView);
    return true;
  }

  if (m_editor.m_userTableItems.contains(item)) {
    const auto name = m_editor.m_userTableItems.value(item);
    if (m_editor.m_selectedUserTable != name) {
      m_editor.m_selectedUserTable = name;
      Q_EMIT m_editor.selectedUserTableChanged();
    }
    m_editor.setCurrentView(UserTableView);
    return true;
  }

  if (m_editor.m_tableFolderItems.contains(item)) {
    const int fid = m_editor.m_tableFolderItems.value(item);
    if (m_editor.m_selectedTableFolderId != fid) {
      m_editor.m_selectedTableFolderId = fid;
      Q_EMIT m_editor.selectedTableFolderIdChanged();
    }
    m_editor.setCurrentView(TableFolderView);
    return true;
  }

  return false;
}

/**
 * @brief Routes selections under the workspaces tree branch.
 */
bool EditorSelection::selectWorkspaceTreeItem(QStandardItem* item)
{
  if (item == m_editor.m_workspacesRootItem) {
    m_editor.setCurrentView(WorkspacesView);
    return true;
  }

  if (m_editor.m_workspaceItems.contains(item)) {
    const int wid = m_editor.m_workspaceItems.value(item);
    if (m_editor.m_selectedWorkspaceId != wid) {
      m_editor.m_selectedWorkspaceId = wid;
      Q_EMIT m_editor.selectedWorkspaceIdChanged();
    }
    m_editor.setCurrentView(WorkspaceView);
    return true;
  }

  if (m_editor.m_workspaceFolderItems.contains(item)) {
    const int fid = m_editor.m_workspaceFolderItems.value(item);
    if (m_editor.m_selectedFolderId != fid) {
      m_editor.m_selectedFolderId = fid;
      Q_EMIT m_editor.selectedFolderIdChanged();
    }
    m_editor.setCurrentView(WorkspaceFolderView);
    return true;
  }

  return false;
}

/**
 * @brief Routes selection of the single MQTT Publisher tree node.
 */
bool EditorSelection::selectMqttPublisherItem(QStandardItem* item)
{
  if (item != m_editor.m_mqttPublisherItem || item == nullptr)
    return false;

  m_editor.m_mqtt.buildMqttPublisherModel();
  m_editor.setCurrentView(MqttPublisherView);
  return true;
}

/**
 * @brief Routes selection of the single InfluxDB sink tree node.
 */
bool EditorSelection::selectInfluxSinkItem(QStandardItem* item)
{
  if (item != m_editor.m_influxSinkItem || item == nullptr)
    return false;

  m_editor.m_influx.buildInfluxSinkModel();
  m_editor.setCurrentView(InfluxSinkView);
  return true;
}

/**
 * @brief Switches to the control-script view when its tree node is selected.
 */
bool EditorSelection::selectControlScriptItem(QStandardItem* item)
{
  if (item != m_editor.m_controlScriptItem || item == nullptr)
    return false;

  m_editor.setCurrentView(ControlScriptView);
  return true;
}

/**
 * @brief Switches to the Lua library view when its tree node is selected.
 */
bool EditorSelection::selectTransformLibraryItem(QStandardItem* item)
{
  if (item != m_editor.m_transformLibraryItem || item == nullptr)
    return false;

  m_editor.setCurrentView(TransformLibraryView);
  return true;
}

/**
 * @brief Routes the Project Scripts node and the JavaScript library child to their views.
 */
bool EditorSelection::selectScriptsTreeItem(QStandardItem* item)
{
  if (item == nullptr)
    return false;

  if (item == m_editor.m_scriptsRootItem) {
    m_editor.setCurrentView(ProjectScriptsView);
    return true;
  }

  if (item == m_editor.m_jsLibraryItem) {
    m_editor.setCurrentView(JsLibraryView);
    return true;
  }

  return false;
}

/**
 * @brief Routes the Data Export node to its overview view.
 */
bool EditorSelection::selectExportRootItem(QStandardItem* item)
{
  if (item != m_editor.m_exportRootItem || item == nullptr)
    return false;

  m_editor.setCurrentView(DataExportView);
  return true;
}

/**
 * @brief Switches the active editor view based on the newly selected tree item.
 */
void EditorSelection::onCurrentSelectionChanged(const QModelIndex& current,
                                                const QModelIndex& previous)
{
  (void)previous;

  if (!m_editor.m_treeModel)
    return;

  if (m_editor.m_multiSelect.tryMultiSelection())
    return;

  m_editor.m_batchKind = ProjectEditor::KindNone;
  m_editor.m_batchItems.clear();

  if (!current.isValid())
    return;

  auto* item = m_editor.m_treeModel->itemFromIndex(current);
  if (!item)
    return;

  if (!m_nav.navigating() && m_nav.push(captureNavEntry(item)))
    Q_EMIT m_editor.navHistoryChanged();

  const bool handled =
    selectSourceParserItem(item) || selectSourceItem(item) || selectGroupItem(item)
    || selectGroupFolderItem(item) || selectDatasetItem(item) || selectActionItem(item)
    || selectOutputWidgetItem(item) || selectDataTableItem(item) || selectWorkspaceTreeItem(item)
    || selectMqttPublisherItem(item) || selectInfluxSinkItem(item) || selectControlScriptItem(item)
    || selectTransformLibraryItem(item) || selectScriptsTreeItem(item)
    || selectExportRootItem(item);

  if (!handled && m_editor.m_rootItems.contains(item)) {
    m_editor.setCurrentView(ProjectView);
    m_editor.m_forms.buildProjectModel();
  }

  Q_EMIT m_editor.editableOptionsChanged();
}

//--------------------------------------------------------------------------------------------------
// Back / forward navigation history
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when a previously visited tree node is available behind the cursor.
 */
bool EditorSelection::canGoBack() const noexcept
{
  return m_nav.canGoBack();
}

/**
 * @brief True when a visited tree node is available ahead of the cursor.
 */
bool EditorSelection::canGoForward() const noexcept
{
  return m_nav.canGoForward();
}

/**
 * @brief Snapshots a visited tree node as a rebuild-stable logical identity (roles for entity
 *        nodes; destination view for container/root nodes, which carry no TreeItemKind role).
 */
auto EditorSelection::captureNavEntry(QStandardItem* item) const -> NavEntry
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
      entry.key = m_editor.m_userTableItems.value(item);

    return entry;
  }

  entry.container = true;
  entry.valid     = true;
  if (m_editor.m_rootItems.contains(item)) {
    entry.view = static_cast<int>(ProjectView);
    return entry;
  }
  if (item == m_editor.m_groupsRootItem) {
    entry.view = static_cast<int>(GroupsView);
    return entry;
  }
  if (item == m_editor.m_tablesRootItem) {
    entry.view = static_cast<int>(DataTablesView);
    return entry;
  }
  if (item == m_editor.m_systemDatasetsItem) {
    entry.view = static_cast<int>(SystemDatasetsView);
    return entry;
  }
  if (item == m_editor.m_workspacesRootItem) {
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
QStandardItem* EditorSelection::resolveNavEntry(const NavEntry& entry) const
{
  if (!entry.valid)
    return nullptr;

  if (entry.container) {
    switch (static_cast<CurrentView>(entry.view)) {
      case ProjectView:
        return m_editor.m_rootItems.isEmpty() ? nullptr : m_editor.m_rootItems.firstKey();
      case GroupsView:
        return m_editor.m_groupsRootItem;
      case DataTablesView:
        return m_editor.m_tablesRootItem;
      case SystemDatasetsView:
        return m_editor.m_systemDatasetsItem;
      case WorkspacesView:
        return m_editor.m_workspacesRootItem;
      default:
        return nullptr;
    }
  }

  const int id  = entry.id;
  const int pid = entry.parentId;
  switch (static_cast<ItemKind>(entry.kind)) {
    case ProjectEditor::KindGroup:
      return navFind(m_editor.m_groupItems, [id](const auto& v) { return v.groupId == id; });
    case ProjectEditor::KindDataset:
      return navFind(m_editor.m_datasetItems,
                     [id, pid](const auto& v) { return v.groupId == pid && v.datasetId == id; });
    case ProjectEditor::KindAction:
      return navFind(m_editor.m_actionItems, [id](const auto& v) { return v.actionId == id; });
    case ProjectEditor::KindOutputWidget:
      return navFind(m_editor.m_outputWidgetItems,
                     [id, pid](const auto& v) { return v.groupId == pid && v.widgetId == id; });
    case ProjectEditor::KindSource:
      return navFind(m_editor.m_sourceItems, [id](const auto& v) { return v.sourceId == id; });
    case ProjectEditor::KindGroupFolder:
      return navFind(m_editor.m_groupFolderItems, [id](const auto& v) { return v == id; });
    case ProjectEditor::KindTableFolder:
      return navFind(m_editor.m_tableFolderItems, [id](const auto& v) { return v == id; });
    case ProjectEditor::KindWorkspace:
      return navFind(m_editor.m_workspaceItems, [id](const auto& v) { return v == id; });
    case ProjectEditor::KindWorkspaceFolder:
      return navFind(m_editor.m_workspaceFolderItems, [id](const auto& v) { return v == id; });
    case ProjectEditor::KindUserTable: {
      const QString& key = entry.key;
      return navFind(m_editor.m_userTableItems, [&key](const auto& v) { return v == key; });
    }
    case ProjectEditor::KindMqttPublisher:
      return m_editor.m_mqttPublisherItem;
    case ProjectEditor::KindInfluxSink:
      return m_editor.m_influxSinkItem;
    case ProjectEditor::KindControlScript:
      return m_editor.m_controlScriptItem;
    case ProjectEditor::KindTransformLibrary:
      return m_editor.m_transformLibraryItem;
    case ProjectEditor::KindJsLibrary:
      return m_editor.m_jsLibraryItem;
    case ProjectEditor::KindScriptsRoot:
      return m_editor.m_scriptsRootItem;
    case ProjectEditor::KindExportRoot:
      return m_editor.m_exportRootItem;
    default:
      return nullptr;
  }
}

/**
 * @brief Drops the whole history (used when a different project is loaded).
 */
void EditorSelection::clearNavHistory()
{
  if (m_nav.clear())
    Q_EMIT m_editor.navHistoryChanged();
}

/**
 * @brief Steps back to the nearest still-resolvable earlier node; the resolver skips deleted
 *        entries and the replay guard keeps the resulting selection out of the history.
 */
void EditorSelection::navigateBack()
{
  if (!canGoBack() || !m_editor.m_selectionModel)
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
  m_editor.m_selectionModel->setCurrentIndex(target->index(), QItemSelectionModel::ClearAndSelect);
  m_nav.setDirection(0);
  m_nav.setNavigating(false);
  Q_EMIT m_editor.navHistoryChanged();
}

/**
 * @brief Reveal direction of the in-flight selection change: -1 back, +1 forward, 0 normal.
 *        Read by the tree view during currentChanged to expand forward / collapse on back.
 */
int EditorSelection::navDirection() const noexcept
{
  return m_nav.direction();
}

/**
 * @brief Steps forward to the nearest still-resolvable later node; the resolver skips deleted
 *        entries and the replay guard keeps the resulting selection out of the history.
 */
void EditorSelection::navigateForward()
{
  if (!canGoForward() || !m_editor.m_selectionModel)
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
  m_editor.m_selectionModel->setCurrentIndex(target->index(), QItemSelectionModel::ClearAndSelect);
  m_nav.setDirection(0);
  m_nav.setNavigating(false);
  Q_EMIT m_editor.navHistoryChanged();
}

/**
 * @brief Selects the source item with the given sourceId in the tree.
 */
void EditorSelection::selectSource(int sourceId)
{
  if (!m_editor.m_selectionModel)
    return;

  for (auto it = m_editor.m_sourceItems.begin(); it != m_editor.m_sourceItems.end(); ++it) {
    if (it.value().sourceId == sourceId) {
      m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                 QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the group item with the given groupId in the tree.
 */
void EditorSelection::selectGroup(int groupId)
{
  if (!m_editor.m_selectionModel)
    return;

  for (auto it = m_editor.m_groupItems.begin(); it != m_editor.m_groupItems.end(); ++it) {
    if (it.value().groupId == groupId) {
      m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                 QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the dataset item (groupId, datasetId) in the tree.
 */
void EditorSelection::selectDataset(int groupId, int datasetId)
{
  if (!m_editor.m_selectionModel)
    return;

  for (auto it = m_editor.m_datasetItems.begin(); it != m_editor.m_datasetItems.end(); ++it) {
    if (it.value().groupId == groupId && it.value().datasetId == datasetId) {
      m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                 QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the action item with the given actionId in the tree.
 */
void EditorSelection::selectAction(int actionId)
{
  if (!m_editor.m_selectionModel)
    return;

  for (auto it = m_editor.m_actionItems.begin(); it != m_editor.m_actionItems.end(); ++it) {
    if (it.value().actionId == actionId) {
      m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                 QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the Frame Parser tree node for the given source.
 */
void EditorSelection::selectFrameParser(int sourceId)
{
  displayFrameParserView(sourceId);
}

/**
 * @brief Selects the output widget (groupId, widgetId) in the tree.
 */
void EditorSelection::selectOutputWidget(int groupId, int widgetId)
{
  if (!m_editor.m_selectionModel)
    return;

  for (auto it = m_editor.m_outputWidgetItems.begin(); it != m_editor.m_outputWidgetItems.end();
       ++it) {
    if (it.value().groupId == groupId && it.value().widgetId == widgetId) {
      m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                 QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the Frame Parser tree item for the given source, deferred.
 */
void EditorSelection::displayFrameParserView(int sourceId)
{
  QTimer::singleShot(100, &m_editor, [this, sourceId] {
    if (!m_editor.m_selectionModel)
      return;

    for (auto it = m_editor.m_sourceParserItems.begin(); it != m_editor.m_sourceParserItems.end();
         ++it) {
      if (it.value().sourceId != sourceId)
        continue;

      m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                 QItemSelectionModel::ClearAndSelect);
      break;
    }
  });
}

}  // namespace DataModel
