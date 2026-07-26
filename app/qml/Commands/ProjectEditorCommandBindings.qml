/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
  // Host references supplied by the Project Editor window; entries stay
  // hidden until each ref is assigned.
  //
  property var pePalette: null
  property var editorWindow: null

  //
  // Command id -> behavior entry, consumed by CommandModel.join().
  //
  readonly property var map: ({
    "editor.new": root.cmdEditorNew,
    "editor.open": root.cmdEditorOpen,
    "editor.save": root.cmdEditorSave,
    "editor.saveAs": root.cmdEditorSaveAs,
    "editor.undo": root.cmdEditorUndo,
    "editor.redo": root.cmdEditorRedo,
    "editor.importProto": root.cmdEditorImportProto,
    "editor.restoreBackup": root.cmdEditorRestoreBackup,
    "editor.lock": root.cmdEditorLock,
    "editor.addDevice": root.cmdEditorAddDevice,
    "editor.addGroup": root.cmdEditorAddGroup,
    "editor.addImageView": root.cmdEditorAddImageView,
    "editor.addWebView": root.cmdEditorAddWebView,
    "editor.addPainter": root.cmdEditorAddPainter,
    "editor.addDataGrid": root.cmdEditorAddDataGrid,
    "editor.addMultiPlot": root.cmdEditorAddMultiPlot,
    "editor.addPlot3D": root.cmdEditorAddPlot3D,
    "editor.addAccelerometer": root.cmdEditorAddAccelerometer,
    "editor.addGyroscope": root.cmdEditorAddGyroscope,
    "editor.addGPS": root.cmdEditorAddGPS,
    "editor.addDataset": root.cmdEditorAddDataset,
    "editor.addPlot": root.cmdEditorAddPlot,
    "editor.addFFT": root.cmdEditorAddFFT,
    "editor.addGauge": root.cmdEditorAddGauge,
    "editor.addBar": root.cmdEditorAddBar,
    "editor.addCompass": root.cmdEditorAddCompass,
    "editor.addLED": root.cmdEditorAddLED,
    "editor.addAction": root.cmdEditorAddAction,
    "editor.addOutputPanel": root.cmdEditorAddOutputPanel,
    "editor.addOutputSlider": root.cmdEditorAddOutputSlider,
    "editor.addOutputToggle": root.cmdEditorAddOutputToggle,
    "editor.addOutputKnob": root.cmdEditorAddOutputKnob,
    "editor.addOutputTextField": root.cmdEditorAddOutputTextField,
    "editor.addOutputButton": root.cmdEditorAddOutputButton,
    "editor.helpCenter": root.cmdEditorHelpCenter,
    "editor.navigateBack": root.cmdEditorNavigateBack,
    "editor.navigateForward": root.cmdEditorNavigateForward,
    "editor.expandTree": root.cmdEditorExpandTree,
    "editor.collapseTree": root.cmdEditorCollapseTree,
    "app.ai": root.cmdAppAi,
    "app.helpCenter": root.cmdAppHelpCenter,
    "app.problems": root.cmdAppProblems,
    "app.connectionDiagnostics": root.cmdAppConnectionDiagnostics,
    "app.preferences": root.cmdAppPreferences,
    "palette.open": root.cmdPaletteOpen,
    "app.quit": root.cmdAppQuit
  })

  //
  // File actions (ProjectEditorActions.qml); save/saveAs add the
  // ProjectToolbar enabled + dynamic-tooltip pair.
  //
  readonly property QtObject cmdEditorNew: QtObject {
    function run() { Cpp_JSON_ProjectModel.newJsonFile() }
  }

  readonly property QtObject cmdEditorOpen: QtObject {
    function run() { Cpp_JSON_ProjectModel.openJsonFile() }
  }

  readonly property QtObject cmdEditorSave: QtObject {
    readonly property bool enabled: Cpp_JSON_ProjectModel.modified
        && Cpp_JSON_ProjectModel.canSave
    readonly property string tooltip: Cpp_JSON_ProjectModel.canSave
        ? qsTr("Save the current project") : Cpp_JSON_ProjectModel.saveBlockerTitle
    function run() { Cpp_JSON_ProjectModel.saveJsonFile(false) }
  }

  readonly property QtObject cmdEditorSaveAs: QtObject {
    readonly property bool enabled: Cpp_JSON_ProjectModel.canSave
    readonly property string tooltip: Cpp_JSON_ProjectModel.canSave
        ? qsTr("Save the current project under a new name")
        : Cpp_JSON_ProjectModel.saveBlockerTitle
    function run() { Cpp_JSON_ProjectModel.saveJsonFile(true) }
  }

  readonly property QtObject cmdEditorUndo: QtObject {
    readonly property bool enabled: Cpp_JSON_ProjectModel.canUndo
    readonly property string tooltip: Cpp_JSON_ProjectModel.canUndo
        ? qsTr("Undo: %1").arg(Cpp_JSON_ProjectModel.undoText)
        : qsTr("Nothing to undo")
    function run() { Cpp_JSON_ProjectModel.undo() }
  }

  readonly property QtObject cmdEditorRedo: QtObject {
    readonly property bool enabled: Cpp_JSON_ProjectModel.canRedo
    readonly property string tooltip: Cpp_JSON_ProjectModel.canRedo
        ? qsTr("Redo: %1").arg(Cpp_JSON_ProjectModel.redoText)
        : qsTr("Nothing to redo")
    function run() { Cpp_JSON_ProjectModel.redo() }
  }

  readonly property QtObject cmdEditorImportProto: QtObject {
    function run() { Cpp_JSON_ProtoImporter.importProto() }
  }

  readonly property QtObject cmdEditorRestoreBackup: QtObject {
    readonly property bool visible: root.editorWindow !== null
    function run() { root.editorWindow.showBackupRecovery() }
  }

  readonly property QtObject cmdEditorLock: QtObject {
    function run() { Cpp_JSON_ProjectModel.lockProject() }
  }

  readonly property QtObject cmdEditorAddDevice: QtObject {
    readonly property bool visible: Cpp_CommercialBuild
    function run() { Cpp_JSON_ProjectModel.addSource() }
  }

  //
  // Group/dataset/output-control factories (ProjectEditorActions.qml);
  // default names stay localized via qsTr, matching the toolbar buttons.
  //
  readonly property QtObject cmdEditorAddGroup: QtObject {
    function run() {
      Cpp_JSON_ProjectModel.addGroup(qsTr("Dataset Container"), SerialStudio.NoGroupWidget)
    }
  }

  readonly property QtObject cmdEditorAddImageView: QtObject {
    function run() { Cpp_JSON_ProjectModel.addGroup(qsTr("Image View"), SerialStudio.ImageView) }
  }

  readonly property QtObject cmdEditorAddWebView: QtObject {
    function run() { Cpp_JSON_ProjectModel.addGroup(qsTr("Web View"), SerialStudio.WebView) }
  }

  readonly property QtObject cmdEditorAddPainter: QtObject {
    readonly property bool hasPro: Cpp_CommercialBuild
        && (Cpp_Licensing_LemonSqueezy.isActivated || Cpp_Licensing_Trial.trialEnabled)
    readonly property string tooltip: hasPro
        ? qsTr("Add a custom JavaScript-rendered painter widget")
        : qsTr("Painter widgets require a Pro license, adding one will fall back to a data grid")
    function run() { Cpp_JSON_ProjectModel.addGroup(qsTr("Painter Widget"), SerialStudio.Painter) }
  }

  readonly property QtObject cmdEditorAddDataGrid: QtObject {
    function run() { Cpp_JSON_ProjectModel.addGroup(qsTr("Data Grid"), SerialStudio.DataGrid) }
  }

  readonly property QtObject cmdEditorAddMultiPlot: QtObject {
    function run() {
      Cpp_JSON_ProjectModel.addGroup(qsTr("Multiple Plot"), SerialStudio.MultiPlot)
    }
  }

  readonly property QtObject cmdEditorAddPlot3D: QtObject {
    function run() { Cpp_JSON_ProjectModel.addGroup(qsTr("3D Plot"), SerialStudio.Plot3D) }
  }

  readonly property QtObject cmdEditorAddAccelerometer: QtObject {
    function run() {
      Cpp_JSON_ProjectModel.addGroup(qsTr("Accelerometer"), SerialStudio.Accelerometer)
    }
  }

  readonly property QtObject cmdEditorAddGyroscope: QtObject {
    function run() { Cpp_JSON_ProjectModel.addGroup(qsTr("Gyroscope"), SerialStudio.Gyroscope) }
  }

  readonly property QtObject cmdEditorAddGPS: QtObject {
    function run() { Cpp_JSON_ProjectModel.addGroup(qsTr("GPS Map"), SerialStudio.GPS) }
  }

  readonly property QtObject cmdEditorAddDataset: QtObject {
    function run() { Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetGeneric) }
  }

  readonly property QtObject cmdEditorAddPlot: QtObject {
    function run() { Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetPlot) }
  }

  readonly property QtObject cmdEditorAddFFT: QtObject {
    function run() { Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetFFT) }
  }

  readonly property QtObject cmdEditorAddGauge: QtObject {
    function run() { Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetGauge) }
  }

  readonly property QtObject cmdEditorAddBar: QtObject {
    function run() { Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetBar) }
  }

  readonly property QtObject cmdEditorAddCompass: QtObject {
    function run() { Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetCompass) }
  }

  readonly property QtObject cmdEditorAddLED: QtObject {
    function run() { Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetLED) }
  }

  readonly property QtObject cmdEditorAddAction: QtObject {
    function run() { Cpp_JSON_ProjectModel.addAction() }
  }

  readonly property QtObject cmdEditorAddOutputPanel: QtObject {
    function run() { Cpp_JSON_ProjectModel.addOutputPanel() }
  }

  readonly property QtObject cmdEditorAddOutputSlider: QtObject {
    function run() { Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputSlider) }
  }

  readonly property QtObject cmdEditorAddOutputToggle: QtObject {
    function run() { Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputToggle) }
  }

  readonly property QtObject cmdEditorAddOutputKnob: QtObject {
    function run() { Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputKnob) }
  }

  readonly property QtObject cmdEditorAddOutputTextField: QtObject {
    function run() { Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputTextField) }
  }

  readonly property QtObject cmdEditorAddOutputButton: QtObject {
    function run() { Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputButton) }
  }

  //
  // Help/navigation/context commands (JSON-only ids, no toolbar button).
  //
  readonly property QtObject cmdEditorHelpCenter: QtObject {
    function run() { app.showHelpCenter("project-editor") }
  }

  readonly property QtObject cmdAppAi: QtObject {
    readonly property bool visible: Cpp_CommercialBuild
    function run() { app.showAIAssistant() }
  }

  readonly property QtObject cmdAppHelpCenter: QtObject {
    function run() { app.showHelpCenter() }
  }

  readonly property QtObject cmdAppProblems: QtObject {
    function run() { app.showProblemCenter() }
  }

  readonly property QtObject cmdAppConnectionDiagnostics: QtObject {
    readonly property bool enabled: !Cpp_Misc_ConnectionDiagnostics.running
    function run() { app.runConnectionDiagnostics() }
  }

  readonly property QtObject cmdAppPreferences: QtObject {
    readonly property bool visible: !app.runtimeMode
    function run() { app.showSettingsDialog() }
  }

  readonly property QtObject cmdEditorNavigateBack: QtObject {
    function run() { Cpp_JSON_ProjectEditor.navigateBack() }
  }

  readonly property QtObject cmdEditorNavigateForward: QtObject {
    function run() { Cpp_JSON_ProjectEditor.navigateForward() }
  }

  readonly property QtObject cmdEditorExpandTree: QtObject {
    function run() { Cpp_JSON_ProjectEditor.expandAllTreeItems() }
  }

  readonly property QtObject cmdEditorCollapseTree: QtObject {
    function run() { Cpp_JSON_ProjectEditor.collapseTreeToOverview() }
  }

  readonly property QtObject cmdPaletteOpen: QtObject {
    function run() { if (root.pePalette) root.pePalette.toggle() }
  }

  readonly property QtObject cmdAppQuit: QtObject {
    readonly property bool visible: false
    function run() { app.quitApplication() }
  }
}
