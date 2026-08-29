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
// Shared state for the diagram's per-context menus: a diagram node becomes an
// editor kind/id target plus the registry surface that renders its menu.
//
QtObject {
  id: menuController

  //
  // Injected plumbing: `menu` is the Widgets.CommandMenu instance, `bindings`
  // the Commands.ProjectEditorMenuBindings instance that carries the target.
  //
  required property var menu
  required property var bindings

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

  function openForBackground() {
    pinnedKey = "background"
    currentType = "background"
    menuController.resetNode()
    menuController.bindings.setTarget({ "kind": ProjectEditor.KindProjectRoot, "id": -1 })
    menuController.menu.openSurface("editor-menu/project-root")
  }

  function resetNode() {
    currentWidget = ""
    currentLabel = ""
    currentTableName = ""
    currentSourceId = -1
    currentGroupId = -1
    currentDatasetId = -1
    currentWidgetId = -1
    currentActionId = -1
    currentSiblingCount = 0
  }

  function openForNode(node) {
    pinnedKey = node.key !== undefined ? node.key : ""
    currentType = node.type
    currentWidget = node.widget !== undefined ? node.widget : ""
    currentLabel = node.label !== undefined ? node.label : ""
    currentTableName = node.tableName !== undefined ? node.tableName : ""
    currentSourceId = node.sourceId !== undefined ? node.sourceId : -1
    currentGroupId = node.groupId !== undefined ? node.groupId : -1
    currentDatasetId = node.datasetId !== undefined ? node.datasetId : -1
    currentWidgetId = node.widgetId !== undefined ? node.widgetId : -1
    currentActionId = node.actionId !== undefined ? node.actionId : -1
    currentSiblingCount = node.siblingCount !== undefined ? node.siblingCount : 0

    const surface = menuController.surfaceFor(node.type)
    if (surface.length === 0) {
      pinnedKey = ""
      return
    }

    menuController.bindings.setTarget(menuController.targetFor(node))
    menuController.menu.openSurface("editor-menu/" + surface)
  }

  //
  // Diagram node type -> menu surface; an unmapped type opens nothing.
  //
  function surfaceFor(type) {
    const surfaces = {
      "source": "source",
      "frameparser": "frame-parser",
      "group": "group",
      "dataset": "dataset",
      "output": "output",
      "output-panel": "output-panel",
      "action": "action",
      "table": "user-table",
      "transform": "transform",
      "controlscript": "control-script",
      "mqtt-publisher": "mqtt-publisher"
    }

    const surface = surfaces[type]
    return surface !== undefined ? surface : ""
  }

  //
  // Diagram node -> menu target, in the tree's own vocabulary of kinds and ids.
  //
  function targetFor(node) {
    const type = node.type
    const target = { "kind": ProjectEditor.KindNone, "id": -1, "parentId": -1,
                     "sourceId": currentSourceId, "path": currentTableName,
                     "widget": currentWidget, "siblingCount": currentSiblingCount }

    if (type === "source" || type === "frameparser") {
      target.kind = type === "source" ? ProjectEditor.KindSource
                                      : ProjectEditor.KindFrameParser
      target.id = currentSourceId
    } else if (type === "group" || type === "output-panel") {
      target.kind = ProjectEditor.KindGroup
      target.id = currentGroupId
      target.widget = type === "output-panel" ? "output-panel" : currentWidget
    } else if (type === "dataset" || type === "transform") {
      target.kind = ProjectEditor.KindDataset
      target.id = currentDatasetId
      target.parentId = currentGroupId
    } else if (type === "output") {
      target.kind = ProjectEditor.KindOutputWidget
      target.id = currentWidgetId
      target.parentId = currentGroupId
    } else if (type === "action") {
      target.kind = ProjectEditor.KindAction
      target.id = currentActionId
    } else if (type === "table") {
      target.kind = ProjectEditor.KindUserTable
    } else if (type === "controlscript") {
      target.kind = ProjectEditor.KindControlScript
    } else if (type === "mqtt-publisher") {
      target.kind = ProjectEditor.KindMqttPublisher
    }

    return target
  }

  function unpin() { pinnedKey = "" }
}
