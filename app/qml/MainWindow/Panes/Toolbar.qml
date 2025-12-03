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
  // Top toolbar section
  //
  Rectangle {
    height: root.titlebarHeight
    color: Cpp_ThemeManager.colors["toolbar_top"]
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
  }

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
  Rectangle {
    anchors.fill: parent
    anchors.topMargin: root.titlebarHeight
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
    // Resource buttons
    //
    GridLayout {
      rows: 3
      columns: 1
      rowSpacing: 4
      columnSpacing: 4
      Layout.alignment: Qt.AlignVCenter

      Widgets.ToolbarButton {
        iconSize: 16
        horizontalLayout: true
        text: qsTr("Open Project")
        Layout.alignment: Qt.AlignLeft
        onClicked: Cpp_JSON_ProjectModel.openJsonFile()
        ToolTip.text: qsTr("Open an existing JSON project")
        icon.source: "qrc:/rcc/icons/toolbar/open-project.svg"
      }

      Widgets.ToolbarButton {
        iconSize: 16
        text: qsTr("Load CSV")
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        onClicked: Cpp_CSV_Player.openFile()
        icon.source: "qrc:/rcc/icons/toolbar/csv.svg"
        enabled: !Cpp_CSV_Player.isOpen && !Cpp_IO_Manager.isConnected
        ToolTip.text: qsTr("Play a CSV file as if it were live sensor data")
      }

      Widgets.ToolbarButton {
        iconSize: 16
        text: qsTr("Preferences")
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        onClicked: app.showSettingsDialog()
        icon.source: "qrc:/rcc/icons/toolbar/settings.svg"
        ToolTip.text: qsTr("Open application settings and preferences")
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
    // Setup
    //
    Widgets.ToolbarButton {
      id: setupBt
      text: qsTr("Setup")
      onClicked: root.setupClicked()
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/toolbar/device-setup.svg"
      ToolTip.text: qsTr("Configure device connection settings")
    }

    //
    // Driver selection group
    //
    GridLayout {
      rows: 3
      rowSpacing: 4
      columnSpacing: 4
      Layout.alignment: Qt.AlignVCenter
      columns: Cpp_CommercialBuild ? 2 : 1

      Widgets.ToolbarButton {
        iconSize: 16
        text: qsTr("UART")
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        font.bold: Cpp_IO_Manager.busType === SerialStudio.UART
        onClicked: Cpp_IO_Manager.busType = SerialStudio.UART
        ToolTip.text: qsTr("Select Serial port (UART) communication")
        icon.source: "qrc:/rcc/icons/toolbar/drivers/uart.svg"
      }

      Loader {
        active: Cpp_CommercialBuild
        Layout.alignment: Qt.AlignLeft
        sourceComponent: Component {
          Widgets.ToolbarButton {
            iconSize: 16
            text: qsTr("Audio")
            horizontalLayout: true
            Layout.alignment: Qt.AlignLeft
            font.bold: Cpp_IO_Manager.busType === SerialStudio.Audio
            onClicked: Cpp_IO_Manager.busType = SerialStudio.Audio
            ToolTip.text: qsTr("Select audio input device (Pro)")
            icon.source: "qrc:/rcc/icons/toolbar/drivers/audio.svg"
          }
        }
      }

      Widgets.ToolbarButton {
        iconSize: 16
        text: qsTr("Network")
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        font.bold: Cpp_IO_Manager.busType === SerialStudio.Network
        onClicked: Cpp_IO_Manager.busType = SerialStudio.Network
        ToolTip.text: qsTr("Select TCP/UDP network communication")
        icon.source: "qrc:/rcc/icons/toolbar/drivers/network.svg"
      }

      Loader {
        active: Cpp_CommercialBuild
        Layout.alignment: Qt.AlignLeft
        sourceComponent: Component {
          Widgets.ToolbarButton {
            iconSize: 16
            enabled: false
            text: qsTr("ModBus")
            horizontalLayout: true
            Layout.alignment: Qt.AlignLeft
            font.bold: Cpp_IO_Manager.busType === SerialStudio.ModBus
            onClicked: Cpp_IO_Manager.busType = SerialStudio.ModBus
            ToolTip.text: qsTr("Select MODBUS communication (Pro)")
            icon.source: "qrc:/rcc/icons/toolbar/drivers/modbus.svg"
          }
        }
      }

      Widgets.ToolbarButton {
        iconSize: 16
        horizontalLayout: true
        text: qsTr("Bluetooth")
        Layout.alignment: Qt.AlignLeft
        font.bold: Cpp_IO_Manager.busType === SerialStudio.BluetoothLE
        onClicked: Cpp_IO_Manager.busType = SerialStudio.BluetoothLE
        ToolTip.text: qsTr("Select Bluetooth Low Energy communication")
        icon.source: "qrc:/rcc/icons/toolbar/drivers/bluetooth.svg"
      }

      Loader {
        active: Cpp_CommercialBuild
        Layout.alignment: Qt.AlignLeft
        sourceComponent: Component {
          Widgets.ToolbarButton {
            iconSize: 16
            enabled: false
            text: qsTr("CAN Bus")
            horizontalLayout: true
            Layout.alignment: Qt.AlignLeft
            font.bold: Cpp_IO_Manager.busType === SerialStudio.CanBus
            onClicked: Cpp_IO_Manager.busType = SerialStudio.CanBus
            ToolTip.text: qsTr("Select CAN Bus communication (Pro)")
            icon.source: "qrc:/rcc/icons/toolbar/drivers/canbus.svg"
          }
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
    // About button
    //
    Widgets.ToolbarButton {
      text: qsTr("About")
      Layout.alignment: Qt.AlignLeft
      onClicked: app.showAboutDialog()
      icon.source: "qrc:/rcc/icons/toolbar/about.svg"
      ToolTip.text: qsTr("Show application info and license details")
    }

    //
    // Resource buttons
    //
    GridLayout {
      rows: 3
      columns: 1
      rowSpacing: 4
      columnSpacing: 4
      Layout.alignment: Qt.AlignVCenter

      Widgets.ToolbarButton {
        iconSize: 16
        text: qsTr("Examples")
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        icon.source: "qrc:/rcc/icons/toolbar/examples.svg"
        ToolTip.text: qsTr("Browse example projects on GitHub")
        onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/tree/master/examples")
      }

      Widgets.ToolbarButton {
        iconSize: 16
        horizontalLayout: true
        text: qsTr("Getting Started")
        Layout.alignment: Qt.AlignLeft
        icon.source: "qrc:/rcc/icons/toolbar/help.svg"
        ToolTip.text: qsTr("Open the online documentation for help and guidance")
        onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/blob/master/DISCOVER.md")
      }

      Widgets.ToolbarButton {
        iconSize: 16
        horizontalLayout: true
        text: qsTr("AI Wiki & Chat")
        Layout.alignment: Qt.AlignLeft
        icon.source: "qrc:/rcc/icons/toolbar/deepwiki.svg"
        ToolTip.text: qsTr("View detailed documentation and ask questions on DeepWiki")
        onClicked: Qt.openUrlExternally("https://deepwiki.com/Serial-Studio/Serial-Studio")
      }
    }

    //
    // Horizontal spacer
    //
    Item {
      implicitWidth: 1
      Layout.fillWidth: true
    }

    //
    // License activation (Pro only)
    //
    Widgets.ToolbarButton {
      text: qsTr("Activate")
      onClicked: app.showLicenseDialog()
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/toolbar/activate.svg"
      ToolTip.text: qsTr("Manage license and activate Serial Studio Pro")
      visible: Cpp_CommercialBuild ? Cpp_Licensing_Trial.trialExpired && !Cpp_Licensing_LemonSqueezy.isActivated : false
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

      visible: Cpp_CommercialBuild ? (Cpp_Licensing_Trial.trialExpired && !Cpp_Licensing_LemonSqueezy.isActivated ? false : true) : true

      readonly property bool mqttSubscriber: Cpp_CommercialBuild ? (Cpp_MQTT_Client.isConnected && Cpp_MQTT_Client.isSubscriber) : false

      enabled: (Cpp_IO_Manager.configurationOk && !Cpp_CSV_Player.isOpen) || mqttSubscriber

      onClicked: {
        if (mqttSubscriber)
          Cpp_MQTT_Client.toggleConnection()
        else
          Cpp_IO_Manager.toggleConnection()
      }

      TextMetrics {
        id: metrics
        text: " " + qsTr("Disconnect") + " "
        font: Cpp_Misc_CommonFonts.boldUiFont
      }
    }

    //
    // Horizontal spacer
    //
    Item {
      implicitWidth: 1
    }
  }
}
