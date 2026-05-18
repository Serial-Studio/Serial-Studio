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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../../../../Widgets" as Widgets

Item {
  id: root

  implicitHeight: layout.implicitHeight
  implicitWidth: layout.implicitWidth + 16

  ColumnLayout {
    id: layout

    anchors.margins: 0
    anchors.fill: parent

    GridLayout {
      columns: 2
      rowSpacing: 4
      columnSpacing: 4
      Layout.fillWidth: true

      //
      // Hostname
      //
      Label {
        text: qsTr("Hostname") + ":"
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
      } Widgets.BoundField {
        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        externalValue: Cpp_IO_Mqtt.hostname
        placeholderText: qsTr("e.g. broker.hivemq.com")
        onEdited: text => Cpp_IO_Mqtt.hostname = text
      }

      //
      // Port
      //
      Label {
        text: qsTr("Port") + ":"
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
      } Widgets.BoundField {
        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        placeholderText: "1883"
        externalValue: Cpp_IO_Mqtt.port
        validator: IntValidator { bottom: 1; top: 65535 }
        onEdited: text => {
          if (text.length > 0)
            Cpp_IO_Mqtt.port = parseInt(text)
        }
      }

      //
      // Topic filter
      //
      Label {
        text: qsTr("Topic Filter") + ":"
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
      } Widgets.BoundField {
        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        externalValue: Cpp_IO_Mqtt.topicFilter
        placeholderText: qsTr("e.g. sensors/#")
        onEdited: text => Cpp_IO_Mqtt.topicFilter = text
      }

      //
      // Client ID + regenerate
      //
      Label {
        text: qsTr("Client ID") + ":"
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
      } RowLayout {
        spacing: 4
        Layout.fillWidth: true

        Widgets.BoundField {
          Layout.fillWidth: true
          enabled: app.ioEnabled
          opacity: enabled ? 1 : 0.5
          externalValue: Cpp_IO_Mqtt.clientId
          onEdited: text => Cpp_IO_Mqtt.clientId = text
        }

        Button {
          enabled: app.ioEnabled
          text: qsTr("Regenerate")
          opacity: enabled ? 1 : 0.5
          onClicked: Cpp_IO_Mqtt.regenerateClientId()
        }
      }

      //
      // Username
      //
      Label {
        text: qsTr("Username") + ":"
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
      } Widgets.BoundField {
        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        externalValue: Cpp_IO_Mqtt.username
        onEdited: text => Cpp_IO_Mqtt.username = text
      }

      //
      // Password
      //
      Label {
        text: qsTr("Password") + ":"
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
      } Widgets.BoundField {
        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        echoMode: TextInput.Password
        externalValue: Cpp_IO_Mqtt.password
        onEdited: text => Cpp_IO_Mqtt.password = text
      }

      //
      // MQTT version
      //
      Label {
        text: qsTr("Version") + ":"
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
      } Widgets.Combo {
        id: _version

        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        model: Cpp_IO_Mqtt.mqttVersions
        currentIndex: Cpp_IO_Mqtt.mqttVersion
        onActivated: Cpp_IO_Mqtt.mqttVersion = currentIndex

        Connections {
          target: Cpp_IO_Mqtt
          function onMqttConfigurationChanged() {
            if (_version.currentIndex !== Cpp_IO_Mqtt.mqttVersion)
              _version.currentIndex = Cpp_IO_Mqtt.mqttVersion
          }
        }
      }

      //
      // Keep alive
      //
      Label {
        text: qsTr("Keep Alive (s)") + ":"
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
      } Widgets.BoundField {
        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        externalValue: Cpp_IO_Mqtt.keepAlive
        validator: IntValidator { bottom: 0; top: 65535 }
        onEdited: text => {
          if (text.length > 0)
            Cpp_IO_Mqtt.keepAlive = parseInt(text)
        }
      }

      //
      // Clean session
      //
      Label {
        text: qsTr("Clean Session") + ":"
        enabled: app.ioEnabled
        opacity: _clean.enabled ? 1 : 0.5
      } CheckBox {
        id: _clean

        Layout.leftMargin: -8
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignLeft
        checked: Cpp_IO_Mqtt.cleanSession
        onClicked: Cpp_IO_Mqtt.cleanSession = checked

        Connections {
          target: Cpp_IO_Mqtt
          function onMqttConfigurationChanged() {
            if (_clean.checked !== Cpp_IO_Mqtt.cleanSession)
              _clean.checked = Cpp_IO_Mqtt.cleanSession
          }
        }
      }

      //
      // SSL / TLS
      //
      Label {
        text: qsTr("Use SSL/TLS") + ":"
        enabled: app.ioEnabled
        opacity: _ssl.enabled ? 1 : 0.5
      } CheckBox {
        id: _ssl

        Layout.leftMargin: -8
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignLeft
        checked: Cpp_IO_Mqtt.sslEnabled
        onClicked: Cpp_IO_Mqtt.sslEnabled = checked

        Connections {
          target: Cpp_IO_Mqtt
          function onSslConfigurationChanged() {
            if (_ssl.checked !== Cpp_IO_Mqtt.sslEnabled)
              _ssl.checked = Cpp_IO_Mqtt.sslEnabled
          }
        }
      }

      //
      // SSL protocol
      //
      Label {
        text: qsTr("SSL Protocol") + ":"
        visible: Cpp_IO_Mqtt.sslEnabled
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
        opacity: enabled ? 1 : 0.5
      } Widgets.Combo {
        id: _sslProtocol

        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        visible: Cpp_IO_Mqtt.sslEnabled
        model: Cpp_IO_Mqtt.sslProtocols
        currentIndex: Cpp_IO_Mqtt.sslProtocol
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
        onActivated: {
          if (Cpp_IO_Mqtt.sslEnabled)
            Cpp_IO_Mqtt.sslProtocol = currentIndex
        }

        Connections {
          target: Cpp_IO_Mqtt
          function onSslConfigurationChanged() {
            if (_sslProtocol.currentIndex !== Cpp_IO_Mqtt.sslProtocol)
              _sslProtocol.currentIndex = Cpp_IO_Mqtt.sslProtocol
          }
        }
      }

      //
      // Peer verify mode
      //
      Label {
        text: qsTr("Peer Verify") + ":"
        visible: Cpp_IO_Mqtt.sslEnabled
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
        opacity: enabled ? 1 : 0.5
      } Widgets.Combo {
        id: _peerVerifyMode

        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        visible: Cpp_IO_Mqtt.sslEnabled
        model: Cpp_IO_Mqtt.peerVerifyModes
        currentIndex: Cpp_IO_Mqtt.peerVerifyMode
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
        onActivated: {
          if (Cpp_IO_Mqtt.sslEnabled)
            Cpp_IO_Mqtt.peerVerifyMode = currentIndex
        }

        Connections {
          target: Cpp_IO_Mqtt
          function onSslConfigurationChanged() {
            if (_peerVerifyMode.currentIndex !== Cpp_IO_Mqtt.peerVerifyMode)
              _peerVerifyMode.currentIndex = Cpp_IO_Mqtt.peerVerifyMode
          }
        }
      }

      //
      // Peer verify depth
      //
      Label {
        text: qsTr("Verify Depth") + ":"
        visible: Cpp_IO_Mqtt.sslEnabled
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
        opacity: enabled ? 1 : 0.5
      } Widgets.BoundField {
        Layout.fillWidth: true
        visible: Cpp_IO_Mqtt.sslEnabled
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
        opacity: enabled ? 1 : 0.5
        externalValue: Cpp_IO_Mqtt.peerVerifyDepth
        validator: IntValidator { bottom: 0; top: 100 }
        onEdited: text => {
          if (text.length > 0)
            Cpp_IO_Mqtt.peerVerifyDepth = parseInt(text)
        }
      }

      //
      // CA certificates
      //
      Label {
        text: qsTr("CA Certificates") + ":"
        visible: Cpp_IO_Mqtt.sslEnabled
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
        opacity: enabled ? 1 : 0.5
      } Button {
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        visible: Cpp_IO_Mqtt.sslEnabled
        text: qsTr("Load From Folder…")
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
        onClicked: Cpp_IO_Mqtt.addCaCertificates()
      }
    }

    //
    // Spacer
    //
    Item {
      Layout.fillHeight: true
    }
  }
}
