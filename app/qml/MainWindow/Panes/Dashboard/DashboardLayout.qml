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
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import SerialStudio.UI as SS_Ui

import "../../../Widgets" as Widgets

Widgets.Pane {
  id: root
  title: qsTr("Dashboard")
  icon: "qrc:/rcc/icons/panes/dashboard.svg"
  headerVisible: mainWindow.toolbarVisible && !isExternalWindow

  //
  // Required data inputs
  //
  required property SS_Ui.TaskBar taskBar

  //
  // Custom properties
  //
  property bool isExternalWindow: false

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
  // API Server status indicator
  //
  actionComponent: Component {
    Item {
      implicitWidth: label.implicitWidth
      implicitHeight: label.implicitHeight
      opacity: Cpp_API_Server.enabled ?
                 (Cpp_API_Server.clientCount > 0 ? 1 : 0.5) :
                 0.0
      Behavior on opacity { NumberAnimation { duration: 200 } }

      MultiEffect {
        id: glow
        source: label
        shadowBlur: 2.0
        anchors.fill: label
        shadowEnabled: true
        shadowVerticalOffset: 0
        shadowHorizontalOffset: 0
        visible: Cpp_API_Server.enabled && Cpp_API_Server.clientCount > 0
        shadowColor: Cpp_API_Server.enabled ? Cpp_ThemeManager.colors["highlight"] :
                                              Cpp_ThemeManager.colors["pane_caption_border"]

        SequentialAnimation on opacity {
          loops: Animation.Infinite
          running: Cpp_API_Server.enabled && Cpp_API_Server.clientCount > 0

          NumberAnimation {
            from: 0.4
            to: 1.00
            duration: 800
            easing.type: Easing.InOutSine
          }
          NumberAnimation {
            from: 1.00
            to: 0.4
            duration: 800
            easing.type: Easing.InOutSine
          }
        }

        SequentialAnimation on brightness {
          loops: Animation.Infinite
          running: Cpp_API_Server.enabled && Cpp_API_Server.clientCount > 0

          NumberAnimation {
            from: 0.15
            to: 0.6
            duration: 800
            easing.type: Easing.InOutSine
          }
          NumberAnimation {
            from: 0.6
            to: 0.15
            duration: 800
            easing.type: Easing.InOutSine
          }
        }
      }

      Label {
        id: label
        visible: opacity > 0
        font: Cpp_Misc_CommonFonts.customUiFont(0.85, true)
        color: Cpp_ThemeManager.colors["pane_caption_foreground"]
        text: Cpp_API_Server.enabled ?
                (Cpp_API_Server.clientCount > 0 ? qsTr("API Server Active (%1)").arg(Cpp_API_Server.clientCount) :
                                                  qsTr("API Server Ready")) :
                qsTr("API Server Off")
      }
    }
  }

  //
  // Container
  //
  Item {
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -9
    anchors.rightMargin: -9
    anchors.bottomMargin: -9

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
      visible: source !== ""
      anchors.fill: parent
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
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]
        visible: Cpp_UI_Dashboard.actionCount > 0
                 && Cpp_UI_Dashboard.showActionPanel

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
            maxButtonWidth: 256
            text: model["text"]
            toolbarButton: false
            horizontalLayout: true
            checked: model["checked"]
            icon.source: model["icon"]
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
        taskBar: root.taskBar
        Layout.topMargin: -1
        Layout.leftMargin: -1
        Layout.rightMargin: -1
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumWidth: 480
      }

      //
      // Taskbar
      //
      Taskbar {
        id: _taskbar
        taskBar: root.taskBar
        Layout.fillWidth: true
        onStartClicked: {
          if (_startMenu.visible)
            _startMenu.close()
          else
            _startMenu.open()
        }
      }
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
    }
  }
}
