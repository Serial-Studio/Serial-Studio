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
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import "../../Widgets" as Widgets

Rectangle {
  id: root
  implicitWidth: (layout.implicitWidth + 32)

  //
  // Custom signals
  //
  signal setupClicked()
  signal consoleClicked()
  signal dashboardClicked()
  signal projectEditorClicked()

  property bool toolbarEnabled: true

  //
  // Aliases to button check status
  //
  property alias setupChecked: setupBt.checked

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
  Layout.minimumHeight: titlebarHeight + (toolbarEnabled ? 64 + 16 : 0)
  Layout.maximumHeight: titlebarHeight + (toolbarEnabled ? 64 + 16 : 0)

  //
  // Titlebar text
  //
  Label {
    text: mainWindow.title
    visible: root.titlebarHeight > 0
    color: Cpp_ThemeManager.colors["titlebar_text"]
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)

    anchors {
      topMargin: 6
      top: parent.top
      horizontalCenter: parent.horizontalCenter
    }
  }

  //
  // Toolbar background
  //
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

  //
  // Toolbar border
  //
  Rectangle {
    height: 1
    color: Cpp_ThemeManager.colors["toolbar_border"]

    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
  }

  //
  // Drag main window with the toolbar
  //
  DragHandler {
    target: null
    onActiveChanged: {
      if (active)
        mainWindow.startSystemMove()
    }
  }

  //
  // Toolbar controls
  //
  RowLayout {
    id: layout
    spacing: 4
    visible: root.toolbarEnabled
    enabled: root.toolbarEnabled

    anchors {
      margins: 2
      left: parent.left
      right: parent.right
      verticalCenter: parent.verticalCenter
      verticalCenterOffset: root.titlebarHeight / 2
    }

    //
    // Horizontal spacer
    //
    Item {
      implicitWidth: 1
    }

    //
    // Project Editor
    //
    Widgets.ToolbarButton {
      text: qsTr("Project Editor")
      Layout.alignment: Qt.AlignVCenter
      onClicked: app.showProjectEditor()
      icon.source: "qrc:/rcc/icons/toolbar/project-setup.svg"
      ToolTip.text: qsTr("Open the Project Editor to create or modify your JSON layout")
    }

    //
    // CSV Player
    //
    Widgets.ToolbarButton {
      text: qsTr("CSV Player")
      Layout.alignment: Qt.AlignVCenter
      onClicked: Cpp_CSV_Player.openFile()
      icon.source: "qrc:/rcc/icons/toolbar/csv.svg"
      enabled: !Cpp_CSV_Player.isOpen && !Cpp_IO_Manager.isConnected
      ToolTip.text: qsTr("Play a CSV file as if it were live sensor data")
    }

    //
    // Separator
    //
    Rectangle {
      implicitWidth: 1
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      Layout.alignment: Qt.AlignVCenter
      visible: Cpp_CommercialBuild
      color: Cpp_ThemeManager.colors["toolbar_separator"]
    }

    //
    // Setup
    //
    Widgets.ToolbarButton {
      id: setupBt
      text: qsTr("Devices")
      onClicked: root.setupClicked()
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/toolbar/device-setup.svg"
      ToolTip.text: qsTr("Configure device connection via Serial, BLE, or network socket")
    }

    //
    // Settings
    //
    Widgets.ToolbarButton {
      text: qsTr("Preferences")
      Layout.alignment: Qt.AlignVCenter
      onClicked: app.showSettingsDialog()
      icon.source: "qrc:/rcc/icons/toolbar/settings.svg"
      ToolTip.text: qsTr("Open application settings and preferences")
    }

    //
    // Separator
    //
    Rectangle {
      implicitWidth: 1
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      Layout.alignment: Qt.AlignVCenter
      visible: Cpp_CommercialBuild
      color: Cpp_ThemeManager.colors["toolbar_separator"]
    }

    //
    // MQTT Setup
    //
    Loader {
      active: Cpp_CommercialBuild
      sourceComponent: Component {
        Widgets.ToolbarButton {
          text: qsTr("MQTT")
          onClicked: app.showMqttConfiguration()
          icon.source: Cpp_MQTT_Client.isConnected ?
                         (Cpp_MQTT_Client.isSubscriber ?
                            "qrc:/rcc/icons/toolbar/mqtt-subscriber.svg" :
                            "qrc:/rcc/icons/toolbar/mqtt-publisher.svg") :
                         "qrc:/rcc/icons/toolbar/mqtt.svg"
          ToolTip.text: qsTr("Configure MQTT connection (publish or subscribe)")
        }
      }
    }

    //
    // Separator
    //
    Rectangle {
      implicitWidth: 1
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      Layout.alignment: Qt.AlignVCenter
      color: Cpp_ThemeManager.colors["toolbar_separator"]
    }

    //
    // Examples
    //
    Widgets.ToolbarButton {
      text: qsTr("Examples")
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/toolbar/examples.svg"
      ToolTip.text: qsTr("Browse example projects on GitHub")
      onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/tree/master/examples")
    }

    //
    // Help
    //
    Widgets.ToolbarButton {
      text: qsTr("Help")
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/toolbar/help.svg"
      ToolTip.text: qsTr("Open the online documentation for help and guidance")
      onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/wiki")
    }

    //
    // DeepWiki
    //
    Widgets.ToolbarButton {
      text: qsTr("AI Help")
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/toolbar/deepwiki.svg"
      ToolTip.text: qsTr("View detailed documentation and ask questions on DeepWiki")
      onClicked: Qt.openUrlExternally("https://deepwiki.com/Serial-Studio/Serial-Studio")
    }

    //
    // About
    //
    Widgets.ToolbarButton {
      text: qsTr("About")
      onClicked: app.showAboutDialog()
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/toolbar/about.svg"
      ToolTip.text: qsTr("Show application info and license details")
    }

    //
    // Horizontal spacer
    //
    Item {
      implicitWidth: 1
      Layout.fillWidth: true
    }

    //
    // Connect/Disconnect button
    //
    Widgets.ToolbarButton {
      id: _connectButton
      Layout.alignment: Qt.AlignVCenter
      implicitWidth: metrics.width + 16
      font: Cpp_Misc_CommonFonts.boldUiFont
      Layout.minimumWidth: metrics.width + 16
      Layout.maximumWidth: metrics.width + 16
      checked: Cpp_IO_Manager.isConnected || mqttSubscriber
      text: checked ? qsTr("Disconnect") : qsTr("Connect")
      icon.source: checked ? "qrc:/rcc/icons/toolbar/connect.svg" :
                             "qrc:/rcc/icons/toolbar/disconnect.svg"
      ToolTip.text: qsTr("Connect or disconnect from device or MQTT broker")

      //
      // Hide button when trial expires
      //
      visible: Cpp_CommercialBuild ? (Cpp_Licensing_Trial.trialExpired && !Cpp_Licensing_LemonSqueezy.isActivated ? false : true) : true

      //
      // Get MQTT status
      //
      readonly property bool mqttSubscriber: Cpp_CommercialBuild ? (Cpp_MQTT_Client.isConnected && Cpp_MQTT_Client.isSubscriber) : false

      //
      // Enable/disable the connect button
      //
      enabled: (Cpp_IO_Manager.configurationOk && !Cpp_CSV_Player.isOpen) || mqttSubscriber

      //
      // Connect/disconnect device when button is clicked
      //
      onClicked: {
        if (mqttSubscriber)
          Cpp_MQTT_Client.toggleConnection()
        else
          Cpp_IO_Manager.toggleConnection()
      }

      //
      // Obtain maximum width of the button
      //
      TextMetrics {
        id: metrics
        text: " " + qsTr("Disconnect") + " "
        font: Cpp_Misc_CommonFonts.boldUiFont
      }
    }

    //
    // Activate button
    //
    Widgets.ToolbarButton {
      text: qsTr("Activate")
      onClicked: app.showLicenseDialog()
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/toolbar/activate.svg"
      ToolTip.text: qsTr("Manage license and activate the application")
      visible: Cpp_CommercialBuild ? Cpp_Licensing_Trial.trialExpired && !Cpp_Licensing_LemonSqueezy.isActivated : false
    }

    //
    // Horizontal spacer
    //
    Item {
      implicitWidth: 1
    }
  }
}
