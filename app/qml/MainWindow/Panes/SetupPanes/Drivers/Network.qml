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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../../../../Widgets" as Widgets

Item {
  id: root

  implicitHeight: layout.implicitHeight
  implicitWidth: layout.implicitWidth + 16

  //
  // Transport aliases, so no row carries a four-way index comparison
  //
  readonly property bool isTcp: Cpp_IO_Network.socketTypeIndex === 0
  readonly property bool isUdp: Cpp_IO_Network.socketTypeIndex === 1
  readonly property bool isHttp: Cpp_IO_Network.socketTypeIndex === 3
  readonly property bool isUrlTransport: root.isWebSocket || root.isHttp
  readonly property bool isWebSocket: Cpp_IO_Network.socketTypeIndex === 2

  //
  // React to network manager events
  //
  Connections {
    target: Cpp_IO_Network

    function onAddressChanged() {
      if (_address.text.length > 0)
        _address.text = Cpp_IO_Network.remoteAddress
    }

    function onPortChanged() {
      if (_tcpPort.text.length > 0)
        _tcpPort.text = Cpp_IO_Network.tcpPort

      if (_udpLocalPort.text.length > 0)
        _udpLocalPort.text = Cpp_IO_Network.udpLocalPort

      if (_udpRemotePort.text.length > 0)
        _udpRemotePort.text = Cpp_IO_Network.udpRemotePort
    }

    function onWebSocketChanged() {
      if (!_wsUrl.activeFocus)
        _wsUrl.text = Cpp_IO_Network.webSocketUrl
    }

    function onHttpChanged() {
      if (!_httpUrl.activeFocus)
        _httpUrl.text = Cpp_IO_Network.httpUrl

      if (!_httpBody.activeFocus)
        _httpBody.text = Cpp_IO_Network.httpBody

      if (!_httpHeaders.activeFocus)
        _httpHeaders.text = Cpp_IO_Network.httpHeaders

      if (!_httpInterval.activeFocus)
        _httpInterval.text = Cpp_IO_Network.httpInterval
    }
  }

  //
  // Layout
  //
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
      // Socket type
      //
      Label {
        opacity: enabled ? 1 : 0.5
        text: qsTr("Socket Type") + ":"
        enabled: app.ioEnabled
      } Widgets.Combo {
        id: _typeCombo

        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        model: Cpp_IO_Network.socketTypes
        currentIndex: Cpp_IO_Network.socketTypeIndex
        onActivated: (index) => {
          if (index !== Cpp_IO_Network.socketTypeIndex)
            Cpp_IO_Network.socketTypeIndex = index
        }
      }

      //
      // UDP port
      //
      Label {
        opacity: enabled ? 1 : 0.5
        text: qsTr("Local Port") + ":"
        enabled: app.ioEnabled
        visible: root.isUdp
      } Widgets.LineField {
        id: _udpLocalPort

        Layout.fillWidth: true
        placeholderText: qsTr("Type 0 for automatic port")
        Component.onCompleted: text = Cpp_IO_Network.udpLocalPort
        onTextEdited: {
          const value = parseInt(text)
          if (!isNaN(value) && Cpp_IO_Network.udpLocalPort !== value)
            Cpp_IO_Network.udpLocalPort = value
        }
        onEditingFinished: {
          if (Cpp_IO_Network.udpLocalPort !== text && text.length > 0)
            Cpp_IO_Network.udpLocalPort = text

          if (text.length === 0)
            Cpp_IO_Network.udpLocalPort = Cpp_IO_Network.defaultUdpLocalPort
        }

        validator: IntValidator {
          bottom: 0
          top: 65535
        }

        visible: root.isUdp
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
      }

      //
      // Address
      //
      Label {
        opacity: enabled ? 1 : 0.5
        enabled: app.ioEnabled
        visible: root.isTcp || root.isUdp
        text: qsTr("Remote Address") + ":"
      } Widgets.LineField {
        id: _address

        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        visible: root.isTcp || root.isUdp
        placeholderText: Cpp_IO_Network.defaultAddress
        Component.onCompleted: text = Cpp_IO_Network.remoteAddress
        onTextEdited: {
          if (Cpp_IO_Network.remoteAddress !== text && text.length > 0)
            Cpp_IO_Network.remoteAddress = text
        }
        onEditingFinished: {
          if (Cpp_IO_Network.remoteAddress !== text && text.length > 0)
            Cpp_IO_Network.remoteAddress = text

          if (text.length === 0)
            Cpp_IO_Network.remoteAddress = Cpp_IO_Network.defaultAddress
        }
      }

      //
      // TCP port
      //
      Label {
        opacity: enabled ? 1 : 0.5
        enabled: app.ioEnabled
        text: qsTr("Remote Port") + ":"
        visible: root.isTcp
      } Widgets.LineField {
        id: _tcpPort

        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        placeholderText: Cpp_IO_Network.defaultTcpPort
        Component.onCompleted: text = Cpp_IO_Network.tcpPort
        onTextEdited: {
          const value = parseInt(text)
          if (!isNaN(value) && Cpp_IO_Network.tcpPort !== value)
            Cpp_IO_Network.tcpPort = value
        }
        onEditingFinished: {
          if (Cpp_IO_Network.tcpPort !== text && text.length > 0)
            Cpp_IO_Network.tcpPort = text

          if (text.length === 0)
            Cpp_IO_Network.tcpPort = Cpp_IO_Network.defaultTcpPort
        }

        validator: IntValidator {
          bottom: 0
          top: 65535
        }


        visible: root.isTcp
      }

      //
      // Output port
      //
      Label {
        opacity: enabled ? 1 : 0.5
        enabled: app.ioEnabled
        text: qsTr("Remote Port") + ":"
        visible: root.isUdp && !_udpMulticast.checked
      } Widgets.LineField {
        id: _udpRemotePort

        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        visible: root.isUdp && !_udpMulticast.checked
        placeholderText: Cpp_IO_Network.defaultUdpRemotePort
        Component.onCompleted: text = Cpp_IO_Network.udpRemotePort

        onTextEdited: {
          const value = parseInt(text)
          if (!isNaN(value) && Cpp_IO_Network.udpRemotePort !== value)
            Cpp_IO_Network.udpRemotePort = value
        }
        onEditingFinished: {
          if (Cpp_IO_Network.udpRemotePort !== text && text.length > 0)
            Cpp_IO_Network.udpRemotePort = text

          if (text.length === 0)
            Cpp_IO_Network.udpRemotePort = Cpp_IO_Network.defaultUdpRemotePort
        }

        validator: IntValidator {
          bottom: 0
          top: 65535
        }
      }

      //
      // UDP multicast checkbox
      //
      Label {
        text: qsTr("Multicast") + ":"
        opacity: _udpMulticast.enabled ? 1 : 0.5
        visible: root.isUdp
      } CheckBox {
        id: _udpMulticast

        visible: root.isUdp
        Layout.leftMargin: -8
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignLeft
        checked: Cpp_IO_Network.udpMulticast
        enabled: root.isUdp && app.ioEnabled

        onCheckedChanged: {
          if (Cpp_IO_Network.udpMulticast !== checked)
            Cpp_IO_Network.udpMulticast = checked
        }
      }

      //
      // WebSocket URL
      //
      Label {
        opacity: enabled ? 1 : 0.5
        enabled: app.ioEnabled
        visible: root.isWebSocket
        text: qsTr("URL") + ":"
      } Widgets.LineField {
        id: _wsUrl

        Layout.fillWidth: true
        enabled: app.ioEnabled
        visible: root.isWebSocket
        opacity: enabled ? 1 : 0.5
        placeholderText: Cpp_IO_Network.defaultWebSocketUrl
        Component.onCompleted: text = Cpp_IO_Network.webSocketUrl
        onTextEdited: {
          if (Cpp_IO_Network.webSocketUrl !== text)
            Cpp_IO_Network.webSocketUrl = text
        }
        onEditingFinished: {
          if (text.length === 0)
            Cpp_IO_Network.webSocketUrl = Cpp_IO_Network.defaultWebSocketUrl
          else if (Cpp_IO_Network.webSocketUrl !== text)
            Cpp_IO_Network.webSocketUrl = text
        }
      }

      //
      // WebSocket send format
      //
      Label {
        opacity: enabled ? 1 : 0.5
        enabled: app.ioEnabled
        visible: root.isWebSocket
        text: qsTr("Send Format") + ":"
      } Widgets.Combo {
        id: _wsFormat

        Layout.fillWidth: true
        enabled: app.ioEnabled
        visible: root.isWebSocket
        opacity: enabled ? 1 : 0.5
        model: Cpp_IO_Network.webSocketFormats
        currentIndex: Cpp_IO_Network.webSocketFormatIndex
        onActivated: (index) => {
          if (index !== Cpp_IO_Network.webSocketFormatIndex)
            Cpp_IO_Network.webSocketFormatIndex = index
        }
      }

      //
      // HTTP URL
      //
      Label {
        opacity: enabled ? 1 : 0.5
        enabled: app.ioEnabled
        visible: root.isHttp
        text: qsTr("URL") + ":"
      } Widgets.LineField {
        id: _httpUrl

        visible: root.isHttp
        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        placeholderText: Cpp_IO_Network.defaultHttpUrl
        Component.onCompleted: text = Cpp_IO_Network.httpUrl
        onTextEdited: {
          if (Cpp_IO_Network.httpUrl !== text)
            Cpp_IO_Network.httpUrl = text
        }
        onEditingFinished: {
          if (text.length === 0)
            Cpp_IO_Network.httpUrl = Cpp_IO_Network.defaultHttpUrl
          else if (Cpp_IO_Network.httpUrl !== text)
            Cpp_IO_Network.httpUrl = text
        }
      }

      //
      // HTTP method
      //
      Label {
        opacity: enabled ? 1 : 0.5
        enabled: app.ioEnabled
        visible: root.isHttp
        text: qsTr("Method") + ":"
      } Widgets.Combo {
        id: _httpMethod

        visible: root.isHttp
        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        model: Cpp_IO_Network.httpMethods
        currentIndex: Cpp_IO_Network.httpMethodIndex
        onActivated: (index) => {
          if (index !== Cpp_IO_Network.httpMethodIndex)
            Cpp_IO_Network.httpMethodIndex = index
        }
      }

      //
      // HTTP poll interval
      //
      Label {
        opacity: enabled ? 1 : 0.5
        enabled: app.ioEnabled
        visible: root.isHttp
        text: qsTr("Poll Interval") + ":"
      } Widgets.LineField {
        id: _httpInterval

        visible: root.isHttp
        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        Component.onCompleted: text = Cpp_IO_Network.httpInterval
        placeholderText: qsTr("Milliseconds; 0 sends only on write")
        onTextEdited: {
          const value = parseInt(text)
          if (!isNaN(value) && Cpp_IO_Network.httpInterval !== value)
            Cpp_IO_Network.httpInterval = value
        }
        onEditingFinished: {
          if (text.length === 0)
            Cpp_IO_Network.httpInterval = Cpp_IO_Network.defaultHttpInterval
        }

        validator: IntValidator {
          bottom: 0
          top: 3600000
        }
      }

      //
      // HTTP request body
      //
      Label {
        opacity: enabled ? 1 : 0.5
        enabled: app.ioEnabled
        visible: root.isHttp
        text: qsTr("Request Body") + ":"
      } Widgets.LineField {
        id: _httpBody

        visible: root.isHttp
        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        placeholderText: qsTr("Optional")
        Component.onCompleted: text = Cpp_IO_Network.httpBody
        onTextEdited: {
          if (Cpp_IO_Network.httpBody !== text)
            Cpp_IO_Network.httpBody = text
        }
        onEditingFinished: {
          if (Cpp_IO_Network.httpBody !== text)
            Cpp_IO_Network.httpBody = text
        }
      }

      //
      // HTTP request headers
      //
      Label {
        opacity: enabled ? 1 : 0.5
        enabled: app.ioEnabled
        visible: root.isHttp
        Layout.alignment: Qt.AlignTop
        text: qsTr("Request Headers") + ":"
      } Rectangle {
        Layout.fillWidth: true
        visible: root.isHttp
        Layout.preferredHeight: 72
        opacity: app.ioEnabled ? 1 : 0.5
        color: Cpp_ThemeManager.colors["base"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        ScrollView {
          clip: true
          anchors.margins: 4
          anchors.fill: parent

          TextArea {
            id: _httpHeaders

            selectByMouse: true
            enabled: app.ioEnabled
            wrapMode: TextArea.WrapAnywhere
            color: Cpp_ThemeManager.colors["text"]
            placeholderText: qsTr("One %1 pair per line").arg("Name: Value")
            Component.onCompleted: text = Cpp_IO_Network.httpHeaders
            onEditingFinished: {
              if (Cpp_IO_Network.httpHeaders !== text)
                Cpp_IO_Network.httpHeaders = text
            }
          }
        }
      }

      //
      // TLS bypass, shared by the URL transports
      //
      Label {
        opacity: _ignoreTls.enabled ? 1 : 0.5
        visible: root.isUrlTransport
        text: qsTr("Ignore TLS Errors") + ":"
      } CheckBox {
        id: _ignoreTls

        Layout.leftMargin: -8
        opacity: enabled ? 1 : 0.5
        visible: root.isUrlTransport
        Layout.alignment: Qt.AlignLeft
        checked: Cpp_IO_Network.ignoreTlsErrors
        enabled: root.isUrlTransport && app.ioEnabled

        onCheckedChanged: {
          if (Cpp_IO_Network.ignoreTlsErrors !== checked)
            Cpp_IO_Network.ignoreTlsErrors = checked
        }
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
