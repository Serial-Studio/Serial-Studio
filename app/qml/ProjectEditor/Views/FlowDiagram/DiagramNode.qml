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

//
// One positioned node of the flow diagram: picks the visual for the node type,
// tracks hover/pin highlighting and turns input into signals.
//
Item {
  id: nd

  //
  // Input properties: the layout node record, the view's zoom and the key of
  // the node whose context menu is currently open.
  //
  required property var modelData
  property real zoom: 1.0
  property string pinnedKey: ""

  //
  // Emitted on a chevron/folder click; the view owns the collapse map
  //
  signal collapseToggled(string key)

  //
  // Emitted on right-click; the view opens the registry menu surface
  //
  signal contextMenuRequested(var node)

  //
  // Emitted on left double-click; the view selects the node in the editor
  //
  signal activated(var node)

  x:      nd.modelData.x * nd.zoom
  y:      nd.modelData.y * nd.zoom
  width:  nd.modelData.w * nd.zoom
  height: nd.modelData.h * nd.zoom

  property bool hovered:       false
  property bool isPill:        isDataset || isOutput
  property bool isTable:       nd.modelData.type === "table"
  property bool isOutput:      nd.modelData.type === "output"
  property bool isAction:      nd.modelData.type === "action"
  property bool isSource:      nd.modelData.type === "source"
  property bool isDataset:     nd.modelData.type === "dataset"
  property bool isTransform:   nd.modelData.type === "transform"
  property bool isFP:          nd.modelData.type === "frameparser"
  property bool isGroupFolder: nd.modelData.type === "groupfolder"
  property bool isTableFolder: nd.modelData.type === "tablefolder"
  property bool isOutputPanel: nd.modelData.type === "output-panel"
  property bool isOutputsFolder: nd.modelData.type === "outputsfolder"
  property bool isControlScript: nd.modelData.type === "controlscript"
  property bool collapsibleCard: !isFolder && !!nd.modelData.collapseKey
  property bool isFolder: isGroupFolder || isTableFolder || isOutputsFolder

  readonly property string nodeKey: nd.modelData.key !== undefined
                                    ? nd.modelData.key : ""
  readonly property bool isPinned: nd.pinnedKey === nodeKey
                                   && nodeKey !== ""
  readonly property bool active: hovered || isPinned

  //
  // -- Pill (dataset / output widget) ---------------------------
  //
  NodePill {
    zoom: nd.zoom
    active: nd.active
    node: nd.modelData
    visible: nd.isPill
    anchors.fill: parent
  }

  //
  // Transform block (its own node, left-click opens editor)
  //
  NodeTransform {
    zoom: nd.zoom
    active: nd.active
    anchors.fill: parent
    visible: nd.isTransform
    showTooltip: nd.hovered && nd.isTransform
  }

  //
  // -- Card (source / group / frame-parser / action / table) ----
  //
  NodeCard {
    isFP: nd.isFP
    zoom: nd.zoom
    active: nd.active
    node: nd.modelData
    anchors.fill: parent
    isSource: nd.isSource
    badgeVisible: nd.isSource || nd.isTable || nd.isControlScript
    visible: !nd.isPill && !nd.isTransform && !nd.isGroupFolder && !nd.isOutputsFolder
  }

  //
  // -- Folder (group / table): a card in the flow; chevron shows expand/collapse state --
  //
  NodeFolder {
    zoom: nd.zoom
    active: nd.active
    node: nd.modelData
    visible: nd.isFolder
    anchors.fill: parent
  }

  MouseArea {
    hoverEnabled: true
    anchors.fill: parent
    enabled: nd.isFolder
    cursorShape: Qt.PointingHandCursor
    onEntered: nd.hovered = true
    onExited:  nd.hovered = false
    onClicked: nd.collapseToggled(nd.modelData.collapseKey)
  }

  MouseArea {
    hoverEnabled: true
    anchors.fill: parent
    enabled: !nd.isFolder
    cursorShape:  Qt.PointingHandCursor
    acceptedButtons: Qt.LeftButton | Qt.RightButton
    onEntered: nd.hovered = true
    onExited:  nd.hovered = false

    onClicked: (mouse) => {
      if (mouse.button === Qt.RightButton)
        nd.contextMenuRequested(nd.modelData)
    }

    onDoubleClicked: (mouse) => {
      if (mouse.button === Qt.LeftButton)
        nd.activated(nd.modelData)
    }
  }

  //
  // -- Collapse chevron for collapsible cards (device / group / output panel) ----
  //
  Image {
    smooth: true
    width:  9 * nd.zoom
    height: 9 * nd.zoom
    sourceSize: Qt.size(9, 9)
    visible: nd.collapsibleCard
    rotation: nd.modelData.collapsed ? 270 : 0
    source: Cpp_Misc_IconRegistry.icon("editor", "indicator", 16)
    anchors {
      right: parent.right
      rightMargin: 8 * nd.zoom
      verticalCenter: parent.verticalCenter
    }
  }

  MouseArea {
    width: 24 * nd.zoom
    visible: nd.collapsibleCard
    enabled: nd.collapsibleCard
    cursorShape: Qt.PointingHandCursor
    anchors { top: parent.top; right: parent.right; bottom: parent.bottom }
    onClicked: nd.collapseToggled(nd.modelData.collapseKey)
  }
}
