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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio.UI as SS_UI

import "Dashboard" as DbItems
import "../../Widgets" as Widgets
import "../../Dialogs" as Dialogs

Widgets.Pane {
  id: root
  title: qsTr("Dashboard")
  headerVisible: mainWindow.toolbarVisible
  icon: "qrc:/rcc/icons/panes/dashboard.svg"

  //
  // Autolayout
  //
  function loadLayout() {
    canvas.windowManager.loadLayout()
  }

  //
  // Taskbar helper
  //
  property SS_UI.TaskBar taskBar: SS_UI.TaskBar {}

  //
  // Default background
  //
  Rectangle {
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -9
    anchors.rightMargin: -9
    anchors.bottomMargin: -9
    color: Cpp_ThemeManager.colors["dashboard_background"]
  }

  //
  // User-selected background image
  //
  Image {
    visible: source !== ""
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -9
    anchors.rightMargin: -9
    anchors.bottomMargin: -9
    source: canvas.backgroundImage
    fillMode: Image.PreserveAspectCrop
  }

  //
  // Desktop layout
  //
  ColumnLayout {
    spacing: -1
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -9
    anchors.rightMargin: -9
    anchors.bottomMargin: -9

    //
    // Actions rectangle
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
      visible: Cpp_UI_Dashboard.actionCount > 0 && Cpp_UI_Dashboard.showActionPanel

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
          enabled: !Cpp_IO_Manager.paused && Cpp_IO_Manager.isConnected
          onClicked: Cpp_UI_Dashboard.activateAction(model["id"], true)
        }
      }
    }

    //
    // Widget windows
    //
    DbItems.Canvas {
      id: canvas
      taskBar: root.taskBar
      Layout.topMargin: -1
      Layout.leftMargin: -1
      Layout.rightMargin: -1
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 480
    }

    //
    // Task bar
    //
    DbItems.Taskbar {
      id: _taskBar
      taskBar: root.taskBar
      Layout.fillWidth: true
      onStartClicked: {
        if (startMenu.visible)
          startMenu.close()
        else
          startMenu.open()
      }
    }
  }

  //
  // Start menu
  //
  DbItems.StartMenu {
    id: startMenu
    x: -9
    taskBar: root.taskBar
    y: root.height - height - _taskBar.height - root.topPadding + 2
  }
}
