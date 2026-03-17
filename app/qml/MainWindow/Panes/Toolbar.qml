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
  signal projectEditorClicked()
  property bool autoHide: false
  property bool toolbarEnabled: true
  property bool dashboardVisible: false
  readonly property bool showContent: toolbarEnabled && !(autoHide && dashboardVisible)
  readonly property bool dashboardMode: !showContent && dashboardVisible

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
  // Animated toolbar content height
  //
  property int toolbarContentHeight: showContent ? 64 + 16 : 0
  Behavior on toolbarContentHeight {
    NumberAnimation {
      duration: 250
      easing.type: Easing.OutCubic
    }
  }

  Layout.minimumHeight: titlebarHeight + toolbarContentHeight
  Layout.maximumHeight: titlebarHeight + toolbarContentHeight

  //
  // Top toolbar section
  //
  Rectangle {
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }

    Behavior on color {
      ColorAnimation { duration: 250; easing.type: Easing.OutCubic }
    }

    height: root.titlebarHeight
    color: Cpp_ThemeManager.colors["toolbar_top"]
  }

  //
  // Titlebar text
  //
  Label {
    anchors {
      topMargin: 6
      top: parent.top
      horizontalCenter: parent.horizontalCenter
    }

    Behavior on color {
      ColorAnimation { duration: 250; easing.type: Easing.OutCubic }
    }

    text: mainWindow.title
    visible: root.titlebarHeight > 0
    color: Cpp_ThemeManager.colors["titlebar_text"]
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)
  }

  //
  // Toolbar background
  //
  Rectangle {
    anchors.fill: parent
    gradient: Gradient {
      GradientStop {
        position: 0
        color: Cpp_ThemeManager.colors["toolbar_top"]

        Behavior on color {
          ColorAnimation { duration: 250; easing.type: Easing.OutCubic }
        }
      }

      GradientStop {
        position: 1
        color: Cpp_ThemeManager.colors["toolbar_bottom"]

        Behavior on color {
          ColorAnimation { duration: 250; easing.type: Easing.OutCubic }
        }
      }
    }

    anchors.topMargin: root.titlebarHeight
  }

  //
  // Toolbar border
  //
  Rectangle {
    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }

    height: 1
    color: Cpp_ThemeManager.colors["toolbar_border"]
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
    visible: root.showContent
    enabled: root.showContent

    Behavior on opacity {
      NumberAnimation {
        duration: 200
        easing.type: Easing.InOutQuad
      }
    }

    opacity: root.showContent ? 1 : 0

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
      icon.source: "qrc:/rcc/icons/toolbar/project-setup.svg"
      ToolTip.text: qsTr("Open the Project Editor to create or modify your JSON layout")

      onClicked: app.showProjectEditor()
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
        ToolTip.text: qsTr("Open an existing JSON project")
        icon.source: "qrc:/rcc/icons/toolbar/open-project.svg"
        onClicked: {
          Cpp_AppState.operationMode = SerialStudio.ProjectFile
          Cpp_JSON_ProjectModel.openJsonFile()
        }
      }

      Widgets.ToolbarButton {
        iconSize: 16
        text: qsTr("Open CSV")
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        onClicked: Cpp_CSV_Player.openFile()
        icon.source: "qrc:/rcc/icons/toolbar/csv.svg"
        ToolTip.text: qsTr("Play a CSV file as if it were live sensor data")
        enabled: !Cpp_CSV_Player.isOpen && !Cpp_IO_Manager.isConnected
      }

      Widgets.ToolbarButton {
        iconSize: 16
        text: qsTr("Open MDF4")
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        onClicked: Cpp_MDF4_Player.openFile()
        icon.source: "qrc:/rcc/icons/toolbar/mf4.svg"
        enabled: !Cpp_MDF4_Player.isOpen && !Cpp_IO_Manager.isConnected
        ToolTip.text: qsTr("Play an MDF4 file as if it were live sensor data (Pro)")
      }
    }

    //
    // Separator
    //
    Rectangle {
      implicitWidth: 1
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      visible: Cpp_CommercialBuild
      Layout.alignment: Qt.AlignVCenter
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
    // Preferences
    //
    Widgets.ToolbarButton {
      text: qsTr("Preferences")
      Layout.alignment: Qt.AlignLeft
      onClicked: app.showSettingsDialog()
      ToolTip.text: qsTr("Open application settings and preferences")
      icon.source: "qrc:/rcc/icons/toolbar/settings.svg"
    }

    //
    // Driver selection group
    //
    GridLayout {
      id: driverGrid

      rows: 3
      rowSpacing: 4
      columnSpacing: 4
      Layout.alignment: Qt.AlignVCenter
      columns: Cpp_CommercialBuild ? 3 : 1

      readonly property bool driverSelectionEnabled:
        Cpp_AppState.operationMode !== SerialStudio.ProjectFile ||
        Cpp_JSON_ProjectModel.sourceCount <= 1

      Widgets.ToolbarButton {
        iconSize: 16
        text: qsTr("UART")
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        enabled: driverGrid.driverSelectionEnabled
        onClicked: Cpp_IO_Manager.busType = SerialStudio.UART
        icon.source: "qrc:/rcc/icons/devices/drivers/uart.svg"
        font: Cpp_IO_Manager.busType === SerialStudio.UART ? Cpp_Misc_CommonFonts.boldUiFont : Cpp_Misc_CommonFonts.uiFont
        ToolTip.text: qsTr("Select Serial port (UART) communication")
      }

      Loader {
        visible: active
        active: Cpp_CommercialBuild
        Layout.alignment: Qt.AlignLeft
        sourceComponent: Component {
          Widgets.ToolbarButton {
            iconSize: 16
            text: qsTr("Audio")
            horizontalLayout: true
            Layout.alignment: Qt.AlignLeft
            enabled: driverGrid.driverSelectionEnabled
            ToolTip.text: qsTr("Select audio input device (Pro)")
            onClicked: Cpp_IO_Manager.busType = SerialStudio.Audio
            icon.source: "qrc:/rcc/icons/devices/drivers/audio.svg"
            font: Cpp_IO_Manager.busType === SerialStudio.Audio ? Cpp_Misc_CommonFonts.boldUiFont : Cpp_Misc_CommonFonts.uiFont
          }
        }
      }

      Loader {
        visible: active
        active: Cpp_CommercialBuild
        Layout.alignment: Qt.AlignLeft
        sourceComponent: Component {
          Widgets.ToolbarButton {
            iconSize: 16
            text: qsTr("USB")
            horizontalLayout: true
            Layout.alignment: Qt.AlignLeft
            enabled: driverGrid.driverSelectionEnabled
            ToolTip.text: qsTr("Select raw USB communication (Pro)")
            icon.source: "qrc:/rcc/icons/devices/drivers/usb.svg"
            onClicked: Cpp_IO_Manager.busType = SerialStudio.RawUsb
            font: Cpp_IO_Manager.busType === SerialStudio.RawUsb ? Cpp_Misc_CommonFonts.boldUiFont : Cpp_Misc_CommonFonts.uiFont
          }
        }
      }

      Widgets.ToolbarButton {
        iconSize: 16
        text: qsTr("Network")
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        enabled: driverGrid.driverSelectionEnabled
        ToolTip.text: qsTr("Select TCP/UDP network communication")
        icon.source: "qrc:/rcc/icons/devices/drivers/network.svg"
        onClicked: Cpp_IO_Manager.busType = SerialStudio.Network
        font: Cpp_IO_Manager.busType === SerialStudio.Network ? Cpp_Misc_CommonFonts.boldUiFont : Cpp_Misc_CommonFonts.uiFont
      }

      Loader {
        visible: active
        active: Cpp_CommercialBuild
        Layout.alignment: Qt.AlignLeft
        sourceComponent: Component {
          Widgets.ToolbarButton {
            iconSize: 16
            text: qsTr("Modbus")
            horizontalLayout: true
            Layout.alignment: Qt.AlignLeft
            enabled: driverGrid.driverSelectionEnabled
            ToolTip.text: qsTr("Select MODBUS communication (Pro)")
            icon.source: "qrc:/rcc/icons/devices/drivers/modbus.svg"
            onClicked: Cpp_IO_Manager.busType = SerialStudio.ModBus
            font: Cpp_IO_Manager.busType === SerialStudio.ModBus ? Cpp_Misc_CommonFonts.boldUiFont : Cpp_Misc_CommonFonts.uiFont
          }
        }
      }

      Loader {
        visible: active
        active: Cpp_CommercialBuild
        Layout.alignment: Qt.AlignLeft
        sourceComponent: Component {
          Widgets.ToolbarButton {
            iconSize: 16
            text: qsTr("HID")
            horizontalLayout: true
            Layout.alignment: Qt.AlignLeft
            enabled: driverGrid.driverSelectionEnabled
            ToolTip.text: qsTr("Select HID device communication (Pro)")
            icon.source: "qrc:/rcc/icons/devices/drivers/hid.svg"
            onClicked: Cpp_IO_Manager.busType = SerialStudio.HidDevice
            font: Cpp_IO_Manager.busType === SerialStudio.HidDevice ? Cpp_Misc_CommonFonts.boldUiFont : Cpp_Misc_CommonFonts.uiFont
          }
        }
      }

      Widgets.ToolbarButton {
        iconSize: 16
        horizontalLayout: true
        text: qsTr("Bluetooth")
        Layout.alignment: Qt.AlignLeft
        enabled: driverGrid.driverSelectionEnabled
        ToolTip.text: qsTr("Select Bluetooth Low Energy communication")
        icon.source: "qrc:/rcc/icons/devices/drivers/bluetooth.svg"
        onClicked: Cpp_IO_Manager.busType = SerialStudio.BluetoothLE
        font: Cpp_IO_Manager.busType === SerialStudio.BluetoothLE ? Cpp_Misc_CommonFonts.boldUiFont : Cpp_Misc_CommonFonts.uiFont
      }

      Loader {
        visible: active
        active: Cpp_CommercialBuild
        Layout.alignment: Qt.AlignLeft
        sourceComponent: Component {
          Widgets.ToolbarButton {
            iconSize: 16
            text: qsTr("CAN Bus")
            horizontalLayout: true
            Layout.alignment: Qt.AlignLeft
            enabled: driverGrid.driverSelectionEnabled
            ToolTip.text: qsTr("Select CAN Bus communication (Pro)")
            icon.source: "qrc:/rcc/icons/devices/drivers/canbus.svg"
            onClicked: Cpp_IO_Manager.busType = SerialStudio.CanBus
            font: Cpp_IO_Manager.busType === SerialStudio.CanBus ? Cpp_Misc_CommonFonts.boldUiFont : Cpp_Misc_CommonFonts.uiFont
          }
        }
      }

      Loader {
        visible: active
        active: Cpp_CommercialBuild
        Layout.alignment: Qt.AlignLeft
        sourceComponent: Component {
          Widgets.ToolbarButton {
            iconSize: 16
            text: qsTr("Process")
            horizontalLayout: true
            Layout.alignment: Qt.AlignLeft
            enabled: driverGrid.driverSelectionEnabled
            ToolTip.text: qsTr("Select process pipe communication (Pro)")
            icon.source: "qrc:/rcc/icons/devices/drivers/process.svg"
            onClicked: Cpp_IO_Manager.busType = SerialStudio.Process
            font: Cpp_IO_Manager.busType === SerialStudio.Process ? Cpp_Misc_CommonFonts.boldUiFont : Cpp_Misc_CommonFonts.uiFont
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
      ToolTip.text: qsTr("Show application info and license details")
      icon.source: "qrc:/rcc/icons/toolbar/about.svg"
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
        ToolTip.text: qsTr("Browse example projects")
        onClicked: app.showExamplesBrowser()
      }

      Widgets.ToolbarButton {
        iconSize: 16
        horizontalLayout: true
        text: qsTr("Help Center")
        Layout.alignment: Qt.AlignLeft
        onClicked: app.showHelpCenter()
        icon.source: "qrc:/rcc/icons/toolbar/help.svg"
        ToolTip.text: qsTr("Browse documentation, FAQ, and wiki")
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
      text: checked ? qsTr("Disconnect") : qsTr("Connect")
      checked: Cpp_IO_Manager.isConnected || mqttSubscriber
      ToolTip.text: qsTr("Connect or disconnect from device or MQTT broker")
      icon.source: checked ? "qrc:/rcc/icons/toolbar/connect.svg" :
                             "qrc:/rcc/icons/toolbar/disconnect.svg"

      visible: Cpp_CommercialBuild ? (Cpp_Licensing_Trial.trialExpired && !Cpp_Licensing_LemonSqueezy.isActivated ? false : true) : true

      readonly property bool mqttSubscriber: Cpp_CommercialBuild ? (Cpp_MQTT_Client.isConnected && Cpp_MQTT_Client.isSubscriber) : false

      enabled: (Cpp_IO_Manager.configurationOk && !Cpp_CSV_Player.isOpen && !Cpp_MDF4_Player.isOpen) || mqttSubscriber

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
