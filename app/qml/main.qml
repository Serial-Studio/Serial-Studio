/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
  MainWindow.Root {
    id: mainWindow
    onClosing: (close) => app.handleClose(close)

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
      source: "qrc:/qml/Dialogs/MQTTConfiguration.qml"
    }

    DialogLoader {
      id: aboutDialog
      source: "qrc:/qml/Dialogs/About.qml"
    }

    DialogLoader {
      id: acknowledgementsDialog
      source: "qrc:/qml/Dialogs/Acknowledgements.qml"
    }
  }

  //
  // Project Editor
  //
  ProjectEditor.Root {
    id: projectEditor
  }

  //
  // External console
  //
  DialogLoader {
    id: externalConsole
    source: "qrc:/qml/Dialogs/ExternalConsole.qml"
  }

  //
  // Dialog display functions
  //
  function showAboutDialog()       { aboutDialog.active = true }
  function showProjectEditor()     { projectEditor.displayWindow() }
  function showExternalConsole()   { externalConsole.active = true }
  function showMqttConfiguration() { mqttConfiguration.active = true }
  function showAcknowledgements()  { acknowledgementsDialog.active = true }
}
