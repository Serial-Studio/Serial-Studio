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
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio.UI as SS_UI

import "Dashboard" as DbItems
import "../../Widgets" as Widgets

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
  // Custom properties
  //
  property bool isExternalWindow: false

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

    /*Image {
      opacity: 0.5
      anchors.centerIn: parent
      source: "qrc:/rcc/images/banner.png"
    }*/
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
    isExternalWindow: root.isExternalWindow
    y: root.height - height - _taskBar.height - root.topPadding + 2
    onExternalWindowClicked: {
      var dialog = Qt.createQmlObject('
        import QtQuick
        import QtQuick.Controls

        Window {
          id: externalWindow

          visible: true
          minimumWidth: 640
          minimumHeight: 480
          title: qsTr("Dashboard")

          Connections {
            target: Cpp_IO_Manager
            function onConnectedChanged() {
                if (!Cpp_IO_Manager.isConnected)
                    externalWindow.close()
            }
          }

          Loader {
            id: loader
            anchors.fill: parent
            source: "qrc:/serial-studio.com/gui/qml/MainWindow/Panes/Dashboard.qml"

            onLoaded: {
              if (item) {
                item.headerVisible = false
                item.isExternalWindow = true
              }

              else
                console.error("Dashboard.qml loaded, but item is null.")
            }

            onStatusChanged: {
              if (status === Loader.Error)
                console.error("Failed to load Dashboard.qml:", loader.errorString())
            }
          }
        }
        ', root, "ExternalDashboardWindow")
    }
  }
}
