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

import QtQuick
import QtQuick.Window
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import SerialStudio.UI as SS_Ui

import "../../../Widgets" as Widgets
import "../../../Dialogs" as Dialogs

Widgets.Pane {
  id: root

  title: qsTr("Dashboard")
  icon: "qrc:/icons/panes/dashboard.svg"
  headerVisible: mainWindow.toolbarVisible && !isExternalWindow

  //
  // Required data inputs
  //
  required property SS_Ui.TaskBar taskBar

  //
  // Custom properties
  //
  property bool isExternalWindow: false
  readonly property bool operatorMode: !isExternalWindow && app.runtimeMode
  readonly property bool zeroBottom: operatorMode && Cpp_UI_TaskbarSettings.taskbarHidden

  //
  // Signals
  //
  signal externalWindowClicked()

  //
  // Public function to trigger auto-layout
  //
  function loadLayout() {
    _canvas.windowManager.loadLayout()
  }

  //
  // Force auto-layout while taskbar is hidden -- no controls to escape manual overlap
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
  function focusTaskbarSearch()   { _taskbar.focusSearch() }
  function toggleStartMenu()      { _taskbar.toggleStartMenu() }
  function cycleWorkspace(delta)  { _taskbar.cycleWorkspace(delta) }

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
    if (taskBar.activeWindow)
      taskBar.closeWindow(taskBar.activeWindow)
  }

  function minimizeActiveWindow() {
    if (taskBar.activeWindow)
      taskBar.minimizeWindow(taskBar.activeWindow)
  }

  function clearActiveWindow() {
    taskBar.activeWindow = null
  }

  function toggleAutoLayout() {
    taskBar.windowManager.autoLayoutEnabled = !taskBar.windowManager.autoLayoutEnabled
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
  // API Server status indicator
  //
  actionComponent: Component {
    Item {
      opacity: Cpp_API_Server.enabled ?
                 (Cpp_API_Server.clientCount > 0 ? 1 : 0.5) :
                 0.0
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
        visible: Cpp_API_Server.enabled && Cpp_API_Server.clientCount > 0
        shadowColor: Cpp_API_Server.enabled ? Cpp_ThemeManager.colors["highlight"] :
                                              Cpp_ThemeManager.colors["pane_caption_border"]

        SequentialAnimation on opacity {
          loops: Animation.Infinite
          running: Cpp_API_Server.enabled && Cpp_API_Server.clientCount > 0

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
          running: Cpp_API_Server.enabled && Cpp_API_Server.clientCount > 0

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
        text: Cpp_API_Server.enabled ?
                (Cpp_API_Server.clientCount > 0 ? qsTr("API Server Active (%1)").arg(Cpp_API_Server.clientCount) :
                                                  qsTr("API Server Ready")) :
                qsTr("API Server Off")
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
    anchors.topMargin: zeroBottom ? -18 : -16
    anchors.leftMargin: zeroBottom ? -14 : -9
    anchors.rightMargin: zeroBottom ? -14 : -9
    anchors.bottomMargin: zeroBottom ? -14 : -9

    //
    // Default background
    //
    Rectangle {
      anchors.fill: parent
      color: Cpp_ThemeManager.colors["dashboard_background"]
    }

    //
    // User-selected background image
    //
    Image {
      mipmap: true
      smooth: true
      anchors.fill: parent
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

        Layout.topMargin: -1
        taskBar: root.taskBar
        taskbarView: _taskbar
        Layout.leftMargin: -1
        Layout.rightMargin: -1
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumWidth: 480
      }

      //
      // Taskbar (with autohide wrapper)
      //
      Item {
        id: _taskbarHost

        Layout.fillWidth: true
        Layout.preferredHeight: hidden ? 0 : (revealed ? _taskbar.implicitHeight : 0)
        //
        // Slide the layout slot open/closed
        //
        Behavior on Layout.preferredHeight {
          NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
        }
        clip: true
        visible: !hidden

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

          //
          // Anchor to the bottom of the host
          //
          y: parent.height - height

          opacity: Cpp_UI_Dashboard.available ? 1 : 0
          Behavior on opacity {
            NumberAnimation { duration: 350; easing.type: Easing.OutCubic }
          }

          onNewWorkspaceRequested: _wsDialog.openNew(root.taskBar)
          onEditWorkspaceRequested: (wsId, name) => _wsDialog.openEdit(root.taskBar, wsId, name)
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
      isExternalWindow: root.isExternalWindow
      y: parent.height - height - _taskbar.height + 1
      onExternalWindowClicked: root.externalWindowClicked()
      onNewWorkspaceRequested: _wsDialog.openNew(root.taskBar)
      onRenameWorkspaceRequested: (wsId, name) => _wsDialog.openEdit(root.taskBar, wsId, name)
    }
  }

  //
  // Shared workspace dialog (accessible from Taskbar + StartMenu)
  //
  Dialogs.WorkspaceDialog {
    id: _wsDialog
  }
}
