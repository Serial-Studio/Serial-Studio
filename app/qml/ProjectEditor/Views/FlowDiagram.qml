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
import QtQuick.Controls

import SerialStudio

import "../../Widgets" as Widgets
import "../../Commands" as Commands
import "FlowDiagram" as Diagram

Item {
  id: root

  //
  // Zoom state
  //
  property real zoom:    1.0
  property real maxZoom: 3.0
  property real minZoom: 0.25

  //
  // Layout output
  //
  property var  nodes:    []
  property var  arrows:   []
  property real contentW: 0
  property real contentH: 0

  //
  // Collapse state: a map of stable-id node keys (grpfolder:<id>, tblfolder:<id>, grp:<groupId>)
  // whose children are hidden. Seeded from the project file and persisted back as editor UI state.
  //
  property var collapsedKeys: ({})

  function seedCollapseFromModel() {
    collapsedKeys = Object.assign({}, Cpp_JSON_ProjectModel.diagramCollapse)
  }

  function toggleCollapse(key) {
    const next = Object.assign({}, root.collapsedKeys)
    next[key] = !diagramLayout.isCollapsed(key)

    root.collapsedKeys = next
    Cpp_JSON_ProjectModel.setDiagramCollapse(next)
    root.reloadDiagram()
  }

  //
  // Public interface.
  //
  function reloadDiagram() {
    const diagram = diagramLayout.build(
      Cpp_JSON_ProjectModel.sourcesForDiagram(),
      Cpp_JSON_ProjectModel.groupsForDiagram(),
      Cpp_JSON_ProjectModel.actionsForDiagram(),
      Cpp_JSON_ProjectModel.tablesForDiagram()
    )

    root.contentW = diagram.contentW
    root.contentH = diagram.contentH
    root.nodes    = diagram.nodes
    root.arrows   = diagram.arrows

    canvas.requestPaint()
  }

  function resetZoom() {
    zoom = 1.0
    flickable.contentX = 0
    flickable.contentY = 0
  }

  //
  // Double-click on a node: reveal it in the project editor.
  //
  function selectNode(node) {
    switch (node.type) {
      case "source":
        Cpp_JSON_ProjectEditor.selectSource(node.sourceId)
        break
      case "frameparser":
        Cpp_JSON_ProjectEditor.selectFrameParser(node.sourceId)
        break
      case "group":
        Cpp_JSON_ProjectEditor.selectGroup(node.groupId)
        break
      case "dataset":
        Cpp_JSON_ProjectEditor.selectDataset(node.groupId, node.datasetId)
        break
      case "output":
        Cpp_JSON_ProjectEditor.selectOutputWidget(node.groupId, node.widgetId)
        break
      case "output-panel":
        Cpp_JSON_ProjectEditor.selectGroup(node.groupId)
        break
      case "action":
        Cpp_JSON_ProjectEditor.selectAction(node.actionId)
        break
      case "table":
        Cpp_JSON_ProjectEditor.selectUserTable(node.tableName)
        break
      case "controlscript":
        Cpp_JSON_ProjectEditor.selectControlScript()
        break
      case "transform":
        Cpp_JSON_ProjectEditor.openTransformEditorFor(node.groupId, node.datasetId)
        break
      case "mqtt-publisher":
        Cpp_JSON_ProjectEditor.selectMqttPublisher()
        break
    }
  }

  //
  // Geometry engine + its icon vocabulary.
  //
  Diagram.DiagramIcons {
    id: diagramIcons
  }

  Diagram.DiagramLayout {
    id: diagramLayout

    icons: diagramIcons
    collapsedKeys: root.collapsedKeys
  }

  //
  // Reactive connections.
  //
  Connections {
    target: Cpp_JSON_ProjectModel
    function onGroupsChanged()     { root.reloadDiagram() }
    function onGroupDataChanged()  { root.reloadDiagram() }
    function onActionsChanged()    { root.reloadDiagram() }
    function onSourcesChanged()       { root.reloadDiagram() }
    function onTablesChanged()        { root.reloadDiagram() }
    function onTitleChanged()         { root.reloadDiagram() }
    function onControlScriptChanged() { root.reloadDiagram() }
    function onJsonFileChanged()      { root.seedCollapseFromModel(); root.reloadDiagram() }
  }

  //
  // Repaint when the publisher is toggled on/off so the node appears/disappears.
  //
  Loader {
    active: Cpp_CommercialBuild
    sourceComponent: Connections {
      target: Cpp_MQTT_Publisher
      function onConfigurationChanged() { root.reloadDiagram() }
    }
  }

  //
  // Re-flow the diagram when the active language toggles RTL/LTR.
  //
  Connections {
    target: Cpp_Misc_Translator
    function onLanguageChanged() { root.reloadDiagram() }
  }

  Component.onCompleted: { seedCollapseFromModel(); reloadDiagram() }
  onVisibleChanged: if (visible) { reloadDiagram(); canvas.requestPaint() }
  onWidthChanged:   if (visible && width > 0) reloadDiagram()
  onHeightChanged:  if (visible && height > 0) reloadDiagram()

  //
  // Flickable + scaled canvas.
  //
  Flickable {
    id: flickable

    clip: true
    anchors.fill: parent
    contentWidth:  Math.max(root.contentW * root.zoom, width)
    contentHeight: Math.max(root.contentH * root.zoom, height)
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical:   ScrollBar { policy: ScrollBar.AsNeeded }
    ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }

    //
    // Ctrl+Wheel -> zoom toward cursor
    //
    WheelHandler {
      acceptedModifiers: Qt.ControlModifier
      onWheel: (ev) => {
        const factor  = ev.angleDelta.y > 0 ? 1.1 : 0.9
        const newZoom = Math.min(root.maxZoom,
                          Math.max(root.minZoom, root.zoom * factor))
        const cx = flickable.contentX + ev.x
        const cy = flickable.contentY + ev.y
        const r  = newZoom / root.zoom
        root.zoom          = newZoom
        flickable.contentX = cx * r - ev.x
        flickable.contentY = cy * r - ev.y
      }
    }

    //
    // Plain wheel -> vertical scroll; Shift+Wheel -> horizontal
    //
    WheelHandler {
      acceptedModifiers: Qt.NoModifier
      onWheel: (ev) => {
        if (ev.modifiers & Qt.ShiftModifier)
          flickable.contentX = Math.max(0,
            Math.min(flickable.contentX - ev.angleDelta.y * 0.5,
                     flickable.contentWidth  - flickable.width))
        else
          flickable.contentY = Math.max(0,
            Math.min(flickable.contentY - ev.angleDelta.y * 0.5,
                     flickable.contentHeight - flickable.height))
      }
    }

    //
    // Middle-button drag -> pan
    //
    MouseArea {
      property real lx: 0
      property real ly: 0
      anchors.fill: parent
      acceptedButtons: Qt.MiddleButton
      cursorShape: pressed ? Qt.ClosedHandCursor : Qt.ArrowCursor
      onPressed:         (m) => { lx = m.x; ly = m.y }
      onPositionChanged: (m) => {
        flickable.contentX -= m.x - lx
        flickable.contentY -= m.y - ly
        lx = m.x; ly = m.y
      }
    }

    //
    // Scaled content item (sized to max(content, viewport) for the bg MouseArea)
    //
    Item {
      width:  Math.max(root.contentW * root.zoom, flickable.width)
      height: Math.max(root.contentH * root.zoom, flickable.height)

      //
      // Background right-click -> "Add ..." menu
      //
      MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: menuController.openForBackground()
      }

      //
      // Arrow canvas
      //
      Diagram.DiagramArrows {
        id: canvas

        zoom: root.zoom
        arrows: root.arrows
        anchors.fill: parent

        onAvailableChanged: if (available) root.reloadDiagram()
      }

      //
      // Node repeater
      //
      Repeater {
        model: root.nodes

        delegate: Diagram.DiagramNode {
          zoom: root.zoom
          pinnedKey: menuController.pinnedKey

          onActivated: (node) => root.selectNode(node)
          onCollapseToggled: (key) => root.toggleCollapse(key)
          onContextMenuRequested: (node) => menuController.openForNode(node)
        }
      }
    }
  }


  //
  // Painter Code Dialog (commercial; resolved on first open)
  //
  Loader {
    id: painterCodeDialog

    active: false
    asynchronous: false
    source: "qrc:/serial-studio.com/gui/qml/ProjectEditor/Dialogs/PainterCodeDialog.qml"

    function showDialog() {
      painterCodeDialog.active = true
      if (painterCodeDialog.item) {
        painterCodeDialog.item.closing.connect(() => painterCodeDialog.active = false)
        painterCodeDialog.item.showDialog()
      }
    }
  }

  //
  // Menu controller (shared state for per-context Menus, no visible: false items)
  //
  Diagram.DiagramMenuController {
    id: menuController

    menu: contextMenu
    bindings: menuBindings
  }

  //
  // Context-menu plumbing: node ids go into the bindings, layout comes from the registry
  // surface for that kind, and mutations run with the current view pinned.
  //
  Commands.ProjectEditorMenuBindings {
    id: menuBindings

    suppressViewChange: true
    onPainterCodeRequested: painterCodeDialog.showDialog()
  }

  Commands.CommandModel {
    id: menuModel

    context: "editor"
    bindingSets: [menuBindings]
  }

  Widgets.CommandMenu {
    id: contextMenu

    model: menuModel
    onClosed: {
      menuBindings.clearTarget()
      menuController.unpin()
    }
  }
}
