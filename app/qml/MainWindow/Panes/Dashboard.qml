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
  // Workspace switcher
  //
  actionComponent: Component {
    ComboBox {
      id: control
      textRole: "text"
      model: taskBar.groupModel
      currentIndex: taskBar.activeGroupIndex
      onCurrentIndexChanged: {
        if (currentIndex !== taskBar.activeGroupIndex)
          taskBar.activeGroupIndex = currentIndex
      }

      indicator: Item {}

      background: Rectangle {
        color: "transparent"
        border.width: 0
      }

      delegate: ItemDelegate {
        width: control.width
        required property var model
        contentItem: RowLayout {
          spacing: 8
          anchors.verticalCenter: parent.verticalCenter
          Component.onCompleted: {
            var itemWidth = Math.min(480, implicitWidth + 32)
            if (control.implicitWidth < itemWidth)
              control.implicitWidth = itemWidth
          }

          Image {
            source: model["icon"]
            sourceSize: Qt.size(16, 16)
            fillMode: Image.PreserveAspectFit
          }

          Label {
            text: model["text"]
            elide: Text.ElideRight
            Layout.fillWidth: true
            verticalAlignment: Text.AlignVCenter
            font: text === control.currentText
                  ? Cpp_Misc_CommonFonts.boldUiFont
                  : Cpp_Misc_CommonFonts.uiFont
          }
        }
      }

      contentItem: RowLayout {
        spacing: 4
        anchors.verticalCenter: parent.verticalCenter

        Label {
          Layout.fillWidth: true
          text: control.currentText
          horizontalAlignment: Text.AlignRight
          verticalAlignment: Text.AlignVCenter
          font: Cpp_Misc_CommonFonts.boldUiFont
          color: Cpp_ThemeManager.colors["pane_caption_foreground"]
        }

        Canvas {
          width: 18
          height: 18
          opacity: 0.8
          Layout.alignment: Qt.AlignVCenter

          onPaint: {
            const ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            ctx.fillStyle = Cpp_ThemeManager.colors["pane_caption_foreground"];

            const spacing = 2;
            const triangleWidth = 8;
            const triangleHeight = 4;

            const centerX = width / 2;
            const totalHeight = triangleHeight * 2 + spacing;
            const topY = (height - totalHeight) / 2;
            const downTopY = topY + triangleHeight + spacing;

            // Up Triangle
            ctx.beginPath();
            ctx.moveTo(centerX, topY);
            ctx.lineTo(centerX - triangleWidth / 2, topY + triangleHeight);
            ctx.lineTo(centerX + triangleWidth / 2, topY + triangleHeight);
            ctx.closePath();
            ctx.fill();

            // Down Triangle
            ctx.beginPath();
            ctx.moveTo(centerX, downTopY + triangleHeight);
            ctx.lineTo(centerX - triangleWidth / 2, downTopY);
            ctx.lineTo(centerX + triangleWidth / 2, downTopY);
            ctx.closePath();
            ctx.fill();
          }
        }
      }
    }
  }

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
