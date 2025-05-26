/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
  id: root
  implicitHeight: layout.implicitHeight

  //
  // Save settings
  //
  Settings {
    category: "NetworkDriver"
    property alias address: _address.text
    property alias tcpPort: _tcpPort.text
    property alias udpLocalPort: _udpLocalPort.text
    property alias udpRemotePort: _udpRemotePort.text
    property alias socketType: _typeCombo.currentIndex
    property alias udpMulticastEnabled: _udpMulticast.checked
  }

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
        enabled: !Cpp_IO_Manager.isConnected
      } ComboBox {
        id: _typeCombo
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        model: Cpp_IO_Network.socketTypes
        enabled: !Cpp_IO_Manager.isConnected
        currentIndex: Cpp_IO_Network.socketTypeIndex
        onCurrentIndexChanged: {
          if (currentIndex !== Cpp_IO_Network.socketTypeIndex)
            Cpp_IO_Network.socketTypeIndex = currentIndex
        }
      }

      //
      // Address
      //
      Label {
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.isConnected
        text: qsTr("Remote Address") + ":"
      } TextField {
        id: _address
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.isConnected
        placeholderText: Cpp_IO_Network.defaultAddress
        Component.onCompleted: text = Cpp_IO_Network.remoteAddress
        onTextChanged: {
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
        text: qsTr("Port") + ":"
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.isConnected
        visible: Cpp_IO_Network.socketTypeIndex === 0
      } TextField {
        id: _tcpPort
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.isConnected
        placeholderText: Cpp_IO_Network.defaultTcpPort
        Component.onCompleted: text = Cpp_IO_Network.tcpPort
        onTextChanged: {
          if (Cpp_IO_Network.tcpPort !== text && text.length > 0)
            Cpp_IO_Network.tcpPort = text

          if (text.length === 0)
            Cpp_IO_Network.port = Cpp_IO_Network.defaultTcpPort
        }

        validator: IntValidator {
          bottom: 0
          top: 65535
        }


        visible: Cpp_IO_Network.socketTypeIndex === 0
      }


      //
      // TCP port
      //
      Label {
        opacity: enabled ? 1 : 0.5
        text: qsTr("Local Port") + ":"
        enabled: !Cpp_IO_Manager.isConnected
        visible: Cpp_IO_Network.socketTypeIndex === 1
      } TextField {
        id: _udpLocalPort
        Layout.fillWidth: true
        placeholderText: qsTr("Type 0 for automatic port")
        Component.onCompleted: text = Cpp_IO_Network.udpLocalPort
        onTextChanged: {
          if (Cpp_IO_Network.udpLocalPort !== text && text.length > 0)
            Cpp_IO_Network.udpLocalPort = text

          if (text.length === 0)
            Cpp_IO_Network.udpLocalPort = Cpp_IO_Network.defaultUdpLocalPort
        }

        validator: IntValidator {
          bottom: 0
          top: 65535
        }

        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.isConnected
        visible: Cpp_IO_Network.socketTypeIndex === 1
      }

      //
      // Output port
      //
      Label {
        opacity: enabled ? 1 : 0.5
        text: qsTr("Remote Port") + ":"
        enabled: !Cpp_IO_Manager.isConnected
        visible: Cpp_IO_Network.socketTypeIndex === 1 && !_udpMulticast.checked
      } TextField {
        id: _udpRemotePort
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.isConnected
        placeholderText: Cpp_IO_Network.defaultUdpRemotePort
        Component.onCompleted: text = Cpp_IO_Network.udpRemotePort
        visible: Cpp_IO_Network.socketTypeIndex === 1 && !_udpMulticast.checked

        onTextChanged: {
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
        visible: Cpp_IO_Network.socketTypeIndex === 1
      } CheckBox {
        id: _udpMulticast
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignLeft
        Layout.leftMargin: -8
        checked: Cpp_IO_Network.udpMulticast
        visible: Cpp_IO_Network.socketTypeIndex === 1
        enabled: Cpp_IO_Network.socketTypeIndex === 1 && !Cpp_IO_Manager.isConnected

        onCheckedChanged: {
          if (Cpp_IO_Network.udpMulticast !== checked)
            Cpp_IO_Network.udpMulticast = checked
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
