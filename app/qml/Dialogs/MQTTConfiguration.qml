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

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Window {
  id: root

  //
  // Custom properties
  //
  readonly property int year: new Date().getFullYear()

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

  //
  // Make window stay on top
  //
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowStaysOnTopHint |
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
    property alias version: _version.currentIndex
    property alias mode: _mode.currentIndex
    property alias host: _host.text
    property alias port: _port.text
    property alias qos: _qos.currentIndex
    property alias keepAlive: _keepAlive.text
    property alias topic: _topic.text
    property alias retain: _retain.checked
    property alias user: _user.text
    property alias password: _password.text
    property alias clientId: _clientId.text
    property alias ssl: _ssl.checked
    property alias certificate: _certificateMode.currentIndex
    property alias protocol: _protocols.currentIndex
  }

  //
  // Close shortcut
  //
  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
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

    //
    // Window controls
    //
    ColumnLayout {
      id: column
      spacing: 8
      anchors.centerIn: parent

      GroupBox {
        Layout.fillWidth: true

        background: Rectangle {
          radius: 2
          border.width: 1
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          columns: 2
          rowSpacing: 0
          Layout.fillWidth: true
          columnSpacing: 8

          //
          // Version & mode titles
          //
          Label {
            text: qsTr("Version") + ":"
          } Label {
            text: qsTr("Mode") + ":"
          }

          //
          // MQTT version
          //
          ComboBox {
            id: _version
            Layout.fillWidth: true
            Layout.minimumWidth: 256
            model: Cpp_MQTT_Client.mqttVersions
            currentIndex: Cpp_MQTT_Client.mqttVersion
            onCurrentIndexChanged: {
              if (Cpp_MQTT_Client.mqttVersion !== currentIndex)
                Cpp_MQTT_Client.mqttVersion = currentIndex
            }
          }

          //
          // Client mode version
          //
          ComboBox {
            id: _mode
            Layout.fillWidth: true
            Layout.minimumWidth: 256
            model: Cpp_MQTT_Client.clientModes
            currentIndex: Cpp_MQTT_Client.clientMode
            onCurrentIndexChanged: {
              if (Cpp_MQTT_Client.clientMode !== currentIndex)
                Cpp_MQTT_Client.clientMode = currentIndex
            }
          }

          //
          // Spacers
          //
          Item {
            implicitHeight: 8
          } Item {
            implicitHeight: 8
          }

          //
          // QOS Level & Keep Alive
          //
          Label {
            opacity: enabled ? 1 : 0.5
            text: qsTr("QOS Level") + ":"
            enabled: !Cpp_MQTT_Client.isConnectedToHost
          } Label {
            opacity: enabled ? 1 : 0.5
            text: qsTr("Keep Alive (s)") + ":"
            enabled: !Cpp_MQTT_Client.isConnectedToHost
          }

          //
          // QOS
          //
          ComboBox {
            id: _qos
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            model: Cpp_MQTT_Client.qosLevels
            currentIndex: Cpp_MQTT_Client.qos
            enabled: !Cpp_MQTT_Client.isConnectedToHost

            onCurrentIndexChanged: {
              if (Cpp_MQTT_Client.qos !== currentIndex)
                Cpp_MQTT_Client.qos = currentIndex
            }
          }

          //
          // Keep alive
          //
          TextField {
            id: _keepAlive
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            placeholderText: Cpp_MQTT_Client.keepAlive
            enabled: !Cpp_MQTT_Client.isConnectedToHost
            Component.onCompleted: text = Cpp_MQTT_Client.keepAlive

            onTextChanged: {
              if (Cpp_MQTT_Client.keepAlive !== text && text.length > 0)
                Cpp_MQTT_Client.keepAlive = text

              if (text.length === 0)
                Cpp_MQTT_Client.keepAlive = 1
            }

            validator: IntValidator {
              bottom: 1
              top: 65535
            }
          }

          //
          // Spacers
          //
          Item {
            implicitHeight: 8
          } Item {
            implicitHeight: 8
          }

          //
          // Host & port titles
          //
          Label {
            text: qsTr("Host") + ":"
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_MQTT_Client.isConnectedToHost
          } Label {
            text: qsTr("Port") + ":"
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_MQTT_Client.isConnectedToHost
          }

          //
          // Host
          //
          TextField {
            id: _host
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_MQTT_Client.isConnectedToHost
            placeholderText: Cpp_MQTT_Client.defaultHost

            onTextChanged: {
              if (Cpp_MQTT_Client.host !== text && text.length > 0)
                Cpp_MQTT_Client.host = text

              else if (text.length === 0)
                Cpp_MQTT_Client.host = Cpp_MQTT_Client.defaultHost
            }
          }

          //
          // Port
          //
          TextField {
            id: _port
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_MQTT_Client.isConnectedToHost
            placeholderText: Cpp_MQTT_Client.defaultPort
            Component.onCompleted: text = Cpp_MQTT_Client.port

            onTextChanged: {
              if (Cpp_MQTT_Client.port !== text && text.length > 0)
                Cpp_MQTT_Client.port = text

              if (text.length === 0)
                Cpp_MQTT_Client.port = Cpp_MQTT_Client.defaultPort
            }

            validator: IntValidator {
              bottom: 0
              top: 65535
            }
          }

          //
          // Spacers
          //
          Item {
            implicitHeight: 8
          } Item {
            implicitHeight: 8
          }

          //
          // Topic & retain labels
          //
          Label {
            text: qsTr("Topic") + ":"
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_MQTT_Client.isConnectedToHost
          } Label {
            text: qsTr("Retain") + ":"
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_MQTT_Client.isConnectedToHost
          }

          //
          // Topic
          //
          TextField {
            id: _topic
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            text: Cpp_MQTT_Client.topic
            placeholderText: qsTr("MQTT Topic")
            enabled: !Cpp_MQTT_Client.isConnectedToHost

            onTextChanged: {
              if (Cpp_MQTT_Client.topic !== text)
                Cpp_MQTT_Client.topic = text
            }
          }

          //
          // Retain checkbox
          //
          CheckBox {
            id: _retain
            Layout.leftMargin: -6
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            text: qsTr("Add Retain Flag")
            checked: Cpp_MQTT_Client.retain
            enabled: !Cpp_MQTT_Client.isConnectedToHost

            onCheckedChanged: {
              if (Cpp_MQTT_Client.retain != checked)
                Cpp_MQTT_Client.retain = checked
            }
          }

          //
          // Spacers
          //
          Item {
            implicitHeight: 8
          } Item {
            implicitHeight: 8
          }

          //
          // Username & password titles
          //
          Label {
            text: qsTr("User") + ":"
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_MQTT_Client.isConnectedToHost
          } Label {
            text: qsTr("Password") + ":"
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_MQTT_Client.isConnectedToHost
          }

          //
          // Username
          //
          TextField {
            id: _user
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            text: Cpp_MQTT_Client.username
            placeholderText: qsTr("MQTT Username")
            enabled: !Cpp_MQTT_Client.isConnectedToHost

            onTextChanged: {
              if (Cpp_MQTT_Client.username !== text)
                Cpp_MQTT_Client.username = text
            }
          }

          //
          // Password
          //
          RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
              id: _password
              Layout.fillWidth: true
              opacity: enabled ? 1 : 0.5
              echoMode: TextField.Password
              text: Cpp_MQTT_Client.password
              placeholderText: qsTr("MQTT Password")
              enabled: !Cpp_MQTT_Client.isConnectedToHost

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
              icon.source: "qrc:/rcc/icons/buttons/visibility.svg"
              onCheckedChanged: _password.echoMode = (checked ? TextField.Normal :
                                                                TextField.Password)
            }
          }

          //
          // Spacers
          //
          Item {
            implicitHeight: 8
          } Item {
            implicitHeight: 8
          }

          //
          // Client ID + SSL Switch
          //
          Label {
            text: qsTr("Client ID:")
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_MQTT_Client.isConnectedToHost
          } Label {
            text: qsTr("Enable SSL/TLS:")
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_MQTT_Client.isConnectedToHost
          }

          //
          // Client ID
          //
          TextField {
            id: _clientId
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            text: Cpp_MQTT_Client.clientId
            placeholderText: qsTr("MQTT Client ID")
            enabled: !Cpp_MQTT_Client.isConnectedToHost

            onTextChanged: {
              if (Cpp_MQTT_Client.clientId !== text)
                Cpp_MQTT_Client.clientId = text
            }
          }

          //
          // SSL/TLS switch
          //
          Switch {
            id: _ssl
            opacity: enabled ? 1 : 0.5
            Layout.leftMargin: -8
            checked: Cpp_MQTT_Client.sslEnabled
            enabled: !Cpp_MQTT_Client.isConnectedToHost

            onCheckedChanged: {
              if (Cpp_MQTT_Client.sslEnabled !== checked)
                Cpp_MQTT_Client.sslEnabled = checked
            }
          }

          //
          // Spacers
          //
          Item {
            implicitHeight: 8
          } Item {
            implicitHeight: 8
          }

          //
          // SSL Protocol & certificate titles
          //
          Label {
            text: qsTr("Protocol:")
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_MQTT_Client.isConnectedToHost && _ssl.checked
          } Label {
            text: qsTr("Certificate:")
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_MQTT_Client.isConnectedToHost && _ssl.checked
          }

          //
          // SSL/TLS protocol selection
          //
          ComboBox {
            id: _protocols
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            model: Cpp_MQTT_Client.sslProtocols
            enabled: !Cpp_MQTT_Client.isConnectedToHost && _ssl.checked

            onCurrentIndexChanged: {
              if (currentIndex !== Cpp_MQTT_Client.sslProtocol)
                Cpp_MQTT_Client.sslProtocol = currentIndex
            }
          }

          //
          // Certificate selection
          //
          RowLayout {
            spacing: 8
            Layout.fillWidth: true

            ComboBox {
              id: _certificateMode
              Layout.fillWidth: true
              opacity: enabled ? 1 : 0.5
              enabled: !Cpp_MQTT_Client.isConnectedToHost && _ssl.checked
              model: [
                qsTr("Use System Database"),
                qsTr("Custom CA File")
              ]

              onCurrentIndexChanged: {
                if (currentIndex === 0)
                  Cpp_MQTT_Client.loadCaFile("")
              }
            }

            Button {
              icon.color: palette.text
              opacity: enabled ? 1 : 0.5
              Layout.maximumWidth: height
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_MQTT_Client.loadCaFile()
              icon.source: "qrc:/rcc/icons/buttons/open.svg"
              enabled: _certificateMode.currentIndex === 1  && _ssl.checked
            }
          }

          //
          // Spacers
          //
          Item {
            implicitHeight: 8
          } Item {
            implicitHeight: 8
          }
        }
      }

      //
      // Buttons
      //
      RowLayout {
        spacing: 4
        Layout.fillWidth: true

        Button {
          icon.width: 18
          icon.height: 18
          onClicked: root.close()
          text: qsTr("Close") + "  "
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
          checked: Cpp_MQTT_Client.isConnectedToHost
          onClicked: Cpp_MQTT_Client.toggleConnection()
          icon.color: Cpp_ThemeManager.colors["button_text"]
          text: (checked ? qsTr("Disconnect") : qsTr("Connect")) + "  "
          icon.source: checked ? "qrc:/rcc/icons/buttons/connected.svg" :
                                 "qrc:/rcc/icons/buttons/disconnected.svg"
        }
      }
    }
  }
}
