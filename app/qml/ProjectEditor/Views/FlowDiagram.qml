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
import QtQuick.Controls

import SerialStudio

Item {
  id: root

  //
  // Layout metrics (logical pixels, unscaled)
  //
  readonly property int nodeW:    160  // card width
  readonly property int pad:      24   // outer margin
  readonly property int chipW:    150  // dataset pill width
  readonly property int chipH:    24   // dataset pill height
  readonly property int transW:   32   // transform block width
  readonly property int vGap:     12   // vertical gap between rows
  readonly property int hGap:     56   // horizontal gap between columns
  readonly property int nodeH:    48   // card height (fixed for all cards)
  readonly property int transGap: 16   // gap between transform block and dataset pill

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

  function isCollapsed(key) {
    if (key in collapsedKeys)
      return collapsedKeys[key] === true

    // Auto-collapse: folders start folded so a foldered project opens compact; groups start open.
    return key.indexOf("grpfolder:") === 0 || key.indexOf("tblfolder:") === 0
  }

  function seedCollapseFromModel() {
    collapsedKeys = Object.assign({}, Cpp_JSON_ProjectModel.diagramCollapse)
  }

  function toggleCollapse(key) {
    const next = Object.assign({}, collapsedKeys)
    next[key] = !isCollapsed(key)

    collapsedKeys = next
    Cpp_JSON_ProjectModel.setDiagramCollapse(next)
    reloadDiagram()
  }

  //
  // Public interface.
  //
  function reloadDiagram() {
    layoutDiagram(
      Cpp_JSON_ProjectModel.sourcesForDiagram(),
      Cpp_JSON_ProjectModel.groupsForDiagram(),
      Cpp_JSON_ProjectModel.actionsForDiagram(),
      Cpp_JSON_ProjectModel.tablesForDiagram()
    )
  }

  function resetZoom() {
    zoom = 1.0
    flickable.contentX = 0
    flickable.contentY = 0
  }

  //
  // Columns L->R: Device | Frame Parser/Actions | Groups | Dataset pills.
  //
  function layoutDiagram(sources, groups, actions, tables) {
    const newNodes  = []
    const newArrows = []
    tables = tables || []

    // -- fixed left columns (device + frame parser); folder/group columns computed below -----
    const colW   = nodeW + hGap
    const colDev = pad
    const colFP  = pad + colW

    // Collected dataset pill centres, fed into the MQTT Publisher node below
    const datasetAnchors = []

    // -- slot height helper -------------------------------------------------
    function slotH(dsCount) {
      if (dsCount === 0) return nodeH
      return Math.max(nodeH, dsCount * (chipH + vGap) - vGap)
    }

    // zero-source case: treat as a single source 0
    const fallback = [{ sourceId: 0, busType: SerialStudio.UART, title: "" }]
    const srcList = sources.length > 0 ? sources : fallback

    // -- group folder metadata: DFS parent + title maps ---------------------
    const gfTree   = Cpp_JSON_ProjectEditor.groupFolderTree()
    const gfParent = {}
    const gfTitle  = {}
    function walkGF(nodes, parentId) {
      for (const n of nodes) {
        gfParent[n.id] = parentId
        gfTitle[n.id]  = n.title
        walkGF(n.children || [], n.id)
      }
    }
    walkGF(gfTree, -1)
    const gfCount = Object.keys(gfParent).length

    function folderOf(grp) {
      return (grp.parentFolderId === undefined) ? -1 : grp.parentFolderId
    }
    function folderDepth(fid) {
      let cur = fid, d = 0, guard = 0
      while (cur !== -1 && cur !== undefined && guard <= gfCount) {
        cur = gfParent[cur]; ++d; ++guard
      }
      return Math.max(0, d - 1)
    }
    function folderIsSelfOrAncestor(anc, node) {
      let cur = node, guard = 0
      while (cur !== -1 && cur !== undefined && guard <= gfCount) {
        if (cur === anc) return true
        cur = gfParent[cur]; ++guard
      }
      return false
    }
    function subtreeHasSourceGroups(fid, sid) {
      for (const g of groups) {
        if (g.groupType === SerialStudio.GroupOutput) continue
        if ((g.sourceId || 0) !== sid) continue
        if (folderOf(g) !== -1 && folderIsSelfOrAncestor(fid, folderOf(g)))
          return true
      }
      return false
    }

    // Deepest folder that actually owns groups -> how many folder columns to reserve.
    let maxFolderDepth = -1
    for (const g of groups) {
      if (g.groupType === SerialStudio.GroupOutput) continue

      const fid = folderOf(g)
      if (fid !== -1)
        maxFolderDepth = Math.max(maxFolderDepth, folderDepth(fid))
    }

    // -- folder / group / dataset columns (one column per folder level) -----
    const colFolder = colFP + colW                             // first folder column
    const colGrp    = colFolder + (maxFolderDepth + 1) * colW  // group column
    const colTrans  = colGrp + colW                            // transform block column
    const colChip   = colTrans + transW + transGap             // dataset column
    const colMqtt   = colChip + chipW + hGap                   // mqtt publisher column

    function folderX(depth) {
      return colFolder + depth * colW
    }

    // Tidy-tree cursor (advances down as leaves are placed) + frame-parser Y per source.
    const fpNodeY = {}
    let cursorY   = pad

    // Places a group centred on its pills (one slot when it has none or is collapsed); returns Y.
    function placeGroup(grp, sid) {
      const dsList    = grp.datasets || []
      const collapsed = root.isCollapsed("grp:" + grp.groupId)
      const pills     = collapsed ? [] : dsList

      let centerY
      const chipYs = []
      if (pills.length === 0) {
        centerY = cursorY + nodeH / 2
        cursorY += nodeH + vGap
      } else {
        for (let di = 0; di < pills.length; ++di) {
          chipYs.push(cursorY)
          cursorY += chipH + vGap
        }
        centerY = (chipYs[0] + chipYs[chipYs.length - 1] + chipH) / 2
      }

      const cardY = centerY - nodeH / 2
      newNodes.push({
        type:         "group",
        sourceId:     sid,
        groupId:      grp.groupId,
        datasetId:    -1,
        actionId:     -1,
        widget:       grp.widget || "",
        collapsed:    collapsed,
        collapseKey:  dsList.length > 0 ? ("grp:" + grp.groupId) : undefined,
        siblingCount: groups.length,
        x:            colGrp,
        y:            cardY,
        w:            nodeW,
        h:            nodeH,
        label:        grp.title || qsTr("Group"),
        icon:         groupIcon(grp),
        badge:        ""
      })

      for (let di = 0; di < pills.length; ++di) {
        const ds    = pills[di]
        const chipY = chipYs[di]
        const hasTransform = ds.hasTransform === true

        if (hasTransform) {
          newNodes.push({
            type:      "transform",
            sourceId:  sid,
            groupId:   grp.groupId,
            datasetId: ds.datasetId,
            widgetId:  -1,
            actionId:  -1,
            x:         colTrans,
            y:         chipY,
            w:         transW,
            h:         chipH,
            label:     "",
            icon:      "",
            badge:     ""
          })

          newArrows.push({
            x1: colGrp + nodeW, y1: cardY + nodeH / 2,
            x2: colTrans,       y2: chipY + chipH / 2
          })
          newArrows.push({
            x1: colTrans + transW, y1: chipY + chipH / 2,
            x2: colChip,           y2: chipY + chipH / 2
          })
        } else {
          newArrows.push({
            x1: colGrp + nodeW, y1: cardY + nodeH / 2,
            x2: colTrans,       y2: chipY + chipH / 2,
            noHead: true
          })
          newArrows.push({
            x1: colTrans, y1: chipY + chipH / 2,
            x2: colChip,  y2: chipY + chipH / 2
          })
        }

        newNodes.push({
          type:         "dataset",
          sourceId:     sid,
          groupId:      grp.groupId,
          datasetId:    ds.datasetId,
          widgetId:     -1,
          actionId:     -1,
          siblingCount: pills.length,
          x:            colChip,
          y:            chipY,
          w:            chipW,
          h:            chipH,
          label:        ds.units ? (ds.title + " [" + ds.units + "]") : ds.title,
          icon:         datasetIcon(),
          badge:        "",
          hasTransform: hasTransform
        })

        datasetAnchors.push(chipY + chipH / 2)
      }

      return centerY
    }

    function placeFolder(f, sid) {
      const depth     = folderDepth(f.id)
      const collapsed = root.isCollapsed("grpfolder:" + f.id)
      const x         = folderX(depth)

      let centerY
      if (collapsed) {
        centerY = cursorY + nodeH / 2
        cursorY += nodeH + vGap
      } else {
        const kids = []
        for (const sub of (f.children || []))
          if (subtreeHasSourceGroups(sub.id, sid))
            kids.push({ y: placeFolder(sub, sid), x: folderX(folderDepth(sub.id)) })

        for (const g of groups)
          if (g.groupType !== SerialStudio.GroupOutput
              && (g.sourceId || 0) === sid && folderOf(g) === f.id)
            kids.push({ y: placeGroup(g, sid), x: colGrp })

        if (kids.length === 0) {
          centerY = cursorY + nodeH / 2
          cursorY += nodeH + vGap
        } else {
          centerY = (kids[0].y + kids[kids.length - 1].y) / 2
          for (const k of kids)
            newArrows.push({ x1: x + nodeW, y1: centerY, x2: k.x, y2: k.y })
        }
      }

      newNodes.push({
        type:       "groupfolder",
        collapsed:  collapsed,
        collapseKey: "grpfolder:" + f.id,
        folderId:   f.id,
        sourceId:   sid,
        groupId:    -1,
        datasetId:  -1,
        actionId:   -1,
        depth:      depth,
        x:          x,
        y:          centerY - nodeH / 2,
        w:          nodeW,
        h:          nodeH,
        label:      f.title || qsTr("Folder"),
        icon:       "qrc:/icons/project-editor/treeview/folder.svg"
      })

      return centerY
    }

    function sourceHasContent(sid) {
      for (const g of groups)
        if ((g.sourceId || 0) === sid) return true

      for (const a of actions)
        if ((a.sourceId || 0) === sid) return true

      return false
    }

    //
    // Control Loop card (project-global): the first root, at device level.
    //
    const controlLoopY = cursorY
    cursorY += nodeH + vGap * 3
    newNodes.push({
      type:      "controlscript",
      sourceId:  -1,
      groupId:   -1,
      datasetId: -1,
      actionId:  -1,
      x:         colDev,
      y:         controlLoopY,
      w:         nodeW,
      h:         nodeH,
      label:     qsTr("Control Loop"),
      icon:      "qrc:/icons/project-editor/treeview/control-script.svg",
      badge:     Cpp_JSON_ProjectModel.controlScriptCode.length > 0 ? "" : qsTr("empty")
    })

    for (const src of srcList) {
      const sid          = src.sourceId
      const srcCollapsed = isCollapsed("src:" + sid)
      const tops         = []

      if (!srcCollapsed) {
        for (const f of gfTree)
          if (subtreeHasSourceGroups(f.id, sid))
            tops.push({ y: placeFolder(f, sid), x: folderX(0) })

        for (const g of groups)
          if (g.groupType !== SerialStudio.GroupOutput
              && (g.sourceId || 0) === sid && folderOf(g) === -1)
            tops.push({ y: placeGroup(g, sid), x: colGrp })
      }

      let fpCenter
      if (tops.length === 0) {
        fpCenter = cursorY + nodeH / 2
        cursorY += nodeH + vGap
      } else {
        fpCenter = (tops[0].y + tops[tops.length - 1].y) / 2
      }
      fpNodeY[sid] = fpCenter - nodeH / 2

      //
      // Frame-parser card (hidden when the device is collapsed)
      //
      if (!srcCollapsed)
        newNodes.push({
          type:      "frameparser",
          sourceId:  sid,
          groupId:   -1,
          datasetId: -1,
          actionId:  -1,
          x:         colFP,
          y:         fpCenter - nodeH / 2,
          w:         nodeW,
          h:         nodeH,
          label:     qsTr("Frame Parser"),
          icon:      "qrc:/icons/project-editor/treeview/code.svg"
        })

      //
      // Device card (always shown; carries the collapse chevron when it has downstream content)
      //
      const devTitle = src.title || qsTr("Device %1").arg(sid + 1)
      newNodes.push({
        type:        "source",
        sourceId:    sid,
        groupId:     -1,
        datasetId:   -1,
        actionId:    -1,
        collapsed:   srcCollapsed,
        collapseKey: sourceHasContent(sid) ? ("src:" + sid) : undefined,
        x:           colDev,
        y:           fpCenter - nodeH / 2,
        w:           nodeW,
        h:           nodeH,
        label:       devTitle,
        icon:        busTypeIcon(src.busType),
        badge:       sources.length > 1
          ? "[" + String.fromCharCode(65 + sid) + "]"
          : ""
      })

      //
      // Arrows: device -> parser, then parser -> each top-level folder / group.
      //
      if (!srcCollapsed) {
        newArrows.push({
          x1: colDev + nodeW, y1: fpCenter,
          x2: colFP,          y2: fpCenter
        })

        for (const t of tops)
          newArrows.push({
            x1: colFP + nodeW, y1: fpCenter,
            x2: t.x,           y2: t.y
          })
      }

      cursorY += vGap * 3
    }

    let blockCursor = cursorY

    if (actions.length > 0) {
      let actY          = blockCursor + vGap * 2
      let placedActions = 0

      for (let ai = 0; ai < actions.length; ++ai) {
        const act = actions[ai]
        const sid = act.sourceId || 0
        if (isCollapsed("src:" + sid))
          continue

        newNodes.push({
          type:         "action",
          sourceId:     sid,
          groupId:      -1,
          datasetId:    -1,
          actionId:     act.actionId,
          siblingCount: actions.length,
          x:            colFP,
          y:            actY,
          w:            nodeW,
          h:            nodeH,
          label:        act.title || qsTr("Action"),
          icon:         act.icon  || "qrc:/icons/project-editor/treeview/action.svg",
          badge:        ""
        })

        //
        // Arrow from action up into the bottom-center of its target device.
        //
        const devTopY = fpNodeY[sid] !== undefined ? fpNodeY[sid] : pad
        newArrows.push({
          x1: colFP,             y1: actY + nodeH / 2,
          x2: colDev + nodeW / 2, y2: devTopY + nodeH,
          verticalEnd: true
        })

        actY += nodeH + vGap
        ++placedActions
      }

      if (placedActions > 0)
        blockCursor = actY
    }

    //
    // Output panels (TX direction): mirror image of the RX flow
    //
    const allOutputGroups = groups.filter(g => g.groupType === SerialStudio.GroupOutput)
    const outputGroups = allOutputGroups.filter(g => !isCollapsed("src:" + (g.sourceId || 0)))

    if (outputGroups.length > 0) {
      let outCursor = blockCursor + vGap * 2

      for (const grp of outputGroups) {
        const sid            = grp.sourceId || 0
        const panelCollapsed = isCollapsed("grp:" + grp.groupId)
        const owList         = grp.outputWidgets || []
        const wCount         = panelCollapsed ? 0 : owList.length
        const wsh            = slotH(wCount)
        const panelY         = outCursor + (wsh - nodeH) / 2

        //
        // Panel card (the parent group) in colFP
        //
        newNodes.push({
          type:         "output-panel",
          sourceId:     sid,
          groupId:      grp.groupId,
          datasetId:    -1,
          widgetId:     -1,
          actionId:     -1,
          widget:       grp.widget || "",
          collapsed:    panelCollapsed,
          collapseKey:  owList.length > 0 ? ("grp:" + grp.groupId) : undefined,
          siblingCount: allOutputGroups.length,
          x:            colFP,
          y:            panelY,
          w:            nodeW,
          h:            nodeH,
          label:        grp.title || qsTr("Output Panel"),
          icon:         "qrc:/icons/project-editor/treeview/output-panel.svg",
          badge:        ""
        })

        //
        // Arrow from panel up into the bottom-center of its target device.
        //
        const devTopY = fpNodeY[sid] !== undefined ? fpNodeY[sid] : pad
        newArrows.push({
          x1: colFP,             y1: panelY + nodeH / 2,
          x2: colDev + nodeW / 2, y2: devTopY + nodeH,
          verticalEnd: true
        })

        //
        // Control pills stacked at colChip, one per widget, mirroring the
        // group -> dataset layout. Each gets a single panel -> widget arrow.
        //
        if (wCount > 0) {
          const blockH   = wCount * chipH + (wCount - 1) * vGap
          const blockTop = outCursor + (wsh - blockH) / 2

          for (let oi = 0; oi < wCount; ++oi) {
            const ow    = owList[oi]
            const chipY = blockTop + oi * (chipH + vGap)

            newArrows.push({
              x1: colFP + nodeW, y1: panelY + nodeH / 2,
              x2: colChip,       y2: chipY + chipH / 2
            })

            newNodes.push({
              type:         "output",
              sourceId:     sid,
              groupId:      grp.groupId,
              datasetId:    -1,
              widgetId:     oi,
              actionId:     -1,
              siblingCount: wCount,
              x:            colChip,
              y:            chipY,
              w:            chipW,
              h:            chipH,
              label:        ow.title || qsTr("Control"),
              icon:         outputWidgetIcon(ow.outputType),
              badge:        ""
            })
          }
        }

        outCursor += wsh + vGap
      }

      blockCursor = outCursor
    }

    //
    // Data tables (shared scratch space): table folders flow into their table cards, mirroring the
    // group folder columns. No arrows from the frame pipeline.
    //
    if (tables.length > 0) {
      const tfTree   = Cpp_JSON_ProjectEditor.tableFolderTree()
      const tfParent = {}
      const tfTitle  = {}
      function walkTF(nodes, parentId) {
        for (const n of nodes) {
          tfParent[n.id] = parentId
          tfTitle[n.id]  = n.title
          walkTF(n.children || [], n.id)
        }
      }
      walkTF(tfTree, -1)
      const tfCount = Object.keys(tfParent).length

      function tableFolderOf(tbl) {
        return (tbl.parentFolderId === undefined) ? -1 : tbl.parentFolderId
      }
      function tableFolderDepth(fid) {
        let cur = fid, d = 0, guard = 0
        while (cur !== -1 && cur !== undefined && guard <= tfCount) {
          cur = tfParent[cur]; ++d; ++guard
        }
        return Math.max(0, d - 1)
      }
      function tfIsSelfOrAncestor(anc, node) {
        let cur = node, guard = 0
        while (cur !== -1 && cur !== undefined && guard <= tfCount) {
          if (cur === anc) return true
          cur = tfParent[cur]; ++guard
        }
        return false
      }
      function subtreeHasTables(fid) {
        for (const t of tables)
          if (tableFolderOf(t) !== -1 && tfIsSelfOrAncestor(fid, tableFolderOf(t)))
            return true

        return false
      }

      let maxTblDepth = -1
      for (const t of tables) {
        const fid = tableFolderOf(t)
        if (fid !== -1)
          maxTblDepth = Math.max(maxTblDepth, tableFolderDepth(fid))
      }

      const colTblFolder = colFP
      const colTbl       = colTblFolder + (maxTblDepth + 1) * colW
      function tblFolderX(depth) {
        return colTblFolder + depth * colW
      }

      function placeTable(tbl) {
        const centerY = cursorY + nodeH / 2
        cursorY += nodeH + vGap

        const regs  = tbl.registerCount || 0
        const label = tbl.name && tbl.name.length > 0 ? tbl.name : qsTr("Table")
        newNodes.push({
          type:      "table",
          sourceId:  -1,
          groupId:   -1,
          datasetId: -1,
          actionId:  -1,
          tableName: tbl.name || "",
          x:         colTbl,
          y:         centerY - nodeH / 2,
          w:         nodeW,
          h:         nodeH,
          label:     label,
          icon:      "qrc:/icons/project-editor/treeview/shared-table.svg",
          badge:     regs > 0 ? qsTr("%1 regs").arg(regs) : qsTr("empty")
        })
        return centerY
      }

      function placeTableFolder(f) {
        const depth     = tableFolderDepth(f.id)
        const collapsed = root.isCollapsed("tblfolder:" + f.id)
        const x         = tblFolderX(depth)

        let centerY
        if (collapsed) {
          centerY = cursorY + nodeH / 2
          cursorY += nodeH + vGap
        } else {
          const kids = []
          for (const sub of (f.children || []))
            if (subtreeHasTables(sub.id))
              kids.push({ y: placeTableFolder(sub), x: tblFolderX(tableFolderDepth(sub.id)) })

          for (const t of tables)
            if (tableFolderOf(t) === f.id)
              kids.push({ y: placeTable(t), x: colTbl })

          if (kids.length === 0) {
            centerY = cursorY + nodeH / 2
            cursorY += nodeH + vGap
          } else {
            centerY = (kids[0].y + kids[kids.length - 1].y) / 2
            for (const k of kids)
              newArrows.push({ x1: x + nodeW, y1: centerY, x2: k.x, y2: k.y })
          }
        }

        newNodes.push({
          type:        "tablefolder",
          collapsed:   collapsed,
          collapseKey: "tblfolder:" + f.id,
          folderId:    f.id,
          sourceId:    -1,
          groupId:     -1,
          datasetId:   -1,
          actionId:    -1,
          depth:       depth,
          x:           x,
          y:           centerY - nodeH / 2,
          w:           nodeW,
          h:           nodeH,
          label:       f.title || qsTr("Folder"),
          icon:        "qrc:/icons/project-editor/treeview/folder.svg"
        })
        return centerY
      }

      cursorY = blockCursor + vGap * 2
      const smCollapsed = root.isCollapsed("sharedmem")
      const tblTops     = []

      if (!smCollapsed) {
        for (const f of tfTree)
          if (subtreeHasTables(f.id))
            tblTops.push({ y: placeTableFolder(f), x: colTblFolder })

        for (const t of tables)
          if (tableFolderOf(t) === -1)
            tblTops.push({ y: placeTable(t), x: colTbl })
      }

      let smCenter
      if (tblTops.length === 0) {
        smCenter = cursorY + nodeH / 2
        cursorY += nodeH + vGap
      } else {
        smCenter = (tblTops[0].y + tblTops[tblTops.length - 1].y) / 2
      }

      //
      // Shared Memory root at device level, centred on its tables.
      //
      newNodes.push({
        type:        "shared-memory",
        sourceId:    -1,
        groupId:     -1,
        datasetId:   -1,
        actionId:    -1,
        collapsed:   smCollapsed,
        collapseKey: "sharedmem",
        x:           colDev,
        y:           smCenter - nodeH / 2,
        w:           nodeW,
        h:           nodeH,
        label:       qsTr("Shared Memory"),
        icon:        "qrc:/icons/project-editor/treeview/shared-memory.svg",
        badge:       ""
      })

      if (!smCollapsed)
        for (const t of tblTops)
          newArrows.push({
            x1: colDev + nodeW, y1: smCenter,
            x2: t.x,            y2: t.y
          })

      blockCursor = cursorY
    }

    //
    // MQTT Publisher node: collects every dataset pill (commercial, opt-in)
    //
    if (mqttPublisherEnabled() && datasetAnchors.length > 0) {
      const minY     = datasetAnchors[0]
      const maxYAnch = datasetAnchors[datasetAnchors.length - 1]
      const midY     = (minY + maxYAnch) / 2
      const mqttY    = midY - nodeH / 2

      newNodes.push({
        type:      "mqtt-publisher",
        sourceId:  -1,
        groupId:   -1,
        datasetId: -1,
        widgetId:  -1,
        actionId:  -1,
        x:         colMqtt,
        y:         mqttY,
        w:         nodeW,
        h:         nodeH,
        label:     qsTr("MQTT Publisher"),
        icon:      "qrc:/icons/project-editor/treeview/mqtt-publisher.svg",
        badge:     ""
      })

      for (const dy of datasetAnchors) {
        newArrows.push({
          x1: colChip + chipW, y1: dy,
          x2: colMqtt,         y2: mqttY + nodeH / 2
        })
      }
    }

    //
    // -- content bounds -----------------------------------------------------
    //
    let maxX = 0, maxY = 0
    for (const n of newNodes) {
      maxX = Math.max(maxX, n.x + n.w)
      maxY = Math.max(maxY, n.y + n.h)
    }

    //
    // Mirror X coordinates for right-to-left languages.
    //
    if (Cpp_Misc_Translator.rtl) {
      const totalW = maxX + pad
      for (const n of newNodes)
        n.x = totalW - n.x - n.w

      for (const a of newArrows) {
        a.x1 = totalW - a.x1
        a.x2 = totalW - a.x2
      }
    }

    root.contentW = maxX + pad
    root.contentH = maxY + pad
    root.nodes    = newNodes
    root.arrows   = newArrows

    canvas.requestPaint()
  }

  // Icon helpers.
  function busTypeIcon(busType) {
    const base = "qrc:/icons/devices/drivers/"
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
      case 9:  return base + "mqtt.svg"
      default: return base + "uart.svg"
    }
  }

  function groupIcon(grp) {
    const base = "qrc:/icons/project-editor/treeview/"

    // Output groups use the output-panel icon
    if (grp.groupType === SerialStudio.GroupOutput)
      return "qrc:/icons/dashboard-small/output-panel.svg"

    switch ((grp.widget || "").toLowerCase()) {
      case "multiplot":      return base + "multiplot.svg"
      case "accelerometer":  return base + "accelerometer.svg"
      case "gyroscope":      return base + "gyroscope.svg"
      case "gps":            return base + "gps.svg"
      case "image":          return base + "image.svg"
      case "painter":        return base + "painter.svg"
      case "plot3d":         return base + "plot3d.svg"
      case "datagrid":       return base + "datagrid.svg"
      default:               return base + "group.svg"
    }
  }

  function datasetIcon() {
    return "qrc:/icons/project-editor/treeview/dataset.svg"
  }

  function outputWidgetIcon(type) {
    const base = "qrc:/icons/project-editor/treeview/"
    switch (type) {
      case SerialStudio.OutputButton:        return base + "output-button.svg"
      case SerialStudio.OutputSlider:        return base + "output-slider.svg"
      case SerialStudio.OutputToggle:        return base + "output-toggle.svg"
      case SerialStudio.OutputTextField:     return base + "output-textfield.svg"
      case SerialStudio.OutputKnob:          return base + "output-knob.svg"
      default:                               return base + "output-button.svg"
    }
  }

  // True when the project's MQTT Publisher is enabled (Pro-only context property).
  function mqttPublisherEnabled() {
    return Cpp_CommercialBuild
        && typeof Cpp_MQTT_Publisher !== "undefined"
        && Cpp_MQTT_Publisher.enabled === true
  }

  // Reactive connections.
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

  // Repaint when the publisher is toggled on/off so the node appears/disappears.
  Loader {
    active: Cpp_CommercialBuild
    sourceComponent: Connections {
      target: Cpp_MQTT_Publisher
      function onConfigurationChanged() { root.reloadDiagram() }
    }
  }

  // Re-flow the diagram when the active language toggles RTL/LTR.
  Connections {
    target: Cpp_Misc_Translator
    function onLanguageChanged() { root.reloadDiagram() }
  }

  Component.onCompleted: { seedCollapseFromModel(); reloadDiagram() }
  onVisibleChanged: if (visible) { reloadDiagram(); canvas.requestPaint() }
  onWidthChanged:   if (visible && width > 0) reloadDiagram()
  onHeightChanged:  if (visible && height > 0) reloadDiagram()

  // Flickable + scaled canvas.
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

            ctx.strokeStyle = Cpp_ThemeManager.colors["mid"]
            ctx.fillStyle   = Cpp_ThemeManager.colors["mid"]
            ctx.lineWidth   = 1.5 * z
            ctx.setLineDash([])

            const hl  = 7 * z
            const sin = Math.sin(Math.PI / 6)
            ctx.setLineDash([])

            if (a.verticalEnd) {
              const dirY = (y2 >= y1) ? 1 : -1
              const dirX = (x2 >= x1) ? 1 : -1
              const hly  = hl * dirY
              const y2a  = a.noHead ? y2 : (y2 - hly)

              const exitDx = Math.max(30 * z, Math.abs(x2 - x1) * 0.3) * dirX
              const c1x    = x1 + exitDx
              const c1y    = y1
              const c2x    = x2
              const c2y    = (y1 + y2a) / 2

              ctx.beginPath()
              ctx.moveTo(x1, y1)
              ctx.bezierCurveTo(c1x, c1y, c2x, c2y, x2, y2a)
              ctx.stroke()

              if (!a.noHead) {
                ctx.beginPath()
                ctx.moveTo(x2, y2)
                ctx.lineTo(x2 - hl * sin, y2 - hly)
                ctx.lineTo(x2 + hl * sin, y2 - hly)
                ctx.closePath()
                ctx.fill()
              }
            } else {
              const dirX = (x2 >= x1) ? 1 : -1
              const hlx  = hl * dirX
              const x2a  = a.noHead ? x2 : (x2 - hlx)
              const mx   = (x1 + x2a) / 2

              ctx.beginPath()
              ctx.moveTo(x1, y1)
              ctx.bezierCurveTo(mx, y1, mx, y2, x2a, y2)
              ctx.stroke()

              if (!a.noHead) {
                ctx.beginPath()
                ctx.moveTo(x2, y2)
                ctx.lineTo(x2 - hlx, y2 - hl * sin)
                ctx.lineTo(x2 - hlx, y2 + hl * sin)
                ctx.closePath()
                ctx.fill()
              }
            }
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

      //
      // Node repeater
      //
      Repeater {
        model: root.nodes

        delegate: Item {
          id: nd

          x:      modelData.x * root.zoom
          y:      modelData.y * root.zoom
          width:  modelData.w * root.zoom
          height: modelData.h * root.zoom

          property bool hovered:       false
          property bool isPill:        isDataset || isOutput
          property bool isTable:       modelData.type === "table"
          property bool isOutput:      modelData.type === "output"
          property bool isAction:      modelData.type === "action"
          property bool isSource:      modelData.type === "source"
          property bool isDataset:     modelData.type === "dataset"
          property bool isTransform:   modelData.type === "transform"
          property bool isFolder:      isGroupFolder || isTableFolder
          property bool isFP:          modelData.type === "frameparser"
          property bool isGroupFolder: modelData.type === "groupfolder"
          property bool isTableFolder: modelData.type === "tablefolder"
          property bool isOutputPanel: modelData.type === "output-panel"
          property bool isControlScript: modelData.type === "controlscript"
          property bool collapsibleCard: !isFolder && !!modelData.collapseKey

          readonly property string nodeKey: menuController.keyOf(modelData)
          readonly property bool isPinned: menuController.pinnedKey === nodeKey
                                           && nodeKey !== ""
          readonly property bool active: hovered || isPinned

          //
          // -- Pill (dataset / output widget) ---------------------------
          //
          Rectangle {
            visible: nd.isPill
            anchors.fill: parent
            radius: height / 2
            color: nd.active
              ? Cpp_ThemeManager.colors["highlight"]
              : Cpp_ThemeManager.colors["groupbox_background"]
            border.width: 1
            border.color: nd.active
              ? Cpp_ThemeManager.colors["highlight"]
              : Cpp_ThemeManager.colors["groupbox_border"]

            Row {
              spacing: 4 * root.zoom
              anchors.centerIn: parent
              width: parent.width - 12

              Image {
                visible: (modelData.icon || "") !== ""
                width: 12 * root.zoom
                height: 12 * root.zoom
                anchors.verticalCenter: parent.verticalCenter
                source: modelData.icon || ""
                sourceSize: Qt.size(12, 12)
                smooth: true
              }

              Text {
                width: parent.width - (modelData.icon ? 16 * root.zoom : 0)
                elide: Text.ElideRight
                anchors.verticalCenter: parent.verticalCenter
                text: modelData.label
                font.pixelSize: Math.max(8, 11 * root.zoom)
                color: nd.active
                  ? Cpp_ThemeManager.colors["highlighted_text"]
                  : Cpp_ThemeManager.colors["text"]
              }
            }
          }

          //
          // Transform block (its own node, left-click opens editor)
          //
          Rectangle {
            visible: nd.isTransform
            anchors.fill: parent
            radius: 4 * root.zoom
            color: nd.active
              ? Cpp_ThemeManager.colors["highlight"]
              : Cpp_ThemeManager.colors["groupbox_background"]
            border.width: 1
            border.color: nd.active
              ? Cpp_ThemeManager.colors["highlight"]
              : Cpp_ThemeManager.colors["groupbox_border"]

            Image {
              smooth: true
              width: 14 * root.zoom
              height: 14 * root.zoom
              anchors.centerIn: parent
              sourceSize: Qt.size(14, 14)
              opacity: nd.active ? 1.0 : 0.85
              source: "qrc:/icons/project-editor/treeview/transform.svg"
            }

            ToolTip.delay: 400
            ToolTip.visible: nd.hovered && nd.isTransform
            ToolTip.text: qsTr("Open the transform code editor for this dataset.")
          }

          //
          // -- Card (source / group / frame-parser / action / table) ----
          //
          Rectangle {
            visible: !nd.isPill && !nd.isTransform && !nd.isGroupFolder
            anchors.fill: parent
            radius: 6 * root.zoom
            color: nd.active
              ? Cpp_ThemeManager.colors["highlight"]
              : Cpp_ThemeManager.colors["groupbox_background"]
            border.width: 1
            border.color: nd.active
              ? Cpp_ThemeManager.colors["highlight"]
              : Cpp_ThemeManager.colors["groupbox_border"]

            Row {
              anchors {
                left: parent.left
                right: parent.right
                leftMargin:  10 * root.zoom
                rightMargin:  8 * root.zoom
                verticalCenter: parent.verticalCenter
              }
              spacing: 8 * root.zoom

              Image {
                smooth: true
                width:  20 * root.zoom
                height: 20 * root.zoom
                source: modelData.icon
                sourceSize: Qt.size(20, 20)
                anchors.verticalCenter: parent.verticalCenter
                opacity: nd.isFP ? 0.7 : (nd.active ? 1.0 : 0.85)
              }

              Text {
                width: parent.width - 36 * root.zoom
                elide: Text.ElideRight
                anchors.verticalCenter: parent.verticalCenter
                text: modelData.label
                font.pixelSize: Math.max(8, 12 * root.zoom)
                font.bold: nd.isSource
                font.italic: nd.isFP
                color: nd.active
                  ? Cpp_ThemeManager.colors["highlighted_text"]
                  : (nd.isFP
                      ? Qt.lighter(Cpp_ThemeManager.colors["text"], 1.3)
                      : Cpp_ThemeManager.colors["text"])
              }
            }

            //
            // Badge in the corner: "[A]"/"[B]" on source cards,
            // "N regs"/"empty" on table cards.
            //
            Text {
              visible: (nd.isSource || nd.isTable || nd.isControlScript)
                       && (modelData.badge || "") !== ""
              anchors { right: parent.right; bottom: parent.bottom; margins: 4 * root.zoom }
              text: modelData.badge || ""
              font.family: Cpp_Misc_CommonFonts.monoFont.family
              font.pixelSize: Math.max(7, 9 * root.zoom)
              opacity: 0.6
              color: nd.active
                ? Cpp_ThemeManager.colors["highlighted_text"]
                : Cpp_ThemeManager.colors["text"]
            }
          }

          //
          // -- Folder (group / table): a card in the flow; chevron shows expand/collapse state --
          //
          Rectangle {
            visible: nd.isFolder
            anchors.fill: parent
            radius: 6 * root.zoom
            color: nd.active
              ? Cpp_ThemeManager.colors["highlight"]
              : Cpp_ThemeManager.colors["groupbox_background"]
            border.width: 1
            border.color: nd.active
              ? Cpp_ThemeManager.colors["highlight"]
              : Cpp_ThemeManager.colors["groupbox_border"]

            Row {
              spacing: 6 * root.zoom
              anchors {
                left: parent.left
                right: parent.right
                leftMargin:  8 * root.zoom
                rightMargin: 8 * root.zoom
                verticalCenter: parent.verticalCenter
              }

              Image {
                smooth: true
                width:  9 * root.zoom
                height: 9 * root.zoom
                sourceSize: Qt.size(9, 9)
                rotation: modelData.collapsed ? 270 : 0
                anchors.verticalCenter: parent.verticalCenter
                source: "qrc:/icons/project-editor/treeview/indicator.svg"
              }

              Image {
                smooth: true
                width:  18 * root.zoom
                height: 18 * root.zoom
                source: modelData.icon
                sourceSize: Qt.size(18, 18)
                opacity: nd.active ? 1.0 : 0.85
                anchors.verticalCenter: parent.verticalCenter
              }

              Text {
                width: parent.width - 45 * root.zoom
                elide: Text.ElideRight
                text: modelData.label
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: Math.max(8, 12 * root.zoom)
                color: nd.active
                  ? Cpp_ThemeManager.colors["highlighted_text"]
                  : Cpp_ThemeManager.colors["text"]
              }
            }

            MouseArea {
              hoverEnabled: true
              anchors.fill: parent
              cursorShape: Qt.PointingHandCursor
              onEntered: nd.hovered = true
              onExited:  nd.hovered = false
              onClicked: root.toggleCollapse(modelData.collapseKey)
            }
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
                menuController.openForNode(modelData)
            }

            onDoubleClicked: (mouse) => {
              if (mouse.button !== Qt.LeftButton)
                return

              switch (modelData.type) {
                case "source":
                  Cpp_JSON_ProjectEditor.selectSource(modelData.sourceId)
                  break
                case "frameparser":
                  Cpp_JSON_ProjectEditor.selectFrameParser(modelData.sourceId)
                  break
                case "group":
                  Cpp_JSON_ProjectEditor.selectGroup(modelData.groupId)
                  break
                case "dataset":
                  Cpp_JSON_ProjectEditor.selectDataset(modelData.groupId, modelData.datasetId)
                  break
                case "output":
                  Cpp_JSON_ProjectEditor.selectOutputWidget(modelData.groupId, modelData.widgetId)
                  break
                case "output-panel":
                  Cpp_JSON_ProjectEditor.selectGroup(modelData.groupId)
                  break
                case "action":
                  Cpp_JSON_ProjectEditor.selectAction(modelData.actionId)
                  break
                case "table":
                  Cpp_JSON_ProjectEditor.selectUserTable(modelData.tableName)
                  break
                case "controlscript":
                  Cpp_JSON_ProjectEditor.selectControlScript()
                  break
                case "transform":
                  Cpp_JSON_ProjectEditor.openTransformEditorFor(
                    modelData.groupId, modelData.datasetId)
                  break
                case "mqtt-publisher":
                  Cpp_JSON_ProjectEditor.selectMqttPublisher()
                  break
              }
            }
          }

          //
          // -- Collapse chevron for collapsible cards (device / group / output panel) ----
          //
          Image {
            smooth: true
            width:  9 * root.zoom
            height: 9 * root.zoom
            sourceSize: Qt.size(9, 9)
            visible: nd.collapsibleCard
            rotation: modelData.collapsed ? 270 : 0
            source: "qrc:/icons/project-editor/treeview/indicator.svg"
            anchors {
              right: parent.right
              rightMargin: 8 * root.zoom
              verticalCenter: parent.verticalCenter
            }
          }

          MouseArea {
            width: 24 * root.zoom
            visible: nd.collapsibleCard
            enabled: nd.collapsibleCard
            cursorShape: Qt.PointingHandCursor
            anchors { top: parent.top; right: parent.right; bottom: parent.bottom }
            onClicked: root.toggleCollapse(modelData.collapseKey)
          }
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
  QtObject {
    id: menuController

    property string pinnedKey: ""

    property string currentType: ""
    property string currentLabel: ""
    property string currentWidget: ""
    property string currentTableName: ""
    property int    currentSiblingCount: 0
    property int    currentSourceId:     -1
    property int    currentGroupId:      -1
    property int    currentDatasetId:    -1
    property int    currentWidgetId:     -1
    property int    currentActionId:     -1

    readonly property bool canMoveUp:
      currentSiblingCount > 1 && currentMoveIndex() > 0
    readonly property bool canMoveDown:
      currentSiblingCount > 1 && currentMoveIndex() < currentSiblingCount - 1

    function currentMoveIndex() {
      switch (currentType) {
        case "group":   return currentGroupId
        case "dataset": return currentDatasetId
        case "output":  return currentWidgetId
        case "action":  return currentActionId
      }
      return -1
    }

    function keyOf(node) {
      if (!node || !node.type) return ""
      switch (node.type) {
        case "source":         return "src:"   + node.sourceId
        case "frameparser":    return "fp:"    + node.sourceId
        case "group":          return "grp:"   + node.groupId
        case "dataset":        return "ds:"    + node.groupId + ":" + node.datasetId
        case "output":         return "out:"   + node.groupId + ":" + node.widgetId
        case "output-panel":   return "opnl:"  + node.groupId
        case "transform":      return "tx:"    + node.groupId + ":" + node.datasetId
        case "action":         return "act:"   + node.actionId
        case "table":          return "tbl:"   + (node.tableName || "")
        case "controlscript":  return "controlscript"
        case "mqtt-publisher": return "mqtt-publisher"
      }
      return ""
    }

    function openForBackground() {
      pinnedKey           = "background"
      currentType         = "background"
      currentWidget       = ""
      currentLabel        = ""
      currentTableName    = ""
      currentSourceId     = -1
      currentGroupId      = -1
      currentDatasetId    = -1
      currentWidgetId     = -1
      currentActionId     = -1
      currentSiblingCount = 0
      backgroundMenu.popup()
    }

    function openForNode(node) {
      pinnedKey           = keyOf(node)
      currentType         = node.type
      currentWidget       = node.widget       !== undefined ? node.widget       : ""
      currentLabel        = node.label        !== undefined ? node.label        : ""
      currentTableName    = node.tableName    !== undefined ? node.tableName    : ""
      currentSourceId     = node.sourceId     !== undefined ? node.sourceId     : -1
      currentGroupId      = node.groupId      !== undefined ? node.groupId      : -1
      currentDatasetId    = node.datasetId    !== undefined ? node.datasetId    : -1
      currentWidgetId     = node.widgetId     !== undefined ? node.widgetId     : -1
      currentActionId     = node.actionId     !== undefined ? node.actionId     : -1
      currentSiblingCount = node.siblingCount !== undefined ? node.siblingCount : 0

      switch (currentType) {
        case "source":       sourceMenu.popup();      break
        case "frameparser":  frameparserMenu.popup(); break
        case "group":        groupMenu.popup();       break
        case "dataset":      datasetMenu.popup();     break
        case "output":       outputMenu.popup();      break
        case "output-panel": outputPanelMenu.popup(); break
        case "action":        actionMenu.popup();        break
        case "table":         tableMenu.popup();         break
        case "transform":     transformMenu.popup();     break
        case "controlscript": controlScriptMenu.popup(); break
        default:             pinnedKey = ""
      }
    }

    function unpin() { pinnedKey = "" }

    function selectTargetGroup() {
      if ((currentType === "group" || currentType === "output-panel")
          && currentGroupId >= 0)
        Cpp_JSON_ProjectEditor.selectGroup(currentGroupId)
    }

    function targetSourceId() {
      if ((currentType === "source" || currentType === "frameparser")
          && currentSourceId >= 0)
        return currentSourceId
      return -1
    }

    function locked(fn) {
      Cpp_JSON_ProjectEditor.setSuppressViewChange(true)
      try { fn() } finally {
        Qt.callLater(() => Cpp_JSON_ProjectEditor.setSuppressViewChange(false))
      }
    }

  }

  //
  // Reusable Actions for "Add ..." (shared between background and per-group menus)
  //
  Item {
    visible: false

    //
    // Group variants
    //
    Action {
      id: actAddGroupGeneric

      icon.width: 16
      icon.height: 16
      text: qsTr("Dataset Container")
      icon.source: "qrc:/icons/project-editor/toolbar/add-group.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.addGroup(qsTr("Dataset Container"),
                                       SerialStudio.NoGroupWidget,
                                       menuController.targetSourceId()))
    }
    Action {
      id: actAddGroupMultiPlot

      icon.width: 16
      icon.height: 16
      text: qsTr("Multi-Plot")
      icon.source: "qrc:/icons/project-editor/toolbar/add-multiplot.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.addGroup(qsTr("Multiple Plot"),
                                       SerialStudio.MultiPlot,
                                       menuController.targetSourceId()))
    }
    Action {
      id: actAddGroupAccel

      icon.width: 16
      icon.height: 16
      text: qsTr("Accelerometer")
      icon.source: "qrc:/icons/project-editor/toolbar/add-accelerometer.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.addGroup(qsTr("Accelerometer"),
                                       SerialStudio.Accelerometer,
                                       menuController.targetSourceId()))
    }
    Action {
      id: actAddGroupGyro

      icon.width: 16
      icon.height: 16
      text: qsTr("Gyroscope")
      icon.source: "qrc:/icons/project-editor/toolbar/add-gyroscope.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.addGroup(qsTr("Gyroscope"),
                                       SerialStudio.Gyroscope,
                                       menuController.targetSourceId()))
    }
    Action {
      id: actAddGroupGps

      icon.width: 16
      icon.height: 16
      text: qsTr("GPS Map")
      icon.source: "qrc:/icons/project-editor/toolbar/add-gps.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.addGroup(qsTr("GPS Map"), SerialStudio.GPS,
                                       menuController.targetSourceId()))
    }
    Action {
      id: actAddGroupPlot3D

      icon.width: 16
      icon.height: 16
      text: qsTr("3D Plot")
      icon.source: "qrc:/icons/project-editor/toolbar/add-plot3d.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.addGroup(qsTr("3D Plot"), SerialStudio.Plot3D,
                                       menuController.targetSourceId()))
    }
    Action {
      id: actAddGroupImage

      icon.width: 16
      icon.height: 16
      text: qsTr("Image View")
      icon.source: "qrc:/icons/project-editor/toolbar/image.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.addGroup(qsTr("Image View"),
                                       SerialStudio.ImageView,
                                       menuController.targetSourceId()))
    }
    Action {
      id: actAddGroupPainter

      icon.width: 16
      icon.height: 16
      text: qsTr("Painter Widget")
      icon.source: "qrc:/icons/project-editor/toolbar/add-painter.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.addGroup(qsTr("Painter Widget"),
                                       SerialStudio.Painter,
                                       menuController.targetSourceId()))
    }
    Action {
      id: actAddGroupWebView

      icon.width: 16
      icon.height: 16
      text: qsTr("Web View")
      icon.source: "qrc:/icons/project-editor/toolbar/add-webview.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.addGroup(qsTr("Web View"),
                                       SerialStudio.WebView,
                                       menuController.targetSourceId()))
    }
    Action {
      id: actAddGroupDataGrid

      icon.width: 16
      icon.height: 16
      text: qsTr("Data Grid")
      icon.source: "qrc:/icons/project-editor/toolbar/add-datagrid.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.addGroup(qsTr("Data Grid"),
                                       SerialStudio.DataGrid,
                                       menuController.targetSourceId()))
    }

    //
    // Dataset variants (target-group selection lives on menuController)
    //
    Action {
      id: actAddDsGeneric

      icon.width: 16
      icon.height: 16
      text: qsTr("Generic")
      icon.source: "qrc:/icons/project-editor/toolbar/add-dataset.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetGeneric,
                                         menuController.targetSourceId())
      })
    }
    Action {
      id: actAddDsPlot

      icon.width: 16
      icon.height: 16
      text: qsTr("Plot")
      icon.source: "qrc:/icons/project-editor/toolbar/add-plot.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetPlot,
                                         menuController.targetSourceId())
      })
    }
    Action {
      id: actAddDsFFT

      icon.width: 16
      icon.height: 16
      text: qsTr("FFT Plot")
      icon.source: "qrc:/icons/project-editor/toolbar/add-fft.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetFFT,
                                         menuController.targetSourceId())
      })
    }
    Action {
      id: actAddDsGauge

      icon.width: 16
      icon.height: 16
      text: qsTr("Gauge")
      icon.source: "qrc:/icons/project-editor/toolbar/add-gauge.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetGauge,
                                         menuController.targetSourceId())
      })
    }
    Action {
      id: actAddDsBar

      icon.width: 16
      icon.height: 16
      text: qsTr("Level Indicator")
      icon.source: "qrc:/icons/project-editor/toolbar/add-bar.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetBar,
                                         menuController.targetSourceId())
      })
    }
    Action {
      id: actAddDsCompass

      icon.width: 16
      icon.height: 16
      text: qsTr("Compass")
      icon.source: "qrc:/icons/project-editor/toolbar/add-compass.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetCompass,
                                         menuController.targetSourceId())
      })
    }
    Action {
      id: actAddDsMeter

      icon.width: 16
      icon.height: 16
      text: qsTr("Meter")
      icon.source: "qrc:/icons/project-editor/toolbar/add-meter.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetMeter,
                                         menuController.targetSourceId())
      })
    }
    Action {
      id: actAddDsLED

      icon.width: 16
      icon.height: 16
      text: qsTr("LED Indicator")
      icon.source: "qrc:/icons/project-editor/toolbar/add-led.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetLED,
                                         menuController.targetSourceId())
      })
    }

    //
    // Output variants (Pro)
    //
    Action {
      id: actAddOutPanel

      icon.width: 16
      icon.height: 16
      text: qsTr("Output Panel")
      icon.source: "qrc:/icons/project-editor/toolbar/add-output-panel.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addOutputPanel(menuController.targetSourceId())
      })
    }
    Action {
      id: actAddOutSlider

      icon.width: 16
      icon.height: 16
      text: qsTr("Slider")
      icon.source: "qrc:/icons/project-editor/toolbar/add-output-slider.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputSlider,
                                               menuController.targetSourceId())
      })
    }
    Action {
      id: actAddOutToggle

      icon.width: 16
      icon.height: 16
      text: qsTr("Toggle")
      icon.source: "qrc:/icons/project-editor/toolbar/add-output-toggle.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputToggle,
                                               menuController.targetSourceId())
      })
    }
    Action {
      id: actAddOutKnob

      icon.width: 16
      icon.height: 16
      text: qsTr("Knob")
      icon.source: "qrc:/icons/project-editor/toolbar/add-output-knob.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputKnob,
                                               menuController.targetSourceId())
      })
    }
    Action {
      id: actAddOutText

      icon.width: 16
      icon.height: 16
      text: qsTr("Text Field")
      icon.source: "qrc:/icons/project-editor/toolbar/add-output-textfield.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputTextField,
                                               menuController.targetSourceId())
      })
    }
    Action {
      id: actAddOutButton

      icon.width: 16
      icon.height: 16
      text: qsTr("Button")
      icon.source: "qrc:/icons/project-editor/toolbar/add-output-button.svg"
      onTriggered: menuController.locked(() => {
        menuController.selectTargetGroup()
        Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputButton,
                                               menuController.targetSourceId())
      })
    }
  }

  //
  // -- Per-context Menus --------------------------------------------------
  //
  // Each Menu is a fixed list of items relevant to one node type. No
  // conditional visibility on individual items (avoids empty-slot bugs).
  //
  Menu {
    id: backgroundMenu

    onClosed: menuController.unpin()

    Menu {
      icon.width: 16
      icon.height: 16
      title: qsTr("Add Group")
      icon.source: "qrc:/icons/project-editor/treeview/group.svg"
      MenuItem { action: actAddGroupGeneric }
      MenuItem { action: actAddGroupMultiPlot }
      MenuItem { action: actAddGroupAccel }
      MenuItem { action: actAddGroupGyro }
      MenuItem { action: actAddGroupGps }
      MenuItem { action: actAddGroupPlot3D }
      MenuItem { action: actAddGroupImage }
      MenuItem { action: actAddGroupPainter }
      MenuItem { action: actAddGroupWebView }
      MenuItem { action: actAddGroupDataGrid }
    }
    Menu {
      icon.width: 16
      icon.height: 16
      title: qsTr("Add Dataset")
      icon.source: "qrc:/icons/project-editor/treeview/dataset.svg"
      MenuItem { action: actAddDsGeneric }
      MenuItem { action: actAddDsPlot }
      MenuItem { action: actAddDsFFT }
      MenuItem { action: actAddDsGauge }
      MenuItem { action: actAddDsBar }
      MenuItem { action: actAddDsCompass }
      MenuItem { action: actAddDsMeter }
      MenuItem { action: actAddDsLED }
    }
    Menu {
      icon.width: 16
      icon.height: 16
      title: qsTr("Add Output")
      enabled: Cpp_CommercialBuild
      icon.source: "qrc:/icons/project-editor/treeview/output-panel.svg"
      MenuItem { action: actAddOutPanel }
      MenuItem { action: actAddOutSlider }
      MenuItem { action: actAddOutToggle }
      MenuItem { action: actAddOutKnob }
      MenuItem { action: actAddOutText }
      MenuItem { action: actAddOutButton }
    }

    MenuSeparator {}

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Add Action")
      icon.source: "qrc:/icons/project-editor/toolbar/add-action.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.addAction(menuController.targetSourceId()))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      enabled: Cpp_CommercialBuild
      text: qsTr("Add Data Source")
      icon.source: "qrc:/icons/project-editor/toolbar/add-device.svg"
      onTriggered: menuController.locked(() => Cpp_JSON_ProjectModel.addSource())
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Add Data Table")
      icon.source: "qrc:/icons/project-editor/treeview/shared-table.svg"
      onTriggered: menuController.locked(() => Cpp_JSON_ProjectModel.promptAddTable())
    }
  }

  Menu {
    id: sourceMenu

    onClosed: menuController.unpin()

    Menu {
      icon.width: 16
      icon.height: 16
      title: qsTr("Add Group")
      icon.source: "qrc:/icons/project-editor/treeview/group.svg"
      MenuItem { action: actAddGroupGeneric }
      MenuItem { action: actAddGroupMultiPlot }
      MenuItem { action: actAddGroupAccel }
      MenuItem { action: actAddGroupGyro }
      MenuItem { action: actAddGroupGps }
      MenuItem { action: actAddGroupPlot3D }
      MenuItem { action: actAddGroupImage }
      MenuItem { action: actAddGroupPainter }
      MenuItem { action: actAddGroupWebView }
      MenuItem { action: actAddGroupDataGrid }
    }
    Menu {
      icon.width: 16
      icon.height: 16
      title: qsTr("Add Dataset")
      icon.source: "qrc:/icons/project-editor/treeview/dataset.svg"
      MenuItem { action: actAddDsGeneric }
      MenuItem { action: actAddDsPlot }
      MenuItem { action: actAddDsFFT }
      MenuItem { action: actAddDsGauge }
      MenuItem { action: actAddDsBar }
      MenuItem { action: actAddDsCompass }
      MenuItem { action: actAddDsMeter }
      MenuItem { action: actAddDsLED }
    }
    Menu {
      icon.width: 16
      icon.height: 16
      title: qsTr("Add Output")
      enabled: Cpp_CommercialBuild
      icon.source: "qrc:/icons/project-editor/treeview/output-panel.svg"
      MenuItem { action: actAddOutPanel }
      MenuItem { action: actAddOutSlider }
      MenuItem { action: actAddOutToggle }
      MenuItem { action: actAddOutKnob }
      MenuItem { action: actAddOutText }
      MenuItem { action: actAddOutButton }
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Add Action")
      icon.source: "qrc:/icons/project-editor/toolbar/add-action.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.addAction(menuController.targetSourceId()))
    }

    MenuSeparator {}

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Rename…")
      icon.source: "qrc:/icons/project-editor/actions/rename.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.promptRenameSource(menuController.currentSourceId))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Duplicate")
      icon.source: "qrc:/icons/project-editor/actions/duplicate.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.duplicateSource(menuController.currentSourceId))
    }

    MenuSeparator {}

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Delete…")
      icon.source: "qrc:/icons/project-editor/actions/delete.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.deleteSource(menuController.currentSourceId, true))
    }
  }

  Menu {
    id: frameparserMenu

    onClosed: menuController.unpin()

    Menu {
      icon.width: 16
      icon.height: 16
      title: qsTr("Add Group")
      icon.source: "qrc:/icons/project-editor/treeview/group.svg"
      MenuItem { action: actAddGroupGeneric }
      MenuItem { action: actAddGroupMultiPlot }
      MenuItem { action: actAddGroupAccel }
      MenuItem { action: actAddGroupGyro }
      MenuItem { action: actAddGroupGps }
      MenuItem { action: actAddGroupPlot3D }
      MenuItem { action: actAddGroupImage }
      MenuItem { action: actAddGroupPainter }
      MenuItem { action: actAddGroupWebView }
      MenuItem { action: actAddGroupDataGrid }
    }
    Menu {
      icon.width: 16
      icon.height: 16
      title: qsTr("Add Dataset")
      icon.source: "qrc:/icons/project-editor/treeview/dataset.svg"
      MenuItem { action: actAddDsGeneric }
      MenuItem { action: actAddDsPlot }
      MenuItem { action: actAddDsFFT }
      MenuItem { action: actAddDsGauge }
      MenuItem { action: actAddDsBar }
      MenuItem { action: actAddDsCompass }
      MenuItem { action: actAddDsMeter }
      MenuItem { action: actAddDsLED }
    }
    Menu {
      icon.width: 16
      icon.height: 16
      title: qsTr("Add Output")
      enabled: Cpp_CommercialBuild
      icon.source: "qrc:/icons/project-editor/treeview/output-panel.svg"
      MenuItem { action: actAddOutPanel }
      MenuItem { action: actAddOutSlider }
      MenuItem { action: actAddOutToggle }
      MenuItem { action: actAddOutKnob }
      MenuItem { action: actAddOutText }
      MenuItem { action: actAddOutButton }
    }

    MenuSeparator {}

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Edit Frame Parser…")
      icon.source: "qrc:/icons/project-editor/actions/edit-code.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectEditor.selectFrameParser(menuController.currentSourceId))
    }
  }

  Menu {
    id: groupMenu

    onClosed: menuController.unpin()

    Menu {
      icon.width: 16
      icon.height: 16
      title: qsTr("Add Dataset")
      icon.source: "qrc:/icons/project-editor/treeview/dataset.svg"
      MenuItem { action: actAddDsGeneric }
      MenuItem { action: actAddDsPlot }
      MenuItem { action: actAddDsFFT }
      MenuItem { action: actAddDsGauge }
      MenuItem { action: actAddDsBar }
      MenuItem { action: actAddDsCompass }
      MenuItem { action: actAddDsMeter }
      MenuItem { action: actAddDsLED }
    }
    Menu {
      icon.width: 16
      icon.height: 16
      title: qsTr("Add Output")
      enabled: Cpp_CommercialBuild
      icon.source: "qrc:/icons/project-editor/treeview/output-panel.svg"
      MenuItem { action: actAddOutPanel }
      MenuItem { action: actAddOutSlider }
      MenuItem { action: actAddOutToggle }
      MenuItem { action: actAddOutKnob }
      MenuItem { action: actAddOutText }
      MenuItem { action: actAddOutButton }
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Edit Painter Code…")
      icon.source: "qrc:/icons/project-editor/actions/edit-code.svg"
      enabled: menuController.currentWidget === "painter" && Cpp_CommercialBuild
      onTriggered: menuController.locked(() => {
        Cpp_JSON_ProjectEditor.selectGroup(menuController.currentGroupId)
        painterCodeDialog.showDialog()
      })
    }

    MenuSeparator {}

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Rename…")
      icon.source: "qrc:/icons/project-editor/actions/rename.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.promptRenameGroup(menuController.currentGroupId))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Move Up")
      opacity: enabled ? 1 : 0.5
      enabled: menuController.canMoveUp
      icon.source: "qrc:/icons/project-editor/actions/move-up.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.moveGroup(menuController.currentGroupId,
                                        menuController.currentGroupId - 1))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Move Down")
      opacity: enabled ? 1 : 0.5
      enabled: menuController.canMoveDown
      icon.source: "qrc:/icons/project-editor/actions/move-down.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.moveGroup(menuController.currentGroupId,
                                        menuController.currentGroupId + 1))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Duplicate")
      icon.source: "qrc:/icons/project-editor/actions/duplicate.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.duplicateGroup(menuController.currentGroupId))
    }

    MenuSeparator {}

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Delete…")
      icon.source: "qrc:/icons/project-editor/actions/delete.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.deleteGroup(menuController.currentGroupId, true))
    }
  }

  Menu {
    id: datasetMenu

    onClosed: menuController.unpin()

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Edit Transform Code…")
      icon.source: "qrc:/icons/project-editor/actions/transform.svg"
      onTriggered: Cpp_JSON_ProjectEditor.openTransformEditorFor(
        menuController.currentGroupId, menuController.currentDatasetId)
    }

    MenuSeparator {}

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Rename…")
      icon.source: "qrc:/icons/project-editor/actions/rename.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.promptRenameDataset(menuController.currentGroupId,
                                                  menuController.currentDatasetId))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Move Up")
      opacity: enabled ? 1 : 0.5
      enabled: menuController.canMoveUp
      icon.source: "qrc:/icons/project-editor/actions/move-up.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.moveDataset(menuController.currentGroupId,
                                          menuController.currentDatasetId,
                                          menuController.currentDatasetId - 1))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Move Down")
      opacity: enabled ? 1 : 0.5
      enabled: menuController.canMoveDown
      icon.source: "qrc:/icons/project-editor/actions/move-down.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.moveDataset(menuController.currentGroupId,
                                          menuController.currentDatasetId,
                                          menuController.currentDatasetId + 1))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Duplicate")
      icon.source: "qrc:/icons/project-editor/actions/duplicate.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.duplicateDataset(menuController.currentGroupId,
                                               menuController.currentDatasetId))
    }

    MenuSeparator {}

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Delete…")
      icon.source: "qrc:/icons/project-editor/actions/delete.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.deleteDataset(menuController.currentGroupId,
                                            menuController.currentDatasetId, true))
    }
  }

  Menu {
    id: outputMenu

    onClosed: menuController.unpin()

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Move Up")
      opacity: enabled ? 1 : 0.5
      enabled: menuController.canMoveUp
      icon.source: "qrc:/icons/project-editor/actions/move-up.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.moveOutputWidget(menuController.currentGroupId,
                                               menuController.currentWidgetId,
                                               menuController.currentWidgetId - 1))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Move Down")
      opacity: enabled ? 1 : 0.5
      enabled: menuController.canMoveDown
      icon.source: "qrc:/icons/project-editor/actions/move-down.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.moveOutputWidget(menuController.currentGroupId,
                                               menuController.currentWidgetId,
                                               menuController.currentWidgetId + 1))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Duplicate")
      icon.source: "qrc:/icons/project-editor/actions/duplicate.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.duplicateOutputWidget(menuController.currentGroupId,
                                                    menuController.currentWidgetId))
    }

    MenuSeparator {}

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Delete…")
      icon.source: "qrc:/icons/project-editor/actions/delete.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.deleteOutputWidget(menuController.currentGroupId,
                                                 menuController.currentWidgetId, true))
    }
  }

  Menu {
    id: outputPanelMenu

    onClosed: menuController.unpin()

    Menu {
      icon.width: 16
      icon.height: 16
      title: qsTr("Add Output")
      enabled: Cpp_CommercialBuild
      icon.source: "qrc:/icons/project-editor/treeview/output-panel.svg"
      MenuItem { action: actAddOutSlider }
      MenuItem { action: actAddOutToggle }
      MenuItem { action: actAddOutKnob }
      MenuItem { action: actAddOutText }
      MenuItem { action: actAddOutButton }
    }

    MenuSeparator {}

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Rename…")
      icon.source: "qrc:/icons/project-editor/actions/rename.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.promptRenameGroup(menuController.currentGroupId))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Duplicate")
      icon.source: "qrc:/icons/project-editor/actions/duplicate.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.duplicateGroup(menuController.currentGroupId))
    }

    MenuSeparator {}

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Delete…")
      icon.source: "qrc:/icons/project-editor/actions/delete.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.deleteGroup(menuController.currentGroupId, true))
    }
  }

  Menu {
    id: actionMenu

    onClosed: menuController.unpin()

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Rename…")
      icon.source: "qrc:/icons/project-editor/actions/rename.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.promptRenameAction(menuController.currentActionId))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Move Up")
      opacity: enabled ? 1 : 0.5
      enabled: menuController.canMoveUp
      icon.source: "qrc:/icons/project-editor/actions/move-up.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.moveAction(menuController.currentActionId,
                                         menuController.currentActionId - 1))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Move Down")
      opacity: enabled ? 1 : 0.5
      enabled: menuController.canMoveDown
      icon.source: "qrc:/icons/project-editor/actions/move-down.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.moveAction(menuController.currentActionId,
                                         menuController.currentActionId + 1))
    }
    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Duplicate")
      icon.source: "qrc:/icons/project-editor/actions/duplicate.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.duplicateAction(menuController.currentActionId))
    }

    MenuSeparator {}

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Delete…")
      icon.source: "qrc:/icons/project-editor/actions/delete.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.deleteAction(menuController.currentActionId, true))
    }
  }

  Menu {
    id: tableMenu

    onClosed: menuController.unpin()

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Rename…")
      icon.source: "qrc:/icons/project-editor/actions/rename.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.promptRenameTable(menuController.currentTableName))
    }

    MenuSeparator {}

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Delete…")
      icon.source: "qrc:/icons/project-editor/actions/delete.svg"
      onTriggered: menuController.locked(() =>
        Cpp_JSON_ProjectModel.confirmDeleteTable(menuController.currentTableName))
    }
  }

  Menu {
    id: transformMenu

    onClosed: menuController.unpin()

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Edit Code…")
      icon.source: "qrc:/icons/project-editor/actions/transform.svg"
      onTriggered: Cpp_JSON_ProjectEditor.openTransformEditorFor(
        menuController.currentGroupId, menuController.currentDatasetId)
    }
  }

  Menu {
    id: controlScriptMenu

    onClosed: menuController.unpin()

    MenuItem {
      icon.width: 16
      icon.height: 16
      text: qsTr("Edit Control Loop…")
      icon.source: "qrc:/icons/project-editor/actions/edit-code.svg"
      onTriggered: Cpp_JSON_ProjectEditor.selectControlScript()
    }
  }

}
