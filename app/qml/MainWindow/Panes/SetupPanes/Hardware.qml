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
import QtCore as QtSettings

import "Devices" as Devices

Item {
  id: root
  implicitHeight: layout.implicitHeight + 32

  //
  // Save settings
  //
  QtSettings.Settings {
    category: "HardwareSetup"
    property alias serialDtr: serial.dtr
    property alias serialParity: serial.parity
    property alias serialBaudRate: serial.baudRate
    property alias serialDataBits: serial.dataBits
    property alias serialStopBits: serial.stopBits
    property alias serialFlowControl: serial.flowControl
    property alias serialAutoReconnect: serial.autoReconnect
    property alias serialIgnoreDataDelimeters: serial.ignoreDataDelimeters

    property alias networkAddress: network.address
    property alias newtorkTcpPort: network.tcpPort
    property alias networkSocketType: network.socketType
    property alias networkUdpLocalPort: network.udpLocalPort
    property alias networkUdpRemotePort: network.udpRemotePort
    property alias networkUdpMulticastEnabled: network.udpMulticastEnabled
    property alias networkIgnoreDataDelimeters: network.ignoreDataDelimeters

    property alias bleIgnoreDataDelimeters: bluetoothLE.ignoreDataDelimeters
  }

  //
  // Background
  //
  Rectangle {
    radius: 2
    border.width: 1
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["groupbox_background"]
    border.color: Cpp_ThemeManager.colors["groupbox_border"]
  }

  //
  // Layout
  //
  ColumnLayout {
    id: layout
    anchors.margins: 8
    anchors.fill: parent

    //
    // Device configuration
    //
    StackLayout {
      id: stack
      clip: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      currentIndex: Cpp_IO_Manager.busType
      implicitHeight: Math.max(serial.implicitHeight, network.implicitHeight, bluetoothLE.implicitHeight)

      Devices.Serial {
        id: serial
        Layout.fillWidth: true
        Layout.fillHeight: true
      }

      Devices.Network {
        id: network
        Layout.fillWidth: true
        Layout.fillHeight: true
      }

      Devices.BluetoothLE {
        id: bluetoothLE
        Layout.fillWidth: true
        Layout.fillHeight: true
      }
    }
  }
}
