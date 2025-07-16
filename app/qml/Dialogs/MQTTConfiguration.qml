/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Window {
  id: root

  //
  // Window options
  //
  width: minimumWidth
  height: minimumHeight
  title: qsTr("MQTT Setup")
  minimumWidth: column.implicitWidth + 32
  maximumWidth: column.implicitWidth + 32
  minimumHeight: column.implicitHeight + root.titlebarHeight + 32
  maximumHeight: column.implicitHeight + root.titlebarHeight + 32
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowCloseButtonHint
  }

  //
  // Native window registration
  //
  property real titlebarHeight: 0
  onVisibleChanged: {
    if (visible) {
      Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["base"])
      root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
    }

    else {
      root.titlebarHeight = 0
      Cpp_NativeWindow.removeWindow(root)
    }
  }

  //
  // Background + window title on macOS
  //
  Rectangle {
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["window"]

    //
    // Drag the window anywhere
    //
    DragHandler {
      target: null
      onActiveChanged: {
        if (active)
          root.startSystemMove()
      }
    }

    //
    // Titlebar text
    //
    Label {
      text: root.title
      visible: root.titlebarHeight > 0
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)

      anchors {
        topMargin: 6
        top: parent.top
        horizontalCenter: parent.horizontalCenter
      }
    }
  }

  //
  // Save settings
  //
  Settings {
    category: "mqtt"

    property alias host: _host.text
    property alias port: _port.text
    property alias clientId: _clientID.text
    property alias keepAlive: _keepAlive.text
    property alias cleanSession: _cleanSession.checked

    property alias username: _username.text
    property alias password: _password.text

    property alias version: _version.currentIndex
    property alias mode: _mode.currentIndex
    property alias topic: _topic.text

    property alias willRetain: _willRetain.checked
    property alias willQoS: _willQoS.currentIndex
    property alias willTopic: _willTopic.text
    property alias willMessage: _willMessage.text

    property alias sslEnabled: _enableSSL.checked
    property alias sslProtocol: _sslProtocol.currentIndex
    property alias sslVerifyDepth: _verifyDepth.text
    property alias sslVerifyMode: _verifyMode.currentIndex
  }

  //
  // Close shortcut
  //
  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  //
  // Highlight MQTT topic control if backend requires it
  //
  Connections {
    target: Cpp_MQTT_Client
    function onHighlightMqttTopicControl() {
      _tab.currentIndex = 2
      _topic.forceActiveFocus()
    }
  }

  //
  // Use page item to set application palette
  //
  Page {
    anchors.fill: parent
    anchors.topMargin: root.titlebarHeight
    palette.mid: Cpp_ThemeManager.colors["mid"]
    palette.dark: Cpp_ThemeManager.colors["dark"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.link: Cpp_ThemeManager.colors["link"]
    palette.light: Cpp_ThemeManager.colors["light"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.shadow: Cpp_ThemeManager.colors["shadow"]
    palette.accent: Cpp_ThemeManager.colors["accent"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.midlight: Cpp_ThemeManager.colors["midlight"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.windowText: Cpp_ThemeManager.colors["window_text"]
    palette.brightText: Cpp_ThemeManager.colors["bright_text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
    palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
    palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
    palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    ColumnLayout {
      id: column
      spacing: 12
      anchors.centerIn: parent

      //
      // Commercial license required pane
      //
      Widgets.ProNotice {
        radius: 2
        activationFlag: true
        expandSubtitle: false
        Layout.fillWidth: true
        closeButtonEnabled: false
        titleText: qsTr("MQTT is a Pro Feature")
        subtitleText: qsTr("Activate your license or visit the store to unlock MQTT support.")
      }

      //
      // Tab bar
      //
      TabBar {
        id: _tab
        implicitHeight: 24
        Layout.fillWidth: true

        TabButton {
          text: qsTr("General")
          height: _tab.height + 3
          width: implicitWidth + 2 * 8
        }

        TabButton {
          text: qsTr("Authentication")
          height: _tab.height + 3
          width: implicitWidth + 2 * 8
        }

        TabButton {
          text: qsTr("MQTT Options")
          height: _tab.height + 3
          width: implicitWidth + 2 * 8
        }

        TabButton {
          text: qsTr("SSL Properties")
          height: _tab.height + 3
          width: implicitWidth + 2 * 8
        }
      }

      //
      // Tab bar contents
      //
      StackLayout {
        id: stack
        clip: true
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumWidth: 356
        currentIndex: _tab.currentIndex
        Layout.topMargin: -parent.spacing - 1
        implicitHeight: Math.max(connectionSettings.implicitHeight,
                                 authentication.implicitHeight,
                                 mqttOptions.implicitHeight,
                                 sslProperties.implicitHeight)

        //
        // Connection settings
        //
        Item {
          id: connectionSettings

          Layout.fillWidth: true
          Layout.fillHeight: true
          implicitHeight: connectionSettingsLayout.implicitHeight + 16

          Rectangle {
            radius: 2
            border.width: 1
            anchors.fill: parent
            color: Cpp_ThemeManager.colors["groupbox_background"]
            border.color: Cpp_ThemeManager.colors["groupbox_border"]

            DragHandler {
              target: null
            }
          }

          GridLayout {
            id: connectionSettingsLayout

            enabled: app.proVersion
            opacity: enabled ? 1 : 0.8

            columns: 2
            rowSpacing: 4
            columnSpacing: 4
            anchors.margins: 8
            anchors.fill: parent

            Label { text: qsTr("Host") + ":" }
            TextField {
              id: _host
              Layout.fillWidth: true
              placeholderText: "127.0.0.1"
              text: Cpp_MQTT_Client.hostname
              onTextChanged: {
                if (Cpp_MQTT_Client.hostname !== text)
                  Cpp_MQTT_Client.hostname = text
              }
            }

            Label { text: qsTr("Port") + ":" }
            TextField {
              id: _port
              Layout.fillWidth: true
              placeholderText: "1883"
              text: Cpp_MQTT_Client.port
              inputMethodHints: Qt.ImhDigitsOnly
              validator: IntValidator { bottom: 0; top: 65535 }
              onTextChanged: {
                if (Cpp_MQTT_Client.port !== parseInt(text))
                  Cpp_MQTT_Client.port = parseInt(text)
              }
            }

            Label {
              opacity: enabled ? 1 : 0.5
              text: qsTr("Client ID") + ":"
            } TextField {
              id: _clientID
              Layout.fillWidth: true
              opacity: enabled ? 1 : 0.5
              text: Cpp_MQTT_Client.clientId
              onTextChanged: {
                if (Cpp_MQTT_Client.clientId !== text)
                  Cpp_MQTT_Client.clientId = text
              }
            }

            Label { text: qsTr("Keep Alive (s)") + ":" }
            TextField {
              id: _keepAlive
              Layout.fillWidth: true
              text: Cpp_MQTT_Client.keepAlive.toString()
              inputMethodHints: Qt.ImhDigitsOnly
              validator: IntValidator { bottom: 1; top: 65535 }
              onTextChanged: {
                if (Cpp_MQTT_Client.keepAlive !== parseInt(text))
                  Cpp_MQTT_Client.keepAlive = parseInt(text)
              }
            }

            Label { text: qsTr("Clean Session") + ":" }
            Switch {
              id: _cleanSession
              Layout.leftMargin: -8
              checked: Cpp_MQTT_Client.cleanSession
              onCheckedChanged: {
                if (Cpp_MQTT_Client.cleanSession !== checked)
                  Cpp_MQTT_Client.cleanSession = checked
              }
            }

            Item { Layout.fillHeight: true }
            Item { Layout.fillHeight: true }
          }
        }

        //
        // Authentication
        //
        Item {
          id: authentication
          Layout.fillWidth: true
          Layout.fillHeight: true
          implicitHeight: authenticationLayout.implicitHeight + 16

          Rectangle {
            radius: 2
            border.width: 1
            anchors.fill: parent
            color: Cpp_ThemeManager.colors["groupbox_background"]
            border.color: Cpp_ThemeManager.colors["groupbox_border"]

            DragHandler {
              target: null
            }
          }

          GridLayout {
            id: authenticationLayout

            enabled: app.proVersion
            opacity: enabled ? 1 : 0.8

            columns: 2
            rowSpacing: 4
            columnSpacing: 4
            anchors.margins: 8
            anchors.fill: parent

            Label { text: qsTr("Username") + ":" }
            TextField {
              id: _username
              Layout.fillWidth: true
              text: Cpp_MQTT_Client.username
              placeholderText: qsTr("MQTT Username")
              onTextChanged: {
                if (Cpp_MQTT_Client.username !== text)
                  Cpp_MQTT_Client.username = text
              }
            }

            Label { text: qsTr("Password") + ":" }
            RowLayout {
              spacing: 4
              Layout.fillWidth: true

              TextField {
                id: _password
                Layout.fillWidth: true
                echoMode: TextInput.Password
                text: Cpp_MQTT_Client.password
                placeholderText: qsTr("MQTT Password")
                onTextChanged: {
                  if (Cpp_MQTT_Client.password !== text)
                    Cpp_MQTT_Client.password = text
                }
              }

              Button {
                checkable: true
                icon.color: palette.text
                Layout.maximumWidth: height
                Layout.alignment: Qt.AlignVCenter
                icon.source: checked ? "qrc:/rcc/icons/buttons/invisible.svg" :
                                       "qrc:/rcc/icons/buttons/visible.svg"
                onCheckedChanged: _password.echoMode = (checked ? TextField.Normal :
                                                                  TextField.Password)
              }
            }

            Item { Layout.fillHeight: true }
            Item { Layout.fillHeight: true }
          }
        }

        //
        // MQTT options
        //
        Item {
          id: mqttOptions
          Layout.fillWidth: true
          Layout.fillHeight: true
          implicitHeight: mqttOptionsLayout.implicitHeight + 16

          Rectangle {
            radius: 2
            border.width: 1
            anchors.fill: parent
            color: Cpp_ThemeManager.colors["groupbox_background"]
            border.color: Cpp_ThemeManager.colors["groupbox_border"]

            DragHandler {
              target: null
            }
          }

          GridLayout {
            id: mqttOptionsLayout

            enabled: app.proVersion
            opacity: enabled ? 1 : 0.8

            columns: 2
            rowSpacing: 4
            columnSpacing: 4
            anchors.margins: 8
            anchors.fill: parent

            Label { text: qsTr("Version") + ":" }
            ComboBox {
              id: _version
              Layout.fillWidth: true
              model: Cpp_MQTT_Client.mqttVersions
              currentIndex: Cpp_MQTT_Client.mqttVersion
              onCurrentIndexChanged: {
                if (Cpp_MQTT_Client.mqttVersion !== currentIndex)
                  Cpp_MQTT_Client.mqttVersion = currentIndex
              }
            }

            Label { text: qsTr("Mode") + ":" }
            ComboBox {
              id: _mode
              Layout.fillWidth: true
              model: Cpp_MQTT_Client.modes
              currentIndex: Cpp_MQTT_Client.mode
              onCurrentIndexChanged: {
                if (Cpp_MQTT_Client.mode !== currentIndex)
                  Cpp_MQTT_Client.mode = currentIndex
              }
            }

            Label { text: qsTr("Topic") + ":" }
            TextField {
              id: _topic
              Layout.fillWidth: true
              text: Cpp_MQTT_Client.topicFilter
              placeholderText: qsTr("e.g. sensors/temperature or home/+/status")
              onTextChanged: {
                if (Cpp_MQTT_Client.topicFilter !== text)
                  Cpp_MQTT_Client.topicFilter = text
              }
            }

            Label { text: qsTr("Will Retain") + ":" }
            Switch {
              id: _willRetain
              Layout.leftMargin: -8
              checked: Cpp_MQTT_Client.willRetain
              onCheckedChanged: {
                if (Cpp_MQTT_Client.willRetain !== checked)
                  Cpp_MQTT_Client.willRetain = checked
              }
            }

            Label { text: qsTr("Will QoS") + ":" }
            ComboBox {
              id: _willQoS
              model: ["0", "1", "2"]
              Layout.fillWidth: true
              currentIndex: Cpp_MQTT_Client.willQoS
              onCurrentIndexChanged: {
                if (Cpp_MQTT_Client.willQoS !== currentIndex)
                  Cpp_MQTT_Client.willQoS = currentIndex
              }
            }

            Label { text: qsTr("Will Topic") + ":" }
            TextField {
              id: _willTopic
              Layout.fillWidth: true
              text: Cpp_MQTT_Client.willTopic
              placeholderText: qsTr("e.g. device/alerts/offline")
              onTextChanged: {
                if (Cpp_MQTT_Client.willTopic !== text)
                  Cpp_MQTT_Client.willTopic = text
              }
            }

            Label { text: qsTr("Will Message") + ":" }
            TextField {
              id: _willMessage
              Layout.fillWidth: true
              text: Cpp_MQTT_Client.willMessage
              placeholderText: qsTr("e.g. Device unexpectedly disconnected")
              onTextChanged: {
                if (Cpp_MQTT_Client.willMessage !== text)
                  Cpp_MQTT_Client.willMessage = text
              }
            }

            Item { Layout.fillHeight: true }
            Item { Layout.fillHeight: true }
          }
        }

        //
        // SSL Properties
        //
        Item {
          id: sslProperties
          Layout.fillWidth: true
          Layout.fillHeight: true
          implicitHeight: sslPropertiesLayout.implicitHeight + 16

          Rectangle {
            radius: 2
            border.width: 1
            anchors.fill: parent
            color: Cpp_ThemeManager.colors["groupbox_background"]
            border.color: Cpp_ThemeManager.colors["groupbox_border"]

            DragHandler {
              target: null
            }
          }

          GridLayout {
            id: sslPropertiesLayout

            enabled: app.proVersion
            opacity: enabled ? 1 : 0.8

            columns: 2
            rowSpacing: 4
            columnSpacing: 4
            anchors.margins: 8
            anchors.fill: parent

            Label { text: qsTr("Enable SSL") + ":" }
            Switch {
              id: _enableSSL
              Layout.leftMargin: -8
              checked: Cpp_MQTT_Client.sslEnabled
              onCheckedChanged: {
                if (Cpp_MQTT_Client.sslEnabled !== checked)
                  Cpp_MQTT_Client.sslEnabled = checked
              }
            }

            Label { text: qsTr("SSL Protocol") + ":"
              opacity: enabled ? 1 : 0.8
              enabled: Cpp_MQTT_Client.sslEnabled
            } ComboBox {
              id: _sslProtocol
              Layout.fillWidth: true
              opacity: enabled ? 1 : 0.8
              enabled: Cpp_MQTT_Client.sslEnabled
              model: Cpp_MQTT_Client.sslProtocols
              currentIndex: Cpp_MQTT_Client.sslProtocol
              onCurrentIndexChanged: {
                if (Cpp_MQTT_Client.sslProtocol !== currentIndex)
                  Cpp_MQTT_Client.sslProtocol = currentIndex
              }
            }

            Label { text: qsTr("Verify Depth") + ":"
              opacity: enabled ? 1 : 0.8
              enabled: Cpp_MQTT_Client.sslEnabled
            } TextField {
              id: _verifyDepth
              Layout.fillWidth: true
              opacity: enabled ? 1 : 0.8
              enabled: Cpp_MQTT_Client.sslEnabled
              text: Cpp_MQTT_Client.peerVerifyDepth.toString()
              inputMethodHints: Qt.ImhDigitsOnly
              validator: IntValidator { bottom: 0; top: 10 }
              onTextChanged: {
                if (Cpp_MQTT_Client.peerVerifyDepth !== parseInt(text))
                  Cpp_MQTT_Client.peerVerifyDepth = parseInt(text)
              }
            }

            Label {
              text: qsTr("Verify Mode") + ":"
              opacity: enabled ? 1 : 0.8
              enabled: Cpp_MQTT_Client.sslEnabled
            } ComboBox {
              id: _verifyMode
              Layout.fillWidth: true
              opacity: enabled ? 1 : 0.8
              enabled: Cpp_MQTT_Client.sslEnabled
              model: Cpp_MQTT_Client.peerVerifyModes
              currentIndex: Cpp_MQTT_Client.peerVerifyMode
              onCurrentIndexChanged: {
                if (Cpp_MQTT_Client.peerVerifyMode !== currentIndex)
                  Cpp_MQTT_Client.peerVerifyMode = currentIndex
              }
            }

            Item { Layout.fillHeight: true }
            Item { Layout.fillHeight: true }
          }
        }
      }

      //
      // Buttons
      //
      RowLayout {
        Layout.alignment: Qt.AlignRight
        spacing: 12

        Button {
          icon.width: 18
          icon.height: 18
          text: qsTr("Close")
          onClicked: root.close()
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/rcc/icons/buttons/close.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
        }

        Item {
          Layout.fillWidth: true
        }

        Button {
          icon.width: 18
          icon.height: 18
          highlighted: true
          Layout.alignment: Qt.AlignVCenter
          checked: Cpp_MQTT_Client.isConnected
          onClicked: Cpp_MQTT_Client.toggleConnection()
          icon.color: Cpp_ThemeManager.colors["button_text"]
          text: Cpp_MQTT_Client.isConnected ? qsTr("Disconnect") : qsTr("Connect")
          icon.source: Cpp_MQTT_Client.isConnected ? "qrc:/rcc/icons/buttons/connected.svg" :
                                                     "qrc:/rcc/icons/buttons/disconnected.svg"

          opacity: enabled ? 1 : 0.8
          enabled: app.proVersion ?
                     (Cpp_MQTT_Client.isSubscriber ? !Cpp_IO_Manager.isConnected : true) :
                     false
        }
      }
    }
  }
}
