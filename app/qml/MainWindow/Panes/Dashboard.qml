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
    DbItems.DashboardCanvas {
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
