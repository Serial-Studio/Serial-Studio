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
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import "Panes" as Panes
import "../Widgets" as Widgets

Widgets.SmartWindow {
  id: root
  title: documentTitle
  category: "MainWindow"
  minimumWidth: layout.implicitWidth
  minimumHeight: layout.implicitHeight

  //
  // Custom properties
  //
  property int appLaunchCount: 0
  property string documentTitle: ""
  property bool firstValidFrame: false
  property bool automaticUpdates: false
  property alias toolbarVisible: toolbar.toolbarEnabled

  //
  // Show window full screen when toolbar is hidden
  //
  onToolbarVisibleChanged: {
    if (toolbarVisible)
      root.showNormal()
    else
      root.showFullScreen()
  }

  //
  // Save settings
  //
  Settings {
    property alias launchCount: root.appLaunchCount
    property alias automaticUpdater: root.automaticUpdates
  }

  //
  // Global properties
  //
  readonly property bool setupVisible: setup.visible
  readonly property bool consoleVisible: terminal.visible
  readonly property bool dashboardVisible: dashboard.visible

  //
  // Toolbar functions aliases
  //
  function showSetup()     { toolbar.setupClicked() }
  function showConsole()   { stack.pop()            }
  function showDashboard() { dbTimer.start()        }

  //
  // Obtain document title
  //
  function updateDocumentTitle() {
    if (Cpp_JSON_FrameBuilder.operationMode == SerialStudio.DeviceSendsJSON)
      documentTitle = qsTr("Device Defined Project")

    else if (Cpp_JSON_FrameBuilder.operationMode == SerialStudio.QuickPlot)
      documentTitle = qsTr("Quick Plot Mode")

    else if (Cpp_JSON_FrameBuilder.jsonMapFilename.length > 0)
      documentTitle = Cpp_JSON_ProjectModel.title

    else
      documentTitle = qsTr("Empty Project")
  }

  //
  // Wait a little before showing the dashboard to avoid UI glitches and/or
  // overloading the rendering engine
  //
  Timer {
    id: dbTimer
    interval: 500
    onTriggered: {
      if (Cpp_UI_Dashboard.available) {
        if (stack.currentItem != dashboard) {
          stack.push(dashboard)
          dashboard.loadLayout()
        }
      }

      else
        root.showConsole()
    }
  }

  //
  // Update document title automatically
  //
  Connections {
    target: Cpp_JSON_FrameBuilder
    function onOperationModeChanged() { root.updateDocumentTitle() }
    function onJsonFileMapChanged()   { root.updateDocumentTitle() }
  } Connections {
    target: Cpp_JSON_ProjectModel
    function onJsonFileChanged() { root.updateDocumentTitle() }
  } Connections {
    target: Cpp_Misc_Translator
    function onLanguageChanged() { root.updateDocumentTitle() }
  }

  //
  // Show console tab on serial disconnect
  //
  Connections {
    target: Cpp_UI_Dashboard

    function onDataReset() {
      setup.show()
      root.showConsole()
      root.firstValidFrame = false
    }
  }

  //
  // Hide console & device manager when we receive first valid frame
  //
  Connections {
    target: Cpp_UI_Dashboard

    function onUpdated()  {
      if (root.firstValidFrame)
        return

      if (Cpp_UI_Dashboard.available) {
        setup.hide()
        root.showDashboard()
        root.firstValidFrame = true
      }

      else {
        setup.show()
        root.showConsole()
        root.toolbarVisible = true
        root.firstValidFrame = false
      }
    }
  }

  //
  // Loading code
  //
  Component.onCompleted: {
    // Increment app launch count
    ++appLaunchCount

    // Show donations dialog every 15 launches
    if (root.appLaunchCount % 15 == 0)
      donateDialog.showAutomatically()

    // Ask user if he/she wants to enable automatic updates
    if (root.appLaunchCount == 2 && Cpp_UpdaterEnabled) {
      if (Cpp_Misc_Utilities.askAutomaticUpdates()) {
        root.automaticUpdates = true
        Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
      }

      else
        root.automaticUpdates = false
    }

    // Check for updates (if we are allowed)
    if (root.automaticUpdates && Cpp_UpdaterEnabled)
      Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)

    // Obtain document title from JSON project editor & display the window
    root.updateDocumentTitle()
    root.displayWindow()
  }

  //
  // Handle platform-specific window initialization code
  //
  onVisibilityChanged: {
    if (visible)
      Cpp_NativeWindow.addWindow(root)
    else
      Cpp_NativeWindow.removeWindow(root)
  }

  //
  // Shortcuts
  //
  Shortcut {
    sequences: [StandardKey.Preferences]
    onActivated: app.showSettingsDialog()
  } Shortcut {
    sequences: [StandardKey.Quit]
    onActivated: root.close()
  } Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  } Shortcut {
    sequences: [StandardKey.Open]
    onActivated: Cpp_CSV_Player.openFile()
  }

  //
  // Load user interface component
  //
  Page {
    anchors.fill: parent
    palette.mid: Cpp_ThemeManager.colors["mid"]
    palette.dark: Cpp_ThemeManager.colors["dark"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.link: Cpp_ThemeManager.colors["link"]
    palette.light: Cpp_ThemeManager.colors["light"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.shadow: Cpp_ThemeManager.colors["shadow"]
    palette.accent: Cpp_ThemeManager.colors["accent"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.midlight: Cpp_ThemeManager.colors["midlight"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.windowText: Cpp_ThemeManager.colors["window_text"]
    palette.brightText: Cpp_ThemeManager.colors["bright_text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
    palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
    palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
    palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    ColumnLayout {
      id: layout
      spacing: 0
      anchors.fill: parent

      //
      // Toolbar
      //
      Panes.Toolbar {
        z: 2
        id: toolbar
        Layout.fillWidth: true
        setupChecked: root.setupVisible
        onSetupClicked: setup.visible ? setup.hide() : setup.show()
      }

      //
      // User interface
      //
      RowLayout {
        z: 1
        spacing: 0
        Layout.topMargin: -1

        //
        // Dashboard + Console view
        //
        StackView {
          id: stack
          initialItem: terminal
          Layout.fillWidth: true
          Layout.fillHeight: true
          Layout.minimumHeight: Math.max(dashboard.implicitHeight, terminal.implicitHeight, setup.implicitHeight)
          Layout.minimumWidth: Math.max(dashboard.implicitWidth, terminal.implicitWidth) + (setup.visible ? 0 : setup.displayedWidth + 1)

          data: [
            Panes.Console {
              id: terminal
              visible: false
              width: parent.width
              height: parent.height
            },

            Panes.Dashboard {
              id: dashboard
              visible: false
              width: parent.width
              height: parent.height
            }
          ]
        }

        //
        // Panel border rectangle
        //
        Rectangle {
          z: 2
          implicitWidth: 1
          visible: setup.visible
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["setup_border"]

          Rectangle {
            width: 1
            height: 32
            anchors.top: parent.top
            anchors.left: parent.left
            color: Cpp_ThemeManager.colors["pane_caption_border"]
          }
        }

        //
        // Setup panel
        //
        Panes.Setup {
          id: setup
          Layout.fillHeight: true
          Layout.rightMargin: setupMargin
          Layout.minimumWidth: displayedWidth
          Layout.maximumWidth: displayedWidth
        }
      }
    }

    //
    // JSON project drop area
    //
    Widgets.JSONDropArea {
      anchors.fill: parent
      enabled: !Cpp_IO_Manager.isConnected
    }
  }
}
