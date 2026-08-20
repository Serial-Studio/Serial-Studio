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

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Controls
import SerialStudio

import "Widgets" as Widgets
import "Dialogs" as Dialogs
import "MainWindow" as MainWindow
import "ProjectEditor" as ProjectEditor

Item {
  id: app

  //
  // Transient quit / dialog flags
  //
  property bool dontNag: false
  property bool quitting: false

  //
  // True while the command palette overlay holds keyboard focus; dashboard
  // focus-grabbers (hover-follow, terminal auto-focus) stand down while set
  //
  property bool commandPaletteOpen: false

  //
  // True when a commercial build holds an active license or trial
  //
  readonly property bool proVersion: Cpp_CommercialBuild
                                     ? (Cpp_Licensing_LemonSqueezy.isActivated
                                        || Cpp_Licensing_Trial.trialEnabled)
                                     : false

  //
  // True when launched from a generated operator-deployment shortcut
  //
  readonly property bool runtimeMode: typeof CLI_RUNTIME_MODE !== "undefined"
                                      && CLI_RUNTIME_MODE === true

  //
  // Session-scoped macro-editor draft, kept while the Macros window is closed
  //
  property string macroDraft: ""
  property int macroDraftLanguage: 0

  //
  // Per-deployment QSettings suffix injected by the C++ launcher
  //
  readonly property string settingsSuffix: runtimeMode
                                           ? (typeof CLI_SETTINGS_SUFFIX !== "undefined"
                                              ? CLI_SETTINGS_SUFFIX
                                              : "")
                                           : ""

  //
  // True while a recorded session is playing back
  //
  readonly property bool sessionPlayerOpen: Cpp_CommercialBuild
                                            ? Cpp_Sessions_Player.isOpen
                                            : false

  //
  // True when setup-pane controls are allowed to mutate the connection
  //
  readonly property bool ioEnabled: !Cpp_IO_Manager.isConnected
                                    && !Cpp_CSV_Player.isOpen
                                    && !Cpp_MDF4_Player.isOpen
                                    && !sessionPlayerOpen

  //
  // Cross-launch app flags: shared store, NOT per-deployment
  //
  Settings {
    category: "App"
    property alias hideWelcomeDialog: app.dontNag
  }

  //
  // Trigger an interactive update check from the toolbar
  //
  function checkForUpdates() {
    Cpp_Updater.setNotifyOnFinish(Cpp_AppUpdaterUrl, true)
    Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
  }

  //
  // Centralized quit: save prompt in author mode, then defer C++ teardown
  //
  function quitApplication() {
    if (app.quitting)
      return

    if (!app.runtimeMode && !Cpp_JSON_ProjectModel.askSave())
      return

    app.quitting = true
    mainWindow.visible = false
    if (projectEditorLoader.item)
      projectEditorLoader.item.visible = false

    helpCenter.close()
    problemCenter.close()
    dbExplorerLoader.close()
    aiAssistantLoader.close()
    aiProUpgradeLoader.close()
    quitTimer.restart()
  }

  //
  // Defer the actual quit until close animations settle
  //
  Timer {
    id: quitTimer

    repeat: false
    interval: 150
    onTriggered: Cpp_Misc_ModuleManager.onQuit()
  }

  //
  // Confirm the configured rendering backend works
  //
  property bool rhiStartupConfirmed: false
  Timer {
    id: rhiConfirmTimer

    repeat: false
    interval: 5000
    onTriggered: Cpp_Misc_GraphicsBackend.confirmStartupSuccess()
  }

  //
  // Boot path: runtime mode skips the welcome dialog
  //
  Component.onCompleted: {
    if (Cpp_Misc_CrashTracker.previousRunCrashed
        && Cpp_Misc_CrashTracker.recoveryRecommended) {
      crashRecoveryDialog.activate()
      return
    }

    app.continueBoot()
  }

  //
  // Boot continuation: skipped only when the crash-recovery dialog is up
  //
  function continueBoot() {
    if (Cpp_CommercialBuild
        && !app.runtimeMode
        && !Cpp_Licensing_LemonSqueezy.isActivated) {
      app.showWelcomeDialog()
      return
    }

    app.showMainWindow()
  }

  //
  // Crash recovery dialog: shown only when several consecutive crashes are detected
  //
  DialogLoader {
    id: crashRecoveryDialog

    source: "qrc:/serial-studio.com/gui/qml/Dialogs/CrashRecovery.qml"
  }

  //
  // Auto-import a downloaded example project into the editor
  //
  Connections {
    target: Cpp_Examples
    function onProjectFileReady(path) {
      Cpp_AppState.operationMode = SerialStudio.ProjectFile
      Cpp_JSON_ProjectModel.openJsonFile(path)
      app.showProjectEditor()
    }
  }

  //
  // Main window: hosts dashboard, terminal, and every transient dialog
  //
  MainWindow.MainWindow {
    id: mainWindow

    //
    // Schedule application when the main window is closing
    //
    onClosing: (close) => {
      if (app.quitting) {
        close.accepted = true
        return
      }

      close.accepted = false
      app.quitApplication()
    }

    //
    // Dismiss every floating dialog when the main window hides
    //
    onVisibleChanged: {
      if (visible) {
        if (!app.rhiStartupConfirmed) {
          app.rhiStartupConfirmed = true
          rhiConfirmTimer.start()
        }
        return
      }

      aboutDialog.close()
      donateDialog.close()
      licenseDialog.close()
      settingsDialog.close()
      examplesBrowser.close()
      extensionManager.close()
      acknowledgementsDialog.close()
      benchmarkDialog.close()
      shortcutGeneratorDialog.close()
      runtimeReconfigureDialog.close()
      fileTransmissionDialog.close()
      remoteAttachDialog.close()
      macrosLoader.close()

      if (csvPlayerLoader.item)
        csvPlayerLoader.item.close()

      if (mdf4PlayerLoader.item)
        mdf4PlayerLoader.item.close()
    }

    //
    // About dialog (gated to author mode by showAboutDialog)
    //
    DialogLoader {
      id: aboutDialog

      source: "qrc:/serial-studio.com/gui/qml/Dialogs/About.qml"
    }

    //
    // Runtime-mode reconfigure prompt (driver lost / reconnect failed)
    //
    DialogLoader {
      id: runtimeReconfigureDialog

      property string pendingMode: "failed"
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/RuntimeReconfigure.qml"

      Connections {
        target: runtimeReconfigureDialog
        function onLoaded() {
          if (runtimeReconfigureDialog.item)
            runtimeReconfigureDialog.item.dialogMode = runtimeReconfigureDialog.pendingMode
        }
      }
    }

    //
    // App preferences (gated to author mode)
    //
    DialogLoader {
      id: settingsDialog

      source: "qrc:/serial-studio.com/gui/qml/Dialogs/Settings.qml"
    }

    //
    // Donate prompt (gated to author mode)
    //
    DialogLoader {
      id: donateDialog

      source: "qrc:/serial-studio.com/gui/qml/Dialogs/Donate.qml"
    }

    //
    // Acknowledgements / credits (gated to author mode)
    //
    DialogLoader {
      id: acknowledgementsDialog

      source: "qrc:/serial-studio.com/gui/qml/Dialogs/Acknowledgements.qml"
    }

    //
    // Hotpath benchmark (gated to author mode)
    //
    DialogLoader {
      id: benchmarkDialog

      source: "qrc:/serial-studio.com/gui/qml/Dialogs/Benchmark.qml"
    }

    //
    // Examples browser (gated to author mode)
    //
    DialogLoader {
      id: examplesBrowser

      asynchronous: false
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/ExamplesBrowser.qml"
    }

    //
    // Extension manager (gated to author mode)
    //
    DialogLoader {
      id: extensionManager

      asynchronous: false
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/ExtensionManager.qml"
    }

    //
    // Operator-deployment generator (Pro, gated to author mode)
    //
    DialogLoader {
      id: shortcutGeneratorDialog

      source: "qrc:/serial-studio.com/gui/qml/Dialogs/ShortcutGenerator.qml"
    }

    //
    // File transmission (XMODEM / YMODEM / ZMODEM / plain text / raw binary)
    //
    DialogLoader {
      id: fileTransmissionDialog

      source: "qrc:/serial-studio.com/gui/qml/Dialogs/FileTransmission.qml"
    }

    //
    // Remote dashboard attach: host, port, token and the live link status
    //
    DialogLoader {
      id: remoteAttachDialog

      source: "qrc:/serial-studio.com/gui/qml/Dialogs/RemoteAttach.qml"
    }

    //
    // CSV file playback dialog: auto-shown by Cpp_CSV_Player.isOpen
    //
    Loader {
      id: csvPlayerLoader

      active: !app.runtimeMode
      sourceComponent: Component {
        Dialogs.CsvPlayer {}
      }
    }

    //
    // MDF4 file playback dialog (Pro): auto-shown by Cpp_MDF4_Player.isOpen
    //
    Loader {
      id: mdf4PlayerLoader

      active: !app.runtimeMode
      sourceComponent: Component {
        Dialogs.Mdf4Player {}
      }
    }

    //
    // Session-database playback dialog (Pro): auto-shown by Cpp_Sessions_Player.isOpen
    //
    Loader {
      id: sqlitePlayerLoader

      source: "Dialogs/SqlitePlayer.qml"
      active: Cpp_CommercialBuild && !app.runtimeMode
    }

  }

  //
  // Project editor: separate top-level window, skipped in runtime mode
  //
  Loader {
    id: projectEditorLoader

    active: !app.runtimeMode
    sourceComponent: Component {
      ProjectEditor.ProjectEditor {
        id: projectEditor
      }
    }
  }

  //
  // Historian (Pro): lazy DialogLoader keeps its QSettings out of operator builds
  //
  DialogLoader {
    id: dbExplorerLoader

    source: "qrc:/serial-studio.com/gui/qml/DatabaseExplorer/DatabaseExplorer.qml"
  }

  //
  // Macros: interactive in-process command terminal + script macro editor
  //
  DialogLoader {
    id: macrosLoader

    source: "qrc:/serial-studio.com/gui/qml/Dialogs/Macros.qml"
  }

  //
  // Help center: top-level window independent of the main window;
  // synchronous load (WebEngineView races otherwise)
  //
  DialogLoader {
    id: helpCenter

    asynchronous: false
    source: "qrc:/serial-studio.com/gui/qml/Dialogs/HelpCenter.qml"
  }

  //
  // Problem center: top-level window, reachable from the main window and the
  // project editor
  //
  DialogLoader {
    id: problemCenter

    source: "qrc:/serial-studio.com/gui/qml/Dialogs/ProblemCenter.qml"
  }

  //
  // Project-entity navigation for a problem finding. The collector only emits
  // the request; the editor lookup lives here so no C++ module depends on it.
  //
  Connections {
    target: Cpp_Misc_ProblemCenter
    function onJumpRequested(kind, uniqueId) {
      app.jumpToProblemTarget(kind, uniqueId)
    }
  }

  //
  // Widget-extension consent: asked once per package version, before the package runs
  //
  DialogLoader {
    id: extensionConsentDialog

    property var pendingIds: []
    source: "qrc:/serial-studio.com/gui/qml/Dialogs/ExtensionConsent.qml"

    Connections {
      target: extensionConsentDialog
      function onLoaded() {
        if (!extensionConsentDialog.item)
          return

        extensionConsentDialog.item.enqueue(extensionConsentDialog.pendingIds)
        extensionConsentDialog.pendingIds = []
      }
    }
  }

  //
  // A package asks for a decision the first time a project places one of its widgets
  //
  Connections {
    target: Cpp_UI_WidgetExtensions
    function onConsentRequested(id) {
      app.showExtensionConsent(id)
    }
  }

  //
  // License activation dialog
  //
  DialogLoader {
    id: licenseDialog

    source: "qrc:/serial-studio.com/gui/qml/Dialogs/LicenseManagement.qml"
  }

  //
  // First-launch welcome dialog (commercial only, non-runtime)
  //
  DialogLoader {
    id: welcomeDialog

    source: "qrc:/serial-studio.com/gui/qml/Dialogs/Welcome.qml"
  }

  //
  // AI assistant (Pro, author-only): lazy DialogLoader, hosts a SmartWindow
  //
  DialogLoader {
    id: aiAssistantLoader

    asynchronous: false
    source: "qrc:/serial-studio.com/gui/qml/AI/AssistantPanel.qml"
  }

  //
  // AI Pro-upgrade notice: shown on non-Pro builds when the AI button is clicked
  //
  DialogLoader {
    id: aiProUpgradeLoader

    source: "qrc:/serial-studio.com/gui/qml/AI/ProUpgradeNotice.qml"
  }

  //
  // Show the main window
  //
  function showMainWindow() {
    mainWindow.showWindow()
  }

  //
  // Help center: accessible to operators, may pre-select a page id
  //
  function showHelpCenter(pageId) {
    if (pageId)
      Cpp_HelpCenter.showPage(pageId)

    helpCenter.activate()
  }

  //
  // Problem center: standing project, link and script diagnostics
  //
  function showProblemCenter() {
    problemCenter.activate()
  }

  //
  // Remote dashboard attach: watch another instance's dashboard, read-only
  //
  function showRemoteAttach() {
    remoteAttachDialog.activate()
  }

  //
  // Connection diagnostics: instant checks land before the panel opens, probes follow
  //
  function runConnectionDiagnostics() {
    Cpp_Misc_ConnectionDiagnostics.runAll()
    problemCenter.activate()
  }

  //
  // Opens the surface owning a finding: sourceId, actionId, groupId, dataset uniqueId or settings
  //
  function jumpToProblemTarget(kind, uniqueId) {
    if (kind.indexOf("settings/") === 0) {
      app.showSettingsDialog()
      return
    }

    app.showProjectEditor()

    if (kind === "source")
      Cpp_JSON_ProjectEditor.selectSource(uniqueId)
    else if (kind === "action")
      Cpp_JSON_ProjectEditor.selectAction(uniqueId)
    else if (kind === "group")
      Cpp_JSON_ProjectEditor.selectGroup(uniqueId)
    else if (kind === "dataset")
      app.selectDatasetByUniqueId(uniqueId)
  }

  //
  // Maps a dataset uniqueId to (groupId, datasetId): both summaries walk the same vectors in order
  //
  function selectDatasetByUniqueId(uniqueId) {
    var summary = Cpp_JSON_ProjectEditor.systemDatasetsSummary()
    var position = -1
    for (var i = 0; i < summary.length; ++i) {
      if (summary[i]["uniqueId"] === uniqueId) {
        position = i
        break
      }
    }

    if (position < 0)
      return

    var groups = Cpp_JSON_ProjectModel.groupsForDiagram()
    var flat = 0
    for (var g = 0; g < groups.length; ++g) {
      var rows = groups[g]["datasets"]
      if (position < flat + rows.length) {
        Cpp_JSON_ProjectEditor.selectDataset(groups[g]["groupId"],
                                             rows[position - flat]["datasetId"])
        return
      }

      flat += rows.length
    }
  }

  //
  // Widget-extension consent: reachable to operators too, since a placeholder is the
  // alternative; the dialog itself queues packages and drops decided ones
  //
  function showExtensionConsent(id) {
    if (!id || id.length <= 0)
      return

    if (extensionConsentDialog.item) {
      extensionConsentDialog.item.enqueue([id])
      return
    }

    if (extensionConsentDialog.pendingIds.indexOf(id) < 0)
      extensionConsentDialog.pendingIds.push(id)

    extensionConsentDialog.activate()
  }

  //
  // License activation dialog: accessible to operators
  //
  function showLicenseDialog() {
    if (Cpp_CommercialBuild)
      licenseDialog.activate()
  }

  //
  // Runtime reconfigure prompt
  //
  function showRuntimeReconfigure(mode) {
    if (!Cpp_CommercialBuild)
      return

    const resolved = mode || "failed"
    runtimeReconfigureDialog.pendingMode = resolved
    if (runtimeReconfigureDialog.item)
      runtimeReconfigureDialog.item.dialogMode = resolved

    runtimeReconfigureDialog.activate()
  }

  //
  // Welcome dialog: short-circuits to the main window if trial banner is dismissed
  //
  function showWelcomeDialog() {
    if (!Cpp_CommercialBuild)
      return

    if (!Cpp_Licensing_Trial.trialExpired
        && Cpp_Licensing_Trial.trialEnabled
        && app.dontNag
        && Cpp_Licensing_Trial.daysRemaining > 1)
      showMainWindow()
    else
      welcomeDialog.activate()
  }

  //
  // About dialog: author-only
  //
  function showAboutDialog() {
    if (!app.runtimeMode)
      aboutDialog.activate()
  }

  //
  // App preferences: author-only
  //
  function showSettingsDialog() {
    if (!app.runtimeMode)
      settingsDialog.activate()
  }

  //
  // Acknowledgements: author-only
  //
  function showAcknowledgements() {
    if (!app.runtimeMode)
      acknowledgementsDialog.activate()
  }

  //
  // Hotpath benchmark: author-only
  //
  function showBenchmarkDialog() {
    if (!app.runtimeMode)
      benchmarkDialog.activate()
  }

  //
  // File transmission: author-only unless the deployment grants runtime access
  //
  function showFileTransmission() {
    if (!app.runtimeMode || (app.proVersion && Cpp_IO_FileTransmission.runtimeAccessAllowed))
      fileTransmissionDialog.activate()
  }

  //
  // Examples browser: author-only
  //
  function showExamplesBrowser() {
    if (!app.runtimeMode)
      examplesBrowser.activate()
  }

  //
  // Extension manager: author-only
  //
  function showExtensionManager() {
    if (!app.runtimeMode)
      extensionManager.activate()
  }

  //
  // Project editor: author-only
  //
  function showProjectEditor() {
    if (!app.runtimeMode && projectEditorLoader.item)
      projectEditorLoader.item.displayWindow()
  }

  //
  // Historian: author-only by default; in operator mode requires
  // session-export ON, and pins the explorer to the project's session DB.
  //
  function showDatabaseExplorer() {
    if (!Cpp_CommercialBuild)
      return

    if (app.runtimeMode) {
      if (!Cpp_Sessions_Export.exportEnabled)
        return

      Cpp_Sessions_Manager.openDatabase(
        Cpp_Sessions_Manager.canonicalDbPath(Cpp_JSON_ProjectModel.title))
    }

    dbExplorerLoader.activate()
  }

  //
  // Macros window: available in every build variant
  //
  function showMacros() {
    macrosLoader.activate()
  }

  //
  // Operator-deployment generator: Pro, author-only
  //
  function showShortcutGenerator() {
    if (Cpp_CommercialBuild && !app.runtimeMode)
      shortcutGeneratorDialog.activate()
  }

  //
  // AI assistant: Pro, author-only
  //
  function showAIAssistant() {
    if (Cpp_CommercialBuild && !app.runtimeMode)
      aiAssistantLoader.activate()
  }

  //
  // AI Pro-upgrade notice: shown on non-Pro builds when the AI button is clicked
  //
  function showAIProUpgradeNotice() {
    if (!app.runtimeMode)
      aiProUpgradeLoader.activate()
  }
}
