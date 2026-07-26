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

QtObject {
  id: root

  //
  // Context configuration. A context is just a set of these refs/flags: the dashboard passes a
  // taskBar + host with workspaces on; the main window passes tools only with browse off.
  //
  property var host: null
  property var taskBar: null
  property var toolActions: null
  property bool workspacesEnabled: true

  //
  // Optional context-specific actions provider (the CommandModel items(filter) shape); its
  // entries land in the ordered category sections alongside toolActions.
  //
  property var extraTools: null

  //
  // Navigation outcomes raised to the host palette (folder drill-in, dialog close).
  //
  signal enterFolderRequested(var node)
  signal closeRequested()

  //
  // Workspace/folder tree pulled on demand; empty when workspaces are off or no taskBar exists.
  //
  function workspaceTree() {
    if (root.workspacesEnabled && root.taskBar)
      return root.taskBar.workspaceTree()

    return []
  }

  //
  // Flattens every workspace leaf, tagging each with its parent path as a subtitle.
  //
  function flattenWorkspaces(nodes, prefix, out) {
    for (let i = 0; i < nodes.length; ++i) {
      const node = nodes[i]
      const path = prefix.length > 0 ? (prefix + " / " + node.text) : node.text
      if (node.isFolder)
        root.flattenWorkspaces(node.children, path, out)
      else
        out.push({ id: node.id, text: node.text, icon: node.icon, iconId: node.iconId,
                   isFolder: false, subtitle: prefix })
    }
  }

  //
  // Flattens every folder at any depth, tagging each with its parent path; children are kept
  // intact so search results can still drill into a folder.
  //
  function flattenFolders(nodes, prefix, out) {
    for (let i = 0; i < nodes.length; ++i) {
      const node = nodes[i]
      if (!node.isFolder)
        continue

      const path = prefix.length > 0 ? (prefix + " / " + node.text) : node.text
      out.push({ id: node.id, text: node.text, icon: node.icon, iconId: node.iconId,
                 isFolder: true, children: node.children, subtitle: prefix })
      root.flattenFolders(node.children, path, out)
    }
  }

  //
  // Wraps an actions provider into palette nodes carrying their own run closure.
  //
  function actionNodes(provider, query) {
    let out = []
    if (!provider)
      return out

    const items = provider.items(query)
    for (let i = 0; i < items.length; ++i)
      out.push({ isTool: true, isFolder: false,
                 text: items[i].name, icon: items[i].icon, iconId: items[i].iconId,
                 shortcut: items[i].shortcut, category: items[i].category,
                 enabled: items[i].enabled !== false, run: items[i].run })

    return out
  }

  //
  // Palette section order and display names for command categories (manifest `category`).
  //
  readonly property var categoryOrder: ["file", "mode", "connection", "view", "export",
                                        "console", "project", "license", "tools", "help"]

  function categoryLabel(key) {
    switch (key) {
      case "file":       return qsTr("File")
      case "mode":       return qsTr("Operation Mode")
      case "connection": return qsTr("Connection")
      case "view":       return qsTr("View")
      case "export":     return qsTr("Data Export")
      case "console":    return qsTr("Console")
      case "project":    return qsTr("Project")
      case "license":    return qsTr("License")
      case "tools":      return qsTr("Tools")
      case "help":       return qsTr("Help")
    }

    return qsTr("Other")
  }

  //
  // Merges the tool/action providers and groups their entries into ordered category sections.
  //
  function categorizedSections(providers, query) {
    let nodes = []
    for (let p = 0; p < providers.length; ++p)
      nodes = nodes.concat(root.actionNodes(providers[p], query))

    let byCat = ({})
    let order = root.categoryOrder.slice()
    for (let i = 0; i < nodes.length; ++i) {
      const key = (nodes[i].category && nodes[i].category.length > 0) ? nodes[i].category : "other"
      if (byCat[key] === undefined) {
        byCat[key] = []
        if (order.indexOf(key) === -1)
          order.push(key)
      }
      byCat[key].push(nodes[i])
    }

    let secs = []
    for (let j = 0; j < order.length; ++j) {
      const items = byCat[order[j]]
      if (items !== undefined && items.length > 0)
        secs.push({ "title": root.categoryLabel(order[j]), "items": items })
    }

    return secs
  }

  //
  // Root browse shows the Workspaces category (+ Add Workspace) and Tools; sub-folders stay
  // headerless; with workspaces off only Tools is shown.
  //
  function browseSections(currentNodes, currentFolderId) {
    let secs = []

    if (root.workspacesEnabled) {
      const addCell = {
        "id": -1,
        "isAdd": true,
        "isFolder": false,
        "folderId": currentFolderId,
        "text": qsTr("Add Workspace"),
        "icon": "qrc:/icons/buttons/add-workspace.svg"
      }
      const title = currentFolderId === -1 ? qsTr("Workspaces") : ""
      secs.push({ "title": title, "items": currentNodes.concat([addCell]) })
    }

    if (!root.workspacesEnabled || currentFolderId === -1) {
      const toolSecs = root.categorizedSections([root.extraTools, root.toolActions], "")
      for (let i = 0; i < toolSecs.length; ++i)
        secs.push(toolSecs[i])
    }

    return secs
  }

  //
  // Builds the categorized search sections for a query: Folders / Workspaces / Groups / Widgets /
  // Tools, each hidden when empty. Groups and datasets both surface as widget-type entries.
  //
  function searchSections(query) {
    const q = (query || "").trim().toLowerCase()
    let secs = []
    if (q.length === 0)
      return secs

    if (root.taskBar)
      root.taskBar.searchFilter = (query || "").trim()

    if (root.workspacesEnabled) {
      const tree = root.workspaceTree()

      let ws = []
      root.flattenWorkspaces(tree, "", ws)
      ws = ws.filter(n => SerialStudio.searchMatches(q, n.text))

      let folders = []
      root.flattenFolders(tree, "", folders)
      folders = folders.filter(n => SerialStudio.searchMatches(q, n.text))

      let groups = []
      let widgets = []
      const wr = root.taskBar ? root.taskBar.searchResults : []
      for (let i = 0; i < wr.length; ++i) {
        const w = wr[i]
        const node = {
          "id": w.windowId,
          "isWidget": true,
          "isFolder": false,
          "text": w.widgetName,
          "icon": w.widgetIcon,
          "groupId": w.groupId,
          "subtitle": w.groupName
        }
        if (w.isGroupWidget)
          groups.push(node)
        else
          widgets.push(node)
      }

      if (folders.length > 0)
        secs.push({ "title": qsTr("Folders"), "items": folders })

      if (ws.length > 0)
        secs.push({ "title": qsTr("Workspaces"), "items": ws })

      if (groups.length > 0)
        secs.push({ "title": qsTr("Groups"), "items": groups })

      if (widgets.length > 0)
        secs.push({ "title": qsTr("Widgets"), "items": widgets })
    }

    const toolSecs = root.categorizedSections([root.extraTools, root.toolActions], query)
    for (let i = 0; i < toolSecs.length; ++i)
      secs.push(toolSecs[i])

    return secs
  }

  //
  // Clears the C++ search filter on palette dismissal so other consumers never read stale results.
  //
  function dismiss() {
    if (root.taskBar)
      root.taskBar.searchFilter = ""
  }

  //
  // Activation policy: a member widget reveals inside its workspace, an orphan widget opens a
  // single-widget window, and no group-level pseudo-workspace is ever activated (R6).
  //
  function activate(node) {
    if (!node)
      return

    if (node.isAdd) {
      const folderId = node.folderId !== undefined ? node.folderId : -1
      Cpp_JSON_ProjectModel.promptAddWorkspaceInFolder(folderId)
      return
    }

    if (node.isTool) {
      if (node.enabled === false)
        return

      if (typeof node.run === "function")
        node.run()

      root.closeRequested()
      return
    }

    if (node.isWidget) {
      if (root.taskBar) {
        const containing = root.taskBar.workspaceContainingWidget(node.id)
        if (containing >= 1000) {
          root.taskBar.selectWorkspaceById(containing)
          root.taskBar.navigateToWidget(node.id, node.groupId, false)
        } else if (root.host && typeof root.host.openWidgetWindow === "function") {
          root.host.openWidgetWindow(node.id)
        }
      }

      root.closeRequested()
      return
    }

    if (node.isFolder) {
      root.enterFolderRequested(node)
      return
    }

    if (root.taskBar)
      root.taskBar.selectWorkspaceById(node.id)

    root.closeRequested()
  }
}
