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

import SerialStudio

//
// Geometry engine for the project flow diagram: project model descriptors in,
// positioned node/arrow lists out. Pure computation, no visuals.
//
QtObject {
  id: layout

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
  // Icon vocabulary, injected by the view.
  //
  required property DiagramIcons icons

  //
  // Collapse state: a map of stable-id node keys (grpfolder:<id>, tblfolder:<id>, grp:<groupId>)
  // whose children are hidden. Owned by the view, read here.
  //
  required property var collapsedKeys

  function isCollapsed(key) {
    if (key in layout.collapsedKeys)
      return layout.collapsedKeys[key] === true

    // Auto-collapse: folders start folded so a foldered project opens compact; groups start open.
    return key.indexOf("grpfolder:") === 0 || key.indexOf("tblfolder:") === 0
  }

  //
  // Stable identity of a node; unmapped node types have no key.
  //
  function nodeKey(node) {
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

  //
  // True when the project's MQTT Publisher is enabled (Pro-only context property).
  //
  function mqttPublisherEnabled() {
    return Cpp_CommercialBuild
        && typeof Cpp_MQTT_Publisher !== "undefined"
        && Cpp_MQTT_Publisher.enabled === true
  }

  //
  // Column layout L->R: Device | Frame Parser/Actions/Outputs | Folders | Groups & Datasets.
  // Returns { nodes, arrows, contentW, contentH }.
  //
  function build(sources, groups, actions, tables) {
    const newNodes  = []
    const newArrows = []
    tables = tables || []

    // -- fixed left columns (device + frame parser); folder/group columns computed below -----
    const colW   = layout.nodeW + layout.hGap
    const colDev = layout.pad
    const colFP  = layout.pad + colW

    // Collected dataset pill centres, fed into the MQTT Publisher node below
    const datasetAnchors = []

    // -- slot height helper -------------------------------------------------
    function slotH(dsCount) {
      if (dsCount === 0) return layout.nodeH
      return Math.max(layout.nodeH, dsCount * (layout.chipH + layout.vGap) - layout.vGap)
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
    function subtreeHasDatasets(fid, sid) {
      for (const g of groups) {
        if (g.groupType === SerialStudio.GroupOutput) continue
        if ((g.sourceId || 0) !== sid) continue
        if (folderOf(g) !== -1 && folderIsSelfOrAncestor(fid, folderOf(g))
            && (g.datasets || []).length > 0)
          return true
      }
      return false
    }

    // Deepest folder column actually rendered (collapsed folders hide children -> reserve less).
    let maxFolderDepth = -1
    function scanVisibleFolders(nodes, sid) {
      for (const n of nodes) {
        if (!subtreeHasSourceGroups(n.id, sid))
          continue

        maxFolderDepth = Math.max(maxFolderDepth, folderDepth(n.id))
        if (!layout.isCollapsed("grpfolder:" + n.id))
          scanVisibleFolders(n.children || [], sid)
      }
    }
    for (const src of srcList)
      if (!layout.isCollapsed("src:" + src.sourceId))
        scanVisibleFolders(gfTree, src.sourceId)

    // -- folder / group / dataset columns (one column per folder level) -----
    const colFolder = colFP + colW                             // first folder column
    const colGrp    = colFolder + (maxFolderDepth + 1) * colW  // group column
    const colTrans  = colGrp + colW                            // transform block column
    const colChip   = colTrans + layout.transW + layout.transGap  // dataset column

    function folderX(depth) {
      return colFolder + depth * colW
    }

    // Tidy-tree cursor (advances down as leaves are placed) + frame-parser Y per source.
    const fpNodeY = {}
    let cursorY   = layout.pad

    // Places a group centred on its pills (one slot when it has none or is collapsed); returns Y.
    function placeGroup(grp, sid) {
      const dsList    = grp.datasets || []
      const collapsed = layout.isCollapsed("grp:" + grp.groupId)
      const pills     = collapsed ? [] : dsList

      let centerY
      const chipYs = []
      if (pills.length === 0) {
        centerY = cursorY + layout.nodeH / 2
        cursorY += layout.nodeH + layout.vGap
      } else {
        const slot   = slotH(pills.length)
        const chipsH = pills.length * (layout.chipH + layout.vGap) - layout.vGap
        let chipY    = cursorY + (slot - chipsH) / 2
        for (let di = 0; di < pills.length; ++di) {
          chipYs.push(chipY)
          chipY += layout.chipH + layout.vGap
        }
        centerY  = cursorY + slot / 2
        cursorY += slot + layout.vGap
      }

      const cardY = centerY - layout.nodeH / 2
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
        w:            layout.nodeW,
        h:            layout.nodeH,
        label:        grp.title || qsTr("Group"),
        icon:         layout.icons.groupIcon(grp),
        badge:        ""
      })

      //
      // Collapsed group still feeds the MQTT Publisher; anchor the fan-in on the card edge.
      //
      if (collapsed && dsList.length > 0)
        datasetAnchors.push({ x: colGrp + layout.nodeW, y: centerY })

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
            w:         layout.transW,
            h:         layout.chipH,
            label:     "",
            icon:      "",
            badge:     ""
          })

          newArrows.push({
            x1: colGrp + layout.nodeW, y1: cardY + layout.nodeH / 2,
            x2: colTrans,              y2: chipY + layout.chipH / 2
          })
          newArrows.push({
            x1: colTrans + layout.transW, y1: chipY + layout.chipH / 2,
            x2: colChip,                  y2: chipY + layout.chipH / 2
          })
        } else {
          newArrows.push({
            x1: colGrp + layout.nodeW, y1: cardY + layout.nodeH / 2,
            x2: colTrans,              y2: chipY + layout.chipH / 2,
            noHead: true
          })
          newArrows.push({
            x1: colTrans, y1: chipY + layout.chipH / 2,
            x2: colChip,  y2: chipY + layout.chipH / 2
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
          w:            layout.chipW,
          h:            layout.chipH,
          label:        ds.units ? (ds.title + " [" + ds.units + "]") : ds.title,
          icon:         layout.icons.datasetIcon(),
          badge:        "",
          hasTransform: hasTransform
        })

        datasetAnchors.push({ x: colChip + layout.chipW, y: chipY + layout.chipH / 2 })
      }

      return centerY
    }

    function placeFolder(f, sid) {
      const depth     = folderDepth(f.id)
      const collapsed = layout.isCollapsed("grpfolder:" + f.id)
      const x         = folderX(depth)

      let centerY
      if (collapsed) {
        centerY = cursorY + layout.nodeH / 2
        cursorY += layout.nodeH + layout.vGap

        //
        // Collapsed folder hides its groups; still feed MQTT from the folder card edge.
        //
        if (subtreeHasDatasets(f.id, sid))
          datasetAnchors.push({ x: x + layout.nodeW, y: centerY })
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
          centerY = cursorY + layout.nodeH / 2
          cursorY += layout.nodeH + layout.vGap
        } else {
          centerY = (kids[0].y + kids[kids.length - 1].y) / 2
          for (const k of kids)
            newArrows.push({ x1: x + layout.nodeW, y1: centerY, x2: k.x, y2: k.y })
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
        y:          centerY - layout.nodeH / 2,
        w:          layout.nodeW,
        h:          layout.nodeH,
        label:      f.title || qsTr("Folder"),
        icon:       Cpp_Misc_IconRegistry.icon("widgets", "folder", 16)
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
    cursorY += layout.nodeH + layout.vGap * 3
    newNodes.push({
      type:      "controlscript",
      sourceId:  -1,
      groupId:   -1,
      datasetId: -1,
      actionId:  -1,
      x:         colDev,
      y:         controlLoopY,
      w:         layout.nodeW,
      h:         layout.nodeH,
      label:     qsTr("Control Loop"),
      icon:      Cpp_Misc_IconRegistry.icon("editor", "control-script", 16),
      badge:     Cpp_JSON_ProjectModel.controlScriptCode.length > 0 ? "" : qsTr("empty")
    })

    const allOutputGroups = groups.filter(g => g.groupType === SerialStudio.GroupOutput)

    for (const src of srcList) {
      const sid          = src.sourceId
      const srcCollapsed = layout.isCollapsed("src:" + sid)
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
        fpCenter = cursorY + layout.nodeH / 2
        cursorY += layout.nodeH + layout.vGap
      } else {
        fpCenter = (tops[0].y + tops[tops.length - 1].y) / 2
      }
      fpNodeY[sid] = fpCenter - layout.nodeH / 2

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
          y:         fpCenter - layout.nodeH / 2,
          w:         layout.nodeW,
          h:         layout.nodeH,
          label:     qsTr("Frame Parser"),
          icon:      Cpp_Misc_IconRegistry.icon("editor", "code", 16)
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
        y:           fpCenter - layout.nodeH / 2,
        w:           layout.nodeW,
        h:           layout.nodeH,
        label:       devTitle,
        icon:        layout.icons.busTypeIcon(src.busType),
        badge:       sources.length > 1
          ? "[" + String.fromCharCode(65 + sid) + "]"
          : ""
      })

      //
      // Arrows: device -> parser, then parser -> each top-level folder / group.
      //
      if (!srcCollapsed) {
        newArrows.push({
          x1: colDev + layout.nodeW, y1: fpCenter,
          x2: colFP,                 y2: fpCenter
        })

        for (const t of tops)
          newArrows.push({
            x1: colFP + layout.nodeW, y1: fpCenter,
            x2: t.x,                  y2: t.y
          })
      }

      //
      // Outputs category (TX direction): a collapsible card feeding this source's output
      // panels and their control pills, so the next source starts below its outputs.
      //
      const srcOutGroups = allOutputGroups.filter(g => (g.sourceId || 0) === sid)
      if (!srcCollapsed && srcOutGroups.length > 0) {
        const outCollapsed = layout.isCollapsed("outputs:" + sid)
        cursorY += layout.vGap * 2

        let catCenter
        if (outCollapsed) {
          catCenter = cursorY + layout.nodeH / 2
          cursorY += layout.nodeH + layout.vGap
        } else {
          const panelCenters = []

          for (const grp of srcOutGroups) {
            const panelCollapsed = layout.isCollapsed("grp:" + grp.groupId)
            const owList         = grp.outputWidgets || []
            const wCount         = panelCollapsed ? 0 : owList.length
            const wsh            = slotH(wCount)
            const panelY         = cursorY + (wsh - layout.nodeH) / 2

            //
            // Panel card (the parent group), mirroring the group column
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
              x:            colGrp,
              y:            panelY,
              w:            layout.nodeW,
              h:            layout.nodeH,
              label:        grp.title || qsTr("Output Panel"),
              icon:         Cpp_Misc_IconRegistry.icon("widgets", "output-panel", 16),
              badge:        ""
            })

            panelCenters.push(panelY + layout.nodeH / 2)

            //
            // Control pills stacked at colChip, one per widget, mirroring the
            // group -> dataset layout. Each gets a single panel -> widget arrow.
            //
            if (wCount > 0) {
              const blockH   = wCount * layout.chipH + (wCount - 1) * layout.vGap
              const blockTop = cursorY + (wsh - blockH) / 2

              for (let oi = 0; oi < wCount; ++oi) {
                const ow    = owList[oi]
                const chipY = blockTop + oi * (layout.chipH + layout.vGap)

                newArrows.push({
                  x1: colGrp + layout.nodeW, y1: panelY + layout.nodeH / 2,
                  x2: colChip,               y2: chipY + layout.chipH / 2
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
                  w:            layout.chipW,
                  h:            layout.chipH,
                  label:        ow.title || qsTr("Control"),
                  icon:         layout.icons.outputWidgetIcon(ow.outputType),
                  badge:        ""
                })
              }
            }

            cursorY += wsh + layout.vGap
          }

          catCenter = (panelCenters[0] + panelCenters[panelCenters.length - 1]) / 2
          for (const pc of panelCenters)
            newArrows.push({
              x1: colFP + layout.nodeW, y1: catCenter,
              x2: colGrp,               y2: pc
            })
        }

        //
        // Category card, with an arrow up into the bottom-center of its device.
        //
        newNodes.push({
          type:        "outputsfolder",
          collapsed:   outCollapsed,
          collapseKey: "outputs:" + sid,
          sourceId:    sid,
          groupId:     -1,
          datasetId:   -1,
          actionId:    -1,
          x:           colFP,
          y:           catCenter - layout.nodeH / 2,
          w:           layout.nodeW,
          h:           layout.nodeH,
          label:       qsTr("Outputs"),
          icon:        Cpp_Misc_IconRegistry.icon("widgets", "output-panel", 16)
        })

        const devTopY = fpNodeY[sid] !== undefined ? fpNodeY[sid] : layout.pad
        newArrows.push({
          x1: colFP,                     y1: catCenter,
          x2: colDev + layout.nodeW / 2, y2: devTopY + layout.nodeH,
          verticalEnd: true
        })
      }

      cursorY += layout.vGap * 3
    }

    let blockCursor = cursorY

    if (actions.length > 0) {
      let actY          = blockCursor + layout.vGap * 2
      let placedActions = 0

      for (let ai = 0; ai < actions.length; ++ai) {
        const act = actions[ai]
        const sid = act.sourceId || 0
        if (layout.isCollapsed("src:" + sid))
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
          w:            layout.nodeW,
          h:            layout.nodeH,
          label:        act.title || qsTr("Action"),
          icon:         act.icon  || Cpp_Misc_IconRegistry.icon("editor", "action", 16),
          badge:        ""
        })

        //
        // Arrow from action up into the bottom-center of its target device.
        //
        const devTopY = fpNodeY[sid] !== undefined ? fpNodeY[sid] : layout.pad
        newArrows.push({
          x1: colFP,                     y1: actY + layout.nodeH / 2,
          x2: colDev + layout.nodeW / 2, y2: devTopY + layout.nodeH,
          verticalEnd: true
        })

        actY += layout.nodeH + layout.vGap
        ++placedActions
      }

      if (placedActions > 0)
        blockCursor = actY
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
        const centerY = cursorY + layout.nodeH / 2
        cursorY += layout.nodeH + layout.vGap

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
          y:         centerY - layout.nodeH / 2,
          w:         layout.nodeW,
          h:         layout.nodeH,
          label:     label,
          icon:      Cpp_Misc_IconRegistry.icon("editor", "shared-table-alt", 16),
          badge:     regs > 0 ? qsTr("%1 regs").arg(regs) : qsTr("empty")
        })
        return centerY
      }

      function placeTableFolder(f) {
        const depth     = tableFolderDepth(f.id)
        const collapsed = layout.isCollapsed("tblfolder:" + f.id)
        const x         = tblFolderX(depth)

        let centerY
        if (collapsed) {
          centerY = cursorY + layout.nodeH / 2
          cursorY += layout.nodeH + layout.vGap
        } else {
          const kids = []
          for (const sub of (f.children || []))
            if (subtreeHasTables(sub.id))
              kids.push({ y: placeTableFolder(sub), x: tblFolderX(tableFolderDepth(sub.id)) })

          for (const t of tables)
            if (tableFolderOf(t) === f.id)
              kids.push({ y: placeTable(t), x: colTbl })

          if (kids.length === 0) {
            centerY = cursorY + layout.nodeH / 2
            cursorY += layout.nodeH + layout.vGap
          } else {
            centerY = (kids[0].y + kids[kids.length - 1].y) / 2
            for (const k of kids)
              newArrows.push({ x1: x + layout.nodeW, y1: centerY, x2: k.x, y2: k.y })
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
          y:           centerY - layout.nodeH / 2,
          w:           layout.nodeW,
          h:           layout.nodeH,
          label:       f.title || qsTr("Folder"),
          icon:        Cpp_Misc_IconRegistry.icon("widgets", "folder", 16)
        })
        return centerY
      }

      cursorY = blockCursor + layout.vGap * 2
      const smCollapsed = layout.isCollapsed("sharedmem")
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
        smCenter = cursorY + layout.nodeH / 2
        cursorY += layout.nodeH + layout.vGap
      } else {
        smCenter = (tblTops[0].y + tblTops[tblTops.length - 1].y) / 2
      }

      //
      // Variables root at device level, centred on its tables.
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
        y:           smCenter - layout.nodeH / 2,
        w:           layout.nodeW,
        h:           layout.nodeH,
        label:       qsTr("Variables"),
        icon:        Cpp_Misc_IconRegistry.icon("editor", "shared-memory", 24),
        badge:       ""
      })

      if (!smCollapsed)
        for (const t of tblTops)
          newArrows.push({
            x1: colDev + layout.nodeW, y1: smCenter,
            x2: t.x,                   y2: t.y
          })

      blockCursor = cursorY
    }

    //
    // MQTT Publisher node: collects every dataset pill (commercial, opt-in)
    //
    if (layout.mqttPublisherEnabled() && datasetAnchors.length > 0) {
      const minY     = datasetAnchors[0].y
      const maxYAnch = datasetAnchors[datasetAnchors.length - 1].y
      const midY     = (minY + maxYAnch) / 2
      const mqttY    = midY - layout.nodeH / 2

      //
      // Sit just right of the rightmost fan-in origin so a collapsed project stays compact.
      //
      let colMqtt = 0
      for (const a of datasetAnchors)
        colMqtt = Math.max(colMqtt, a.x)

      colMqtt += layout.hGap

      newNodes.push({
        type:      "mqtt-publisher",
        sourceId:  -1,
        groupId:   -1,
        datasetId: -1,
        widgetId:  -1,
        actionId:  -1,
        x:         colMqtt,
        y:         mqttY,
        w:         layout.nodeW,
        h:         layout.nodeH,
        label:     qsTr("MQTT Publisher"),
        icon:      Cpp_Misc_IconRegistry.icon("editor", "mqtt-publisher", 16),
        badge:     ""
      })

      for (const a of datasetAnchors) {
        newArrows.push({
          x1: a.x,     y1: a.y,
          x2: colMqtt, y2: mqttY + layout.nodeH / 2
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
      const totalW = maxX + layout.pad
      for (const n of newNodes)
        n.x = totalW - n.x - n.w

      for (const a of newArrows) {
        a.x1 = totalW - a.x1
        a.x2 = totalW - a.x2
      }
    }

    //
    // Stamp each node with its stable key so the view can pin/highlight it.
    //
    for (const n of newNodes)
      n.key = layout.nodeKey(n)

    return {
      "nodes":    newNodes,
      "arrows":   newArrows,
      "contentW": maxX + layout.pad,
      "contentH": maxY + layout.pad
    }
  }
}
