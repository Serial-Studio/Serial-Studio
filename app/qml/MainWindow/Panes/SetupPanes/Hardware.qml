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
    property alias dtr: serial.dtr
    property alias parity: serial.parity
    property alias baudRate: serial.baudRate
    property alias dataBits: serial.dataBits
    property alias stopBits: serial.stopBits
    property alias flowControl: serial.flowControl
    property alias autoReconnect: serial.autoReconnect

    property alias address: network.address
    property alias tcpPort: network.tcpPort
    property alias socketType: network.socketType
    property alias udpLocalPort: network.udpLocalPort
    property alias udpRemotePort: network.udpRemotePort
    property alias udpMulticastEnabled: network.udpMulticastEnabled
    property alias udpProcessDatagramsDirectly: network.udpProcessDatagramsDirectly
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
      currentIndex: Cpp_IO_Manager.selectedDriver
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
