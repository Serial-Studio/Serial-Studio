/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets

Control {
  id: root
  implicitHeight: 76

  //
  // Reference to parent window to be able to drag it with the toolbar
  //
  property Window window

  //
  // Dummy string to increase width of buttons
  //
  readonly property string _btSpacer: "  "

  //
  // Custom signals
  //
  signal setupClicked()
  signal consoleClicked()
  signal dashboardClicked()
  signal projectEditorClicked()

  //
  // Aliases to button check status
  //
  property alias setupChecked: setupBt.checked
  property alias consoleChecked: consoleBt.checked
  property alias dashboardChecked: dashboardBt.checked

  //
  // Background gradient + border
  //
  Rectangle {
    id: bg
    anchors.fill: parent

    gradient: Gradient {
      GradientStop { position: 0; color: Cpp_ThemeManager.toolbarGradient1 }
      GradientStop { position: 1; color: Cpp_ThemeManager.toolbarGradient2 }
    }

    Rectangle {
      border.width: 1
      anchors.fill: parent
      color: "transparent"
      visible: Cpp_ThemeManager.titlebarSeparator
      border.color: Qt.darker(Cpp_ThemeManager.toolbarGradient2, 1.5)
    }

    Rectangle {
      height: 1
      visible: Cpp_ThemeManager.titlebarSeparator
      color: Qt.darker(Cpp_ThemeManager.toolbarGradient1, 1.5)

      anchors {
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }
    }
  }

  //
  // Toolbar icons
  //
  RowLayout {
    spacing: app.spacing
    anchors.fill: parent
    anchors.margins: app.spacing

    Button {
      flat: true
      icon.width: 27
      icon.height: 27
      Layout.fillHeight: true
      icon.color: "transparent"
      text: qsTr("Project Setup")
      display: AbstractButton.TextUnderIcon
      onClicked: root.projectEditorClicked()
      Layout.minimumWidth: Math.max(implicitWidth, 81)
      Layout.maximumWidth: Math.max(implicitWidth, 81)
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1
      icon.source: "qrc:/toolbar-icons/project-editor.svg"

      background: Item {}
    }

    //
    // Separator
    //
    Rectangle {
      width: 1
      opacity: 0.2
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      Layout.alignment: Qt.AlignVCenter
      color: Cpp_ThemeManager.menubarText
    }

    Button {
      id: setupBt

      flat: true
      icon.width: 27
      icon.height: 27
      text: qsTr("Settings")
      Layout.fillHeight: true
      icon.color: "transparent"
      onClicked: root.setupClicked()
      display: AbstractButton.TextUnderIcon
      icon.source: "qrc:/toolbar-icons/setup.svg"
      Layout.minimumWidth: Math.max(implicitWidth, 81)
      Layout.maximumWidth: Math.max(implicitWidth, 81)
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1

      background: Rectangle {
        radius: 3
        border.width: 1
        color: "transparent"
        border.color: "#040600"
        opacity: parent.checked ? 0.2 : 0.0

        Rectangle {
          border.width: 1
          color: "#626262"
          anchors.fill: parent
          border.color: "#c2c2c2"
          radius: parent.radius - 1
          anchors.margins: parent.border.width
        }
      }
    }

    Button {
      id: consoleBt

      flat: true
      icon.width: 27
      icon.height: 27
      Layout.fillHeight: true
      icon.color: "transparent"
      enabled: dashboardBt.enabled
      onClicked: root.consoleClicked()
      text: qsTr("Console")
      display: AbstractButton.TextUnderIcon
      icon.source: "qrc:/toolbar-icons/console.svg"
      Layout.minimumWidth: Math.max(implicitWidth, 81)
      Layout.maximumWidth: Math.max(implicitWidth, 81)
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1

      background: Rectangle {
        radius: 3
        border.width: 1
        color: "transparent"
        border.color: "#040600"
        opacity: parent.checked ? 0.2 : 0.0

        Rectangle {
          border.width: 1
          color: "#626262"
          anchors.fill: parent
          border.color: "#c2c2c2"
          radius: parent.radius - 1
          anchors.margins: parent.border.width
        }
      }
    }

    Button {
      id: dashboardBt

      flat: true
      icon.width: 27
      icon.height: 27
      Layout.fillHeight: true
      icon.color: "transparent"
      opacity: enabled ? 1 : 0.5
      onClicked: root.dashboardClicked()
      enabled: Cpp_UI_Dashboard.available
      text: qsTr("Dashboard")
      display: AbstractButton.TextUnderIcon
      icon.source: "qrc:/toolbar-icons/dashboard.svg"
      Layout.minimumWidth: Math.max(implicitWidth, 81)
      Layout.maximumWidth: Math.max(implicitWidth, 81)
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1

      background: Rectangle {
        radius: 3
        border.width: 1
        color: "transparent"
        border.color: "#040600"
        opacity: parent.checked ? 0.2 : 0.0

        Rectangle {
          border.width: 1
          color: "#626262"
          anchors.fill: parent
          border.color: "#c2c2c2"
          radius: parent.radius - 1
          anchors.margins: parent.border.width
        }
      }
    }

    //
    // Separator
    //
    Rectangle {
      width: 1
      opacity: 0.2
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      Layout.alignment: Qt.AlignVCenter
      color: Cpp_ThemeManager.menubarText
    }

    Button {
      flat: true
      icon.width: 27
      icon.height: 27
      Layout.fillHeight: true
      icon.color: "transparent"
      text: qsTr("Help")
      display: AbstractButton.TextUnderIcon
      Layout.minimumWidth: Math.max(implicitWidth, 81)
      Layout.maximumWidth: Math.max(implicitWidth, 81)
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1
      icon.source: "qrc:/toolbar-icons/help.svg"

      background: Item {}
    }

    Button {
      flat: true
      icon.width: 27
      icon.height: 27
      Layout.fillHeight: true
      icon.color: "transparent"
      text: qsTr("Report Bug")
      display: AbstractButton.TextUnderIcon
      Layout.minimumWidth: Math.max(implicitWidth, 81)
      Layout.maximumWidth: Math.max(implicitWidth, 81)
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1
      icon.source: "qrc:/toolbar-icons/bug.svg"

      background: Item {}
    }

    //
    // Window drag handler
    //
    Item {
      height: parent.height
      Layout.fillWidth: true

      MouseArea {
        anchors.fill: parent
        onPressedChanged: {
          if (pressed)
            window.startSystemMove()
        }
      }
    }

    Button {
      flat: true
      icon.width: 27
      icon.height: 27
      Layout.fillHeight: true
      icon.color: "transparent"
      opacity: enabled ? 1 : 0.5
      enabled: !Cpp_CSV_Player.isOpen
      text: qsTr("Open CSV")
      display: AbstractButton.TextUnderIcon
      icon.source: "qrc:/toolbar-icons/csv.svg"
      Layout.minimumWidth: Math.max(implicitWidth, 81)
      Layout.maximumWidth: Math.max(implicitWidth, 81)
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1

      onClicked: {
        if (Cpp_CSV_Export.isOpen)
          Cpp_CSV_Export.openCurrentCsv()
        else
          Cpp_CSV_Player.openFile()
      }

      background: Item{}
    }

    Button {
      id: connectBt

      //
      // Button properties
      //
      flat: true
      icon.width: 27
      icon.height: 27
      Layout.fillHeight: true
      font: Cpp_Misc_CommonFonts.customUiFont(12, true)

      //
      // Connection-dependent
      //
      icon.color: "transparent"
      checked: Cpp_IO_Manager.connected
      display: AbstractButton.TextUnderIcon
      Layout.minimumWidth: Math.max(implicitWidth, 81)
      Layout.maximumWidth: Math.max(implicitWidth, 81)
      palette.buttonText: Cpp_ThemeManager.menubarText
      palette.button: Cpp_ThemeManager.toolbarGradient1
      palette.window: Cpp_ThemeManager.toolbarGradient1
      text: (checked ? qsTr("Disconnect") : qsTr("Connect"))
      icon.source: checked ? "qrc:/toolbar-icons/disconnect.svg" :
                             "qrc:/toolbar-icons/connect.svg"

      //
      // Only enable button if it can be clicked
      //
      opacity: enabled ? 1 : 0.5
      enabled: Cpp_IO_Manager.configurationOk

      //
      // Connect/disconnect device when button is clicked
      //
      onClicked: Cpp_IO_Manager.toggleConnection()

      //
      // Custom button background
      //
      background: Rectangle {
        radius: 3
        border.width: 1
        color: "transparent"
        border.color: "#040600"
        opacity: parent.checked ? 0.2 : 0.0

        Rectangle {
          border.width: 1
          color: "#626262"
          anchors.fill: parent
          border.color: "#c2c2c2"
          radius: parent.radius - 1
          anchors.margins: parent.border.width
        }
      }
    }
  }
}
