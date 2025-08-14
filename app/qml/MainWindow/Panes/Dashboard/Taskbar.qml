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
      implicitWidth: start.layout.implicitWidth + 8
      icon.source: Cpp_ThemeManager.parameters["start-icon"]
      onClicked: {
        root.startClicked()
        taskBar.activeWindow = null
      }
    }

    //
    // Shortcuts
    //
    Widgets.TaskbarButton {
      forceVisible: true
      icon.source: "qrc:/rcc/icons/taskbar/adjust.svg"
      onClicked: {
        app.showSettingsDialog()
        taskBar.activeWindow = null
      }
    } Widgets.TaskbarButton {
      forceVisible: true
      focused: Cpp_UI_Dashboard.terminalEnabled
      icon.source: "qrc:/rcc/icons/taskbar/console.svg"
      onClicked: {
        taskBar.activeWindow = null
        Cpp_UI_Dashboard.terminalEnabled = !Cpp_UI_Dashboard.terminalEnabled
      }
    } Widgets.TaskbarButton {
      forceVisible: true
      focused: Cpp_IO_Manager.paused
      icon.source: Cpp_IO_Manager.paused ?
                     "qrc:/rcc/icons/taskbar/resume.svg" :
                     "qrc:/rcc/icons/taskbar/pause.svg"
      onClicked: {
        taskBar.activeWindow = null
        Cpp_IO_Manager.paused = !Cpp_IO_Manager.paused
      }
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
          onClicked: {
            taskBar.activeWindow = null
            buttonsView.contentX = Math.max(0, buttonsView.contentX - 150)
          }
        }

        ListView {
          id: buttonsView

          clip: true
          spacing: 2
          interactive: true
          Layout.fillWidth: true
          Layout.fillHeight: true
          model: taskBar.taskbarButtons
          orientation: ListView.Horizontal
          boundsBehavior: Flickable.StopAtBounds

          delegate: Widgets.TaskbarButton {
            required property var model

            id: button
            text: model.widgetName
            icon.source: SerialStudio.dashboardWidgetIcon(model.widgetType)
            forceVisible: Cpp_UI_Dashboard.showTaskbarButtons || taskBar.hasMaximizedWindow

            width: opacity > 0 ? 144 : 0
            Behavior on width { NumberAnimation{} }

            Component.onCompleted: updateState()

            onClicked: {
              const window = taskBar.windowData(model.windowId)
              if (window !== null) {
                if (taskBar.windowState(window) !== SS_Ui.TaskbarModel.WindowNormal)
                  taskBar.showWindow(window)

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
          onClicked: {
            taskBar.activeWindow = null
            buttonsView.contentX = Math.min(buttonsView.contentWidth - buttonsView.width, buttonsView.contentX + 150)
          }
        }
      }
    }

    //
    // Workspace switcher
    //
    ComboBox {
      id: _switcher
      textRole: "text"
      model: taskBar.groupModel
      Layout.alignment: Qt.AlignVCenter
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
        width: _switcher.width

        contentItem: RowLayout {
          spacing: 8
          anchors.verticalCenter: parent.verticalCenter
          Component.onCompleted: {
            var itemWidth = Math.min(480, implicitWidth + 32)
            if (_switcher.implicitWidth < itemWidth)
              _switcher.implicitWidth = itemWidth
          }

          Image {
            source: modelData["icon"]
            sourceSize: Qt.size(16, 16)
            fillMode: Image.PreserveAspectFit
          }

          Label {
            text: modelData["text"]
            elide: Text.ElideRight
            Layout.fillWidth: true
            verticalAlignment: Text.AlignVCenter
            font: text === _switcher.currentText
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
          text: _switcher.currentText
          horizontalAlignment: Text.AlignRight
          verticalAlignment: Text.AlignVCenter
          font: Cpp_Misc_CommonFonts.boldUiFont
          color: Cpp_ThemeManager.colors["pane_caption_foreground"]
        }

        Canvas {
          id: _canvas

          width: 18
          height: 18
          opacity: 0.8
          Layout.alignment: Qt.AlignVCenter
          Connections {
            target: Cpp_ThemeManager
            function onThemeChanged() {
              _canvas.requestPaint()
            }
          }

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
                    Cpp_ThemeManager.colors["taskbar_highlight"] :
                    Cpp_ThemeManager.colors["taskbar_text"]
      onClicked: {
        taskBar.activeWindow = null
        taskBar.windowManager.autoLayoutEnabled = !taskBar.windowManager.autoLayoutEnabled
      }
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
