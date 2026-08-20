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
// Behavior for the Project Editor context menus, and the target holder: the right-clicked row
// lives here as plain properties so enabled/visible/checked stay ordinary bindings.
//
QtObject {
  id: root

  //
  // The right-clicked item. Hosts fill these through setTarget() before opening a menu.
  //
  property int targetId: -1
  property string targetPath: ""
  property int targetParentId: -1
  property int targetSourceId: -1
  property string targetWidget: ""
  property bool targetEnabled: true
  property int targetSiblingCount: 0
  property int targetKind: ProjectEditor.KindNone

  //
  // Tree hosts drive bulk behavior from the live selection; the diagram acts on one block and
  // wraps its mutations so a right-click never yanks the current view.
  //
  property int selectionCount: 1
  property int deletableCount: 0
  property int selectableCount: 0
  property int enableableCount: 0
  property bool treeSelection: false
  property bool suppressViewChange: false

  //
  // Raised for the entries whose dialog lives inside a view; hosts that own one connect, the
  // rest fall back to the navigation the run() already did.
  //
  signal painterCodeRequested(int groupId)

  //
  // Command id -> behavior entry, consumed by CommandModel.join().
  //
  readonly property var map: ({
    "editor.menu.addSource": root.cmdAddSource,
    "editor.menu.addAction": root.cmdAddAction,
    "editor.menu.addTable": root.cmdAddTable,
    "editor.menu.addRegister": root.cmdAddRegister,
    "editor.menu.importTable": root.cmdImportTable,
    "editor.menu.exportTable": root.cmdExportTable,
    "editor.menu.addWorkspace": root.cmdAddWorkspace,
    "editor.menu.addFolder": root.cmdAddFolder,
    "editor.menu.addSubFolder": root.cmdAddSubFolder,
    "editor.menu.addGroup.generic": root.cmdAddGroupGeneric,
    "editor.menu.addGroup.dataGrid": root.cmdAddGroupDataGrid,
    "editor.menu.addGroup.barPanel": root.cmdAddGroupBarPanel,
    "editor.menu.addGroup.multiPlot": root.cmdAddGroupMultiPlot,
    "editor.menu.addGroup.plot3D": root.cmdAddGroupPlot3D,
    "editor.menu.addGroup.accelerometer": root.cmdAddGroupAccelerometer,
    "editor.menu.addGroup.gyroscope": root.cmdAddGroupGyroscope,
    "editor.menu.addGroup.gps": root.cmdAddGroupGps,
    "editor.menu.addGroup.imageView": root.cmdAddGroupImageView,
    "editor.menu.addGroup.webView": root.cmdAddGroupWebView,
    "editor.menu.addGroup.painter": root.cmdAddGroupPainter,
    "editor.menu.addDataset.generic": root.cmdAddDatasetGeneric,
    "editor.menu.addDataset.plot": root.cmdAddDatasetPlot,
    "editor.menu.addDataset.fft": root.cmdAddDatasetFFT,
    "editor.menu.addDataset.gauge": root.cmdAddDatasetGauge,
    "editor.menu.addDataset.bar": root.cmdAddDatasetBar,
    "editor.menu.addDataset.compass": root.cmdAddDatasetCompass,
    "editor.menu.addDataset.meter": root.cmdAddDatasetMeter,
    "editor.menu.addDataset.led": root.cmdAddDatasetLED,
    "editor.menu.addOutput.panel": root.cmdAddOutputPanel,
    "editor.menu.addOutput.slider": root.cmdAddOutputSlider,
    "editor.menu.addOutput.toggle": root.cmdAddOutputToggle,
    "editor.menu.addOutput.knob": root.cmdAddOutputKnob,
    "editor.menu.addOutput.textField": root.cmdAddOutputTextField,
    "editor.menu.addOutput.button": root.cmdAddOutputButton,
    "editor.menu.rename": root.cmdRename,
    "editor.menu.toggleEnabled": root.cmdToggleEnabled,
    "editor.menu.duplicate": root.cmdDuplicate,
    "editor.menu.delete": root.cmdDelete,
    "editor.menu.moveUp": root.cmdMoveUp,
    "editor.menu.moveDown": root.cmdMoveDown,
    "editor.menu.moveToTopLevel": root.cmdMoveToTopLevel,
    "editor.menu.editFrameParser": root.cmdEditFrameParser,
    "editor.menu.editTransform": root.cmdEditTransform,
    "editor.menu.editPainterCode": root.cmdEditPainterCode,
    "editor.menu.editControlLoop": root.cmdEditControlLoop,
    "editor.menu.configureMqtt": root.cmdConfigureMqtt,
    "editor.menu.seedAliases": root.cmdSeedAliases,
    "editor.menu.showHiddenGroups": root.cmdShowHiddenGroups,
    "editor.menu.sharedMemoryHelp": root.cmdSharedMemoryHelp,
    "editor.menu.editWorkspace": root.cmdEditWorkspace,
    "editor.menu.customizeWorkspaces": root.cmdCustomizeWorkspaces,
    "editor.menu.cleanupWorkspaces": root.cmdCleanupWorkspaces,
    "editor.menu.resetWorkspaces": root.cmdResetWorkspaces,
    "editor.menu.clearWorkspaces": root.cmdClearWorkspaces,
    "editor.menu.dataset.plot": root.cmdDatasetPlot,
    "editor.menu.dataset.fft": root.cmdDatasetFFT,
    "editor.menu.dataset.waterfall": root.cmdDatasetWaterfall,
    "editor.menu.dataset.bar": root.cmdDatasetBar,
    "editor.menu.dataset.gauge": root.cmdDatasetGauge,
    "editor.menu.dataset.compass": root.cmdDatasetCompass,
    "editor.menu.dataset.meter": root.cmdDatasetMeter,
    "editor.menu.dataset.led": root.cmdDatasetLED,
    "editor.expandTree": root.cmdExpandTree,
    "editor.collapseTree": root.cmdCollapseTree
  })

  //
  // Kind predicates, shared by the visibility expressions below.
  //
  readonly property bool isFolder: targetKind === ProjectEditor.KindGroupFolder
                                   || targetKind === ProjectEditor.KindTableFolder
                                   || targetKind === ProjectEditor.KindWorkspaceFolder

  readonly property bool supportsFolders: isFolder
                                          || targetKind === ProjectEditor.KindGroup
                                          || targetKind === ProjectEditor.KindWorkspace
                                          || targetKind === ProjectEditor.KindUserTable

  readonly property bool orderable: targetKind === ProjectEditor.KindGroup
                                    || targetKind === ProjectEditor.KindDataset
                                    || targetKind === ProjectEditor.KindAction
                                    || targetKind === ProjectEditor.KindOutputWidget
                                    || targetKind === ProjectEditor.KindWorkspace
                                    || isFolder

  readonly property bool renameable: isFolder
                                     || targetKind === ProjectEditor.KindGroup
                                     || targetKind === ProjectEditor.KindDataset
                                     || targetKind === ProjectEditor.KindAction
                                     || targetKind === ProjectEditor.KindSource
                                     || targetKind === ProjectEditor.KindWorkspace
                                     || targetKind === ProjectEditor.KindUserTable

  readonly property bool singleTarget: root.selectionCount <= 1

  //
  // The visualization toggles read the selected dataset's options, which only the tree keeps
  // in step with the right-clicked row.
  //
  readonly property bool datasetVisualsVisible: root.treeSelection
                                                && root.targetKind === ProjectEditor.KindDataset

  //
  // Folder family the target belongs to; drives every folder entry so one command serves
  // groups, tables and workspaces alike.
  //
  readonly property string folderSection: {
    if (targetKind === ProjectEditor.KindGroup
        || targetKind === ProjectEditor.KindGroupFolder
        || targetKind === ProjectEditor.KindGroupsRoot)
      return "group"

    if (targetKind === ProjectEditor.KindUserTable
        || targetKind === ProjectEditor.KindTableFolder
        || targetKind === ProjectEditor.KindTablesRoot)
      return "table"

    if (targetKind === ProjectEditor.KindWorkspace
        || targetKind === ProjectEditor.KindWorkspaceFolder
        || targetKind === ProjectEditor.KindWorkspacesRoot)
      return "workspace"

    return ""
  }

  //
  // Assigns the right-clicked item; missing keys fall back to "not applicable".
  //
  function setTarget(target) {
    root.targetKind = target.kind !== undefined ? target.kind : ProjectEditor.KindNone
    root.targetId = target.id !== undefined ? target.id : -1
    root.targetParentId = target.parentId !== undefined ? target.parentId : -1
    root.targetSourceId = target.sourceId !== undefined ? target.sourceId : -1
    root.targetPath = target.path !== undefined ? target.path : ""
    root.targetWidget = target.widget !== undefined ? target.widget : ""
    root.targetEnabled = target.enabled !== undefined ? target.enabled : true
    root.targetSiblingCount = target.siblingCount !== undefined ? target.siblingCount : 0
    root.selectionCount = target.selectionCount !== undefined ? target.selectionCount : 1
  }

  //
  // Drops the target once a menu closes; the ids go stale on the next model rebuild.
  //
  function clearTarget() {
    root.targetKind = ProjectEditor.KindNone
    root.targetId = -1
    root.targetParentId = -1
    root.targetSourceId = -1
    root.targetPath = ""
    root.targetWidget = ""
    root.targetEnabled = true
    root.targetSiblingCount = 0
  }

  //
  // Runs a mutation, optionally holding the current view in place (the diagram's behavior).
  //
  function apply(action) {
    if (!root.suppressViewChange) {
      action()
      return
    }

    Cpp_JSON_ProjectEditor.setSuppressViewChange(true)
    try {
      action()
    } finally {
      Qt.callLater(() => Cpp_JSON_ProjectEditor.setSuppressViewChange(false))
    }
  }

  //
  // The tree selection filtered to the kinds an operation accepts; other hosts act on the
  // single target instead.
  //
  function selectedOfKinds(kinds) {
    if (!root.treeSelection)
      return []

    return Cpp_JSON_ProjectEditor.selectedTreeItems().filter(it => kinds.indexOf(it.kind) >= 0)
  }

  //
  // Owning source id for a new object; -1 defers to the active source.
  //
  function targetSource() {
    if (targetKind === ProjectEditor.KindSource
        || targetKind === ProjectEditor.KindFrameParser)
      return root.targetId

    return root.targetSourceId
  }

  //
  // Folder the new object is filed into; -1 when the target is not a folder.
  //
  function targetFolder() {
    return root.isFolder ? root.targetId : -1
  }

  //
  // Selects the group a dataset or output widget is being added to, so the model's
  // "current group" is the one the user right-clicked.
  //
  function selectTargetGroup() {
    if (targetKind === ProjectEditor.KindGroup && root.targetId >= 0)
      Cpp_JSON_ProjectEditor.selectGroup(root.targetId)
  }

  //
  // Adds one group template, filed into the target's source and folder.
  //
  function addGroup(title, widget) {
    root.apply(() => Cpp_JSON_ProjectModel.addGroup(title, widget, root.targetSource(),
                                                    root.targetFolder()))
  }

  //
  // Adds one dataset of `option` to the group the menu was opened from.
  //
  function addDataset(option) {
    root.apply(() => {
                 root.selectTargetGroup()
                 Cpp_JSON_ProjectModel.addDataset(option, root.targetSource())
               })
  }

  //
  // Adds one output control to the group the menu was opened from.
  //
  function addOutput(type) {
    root.apply(() => {
                 root.selectTargetGroup()
                 Cpp_JSON_ProjectModel.addOutputControl(type, root.targetSource())
               })
  }

  //
  // True when `option` is already on for the selected dataset.
  //
  function datasetOptionOn(option) {
    return (Cpp_JSON_ProjectEditor.datasetOptions & option) !== 0
  }

  //
  // Flips one visualization on the dataset the menu was opened from.
  //
  function toggleDatasetOption(option) {
    root.apply(() => {
                 Cpp_JSON_ProjectEditor.selectDataset(root.targetParentId, root.targetId)
                 Cpp_JSON_ProjectModel.changeDatasetOption(option, !root.datasetOptionOn(option))
               })
  }

  //
  // Creation: sources, actions, tables, workspaces and folders.
  //
  readonly property QtObject cmdAddSource: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.apply(() => Cpp_JSON_ProjectModel.addSource()) }
  }

  readonly property QtObject cmdAddAction: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.apply(() => Cpp_JSON_ProjectModel.addAction(root.targetSource())) }
  }

  readonly property QtObject cmdAddTable: QtObject {
    readonly property bool visible: root.singleTarget
    function run() {
      if (root.targetKind === ProjectEditor.KindTableFolder)
        Cpp_JSON_ProjectModel.promptAddTableInFolder(root.targetId)
      else
        Cpp_JSON_ProjectModel.promptAddTable()
    }
  }

  readonly property QtObject cmdAddRegister: QtObject {
    readonly property bool visible: root.singleTarget
                                    && root.targetKind === ProjectEditor.KindUserTable
    function run() { Cpp_JSON_ProjectModel.promptAddRegister(root.targetPath) }
  }

  readonly property QtObject cmdImportTable: QtObject {
    readonly property bool visible: root.singleTarget
                                    && root.targetKind === ProjectEditor.KindUserTable
    function run() { Cpp_JSON_ProjectModel.importTableFromCsv(root.targetPath) }
  }

  readonly property QtObject cmdExportTable: QtObject {
    readonly property bool visible: root.singleTarget
                                    && root.targetKind === ProjectEditor.KindUserTable
    function run() { Cpp_JSON_ProjectModel.exportTableToCsv(root.targetPath) }
  }

  readonly property QtObject cmdAddWorkspace: QtObject {
    readonly property bool visible: root.singleTarget
    function run() {
      if (root.targetKind === ProjectEditor.KindWorkspaceFolder)
        Cpp_JSON_ProjectModel.promptAddWorkspaceInFolder(root.targetId)
      else
        Cpp_JSON_ProjectModel.promptAddWorkspace()
    }
  }

  readonly property QtObject cmdAddFolder: QtObject {
    readonly property bool visible: root.singleTarget && root.folderSection !== ""
    function run() {
      if (root.folderSection === "group")
        Cpp_JSON_ProjectModel.promptAddGroupFolder(-1)
      else if (root.folderSection === "table")
        Cpp_JSON_ProjectModel.promptAddTableFolder(-1)
      else
        Cpp_JSON_ProjectModel.promptAddWorkspaceFolder(-1)
    }
  }

  readonly property QtObject cmdAddSubFolder: QtObject {
    readonly property bool visible: root.singleTarget && root.isFolder
    function run() {
      if (root.targetKind === ProjectEditor.KindGroupFolder)
        Cpp_JSON_ProjectModel.promptAddGroupFolder(root.targetId)
      else if (root.targetKind === ProjectEditor.KindTableFolder)
        Cpp_JSON_ProjectModel.promptAddTableFolder(root.targetId)
      else
        Cpp_JSON_ProjectModel.promptAddWorkspaceFolder(root.targetId)
    }
  }

  //
  // Creation: group templates.
  //
  readonly property QtObject cmdAddGroupGeneric: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addGroup(qsTr("Dataset Container"), SerialStudio.NoGroupWidget) }
  }

  readonly property QtObject cmdAddGroupDataGrid: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addGroup(qsTr("Data Grid"), SerialStudio.DataGrid) }
  }

  readonly property QtObject cmdAddGroupBarPanel: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addGroup(qsTr("Bar Panel"), SerialStudio.BarPanel) }
  }

  readonly property QtObject cmdAddGroupMultiPlot: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addGroup(qsTr("Multi-Plot"), SerialStudio.MultiPlot) }
  }

  readonly property QtObject cmdAddGroupPlot3D: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addGroup(qsTr("3D Plot"), SerialStudio.Plot3D) }
  }

  readonly property QtObject cmdAddGroupAccelerometer: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addGroup(qsTr("Accelerometer"), SerialStudio.Accelerometer) }
  }

  readonly property QtObject cmdAddGroupGyroscope: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addGroup(qsTr("Gyroscope"), SerialStudio.Gyroscope) }
  }

  readonly property QtObject cmdAddGroupGps: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addGroup(qsTr("GPS Map"), SerialStudio.GPS) }
  }

  readonly property QtObject cmdAddGroupImageView: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addGroup(qsTr("Image View"), SerialStudio.ImageView) }
  }

  readonly property QtObject cmdAddGroupWebView: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addGroup(qsTr("Web View"), SerialStudio.WebView) }
  }

  readonly property QtObject cmdAddGroupPainter: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addGroup(qsTr("Canvas Widget"), SerialStudio.Painter) }
  }

  //
  // Creation: dataset types.
  //
  readonly property QtObject cmdAddDatasetGeneric: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addDataset(SerialStudio.DatasetGeneric) }
  }

  readonly property QtObject cmdAddDatasetPlot: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addDataset(SerialStudio.DatasetPlot) }
  }

  readonly property QtObject cmdAddDatasetFFT: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addDataset(SerialStudio.DatasetFFT) }
  }

  readonly property QtObject cmdAddDatasetGauge: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addDataset(SerialStudio.DatasetGauge) }
  }

  readonly property QtObject cmdAddDatasetBar: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addDataset(SerialStudio.DatasetBar) }
  }

  readonly property QtObject cmdAddDatasetCompass: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addDataset(SerialStudio.DatasetCompass) }
  }

  readonly property QtObject cmdAddDatasetMeter: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addDataset(SerialStudio.DatasetMeter) }
  }

  readonly property QtObject cmdAddDatasetLED: QtObject {
    readonly property bool visible: root.singleTarget
    function run() { root.addDataset(SerialStudio.DatasetLED) }
  }

  //
  // Creation: output controls (Pro).
  //
  readonly property QtObject cmdAddOutputPanel: QtObject {
    readonly property bool visible: root.singleTarget
    readonly property bool enabled: Cpp_CommercialBuild
    function run() {
      root.apply(() => {
                   root.selectTargetGroup()
                   Cpp_JSON_ProjectModel.addOutputPanel(root.targetSource())
                 })
    }
  }

  readonly property QtObject cmdAddOutputSlider: QtObject {
    readonly property bool visible: root.singleTarget
    readonly property bool enabled: Cpp_CommercialBuild
    function run() { root.addOutput(SerialStudio.OutputSlider) }
  }

  readonly property QtObject cmdAddOutputToggle: QtObject {
    readonly property bool visible: root.singleTarget
    readonly property bool enabled: Cpp_CommercialBuild
    function run() { root.addOutput(SerialStudio.OutputToggle) }
  }

  readonly property QtObject cmdAddOutputKnob: QtObject {
    readonly property bool visible: root.singleTarget
    readonly property bool enabled: Cpp_CommercialBuild
    function run() { root.addOutput(SerialStudio.OutputKnob) }
  }

  readonly property QtObject cmdAddOutputTextField: QtObject {
    readonly property bool visible: root.singleTarget
    readonly property bool enabled: Cpp_CommercialBuild
    function run() { root.addOutput(SerialStudio.OutputTextField) }
  }

  readonly property QtObject cmdAddOutputButton: QtObject {
    readonly property bool visible: root.singleTarget
    readonly property bool enabled: Cpp_CommercialBuild
    function run() { root.addOutput(SerialStudio.OutputButton) }
  }

  //
  // Items the current operation applies to: the filtered tree selection, or the single target
  // every other host works with.
  //
  function operationItems(kinds) {
    const selected = root.selectedOfKinds(kinds)
    if (selected.length > 0)
      return selected

    if (kinds.indexOf(root.targetKind) < 0)
      return []

    return [{ "kind": root.targetKind, "id": root.targetId,
              "parentId": root.targetParentId, "path": root.targetPath }]
  }

  //
  // Kinds each bulk operation accepts, mirroring the model's own switch statements.
  //
  readonly property var duplicableKinds: [ProjectEditor.KindGroup, ProjectEditor.KindDataset,
                                          ProjectEditor.KindAction,
                                          ProjectEditor.KindOutputWidget,
                                          ProjectEditor.KindUserTable,
                                          ProjectEditor.KindGroupFolder,
                                          ProjectEditor.KindTableFolder]

  readonly property var deletableKinds: [ProjectEditor.KindGroup, ProjectEditor.KindDataset,
                                         ProjectEditor.KindAction,
                                         ProjectEditor.KindOutputWidget,
                                         ProjectEditor.KindWorkspace,
                                         ProjectEditor.KindWorkspaceFolder,
                                         ProjectEditor.KindGroupFolder,
                                         ProjectEditor.KindUserTable,
                                         ProjectEditor.KindTableFolder]

  readonly property var enableableKinds: [ProjectEditor.KindGroup, ProjectEditor.KindDataset,
                                          ProjectEditor.KindGroupFolder]

  //
  // Same-section selection for the folder moves; the model reads folderId per section.
  //
  function folderItems() {
    if (!root.treeSelection)
      return root.operationItems([root.targetKind])

    return Cpp_JSON_ProjectEditor.selectedTreeItems()
    .filter(it => root.sectionOf(it.kind) === root.folderSection)
  }

  //
  // Folder family of an arbitrary kind, used to keep a mixed selection out of a folder move.
  //
  function sectionOf(kind) {
    if (kind === ProjectEditor.KindGroup || kind === ProjectEditor.KindGroupFolder)
      return "group"

    if (kind === ProjectEditor.KindUserTable || kind === ProjectEditor.KindTableFolder)
      return "table"

    if (kind === ProjectEditor.KindWorkspace || kind === ProjectEditor.KindWorkspaceFolder)
      return "workspace"

    return ""
  }

  //
  // Files the current selection into `folderId` (-1 = top level).
  //
  function moveToFolder(folderId) {
    const items = root.folderItems()
    if (items.length > 0)
      Cpp_JSON_ProjectModel.moveSelectedItemsToFolder(items, folderId)
  }

  //
  // Reorders the target among its siblings.
  //
  function moveBy(direction) {
    root.apply(() => {
                 if (root.targetKind === ProjectEditor.KindGroup)
                   Cpp_JSON_ProjectModel.moveGroup(root.targetId, root.targetId + direction)
                 else if (root.targetKind === ProjectEditor.KindDataset)
                   Cpp_JSON_ProjectModel.moveDataset(root.targetParentId, root.targetId,
                                                     root.targetId + direction)
                 else if (root.targetKind === ProjectEditor.KindAction)
                   Cpp_JSON_ProjectModel.moveAction(root.targetId, root.targetId + direction)
                 else if (root.targetKind === ProjectEditor.KindOutputWidget)
                   Cpp_JSON_ProjectModel.moveOutputWidget(root.targetParentId, root.targetId,
                                                          root.targetId + direction)
                 else if (root.targetKind === ProjectEditor.KindWorkspace)
                   Cpp_JSON_ProjectEditor.moveWorkspace(root.targetId, direction)
                 else if (root.targetKind === ProjectEditor.KindWorkspaceFolder)
                   Cpp_JSON_ProjectModel.moveWorkspaceFolderInParent(root.targetId, direction)
                 else if (root.targetKind === ProjectEditor.KindGroupFolder)
                   Cpp_JSON_ProjectModel.moveGroupFolderInParent(root.targetId, direction)
                 else if (root.targetKind === ProjectEditor.KindTableFolder)
                   Cpp_JSON_ProjectModel.moveTableFolderInParent(root.targetId, direction)
               })
  }

  //
  // Deletes one item through the confirming per-kind slot the diagram menus have always used.
  //
  function deleteSingleTarget() {
    const kind = root.targetKind
    if (kind === ProjectEditor.KindGroup)
      Cpp_JSON_ProjectModel.deleteGroup(root.targetId, true)
    else if (kind === ProjectEditor.KindDataset)
      Cpp_JSON_ProjectModel.deleteDataset(root.targetParentId, root.targetId, true)
    else if (kind === ProjectEditor.KindAction)
      Cpp_JSON_ProjectModel.deleteAction(root.targetId, true)
    else if (kind === ProjectEditor.KindOutputWidget)
      Cpp_JSON_ProjectModel.deleteOutputWidget(root.targetParentId, root.targetId, true)
    else if (kind === ProjectEditor.KindUserTable)
      Cpp_JSON_ProjectModel.confirmDeleteTable(root.targetPath)
  }

  //
  // Deletes the single tree row through the confirmation its kind carries.
  //
  function deleteSingleTreeRow() {
    const kind = root.targetKind
    if (kind === ProjectEditor.KindWorkspace)
      Cpp_JSON_ProjectModel.confirmDeleteWorkspace(root.targetId)
    else if (kind === ProjectEditor.KindWorkspaceFolder)
      Cpp_JSON_ProjectModel.confirmDeleteWorkspaceFolder(root.targetId)
    else if (kind === ProjectEditor.KindGroupFolder)
      Cpp_JSON_ProjectModel.confirmDeleteGroupFolder(root.targetId)
    else if (kind === ProjectEditor.KindTableFolder)
      Cpp_JSON_ProjectModel.confirmDeleteTableFolder(root.targetId)
    else if (kind === ProjectEditor.KindUserTable)
      Cpp_JSON_ProjectModel.confirmDeleteTable(root.targetPath)
    else
      Cpp_JSON_ProjectModel.confirmDeleteSelectedItems(root.operationItems(root.deletableKinds))
  }

  //
  // Item operations.
  //
  readonly property QtObject cmdRename: QtObject {
    readonly property bool visible: root.singleTarget && root.renameable
    function run() {
      const kind = root.targetKind
      if (kind === ProjectEditor.KindGroup)
        Cpp_JSON_ProjectModel.promptRenameGroup(root.targetId)
      else if (kind === ProjectEditor.KindDataset)
        Cpp_JSON_ProjectModel.promptRenameDataset(root.targetParentId, root.targetId)
      else if (kind === ProjectEditor.KindAction)
        Cpp_JSON_ProjectModel.promptRenameAction(root.targetId)
      else if (kind === ProjectEditor.KindSource)
        Cpp_JSON_ProjectModel.promptRenameSource(root.targetId)
      else if (kind === ProjectEditor.KindWorkspace)
        Cpp_JSON_ProjectModel.promptRenameWorkspace(root.targetId)
      else if (kind === ProjectEditor.KindUserTable)
        Cpp_JSON_ProjectModel.promptRenameTable(root.targetPath)
      else if (kind === ProjectEditor.KindGroupFolder)
        Cpp_JSON_ProjectModel.promptRenameGroupFolder(root.targetId)
      else if (kind === ProjectEditor.KindTableFolder)
        Cpp_JSON_ProjectModel.promptRenameTableFolder(root.targetId)
      else if (kind === ProjectEditor.KindWorkspaceFolder)
        Cpp_JSON_ProjectModel.promptRenameWorkspaceFolder(root.targetId)
    }
  }

  readonly property QtObject cmdToggleEnabled: QtObject {
    readonly property bool checked: !root.targetEnabled
    readonly property bool visible: root.enableableCount > 0
    readonly property string title: {
      if (root.enableableCount <= 1)
        return ""

      return root.targetEnabled ? qsTr("Hide Selected (%1)").arg(root.enableableCount)
                                : qsTr("Show Selected (%1)").arg(root.enableableCount)
    }

    function run() {
      const items = root.operationItems(root.enableableKinds)
      if (items.length > 0)
        Cpp_JSON_ProjectModel.setItemsEnabled(items, !root.targetEnabled)
    }
  }

  readonly property QtObject cmdDuplicate: QtObject {
    readonly property string title: root.selectableCount > 1
                                    ? qsTr("Duplicate Selected (%1)").arg(root.selectableCount)
                                    : ""
    readonly property bool visible: root.targetKind === ProjectEditor.KindSource
                                    || root.duplicableKinds.indexOf(root.targetKind) >= 0

    function run() {
      if (root.targetKind === ProjectEditor.KindSource) {
        root.apply(() => Cpp_JSON_ProjectModel.duplicateSource(root.targetId))
        return
      }

      const items = root.operationItems(root.duplicableKinds)
      if (items.length > 0)
        root.apply(() => Cpp_JSON_ProjectModel.duplicateSelectedItems(items))
    }
  }

  readonly property QtObject cmdDelete: QtObject {
    readonly property string title: root.deletableCount > 1
                                    ? qsTr("Delete Selected (%1)").arg(root.deletableCount)
                                    : ""
    readonly property bool visible: root.targetKind === ProjectEditor.KindSource
                                    || root.deletableKinds.indexOf(root.targetKind) >= 0

    function run() {
      if (root.targetKind === ProjectEditor.KindSource) {
        root.apply(() => Cpp_JSON_ProjectModel.deleteSource(root.targetId, true))
        return
      }

      if (!root.treeSelection) {
        root.apply(() => root.deleteSingleTarget())
        return
      }

      if (root.deletableCount > 1) {
        Cpp_JSON_ProjectModel.confirmDeleteSelectedItems(
              root.operationItems(root.deletableKinds))
        return
      }

      root.deleteSingleTreeRow()
    }
  }

  readonly property QtObject cmdMoveUp: QtObject {
    readonly property bool visible: root.singleTarget && root.orderable
    readonly property bool enabled: root.targetSiblingCount <= 0 || root.targetId > 0
    function run() { root.moveBy(-1) }
  }

  readonly property QtObject cmdMoveDown: QtObject {
    readonly property bool visible: root.singleTarget && root.orderable
    readonly property bool enabled: root.targetSiblingCount <= 0
                                    || root.targetId < root.targetSiblingCount - 1
    function run() { root.moveBy(1) }
  }

  readonly property QtObject cmdMoveToTopLevel: QtObject {
    readonly property bool visible: root.supportsFolders
    function run() { root.moveToFolder(-1) }
  }

  //
  // Navigation into the views and editors that own a surface.
  //
  readonly property QtObject cmdEditFrameParser: QtObject {
    readonly property bool visible: root.targetKind === ProjectEditor.KindSource
                                    || root.targetKind === ProjectEditor.KindFrameParser
    function run() { Cpp_JSON_ProjectEditor.selectFrameParser(root.targetSource()) }
  }

  readonly property QtObject cmdEditTransform: QtObject {
    readonly property bool visible: root.targetKind === ProjectEditor.KindDataset
    function run() {
      Cpp_JSON_ProjectEditor.openTransformEditorFor(root.targetParentId, root.targetId)
    }
  }

  readonly property QtObject cmdEditPainterCode: QtObject {
    readonly property bool enabled: Cpp_CommercialBuild
    readonly property bool visible: root.targetKind === ProjectEditor.KindGroup
                                    && root.targetWidget === "painter"
    function run() {
      const groupId = root.targetId
      root.apply(() => Cpp_JSON_ProjectEditor.selectGroup(groupId))
      root.painterCodeRequested(groupId)
    }
  }

  readonly property QtObject cmdEditControlLoop: QtObject {
    function run() { Cpp_JSON_ProjectEditor.selectControlScript() }
  }

  readonly property QtObject cmdConfigureMqtt: QtObject {
    readonly property bool enabled: Cpp_CommercialBuild
    function run() { Cpp_JSON_ProjectEditor.selectMqttPublisher() }
  }

  readonly property QtObject cmdEditWorkspace: QtObject {
    readonly property bool visible: root.targetKind === ProjectEditor.KindWorkspace
    function run() { Cpp_JSON_ProjectEditor.selectWorkspace(root.targetId) }
  }

  //
  // Project-wide entries.
  //
  readonly property QtObject cmdSeedAliases: QtObject {
    function run() { Cpp_JSON_ProjectModel.seedDatasetAliases() }
  }

  readonly property QtObject cmdShowHiddenGroups: QtObject {
    function run() { Cpp_JSON_ProjectModel.showAllHiddenGroups() }
  }

  readonly property QtObject cmdSharedMemoryHelp: QtObject {
    function run() { app.showHelpCenter("data-tables") }
  }

  readonly property QtObject cmdExpandTree: QtObject {
    function run() { Cpp_JSON_ProjectEditor.expandAllTreeItems() }
  }

  readonly property QtObject cmdCollapseTree: QtObject {
    function run() { Cpp_JSON_ProjectEditor.collapseTreeToOverview() }
  }

  //
  // Workspace section entries.
  //
  readonly property QtObject cmdCustomizeWorkspaces: QtObject {
    readonly property bool checked: Cpp_JSON_ProjectModel.customizeWorkspaces
    function run() {
      Cpp_JSON_ProjectModel.setCustomizeWorkspaces(!Cpp_JSON_ProjectModel.customizeWorkspaces)
    }
  }

  readonly property QtObject cmdCleanupWorkspaces: QtObject {
    function run() { Cpp_JSON_ProjectEditor.confirmCleanupUnresolvedWorkspaceWidgets() }
  }

  readonly property QtObject cmdResetWorkspaces: QtObject {
    function run() { Cpp_JSON_ProjectModel.confirmResetWorkspacesToAuto() }
  }

  readonly property QtObject cmdClearWorkspaces: QtObject {
    function run() { Cpp_JSON_ProjectModel.clearAllWorkspaces() }
  }

  //
  // Dataset visualizations: checkable mirrors of the dataset view's button row. Tree-only,
  // because the check state reads the selected dataset and the tree selects what you click.
  //
  readonly property QtObject cmdDatasetPlot: QtObject {
    readonly property bool visible: root.datasetVisualsVisible
    readonly property bool checked: root.datasetOptionOn(SerialStudio.DatasetPlot)
    function run() { root.toggleDatasetOption(SerialStudio.DatasetPlot) }
  }

  readonly property QtObject cmdDatasetFFT: QtObject {
    readonly property bool visible: root.datasetVisualsVisible
    readonly property bool checked: root.datasetOptionOn(SerialStudio.DatasetFFT)
    function run() { root.toggleDatasetOption(SerialStudio.DatasetFFT) }
  }

  readonly property QtObject cmdDatasetWaterfall: QtObject {
    readonly property bool visible: root.datasetVisualsVisible
    readonly property bool checked: root.datasetOptionOn(SerialStudio.DatasetWaterfall)
    function run() { root.toggleDatasetOption(SerialStudio.DatasetWaterfall) }
  }

  readonly property QtObject cmdDatasetBar: QtObject {
    readonly property bool visible: root.datasetVisualsVisible
    readonly property bool checked: root.datasetOptionOn(SerialStudio.DatasetBar)
    function run() { root.toggleDatasetOption(SerialStudio.DatasetBar) }
  }

  readonly property QtObject cmdDatasetGauge: QtObject {
    readonly property bool visible: root.datasetVisualsVisible
    readonly property bool checked: root.datasetOptionOn(SerialStudio.DatasetGauge)
    function run() { root.toggleDatasetOption(SerialStudio.DatasetGauge) }
  }

  readonly property QtObject cmdDatasetCompass: QtObject {
    readonly property bool visible: root.datasetVisualsVisible
    readonly property bool checked: root.datasetOptionOn(SerialStudio.DatasetCompass)
    function run() { root.toggleDatasetOption(SerialStudio.DatasetCompass) }
  }

  readonly property QtObject cmdDatasetMeter: QtObject {
    readonly property bool visible: root.datasetVisualsVisible
    readonly property bool checked: root.datasetOptionOn(SerialStudio.DatasetMeter)
    function run() { root.toggleDatasetOption(SerialStudio.DatasetMeter) }
  }

  readonly property QtObject cmdDatasetLED: QtObject {
    readonly property bool visible: root.datasetVisualsVisible
    readonly property bool checked: root.datasetOptionOn(SerialStudio.DatasetLED)
    function run() { root.toggleDatasetOption(SerialStudio.DatasetLED) }
  }
}
