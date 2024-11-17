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
import QtQuick.Layouts
import QtQuick.Controls

Item {
  id: root
  implicitHeight: layout.implicitHeight

  //
  // Access to properties
  //
  property alias address: _address.text
  property alias tcpPort: _tcpPort.text
  property alias udpLocalPort: _udpLocalPort.text
  property alias udpRemotePort: _udpRemotePort.text
  property alias socketType: _typeCombo.currentIndex
  property alias udpMulticastEnabled: _udpMulticast.checked

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
        text: qsTr("Socket type") + ":"
        enabled: !Cpp_IO_Manager.connected
      } ComboBox {
        id: _typeCombo
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        model: Cpp_IO_Network.socketTypes
        enabled: !Cpp_IO_Manager.connected
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
        enabled: !Cpp_IO_Manager.connected
        text: qsTr("Remote address") + ":"
      } TextField {
        id: _address
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.connected
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
        enabled: !Cpp_IO_Manager.connected
        visible: Cpp_IO_Network.socketTypeIndex === 0
      } TextField {
        id: _tcpPort
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.connected
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
        text: qsTr("Local port") + ":"
        enabled: !Cpp_IO_Manager.connected
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
        enabled: !Cpp_IO_Manager.connected
        visible: Cpp_IO_Network.socketTypeIndex === 1
      }

      //
      // Output port
      //
      Label {
        opacity: enabled ? 1 : 0.5
        text: qsTr("Remote port") + ":"
        enabled: !Cpp_IO_Manager.connected
        visible: Cpp_IO_Network.socketTypeIndex === 1 && !udpMulticastEnabled
      } TextField {
        id: _udpRemotePort
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        enabled: !Cpp_IO_Manager.connected
        placeholderText: Cpp_IO_Network.defaultUdpRemotePort
        Component.onCompleted: text = Cpp_IO_Network.udpRemotePort
        visible: Cpp_IO_Network.socketTypeIndex === 1 && !udpMulticastEnabled

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
        enabled: Cpp_IO_Network.socketTypeIndex === 1 && !Cpp_IO_Manager.connected

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
