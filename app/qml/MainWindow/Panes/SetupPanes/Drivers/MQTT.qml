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

  //
  // Enabling Sparkplug reveals rows below the checkbox; a scrolled pane would hide them, so the
  // hosting Flickable is nudged just far enough to show the last of them.
  //
  property bool sparkplugWasEnabled: false

  Component.onCompleted: root.sparkplugWasEnabled = Cpp_IO_Mqtt.sparkplugEnabled

  function hostFlickable() {
    let item = root.parent
    for (let i = 0; i < 16 && item; ++i) {
      if (item.contentY !== undefined && item.contentHeight !== undefined)
        return item

      item = item.parent
    }

    return null
  }

  function revealSparkplugRows() {
    const flick = root.hostFlickable()
    if (!flick || flick.contentHeight <= flick.height || !_generate.visible)
      return

    const bottom = _generate.mapToItem(flick.contentItem, 0, _generate.height).y
    const target = Math.min(bottom + 8 - flick.height, flick.contentHeight - flick.height)
    if (target > flick.contentY)
      flick.contentY = Math.max(0, target)
  }

  Timer {
    id: _revealTimer

    interval: 50
    repeat: false
    onTriggered: root.revealSparkplugRows()
  }

  Connections {
    target: Cpp_IO_Mqtt
    function onMqttConfigurationChanged() {
      const on = Cpp_IO_Mqtt.sparkplugEnabled
      if (on && !root.sparkplugWasEnabled)
        _revealTimer.restart()

      root.sparkplugWasEnabled = on
    }
  }

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
      // Sparkplug
      //
      Label {
        text: qsTr("Sparkplug") + ":"
        enabled: app.ioEnabled
        opacity: _sparkplug.enabled ? 1 : 0.5
      } CheckBox {
        id: _sparkplug

        Layout.leftMargin: -8
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignLeft
        checked: Cpp_IO_Mqtt.sparkplugEnabled
        onClicked: Cpp_IO_Mqtt.sparkplugEnabled = checked

        Connections {
          target: Cpp_IO_Mqtt
          function onMqttConfigurationChanged() {
            if (_sparkplug.checked !== Cpp_IO_Mqtt.sparkplugEnabled)
              _sparkplug.checked = Cpp_IO_Mqtt.sparkplugEnabled
          }
        }
      }

      //
      // Sparkplug group ID
      //
      Label {
        text: qsTr("Group ID") + ":"
        visible: Cpp_IO_Mqtt.sparkplugEnabled
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sparkplugEnabled
        opacity: enabled ? 1 : 0.5
      } Widgets.BoundField {
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        placeholderText: qsTr("All groups")
        visible: Cpp_IO_Mqtt.sparkplugEnabled
        externalValue: Cpp_IO_Mqtt.sparkplugGroupId
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sparkplugEnabled
        onEdited: text => Cpp_IO_Mqtt.sparkplugGroupId = text
      }

      //
      // Sparkplug project generation
      //
      Label {
        text: qsTr("Sparkplug Project") + ":"
        visible: Cpp_IO_Mqtt.sparkplugEnabled
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sparkplugEnabled
        opacity: enabled ? 1 : 0.5
      } Button {
        id: _generate

        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        visible: Cpp_IO_Mqtt.sparkplugEnabled
        text: qsTr("Create Project from Births")
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sparkplugEnabled && Cpp_IO_Manager.isConnected
        onClicked: Cpp_IO_Mqtt.generateProject()
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

      //
      // Client certificate (mutual TLS)
      //
      Label {
        text: qsTr("Client Certificate") + ":"
        visible: Cpp_IO_Mqtt.sslEnabled
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
        opacity: enabled ? 1 : 0.5
      } RowLayout {
        spacing: 4
        Layout.fillWidth: true
        visible: Cpp_IO_Mqtt.sslEnabled

        Widgets.BoundField {
          Layout.fillWidth: true
          opacity: enabled ? 1 : 0.5
          placeholderText: qsTr("Optional (mutual TLS)")
          enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
          externalValue: Cpp_IO_Mqtt.clientCertificatePath
          onEdited: text => Cpp_IO_Mqtt.clientCertificatePath = text
        }

        Button {
          text: qsTr("Browse…")
          opacity: enabled ? 1 : 0.5
          enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
          onClicked: Cpp_IO_Mqtt.selectClientCertificate()
        }
      }

      //
      // Private key (mutual TLS)
      //
      Label {
        text: qsTr("Private Key") + ":"
        visible: Cpp_IO_Mqtt.sslEnabled
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
        opacity: enabled ? 1 : 0.5
      } RowLayout {
        spacing: 4
        Layout.fillWidth: true
        visible: Cpp_IO_Mqtt.sslEnabled

        Widgets.BoundField {
          Layout.fillWidth: true
          opacity: enabled ? 1 : 0.5
          externalValue: Cpp_IO_Mqtt.privateKeyPath
          enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
          onEdited: text => Cpp_IO_Mqtt.privateKeyPath = text
          placeholderText: qsTr("Defaults to the certificate file")
        }

        Button {
          text: qsTr("Browse…")
          opacity: enabled ? 1 : 0.5
          enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
          onClicked: Cpp_IO_Mqtt.selectPrivateKey()
        }
      }

      //
      // Private key passphrase
      //
      Label {
        text: qsTr("Key Passphrase") + ":"
        visible: Cpp_IO_Mqtt.sslEnabled
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
        opacity: enabled ? 1 : 0.5
      } Widgets.BoundField {
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        echoMode: TextInput.Password
        visible: Cpp_IO_Mqtt.sslEnabled
        externalValue: Cpp_IO_Mqtt.keyPassphrase
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
        onEdited: text => Cpp_IO_Mqtt.keyPassphrase = text
      }

      //
      // ALPN (MQTT over port 443)
      //
      Label {
        text: qsTr("ALPN") + ":"
        visible: Cpp_IO_Mqtt.sslEnabled
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled
        opacity: _alpn.enabled ? 1 : 0.5
      } CheckBox {
        id: _alpn

        Layout.leftMargin: -8
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignLeft
        visible: Cpp_IO_Mqtt.sslEnabled
        checked: Cpp_IO_Mqtt.alpnEnabled
        onClicked: Cpp_IO_Mqtt.alpnEnabled = checked
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled

        Connections {
          target: Cpp_IO_Mqtt
          function onSslConfigurationChanged() {
            if (_alpn.checked !== Cpp_IO_Mqtt.alpnEnabled)
              _alpn.checked = Cpp_IO_Mqtt.alpnEnabled
          }
        }
      }

      //
      // ALPN protocol name
      //
      Label {
        text: qsTr("ALPN Protocol") + ":"
        visible: Cpp_IO_Mqtt.sslEnabled && Cpp_IO_Mqtt.alpnEnabled
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled && Cpp_IO_Mqtt.alpnEnabled
        opacity: enabled ? 1 : 0.5
      } Widgets.BoundField {
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        placeholderText: "x-amzn-mqtt-ca"
        externalValue: Cpp_IO_Mqtt.alpnProtocol
        onEdited: text => Cpp_IO_Mqtt.alpnProtocol = text
        visible: Cpp_IO_Mqtt.sslEnabled && Cpp_IO_Mqtt.alpnEnabled
        enabled: app.ioEnabled && Cpp_IO_Mqtt.sslEnabled && Cpp_IO_Mqtt.alpnEnabled
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
