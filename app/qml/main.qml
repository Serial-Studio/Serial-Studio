/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
  readonly property string appIcon: proVersion ? "qrc:/rcc/logo/icon-pro.png" : "qrc:/rcc/logo/icon.png"
  readonly property bool proVersion: Cpp_QtCommercial_Available ? Cpp_Licensing_LemonSqueezy.isActivated : false

  //
  // Check for updates (non-silent mode)
  //
  function checkForUpdates() {
    Cpp_Updater.setNotifyOnFinish(Cpp_AppUpdaterUrl, true)
    Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
  }

  //
  // Ask the user to save the project file before closing the app
  //
  function handleClose(close) {
    close.accepted = false
    if (Cpp_JSON_ProjectModel.askSave()) {
      close.accepted = true
      Qt.quit();
    }
  }

  //
  // Main window + subdialogs
  //
  MainWindow.MainWindow {
    id: mainWindow
    onClosing: (close) => app.handleClose(close)

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

    DialogLoader {
      id: licenseDialog
      source: "qrc:/serial-studio.com/gui/qml/Dialogs/LicenseManagement.qml"
    }
  }

  //
  // Project Editor
  //
  ProjectEditor.ProjectEditor {
    id: projectEditor
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
    if (Cpp_QtCommercial_Available)
      licenseDialog.activate()
  } function showMqttConfiguration() {
    if (Cpp_QtCommercial_Available)
      mqttConfiguration.activate()
  }
}
