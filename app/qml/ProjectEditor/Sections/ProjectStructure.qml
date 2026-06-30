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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  title: qsTr("Project Structure")
  icon: "qrc:/icons/project-editor/windows/project-structure.svg"

  signal groupClicked(var title)
  signal datasetClicked(var title)

  ColumnLayout {
    spacing: 0

    anchors {
      fill: parent
      leftMargin: -9
      topMargin: -16
      rightMargin: -9
      bottomMargin: -9
    }

    //
    // Search rectangle
    //
    Rectangle {
      id: searchBarOverlay

      implicitHeight: 48
      Layout.topMargin: -1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_background"]

      Rectangle {
        height: 1
        width: parent.width
        anchors.bottom: parent.bottom
        color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      Widgets.SearchField {
        id: searchField

        implicitHeight: 32
        placeholderText: qsTr("Search")
        color: Cpp_ThemeManager.colors["base"]
        text: Cpp_JSON_ProjectEditor.treeSearchQuery
        onTextChanged: Cpp_JSON_ProjectEditor.treeSearchQuery = text

        anchors {
          leftMargin: 6
          rightMargin: 6
          left: parent.left
          right: parent.right
          verticalCenter: parent.verticalCenter
        }
      }
    }

    //
    // Treeview
    //
    TreeView {
      id: treeView

      clip: true
      focus: true
      reuseItems: true
      interactive: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      boundsBehavior: Flickable.StopAtBounds
      model: Cpp_JSON_ProjectEditor.treeModel
      selectionModel: Cpp_JSON_ProjectEditor.selectionModel

      selectionBehavior: TableView.SelectRows
      selectionMode: TableView.ExtendedSelection

      //
      // Force layout walk so contentHeight is known before scrolling.
      //
      function warmupContentHeight() {
        const lastRow = treeView.rows - 1
        if (lastRow < 0 || treeView.height <= 0)
          return

        const savedY = treeView.contentY
        treeView.positionViewAtRow(lastRow, Qt.AlignBottom)
        treeView.forceLayout()

        const maxY = Math.max(0, treeView.contentHeight - treeView.height)
        treeView.contentY = Math.min(savedY, maxY)
      }

      onHeightChanged: Qt.callLater(warmupContentHeight)

      //
      // Shared context menu populated on right-click, outside the delegate.
      //
      property int ctxItemId: -1
      property int ctxItemParentId: -1
      property string ctxItemPath: ""
      property int ctxItemKind: ProjectEditor.KindNone
      property int moveExcludeId: -1
      property bool moveHasTargets: false
      property var moveTargetsTree: []
      property var dynamicMoveMenus: []

      readonly property bool ctxItemOrderable:
        ctxItemKind === ProjectEditor.KindGroup
        || ctxItemKind === ProjectEditor.KindDataset
        || ctxItemKind === ProjectEditor.KindAction
        || ctxItemKind === ProjectEditor.KindOutputWidget
        || ctxItemKind === ProjectEditor.KindWorkspace
        || ctxItemKind === ProjectEditor.KindWorkspaceFolder
        || ctxItemKind === ProjectEditor.KindGroupFolder
        || ctxItemKind === ProjectEditor.KindTableFolder

      readonly property bool ctxIsFolder:
        ctxItemKind === ProjectEditor.KindWorkspaceFolder
        || ctxItemKind === ProjectEditor.KindGroupFolder
        || ctxItemKind === ProjectEditor.KindTableFolder

      readonly property bool ctxSupportsFolders:
        ctxIsFolder
        || ctxItemKind === ProjectEditor.KindWorkspace
        || ctxItemKind === ProjectEditor.KindGroup
        || ctxItemKind === ProjectEditor.KindUserTable

      readonly property bool ctxRenameable:
        ctxIsFolder
        || ctxItemKind === ProjectEditor.KindGroup
        || ctxItemKind === ProjectEditor.KindDataset
        || ctxItemKind === ProjectEditor.KindAction
        || ctxItemKind === ProjectEditor.KindSource
        || ctxItemKind === ProjectEditor.KindWorkspace
        || ctxItemKind === ProjectEditor.KindUserTable

      function renameCtxItem() {
        if (ctxItemKind === ProjectEditor.KindGroup)
          Cpp_JSON_ProjectModel.promptRenameGroup(ctxItemId)
        else if (ctxItemKind === ProjectEditor.KindDataset)
          Cpp_JSON_ProjectModel.promptRenameDataset(ctxItemParentId, ctxItemId)
        else if (ctxItemKind === ProjectEditor.KindAction)
          Cpp_JSON_ProjectModel.promptRenameAction(ctxItemId)
        else if (ctxItemKind === ProjectEditor.KindSource)
          Cpp_JSON_ProjectModel.promptRenameSource(ctxItemId)
        else if (ctxItemKind === ProjectEditor.KindWorkspace)
          Cpp_JSON_ProjectModel.promptRenameWorkspace(ctxItemId)
        else if (ctxItemKind === ProjectEditor.KindUserTable)
          Cpp_JSON_ProjectModel.promptRenameTable(ctxItemPath)
        else if (ctxIsFolder)
          renameFolderForCtx()
      }

      function folderTreeForCtx() {
        if (ctxItemKind === ProjectEditor.KindGroup
            || ctxItemKind === ProjectEditor.KindGroupFolder)
          return Cpp_JSON_ProjectEditor.groupFolderTree()

        if (ctxItemKind === ProjectEditor.KindUserTable
            || ctxItemKind === ProjectEditor.KindTableFolder)
          return Cpp_JSON_ProjectEditor.tableFolderTree()

        return Cpp_JSON_ProjectEditor.workspaceFolderTree()
      }

      //
      // Build the cascading "Move to Folder" submenu (directory-explorer style): one level of
      // folders per menu, drilling into children. Rebuilt on each context-menu open.
      //
      function clearMoveMenus() {
        for (let i = 0; i < dynamicMoveMenus.length; ++i) {
          const m = dynamicMoveMenus[i]
          if (m) {
            moveToFolderMenu.removeMenu(m)
            m.destroy()
          }
        }

        dynamicMoveMenus = []
      }

      function populateMoveMenu(menu, nodes) {
        for (let i = 0; i < nodes.length; ++i) {
          const node = nodes[i]
          if (node.id === moveExcludeId)
            continue

          const kids = (node.children !== undefined) && (node.children.length > 0)
          const sub = _moveFolderMenu.createObject(
                        null, { folderId2: node.id, folderTitle: node.title, hasChildren: kids })
          menu.addMenu(sub)
          if (menu === moveToFolderMenu)
            dynamicMoveMenus.push(sub)

          if (kids)
            treeView.populateMoveMenu(sub, node.children)
        }
      }

      function rebuildMoveMenu() {
        clearMoveMenus()
        moveExcludeId   = ctxIsFolder ? ctxItemId : -1
        moveTargetsTree = folderTreeForCtx()
        populateMoveMenu(moveToFolderMenu, moveTargetsTree)
        moveHasTargets = dynamicMoveMenus.length > 0
      }

      function moveContextItemBy(direction) {
        if (ctxItemKind === ProjectEditor.KindGroup)
          Cpp_JSON_ProjectModel.moveGroup(ctxItemId, ctxItemId + direction)
        else if (ctxItemKind === ProjectEditor.KindDataset)
          Cpp_JSON_ProjectModel.moveDataset(ctxItemParentId,
                                            ctxItemId, ctxItemId + direction)
        else if (ctxItemKind === ProjectEditor.KindAction)
          Cpp_JSON_ProjectModel.moveAction(ctxItemId, ctxItemId + direction)
        else if (ctxItemKind === ProjectEditor.KindOutputWidget)
          Cpp_JSON_ProjectModel.moveOutputWidget(ctxItemParentId,
                                                 ctxItemId, ctxItemId + direction)
        else if (ctxItemKind === ProjectEditor.KindWorkspace)
          Cpp_JSON_ProjectEditor.moveWorkspace(ctxItemId, direction)
        else if (ctxItemKind === ProjectEditor.KindWorkspaceFolder)
          Cpp_JSON_ProjectModel.moveWorkspaceFolderInParent(ctxItemId, direction)
        else if (ctxItemKind === ProjectEditor.KindGroupFolder)
          Cpp_JSON_ProjectModel.moveGroupFolderInParent(ctxItemId, direction)
        else if (ctxItemKind === ProjectEditor.KindTableFolder)
          Cpp_JSON_ProjectModel.moveTableFolderInParent(ctxItemId, direction)
      }

      function ctxSectionOf(kind) {
        if (kind === ProjectEditor.KindGroup || kind === ProjectEditor.KindGroupFolder)
          return "group"

        if (kind === ProjectEditor.KindUserTable || kind === ProjectEditor.KindTableFolder)
          return "table"

        if (kind === ProjectEditor.KindWorkspace || kind === ProjectEditor.KindWorkspaceFolder)
          return "workspace"

        return ""
      }

      function moveCtxItemToFolder(folderId) {
        const section = ctxSectionOf(ctxItemKind)
        if (section === "")
          return

        const items = Cpp_JSON_ProjectEditor.selectedTreeItems()
                        .filter(it => treeView.ctxSectionOf(it.kind) === section)
        if (items.length > 0)
          Cpp_JSON_ProjectModel.moveSelectedItemsToFolder(items, folderId)
      }

      function newTopFolderForCtx() {
        if (ctxItemKind === ProjectEditor.KindGroup
            || ctxItemKind === ProjectEditor.KindGroupFolder)
          Cpp_JSON_ProjectModel.promptAddGroupFolder(-1)
        else if (ctxItemKind === ProjectEditor.KindUserTable
                 || ctxItemKind === ProjectEditor.KindTableFolder)
          Cpp_JSON_ProjectModel.promptAddTableFolder(-1)
        else
          Cpp_JSON_ProjectModel.promptAddWorkspaceFolder(-1)
      }

      function newSubFolderForCtx() {
        if (ctxItemKind === ProjectEditor.KindGroupFolder)
          Cpp_JSON_ProjectModel.promptAddGroupFolder(ctxItemId)
        else if (ctxItemKind === ProjectEditor.KindTableFolder)
          Cpp_JSON_ProjectModel.promptAddTableFolder(ctxItemId)
        else
          Cpp_JSON_ProjectModel.promptAddWorkspaceFolder(ctxItemId)
      }

      function renameFolderForCtx() {
        if (ctxItemKind === ProjectEditor.KindGroupFolder)
          Cpp_JSON_ProjectModel.promptRenameGroupFolder(ctxItemId)
        else if (ctxItemKind === ProjectEditor.KindTableFolder)
          Cpp_JSON_ProjectModel.promptRenameTableFolder(ctxItemId)
        else
          Cpp_JSON_ProjectModel.promptRenameWorkspaceFolder(ctxItemId)
      }

      //
      // Reactive selection counters
      //
      property int deletableSelectionCount: 0
      property int selectableSelectionCount: 0
      function refreshSelectableCount() {
        const sel = Cpp_JSON_ProjectEditor.selectedTreeItems()
        selectableSelectionCount = sel.filter(
          it => it.kind === ProjectEditor.KindGroup
                || it.kind === ProjectEditor.KindDataset
                || it.kind === ProjectEditor.KindAction
                || it.kind === ProjectEditor.KindOutputWidget).length
        deletableSelectionCount = sel.filter(
          it => it.kind === ProjectEditor.KindGroup
                || it.kind === ProjectEditor.KindDataset
                || it.kind === ProjectEditor.KindAction
                || it.kind === ProjectEditor.KindOutputWidget
                || it.kind === ProjectEditor.KindWorkspace
                || it.kind === ProjectEditor.KindWorkspaceFolder
                || it.kind === ProjectEditor.KindGroupFolder
                || it.kind === ProjectEditor.KindUserTable
                || it.kind === ProjectEditor.KindTableFolder).length
      }

      Connections {
        target: treeView.selectionModel
        function onSelectionChanged() { treeView.refreshSelectableCount() }
        function onCurrentChanged()   { treeView.refreshSelectableCount() }
      }

      Component.onCompleted: refreshSelectableCount()

      Menu {
        id: sharedContextMenu

        MenuItem {
          icon.width: 16
          icon.height: 16
          text: qsTr("Move Up")
          visible: treeView.ctxItemOrderable
          height: visible ? implicitHeight : 0
          icon.source: "qrc:/icons/project-editor/actions/move-up.svg"
          onTriggered: treeView.moveContextItemBy(-1)
        }

        MenuItem {
          icon.width: 16
          icon.height: 16
          text: qsTr("Move Down")
          visible: treeView.ctxItemOrderable
          height: visible ? implicitHeight : 0
          icon.source: "qrc:/icons/project-editor/actions/move-down.svg"
          onTriggered: treeView.moveContextItemBy(1)
        }

        MenuSeparator {
          height: visible ? implicitHeight : 0
          visible: treeView.ctxItemOrderable && treeView.deletableSelectionCount > 0
        }

        MenuItem {
          icon.width: 16
          icon.height: 16
          text: qsTr("Rename")
          visible: treeView.ctxRenameable
          height: visible ? implicitHeight : 0
          icon.source: "qrc:/icons/project-editor/actions/rename.svg"
          onTriggered: treeView.renameCtxItem()
        }

        //
        // Bulk-aware duplicate
        //
        MenuItem {
          icon.width: 16
          icon.height: 16
          text: treeView.selectableSelectionCount > 1
                ? qsTr("Duplicate Selected (%1)").arg(treeView.selectableSelectionCount)
                : qsTr("Duplicate")
          visible: treeView.selectableSelectionCount > 0
          height: visible ? implicitHeight : 0
          icon.source: "qrc:/icons/project-editor/actions/duplicate.svg"
          onTriggered: {
            const items = Cpp_JSON_ProjectEditor.selectedTreeItems()
                            .filter(it => it.kind !== ProjectEditor.KindWorkspace
                                          && it.kind !== ProjectEditor.KindWorkspaceFolder
                                          && it.kind !== ProjectEditor.KindNone)
            if (items.length > 0)
              Cpp_JSON_ProjectModel.duplicateSelectedItems(items)
          }
        }

        MenuItem {
          icon.width: 16
          icon.height: 16
          text: treeView.deletableSelectionCount > 1
                ? qsTr("Delete Selected (%1)").arg(treeView.deletableSelectionCount)
                : qsTr("Delete")
          visible: treeView.deletableSelectionCount > 0
          height: visible ? implicitHeight : 0
          icon.source: "qrc:/icons/project-editor/actions/delete.svg"
          onTriggered: {
            const items = Cpp_JSON_ProjectEditor.selectedTreeItems()
                            .filter(it => it.kind !== ProjectEditor.KindNone)
            if (items.length === 1) {
              const it = items[0]
              if (it.kind === ProjectEditor.KindWorkspace) {
                Cpp_JSON_ProjectModel.confirmDeleteWorkspace(it.id)
                return
              }

              if (it.kind === ProjectEditor.KindWorkspaceFolder) {
                Cpp_JSON_ProjectModel.confirmDeleteWorkspaceFolder(it.id)
                return
              }

              if (it.kind === ProjectEditor.KindGroupFolder) {
                Cpp_JSON_ProjectModel.confirmDeleteGroupFolder(it.id)
                return
              }

              if (it.kind === ProjectEditor.KindTableFolder) {
                Cpp_JSON_ProjectModel.confirmDeleteTableFolder(it.id)
                return
              }

              if (it.kind === ProjectEditor.KindUserTable) {
                Cpp_JSON_ProjectModel.confirmDeleteTable(it.path)
                return
              }
            }

            if (items.length > 0)
              Cpp_JSON_ProjectModel.confirmDeleteSelectedItems(items)
          }
        }

        //
        // Folder actions (workspaces, groups, shared-memory tables)
        //
        MenuSeparator {
          height: visible ? implicitHeight : 0
          visible: treeView.ctxSupportsFolders
        }

        MenuItem {
          icon.width: 16
          icon.height: 16
          text: qsTr("New Folder")
          height: visible ? implicitHeight : 0
          visible: treeView.ctxSupportsFolders
          icon.source: "qrc:/icons/project-editor/actions/add-folder.svg"
          onTriggered: treeView.newTopFolderForCtx()
        }

        MenuItem {
          icon.width: 16
          icon.height: 16
          text: qsTr("New Sub-folder")
          visible: treeView.ctxIsFolder
          height: visible ? implicitHeight : 0
          icon.source: "qrc:/icons/project-editor/actions/add-folder.svg"
          onTriggered: treeView.newSubFolderForCtx()
        }

        //
        // Move an item or folder into another folder, navigated like a directory tree.
        // Child folders are filled dynamically by rebuildMoveMenu() on each open.
        //
        Menu {
          id: moveToFolderMenu

          icon.width: 16
          icon.height: 16
          title: qsTr("Move to Folder")
          enabled: treeView.ctxSupportsFolders
          icon.source: "qrc:/icons/project-editor/treeview/folder.svg"

          MenuItem {
            icon.width: 16
            icon.height: 16
            text: qsTr("Top Level")
            icon.source: "qrc:/icons/project-editor/treeview/folder.svg"
            onTriggered: treeView.moveCtxItemToFolder(-1)
          }

          MenuSeparator {
            visible: treeView.moveHasTargets
            height: visible ? implicitHeight : 0
          }
        }
      }

      //
      // One cascading sub-menu per folder: a "Move Here" target plus its child folders.
      //
      Component {
        id: _moveFolderMenu

        Menu {
          id: folderSubMenu

          property int folderId2: -1
          property string folderTitle: ""
          property bool hasChildren: false

          icon.width: 16
          icon.height: 16
          title: folderTitle
          icon.source: "qrc:/icons/project-editor/treeview/folder.svg"

          MenuItem {
            icon.width: 16
            icon.height: 16
            text: qsTr("Move Here")
            icon.source: "qrc:/icons/project-editor/treeview/folder.svg"
            onTriggered: treeView.moveCtxItemToFolder(folderSubMenu.folderId2)
          }

          MenuSeparator {
            visible: folderSubMenu.hasChildren
            height: visible ? implicitHeight : 0
          }
        }
      }

      Connections {
        target: Cpp_JSON_ProjectEditor

        //
        // Fired at the end of every buildTreeModel()
        //
        function onTreeRebuildFinished(revealIndex) {
          // Snapshot before the new model resets contentY to 0
          const previousY = treeView.contentY

          Qt.callLater(function() {
            if (revealIndex && revealIndex.valid) {
              treeView.expandToIndex(revealIndex)
              treeView.forceLayout()
              treeView.warmupContentHeight()
              const row = treeView.rowAtIndex(revealIndex)
              if (row >= 0)
                treeView.positionViewAtRow(row, Qt.AlignVCenter)

              return
            }

            treeView.warmupContentHeight()
            const maxY = Math.max(0, treeView.contentHeight - treeView.height)
            treeView.contentY = Math.min(previousY, maxY)
          })
        }
      }

      ScrollBar.vertical: ScrollBar {
        policy: treeView.contentHeight > treeView.height ? ScrollBar.AlwaysOn :
                                                           ScrollBar.AsNeeded
      }

      //
      // Reorder the current selection (group/dataset/action/workspace)
      //
      function reorderCurrentSelection(direction) {
        const view = Cpp_JSON_ProjectEditor.currentView
        if (view === ProjectEditor.GroupView)
          return Cpp_JSON_ProjectEditor.moveCurrentGroup(direction)

        if (view === ProjectEditor.DatasetView)
          return Cpp_JSON_ProjectEditor.moveCurrentDataset(direction)

        if (view === ProjectEditor.ActionView)
          return Cpp_JSON_ProjectEditor.moveCurrentAction(direction)

        if (view === ProjectEditor.OutputWidgetView)
          return Cpp_JSON_ProjectEditor.moveCurrentOutputWidget(direction)

        if (view === ProjectEditor.WorkspaceView)
          return Cpp_JSON_ProjectEditor.moveWorkspace(
                   Cpp_JSON_ProjectEditor.selectedWorkspaceId, direction)

        return false
      }

      //
      // Keyboard navigation
      //
      Keys.onPressed: (event) => {
                        // Alt+Up / Alt+Down: reorder the current selection
                        if ((event.modifiers & Qt.AltModifier)
                            && (event.key === Qt.Key_Up || event.key === Qt.Key_Down)) {
                          const dir = event.key === Qt.Key_Up ? -1 : 1
                          if (treeView.reorderCurrentSelection(dir))
                            event.accepted = true

                          return
                        }

                        // Move down to the next sibling (or parent if collapsed)
                        if (event.key === Qt.Key_Down) {
                          let nextIndex = treeView.index(treeView.currentRow + 1, treeView.currentColumn)
                          if (nextIndex.isValid)
                          treeView.selectionModel.setCurrentIndex(nextIndex, ItemSelectionModel.ClearAndSelect)
                        }

                        // Move up to the previous sibling (or parent)
                        else if (event.key === Qt.Key_Up) {
                          let prevIndex = treeView.index(treeView.currentRow - 1, treeView.currentColumn)
                          if (prevIndex.isValid)
                          treeView.selectionModel.setCurrentIndex(prevIndex, ItemSelectionModel.ClearAndSelect)
                        }

                        else if (event.key === Qt.Key_Delete) {
                          const items = Cpp_JSON_ProjectEditor.selectedTreeItems()
                            .filter(it => it.kind !== ProjectEditor.KindWorkspace
                                          && it.kind !== ProjectEditor.KindNone)
                          if (items.length > 0) {
                            Cpp_JSON_ProjectModel.confirmDeleteSelectedItems(items)
                            event.accepted = true
                          } else if (Cpp_JSON_ProjectEditor.currentView === ProjectEditor.DatasetView) {
                            Cpp_JSON_ProjectModel.deleteCurrentDataset()
                          } else if (Cpp_JSON_ProjectEditor.currentView === ProjectEditor.GroupView) {
                            Cpp_JSON_ProjectModel.deleteCurrentGroup()
                          }
                        }
                      }

      //
      // Set background item
      //
      delegate: Item {
        id: item

        implicitWidth: treeView.width
        implicitHeight: depth === 0 ? 30 : 18
        TableView.onReused: syncExpandedState()
        Component.onCompleted: syncExpandedState()

        required property int row
        required property int depth
        required property int column
        required property bool current
        required property bool selected
        required property bool expanded
        required property bool isTreeNode
        required property bool hasChildren
        required property TreeView treeView

        readonly property real padding: 4
        readonly property real indentation: 16

        readonly property int itemKind: model.treeItemKind === undefined
                                        ? ProjectEditor.KindNone
                                        : model.treeItemKind
        readonly property int itemId: model.treeItemId === undefined ? -1 : model.treeItemId
        readonly property int itemParentId: model.treeItemParentId === undefined
                                            ? -1 : model.treeItemParentId
        readonly property string itemPath: model.treeItemPath === undefined
                                           ? "" : model.treeItemPath

        //
        // Restore expanded state from C++ model
        //
        function syncExpandedState() {
          if (model.treeViewExpanded === true)
            treeView.expand(row)
          else
            treeView.collapse(row)
        }

        //
        // Show/hide the children of the current item.
        //
        function toggleExpanded() {
          if (hasChildren) {
            treeView.toggleExpanded(row)
            model.treeViewExpanded = expanded
            Cpp_JSON_ProjectEditor.persistTreeExpansion()
            return true
          }

          return false
        }

        //
        // Select the item and open the associated view automatically.
        // Skip selection for spacer items (items with only whitespace text).
        //
        function onLabelClicked() {
          if (model.treeViewText.trim().length === 0)
            return

          treeView.forceActiveFocus()
          let index = treeView.index(row, column)
          treeView.selectionModel.setCurrentIndex(index, ItemSelectionModel.ClearAndSelect)
        }

        //
        // If item has children, expand on double click.
        // Otherwise, select the item and open the associated view.
        //
        function onLabelDoubleClicked() {
          if (model.treeViewText.trim().length === 0)
            return

          treeView.forceActiveFocus()
          if (!toggleExpanded()) {
            onLabelClicked()
          }
        }

        //
        // Item background + click/right-click handler
        //
        Rectangle {
          id: background

          anchors.fill: parent
          color: (selected || current) ? Cpp_ThemeManager.colors["highlight"] : "transparent"

          MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: (mouse) => {
              const idx = treeView.index(row, column)

              //
              // Right click: preserve multi-selection when the click lands on an
              // already-selected row; otherwise drop to single-select on that row.
              //
              if (mouse.button === Qt.RightButton) {
                if (item.itemKind !== ProjectEditor.KindNone) {
                  treeView.forceActiveFocus()
                  if (!treeView.selectionModel.isSelected(idx))
                    onLabelClicked()

                  treeView.ctxItemKind     = item.itemKind
                  treeView.ctxItemId       = item.itemId
                  treeView.ctxItemParentId = item.itemParentId
                  treeView.ctxItemPath     = item.itemPath
                  treeView.rebuildMoveMenu()
                  sharedContextMenu.popup()
                }
                return
              }

              //
              // Left click: modifiers drive multi-select. Skip spacer rows so
              // they still cannot become "selected".
              //
              if (model.treeViewText.trim().length === 0) {
                onLabelClicked()
                return
              }

              treeView.forceActiveFocus()

              if (mouse.modifiers & Qt.ControlModifier) {
                treeView.selectionModel.select(idx, ItemSelectionModel.Toggle)
                treeView.selectionModel.setCurrentIndex(idx, ItemSelectionModel.NoUpdate)
                return
              }

              if (mouse.modifiers & Qt.ShiftModifier) {
                const curIdx = treeView.selectionModel.currentIndex
                const anchorRow = curIdx && curIdx.valid ? treeView.rowAtIndex(curIdx) : -1
                if (anchorRow >= 0 && anchorRow !== row) {
                  const startRow = Math.min(anchorRow, row)
                  const endRow   = Math.max(anchorRow, row)
                  treeView.selectionModel.clear()
                  for (let r = startRow; r <= endRow; ++r) {
                    const ri = treeView.index(r, column)
                    if (ri && ri.valid)
                      treeView.selectionModel.select(ri, ItemSelectionModel.Select)
                  }

                  treeView.selectionModel.setCurrentIndex(idx, ItemSelectionModel.NoUpdate)
                  return
                }
              }

              onLabelClicked()
            }
            onDoubleClicked: onLabelDoubleClicked()
          }
        }

        //
        // Item controls
        //
        RowLayout {
          spacing: 0
          anchors.fill: parent
          anchors.rightMargin: 16
          anchors.leftMargin: padding + (isTreeNode ? depth * indentation : 0)

          //
          // Expanded indicator
          //
          Image {
            id: indicator

            enabled: hasChildren
            sourceSize: Qt.size(8, 8)
            opacity: hasChildren ? 1 : 0
            Layout.alignment: Qt.AlignVCenter
            rotation: model.treeViewExpanded ? 0 : 270
            source: "qrc:/icons/project-editor/treeview/indicator.svg"

            MouseArea {
              anchors.fill: parent
              onClicked: toggleExpanded()
            }
          }

          //
          // Spacer
          //
          Item {
            width: 6
          }

          //
          // Item icon
          //
          Image {
            id: icon

            source: model.treeViewIcon ?? ""
            sourceSize: Qt.size(12, 12)
            Layout.alignment: Qt.AlignVCenter
          }

          //
          // Spacer
          //
          Item {
            width: 4
          }

          //
          // Item text
          //
          Label {
            id: label

            readonly property bool stale: model.treeViewWorkspaceStale === true

            Layout.fillWidth: true
            elide: Label.ElideRight
            text: model.treeViewText ?? ""
            Layout.alignment: Qt.AlignVCenter
            LayoutMirroring.enabled: false
            horizontalAlignment: Cpp_Misc_Translator.rtl ? Text.AlignRight
                                                         : Text.AlignLeft
            font: depth === 0 ? Cpp_Misc_CommonFonts.boldUiFont :
                                Cpp_Misc_CommonFonts.uiFont
            color: (selected || current)
                   ? Cpp_ThemeManager.colors["highlighted_text"]
                   : (label.stale ? "#B8860B" : Cpp_ThemeManager.colors["text"])
          }

          Label {
            id: sourceBadge

            opacity: (selected || current) ? 1.0 : 0.85
            font: Cpp_Misc_CommonFonts.monoFont
            text: "[" + String.fromCharCode(65 + model.treeViewSourceId) + "]"
            visible: model.treeViewSourceName !== undefined
                     && model.treeViewSourceName !== ""
            Layout.alignment: Qt.AlignVCenter
            color: {
              if (selected || current)
                return Cpp_ThemeManager.colors["highlighted_text"]

              if (Cpp_JSON_ProjectModel.sourceCount > 1)
                return SerialStudio.getDeviceColor(model.treeViewSourceId + 1)

              return Cpp_ThemeManager.colors["text"]
            }
          }

          Label {
            id: frameIndex

            opacity: (selected || current) ? 1.0 : 0.85
            font: Cpp_Misc_CommonFonts.monoFont
            visible: depth > 1 && (model.treeViewVirtual === true
                                   || model.treeViewFrameIndex >= 0
                                   || model.treeViewFrameIndex === -2)
            text: {
              if (model.treeViewVirtual === true)
                return "[VRT]"

              var letter = String.fromCharCode(65 + model.treeViewSourceId)
              if (model.treeViewFrameIndex === -2)
                return "[" + letter + "]"

              return "[" + letter + "-" + model.treeViewFrameIndex + "]"
            }
            Layout.alignment: Qt.AlignVCenter
            color: {
              if (selected || current)
                return Cpp_ThemeManager.colors["highlighted_text"]

              if (Cpp_JSON_ProjectModel.sourceCount > 1)
                return SerialStudio.getDeviceColor(model.treeViewSourceId + 1)

              return Cpp_ThemeManager.colors["text"]
            }
          }
        }
      }
    }

    //
    // Save-status banner
    //
    Rectangle {
      id: saveBlockerBanner

      Layout.fillWidth: true
      visible: !Cpp_JSON_ProjectModel.canSave
      implicitHeight: bannerLayout.implicitHeight + 24
      color: Cpp_ThemeManager.colors["groupbox_background"]

      Rectangle {
        height: 1
        width: parent.width
        anchors.top: parent.top
        color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      ColumnLayout {
        id: bannerLayout

        spacing: 4
        anchors.margins: 12
        anchors.fill: parent

        RowLayout {
          spacing: 10
          Layout.fillWidth: true

          Image {
            sourceSize: Qt.size(24, 24)
            Layout.alignment: Qt.AlignTop
            source: "qrc:/icons/project-editor/treeview/save-warning.svg"
          }

          Label {
            Layout.fillWidth: true
            wrapMode: Label.WordWrap
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["text"]
            text: Cpp_JSON_ProjectModel.saveBlockerTitle
            font: Cpp_Misc_CommonFonts.customUiFont(1.2, true)
          }   
        }

        Label {
          opacity: 0.85
          Layout.fillWidth: true
          wrapMode: Label.WordWrap
          color: Cpp_ThemeManager.colors["text"]
          text: Cpp_JSON_ProjectModel.saveBlockerDetail
        }
      }
    }
  }
}
