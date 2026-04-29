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
  // Per-deployment QSettings suffix injected by the C++ launcher
  //
  readonly property string settingsSuffix: runtimeMode
                                           ? (typeof CLI_SETTINGS_SUFFIX !== "undefined"
                                              ? CLI_SETTINGS_SUFFIX
                                              : "")
                                           : ""

  //
  // True when an MQTT subscription is driving the dashboard
  //
  readonly property bool mqttSubscriber: Cpp_CommercialBuild
                                         ? (Cpp_MQTT_Client.isConnected
                                            && Cpp_MQTT_Client.isSubscriber)
                                         : false

  //
  // True while a recorded session is playing back
  //
  readonly property bool sessionPlayerOpen: Cpp_CommercialBuild
                                            ? Cpp_Sessions_Player.isOpen
                                            : false

  //
  // True when setup-pane controls are allowed to mutate the connection
  //
  readonly property bool ioEnabled: (!Cpp_IO_Manager.isConnected
                                     && !Cpp_CSV_Player.isOpen
                                     && !Cpp_MDF4_Player.isOpen
                                     && !sessionPlayerOpen)
                                    || mqttSubscriber

  //
  // Cross-launch app flags — shared store, NOT per-deployment
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
  // Centralized quit — save prompt in author mode, then defer C++ teardown
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

    dbExplorerLoader.close()
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
  // Boot path — runtime mode skips the welcome dialog
  //
  Component.onCompleted: {
    if (Cpp_CommercialBuild
        && !app.runtimeMode
        && !Cpp_Licensing_LemonSqueezy.isActivated) {
      app.showWelcomeDialog()
      return
    }

    app.showMainWindow()
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
  // Main window — hosts dashboard, terminal, and every transient dialog
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
      if (visible)
        return

      aboutDialog.close()
      donateDialog.close()
      helpCenter.close()
      licenseDialog.close()
      settingsDialog.close()
      examplesBrowser.close()
      extensionManager.close()
      mqttConfiguration.close()
      acknowledgementsDialog.close()
      shortcutGeneratorDialog.close()
      runtimeReconfigureDialog.close()
      fileTransmissionDialog.close()

      actionIconPicker.close()
      onlineIconPicker.close()

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
    // Help center — synchronous load (WebEngineView races on Linux otherwise)
    //
    DialogLoader {
      id: helpCenter

      asynchronous: false
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/HelpCenter.qml"
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
    // MQTT broker configuration (Pro, gated to author mode)
    //
    DialogLoader {
      id: mqttConfiguration

      source: "qrc:/serial-studio.com/gui/qml/Dialogs/MQTTConfiguration.qml"
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
    // CSV file playback dialog — auto-shown by Cpp_CSV_Player.isOpen
    //
    Loader {
      id: csvPlayerLoader

      active: !app.runtimeMode
      sourceComponent: Component {
        Dialogs.CsvPlayer {}
      }
    }

    //
    // MDF4 file playback dialog (Pro) — auto-shown by Cpp_MDF4_Player.isOpen
    //
    Loader {
      id: mdf4PlayerLoader

      active: !app.runtimeMode
      sourceComponent: Component {
        Dialogs.Mdf4Player {}
      }
    }

    //
    // Session-database playback dialog (Pro) — auto-shown by Cpp_Sessions_Player.isOpen
    //
    Loader {
      id: sqlitePlayerLoader

      source: "Dialogs/SqlitePlayer.qml"
      active: Cpp_CommercialBuild && !app.runtimeMode
    }

    //
    // Project-editor icon picker — direct instance, resolved by id from editor views
    //
    Dialogs.IconPicker {
      id: actionIconPicker
    }

    //
    // Online icon search (Iconify-backed)
    //
    Dialogs.OnlineIconPicker {
      id: onlineIconPicker

      onIconSelected: function(icon) {
        actionIconPicker.iconSelected(icon)
      }
    }
  }

  //
  // Project editor — separate top-level window, skipped in runtime mode
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
  // Database explorer (Pro) — lazy DialogLoader keeps its QSettings out of operator builds
  //
  DialogLoader {
    id: dbExplorerLoader

    source: "qrc:/serial-studio.com/gui/qml/DatabaseExplorer/DatabaseExplorer.qml"
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
  // Show the main window
  //
  function showMainWindow() {
    mainWindow.showWindow()
  }

  //
  // Help center — accessible to operators, may pre-select a page id
  //
  function showHelpCenter(pageId) {
    if (pageId)
      Cpp_HelpCenter.showPage(pageId)

    helpCenter.activate()
  }

  //
  // License activation dialog — accessible to operators
  //
  function showLicenseDialog() {
    if (Cpp_CommercialBuild)
      licenseDialog.activate()
  }

  //
  // Runtime reconfigure prompt — used by the runtime-mode reconnect path
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
  // Welcome dialog — short-circuits to the main window if trial banner is dismissed
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
  // About dialog — author-only
  //
  function showAboutDialog() {
    if (!app.runtimeMode)
      aboutDialog.activate()
  }

  //
  // App preferences — author-only
  //
  function showSettingsDialog() {
    if (!app.runtimeMode)
      settingsDialog.activate()
  }

  //
  // Acknowledgements — author-only
  //
  function showAcknowledgements() {
    if (!app.runtimeMode)
      acknowledgementsDialog.activate()
  }

  //
  // File transmission — author-only
  //
  function showFileTransmission() {
    if (!app.runtimeMode)
      fileTransmissionDialog.activate()
  }

  //
  // Examples browser — author-only
  //
  function showExamplesBrowser() {
    if (!app.runtimeMode)
      examplesBrowser.activate()
  }

  //
  // Extension manager — author-only
  //
  function showExtensionManager() {
    if (!app.runtimeMode)
      extensionManager.activate()
  }

  //
  // Project editor — author-only
  //
  function showProjectEditor() {
    if (!app.runtimeMode && projectEditorLoader.item)
      projectEditorLoader.item.displayWindow()
  }

  //
  // Database explorer — author-only by default; in operator mode requires
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
  // MQTT configuration — Pro, author-only
  //
  function showMqttConfiguration() {
    if (Cpp_CommercialBuild && !app.runtimeMode)
      mqttConfiguration.activate()
  }

  //
  // Operator-deployment generator — Pro, author-only
  //
  function showShortcutGenerator() {
    if (Cpp_CommercialBuild && !app.runtimeMode)
      shortcutGeneratorDialog.activate()
  }
}
