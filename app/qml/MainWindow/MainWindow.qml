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
  // Aliases for tracking minimum content size on native/CSD windows
  //
  property alias preferredWidth: layout.implicitWidth
  property alias preferredHeight: layout.implicitHeight

  //
  // Custom properties
  //
  property int appLaunchCount: 0
  property bool windowShown: false
  property string documentTitle: ""
  property bool sawConnection: false
  property bool firstValidFrame: false
  property bool userInitiatedDisconnect: false
  property alias toolbarVisible: toolbar.toolbarEnabled

  //
  // Wraps any QML disconnect entry point so device-initiated drops can be told apart.
  //
  function userDisconnect() {
    root.userInitiatedDisconnect = true
    Cpp_IO_Manager.disconnectDevice()
  }

  function userToggleConnection() {
    if (Cpp_IO_Manager.isConnected)
      root.userInitiatedDisconnect = true
    Cpp_IO_Manager.toggleConnection()
  }

  //
  // Runtime mode: user-initiated disconnect quits the app, device drops pop the
  // reconfigure dialog so the operator can recover instead of losing the session.
  //
  Connections {
    target: Cpp_IO_Manager
    function onConnectedChanged() {
      if (Cpp_IO_Manager.isConnected) {
        root.sawConnection = true
        root.userInitiatedDisconnect = false
        return
      }

      // Ignore disconnects fired while the app is already shutting down.
      if (app.quitting)
        return

      if (!app.runtimeMode || !root.sawConnection)
        return

      if (root.userInitiatedDisconnect) {
        root.userInitiatedDisconnect = false
        app.quitApplication()
      } else {
        app.showRuntimeReconfigure("lost")
      }
    }
  }

  //
  // Pops the reconfigure dialog when auto-connect doesn't settle in time.
  //
  Timer {
    id: runtimeFailGuard
    interval: 4000
    repeat: false
    running: app.runtimeMode
             && !Cpp_IO_Manager.isConnected
             && !root.sawConnection
    onTriggered: {
      if (app.runtimeMode && !Cpp_IO_Manager.isConnected && !root.sawConnection)
        app.showRuntimeReconfigure("failed")
    }
  }

  //
  // Shows the window. Runtime mode lays the dashboard out up front so the
  // connecting overlay covers it while the device handshake completes.
  //
  function presentWindow() {
    if (windowShown)
      return

    windowShown = true

    if (app.runtimeMode)
      dashboard.loadLayout()

    if (typeof CLI_START_FULLSCREEN !== "undefined" && CLI_START_FULLSCREEN)
      root.showFullScreen()
    else
      root.displayWindow()
  }

  //
  // Save settings
  //
  Settings {
    property alias launchCount: root.appLaunchCount
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

  function showDashboardNow() {
    if (stack.currentItem !== dashboard) {
      stack.push(dashboard)
      dashboard.loadLayout()

      // Runtime mode handles the toolbar via the app.runtimeMode binding directly.
      if (!app.runtimeMode
          && typeof CLI_HIDE_TOOLBAR !== "undefined"
          && CLI_HIDE_TOOLBAR)
        mainWindow.toolbarVisible = false

      if (typeof CLI_START_FULLSCREEN !== "undefined" && CLI_START_FULLSCREEN)
        mainWindow.showFullScreen()
    }
  }

  Timer {
    id: dashboardDelayTimer
    interval: 270
    repeat: false
    onTriggered: root.showDashboardNow()
  }

  //
  // Obtain document title
  //
  function updateDocumentTitle() {
    if (Cpp_AppState.operationMode == SerialStudio.ConsoleOnly)
      documentTitle = qsTr("Console Only Mode")

    else if (Cpp_AppState.operationMode == SerialStudio.QuickPlot)
      documentTitle = qsTr("Quick Plot Mode")

    else if (Cpp_AppState.projectFileName.length > 0)
      documentTitle = Cpp_JSON_ProjectModel.title

    else
      documentTitle = qsTr("Empty Project")
  }

  //
  // Window initialization code
  //
  function showWindow() {
    // Increment app launch count
    ++appLaunchCount

    // Obtain document title from JSON project editor
    root.updateDocumentTitle()

    // Always present the window immediately. The connecting overlay covers the
    // empty dashboard while the device is handshaking.
    root.presentWindow()

    // Defer dialogs and update checks until after window is fully rendered
    Qt.callLater(function() {
      // Runtime mode is unattended — skip nags entirely.
      if (app.runtimeMode)
        return

      // Show donations dialog every 15 launches (GPL builds only — Pro skips it)
      if (root.appLaunchCount % 15 == 0 && !Cpp_CommercialBuild)
        donateDialog.activate()

      // Auto-update is opt-out: enabled by default, user can disable in Settings.
      if (Cpp_Misc_ModuleManager.automaticUpdates && Cpp_UpdaterEnabled)
        Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
    })
  }

  //
  // Update document title automatically
  //
  Connections {
    target: Cpp_AppState
    function onOperationModeChanged() { root.updateDocumentTitle() }
    function onProjectFileChanged()   { root.updateDocumentTitle() }
  } Connections {
    target: Cpp_JSON_ProjectModel
    function onTitleChanged() { root.updateDocumentTitle() }
  } Connections {
    target: Cpp_Misc_Translator
    function onLanguageChanged() { root.updateDocumentTitle() }
  }

  //
  // Show console tab on serial disconnect (runtime mode quits instead).
  //
  Connections {
    target: Cpp_UI_Dashboard

    function onDataReset() {
      if (app.runtimeMode)
        return

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
        root.firstValidFrame = true
        if (setup.visible) {
          setup.hide()
          dashboardDelayTimer.start()
        } else {
          root.showDashboardNow()
        }
      }

      else if (!app.runtimeMode) {
        setup.show()
        root.showConsole()
        root.toolbarVisible = true
        root.firstValidFrame = false
      }
    }
  }

  //
  // Handle platform-specific window initialization code
  //
  onVisibilityChanged: {
    if (visible) {
      const tint = app.runtimeMode
                   ? Cpp_ThemeManager.colors["dashboard_background"]
                   : ""
      Cpp_NativeWindow.addWindow(root, tint)
    }

    else
      Cpp_NativeWindow.removeWindow(root)
  }

  //
  // Shortcuts
  //
  Shortcut {
    enabled: !app.runtimeMode
    sequences: [StandardKey.Preferences]
    onActivated: app.showSettingsDialog()
  } Shortcut {
    sequences: [StandardKey.Quit]
    onActivated: app.quitApplication()
  } Shortcut {
    enabled: !app.runtimeMode
    sequences: [StandardKey.Open]
    onActivated: Cpp_CSV_Player.openFile()
  }

  //
  // Load user interface component
  //
  Page {
    clip: true
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
        id: toolbar

        z: 2
        Layout.fillWidth: true
        toolbarEnabled: !app.runtimeMode
        dashboardVisible: root.dashboardVisible
        autoHide: Cpp_UI_Dashboard.autoHideToolbar
      }

      //
      // User interface
      //
      RowLayout {
        id: mainLayout

        z: 1
        spacing: 0
        Layout.topMargin: -1

        //
        // Dashboard + Console view
        //
        StackView {
          id: stack

          Layout.fillWidth: true
          Layout.fillHeight: true
          initialItem: app.runtimeMode ? dashboard : terminal
          Layout.minimumHeight: Math.max(terminal.implicitHeight, setup.implicitHeight)
          Layout.minimumWidth: terminal.implicitWidth + (setup.visible ? 0 : setup.kMinPaneWidth + 1)

          data: [
            Panes.Console {
              id: terminal

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

          MouseArea {
            width: 16
            height: parent.height
            anchors.leftMargin: -4
            anchors.left: parent.left
            cursorShape: Qt.SplitHCursor

            property int _startX: 0
            property int _startWidth: 0

            onPressed: (mouse) => {
              _startX = mouse.x
              _startWidth = setup.width
            }

            onPositionChanged: (mouse) => {
              if (!pressed)
                return

              const delta = _startX - mouse.x
              const maxW = mainLayout.width - stack.Layout.minimumWidth - 1
              const newW = Math.max(setup.kMinPaneWidth,
                                    Math.min(_startWidth + delta, maxW))
              setup.userPaneWidth = newW
            }
          }
        }

        //
        // Setup panel
        //
        Panes.Setup {
          id: setup

          Layout.fillHeight: true
          visible: !app.runtimeMode
          Layout.rightMargin: setupMargin
          Layout.minimumWidth: app.runtimeMode ? 0 : setup.kMinPaneWidth
          Layout.preferredWidth: app.runtimeMode ? 0 : setup.displayedWidth
          Layout.maximumWidth: mainLayout.width - stack.Layout.minimumWidth - 1
        }
      }
    }

    //
    // File drop area
    //
    Widgets.FileDropArea {
      anchors.fill: parent
      enabled: !Cpp_IO_Manager.isConnected
    }

    //
    // Runtime-mode "Connecting…" overlay shown until the first valid frame.
    //
    Rectangle {
      id: connectingOverlay
      z: 100
      anchors.fill: parent
      visible: opacity > 0
      opacity: app.runtimeMode && !Cpp_UI_Dashboard.available ? 1 : 0
      color: Cpp_ThemeManager.colors["dashboard_background"]

      Behavior on opacity {
        NumberAnimation { duration: 350; easing.type: Easing.InOutCubic }
      }

      MouseArea {
        hoverEnabled: true
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
      }

      ColumnLayout {
        spacing: 16
        anchors.centerIn: parent

        BusyIndicator {
          implicitWidth: 64
          implicitHeight: 64
          Layout.alignment: Qt.AlignHCenter
          running: connectingOverlay.visible
        }

        Label {
          Layout.alignment: Qt.AlignHCenter
          text: Cpp_JSON_ProjectModel.title.length > 0
                ? Cpp_JSON_ProjectModel.title
                : qsTr("Serial Studio")
          font: Cpp_Misc_CommonFonts.customUiFont(1.4, true)
          color: Cpp_ThemeManager.colors["text"]
        }

        Label {
          Layout.alignment: Qt.AlignHCenter
          text: Cpp_IO_Manager.isConnected
                ? qsTr("Waiting for data…")
                : qsTr("Connecting to device…")
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["placeholder_text"]
        }
      }
    }
  }
}
