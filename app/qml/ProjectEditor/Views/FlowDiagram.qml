/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
import QtQuick.Controls

Item {
  id: root

  //
  // Layout metrics (logical pixels, unscaled)
  //
  readonly property int nodeW: 160   // card width
  readonly property int nodeH: 48    // card height (fixed for all cards)
  readonly property int chipH: 24    // dataset pill height
  readonly property int chipW: 150   // dataset pill width
  readonly property int hGap:  56    // horizontal gap between columns
  readonly property int vGap:  12    // vertical gap between rows
  readonly property int pad:   24    // outer margin

  //
  // Zoom state
  //
  property real zoom:    1.0
  property real minZoom: 0.25
  property real maxZoom: 3.0

  //
  // Layout output
  //
  property var  nodes:    []
  property var  arrows:   []
  property real contentW: 0
  property real contentH: 0

  // ─────────────────────────────────────────────────────────────────────────
  // Public interface
  // ─────────────────────────────────────────────────────────────────────────

  function reloadDiagram() {
    layoutDiagram(
      Cpp_JSON_ProjectModel.sourcesForDiagram(),
      Cpp_JSON_ProjectModel.groupsForDiagram(),
      Cpp_JSON_ProjectModel.actionsForDiagram()
    )
  }

  function resetZoom() {
    zoom = 1.0
    flickable.contentX = 0
    flickable.contentY = 0
  }

  // ─────────────────────────────────────────────────────────────────────────
  // Layout engine
  //
  // Column layout (left → right):
  //   Col 0  Device cards  (one per source; hidden when single-source)
  //   Col 1  Frame Parser  (one per source; always shown)
  //   Col 2  Group cards   (one per group)
  //   Col 3  Dataset pills (stacked per group)
  //   Col 4  Action pills  (one per action; all at same x; attached to
  //                         the first source / frame-parser node)
  //
  // Each group occupies a "slot" of height:
  //   slotH = max(nodeH, dsCount*(chipH+vGap) - vGap)
  //
  // The group card is vertically centred within its slot.
  // The dataset block is vertically centred within the same slot.
  // The frame-parser card for a source is centred over the total slot
  // height of all groups belonging to that source.
  // The device card is centred over the frame-parser card (same y).
  // Action pills hang below all other content, attached to the last
  // frame-parser node's right side.
  // ─────────────────────────────────────────────────────────────────────────

  function layoutDiagram(sources, groups, actions) {
    const newNodes  = []
    const newArrows = []

    const multiSrc = sources.length > 1

    // ── column x positions ────────────────────────────────────────────────
    const colDev  = pad                               // device column (hidden if single)
    const colFP   = multiSrc ? pad + nodeW + hGap : pad  // frame-parser column
    const colGrp  = colFP  + nodeW + hGap            // group column
    const colChip = colGrp + nodeW + hGap            // dataset column
    const colAct  = colChip + chipW + hGap           // action column

    // ── slot height helper ─────────────────────────────────────────────────
    function slotH(dsCount) {
      if (dsCount === 0) return nodeH
      return Math.max(nodeH, dsCount * (chipH + vGap) - vGap)
    }

    // ── per-source vertical state ──────────────────────────────────────────
    // groupY[sid]     = running y-cursor for placing next group in this source
    // srcTotalH[sid]  = sum of all slot heights (incl inter-slot gaps) for source
    const groupY    = {}
    const srcTotalH = {}

    // initialise cursors; handle zero-source case (treat as single source 0)
    const srcList = sources.length > 0 ? sources : [{ sourceId: 0, busType: 0, title: "" }]

    // first pass: measure total height per source
    for (const src of srcList)
      srcTotalH[src.sourceId] = 0

    for (const grp of groups) {
      const sid = grp.sourceId || 0
      if (srcTotalH[sid] === undefined) srcTotalH[sid] = 0
      srcTotalH[sid] += slotH(grp.datasets.length) + vGap
    }

    // remove trailing vGap; ensure minimum nodeH
    for (const sid in srcTotalH)
      srcTotalH[sid] = Math.max((srcTotalH[sid] || 0) - vGap, nodeH)

    // sources stack vertically — compute starting y for each source's block
    const srcBlockY = {}
    let   nextBlockY = pad
    for (const src of srcList) {
      srcBlockY[src.sourceId] = nextBlockY
      groupY[src.sourceId]    = nextBlockY
      nextBlockY += srcTotalH[src.sourceId] + vGap * 3   // extra gap between sources
    }

    // ── place device + frame-parser nodes ─────────────────────────────────
    const fpNodeY = {}   // frame-parser card y per source (for arrow origins)

    for (const src of srcList) {
      const sid  = src.sourceId
      const th   = srcTotalH[sid]
      const midY = srcBlockY[sid] + (th - nodeH) / 2

      fpNodeY[sid] = midY

      // Frame-parser card (always shown)
      newNodes.push({
        type:      "frameparser",
        sourceId:  sid,
        groupId:   -1,
        datasetId: -1,
        actionId:  -1,
        x:         colFP,
        y:         midY,
        w:         nodeW,
        h:         nodeH,
        label:     qsTr("Frame Parser"),
        icon:      "qrc:/rcc/icons/project-editor/treeview/code.svg"
      })

      if (multiSrc) {
        // Device card
        const devTitle = src.title || qsTr("Device %1").arg(sid + 1)
        newNodes.push({
          type:      "source",
          sourceId:  sid,
          groupId:   -1,
          datasetId: -1,
          actionId:  -1,
          x:         colDev,
          y:         midY,
          w:         nodeW,
          h:         nodeH,
          label:     devTitle,
          icon:      busTypeIcon(src.busType),
          badge:     "[" + String.fromCharCode(65 + sid) + "]"
        })

        // Arrow: device → frame-parser
        newArrows.push({
          x1: colDev + nodeW, y1: midY + nodeH / 2,
          x2: colFP,          y2: midY + nodeH / 2,
          dashed: false
        })
      }
    }

    // ── place group + dataset nodes ────────────────────────────────────────
    for (const grp of groups) {
      const sid    = grp.sourceId || 0
      const dsList = grp.datasets
      const dsN    = dsList.length
      const sh     = slotH(dsN)

      const slotTop = groupY[sid] !== undefined ? groupY[sid] : pad
      const cardY   = slotTop + (sh - nodeH) / 2

      // Group card
      newNodes.push({
        type:      "group",
        sourceId:  sid,
        groupId:   grp.groupId,
        datasetId: -1,
        actionId:  -1,
        x:         colGrp,
        y:         cardY,
        w:         nodeW,
        h:         nodeH,
        label:     grp.title || qsTr("Group"),
        icon:      groupIcon(grp.widget),
        badge:     ""
      })

      // Arrow: frame-parser → group
      const fpMidY = fpNodeY[sid] !== undefined
        ? fpNodeY[sid] + nodeH / 2
        : cardY + nodeH / 2
      newArrows.push({
        x1: colFP + nodeW, y1: fpMidY,
        x2: colGrp,        y2: cardY + nodeH / 2,
        dashed: false
      })

      // Dataset pills
      if (dsN > 0) {
        const blockH   = dsN * chipH + (dsN - 1) * vGap
        const blockTop = slotTop + (sh - blockH) / 2

        for (let di = 0; di < dsN; ++di) {
          const ds    = dsList[di]
          const label = ds.units
            ? (ds.title + " [" + ds.units + "]")
            : ds.title
          const chipY = blockTop + di * (chipH + vGap)

          newNodes.push({
            type:      "dataset",
            sourceId:  sid,
            groupId:   grp.groupId,
            datasetId: ds.datasetId,
            actionId:  -1,
            x:         colChip,
            y:         chipY,
            w:         chipW,
            h:         chipH,
            label:     label,
            icon:      "",
            badge:     ""
          })

          // Arrow: group → dataset pill
          newArrows.push({
            x1: colGrp + nodeW, y1: cardY + nodeH / 2,
            x2: colChip,        y2: chipY + chipH / 2,
            dashed: false
          })
        }
      }

      // Advance cursor for this source
      if (groupY[sid] !== undefined)
        groupY[sid] = slotTop + sh + vGap
    }

    // ── place action pills ─────────────────────────────────────────────────
    // Hang actions below the main content, connected from the first
    // frame-parser node with a dashed arrow (TX direction)
    if (actions.length > 0) {
      // Find the topmost frame-parser node to anchor actions from
      let anchorY = pad + nodeH / 2
      if (srcList.length > 0)
        anchorY = (fpNodeY[srcList[0].sourceId] || pad) + nodeH / 2

      // Find the y start for action column: below all content so far
      let maxGroupY = pad
      for (const sid in groupY)
        maxGroupY = Math.max(maxGroupY, groupY[sid])

      const actBlockTop = maxGroupY + vGap * 2

      for (let ai = 0; ai < actions.length; ++ai) {
        const act  = actions[ai]
        const actY = actBlockTop + ai * (nodeH + vGap)

        newNodes.push({
          type:      "action",
          sourceId:  0,
          groupId:   -1,
          datasetId: -1,
          actionId:  act.actionId,
          x:         colAct,
          y:         actY,
          w:         chipW,
          h:         nodeH,
          label:     act.title || qsTr("Action"),
          icon:      act.icon  || "qrc:/rcc/icons/project-editor/treeview/action.svg",
          badge:     ""
        })

        // Dashed arrow from dataset column area to action (TX)
        newArrows.push({
          x1: colChip,       y1: anchorY,
          x2: colAct,        y2: actY + nodeH / 2,
          dashed: true
        })
      }
    }

    // ── content bounds ─────────────────────────────────────────────────────
    let maxX = 0, maxY = 0
    for (const n of newNodes) {
      maxX = Math.max(maxX, n.x + n.w)
      maxY = Math.max(maxY, n.y + n.h)
    }

    root.contentW = maxX + pad
    root.contentH = maxY + pad
    root.nodes    = newNodes
    root.arrows   = newArrows

    canvas.requestPaint()
  }

  // ─────────────────────────────────────────────────────────────────────────
  // Icon helpers
  // ─────────────────────────────────────────────────────────────────────────

  function busTypeIcon(busType) {
    const base = "qrc:/rcc/icons/devices/drivers/"
    switch (busType) {
      case 0:  return base + "uart.svg"
      case 1:  return base + "network.svg"
      case 2:  return base + "bluetooth.svg"
      case 3:  return base + "audio.svg"
      case 4:  return base + "modbus.svg"
      case 5:  return base + "canbus.svg"
      case 6:  return base + "usb.svg"
      case 7:  return base + "hid.svg"
      case 8:  return base + "process.svg"
      default: return base + "uart.svg"
    }
  }

  function groupIcon(widget) {
    const base = "qrc:/rcc/icons/project-editor/treeview/"
    switch ((widget || "").toLowerCase()) {
      case "multiplot":      return base + "multiplot.svg"
      case "accelerometer":  return base + "accelerometer.svg"
      case "gyroscope":      return base + "gyroscope.svg"
      case "gps":            return base + "gps.svg"
      case "image":          return base + "image.svg"
      case "plot3d":         return base + "plot3d.svg"
      case "datagrid":       return base + "datagrid.svg"
      default:               return base + "group.svg"
    }
  }

  // ─────────────────────────────────────────────────────────────────────────
  // Reactive connections
  // ─────────────────────────────────────────────────────────────────────────

  Connections {
    target: Cpp_JSON_ProjectModel
    function onGroupsChanged()     { root.reloadDiagram() }
    function onGroupDataChanged()  { root.reloadDiagram() }
    function onActionsChanged()    { root.reloadDiagram() }
    function onSourcesChanged()    { root.reloadDiagram() }
    function onTitleChanged()      { root.reloadDiagram() }
  }

  Component.onCompleted: reloadDiagram()
  onVisibleChanged: if (visible) { reloadDiagram(); canvas.requestPaint() }
  onWidthChanged:   if (visible && width > 0) reloadDiagram()
  onHeightChanged:  if (visible && height > 0) reloadDiagram()

  // ─────────────────────────────────────────────────────────────────────────
  // Flickable + scaled canvas
  // ─────────────────────────────────────────────────────────────────────────

  Flickable {
    id: flickable

    anchors.fill: parent
    clip: true
    contentWidth:  Math.max(root.contentW * root.zoom, width)
    contentHeight: Math.max(root.contentH * root.zoom, height)
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical:   ScrollBar { policy: ScrollBar.AsNeeded }
    ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }

    // Ctrl+Wheel → zoom toward cursor
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

    // Plain wheel → vertical scroll; Shift+Wheel → horizontal
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

    // Middle-button drag → pan
    MouseArea {
      anchors.fill: parent
      acceptedButtons: Qt.MiddleButton
      cursorShape: pressed ? Qt.ClosedHandCursor : Qt.ArrowCursor
      property real lx: 0
      property real ly: 0
      onPressed:         (m) => { lx = m.x; ly = m.y }
      onPositionChanged: (m) => {
        flickable.contentX -= m.x - lx
        flickable.contentY -= m.y - ly
        lx = m.x; ly = m.y
      }
    }

    // ── Scaled content item ──────────────────────────────────────────────
    Item {
      width:  root.contentW * root.zoom
      height: root.contentH * root.zoom

      // Arrow canvas
      Canvas {
        id: canvas

        anchors.fill: parent

        onPaint: {
          const ctx = getContext("2d")
          ctx.clearRect(0, 0, width, height)

          const z = root.zoom

          for (const a of root.arrows) {
            const x1 = a.x1 * z, y1 = a.y1 * z
            const x2 = a.x2 * z, y2 = a.y2 * z
            const mx = (x1 + x2) / 2

            ctx.strokeStyle = Cpp_ThemeManager.colors["mid"]
            ctx.fillStyle   = Cpp_ThemeManager.colors["mid"]
            ctx.lineWidth   = 1.5 * z

            if (a.dashed) {
              ctx.setLineDash([4 * z, 4 * z])
              ctx.strokeStyle = Cpp_ThemeManager.colors["highlight"]
              ctx.fillStyle   = Cpp_ThemeManager.colors["highlight"]
            } else {
              ctx.setLineDash([])
            }

            ctx.beginPath()
            ctx.moveTo(x1, y1)
            ctx.bezierCurveTo(mx, y1, mx, y2, x2, y2)
            ctx.stroke()

            // Arrowhead
            const hl    = 7 * z
            const angle = Math.atan2(y2 - y1, x2 - x1)
            ctx.setLineDash([])
            ctx.beginPath()
            ctx.moveTo(x2, y2)
            ctx.lineTo(x2 - hl * Math.cos(angle - Math.PI / 6),
                       y2 - hl * Math.sin(angle - Math.PI / 6))
            ctx.lineTo(x2 - hl * Math.cos(angle + Math.PI / 6),
                       y2 - hl * Math.sin(angle + Math.PI / 6))
            ctx.closePath()
            ctx.fill()
          }
        }

        Connections {
          target: Cpp_ThemeManager
          function onThemeChanged() { canvas.requestPaint() }
        }

        onAvailableChanged: if (available) root.reloadDiagram()
        onWidthChanged:  requestPaint()
        onHeightChanged: requestPaint()
      }

      // Node repeater
      Repeater {
        model: root.nodes

        delegate: Item {
          id: nd

          x:      modelData.x * root.zoom
          y:      modelData.y * root.zoom
          width:  modelData.w * root.zoom
          height: modelData.h * root.zoom

          property bool hovered:   false
          property bool isDataset: modelData.type === "dataset"
          property bool isAction:  modelData.type === "action"
          property bool isPill:    isDataset || isAction
          property bool isSource:  modelData.type === "source"
          property bool isFP:      modelData.type === "frameparser"

          // ── Pill (dataset / action) ──────────────────────────────────
          Rectangle {
            visible: nd.isPill
            anchors.fill: parent
            radius: nd.isDataset ? height / 2 : 6 * root.zoom
            color: nd.hovered
              ? Cpp_ThemeManager.colors["highlight"]
              : Cpp_ThemeManager.colors["groupbox_background"]
            border.width: 1
            border.color: nd.isAction
              ? (nd.hovered
                  ? Cpp_ThemeManager.colors["highlight"]
                  : Cpp_ThemeManager.colors["highlight"] + "80")
              : (nd.hovered
                  ? Cpp_ThemeManager.colors["highlight"]
                  : Cpp_ThemeManager.colors["groupbox_border"])

            Row {
              visible: nd.isAction
              anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
                leftMargin:  8 * root.zoom
                rightMargin: 8 * root.zoom
              }
              spacing: 6 * root.zoom

              Image {
                width:  16 * root.zoom
                height: 16 * root.zoom
                anchors.verticalCenter: parent.verticalCenter
                source: modelData.icon
                sourceSize: Qt.size(16, 16)
                smooth: true
                opacity: 0.8
              }

              Text {
                width: parent.width - 28 * root.zoom
                elide: Text.ElideRight
                anchors.verticalCenter: parent.verticalCenter
                text: modelData.label
                font.pixelSize: Math.max(8, 11 * root.zoom)
                color: nd.hovered
                  ? Cpp_ThemeManager.colors["highlighted_text"]
                  : Cpp_ThemeManager.colors["text"]
              }
            }

            Text {
              visible: nd.isDataset
              anchors.centerIn: parent
              width: parent.width - 12
              elide: Text.ElideRight
              horizontalAlignment: Text.AlignHCenter
              text: modelData.label
              font.pixelSize: Math.max(8, 11 * root.zoom)
              color: nd.hovered
                ? Cpp_ThemeManager.colors["highlighted_text"]
                : Cpp_ThemeManager.colors["text"]
            }
          }

          // ── Card (source / group / frame-parser) ─────────────────────
          Rectangle {
            visible: !nd.isPill
            anchors.fill: parent
            radius: 6 * root.zoom
            color: nd.hovered
              ? Cpp_ThemeManager.colors["highlight"]
              : Cpp_ThemeManager.colors["groupbox_background"]
            border.width: nd.isFP ? 1 : 1
            border.color: nd.isFP
              ? (nd.hovered
                  ? Cpp_ThemeManager.colors["highlight"]
                  : Cpp_ThemeManager.colors["groupbox_border"])
              : (nd.hovered
                  ? Cpp_ThemeManager.colors["highlight"]
                  : Cpp_ThemeManager.colors["groupbox_border"])

            Row {
              anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
                leftMargin:  10 * root.zoom
                rightMargin:  8 * root.zoom
              }
              spacing: 8 * root.zoom

              Image {
                width:  20 * root.zoom
                height: 20 * root.zoom
                anchors.verticalCenter: parent.verticalCenter
                source: modelData.icon
                sourceSize: Qt.size(20, 20)
                smooth: true
                opacity: nd.isFP ? 0.7 : (nd.hovered ? 1.0 : 0.85)
              }

              Text {
                width: parent.width - 36 * root.zoom
                elide: Text.ElideRight
                anchors.verticalCenter: parent.verticalCenter
                text: modelData.label
                font.pixelSize: Math.max(8, 12 * root.zoom)
                font.bold: nd.isSource
                font.italic: nd.isFP
                color: nd.hovered
                  ? Cpp_ThemeManager.colors["highlighted_text"]
                  : (nd.isFP
                      ? Qt.lighter(Cpp_ThemeManager.colors["text"], 1.3)
                      : Cpp_ThemeManager.colors["text"])
              }
            }

            // Badge [A] [B] on source cards
            Text {
              visible: nd.isSource && (modelData.badge || "") !== ""
              anchors { right: parent.right; bottom: parent.bottom; margins: 4 * root.zoom }
              text: modelData.badge || ""
              font.family: Cpp_Misc_CommonFonts.monoFont.family
              font.pixelSize: Math.max(7, 9 * root.zoom)
              opacity: 0.6
              color: nd.hovered
                ? Cpp_ThemeManager.colors["highlighted_text"]
                : Cpp_ThemeManager.colors["text"]
            }
          }

          MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape:  Qt.PointingHandCursor
            onEntered: nd.hovered = true
            onExited:  nd.hovered = false
            onClicked: {
              switch (modelData.type) {
                case "source":
                  Cpp_JSON_ProjectEditor.selectSource(modelData.sourceId)
                  break
                case "frameparser":
                  Cpp_JSON_ProjectEditor.selectFrameParser()
                  break
                case "group":
                  Cpp_JSON_ProjectEditor.selectGroup(modelData.groupId)
                  break
                case "dataset":
                  Cpp_JSON_ProjectEditor.selectDataset(modelData.groupId, modelData.datasetId)
                  break
                case "action":
                  Cpp_JSON_ProjectEditor.selectAction(modelData.actionId)
                  break
              }
            }
          }
        }
      }
    }
  }

  // ─────────────────────────────────────────────────────────────────────────
  // Empty-state placeholder
  // ─────────────────────────────────────────────────────────────────────────

  Column {
    anchors.centerIn: parent
    spacing: 8
    visible: root.nodes.length === 0

    Image {
      anchors.horizontalCenter: parent.horizontalCenter
      source: "qrc:/rcc/icons/project-editor/treeview/group.svg"
      sourceSize: Qt.size(48, 48)
      opacity: 0.25
    }

    Text {
      anchors.horizontalCenter: parent.horizontalCenter
      text: qsTr("No groups defined yet")
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["mid"]
    }
  }
}
