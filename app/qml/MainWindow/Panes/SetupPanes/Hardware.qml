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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtCore as QtSettings

import "Devices" as Devices

Item {
  id: root
  implicitHeight: layout.implicitHeight + 32

  //
  // Save settings
  //
  QtSettings.Settings {
    category: "DeviceSetup"

    property alias serialDtr: serial.dtr
    property alias serialParity: serial.parity
    property alias serialBaudRate: serial.baudRate
    property alias serialDataBits: serial.dataBits
    property alias serialStopBits: serial.stopBits
    property alias serialFlowControl: serial.flowControl
    property alias serialAutoReconnect: serial.autoReconnect

    property alias networkAddress: network.address
    property alias newtorkTcpPort: network.tcpPort
    property alias networkSocketType: network.socketType
    property alias networkUdpLocalPort: network.udpLocalPort
    property alias networkUdpRemotePort: network.udpRemotePort
    property alias networkUdpMulticastEnabled: network.udpMulticastEnabled
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
