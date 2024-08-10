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

import "Dialogs" as Windows
import "MainWindow" as MainWindow
import "ProjectEditor" as ProjectEditor

Item {
  id: app

  //
  // Global propeties
  //
  readonly property int spacing: 8
  readonly property string monoFont: "Roboto Mono"

  //
  // Access to dialogs & windows
  //
  property Window aboutDialog: null
  property Window donateDialog: null
  property Window mainWindow: null
  property Window csvPlayerDialog: null
  property Window projectEditorWindow: null
  property Window acknowledgementsDialog: null

  //
  // Check for updates (non-silent mode)
  //
  function checkForUpdates() {
    Cpp_Updater.setNotifyOnFinish(Cpp_AppUpdaterUrl, true)
    Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
  }

  //
  // MainWindow
  //
  Loader {
    asynchronous: true
    sourceComponent: MainWindow.Root {
      Component.onCompleted: {
        app.forceActiveFocus()
        app.mainWindow = this
      }
    }
  }

  //
  // About window
  //
  Loader {
    asynchronous: true
    sourceComponent: Windows.About {
      Component.onCompleted: app.aboutDialog = this
    }
  }

  //
  // CSV player window
  //
  Loader {
    asynchronous: true
    sourceComponent: Windows.CsvPlayer {
      Component.onCompleted: app.csvPlayerDialog = this
    }
  }

  //
  // Project editor dialog
  //
  Loader {
    asynchronous: true
    sourceComponent: ProjectEditor.Root {
      Component.onCompleted: app.projectEditorWindow = this
    }
  }

  //
  // Donations dialog
  //
  Loader {
    asynchronous: false
    sourceComponent: Windows.Donate {
      Component.onCompleted: app.donateDialog = this
    }
  }

  //
  // Acknowledgements dialog
  //
  Loader {
    asynchronous: true
    sourceComponent: Windows.Acknowledgements {
      Component.onCompleted: app.acknowledgementsDialog = this
    }
  }
}
