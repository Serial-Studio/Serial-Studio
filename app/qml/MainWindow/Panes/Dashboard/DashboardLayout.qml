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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Window
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import SerialStudio.UI as SS_Ui

import "../../../Widgets" as Widgets
import "../../../Dialogs" as Dialogs
import "../../../Commands" as Commands

Widgets.Pane {
  id: root

  title: qsTr("Dashboard")
  icon: Cpp_Misc_IconRegistry.icon("panes", "dashboard", 16)
  headerVisible: mainWindow.toolbarBarShown && !isExternalWindow && !app.runtimeMode

  //
  // Required data inputs
  //
  required property SS_Ui.TaskBar taskBar

  //
  // Custom properties
  //
  property var hostWindow: null
  property bool isExternalWindow: false
  property bool widgetsStayOnTop: false
  property alias paletteModel: _paletteModel
  readonly property bool operatorMode: !isExternalWindow && app.runtimeMode
  readonly property bool zeroBottom: operatorMode && Cpp_UI_TaskbarSettings.taskbarHidden

  //
  // Signals
  //
  signal externalWindowClicked()
  signal widgetWindowRequested(int windowId)

  //
  // Opens a widget pop-out window hosted by this layout's canvas
  //
  function openWidgetWindow(windowId) {
    _canvas.openExternalWidgetWindow(windowId)
  }

  //
  // Public function to trigger auto-layout
  //
  function loadLayout() {
    _canvas.windowManager.loadLayout()
  }

  //
  // Force auto-layout while taskbar is hidden (no controls to escape manual overlap)
  //
  Connections {
    target: Cpp_UI_TaskbarSettings
    function onTaskbarHiddenChanged() {
      if (Cpp_UI_TaskbarSettings.taskbarHidden && taskBar && taskBar.windowManager)
        taskBar.windowManager.autoLayoutEnabled = true
    }
  }
  Connections {
    target: taskBar && taskBar.windowManager ? taskBar.windowManager : null
    function onAutoLayoutEnabledChanged() {
      if (Cpp_UI_TaskbarSettings.taskbarHidden
          && taskBar.windowManager
          && !taskBar.windowManager.autoLayoutEnabled)
        taskBar.windowManager.autoLayoutEnabled = true
    }
  }
  Component.onCompleted: {
    if (Cpp_UI_TaskbarSettings.taskbarHidden && taskBar && taskBar.windowManager)
      taskBar.windowManager.autoLayoutEnabled = true
  }

  //
  // Shortcut hooks
  //
  function focusTaskbarSearch()    { _taskbar.focusSearch() }
  function toggleStartMenu()       { _taskbar.toggleStartMenu() }
  function cycleWorkspace(delta)   { _taskbar.cycleWorkspace(delta) }

  //
  // Only external windows own a palette overlay; the main dashboard defers to the main
  // window's single command palette (spec 0028 unification).
  //
  signal paletteRequested()
  function openWorkspaceSwitcher() {
    if (root.isExternalWindow && _switcherLoader.item)
      _switcherLoader.item.toggle()
    else
      root.paletteRequested()
  }

  function cycleWindow(delta) {
    const next = taskBar.nextActiveWindow(delta)
    if (!next)
      return

    if (taskBar.windowState(next) !== SS_Ui.TaskbarModel.WindowNormal)
      taskBar.showWindow(next)

    taskBar.activeWindow = next
    taskBar.windowManager.bringToFront(next)
  }

  function closeActiveWindow() {
    if (Cpp_UI_Dashboard.frozen)
      return

    if (taskBar.activeWindow)
      taskBar.closeWindow(taskBar.activeWindow)
  }

  function minimizeActiveWindow() {
    if (Cpp_UI_Dashboard.frozen)
      return

    if (taskBar.activeWindow)
      taskBar.minimizeWindow(taskBar.activeWindow)
  }

  function clearActiveWindow() {
    taskBar.activeWindow = null
  }

  function toggleAutoLayout() {
    if (Cpp_UI_Dashboard.frozen)
      return

    taskBar.windowManager.autoLayoutEnabled = !taskBar.windowManager.autoLayoutEnabled
  }

  function toggleFreeze() {
    const target = !Cpp_UI_Dashboard.frozen
    Cpp_UI_Dashboard.setFrozen(target)
    if (target && !Cpp_UI_Dashboard.frozen)
      app.showLicenseDialog()
  }

  function jumpToWorkspaceIndex(index) {
    if (!taskBar)
      return

    const list = taskBar.workspaceModel
    if (!list || index < 0 || index >= list.length)
      return

    taskBar.activeGroupIndex = index
  }

  //
  // API server / remote attach status indicator
  //
  actionComponent: Component {
    Item {
      id: indicator

      readonly property bool mirrored: Cpp_API_Mirror.attached
      readonly property bool serving: Cpp_API_Server.enabled && Cpp_API_Server.clientCount > 0
      readonly property bool pulsing: indicator.mirrored ? Cpp_API_Mirror.live : indicator.serving

      opacity: indicator.mirrored ? 1.0 :
                 (Cpp_API_Server.enabled ? (indicator.serving ? 1 : 0.5) : 0.0)
      implicitWidth: label.implicitWidth
      implicitHeight: label.implicitHeight
      Behavior on opacity { NumberAnimation { duration: 200 } }

      MultiEffect {
        id: glow

        source: label
        shadowBlur: 2.0
        shadowEnabled: true
        anchors.fill: label
        shadowVerticalOffset: 0
        shadowHorizontalOffset: 0
        enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
        visible: Cpp_Misc_GraphicsBackend.effectsEnabled && indicator.pulsing
        shadowColor: (indicator.mirrored || Cpp_API_Server.enabled) ?
                       Cpp_ThemeManager.colors["highlight"] :
                       Cpp_ThemeManager.colors["pane_caption_border"]

        SequentialAnimation on opacity {
          loops: Animation.Infinite
          running: indicator.pulsing

          NumberAnimation {
            to: 1.00
            from: 0.4
            duration: 800
            easing.type: Easing.InOutSine
          }
          NumberAnimation {
            to: 0.4
            from: 1.00
            duration: 800
            easing.type: Easing.InOutSine
          }
        }

        SequentialAnimation on brightness {
          loops: Animation.Infinite
          running: indicator.pulsing

          NumberAnimation {
            to: 0.6
            from: 0.15
            duration: 800
            easing.type: Easing.InOutSine
          }
          NumberAnimation {
            to: 0.15
            from: 0.6
            duration: 800
            easing.type: Easing.InOutSine
          }
        }
      }

      Label {
        id: label

        visible: opacity > 0
        text: {
          if (indicator.mirrored) {
            if (Cpp_API_Mirror.stale)
              return qsTr("Remote %1 - Stale").arg(Cpp_API_Mirror.endpoint)

            if (Cpp_API_Mirror.live)
              return qsTr("Remote %1 - Live").arg(Cpp_API_Mirror.endpoint)

            return qsTr("Remote %1 - No Data").arg(Cpp_API_Mirror.endpoint)
          }

          if (!Cpp_API_Server.enabled)
            return qsTr("API Server Off")

          if (Cpp_API_Server.clientCount > 0)
            return qsTr("API Server Active (%1)").arg(Cpp_API_Server.clientCount)

          return qsTr("API Server Ready")
        }
        font: Cpp_Misc_CommonFonts.customUiFont(0.85, true)
        color: Cpp_ThemeManager.colors["pane_caption_foreground"]
      }
    }
  }

  //
  // Container
  //
  Item {
    anchors.fill: parent
    anchors.leftMargin: -9
    anchors.rightMargin: -9
    anchors.bottomMargin: (zeroBottom ? -14 : -9)
    anchors.topMargin: (headerVisible ? -16 : -9)

    //
    // Default background
    //
    Rectangle {
      anchors.fill: parent
      anchors.topMargin: -16
      color: Cpp_ThemeManager.colors["dashboard_background"]
    }

    //
    // User-selected background image
    //
    Image {
      mipmap: true
      smooth: true
      anchors.fill: parent
      anchors.topMargin: -16
      visible: source !== ""
      source: _canvas.backgroundImage
      fillMode: Image.PreserveAspectCrop
    }

    //
    // Desktop layout
    //
    ColumnLayout {
      spacing: -1
      implicitHeight: 0
      anchors.fill: parent

      //
      // Operator-mode titlebar strip (clears macOS traffic lights, draws title)
      //
      Rectangle {
        id: operatorTitlebar

        z: 0
        Layout.fillWidth: true
        visible: operatorMode && titlebarHeight > 0
        color: Cpp_ThemeManager.colors["dashboard_background"]
        Layout.preferredHeight: visible ? titlebarHeight : 0

        property int titlebarHeight: operatorMode
                                     ? Cpp_NativeWindow.titlebarHeight(mainWindow)
                                     : 0

        Connections {
          target: mainWindow
          function onVisibilityChanged() {
            operatorTitlebar.titlebarHeight = operatorMode
                                              ? Cpp_NativeWindow.titlebarHeight(mainWindow)
                                              : 0
          }
        }

        Label {
          text: mainWindow.title
          anchors.centerIn: parent
          anchors.verticalCenterOffset: -4
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)
        }

        DragHandler {
          target: null
          onActiveChanged: {
            if (active)
              mainWindow.startSystemMove()
          }
        }
      }

      //
      // Actions panel
      //
      Rectangle {
        z: 1000
        border.width: 1
        Layout.topMargin: -1
        Layout.leftMargin: -1
        Layout.rightMargin: -1
        Layout.fillWidth: true
        implicitHeight: _actions.implicitHeight + 20
        visible: Cpp_UI_Dashboard.actionCount > 0
                 && Cpp_UI_Dashboard.showActionPanel
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        ListView {
          id: _actions

          spacing: 2
          interactive: true
          implicitHeight: 32
          model: Cpp_UI_Dashboard.actions
          orientation: ListView.Horizontal

          anchors {
            leftMargin: 8
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          delegate: Widgets.ToolbarButton {
            required property var model

            iconSize: 24
            implicitHeight: 32
            text: model["text"]
            maxButtonWidth: 256
            toolbarButton: false
            horizontalLayout: true
            icon.source: model["icon"]
            checked: model["checked"]
            enabled: !Cpp_IO_Manager.paused
                     && Cpp_IO_Manager.isConnected
            onClicked: Cpp_UI_Dashboard.activateAction(
                         model["id"], true)
          }
        }
      }

      //
      // Widget canvas
      //
      DashboardCanvas {
        id: _canvas

        z: 1
        Layout.topMargin: -1
        taskBar: root.taskBar
        taskbarView: _taskbar
        Layout.leftMargin: -1
        Layout.rightMargin: -1
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumWidth: 480
        isExternalWindow: root.isExternalWindow
        widgetsStayOnTop: root.widgetsStayOnTop
        onExternalWidgetWindowRequested: (windowId) => root.widgetWindowRequested(windowId)
      }

      //
      // Taskbar (with autohide wrapper)
      //
      Item {
        id: _taskbarHost

        z: 2000
        clip: true
        visible: !hidden
        Layout.fillWidth: true
        Layout.preferredHeight: hidden ? 0 : (revealed ? _taskbar.implicitHeight : 0)

        Behavior on Layout.preferredHeight {
          NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
        }

        property bool hovered: false
        property bool revealed: !autohide
        readonly property bool busy: _taskbar.isBusy
        readonly property bool hidden: Cpp_UI_TaskbarSettings.taskbarHidden
        readonly property bool autohide: Cpp_UI_TaskbarSettings.autohide && !hidden

        //
        // Reset visibility when autohide flips
        //
        onAutohideChanged: revealed = !autohide

        //
        // Inactivity timer
        //
        Timer {
          id: _hideTimer

          repeat: false
          interval: Cpp_UI_TaskbarSettings.autohideDelayMs
          onTriggered: {
            if (_taskbarHost.autohide && !_taskbarHost.hovered && !_taskbarHost.busy)
              _taskbarHost.revealed = false
          }
        }

        function poke() {
          if (!autohide)
            return

          revealed = true
          _hideTimer.restart()
        }

        //
        // Re-arm whenever the user hovers or the busy state clears
        //
        onHoveredChanged: {
          if (autohide && hovered) {
            revealed = true
            _hideTimer.stop()
          } else if (autohide && !hovered) {
            _hideTimer.restart()
          }
        }

        onBusyChanged: {
          if (autohide && busy) {
            revealed = true
            _hideTimer.stop()
          } else if (autohide && !busy) {
            _hideTimer.restart()
          }
        }

        Taskbar {
          id: _taskbar

          width: parent.width
          taskBar: root.taskBar
          startMenu: _startMenu
          paletteModel: _paletteModel

          //
          // Anchor to the bottom of the host
          //
          y: parent.height - height

          visible: opacity > 0
          opacity: Cpp_UI_Dashboard.available ? 1 : 0
          Behavior on opacity {
            NumberAnimation { duration: 350; easing.type: Easing.OutCubic }
          }

          onEditWorkspaceRequested: (wsId, name) => _wsDialog.openEdit(root.taskBar, wsId, name)
          onWorkspaceSwitcherRequested: root.openWorkspaceSwitcher()
          onStartClicked: {
            if (_startMenu.visible)
              _startMenu.close()
            else
              _startMenu.open()
          }
        }

        // Mouse tracking over the taskbar (HoverHandler avoids stealing button hover)
        HoverHandler {
          id: _taskbarHover

          target: _taskbar
          onHoveredChanged: {
            _taskbarHost.hovered = hovered
            if (hovered)
              _taskbarHost.poke()
          }
        }
      }
    }

    //
    // 12 px hot-zone for taskbar autohide reveal
    //
    MouseArea {
      id: _autohideHotZone

      hoverEnabled: true
      acceptedButtons: Qt.NoButton

      anchors {
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }
      height: 12

      visible: Cpp_UI_TaskbarSettings.autohide
               && !Cpp_UI_TaskbarSettings.taskbarHidden
               && !_taskbarHost.revealed
      onEntered: _taskbarHost.poke()
    }

    //
    // Start menu
    //
    StartMenu {
      id: _startMenu

      taskBar: root.taskBar
      hostWindow: root.hostWindow
      isExternalWindow: root.isExternalWindow
      y: parent.height - height - _taskbar.height + 1
      x: Cpp_Misc_Translator.rtl ? parent.width - width : 0
      onExternalWindowClicked: root.externalWindowClicked()
      onNewWorkspaceRequested: _wsDialog.openNew(root.taskBar)
      onRenameWorkspaceRequested: (wsId, name) => _wsDialog.openEdit(root.taskBar, wsId, name)
      onFullScreenRequested: {
        if (root.hostWindow)
          root.hostWindow.toggleFullScreen()
      }
    }
  }

  //
  // Command palette (opened via Ctrl+K), backed by the shared dashboard-context model.
  //
  Commands.DashboardCommandBindings {
    id: _dashBindings

    taskBar: root.taskBar
    hostWindow: root.hostWindow
    onExternalWindowRequested: root.externalWindowClicked()
    onFullScreenRequested: {
      if (root.hostWindow)
        root.hostWindow.toggleFullScreen()
    }
  }

  //
  // App-level commands (Project Editor, Deploy, Extensions...) shown in the palette when
  // connected; the dashboard bindings take precedence for the ids both sets define.
  //
  Commands.AppCommandBindings {
    id: _dashAppBindings

    dashboard: root
    dashboardVisible: true
  }

  Commands.CommandModel {
    id: _paletteTools

    context: "dashboard"
    bindingSets: [_dashBindings, _dashAppBindings]
  }

  //
  // The main dashboard's model is consumed by the main window's palette (see
  // `paletteModel` alias); external windows render it through their own overlay below.
  //
  PaletteModel {
    id: _paletteModel

    host: root
    taskBar: root.taskBar
    workspacesEnabled: true
    toolActions: _paletteTools
  }

  //
  // Registry palette shortcut fires here too: MainWindow only instantiates its own
  // window-scoped shortcuts, so pop-out dashboards must wire Ctrl+K themselves.
  //
  Shortcut {
    context: Qt.WindowShortcut
    enabled: root.isExternalWindow
    sequences: Cpp_UI_CommandRegistry.command("palette.open").sequences
    onActivated: root.openWorkspaceSwitcher()
  }

  Loader {
    id: _switcherLoader

    active: root.isExternalWindow
    sourceComponent: Widgets.CommandPalette {
      z: 5000
      model: _paletteModel
      title: qsTr("Command Palette")
      titleIcon: "qrc:/icons/buttons/workspaces.svg"

      //
      // Parented to the host window so the dialog centers on the whole window, not the Loader.
      //
      parent: root.hostWindow ? root.hostWindow.contentItem : root
    }
  }

  //
  // Shared workspace dialog (accessible from Taskbar + StartMenu)
  //
  Dialogs.WorkspaceDialog {
    id: _wsDialog
  }
}
