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
  minimumWidth: app.runtimeMode ? 640 : layout.implicitWidth
  minimumHeight: app.runtimeMode ? 480 : layout.implicitHeight

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
  property bool toolbarVisible: true
  property bool sawConnection: false
  property bool firstValidFrame: false
  property bool userInitiatedDisconnect: false

  //
  // Toolbar full-screen restore state
  //
  property bool toolbarAutoHidden: false
  property bool toolbarVisibleBeforeFullScreen: true

  //
  // True while the toolbar bar (not the titlebar) occupies a layout slot
  //
  readonly property bool toolbarBarShown: root.toolbarVisible
                                          && !(Cpp_UI_Dashboard.autoHideToolbar
                                               && root.dashboardVisible)

  //
  // Native caption tint so the OS-drawn caption (Windows/DWM) tracks the QML strip; empty =
  // default system caption (tinted while the toolbar bar is collapsed over the dashboard).
  //
  readonly property string nativeCaptionColor:
    (app.runtimeMode || (root.dashboardVisible && !root.toolbarBarShown))
      ? Cpp_ThemeManager.colors["dashboard_background"]
      : ""

  onNativeCaptionColorChanged: {
    if (root.visible)
      Cpp_NativeWindow.addWindow(root, root.nativeCaptionColor)
  }

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

    if (app.runtimeMode) {
      dashboardLoader.active = true
      dashboard.loadLayout()
    }

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
  readonly property Item dashboard: dashboardLoader.item
  readonly property bool dashboardVisible: dashboardLoader.item
                                            && stack.currentItem === dashboardLoader

  //
  // Toolbar functions aliases
  //
  function showConsole() { stack.pop() }

  function showDashboardNow() {
    dashboardLoader.active = true
    if (stack.currentItem !== dashboardLoader) {
      stack.push(dashboardLoader)
      dashboard.loadLayout()

      if (!app.runtimeMode
          && typeof CLI_HIDE_TOOLBAR !== "undefined"
          && CLI_HIDE_TOOLBAR)
        mainWindow.toolbarVisible = false

      if (typeof CLI_START_FULLSCREEN !== "undefined" && CLI_START_FULLSCREEN)
        mainWindow.showFullScreen()
    }
  }

  //
  // Obtain document title
  //
  function updateDocumentTitle() {
    if (Cpp_AppState.operationMode == SerialStudio.ConsoleOnly)
      documentTitle = qsTr("Console Only Mode")

    else if (Cpp_AppState.operationMode == SerialStudio.QuickPlot)
      documentTitle = qsTr("Quick Plot Mode")

    // Prefer the project title whenever it's set
    else if (Cpp_JSON_ProjectModel.title.length > 0)
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

    // Present immediately; the connecting overlay covers the dashboard during handshake.
    root.presentWindow()

    // Defer dialogs and update checks until after window is fully rendered
    Qt.callLater(function() {
      //
      // Runtime mode is unattended: skip nags entirely.
      //
      if (app.runtimeMode)
        return

      //
      // Show donations dialog every 15 launches (GPL builds only; Pro skips it)
      //
      if (root.appLaunchCount % 15 == 0 && !Cpp_CommercialBuild)
        donateDialog.activate()

      //
      // Auto-update is opt-out: enabled by default, user can disable in Settings.
      //
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

      root.showConsole()
      root.firstValidFrame = false

      // Drop out of full-screen on disconnect
      if (root.visibility === Window.FullScreen)
        root.showNormal()
    }
  }

  //
  // Hide console & device manager on first valid frame
  //
  Connections {
    target: Cpp_UI_Dashboard

    function onUpdated()  {
      if (root.firstValidFrame)
        return

      if (Cpp_UI_Dashboard.available) {
        root.firstValidFrame = true
        root.showDashboardNow()
      }

      else if (!app.runtimeMode) {
        root.showConsole()
        root.toolbarVisible = true
        root.firstValidFrame = false
      }
    }
  }

  //
  // Show the dashboard while the benchmark exercises it; reset to console when it ends so
  // repeated runs stay deterministic (no live stream drives the normal onUpdated auto-switch).
  //
  Connections {
    target: Cpp_Benchmark_Runner

    function onDashboardPreviewActive(active) {
      if (app.runtimeMode)
        return

      if (active) {
        root.showDashboardNow()
        if (root.dashboard)
          root.dashboard.loadLayout()
      }

      else {
        root.showConsole()
        root.firstValidFrame = false
      }
    }
  }

  //
  // Handle platform-specific window initialization code
  //
  onVisibilityChanged: {
    if (root.visible)
      Cpp_NativeWindow.addWindow(root, root.nativeCaptionColor)

    else
      Cpp_NativeWindow.removeWindow(root)

    // Auto-hide the toolbar in full-screen with the dashboard active
    if (root.visibility === Window.FullScreen && root.dashboardVisible) {
      if (!root.toolbarAutoHidden) {
        root.toolbarVisibleBeforeFullScreen = root.toolbarVisible
        root.toolbarAutoHidden              = true
        root.toolbarVisible                  = false
      }
    }

    else if (root.toolbarAutoHidden) {
      root.toolbarVisible    = root.toolbarVisibleBeforeFullScreen
      root.toolbarAutoHidden = false
    }
  }

  //
  // Returns true if the active focus item consumes Tab
  //
  function _focusOwnsTab() {
    var item = root.activeFocusItem
    if (!item)
      return false

    // TextInput / TextEdit primitives behind TextField, TextArea, SpinBox, ComboBox editor
    if (item instanceof TextInput || item instanceof TextEdit)
      return true

    // QCodeEditor (QPlainTextEdit wrapped in QQuickWidget)
    if (item.objectName === "QCodeEditor")
      return true

    return false
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
  } Shortcut {
    sequence: "Ctrl+F"
    enabled: root.dashboardVisible && Cpp_UI_TaskbarSettings.searchEnabled
    onActivated: dashboard.focusTaskbarSearch()
  } Shortcut {
    sequence: "Ctrl+M"
    enabled: root.dashboardVisible
    context: Qt.ApplicationShortcut
    onActivated: dashboard.toggleStartMenu()
  } Shortcut {
    sequence: "PgDown"
    enabled: root.dashboardVisible
    onActivated: dashboard.cycleWorkspace(+1)
  } Shortcut {
    sequence: "PgUp"
    enabled: root.dashboardVisible
    onActivated: dashboard.cycleWorkspace(-1)
  } Shortcut {
    enabled: root.dashboardVisible && !root._focusOwnsTab()
    sequences: ["Tab"]
    onActivated: dashboard.cycleWindow(-1)
  } Shortcut {
    enabled: root.dashboardVisible && !root._focusOwnsTab()
    sequences: ["Backtab", "Shift+Tab"]
    onActivated: dashboard.cycleWindow(+1)
  } Shortcut {
    sequence: "Ctrl+Shift+W"
    enabled: root.dashboardVisible
    onActivated: dashboard.closeActiveWindow()
  } Shortcut {
    sequence: "Ctrl+Shift+M"
    enabled: root.dashboardVisible
    onActivated: dashboard.minimizeActiveWindow()
  } Shortcut {
    sequence: "Ctrl+Shift+L"
    enabled: root.dashboardVisible
    onActivated: dashboard.toggleAutoLayout()
  } Shortcut {
    sequence: "Ctrl+Home"
    enabled: root.dashboardVisible
    onActivated: dashboard.clearActiveWindow()
  } Shortcut {
    sequence: "Ctrl+1"
    enabled: root.dashboardVisible
    onActivated: dashboard.jumpToWorkspaceIndex(0)
  } Shortcut {
    sequence: "Ctrl+2"
    enabled: root.dashboardVisible
    onActivated: dashboard.jumpToWorkspaceIndex(1)
  } Shortcut {
    sequence: "Ctrl+3"
    enabled: root.dashboardVisible
    onActivated: dashboard.jumpToWorkspaceIndex(2)
  } Shortcut {
    sequence: "Ctrl+4"
    enabled: root.dashboardVisible
    onActivated: dashboard.jumpToWorkspaceIndex(3)
  } Shortcut {
    sequence: "Ctrl+5"
    enabled: root.dashboardVisible
    onActivated: dashboard.jumpToWorkspaceIndex(4)
  } Shortcut {
    sequence: "Ctrl+6"
    enabled: root.dashboardVisible
    onActivated: dashboard.jumpToWorkspaceIndex(5)
  } Shortcut {
    sequence: "Ctrl+7"
    enabled: root.dashboardVisible
    onActivated: dashboard.jumpToWorkspaceIndex(6)
  } Shortcut {
    sequence: "Ctrl+8"
    enabled: root.dashboardVisible
    onActivated: dashboard.jumpToWorkspaceIndex(7)
  } Shortcut {
    sequence: "Ctrl+9"
    enabled: root.dashboardVisible
    onActivated: dashboard.jumpToWorkspaceIndex(8)
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
      Loader {
        id: toolbarLoader

        z: 2
        Layout.fillWidth: true
        active: !app.runtimeMode

        property int titlebarHeight: Cpp_NativeWindow.titlebarHeight(root)
        property int slotHeight: active
                                 ? titlebarHeight + (root.toolbarBarShown ? 64 + 16 : 0)
                                 : 0

        Layout.minimumHeight: slotHeight
        Layout.maximumHeight: slotHeight

        Behavior on slotHeight {
          NumberAnimation {
            duration: 250
            easing.type: Easing.OutCubic
          }
        }

        Connections {
          target: root
          function onVisibilityChanged() {
            toolbarLoader.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
          }
        }

        sourceComponent: Panes.Toolbar {
          toolbarEnabled: root.toolbarVisible
          dashboardVisible: root.dashboardVisible
          autoHide: Cpp_UI_Dashboard.autoHideToolbar
        }
      }

      //
      // Main layout
      //
      StackView {
        id: stack

        z: 1
        Layout.fillWidth: true
        Layout.fillHeight: true
        initialItem: app.runtimeMode ? dashboardLoader : consoleSetupLoader
        Layout.minimumWidth: consoleSetupLoader.item ? consoleSetupLoader.item.contentMinWidth : 0
        Layout.minimumHeight: consoleSetupLoader.item ? consoleSetupLoader.item.contentMinHeight : 0
        Layout.topMargin: (root.visibility === Window.FullScreen || !root.toolbarBarShown) ? -6 : -1

        data: [
          Loader {
            id: consoleSetupLoader

            active: !app.runtimeMode
            width: parent ? parent.width : 0
            height: parent ? parent.height : 0

            sourceComponent: RowLayout {
              id: consoleSetupPage

              spacing: 0

              readonly property int contentMinWidth: terminal.implicitWidth + 1 + setup.displayedWidth
              readonly property int contentMinHeight: Math.max(terminal.implicitHeight, setup.implicitHeight)

              Panes.Console {
                id: terminal

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: terminal.implicitWidth
              }

              Rectangle {
                z: 2
                implicitWidth: 1
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

                  property int dragStartX: 0
                  property int dragStartWidth: 0

                  onPressed: (mouse) => {
                               dragStartX = mouse.x
                               dragStartWidth = setup.width
                             }

                  onPositionChanged: (mouse) => {
                                       if (!pressed)
                                       return

                                       const delta = dragStartX - mouse.x
                                       const maxW = consoleSetupPage.width
                                                    - terminal.Layout.minimumWidth - 1
                                       const newW = Math.max(setup.kMinPaneWidth,
                                                             Math.min(dragStartWidth + delta, maxW))
                                       setup.userPaneWidth = newW
                                     }
                }
              }

              Panes.Setup {
                id: setup

                readonly property int kMaxPaneWidth: 720

                Layout.fillHeight: true
                Layout.minimumWidth: setup.kMinPaneWidth
                Layout.maximumWidth: setup.kMaxPaneWidth
                Layout.preferredWidth: setup.displayedWidth
              }
            }
          },

          Loader {
            id: dashboardLoader

            active: false
            width: parent ? parent.width : 0
            height: parent ? parent.height : 0

            sourceComponent: Panes.Dashboard {}
          }
        ]
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
    // Runtime-mode "Connecting..." overlay shown until the first valid frame.
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
