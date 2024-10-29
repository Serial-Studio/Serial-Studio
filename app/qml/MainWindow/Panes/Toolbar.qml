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

import SerialStudio
import "../../Widgets" as Widgets

ToolBar {
  id: root

  //
  // Custom signals
  //
  signal setupClicked()
  signal consoleClicked()
  signal dashboardClicked()
  signal structureClicked()
  signal projectEditorClicked()

  //
  // Aliases to button check status
  //
  property alias setupChecked: setupBt.checked
  property alias consoleChecked: consoleBt.checked
  property alias dashboardChecked: dashboardBt.checked
  property alias structureChecked: structureBt.checked

  //
  // Calculate offset based on platform
  //
  property int titlebarHeight: Cpp_NativeWindow.titlebarHeight(mainWindow)
  Connections {
    target: mainWindow
    function onVisibilityChanged() {
      root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(mainWindow)
    }
  }

  //
  // Set toolbar height
  //
  Layout.minimumHeight: titlebarHeight + 76
  Layout.maximumHeight: titlebarHeight + 76

  //
  // Titlebar text
  //
  Label {
    text: mainWindow.title
    visible: root.titlebarHeight > 0
    color: Cpp_ThemeManager.colors["titlebar_text"]
    font: Cpp_Misc_CommonFonts.customUiFont(13, true)

    anchors {
      topMargin: 6
      top: parent.top
      horizontalCenter: parent.horizontalCenter
    }
  }

  //
  // Toolbar background
  //
  background: Rectangle {
    gradient: Gradient {
      GradientStop {
        position: 0
        color: Cpp_ThemeManager.colors["toolbar_top"]
      }

      GradientStop {
        position: 1
        color: Cpp_ThemeManager.colors["toolbar_bottom"]
      }
    }

    Rectangle {
      height: 1
      color: Cpp_ThemeManager.colors["toolbar_border"]

      anchors {
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }
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
  // Toolbar controls
  //
  RowLayout {
    spacing: 8

    anchors {
      margins: 2
      left: parent.left
      right: parent.right
      verticalCenter: parent.verticalCenter
      verticalCenterOffset: root.titlebarHeight / 2
    }

    //
    // Project Editor
    //
    Widgets.BigButton {
      text: qsTr("Project Editor")
      Layout.alignment: Qt.AlignVCenter
      onClicked: app.showProjectEditor()
      icon.source: "qrc:/rcc/icons/toolbar/project-setup.svg"
      enabled: Cpp_JSON_FrameBuilder.operationMode == JsonGenerator.ProjectFile
    }

    //
    // CSV Player
    //
    Widgets.BigButton {
      text: qsTr("CSV Player")
      Layout.alignment: Qt.AlignVCenter
      onClicked: Cpp_CSV_Player.openFile()
      icon.source: "qrc:/rcc/icons/toolbar/csv.svg"
      enabled: !Cpp_CSV_Player.isOpen && !Cpp_IO_Manager.connected
    }

    //
    // Separator
    //
    Rectangle {
      width: 1
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      Layout.alignment: Qt.AlignVCenter
      color: Cpp_ThemeManager.colors["toolbar_separator"]
    }

    //
    // Setup
    //
    Widgets.BigButton {
      id: setupBt
      text: qsTr("Setup")
      onClicked: root.setupClicked()
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/toolbar/device-setup.svg"
    }

    //
    // Console
    //
    Widgets.BigButton {
      id: consoleBt
      opacity: 1
      text: qsTr("Console")
      enabled: dashboardBt.enabled
      onClicked: root.consoleClicked()
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/toolbar/console.svg"
    }

    //
    // Separator
    //
    Rectangle {
      width: 1
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      Layout.alignment: Qt.AlignVCenter
      color: Cpp_ThemeManager.colors["toolbar_separator"]
    }

    //
    // Dashboard Structure
    //
    Widgets.BigButton {
      id: structureBt
      text: qsTr("Widgets")
      enabled: dashboardBt.checked
      Layout.alignment: Qt.AlignVCenter
      onClicked: root.structureClicked()
      icon.source: "qrc:/rcc/icons/toolbar/structure.svg"
    }

    //
    // Dashboard
    //
    Widgets.BigButton {
      id: dashboardBt
      text: qsTr("Dashboard")
      onClicked: root.dashboardClicked()
      Layout.alignment: Qt.AlignVCenter
      enabled: Cpp_UI_Dashboard.available
      icon.source: "qrc:/rcc/icons/toolbar/dashboard.svg"
    }

    //
    // Separator
    //
    Rectangle {
      width: 1
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      Layout.alignment: Qt.AlignVCenter
      color: Cpp_ThemeManager.colors["toolbar_separator"]
    }

    //
    // MQTT Setup
    //
    Widgets.BigButton {
      text: qsTr("MQTT")
      Layout.alignment: Qt.AlignVCenter
      onClicked: app.showMqttConfiguration()
      icon.source: Cpp_MQTT_Client.isConnectedToHost ?
                     (Cpp_MQTT_Client.clientMode === 1 ?
                        "qrc:/rcc/icons/toolbar/mqtt-subscriber.svg" :
                        "qrc:/rcc/icons/toolbar/mqtt-publisher.svg") :
                     "qrc:/rcc/icons/toolbar/mqtt.svg"
    }

    //
    // Separator
    //
    Rectangle {
      width: 1
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      Layout.alignment: Qt.AlignVCenter
      color: Cpp_ThemeManager.colors["toolbar_separator"]
    }

    //
    // Examples
    //
    Widgets.BigButton {
      text: qsTr("Examples")
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/toolbar/examples.svg"
      onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/tree/master/examples")
    }

    //
    // Help
    //
    Widgets.BigButton {
      text: qsTr("Help")
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/toolbar/help.svg"
      onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/wiki")
    }

    //
    // About
    //
    Widgets.BigButton {
      text: qsTr("About")
      onClicked: app.showAboutDialog()
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/toolbar/about.svg"
    }

    //
    // Horizontal spacer
    //
    Item {
      Layout.fillWidth: true
    }

    //
    // Connect/Disconnect button
    //
    Widgets.BigButton {
      checked: Cpp_IO_Manager.connected
      Layout.alignment: Qt.AlignVCenter
      font: Cpp_Misc_CommonFonts.boldUiFont
      Layout.minimumWidth: metrics.width + 32
      Layout.maximumWidth: metrics.width + 32
      enabled: Cpp_IO_Manager.configurationOk && !Cpp_CSV_Player.isOpen
      text: checked ? qsTr("Disconnect") : qsTr("Connect")
      icon.source: checked ? "qrc:/rcc/icons/toolbar/connect.svg" :
                             "qrc:/rcc/icons/toolbar/disconnect.svg"


      //
      // Connect/disconnect device when button is clicked
      //
      onClicked: Cpp_IO_Manager.toggleConnection()

      //
      // Obtain maximum width of the button
      //
      TextMetrics {
        id: metrics
        font: Cpp_Misc_CommonFonts.boldUiFont
        text: " " + qsTr("Disconnect") + " "
      }
    }
  }
}
