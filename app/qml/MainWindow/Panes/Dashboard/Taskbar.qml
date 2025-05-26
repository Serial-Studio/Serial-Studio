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

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import SerialStudio.UI as SS_Ui

import "../../../Widgets" as Widgets

Item {
  id: root
  implicitHeight: 30

  //
  // Custom properties
  //
  readonly property int iconSize: 18
  required property SS_Ui.TaskBar taskBar

  //
  // Signals
  //
  signal startClicked()
  signal settingsClicked()
  signal extendWindowClicked()

  //
  // Taskbar background
  //
  Rectangle {
    anchors.fill: parent
    gradient: Gradient {
      GradientStop {
        position: 0
        color: Cpp_ThemeManager.colors["taskbar_top"]
      }

      GradientStop {
        position: 1
        color: Cpp_ThemeManager.colors["taskbar_bottom"]
      }
    }
  }

  //
  // Taskbar components
  //
  RowLayout {
    spacing: 2
    anchors {
      left: parent.left
      right: parent.right
      leftMargin: 2
      rightMargin: 2
      verticalCenter: parent.verticalCenter
    }

    //
    // Start Menu
    //
    Widgets.TaskbarButton {
      id: start
      iconSize: 16
      startMenu: true
      text: qsTr("Menu")
      onClicked: root.startClicked()
      implicitWidth: start.layout.implicitWidth + 8
      icon.source: Cpp_Misc_Utilities.hdpiImagePath("qrc:/rcc/logo/start.png")
    }

    //
    // Taskbar Buttons
    //
    Item {
      id: buttonsContainer
      implicitHeight: 24
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignVCenter

      property bool showNavButtons: buttonsView.contentWidth > buttonsView.width

      RowLayout {
        anchors.fill: parent
        spacing: 4

        Button {
          icon.width: 24
          icon.height: 24
          background: Item{}
          Layout.preferredWidth: 24
          Layout.preferredHeight: 24
          visible: buttonsContainer.showNavButtons
          icon.source: "qrc:/rcc/icons/buttons/backward.svg"
          icon.color: Cpp_ThemeManager.colors["taskbar_text"]
          onClicked: buttonsView.contentX = Math.max(0, buttonsView.contentX - 150)
        }

        ListView {
          id: buttonsView
          clip: true
          spacing: 2
          Layout.fillWidth: true
          Layout.fillHeight: true
          model: taskBar.taskbarButtons
          orientation: ListView.Horizontal
          interactive: true
          boundsBehavior: Flickable.StopAtBounds

          delegate: Widgets.TaskbarButton {
            required property var model

            id: button
            width: 144
            text: model.widgetName
            icon.source: SerialStudio.dashboardWidgetIcon(model.widgetType)

            Component.onCompleted: updateState()

            onClicked: {
              const window = taskBar.windowData(model.windowId)
              if (window !== null) {
                if (taskBar.windowState(window) !== SS_Ui.TaskbarModel.WindowNormal)
                  taskBar.showWindow(window)
                else if (taskBar.activeWindow === window)
                  taskBar.minimizeWindow(window)

                taskBar.activeWindow = window
              }
            }

            function updateState() {
              const window = taskBar.windowData(model.windowId)
              if (window !== null) {
                let state = taskBar.windowState(window)
                button.open      = (state !== SS_Ui.TaskbarModel.WindowClosed)
                button.minimized = (state === SS_Ui.TaskbarModel.WindowMinimized)
                button.focused   = (state === SS_Ui.TaskbarModel.WindowNormal && taskBar.activeWindow === window)
              }
            }

            Connections {
              target: taskBar
              function onActiveWindowChanged() { button.updateState() }
              function onWindowStatesChanged() { button.updateState() }
            }
          }
        }

        Button {
          icon.width: 24
          icon.height: 24
          background: Item{}
          Layout.preferredWidth: 24
          Layout.preferredHeight: 24
          Layout.alignment: Qt.AlignVCenter
          visible: buttonsContainer.showNavButtons
          icon.source: "qrc:/rcc/icons/buttons/forward.svg"
          icon.color: Cpp_ThemeManager.colors["taskbar_text"]
          onClicked: buttonsView.contentX = Math.min(buttonsView.contentWidth - buttonsView.width, buttonsView.contentX + 150)
        }
      }
    }

    //
    // Clock text
    //
    Label {
      font: Cpp_Misc_CommonFonts.monoFont
      Layout.alignment: Qt.AlignVCenter
      text: Qt.formatTime(new Date(), "hh:mm:ss AP")
      color: Cpp_ThemeManager.colors["taskbar_text"]

      Timer {
        repeat: true
        running: true
        interval: 1000
        onTriggered: parent.text = Qt.formatTime(new Date(), "hh:mm:ss AP")
      }
    } Item {
      implicitWidth: 6
    }

    //
    // Auto-layout button
    //
    Button {
      icon.width: 24
      icon.height: 24
      background: Item{}
      Layout.preferredWidth: 24
      Layout.preferredHeight: 24
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/buttons/auto-layout.svg"
      icon.color: taskBar.windowManager.autoLayoutEnabled ?
                    Cpp_ThemeManager.colors["tasbkar_highlight"] :
                    Cpp_ThemeManager.colors["taskbar_text"]
      onClicked: taskBar.windowManager.autoLayoutEnabled = !taskBar.windowManager.autoLayoutEnabled
    } Item {
      implicitWidth: 4
    }
  }

  //
  // Taskbar border
  //
  Rectangle {
    height: 1
    color: Cpp_ThemeManager.colors["taskbar_border"]
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
  }
}
