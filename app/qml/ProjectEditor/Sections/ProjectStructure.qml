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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import "../../Widgets" as Widgets
import "../../Commands" as Commands

Widgets.Pane {
  id: root

  title: qsTr("Project Structure")
  icon: Cpp_Misc_IconRegistry.icon("editor", "project-structure", 16)

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
        placeholderText: qsTr("Search…")
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

      Rectangle {
        z: -1
        parent: treeView
        anchors.fill: parent
        color: Cpp_ThemeManager.colors["base"]
      }

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
      // Right-clicked row, mirrored into the menu bindings on open. The cascade below needs
      // the folder ids locally, everything else lives on the bindings.
      //
      property int ctxItemId: -1
      property int ctxItemKind: ProjectEditor.KindNone
      property int moveExcludeId: -1
      property var dynamicFolderMenus: []

      //
      // Tree kind -> menu surface; a kind without a surface has no context menu.
      //
      readonly property var kindSurfaces: ({
        [ProjectEditor.KindProjectRoot]: "project-root",
        [ProjectEditor.KindSource]: "source",
        [ProjectEditor.KindFrameParser]: "frame-parser",
        [ProjectEditor.KindGroupsRoot]: "groups-root",
        [ProjectEditor.KindGroupFolder]: "group-folder",
        [ProjectEditor.KindGroup]: "group",
        [ProjectEditor.KindDataset]: "dataset",
        [ProjectEditor.KindOutputWidget]: "output",
        [ProjectEditor.KindAction]: "action",
        [ProjectEditor.KindTablesRoot]: "tables-root",
        [ProjectEditor.KindSystemDatasets]: "system-datasets",
        [ProjectEditor.KindTableFolder]: "table-folder",
        [ProjectEditor.KindUserTable]: "user-table",
        [ProjectEditor.KindWorkspacesRoot]: "workspaces-root",
        [ProjectEditor.KindWorkspaceFolder]: "workspace-folder",
        [ProjectEditor.KindWorkspace]: "workspace",
        [ProjectEditor.KindControlScript]: "control-script",
        [ProjectEditor.KindMqttPublisher]: "mqtt-publisher",
        [ProjectEditor.KindInfluxSink]: "influx-sink"
      })

      //
      // Opens the menu for one row; a kind with an output-panel group swaps to its own surface.
      //
      function openContextMenu(target) {
        const surface = treeView.surfaceFor(target)
        if (surface.length === 0)
          return

        treeView.ctxItemId = target.id
        treeView.ctxItemKind = target.kind
        treeView.moveExcludeId = treeView.isFolderKind(target.kind) ? target.id : -1
        treeView.refreshSelectableCount()
        menuBindings.setTarget(target)
        menuBindings.selectionCount = treeView.selectionCount()
        contextMenu.openSurface("editor-menu/" + surface)
      }

      //
      // Surface name for a target, or "" when the row carries no menu.
      //
      function surfaceFor(target) {
        if (target.kind === ProjectEditor.KindGroup && target.widget === "output-panel")
          return "output-panel"

        const surface = treeView.kindSurfaces[target.kind]
        return surface !== undefined ? surface : ""
      }

      //
      // True for the three folder kinds, which exclude themselves as a move destination.
      //
      function isFolderKind(kind) {
        return kind === ProjectEditor.KindGroupFolder
            || kind === ProjectEditor.KindTableFolder
            || kind === ProjectEditor.KindWorkspaceFolder
      }

      //
      // Rows currently selected, used to switch the menu between single and bulk entries.
      //
      function selectionCount() {
        return Cpp_JSON_ProjectEditor.selectedTreeItems()
        .filter(it => treeView.contentKinds.indexOf(it.kind) >= 0).length
      }

      //
      // Folder tree of the section the target belongs to.
      //
      function folderTreeForCtx() {
        const section = menuBindings.folderSection
        if (section === "group")
          return Cpp_JSON_ProjectEditor.groupFolderTree()

        if (section === "table")
          return Cpp_JSON_ProjectEditor.tableFolderTree()

        return Cpp_JSON_ProjectEditor.workspaceFolderTree()
      }

      //
      // Fills the "Move to Folder" cascade: one submenu per folder, drilled like a directory
      // tree. Returns the top-level menus so CommandMenu destroys them with the rest.
      //
      function buildFolderCascade(menu) {
        return treeView.populateMoveMenu(menu, treeView.folderTreeForCtx(), true)
      }

      function populateMoveMenu(menu, nodes, topLevel) {
        let created = []
        for (let i = 0; i < nodes.length; ++i) {
          const node = nodes[i]
          if (node.id === treeView.moveExcludeId)
            continue

          const kids = (node.children !== undefined) && (node.children.length > 0)
          const sub = _moveFolderMenu.createObject(
                      menu, { folderId2: node.id, folderTitle: node.title, hasChildren: kids })
          menu.addMenu(sub)
          if (topLevel)
            created.push({ "type": "menu", "obj": sub, "owner": menu })

          if (kids)
            treeView.populateMoveMenu(sub, node.children, false)
        }

        return created
      }

      //
      // Selection counters, pushed into the menu bindings so bulk entries can label and gate
      // themselves. Container rows (the section headers) never count as a selected item.
      //
      readonly property var contentKinds: [ProjectEditor.KindGroup, ProjectEditor.KindDataset,
                                           ProjectEditor.KindAction,
                                           ProjectEditor.KindOutputWidget,
                                           ProjectEditor.KindWorkspace,
                                           ProjectEditor.KindWorkspaceFolder,
                                           ProjectEditor.KindGroupFolder,
                                           ProjectEditor.KindUserTable,
                                           ProjectEditor.KindTableFolder,
                                           ProjectEditor.KindSource]

      function refreshSelectableCount() {
        const sel = Cpp_JSON_ProjectEditor.selectedTreeItems()
        menuBindings.selectableCount = sel.filter(
              it => it.kind === ProjectEditor.KindGroup
              || it.kind === ProjectEditor.KindDataset
              || it.kind === ProjectEditor.KindAction
              || it.kind === ProjectEditor.KindOutputWidget).length
        menuBindings.enableableCount = sel.filter(
              it => it.kind === ProjectEditor.KindGroup
              || it.kind === ProjectEditor.KindDataset
              || it.kind === ProjectEditor.KindGroupFolder).length
        menuBindings.deletableCount = sel.filter(
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

      //
      // Transient stack of nodes expanded by navigation (not by the user). Never persisted; it is
      // unwound one node per Back step and dropped on every rebuild (its indices go stale).
      //
      property var navExpandStack: []
      property bool rebuilding: false

      //
      // Expand into the current node via its model role (recorded for Back); no-op for leaves and
      // already-expanded nodes so saved expansions are never unwound.
      //
      function navExpandCurrent(idx) {
        if (!Cpp_JSON_ProjectEditor.treeIndexHasChildren(idx)
            || Cpp_JSON_ProjectEditor.treeIndexExpanded(idx))
          return

        Cpp_JSON_ProjectEditor.setTreeIndexExpanded(idx, true)
        treeView.navExpandStack = treeView.navExpandStack.concat([idx])
      }

      //
      // Collapse the most recently navigation-expanded node (the last Back step's counterpart).
      //
      function collapseLastNavExpanded() {
        const stack = treeView.navExpandStack
        if (stack.length === 0)
          return

        const idx = stack[stack.length - 1]
        treeView.navExpandStack = stack.slice(0, stack.length - 1)
        if (idx && idx.valid)
          Cpp_JSON_ProjectEditor.setTreeIndexExpanded(idx, false)
      }

      //
      // Reveal the current node as the user navigates (all via the model role, never the view):
      // Back collapses the last auto-expanded node, ancestors expand, forward expands into it.
      //
      function revealCurrent() {
        const idx = treeView.selectionModel ? treeView.selectionModel.currentIndex : null
        if (!idx || !idx.valid)
          return

        if (!treeView.rebuilding) {
          const dir = Cpp_JSON_ProjectEditor.navDirection()
          if (dir < 0)
            treeView.collapseLastNavExpanded()

          Cpp_JSON_ProjectEditor.expandTreeToIndex(idx)

          if (dir > 0)
            treeView.navExpandCurrent(idx)
        }

        Qt.callLater(function() {
          treeView.forceLayout()
          treeView.warmupContentHeight()
          const row = treeView.rowAtIndex(idx)
          if (row >= 0)
            treeView.positionViewAtRow(row, Qt.AlignVCenter)
        })
      }

      Connections {
        target: treeView.selectionModel
        function onSelectionChanged() { treeView.refreshSelectableCount() }
        function onCurrentChanged() {
          treeView.refreshSelectableCount()
          treeView.revealCurrent()
        }
      }

      Component.onCompleted: refreshSelectableCount()

      Commands.ProjectEditorMenuBindings {
        id: menuBindings

        treeSelection: true
      }

      Commands.CommandModel {
        id: menuModel

        context: "editor"
        bindingSets: [menuBindings]
      }

      Widgets.CommandMenu {
        id: contextMenu

        model: menuModel
        dynamicHandlers: ({ "folder-tree": treeView.buildFolderCascade })
        onClosed: menuBindings.clearTarget()
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
          icon.source: Cpp_Misc_IconRegistry.icon("widgets", "folder", 16)

          MenuItem {
            icon.width: 16
            icon.height: 16
            text: qsTr("Move Here")
            icon.source: Cpp_Misc_IconRegistry.icon("editor", "move-here", 16)
            onTriggered: menuBindings.moveToFolder(folderSubMenu.folderId2)
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
        // A fresh model invalidates the transient nav-expansion stack; guard against the
        // restore-selection reveal being mistaken for a user navigation.
        //
        function onTreeModelChanged() {
          treeView.rebuilding = true
          treeView.navExpandStack = []
        }

        //
        // Fired at the end of every buildTreeModel()
        //
        function onTreeRebuildFinished(revealIndex) {
          treeView.rebuilding = false

          // Snapshot before the new model resets contentY to 0
          const previousY = treeView.contentY

          Qt.callLater(function() {
            if (revealIndex && revealIndex.valid) {
              Cpp_JSON_ProjectEditor.expandTreeToIndex(revealIndex)
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
      // Right-click on the empty space under the last row opens the project menu; rows sit
      // above this handler, so a click on one never reaches it.
      //
      TapHandler {
        acceptedButtons: Qt.RightButton
        gesturePolicy: TapHandler.ReleaseWithinBounds
        onSingleTapped: {
          treeView.forceActiveFocus()
          treeView.openContextMenu({ "kind": ProjectEditor.KindProjectRoot, "id": -1 })
        }
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

                          event.accepted = true
                        }

                        // Move up to the previous sibling (or parent)
                        else if (event.key === Qt.Key_Up) {
                          let prevIndex = treeView.index(treeView.currentRow - 1, treeView.currentColumn)
                          if (prevIndex.isValid)
                          treeView.selectionModel.setCurrentIndex(prevIndex, ItemSelectionModel.ClearAndSelect)

                          event.accepted = true
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
        onModelExpandedChanged: syncExpandedState()

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
        readonly property bool itemEnabled: model.treeViewEnabled === undefined
                                            ? true : model.treeViewEnabled
        readonly property bool itemSelfEnabled: model.treeViewSelfEnabled === undefined
                                                ? true : model.treeViewSelfEnabled
        readonly property bool modelExpanded: model.treeViewExpanded === true

        //
        // Group flavour of the current row, for the menus that differ by widget type.
        //
        readonly property string widgetRole: {
          if (itemKind !== ProjectEditor.KindGroup)
            return ""

          if (Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel)
            return "output-panel"

          return Cpp_JSON_ProjectEditor.currentGroupIsPainter ? "painter" : ""
        }

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

                             treeView.openContextMenu({
                                                        "kind": item.itemKind,
                                                        "id": item.itemId,
                                                        "parentId": item.itemParentId,
                                                        "path": item.itemPath,
                                                        "enabled": item.itemSelfEnabled,
                                                        "widget": item.widgetRole
                                                      })
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
          opacity: item.itemEnabled ? 1.0 : 0.5
          anchors.leftMargin: padding + (isTreeNode ? depth * indentation : 0)

          //
          // Expanded indicator
          //
          Image {
            id: indicator

            enabled: hasChildren
            sourceSize: Qt.size(8, 8)
            opacity: hasChildren ? 1 : 0
            rotation: expanded ? 0 : 270
            Layout.alignment: Qt.AlignVCenter
            source: Cpp_Misc_IconRegistry.icon("editor", "indicator", 16)

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
            maximumLineCount: 1
            wrapMode: Text.NoWrap
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
            source: Cpp_Misc_IconRegistry.icon("editor", "save-warning", 24)
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
