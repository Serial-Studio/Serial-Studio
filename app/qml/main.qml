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
import Qt5Compat.GraphicalEffects
import SerialStudio

import "Widgets" as Widgets
import "Dialogs" as Dialogs
import "MainWindow" as MainWindow
import "ProjectEditor" as ProjectEditor

Item {
  id: app

  //
  // Define application name
  //
  property bool dontNag: false
  property bool quitting: false
  readonly property bool proVersion: Cpp_CommercialBuild ?
                                       Cpp_Licensing_LemonSqueezy.isActivated ||
                                       Cpp_Licensing_Trial.trialEnabled : false

  //
  // Runtime identification for unique settings for each shortcut
  //
  readonly property bool runtimeMode: typeof CLI_RUNTIME_MODE !== "undefined" && CLI_RUNTIME_MODE === true
  readonly property string settingsSuffix: runtimeMode
                                           ? (typeof CLI_SETTINGS_SUFFIX !== "undefined" ? CLI_SETTINGS_SUFFIX : "")
                                           : ""


  //
  // IO options enabled/disabled
  //
  readonly property bool mqttSubscriber: Cpp_CommercialBuild
                                         ? (Cpp_MQTT_Client.isConnected
                                            && Cpp_MQTT_Client.isSubscriber)
                                         : false
  readonly property bool sessionPlayerOpen: Cpp_CommercialBuild
                                            ? Cpp_Sessions_Player.isOpen
                                            : false
  readonly property bool ioEnabled: (!Cpp_IO_Manager.isConnected
                                     && !Cpp_CSV_Player.isOpen
                                     && !Cpp_MDF4_Player.isOpen
                                     && !sessionPlayerOpen)
                                    || mqttSubscriber

  //
  // App-level settings
  //
  Settings {
    category: "App"
    property alias hideWelcomeDialog: app.dontNag
  }

  //
  // Check for updates (non-silent mode)
  //
  function checkForUpdates() {
    Cpp_Updater.setNotifyOnFinish(Cpp_AppUpdaterUrl, true)
    Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
  }

  //
  // Centralized quit handler — respects save dialogs before quitting
  //
  function quitApplication() {
    if (app.quitting)
      return

    // Skip the save prompt in runtime mode — operators have no project to save.
    const runtimeMode = typeof CLI_RUNTIME_MODE !== "undefined" && CLI_RUNTIME_MODE === true
    if (!runtimeMode && !Cpp_JSON_ProjectModel.askSave())
      return

    // Hide windows, then quit after they disappear
    app.quitting = true
    mainWindow.visible = false
    projectEditor.visible = false
    if (dbExplorerLoader.item)
      dbExplorerLoader.item.visible = false
    quitTimer.restart()
  }

  Timer {
    id: quitTimer

    interval: 150
    repeat: false
    onTriggered: Cpp_Misc_ModuleManager.onQuit()
  }

  //
  // Launch welcome dialog or show main window during starup
  //
  Component.onCompleted: {
    const runtimeMode = typeof CLI_RUNTIME_MODE !== "undefined" && CLI_RUNTIME_MODE === true
    if (Cpp_CommercialBuild) {
      // Runtime mode bypasses the welcome dialog — main window handles licensing UI.
      if (!Cpp_Licensing_LemonSqueezy.isActivated && !runtimeMode) {
        app.showWelcomeDialog()
        return
      }
    }

    app.showMainWindow()
  }

  //
  // Open downloaded example project in the editor
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
  // Main window + subdialogs
  //
  MainWindow.MainWindow {
    id: mainWindow
    onClosing: (close) => {
                 if (app.quitting) {
                   close.accepted = true
                   return
                 }

                 close.accepted = false
                 app.quitApplication()
               }

    onVisibleChanged: {
      if (!visible) {
        settingsDialog.hide()
        actionIconPicker.hide()
        onlineIconPicker.hide()
        csvPlayer.hide()
        mdf4Player.hide()
        donateDialog.hide()
        mqttConfiguration.hide()
        aboutDialog.hide()
        acknowledgementsDialog.hide()
        fileTransmissionDialog.hide()
        examplesBrowser.hide()
        extensionManager.hide()
        helpCenter.hide()
        licenseDialog.hide()
      }
    }

    DialogLoader {
      id: settingsDialog
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/Settings.qml"
    }

    Dialogs.IconPicker {
      id: actionIconPicker
    }

    Dialogs.OnlineIconPicker {
      id: onlineIconPicker

      onIconSelected: function(icon) {
        actionIconPicker.iconSelected(icon)
      }
    }

    Dialogs.CsvPlayer {
      id: csvPlayer
    }

    Dialogs.Mdf4Player {
      id: mdf4Player
    }

    Loader {
      id: sqlitePlayerLoader

      active: Cpp_CommercialBuild
      source: "Dialogs/SqlitePlayer.qml"
    }

    DialogLoader {
      id: donateDialog
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/Donate.qml"
    }

    DialogLoader {
      id: fileTransmissionDialog
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/FileTransmission.qml"
    }

    DialogLoader {
      id: mqttConfiguration
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/MQTTConfiguration.qml"
    }

    DialogLoader {
      id: shortcutGeneratorDialog
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/ShortcutGenerator.qml"
    }

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

    DialogLoader {
      id: aboutDialog
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/About.qml"
    }

    DialogLoader {
      id: acknowledgementsDialog
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/Acknowledgements.qml"
    }

    DialogLoader {
      id: examplesBrowser
      asynchronous: false
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/ExamplesBrowser.qml"
    }

    DialogLoader {
      id: extensionManager
      asynchronous: false
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/ExtensionManager.qml"
    }

    DialogLoader {
      id: helpCenter
      asynchronous: false
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/HelpCenter.qml"
    }
  }

  //
  // Project editor dialog
  //
  ProjectEditor.ProjectEditor {
    id: projectEditor
  }

  //
  // Database explorer (Pro)
  //
  Loader {
    id: dbExplorerLoader

    active: Cpp_CommercialBuild
    source: "DatabaseExplorer/DatabaseExplorer.qml"
  }

  //
  // License activation dialog
  //
  DialogLoader {
    id: licenseDialog
    source: "qrc:/serial-studio.com/gui/qml/Dialogs/LicenseManagement.qml"
  }

  //
  // Welcome dialog
  //
  DialogLoader {
    id: welcomeDialog
    source: "qrc:/serial-studio.com/gui/qml/Dialogs/Welcome.qml"
  }

  //
  // Main Window display function
  //
  function showMainWindow() {
    mainWindow.showWindow()
  }

  //
  // Dialog display functions (FOSS)
  //
  function showAboutDialog()       { aboutDialog.activate() }
  function showSettingsDialog()    { settingsDialog.activate() }
  function showProjectEditor()     { projectEditor.displayWindow() }
  function showAcknowledgements()  { acknowledgementsDialog.activate() }
  function showFileTransmission()  { fileTransmissionDialog.activate() }
  function showExamplesBrowser()   { examplesBrowser.activate() }
  function showExtensionManager()  { extensionManager.activate() }
  function showDatabaseExplorer()  {
    if (dbExplorerLoader.item)
      dbExplorerLoader.item.displayWindow()
  }
  function showHelpCenter(pageId) {
    if (pageId)
      Cpp_HelpCenter.showPage(pageId)

    helpCenter.activate()
  }

  //
  // Dialog display functions (commercial)
  //
  function showLicenseDialog() {
    if (Cpp_CommercialBuild)
      licenseDialog.activate()
  } function showMqttConfiguration() {
    if (Cpp_CommercialBuild)
      mqttConfiguration.activate()
  } function showShortcutGenerator() {
    if (Cpp_CommercialBuild)
      shortcutGeneratorDialog.activate()
  } function showRuntimeReconfigure(mode) {
    if (Cpp_CommercialBuild) {
      const resolved = mode || "failed"
      runtimeReconfigureDialog.pendingMode = resolved
      if (runtimeReconfigureDialog.item)
        runtimeReconfigureDialog.item.dialogMode = resolved
      runtimeReconfigureDialog.activate()
    }
  } function showWelcomeDialog() {
    if (Cpp_CommercialBuild) {
      if (!Cpp_Licensing_Trial.trialExpired && Cpp_Licensing_Trial.trialEnabled && app.dontNag && Cpp_Licensing_Trial.daysRemaining > 1)
        showMainWindow()

      else
        welcomeDialog.activate()
    }
  }
}
