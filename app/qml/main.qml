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
  readonly property bool proVersion: Cpp_CommercialBuild ? Cpp_Licensing_LemonSqueezy.isActivated || Cpp_Licensing_Trial.trialEnabled : false

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
  // Launch welcome dialog or show main window during starup
  //
  Component.onCompleted: {
    if (Cpp_CommercialBuild) {
      if (!Cpp_Licensing_LemonSqueezy.isActivated) {
        app.showWelcomeDialog()
        return
      }
    }

    app.showMainWindow()
  }

  //
  // Main window + subdialogs
  //
  MainWindow.MainWindow {
    id: mainWindow
    onClosing: (close) => {
                 close.accepted = Cpp_JSON_ProjectModel.askSave()
                 if (close.accepted)
                 Qt.quit()
               }

    Dialogs.Settings {
      id: settingsDialog
    }

    Dialogs.IconPicker {
      id: actionIconPicker
    }

    Dialogs.CsvPlayer {
      id: csvPlayer
    }

    Dialogs.Donate {
      id: donateDialog
    }

    DialogLoader {
      id: mqttConfiguration
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/MQTTConfiguration.qml"
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
      id: fileTransmissionDialog
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/FileTransmission.qml"
    }
  }

  //
  // Project editor dialog
  //
  ProjectEditor.ProjectEditor {
    id: projectEditor
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
  function showSettingsDialog()    { settingsDialog.showNormal() }
  function showProjectEditor()     { projectEditor.displayWindow() }
  function showAcknowledgements()  { acknowledgementsDialog.activate() }
  function showFileTransmission()  { fileTransmissionDialog.activate() }

  //
  // Dialog display functions (commercial)
  //
  function showLicenseDialog() {
    if (Cpp_CommercialBuild)
      licenseDialog.activate()
  } function showMqttConfiguration() {
    if (Cpp_CommercialBuild)
      mqttConfiguration.activate()
  } function showWelcomeDialog() {
    if (Cpp_CommercialBuild) {
      if (!Cpp_Licensing_Trial.trialExpired && Cpp_Licensing_Trial.trialEnabled && app.dontNag && Cpp_Licensing_Trial.daysRemaining > 1)
        showMainWindow()

      else
        welcomeDialog.activate()
    }
  }
}
