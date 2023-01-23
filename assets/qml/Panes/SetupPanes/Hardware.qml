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
import QtCore as QtCore

import "Devices" as Devices
import "../../Windows" as Windows

Control {
    id: root

    //
    // Save settings
    //
    QtCore.Settings {
        property alias driver: _driverCombo.currentIndex
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

    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: app.spacing

        //
        // Device type selector
        //
        RowLayout {
            spacing: app.spacing
            Layout.fillWidth: true

            Label {
                text: qsTr("Data source") + ":"
                Layout.alignment: Qt.AlignVCenter
            }

            ComboBox {
                id: _driverCombo
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                model: Cpp_IO_Manager.availableDrivers()
                onCurrentIndexChanged: Cpp_IO_Manager.selectedDriver = currentIndex
            }
        }

        //
        // Device configuration
        //
        StackLayout {
            id: stack
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: Cpp_IO_Manager.selectedDriver

            Devices.Serial {
                id: serial
                Layout.fillWidth: true
                Layout.fillHeight: true
                background: TextField {
                    enabled: false
                }
            }

            Devices.Network {
                id: network
                Layout.fillWidth: true
                Layout.fillHeight: true
                background: TextField {
                    enabled: false
                }
            }

            Devices.BluetoothLE {
                id: bluetoothLE
                Layout.fillWidth: true
                Layout.fillHeight: true
                background: TextField {
                    enabled: false
                }
            }
        }
    }
}
